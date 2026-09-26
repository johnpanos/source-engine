#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Run a frame-pacing scenario on an installed iOS or tvOS app and analyze it.

The scenario, its chained cfg files and the analysis are frame_pacing.py's;
only the transport differs. The Mac that deploys the app (ios-deploy.sh,
reached over ssh) copies into the app's data container with
`xcrun devicectl`:

  <content>/portal/custom/frame_pacing/cfg/   the scenario's cfg files
  <content>/commandline.txt                   the run's engine arguments

then launches the app with its console attached until the scenario's `quit`,
and copies <content>/frame-stats.jsonl (-vkframestats) back. The content
root is the app's (launcher_main/ios_main.cpp): Documents on iOS,
Library/Caches on tvOS. commandline.txt is emptied afterwards so the next
normal launch is unaffected.

  tools/quality/frame_pacing_device.py --device F900AE60-... --platform tvos \\
      --out /tmp/fp/tv-base

The app must already be installed with its content (ios-deploy.sh). Evidence
is frame_pacing.py's schema plus the device transport facts.
"""

import argparse
import datetime
import json
from pathlib import Path
import shlex
import subprocess
import sys
import tempfile

import conformance
import frame_pacing


CONTENT_ROOTS = {"ios": "Documents", "tvos": "Library/Caches"}
CFG_DIRECTORY = "portal/custom/frame_pacing/cfg"
STATS_NAME = "frame-stats.jsonl"
BUDGETS = Path(__file__).resolve().parents[2] / "quality/budgets/render-v1.json"


class DeviceError(RuntimeError):
    pass


def remote(host, command, timeout, log=None):
    """Run a shell command on the Mac; returns its output (stdout and stderr)."""
    try:
        result = subprocess.run(["ssh", "-o", "BatchMode=yes", host, command], capture_output=True,
                                text=True, errors="replace", timeout=timeout)
    except subprocess.TimeoutExpired as expired:
        output = (expired.stdout or "") if isinstance(expired.stdout, str) else ""
        if log:
            log.write_text(output)
        raise DeviceError("timed out after %.0f s: %s" % (timeout, command[:120]))
    output = result.stdout + result.stderr
    if log:
        log.write_text(output)
    if result.returncode != 0:
        raise DeviceError("%s failed (%d): %s" % (command[:120], result.returncode, output[-800:]))
    return output


def devicectl(args, device, bundle_id):
    return "xcrun devicectl device %s --device %s --domain-type appDataContainer " \
           "--domain-identifier %s" % (args, shlex.quote(device), shlex.quote(bundle_id))


def budget_limits(row_id, vsync):
    """The limits of one render-v1.json row for this run's presentation mode."""
    budgets = json.loads(BUDGETS.read_text())
    for row in budgets["rows"]:
        if row["id"] == row_id:
            limits = dict(row["modes"]["vsync" if vsync else "headroom"])
            limits.pop("meaning", None)
            return dict(limits, row=row_id, mode="vsync" if vsync else "headroom")
    raise DeviceError("no budget row %s in %s" % (row_id, BUDGETS))


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


SETTINGS_CFG = "frame_pacing_settings.cfg"


