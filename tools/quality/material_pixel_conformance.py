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

Families:

* lightmap: LightmappedGeneric's lightmap term, its orientation and its
  tone-mapping scale;
* exposure: the luminance histogram integer-HDR auto-exposure is computed from
  (dev/lumcompare drawn under occlusion queries over a frame of known regions).
  The expected count of every luminance range follows exactly from the region
  geometry, so the counts are held to that model and to the reference exactly;
* skinning: model placement through the MODEL matrix LoadBoneMatrix(0) loads
  and through hardware skinning (bone matrices, two stored weights plus the
  implicit third, bone index bytes). Each case must cover exactly the third of
  the frame its bones place it in, and a back-facing model quad must be culled.

A backend that reports a different HDR mode than the run requested fails with
that reason; it is never compared as if it supported the mode.

Run a backend:  material_pixel_conformance.py run --runtime run/runtime --build build
                    --renderer native-vulkan --hdr none [--family exposure] --out OUT
                    [--display desktop]
                    [--reference quality/fixtures/material-pixels/lightmap-dx9-none.json]
Check a capture: material_pixel_conformance.py check --pixels OUT/pixels.json
                    --hdr none [--reference ...]   (the family is read from the capture)

Runs are headless by default: SDL's offscreen video driver gives the backend a
Vulkan surface through VK_EXT_headless_surface, so the real GPU and swapchain
render with no window, compositor or display, and the drawable is exactly the
requested 256x256 whatever the desktop's scale. `--display desktop` presents in
a window on the session's display instead (the drawable then follows its scale).

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
import material_pixel_frames  # noqa: E402
import material_pixel_modellight  # noqa: E402
import material_pixel_portal  # noqa: E402
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
FAMILIES = ("lightmap", "exposure", "skinning", "portal", "modellight")
# Families whose harness writes whole frames, and the oracle module of each
# (validate, evaluate).
FRAME_FAMILIES = {"portal": material_pixel_portal, "modellight": material_pixel_modellight}
# Which third of the frame (left, middle, right) each skinning case's bones place
# its quad in. The oracle's own copy: the harness reports its expectation too,
# but a harness that computed placements wrongly must not pass itself.
# None: the quad is back-facing and model materials cull it, so no third holds it.
SKINNING_CASES = {"rigid_bone0": 0, "one_bone": 0, "implicit_third_weight": 1,
                  "blend_half": 1, "high_index": 2, "rigid_back_facing": None}
# The model material is unlit: a covered pixel is its base color up to output
# rounding and filtering.
SKIN_COLOR_TOLERANCE = 12
EXPOSURE_REGIONS = ("black", "gray64", "gray128", "gray190", "white", "red", "green", "blue")
LUMINANCE_RANGES = 17
# luminance_compare_ps2x.fxc: NTSC weights applied to the sRGB-decoded color.
LUMINANCE_WEIGHTS = (0.2125, 0.7154, 0.0721)


class PixelsError(ValueError):
    pass


def read_pixels(path):
    try:
        report = json.loads(Path(path).read_text())
    except (OSError, ValueError) as error:
        raise PixelsError("%s is not readable JSON: %s" % (path, error)) from error
    if report.get("schema") != SCHEMA:
        raise PixelsError("%s is not a %s capture" % (path, SCHEMA))
    family = report.get("family")
    if family not in FAMILIES:
        raise PixelsError("%s holds family %r; defined families are %s"
                          % (path, family, list(FAMILIES)))
    if family == "exposure":
        return _read_exposure(path, report)
    if family in FRAME_FAMILIES:
        try:
            return FRAME_FAMILIES[family].validate(path, report)
        except material_pixel_frames.CaptureError as error:
            raise PixelsError(str(error)) from error
    if family == "skinning":
        names = [case.get("name") for case in report.get("cases", [])]
        if names != list(SKINNING_CASES) or any(len(case.get("pixels", [])) != 3
                                               for case in report["cases"]):
            raise PixelsError("%s has skinning cases %s, expected %s"
                              % (path, names, list(SKINNING_CASES)))
        return report
    names = [case.get("name") for case in report.get("cases", [])]
    if names != list(LIGHTMAP_CASES):
        raise PixelsError("%s has cases %s, expected %s" % (path, names, list(LIGHTMAP_CASES)))
    for capture in ("orientation", "tone_scale"):
        if len(report.get(capture, {}).get("pixels", [])) != 2:
            raise PixelsError("%s has no %s capture" % (path, capture))
    return report


