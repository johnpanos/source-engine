#!/usr/bin/env python3
"""Run scripted Portal 2 gameplay scenarios against the built Portal 2 target.

Each scenario loads one retail map in a private staged runtime, runs a VScript
driver (quality/workloads/portal2-wheatley-v1) that teleports to key story
events and drives the player through console input, and prints one
``QA_CHECK`` line per check. A scenario passes only when the process exits
normally, every declared check is reported exactly once as PASS, no undeclared
check appears, the driver prints a matching ``QA_DONE`` record, and no
scenario script raised a Squirrel error.

A scenario may also declare ``console_checks`` for evidence the server-side
driver cannot see, such as client debug output. The driver brackets a window
with ``QA_WINDOW <label> BEGIN`` and ``QA_WINDOW <label> END``; a check then
needs at least one line in that window matching ``select``, every one of them
matching ``expect``, or no line matching ``absent``. A ``view_continuity``
check instead judges the client's per-frame view trace in the window
(``cl_portal_view_trace``; tools/quality/portal_view_trace.py): no frame may
jump or turn beyond its limits once portal crossings are undone, and the
window must hold the declared number of crossings.

A scenario may continue on another map: ``arrival`` names the map the start
map's own level change leads to and the script to run there. The start script
ends its part with ``QA_Handoff()``, which prints ``QA_HANDOFF`` with its counts
(the level change resets the VM); a ``mapspawn.nut`` hook, rebuilt from the
installed game's own file for each run, starts the arrival script one second
after the arrival map spawns, and that script finishes with ``QA_DONE``. The
QA_DONE counts then cover only the arrival map, and the harness adds the
handoff's. A scenario with ``arrival`` needs exactly one handoff from its start
map and a QA_DONE from the arrival map; one without must not hand off.

A workload may declare ``maps`` that are not retail content: each names a
builder script (relative to the repository root) that compiles the map and
installs it into the runtime's portal2/maps before any scenario runs.

The client renders offscreen (SDL's offscreen video driver), so no window
reaches the desktop. Retail content comes from a local Steam installation.
"""

import argparse
import datetime
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import time

import conformance
import launch_sandbox
import portal_view_trace
import stage_portal2_runtime

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "kiln"))
import sepipe_loader  # noqa: E402


SCHEMA = "portal2-scenarios/v1"
EVIDENCE_SCHEMA = "portal2-scenario-evidence/v1"
DEFAULT_WORKLOAD = Path("quality/workloads/portal2-wheatley-v1/scenarios.json")
SIMPLE_NAME = re.compile(r"[A-Za-z0-9_]+")
CHECK_NAME = re.compile(r"[A-Za-z0-9_]+(\.[A-Za-z0-9_]+)+")
CHECK_LINE = re.compile(r"^QA_CHECK (\S+) (PASS|FAIL)(?: (.*))?$")
DONE_LINE = re.compile(r"^QA_DONE (\S+) checks=(\d+) failures=(\d+)\s*$")
WINDOW_LINE = re.compile(r"^QA_WINDOW (\S+) (BEGIN|END)\s*$")
HANDOFF_LINE = re.compile(r"^QA_HANDOFF (\S+) map=(\S+) checks=(\d+) failures=(\d+)\s*$")
START_LINE = re.compile(r"^QA_LOG (\S+) t=\S+ start map=(\S+) ")
MAPSPAWN = "mapspawn.nut"
SCRIPT_ERROR = "AN ERROR HAS OCCURED"
# The callstack Squirrel prints after an error names the failing script file.
SCRIPT_ERROR_CONTEXT_LINES = 24
SCRIPT_DIRECTORY = "qa"
DRIVER_LOAD_FAILURE = "Error running script named %s/" % SCRIPT_DIRECTORY


class ScenarioError(Exception):
    pass


