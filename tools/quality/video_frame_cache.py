#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Native video-cache operation sequences and a deliberately wrong color fixture.

Uses only authored lossless 16x16 clips, with ordinary provider textures and the
installed Portal 2 composition. Retail movie captures are separate evidence.
The same sequences run twice: FFV1 clips through the Bink provider's material,
and lossless AV1 clips through the av1 provider's (which must also refuse the
FFV1 clip, a valid non-AV1 movie).
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

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "kiln"))
import sepipe_loader  # noqa: E402


def fixture(root, name, colors, size=16, codec="ffv1"):
    raw = root / (name + ".yuv")
    raw.write_bytes(b"".join(bytes([y]) * (size * size) +
                             bytes([u]) * (size * size // 4) +
                             bytes([v]) * (size * size // 4) for y, u, v in colors))
    clip = root / (name + (".mkv" if codec == "ffv1" else ".webm"))
    # AV1 clips are lossless (libaom, crf 0), so the colours are the authored ones.
    encoder = ["-c:v", "ffv1"] if codec == "ffv1" else \
        ["-c:v", "libaom-av1", "-crf", "0", "-cpu-used", "8", "-g", "1"]
    subprocess.run(["ffmpeg", "-v", "error", "-f", "rawvideo", "-pixel_format", "yuv420p",
                    "-video_size", "%dx%d" % (size, size), "-framerate", "24", "-i", str(raw),
                    *encoder, str(clip)], check=True, timeout=60)
    return clip


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    root = Path(conformance.repo_root())
    sepipe_loader.add_arguments(parser, "portal2")
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
        av1_positive = fixture(output, "rgb", [red, green, blue], codec="av1")
        av1_negative = fixture(output, "wrong-green", [red, red, blue], codec="av1")
        av1_invalid = fixture(output, "too-small", [red], size=4, codec="av1")
        evidence["fixtures"] = {p.name: portal_boot.sha256(p)
                                for p in (positive, negative, invalid, av1_positive,
                                          av1_negative, av1_invalid)}
        evidence["ffmpeg"] = subprocess.run(["ffmpeg", "-version"], capture_output=True,
                                            text=True, check=True).stdout.splitlines()[0]
        boot = output / "boot"
        result = portal_boot.main([
            *sepipe_loader.boot_arguments(args), "--out", str(boot),
            "--map", "sp_a1_intro4_relit", "--headless",
            "--capture-wait", "30", "--startup-command", "r_core_world 1",
            "--console-command", "video_bink_cache_probe %s %s" % (positive, invalid),
            "--console-command", "video_bink_cache_probe %s %s" % (negative, invalid),
            "--console-command", "video_bink_cache_probe %s %s av1 %s" % (
                av1_positive, av1_invalid, positive),
            "--console-command", "video_bink_cache_probe %s %s av1 %s" % (
                av1_negative, av1_invalid, positive),
            "--console-command", "vgui_texture_borrow_probe",
            "--console-command", "r_drawviewmodel 0", "--console-command", "cl_drawhud 0",
            "--console-command", "noclip", "--console-command", "cmd setpos -1552 37 -96",
            "--console-command", "cmd setang 0 -90 0"])
        checks.equal(result, 0, "native boot")
        log = (boot / "runtime/portal2/console.log").read_text(errors="replace")
        records = [(mode, int(count), int(failed)) for mode, count, failed in
                   re.findall(r"VIDEO_CACHE_PROBE mode=(\w+) checks=(\d+) failures=(\d+)", log)]
        evidence["native_records"] = records
        if checks.equal([r[0] for r in records], ["any", "any", "av1", "av1"],
                        "all four native probes executed"):
            for offset, mode, count in ((0, "ffv1", 22), (2, "av1", 24)):
                checks.check(records[offset][1] >= count, mode + " positive operation count")
                checks.equal(records[offset][2], 0, mode + " positive operation sequence")
                checks.check(records[offset + 1][1] >= count, mode + " negative operation count")
                checks.check(records[offset + 1][2] > 0, mode + " wrong green rejected")
        borrowed = re.findall(r"VGUI_TEXTURE_BORROW_PROBE checks=(\d+) failures=(\d+)", log)
        evidence["vgui_records"] = borrowed
        if checks.equal(len(borrowed), 1, "borrowed texture probe executed"):
            checks.check(int(borrowed[0][0]) >= 7, "borrowed texture operation count")
            checks.equal(int(borrowed[0][1]), 0, "borrowed texture operation sequence")
        evidence["status"] = "pass" if checks.failures == 0 else "fail"
    except (OSError, ValueError, subprocess.SubprocessError) as error:
        checks.check(False, "video cache run", str(error))
    (output / "evidence.json").write_text(json.dumps(evidence, indent=2) + "\n")
    return checks.report()


if __name__ == "__main__":
    sys.exit(main())
