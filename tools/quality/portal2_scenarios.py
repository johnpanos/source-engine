#!/usr/bin/env python3
"""Run scripted Portal 2 gameplay scenarios against the built Portal 2 target.

Each scenario loads one retail map in a private staged runtime, runs a VScript
driver (quality/workloads/portal2-wheatley-v1) that teleports to key story
events and drives the player through console input, and prints one
``QA_CHECK`` line per check. A scenario passes only when the process exits
normally, every declared check is reported exactly once as PASS, no undeclared
check appears, the driver prints a matching ``QA_DONE`` record, and no
scenario script raised a Squirrel error.

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
import signal
import subprocess
import sys
import time

import conformance
import stage_portal2_runtime


SCHEMA = "portal2-scenarios/v1"
EVIDENCE_SCHEMA = "portal2-scenario-evidence/v1"
DEFAULT_WORKLOAD = Path("quality/workloads/portal2-wheatley-v1/scenarios.json")
SIMPLE_NAME = re.compile(r"[A-Za-z0-9_]+")
CHECK_NAME = re.compile(r"[A-Za-z0-9_]+(\.[A-Za-z0-9_]+)+")
CHECK_LINE = re.compile(r"^QA_CHECK (\S+) (PASS|FAIL)(?: (.*))?$")
DONE_LINE = re.compile(r"^QA_DONE (\S+) checks=(\d+) failures=(\d+)\s*$")
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
        script = scenario.get("script")
        if not isinstance(script, str) or not script.endswith(".nut") or \
                not (path.parent / script).is_file():
            raise ScenarioError("%s: %s script is missing" % (path, name))
        timeout = scenario.get("timeout_seconds")
        if not isinstance(timeout, (int, float)) or timeout <= 0:
            raise ScenarioError("%s: %s needs a positive timeout" % (path, name))
        checks = scenario.get("required_checks")
        if not isinstance(checks, list) or not checks or len(set(checks)) != len(checks) or \
                not all(isinstance(check, str) and CHECK_NAME.fullmatch(check) for check in checks):
            raise ScenarioError("%s: %s needs unique dotted required checks" % (path, name))
    return workload


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
    failures += ["required check not reported: " + check
                 for check in required if check not in checks]

    done = [DONE_LINE.match(line.strip()) for line in lines]
    done = [match for match in done if match]
    if len(done) != 1:
        failures.append("expected one QA_DONE record, found %d" % len(done))
    else:
        record = done[0]
        reported_failures = sum(1 for value in checks.values() if value["outcome"] != "PASS")
        if record.group(1) != name:
            failures.append("QA_DONE names scenario " + record.group(1))
        elif (int(record.group(2)), int(record.group(3))) != (len(checks), reported_failures):
            failures.append("QA_DONE counts %s/%s differ from the %d checks and %d failures "
                            "in the log" % (record.group(2), record.group(3), len(checks),
                                            reported_failures))

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
    scripts = [workload["driver"]] + [scenario["script"] for scenario in workload["scenarios"]]
    for script in scripts:
        shutil.copy2(source / script, vscripts / script)
    config = Path(runtime) / "portal2/cfg"
    for scenario in workload["scenarios"]:
        # Only a single-line config honours wait; this one only starts the driver.
        (config / ("qa_%s.cfg" % scenario["name"])).write_text(
            "script_execute %s/%s\n" % (SCRIPT_DIRECTORY, Path(scenario["script"]).stem))


def write_fake_zenity(directory):
    """Error() dialogs go through zenity; never let one open or block a run."""
    directory.mkdir(parents=True, exist_ok=True)
    zenity = directory / "zenity"
    zenity.write_text("#!/bin/sh\n"
                      "if [ \"$1\" = --version ]; then echo 4.0.2; exit 0; fi\n"
                      "echo \"zenity $*\" >&2\nexit 1\n")
    zenity.chmod(0o755)


def run_scenario(scenario, runtime, output, start_frames, width, height, tool_directory):
    runtime = Path(runtime).resolve()
    console = runtime / "portal2/console.log"
    console.unlink(missing_ok=True)
    environment = dict(os.environ)
    for variable in ("DISPLAY", "WAYLAND_DISPLAY"):
        environment.pop(variable, None)
    environment.update({
        "SteamAppId": "620", "SteamGameId": "620",
        "LD_LIBRARY_PATH": str(runtime / "bin"),
        "SDL_VIDEODRIVER": "offscreen", "SDL_VIDEO_DRIVER": "offscreen",
        "PATH": str(tool_directory) + os.pathsep + environment.get("PATH", ""),
    })
    command = [str(runtime / "hl2_launcher"), "-game", "portal2", "-multirun", "-novid",
               "-insecure", "-windowed", "-w", str(width), "-h", str(height), "-condebug",
               "+volume", "0", "+map", scenario["map"], "+wait", str(start_frames),
               "+exec", "qa_" + scenario["name"]]
    output.mkdir(parents=True, exist_ok=True)
    started = time.monotonic()
    timed_out = False
    with (output / "stdout.log").open("wb") as stream:
        process = subprocess.Popen(command, cwd=runtime, env=environment, stdout=stream,
                                   stderr=subprocess.STDOUT, start_new_session=True)
        # The driver quits the game after QA_DONE. A driver that failed to
        # load never will, so stop waiting for it as soon as the log says so.
        deadline = started + scenario["timeout_seconds"]
        while process.poll() is None and time.monotonic() < deadline:
            time.sleep(0.5)
            if console.is_file() and DRIVER_LOAD_FAILURE in console.read_text(errors="replace"):
                deadline = min(deadline, time.monotonic() + 2.0)
        if process.poll() is None:
            timed_out = True
            os.killpg(process.pid, signal.SIGTERM)
            try:
                process.wait(timeout=10)
            except subprocess.TimeoutExpired:
                os.killpg(process.pid, signal.SIGKILL)
                process.wait()
    seconds = time.monotonic() - started
    log = console.read_text(errors="replace") if console.is_file() else ""
    (output / "console.log").write_text(log)
    result = evaluate(scenario, log, process.returncode, timed_out)
    result["seconds"] = round(seconds, 1)
    result["command"] = command
    return result


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    root = Path(conformance.repo_root())
    parser.add_argument("--workload", type=Path, default=root / DEFAULT_WORKLOAD)
    parser.add_argument("--steam-root", type=Path, default=Path(os.environ.get(
        "P2_STEAM_ROOT", Path.home() / ".local/share/Steam/steamapps/common/Portal 2")))
    parser.add_argument("--build", type=Path, default=root / "build-p2",
                        help="Waf output configured with --build-games=portal2")
    parser.add_argument("--runtime", type=Path, default=root / "run/runtime-p2-scenarios",
                        help="private staged runtime (created on first use)")
    parser.add_argument("--out", type=Path, required=True, help="new evidence directory")
    parser.add_argument("--scenario", action="append", default=[],
                        help="run only this scenario (repeatable)")
    parser.add_argument("--start-frames", type=int, default=300,
                        help="client frames to wait after the map loads before the driver starts")
    parser.add_argument("--width", type=int, default=1280)
    parser.add_argument("--height", type=int, default=720)
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
        stage_portal2_runtime.stage_content(args.steam_root, args.runtime)
        evidence["installed"] = stage_portal2_runtime.portal_boot.install_build(
            args.build, args.runtime, game="portal2")
        install_scripts(args.workload, workload, args.runtime)
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
                              args.width, args.height, tools)
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