def load_workload(path):
    path = Path(path)
    try:
        workload = json.loads(path.read_text())
    except (OSError, json.JSONDecodeError) as error:
        raise ScenarioError("cannot read %s: %s" % (path, error))
    if workload.get("schema") != SCHEMA:
        raise ScenarioError("%s: schema must be %s" % (path, SCHEMA))
    driver = workload.get("driver")
    if not isinstance(driver, str) or not (path.parent / driver).is_file():
        raise ScenarioError("%s: driver script is missing" % path)
    includes = workload.get("includes", [])
    if not isinstance(includes, list) or not all(
            isinstance(script, str) and script.endswith(".nut") and (path.parent / script).is_file()
            for script in includes):
        raise ScenarioError("%s: an included script is missing" % path)
    maps = workload.get("maps", {})
    if not isinstance(maps, dict) or not all(
            SIMPLE_NAME.fullmatch(str(name)) and isinstance(builder, str) and
            (Path(conformance.repo_root()) / builder).is_file() for name, builder in maps.items()):
        raise ScenarioError("%s: a map builder is missing" % path)
    installed = [Path(script).name for script in [driver] + includes] + \
        [Path(str(script)).name for scenario in workload.get("scenarios", [])
         for script in scenario_scripts(scenario)]
    if len(set(installed)) != len(installed):
        raise ScenarioError("%s: scripts must have distinct file names" % path)
    scenarios = workload.get("scenarios")
    if not isinstance(scenarios, list) or not scenarios:
        raise ScenarioError("%s: no scenarios" % path)
    names = set()
    for scenario in scenarios:
        name = scenario.get("name")
        if not isinstance(name, str) or not SIMPLE_NAME.fullmatch(name) or name in names:
            raise ScenarioError("%s: scenario names must be unique simple names" % path)
        names.add(name)
        if not SIMPLE_NAME.fullmatch(str(scenario.get("map", ""))):
            raise ScenarioError("%s: %s needs a simple map name" % (path, name))
        if "retail_map" in scenario and not SIMPLE_NAME.fullmatch(str(scenario["retail_map"])):
            raise ScenarioError("%s: %s needs a simple retail map name" % (path, name))
        script = scenario.get("script")
        if not isinstance(script, str) or not script.endswith(".nut") or \
                not (path.parent / script).is_file():
            raise ScenarioError("%s: %s script is missing" % (path, name))
        arrival = scenario.get("arrival")
        if arrival is not None:
            if not isinstance(arrival, dict) or set(arrival) != {"map", "script"} or \
                    not SIMPLE_NAME.fullmatch(str(arrival["map"])) or \
                    arrival["map"] == scenario["map"] or \
                    not str(arrival["script"]).endswith(".nut") or \
                    not (path.parent / arrival["script"]).is_file():
                raise ScenarioError("%s: %s arrival needs another simple map name and an "
                                    "existing script" % (path, name))
        timeout = scenario.get("timeout_seconds")
        if not isinstance(timeout, (int, float)) or timeout <= 0:
            raise ScenarioError("%s: %s needs a positive timeout" % (path, name))
        checks = scenario.get("required_checks")
        if not isinstance(checks, list) or not checks or len(set(checks)) != len(checks) or \
                not all(isinstance(check, str) and CHECK_NAME.fullmatch(check) for check in checks):
            raise ScenarioError("%s: %s needs unique dotted required checks" % (path, name))
        load_console_checks(path, scenario)
    return workload


def retail_map(scenario):
    """The shipped map retail runs, and whose BSP holds the gameplay data:
    ``retail_map`` for a scenario on a published derivative of a shipped map
    (a relit map keeps every gameplay lump byte-identical), else ``map``."""
    return scenario.get("retail_map", scenario["map"])


def scenario_scripts(scenario):
    """The scenario's start script, then its arrival script if it has one."""
    scripts = [scenario.get("script", "")]
    if isinstance(scenario.get("arrival"), dict):
        scripts.append(scenario["arrival"].get("script", ""))
    return scripts


