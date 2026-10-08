#!/usr/bin/env python3
"""render.device.v2.pica (RFC 0026): the shared render.device.v2 suite against
render.device.pica in Azahar, and its sensitivity runs.

Builds come from tools/n3ds/build_device_suite.sh: pica_device.3dsx (the
adapter as shipped) and pica_device_N.3dsx (N = 1-8, one seeded defect each,
test_device_pica.cpp). Each runs headless in its own Azahar namespace
(azahar_ns.py; default "pica-device"), writes sdmc:/pica_device.txt and exits.

  run_device_suite.py                 the clean build, then every sensitivity build
  run_device_suite.py --only clean    one of: clean, 1-8
  run_device_suite.py --renderer opengl
                                      Azahar's OpenGL renderer instead of its
                                      software one, on a private headless
                                      mutter (Wayland; never the user's
                                      desktop): it runs the checks the
                                      software renderer cannot (texture LOD)

Verdicts: the clean build must report at least MIN_CHECKS checks and no
failure; each sensitivity build must fail, on the clause its defect breaks.
Exit status 0 when every verdict holds.
"""

import argparse
import contextlib
import os
import signal
import subprocess
import re
import shutil
import sys
import time
from pathlib import Path

os.environ.setdefault("N3DS_NS", "pica-device")
sys.path.insert(0, str(Path(__file__).resolve().parent))

import azahar_ns  # noqa: E402

ROOT = Path(__file__).resolve().parents[2]
BUILD = ROOT / "build-3ds-device"
AZAHAR_BINARY = ROOT / "dependencies/3ds/src/azahar/build/bin/Release/azahar"
MIN_CHECKS = 376
# Checks Azahar's software renderer (the headless one) cannot run: it samples
# texture level 0 only. A failure of one is reported unverified, not passed.
NEEDS_HARDWARE = "[needs texture LOD]"
# The clause each seeded defect must fail (test_device_pica.cpp's PICA_SENSITIVITY).
SENSITIVITY = {
    "1": ("depth not negated", "D13 clip z 0.25 stores depth 0.25"),
    "2": ("draw constants dropped", "D16 each draw shows its own constants"),
    "3": ("colour write masks ignored", "D17 a red-and-alpha mask"),
    "4": ("blends drawn opaque", "D21 the independent color equation"),
    "5": ("textures stored upside down", "sampled texels sampled at their centers"),
    "6": ("winding reversed", "raster counter-clockwise front, back culled"),
    "7": ("CPU writes race the GPU", "raster a draw keeps the vertices written before it"),
    "8": ("ETC1 bytes unswapped", "D40 an ETC1 block decodes as its specification gives"),
}


@contextlib.contextmanager
def compositor():
    """A private headless mutter for the OpenGL renderer; yields its Wayland
    display name. It is kiln's private display session (RFC 0027): its
    D-Bus session never starts the document portal, whose shared mount a
    private session would otherwise take down. The compositor stands for the
    suite, so it runs a placeholder child until it is stopped."""
    sys.path.insert(0, str(ROOT / "tools/kiln"))
    import sepipe_loader
    home = azahar_ns.HOME / "compositor"
    home.mkdir(parents=True, exist_ok=True)
    session = sepipe_loader.Display("private", home / "display", (800, 600, 60))
    prefix = session.prefix()
    display = prefix[prefix.index("--wayland-display") + 1]
    socket = Path(os.environ.get("XDG_RUNTIME_DIR", "/run/user/%d" % os.getuid())) / display
    command, environment = session.wrap(["sleep", "infinity"], os.environ)
    process = subprocess.Popen(command, env=environment, stdout=open(home / "mutter.log", "w"),
                               stderr=subprocess.STDOUT, start_new_session=True)
    try:
        for _ in range(100):
            if socket.exists():
                break
            time.sleep(0.1)
        else:
            raise SystemExit("mutter did not open %s (see %s)" % (socket, home / "mutter.log"))
        yield display
    finally:
        os.killpg(process.pid, signal.SIGTERM)
        process.wait(timeout=10)
        session.close()


