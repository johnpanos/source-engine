#!/usr/bin/env python3
"""Self-test of android_lifecycle.py against a fake `adb` (no device needed).

The fake is a small device state machine: a process that starts, pauses,
loses and regains its surface, rotates, is killed and is trimmed, writing the
SDL Java lifecycle lines to a logcat buffer and PNG frames to screencap. One
named fault per run breaks one behavior; each fault must fail the check that
judges it, and the healthy device must pass all of them. Like the other
Android lanes it proves the oracles, not the product: a device run is
separate and is what certifies anything.

  python3 -m unittest tools/quality/test_android_lifecycle.py
"""

import json
import os
import stat
import subprocess
import sys
import tempfile
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import android_lifecycle as lifecycle  # noqa: E402

PKG = lifecycle.DEFAULT_PACKAGE

FAKE_ADB = r'''#!/usr/bin/env python3
import json, os, struct, sys, zlib

state_path = os.environ["FAKE_ADB_STATE"]
fault = os.environ.get("FAKE_ADB_FAULT", "")
pkg = "%(pkg)s"
args = sys.argv[1:]
if args[:1] == ["-s"]:
    args = args[2:]
try:
    st = json.load(open(state_path))
except OSError:
    st = {"pid": 0, "next": 4000, "running": False, "fg": False, "rot": 0, "log": []}


def save():
    json.dump(st, open(state_path, "w"))


def log(line):
    st["log"].append(line)


def png(width, height, flat):
    rows = b""
    for y in range(height):
        row = bytes(width * 3) if flat else os.urandom(width * 3)
        rows += b"\x00" + row

    def chunk(kind, data):
        body = kind + data
        return struct.pack(">I", len(data)) + body + struct.pack(">I", zlib.crc32(body))
    return (b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0)) +
            chunk(b"IDAT", zlib.compress(rows)) + chunk(b"IEND", b""))


def sdl(text):
    log("10-10 12:00:00.000  %%d  %%d I SDL     : %%s" %% (st["pid"], st["pid"], text))


def start():
    if not st["running"]:
        st["pid"], st["next"], st["running"] = st["next"], st["next"] + 1, True
        if fault != "no-resume":
            sdl("onResume()")
        sdl("surfaceCreated()")
    else:
        if fault != "no-resume":
            sdl("onResume()")
        if fault != "no-surface":
            sdl("surfaceChanged()")
    st["fg"] = True
    if fault == "crash-launch":
        log("10-10 12:00:01.000  %%d  %%d E AndroidRuntime: FATAL EXCEPTION: main" %% (st["pid"], st["pid"]))


cmd = args[0] if args else ""
rest = args[1:]
if cmd == "get-state":
    if fault == "nodevice":
        sys.stderr.write("error: no devices/emulators found\n")
        sys.exit(1)
    print("device")
elif cmd == "logcat":
    if "-c" in rest:
        st["log"] = []
    else:
        print("\n".join(st["log"]))
elif cmd == "exec-out" and rest[:2] == ["screencap", "-p"]:
    width, height = 320, 200
    if st["rot"] %% 2 and fault != "no-rotate-frame":
        width, height = height, width
    sys.stdout.buffer.write(png(width, height, fault == "blank"))
elif cmd == "shell":
    line = " ".join(rest)
    if line.startswith("pm path"):
        if fault != "notinstalled":
            print("package:/data/app/%%s/base.apk" %% pkg)
        else:
            sys.exit(1)
    elif line.startswith("pidof"):
        if st["running"]:
            print(st["pid"])
        else:
            sys.exit(1)
    elif line.startswith("am start"):
        start()
        print("Status: ok")
    elif line.startswith("am force-stop"):
        st["running"] = st["fg"] = False
    elif line.startswith("am kill"):
        if st["running"] and not st["fg"] and fault != "survives-kill":
            st["running"] = False
            log("10-10 12:00:02.000  500  500 I ActivityManager: Process %%s (pid %%d) has died: cch  CEM" %% (pkg, st["pid"]))
    elif line.startswith("am send-trim-memory"):
        if fault == "trim-crash":
            st["running"] = False
            log("10-10 12:00:03.000  %%d  %%d F libc    : Fatal signal 11 (SIGSEGV)" %% (st["pid"], st["pid"]))
        elif fault == "trim-rejected":
            print("Error: Unknown trim level")
    elif line.startswith("input keyevent KEYCODE_HOME"):
        st["fg"] = False
        if fault != "no-pause":
            sdl("onPause()")
        sdl("surfaceDestroyed()")
    elif line.startswith("cmd window user-rotation lock"):
        st["rot"] = int(line.split()[-1])
        if fault != "no-surface":
            sdl("surfaceChanged()")
        if fault == "rotate-restarts":
            st["pid"], st["next"] = st["next"], st["next"] + 1
    elif line.startswith("cmd window user-rotation free") or line.startswith("settings"):
        pass
    else:
        sys.exit(2)
else:
    sys.exit(2)
save()
''' % {"pkg": PKG}


