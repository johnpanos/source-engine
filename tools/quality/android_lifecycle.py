#!/usr/bin/env python3
"""Android activity lifecycle lane for the installed Portal 2 APK (R29, M7).

Drives one attached device through the lifecycle the platform obligations in
AGENTS.md name, each step judged by an oracle read from the device (logcat,
the process table, a captured frame) rather than by the commands succeeding:

  launch      cold start: process exists, the activity resumes, frame drawn
  rotate      rotation and surface recreation: the frame's size follows the
              rotation, the surface is recreated, the process is the same
  background  home, then foreground: pause and surface loss on the way out,
              resume and a new surface on the way back, same process
  kill        the system kills the backgrounded process, then relaunch:
              the pid is gone, a new one starts and draws
  trim        am send-trim-memory (RUNNING_CRITICAL): process survives and
              keeps drawing, no crash

Every step ends with a frame captured after resume (`screencap -p`); a blank
or missing frame fails the step. A crash record in logcat (FATAL EXCEPTION,
a native signal, an ANR) fails every step it falls in.

  tools/quality/android_lifecycle.py check [--device SERIAL] [--adb PATH]
        [--package ID] [--activity CLASS] [--out DIR] [--steps a,b,...]

Install and content sync are not this tool's: `./kiln play`/`kiln deploy`
over the adb transport own them. `check` exits 3 when no device is attached
or the package is not installed: the profile is then unverified, never
passed (AGENTS.md). Exit 1 is a failed check, 0 is every check passing.

The logcat patterns are data (PATTERNS below), the SDL Java activity's
verbose lifecycle lines, and are to be confirmed on the first device run;
a pattern that never matches fails its check by name, it does not pass.
"""

from __future__ import annotations

import argparse
import json
import os
import re
import struct
import subprocess
import sys
import time
import zlib

DEFAULT_PACKAGE = "org.sourceengine.portal2"
DEFAULT_ACTIVITY = "org.libsdl.app.SDLActivity"

# Oracles read from logcat (`logcat -d -v threadtime`) after the step began.
PATTERNS = {
    "resumed": r"\bSDL\b.*onResume\(\)",
    "paused": r"\bSDL\b.*onPause\(\)",
    "surface_created": r"\bSDL\b.*surfaceCreated\(\)",
    "surface_changed": r"\bSDL\b.*surfaceChanged\(\)",
    "surface_destroyed": r"\bSDL\b.*surfaceDestroyed\(\)",
    "died": r"Process \S*%PKG% \(pid \d+\) has died|Killing \d+:%PKG%/|"
            r"Force removing ProcessRecord\{[^}]*%PKG%",
}
# A crash anywhere in a step's window fails it.
CRASH = re.compile(
    r"FATAL EXCEPTION|Fatal signal \d+|\*\*\* \*\*\* \*\*\*|ANR in |"
    r"am_crash|am_anr|SIGABRT|SIGSEGV")

# A captured frame is blank when its PNG is this small per pixel (a flat
# colour compresses to a few bytes; a rendered scene does not).
MIN_PNG_BYTES_PER_PIXEL = 0.02

STEPS = ("launch", "rotate", "background", "kill", "trim")


class Unverified(Exception):
    """No device, or the product is not installed: not a pass, not a failure."""


class Device:
    def __init__(self, adb, serial, package, activity, out_dir, settle=3.0, timeout=60.0):
        self.adb = adb
        self.serial = serial
        self.package = package
        self.activity = activity
        self.out_dir = out_dir
        self.settle = settle
        self.timeout = timeout
        self.frames = 0

    def limit(self, timeout):
        """A step's wait, never longer than the run's --timeout."""
        return self.timeout if timeout is None else min(timeout, self.timeout)

    def _cmd(self, *args):
        return [self.adb] + (["-s", self.serial] if self.serial else []) + list(args)

    def run(self, *args, timeout=60.0, binary=False):
        try:
            done = subprocess.run(self._cmd(*args), capture_output=True, timeout=timeout)
        except (OSError, subprocess.TimeoutExpired) as error:
            return 255, b"" if binary else "", str(error)
        out = done.stdout if binary else done.stdout.decode("utf-8", "replace")
        return done.returncode, out, done.stderr.decode("utf-8", "replace")

    def shell(self, text, timeout=60.0):
        code, out, err = self.run("shell", text, timeout=timeout)
        return code, out + err

    def state(self):
        code, out, _ = self.run("get-state", timeout=15.0)
        return out.strip() if code == 0 else ""

    def installed(self):
        code, out = self.shell("pm path %s" % self.package)
        return code == 0 and "package:" in out

    def pid(self):
        code, out, _ = self.run("shell", "pidof %s" % self.package, timeout=15.0)
        text = out.strip().split()
        return int(text[0]) if code == 0 and text and text[0].isdigit() else None

    def wait_pid(self, present, timeout=None):
        end = time.time() + self.limit(timeout)
        while time.time() < end:
            pid = self.pid()
            if (pid is not None) == present:
                return pid
            time.sleep(0.2)
        return self.pid()

    def clear_log(self):
        self.run("logcat", "-b", "all", "-c", timeout=15.0)

    def log(self):
        code, out, _ = self.run("logcat", "-b", "all", "-d", "-v", "threadtime", timeout=60.0)
        return out if code == 0 else ""

    def wait_log(self, key, timeout=None):
        """The first logcat line matching PATTERNS[key], or None at the timeout."""
        pattern = re.compile(PATTERNS[key].replace("%PKG%", re.escape(self.package)))
        end = time.time() + self.limit(timeout)
        while True:
            for line in self.log().splitlines():
                if pattern.search(line):
                    return line
            if time.time() >= end:
                return None
            time.sleep(0.5)

    def crashes(self):
        return [l for l in self.log().splitlines() if CRASH.search(l)]

    def start(self):
        return self.shell("am start -W -n %s/%s" % (self.package, self.activity), timeout=90.0)

    def frame(self, label):
        """Capture the screen; (width, height, density, path) or None."""
        code, png, _ = self.run("exec-out", "screencap", "-p", timeout=60.0, binary=True)
        info = png_info(png) if code == 0 else None
        if info is None:
            return None
        self.frames += 1
        path = os.path.join(self.out_dir, "frame-%02d-%s.png" % (self.frames, label))
        os.makedirs(self.out_dir, exist_ok=True)
        with open(path, "wb") as handle:
            handle.write(png)
        width, height = info
        return width, height, len(png) / float(width * height), path


