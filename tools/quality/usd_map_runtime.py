#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
#
# Runtime smoke of a USD-compiled BSP2 map (RFC 0009 U1-U2, roadmap R59).
#
#   python3 tools/quality/usd_map_runtime.py --map-file run/maps/usd_room/maps/usd_room.bsp \
#       --report authoring-report.json --runtime run/runtime \
#       --build build-r03-dedicated --product dedicated --out quality-results/u1-dedicated
#
# Boots the map in a private staged runtime (portal_boot staging, as
# bsp2_dedicated.py does) and checks, from the product's console log:
#   - the engine loads the file as a BSP2 container and the map spawns
#     (status names it; the client's player becomes active);
#   - report_entities lists every authored runtime entity class;
#   - map_container_probe (engine collision map, CM_BoxTrace) agrees with
#     the independent Python traces of usd_map_check.py: same hit fraction,
#     an open cluster at each authored placement, and the PVS row present.
# With --roles (RFC 0009 U2) it also probes the running game with
# map_entity_probe, map_entity_probe_schedule (samples at game-time
# intervals) and map_trace_probe (game/server/entitylist.cpp) and checks each
# role's behaviour against the authored stage:
#   - prop_static: a ray and a player hull dropped onto a vphysics prop stop on
#     its model's top (a static-prop hit), while the world alone would let
#     them reach the floor; a collision-none prop lets both through;
#   - prop_physics: a vphysics body whose z falls and then rests with its
#     model's hull on the floor;
#   - prop_dynamic: an entity that is not a simulated physics body: it keeps
#     its authored height, starts in its default animation and plays the
#     animation a connection sets;
#   - triggers: every authored connection fires (the engine's own I/O trace at
#     developer 2), and no input goes unhandled;
#   - func_movelinear: a pushed brush that moves from rest to its authored
#     displacement after the trigger opens it.
# The dedicated product runs headless. The client product runs with the null
# renderer and SDL's offscreen video driver: no window is created, so nothing
# appears on a desktop. Prints the checks-v1 record.
#
# ============================================================================

import argparse
import json
import re
import shutil
import sys
import tempfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import bsp2_dedicated  # noqa: E402
import bsp2_server_compare  # noqa: E402
import portal_boot  # noqa: E402
import source_content  # noqa: E402
import source_model  # noqa: E402
import stage_runtime  # noqa: E402
import usd_map_check  # noqa: E402
from conformance_result import Checks  # noqa: E402

CLASS_LINE = re.compile(r"Class: (\S+) \((\d+)\)$")
ENTITY_LINE = re.compile(
    r"map_entity_probe: name=(\S*) class=(\S+) time=([0-9.]+) origin=(\S+) (\S+) (\S+) "
    r"movetype=(\d+) solid=(\d+) vphysics=([01]) motion=([01]) gravity=([01]) shadow=([01]) "
    r"asleep=([01]) "
    r"sequence=(\S+) cycle=(-?[0-9.]+)$")
TRACE_LINE = re.compile(
    r"map_trace_probe: fraction=(\S+) startsolid=([01]) allsolid=([01]) hit=(\S+) "
    r"static_prop=(-?\d+) entity=(\S+) end=(\S+) (\S+) (\S+)$")
OUTPUT_LINE = re.compile(r"\(([0-9.]+)\) output: \(([^,]+),([^)]*)\) -> \(([^,]+),([^)]*)\)"
                         r"\(([^)]*)\)$")
DIRECTIONS = ((1, 0, 0), (-1, 0, 0), (0, 1, 0), (0, -1, 0), (0, 0, 1), (0, 0, -1))
FRACTION_TOLERANCE = 1e-3
# Unnamed light entities remove themselves at spawn; vrad already baked them.
RUNTIME_CLASSES = ("info_player_start", "trigger_multiple", "trigger_once", "prop_physics",
                   "prop_dynamic", "func_movelinear")
