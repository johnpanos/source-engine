#!/usr/bin/env python3
"""Portal 2 paint retail conformance: gel on world surfaces and its powers.

quality/workloads/portal2-paint-world-v1 paints sp_a3_jump_intro's floor
trench and end wall with moved info_paint_sprayers and measures, on the
server, what the gel does: the rebound off repulsion gel, the speed on
propulsion gel, the rebound after water erased the gel, and portal placement
on conversion gel. It also takes a shot of each coat. This tool runs that
workload on this build and on the retail Portal 2 binary at 1024x768 and
judges this build against the retail numbers:

  capture --side build|retail   runs the workload through
                         tools/quality/portal2_material_shots.py capture (this
                         build: SDL offscreen, native Vulkan; retail: an
                         isolated headless compositor), keeping the console
                         log, QA_MEASURE numbers and shots as PNG
  record                 writes the workload's reference.json from a retail
                         capture: the measures, and per declared region the
                         pixel class fractions and mean colour. Numbers only;
                         no retail pixels are stored in the repository.
  check                  judges a build capture against the reference and
                         prints one checks-v1 record
  suite                  capture --side build, then check (the manifest row)
  self-test              the comparator on synthetic measures and frames,
                         with every seeded defect required to fail

Checks (quality/workloads/<workload>/checks.json), per scenario:

  <scenario>.run          the scenario ran: every required step passed
  <scenario>.<check>      a measure rule (relative, absolute, at_least,
                          at_most) on one QA_MEASURE number, or a region rule
                          (fraction, fraction_near, mean) on one shot region

Each check names the defect it pins. Retail content and binaries are not in
the repository: the suite needs a local Portal 2 installation
(SOURCE_PORTAL2_STEAM_ROOT) and is optional in the conformance manifest,
never certified without it.

    python3 tools/quality/portal2_paint.py capture --side retail --out <dir>
    python3 tools/quality/portal2_paint.py record --retail <dir>
    python3 tools/quality/portal2_paint.py suite --build build-p2 --out <dir>
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
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import conformance  # noqa: E402
import conformance_result  # noqa: E402


ROOT = Path(conformance.repo_root())
WORKLOAD = ROOT / "quality/workloads/portal2-paint-world-v1"
CHECKS_SCHEMA = "portal2-paint-checks/v1"
REFERENCE_SCHEMA = "portal2-paint-reference/v1"
REPORT_SCHEMA = "portal2-paint-report/v1"
DEFAULT_STEAM_ROOT = Path.home() / ".local/share/Steam/steamapps/common/Portal 2"
SHOTS_TOOL = ROOT / "tools/quality/portal2_material_shots.py"
MEASURE_LINE = re.compile(r"^QA_MEASURE (\S+) (\S+) (-?[0-9.]+)\s*$")
MEASURE_RULES = ("relative", "absolute", "at_least", "at_most")
REGION_RULES = ("fraction", "fraction_near", "mean")
CLASSES = ("blue", "orange", "white", "gel")


class PaintError(Exception):
    pass


def now_iso():
    return datetime.datetime.now(datetime.timezone.utc).isoformat()


# ---------------------------------------------------------------------------
# Pixel classes and region statistics
# ---------------------------------------------------------------------------

def class_masks(pixels):
    """HxWx3 uint8 -> class name -> boolean mask.

    Thresholds were read off retail frames of lit gel: repulsion gel is a
    saturated sky blue, propulsion gel a saturated orange, and conversion gel
    a neutral grey where the bare concrete and metal around it are warm.
    """
    import numpy as np
    r, g, b = (pixels[..., k].astype(int) for k in range(3))
    chroma = pixels.max(axis=2).astype(int) - pixels.min(axis=2).astype(int)
    luma = 0.299 * r + 0.587 * g + 0.114 * b
    blue = (b > r + 50) & (b > 110) & (b >= g)
    orange = (r > 140) & (r > g + 50) & (g > b + 10) & (b < 120)
    white = (chroma < 14) & (luma > 80)
    return {"blue": blue, "orange": orange, "white": white, "gel": blue | orange,
            "_luma": np.asarray(luma)}


def region_stats(pixels, rect):
    x0, y0, x1, y1 = rect
    sub = pixels[y0:y1, x0:x1]
    if sub.size == 0:
        raise PaintError("empty region %s" % (rect,))
    masks = class_masks(sub)
    return {
        "fractions": {name: round(float(masks[name].mean()), 4) for name in CLASSES},
        "mean": [round(float(v), 1) for v in sub.reshape(-1, 3).mean(axis=0)],
    }


def load_rgb(path):
    import numpy as np
    from PIL import Image
    return np.asarray(Image.open(path).convert("RGB"))


# ---------------------------------------------------------------------------
# Workload files
# ---------------------------------------------------------------------------

def load_checks(workload_dir):
    path = Path(workload_dir) / "checks.json"
    try:
        data = json.loads(path.read_text())
    except (OSError, json.JSONDecodeError) as error:
        raise PaintError("cannot read %s: %s" % (path, error))
    if data.get("schema") != CHECKS_SCHEMA:
        raise PaintError("%s: schema must be %s" % (path, CHECKS_SCHEMA))
    names = set()
    for scenario, spec in data.get("scenarios", {}).items():
        for rule in spec.get("measures", []):
            if rule.get("rule") not in MEASURE_RULES or not rule.get("measure"):
                raise PaintError("%s: %s needs a measure and a rule" % (path, rule.get("check")))
        for rule in spec.get("regions", []):
            if rule.get("rule") not in REGION_RULES or len(rule.get("region", [])) != 4:
                raise PaintError("%s: %s needs a region and a rule" % (path, rule.get("check")))
            if rule["rule"] != "mean" and rule.get("class") not in CLASSES:
                raise PaintError("%s: %s needs a pixel class" % (path, rule.get("check")))
        for rule in spec.get("measures", []) + spec.get("regions", []):
            name = "%s.%s" % (scenario, rule.get("check"))
            if not isinstance(rule.get("requires", []), list):
                raise PaintError("%s: %s requires must be a list" % (path, name))
            if not rule.get("check") or name in names or not rule.get("defect"):
                raise PaintError("%s: checks need unique names and a defect" % path)
            names.add(name)
    if not names:
        raise PaintError("%s declares no checks" % path)
    for scenario, spec in data.get("scenarios", {}).items():
        for rule in spec.get("measures", []) + spec.get("regions", []):
            for need in rule.get("requires", []):
                if "%s.%s" % (scenario, need) not in names:
                    raise PaintError("%s: %s requires unknown check %s" % (
                        path, rule["check"], need))
    return data


def read_capture(capture_dir):
    capture_dir = Path(capture_dir)
    try:
        capture = json.loads((capture_dir / "capture.json").read_text())
    except (OSError, json.JSONDecodeError) as error:
        raise PaintError("cannot read %s/capture.json: %s" % (capture_dir, error))
    if capture.get("status") != "complete":
        raise PaintError("%s is not a complete capture" % capture_dir)
    return capture


def scenario_measures(capture_dir, scenario):
    """name -> value from the scenario's QA_MEASURE lines (duplicates fail)."""
    log = (Path(capture_dir) / scenario / "console.log").read_text(errors="replace")
    measures = {}
    for line in log.splitlines():
        match = MEASURE_LINE.match(line.strip())
        if match and match.group(1) == scenario:
            if match.group(2) in measures:
                raise PaintError("%s reported %s twice" % (scenario, match.group(2)))
            measures[match.group(2)] = float(match.group(3))
    return measures