def load_console_checks(path, scenario):
    """Validates a scenario's console checks and compiles their patterns."""
    name = scenario["name"]
    console_checks = scenario.get("console_checks", [])
    if not isinstance(console_checks, list):
        raise ScenarioError("%s: %s console_checks must be a list" % (path, name))
    seen = set()
    for check in console_checks:
        if not isinstance(check, dict) or check.get("name") not in scenario["required_checks"] or \
                check["name"] in seen:
            raise ScenarioError("%s: %s console checks need unique required names" % (path, name))
        seen.add(check["name"])
        if not SIMPLE_NAME.fullmatch(str(check.get("window", ""))):
            raise ScenarioError("%s: %s needs a simple window name" % (path, check["name"]))
        keys = set(check) - {"name", "window"}
        if keys == {"view_continuity"}:
            limits = check["view_continuity"]
            if not isinstance(limits, dict) or not set(limits) <= set(portal_view_trace.DEFAULTS) or \
                    not all(isinstance(value, (int, float)) for value in limits.values()):
                raise ScenarioError("%s: %s view_continuity takes %s" % (
                    path, check["name"], ", ".join(sorted(portal_view_trace.DEFAULTS))))
            continue
        if keys not in ({"select", "expect"}, {"absent"}):
            raise ScenarioError("%s: %s needs select and expect, absent, or view_continuity" % (
                path, check["name"]))
        for key in keys:
            try:
                re.compile(check[key])
            except (TypeError, re.error) as error:
                raise ScenarioError("%s: %s %s: %s" % (path, check["name"], key, error))


def console_windows(lines):
    """Lines between each QA_WINDOW label's BEGIN and END; None if not bracketed once."""
    windows, open_windows, markers = {}, {}, {}
    for line in lines:
        match = WINDOW_LINE.match(line.strip())
        if match:
            label, edge = match.group(1), match.group(2)
            markers.setdefault(label, []).append(edge)
            if edge == "BEGIN":
                open_windows[label] = []
            elif label in open_windows:
                windows[label] = open_windows.pop(label)
            continue
        for window in open_windows.values():
            window.append(line)
    return {label: windows.get(label) if edges == ["BEGIN", "END"] else None
            for label, edges in markers.items()}


def evaluate_console_check(check, windows, lines=()):
    """Returns (outcome, detail) for one console check."""
    window = windows.get(check["window"])
    if window is None:
        return "FAIL", "window %s was not bracketed once" % check["window"]
    if "view_continuity" in check:
        before, window = portal_view_trace.window_lines(list(lines), check["window"])
        ok, detail, _ = portal_view_trace.judge(before, window, check["view_continuity"])
        return ("PASS" if ok else "FAIL"), detail
    if "absent" in check:
        found = [line for line in window if re.search(check["absent"], line)]
        if found:
            return "FAIL", "%d lines match: %s" % (len(found), found[0].strip())
        return "PASS", "none of %d lines match" % len(window)
    selected = [line for line in window if re.search(check["select"], line)]
    if not selected:
        return "FAIL", "no line in %d matches the selection" % len(window)
    wrong = [line for line in selected if not re.search(check["expect"], line)]
    if wrong:
        return "FAIL", "%d of %d selected lines differ: %s" % (len(wrong), len(selected),
                                                                wrong[0].strip())
    return "PASS", "%d selected lines match" % len(selected)


def script_errors(lines):
    """Squirrel errors, each with the callstack lines that follow it."""
    errors = []
    for index, line in enumerate(lines):
        if SCRIPT_ERROR in line:
            errors.append(lines[index:index + SCRIPT_ERROR_CONTEXT_LINES])
    return errors


