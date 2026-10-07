#!/usr/bin/env python3
"""One entry point for the 3DS build, the Azahar harness and their reports.

  n3ds.py build                         ./build-3ds.sh (errors only)
  n3ds.py run [--map M] [probe args]    fresh boot, judged after the map loads
                                        (tools/n3ds/probe.py); prints the
                                        console after its marker, counters,
                                        a screenshot, and on a fatal error
                                        the error and the symbolized ledger
  n3ds.py debug [harness debug args]    gdb run (azahar_harness.py debug)
  n3ds.py ledger [console.log]          symbolize the LEDGER lines (large
                                        allocations by stack; printed by the
                                        engine on out-of-memory)
  n3ds.py census <heap.bin>             heap census (heap_census.py)
  n3ds.py log [-n N] [pattern]          last console lines without the noise
  n3ds.py speed [PERCENT]               emulation speed of the running emulator:
                                        100 (default) is normal, 0 unlimited;
                                        for this session only (your saved
                                        Azahar setting is untouched). A new
                                        run takes `run --speed PERCENT`.
  n3ds.py ns                            where this namespace's files live
  n3ds.py stop                          stop this namespace's emulator only

--map picks the staged content set build-3ds-content/<map> when one exists.

  n3ds.py --ns NAME <command> ...       run in namespace NAME (or set N3DS_NS):
                                        a private Azahar user directory, SD card,
                                        socket and GDB port under
                                        build-3ds/azahar/NAME, so sessions run
                                        at the same time; only that namespace's
                                        emulator is ever stopped (azahar_ns.py)
"""

import os
import re
import subprocess
import sys
from pathlib import Path

# --ns must reach azahar_ns before the harness modules import it; the
# environment carries it to probe.py and azahar_harness.py subprocesses.
if len(sys.argv) > 2 and sys.argv[1] == "--ns":
    os.environ["N3DS_NS"] = sys.argv[2]
    del sys.argv[1:3]

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import azahar_harness as harness  # noqa: E402

ROOT = harness.ROOT
CONSOLE = harness.GAME / "console.log"
NOISE = re.compile(r"Missing Vgui|MetaClass missing|not found\.|Can't find module|Unknown command|"
                   r"Can't use cheat|LoadTextureBitsFromFile|^pica: textures|Found font|"
                   r"Couldn't find custom font|Failed to load Res|AddFile")


def ledger(path=CONSOLE):
    lines = Path(path).read_text(errors="replace").splitlines()
    rows = [l.split() for l in lines if l.startswith("LEDGER ") and not l.startswith("LEDGER total")]
    header = [l for l in lines if l.startswith("LEDGER total") or l.startswith("OOM:")]
    if not rows:
        print("no LEDGER lines in %s" % path)
        return 1
    for line in header:
        print(line)
    addresses = [int(x, 16) for row in rows for x in row[3:]]
    names = harness.symbolize(addresses)
    for row in rows:
        size, count = int(row[1]), int(row[2])
        print("%8.2f MB %4d blocks" % (size / 1e6, count))
        for x in row[3:]:
            a = int(x, 16)
            if a:
                print("      %s" % names.get(a, x))
    return 0


def log(argv):
    count, pattern = 40, None
    if argv[:1] == ["-n"]:
        count, argv = int(argv[1]), argv[2:]
    if argv:
        pattern = re.compile(argv[0])
    lines = [l for l in CONSOLE.read_text(errors="replace").splitlines() if l.strip() and not NOISE.search(l)]
    if pattern:
        lines = [l for l in lines if pattern.search(l)]
    print("\n".join(lines[-count:]))
    return 0


def run(argv):
    args = list(argv)
    if "--map" in args:
        name = args[args.index("--map") + 1]
        content = ROOT / "build-3ds-content" / name
        if "--content" not in args and content.is_dir():
            args += ["--content", str(content.relative_to(ROOT))]
    result = subprocess.run([sys.executable, "-u", str(HERE / "probe.py")] + args, cwd=ROOT)
    if result.returncode and CONSOLE.exists():
        text = CONSOLE.read_text(errors="replace")
        errors = [l for l in text.splitlines() if "Sys_Error" in l or l.startswith("OOM:")]
        if errors:
            print("-- fatal")
            print("\n".join(errors[-3:]))
        if "LEDGER " in text:
            print("-- large allocations at the failure")
            ledger()
    return result.returncode


def main():
    if len(sys.argv) < 2 or sys.argv[1] in ("-h", "--help"):
        print(__doc__)
        return 0
    command, argv = sys.argv[1], sys.argv[2:]
    if command == "build":
        result = subprocess.run(["./build-3ds.sh"], cwd=ROOT, capture_output=True, text=True)
        log_path = ROOT / "build-3ds.build.log"
        errors = [l for l in log_path.read_text(errors="replace").splitlines() if "error" in l.lower()]
        print("\n".join(errors[:20]) if result.returncode else "build ok")
        return result.returncode
    if command == "ns":
        print(harness.azahar_ns.describe())
        return 0
    if command == "stop":
        harness.azahar_ns.stop()
        return 0
    if command == "run":
        return run(argv)
    if command == "debug":
        return subprocess.run([sys.executable, "-u", str(HERE / "azahar_harness.py"), "debug"] + argv,
                              cwd=ROOT).returncode
    if command == "ledger":
        return ledger(argv[0] if argv else CONSOLE)
    if command == "census":
        return subprocess.run([sys.executable, str(HERE / "heap_census.py")] + argv, cwd=ROOT).returncode
    if command == "log":
        return log(argv)
    if command == "speed":
        percent = float(argv[0]) if argv else 100.0
        reply = harness.Session(connect_within=2.0).request("speed %g" % percent)
        print("speed %g%%: %s" % (percent, reply))
        return 0 if reply.get("ok") else 1
    print("unknown command %s\n%s" % (command, __doc__))
    return 2


if __name__ == "__main__":
    sys.exit(main())
