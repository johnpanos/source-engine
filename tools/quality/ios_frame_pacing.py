#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Frame pacing of a scripted Portal workload on the connected iPhone.

The device form of frame_pacing.py: the same versioned scenario
(quality/workloads/*.json), the same chained cfgs, the same -vkframestats
stream and the same analysis and budget checks. Only staging and launch
differ. The installed app (ios-deploy.sh) reads its extra arguments from
Documents/commandline.txt (launcher_main/ios_main.cpp), so the harness writes
the scenario cfgs and that file into the app's container, launches it
attached to its console, and copies the frame stream back.

    python3 tools/quality/ios_frame_pacing.py --out quality-results/ios-pacing

The app keeps its fixed mobile arguments (fullscreen at the display's native
size, mat_queue_mode 0). commandline.txt is emptied again afterwards.
"""

import argparse
import datetime
import json
from pathlib import Path
import sys

import conformance
import frame_pacing
import ios_device

REPO = Path(__file__).resolve().parents[2]
DEFAULT_SCENARIO = REPO / "quality/workloads/portal-frame-pacing-v1.json"
PROFILE = REPO / "quality/product_profiles/portal-ios-native-vulkan.json"
STATS_NAME = "frame-stats.jsonl"


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--scenario", type=Path, default=DEFAULT_SCENARIO)
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--passes", type=int, help="override the scenario's pass count")
    parser.add_argument("--fps-max", type=int, default=1000)
    parser.add_argument("--timeout", type=float, default=600)
    parser.add_argument("--hitch-ratio", type=float, default=frame_pacing.DEFAULT_HITCH_RATIO)
    parser.add_argument("--hitch-floor-ms", type=float,
                        default=frame_pacing.DEFAULT_HITCH_FLOOR_MS)
    parser.add_argument("--extra-arg", action="append", default=[],
                        help="further engine argument (repeatable)")
    parser.add_argument("--host", default=ios_device.DEFAULT_HOST)
    args = parser.parse_args(argv)

    scenario = frame_pacing.load_scenario(args.scenario)
    passes = args.passes or scenario.get("passes", 1)
    output = args.out.resolve()
    output.mkdir(parents=True, exist_ok=True)
    if (output / "evidence.json").exists():
        raise SystemExit("evidence already exists in %s; use a new output directory" % output)

    ios = json.loads(PROFILE.read_text())["ios"]
    device = ios_device.Device(ios["bundle_id"], host=args.host)
    evidence = {"schema": frame_pacing.EVIDENCE_SCHEMA, "status": "fail", "platform": "ios",
                "started_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
                "source": conformance.source_identity(conformance.repo_root()),
                "device": {"host": device.describe()},
                "scenario": {"path": str(args.scenario.resolve()), "id": scenario["id"],
                             "sha256": frame_pacing.portal_boot.sha256(args.scenario),
                             "passes": passes}}
    failures = []
    try:
        cfgs = frame_pacing.scenario_cfgs(frame_pacing.scenario_commands(scenario, passes))
        for name, text in cfgs.items():
            device.put_text(text, "Documents/portal/cfg/" + name)
        # Relative to the app's working directory, its Documents container.
        engine_args = ["-novid", "-console", "-condebug", "-dev", "-vkframestats", STATS_NAME
                       ] + args.extra_arg + [
                       "+sv_cheats", "1", "+mat_vsync", "0", "+mat_antialias", "0",
                       "+fps_max", str(args.fps_max),
                       "+host_framerate", str(scenario["host_framerate"]),
                       "+volume", "0", "+map", scenario["map"],
                       "+wait", "120", "+exec", next(iter(cfgs))]
        evidence["command"] = engine_args
        # No stale stream from an earlier run is mistaken for this one.
        device.put_text("", "Documents/" + STATS_NAME)
        device.put_text(" ".join(engine_args) + "\n", "Documents/commandline.txt")
        try:
            run = device.launch(ios["executable"], timeout=args.timeout)
        finally:
            device.put_text("", "Documents/commandline.txt")
        (output / "console.log").write_text(run["console"])
        evidence.update(returncode=run["exit_code"], signal=run["signal"],
                        timed_out=run["timed_out"])
        if run["timed_out"]:
            failures.append("product timed out after %.0f s" % args.timeout)
        elif run["exit_code"] != 0:
            failures.append("product exited with %s (signal %s)" % (run["exit_code"],
                                                                     run["signal"]))
        stats_path = output / STATS_NAME
        if not device.get("Documents/" + STATS_NAME, stats_path) or \
                not stats_path.stat().st_size:
            failures.append("no frame stats were written")
        else:
            header, frames, truncated = frame_pacing.read_stats(stats_path)
            evidence["device"]["renderer"] = header
            evidence["frames_recorded"] = len(frames)
            evidence["truncated_lines"] = truncated
            report, analysis_failures = frame_pacing.analyze(frames, passes, args.hitch_ratio,
                                                             args.hitch_floor_ms)
            failures.extend(analysis_failures)
            evidence["analysis"] = report
            budgets = scenario.get("budgets", {})
            evidence["budgets"] = budgets
            evidence["budget_failures"] = frame_pacing.check_budgets(report, budgets)
    except (OSError, ValueError, ios_device.DeviceError) as error:
        failures.append(str(error))
    evidence["failures"] = failures
    if failures:
        evidence["status"] = "fail"
    else:
        evidence["status"] = "over-budget" if evidence.get("budget_failures") else "pass"
    (output / "evidence.json").write_text(json.dumps(evidence, indent=2) + "\n")
    if evidence.get("analysis"):
        frame_pacing.print_report(evidence["analysis"])
    print("iOS frame pacing (%s, %s): %s (%s)" % (
        scenario["id"], evidence["device"]["host"].get("model"), evidence["status"],
        output / "evidence.json"))
    for failure in failures + evidence.get("budget_failures", []):
        print("  " + str(failure))
    return 0 if evidence["status"] == "pass" else 1


if __name__ == "__main__":
    sys.exit(main())