def evaluate(scenario, log, returncode, timed_out):
    """Judge one run from its console log; returns the evidence record."""
    name = scenario["name"]
    required = scenario["required_checks"]
    lines = log.splitlines()
    checks, failures, undeclared = {}, [], []
    for line in lines:
        match = CHECK_LINE.match(line.strip())
        if not match:
            continue
        qualified, outcome, detail = match.group(1), match.group(2), match.group(3) or ""
        prefix, _, check = qualified.partition(".")
        if prefix != name:
            failures.append("check from another scenario: " + qualified)
            continue
        if check in checks:
            failures.append("check reported twice: " + check)
            continue
        checks[check] = {"outcome": outcome, "detail": detail}
        if check not in required:
            undeclared.append(check)
        if outcome != "PASS":
            failures.append("%s failed: %s" % (check, detail))
    failures += ["undeclared check: " + check for check in undeclared]

    # A scenario with an arrival map hands off once on its start map; the
    # arrival map's QA_DONE counts only its own checks.
    handoffs = [HANDOFF_LINE.match(line.strip()) for line in lines]
    handoffs = [match for match in handoffs if match]
    arrival = scenario.get("arrival")
    handed = (0, 0)
    if arrival is None:
        if handoffs:
            failures.append("QA_HANDOFF from a scenario without an arrival map")
    elif len(handoffs) != 1:
        failures.append("expected one QA_HANDOFF record, found %d" % len(handoffs))
    elif handoffs[0].group(1) != name or handoffs[0].group(2) != scenario["map"]:
        failures.append("QA_HANDOFF names %s on %s" % (handoffs[0].group(1), handoffs[0].group(2)))
    else:
        handed = (int(handoffs[0].group(3)), int(handoffs[0].group(4)))
    if arrival is not None:
        started = [START_LINE.match(line.strip()) for line in lines]
        started = [match.group(2) for match in started if match and match.group(1) == name]
        if started != [scenario["map"], arrival["map"]]:
            failures.append("driver started on %s, expected %s then %s" % (
                ", ".join(started) or "no map", scenario["map"], arrival["map"]))

    done = [DONE_LINE.match(line.strip()) for line in lines]
    done = [match for match in done if match]
    if len(done) != 1:
        failures.append("expected one QA_DONE record, found %d" % len(done))
    else:
        record = done[0]
        reported_failures = sum(1 for value in checks.values() if value["outcome"] != "PASS")
        counts = (int(record.group(2)) + handed[0], int(record.group(3)) + handed[1])
        if record.group(1) != name:
            failures.append("QA_DONE names scenario " + record.group(1))
        elif counts != (len(checks), reported_failures):
            failures.append("QA_DONE counts %d/%d differ from the %d checks and %d failures "
                            "in the log" % (counts[0], counts[1], len(checks),
                                            reported_failures))

    windows = console_windows(lines)
    for console_check in scenario.get("console_checks", []):
        check = console_check["name"]
        if check in checks:
            failures.append("check reported twice: " + check)
            continue
        outcome, detail = evaluate_console_check(console_check, windows, lines)
        checks[check] = {"outcome": outcome, "detail": detail}
        if outcome != "PASS":
            failures.append("%s failed: %s" % (check, detail))
    failures += ["required check not reported: " + check
                 for check in required if check not in checks]

    errors = script_errors(lines)
    own_errors = [error for error in errors
                  if any(SCRIPT_DIRECTORY + "/" in line or "qa_driver" in line for line in error)]
    failures += ["scenario script error: " + error[0].strip() for error in own_errors]

    if timed_out:
        failures.append("timed out")
    elif returncode != 0:
        failures.append("process exited with status %d" % returncode)

    return {
        "scenario": name,
        "map": scenario["map"],
        "status": "pass" if not failures else "fail",
        "returncode": returncode,
        "timed_out": timed_out,
        "checks": checks,
        "failures": failures,
        "map_script_errors": [error[0].strip() for error in errors if error not in own_errors],
    }


def install_scripts(workload_path, workload, runtime):
    source = Path(workload_path).parent
    vscripts = Path(runtime) / "portal2/scripts/vscripts" / SCRIPT_DIRECTORY
    if vscripts.exists():
        shutil.rmtree(vscripts)
    vscripts.mkdir(parents=True)
    scripts = [workload["driver"]] + workload.get("includes", []) + \
        [script for scenario in workload["scenarios"] for script in scenario_scripts(scenario)]
    for script in scripts:
        # A workload may share another workload's driver; scripts install flat
        # under qa/, so scenarios include them as qa/<name>.
        shutil.copy2(source / script, vscripts / Path(script).name)
    config = Path(runtime) / "portal2/cfg"
    for scenario in workload["scenarios"]:
        # Only a single-line config honours wait; this one only starts the driver.
        (config / ("qa_%s.cfg" % scenario["name"])).write_text(
            "script_execute %s/%s\n" % (SCRIPT_DIRECTORY, Path(scenario["script"]).stem))


