#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Native video-cache operation sequences and a deliberately wrong color fixture.

Uses only authored lossless 16x16 clips, with ordinary provider textures and the
installed Portal 2 composition. Retail movie captures are separate evidence.
"""

import argparse
import json
from pathlib import Path
import re
import subprocess
import sys

import conformance
from conformance_result import Checks
import portal_boot


def fixture(root, name, colors, size=16):
    raw = root / (name + ".yuv")
    raw.write_bytes(b"".join(bytes([y]) * (size * size) +
                             bytes([u]) * (size * size // 4) +
                             bytes([v]) * (size * size // 4) for y, u, v in colors))
    clip = root / (name + ".mkv")
    subprocess.run(["ffmpeg", "-v", "error", "-f", "rawvideo", "-pixel_format", "yuv420p",
                    "-video_size", "%dx%d" % (size, size), "-framerate", "24", "-i", str(raw),
                    "-c:v", "ffv1", str(clip)], check=True, timeout=30)
    return clip


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    root = Path(conformance.repo_root())
    parser.add_argument("--runtime", type=Path, default=root / "run/runtime-p2")
    parser.add_argument("--build", type=Path, default=root / "build-p2")
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args(argv)
    output = args.out.resolve()
    output.mkdir(parents=True, exist_ok=True)
    checks = Checks()
    evidence = {"schema": "video-frame-cache-evidence/v1", "status": "fail"}
    try:
        # Limited-range BT.601 solid red, green, blue; no decoder-derived oracle.
        red, green, blue = (82, 90, 240), (145, 54, 34), (41, 240, 110)
        positive = fixture(output, "rgb", [red, green, blue])
        negative = fixture(output, "wrong-green", [red, red, blue])
        invalid = fixture(output, "too-small", [red], size=4)
        evidence["fixtures"] = {p.name: portal_boot.sha256(p)
                                for p in (positive, negative, invalid)}
        evidence["ffmpeg"] = subprocess.run(["ffmpeg", "-version"], capture_output=True,
                                            text=True, check=True).stdout.splitlines()[0]
        boot = output / "boot"
        result = portal_boot.main([
            "--runtime", str(args.runtime), "--build", str(args.build), "--out", str(boot),
            "--game", "portal2", "--map", "sp_a1_intro4_probe64", "--headless",
            "--capture-wait", "30", "--startup-command", "r_core_world 1",
            "--console-command", "video_bink_cache_probe %s %s" % (positive, invalid),
            "--console-command", "video_bink_cache_probe %s %s" % (negative, invalid),
            "--console-command", "r_drawviewmodel 0", "--console-command", "cl_drawhud 0",
            "--console-command", "noclip", "--console-command", "cmd setpos -1552 37 -96",
            "--console-command", "cmd setang 0 -90 0"])
        checks.equal(result, 0, "native boot")
        log = (boot / "runtime/portal2/console.log").read_text(errors="replace")
        records = [(int(count), int(failed)) for count, failed in
                   re.findall(r"VIDEO_CACHE_PROBE checks=(\d+) failures=(\d+)", log)]
        evidence["native_records"] = records
        if checks.equal(len(records), 2, "both native probes executed"):
            checks.check(records[0][0] >= 25, "positive operation count")
            checks.equal(records[0][1], 0, "positive operation sequence")
            checks.check(records[1][0] >= 25, "negative operation count")
            checks.check(records[1][1] > 0, "wrong green rejected")
        evidence["status"] = "pass" if checks.failures == 0 else "fail"
    except (OSError, ValueError, subprocess.SubprocessError) as error:
        checks.check(False, "video cache run", str(error))
    (output / "evidence.json").write_text(json.dumps(evidence, indent=2) + "\n")
    return checks.report()


if __name__ == "__main__":
    sys.exit(main())
