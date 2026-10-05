#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Run a frame-pacing scenario on an installed iOS, tvOS or Android app and analyze it.

The scenario, its chained cfg files and the analysis are frame_pacing.py's;
only the transport differs. The Mac that deploys the app (ios-deploy.sh,
reached over ssh) copies into the app's data container with
`xcrun devicectl`:

  <content>/portal/custom/frame_pacing/cfg/   the scenario's cfg files
  <content>/commandline.txt                   the run's engine arguments

then launches the app with its console attached until the scenario's `quit`,
and copies <content>/frame-stats.jsonl (-vkframestats) back. The content
root is the app's (launcher_main/apple_main.cpp): Documents on iOS,
Library/Caches on tvOS. commandline.txt is emptied afterwards so the next
normal launch is unaffected.

  tools/quality/frame_pacing_device.py --device F900AE60-... --platform tvos \\
      --out /tmp/fp/tv-base

The app must already be installed with its content (ios-deploy.sh). Evidence
is frame_pacing.py's schema plus the device transport facts.

Android (--platform android) uses adb directly: the content root is the
app's external files directory (launcher_main/android_main.cpp reads
commandline.txt there and runs in it), the app is started with `am start`,
the run ends when the process exits, and the stats are pulled with adb.

  tools/quality/frame_pacing_device.py --platform android --device <serial> \
      --bundle-id org.sourceengine.portal --out /tmp/fp/fold7
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


CONTENT_ROOTS = {"ios": "Documents", "tvos": "Library/Caches",
                 "android": "/sdcard/Android/data/%s/files"}
ANDROID_ACTIVITY = "org.libsdl.app.SDLActivity"
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


def adb(device, arguments, timeout=120, check=True):
    """Run adb against one device; returns its output."""
    command = ["adb"] + (["-s", device] if device else []) + list(arguments)
    try:
        result = subprocess.run(command, capture_output=True, text=True, errors="replace",
                                timeout=timeout)
    except subprocess.TimeoutExpired:
        raise DeviceError("adb timed out after %.0f s: %s" % (timeout, " ".join(arguments)[:120]))
    output = result.stdout + result.stderr
    if check and result.returncode != 0:
        raise DeviceError("adb %s failed (%d): %s" % (" ".join(arguments)[:120], result.returncode,
                                                     output[-800:]))
    return output


def run_android(args, cfgs, arguments, output):
    """Stage, run and collect one Android run; returns the local stats path."""
    import time
    package = args.bundle_id
    content = CONTENT_ROOTS["android"] % package
    if "package:%s" % package not in adb(args.device, ["shell", "pm", "list", "packages", package]).split():
        raise DeviceError("%s is not installed on the device" % package)
    with tempfile.TemporaryDirectory() as local:
        local = Path(local)
        (local / "cfg").mkdir()
        for name, text in cfgs.items():
            (local / "cfg" / name).write_text(text)
        (local / "commandline.txt").write_text(" ".join(arguments) + "\n")
        (local / "empty.txt").write_text("")
        adb(args.device, ["shell", "mkdir", "-p", "%s/%s" % (content, CFG_DIRECTORY)])
        for name in cfgs:
            adb(args.device, ["push", str(local / "cfg" / name), "%s/%s/%s" % (content, CFG_DIRECTORY, name)])
        # An empty stats file first: a run that fails to start must not leave
        # the previous run's stream to be read as its own.
        adb(args.device, ["push", str(local / "empty.txt"), "%s/%s" % (content, STATS_NAME)])
        adb(args.device, ["push", str(local / "commandline.txt"), "%s/commandline.txt" % content])
        # adb creates what it pushes as shell-owned 2770 directories, which the
        # app cannot read; the content it already reads is world-accessible.
        # Only what was pushed: the game writes app-owned files under the
        # custom folder (its sound cache), which a recursive chmod cannot touch.
        pushed = ["%s/portal/custom/frame_pacing" % content, "%s/%s" % (content, CFG_DIRECTORY)]
        pushed += ["%s/%s/%s" % (content, CFG_DIRECTORY, name) for name in cfgs]
        pushed += ["%s/%s" % (content, STATS_NAME), "%s/commandline.txt" % content]
        adb(args.device, ["shell", "chmod", "a+rwX"] + pushed)
        try:
            adb(args.device, ["shell", "am", "force-stop", package])
            adb(args.device, ["logcat", "-c"], check=False)
            adb(args.device, ["shell", "am", "start", "-W", "-n", "%s/%s" % (package, ANDROID_ACTIVITY)])
            deadline = time.monotonic() + args.timeout
            started = False
            while time.monotonic() < deadline:
                running = adb(args.device, ["shell", "pidof", package], check=False).strip() != ""
                started |= running
                if started and not running:
                    break
                time.sleep(2)
            else:
                adb(args.device, ["shell", "am", "force-stop", package], check=False)
                raise DeviceError("the app did not finish the scenario within %.0f s" % args.timeout)
        finally:
            (output / "stdout.log").write_text(adb(args.device, ["logcat", "-d"], timeout=120, check=False))
            # The next ordinary launch must not rerun the scenario.
            adb(args.device, ["push", str(local / "empty.txt"), "%s/commandline.txt" % content], check=False)
    adb(args.device, ["pull", "%s/%s" % (content, STATS_NAME), str(output / STATS_NAME)], timeout=300)
    return output / STATS_NAME


