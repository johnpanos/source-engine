#!/usr/bin/env python3
"""RFC 0011 G8.2: the indirect-light switching soak on the installed product.

    python3 tools/quality/gi_soak.py --minutes 30 --validation \\
        --out quality-results/rfc0011-g8/soak-linux

Boots the installed native-Vulkan product (portal_boot.py's private copy of
the runtime with the build's binaries) headless, with the published GI maps
(run/maps/<map>: every producer's data) beside the stock Portal chambers
(baked only), and plays a looping console script for --minutes:

  on each map, in turn: every ordered pair of producers (baked, radiosity,
  sdf, rayquery), one switch every STEP frames, the door toggled between
  switches where the map has one; then the next map (a map change), and
  round again.

A switch a map does not offer is refused and the previous producer kept;
that refusal is part of what the soak exercises. On a stock chamber (no
probe volume) there is no switcher: the setting is only stored, and applies
from the next map that has one. An observer samples the
game process's resident memory every few seconds.

Gate (soak.json):
  - the product runs the whole time and quits cleanly (no crash, no hang);
  - every switch request is either taken ("switching to") or refused with
    the structured reason ("unavailable (...)"), and at least one of each
    producer is taken;
  - no leak: across the completed loops after the first, the memory floor
    of a loop (the level between map loads; a load's transient peak is not
    a leak) grows by at most LEAK_MB over the second loop's;
  - every producer some map offered is taken, and none it did not offer
    (Android declares sdf and rayquery unsupported: refused, never taken);
  - with --validation (the Khronos layer, from VK_ADD_LAYER_PATH or
    --layer-path), no validation error or warning.
"""

import argparse
import json
import os
import re
import shutil
import subprocess
import sys
import time
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
sys.path.insert(0, str(HERE))
import pbrt_map_toolchain  # noqa: E402

SCHEMA = "gi-soak/v1"
PRODUCERS = ("baked", "radiosity", "sdf", "rayquery")
GI_MAPS = ("gi_door", "gi_room_states", "gi_portal_view")
LEGACY_MAPS = ("testchmb_a_00", "testchmb_a_01")
STEP = 150          # frames between switches
LOAD_WAIT = 300     # frames after a map change before the first switch
LEAK_MB = 64.0
SAMPLE_SECONDS = 5.0
FPS = 60


def ordered_pairs():
    """A switch sequence visiting every ordered producer pair (a -> b)."""
    sequence = []
    for a in PRODUCERS:
        for b in PRODUCERS:
            if a != b:
                sequence += [a, b]
    return sequence


def merged_content(out, maps):
    """One content root with every published map's files (copies: the boot
    refuses links)."""
    root = out / "content"
    shutil.rmtree(root, ignore_errors=True)
    for name in maps:
        source = ROOT / "run" / "maps" / name
        if not (source / "maps" / (name + ".bsp")).is_file():
            raise FileNotFoundError("map %s is not published (run/maps/%s)" % (name, name))
        for path in source.rglob("*"):
            relative = path.relative_to(source)
            if path.is_file() and relative.parts[0] in ("maps", "materials", "models"):
                target = root / relative
                target.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(path, target)
    return root


STOP_CFG = "gi_soak_stop"  # cfg/<this>.cfg: empty until the harness writes `quit`


def script(maps, doors, step=STEP):
    """Console lines: one short alias per step (a console line is cut at
    about 512 characters), chained into a loop. Every step runs the stop cfg,
    which the harness fills with `quit` when the time is up: the product
    quits cleanly within one step."""
    lines, steps = [], []
    for name in maps:
        steps.append("echo gi_soak map %s; map %s; wait %d" % (name, name, LOAD_WAIT))
        for producer in ordered_pairs():
            command = "r_indirect_producer %s; wait %d" % (producer, step)
            if name in doors:
                command += "; ent_fire Door Toggle"
            steps.append(command)
    steps.append("echo gi_soak loop")
    for index, step in enumerate(steps):
        following = "gs_%d" % ((index + 1) % len(steps))
        lines.append('alias gs_%d "%s; exec %s; %s"' % (index, step, STOP_CFG, following))
    return lines, "r_drawvgui 0; r_indirect_report 0; gs_0"