def shot_path(capture_dir, scenario, shot):
    return Path(capture_dir) / scenario / "shots" / (shot + ".png")


# ---------------------------------------------------------------------------
# Record and judge
# ---------------------------------------------------------------------------

def record_reference(retail_dir, checks):
    capture = read_capture(retail_dir)
    if capture.get("side") != "retail":
        raise PaintError("%s is not a retail capture" % retail_dir)
    reference = {"schema": REFERENCE_SCHEMA, "recorded_utc": now_iso(),
                 "retail_capture_started_utc": capture.get("started_utc"), "scenarios": {}}
    for scenario, spec in checks["scenarios"].items():
        record = capture["scenarios"].get(scenario)
        if not record or record.get("status") != "pass":
            raise PaintError("retail scenario %s did not pass: %s" % (
                scenario, record and record.get("failures")))
        measures = scenario_measures(retail_dir, scenario)
        regions = {}
        for rule in spec.get("regions", []):
            pixels = load_rgb(shot_path(retail_dir, scenario, rule["shot"]))
            key = "%s:%s" % (rule["shot"], ",".join(str(v) for v in rule["region"]))
            regions[key] = region_stats(pixels, rule["region"])
        missing = [r["measure"] for r in spec.get("measures", []) if r["measure"] not in measures]
        if missing:
            raise PaintError("retail %s did not report %s" % (scenario, ", ".join(missing)))
        reference["scenarios"][scenario] = {"measures": measures, "regions": regions}
    return reference