def png_info(data):
    """(width, height) of a PNG, or None when the bytes are not one."""
    if len(data) < 33 or data[:8] != b"\x89PNG\r\n\x1a\n" or data[12:16] != b"IHDR":
        return None
    width, height = struct.unpack(">II", data[16:24])
    if width == 0 or height == 0:
        return None
    try:  # the image data must inflate: a truncated capture is not a frame
        pos, joined = 8, b""
        while pos + 8 <= len(data):
            size, kind = struct.unpack(">I4s", data[pos:pos + 8])
            if kind == b"IDAT":
                joined += data[pos + 8:pos + 8 + size]
            pos += 12 + size
        zlib.decompress(joined)
    except (zlib.error, struct.error):
        return None
    return width, height


class Report:
    def __init__(self):
        self.checks = []

    def check(self, step, name, ok, detail=""):
        self.checks.append({"step": step, "check": name, "ok": bool(ok), "detail": detail})
        print("%s %s: %s%s" % ("PASS" if ok else "FAIL", step, name,
                               " (%s)" % detail if detail and not ok else ""))
        return ok

    @property
    def failures(self):
        return [c for c in self.checks if not c["ok"]]


def frame_check(report, step, device, label, expect=None):
    """The post-resume frame: present, not blank, and (when given) its shape."""
    time.sleep(device.settle)
    frame = device.frame(label)
    if not report.check(step, "frame captured after resume", frame is not None,
                        "screencap produced no PNG"):
        return None
    width, height, density, path = frame
    report.check(step, "frame is not blank", density >= MIN_PNG_BYTES_PER_PIXEL,
                 "%.4f PNG bytes per pixel in %s" % (density, path))
    if expect == "portrait" or expect == "landscape":
        want = (height > width) if expect == "portrait" else (width > height)
        report.check(step, "frame is %s" % expect, want, "%dx%d" % (width, height))
    return frame


def no_crash(report, step, device):
    found = device.crashes()
    report.check(step, "no crash, native signal or ANR in logcat", not found,
                 found[0].strip()[:160] if found else "")


def step_launch(report, device):
    device.shell("am force-stop %s" % device.package)
    device.wait_pid(False, 15.0)
    device.clear_log()
    device.start()
    pid = device.wait_pid(True)
    report.check("launch", "process exists after am start", pid is not None)
    report.check("launch", "activity resumed (SDL onResume)",
                 device.wait_log("resumed") is not None)
    report.check("launch", "surface created",
                 device.wait_log("surface_created", 15.0) is not None)
    frame_check(report, "launch", device, "launch")
    no_crash(report, "launch", device)
    return pid


def user_rotation(device, rotation):
    code, _ = device.shell("cmd window user-rotation lock %d" % rotation)
    if code != 0:  # the pre-Android-11 route
        device.shell("settings put system accelerometer_rotation 0")
        code, _ = device.shell("settings put system user_rotation %d" % rotation)
    return code == 0


def step_rotate(report, device, pid):
    before = device.frame("rotate-before")
    if before is None:
        report.check("rotate", "frame before rotating", False, "screencap produced no PNG")
        return pid
    natural = "landscape" if before[0] > before[1] else "portrait"
    turned = "portrait" if natural == "landscape" else "landscape"
    for rotation, expect in ((1, turned), (0, natural)):
        device.clear_log()
        step = "rotate-%d" % (rotation * 90)
        report.check(step, "rotation request accepted", user_rotation(device, rotation))
        report.check(step, "surface recreated (surfaceChanged or surfaceCreated)",
                     device.wait_log("surface_changed", 20.0) is not None or
                     device.wait_log("surface_created", 5.0) is not None)
        frame_check(report, step, device, step, expect)
        report.check(step, "same process (the activity handles the change)",
                     device.pid() == pid, "pid %s -> %s" % (pid, device.pid()))
        no_crash(report, step, device)
    device.shell("cmd window user-rotation free")
    return pid


