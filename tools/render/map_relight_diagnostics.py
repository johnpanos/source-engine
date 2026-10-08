#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Capture matched map lighting views and a source-light inventory.

Example:
  python3 tools/render/map_relight_diagnostics.py sp_a2_laser_intro \
      sp_a2_laser_intro_source2 --pose=-1312,0,-208:0,0,0 \
      --roi 'stairs:0.44,0.30,0.58,0.65' \
      --out quality-results/map-diagnostics/laser-entry

Each view starts a fresh game process at the same verified camera. RGB samples
decode the screenshot's sRGB bytes for *relative* inspection; final-color
samples still include the game's tone map at a fixed exposure, so they are not
bake radiance measurements. The source-light list is provenance, not a claim
about which lights reached a particular pixel.
"""

import argparse
import datetime
import html
import hashlib
import json
import math
import os
from pathlib import Path
import re
import subprocess
import sys

import numpy as np
from PIL import Image

import map_swipe_compare as swipe


MODES = {
    "original": ("Original final", (), "original"),
    "final": ("Relit final", (), "relit"),
    "albedo": ("Relit albedo", ("cl_render_debug_view 1", "cl_render_debug_legacy 2"),
               "relit"),
    "baked": ("Relit baked irradiance", ("cl_render_debug_view 8",
                                         "cl_render_debug_legacy 2"), "relit"),
    "runtime-direct": ("Relit runtime direct", ("cl_render_debug_view 9",
                                               "cl_render_debug_legacy 2"), "relit"),
    "image-specular": ("Relit image specular", ("cl_render_debug_view 10",
                                                 "cl_render_debug_legacy 2"), "relit"),
    "baked-off": ("Relit without baked term", ("cl_render_debug_term baked",), "relit"),
    "probes-off": ("Relit without diffuse probes", ("cl_render_debug_term probes",),
                   "relit"),
    "ibl-off": ("Relit without image specular", ("cl_render_debug_term ibl",), "relit"),
    # Rank channels are IDs: MSAA resolve would blend different IDs at edges.
    # Keep final-color captures at the normal sample count; this readback is point sampled.
    "probe-selection": ("Reflection probe selection", ("mat_antialias 0",
                                                        "cl_render_debug_view 24",
                                                        "cl_render_debug_legacy 2"), "relit"),
    "probe-radiance": ("Raw reflection probe radiance", ("cl_render_debug_view 25",
                                                         "cl_render_debug_legacy 2"), "relit"),
    "probe-weight": ("Reflection probe specular weight", ("cl_render_debug_view 26",
                                                            "cl_render_debug_legacy 2"), "relit"),
}
DEFAULT_MODES = tuple(MODES)[:9]


def parse_roi(value):
    try:
        name, coords = value.split(":", 1)
        box = tuple(float(part) for part in coords.split(","))
    except ValueError as error:
        raise argparse.ArgumentTypeError("ROI must be name:x0,y0,x1,y1") from error
    if not name or len(box) != 4 or not all(math.isfinite(part) for part in box) or \
            not (0 <= box[0] < box[2] <= 1 and 0 <= box[1] < box[3] <= 1):
        raise argparse.ArgumentTypeError("ROI coordinates must satisfy 0 <= low < high <= 1")
    return name, box


def roi_metrics(image, box, exclude_hatch=False):
    width, height = image.size
    bounds = (int(box[0] * width), int(box[1] * height),
              max(int(box[0] * width) + 1, int(box[2] * width)),
              max(int(box[1] * height) + 1, int(box[3] * height)))
    pixels = np.asarray(image.crop(bounds).convert("RGB"))
    hatch = (np.all(pixels == 137, axis=2) | np.all(pixels == 188, axis=2)) \
        if exclude_hatch else np.zeros(pixels.shape[:2], dtype=bool)
    encoded = pixels.astype(np.float64) / 255.0
    linear = np.where(encoded <= 0.04045, encoded / 12.92,
                      ((encoded + 0.055) / 1.055) ** 2.4)
    valid = ~hatch
    mean = linear[valid].mean(axis=0) if valid.any() else np.zeros(3)
    return {"linearized_mean_rgb": [float(value) for value in mean] if valid.any() else None,
            "red_blue_ratio": float(mean[0] / mean[2]) if mean[2] > 0 and \
                mean.max() >= 0.001 else None,
            "clipped_pixel_fraction": float(np.any(encoded[valid] >= 1.0, axis=1).mean())
            if valid.any() else None,
            "hatch_fraction": float(hatch.mean())}


def probe_selection_metrics(image, box, global_rank=None):
    maximum = 64 if global_rank is not None and global_rank >= 16 else 16
    divisor = 2.0 * maximum
    width, height = image.size
    pixels = np.asarray(image.convert("RGB"))[int(box[1] * height):int(box[3] * height),
                                               int(box[0] * width):int(box[2] * width)]
    encoded = pixels.astype(np.float64) / 255.0
    linear = np.where(encoded <= 0.04045, encoded / 12.92,
                      ((encoded + 0.055) / 1.055) ** 2.4)
    hatch = np.all(pixels == 137, axis=2) | np.all(pixels == 188, axis=2)
    fallback = ((pixels[:, :, 0] == 255) & (pixels[:, :, 2] == 255)) | \
        ((pixels[:, :, 0] == 0) & (pixels[:, :, 1] == 255)) | \
        ((pixels[:, :, 0] == 255) & (pixels[:, :, 1] == 255)) | \
        ((pixels[:, :, 0] == 0) & (pixels[:, :, 1] == 0) &
         (pixels[:, :, 2] == 255))
    first = np.rint(linear[:, :, 0] * divisor - 1.0).astype(int)
    second = np.rint(linear[:, :, 1] * divisor - 1.0).astype(int)
    weight = linear[:, :, 2]
    rank_values = np.arange(maximum + 1) / divisor
    rank_bytes = np.rint(np.where(rank_values <= 0.0031308, rank_values * 12.92,
                                  1.055 * rank_values ** (1 / 2.4) - 0.055) * 255)
    valid = (~hatch & ~fallback & (first >= 0) & (first < maximum) &
             (second >= -1) & (second < maximum) & (first != second))
    for channel, ranks in ((0, first), (1, second)):
        expected = rank_bytes[np.clip(ranks + 1, 0, maximum)]
        valid &= np.abs(pixels[:, :, channel].astype(float) - expected) <= 1
    # No second probe requires full first weight (one byte of display tolerance).
    valid &= (second >= 0) | (pixels[:, :, 2] >= 254)
    invalid = ~hatch & ~fallback & ~valid
    ranks, counts = np.unique(first[valid], return_counts=True)
    total = int(valid.sum())
    global_weight = np.where(first == global_rank, weight, 0.0) + \
        np.where(second == global_rank, 1.0 - weight, 0.0) if global_rank is not None else None
    return {"rank_fraction": {str(int(rank)): float(count / total)
                              for rank, count in zip(ranks, counts)},
            "mean_first_weight": float(weight[valid].mean()) if total else None,
            "mean_global_weight": float(global_weight[valid].mean())
            if global_weight is not None and total else None,
            "fallback_fraction": float((fallback & ~hatch).sum() / (~hatch).sum())
            if (~hatch).any() else None,
            "invalid_pixels": int(invalid.sum()), "sampled_pixels": total}


def parse_global_limit(value):
    try:
        name, number = value.rsplit(":", 1)
        limit = float(number)
        if name and math.isfinite(limit) and 0 <= limit <= 1:
            return name, limit
    except ValueError:
        pass
    raise argparse.ArgumentTypeError("global weight limit must be region:0..1")


def check_probe_regions(receipt, directory, limits):
    """Check actual shader selection; absent/invalid/fallback pixels cannot certify it."""
    checks = []
    record = receipt.get("modes", {}).get("probe-selection")
    layout = receipt.get("probe_layout") or {}
    rank = layout.get("global_rank")
    image = None
    if record:
        with Image.open(directory / record["image"]) as source:
            image = source.convert("RGB")
    for name, limit in limits.items():
        box = receipt.get("regions", {}).get(name)
        sample = probe_selection_metrics(image, box, rank) if image and box else {}
        measured = sample.get("mean_global_weight")
        passed = (isinstance(rank, int) and 0 <= rank < 64 and
                  sample.get("sampled_pixels", 0) > 0 and
                  sample.get("invalid_pixels") == 0 and sample.get("fallback_fraction") == 0 and
                  measured is not None and measured <= limit)
        checks.append({"region": name, "status": "pass" if passed else "fail",
                       "max_mean_global_weight": limit, "global_rank": rank, **sample})
    return {"schema": "map-probe-selection-checks/v1",
            "status": "pass" if checks and all(c["status"] == "pass" for c in checks)
            else "fail", "checks": checks}


def check_existing(argv):
    parser = argparse.ArgumentParser(description="Check saved native probe-selection captures")
    parser.add_argument("gallery", type=Path)
    parser.add_argument("--max-global-weight", type=parse_global_limit, action="append",
                        required=True, help="region:max mean global weight; repeatable")
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args(argv)
    try:
        path = args.gallery / "diagnostics.json"
        receipt = json.loads(path.read_text())
        result = check_probe_regions(receipt, args.gallery, dict(args.max_global_weight))
        result["source_receipt"] = str(path.resolve())
        result["source_receipt_sha256"] = hashlib.sha256(path.read_bytes()).hexdigest()
        selection = receipt.get("modes", {}).get("probe-selection")
        if selection:
            result["selection_image_sha256"] = hashlib.sha256(
                (args.gallery / selection["image"]).read_bytes()).hexdigest()
        args.out.parent.mkdir(parents=True, exist_ok=True)
        args.out.write_text(json.dumps(result, indent=2) + "\n")
        print(json.dumps(result, indent=2))
        return 0 if result["status"] == "pass" else 1
    except (OSError, ValueError, KeyError) as error:
        print("map_relight_diagnostics check: " + str(error), file=sys.stderr)
        return 1


def source_lights(content):
    if content is None:
        return None
    receipt = content.parent / "legacy-scene/scene-receipt.json"
    if not receipt.is_file():
        return None
    data = json.loads(receipt.read_text())
    result = []
    for light in data.get("lights", []):
        rgb = light.get("radiance", light.get("irradiance"))
        if not rgb:
            continue
        result.append({"world_light": light.get("world_light"),
                       "hammerid": light.get("hammerid"), "type": light.get("type"),
                       "rgb": rgb, "red_blue_ratio": rgb[0] / rgb[2] if rgb[2] > 0 else None})
    return {"receipt": str(receipt), "lights": result}


def probe_layout(content):
    receipt = content.parent / "lighting/reflection_probes.rprb.json"
    if not receipt.is_file():
        return None
    data = json.loads(receipt.read_text())
    count = data.get("probes")
    index = data.get("global_index")
    fits = data.get("fits")
    if not isinstance(count, int) or not isinstance(index, int) or \
            not isinstance(fits, list) or not 1 <= count <= 64 or \
            not 0 <= index < len(fits):
        return None
    return {"receipt": str(receipt), "count": count, "global_rank": count - 1,
            "global_source_index": index, "global_capture": fits[index].get("capture")}


def captured_tonemap_scale(evidence_path):
    log = Path(evidence_path).parent / "runtime/engine.log"
    matches = re.findall(r'"mat_hdr_tonemapscale" = "([0-9.eE+-]+)"', log.read_text())
    if len(matches) != 1:
        raise ValueError("cannot identify captured tone-map scale in " + str(log))
    return float(matches[0])


def bake_layer_previews(content, out):
    layers = content.parent / "lighting/layers"
    paths = {name: layers / (name + "-denoised.exr") for name in ("direct", "indirect")}
    if not all(path.is_file() for path in paths.values()):
        return None
    import OpenImageIO as oiio

    result = {}
    for name, path in paths.items():
        source = oiio.ImageBuf(str(path))
        resized = oiio.ImageBufAlgo.resize(source, roi=oiio.ROI(0, 512, 0, 512, 0, 1, 0, 4))
        if resized.has_error:
            raise ValueError("cannot preview bake layer %s: %s" % (path, resized.geterror()))
        linear = np.maximum(resized.get_pixels(oiio.FLOAT)[:, :, :3], 0.0)
        exposed = linear * 4.0
        mapped = exposed / (1.0 + exposed)
        encoded = np.where(mapped <= 0.0031308, mapped * 12.92,
                           1.055 * mapped ** (1.0 / 2.4) - 0.055)
        preview = "bake-" + name + ".png"
        Image.fromarray(np.uint8(np.clip(encoded * 255 + 0.5, 0, 255))).save(out / preview)
        result[name] = {"source_exr": str(path), "preview": preview,
                        "display": "shared 4x exposure, Reinhard, sRGB; atlas UV space"}
    return result


def make_html(out, records, regions, sources, bake_layers=None, probes=None, probe_checks=None):
    cards = []
    exposures = []
    for mode, record in records.items():
        label = html.escape(MODES[mode][0])
        cards.append('<figure><a href="%s"><img src="%s" alt="%s"></a><figcaption>%s'
                     '</figcaption></figure>' % (record["image"], record["image"], label, label))
        if mode in ("original", "final", "baked-off", "probes-off", "ibl-off") and \
                "captured_tonemap_scale" in record:
            exposures.append("%s %.3f" % (label, record["captured_tonemap_scale"]))
    rows = []
    for name in regions:
        for mode, record in records.items():
            if mode in ("probe-selection", "probe-weight"):
                continue
            sample = record["regions"][name]
            ratio = sample["red_blue_ratio"]
            rows.append("<tr><td>%s</td><td>%s</td><td>%s</td><td>%s</td><td>%.1f%%</td></tr>" % (
                html.escape(name), html.escape(MODES[mode][0]),
                "—" if ratio is None else "%.2f" % ratio,
                "—" if sample["clipped_pixel_fraction"] is None else
                "%.1f%%" % (100 * sample["clipped_pixel_fraction"]),
                100 * sample["hatch_fraction"]))
    lights = ""
    selections = ""
    if "probe-selection" in records:
        selections = "<h2>Reflection probe selection</h2><table><tr><th>Region</th>" \
                     "<th>First probe ranks</th><th>Mean first weight</th>" \
                     "<th>Fallback</th></tr>"
        for name, sample in records["probe-selection"]["probe_selection"].items():
            ranks = ", ".join("%s: %.1f%%" % (rank, 100 * fraction)
                              for rank, fraction in sorted(sample["rank_fraction"].items(),
                                                            key=lambda item: -item[1]))
            selections += "<tr><td>%s</td><td>%s</td><td>%s</td><td>%s</td></tr>" % (
                html.escape(name), ranks or "—",
                "—" if sample["mean_first_weight"] is None else
                "%.3f" % sample["mean_first_weight"],
                "—" if sample["fallback_fraction"] is None else
                "%.1f%%" % (100 * sample["fallback_fraction"]))
        selections += "</table>"
        if probes:
            selections += "<p>Global sky probe: rank %d, source index %d, capture %s.</p>" % (
                probes["global_rank"], probes["global_source_index"],
                html.escape(str(probes["global_capture"])))
    if sources:
        ordered = sorted(sources["lights"], key=lambda light: light["world_light"])
        lights = "<h2>Source lights</h2><p>Exported source colors; not ranked by pixel " \
                 "contribution.</p>"
        lights += "<table><tr><th>World light</th><th>Hammer ID</th><th>Type</th>" \
                  "<th>Linear RGB</th><th>R/B</th></tr>"
        for light in ordered:
            lights += "<tr><td>%s</td><td>%s</td><td>%s</td><td>%s</td><td>%s</td></tr>" % (
                html.escape(str(light["world_light"])),
                html.escape(str(light["hammerid"])), html.escape(str(light["type"])),
                html.escape(", ".join("%.3g" % part for part in light["rgb"])),
                "—" if light["red_blue_ratio"] is None else
                "%.2f" % light["red_blue_ratio"])
        lights += "</table>"
    bake = ""
    if bake_layers:
        bake = "<h2>Bake layers</h2><p>Direct and indirect lightmap atlases in UV space, " \
               "shown at the same display exposure. These are not camera views.</p><section>"
        for name, layer in bake_layers.items():
            label = html.escape(name.capitalize() + " bake")
            bake += '<figure><a href="%s"><img src="%s" alt="%s"></a><figcaption>%s' \
                    '</figcaption></figure>' % (layer["preview"], layer["preview"], label, label)
        bake += "</section>"
    check_summary = ""
    if probe_checks:
        check_summary = "<h2>Native probe checks: %s</h2><ul>" % probe_checks["status"]
        for check in probe_checks["checks"]:
            check_summary += "<li>%s: %s; mean global weight %s, limit %.3f</li>" % (
                html.escape(check["region"]), check["status"],
                str(check.get("mean_global_weight")), check["max_mean_global_weight"])
        check_summary += "</ul>"
    page = """<!doctype html><html lang="en"><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1"><title>Map relight diagnostics</title>
