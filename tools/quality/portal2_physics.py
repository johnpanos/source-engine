#!/usr/bin/env python3
"""Portal 2 physics against retail: portals, carried objects, plates, funnels, bridges.

The scenarios in quality/workloads/portal2-physics-v1 are VScript drivers
(the qa_driver.nut of tools/quality/portal2_scenarios.py) that set up one
physical situation each on a retail map: a fling through a floor/wall portal
pair, a cube carried through a portal, a faith plate launch, a funnel ride, a
light bridge underfoot, a cube on a floor button. Besides their QA_CHECK
lines they print measurements:

    QA_METRIC <scenario>.<name> <number>

The same scripts run on the retail Linux Portal 2 (`portal2_linux`, 32-bit,
in a private symlink mirror of the Steam installation, started from a
mapspawn.nut hook because retail ignores +wait on the command line) and on
this repository's Portal 2 target. The workload declares each metric's
tolerance, chosen per operation: speeds relative, positions absolute,
success flags exact. Trajectories are never compared tick for tick, so IVP
and Box3D need not match retail's solver, only its outcomes.

Commands:

  record   run retail (--runs N) and write the reference file
           (quality/workloads/portal2-physics-v1/retail-reference.json).
  run      run one target (retail or ours) and write evidence.
  check    run this build (--physics box3d|ivp, default box3d, as ./play_p2)
           and judge it against the reference; prints one checks-v1 record.
           --seed-fault injects a known defect as a negative control.
  compare  judge an existing evidence directory against the reference.

Retail needs the Steam client running (it refuses to start otherwise, and
aborts on a stalled IPC pipe when Steam exits under it). Retail content comes from a local Steam installation (--steam-root, default
$P2_STEAM_ROOT or ~/.local/share/Steam/steamapps/common/Portal 2); without it
the command fails, so a run is never silently certified. Our build renders
offscreen (SDL); retail runs inside a private headless mutter with Xwayland,
never on the desktop.
"""

import argparse
import datetime
import json
import math
import os
from pathlib import Path
import re
import shutil
import signal
import subprocess
import sys
import time

import conformance
import conformance_result
import portal2_scenarios
import stage_portal2_runtime


ROOT = Path(conformance.repo_root())
DEFAULT_WORKLOAD = ROOT / "quality/workloads/portal2-physics-v1/scenarios.json"
REFERENCE_SCHEMA = "portal2-physics-reference/v1"
EVIDENCE_SCHEMA = "portal2-physics-evidence/v1"
METRIC_LINE = re.compile(r"^QA_METRIC (\S+) (-?[0-9.]+(?:e[-+]?[0-9]+)?|nan|-?inf)\s*$")
COMPILE_FAILURE = "FAILED to compile and execute script file named scripts/vscripts/qa/"
# ./play_p2's physics selection: Box3D with RFC 0013 shape inertia (its
# PHYSICS and PHYSICS_ARGS defaults). IVP is PHYSICS=vphysics.
PHYSICS_ARGS = {
    "box3d": ["-physics", "vphysics_box3d", "-physics_shape_inertia"],
    "ivp": ["-physics", "vphysics"],
}
# Negative controls: each seeds a defect the suite must detect. Engine
# settings stand in for game defects with the same symptom.
SEED_FAULTS = {
    # Gravity 25% high: flings land short, plates overshoot their arcs.
    "gravity": {"args": ["+sv_gravity", "750"]},
    # Speed clamp far below a fling's exit speed: momentum is lost.
    "speed-clamp": {"args": ["+sv_maxvelocity", "300"]},
    # Evaluator-only: every measured speed out of a portal is halved.
    "halve-exit-speed": {"scale_metrics": {"speed_out": 0.5}},
    # Evaluator-only: every exact metric (success flags, counts) reads 0.
    "drop-flags": {"clear_flags": True},
}


class PhysicsError(Exception):
    pass