class FakeAdbTest(unittest.TestCase):
    def setUp(self):
        self.dir = tempfile.TemporaryDirectory()
        self.addCleanup(self.dir.cleanup)
        self.adb = os.path.join(self.dir.name, "adb")
        with open(self.adb, "w") as handle:
            handle.write(FAKE_ADB)
        os.chmod(self.adb, os.stat(self.adb).st_mode | stat.S_IXUSR)
        self.state = os.path.join(self.dir.name, "state.json")
        self.out = os.path.join(self.dir.name, "out")

    def lifecycle(self, fault="", steps=None):
        os.environ["FAKE_ADB_STATE"] = self.state
        os.environ["FAKE_ADB_FAULT"] = fault
        if os.path.exists(self.state):
            os.remove(self.state)
        argv = ["check", "--adb", self.adb, "--out", self.out, "--settle", "0",
                "--timeout", "1"]
        if steps:
            argv += ["--steps", steps]
        code = lifecycle.main(argv)
        report = os.path.join(self.out, "lifecycle.json")
        checks = []
        if os.path.exists(report):
            with open(report, encoding="utf-8") as handle:
                checks = json.load(handle)["checks"]
            os.remove(report)
        return code, checks

    def failed(self, checks):
        return sorted({"%s: %s" % (c["step"], c["check"]) for c in checks if not c["ok"]})

    def assertFails(self, fault, step, name, steps=None):
        code, checks = self.lifecycle(fault, steps)
        self.assertEqual(code, 1, "fault %r must fail the run" % fault)
        self.assertIn("%s: %s" % (step, name), self.failed(checks),
                      "fault %r must fail %s: %s; failed: %s" %
                      (fault, step, name, self.failed(checks)))

    def test_healthy_device_passes_every_check(self):
        code, checks = self.lifecycle()
        self.assertEqual(self.failed(checks), [])
        self.assertEqual(code, 0)
        steps = {c["step"] for c in checks}
        for want in ("launch", "rotate-90", "rotate-0", "background", "kill", "trim"):
            self.assertIn(want, steps)
        self.assertGreaterEqual(len(checks), 30)

    def test_frames_are_kept_after_resume(self):
        self.lifecycle()
        names = os.listdir(self.out)
        for label in ("launch", "rotate-90", "foreground", "relaunch", "trim"):
            self.assertTrue(any(label in n for n in names), label)

    def test_no_device_is_unverified_never_a_pass(self):
        code, checks = self.lifecycle("nodevice")
        self.assertEqual(code, 3)
        self.assertEqual(checks, [])

    def test_missing_package_is_unverified(self):
        code, checks = self.lifecycle("notinstalled")
        self.assertEqual(code, 3)

    def test_missing_resume_line_fails_launch(self):
        self.assertFails("no-resume", "launch", "activity resumed (SDL onResume)")

    def test_blank_frame_fails(self):
        self.assertFails("blank", "launch", "frame is not blank")

    def test_crash_in_logcat_fails_the_step(self):
        self.assertFails("crash-launch", "launch", "no crash, native signal or ANR in logcat")

    def test_rotation_that_keeps_the_frame_shape_fails(self):
        self.assertFails("no-rotate-frame", "rotate-90", "frame is portrait")

    def test_missing_surface_recreation_fails_rotation(self):
        self.assertFails("no-surface", "rotate-90",
                         "surface recreated (surfaceChanged or surfaceCreated)")

    def test_a_restart_on_rotation_fails_same_process(self):
        self.assertFails("rotate-restarts", "rotate-90",
                         "same process (the activity handles the change)")

    def test_missing_pause_line_fails_background(self):
        self.assertFails("no-pause", "background", "activity paused (SDL onPause)")

    def test_process_that_survives_the_kill_fails(self):
        self.assertFails("survives-kill", "kill", "process gone after am kill")

    def test_trim_crash_fails(self):
        self.assertFails("trim-crash", "trim", "process survives RUNNING_CRITICAL")

    def test_rejected_trim_fails(self):
        self.assertFails("trim-rejected", "trim", "send-trim-memory accepted")

    def test_zero_checks_never_pass(self):
        # an adb whose every call fails with a device present yields launch
        # failures, never an empty pass
        code, checks = self.lifecycle("no-resume", steps="launch")
        self.assertNotEqual(code, 0)
        self.assertTrue(checks)

    def test_png_info_rejects_truncated_captures(self):
        good = subprocess.run([self.adb, "exec-out", "screencap", "-p"], capture_output=True,
                              env=dict(os.environ, FAKE_ADB_STATE=self.state)).stdout
        self.assertIsNotNone(lifecycle.png_info(good))
        self.assertIsNone(lifecycle.png_info(good[:len(good) // 2]))
        self.assertIsNone(lifecycle.png_info(b"not a png"))


if __name__ == "__main__":
    unittest.main()
