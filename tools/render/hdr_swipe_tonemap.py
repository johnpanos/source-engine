#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Recreate swipe PNGs from linear HDR, fitting the original's SDR style.

One monotonic, bounded contrast/shoulder curve is shared by every pose. Each
pose's exposure matches the original image's luminance distribution. No spatial
correction or per-channel colour matching is applied. PFM archives and A stay
unchanged; b-engine.png retains the original engine preview.

python3 tools/render/hdr_swipe_tonemap.py --gallery <capture-directory>
Add --preview to fit and inspect a contact sheet before regenerating the gallery.
Requires NumPy and Pillow, as the other offline relighting tools do.
"""

import argparse
import datetime
import hashlib
import json
from pathlib import Path
import shutil
from types import SimpleNamespace

import numpy as np
from PIL import Image, ImageDraw, PngImagePlugin

import map_swipe_compare as swipe


LUMA = np.array([0.2126, 0.7152, 0.0722], dtype=np.float32)
QUANTILES = np.array([.02, .05, .10, .20, .30, .40, .50, .60, .70,
                      .80, .85, .90, .95, .98, .995])


def srgb_decode(rgb):
    return np.where(rgb <= .04045, rgb / 12.92, ((rgb + .055) / 1.055) ** 2.4)


def srgb_encode(rgb):
    rgb = np.maximum(rgb, 0)
    return np.where(rgb <= .0031308, rgb * 12.92, 1.055 * rgb ** (1 / 2.4) - .055)


def read_pfm(path):
    metadata = json.loads(Path(str(path) + ".json").read_text())
    if (metadata.get("schema") != "source-hdr-capture/v1" or
            metadata.get("encoding") != "linear" or metadata.get("primaries") != "Rec.709" or
            metadata.get("stage") != "scene-before-output" or
            metadata.get("reference_white_nits") != 203):
        raise ValueError("unknown HDR colour convention: " + str(path))
    with path.open("rb") as file:
        if file.readline().strip() != b"PF":
            raise ValueError("expected RGB PFM: " + str(path))
        width, height = map(int, file.readline().split())
        scale = float(file.readline())
        offset = file.tell()
    if not (0 < width <= 8192 and 0 < height <= 8192) or abs(scale) != 1:
        raise ValueError("invalid PFM extent or scale: " + str(path))
    if [width, height] != [metadata.get("width"), metadata.get("height")] or \
            path.stat().st_size != offset + width * height * 12:
        raise ValueError("incomplete PFM or mismatched metadata: " + str(path))
    # PFM stores its bottom row first. Keep a bounded memory-mapped view.
    pixels = np.memmap(path, dtype="<f4" if scale < 0 else ">f4", mode="r",
                       offset=offset, shape=(height, width, 3))
    return pixels[::-1], metadata


def tone_rgb(rgb, exposure, contrast):
    if not np.isfinite(rgb).all():
        raise ValueError("non-finite HDR pixels")
    rgb = np.maximum(rgb, 0)
    luminance = rgb @ LUMA
    # Zero remains black; the smooth shoulder approaches display white at
    # infinity. Exposure is linear; contrast is common to the complete gallery.
    mapped = -np.expm1(-(luminance * exposure) ** contrast)
    ratio = np.divide(mapped, luminance, out=np.zeros_like(mapped), where=luminance > 0)
    rgb = rgb * ratio[..., None]
    # Compress out-of-gamut chroma towards the mapped luminance, preserving
    # luminance and hue rather than clipping each channel independently.
    chroma = rgb - mapped[..., None]
    high = np.maximum(chroma.max(axis=-1), 1e-12)
    low = np.maximum(-chroma.min(axis=-1), 1e-12)
    saturation = np.minimum(1, np.minimum((1 - mapped) / high, mapped / low))
    rgb = mapped[..., None] + chroma * saturation[..., None]
    return np.rint(np.clip(srgb_encode(rgb), 0, 1) * 255).astype(np.uint8)


def samples(directory, receipt):
    raw, metadata = read_pfm(Path(receipt["right"]["hdr_export"]["path"]))
    stride = max(1, max(raw.shape[:2]) // 960)
    # Quantile matching depends on distributions, not corresponding pixels.
    rgb = np.array(raw[::stride, ::stride], dtype=np.float32)
    if not np.isfinite(rgb).all():
        raise ValueError("non-finite HDR calibration samples")
    with Image.open(directory / "a.png") as image:
        if image.size != (metadata["width"], metadata["height"]):
            raise ValueError("original/HDR dimensions differ")
        reference = np.asarray(image.convert("RGB").resize(
            (rgb.shape[1], rgb.shape[0]), Image.Resampling.LANCZOS), dtype=np.float32) / 255
    raw_q = np.quantile(np.maximum(rgb, 0) @ LUMA, QUANTILES)
    target_q = np.quantile(srgb_decode(reference) @ LUMA, QUANTILES)
    return rgb, reference, raw_q, target_q


def fit_curve(raw_quantiles, reference_quantiles):
    raw = np.asarray(raw_quantiles)
    target = np.asarray(reference_quantiles)
    if not np.isfinite(raw).all() or not np.isfinite(target).all():
        raise ValueError("non-finite calibration quantiles")
    if np.any(raw[:, 6] <= 1e-8):
        raise ValueError("cannot fit exposure to a black HDR scene")
    weights = np.ones(len(QUANTILES))
    weights[-2:] = .5  # Small bright effects should not set the whole exposure.
    offsets = np.linspace(-2, 2, 81)
    best = None
    for contrast in np.linspace(.6, 1.8, 241):
        inverse = (-np.log1p(-np.clip(target, 1e-8, 1 - 1e-6))) ** (1 / contrast)
        base = np.median((inverse / np.maximum(raw, 1e-8))[:, 3:12], axis=1)
        exposures = np.clip(base[:, None] * 2 ** offsets[None, :], 1 / 64, 64)
        predicted = -np.expm1(-(raw[:, None, :] * exposures[:, :, None]) ** contrast)
        errors = ((srgb_encode(predicted) - srgb_encode(target[:, None, :])) ** 2 *
                  weights).sum(axis=-1) / weights.sum()
        indices = errors.argmin(axis=1)
        score = float(errors[np.arange(len(raw)), indices].mean())
        if best is None or score < best[0]:
            best = (score, float(contrast), exposures[np.arange(len(raw)), indices])
    return best[1], best[2], best[0]


def digest_file(path):
    digest = hashlib.sha256()
    with path.open("rb") as file:
        for block in iter(lambda: file.read(8 * 1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def render_png(raw, path, exposure, contrast):
    height, width = raw.shape[:2]
    result = np.empty((height, width, 3), dtype=np.uint8)
    for y in range(0, height, 128):
        result[y:y + 128] = tone_rgb(np.array(raw[y:y + 128], dtype=np.float32),
                                     exposure, contrast)
    temporary = path.with_name(path.stem + "-temporary.png")
    info = PngImagePlugin.PngInfo()
    info.add(b"sRGB", b"\x00")
    info.add_text("ToneMapping", "original-style-sdr/v1; exposure=%.9g; contrast=%.9g" %
                  (exposure, contrast))
    Image.fromarray(result).save(temporary, pnginfo=info)
    temporary.replace(path)


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--gallery", type=Path, required=True)
    parser.add_argument("--preview", action="store_true")
    args = parser.parse_args(argv)
    gallery = args.gallery.resolve()
    entries = [(path.parent, json.loads(path.read_text()))
               for path in sorted(gallery.glob("*/comparison.json"))]
    if not entries:
        parser.error("no comparison receipts in gallery")
    calibrated = [samples(directory, receipt) for directory, receipt in entries]
    contrast, exposures, score = fit_curve([s[2] for s in calibrated],
                                          [s[3] for s in calibrated])
    mapping = {"schema": "original-style-sdr/v1", "curve": "1-exp(-(exposure*luminance)^contrast)",
               "contrast": contrast, "quantiles": QUANTILES.tolist(),
               "fit_encoded_luminance_rmse": float(np.sqrt(score)),
               "primaries": "Rec.709", "transfer": "sRGB", "output_bits": 8,
               "gamut": "luminance-preserving chroma compression",
               "processed_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
               "poses": {directory.name: {"exposure": float(exposure),
                                          "exposure_stops": float(np.log2(exposure))}
                         for (directory, _), exposure in zip(entries, exposures)}}
    print(json.dumps(mapping, indent=2), flush=True)
    if args.preview:
        # A / engine B / mapped B at representative poses, all in SDR.
        names = ("fizzler-door", "chamber-overview", "lightboard", "warm-front")
        chosen = [i for i, (directory, _) in enumerate(entries) if directory.name in names]
        sheet = Image.new("RGB", (1440, len(chosen) * 290), "#141719")
        for row, i in enumerate(chosen):
            directory, _ = entries[i]
            rgb, reference, _, _ = calibrated[i]
            mapped = tone_rgb(rgb, exposures[i], contrast)
            with Image.open(directory / "b.png") as image:
                engine = image.convert("RGB").resize((480, 270), Image.Resampling.LANCZOS)
            pictures = [Image.fromarray(np.rint(reference * 255).astype(np.uint8)),
                        engine, Image.fromarray(mapped)]
            for column, picture in enumerate(pictures):
                sheet.paste(picture.resize((480, 270), Image.Resampling.LANCZOS),
                            (column * 480, row * 290 + 20))
                ImageDraw.Draw(sheet).text((column * 480 + 8, row * 290 + 3),
                    directory.name + " / " + ("A original", "B engine", "B mapped")[column],
                    fill="white")
        sheet.save(gallery / "tone-map-preview.png")
        (gallery / "tone-map-preview.json").write_text(json.dumps(mapping, indent=2) + "\n")
        return 0
    for (directory, receipt), exposure in zip(entries, exposures):
        raw_path = Path(receipt["right"]["hdr_export"]["path"])
        raw, _ = read_pfm(raw_path)
        pose_mapping = mapping["poses"][directory.name]
        pose_mapping["reference_sha256"] = digest_file(directory / "a.png")
        pose_mapping["raw_sha256"] = digest_file(raw_path)
        pose_mapping["raw_metadata_sha256"] = digest_file(Path(str(raw_path) + ".json"))
        original = directory / "b-engine.png"
        if not original.exists():
            shutil.copy2(directory / "b.png", original)
            shutil.copy2(directory / "comparison.json", directory / "comparison-engine.json")
        render_png(raw, directory / "b.png", float(exposure), contrast)
        pose_mapping["output_sha256"] = digest_file(directory / "b.png")
        pose_mapping["engine_preview_sha256"] = digest_file(original)
        if digest_file(raw_path) != pose_mapping["raw_sha256"] or \
                digest_file(directory / "a.png") != pose_mapping["reference_sha256"]:
            raise ValueError("source images changed while tone mapping")
        receipt["right"]["offline_sdr"] = {
            "schema": mapping["schema"], "curve": mapping["curve"], "contrast": contrast,
            "exposure": float(exposure), "transfer": "sRGB", "mapping": str(gallery / "tone-map.json"),
            "reference": str(directory / "a.png"), "raw_source": str(raw_path),
            "previous_engine_preview": str(original), "processed_utc": mapping["processed_utc"]}
        swipe.write_comparison(SimpleNamespace(panel_relay=receipt["activated_panel_relays"]),
                               directory, receipt["left"], receipt["right"],
                               (receipt["requested_camera"]["origin"],
                                receipt["requested_camera"]["angles"]),
                               captured_utc=receipt["captured_utc"])
    cards = []
    labels = {name: label for name, label, _ in swipe.INTRO4_CAPTURES}
    for directory, _ in entries:
        label = labels.get(directory.name, directory.name)
        cards.append('<a href="%s/compare.html"><img loading="lazy" src="%s/b.png" alt="%s">%s</a>' %
                     (directory.name, directory.name, swipe.html.escape(label), swipe.html.escape(label)))
    swipe.write_gallery(gallery, cards, raw.shape[1], raw.shape[0],
                        processing_note="PNG previews use offline tone mapping fitted to the original A images.")
    (gallery / "tone-map.json").write_text(json.dumps(mapping, indent=2) + "\n")
    print(gallery / "index.html", flush=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