def steam_root_default():
    return Path(os.environ.get("P2_STEAM_ROOT",
                               Path.home() / ".local/share/Steam/steamapps/common/Portal 2"))


def load_workload(path):
    try:
        workload = portal2_scenarios.load_workload(path)
    except portal2_scenarios.ScenarioError as error:
        raise PhysicsError(str(error))
    for scenario in workload["scenarios"]:
        metrics = scenario.get("metrics")
        if not isinstance(metrics, dict) or not metrics:
            raise PhysicsError("%s: %s declares no metrics" % (path, scenario["name"]))
        for name, rule in metrics.items():
            if not portal2_scenarios.CHECK_NAME.fullmatch(name):
                raise PhysicsError("%s: metric %s needs a dotted name" % (path, name))
            keys = set(rule) - {"note"}
            if keys not in ({"abs"}, {"rel"}, {"rel", "abs"}, {"exact"}):
                raise PhysicsError("%s: metric %s needs abs, rel, rel+abs or exact" % (path, name))
    return workload


def parse_metrics(log, scenario):
    """QA_METRIC values of one scenario; a repeated name is an error."""
    metrics, problems = {}, []
    for line in log.splitlines():
        match = METRIC_LINE.match(line.strip())
        if not match:
            continue
        prefix, _, name = match.group(1).partition(".")
        if prefix != scenario:
            continue
        if name in metrics:
            problems.append("metric reported twice: " + name)
            continue
        metrics[name] = float(match.group(2))
    return metrics, problems


# --- Retail -------------------------------------------------------------------

# 32-bit libraries retail needs beyond its own bin that current distributions
# no longer ship. Only these are linked from the Steam runtime: its whole
# library directory would also replace the host's libGL/X11 stack, and then
# SDL finds no GLX visual.
RETAIL_SHIMS = ("libbz2.so.1.0", "libpng12.so.0")


def retail_libraries(mirror):
    runtime = Path.home() / ".local/share/Steam/ubuntu12_32/steam-runtime"
    shims = Path(mirror) / "shims"
    shims.mkdir(exist_ok=True)
    for name in RETAIL_SHIMS:
        for directory in ("lib/i386-linux-gnu", "usr/lib/i386-linux-gnu"):
            source = runtime / directory / name
            link = shims / name
            if source.exists() and not link.is_symlink():
                link.symlink_to(source.resolve())
    return [str(shims)]


def make_retail_mirror(steam_root, mirror):
    """A symlink mirror of the installation with private portal2/cfg and scripts.

    The executable is copied so it resolves the mirror, not the installation,
    as its base directory; scripts/ is copied so the fixture can install its
    VScript files and a mapspawn.nut hook without touching the installation.
    """
    steam_root, mirror = Path(steam_root).resolve(), Path(mirror)
    if not (steam_root / "portal2_linux").is_file():
        raise PhysicsError("%s has no portal2_linux" % steam_root)
    (mirror / "portal2").mkdir(parents=True, exist_ok=True)
    for entry in steam_root.iterdir():
        if entry.name in ("portal2", "portal2_linux"):
            continue
        link = mirror / entry.name
        if not link.is_symlink():
            link.symlink_to(entry)
    if not (mirror / "portal2_linux").is_file():
        shutil.copy2(steam_root / "portal2_linux", mirror / "portal2_linux")
    for entry in (steam_root / "portal2").iterdir():
        if entry.name in ("cfg", "scripts", "console.log", "screenshots"):
            continue
        link = mirror / "portal2" / entry.name
        if not link.is_symlink() and not link.exists():
            link.symlink_to(entry)
    for name in ("cfg", "scripts"):
        if not (mirror / "portal2" / name).is_dir():
            shutil.copytree(steam_root / "portal2" / name, mirror / "portal2" / name)
    return mirror