def install_mapspawn_hook(runtime, steam_root, scenario):
    """Rebuilds scripts/vscripts/mapspawn.nut from the installed game's own
    file, adding the hook that starts the scenario's arrival script."""
    hook = Path(runtime) / "portal2/scripts/vscripts" / MAPSPAWN
    pristine = Path(steam_root) / "portal2/scripts/vscripts" / MAPSPAWN
    text = pristine.read_text(errors="replace").rstrip() + "\n" if pristine.is_file() else ""
    arrival = scenario.get("arrival")
    if arrival is not None:
        # Only the server VM has EntFire, and the driver's QA table guards
        # against a second start in the same VM.
        include = "if ( !( \\\"QA\\\" in getroottable() ) ) DoIncludeScript( \\\"%s/%s\\\", " \
            "getroottable() )" % (SCRIPT_DIRECTORY, Path(arrival["script"]).stem)
        text += ("// PORTAL2_SCENARIOS_ARRIVAL_HOOK\n"
                 "if ( ( \"EntFire\" in getroottable() ) && GetMapName() == \"%s\" )\n"
                 "\tEntFire( \"worldspawn\", \"RunScriptCode\", \"%s\", 1.0 )\n"
                 % (arrival["map"], include))
    if hook.is_symlink() or hook.exists():
        hook.unlink()
    hook.parent.mkdir(parents=True, exist_ok=True)
    hook.write_text(text)


def build_maps(workload, runtime, output):
    """Compile and install the workload's own maps (``maps``) into the runtime."""
    root = Path(conformance.repo_root())
    for name, builder in sorted(workload.get("maps", {}).items()):
        log = output / ("map-%s.log" % name)
        with log.open("wb") as stream:
            code = subprocess.run([sys.executable, str(root / builder), "--runtime", str(runtime),
                                   "--install-game-dir", str(Path(runtime) / "portal2"),
                                   "--out", str(output / "maps" / name)],
                                  stdout=stream, stderr=subprocess.STDOUT).returncode
        if code != 0 or not (Path(runtime) / "portal2/maps" / (name + ".bsp")).is_file():
            raise ValueError("building map %s failed (%s)" % (name, log))


def write_fake_zenity(directory):
    """Error() dialogs go through zenity; never let one open or block a run."""
    directory.mkdir(parents=True, exist_ok=True)
    zenity = directory / "zenity"
    zenity.write_text("#!/bin/sh\n"
                      "if [ \"$1\" = --version ]; then echo 4.0.2; exit 0; fi\n"
                      "echo \"zenity $*\" >&2\nexit 1\n")
    zenity.chmod(0o755)


def package_runtime(client, runtime):
    """Package the client profile into a private runtime (RFC 0027 kiln.api:
    the same packager and steps as `kiln play`); returns the stage summaries."""
    profile, flavor = client
    launch_sandbox.check_write_paths([runtime])
    try:
        built = sepipe_loader.session().build(profile, flavor=flavor, up_to="package",
                                              runtime=str(Path(runtime).resolve()))
    except Exception as error:  # sepipe.KilnError
        raise ScenarioError("kiln package %s --flavor %s: %s" % (profile, flavor, error)) from error
    return {stage["name"]: stage["summary"] for stage in built["stages"]}


