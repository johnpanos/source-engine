#!/usr/bin/env python3
"""Check that every F-Stop mechanic in fstop_mechanics does what its code says.

Each scenario boots the fstop product headless on the map
(tools/quality/fstop_mechanics_map.py) in a private staged runtime, drives it
from the console (setpos/setang, ent_fire, give, +attack in cfgs) and judges
what the game logs:

* entity outputs: the map wires each output it checks to its own logic_relay,
  probe.<entity>.<output>, and developer 2 logs each firing as
  "output: (class,name) -> (probe.<entity>.<output>,Trigger)";
* the player's position (getpos) before and after an effect;
* entity counts (report_entities) for what an entity spawns;
* the entity's own console messages.

Steps are scheduled in game time: the scenario's cfg is one ent_fire per step
to the map's point_servercommand / point_clientcommand with the step's delay
(console `wait` counts command-buffer passes, not time). A scenario passes only
if every assertion holds; a run passes only if every
scenario ran and passed (no scenario is skipped).

    python3 tools/quality/fstop_mechanics_check.py              all scenarios
    python3 tools/quality/fstop_mechanics_check.py dispenser    by name
    python3 tools/quality/fstop_mechanics_check.py --list

The runtime is private (run/runtime-fstop-check, seeded from run/runtime) and
the launcher runs with -multirun and an absolute -game path, so it never
touches a ./play_fstop session. Waits work only on the command line after
+map, so the command line waits once for the map to load and execs the cfg.
"""

import argparse
import json
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import stage_fstop_runtime  # noqa: E402

ROOT = HERE.parents[1]
MAP = "fstop_mechanics"
DEFAULT_CONTENT = Path.home() / "Downloads/portal2-steam2-research/852_0"


class Scenario:
    """A named boot: timed console commands and assertions on the log.

    steps are (whole game seconds after the scenario starts, command); ent_fire
    reads its delay with atoi, so sub-second input goes in one client command
    with client-side waits ("+attack; wait 6; -attack"). A command starting
    with "client:" runs on the player's client (player commands: setpos, give,
    +attack ...), any other on the server. The run quits `tail` seconds after
    the last step.
    """

    def __init__(self, name, summary, steps, checks, tail=2, timeout=240):
        self.name = name
        self.summary = summary
        self.steps = steps
        self.checks = checks
        self.tail = tail
        self.timeout = timeout

    def cfg(self):
        lines = []
        for at, command in sorted(self.steps, key=lambda step: step[0]) + [
                (max([at for at, _ in self.steps] + [0]) + self.tail, "quit")]:
            target = "check_server"
            if command.startswith("client:"):
                target, command = "check_client", command[len("client:"):]
            if '"' in command or at != int(at):
                raise ValueError("scenario %s: step (%r, %r) needs whole seconds and no "
                                 "double quotes" % (self.name, at, command))
            lines.append('ent_fire %s Command "%s" %d' % (target, command, at))
        return "".join(line + "\n" for line in lines)


# ---- log judges ---------------------------------------------------------------------------

def positions(log):
    """Every getpos result in order, as (x, y, z)."""
    return [tuple(float(v) for v in m) for m in
            re.findall(r"setpos (-?[\d.]+) (-?[\d.]+) (-?[\d.]+);", log)]


def entity_tables(log):
    """Every report_entities table in order, each as {classname: count}."""
    tables, start = [], log.find("Command(report_entities)")
    while start >= 0:
        table = log[start:log.find("Total ", start)]
        tables.append({c: int(n) for c, n in re.findall(r"Class: (\S+) \((\d+)\)", table)})
        start = log.find("Command(report_entities)", start + 1)
    return tables


def entity_counts(log):
    """The last report_entities table as {classname: count}."""
    tables = entity_tables(log)
    return tables[-1] if tables else {}


def count_change(classname, delta, first=0, last=-1):
    """classname's count changed by exactly delta between two report_entities tables."""
    def check(log):
        tables = entity_tables(log)
        if len(tables) < 2:
            return False, "need 2 report_entities tables, have %d" % len(tables)
        change = tables[last].get(classname, 0) - tables[first].get(classname, 0)
        return change == delta, "%s count changed by %+d (need %+d)" % (classname, change, delta)
    return check


def fired(entity, output, at_least=1):
    """entity fired output at least at_least times (its probe relay was triggered)."""
    def check(log):
        # Distinct firings: each line carries its game time, "(12.34) output: ...".
        n = len(set(re.findall(r"\(([\d.]+)\) output: \([^,]+,[^)]*\) -> \(%s,"
                               % re.escape(probe(entity, output)), log)))
        return n >= at_least, "%s.%s fired %d time(s) (need >= %d)" % (entity, output, n,
                                                                    at_least)
    return check