def sample_memory(stage, samples, started, stop):
    """Resident memory of the product process (the one running from the
    stage), every SAMPLE_SECONDS, against the process's age."""
    while not stop():
        pid = None
        for entry in Path("/proc").iterdir():
            if not entry.name.isdigit():
                continue
            try:
                command = (entry / "cmdline").read_bytes().split(b"\0")[0].decode()
            except OSError:
                continue
            if command.startswith(str(stage)) and command.endswith("hl2_launcher"):
                pid = entry
                break
        if pid:
            try:
                status = (pid / "status").read_text()
                rss = int(re.search(r"VmRSS:\s+(\d+) kB", status).group(1)) / 1024.0
                # The process's age, the clock its engine log is stamped with.
                ticks = os.sysconf("SC_CLK_TCK")
                begun = int((pid / "stat").read_text().rsplit(")", 1)[1].split()[19]) / ticks
                uptime = float(Path("/proc/uptime").read_text().split()[0])
                samples.append((uptime - begun, rss))
            except (OSError, AttributeError, ValueError, IndexError):
                pass
        time.sleep(SAMPLE_SECONDS)


def plain(log):
    """The engine log as (time, text) lines: `echo` interleaves a timestamp
    between its words; each line keeps its first."""
    lines = []
    for line in log.splitlines():
        stamps = re.findall(r"\[(\d+\.\d+)\] ", line)
        text = re.sub(r"\[\d+\.\d+\] ", "", line)
        lines.append((float(stamps[0]) if stamps else None, text))
    return lines


def analyse(log, stdout, samples, returncode, timed_out):
    lines = plain(log)
    log = "\n".join(text for _, text in lines)
    engine_times = [t for t, text in lines if t is not None and "gi_soak loop" in text]
    switched = re.findall(r"indirect light: switching to (\w+)", log)
    refused = re.findall(r'r_indirect_producer "(\w+)" unavailable \(([\w-]+)\)', log)
    maps = re.findall(r"gi_soak map (\w+)", log)
    validation = [line for line in stdout.splitlines()
                  if re.search(r"Validation (Error|Warning)|VUID-", line)]
    # Per-loop memory floor (loop boundaries from the engine's own clock).
    # A map load's transient peak is not a leak; what a leak raises is the
    # level the process returns to between loads, the loop's minimum.
    bounds = [0.0] + engine_times
    floors, peaks = [], []
    for start, end in zip(bounds, bounds[1:]):
        inside = [rss for t, rss in samples if start <= t < end]
        if inside:
            floors.append(min(inside))
            peaks.append(max(inside))
    growth = (max(floors[2:]) - floors[1]) if len(floors) >= 3 else None
    offered = set()
    for names in re.findall(r"\(offered: ([\w ]+)\)", log):
        offered.update(names.split())
    checks = {
        "ran_to_the_end": returncode == 0 and not timed_out,
        "switches_taken_or_refused": bool(switched) and all(
            reason for _, reason in refused),
        # Every producer some map offered on this device (a profile may
        # declare others unsupported: they must be refused, not taken).
        "every_offered_producer_taken": bool(offered) and all(p in switched for p in offered),
        "unoffered_never_taken": all(p in offered for p in set(switched)),
        "no_leak": growth is not None and growth <= LEAK_MB,
        "no_validation_messages": not validation,
    }
    return {"loops": len(engine_times), "map_changes": len(maps),
            "switches": len(switched), "refusals": len(refused),
            "refusal_reasons": sorted({reason for _, reason in refused}),
            "taken_by_producer": {p: switched.count(p) for p in PRODUCERS},
            "offered_producers": sorted(offered),
            "loop_floor_rss_mb": floors, "loop_peak_rss_mb": peaks, "rss_growth_mb": growth,
            "rss_first_last_mb": [samples[0][1], samples[-1][1]] if samples else None,
            "validation_messages": len(validation), "validation_first": validation[:5],
            "checks": checks,
            "status": "pass" if all(checks.values()) else "fail"}