def run_apple(args, cfgs, arguments, output, content, staging):
    """Stage, run and collect one iOS or tvOS run through the Mac; returns the
    local stats path."""
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
        launched = False
        failure = error
    finally:
        # The next ordinary launch must not rerun the scenario.
        remote(args.host, copy_to("%s/empty.txt" % staging, "%s/commandline.txt" % content), 120)
    if not launched:
        raise DeviceError("app run: %s; its stats are not analyzed" % failure)
    remote(args.host, devicectl("copy from --source %s/%s --destination %s/%s --quiet"
                                % (content, STATS_NAME, staging, STATS_NAME),
                                args.device, args.bundle_id), 300)
    subprocess.run(["scp", "-q", "-o", "BatchMode=yes", "%s:%s/%s" % (args.host, staging, STATS_NAME),
                    str(output / STATS_NAME)], check=True, timeout=300)
    return output / STATS_NAME


def budget_limits(row_id, vsync):
    """The limits of one render-v1.json row (frame_pacing owns the lookup)."""
    try:
        return frame_pacing.budget_limits(row_id, vsync, BUDGETS)
    except ValueError as error:
        raise DeviceError(str(error))


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
    parser.add_argument("--device", required=True,
                        help="devicectl device identifier, or the adb serial for android")
    parser.add_argument("--platform", choices=sorted(CONTENT_ROOTS), default="tvos")
    parser.add_argument("--host", default="macvm", help="ssh host of the Mac with the device")
    parser.add_argument("--bundle-id", help="app bundle id or Android package "
                        "(default: com.panos.sourceengine, or org.sourceengine.portal for android)")
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
    if not args.bundle_id:
        args.bundle_id = "org.sourceengine.portal" if args.platform == "android" else "com.panos.sourceengine"

    scenario = frame_pacing.load_scenario(args.scenario)
    passes = args.passes or scenario.get("passes", 1)
    output = args.out.resolve()
    output.mkdir(parents=True, exist_ok=True)
    if (output / "evidence.json").exists():
        parser.error("evidence already exists in %s; use a new output directory" % output)
    content = CONTENT_ROOTS[args.platform]
    if args.platform == "android":
        content = content % args.bundle_id
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
        if args.platform == "android":
            if args.env:
                raise DeviceError("--env is not supported on android")
            stats = run_android(args, cfgs, arguments, output)
        else:
            stats = run_apple(args, cfgs, arguments, output, content, staging)
        header, frames, truncated = frame_pacing.read_stats(stats)
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