MOVETYPE_VPHYSICS = 6
MOVETYPE_PUSH = 7
PLAYER_HULL = ((-16.0, -16.0, 0.0), (16.0, 16.0, 72.0))
# Role sampling on the client: ROLE_ROUNDS samples ROLE_INTERVAL seconds of
# game time apart from the moment the physics clock is released; the game must
# simulate at least MIN_SECONDS. A dedicated server takes one sample.
ROLE_ROUNDS = 40
ROLE_INTERVAL = 0.1
MIN_SECONDS = 2.5
AFTER_CONFIG = "usd_map_roles_after.cfg"
REST_TOLERANCE = 1.5      # units between a resting body and its hull on the floor
TOP_TOLERANCE = 2.0       # units between a static-prop hit and its model's top
STILL = 0.01              # units a non-simulated entity may drift
DROP_ABOVE = 64.0         # units above a prop's top that the static-prop probes start


def probes(report, bsp2):
    """(start, end, python fraction) rays from each authored placement."""
    m = usd_map_check.CompiledMap(bsp2)
    brushes = m.brushes()
    m.world_brushes = [brushes[b["index"]] for b in m.bsp.world_brushes()]
    rays = []
    for record in sorted(report["objects"], key=lambda o: o["id"]):
        if "origin" not in record or record["role"] == "entity_brush":
            continue
        o = record["origin"]
        start = (o[0], o[1], o[2] + 1.0)
        for direction in DIRECTIONS:
            end = tuple(start[a] + 2048.0 * direction[a] for a in range(3))
            rays.append({"id": record["id"], "start": start, "end": end,
                         "fraction": m.trace(start, end)})
    return rays, m


def number(value):
    return format(float(value), ".9g")


def role_plan(report, provenance, compiled, runtime):
    """What --roles samples and traces, and the independent expectations."""
    resolver = source_content.ContentResolver(str(runtime))
    outputs = {o["id"]: o["output"] for o in provenance["objects"]}
    objects = sorted(report["objects"], key=lambda o: o["id"])
    sampled = [o for o in objects if o["role"] in ("prop_physics", "prop_dynamic") or
               o.get("classname") == "func_movelinear"]
    hulls = {}
    for record in objects:
        if "model" in record:
            data, _ = resolver.read(record["model"])
            hulls[record["id"]] = source_model.read_model(data)["hull"]
    traces = []
    for record in (o for o in objects if o["role"] == "prop_static"):
        top = record["origin"][2] + hulls[record["id"]][1][2]
        start = (record["origin"][0], record["origin"][1], top + DROP_ABOVE)
        end = (start[0], start[1], record["origin"][2] - 256.0)
        world = compiled.trace(start, end)
        floor = start[2] + (end[2] - start[2]) * world
        for hull in (None, PLAYER_HULL):
            traces.append({"id": record["id"], "collision": record["collision"],
                           "static_prop": outputs[record["id"]]["static_prop"],
                           "start": start, "end": end, "hull": hull, "top": top,
                           "world_floor": floor if hull is None else
                           start[2] + (end[2] - start[2]) * compiled.trace(start, end, hull)})
    return {"sampled": sampled, "traces": traces, "hulls": hulls}


def role_commands(plan, simulates):
    """(commands before the world probes, whether they end in a schedule).
    A simulating client releases the physics clock and starts the schedule;
    the rest of the config runs when the schedule completes."""
    names = [record["id"] for record in plan["sampled"]]
    traces = []
    for trace in plan["traces"]:
        values = list(trace["start"]) + list(trace["end"])
        if trace["hull"]:
            values += list(trace["hull"][0]) + list(trace["hull"][1])
        traces.append("map_trace_probe " + " ".join(number(v) for v in values))
    if simulates:
        return ["developer 2", "phys_timescale 1",
                'map_entity_probe_schedule %s %d "exec %s" %s' % (
                    number(ROLE_INTERVAL), ROLE_ROUNDS, AFTER_CONFIG, " ".join(names))], \
            traces + ["developer 0"]
    return ["map_entity_probe " + name for name in names] + traces, []