def step_background(report, device, pid):
    device.clear_log()
    device.shell("input keyevent KEYCODE_HOME")
    report.check("background", "activity paused (SDL onPause)",
                 device.wait_log("paused", 20.0) is not None)
    report.check("background", "surface released (surfaceDestroyed)",
                 device.wait_log("surface_destroyed", 20.0) is not None)
    report.check("background", "process kept while backgrounded", device.pid() == pid,
                 "pid %s -> %s" % (pid, device.pid()))
    device.clear_log()
    device.start()
    report.check("background", "activity resumed (SDL onResume)",
                 device.wait_log("resumed", 30.0) is not None)
    report.check("background", "a new surface after foreground",
                 device.wait_log("surface_changed", 20.0) is not None or
                 device.wait_log("surface_created", 5.0) is not None)
    frame_check(report, "background", device, "foreground")
    report.check("background", "same process after foreground", device.pid() == pid,
                 "pid %s -> %s" % (pid, device.pid()))
    no_crash(report, "background", device)
    return pid


def step_kill(report, device, pid):
    device.shell("input keyevent KEYCODE_HOME")  # `am kill` takes only background processes
    time.sleep(2.0)
    device.clear_log()
    device.shell("am kill %s" % device.package)
    gone = device.wait_pid(False, 20.0) is None
    report.check("kill", "process gone after am kill", gone,
                 "pid still %s" % device.pid())
    device.clear_log()
    device.start()
    new = device.wait_pid(True)
    report.check("kill", "relaunch starts a process", new is not None)
    report.check("kill", "relaunch is a new process", new is not None and new != pid,
                 "pid %s -> %s" % (pid, new))
    report.check("kill", "relaunched activity resumed (SDL onResume)",
                 device.wait_log("resumed") is not None)
    frame_check(report, "kill", device, "relaunch")
    no_crash(report, "kill", device)
    return new


def step_trim(report, device, pid):
    device.clear_log()
    code, out = device.shell("am send-trim-memory %s RUNNING_CRITICAL" % device.package)
    report.check("trim", "send-trim-memory accepted", code == 0 and "Error" not in out,
                 out.strip()[:160])
    time.sleep(device.settle)
    report.check("trim", "process survives RUNNING_CRITICAL", device.pid() == pid,
                 "pid %s -> %s" % (pid, device.pid()))
    frame_check(report, "trim", device, "trim")
    no_crash(report, "trim", device)
    return pid


RUNNERS = {"launch": None, "rotate": step_rotate, "background": step_background,
           "kill": step_kill, "trim": step_trim}


def run(device, steps):
    if device.state() != "device":
        raise Unverified("no device is attached (adb get-state: %r)" % device.state())
    if not device.installed():
        raise Unverified("%s is not installed on the device" % device.package)
    report = Report()
    pid = step_launch(report, device)  # every later step needs a running app
    if pid is None:
        return report
    for name in STEPS[1:]:
        if name in steps:
            pid = RUNNERS[name](report, device, pid)
            if pid is None:
                report.check(name, "process alive at the end of the step", False)
                break
    return report


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    sub = parser.add_subparsers(dest="command", required=True)
    check = sub.add_parser("check")
    check.add_argument("--device", default=os.environ.get("ANDROID_SERIAL"))
    check.add_argument("--adb", default="adb")
    check.add_argument("--package", default=DEFAULT_PACKAGE)
    check.add_argument("--activity", default=DEFAULT_ACTIVITY)
    check.add_argument("--out", default="out/android-lifecycle")
    check.add_argument("--steps", default=",".join(STEPS))
    check.add_argument("--settle", type=float, default=3.0)
    check.add_argument("--timeout", type=float, default=60.0)
    args = parser.parse_args(argv)
    steps = [s for s in args.steps.split(",") if s]
    unknown = [s for s in steps if s not in STEPS]
    if unknown:
        parser.error("unknown steps: %s" % ", ".join(unknown))
    device = Device(args.adb, args.device, args.package, args.activity, args.out,
                    args.settle, args.timeout)
    try:
        report = run(device, steps)
    except Unverified as why:
        print("UNVERIFIED: %s" % why)
        return 3
    os.makedirs(args.out, exist_ok=True)
    with open(os.path.join(args.out, "lifecycle.json"), "w", encoding="utf-8") as handle:
        json.dump({"schema": "android-lifecycle/v1", "package": args.package,
                   "device": args.device, "checks": report.checks}, handle, indent=1)
    total = len(report.checks)
    print("android lifecycle: %d checks, %d failed" % (total, len(report.failures)))
    if total == 0:
        return 1  # zero checks never pass
    return 1 if report.failures else 0


if __name__ == "__main__":
    sys.exit(main())
