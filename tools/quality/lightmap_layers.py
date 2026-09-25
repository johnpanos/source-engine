#!/usr/bin/env python3
"""RFC 0011 G2.2: the separated lightmap bake is consistent.

    python3 tools/quality/lightmap_layers.py check --map-build DIR [--fixture NAME]
        [--seed-defect swap|double-indirect] [--out FILE.json]

The bake (pbrt_lightmap_bake.py --layers) writes the total atlas and its
direct and indirect layers as three independent Cycles bakes of the same
texels, each hash-bound by the bake receipt. Their sum must reproduce the
total without bias: per 16 x 16 block of covered texels the mean residual
total - (direct + indirect) must lie within four standard errors of zero (the
bake's noise band, measured from the residual itself), and over the atlas
within four standard errors plus 0.1% of the mean total. A furnace-style
fixture that declares diffuse_light_direct / diffuse_light_indirect must also
reproduce them within ANALYTIC_TOLERANCE.

--seed-defect applies a known defect to the layers before judging (the
sensitivity controls): `swap` exchanges direct and indirect (the sum still
holds, the analytic check must fail) and `double-indirect` counts indirect
twice (the sum check must fail).
"""

import argparse
import hashlib
import json
import sys
from pathlib import Path

import numpy as np

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import gi_reference  # noqa: E402

SCHEMA = "lightmap-layer-consistency/v1"
BLOCK = 16
MIN_BLOCK_TEXELS = 64
STANDARD_ERRORS = 4.0
MAX_FAILED_BLOCK_FRACTION = 0.01
MEAN_ALLOWANCE = 0.001
ANALYTIC_TOLERANCE = 0.01


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def read_rgb(path):
    import OpenImageIO as oiio
    return oiio.ImageBuf(str(path)).get_pixels(oiio.FLOAT)[..., :3].astype(np.float64)


def luminance(rgb):
    return rgb @ np.array([0.2126, 0.7152, 0.0722])


def judge(total, direct, indirect, covered, analytic=None):
    """(failures, summary) for three layers over the covered texels."""
    failures = []
    residual = luminance(total - direct - indirect)
    mean_total = float(luminance(total)[covered].mean())
    values = residual[covered]
    error = values.std() / np.sqrt(values.size)
    mean = float(values.mean())
    if abs(mean) > STANDARD_ERRORS * error + MEAN_ALLOWANCE * mean_total:
        failures.append("atlas mean residual %.5f exceeds %.1f standard errors (%.5f) + %.1f%% "
                        "of the total" % (mean, STANDARD_ERRORS, error, MEAN_ALLOWANCE * 100))
    height, width = covered.shape
    blocks = failed = 0
    worst = 0.0
    for y in range(0, height, BLOCK):
        for x in range(0, width, BLOCK):
            mask = covered[y:y + BLOCK, x:x + BLOCK]
            if mask.sum() < MIN_BLOCK_TEXELS:
                continue
            block = residual[y:y + BLOCK, x:x + BLOCK][mask]
            blocks += 1
            spread = block.std() / np.sqrt(block.size)
            # A noise-free bake (uniform emission) must match exactly.
            limit = STANDARD_ERRORS * spread if spread > 1e-7 else 1e-5
            score = abs(block.mean()) / max(limit, 1e-12)
            worst = max(worst, score)
            failed += score > 1.0
    if not blocks:
        failures.append("no 16 x 16 block has %d covered texels" % MIN_BLOCK_TEXELS)
    elif failed > MAX_FAILED_BLOCK_FRACTION * blocks:
        failures.append("%d of %d blocks' mean residual exceeds the noise band" % (failed, blocks))
    summary = {"mean_total": mean_total,
               "mean_direct": float(luminance(direct)[covered].mean()),
               "mean_indirect": float(luminance(indirect)[covered].mean()),
               "mean_residual": mean, "mean_residual_standard_error": float(error),
               "blocks": blocks, "failed_blocks": int(failed),
               "worst_block_over_band": float(worst)}
    if analytic:
        for key, measured in (("diffuse_light_direct", summary["mean_direct"]),
                              ("diffuse_light_indirect", summary["mean_indirect"])):
            if key in analytic:
                expected = analytic[key]
                summary[key] = {"expected": expected, "measured": measured}
                if abs(measured - expected) > ANALYTIC_TOLERANCE * expected:
                    failures.append("%s %.4f differs from the analytic %.4f" %
                                    (key, measured, expected))
    return failures, summary


def cmd_check(args):
    lighting = Path(args.map_build) / "lighting"
    receipt = json.loads((lighting / "atlas.exr.json").read_text())
    layers = receipt.get("layers") or {}
    if set(layers) != {"direct", "indirect"}:
        raise SystemExit("%s: the bake receipt names no direct and indirect layers" % lighting)
    files = {"total": lighting / "atlas.exr",
             "direct": lighting / "layers" / layers["direct"]["exr"],
             "indirect": lighting / "layers" / layers["indirect"]["exr"]}
    expected = {"total": receipt["atlas_exr_sha256"], "direct": layers["direct"]["exr_sha256"],
                "indirect": layers["indirect"]["exr_sha256"]}
    for role, path in files.items():
        if sha256(path) != expected[role]:
            raise SystemExit("%s differs from its bake receipt" % path)
    total, direct, indirect = (read_rgb(files[r]) for r in ("total", "direct", "indirect"))
    covered = read_rgb(lighting / "uv-coverage.exr")[..., 0] > 0.5
    if args.seed_defect == "swap":
        direct, indirect = indirect, direct
    elif args.seed_defect == "double-indirect":
        indirect = indirect * 2.0
    analytic = gi_reference.load_fixture(args.fixture).get("analytic") if args.fixture else None
    failures, summary = judge(total, direct, indirect, covered, analytic)
    record = {"schema": SCHEMA, "map_build": str(Path(args.map_build).resolve()),
              "fixture": args.fixture, "seed_defect": args.seed_defect,
              "files": {role: {"path": str(path), "sha256": expected[role]}
                        for role, path in files.items()},
              "summary": summary, "failures": failures,
              "status": "fail" if failures else "pass"}
    if args.out:
        Path(args.out).parent.mkdir(parents=True, exist_ok=True)
        Path(args.out).write_text(json.dumps(record, indent=2, sort_keys=True) + "\n")
    print("lightmap layers %s%s: %s%s" % (
        Path(args.map_build).name, " (seeded %s)" % args.seed_defect if args.seed_defect else "",
        record["status"], "".join("\n  " + f for f in failures)))
    return 0 if not failures else 1


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    commands = parser.add_subparsers(dest="command", required=True)
    check = commands.add_parser("check")
    check.add_argument("--map-build", required=True)
    check.add_argument("--fixture")
    check.add_argument("--seed-defect", choices=("swap", "double-indirect"))
    check.add_argument("--out")
    args = parser.parse_args()
    return cmd_check(args)


if __name__ == "__main__":
    sys.exit(main())
