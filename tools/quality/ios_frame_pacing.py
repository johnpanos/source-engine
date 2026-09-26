#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Frame pacing of a scripted Portal workload on a connected iOS or tvOS device.

The device form of frame_pacing.py: the same versioned scenario
(quality/workloads/*.json), the same chained cfgs, the same -vkframestats
stream and the same analysis and budget checks. Only staging and launch
differ, through ios_device.Device (the one owner of the device plumbing).

The product profile (--profile, default the Portal iOS profile; "extends"
resolved by profile_extends.py) names the platform, the app and its content
directory in the app's data container (content.container_directory:
Documents on iOS, Library/Caches on tvOS). The installed app
(ios-deploy.sh) appends that directory's commandline.txt to its arguments
(launcher_main/ios_main.cpp), so the harness writes the scenario cfgs and
that file into the container, launches the app attached to its console, and
copies the frame stream back. commandline.txt is emptied again afterwards.

    python3 tools/quality/ios_frame_pacing.py --out quality-results/ios-pacing
    python3 tools/quality/ios_frame_pacing.py \\
        --profile quality/product_profiles/portal-tvos-native-vulkan.json \\
        --vsync --budget-row tvos-portal-frame-pacing-60 --out quality-results/tv-pacing

Budgets: the scenario's own (max_hitches on the warm pass), or a row of
quality/budgets/render-v1.json (--budget-row): its vsync limits with
--vsync, its headroom limits without. A row with refresh_hz counts missed
refreshes (frame_pacing.count_missed_refreshes).

The app keeps its fixed mobile arguments and its product defaults (tvOS:
tvos_defaults.cfg). --setting CVAR=VALUE sets further console variables from
a cfg exec'd before the map, off the engine's 512-character command line.
Sound stays on: volume is archived in config.cfg, and a muted run would mute
the next ordinary launch.
"""

import argparse
import datetime
import json
from pathlib import Path
import sys

import conformance
import frame_pacing
import ios_device
from profile_extends import load_profile

REPO = Path(__file__).resolve().parents[2]
DEFAULT_SCENARIO = REPO / "quality/workloads/portal-frame-pacing-v1.json"
DEFAULT_PROFILE = REPO / "quality/product_profiles/portal-ios-native-vulkan.json"
BUDGETS = REPO / "quality/budgets/render-v1.json"
STATS_NAME = "frame-stats.jsonl"
# The scenario's cfgs live in a custom search path, not the game's own cfg.
CFG_DIRECTORY = "portal/custom/frame_pacing/cfg"
SETTINGS_CFG = "frame_pacing_settings.cfg"
# devicectl's platform names.
DEVICE_PLATFORMS = {"ios": "iOS", "tvos": "tvOS"}


def budget_limits(row_id, vsync):
    """The limits of one render-v1.json row for this run's presentation mode."""
    for row in json.loads(BUDGETS.read_text())["rows"]:
        if row["id"] == row_id:
            mode = "vsync" if vsync else "headroom"
            limits = dict(row["modes"][mode])
            limits.pop("meaning", None)
            return dict(limits, row=row_id, mode=mode)
    raise ValueError("no budget row %s in %s" % (row_id, BUDGETS))


def pass_intervals(frames, passes):
    """Each pass's frame intervals (ms), split by the pass marks as analyze() does."""
    marks = {}
    for position, frame in enumerate(frames):
        for mark in filter(None, frame.get("mark", "").split(",")):
            marks.setdefault(mark, position)
    result = []
    for index in range(1, passes + 1):
        begin, end = marks.get("pass_%d_begin" % index), marks.get("pass_%d_end" % index)
        window = frames[begin + 1:end + 1] if begin is not None and end is not None else []
        result.append([frame["interval"] / 1000.0 for frame in window])
    return result


def engine_arguments(scenario, first_cfg, args, settings):
    """Presentation pinned as in frame_pacing.py (no MSAA), except that vsync
    is a choice: off measures the frame's cost, on what a player sees."""
    return (["-novid", "-console", "-condebug", "-dev", "-vkframestats", STATS_NAME]
            + args.extra_arg
            + ["+sv_cheats", "1", "+mat_vsync", "1" if args.vsync else "0", "+mat_antialias", "0",
               "+fps_max", "0" if args.vsync else str(args.fps_max),
               "+host_framerate", str(scenario["host_framerate"])]
            + (["+exec", SETTINGS_CFG] if settings else [])
            + ["+map", scenario["map"], "+wait", "120", "+exec", first_cfg])


def print_gpu_segments(report):
    passes = report["passes"][-1]["summary"].get("gpu_passes") if report["passes"] else None
    if not passes:
        return
    print("  warm pass GPU segments (%d frames):" % passes["frames"])
    for label, row in list(passes["segments"].items())[:16]:
        print("    %6.2f ms mean  %6.2f median  x%-5.2f %5.1f%%  %s"
              % (row["mean_ms"], row["median_ms"], row["per_frame"], 100 * row["share"], label))


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--profile", type=Path, default=DEFAULT_PROFILE)
    parser.add_argument("--scenario", type=Path, default=DEFAULT_SCENARIO)
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--passes", type=int, help="override the scenario's pass count")
    parser.add_argument("--vsync", action="store_true", help="present at the display's rate")
    parser.add_argument("--fps-max", type=int, default=1000, help="without --vsync")
    parser.add_argument("--budget-row", help="row id in quality/budgets/render-v1.json")
    parser.add_argument("--setting", action="append", default=[], metavar="CVAR=VALUE",
                        help="console variable for the run (repeatable)")
    parser.add_argument("--env", action="append", default=[], metavar="KEY=VALUE",
                        help="environment variable for the app process (repeatable)")
    parser.add_argument("--extra-arg", action="append", default=[],
                        help="further engine argument (repeatable)")
    parser.add_argument("--timeout", type=float, default=600)
    parser.add_argument("--hitch-ratio", type=float, default=frame_pacing.DEFAULT_HITCH_RATIO)
    parser.add_argument("--hitch-floor-ms", type=float,
                        default=frame_pacing.DEFAULT_HITCH_FLOOR_MS)
    parser.add_argument("--host", default=ios_device.DEFAULT_HOST)
    parser.add_argument("--device", help="devicectl identifier (default: the connected one)")
    args = parser.parse_args(argv)

    scenario = frame_pacing.load_scenario(args.scenario)
    passes = args.passes or scenario.get("passes", 1)
    output = args.out.resolve()
    output.mkdir(parents=True, exist_ok=True)
    if (output / "evidence.json").exists():
        raise SystemExit("evidence already exists in %s; use a new output directory" % output)

    profile = load_profile(args.profile)
    platform = profile["target"]["os"]
    app = profile[platform]
    content = profile["content"]["container_directory"]
    device = ios_device.Device(app["bundle_id"], host=args.host, identifier=args.device,
                               platform=DEVICE_PLATFORMS[platform])
    settings = [item.split("=", 1) for item in args.setting]
    cfgs = frame_pacing.scenario_cfgs(frame_pacing.scenario_commands(scenario, passes))
    engine_args = engine_arguments(scenario, next(iter(cfgs)), args, bool(settings))
    if settings:
        cfgs[SETTINGS_CFG] = "".join("%s %s\n" % (key, value) for key, value in settings)
    environment = dict(item.split("=", 1) for item in args.env)
    budgets = budget_limits(args.budget_row, args.vsync) if args.budget_row \
        else scenario.get("budgets", {})
    evidence = {"schema": frame_pacing.EVIDENCE_SCHEMA, "status": "fail", "platform": platform,
                "started_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
                "source": conformance.source_identity(conformance.repo_root()),
                "profile": str(args.profile), "device": {"host": device.describe()},
                "scenario": {"path": str(args.scenario.resolve()), "id": scenario["id"],
                             "sha256": frame_pacing.portal_boot.sha256(args.scenario),
                             "passes": passes},
                "vsync": args.vsync, "command": engine_args, "settings": args.setting,
                "environment": args.env, "budgets": budgets}
    failures = []
    try:
        for name, text in cfgs.items():
            device.put_text(text, "%s/%s/%s" % (content, CFG_DIRECTORY, name))
        # No stale stream from an earlier run is mistaken for this one.
        device.put_text("", "%s/%s" % (content, STATS_NAME))
        device.put_text(" ".join(engine_args) + "\n", "%s/commandline.txt" % content)
        try:
            run = device.launch(app["executable"], timeout=args.timeout,
                                environment=environment or None)
        finally:
            device.put_text("", "%s/commandline.txt" % content)
        (output / "console.log").write_text(run["console"])
        evidence.update(returncode=run["exit_code"], signal=run["signal"],
                        timed_out=run["timed_out"])
        if run["timed_out"]:
            failures.append("product timed out after %.0f s" % args.timeout)
        elif run["exit_code"] != 0:
            failures.append("product exited with %s (signal %s)" % (run["exit_code"],
                                                                     run["signal"]))
        stats_path = output / STATS_NAME
        if failures:
            pass  # a run that failed is not analyzed
        elif not device.get("%s/%s" % (content, STATS_NAME), stats_path) or \
                not stats_path.stat().st_size:
            failures.append("no frame stats were written")
        else:
            header, frames, truncated = frame_pacing.read_stats(stats_path)
            evidence["device"]["renderer"] = header
            evidence["frames_recorded"] = len(frames)
            evidence["truncated_lines"] = truncated
            report, analysis_failures = frame_pacing.analyze(frames, passes, args.hitch_ratio,
                                                             args.hitch_floor_ms)
            if budgets.get("refresh_hz"):
                frame_pacing.count_missed_refreshes(report, pass_intervals(frames, passes),
                                                    budgets["refresh_hz"])
            failures.extend(analysis_failures)
            evidence["analysis"] = report
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
        print_gpu_segments(evidence["analysis"])
    print("%s frame pacing (%s, %s): %s (%s)" % (
        DEVICE_PLATFORMS[platform], scenario["id"], evidence["device"]["host"].get("model"),
        evidence["status"], output / "evidence.json"))
    for failure in failures + evidence.get("budget_failures", []):
        print("  " + str(failure))
    return 0 if evidence["status"] == "pass" else 1


if __name__ == "__main__":
    sys.exit(main())
