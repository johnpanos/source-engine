#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""RFC 0016 K5 "Culling matches": the render core's world culling against the
legacy visible sets at the K0 views.

    python3 tools/render/culling_capture.py suite --out DIR [--scenario ID]
    python3 tools/render/culling_capture.py selftest

The engine keeps a render scene of the loaded map's world: one instance per
non-solid BSP leaf, bounded by the box the legacy traversal last tests for it
(engine/render_core_world.cpp). For a view, the core culls that scene with the
view's own frustum planes and the BSP visibility provider (the legacy
traversal's PVS, area and area-frustum decisions), through the same
scene::BuildDrawList every core view uses. r_core_cull_capture records, for
every world view of one frame (main, skybox, portal and water views), the
leaves the core drew next to the legacy visible leaves.

suite boots each scenario of the K0 view workload
(quality/workloads/render-view-oracles-v1.json, through view_oracle.py's own
shot expansion and scripting) and takes a culling capture where the view
oracle takes its screenshot. It checks, per shot, that the capture exists,
that every view's core leaf set equals the legacy set exactly except for
leaves legacy's own view-frustum test rejects (no extra leaf; every missing
leaf lies outside the view frustum by R_CullNodeInternal's box test), that
the core's static props equal the props the client drew in that view by the
same rule, that the pooled draw list (BuildDrawListPooled on the core's
executor) equals the serial one item for item, and per scenario that the
core's frustum pass removed leaves (the provider did not do all the work) and
that props were compared. checks-v1.

The exception (RFC 0016 K5, amended 2026-09-28): legacy tests a leaf in an
area it sees through an area portal against that area's frustum and not the
view's, so it keeps leaves wholly outside the view (they draw no pixel). The
core culls every leaf with the view's own planes. The first run found 255
such leaves on testchmb_a_00, all in areas behind area portals, and no other
difference.

selftest runs the judge on seeded records: a missing leaf, an extra leaf, a
capture without the core (live false) and an empty capture must each fail.
"""

import argparse
import json
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools" / "render"))
sys.path.insert(0, str(ROOT / "tools" / "quality"))
import view_oracle  # noqa: E402
from conformance_result import Checks  # noqa: E402

SCHEMA = "source-core-culling/v2"


def capture_name(index):
    return "core_cull_%03d.jsonl" % index


def read_records(path):
    return [json.loads(line) for line in Path(path).read_text().splitlines() if line.strip()]


def judge(records):
    """[failures] for one shot's records."""
    failures = []
    if not records:
        return ["no world view was captured"]
    for record in records:
        where = "view %d" % record.get("view", -1)
        if record.get("schema") != SCHEMA:
            failures.append("%s: schema %r" % (where, record.get("schema")))
        if not record.get("live"):
            failures.append("%s: the core did not cull (no world scene)" % where)
        if record.get("pooled_equal") != 1:
            failures.append("%s: the pooled draw list %s the serial one" % (
                where, "was not built beside" if record.get("pooled_equal") == -1
                else "differs from"))
        details = {d["leaf"]: d for d in record.get("missing_detail", [])}
        inside = [leaf for leaf in record.get("missing", [])
                  if not details.get(leaf, {}).get("outside_view", False)]
        if inside:
            failures.append("%s: %d legacy-visible leaves inside the view the core dropped "
                            "(first %s)" % (where, len(inside), inside[:5]))
        if record.get("extra"):
            failures.append("%s: %d leaves the core kept that legacy did not (first %s)"
                            % (where, len(record["extra"]), record["extra"][:5]))
        prop_details = {d["prop"]: d for d in record.get("missing_props_detail", [])}
        inside = [prop for prop in record.get("missing_props", [])
                  if not prop_details.get(prop, {}).get("outside_view", False)]
        if inside:
            failures.append("%s: %d props the client drew inside the view the core dropped "
                            "(first %s)" % (where, len(inside), inside[:5]))
        if record.get("extra_props"):
            failures.append("%s: %d props the core kept that the client did not draw (first %s)"
                            % (where, len(record["extra_props"]), record["extra_props"][:5]))
    return failures


def run_scenario(scenario, workload, args, out):
    shots = view_oracle.expand_shots(scenario)
    common = workload["common"]
    directory = out / scenario["id"]
    runtime = view_oracle.prepare_runtime(scenario, args, out)
    build = args.p2_build if scenario["game"] == "portal2" else args.build
    width, height = scenario.get("size", common["size"])
    engine_args = common.get("engine_args", []) + scenario.get("engine_args", [])
    startup = common.get("startup_commands", []) + scenario.get("startup_commands", [])
    physics = scenario.get("physics", common["physics"])
    if scenario.get("settings") == "legacy-ports":
        width, height = view_oracle.legacy_ports_views.WIDTH, view_oracle.legacy_ports_views.HEIGHT
        engine_args = common.get("engine_args", []) + \
            list(view_oracle.legacy_ports_views.ENGINE_ARGS)
        startup = common.get("startup_commands", []) + \
            list(view_oracle.legacy_ports_views.STARTUP_COMMANDS)
        physics = "vphysics_box3d"
    command = [sys.executable, str(view_oracle.PORTAL_BOOT), "--game", scenario["game"],
               "--runtime", str(runtime), "--build", str(build), "--out", str(directory),
               "--headless", "--renderer", common["renderer"], "--map", scenario["map"],
               "--physics", physics, "--width", str(width), "--height", str(height),
               "--capture-wait", str(view_oracle.script_frames(scenario, shots)),
               "--timeout", str(args.timeout)]
    frame = lambda index, shot: "r_core_cull_capture %s %s" % (capture_name(index), shot["name"])
    for line in view_oracle.console_script(scenario, shots, frame):
        command += ["--console-command", line]
    for argument in engine_args:
        command.append("--engine-arg=" + argument)
    for line in startup:
        command += ["--startup-command", line]
    completed = subprocess.run(command, capture_output=True, text=True)
    (out / (scenario["id"] + ".boot.log")).write_text(completed.stdout + completed.stderr)
    game_dir = directory / "runtime" / scenario["game"]
    results = []
    for index, shot in enumerate(shots):
        path = game_dir / capture_name(index)
        results.append((shot["name"], read_records(path) if path.is_file() else None))
    return completed.returncode, results