def parse_samples(text):
    samples = {}
    for line in text.splitlines():
        match = ENTITY_LINE.fullmatch(line.strip())
        if match:
            samples.setdefault(match[1], []).append({
                "class": match[2], "time": float(match[3]),
                "origin": [float(match[4]), float(match[5]), float(match[6])],
                "movetype": int(match[7]), "vphysics": int(match[9]), "motion": int(match[10]),
                "gravity": int(match[11]), "shadow": int(match[12]), "asleep": int(match[13]),
                "sequence": match[14], "cycle": float(match[15])})
    return samples


def simulated_body(sample):
    """A body the physics simulation moves: vphysics movement with motion and
    gravity, not a shadow object that a pusher or an animation drives."""
    return sample["movetype"] == MOVETYPE_VPHYSICS and sample["vphysics"] and \
        sample["motion"] and sample["gravity"] and not sample["shadow"]


def check_spawn_state(checks, record, series):
    """What holds from the first frame: each role's kind of runtime object."""
    first = series[0]
    if record["role"] == "prop_physics":
        checks.check(first["class"] == "prop_physics" and simulated_body(first),
                     "roles.physics-body", "%s %r" % (record["id"], first))
        checks.check(abs(first["origin"][2] - record["origin"][2]) <= 1.0,
                     "roles.physics-starts-authored",
                     "%s z %.3f vs %.3f" % (record["id"], first["origin"][2],
                                            record["origin"][2]))
    elif record["role"] == "prop_dynamic":
        checks.check(all(s["class"] == "prop_dynamic" for s in series), "roles.dynamic-entity",
                     record["id"])
        checks.check(not any(simulated_body(s) for s in series) and
                     all(s["movetype"] != MOVETYPE_VPHYSICS for s in series),
                     "roles.dynamic-not-physics-body", "%s %r" % (record["id"], first))
        checks.check(all(usd_map_check.close(s["origin"], record["origin"], STILL) for s in series),
                     "roles.dynamic-keeps-placement",
                     "%s at %r..%r, authored %r" % (record["id"], series[0]["origin"],
                                                    series[-1]["origin"], record["origin"]))
        if "default_animation" in record:
            checks.check(first["sequence"].lower() == record["default_animation"].lower(),
                         "roles.dynamic-default-animation",
                         "%s starts in %s" % (record["id"], first["sequence"]))
    elif record.get("classname") == "func_movelinear":
        checks.check(all(s["movetype"] == MOVETYPE_PUSH and not simulated_body(s)
                         for s in series), "roles.movelinear-pushed", record["id"])
        checks.check(usd_map_check.close(first["origin"], [0.0, 0.0, 0.0], 0.01),
                     "roles.movelinear-starts-closed", "%s %r" % (record["id"], first["origin"]))


def check_behaviour(checks, report, plan, record, series):
    """What needs the simulation to run: falling, resting, animating, moving."""
    z = [s["origin"][2] for s in series]
    if record["role"] == "prop_physics":
        # Upright on the floor: the origin sits -hull_min.z above it.
        rest = floor_under(report, record) - plan["hulls"][record["id"]][0][2]
        checks.check(min(z) < z[0] - 1.0 and z[-1] < z[0] - 1.0, "roles.physics-falls",
                     "%s z %.3f -> %.3f" % (record["id"], z[0], z[-1]))
        checks.check(abs(z[-1] - rest) <= REST_TOLERANCE and abs(z[-1] - z[-3]) <= 0.1,
                     "roles.physics-rests-on-floor",
                     "%s ends at z %.3f; its hull rests at %.3f" % (record["id"], z[-1], rest))
    elif record["role"] == "prop_dynamic":
        wanted = [c["parameter"] for o in report["objects"] for c in o.get("connections", [])
                  if c["target"] == record["id"] and c["input"] == "SetAnimation"]
        if wanted:
            # The set animation plays (its cycle advances); a finished one-shot
            # animation returns to the default, so the last sample need not
            # show it.
            playing = [s["cycle"] for s in series if s["sequence"].lower() == wanted[-1].lower()]
            checks.check(len(playing) >= 2 and max(playing) > min(playing),
                         "roles.dynamic-animates", "%s played %s at cycles %r"
                         % (record["id"], wanted[-1], playing))
    elif record.get("classname") == "func_movelinear":
        positions = [s["origin"] for s in series]
        checks.check(usd_map_check.close(positions[-1], record["move"], 0.5),
                     "roles.movelinear-opens", "%s ends at %r, authored move %r" % (
                         record["id"], positions[-1], record["move"]))
        length = sum(b * b for b in record["move"])
        between = [p for p in positions
                   if 0.05 < sum(a * b for a, b in zip(p, record["move"])) / length < 0.95]
        checks.check(between, "roles.movelinear-moves-over-time",
                     "%s has no sample between closed and open" % record["id"])