def _read_exposure(path, report):
    names = [region.get("name") for region in report.get("regions", [])]
    if names != list(EXPOSURE_REGIONS):
        raise PixelsError("%s has regions %s, expected %s" % (path, names, list(EXPOSURE_REGIONS)))
    if len(report.get("ranges", [])) != LUMINANCE_RANGES:
        raise PixelsError("%s has %d luminance ranges, expected %d"
                          % (path, len(report.get("ranges", [])), LUMINANCE_RANGES))
    if len(report.get("query_rect", [])) != 4 or len(report.get("frame", [])) != 2:
        raise PixelsError("%s has no query rectangle or frame size" % path)
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


def check_tone_scale(report):
    """LightmappedGeneric's output is scaled by the linear tone-mapping scale in
    integer HDR (FinalOutput's LINEAR_LIGHT_SCALE); without HDR, D3D9 forces
    that scale to 1."""
    tone = report["tone_scale"]
    case = {c["name"]: c for c in report["cases"]}[tone["case"]]
    failures = []
    for side, lightmap, pixel, unscaled in zip(("left", "right"), case["lightmap"],
                                               tone["pixels"], case["pixels"]):
        if report["hdr_type"] == HDR_TYPES["integer"]:
            expected = lightmap_model(case["base"], [value * tone["scale"] for value in lightmap])
            tolerance = MODEL_TOLERANCE
        else:
            expected, tolerance = unscaled, PIXEL_TOLERANCE
        if not _close(pixel, expected, tolerance):
            failures.append("tone scale %g, %s %s: %s, expected %s"
                            % (tone["scale"], tone["case"], side, pixel, expected))
    return failures


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
    for capture in ("orientation", "tone_scale"):
        for index, (pixel, expected) in enumerate(zip(report[capture]["pixels"],
                                                      reference[capture]["pixels"])):
            if not _close(pixel, expected, PIXEL_TOLERANCE):
                failures.append("%s %d: %s, reference %s" % (capture, index, pixel, expected))
    return failures


def luminance(color, scale=1.0):
    """The luminance luminance_compare_ps2x computes for a back-buffer color."""
    return scale * sum(w * _srgb_to_linear(c) for w, c in zip(LUMINANCE_WEIGHTS, color))


def _overlap(a, b):
    width = min(a[0] + a[2], b[0] + b[2]) - max(a[0], b[0])
    height = min(a[1] + a[3], b[1] + b[3]) - max(a[1], b[1])
    return max(width, 0) * max(height, 0)


def exposure_model(report):
    """The count each luminance range must report: the measured rectangle's
    pixels whose region color has a luminance inside the range (inclusive at
    both ends, as the shader's step() comparisons are)."""
    counts = []
    for bar in report["ranges"]:
        count = 0
        for region in report["regions"]:
            if bar["min"] <= luminance(region["color"], report["scale"]) <= bar["max"]:
                count += _overlap(region["rect"], report["query_rect"])
        counts.append(count)
    return counts