def run(variant, timeout, display=None):
    """(checks, failures, failing lines, raw output) of one build in Azahar."""
    name = "pica_device.3dsx" if variant == "clean" else "pica_device_%s.3dsx" % variant
    app = BUILD / name
    if not app.exists():
        raise SystemExit("%s is missing: run tools/n3ds/build_device_suite.sh %s" %
                         (app, "" if variant == "clean" else variant))
    azahar_ns.prepare(headless=display is None)
    if display:
        # Azahar's OpenGL renderer (graphics_api 1) on the private display.
        config = azahar_ns.USER / "config/qt-config.ini"
        config.write_text(re.sub(r"(?m)^graphics_api=.*$", "graphics_api=1", config.read_text()))
    home_app = azahar_ns.HOME / name
    shutil.copyfile(app, home_app)
    output = azahar_ns.SDMC / "pica_device.txt"
    done = azahar_ns.SDMC / "pica_device.done"
    for path in (output, done):
        path.unlink(missing_ok=True)
    log = open(azahar_ns.HOME / "device-suite.log", "w")
    if display:
        command = ["flatpak", "run", "--filesystem=%s" % ROOT, "--command=%s" % AZAHAR_BINARY,
                   "--socket=wayland", "--nosocket=x11", "--env=QT_QPA_PLATFORM=wayland",
                   azahar_ns.APP, str(home_app)]
        environment = dict(os.environ, WAYLAND_DISPLAY=display)
        environment.pop("DISPLAY", None)
        azahar_ns.stop()
        subprocess.Popen(command, cwd=str(azahar_ns.HOME), env=environment, stdout=log, stderr=log,
                         start_new_session=True)
    else:
        command = ["flatpak", "run", "--filesystem=%s" % ROOT, "--command=%s" % AZAHAR_BINARY,
                   "--env=QT_QPA_PLATFORM=offscreen", azahar_ns.APP, str(home_app)]
        azahar_ns.popen(command, headless=True, stdout=log, stderr=log)
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline and not done.exists():
        time.sleep(0.5)
    finished = done.exists()
    azahar_ns.stop()
    text = output.read_text(errors="replace") if output.exists() else ""
    match = re.search(r"^CONFORMANCE (\d+) (\d+)$", text, re.M)
    if not finished or not match:
        return None, None, [], text
    failing = [line for line in text.splitlines() if line.startswith("FAIL ")]
    return int(match.group(1)), int(match.group(2)), failing, text


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--only", choices=["clean"] + sorted(SENSITIVITY))
    parser.add_argument("--timeout", type=float, default=180.0)
    parser.add_argument("--renderer", choices=["software", "opengl"], default="software")
    args = parser.parse_args()
    if args.renderer == "opengl":
        with compositor() as display:
            return verdicts(args, display)
    return verdicts(args, None)


def verdicts(args, display):
    variants = [args.only] if args.only else ["clean"] + sorted(SENSITIVITY)
    print(azahar_ns.describe())
    azahar_ns.stop()
    ok = True
    for variant in variants:
        checks, failures, failing, text = run(variant, args.timeout, display)
        if checks is None:
            ok = False
            print("[FAIL] %-8s no result within %.0f s" % (variant, args.timeout))
            print("\n".join(text.splitlines()[-10:]))
            continue
        if variant == "clean":
            # Only the software renderer leaves texture LOD unverified.
            hardware = display is not None
            unverified = [] if hardware else [line for line in failing if NEEDS_HARDWARE in line]
            real = [line for line in failing if line not in unverified]
            good = not real and checks >= MIN_CHECKS
            print("[%s] clean    %d checks, %d failures, %d unverified (%s renderer)" % (
                "ok  " if good else "FAIL", checks, len(real), len(unverified),
                "OpenGL" if hardware else "software"))
            for line in failing:
                print("         %s%s" % ("UNVERIFIED " if NEEDS_HARDWARE in line else "",
                                         re.sub(r"^FAIL \S+: ", "", line)))
        else:
            what, clause = SENSITIVITY[variant]
            caught = any(clause in line for line in failing)
            good = failures > 0 and caught
            print("[%s] seed %s   %s: %d failures, %s" % (
                "ok  " if good else "FAIL", variant, what, failures or 0,
                "caught by '%s'" % clause if caught else "NOT caught by '%s'" % clause))
        ok &= good
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
