#!/usr/bin/env python3
"""Package a checked Cycles diffuse lightmap atlas as the map's LMAP v3 lump.

The pages are staged and stitched as linear RGBA16F, written as a lossless
KTX2 master (`--out`, the bake's intermediate and the oracles' exact view),
then encoded as LMAP v3 (`--lmap-out`, world_lightmap_v3.py): the only
lightmap the runtime reads.

Used by the PBRT map pipeline's ktx2 step; `--expected-scope` names the bake
receipt producer this package trusts (for example
`pbrt-shared-lightmap-uv-and-cycles-bake-denoised`).

With `--directional-exr` (from `lightmap_directional.py`) the page is twice as
wide as it is tall: the flat irradiance atlas on the left and the per-texel
world-space luminance gradient beta on the right, at the same texel rows.
`world_pbr.frag` recognises the 2:1 page and samples both halves at the same
lightmap coordinate.

With `--sun-visibility` (the bake's sun mask) the flat half's alpha holds the
sun's [0, 1] visibility, gutter-filled from `--coverage-exr` (the render core's
surface program shadows the sun's runtime light with it; the sun itself is
the map's light_environment). Its diffuse light is already in the bake.

With `--seams` (lightmap_seams.py extract) every page - total, directional
gradient, sun visibility and separated layers - is stitched before encoding:
its bilinear lookups agree on both sides of every lightmap chart seam, and
the package fails when a stitched page's seam discontinuity exceeds
`--max-seam-p99` or `--max-seam`. The receipt records each page's seam
statistics before and after.

With `--layer ROLE=EXR` (denoised separated light from the bake's `--layers`)
the package has layers (public/mapcontainer/world_lightmap.h): layer 0 is
the total page above; then `indirect` (two layers) or `direct` and
`indirect` (three), each the flat light only, with no sun. On a directional page a layer's gradient half is zero, except the
indirect layer's with `--layer-directional indirect=EXR` (its beta from
`lightmap_directional.py --layer indirect`): the core then draws the
indirect layer as a directional page under every light's direct light
(RFC 0016's runtime direct light). Without `--directional-exr` the page is
still directional then, with the total's gradient half zero.
"""

import argparse
import hashlib
import json
import os
import subprocess
import sys
import tempfile
from pathlib import Path

import imageio.v3 as iio
sys.path.insert(0, str(Path(__file__).resolve().parent))
import numpy as np


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def run(command):
    return subprocess.run(command, check=True, capture_output=True, text=True).stdout


# Seam gate: relative luminance discontinuity across stitched chart seams.
DEFAULT_MAX_SEAM_P99 = 0.002
DEFAULT_MAX_SEAM = 0.02