def judge_measure(rule, ours, retail):
    """(ok, detail) for one measure rule."""
    kind = rule["rule"]
    if ours is None:
        return False, "not reported"
    if kind == "relative":
        allowed = rule["tolerance"] * max(abs(retail), 1.0)
        ok = abs(ours - retail) <= allowed
        detail = "%.1f vs retail %.1f (allowed +-%.1f)" % (ours, retail, allowed)
    elif kind == "absolute":
        ok = abs(ours - retail) <= rule["tolerance"]
        detail = "%.1f vs retail %.1f (allowed +-%.1f)" % (ours, retail, rule["tolerance"])
    elif kind == "at_least":
        ok = ours >= rule["value"] and retail >= rule["value"]
        detail = "%.1f (retail %.1f; need >= %.1f)" % (ours, retail, rule["value"])
    else:
        ok = ours <= rule["value"] and retail <= rule["value"]
        detail = "%.1f (retail %.1f; need <= %.1f)" % (ours, retail, rule["value"])
    return ok, detail


def judge_region(rule, ours, retail):
    kind = rule["rule"]
    if kind == "mean":
        worst = max(abs(a - b) for a, b in zip(ours["mean"], retail["mean"]))
        return worst <= rule["tolerance"], "mean %s vs retail %s (max channel difference " \
            "%.1f, allowed %.1f)" % (ours["mean"], retail["mean"], worst, rule["tolerance"])
    name = rule["class"]
    mine, theirs = ours["fractions"][name], retail["fractions"][name]
    if kind == "fraction":
        need = rule["min_ratio"] * theirs
        return mine >= need, "%s %.3f vs retail %.3f (need >= %.3f)" % (name, mine, theirs, need)
    return abs(mine - theirs) <= rule["tolerance"], "%s %.3f vs retail %.3f (allowed +-%.2f)" % (
        name, mine, theirs, rule["tolerance"])


def judge(capture_dir, checks, reference):
    """Returns check name -> {outcome, detail, defect}."""
    capture = read_capture(capture_dir)
    results = {}

    def put(name, ok, detail, defect=""):
        results[name] = {"outcome": "PASS" if ok else "FAIL", "detail": detail, "defect": defect}

    for scenario, spec in checks["scenarios"].items():
        record = capture["scenarios"].get(scenario)
        retail = reference["scenarios"].get(scenario)
        run_ok = bool(record) and record.get("status") == "pass"
        put("%s.run" % scenario, run_ok and retail is not None,
            "no retail reference" if retail is None else
            ("scenario missing" if not record else
             ("; ".join(record.get("failures", [])) or record.get("status"))),
            "the scenario could not drive the map")
        try:
            measures = scenario_measures(capture_dir, scenario) if record else {}
        except (OSError, PaintError) as error:
            measures = {}
            put("%s.measures" % scenario, False, str(error), "measure lines unreadable")
        for rule in spec.get("measures", []):
            name = "%s.%s" % (scenario, rule["check"])
            if retail is None or rule["measure"] not in retail["measures"]:
                put(name, False, "no retail value for %s" % rule["measure"], rule["defect"])
                continue
            ok, detail = judge_measure(rule, measures.get(rule["measure"]),
                                       retail["measures"][rule["measure"]])
            put(name, ok, detail, rule["defect"])
        for rule in spec.get("regions", []):
            name = "%s.%s" % (scenario, rule["check"])
            key = "%s:%s" % (rule["shot"], ",".join(str(v) for v in rule["region"]))
            if retail is None or key not in retail["regions"]:
                put(name, False, "no retail statistics for %s" % key, rule["defect"])
                continue
            try:
                ours = region_stats(load_rgb(shot_path(capture_dir, scenario, rule["shot"])),
                                    rule["region"])
            except (OSError, PaintError) as error:
                put(name, False, "shot unreadable: %s" % error, rule["defect"])
                continue
            ok, detail = judge_region(rule, ours, retail["regions"][key])
            put(name, ok, detail, rule["defect"])
    # A rule whose prerequisite failed demonstrates nothing (water cannot be
    # shown to erase gel that was never there): it fails as unverifiable.
    for scenario, spec in checks["scenarios"].items():
        for rule in spec.get("measures", []) + spec.get("regions", []):
            name = "%s.%s" % (scenario, rule["check"])
            failed = [need for need in rule.get("requires", [])
                      if results.get("%s.%s" % (scenario, need), {}).get("outcome") != "PASS"]
            if failed and results[name]["outcome"] == "PASS":
                put(name, False, "unverifiable: %s failed (%s)" % (
                    ", ".join(failed), results[name]["detail"]), rule["defect"])
    return results