APP = "org.sourceengine.portal"
DEVICE_FILES = "/sdcard/Android/data/%s/files" % APP
SOAK_CUSTOM = "rfc0011-soak"


def adb(*command, check=True):
    return subprocess.run(["adb", *command], capture_output=True, text=True, check=check).stdout


def android(args, out):
    """The same soak on the installed Android product (the app on a
    connected device): the maps pushed as a custom folder, the script as
    autoexec.cfg, memory from dumpsys (total PSS), the engine log pulled
    afterwards. The device is restored: the cfgs and pushed maps removed, a
    custom folder the soak replaced put back, and --restore-apk reinstalled."""
    game = DEVICE_FILES + "/portal"
    maps = list(args.maps) + list(args.legacy_maps)
    aliases, start = script(maps, doors={"gi_door"}, step=args.step)
    moved = []
    try:
        if args.apk:
            adb("install", "-r", str(args.apk))
        # A custom folder with one of our maps would shadow the new copy.
        listing = adb("shell", "ls %s/custom" % game).split()
        for name in args.maps:
            folder = "pbrt-" + name
            if folder in listing:
                adb("shell", "mv %s/custom/%s %s/custom/%s.soak-bak" % (game, folder, game, folder))
                moved.append(folder)
        for name in args.maps:
            source = ROOT / "run" / "maps" / name
            for part in ("maps", "materials", "models"):
                if (source / part).is_dir():
                    adb("shell", "mkdir -p %s/custom/%s/%s" % (game, SOAK_CUSTOM, part))
                    adb("push", "--sync", str(source / part) + "/.",
                        "%s/custom/%s/%s/" % (game, SOAK_CUSTOM, part))
        cfg = out / "autoexec.cfg"
        cfg.write_text("\n".join(aliases + [start]) + "\n")
        adb("push", str(cfg), game + "/cfg/autoexec.cfg")
        stop = out / (STOP_CFG + ".cfg")
        stop.write_text("")
        adb("push", str(stop), "%s/cfg/%s.cfg" % (game, STOP_CFG))
        adb("shell", "rm -f %s/engine.log" % DEVICE_FILES, check=False)
        adb("logcat", "-c", check=False)
        adb("shell", "am start -n %s/org.libsdl.app.SDLActivity" % APP)
        samples, started, crashed = [], time.monotonic(), False
        while time.monotonic() - started < args.minutes * 60:
            meminfo = adb("shell", "dumpsys meminfo %s" % APP, check=False)
            match = re.search(r"TOTAL PSS:\s+(\d+)", meminfo) or \
                re.search(r"TOTAL\s+(\d+)", meminfo)
            if match:
                samples.append((time.monotonic() - started, int(match.group(1)) / 1024.0))
            elif samples:
                crashed = True  # the process is gone before the end
                break
            time.sleep(SAMPLE_SECONDS * 2)
        stop.write_text("quit\n")
        adb("push", str(stop), "%s/cfg/%s.cfg" % (game, STOP_CFG))
        deadline = time.monotonic() + 120
        while time.monotonic() < deadline and adb("shell", "pidof %s" % APP, check=False).strip():
            time.sleep(2)
        quit_cleanly = not adb("shell", "pidof %s" % APP, check=False).strip()
        if not quit_cleanly:
            adb("shell", "am force-stop %s" % APP, check=False)
        adb("pull", DEVICE_FILES + "/engine.log", str(out / "engine.log"), check=False)
        (out / "logcat.txt").write_text(adb("logcat", "-d", check=False))
    finally:
        adb("shell", "rm -f %s/cfg/autoexec.cfg %s/cfg/%s.cfg" % (game, game, STOP_CFG),
            check=False)
        adb("shell", "rm -rf %s/custom/%s" % (game, SOAK_CUSTOM), check=False)
        for folder in moved:
            adb("shell", "mv %s/custom/%s.soak-bak %s/custom/%s" % (game, folder, game, folder),
                check=False)
        if args.restore_apk:
            adb("install", "-r", str(args.restore_apk), check=False)
    logcat = (out / "logcat.txt").read_text(errors="replace") \
        if (out / "logcat.txt").is_file() else ""
    log = log_from_logcat(logcat)
    crashed = crashed or bool(re.search(r"Fatal signal|SIGSEGV|SIGABRT", logcat))
    return log, logcat, samples, 0 if (quit_cleanly and not crashed) else 1