def probe(entity, output):
    """The logic_relay the map wires entity's output to (fstop_mechanics_map.probe)."""
    return "probe.%s.%s" % (entity, output)


def not_fired(entity, output):
    """entity never fired output (a negative control)."""
    def check(log):
        ok, detail = fired(entity, output)(log)
        return not ok, detail.replace("need >= 1", "need 0")
    return check


def peak(axis, at_least, why=""):
    """Over the getpos samples, the player's axis rose (or, with a negative
    at_least, fell) by at least |at_least| from the first sample."""
    i = "xyz".index(axis)

    def check(log):
        p = positions(log)
        if len(p) < 2:
            return False, "need 2 getpos results, have %d" % len(p)
        values = [q[i] - p[0][i] for q in p[1:]]
        best = max(values) if at_least >= 0 else min(values)
        ok = best >= at_least if at_least >= 0 else best <= at_least
        return ok, "player %s peak change %.1f over %d samples (need %s%.1f)%s" % (
            axis, best, len(p), ">=" if at_least >= 0 else "<=", at_least,
            "; " + why if why else "")
    return check


def displaced(at_least, why=""):
    """The player got at least at_least units (3D) from the first getpos sample."""
    def check(log):
        p = positions(log)
        if len(p) < 2:
            return False, "need 2 getpos results, have %d" % len(p)
        best = max(sum((a - b) ** 2 for a, b in zip(q, p[0])) ** 0.5 for q in p[1:])
        return best >= at_least, "player displaced %.1f from its start (need >= %.1f)%s" % (
            best, at_least, "; " + why if why else "")
    return check


def emitted(sound, at_least=1):
    """sound was emitted (needs sv_soundemitter_trace 1 in the scenario)."""
    return logged(r"EmitSound: +'%s" % re.escape(sound), at_least, "sound " + sound)


def jumped(axis, at_least, why=""):
    """Between two consecutive getpos samples the player's axis jumped by at
    least at_least: a teleport, faster than walking (about 200 units/s)."""
    i = "xyz".index(axis)

    def check(log):
        p = positions(log)
        if len(p) < 2:
            return False, "need 2 getpos results, have %d" % len(p)
        best = max(abs(b[i] - a[i]) for a, b in zip(p, p[1:]))
        return best >= at_least, "player %s jumped %.1f between samples (need >= %.1f)%s" % (
            axis, best, at_least, "; " + why if why else "")
    return check


def dumped(field, value):
    """An ent_dump printed `field: value` (ent_dump prints non-zero keyfields)."""
    return logged(r"^\s*%s: %s\b" % (re.escape(field), re.escape(value)), 1,
                  "ent_dump field")


def health_dropped():
    """The player's health (ent_dump !player) fell between the first and last dump."""
    def check(log):
        values = [int(v) for v in re.findall(r"^\s*health: (-?\d+)", log, re.M)]
        if len(values) < 2:
            return False, "need 2 health dumps, have %d" % len(values)
        return values[-1] < values[0], "player health %d -> %d" % (values[0], values[-1])
    return check


def samples(start, stop, command="client:getpos", every=1):
    """(t, command) every `every` seconds from start to stop inclusive."""
    return [(t, command) for t in range(start, stop + 1, every)]


def logged(pattern, at_least=1, why=""):
    def check(log):
        n = len(re.findall(pattern, log, re.M))
        return n >= at_least, "%r seen %d time(s) (need %d)%s" % (
            pattern, n, at_least, "; " + why if why else "")
    return check


def not_logged(pattern, why=""):
    def check(log):
        n = len(re.findall(pattern, log))
        return n == 0, "%r seen %d time(s) (need 0)%s" % (pattern, n, "; " + why if why else "")
    return check


def count_at_least(classname, n):
    def check(log):
        have = entity_counts(log).get(classname, 0)
        return have >= n, "report_entities: %d %s (need >= %d)" % (have, classname, n)
    return check


def moved(axis, at_least, first=0, last=-1, why=""):
    """Player moved along axis ('x', 'y' or 'z') by at_least between two getpos results."""
    i = "xyz".index(axis)

    def check(log):
        p = positions(log)
        if len(p) < 2:
            return False, "need 2 getpos results, have %d" % len(p)
        delta = p[last][i] - p[first][i]
        ok = delta >= at_least if at_least >= 0 else delta <= at_least
        return ok, "player %s moved %.1f (need %s%.1f)%s" % (
            axis, delta, ">=" if at_least >= 0 else "<=", at_least, "; " + why if why else "")
    return check


# ---- running --------------------------------------------------------------------------------

def stage(runtime, content, build, bsp):
    stage_fstop_runtime.stage(runtime, ROOT / "run/runtime", content, build)
    maps = runtime / "fstop/maps"
    maps.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(bsp, maps / (MAP + ".bsp"))