def report(results, out=None, meta=None):
    checks = conformance_result.Checks()
    for name, value in results.items():
        print("  %-4s %s %s" % (value["outcome"], name, value["detail"]))
        checks.check(value["outcome"] == "PASS", name, value["detail"] +
                     (" [defect: %s]" % value["defect"] if value["outcome"] != "PASS" and
                      value.get("defect") else ""))
    if out:
        Path(out).mkdir(parents=True, exist_ok=True)
        body = {"schema": REPORT_SCHEMA, "finished_utc": now_iso(), "checks": results}
        body.update(meta or {})
        (Path(out) / "paint-report.json").write_text(json.dumps(body, indent=2) + "\n")
    return checks.report()


# ---------------------------------------------------------------------------
# Capture (delegated to portal2_material_shots.py, which runs both sides)
# ---------------------------------------------------------------------------

def capture(args, side, out):
    command = [sys.executable, str(SHOTS_TOOL), "capture", "--side", side,
               "--workload", str(Path(args.workload) / "scenarios.json"),
               "--out", str(Path(out).resolve()), "--steam-root", str(args.steam_root)]
    if side == "build":
        command += ["--build", str(args.build), "--runtime", str(args.runtime)]
        for extra in args.extra_arg:
            command.append("--extra-arg=" + extra)
    else:
        command += ["--mirror", str(Path(args.mirror).resolve())]
    Path(out).mkdir(parents=True, exist_ok=True)
    process = subprocess.run(command, cwd=ROOT)
    if not (Path(out) / "capture.json").is_file():
        raise PaintError("capture wrote no capture.json (exit %d)" % process.returncode)
    return read_capture(out)


# ---------------------------------------------------------------------------
# Self-test: the comparator on synthetic data, with seeded defects
# ---------------------------------------------------------------------------

SELF_TEST_FAULTS = {
    # fault -> (what it seeds, checks that must fail)
    "no-bounce": ({"measure": ("bounce.rise", 0.0)}, ["power.bounce", "power.erase"]),
    "no-speed": ({"measure": ("speed.orange_top", 175.0)}, ["power.speed"]),
    "not-erased": ({"measure": ("bounce_erased.rise", 190.0)}, ["power.erase"]),
    "no-portal": ({"measure": ("portal.white_distance", 750.0)}, ["power.portal"]),
    "portal-anywhere": ({"measure": ("portal.control_distance", 0.0)}, ["control.portal"]),
    "missing-measure": ({"drop": "speed.blue_top"}, ["control.speed"]),
    "unpainted-floor": ({"paint": ("floor_blue", None)}, ["coverage.blue_floor",
                                                          "look.blue_floor"]),
    "unpainted-wall": ({"paint": ("wall_white", None)}, ["coverage.white_wall"]),
    "erase-ignored": ({"paint": ("floor_erased", "orange-everywhere")},
                      ["coverage.erased_patch"]),
    "wrong-colour": ({"paint": ("floor_orange", "blue-everywhere")},
                     ["coverage.orange_strip", "look.orange_strip"]),
    "scenario-failed": ({"status": "fail"}, ["run"]),
}


