#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Per-material pixel conformance (source-material-pixels/v1).

The harness (unittests/shaderextensiontest/material_pixel_conformance.cpp) renders
real Source materials through the real material system and a selected render
provider and reports the pixels it measured. This module is the oracle:

* the run is valid only if readback returns a cleared frame's color;
* backend-independent properties of the material's math hold on every backend;
* where a closed form of the D3D9 shader has been validated on D3D9, the pixels
  match it (the lightmap term in integer HDR);
* the pixels agree with a D3D9 reference capture within PIXEL_TOLERANCE.

A backend that reports a different HDR mode than the run requested fails with
that reason; it is never compared as if it supported the mode.

Run a backend:  material_pixel_conformance.py run --runtime run/runtime --build build
                    --renderer native-vulkan --hdr none --out OUT
                    [--reference quality/fixtures/material-pixels/lightmap-dx9-none.json]
Check a capture: material_pixel_conformance.py check --pixels OUT/pixels.json
                    --hdr none [--reference ...]

Exit 0 on pass, 1 on conformance failure, 2 on invalid input or harness error.
"""

import argparse
import datetime
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys

sys.path.insert(0, str(Path(__file__).resolve().parent))
import conformance  # noqa: E402
import portal_boot  # noqa: E402


SCHEMA = "source-material-pixels/v1"
EVIDENCE_SCHEMA = "material-pixel-evidence/v1"
HDR_TYPES = {"none": 0, "integer": 1}
CLEAR = [255, 0, 255]
# Cross-backend agreement, in 8-bit levels per channel. Rasterizers may round
# differently at the last bit; the lightmap cases are solid regions sampled at
# texel centers, so anything beyond a few levels is a difference in the math.
PIXEL_TOLERANCE = 3
# The closed-form lightmap model is exact up to texel and output quantization.
MODEL_TOLERANCE = 3
DARK = 2  # a channel at or below this reads as zero
LIGHTMAP_CASES = ("black_lightmap", "ramp_low", "ramp_mid", "ramp_high", "channels",
                  "base_gray", "base_color")


class PixelsError(ValueError):
    pass


def read_pixels(path):
    try:
        report = json.loads(Path(path).read_text())
    except (OSError, ValueError) as error:
        raise PixelsError("%s is not readable JSON: %s" % (path, error)) from error
    if report.get("schema") != SCHEMA:
        raise PixelsError("%s is not a %s capture" % (path, SCHEMA))
    if report.get("family") != "lightmap":
        raise PixelsError("%s holds family %r; only 'lightmap' is defined" % (path, report.get("family")))
    names = [case.get("name") for case in report.get("cases", [])]
    if names != list(LIGHTMAP_CASES):
        raise PixelsError("%s has cases %s, expected %s" % (path, names, list(LIGHTMAP_CASES)))
    if len(report.get("orientation", {}).get("pixels", [])) != 2:
        raise PixelsError("%s has no orientation capture" % path)
    return report


def _srgb_to_linear(value):
    c = value / 255.0
    return c / 12.92 if c <= 0.04045 else ((c + 0.055) / 1.055) ** 2.4


def _linear_to_srgb(value):
    c = min(max(value, 0.0), 1.0)
    encoded = c * 12.92 if c <= 0.0031308 else 1.055 * c ** (1 / 2.4) - 0.055
    return round(encoded * 255)


def lightmap_model(base, lightmap):
    """LightmappedGeneric's lightmap term as D3D9 computes it: sRGB-decoded
    albedo times linear lightmap, encoded to sRGB on output."""
    return [_linear_to_srgb(_srgb_to_linear(b) * l) for b, l in zip(base, lightmap)]


def _luminance(pixel):
    return sum(pixel)


def _close(a, b, tolerance):
    return all(abs(x - y) <= tolerance for x, y in zip(a, b))


def check_properties(report):
    """Properties any backend implementing the lightmap term satisfies."""
    cases = {case["name"]: case for case in report["cases"]}
    failures = []
    black = cases["black_lightmap"]["pixels"]
    if any(channel > DARK for pixel in black for channel in pixel):
        failures.append("black lightmap: pixels %s are not black" % black)
    ramp = [cases["ramp_low"]["pixels"][0], cases["ramp_low"]["pixels"][1],
            cases["ramp_mid"]["pixels"][0], cases["ramp_mid"]["pixels"][1],
            cases["ramp_high"]["pixels"][0]]
    levels = [_luminance(pixel) for pixel in ramp]
    if any(later <= earlier for earlier, later in zip(levels, levels[1:])):
        failures.append("lightmap ramp 1/16..1 is not strictly brighter at each step: %s" % ramp)
    high = cases["ramp_high"]["pixels"]
    if _luminance(high[1]) < _luminance(high[0]):
        failures.append("overbright lightmap 2.0 is darker than 1.0: %s" % high)
    left, right = cases["channels"]["pixels"]
    if not (left[0] >= 128 and left[1] <= DARK and left[2] <= DARK):
        failures.append("red-only lightmap produced %s (channels mixed or swapped)" % left)
    if not (right[2] >= 128 and right[0] <= DARK and right[1] <= DARK):
        failures.append("blue-only lightmap produced %s (channels mixed or swapped)" % right)
    for name in ("base_gray", "base_color"):
        case = cases[name]
        if not _close(case["pixels"][1], case["base"], PIXEL_TOLERANCE):
            failures.append("%s: lightmap 1.0 gives %s, not the base texture %s"
                            % (name, case["pixels"][1], case["base"]))
    if _luminance(cases["base_gray"]["pixels"][0]) >= _luminance(cases["ramp_mid"]["pixels"][0]):
        failures.append("a darker base texture under the same lightmap is not darker: "
                        "base_gray %s vs white %s" % (cases["base_gray"]["pixels"][0],
                                                     cases["ramp_mid"]["pixels"][0]))
    return failures


def check_orientation(report):
    """Texture row 0 on the clip-space top edge must appear at the top of the
    frame, as D3D9 draws it. A backend that forgets Vulkan's downward clip-space
    Y renders the whole frame upside down and fails here."""
    orientation = report["orientation"]
    top, bottom = orientation["pixels"]
    if _close(top, orientation["top_texel"], PIXEL_TOLERANCE) and \
            _close(bottom, orientation["bottom_texel"], PIXEL_TOLERANCE):
        return []
    if _close(top, orientation["bottom_texel"], PIXEL_TOLERANCE) and \
            _close(bottom, orientation["top_texel"], PIXEL_TOLERANCE):
        return ["frame is upside down: the texture's top rows %s render at the bottom"
                % orientation["top_texel"]]
    return ["orientation: top %s and bottom %s, expected %s over %s"
            % (top, bottom, orientation["top_texel"], orientation["bottom_texel"])]


def check_model(report):
    """The D3D9-validated closed form, which holds exactly in integer HDR."""
    failures = []
    for case in report["cases"]:
        for side, lightmap, pixel in zip(("left", "right"), case["lightmap"], case["pixels"]):
            expected = lightmap_model(case["base"], lightmap)
            if not _close(pixel, expected, MODEL_TOLERANCE):
                failures.append("%s %s: %s, model %s" % (case["name"], side, pixel, expected))
    return failures


def compare(report, reference):
    failures = []
    if report["hdr_type"] != reference["hdr_type"]:
        return ["reference was captured in HDR type %s, candidate in %s"
                % (reference["hdr_type"], report["hdr_type"])]
    for case, ref in zip(report["cases"], reference["cases"]):
        if case["base"] != ref["base"] or case["lightmap"] != ref["lightmap"]:
            failures.append("%s: inputs differ from the reference capture" % case["name"])
            continue
        for side, pixel, expected in zip(("left", "right"), case["pixels"], ref["pixels"]):
            if not _close(pixel, expected, PIXEL_TOLERANCE):
                failures.append("%s %s: %s, reference %s" % (case["name"], side, pixel, expected))
    return failures


def evaluate(report, hdr, reference=None):
    """All failures of a capture requested in HDR mode `hdr`."""
    probe = report.get("clear_probe", {})
    if not _close(probe.get("pixel", [0, 0, 0]), CLEAR, 1):
        return ["readback returned %s for a frame cleared to %s; the capture measured nothing"
                % (probe.get("pixel"), CLEAR)]
    if report["hdr_type"] != HDR_TYPES[hdr]:
        return ["backend does not support HDR mode '%s' (it reports HDR type %s)"
                % (hdr, report["hdr_type"])]
    failures = check_orientation(report) + check_properties(report)
    if report["hdr_type"] == HDR_TYPES["integer"]:
        failures += check_model(report)
    if reference is not None:
        failures += compare(report, reference)
    return failures


def find_harness(build):
    candidates = sorted(Path(build).rglob("material_pixel_conformance"))
    candidates = [path for path in candidates if path.is_file() and os.access(path, os.X_OK)]
    if len(candidates) != 1:
        raise PixelsError("build must contain exactly one material_pixel_conformance executable")
    return candidates[0]


def run(args):
    output = args.out.resolve()
    output.mkdir(parents=True, exist_ok=True)
    if (output / "evidence.json").exists():
        raise PixelsError("evidence already exists; use a new output directory")
    evidence = {"schema": EVIDENCE_SCHEMA, "status": "fail",
                "started_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
                "source": conformance.source_identity(conformance.repo_root()),
                "renderer": args.renderer, "hdr": args.hdr,
                "tolerances": {"pixel": PIXEL_TOLERANCE, "model": MODEL_TOLERANCE}}
    stage = output / "runtime"
    evidence["staging"] = portal_boot.stage_runtime(args.runtime, stage)
    evidence["build_overrides"] = portal_boot.install_build(args.build, stage)
    harness = stage / "material_pixel_conformance"
    shutil.copy2(find_harness(args.build), harness)
    evidence["harness_sha256"] = portal_boot.sha256(harness)
    pixels = output / "pixels.json"
    command = [str(harness), "-game", "portal", "-renderer", args.renderer, "-hdr", args.hdr,
               "-out", str(pixels)]
    environment = os.environ.copy()
    environment["LD_LIBRARY_PATH"] = str(stage / "bin") + ":" + environment.get("LD_LIBRARY_PATH", "")
    # DXVK presents through SDL3 in these products; the native backend ignores it.
    environment["DXVK_WSI_DRIVER"] = "SDL3"
    evidence["command"] = command
    with open(output / "harness.log", "w") as log:
        try:
            completed = subprocess.run(command, cwd=stage, env=environment, stdout=log,
                                       stderr=subprocess.STDOUT, timeout=args.timeout)
            evidence["returncode"] = completed.returncode
        except subprocess.TimeoutExpired:
            evidence["returncode"] = None
            evidence["failures"] = ["harness timed out after %gs" % args.timeout]
            (output / "evidence.json").write_text(json.dumps(evidence, indent=2) + "\n")
            return 2
    if evidence["returncode"] != 0 or not pixels.is_file():
        evidence["failures"] = ["harness exited %s without a capture" % evidence["returncode"]]
        (output / "evidence.json").write_text(json.dumps(evidence, indent=2) + "\n")
        return 2
    report = read_pixels(pixels)
    reference = read_pixels(args.reference) if args.reference else None
    if args.reference:
        evidence["reference"] = {"path": str(args.reference),
                                 "sha256": portal_boot.sha256(args.reference)}
    evidence["pixels_sha256"] = portal_boot.sha256(pixels)
    evidence["failures"] = evaluate(report, args.hdr, reference)
    evidence["status"] = "pass" if not evidence["failures"] else "fail"
    (output / "evidence.json").write_text(json.dumps(evidence, indent=2) + "\n")
    return _print_result(args.renderer, args.hdr, evidence["failures"], output / "evidence.json")


def _print_result(renderer, hdr, failures, where):
    print("material pixel conformance (lightmap, %s, hdr %s): %s (%s)"
          % (renderer, hdr, "pass" if not failures else "fail", where))
    for failure in failures:
        print("  " + failure)
    return 0 if not failures else 1


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    commands = parser.add_subparsers(dest="command", required=True)
    run_parser = commands.add_parser("run", help="stage a runtime, render, and judge")
    run_parser.add_argument("--runtime", type=Path, required=True)
    run_parser.add_argument("--build", type=Path, required=True)
    run_parser.add_argument("--renderer", required=True)
    run_parser.add_argument("--hdr", choices=sorted(HDR_TYPES), required=True)
    run_parser.add_argument("--out", type=Path, required=True)
    run_parser.add_argument("--reference", type=Path)
    run_parser.add_argument("--timeout", type=float, default=120)
    check_parser = commands.add_parser("check", help="judge an existing capture")
    check_parser.add_argument("--pixels", type=Path, required=True)
    check_parser.add_argument("--hdr", choices=sorted(HDR_TYPES), required=True)
    check_parser.add_argument("--reference", type=Path)
    args = parser.parse_args(argv)
    try:
        if args.command == "run":
            return run(args)
        report = read_pixels(args.pixels)
        reference = read_pixels(args.reference) if args.reference else None
        return _print_result(report.get("renderer"), args.hdr, evaluate(report, args.hdr, reference),
                             args.pixels)
    except (PixelsError, ValueError, OSError) as error:
        print("material pixel conformance: invalid input: %s" % error, file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