<style>body{font:16px system-ui,sans-serif;max-width:1500px;margin:auto;padding:24px;
background:#111619;color:#edf3f5}p,figcaption{color:#b8c7ce}section{display:grid;
grid-template-columns:repeat(auto-fit,minmax(400px,1fr));gap:14px}figure{margin:0}
img{width:100%;height:auto;border:1px solid #405157}figcaption{margin:5px 0 15px}
table{border-collapse:collapse}td,th{padding:6px 14px;border:1px solid #405157;text-align:left}</style>
<h1>Map relight diagnostics</h1><p>Each frame uses a fresh native Vulkan boot at the same
verified camera and fixed final-frame exposure. Grey hatching in debug views marks content the core does not draw.
The baked view is the total lightmap; runtime direct excludes its baked direct layer.
Final frames include exposure and tone mapping. ROI RGB is decoded from display PNGs,
so its ratios are diagnostic, not measured scene radiance.</p><p>Recorded final-frame
tone-map scales: EXPOSURES</p>PROBECHECKS<section>CARDS</section>
<h2>Selected regions</h2><table><tr><th>Region</th><th>View</th><th>Mean R/B</th>
<th>Clipped pixels</th><th>Hatched pixels</th></tr>ROWS</table>SELECTIONSBAKELIGHTLIGHTS</html>"""
    (out / "index.html").write_text(page.replace("EXPOSURES", ", ".join(exposures) or "not recorded")
                                     .replace("PROBECHECKS", check_summary).replace("CARDS", "".join(cards))
                                     .replace("ROWS", "".join(rows)).replace("BAKELIGHT", bake)
                                     .replace("SELECTIONS", selections)
                                     .replace("LIGHTS", lights))


def run(args, out):
    if any(out.iterdir()):
        allowed = {"dbus", "compositor.log"}
        if any(item.name not in allowed for item in out.iterdir()):
            raise ValueError("output directory is not empty: " + str(out))
    content = swipe.published_content(args.relit_map)
    if content is None:
        raise ValueError("relit map has no published content: " + args.relit_map)
    sources = source_lights(content)
    probes = probe_layout(content)
    bake_layers = bake_layer_previews(content, out)
    regions = dict(args.roi) if args.roi else {"whole_frame": (0, 0, 1, 1)}
    records = {}
    for mode in args.modes:
        _, commands, target = MODES[mode]
        map_name = args.original_map if target == "original" else args.relit_map
        selected_content = None if target == "original" else content
        startup = ("mat_force_tonemap_scale %.9g" % args.tone_map_scale,) + commands
        record = swipe.capture(map_name, mode, selected_content, args, out, startup)
        record["captured_tonemap_scale"] = captured_tonemap_scale(record["boot_evidence"])
        if mode in ("original", "final", "baked-off", "probes-off", "ibl-off") and \
                abs(record["captured_tonemap_scale"] - args.tone_map_scale) > 0.001:
            raise ValueError("%s final exposure did not lock to %.6g: %.6g" % (
                mode, args.tone_map_scale, record["captured_tonemap_scale"]))
        image = Image.open(out / record["image"])
        debug_view = mode in ("albedo", "baked", "runtime-direct", "image-specular",
                              "probe-selection", "probe-radiance", "probe-weight",
                              "probe-header")
        record["regions"] = {name: roi_metrics(image, box, debug_view)
                             for name, box in regions.items()}
        if mode == "probe-selection":
            record["probe_selection"] = {name: probe_selection_metrics(image, box,
                                             probes["global_rank"] if probes else None)
                                         for name, box in regions.items()}
        records[mode] = record
    cameras = [record["camera"] for record in records.values()]
    if max(camera["fov"] for camera in cameras) - min(camera["fov"] for camera in cameras) > 0.1:
        raise ValueError("captured camera FOV differs between modes")
    receipt = {"schema": "map-relight-diagnostics/v1", "original_map": args.original_map,
               "relit_map": args.relit_map, "requested_camera": args.pose,
               "requested_tonemap_scale": args.tone_map_scale,
               "regions": regions, "modes": records, "source_lights": sources,
               "bake_layers": bake_layers, "probe_layout": probes,
               "captured_utc": datetime.datetime.now(datetime.timezone.utc).isoformat()}
    if args.max_global_weight:
        receipt["probe_checks"] = check_probe_regions(receipt, out, dict(args.max_global_weight))
    (out / "diagnostics.json").write_text(json.dumps(receipt, indent=2) + "\n")
    make_html(out, records, regions, sources, bake_layers, probes, receipt.get("probe_checks"))
    if receipt.get("probe_checks", {}).get("status") == "fail":
        raise ValueError("native probe-selection checks failed; see diagnostics.json")


def main(argv=None):
    argv = list(sys.argv[1:] if argv is None else argv)
    if argv and argv[0] == "check":
        return check_existing(argv[1:])
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("original_map")
    parser.add_argument("relit_map")
    parser.add_argument("--pose", required=True, type=swipe.parse_pose)
    parser.add_argument("--roi", action="append", type=parse_roi, default=[])
    parser.add_argument("--max-global-weight", type=parse_global_limit, action="append",
                        default=[], help="fail if region exceeds mean global probe weight")
    parser.add_argument("--modes", nargs="+", choices=tuple(MODES), default=DEFAULT_MODES)
    swipe.sepipe_loader.add_arguments(parser, "portal2-fsr")
    parser.add_argument("--width", type=int, default=1920)
    parser.add_argument("--height", type=int, default=1080)
    parser.add_argument("--tone-map-scale", type=float, default=4.0,
                        help="fixed final-frame tone-map scale (default 4)")
    parser.add_argument("--timeout", type=int, default=900)
    parser.add_argument("--panel-relay", action="append", default=[])
    parser.add_argument("--out", required=True, type=Path)
    parser.add_argument("--in-compositor", action="store_true", help=argparse.SUPPRESS)
    args = parser.parse_args(argv)
    if any(not re.fullmatch(r"[A-Za-z0-9_]+", name)
           for name in (args.original_map, args.relit_map)):
        parser.error("map names may contain only letters, digits and _")
    if any(not re.fullmatch(r"[A-Za-z0-9_@-]+", name) for name in args.panel_relay):
        parser.error("panel relay names may contain only letters, digits, _, @, and -")
    if not (64 <= args.width <= 8192 and 64 <= args.height <= 8192):
        parser.error("capture dimensions must be between 64 and 8192")
    if args.timeout < 1 or len(set(args.modes)) != len(args.modes):
        parser.error("timeout must be positive and modes must be unique")
    if not math.isfinite(args.tone_map_scale) or args.tone_map_scale <= 0:
        parser.error("tone-map scale must be finite and positive")
    if len(set(name for name, _ in args.roi)) != len(args.roi):
        parser.error("ROI names must be unique")
    if args.max_global_weight and ("probe-selection" not in args.modes or
            any(name not in dict(args.roi) for name, _ in args.max_global_weight)):
        parser.error("global weight checks require probe-selection mode and named --roi regions")
    out = args.out.resolve()
    out.mkdir(parents=True, exist_ok=True)
    if not args.in_compositor:
        command = [sys.executable, str(Path(__file__).resolve()),
                   *(argv if argv is not None else sys.argv[1:]), "--in-compositor"]
        returncode = swipe.sepipe_loader.run_under_display(
            "private", out / "display", (args.width, args.height, 60), command, os.environ,
            out / "compositor.log", args.timeout * len(args.modes) + 120)
        if returncode:
            print("map_relight_diagnostics: see " + str(out / "compositor.log"), file=sys.stderr)
        elif (out / "index.html").is_file():
            print(out / "index.html")
        return returncode
    try:
        run(args, out)
        print(out / "index.html")
        return 0
    except (OSError, ValueError, KeyError, subprocess.SubprocessError) as error:
        print("map_relight_diagnostics: " + str(error), file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
