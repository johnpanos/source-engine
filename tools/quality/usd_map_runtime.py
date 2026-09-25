#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
#
# Runtime smoke of a USD-compiled BSP2 map (RFC 0009 U1, roadmap R59).
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
import stage_runtime  # noqa: E402
import usd_map_check  # noqa: E402
from conformance_result import Checks  # noqa: E402

CLASS_LINE = re.compile(r"Class: (\S+) \((\d+)\)$")
DIRECTIONS = ((1, 0, 0), (-1, 0, 0), (0, 1, 0), (0, -1, 0), (0, 0, 1), (0, 0, -1))
FRACTION_TOLERANCE = 1e-3
# Unnamed light entities remove themselves at spawn; vrad already baked them.
RUNTIME_CLASSES = ("info_player_start", "trigger_multiple", "trigger_once", "prop_physics")


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
    return rays


def smoke(args, checks):
    report = json.loads(Path(args.report).read_text())
    rays = probes(report, args.map_file)
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
        config.write_text("\n".join(
            ["map_container_info"] +
            [bsp2_dedicated.probe_command(list(r["start"]) + list(r["end"])) for r in rays] +
            ["report_entities", "status", ""]))
        log = out / (args.map + "-" + args.product + ".log")
        log.unlink(missing_ok=True)
        run = (bsp2_dedicated.run_server if args.product == "dedicated"
               else bsp2_dedicated.run_client)
        result = run(stage, args.map, log, args.timeout, args.port)
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
    return evidence


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--map-file", required=True, help="the compiled BSP2 map")
    parser.add_argument("--map", default="usd_room")
    parser.add_argument("--report", required=True, help="the validator's authoring report")
    parser.add_argument("--runtime", required=True, help="Portal content root (read only)")
    parser.add_argument("--build", required=True, help="Waf output with the product")
    parser.add_argument("--product", choices=("dedicated", "client"), default="dedicated")
    parser.add_argument("--out", required=True)
    parser.add_argument("--timeout", type=int, default=60)
    parser.add_argument("--port", type=int, default=28140)
    args = parser.parse_args(argv)
    checks = Checks()
    evidence = smoke(args, checks)
    evidence["checks"], evidence["failures"] = checks.checks, checks.failures
    out = Path(args.out)
    out.mkdir(parents=True, exist_ok=True)
    (out / "evidence.json").write_text(json.dumps(evidence, indent=2, sort_keys=True,
                                                  default=str) + "\n")
    return checks.report()


if __name__ == "__main__":
    sys.exit(main())