def install_retail_scripts(workload_path, workload, mirror, scenario):
    source = Path(workload_path).parent
    vscripts = Path(mirror) / "portal2/scripts/vscripts"
    qa = vscripts / portal2_scenarios.SCRIPT_DIRECTORY
    if qa.exists():
        shutil.rmtree(qa)
    qa.mkdir(parents=True)
    for script in [workload["driver"]] + workload.get("includes", []) + [scenario["script"]]:
        shutil.copy2(source / script, qa / Path(script).name)
    stem = Path(scenario["script"]).stem
    # The installation's mapspawn.nut only prints a banner; the hook starts
    # the fixture a second after the scenario's map spawns.
    (vscripts / "mapspawn.nut").write_text(
        'printl("==== calling mapspawn.nut")\n'
        'if ( GetMapName() == "%s" ) EntFire( "worldspawn", "RunScriptCode", '
        '"DoIncludeScript(\\"qa/%s\\", getroottable())", 1.0 )\n' % (scenario["map"], stem))


def run_retail_scenario(scenario, mirror, output, tool_directory, extra_args=()):
    mirror = Path(mirror).resolve()
    console = mirror / "portal2/console.log"
    console.unlink(missing_ok=True)
    output.mkdir(parents=True, exist_ok=True)
    environment = dict(os.environ)
    for variable in ("DISPLAY", "WAYLAND_DISPLAY"):
        environment.pop(variable, None)
    xdg = output / "xdg"
    xdg.mkdir(exist_ok=True)
    environment.update({
        "SteamAppId": "620", "SteamGameId": "620",
        "SDL_AUDIO_DRIVER": "dummy",
        # Xwayland, as the earlier retail comparisons ran it.
        "SDL_VIDEODRIVER": "x11",
        "XDG_CONFIG_HOME": str(xdg),

        "LD_LIBRARY_PATH": os.pathsep.join([str(mirror / "bin/linux32")] +
                                           retail_libraries(mirror)),
        "PATH": str(tool_directory) + os.pathsep + environment.get("PATH", ""),
    })
    display = "wl-p2phys-%d" % os.getpid()
    game = ["./portal2_linux", "-game", "portal2", "-novid", "-multirun", "-condebug",
            "-windowed", "-w", "1024", "-h", "768", "+snd_mute_losefocus", "0", "+volume", "0",
            *extra_args, "+map", scenario["map"]]
    command = ["dbus-run-session", "--", "mutter", "--headless", "--wayland",
               "--virtual-monitor", "1024x768@60", "--wayland-display", display, "--", *game]
    started = time.monotonic()
    timed_out = False
    with (output / "stdout.log").open("wb") as stream:
        process = subprocess.Popen(command, cwd=mirror, env=environment, stdout=stream,
                                   stderr=subprocess.STDOUT, start_new_session=True)
        deadline = started + scenario["timeout_seconds"] + 60
        done_at = None
        while process.poll() is None and time.monotonic() < deadline:
            time.sleep(0.5)
            if not console.is_file():
                continue
            text = console.read_text(errors="replace")
            if done_at is None and ("QA_DONE " in text or COMPILE_FAILURE in text or
                                    portal2_scenarios.DRIVER_LOAD_FAILURE in text):
                done_at = time.monotonic()
            # The driver quits the game; mutter lingers a moment after it.
            if done_at is not None and time.monotonic() > done_at + 20:
                break
        if process.poll() is None:
            timed_out = done_at is None
            os.killpg(process.pid, signal.SIGTERM)
            try:
                process.wait(timeout=10)
            except subprocess.TimeoutExpired:
                os.killpg(process.pid, signal.SIGKILL)
                process.wait()
    log = console.read_text(errors="replace") if console.is_file() else ""
    (output / "console.log").write_text(log)
    # Exit status belongs to the compositor session, not the game.
    result = portal2_scenarios.evaluate(scenario, log, 0, timed_out)
    result["seconds"] = round(time.monotonic() - started, 1)
    result["command"] = command
    return result


# --- Runs ---------------------------------------------------------------------

