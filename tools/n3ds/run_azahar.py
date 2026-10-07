#!/usr/bin/env python3
"""Package the 3DS build and run it in Azahar (on the user's desktop).

  run_azahar.py [--content DIR] [--map NAME] [--args "..."] [--wait SECONDS]

1. 3dsxtool turns build-3ds/launcher_main/hl2_launcher (ELF) into
   build-3ds/Portal2.3dsx with an SMDH title.
2. The game directory on Azahar's emulated SD card, sdmc:/source-engine, gets
   the content tree (tools/n3ds/stage_3ds_content.py's output: portal2/,
   platform/, ...) linked in, and args.txt with the launch arguments.
3. Azahar (flatpak org.azahar_emu.Azahar) starts the .3dsx, and the run is
   followed until it settles, at most --wait seconds: it returns as soon as
   the app writes crash.txt (its exception handler), Azahar's log reports a
   fault, or the app's logs (early.txt, console.log) stop growing for
   --stall seconds (a hang: the last lines say where). The window stays open
   for the user.
"""

import argparse
import os
import shutil
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
DKP = ROOT / "dependencies/3ds/devkitpro"
sys.path.insert(0, str(Path(__file__).resolve().parent))
import azahar_ns  # noqa: E402

SDMC = azahar_ns.SDMC
GAME = azahar_ns.GAME


def package(build):
    elf = build / "launcher_main/hl2_launcher"
    smdh = build / "Portal2.smdh"
    out = build / "Portal2.3dsx"
    subprocess.run([str(DKP / "tools/bin/smdhtool"), "--create", "Portal 2",
                    "Source engine, PICA200 fullbright", "Source Engine",
                    str(DKP / "libctru/default_icon.png"), str(smdh)], check=True)
    subprocess.run([str(DKP / "tools/bin/3dsxtool"), str(elf), str(out), "--smdh=" + str(smdh)], check=True)
    print("packaged %s (%.1f MB)" % (out, out.stat().st_size / 1e6))
    return out


def stage(content, args):
    GAME.mkdir(parents=True, exist_ok=True)
    if content:
        # Incremental: only changed files are copied (the content tree is
        # thousands of files; the engine writes cfg/ and logs beside them).
        subprocess.run(["rsync", "-a", "--delete", str(Path(content)) + "/", str(GAME) + "/"], check=True)
    (GAME / "args.txt").write_text(args + "\n")
    for name in ("console.log", "early.txt", "trace.txt", "crash.txt"):
        if (GAME / name).exists():
            (GAME / name).unlink()


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--build", type=Path, default=ROOT / "build-3ds")
    parser.add_argument("--content", type=Path, help="content tree to copy into sdmc:/source-engine")
    parser.add_argument("--map", default="n3ds_chamber")
    parser.add_argument("--args", default="", help="extra launch arguments")
    parser.add_argument("--wait", type=float, default=120.0, help="longest run to follow")
    parser.add_argument("--stall", type=float, default=8.0, help="seconds without log growth that end the run")
    parser.add_argument("--no-launch", action="store_true")
    options = parser.parse_args()

    app = package(options.build.resolve())
    stage(options.content, ("+map %s " % options.map if options.map else "") + options.args)
    if options.no_launch:
        return 0
    azahar_ns.popen(["flatpak", "run", "--filesystem=%s" % ROOT, "org.azahar_emu.Azahar", str(app)],
                    stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    print("started Azahar with %s" % app)
    return follow(options.wait, options.stall)


EMULATOR_LOG = azahar_ns.EMULATOR_LOG
FAULTS = ("Unhandled", "exception", "Data Abort", "Prefetch Abort", "Undefined instruction", "svcBreak",
          "Fatal", "panic")


def tail(path, count):
    if not path.exists():
        return []
    return path.read_text(errors="replace").splitlines()[-count:]


def follow(wait, stall):
    """Follows the run; returns 0 while it is still progressing at `wait`,
    1 on a crash, an emulator fault or a stall."""
    logs = [GAME / "early.txt", GAME / "trace.txt", GAME / "console.log"]
    start = time.time()
    emulator_start = EMULATOR_LOG.stat().st_size if EMULATOR_LOG.exists() else 0
    last_size, last_change = -1, time.time()
    while time.time() - start < wait:
        time.sleep(0.5)
        crash = GAME / "crash.txt"
        if crash.exists() and crash.stat().st_size:
            print("CRASH (crash.txt):\n" + crash.read_text(errors="replace")[:4000])
            print("\n".join(tail(GAME / "console.log", 15)))
            return 1
        if EMULATOR_LOG.exists() and EMULATOR_LOG.stat().st_size > emulator_start:
            with EMULATOR_LOG.open(errors="replace") as stream:
                stream.seek(emulator_start)
                fresh = stream.read()
            emulator_start += len(fresh.encode())
            faults = [line for line in fresh.splitlines()
                      if any(f in line for f in FAULTS) and "ConfigureNew3DSCPU" not in line]
            if faults:
                print("EMULATOR FAULT:\n" + "\n".join(faults[-10:]))
                for log in logs:
                    print("\n".join(tail(log, 15)))
                return 1
        size = sum(log.stat().st_size for log in logs if log.exists())
        if size != last_size:
            last_size, last_change = size, time.time()
        elif time.time() - last_change > stall:
            print("STALL: no log output for %.0f s after %.0f s" % (stall, time.time() - start))
            for log in logs:
                print("-- %s" % log.name)
                print("\n".join(tail(log, 25)))
            return 1
    print("still running after %.0f s" % wait)
    print("\n".join(tail(GAME / "console.log", 25)))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