def run(scenario, runtime, out):
    game = runtime / "fstop"
    cfg = game / "cfg"
    cfg.mkdir(parents=True, exist_ok=True)
    name = "check_%s.cfg" % scenario.name
    (cfg / name).write_text(scenario.cfg())
    # The only command-line wait: until the map has loaded and the player exists.
    command_line = ["+wait", "300", "+exec", name]
    log_path = game / "console.log"
    for stale in (log_path, runtime / "engine.log"):
        if stale.exists():
            stale.unlink()
    env = dict(os.environ, SDL_VIDEODRIVER="offscreen", SDL_VIDEO_DRIVER="offscreen",
               SteamAppId="400", SteamGameId="400",
               LD_LIBRARY_PATH=str(runtime / "bin") + ":" + os.environ.get("LD_LIBRARY_PATH", ""))
    env.pop("WAYLAND_DISPLAY", None)
    env.pop("DISPLAY", None)
    command = ["./hl2_launcher", "-game", str(game), "-windowed", "-w", "1024", "-h", "640",
               "-renderer", "native-vulkan", "-physics", "vphysics_box3d", "-multirun",
               "-novid", "-insecure", "-condebug", "-dev", "+developer", "2", "+sv_cheats", "1",
               "+volume", "0", "+mat_vsync", "0", "+map", MAP] + command_line
    try:
        result = subprocess.run(command, cwd=runtime, env=env, capture_output=True, text=True,
                                timeout=scenario.timeout)
        code = result.returncode
    except subprocess.TimeoutExpired:
        code = "timeout"
    # One log: -condebug's console.log (engine.log repeats it with timestamps).
    source = log_path if log_path.exists() else runtime / "engine.log"
    log = source.read_text(errors="replace") if source.exists() else ""
    # Developer messages reach the log twice in a row; keep one of each.
    lines, previous = [], None
    for line in log.splitlines():
        text = re.sub(r"^\[[\d.]+\] ", "", line)
        if text != previous:
            lines.append(text)
        previous = text
    log = "\n".join(lines)
    (out / (scenario.name + ".log")).write_text(log)
    results = [check(log) for check in scenario.checks]
    if code != 0:
        results.insert(0, (False, "launcher exited %s" % code))
    if "Spawn Server: %s" % MAP not in log:
        results.insert(0, (False, "the map never loaded"))
    return results


def main(argv=None):
    import fstop_mechanics_scenarios as scenarios_module
    scenarios = scenarios_module.SCENARIOS
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("names", nargs="*", help="scenario names (default: all)")
    parser.add_argument("--list", action="store_true")
    parser.add_argument("--runtime", type=Path, default=ROOT / "run/runtime-fstop-check")
    parser.add_argument("--content-root", type=Path, default=DEFAULT_CONTENT)
    parser.add_argument("--build", type=Path, default=ROOT / "build-fstop")
    parser.add_argument("--bsp", type=Path,
                        default=ROOT / "run/runtime-fstop/fstop/maps" / (MAP + ".bsp"))
    parser.add_argument("--out", type=Path, default=ROOT / "quality-results/fstop-mechanics-check")
    parser.add_argument("--keep", action="store_true", help="keep the private runtime")
    args = parser.parse_args(argv)
    if args.list:
        for s in scenarios:
            print("%-22s %s" % (s.name, s.summary))
        return 0
    chosen = [s for s in scenarios if not args.names or s.name in args.names]
    unknown = set(args.names) - {s.name for s in scenarios}
    if unknown or not chosen:
        print("fstop_mechanics_check: unknown scenario(s): %s" % ", ".join(sorted(unknown)))
        return 2
    args.out.mkdir(parents=True, exist_ok=True)
    runtime = args.runtime.resolve()
    stage(runtime, args.content_root, args.build, args.bsp)
    report, failed = {}, 0
    try:
        for scenario in chosen:
            results = run(scenario, runtime, args.out)
            ok = all(r[0] for r in results)
            failed += not ok
            report[scenario.name] = {"pass": ok, "checks": [{"pass": r[0], "detail": r[1]}
                                                            for r in results]}
            print("%s %-22s %s" % ("PASS" if ok else "FAIL", scenario.name, scenario.summary))
            for r in results:
                if not r[0] or os.environ.get("VERBOSE"):
                    print("     %s %s" % ("ok  " if r[0] else "FAIL", r[1]))
    finally:
        if not args.keep:
            shutil.rmtree(runtime, ignore_errors=True)
    (args.out / "report.json").write_text(json.dumps(report, indent=1))
    print("fstop_mechanics_check: %d of %d scenario(s) passed (%s)"
          % (len(chosen) - failed, len(chosen), args.out / "report.json"))
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