def check_roles(checks, report, plan, text, evidence, simulates):
    """Each role's runtime contract, from the console log. `simulates` is
    false for a dedicated server without players, which never advances game
    time (engine SV_Frame); its behaviour checks are then not run, and the
    evidence says so."""
    samples = parse_samples(text)
    traces = [TRACE_LINE.fullmatch(line.strip()) for line in text.splitlines()
              if line.strip().startswith("map_trace_probe: ")]
    fired = [m for m in (OUTPUT_LINE.fullmatch(line.strip()) for line in text.splitlines()
                         if " output: (" in line) if m]
    evidence["role_samples"] = samples
    evidence["outputs_fired"] = [m.groups() for m in fired]
    times = [s["time"] for series in samples.values() for s in series]
    elapsed = (max(times) - min(times)) if times else 0.0
    evidence["simulated_seconds"] = elapsed
    evidence["behaviour_checked"] = simulates
    if simulates:
        checks.check(elapsed >= MIN_SECONDS, "roles.simulated-time",
                     "sampled %.3f s of game time" % elapsed)
    for record in plan["sampled"]:
        series = samples.get(record["id"], [])
        if not checks.check(len(series) == (ROLE_ROUNDS if simulates else 1), "roles.sampled",
                            "%s: %d samples" % (record["id"], len(series))):
            continue
        check_spawn_state(checks, record, series)
        if simulates:
            check_behaviour(checks, report, plan, record, series)
    if simulates:
        for record in (o for o in report["objects"] if o.get("connections")):
            for connection in record["connections"]:
                hits = [m for m in fired if m[3] == record["id"] and
                        m[4] == connection["target"] and m[5] == connection["input"] and
                        m[6] == connection["parameter"]]
                checks.check(hits, "roles.connection-fired", "%s %s -> %s %s" % (
                    record["id"], connection["output"], connection["target"],
                    connection["input"]))
    checks.check("unhandled input" not in text, "roles.no-unhandled-input")

    checks.equal(len(traces), len(plan["traces"]), "roles.trace-count")
    evidence["role_traces"] = []
    for trace, match in zip(plan["traces"], traces):
        if not checks.check(match is not None, "roles.trace-parsed", trace["id"]):
            continue
        end_z = float(match[9])
        evidence["role_traces"].append({
            "id": trace["id"], "hull": bool(trace["hull"]), "hit": match[4],
            "static_prop": int(match[5]), "end_z": end_z, "top": trace["top"],
            "world_floor": trace["world_floor"]})
        label = "%s %s" % (trace["id"], "hull" if trace["hull"] else "ray")
        if trace["collision"] == "vphysics":
            checks.check(match[4] == "static_prop" and int(match[5]) == trace["static_prop"],
                         "roles.static-prop-blocks", "%s hit %s %s" % (label, match[4], match[5]))
            checks.check(abs(end_z - trace["top"]) <= TOP_TOLERANCE and
                         end_z > trace["world_floor"] + 10.0, "roles.static-prop-top",
                         "%s stops at z %.3f; top %.3f, floor %.3f" % (
                             label, end_z, trace["top"], trace["world_floor"]))
        else:
            checks.check(match[4] == "world" and abs(end_z - trace["world_floor"]) <= 0.05,
                         "roles.static-prop-not-solid", "%s hit %s at z %.3f; floor %.3f" % (
                             label, match[4], end_z, trace["world_floor"]))