def synthetic_frames(checks, scenario, fill_class):
    """Frames whose declared regions hold the classes the checks expect."""
    import numpy as np
    colours = {"blue": (52, 121, 181), "orange": (193, 90, 57), "white": (120, 120, 118),
               "bare": (54, 46, 38)}
    frames = {}
    for rule in checks["scenarios"][scenario].get("regions", []):
        frame = frames.setdefault(rule["shot"], np.zeros((768, 1024, 3), np.uint8))
        if not frame.any():
            frame[...] = colours["bare"]
    for rule in checks["scenarios"][scenario].get("regions", []):
        if rule["rule"] == "mean" or rule["check"].startswith("coverage.erased"):
            continue
        x0, y0, x1, y1 = rule["region"]
        colour = fill_class.get(rule["shot"], rule["class"] if rule["class"] != "gel" else "orange")
        frames[rule["shot"]][y0:y1, x0:x1] = colours[colour]
    return frames


def write_synthetic_capture(directory, checks, measures, frames, status="pass"):
    from PIL import Image
    directory = Path(directory)
    capture = {"schema": "portal2-material-capture/v1", "side": "build", "status": "complete",
               "scenarios": {}}
    for scenario in checks["scenarios"]:
        folder = directory / scenario / "shots"
        folder.mkdir(parents=True, exist_ok=True)
        (directory / scenario / "console.log").write_text("".join(
            "QA_MEASURE %s %s %.3f\n" % (scenario, name, value)
            for name, value in measures.items()))
        for shot, frame in frames.items():
            Image.fromarray(frame).save(folder / (shot + ".png"))
        capture["scenarios"][scenario] = {"status": status, "failures": []}
    (directory / "capture.json").write_text(json.dumps(capture))


def self_test(workload, only_fault=None):
    """The comparator passes identical data and fails every seeded defect."""
    checks = load_checks(workload)
    scenario = next(iter(checks["scenarios"]))
    measures = {"bounce.rise": 199.3, "bounce.floor": -168.5, "speed.blue_top": 175.0,
                "speed.orange_top": 800.0, "bounce_erased.rise": 0.0,
                "portal.control_distance": 591.9, "portal.white_distance": 0.03}
    base_frames = synthetic_frames(checks, scenario, {})
    # The erased patch: gel around, bare inside, as retail shows it.
    erased = base_frames.get("floor_erased")
    if erased is not None:
        erased[...] = (193, 90, 57)
        for rule in checks["scenarios"][scenario]["regions"]:
            if rule["check"] == "coverage.erased_patch":
                x0, y0, x1, y1 = rule["region"]
                erased[y0:y1, x0:x1] = (54, 46, 38)
    tally = conformance_result.Checks()
    with tempfile.TemporaryDirectory(prefix="portal2-paint-selftest-") as temp:
        retail_dir = Path(temp) / "retail"
        write_synthetic_capture(retail_dir, checks, measures, base_frames)
        capture = json.loads((retail_dir / "capture.json").read_text())
        capture["side"] = "retail"
        (retail_dir / "capture.json").write_text(json.dumps(capture))
        reference = record_reference(retail_dir, checks)

        clean = judge(retail_dir, checks, reference)
        failed = [name for name, value in clean.items() if value["outcome"] != "PASS"]
        tally.check(not failed and len(clean) >= 10, "self-test.identical-passes",
                    "identical data failed %s" % failed)

        faults = SELF_TEST_FAULTS if not only_fault else {only_fault: SELF_TEST_FAULTS[only_fault]}
        for fault, (seed, expected) in faults.items():
            seeded_measures = dict(measures)
            frames = {shot: frame.copy() for shot, frame in base_frames.items()}
            status = "pass"
            if "measure" in seed:
                seeded_measures[seed["measure"][0]] = seed["measure"][1]
            if "drop" in seed:
                seeded_measures.pop(seed["drop"])
            if "paint" in seed:
                shot, how = seed["paint"]
                if how is None:
                    frames[shot][...] = (54, 46, 38)
                elif how == "orange-everywhere":
                    frames[shot][...] = (193, 90, 57)
                else:
                    frames[shot][...] = (52, 121, 181)
            if "status" in seed:
                status = seed["status"]
            build_dir = Path(temp) / ("build-" + fault)
            write_synthetic_capture(build_dir, checks, seeded_measures, frames, status)
            results = judge(build_dir, checks, reference)
            wanted = ["%s.%s" % (scenario, check) for check in expected]
            missed = [name for name in wanted if results.get(name, {}).get("outcome") != "FAIL"]
            extra = [name for name, value in results.items()
                     if value["outcome"] == "FAIL" and name not in wanted]
            tally.check(not missed, "self-test.%s.detected" % fault,
                        "seeded %s but %s passed" % (seed, missed))
            tally.check(not extra, "self-test.%s.isolated" % fault,
                        "seeded %s also failed %s" % (seed, extra))
    return tally.report()