def seam_stitcher(args):
    """A function (page name, EXR-oriented image) -> stitched image, with a
    `record` of per-page seam statistics; the identity without --seams."""
    if not args.seams:
        identity = lambda name, image, reference=None, absolute=False: image  # noqa: E731
        identity.record = None
        return identity
    if not args.coverage_exr:
        raise ValueError("--seams needs --coverage-exr")
    import sys
    sys.path.insert(0, str(Path(__file__).resolve().parent))
    import lightmap_seams
    found = lightmap_seams.load_seams(args.seams)
    covered = iio.imread(args.coverage_exr)[:, :, :3].min(axis=2) > 0.5
    if covered.shape != (found["size"], found["size"]):
        raise ValueError("coverage and seam samples are for different atlas sizes")
    buried = None
    if args.buried_exr:
        raw = iio.imread(args.buried_exr)
        if raw.shape[:2] != covered.shape:
            raise ValueError("--buried-exr is not atlas-sized")
        buried = lightmap_seams.buried_texels(raw, covered)
    record = {"seams_sha256": sha256(args.seams), "samples": int(len(found["uv_a"])),
              "buried_texels": int(buried.sum()) if buried is not None else None,
              "gate": {"p99": args.max_seam_p99, "max": args.max_seam}, "pages": {}}
    system = lightmap_seams.stitch_system(found, covered, buried) if len(found["uv_a"]) else None
    refined_system = None

    def stitch(name, image, reference=None, absolute=False):
        """Stitch one page; light pages are judged relative to `reference`
        (the stitched total for a separated layer), `absolute` pages by value."""
        nonlocal refined_system
        if system is None:
            record["pages"][name] = {"stitched": False}
            return image
        before = lightmap_seams.measure(image, found, covered, reference, absolute)
        result = lightmap_seams.stitch(image, system)
        after = lightmap_seams.measure(result, found, covered, reference, absolute)
        recovered = False
        if after["p99"] > args.max_seam_p99 or after["max"] > args.max_seam:
            # A seam is an equality constraint across two filter footprints.
            # Preserve the usual fixed-point behavior first, then let padding
            # absorb more correction only when the measured runtime invariant
            # still fails. This repairs the atlas; it never relaxes the gate.
            if refined_system is None:
                refined_system = lightmap_seams.stitch_system(
                    found, covered, buried, covered_weight=0.03, gutter_weight=1e-5)
            result = lightmap_seams.stitch(image, refined_system)
            after = lightmap_seams.measure(result, found, covered, reference, absolute)
            recovered = True
        record["pages"][name] = {"before": before, "after": after,
                                 "refined": recovered}
        if after["p99"] > args.max_seam_p99 or after["max"] > args.max_seam:
            finding = ("%s page: stitched seam discontinuity p99 %.4g / max %.4g exceeds the "
                       "gate (%g / %g)" % (name, after["p99"], after["max"], args.max_seam_p99,
                                           args.max_seam))
            if not args.record_seam_failure:
                raise ValueError(finding)
            # Reported, not fatal (map bakes, user decision 2026-10-05).
            record["pages"][name]["gate_failed"] = True
            record.setdefault("findings", []).append(finding)
            print("GATE FINDING (reported, not fatal): " + finding, file=sys.stderr)
        return result

    stitch.record = record
    return stitch


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exr", type=Path, required=True)
    parser.add_argument("--bake-evidence", type=Path, required=True)
    parser.add_argument("--lighting-stage", type=Path, required=True)
    parser.add_argument("--ktx-tool", type=Path, required=True)
    parser.add_argument("--preview-gain", type=float, default=1.0,
                        help="linear lightmap gain for the Source preview renderer")
    parser.add_argument("--expected-scope", required=True,
                        help="bake receipt scope, e.g. pbrt-shared-lightmap-uv-and-cycles-bake")
    parser.add_argument("--directional-exr", type=Path,
                        help="beta page from lightmap_directional.py (receipt beside it)")
    parser.add_argument("--sun-visibility", type=Path,
                        help="sun visibility EXR named by the bake receipt's `sun`")
    parser.add_argument("--coverage-exr", type=Path,
                        help="UV coverage used to fill the visibility's gutters")
    parser.add_argument("--sun-bake-evidence", type=Path,
                        help="the bake receipt whose `sun` names the visibility EXR")
    parser.add_argument("--light-masks", type=Path,
                        help="static lights' shadow masks EXR (pbrt_lightmap_bake.py "
                             "--masks-only of --lighting-stage; receipt beside it; needs "
                             "--coverage-exr and --lsmk-out)")
    parser.add_argument("--lsmk-out", type=Path,
                        help="write the LSMK lump (light_shadow_masks.py) of --light-masks")
    parser.add_argument("--layer", action="append", default=[], metavar="ROLE=EXR",
                        help="separated-light layer (direct, indirect) and its denoised EXR")
    parser.add_argument("--layer-directional", action="append", default=[],
                        metavar="ROLE=EXR",
                        help="a separated layer's beta page (indirect only), from "
                             "lightmap_directional.py --layer (receipt beside it)")
    parser.add_argument("--seams", type=Path,
                        help="lightmap-seams/v1 samples of the lighting stage (needs --coverage-exr)")
    parser.add_argument("--buried-exr", type=Path,
                        help="raw (undenoised) total bake: its covered black texels are hidden "
                             "surfaces that stitching may move freely")
    parser.add_argument("--max-seam-p99", type=float, default=DEFAULT_MAX_SEAM_P99,
                        help="gate: stitched seam discontinuity 99th percentile (relative)")
    parser.add_argument("--record-seam-failure", action="store_true",
                        help="record a seam gate miss in the receipt instead of failing")
    parser.add_argument("--max-seam", type=float, default=DEFAULT_MAX_SEAM,
                        help="gate: stitched seam discontinuity maximum (relative)")
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--lmap-out", type=Path,
                        help="also write the LMAP v3 lump (world_lightmap_v3.py): BC6H light "
                             "and BC7 gradient pages, the sun in the gradient's alpha")
    args = parser.parse_args()
    stitch = seam_stitcher(args)
    separated = {}
    for item in args.layer:
        role, _, path = item.partition("=")
        if role not in ("direct", "indirect") or not path or role in separated:
            parser.error("--layer takes distinct direct=EXR / indirect=EXR entries")
        separated[role] = Path(path)
    layer_betas = {}
    for item in args.layer_directional:
        role, _, path = item.partition("=")
        if role != "indirect" or not path or role in layer_betas or role not in separated:
            parser.error("--layer-directional takes indirect=EXR, beside --layer indirect=EXR")
        layer_betas[role] = Path(path)
    # Roles follow the layer count (world_lightmap.h): total, [direct,] indirect.
    order = {(): [], ("indirect",): ["indirect"],
             ("direct", "indirect"): ["direct", "indirect"]}.get(tuple(sorted(separated)))
    if order is None:
        parser.error("separated layers are indirect, or direct and indirect")
    evidence = json.loads(args.bake_evidence.read_text())
    if (evidence.get("status") != "pass" or evidence.get("scope") != args.expected_scope or
            evidence.get("atlas_exr_sha256") != sha256(args.exr) or
            evidence.get("lighting_stage_sha256") != sha256(args.lighting_stage)):
        raise ValueError("Cycles atlas differs from its authored USD receipt")
    pixels = iio.imread(args.exr)
    if (pixels.shape != (evidence["size"], evidence["size"], 4) or
            not np.isfinite(pixels).all() or np.min(pixels[:, :, :3]) < 0):
        raise ValueError("Cycles atlas has invalid dimensions or pixels")
    pixels = stitch("total", pixels)
    if not np.isfinite(args.preview_gain) or not 0 < args.preview_gain <= 1:
        raise ValueError("preview gain must be finite and in (0, 1]")
    size = evidence["size"]
    width = size
    directional = None
    if args.directional_exr:
        directional = json.loads(Path(str(args.directional_exr) + ".json").read_text())
        if (directional.get("status") != "pass" or
                directional.get("directional_exr_sha256") != sha256(args.directional_exr) or
                directional.get("flat_exr_sha256") != sha256(args.exr) or
                directional.get("size") != size):
            raise ValueError("directional page differs from its receipt or flat atlas")
        beta = iio.imread(args.directional_exr)
        if beta.shape != pixels.shape or not np.isfinite(beta).all():
            raise ValueError("directional page has invalid dimensions or pixels")
        beta = stitch("directional", beta, absolute=True)
        width = 2 * size
    elif layer_betas:
        # A layer's own gradient without the total's (runtime direct light,
        # RFC 0016): the page is directional, the total's gradient half zero.
        width = 2 * size
    rgba = np.zeros((size, width, 4), dtype="<f2")
    rgba[:, :size, :3] = (pixels[::-1, :, :3] * args.preview_gain).astype("<f2")
    if directional:
        # beta is a ratio: the preview gain does not scale it.
        rgba[:, size:, :3] = beta[::-1, :, :3].astype("<f2")
    rgba[:, :, 3] = 1.0
    sun = None
    if args.sun_visibility:
        if not args.sun_bake_evidence:
            raise ValueError("--sun-visibility needs --sun-bake-evidence")
        bake = json.loads(args.sun_bake_evidence.read_text())
        if sha256(args.sun_bake_evidence) != evidence.get("source_bake_evidence_sha256"):
            raise ValueError("sun bake receipt is not the atlas's source bake")
        sun = bake.get("sun")
        if not sun or sun.get("visibility_exr_sha256") != sha256(args.sun_visibility):
            raise ValueError("sun visibility differs from the bake receipt")
        if not args.coverage_exr:
            raise ValueError("--sun-visibility needs --coverage-exr")
        from scipy import ndimage
        visibility = iio.imread(args.sun_visibility)[:, :, 0].astype(np.float64)
        covered = iio.imread(args.coverage_exr)[:, :, :3].min(axis=2) > 0.5
        if visibility.shape != (size, size) or covered.shape != (size, size):
            raise ValueError("sun visibility or coverage has the wrong size")
        _, (rows, columns) = ndimage.distance_transform_edt(~covered, return_indices=True)
        filled = stitch("sun_visibility", visibility[rows, columns][:, :, None],
                        absolute=True)[:, :, 0]
        rgba[:, :size, 3] = np.clip(filled, 0.0, 1.0)[::-1].astype("<f2")
    light_masks = None
    if args.light_masks:
        if not (args.coverage_exr and args.lsmk_out):
            raise ValueError("--light-masks needs --coverage-exr and --lsmk-out")
        import light_shadow_masks
        lump, report = light_shadow_masks.pack(args.light_masks, args.coverage_exr,
                                               args.lighting_stage)
        temporary = args.lsmk_out.with_name(args.lsmk_out.name + ".tmp")
        temporary.write_bytes(lump)
        os.replace(temporary, args.lsmk_out)
        light_masks = dict(report, sha256=sha256(args.lsmk_out))
    pages = [rgba]
    layer_receipts = {}
    source_bake = evidence.get("source_bake_evidence_sha256")
    for role in order:
        path = separated[role]
        receipt = json.loads(Path(str(path) + ".json").read_text())
        if (receipt.get("status") != "pass" or receipt.get("layer") != role or
                receipt.get("atlas_exr_sha256") != sha256(path) or not source_bake or
                receipt.get("source_bake_evidence_sha256") != source_bake):
            raise ValueError("%s layer differs from its receipt or the total's bake" % role)
        light = iio.imread(path)
        if light.shape != pixels.shape or not np.isfinite(light).all() or light[..., :3].min() < 0:
            raise ValueError("%s layer has invalid dimensions or pixels" % role)
        light = stitch(role, light, reference=pixels)
        page = np.zeros((size, width, 4), dtype="<f2")
        page[:, :size, :3] = (light[::-1, :, :3] * args.preview_gain).astype("<f2")
        page[:, :, 3] = 1.0
        layer_beta = None
        if role in layer_betas:
            beta_path = layer_betas[role]
            beta_receipt = json.loads(Path(str(beta_path) + ".json").read_text())
            if (beta_receipt.get("status") != "pass" or beta_receipt.get("layer") != role or
                    beta_receipt.get("directional_exr_sha256") != sha256(beta_path) or
                    beta_receipt.get("flat_exr_sha256") != sha256(path) or
                    beta_receipt.get("size") != size):
                raise ValueError("%s layer's directional page differs from its receipt or "
                                 "layer" % role)
            layer_beta = iio.imread(beta_path)
            if layer_beta.shape != pixels.shape or not np.isfinite(layer_beta).all():
                raise ValueError("%s layer's directional page has invalid dimensions or "
                                 "pixels" % role)
            layer_beta = stitch(role + "_directional", layer_beta, absolute=True)
            # beta is a ratio: the preview gain does not scale it.
            page[:, size:, :3] = layer_beta[::-1, :, :3].astype("<f2")
        pages.append(page)
        layer_receipts[role] = {"exr_sha256": sha256(path),
                                "receipt_sha256": sha256(Path(str(path) + ".json")),
                                "directional_exr_sha256": sha256(layer_betas[role])
                                if layer_beta is not None else None,
                                "mean_rgb": light[..., :3].reshape(-1, 3).mean(axis=0).tolist()}
    args.out.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="lightmap-ktx2-", dir=args.out.parent) as name:
        temporary = Path(name)
        package = temporary / "atlas.ktx2"
        raws = []
        for index, page in enumerate(pages):
            raws.append(temporary / ("layer%d.rgba16f" % index))
            raws[-1].write_bytes(page.tobytes())
        tool = str(args.ktx_tool.resolve())
        run([tool, "create", "--format", "R16G16B16A16_SFLOAT", "--raw",
             "--width", str(width), "--height", str(size)] +
            (["--layers", str(len(pages))] if len(pages) > 1 else []) +
            ["--assign-tf", "linear", "--assign-texcoord-origin", "top-left"] +
            [str(raw) for raw in raws] + [str(package)])
        run([tool, "validate", str(package)])
        for index, raw in enumerate(raws):
            extracted = temporary / ("extracted%d.rgba16f" % index)
            run([tool, "extract", "--raw"] + (["--layer", str(index)] if len(pages) > 1 else []) +
                [str(package), str(extracted)])
            if extracted.read_bytes() != raw.read_bytes():
                raise ValueError("KTX2 changed the authored half-float texels of layer %d" % index)
        os.replace(package, args.out)
    lmap_receipt = None
    if args.lmap_out:
        import world_lightmap_v3
        layers = []
        for page in pages:
            flat = page[:, :size, :3].astype(np.float32)
            beta = (page[:, size:, :3].astype(np.float32) if width == 2 * size
                    else np.zeros_like(flat))
            layers.append((flat, beta))
        sun_mask = rgba[:, :size, 3].astype(np.float32) if sun else None
        lump, report = world_lightmap_v3.build(args.ktx_tool.resolve(), layers, sun_mask)
        temporary = args.lmap_out.with_name(args.lmap_out.name + ".tmp")
        temporary.write_bytes(lump)
        os.replace(temporary, args.lmap_out)
        lmap_receipt = dict(report, status="pass", version=world_lightmap_v3.VERSION,
                            lmap_sha256=sha256(args.lmap_out), roles=["total"] + order)
        args.lmap_out.with_name(args.lmap_out.name + ".json").write_text(
            json.dumps(lmap_receipt, indent=2, sort_keys=True) + "\n")
    result = {"status": "pass", "scope": "cycles-l0-ktx2",
              "bake_scope": args.expected_scope,
              "atlas_exr_sha256": sha256(args.exr),
              "bake_evidence_sha256": sha256(args.bake_evidence),
              "lighting_stage_sha256": sha256(args.lighting_stage),
              "ktx2_sha256": sha256(args.out), "format": "R16G16B16A16_SFLOAT",
              "orientation": "top-left", "width": width,
              "height": size, "preview_gain": args.preview_gain,
              "layout": "directional-2x1" if width == 2 * size else "flat",
              "layers": ["total"] + order, "separated_layers": layer_receipts,
              "directional_exr_sha256": sha256(args.directional_exr) if directional else None,
              "max_half_quantization_error": float(np.max(np.abs(
                  rgba[::-1][:, :size, :3].astype(np.float32) -
                  pixels[:, :, :3] * args.preview_gain))),
              "lmap_v3": {"sha256": lmap_receipt["lmap_sha256"], "bytes": lmap_receipt["bytes"]}
              if lmap_receipt else None,
              "seams": stitch.record,
              "light_masks": light_masks,
              "sun": {"visibility_exr_sha256": sha256(args.sun_visibility),
                      "direction_to_sun": (-np.asarray(sun["direction"])).tolist(),
                      "irradiance": sun["irradiance"]} if sun else None}
    args.out.with_name(args.out.name + ".json").write_text(
        json.dumps(result, indent=2, sort_keys=True) + "\n")
    print(json.dumps(result, sort_keys=True))


if __name__ == "__main__":
    main()
