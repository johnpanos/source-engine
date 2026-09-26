#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Run and judge the native Vulkan legacy shader ports.

Stages a Portal runtime with a build tree, draws every case of the legacy
shader case files (quality/fixtures/legacy-shaders/*.vdf) through the material
pixel harness's "legacy" family with -vklegacycapture, and judges the pixels
against the shipped D3D9 bytecode (legacy_shader_oracle.py).

    python3 tools/quality/legacy_shader_conformance.py --runtime RUNTIME \\
        --build build-vk --out quality-results/legacy [--cases FILE ...] [--hdr none]

Writes <out>/evidence.json (legacy-shader-conformance/v1) with the per-case
results, and exits 0 only when every case drew through a port and matched.
"""

import argparse
import datetime
import json
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import conformance  # noqa: E402
import legacy_shader_oracle  # noqa: E402
import material_pixel_conformance  # noqa: E402
import portal_boot  # noqa: E402

EVIDENCE_SCHEMA = "legacy-shader-conformance/v1"
ROOT = Path(__file__).resolve().parents[2]
CASE_DIRECTORY = ROOT / "quality/fixtures/legacy-shaders"
ROOT_KEY = re.compile(r'^\s*"LegacyShaderCases"\s*$', re.M)


def merge_case_files(paths):
    """One KeyValues case file holding the members of every given file."""
    bodies = []
    for path in paths:
        text = Path(path).read_text()
        match = ROOT_KEY.search(text)
        if not match:
            raise ValueError("%s: no \"LegacyShaderCases\" root" % path)
        start = text.index("{", match.end())
        end = text.rindex("}")
        bodies.append("// %s\n%s" % (Path(path).name, text[start + 1:end]))
    return "\"LegacyShaderCases\"\n{\n%s\n}\n" % "\n".join(bodies)


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--runtime", type=Path, required=True)
    parser.add_argument("--build", type=Path, required=True)
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--cases", type=Path, action="append",
                        help="case file (repeatable; default every %s/*.vdf)"
                             % CASE_DIRECTORY.relative_to(ROOT))
    parser.add_argument("--hdr", choices=("none", "integer"), default="none")
    parser.add_argument("--renderer", default="native-vulkan")
    parser.add_argument("--display", choices=("headless", "desktop"), default="headless")
    parser.add_argument("--tolerance", type=int, default=legacy_shader_oracle.DEFAULT_TOLERANCE)
    parser.add_argument("--timeout", type=float, default=300)
    parser.add_argument("--shaders", choices=("source", "retail"), default="source",
                        help="oracle bytecode: compiled from this tree's .fxc with the pinned "
                             "FXC (default) or the retail .vcs files")
    parser.add_argument("--fxc-cache", type=Path, help="compiled-combo cache for --shaders source")
    parser.add_argument("--vpk", type=Path, help="with --shaders retail, the VPK holding shaders/fxc")
    parser.add_argument("--extra-arg", action="append", default=[],
                        help="extra harness argument (repeatable), e.g. --extra-arg=-vkvalidate")
    args = parser.parse_args(argv)

    output = args.out.resolve()
    output.mkdir(parents=True, exist_ok=True)
    if (output / "evidence.json").exists():
        print("legacy shader conformance: evidence already exists; use a new --out",
              file=sys.stderr)
        return 2
    case_files = args.cases or sorted(CASE_DIRECTORY.glob("*.vdf"))
    if not case_files:
        print("legacy shader conformance: no case files", file=sys.stderr)
        return 2
    evidence = {"schema": EVIDENCE_SCHEMA, "status": "fail",
                "started_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
                "source": conformance.source_identity(conformance.repo_root()),
                "renderer": args.renderer, "hdr": args.hdr, "tolerance": args.tolerance,
                "case_files": [str(Path(path).resolve().relative_to(ROOT))
                               if Path(path).resolve().is_relative_to(ROOT) else str(path)
                               for path in case_files]}
    cases = output / "cases.vdf"
    cases.write_text(merge_case_files(case_files))
    stage = output / "runtime"
    evidence["staging"] = portal_boot.stage_runtime(args.runtime, stage)
    evidence["build_overrides"] = portal_boot.install_build(args.build, stage)
    harness = stage / "material_pixel_conformance"
    shutil.copy2(material_pixel_conformance.find_harness(args.build), harness)
    evidence["harness_sha256"] = portal_boot.sha256(harness)
    pixels = output / "pixels.json"
    capture = output / "capture.jsonl"
    command = [str(harness), "-game", "portal", "-renderer", args.renderer, "-hdr", args.hdr,
               "-family", "legacy", "-cases", str(cases), "-vklegacycapture", str(capture),
               "-out", str(pixels)] + args.extra_arg
    environment = os.environ.copy()
    environment["LD_LIBRARY_PATH"] = str(stage / "bin") + ":" + environment.get("LD_LIBRARY_PATH", "")
    if args.display == "headless":
        environment["SDL_VIDEODRIVER"] = "offscreen"
        for variable in ("WAYLAND_DISPLAY", "DISPLAY"):
            environment.pop(variable, None)
    evidence["command"] = command

    def finish(failures, code):
        evidence["failures"] = failures
        evidence["status"] = "pass" if not failures else "fail"
        (output / "evidence.json").write_text(json.dumps(evidence, indent=2) + "\n")
        print("legacy shader conformance (%s, hdr %s): %s (%s)"
              % (args.renderer, args.hdr, evidence["status"], output / "evidence.json"))
        for failure in failures:
            print("  " + failure)
        return code

    with open(output / "harness.log", "w") as log:
        try:
            completed = subprocess.run(command, cwd=stage, env=environment, stdout=log,
                                       stderr=subprocess.STDOUT, timeout=args.timeout)
            evidence["returncode"] = completed.returncode
        except subprocess.TimeoutExpired:
            return finish(["harness timed out after %gs" % args.timeout], 2)
    if evidence["returncode"] != 0 or not pixels.is_file():
        return finish(["harness exited %s without a report" % evidence["returncode"]], 2)
    try:
        source = legacy_shader_oracle.make_source(args.shaders, args.fxc_cache, args.vpk)
    except (legacy_shader_oracle.OracleError, ValueError, OSError) as error:
        return finish(["the oracle cannot run: %s" % error], 2)
    evidence["reference_shaders"] = args.shaders
    if args.shaders == "source":
        evidence["compiler"] = source.profile["compiler"]
    else:
        evidence["vpk"] = source.vpk
    report = json.loads(pixels.read_text())
    passes = legacy_shader_oracle.load_capture(capture) if capture.is_file() else []
    failures, details = legacy_shader_oracle.evaluate(report, passes, source, args.tolerance)
    evidence["pixels_sha256"] = portal_boot.sha256(pixels)
    evidence["cases"] = details
    evidence["case_count"] = len(details)
    return finish(failures, 0 if not failures else 1)


if __name__ == "__main__":
    sys.exit(main())