def floor_under(report, record):
    """The top of the highest world solid under the record's origin."""
    o = record["origin"]
    tops = [s["bounds"][1][2] for s in report["objects"] if s["role"] == "world_solid" and
            s["bounds"][0][0] <= o[0] <= s["bounds"][1][0] and
            s["bounds"][0][1] <= o[1] <= s["bounds"][1][1] and s["bounds"][1][2] <= o[2]]
    return max(tops)


def smoke(args, checks):
    report = json.loads(Path(args.report).read_text())
    rays, compiled = probes(report, args.map_file)
    plan = None
    if args.roles:
        provenance = json.loads(Path(args.provenance).read_text())
        plan = role_plan(report, provenance, compiled, args.runtime)
    evidence = {"schema": "source-usd-map-runtime/v1", "product": args.product,
                "map": args.map, "build": str(Path(args.build).resolve()),
                "map_sha256": usd_map_check.hashlib.sha256(
                    Path(args.map_file).read_bytes()).hexdigest(), "probes": []}
    out = Path(args.out).resolve()
    out.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="usd-map-runtime-") as scratch:
        stage = Path(scratch) / "runtime"
        portal_boot.stage_runtime(Path(args.runtime).resolve(), stage)
        launcher = "dedicated_launcher" if args.product == "dedicated" else "hl2_launcher"
        installed = portal_boot.install_build(Path(args.build).resolve(), stage,
                                              launcher_name=launcher)
        stage_runtime.sanitize(stage)
        checks.check("portal/bin/libserver.so" in installed, "runtime.server-installed")
        staged_map = stage / "portal/maps" / (args.map + ".bsp")
        if staged_map.exists() or staged_map.is_symlink():
            checks.check(False, "runtime.map-name-free", "the runtime already has " + args.map)
            return evidence
        shutil.copyfile(args.map_file, staged_map)
        config = stage / "portal/cfg/rfc0008_probes.cfg"
        config.parent.mkdir(parents=True, exist_ok=True)
        simulates = args.product == "client"
        first, after = role_commands(plan, simulates) if plan else ([], [])
        world = ["map_container_info"] + \
            [bsp2_dedicated.probe_command(list(r["start"]) + list(r["end"])) for r in rays] + \
            ["report_entities", "status"]
        if plan and simulates:
            config.write_text("\n".join(first + [""]))
            (config.parent / AFTER_CONFIG).write_text("\n".join(after + world + ["quit", ""]))
        else:
            config.write_text("\n".join(first + world + [""]))
        log = out / (args.map + "-" + args.product + ".log")
        log.unlink(missing_ok=True)
        run = (bsp2_dedicated.run_server if args.product == "dedicated"
               else bsp2_dedicated.run_client)
        # Console waits do not measure game time (a config's waits all ran
        # within one game tick here). The client holds the physics clock
        # (phys_timescale 0) until its config runs, after the U1 wait; the
        # config releases it and starts a game-time sample schedule, whose
        # completion runs the rest of the probes and quits. A dedicated
        # server never simulates without players (engine SV_Frame), so it
        # checks the spawn state only.
        options = {}
        if plan and simulates:
            options = {"quit_after": False, "extra": ["+phys_timescale", "0"]}
        timeout = args.timeout + (int(ROLE_ROUNDS * ROLE_INTERVAL) + 30 if plan else 0)
        result = run(stage, args.map, log, timeout, args.port, **options)
        evidence["run"] = result
    checks.equal(result.get("exit_code"), 0, "runtime.exit-status")
    if not checks.check(log.is_file(), "runtime.console-log"):
        return evidence
    text = log.read_text(errors="replace")
    try:
        capture = bsp2_server_compare.read_capture(log)
        checks.check(True, "runtime.capture")
    except ValueError as error:
        checks.check(False, "runtime.capture", str(error))
        return evidence
    checks.equal(capture["kind"], "bsp2", "runtime.loaded-bsp2")
    checks.equal(capture["status_map"], args.map, "runtime.status-map")
    checks.equal(len(capture["probes"]), len(rays), "runtime.probe-count")
    for ray, line in zip(rays, capture["probes"]):
        match = bsp2_server_compare.PROBE.fullmatch(line)
        cluster, contents, fraction = int(match[2]), int(match[3], 16), float(match[5])
        evidence["probes"].append({"id": ray["id"], "start": ray["start"], "end": ray["end"],
                                   "engine": fraction, "python": ray["fraction"],
                                   "cluster": cluster})
        checks.check(cluster >= 0 and contents == 0, "runtime.placement-open-cluster",
                     "%s cluster %d contents %x" % (ray["id"], cluster, contents))
        checks.check(abs(fraction - ray["fraction"]) <= FRACTION_TOLERANCE,
                     "runtime.trace-matches-independent",
                     "%s %r->%r engine %.6f python %.6f" % (ray["id"], ray["start"], ray["end"],
                                                            fraction, ray["fraction"]))
    classes = {m[1]: int(m[2]) for m in (CLASS_LINE.fullmatch(l.strip())
                                         for l in text.splitlines()) if m}
    evidence["classes"] = classes
    for record in report["objects"]:
        cls = record.get("classname") or record["role"]
        if cls in RUNTIME_CLASSES:
            checks.check(classes.get(cls, 0) >= 1, "runtime.entity-spawned",
                         "%s (%s)" % (cls, record["id"]))
    if args.product == "client":
        checks.check(bsp2_dedicated.client_player_active(log), "runtime.client-player-active")
    for pattern in ("Host_Error", "Map load failed", "Couldn't spawn", "missing model"):
        checks.check(pattern.lower() not in text.lower(), "runtime.no-load-error", pattern)
    if plan:
        check_roles(checks, report, plan, text, evidence, args.product == "client")
    return evidence


