#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Queued and direct rendering draw the same frame (mat_queue_mode 0 against 2).

The desktop profiles play with the queued material system (mat_queue_mode 2:
the main thread builds the next frame while a render thread replays the
current one); the harnesses mostly boot direct (mode 0). A defect that only
the queued mode shows (state captured on the wrong thread, a draw taken from
a stale binding, a race in the shader API) then reaches the user unseen.

Each scenario boots headless twice through portal_boot.py, once per mode,
with the same map, view and settle wait, and judges the two frames the
renderer captured at the end of the same frame (-core_capture; an engine
screenshot would take its frame out of queued mode): the share of pixels differing by more than 16 levels in any
channel must stay under the scenario's limit, and the mean absolute
difference under its own. A control compares the direct frame with a frame
of the same map at another view, which must exceed both limits (the
comparison detects a different image).

  python3 tools/quality/queue_mode_parity.py run --out /tmp/claude-1000/qmp
  python3 tools/quality/queue_mode_parity.py run --scenario p2-intro3-moss

Prints one `CONFORMANCE <checks> <failures>` record (checks-v1).
"""

import argparse
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools" / "quality"))
from conformance_result import Checks  # noqa: E402

# name: (profile, map, console commands placing the view)
SCENARIOS = {
    "p1-testchmb-a-01": ("portal", "testchmb_a_01", []),
    "p2-intro4-spawn": ("portal2", "sp_a1_intro4", []),
    "p2-intro3-moss": ("portal2", "sp_a1_intro3",
                       ["sv_cheats 1", "setpos -1300 2624 -190", "setang 20 180 0"]),
}
# The control's view: the same map, turned around.
CONTROL = ("portal2", "sp_a1_intro3", ["sv_cheats 1", "setpos -1300 2624 -190", "setang 20 0 0"])
SETTLE = "wait 300"
# The frame the renderer captures: after the map, the view and the settle wait.
CAPTURE_FRAME = 700
# A pixel differs when a channel moves more than this many levels.
PIXEL_LEVELS = 16
# Limits: the share of differing pixels and the mean absolute difference.
MAX_DIFFERING = 0.02
MAX_MEAN = 2.0


def boot(profile, level, commands, mode, out):
    cmd = [sys.executable, str(ROOT / "tools/quality/portal_boot.py"), "--profile", profile,
           "--headless", "--map", level, "--out", str(out),
           "--startup-command", "mat_queue_mode %d" % mode,
           "--engine-arg=-core_capture", "--engine-arg=%d" % CAPTURE_FRAME,
           "--engine-arg=-core_capture_path", "--engine-arg=%s.ppm" % out]
    for command in commands + [SETTLE]:
        cmd.append("--console-command=" + command)
    with open(str(out) + ".log", "w") as log:
        status = subprocess.run(cmd, cwd=ROOT, stdout=log, stderr=subprocess.STDOUT).returncode
    shot = Path(str(out) + ".ppm")
    return status, (shot if shot.is_file() else None)


def compare(a, b):
    """(share of pixels differing by more than PIXEL_LEVELS, mean absolute difference)."""
    from PIL import Image, ImageChops, ImageStat
    first = Image.open(a).convert("RGB")
    second = Image.open(b).convert("RGB")
    if first.size != second.size:
        return 1.0, 255.0
    diff = ImageChops.difference(first, second)
    mean = sum(ImageStat.Stat(diff).mean) / 3.0
    channels = diff.split()
    mask = channels[0].point(lambda v: 255 if v > PIXEL_LEVELS else 0)
    for channel in channels[1:]:
        mask = ImageChops.lighter(mask, channel.point(lambda v: 255 if v > PIXEL_LEVELS else 0))
    differing = mask.histogram()[255] / float(first.size[0] * first.size[1])
    return differing, mean


def run(args):
    checks = Checks()
    out = Path(args.out)
    out.mkdir(parents=True, exist_ok=True)
    names = args.scenario or list(SCENARIOS)
    direct_control = None
    for name in names:
        profile, level, commands = SCENARIOS[name]
        shots = {}
        for mode in (0, 2):
            status, shot = boot(profile, level, commands, mode, out / ("%s-q%d" % (name, mode)))
            checks.check(status == 0 and shot is not None, "%s.mode%d.boots" % (name, mode),
                         "exit %d, screenshot %s" % (status, shot))
            shots[mode] = shot
        if not ( shots[0] and shots[2] ):
            continue
        differing, mean = compare(shots[0], shots[2])
        print("%s: %.4f of pixels differ by more than %d levels, mean %.2f" %
              (name, differing, PIXEL_LEVELS, mean))
        checks.within(differing, None, MAX_DIFFERING, name + ".pixels-match")
        checks.within(mean, None, MAX_MEAN, name + ".mean-matches")
        if name == "p2-intro3-moss":
            direct_control = shots[0]
    if direct_control and not args.no_control:
        profile, level, commands = CONTROL
        status, shot = boot(profile, level, commands, 0, out / "control")
        if checks.check(status == 0 and shot is not None, "control.boots"):
            differing, mean = compare(direct_control, shot)
            print("control: %.4f of pixels differ, mean %.2f" % (differing, mean))
            checks.check(differing > MAX_DIFFERING and mean > MAX_MEAN,
                         "control.another-view-is-detected",
                         "differing %.4f, mean %.2f" % (differing, mean))
    return checks.report()


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = parser.add_subparsers(dest="command", required=True)
    runner = sub.add_parser("run")
    runner.add_argument("--out", default="/tmp/claude-1000/queue-mode-parity")
    runner.add_argument("--scenario", action="append", choices=sorted(SCENARIOS))
    runner.add_argument("--no-control", action="store_true")
    args = parser.parse_args(argv)
    return run(args)


if __name__ == "__main__":
    sys.exit(main())