def run_scenario(scenario, runtime, output, start_frames, width, height, tool_directory,
                 gdb_script=None, extra_args=(), wrapper=(), steam_root=None,
                 client=("portal2", "dev")):
    runtime = Path(runtime).resolve()
    if steam_root is None:
        if scenario.get("arrival") is not None:
            raise ScenarioError("%s has an arrival map but no Steam root for the hook" %
                                scenario["name"])
        return run_game(scenario, runtime, output, start_frames, width, height, tool_directory,
                        gdb_script, extra_args, wrapper, client)
    install_mapspawn_hook(runtime, steam_root, scenario)
    try:
        return run_game(scenario, runtime, output, start_frames, width, height, tool_directory,
                        gdb_script, extra_args, wrapper, client)
    finally:
        # Other workloads share the runtime: leave the game's own file.
        install_mapspawn_hook(runtime, steam_root, {})


def run_game(scenario, runtime, output, start_frames, width, height, tool_directory,
             gdb_script, extra_args, wrapper, client=("portal2", "dev")):
    """One scenario through kiln.api: the client profile's program, display
    session (headless) and run provider; this harness's test command,
    sandbox variables and diagnostic wrapper."""
    console = runtime / "portal2/console.log"
    console.unlink(missing_ok=True)
    # A throwaway HOME/XDG; the private runtime (never a player's) is the write path.
    sandbox = launch_sandbox.Sandbox(Path(output).resolve() / "sandbox", write_paths=[runtime])
    environment = sandbox.environment(os.environ)
    environment["PATH"] = str(tool_directory) + os.pathsep + environment.get("PATH", "")
    overrides = {key: value for key, value in environment.items() if os.environ.get(key) != value}
    overrides.update({key: None for key in os.environ if key not in environment})
    arguments = ["-game", "portal2", "-multirun", "-novid", "-insecure", "-windowed",
                 "-w", str(width), "-h", str(height), "-condebug", "+volume", "0", *extra_args,
                 "+map", scenario["map"], "+wait", str(start_frames),
                 "+exec", "qa_" + scenario["name"]]
    prefix = list(wrapper)
    if gdb_script:
        # Diagnosis only: gdb's own exit status replaces the game's.
        prefix += ["gdb", "-q", "-batch", "-x", str(Path(gdb_script).resolve()), "-ex", "run",
                   "-ex", "bt", "--args"]
    output.mkdir(parents=True, exist_ok=True)
    profile, flavor = client
    started = time.monotonic()
    timed_out = False
    run = sepipe_loader.Run(sepipe_loader.load(), sepipe_loader.session(), "run", profile,
                            flavor=flavor, runtime=str(runtime), exact_arguments=arguments,
                            wrapper=prefix, environment=overrides, display="none",
                            log=str(output / "stdout.log"))
    # The driver quits the game after QA_DONE. A driver that failed to load
    # never will, so stop waiting for it as soon as the log says so.
    deadline = started + scenario["timeout_seconds"]
    while run.poll() is None and time.monotonic() < deadline:
        time.sleep(0.5)
        if console.is_file() and DRIVER_LOAD_FAILURE in console.read_text(errors="replace"):
            deadline = min(deadline, time.monotonic() + 2.0)
    if run.poll() is None:
        timed_out = True
        run.stop()
    seconds = time.monotonic() - started
    log = console.read_text(errors="replace") if console.is_file() else ""
    (output / "console.log").write_text(log)
    result = evaluate(scenario, log, run.returncode, timed_out)
    if run.error:
        result.setdefault("failures", []).append("kiln: " + run.error)
    result["seconds"] = round(seconds, 1)
    result["command"] = {"profile": profile, "flavor": flavor, "wrapper": prefix,
                         "arguments": arguments}
    result["sandbox"] = sandbox.finish()
    return result


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    root = Path(conformance.repo_root())
    parser.add_argument("--workload", type=Path, default=root / DEFAULT_WORKLOAD)
    parser.add_argument("--steam-root", type=Path, default=Path(os.environ.get(
        "P2_STEAM_ROOT", Path.home() / ".local/share/Steam/steamapps/common/Portal 2")))
    sepipe_loader.add_arguments(parser, "portal2")
    parser.add_argument("--runtime", type=Path, default=root / "run/runtime-p2-scenarios",
                        help="private runtime kiln packages the profile into")
    parser.add_argument("--out", type=Path, required=True, help="new evidence directory")
    parser.add_argument("--scenario", action="append", default=[],
                        help="run only this scenario (repeatable)")
    parser.add_argument("--start-frames", type=int, default=300,
                        help="client frames to wait after the map loads before the driver starts")
    parser.add_argument("--width", type=int, default=1280)
    parser.add_argument("--height", type=int, default=720)
    parser.add_argument("--gdb-script", type=Path,
                        help="run the game under gdb with this command file (diagnosis; "
                             "the output lands in each scenario's stdout.log)")
    parser.add_argument("--extra-arg", action="append", default=[],
                        help="extra engine command-line argument, placed before +map "
                             "(repeatable), e.g. --extra-arg='+host_thread_mode 1'")
    parser.add_argument("--list", action="store_true", help="list scenarios and exit")
    args = parser.parse_args(argv)

    try:
        workload = load_workload(args.workload)
    except ScenarioError as error:
        parser.exit(2, "portal2_scenarios: %s\n" % error)
    scenarios = workload["scenarios"]
    if args.list:
        for scenario in scenarios:
            print("%s\t%s\t%d checks" % (scenario["name"], scenario["map"],
                                         len(scenario["required_checks"])))
        return 0
    if args.scenario:
        unknown = set(args.scenario) - {scenario["name"] for scenario in scenarios}
        if unknown:
            parser.error("unknown scenario: " + ", ".join(sorted(unknown)))
        scenarios = [scenario for scenario in scenarios if scenario["name"] in args.scenario]

    output = args.out.resolve()
    if (output / "evidence.json").exists():
        parser.error("evidence already exists; use a new output directory")
    output.mkdir(parents=True, exist_ok=True)
    evidence = {
        "schema": EVIDENCE_SCHEMA,
        "status": "incomplete",
        "started_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
        "source": conformance.source_identity(str(root)),
        "workload": str(args.workload),
        "steam_root": str(args.steam_root),
        "runtime": str(args.runtime.resolve()),
        "results": [],
    }
    evidence_path = output / "evidence.json"
    try:
        # Packaging rewrites the runtime: package_runtime refuses a player's.
        evidence["installed"] = package_runtime((args.profile, args.flavor), args.runtime)
        install_scripts(args.workload, workload, args.runtime)
        build_maps(workload, args.runtime.resolve(), output)
    except (OSError, ValueError) as error:
        evidence["status"] = "fail"
        evidence["error"] = str(error)
        evidence_path.write_text(json.dumps(evidence, indent=2) + "\n")
        parser.exit(1, "portal2_scenarios: %s\n" % error)
    tools = output / "tools"
    write_fake_zenity(tools)

    for scenario in scenarios:
        print("== %s (%s)" % (scenario["name"], scenario["map"]), flush=True)
        result = run_scenario(scenario, args.runtime, output / scenario["name"], args.start_frames,
                              args.width, args.height, tools, args.gdb_script, args.extra_arg,
                              steam_root=args.steam_root, client=(args.profile, args.flavor))
        evidence["results"].append(result)
        evidence_path.write_text(json.dumps(evidence, indent=2) + "\n")
        for check, value in result["checks"].items():
            print("  %-4s %s %s" % (value["outcome"], check, value["detail"]))
        for failure in result["failures"]:
            print("  FAILURE " + failure)
        print("  %s in %.0fs" % (result["status"].upper(), result["seconds"]), flush=True)

    failed = [result["scenario"] for result in evidence["results"] if result["status"] != "pass"]
    evidence["status"] = "fail" if failed else "pass"
    evidence["finished_utc"] = datetime.datetime.now(datetime.timezone.utc).isoformat()
    evidence_path.write_text(json.dumps(evidence, indent=2) + "\n")
    print("%d/%d scenarios passed; evidence: %s" % (
        len(scenarios) - len(failed), len(scenarios), evidence_path))
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