def check_exposure(report):
    """The frame holds the regions it was cleared to, every query completed, and
    every luminance range counts exactly the pixels the model says it holds."""
    failures = []
    for region in report["regions"]:
        if not _close(region["pixel"], region["color"], 1):
            failures.append("frame: region %s reads back %s after the luminance draws, "
                            "cleared to %s (dev/lumcompare must not write color)"
                            % (region["name"], region["pixel"], region["color"]))
    if failures:
        return failures
    counts = [bar["pixels"] for bar in report["ranges"]]
    if any(count < 0 for count in counts):
        return ["occlusion queries gave no result after %s frames: %s"
                % (report.get("poll_frames"), counts)]
    # A query counts samples, so a multisampled target counts each pixel once per
    # sample; the all-pixels range measures that unit, as the client uses it.
    area = report["query_rect"][2] * report["query_rect"][3]
    unit, remainder = divmod(counts[-1], area)
    if unit < 1 or remainder:
        return ["the all-pixels range counted %d, not a whole multiple of the %d pixels "
                "in the measured rectangle" % (counts[-1], area)]
    for index, (count, expected) in enumerate(zip(counts[:-1], exposure_model(report)[:-1])):
        if count != expected * unit:
            bar = report["ranges"][index]
            failures.append("luminance range %d [%.4g, %.4g]: %d samples, model %d pixels x %d"
                            % (index, bar["min"], bar["max"], count, expected, unit))
    return failures


