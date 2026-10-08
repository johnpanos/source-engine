#!/usr/bin/env python3
"""pica_portals (render/lab/pica_portals): runs the 3DS portal lab in Azahar.

Build first with tools/n3ds/build_portal_lab.sh. The program runs headless in
its own Azahar namespace (azahar_ns.py; default "portal-lab"), writes
sdmc:/portal_lab/ and exits. This copies that directory to --out (default
build-3ds-portals/results/<renderer>), converts the PPM gallery to PNG, and
prints the checks and the summary table.

  run_portal_lab.py                     Azahar's software renderer, headless
  run_portal_lab.py --renderer opengl   Azahar's OpenGL renderer on a private
                                        headless mutter (never the user's
                                        desktop)

Exit status 0 when the program reports its checks with no failure.
"""

import argparse
import os
import re
import shutil
import struct
import subprocess
import sys
import time
import zlib
from pathlib import Path

os.environ.setdefault("N3DS_NS", "portal-lab")
sys.path.insert(0, str(Path(__file__).resolve().parent))

import azahar_ns  # noqa: E402
import run_device_suite  # noqa: E402  (its private compositor)

ROOT = Path(__file__).resolve().parents[2]
BUILD = ROOT / "build-3ds-portals"
AZAHAR_BINARY = ROOT / "dependencies/3ds/src/azahar/build/bin/Release/azahar"


def png(ppm, out):
    """A PPM (P6) as an 8-bit RGB PNG, standard library only."""
    data = ppm.read_bytes()
    match = re.match(rb"P6\s+(\d+)\s+(\d+)\s+255\s", data)
    width, height = int(match.group(1)), int(match.group(2))
    pixels = data[match.end():]
    rows = b"".join(b"\0" + pixels[y * width * 3:(y + 1) * width * 3] for y in range(height))

    def chunk(kind, body):
        return (struct.pack(">I", len(body)) + kind + body +
                struct.pack(">I", zlib.crc32(kind + body) & 0xFFFFFFFF))
    out.write_bytes(b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0)) +
                    chunk(b"IDAT", zlib.compress(rows, 6)) + chunk(b"IEND", b""))


def run(timeout, display):
    app = BUILD / "portal_lab.3dsx"
    if not app.exists():
        raise SystemExit("%s is missing: run tools/n3ds/build_portal_lab.sh" % app)
    azahar_ns.prepare(headless=display is None)
    if display:
        config = azahar_ns.USER / "config/qt-config.ini"
        config.write_text(re.sub(r"(?m)^graphics_api=.*$", "graphics_api=1", config.read_text()))
    home_app = azahar_ns.HOME / app.name
    shutil.copyfile(app, home_app)
    results = azahar_ns.SDMC / "portal_lab"
    shutil.rmtree(results, ignore_errors=True)
    done = results / "portal_lab.done"
    log = open(azahar_ns.HOME / "portal-lab.log", "w")
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
    started = time.monotonic()
    deadline = started + timeout
    report = results / "report.txt"
    last = ""
    while time.monotonic() < deadline and not done.exists():
        time.sleep(2)
        if report.exists():
            techniques = re.findall(r"^INFO technique (\S+)", report.read_text(errors="replace"), re.M)
            if techniques and techniques[-1] != last:
                last = techniques[-1]
                print("  %4.0f s: %s" % (time.monotonic() - started, last), flush=True)
    finished = done.exists()
    azahar_ns.stop()
    return finished, results


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--timeout", type=float, default=3600.0)
    parser.add_argument("--renderer", choices=["software", "opengl"], default="software")
    parser.add_argument("--out", type=Path)
    args = parser.parse_args()
    print(azahar_ns.describe())
    azahar_ns.stop()
    if args.renderer == "opengl":
        with run_device_suite.compositor() as display:
            finished, results = run(args.timeout, display)
    else:
        finished, results = run(args.timeout, None)
    out = args.out or BUILD / "results" / args.renderer
    shutil.rmtree(out, ignore_errors=True)
    if results.exists():
        shutil.copytree(results, out)
        for ppm in sorted(out.glob("*.ppm")):
            png(ppm, ppm.with_suffix(".png"))
            ppm.unlink()
    text = (out / "report.txt").read_text(errors="replace") if (out / "report.txt").exists() else ""
    for line in text.splitlines():
        if line.startswith(("FAIL", "SUMMARY", "INFO image", "CONFORMANCE")):
            print(line)
    match = re.search(r"^CONFORMANCE (\d+) (\d+)$", text, re.M)
    if not finished or not match:
        print("[FAIL] no result (finished=%s); last lines:" % finished)
        print("\n".join(text.splitlines()[-15:]))
        return 1
    print("results in %s" % out)
    return 0 if match.group(2) == "0" else 1


if __name__ == "__main__":
    sys.exit(main())