def engine_arguments(scenario, first_cfg, vsync, extra, settings=False):
    """The run's arguments, appended by the app root after its own (so after
    the tvOS render defaults). Same pinned presentation as frame_pacing.py,
    except that vsync is a choice: off measures headroom, on measures what a
    player sees at the display's rate. Sound stays on, as a player has it (and
    volume is archived: a muted run would mute the next ordinary launch)."""
    return (["-vkframestats", STATS_NAME, "-dev"] + list(extra) + [
        "+sv_cheats", "1", "+mat_vsync", "1" if vsync else "0", "+fps_max", "0" if vsync else "1000",
        "+host_framerate", str(scenario["host_framerate"])] + (
        ["+exec", SETTINGS_CFG] if settings else []) + [
        "+map", scenario["map"], "+wait", "120", "+exec", first_cfg])


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--device", required=True, help="devicectl device identifier")
    parser.add_argument("--platform", choices=sorted(CONTENT_ROOTS), default="tvos")
    parser.add_argument("--host", default="macvm", help="ssh host of the Mac with the device")
    parser.add_argument("--bundle-id", default="com.panos.sourceengine")
    parser.add_argument("--scenario", type=Path,
                        default=Path(__file__).resolve().parents[2] / "quality/workloads/portal-frame-pacing-v1.json")
    parser.add_argument("--passes", type=int, help="override the scenario's pass count")
    parser.add_argument("--vsync", action="store_true", help="present at the display's rate")
    parser.add_argument("--extra-arg", action="append", default=[], help="another engine argument")
    parser.add_argument("--setting", action="append", default=[], metavar="CVAR=VALUE",
                        help="console variable for the run (repeatable), set by a cfg exec'd before the "
                             "map, which keeps them off the engine's 512-character command line")
    parser.add_argument("--env", action="append", default=[], metavar="KEY=VALUE",
                        help="environment variable for the app process (repeatable)")
    parser.add_argument("--budget-row", help="row id in quality/budgets/render-v1.json to gate the warm pass "
                        "(its vsync limits with --vsync, else its headroom limits)")
    parser.add_argument("--timeout", type=float, default=600)
    parser.add_argument("--hitch-ratio", type=float, default=frame_pacing.DEFAULT_HITCH_RATIO)
    parser.add_argument("--hitch-floor-ms", type=float, default=frame_pacing.DEFAULT_HITCH_FLOOR_MS)
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args(argv)

    scenario = frame_pacing.load_scenario(args.scenario)
    passes = args.passes or scenario.get("passes", 1)
    output = args.out.resolve()
    output.mkdir(parents=True, exist_ok=True)
    if (output / "evidence.json").exists():
        parser.error("evidence already exists in %s; use a new output directory" % output)
    content = CONTENT_ROOTS[args.platform]
    cfgs = frame_pacing.scenario_cfgs(frame_pacing.scenario_commands(scenario, passes))
    settings = [item.split("=", 1) for item in args.setting]
    if settings:
        cfgs = dict(cfgs)
        cfgs[SETTINGS_CFG] = "".join("%s %s\n" % (key, value) for key, value in settings)
    arguments = engine_arguments(scenario, next(iter(cfgs)), args.vsync, args.extra_arg, bool(settings))
    evidence = {"schema": frame_pacing.EVIDENCE_SCHEMA, "status": "fail",
                "started_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
                "source": conformance.source_identity(conformance.repo_root()),
                "transport": {"host": args.host, "device": args.device, "platform": args.platform,
                              "bundle_id": args.bundle_id, "content_root": content},
                "scenario": {"path": str(args.scenario.resolve()), "id": scenario["id"], "passes": passes},
                "vsync": args.vsync, "arguments": arguments, "environment": args.env,
                "settings": args.setting}
    failures = []
    staging = "/tmp/frame-pacing-device-%s" % output.name
    try:
        with tempfile.TemporaryDirectory() as local:
            local = Path(local)
            (local / "cfg").mkdir()
            for name, text in cfgs.items():
                (local / "cfg" / name).write_text(text)
            (local / "commandline.txt").write_text(" ".join(arguments) + "\n")
            (local / "empty.txt").write_text("")
            remote(args.host, "rm -rf %s && mkdir -p %s" % (staging, staging), 60)
            subprocess.run(["scp", "-q", "-r", "-o", "BatchMode=yes", str(local / "cfg"),
                            str(local / "commandline.txt"), str(local / "empty.txt"),
                            "%s:%s/" % (args.host, staging)], check=True, timeout=120)
        copy_to = lambda source, destination: devicectl(
            "copy to --source %s --destination %s --quiet" % (source, shlex.quote(destination)),
            args.device, args.bundle_id)
        # An empty stats file first: a run that fails to start must not leave
        # the previous run's stream to be read as its own.
        remote(args.host, " && ".join([
            copy_to("%s/cfg" % staging, "%s/%s" % (content, CFG_DIRECTORY)),
            copy_to("%s/empty.txt" % staging, "%s/%s" % (content, STATS_NAME)),
            copy_to("%s/commandline.txt" % staging, "%s/commandline.txt" % content)]), 300)
        launched = True
        try:
            environment = dict(item.split("=", 1) for item in args.env)
            remote(args.host, "xcrun devicectl device process launch --device %s --terminate-existing "
                   "--console %s%s" % (shlex.quote(args.device),
                                      "--environment-variables %s " % shlex.quote(json.dumps(environment))
                                      if environment else "", shlex.quote(args.bundle_id)),
                   args.timeout, output / "stdout.log")
        except DeviceError as error:
            failures.append("app run: %s" % error)
            launched = False
        finally:
            # The next ordinary launch must not rerun the scenario.
            remote(args.host, copy_to("%s/empty.txt" % staging, "%s/commandline.txt" % content), 120)
        if not launched:
            raise DeviceError("the app run failed; its stats are not analyzed")
        remote(args.host, devicectl("copy from --source %s/%s --destination %s/%s --quiet"
                                    % (content, STATS_NAME, staging, STATS_NAME),
                                    args.device, args.bundle_id), 300)
        subprocess.run(["scp", "-q", "-o", "BatchMode=yes", "%s:%s/%s" % (args.host, staging, STATS_NAME),
                        str(output / STATS_NAME)], check=True, timeout=300)
        header, frames, truncated = frame_pacing.read_stats(output / STATS_NAME)
        evidence.update(device=header, frames_recorded=len(frames), truncated_lines=truncated)
        report, analysis_failures = frame_pacing.analyze(frames, passes, args.hitch_ratio, args.hitch_floor_ms)
        budgets = budget_limits(args.budget_row, args.vsync) if args.budget_row else {}
        if budgets.get("refresh_hz"):
            frame_pacing.count_missed_refreshes(report, pass_intervals(frames, passes), budgets["refresh_hz"])
        failures.extend(analysis_failures)
        evidence["analysis"] = report
        evidence["budgets"] = budgets if args.budget_row else scenario.get("budgets", {})
        evidence["budget_failures"] = frame_pacing.check_budgets(report, evidence["budgets"])
        frame_pacing.print_report(report)
        passes = report["passes"][-1]["summary"].get("gpu_passes") if report["passes"] else None
        if passes:
            print("  warm pass GPU segments (%d frames):" % passes["frames"])
            for label, row in list(passes["segments"].items())[:16]:
                print("    %6.2f ms mean  %6.2f median  x%-5.2f %5.1f%%  %s"
                      % (row["mean_ms"], row["median_ms"], row["per_frame"], 100 * row["share"], label))
    except (OSError, ValueError, DeviceError, subprocess.SubprocessError) as error:
        failures.append(str(error))
    evidence["failures"] = failures
    if failures:
        evidence["status"] = "fail"
    else:
        evidence["status"] = "over-budget" if evidence.get("budget_failures") else "pass"
    (output / "evidence.json").write_text(json.dumps(evidence, indent=2) + "\n")
    print("%s: %s%s" % (output / "evidence.json", evidence["status"],
                        "".join("\n  " + failure for failure in failures)))
    return 0 if evidence["status"] != "fail" else 1


if __name__ == "__main__":
    sys.exit(main())