def histogram(report):
    """The counts in pixels (each divided by the samples the all-pixels range
    shows per pixel): what the client's histogram sees once calibrated."""
    area = report["query_rect"][2] * report["query_rect"][3]
    unit = max(report["ranges"][-1]["pixels"] // area, 1)
    return [bar["pixels"] / unit for bar in report["ranges"]]


def compare_exposure(report, reference):
    if any(bar["pixels"] < 0 for bar in report["ranges"]):
        return []  # incomplete; check_exposure reports it
    if report["hdr_type"] != reference["hdr_type"]:
        return ["reference was captured in HDR type %s, candidate in %s"
                % (reference["hdr_type"], report["hdr_type"])]
    inputs = ("frame", "query_rect", "scale")
    if any(report[key] != reference[key] for key in inputs) or \
            [(r["color"], r["rect"]) for r in report["regions"]] != \
            [(r["color"], r["rect"]) for r in reference["regions"]] or \
            [(b["min"], b["max"]) for b in report["ranges"]] != \
            [(b["min"], b["max"]) for b in reference["ranges"]]:
        return ["exposure inputs (frame %s, regions, ranges) differ from the reference capture "
                "(frame %s); recapture both at the same drawable size"
                % (report["frame"], reference["frame"])]
    # Compared per pixel: the sample count of the target is not part of the
    # histogram the client computes.
    return ["luminance range %d: %g pixels, reference %g" % (index, count, expected)
            for index, (count, expected) in enumerate(zip(histogram(report),
                                                          histogram(reference)))
            if count != expected]


def check_skinning(report):
    """Every case covers the third its bones place it in and no other."""
    failures = []
    for case in report["cases"]:
        expected = SKINNING_CASES[case["name"]]
        for third, pixel in enumerate(case["pixels"]):
            covered = _close(pixel, report["color"], SKIN_COLOR_TOLERANCE)
            empty = _close(pixel, CLEAR, PIXEL_TOLERANCE)
            where = ("left", "middle", "right")[third]
            if third == expected and not covered:
                failures.append("%s: the model is not in the %s third (pixel %s)"
                                % (case["name"], where, pixel))
            elif third != expected and not empty:
                failures.append("%s: the %s third holds %s, expected the clear color (%s)"
                                % (case["name"], where, pixel,
                                   "a back face must be culled" if expected is None
                                   else "the model is drawn in the wrong place"))
    return failures


def compare_skinning(report, reference):
    return ["%s third %d: %s, reference %s" % (case["name"], third, pixel, expected)
            for case, ref in zip(report["cases"], reference["cases"])
            for third, (pixel, expected) in enumerate(zip(case["pixels"], ref["pixels"]))
            if not _close(pixel, expected, PIXEL_TOLERANCE)]


def evaluate(report, hdr, reference=None):
    """All failures of a capture requested in HDR mode `hdr`."""
    probe = report.get("clear_probe", {})
    if not _close(probe.get("pixel", [0, 0, 0]), CLEAR, 1):
        return ["readback returned %s for a frame cleared to %s; the capture measured nothing"
                % (probe.get("pixel"), CLEAR)]
    if report["hdr_type"] != HDR_TYPES[hdr]:
        return ["backend does not support HDR mode '%s' (it reports HDR type %s)"
                % (hdr, report["hdr_type"])]
    if reference is not None and reference.get("family") != report.get("family"):
        return ["reference holds family %r, capture %r"
                % (reference.get("family"), report.get("family"))]
    if report["family"] in FRAME_FAMILIES:
        return FRAME_FAMILIES[report["family"]].evaluate(report, reference)
    if report["family"] == "skinning":
        failures = check_skinning(report)
        if reference is not None:
            failures += compare_skinning(report, reference)
        return failures
    if report["family"] == "exposure":
        failures = check_exposure(report)
        if reference is not None:
            failures += compare_exposure(report, reference)
        return failures
    failures = check_orientation(report) + check_properties(report) + check_tone_scale(report)
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
                "renderer": args.renderer, "hdr": args.hdr, "family": args.family,
                "display": args.display,
                "tolerances": {"pixel": PIXEL_TOLERANCE, "model": MODEL_TOLERANCE}}
    stage = output / "runtime"
    evidence["staging"] = portal_boot.stage_runtime(args.runtime, stage)
    evidence["build_overrides"] = portal_boot.install_build(args.build, stage)
    harness = stage / "material_pixel_conformance"
    shutil.copy2(find_harness(args.build), harness)
    evidence["harness_sha256"] = portal_boot.sha256(harness)
    pixels = output / "pixels.json"
    command = [str(harness), "-game", "portal", "-renderer", args.renderer, "-hdr", args.hdr,
               "-family", args.family, "-out", str(pixels)]
    environment = os.environ.copy()
    environment["LD_LIBRARY_PATH"] = str(stage / "bin") + ":" + environment.get("LD_LIBRARY_PATH", "")
    # DXVK presents through SDL3 in these products; the native backend ignores it.
    environment["DXVK_WSI_DRIVER"] = "SDL3"
    if args.family == "portal":
        # DXVK's fast-linked graphics pipeline libraries drop D3D9 user clip
        # planes, so the custom clip plane of every nested portal view is
        # ignored (measured: the clipped geometry is drawn in full, identically
        # with and without the plane). D3D9 defines the clipped result, so the
        # reference is taken with pipeline libraries off. Native ignores it.
        environment["DXVK_CONFIG"] = material_pixel_portal.DXVK_CONFIG
        evidence["dxvk_config"] = material_pixel_portal.DXVK_CONFIG
    if args.display == "headless":
        environment["SDL_VIDEODRIVER"] = "offscreen"
        for variable in ("WAYLAND_DISPLAY", "DISPLAY"):
            environment.pop(variable, None)
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
    if args.family in FRAME_FAMILIES:
        # The harness writes each frame to its own raw file; the capture carries
        # them from here on.
        report = json.loads(pixels.read_text())
        material_pixel_frames.embed_frames(report, pixels.parent)
        pixels.write_text(json.dumps(report) + "\n")
    report = read_pixels(pixels)
    reference = read_pixels(args.reference) if args.reference else None
    if args.reference:
        evidence["reference"] = {"path": str(args.reference),
                                 "sha256": portal_boot.sha256(args.reference)}
    evidence["pixels_sha256"] = portal_boot.sha256(pixels)
    evidence["failures"] = evaluate(report, args.hdr, reference)
    evidence["status"] = "pass" if not evidence["failures"] else "fail"
    (output / "evidence.json").write_text(json.dumps(evidence, indent=2) + "\n")
    return _print_result(report["family"], args.renderer, args.hdr, evidence["failures"],
                         output / "evidence.json")


def _print_result(family, renderer, hdr, failures, where):
    print("material pixel conformance (%s, %s, hdr %s): %s (%s)"
          % (family, renderer, hdr, "pass" if not failures else "fail", where))
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
    run_parser.add_argument("--family", choices=FAMILIES, default="lightmap")
    run_parser.add_argument("--display", choices=("headless", "desktop"), default="headless")
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
        return _print_result(report["family"], report.get("renderer"), args.hdr,
                             evaluate(report, args.hdr, reference), args.pixels)
    except (PixelsError, ValueError, OSError) as error:
        print("material pixel conformance: invalid input: %s" % error, file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
