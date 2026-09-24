#!/usr/bin/env python3
"""Package a checked Cycles diffuse lightmap atlas as linear RGBA16F KTX2.

Used by the PBRT map pipeline's ktx2 step; `--expected-scope` names the bake
receipt producer this package trusts (for example
`pbrt-shared-lightmap-uv-and-cycles-bake-denoised`).

With `--directional-exr` (from `lightmap_directional.py`) the page is twice as
wide as it is tall: the flat irradiance atlas on the left and the per-texel
world-space luminance gradient beta on the right, at the same texel rows.
`world_pbr.frag` recognises the 2:1 page and samples both halves at the same
lightmap coordinate. A reflection probe band stays in the top rows (mips from
x = 0) with its marker at the page's top-right texel.

With `--sun-visibility` (the bake's sun mask) the flat half's alpha holds the
sun's [0, 1] visibility, gutter-filled from `--coverage-exr`, and the marker row
carries the sun beside the probe marker: texel (W - 2, 0) = direction toward
the sun (xyz, w = 2) and texel (W - 3, 0) = its irradiance (rgb, w = disc
angle in degrees). `world_pbr.frag` adds the sun's specular dynamically,
shadowed by that alpha; its diffuse light is already in the bake.

With `--layer ROLE=EXR` (denoised separated light from the bake's `--layers`)
the package is an LMAP v2 2D array (public/mapcontainer/world_lightmap.h):
layer 0 is the total page above; then `indirect` (two layers) or `direct`
and `indirect` (three), each the flat light only, with a zero gradient half
on a directional page and no probe or sun texels.
"""

import argparse
import hashlib
import json
import os
import subprocess
import tempfile
from pathlib import Path

import imageio.v3 as iio
import numpy as np


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def run(command):
    return subprocess.run(command, check=True, capture_output=True, text=True).stdout


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
    parser.add_argument("--probe-dir", type=Path,
                        help="reflection probe faces from pbrt_reflection_probe.py")
    parser.add_argument("--probe-width", type=int, default=512,
                        help="equirect width of probe mip 0; the band is half as tall")
    parser.add_argument("--directional-exr", type=Path,
                        help="beta page from lightmap_directional.py (receipt beside it)")
    parser.add_argument("--sun-visibility", type=Path,
                        help="sun visibility EXR named by the bake receipt's `sun`")
    parser.add_argument("--coverage-exr", type=Path,
                        help="UV coverage used to fill the visibility's gutters")
    parser.add_argument("--sun-bake-evidence", type=Path,
                        help="the bake receipt whose `sun` names the visibility EXR")
    parser.add_argument("--layer", action="append", default=[], metavar="ROLE=EXR",
                        help="separated-light layer (direct, indirect) and its denoised EXR")
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()
    separated = {}
    for item in args.layer:
        role, _, path = item.partition("=")
        if role not in ("direct", "indirect") or not path or role in separated:
            parser.error("--layer takes distinct direct=EXR / indirect=EXR entries")
        separated[role] = Path(path)
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
        width = 2 * size
    rgba = np.empty((size, width, 4), dtype="<f2")
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
        rgba[:, :size, 3] = visibility[rows, columns][::-1].astype("<f2")
    band = 0
    if args.probe_dir:
        import sys
        sys.path.insert(0, str(Path(__file__).resolve().parent))
        import reflection_probe
        receipt = json.loads((args.probe_dir / "probe.json").read_text())
        faces = {}
        for name in reflection_probe.FACES:
            path = args.probe_dir / (name + ".exr")
            if receipt.get("status") != "pass" or receipt["faces"].get(name) != sha256(path):
                raise ValueError("probe face differs from its receipt: " + name)
            faces[name] = iio.imread(path)[:, :, :3].astype(np.float64)
        band = args.probe_width // 2
        if evidence.get("reserved_rows", 0) < band:
            raise ValueError("bake did not reserve the probe band's atlas rows")
        # KTX row r holds lightmap v = (r + 0.5) / size; every baked chart
        # (a mesh with UV area) must start above the band. Alpha is not a
        # reliable coverage mask, so check the receipt's UV extents.
        low = band / evidence["size"]
        charts = {name: extent for name, extent in evidence.get("uv_extents", {}).items()
                  if extent[2] > extent[0] and extent[3] > extent[1]}
        if not charts or any(extent[1] < low for extent in charts.values()):
            raise ValueError("probe band rows overlap baked lightmap charts")
        mips = reflection_probe.mip_chain(
            reflection_probe.cube_to_equirect(faces, args.probe_width))
        staging = np.zeros(rgba.shape, dtype=np.float64)
        probe = reflection_probe.write_band(staging, [mip * args.preview_gain for mip in mips])
        rgba[:band] = staging[:band].astype("<f2")
        probe["receipt_sha256"] = sha256(args.probe_dir / "probe.json")
    if sun:
        if not probe:
            raise ValueError("the sun texels live in the probe band's marker row")
        direction = -np.asarray(sun["direction"], dtype=np.float64)
        direction /= np.linalg.norm(direction)
        rgba[0, width - 2] = np.array((*direction, 2.0), dtype="<f2")
        rgba[0, width - 3] = np.array((*sun["irradiance"], sun["angle_degrees"]), dtype="<f2")
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
        page = np.zeros((size, width, 4), dtype="<f2")
        page[:, :size, :3] = (light[::-1, :, :3] * args.preview_gain).astype("<f2")
        page[:, :, 3] = 1.0
        pages.append(page)
        layer_receipts[role] = {"exr_sha256": sha256(path),
                                "receipt_sha256": sha256(Path(str(path) + ".json")),
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
    result = {"status": "pass", "scope": "cycles-l0-ktx2",
              "bake_scope": args.expected_scope,
              "atlas_exr_sha256": sha256(args.exr),
              "bake_evidence_sha256": sha256(args.bake_evidence),
              "lighting_stage_sha256": sha256(args.lighting_stage),
              "ktx2_sha256": sha256(args.out), "format": "R16G16B16A16_SFLOAT",
              "orientation": "top-left", "width": width,
              "height": size, "preview_gain": args.preview_gain,
              "layout": "directional-2x1" if directional else "flat",
              "lmap_version": 2 if len(pages) > 1 else 1,
              "layers": ["total"] + order, "separated_layers": layer_receipts,
              "directional_exr_sha256": sha256(args.directional_exr) if directional else None,
              # Lightmap rows only; the probe band replaces the reserved rows.
              "max_half_quantization_error": float(np.max(np.abs(
                  rgba[::-1][:rgba.shape[0] - band, :size, :3].astype(np.float32) -
                  pixels[:pixels.shape[0] - band, :, :3] * args.preview_gain))),
              "reflection_probe": probe,
              "sun": {"texels": [[width - 2, 0], [width - 3, 0]],
                      "visibility_exr_sha256": sha256(args.sun_visibility),
                      "direction_to_sun": (-np.asarray(sun["direction"])).tolist(),
                      "irradiance": sun["irradiance"]} if sun else None}
    args.out.with_name(args.out.name + ".json").write_text(
        json.dumps(result, indent=2, sort_keys=True) + "\n")
    print(json.dumps(result, sort_keys=True))


if __name__ == "__main__":
    main()