def command_suite(args):
    checks = Checks()
    out = args.out
    out.mkdir(parents=True, exist_ok=True)
    workload = view_oracle.load_workload(args.workload)
    scenarios = view_oracle.select_scenarios(workload, args.scenario)
    summary = []
    for scenario in scenarios:
        status, results = run_scenario(scenario, workload, args, out)
        checks.check(status == 0, "%s.boots" % scenario["id"],
                     "portal_boot exited %d (see %s.boot.log)" % (status, scenario["id"]))
        frustum = 0
        views = 0
        props = 0
        for name, records in results:
            key = "%s.%s" % (scenario["id"], name)
            if not checks.check(records is not None, key + ".captured", "no culling capture"):
                continue
            failures = judge(records)
            checks.check(not failures, key + ".core-culling-equals-legacy", "; ".join(failures[:3]))
            frustum += sum(max(0, r.get("frustum_culled", 0)) for r in records)
            outside = sum(len(r.get("missing", [])) for r in records)
            props += sum(r.get("legacy_props", 0) for r in records)
            views += len(records)
            summary.append({"scenario": scenario["id"], "shot": name, "views": len(records),
                            "legacy": [r.get("legacy") for r in records],
                            "legacy_outside_view": outside,
                            "legacy_props": [r.get("legacy_props") for r in records],
                            "frustum_culled": [r.get("frustum_culled") for r in records],
                            "provider_culled": [r.get("provider_culled") for r in records]})
        checks.check(frustum > 0, "%s.the-core-frustum-culls" % scenario["id"],
                     "the core's frustum pass removed no leaf over %d views" % views)
        checks.check(props > 0, "%s.props-are-compared" % scenario["id"],
                     "no view drew a static prop")
    (out / "culling-summary.json").write_text(json.dumps(summary, indent=1) + "\n")
    for row in summary:
        print("%s %s: %d views, legacy leaves %s (%d outside the view), props %s, "
              "frustum-culled %s, provider-culled %s" % (
                  row["scenario"], row["shot"], row["views"], row["legacy"],
                  row["legacy_outside_view"], row["legacy_props"], row["frustum_culled"],
                  row["provider_culled"]))
    return checks.report()


def command_selftest(_args):
    checks = Checks()
    good = {"schema": SCHEMA, "view": 0, "live": True, "missing": [], "extra": [],
            "frustum_culled": 10, "pooled_equal": 1}
    checks.check(not judge([good]), "control.a-matching-view-passes")
    checks.check(not judge([dict(good, missing=[12], missing_detail=[
        {"leaf": 12, "area": 3, "outside_view": True}])]),
        "control.a-legacy-leaf-outside-the-view-may-be-dropped")
    seeded = {
        "missing-leaf": dict(good, missing=[12]),
        "missing-leaf-inside-the-view": dict(good, missing=[12], missing_detail=[
            {"leaf": 12, "area": 3, "outside_view": False}]),
        "extra-leaf": dict(good, extra=[40]),
        "no-core": dict(good, live=False),
        "missing-prop-inside-the-view": dict(good, missing_props=[3], missing_props_detail=[
            {"prop": 3, "outside_view": False}]),
        "extra-prop": dict(good, extra_props=[7]),
        "pooled-differs": dict(good, pooled_equal=0),
        "no-pooled-list": dict(good, pooled_equal=-1),
        "wrong-schema": dict(good, schema="other"),
    }
    for name, record in seeded.items():
        checks.check(bool(judge([record])), "fault.%s.detected" % name)
    checks.check(bool(judge([])), "fault.empty-capture.detected")
    return checks.report()


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    sub = parser.add_subparsers(dest="command", required=True)
    suite = sub.add_parser("suite")
    suite.add_argument("--out", type=Path, required=True)
    suite.add_argument("--workload", type=Path, default=view_oracle.WORKLOAD)
    suite.add_argument("--scenario", action="append", default=[])
    suite.add_argument("--runtime", type=Path, default=ROOT.parent / "source-engine" / "run" /
                       "runtime")
    suite.add_argument("--build", type=Path, default=ROOT / "build-rc-client" / "install")
    suite.add_argument("--p2-build", type=Path, default=ROOT / "build-rc-p2" / "install")
    suite.add_argument("--p2-runtime", type=Path, default=ROOT / "build-rc-p2" / "p2content")
    suite.add_argument("--steam-root", type=Path)
    suite.add_argument("--timeout", type=int, default=900)
    suite.set_defaults(run=command_suite)
    sub.add_parser("selftest").set_defaults(run=command_selftest)
    args = parser.parse_args(argv)
    return args.run(args)


if __name__ == "__main__":
    sys.exit(main())