def run_target(args, workload, target, output, physics="box3d", fault=None):
    scenarios = select_scenarios(workload, args.scenario)
    tools = output / "tools"
    portal2_scenarios.write_fake_zenity(tools)
    extra = list((SEED_FAULTS.get(fault) or {}).get("args", []))
    evidence = {
        "schema": EVIDENCE_SCHEMA,
        "target": target,
        "physics": physics if target == "ours" else "retail",
        "seed_fault": fault,
        "started_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
        "source": conformance.source_identity(str(ROOT)),
        "workload": str(args.workload),
        "steam_root": str(args.steam_root),
        "results": [],
    }
    if target == "retail":
        mirror = make_retail_mirror(args.steam_root, args.mirror or output / "retail-mirror")
        evidence["mirror"] = str(mirror)
    else:
        runtime = args.runtime.resolve()
        stage_portal2_runtime.stage_content(args.steam_root, runtime)
        evidence["installed"] = stage_portal2_runtime.portal_boot.install_build(
            args.build, runtime, game="portal2")
        portal2_scenarios.install_scripts(args.workload, workload, runtime)
        evidence["runtime"] = str(runtime)
        evidence["build"] = str(Path(args.build).resolve())
    for scenario in scenarios:
        for run in range(args.runs):
            label = scenario["name"] + ("" if args.runs == 1 else "-%d" % run)
            print("== %s %s (%s)" % (target, label, scenario["map"]), flush=True)
            if target == "retail":
                install_retail_scripts(args.workload, workload, mirror, scenario)
                result = run_retail_scenario(scenario, mirror, output / label, tools, extra)
            else:
                result = portal2_scenarios.run_scenario(
                    scenario, runtime, output / label, args.start_frames, 1024, 768, tools,
                    extra_args=PHYSICS_ARGS[physics] + extra)
            log = (output / label / "console.log").read_text(errors="replace")
            if COMPILE_FAILURE in log:
                result["failures"].append("scenario script failed to compile")
                result["status"] = "fail"
            result["metrics"], problems = parse_metrics(log, scenario["name"])
            result["failures"] += problems
            result["run"] = run
            evidence["results"].append(result)
            (output / "evidence.json").write_text(json.dumps(evidence, indent=2) + "\n")
            for check, value in result["checks"].items():
                print("  %-4s %s %s" % (value["outcome"], check, value["detail"][:160]))
            for failure in result["failures"]:
                print("  FAILURE " + failure[:200])
            print("  %d metrics, %s in %.0fs" % (len(result["metrics"]), result["status"].upper(),
                                                 result["seconds"]), flush=True)
    evidence["finished_utc"] = datetime.datetime.now(datetime.timezone.utc).isoformat()
    (output / "evidence.json").write_text(json.dumps(evidence, indent=2) + "\n")
    return evidence


def select_scenarios(workload, names):
    scenarios = workload["scenarios"]
    if names:
        unknown = set(names) - {scenario["name"] for scenario in scenarios}
        if unknown:
            raise PhysicsError("unknown scenario: " + ", ".join(sorted(unknown)))
        scenarios = [scenario for scenario in scenarios if scenario["name"] in names]
    return scenarios


# --- Reference and judgement -----------------------------------------------------

def build_reference(workload, evidence):
    """Per scenario: the checks every retail run passed and each metric's range."""
    scenarios = {}
    for scenario in workload["scenarios"]:
        runs = [result for result in evidence["results"] if result["scenario"] == scenario["name"]]
        if not runs:
            continue
        failing = [result for result in runs if result["status"] != "pass"]
        if failing:
            raise PhysicsError("retail %s failed: %s" % (scenario["name"],
                                                         "; ".join(failing[0]["failures"])))
        metrics = {}
        for name in scenario["metrics"]:
            values = [result["metrics"].get(name) for result in runs]
            if any(value is None or not math.isfinite(value) for value in values):
                raise PhysicsError("retail %s did not report metric %s in every run"
                                   % (scenario["name"], name))
            metrics[name] = {"min": min(values), "max": max(values), "runs": len(values)}
        scenarios[scenario["name"]] = {"map": scenario["map"], "metrics": metrics,
                                       "checks": sorted(runs[0]["checks"])}
    return {
        "schema": REFERENCE_SCHEMA,
        "recorded_utc": evidence.get("finished_utc"),
        "steam_root": evidence.get("steam_root"),
        "retail_build": retail_identity(Path(evidence["steam_root"])),
        "source": evidence.get("source"),
        "scenarios": scenarios,
    }