# ---------------------------------------------------------------------------
# Command line
# ---------------------------------------------------------------------------

def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = parser.add_subparsers(dest="command", required=True)
    steam = Path(os.environ.get("SOURCE_PORTAL2_STEAM_ROOT", DEFAULT_STEAM_ROOT))

    def common(p):
        p.add_argument("--workload", type=Path, default=WORKLOAD)
        p.add_argument("--steam-root", type=Path, default=steam)
        p.add_argument("--build", type=Path, default=ROOT / "build-p2")
        p.add_argument("--runtime", type=Path, default=ROOT / "run/runtime-p2-paint-suite")
        p.add_argument("--mirror", type=Path, default=ROOT / "run/retail-p2-paint")
        p.add_argument("--extra-arg", action="append", default=[],
                       help="extra engine argument for the build side (repeatable)")

    p = sub.add_parser("capture", help="run the workload on this build or on retail")
    common(p)
    p.add_argument("--side", choices=("build", "retail"), required=True)
    p.add_argument("--out", type=Path, required=True)

    p = sub.add_parser("record", help="write reference.json from a retail capture")
    p.add_argument("--workload", type=Path, default=WORKLOAD)
    p.add_argument("--retail", type=Path, required=True)

    p = sub.add_parser("check", help="judge a build capture against the reference")
    p.add_argument("--workload", type=Path, default=WORKLOAD)
    p.add_argument("--capture", type=Path, required=True)
    p.add_argument("--reference", type=Path, help="default: <workload>/reference.json")

    p = sub.add_parser("suite", help="capture this build, then check (manifest row)")
    common(p)
    p.add_argument("--out", type=Path, help="default: $CONFORMANCE_OUT or a new directory")

    p = sub.add_parser("self-test", help="the comparator on synthetic data")
    p.add_argument("--workload", type=Path, default=WORKLOAD)
    p.add_argument("--fault", choices=sorted(SELF_TEST_FAULTS))

    args = parser.parse_args(argv)
    try:
        if args.command == "self-test":
            return self_test(args.workload, args.fault)
        checks = load_checks(args.workload)
        if args.command == "capture":
            capture(args, args.side, args.out)
            return 0
        if args.command == "record":
            reference = record_reference(args.retail, checks)
            path = Path(args.workload) / "reference.json"
            path.write_text(json.dumps(reference, indent=2) + "\n")
            print("portal2_paint: wrote %s" % path)
            return 0
        reference_path = Path(getattr(args, "reference", None) or
                              Path(args.workload) / "reference.json")
        try:
            reference = json.loads(reference_path.read_text())
        except (OSError, json.JSONDecodeError) as error:
            raise PaintError("cannot read reference %s: %s" % (reference_path, error))
        if reference.get("schema") != REFERENCE_SCHEMA:
            raise PaintError("%s: schema must be %s" % (reference_path, REFERENCE_SCHEMA))
        if args.command == "check":
            return report(judge(args.capture, checks, reference), args.capture)
        out = args.out or Path(os.environ.get("CONFORMANCE_OUT") or
                               tempfile.mkdtemp(prefix="portal2-paint-"))
        if not Path(args.steam_root, "portal2_linux").exists():
            raise PaintError("no Portal 2 installation at %s" % args.steam_root)
        capture(args, "build", out)
        return report(judge(out, checks, reference), out,
                      {"build": str(args.build), "source": conformance.source_identity(str(ROOT))})
    except PaintError as error:
        print("portal2_paint: %s" % error)
        return conformance_result.report_conformance(0, 1)


if __name__ == "__main__":
    sys.exit(main())
