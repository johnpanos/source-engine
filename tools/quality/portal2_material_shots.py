#!/usr/bin/env python3
"""Portal 2 video retail conformance: matched screenshots, this build vs retail.

quality/workloads/portal2-materials-v1 holds fixed views (eye, pitch, yaw) of
Portal 2 materials, shaders, material proxies, textures and particles in
retail maps. This tool shoots them on this build and on the retail Portal 2
binary at 1024x768, and judges this build's shots against the retail
reference:

  capture --side build   runs the views on a Waf tree through
                         tools/quality/portal2_scenarios.py (SDL offscreen,
                         native Vulkan); keeps each shot as PNG, the console
                         log and the native backend's per-screenshot census
  capture --side retail  runs the same scripts on retail portal2_linux in an
                         isolated headless compositor (mutter), through a
                         mapspawn.nut hook (retail ignores +wait on the
                         command line). Retail needs a running Steam client,
                         so the session starts one inside the compositor and
                         shuts it down afterwards.
  record                 writes the reference (quality/workloads/.../
                         reference.json) from a retail capture: per shot a
                         coarse tile grid of mean colours and the statistics
                         of each declared region. Numbers only; no retail
                         pixels are stored in the repository.
  check                  judges a build capture against the reference and
                         prints one checks-v1 record
  suite                  capture --side build, then check (the manifest row)
  self-test              the comparator on synthetic frames, with seeded
                         faults (no game, no content)

Checks (quality/workloads/portal2-materials-v1/checks.json):

  <scenario>.run                 the scenario ran and every view was placed
  <scenario>.<shot>.tiles        enough of the frame's tiles match retail's
                                 mean colour
  <scenario>.<shot>.<region>     a declared region (a feature: the cables, a
                                 fizzler, goo) matches retail's mean colour and
                                 contrast; each names the defect it pins
  <scenario>.shaders             no material uses a shader this build lacks,
                                 beyond the declared exemptions
  <scenario>.proxies             no material names a proxy this build lacks
  <scenario>.<shot>.native_drops the native Vulkan backend dropped no draw of
                                 a non-exempt material while the view showed

Retail content and binaries are not in the repository: the suite needs a
local Portal 2 installation (SOURCE_PORTAL2_STEAM_ROOT) and is optional in
the conformance manifest, never certified without it.
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

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import conformance  # noqa: E402
import conformance_result  # noqa: E402
import portal2_scenarios  # noqa: E402
import stage_portal2_runtime  # noqa: E402


CAPTURE_SCHEMA = "portal2-material-capture/v1"
REFERENCE_SCHEMA = "portal2-material-reference/v1"
CHECKS_SCHEMA = "portal2-material-checks/v1"
ROOT = Path(conformance.repo_root())
WORKLOAD = ROOT / "quality/workloads/portal2-materials-v1"
DEFAULT_STEAM_ROOT = Path.home() / ".local/share/Steam/steamapps/common/Portal 2"
WIDTH, HEIGHT = 1024, 768
SHOT_LINE = re.compile(r"^QA_SHOT (\S+) (\S+)\s*$")
UNKNOWN_SHADER = re.compile(r'Material "([^"]+)" uses unknown shader "([^"]+)"')
MISSING_PROXY = re.compile(r'Error: Material "?([^"]*?)"? ?: proxy "([^"]+)" not found')
# The native backend's census, printed at each screenshot (ReadPixels):
# cumulative dropped-draw counts per material.
CENSUS_DROPPED = re.compile(r"^\[vulkan\]\s+dropped material draws=(\d+)\s+(\S+)")
CENSUS_START = re.compile(r"^\[vulkan\] primitive ")
# Retail runs through this hook; it replaces nothing but appends the driver.
RETAIL_HOOK_MARK = "// portal2_material_shots hook"


class ShotError(Exception):
    pass


def now_iso():
    return datetime.datetime.now(datetime.timezone.utc).isoformat()


# ---------------------------------------------------------------------------
# Image statistics (numpy/Pillow imported lazily: `--help` and manifest
# loading need neither).
# ---------------------------------------------------------------------------

def load_rgb(path):
    import numpy as np
    from PIL import Image
    image = Image.open(path).convert("RGB")
    if image.size != (WIDTH, HEIGHT):
        raise ShotError("%s is %dx%d, not %dx%d" % (path, image.size[0], image.size[1],
                                                    WIDTH, HEIGHT))
    return np.asarray(image)


def tile_means(image, columns, rows):
    """Mean RGB per tile of a columns x rows grid, as nested lists (rows)."""
    height, width = image.shape[:2]
    th, tw = height // rows, width // columns
    grid = image[:rows * th, :columns * tw].reshape(rows, th, columns, tw, 3).astype(float)
    return [[[round(float(v), 2) for v in grid[r, :, c, :].reshape(-1, 3).mean(axis=0)]
             for c in range(columns)] for r in range(rows)]


def region_stats(image, rect):
    """Mean RGB, luminance standard deviation of a rect [x0, y0, x1, y1]."""
    x0, y0, x1, y1 = rect
    patch = image[y0:y1, x0:x1].reshape(-1, 3).astype(float)
    luma = patch @ [0.2126, 0.7152, 0.0722]
    return {"mean": [round(float(v), 2) for v in patch.mean(axis=0)],
            "std": round(float(luma.std()), 2)}


def compare_tiles(ours, reference, tolerance):
    """Fraction of tiles whose mean colours differ by at most `tolerance` per channel."""
    total = matched = 0
    worst = (0.0, None)
    for r, (row_a, row_b) in enumerate(zip(ours, reference)):
        for c, (a, b) in enumerate(zip(row_a, row_b)):
            difference = max(abs(x - y) for x, y in zip(a, b))
            total += 1
            if difference <= tolerance:
                matched += 1
            if difference > worst[0]:
                worst = (difference, (c, r))
    return (matched / total if total else 0.0), worst


def compare_region(ours, reference, spec):
    """(ok, detail) for one region: mean colour and contrast against retail."""
    tolerance = spec.get("mean_tolerance", 20.0)
    difference = max(abs(a - b) for a, b in zip(ours["mean"], reference["mean"]))
    ok = difference <= tolerance
    detail = "mean %s vs retail %s (max channel difference %.1f, allowed %.1f)" % (
        fmt_rgb(ours["mean"]), fmt_rgb(reference["mean"]), difference, tolerance)
    if "std_ratio" in spec:
        low, high = spec["std_ratio"]
        ratio = (ours["std"] + 1.0) / (reference["std"] + 1.0)
        ok = ok and low <= ratio <= high
        detail += "; contrast %.1f vs retail %.1f (ratio %.2f, allowed %.2f-%.2f)" % (
            ours["std"], reference["std"], ratio, low, high)
    return ok, detail


def fmt_rgb(values):
    return "(%s)" % " ".join("%.0f" % v for v in values)


# ---------------------------------------------------------------------------
# Workload
# ---------------------------------------------------------------------------

def load_checks(workload_dir):
    path = Path(workload_dir) / "checks.json"
    spec = json.loads(path.read_text())
    if spec.get("schema") != CHECKS_SCHEMA:
        raise ShotError("%s: schema must be %s" % (path, CHECKS_SCHEMA))
    for scenario, shots in spec.get("shots", {}).items():
        for shot, shot_spec in shots.items():
            for name, region in shot_spec.get("regions", {}).items():
                rect = region.get("rect")
                if not (isinstance(rect, list) and len(rect) == 4 and
                        0 <= rect[0] < rect[2] <= WIDTH and 0 <= rect[1] < rect[3] <= HEIGHT):
                    raise ShotError("%s: %s.%s.%s needs a rect inside the frame"
                                    % (path, scenario, shot, name))
                if not region.get("pins"):
                    raise ShotError("%s: %s.%s.%s must name the defect it pins"
                                    % (path, scenario, shot, name))
    return spec


def scenario_views(workload):
    """Scenario name -> the view names its required checks declare, in order."""
    return {s["name"]: [c.split(".", 1)[1] for c in s["required_checks"]
                        if c.startswith("view.")] for s in workload["scenarios"]}


# ---------------------------------------------------------------------------
# Capture: shared parts
# ---------------------------------------------------------------------------

def shot_names(log, scenario):
    names = [m.group(2) for m in map(SHOT_LINE.match, log.splitlines())
             if m and m.group(1) == scenario]
    if len(set(names)) != len(names):
        raise ShotError("duplicate QA_SHOT names: %s" % names)
    return names


def collect_shots(screenshots, started, names, destination):
    """Pairs screenshots written since `started` with the QA_SHOT names."""
    files = sorted(p for p in Path(screenshots).glob("*.tga") if p.stat().st_mtime >= started)
    if len(files) != len(names):
        raise ShotError("%d screenshots written, %d QA_SHOT lines" % (len(files), len(names)))
    from PIL import Image
    destination.mkdir(parents=True, exist_ok=True)
    shots = {}
    for name, path in zip(names, files):
        image = Image.open(path).convert("RGB")
        out = destination / (name + ".png")
        image.save(out)
        shots[name] = {"file": out.name, "size": list(image.size)}
    return shots


def console_issues(log):
    shaders, proxies = {}, {}
    for match in UNKNOWN_SHADER.finditer(log):
        shaders.setdefault(match.group(2), set()).add(match.group(1))
    for match in MISSING_PROXY.finditer(log):
        proxies.setdefault(match.group(2), set()).add(match.group(1))
    return ({k: sorted(v) for k, v in sorted(shaders.items())},
            {k: sorted(v) for k, v in sorted(proxies.items())})


def census_by_shot(stdout, names):
    """Per shot, the draws the native backend dropped per material since the last shot."""
    blocks, current = [], None
    for line in stdout.splitlines():
        if CENSUS_START.match(line):
            current = {}
            blocks.append(current)
            continue
        match = CENSUS_DROPPED.match(line)
        if match and current is not None:
            current[match.group(2)] = int(match.group(1))
    # One census per screenshot; the backend prints one more at shutdown.
    if len(blocks) < len(names):
        return None
    result, previous = {}, {}
    for name, block in zip(names, blocks):
        result[name] = {material: count - previous.get(material, 0)
                        for material, count in block.items()
                        if count - previous.get(material, 0) > 0}
        previous = block
    return result


def write_capture(out, capture):
    (Path(out) / "capture.json").write_text(json.dumps(capture, indent=2) + "\n")


# ---------------------------------------------------------------------------
# Capture: this build
# ---------------------------------------------------------------------------

def capture_build(args, workload_path, workload, scenarios):
    out = Path(args.out).resolve()
    capture = {"schema": CAPTURE_SCHEMA, "side": "build", "status": "incomplete",
               "started_utc": now_iso(), "source": conformance.source_identity(str(ROOT)),
               "build": str(args.build), "extra_args": list(args.extra_arg), "scenarios": {}}
    stage_portal2_runtime.stage_content(args.steam_root, args.runtime)
    capture["installed"] = stage_portal2_runtime.portal_boot.install_build(
        args.build, args.runtime, game="portal2")
    portal2_scenarios.install_scripts(workload_path, workload, args.runtime)
    tools = out / "tools"
    portal2_scenarios.write_fake_zenity(tools)
    screenshots = Path(args.runtime) / "portal2/screenshots"
    for scenario in scenarios:
        name = scenario["name"]
        print("== build %s" % name, flush=True)
        started = time.time()
        result = portal2_scenarios.run_scenario(
            scenario, args.runtime, out / name, args.start_frames, WIDTH, HEIGHT, tools,
            extra_args=args.extra_arg)
        record = finish_scenario(out / name, name, result, screenshots, started,
                                 (out / name / "stdout.log").read_text(errors="replace"))
        capture["scenarios"][name] = record
        write_capture(out, capture)
        print("  %s: %d shots%s" % (record["status"], len(record.get("shots", {})),
                                    "" if not record.get("error") else " (%s)" % record["error"]),
              flush=True)
    capture["status"] = "complete"
    capture["finished_utc"] = now_iso()
    write_capture(out, capture)
    return capture


def finish_scenario(directory, name, result, screenshots, started, stdout):
    log = (directory / "console.log").read_text(errors="replace")
    shaders, proxies = console_issues(log)
    record = {"status": result["status"], "failures": result["failures"],
              "checks": result["checks"], "seconds": result.get("seconds"),
              "unknown_shaders": shaders, "missing_proxies": proxies}
    try:
        names = shot_names(log, name)
        record["shots"] = collect_shots(screenshots, started, names, directory / "shots")
        record["census"] = census_by_shot(stdout, names)
    except (OSError, ShotError) as error:
        record["error"] = str(error)
    return record


# ---------------------------------------------------------------------------
# Capture: retail
# ---------------------------------------------------------------------------

def make_retail_mirror(steam_root, mirror):
    """A symlink mirror of the install whose portal2/cfg and scripts are private copies.

    The executable is copied, not linked: the engine finds its base directory
    from the executable's resolved path.
    """
    steam_root, mirror = Path(steam_root), Path(mirror)
    mirror.mkdir(parents=True, exist_ok=True)
    for entry in steam_root.iterdir():
        target = mirror / entry.name
        if entry.name in ("portal2", "portal2_linux"):
            continue
        if not target.exists() and not target.is_symlink():
            target.symlink_to(entry)
    if not (mirror / "portal2_linux").is_file():
        shutil.copy2(steam_root / "portal2_linux", mirror / "portal2_linux")
    (mirror / "steam_appid.txt").write_text("620\n")
    game = mirror / "portal2"
    game.mkdir(exist_ok=True)
    for entry in (steam_root / "portal2").iterdir():
        target = game / entry.name
        if entry.name in ("cfg", "scripts", "screenshots", "console.log"):
            continue
        if not target.exists() and not target.is_symlink():
            target.symlink_to(entry)
    for private in ("cfg", "scripts"):
        if not (game / private).exists():
            shutil.copytree(steam_root / "portal2" / private, game / private, symlinks=True)
    # vguimatsurface.so needs these two 32-bit libraries, which the Steam
    # runtime ships and the host does not.
    shims = mirror / "qa_shims"
    shims.mkdir(exist_ok=True)
    runtime = Path.home() / ".local/share/Steam/ubuntu12_32/steam-runtime/lib/i386-linux-gnu"
    for name in ("libbz2.so.1.0", "libpng12.so.0"):
        if not (shims / name).exists() and (runtime / name).exists():
            (shims / name).symlink_to(os.path.realpath(runtime / name))
    return mirror


def install_retail_scripts(workload_path, workload, mirror):
    vscripts = Path(mirror) / "portal2/scripts/vscripts"
    qa = vscripts / portal2_scenarios.SCRIPT_DIRECTORY
    if qa.exists():
        shutil.rmtree(qa)
    qa.mkdir(parents=True)
    source = Path(workload_path).parent
    for script in [workload["driver"]] + workload.get("includes", []) + \
            [scenario["script"] for scenario in workload["scenarios"]]:
        shutil.copy2(source / script, qa / Path(script).name)
    original = vscripts / "mapspawn.nut.retail"
    if not original.exists():
        shutil.copy2(vscripts / "mapspawn.nut", original)


def set_retail_hook(mirror, scenario):
    """mapspawn.nut = retail's own, then (server VM only) run the scenario's script."""
    vscripts = Path(mirror) / "portal2/scripts/vscripts"
    script = "%s/%s" % (portal2_scenarios.SCRIPT_DIRECTORY, Path(scenario["script"]).stem)
    hook = (
        "\n%s\n"
        "// The client VM runs mapspawn too and has no EntFire; the scenario runs\n"
        "// on the server, once the entity I/O queue is live.\n"
        "try\n{\n"
        "\tEntFire( \"worldspawn\", \"RunScriptCode\", "
        "\"DoIncludeScript( \\\"%s\\\", getroottable() )\", 1.0 )\n"
        "}\ncatch ( error )\n{\n}\n" % (RETAIL_HOOK_MARK, script))
    (vscripts / "mapspawn.nut").write_text(
        (vscripts / "mapspawn.nut.retail").read_text(errors="replace") + hook)


def capture_retail(args, workload_path, workload, scenarios):
    """Re-runs this tool inside an isolated headless compositor."""
    out = Path(args.out).resolve()
    mirror = make_retail_mirror(args.steam_root, args.mirror)
    install_retail_scripts(workload_path, workload, mirror)
    inner = [sys.executable, os.path.abspath(__file__), "_retail-session", "--out", str(out),
             "--mirror", str(mirror), "--workload", str(workload_path)]
    for scenario in scenarios:
        inner += ["--scenario", scenario["name"]]
    config = out / "compositor-config"
    config.mkdir(parents=True, exist_ok=True)
    environment = dict(os.environ, XDG_CONFIG_HOME=str(config))
    for variable in ("DISPLAY", "WAYLAND_DISPLAY"):
        environment.pop(variable, None)
    command = ["dbus-run-session", "--", "mutter", "--headless", "--wayland",
               "--virtual-monitor", "1920x1080@60",
               "--wayland-display", "p2-material-shots-%d" % os.getpid(), "--"] + inner
    helpers_before = steam_helpers()
    try:
        with (out / "compositor.log").open("wb") as log:
            process = subprocess.run(command, env=environment, stdout=log,
                                     stderr=subprocess.STDOUT, timeout=args.session_timeout)
    finally:
        # `steam -shutdown` leaves the runtime's launcher service behind.
        for pid in steam_helpers() - helpers_before:
            try:
                os.kill(pid, signal.SIGTERM)
            except OSError:
                pass
    capture_path = out / "capture.json"
    if not capture_path.is_file():
        raise ShotError("the retail session wrote no capture (exit %d); see %s"
                        % (process.returncode, out / "compositor.log"))
    return json.loads(capture_path.read_text())


def steam_helpers():
    found = subprocess.run(["pgrep", "-f", "steam-runtime-launcher-service"],
                           capture_output=True, text=True).stdout.split()
    return {int(pid) for pid in found if int(pid) != os.getpid()}


def steam_running():
    return subprocess.run(["pgrep", "-x", "steam"], capture_output=True).returncode == 0


def retail_session(args):
    """Inside the compositor: Steam up, each scenario on retail, Steam down."""
    out = Path(args.out).resolve()
    mirror = Path(args.mirror)
    workload = portal2_scenarios.load_workload(args.workload)
    scenarios = [s for s in workload["scenarios"] if s["name"] in args.scenario]
    capture = {"schema": CAPTURE_SCHEMA, "side": "retail", "status": "incomplete",
               "started_utc": now_iso(), "mirror": str(mirror), "scenarios": {}}
    started_steam = False
    if not steam_running():
        # Retail refuses to start without a Steam client ("Steam is not running").
        with (out / "steam.log").open("wb") as log:
            subprocess.Popen(["steam", "-silent", "-nochatui", "-nofriendsui"],
                             stdout=log, stderr=subprocess.STDOUT, start_new_session=True)
        started_steam = True
        deadline = time.monotonic() + 180
        while time.monotonic() < deadline and not (Path.home() / ".steam/steam.pid").exists():
            time.sleep(1)
        time.sleep(args.steam_settle)
    environment = dict(os.environ)
    environment.update({
        "LD_LIBRARY_PATH": "%s:%s" % (mirror / "bin/linux32", mirror / "qa_shims"),
        "SDL_VIDEODRIVER": "x11", "SteamAppId": "620", "SteamGameId": "620",
    })
    console = mirror / "portal2/console.log"
    screenshots = mirror / "portal2/screenshots"
    try:
        for scenario in scenarios:
            name = scenario["name"]
            set_retail_hook(mirror, scenario)
            console.unlink(missing_ok=True)
            directory = out / name
            directory.mkdir(parents=True, exist_ok=True)
            command = ["./portal2_linux", "-game", "portal2", "-novid", "-windowed",
                       "-w", str(WIDTH), "-h", str(HEIGHT), "-condebug", "+volume", "0",
                       "+map", scenario["map"]]
            started = time.time()
            timed_out = False
            with (directory / "stdout.log").open("wb") as stream:
                process = subprocess.Popen(command, cwd=mirror, env=environment, stdout=stream,
                                           stderr=subprocess.STDOUT, start_new_session=True)
                try:
                    process.wait(timeout=scenario["timeout_seconds"])
                except subprocess.TimeoutExpired:
                    timed_out = True
                    os.killpg(process.pid, signal.SIGKILL)
                    process.wait()
            log = console.read_text(errors="replace") if console.is_file() else ""
            (directory / "console.log").write_text(log)
            result = portal2_scenarios.evaluate(scenario, log, process.returncode, timed_out)
            result["seconds"] = round(time.time() - started, 1)
            record = finish_scenario(directory, name, result, screenshots, started, "")
            record.pop("census", None)
            capture["scenarios"][name] = record
            write_capture(out, capture)
    finally:
        if started_steam:
            subprocess.run(["steam", "-shutdown"], capture_output=True, timeout=60)
            time.sleep(8)
    capture["status"] = "complete"
    capture["finished_utc"] = now_iso()
    write_capture(out, capture)
    return 0


# ---------------------------------------------------------------------------
# Reference and check
# ---------------------------------------------------------------------------

def record_reference(args, checks):
    retail = Path(args.retail).resolve()
    capture = json.loads((retail / "capture.json").read_text())
    if capture.get("side") != "retail" or capture.get("status") != "complete":
        raise ShotError("%s is not a complete retail capture" % retail)
    grid = checks["tiles"]
    reference = {"schema": REFERENCE_SCHEMA, "recorded_utc": now_iso(),
                 "retail_capture_started_utc": capture["started_utc"],
                 "tiles": {"columns": grid["columns"], "rows": grid["rows"]},
                 "scenarios": {}}
    for scenario, shots in checks["shots"].items():
        record = capture["scenarios"].get(scenario)
        if not record or record["status"] != "pass":
            raise ShotError("retail scenario %s did not pass" % scenario)
        entry = {"unknown_shaders": record["unknown_shaders"],
                 "missing_proxies": record["missing_proxies"], "shots": {}}
        for shot, spec in shots.items():
            image = load_rgb(retail / scenario / "shots" / (shot + ".png"))
            entry["shots"][shot] = {
                "tiles": tile_means(image, grid["columns"], grid["rows"]),
                "regions": {name: region_stats(image, region["rect"])
                            for name, region in spec.get("regions", {}).items()},
            }
        reference["scenarios"][scenario] = entry
    Path(args.reference).write_text(json.dumps(reference, indent=1) + "\n")
    print("wrote %s" % args.reference)
    return 0


def judge(capture_dir, capture, checks, reference, report):
    """Runs every check; `report(name, ok, detail)` counts each one."""
    grid = checks["tiles"]
    exempt_shaders = checks.get("exempt_unknown_shaders", {})
    exempt_drops = checks.get("exempt_native_drops", {})
    for scenario, shots in sorted(checks["shots"].items()):
        record = capture["scenarios"].get(scenario)
        ref = reference["scenarios"].get(scenario)
        if record is None or ref is None:
            report("%s.run" % scenario, False, "not captured" if record is None
                   else "no retail reference")
            continue
        ran = record["status"] == "pass" and not record.get("error")
        report("%s.run" % scenario, ran, "; ".join(record["failures"] + (
            [record["error"]] if record.get("error") else [])) or "%d shots" % len(record["shots"]))
        if not ran:
            continue
        ours_shaders = set(record["unknown_shaders"]) - set(ref["unknown_shaders"]) - \
            set(exempt_shaders)
        report("%s.shaders" % scenario, not ours_shaders, ", ".join(
            "%s (%s)" % (s, ", ".join(record["unknown_shaders"][s][:3])) for s in
            sorted(ours_shaders)) or "no unknown shader beyond retail's and the exemptions")
        ours_proxies = set(record["missing_proxies"]) - set(ref["missing_proxies"])
        report("%s.proxies" % scenario, not ours_proxies, ", ".join(
            "%s (%s)" % (p, ", ".join(record["missing_proxies"][p][:3])) for p in
            sorted(ours_proxies)) or "every proxy found")
        for shot, spec in sorted(shots.items()):
            prefix = "%s.%s" % (scenario, shot)
            if shot not in record["shots"] or shot not in ref["shots"]:
                report(prefix + ".tiles", False, "shot missing")
                continue
            image = load_rgb(Path(capture_dir) / scenario / "shots" / (shot + ".png"))
            tolerance = spec.get("tile_tolerance", grid["tolerance"])
            minimum = spec.get("tile_fraction", grid["min_fraction"])
            fraction, worst = compare_tiles(tile_means(image, grid["columns"], grid["rows"]),
                                            ref["shots"][shot]["tiles"], tolerance)
            report(prefix + ".tiles", fraction >= minimum,
                   "%.1f%% of tiles within %d of retail (need %.0f%%); worst %.0f at tile %s"
                   % (100 * fraction, tolerance, 100 * minimum, worst[0], worst[1]))
            for name, region in sorted(spec.get("regions", {}).items()):
                ok, detail = compare_region(region_stats(image, region["rect"]),
                                            ref["shots"][shot]["regions"][name], region)
                report("%s.%s" % (prefix, name), ok, detail + "; pins: " + region["pins"])
            census = (record.get("census") or {}).get(shot)
            if census is None:
                report(prefix + ".native_drops", False, "no native census for this shot")
            else:
                drops = {m: n for m, n in census.items() if m not in exempt_drops}
                report(prefix + ".native_drops", not drops, ", ".join(
                    "%s x%d" % (m, n) for m, n in sorted(drops.items())) or "none")


def check(args, checks):
    capture_dir = Path(args.capture).resolve()
    capture = json.loads((capture_dir / "capture.json").read_text())
    if capture.get("side") != "build":
        raise ShotError("%s is not a build capture" % capture_dir)
    if args.retail:
        # Judge against a fresh retail capture instead of the recorded numbers.
        scratch = argparse.Namespace(retail=args.retail, reference=str(capture_dir /
                                                                          "retail-reference.json"))
        record_reference(scratch, checks)
        reference = json.loads(Path(scratch.reference).read_text())
    else:
        reference = json.loads(Path(args.reference).read_text())
    if reference.get("schema") != REFERENCE_SCHEMA:
        raise ShotError("reference schema must be %s" % REFERENCE_SCHEMA)
    results = Checks()
    judge(capture_dir, capture, checks, reference, results.report)
    (capture_dir / "checks.json").write_text(json.dumps(results.records, indent=2) + "\n")
    return results.finish()


class Checks:
    def __init__(self):
        self.counter = conformance_result.Checks()
        self.records = []

    def report(self, name, ok, detail):
        print("%-4s %s: %s" % ("PASS" if ok else "FAIL", name, detail), flush=True)
        self.records.append({"check": name, "ok": bool(ok), "detail": detail})
        self.counter.checks += 1
        if not ok:
            self.counter.failures += 1

    def finish(self):
        return self.counter.report()


# ---------------------------------------------------------------------------
# Self-test: the comparator on synthetic frames
# ---------------------------------------------------------------------------

def self_test(args):
    """A synthetic retail frame and build frames that must pass or fail.

    The frame is a gradient with a textured 'feature' region. A build frame
    with per-pixel noise and a one-pixel shift must pass; the seeded faults
    must fail the check they name.
    """
    import numpy as np
    rng = np.random.default_rng(20260925)
    y, x = np.mgrid[0:HEIGHT, 0:WIDTH]
    retail = np.stack([x * 200 // WIDTH + 20, y * 180 // HEIGHT + 30,
                       (x + y) * 100 // (WIDTH + HEIGHT) + 60], axis=-1).astype(np.uint8)
    feature = [400, 300, 600, 420]
    stripes = ((x[300:420, 400:600] // 6) % 2) * 120
    retail[300:420, 400:600] = np.stack([stripes + 60] * 3, axis=-1).astype(np.uint8)
    ours = np.roll(retail, 1, axis=1).astype(int) + rng.integers(-6, 7, retail.shape)
    ours = np.clip(ours, 0, 255).astype(np.uint8)
    if args.seed_fault == "missing-feature":
        ours[300:420, 400:600] = ours[290:300, 400:600].mean(axis=(0, 1)).astype(np.uint8)
    elif args.seed_fault == "flat-feature":
        # Right mean colour, no detail: the contrast clause must catch it.
        ours[300:420, 400:600] = retail[300:420, 400:600].reshape(-1, 3).mean(axis=0)
    elif args.seed_fault == "dark-frame":
        ours = (ours * 0.6).astype(np.uint8)
    spec = {"tiles": {"columns": 16, "rows": 12, "tolerance": 16, "min_fraction": 0.9}}
    region = {"rect": feature, "mean_tolerance": 20.0, "std_ratio": [0.6, 1.6],
              "pins": "self-test feature"}
    results = Checks()
    fraction, worst = compare_tiles(tile_means(ours, 16, 12), tile_means(retail, 16, 12), 16)
    results.report("selftest.tiles", fraction >= 0.9, "%.1f%% of tiles match; worst %.0f"
                   % (100 * fraction, worst[0]))
    ok, detail = compare_region(region_stats(ours, feature), region_stats(retail, feature),
                                region)
    results.report("selftest.feature", ok, detail)
    census = census_by_shot("[vulkan] primitive TRIANGLES draws=1\n"
                            "[vulkan]   dropped material draws=5 dev/motion_blur\n"
                            "[vulkan] primitive TRIANGLES draws=2\n"
                            "[vulkan]   dropped material draws=5 dev/motion_blur\n"
                            "[vulkan]   dropped material draws=3 cable/cable\n", ["a", "b"])
    expected = {"a": {"dev/motion_blur": 5}, "b": {"cable/cable": 3}}
    if args.seed_fault == "census":
        expected["b"]["dev/motion_blur"] = 5
    results.report("selftest.census", census == expected, "per-shot drops %s" % census)
    shaders, proxies = console_issues(
        'Error: Material "cable/cable" uses unknown shader "SplineRope"\n'
        'Error: Material "dev/bloomadd" : proxy "BloomAdd" not found!\n')
    results.report("selftest.console", shaders == {"SplineRope": ["cable/cable"]} and
                   proxies == {"BloomAdd": ["dev/bloomadd"]},
                   "unknown shaders %s, missing proxies %s" % (shaders, proxies))
    del spec
    return results.finish()


# ---------------------------------------------------------------------------
# Command line
# ---------------------------------------------------------------------------

def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = parser.add_subparsers(dest="command", required=True)

    def common(p):
        p.add_argument("--workload", type=Path, default=WORKLOAD / "scenarios.json")
        p.add_argument("--scenario", action="append", default=[],
                       help="only this scenario (repeatable)")

    def build_args(p):
        p.add_argument("--build", type=Path, default=ROOT / "build-p2",
                       help="Waf output configured with --build-games=portal2")
        p.add_argument("--runtime", type=Path, default=ROOT / "run/runtime-p2-material-shots",
                       help="private staged runtime (created on first use)")
        p.add_argument("--start-frames", type=int, default=300)
        p.add_argument("--extra-arg", action="append", default=[],
                       help="extra engine argument before +map (repeatable); a seeded "
                            "defect for a negative control, e.g. --extra-arg='+r_drawropes 0'")

    steam = Path(os.environ.get("SOURCE_PORTAL2_STEAM_ROOT") or
                 os.environ.get("P2_STEAM_ROOT") or DEFAULT_STEAM_ROOT)
    p = sub.add_parser("capture", help="shoot the views on this build or on retail")
    common(p)
    build_args(p)
    p.add_argument("--side", choices=("build", "retail"), required=True)
    p.add_argument("--out", type=Path, required=True, help="new capture directory")
    p.add_argument("--steam-root", type=Path, default=steam)
    p.add_argument("--mirror", type=Path, default=ROOT / "run/retail-p2-material-shots",
                   help="retail symlink mirror (created on first use)")
    p.add_argument("--session-timeout", type=int, default=3600)

    p = sub.add_parser("_retail-session", help=argparse.SUPPRESS)
    common(p)
    p.add_argument("--out", type=Path, required=True)
    p.add_argument("--mirror", type=Path, required=True)
    p.add_argument("--steam-settle", type=int, default=30)

    p = sub.add_parser("record", help="write the reference from a retail capture")
    p.add_argument("--retail", type=Path, required=True)
    p.add_argument("--reference", type=Path, default=WORKLOAD / "reference.json")

    p = sub.add_parser("check", help="judge a build capture against retail")
    p.add_argument("--capture", type=Path, required=True)
    p.add_argument("--reference", type=Path, default=WORKLOAD / "reference.json")
    p.add_argument("--retail", type=Path, help="judge against this retail capture instead")

    p = sub.add_parser("suite", help="capture this build, then check (manifest row)")
    common(p)
    build_args(p)
    p.add_argument("--out", type=Path, help="capture directory (default: $CONFORMANCE_OUT or "
                                            "quality-results/portal2-material-shots-<time>)")
    p.add_argument("--steam-root", type=Path, default=steam)
    p.add_argument("--reference", type=Path, default=WORKLOAD / "reference.json")

    p = sub.add_parser("self-test", help="the comparator on synthetic frames")
    p.add_argument("--seed-fault", choices=("missing-feature", "flat-feature", "dark-frame",
                                            "census"))

    args = parser.parse_args(argv)
    try:
        if args.command == "self-test":
            return self_test(args)
        if args.command == "_retail-session":
            return retail_session(args)
        checks = load_checks(WORKLOAD)
        if args.command == "record":
            return record_reference(args, checks)
        if args.command == "check":
            return check(args, checks)
        workload = portal2_scenarios.load_workload(args.workload)
        scenarios = workload["scenarios"]
        if args.scenario:
            unknown = set(args.scenario) - {s["name"] for s in scenarios}
            if unknown:
                parser.error("unknown scenario: " + ", ".join(sorted(unknown)))
            scenarios = [s for s in scenarios if s["name"] in args.scenario]
        if args.command == "suite":
            args.out = args.out or Path(os.environ.get("CONFORMANCE_OUT") or ROOT / (
                "quality-results/portal2-material-shots-%s"
                % datetime.datetime.now().strftime("%Y%m%d-%H%M%S")))
            args.out = Path(args.out) / "build" if os.environ.get("CONFORMANCE_OUT") \
                else Path(args.out)
            args.out.mkdir(parents=True, exist_ok=True)
            capture_build(args, args.workload, workload, scenarios)
            return check(argparse.Namespace(capture=args.out, reference=args.reference,
                                            retail=None), checks)
        out = Path(args.out)
        if (out / "capture.json").exists():
            parser.error("%s already holds a capture; use a new directory" % out)
        out.mkdir(parents=True, exist_ok=True)
        if args.side == "build":
            capture_build(args, args.workload, workload, scenarios)
        else:
            capture_retail(args, args.workload, workload, scenarios)
        return 0
    except (OSError, ValueError, KeyError, ShotError, portal2_scenarios.ScenarioError,
            subprocess.SubprocessError) as error:
        print("FAIL portal2_material_shots: %s" % error, flush=True)
        conformance_result.report_conformance(1, 1)
        return 1


if __name__ == "__main__":
    sys.exit(main())