def log_from_logcat(logcat):
    """The soak process's engine output from logcat (tag SRCENG), as engine
    log lines stamped with seconds since its first line. The on-device
    engine.log belongs to the app's last run, which a relaunch replaces;
    logcat keeps the soak's. `echo` puts each word on its own line: a
    `gi_soak` marker is joined with the words that follow it."""
    entries = []
    for line in logcat.splitlines():
        match = re.match(r"\d+-\d+ (\d+):(\d+):(\d+\.\d+)\s+(\d+)\s+\d+ \w SRCENG\s*: ?(.*)",
                         line)
        if match:
            hours, minutes, seconds, pid, text = match.groups()
            entries.append((int(hours) * 3600 + int(minutes) * 60 + float(seconds), pid, text))
    if not entries:
        return ""
    pid = entries[0][1]
    entries = [(t, text) for t, owner, text in entries if owner == pid]
    start = entries[0][0]
    out, i = [], 0
    while i < len(entries):
        t, text = entries[i]
        if text.strip() == "gi_soak" and i + 1 < len(entries):
            words = [entries[i + 1][1].strip()]
            i += 1
            if words[0] == "map" and i + 1 < len(entries):
                words.append(entries[i + 1][1].strip())
                i += 1
            text = "gi_soak " + " ".join(words)
        out.append("[%.4f] %s" % (t - start, text.strip()))
        i += 1
    return "\n".join(out)


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--minutes", type=float, default=30.0)
    parser.add_argument("--maps", nargs="*", default=list(GI_MAPS))
    parser.add_argument("--legacy-maps", nargs="*", default=list(LEGACY_MAPS))
    parser.add_argument("--validation", action="store_true",
                        help="run under the Khronos validation layer; any message fails")
    parser.add_argument("--layer-path", help="VK_ADD_LAYER_PATH for the validation layer")
    parser.add_argument("--step", type=int, default=STEP, help="frames between switches")
    parser.add_argument("--build", default="build")
    parser.add_argument("--runtime")
    parser.add_argument("--out", required=True)
    parser.add_argument("--android", action="store_true",
                        help="soak the app on the connected Android device instead")
    parser.add_argument("--apk", type=Path, help="with --android: install this APK first")
    parser.add_argument("--restore-apk", type=Path,
                        help="with --android: reinstall this APK afterwards")
    args = parser.parse_args()
    out = Path(args.out)
    out.mkdir(parents=True, exist_ok=True)
    if args.android:
        log, logcat, samples, code = android(args, out)
        # No validation layer ships in the APK: validation is the desktop's.
        record = analyse(log, "", samples, code, False)
        record["checks"]["no_validation_messages"] = True
        record["validation_layer"] = False
        record["status"] = "pass" if all(record["checks"].values()) else "fail"
        record.update({"schema": SCHEMA, "platform": "android", "minutes": args.minutes,
                       "maps": list(args.maps) + list(args.legacy_maps),
                       "producers": list(PRODUCERS), "step_frames": args.step,
                       "leak_limit_mb": LEAK_MB, "pss_samples": samples})
        (out / "soak.json").write_text(json.dumps(record, indent=2, sort_keys=True) + "\n")
        print(json.dumps({k: record[k] for k in ("status", "loops", "map_changes", "switches",
                                                 "refusals", "taken_by_producer",
                                                 "loop_floor_rss_mb", "rss_growth_mb",
                                                 "checks")}, indent=1))
        return 0 if record["status"] == "pass" else 1
    profile, _ = pbrt_map_toolchain.load_profiles()
    toolchain = pbrt_map_toolchain.load(ROOT / profile["layout"]["toolchain_file"])
    runtime = Path(args.runtime or toolchain["runtime"])
    content = merged_content(out, args.maps)
    maps = list(args.maps) + list(args.legacy_maps)
    aliases, start = script(maps, doors={"gi_door"}, step=args.step)
    # The countdown is only a backstop (frames run faster than FPS headless):
    # the harness ends the soak on the wall clock through the stop cfg.
    frames = int(args.minutes * 60 * FPS * 20)
    boot = out / "boot"
    shutil.rmtree(boot, ignore_errors=True)
    environment = os.environ.copy()
    if args.validation:
        if args.layer_path:
            environment["VK_ADD_LAYER_PATH"] = args.layer_path
        environment["VK_INSTANCE_LAYERS"] = "VK_LAYER_KHRONOS_validation"
    command = [sys.executable, str(HERE / "portal_boot.py"), "--runtime", str(runtime),
               "--build", args.build, "--content-root", str(content), "--renderer",
               "native-vulkan", "--headless", "--map", args.maps[0], "--capture-wait",
               str(frames), "--timeout", str(args.minutes * 60 * 3 + 600), "--out", str(boot)]
    for line in aliases + [start]:
        command += ["--console-command", line]
    samples, done = [], []
    started = time.monotonic()
    import threading
    stage_hint = boot
    observer = threading.Thread(target=sample_memory,
                                args=(stage_hint.resolve(), samples, started, lambda: done))
    observer.start()

    def stop_when_due():
        # The process's age (the first memory sample's clock) reaches the soak.
        while not done:
            if samples and samples[-1][0] >= args.minutes * 60:
                for cfg in boot.glob("runtime/*/cfg"):
                    (cfg / (STOP_CFG + ".cfg")).write_text("quit\n")
                return
            time.sleep(1.0)
    stopper = threading.Thread(target=stop_when_due)
    stopper.start()
    result = subprocess.run(command, cwd=ROOT, env=environment, capture_output=True, text=True)
    done.append(True)
    observer.join()
    stopper.join()
    evidence = json.loads((boot / "evidence.json").read_text()) \
        if (boot / "evidence.json").is_file() else {}
    log_path = next(iter(sorted(boot.glob("runtime/engine.log"))), None)
    log = log_path.read_text(errors="replace") if log_path else ""
    stdout = (boot / "stdout.log").read_text(errors="replace") \
        if (boot / "stdout.log").is_file() else ""
    record = analyse(log, stdout, samples, evidence.get("returncode", result.returncode),
                     evidence.get("timed_out", False))
    record.update({"schema": SCHEMA, "minutes": args.minutes, "maps": maps,
                   "producers": list(PRODUCERS), "step_frames": args.step,
                   "validation_layer": args.validation, "leak_limit_mb": LEAK_MB,
                   "boot_status": evidence.get("status"), "boot_failures": evidence.get("failures"),
                   "rss_samples": samples})
    (out / "soak.json").write_text(json.dumps(record, indent=2, sort_keys=True) + "\n")
    print(json.dumps({k: record[k] for k in ("status", "loops", "map_changes", "switches",
                                             "refusals", "taken_by_producer",
                                             "loop_floor_rss_mb", "rss_growth_mb",
                                             "validation_messages", "checks")}, indent=1))
    return 0 if record["status"] == "pass" else 1


if __name__ == "__main__":
    sys.exit(main())