class RecordedChecks(Checks):
    """Checks that also keep each failure's line for the evidence file."""

    def __init__(self):
        super().__init__()
        self.failed = []

    def check(self, condition, name, detail=""):
        if not condition:
            self.failed.append("%s%s" % (name, (": " + detail) if detail else ""))
        return super().check(condition, name, detail)


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--map-file", required=True, help="the compiled BSP2 map")
    parser.add_argument("--map", default="usd_room")
    parser.add_argument("--report", required=True, help="the validator's authoring report")
    parser.add_argument("--provenance", help="the compiler's provenance sidecar (--roles)")
    parser.add_argument("--roles", action="store_true",
                        help="sample and check each role's runtime behaviour (RFC 0009 U2)")
    parser.add_argument("--runtime", required=True, help="Portal content root (read only)")
    parser.add_argument("--build", required=True, help="Waf output with the product")
    parser.add_argument("--product", choices=("dedicated", "client"), default="dedicated")
    parser.add_argument("--out", required=True)
    parser.add_argument("--timeout", type=int, default=60)
    parser.add_argument("--port", type=int, default=28140)
    args = parser.parse_args(argv)
    if args.roles and not args.provenance:
        parser.error("--roles needs --provenance")
    checks = RecordedChecks()
    evidence = smoke(args, checks)
    evidence["checks"], evidence["failures"] = checks.checks, checks.failures
    evidence["failed"] = checks.failed
    out = Path(args.out)
    out.mkdir(parents=True, exist_ok=True)
    (out / "evidence.json").write_text(json.dumps(evidence, indent=2, sort_keys=True,
                                                  default=str) + "\n")
    return checks.report()


if __name__ == "__main__":
    sys.exit(main())