def retail_identity(steam_root):
    """The installed retail build: its steam.inf version and binary size."""
    identity = {}
    inf = steam_root / "portal2/steam.inf"
    if inf.is_file():
        for line in inf.read_text(errors="replace").splitlines():
            key, _, value = line.partition("=")
            if key.strip() in ("ClientVersion", "PatchVersion", "VersionDate"):
                identity[key.strip()] = value.strip()
    server = steam_root / "portal2/bin/linux32/server.so"
    if server.is_file():
        identity["server_so_bytes"] = server.stat().st_size
    return identity


def allowed_range(rule, reference):
    low, high = reference["min"], reference["max"]
    if "exact" in rule:
        return low, high
    margin = 0.0
    if "rel" in rule:
        margin = max(margin, rule["rel"] * max(abs(low), abs(high)))
    if "abs" in rule:
        margin = max(margin, rule["abs"])
    return low - margin, high + margin


def apply_fault(fault, metrics, rules):
    spec = SEED_FAULTS.get(fault) or {}
    seeded = dict(metrics)
    for suffix, factor in spec.get("scale_metrics", {}).items():
        for name in seeded:
            if name.endswith("." + suffix):
                seeded[name] *= factor
    if spec.get("clear_flags"):
        for name in seeded:
            if "exact" in rules.get(name, {}):
                seeded[name] = 0.0
    return seeded


def judge(workload, reference, evidence, checks, fault=None):
    """Adds one check per scenario outcome, retail check and metric."""
    for scenario in workload["scenarios"]:
        name = scenario["name"]
        runs = [result for result in evidence["results"] if result["scenario"] == name]
        if not runs:
            continue
        ref = reference["scenarios"].get(name)
        if not checks.check(ref is not None, name + ".reference",
                            "no retail reference for this scenario"):
            continue
        for result in runs:
            tag = name if len(runs) == 1 else "%s#%d" % (name, result["run"])
            # The run itself: exited cleanly, every declared check reported.
            infrastructure = [failure for failure in result["failures"]
                              if " failed: " not in failure]
            checks.check(not infrastructure, tag + ".run", "; ".join(infrastructure)[:300])
            for check in ref["checks"]:
                value = result["checks"].get(check)
                checks.check(value is not None and value["outcome"] == "PASS",
                             "%s.%s" % (tag, check),
                             "retail passes; here %s" % (
                                 "not reported" if value is None else
                                 "%s: %s" % (value["outcome"], value["detail"][:200])))
            metrics = apply_fault(fault, result["metrics"], scenario["metrics"])
            for metric, rule in scenario["metrics"].items():
                target = ref["metrics"].get(metric)
                if target is None:
                    checks.check(False, "%s.%s" % (tag, metric), "no retail value")
                    continue
                low, high = allowed_range(rule, target)
                value = metrics.get(metric)
                checks.check(value is not None and low - 1e-6 <= value <= high + 1e-6,
                             "%s.%s" % (tag, metric),
                             "%s outside [%.2f, %.2f] (retail %.2f..%.2f)" % (
                                 "missing" if value is None else "%.2f" % value, low, high,
                                 target["min"], target["max"]))


def summarize(workload, reference, evidence):
    """One line per metric: retail range and this run's value."""
    lines = []
    for scenario in workload["scenarios"]:
        ref = reference["scenarios"].get(scenario["name"], {"metrics": {}})
        for result in evidence["results"]:
            if result["scenario"] != scenario["name"]:
                continue
            for metric in scenario["metrics"]:
                target = ref["metrics"].get(metric)
                value = result["metrics"].get(metric)
                lines.append("%-34s %-28s retail %s here %s" % (
                    scenario["name"], metric,
                    "-" if target is None else "%.1f..%.1f" % (target["min"], target["max"]),
                    "-" if value is None else "%.1f" % value))
    return lines


# --- Command line -------------------------------------------------------------

def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("command", choices=("record", "run", "check", "compare"))
    parser.add_argument("--workload", type=Path, default=DEFAULT_WORKLOAD)
    parser.add_argument("--reference", type=Path,
                        help="retail reference (default: retail-reference.json beside the workload)")
    parser.add_argument("--steam-root", type=Path, default=steam_root_default())
    parser.add_argument("--build", type=Path, default=ROOT / "build-p2",
                        help="Waf output configured with --build-games=portal2")
    parser.add_argument("--runtime", type=Path, default=ROOT / "run/runtime-p2-physics",
                        help="private staged runtime for this build (created on first use)")
    parser.add_argument("--mirror", type=Path,
                        help="retail symlink mirror (default: <out>/retail-mirror)")
    parser.add_argument("--target", choices=("ours", "retail"), default="ours")
    parser.add_argument("--physics", choices=sorted(PHYSICS_ARGS), default="box3d",
                        help="VPhysics provider for this build (default box3d, as ./play_p2)")
    parser.add_argument("--seed-fault", choices=sorted(SEED_FAULTS),
                        help="negative control: inject a known defect")
    parser.add_argument("--scenario", action="append", default=[])
    parser.add_argument("--runs", type=int, default=1, help="runs per scenario")
    parser.add_argument("--start-frames", type=int, default=300)
    parser.add_argument("--evidence", type=Path, help="compare: an existing evidence directory")
    parser.add_argument("--out", type=Path, help="new output directory")
    args = parser.parse_args(argv)
    reference_path = args.reference or args.workload.parent / "retail-reference.json"
    checks = conformance_result.Checks()

    try:
        workload = load_workload(args.workload)
        if args.command == "compare":
            evidence = json.loads((args.evidence / "evidence.json").read_text())
        else:
            if args.out is None:
                parser.error("--out is required")
            output = args.out.resolve()
            if (output / "evidence.json").exists():
                parser.error("evidence already exists; use a new output directory")
            output.mkdir(parents=True, exist_ok=True)
            if not (args.steam_root / "portal2/pak01_dir.vpk").is_file():
                raise PhysicsError("no retail Portal 2 at %s (set --steam-root or P2_STEAM_ROOT)"
                                   % args.steam_root)
            target = "retail" if args.command == "record" else (
                "ours" if args.command == "check" else args.target)
            fault = args.seed_fault if args.command == "check" else None
            evidence = run_target(args, workload, target, output, args.physics, fault)
        if args.command == "record":
            reference = build_reference(workload, evidence)
            reference_path.write_text(json.dumps(reference, indent=2) + "\n")
            print("reference: %s" % reference_path)
            return 0
        if args.command == "run":
            failed = [r["scenario"] for r in evidence["results"] if r["status"] != "pass"]
            return 1 if failed else 0
        reference = json.loads(reference_path.read_text())
        if reference.get("schema") != REFERENCE_SCHEMA:
            raise PhysicsError("%s: schema must be %s" % (reference_path, REFERENCE_SCHEMA))
        judge(workload, reference, evidence, checks, args.seed_fault)
        for line in summarize(workload, reference, evidence):
            print(line)
    except (PhysicsError, OSError, ValueError) as error:
        checks.check(False, "portal2-physics.setup", str(error))
    return checks.report()


if __name__ == "__main__":
    sys.exit(main())
