#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
#
# Native USD map compiler (RFC 0009 U1, roadmap R59): an authored USD stage
# (source-authoring v1) -> a playable BSP2, with no VMF and no prior BSP.
#
#   PYTHONPATH=build/toolchains/openusd-25.11/lib/python /usr/bin/python3.12 \
#       tools/quality/usd_map_compile.py compile \
#       quality/fixtures/usd-authoring/room/room.usda --map usd_room \
#       --tools build-u1-tools/install --runtime run/runtime [--store run/maps]
#
# The build is a chain of stages. Each is a function of its declared inputs
# (files, recorded by SHA-256, and JSON parameters) that writes only its own
# directory under <work>/stages/; one ordered commit at the end publishes.
# build.json records every stage's inputs, parameters, outputs and content key,
# so RFC 0008 F6 / R57 can run the chain as a cached dependency graph, with
# this serial run as its oracle. No stage starts a thread pool: vvis and vrad
# run one thread until the engine job system (RFC 0003) schedules them.
#
#   validate  the U0 validator (usd_authoring_validate.py). Its `objects` table
#             (ids, roles, Source-space placements and faces) is the only
#             input later stages read from USD. With --previous (by default
#             the published package's provenance) an id whose role changed
#             fails (id.role-changed). The layers it resolved join its inputs.
#   content   every model (and the collision its role needs) and material is in
#             the game content; records the hash of each file it resolved;
#   game      the private compile game (gameinfo.txt, lights.rad) over the
#             runtime's content;
#   brushset  the typed intermediate source-authored-brushset/v1 (JSON):
#             entities in output order, each world or brush-entity solid as its
#             authored planes, polygons, materials and affine st;
#   vbsp      `vbsp -authored` builds its brushes and entities from the brush
#             set in memory (utils/vbsp/authoredmap.cpp) and runs its normal
#             CSG, BSP, portals, leak detection and writer; a leak fails;
#   vvis      full visibility;
#   vrad      full lighting;
#   assemble  bsp2tool convert + verify, and the provenance sidecar (authored id
#             -> compiled records);
#   check     the independent output checks (usd_map_check.py) against the
#             validator's objects table and the game content;
#   publish   the commit: the content root with provenance.json and build.json
#             goes to the published-map store in one rename
#             (playable_maps.publish), so `./play <map>` loads it.
#
# A failure at any stage, with a code from
# quality/usd_authoring/source_compile_v1.json, publishes nothing and leaves
# the previously published package untouched.
#
# ============================================================================

import argparse
import hashlib
import json
import math
import os
import shutil
import subprocess
import sys
import tempfile
import time
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
sys.path.insert(0, str(HERE))

COMPILE_PROFILE = ROOT / "quality" / "usd_authoring" / "source_compile_v1.json"
ROLE_ORDER = ("entity_point", "entity_brush", "prop_static", "prop_physics", "light")
# No texture lights: authored maps light with light entities. vrad's parser
# has no comment syntax, so the file is empty.
EMPTY_LIGHTS_RAD = ""
BUILD_SCHEMA = "source-usd-map-build/v1"


class CompileError(Exception):
    """A failed build stage: one profile code and its messages."""

    def __init__(self, code, messages):
        super().__init__("%s: %s" % (code, "; ".join(messages)))
        self.code = code
        self.messages = list(messages)


def load_compile_profile(path=COMPILE_PROFILE):
    with open(path, "r", encoding="utf-8") as stream:
        profile = json.load(stream)
    if profile.get("schema") != "source-usd-compile-profile/v1":
        raise ValueError("%s is not a source-usd-compile-profile/v1" % path)
    return profile


def fail(profile, code, messages):
    if code not in profile["errors"]:
        raise KeyError("undeclared compile error code %r" % code)
    raise CompileError(code, messages)


def sha256(path):
    digest = hashlib.sha256()
    with open(path, "rb") as stream:
        for chunk in iter(lambda: stream.read(1 << 20), b""):
            digest.update(chunk)
    return digest.hexdigest()


# ------------------------------------------------------------------ policy

def light_keys(record, profile):
    """The `light` entity keys for a validated SphereLight record (policy
    usdlux-sphere-to-vrad-point/v1; see source_compile_v1.json "light")."""
    policy = profile["light"]
    inputs = record["inputs"]
    unsupported = [name for name, value in policy["unsupported_inputs"].items()
                   if inputs.get(name.split(":", 1)[1]) == value]
    if unsupported:
        fail(profile, "compile.light-unsupported",
             ["%s: %s is not converted by %s" % (record["id"], ", ".join(unsupported),
                                                 policy["policy"])])
    radius_m = inputs["radius"] * policy["meters_per_source_unit"]
    luminance = record["intensity"] * 2.0 ** inputs["exposure"] * inputs["diffuse"]
    candela = luminance * math.pi * radius_m * radius_m
    color = record["color"]
    peak = max(color)
    reference_m = policy["reference_distance_units"] * policy["meters_per_source_unit"]
    brightness = 255.0 * candela * peak / (reference_m * reference_m * policy["full_white_lux"])
    if not (math.isfinite(brightness) and brightness > 0 and peak > 0):
        fail(profile, "compile.light-unsupported",
             ["%s converts to no light (brightness %r)" % (record["id"], brightness)])
    encoded = [255.0 * (max(c, 0.0) / peak) ** (1.0 / policy["srgb_gamma"]) for c in color]
    keys = dict(profile["classes"]["light"]["keys"])
    keys["_light"] = "%s %s %s %s" % tuple(number(v) for v in encoded + [brightness])
    return keys


def number(value):
    """A key value: three decimals, no trailing zeros."""
    text = ("%.3f" % value).rstrip("0").rstrip(".")
    return "0" if text in ("-0", "") else text


def projected_st(face, units):
    """World-aligned st on the face's dominant axis, `units` per repeat: the
    mapping of a brush entity's tool faces, which the profile leaves unauthored."""
    normal = face["plane"][:3]
    axis = max(range(3), key=lambda a: abs(normal[a]))
    rows = []
    for a in (a for a in range(3) if a != axis):
        row = [0.0, 0.0, 0.0, 0.0]
        row[a] = 1.0 / units
        rows.append(row)
    return {"s": rows[0], "t": rows[1]}


# ---------------------------------------------------------------- brushset

def build_brushset(report, profile, name):
    """(brush set, provenance objects) from a clean validator report."""
    objects = sorted(report["objects"], key=lambda r: r["id"])
    world = [r for r in objects if r["role"] == "world_solid"]
    others = sorted((r for r in objects if r["role"] != "world_solid"),
                    key=lambda r: (ROLE_ORDER.index(r["role"]), r["id"]))
    counters = {"brush": 0, "side": 0, "static_prop": 0}
    provenance = []

    def solid(record, material=None):
        counters["brush"] += 1
        sides, keys = [], []
        surfaces = record.get("surfaces") or [{} for _ in record["faces"]]
        for face, surface in zip(record["faces"], surfaces):
            counters["side"] += 1
            st = face.get("st") or projected_st(face, profile["brush_entity_st_units_per_repeat"])
            side = {"side_key": counters["side"], "material": material or surface["material"],
                    "lightmap_scale": profile["lightmap_scale"], "plane": face["plane"],
                    "points": face["points"], "s": st["s"], "t": st["t"]}
            if "surface" in surface:
                side["surface"] = surface["surface"]
            sides.append(side)
            keys.append({"surface": surface.get("surface"), "side_key": counters["side"],
                         "material": side["material"]})
        # vbsp writes map brushes in load order: brush key k is dbrushes[k - 1].
        return ({"id": record["id"], "brush_key": counters["brush"], "sides": sides},
                {"brush": counters["brush"] - 1, "sides": keys})

    worldspawn = {"classname": "worldspawn",
                  "keys": dict(profile["classes"]["worldspawn"]["keys"]), "solids": []}
    entities = [worldspawn]
    for record in world:
        brush, output = solid(record)
        worldspawn["solids"].append(brush)
        provenance.append((record, output))

    for record in others:
        role = record["role"]
        classname = "light" if role == "light" else record.get("classname") or role
        entity = {"id": record["id"], "classname": classname,
                  "keys": dict(profile["classes"][classname]["keys"])}
        output = {"entity_class": classname}
        if "origin" in record and role != "entity_brush":
            entity["origin"] = record["origin"]
        if role in ("entity_point", "prop_static", "prop_physics"):
            entity["angles"] = record["angles"]
        if role in ("prop_static", "prop_physics"):
            entity["keys"]["model"] = record["model"]
        if role == "prop_static":
            entity["keys"]["solid"] = profile["static_prop_solid"][record["collision"]]
            output = {"static_prop": counters["static_prop"]}
            counters["static_prop"] += 1
        if "skin" in record:
            entity["keys"]["skin"] = str(record["skin"])
        if "mass_scale" in record:
            entity["keys"]["massScale"] = number(record["mass_scale"])
        if role == "light":
            entity["keys"].update(light_keys(record, profile))
        if role == "entity_brush":
            brush, brush_output = solid(record, profile["brush_entity_material"])
            entity["solids"] = [brush]
            output.update(brush_output)
        entities.append(entity)
        provenance.append((record, output))

    brushset = {"schema": profile["brushset_schema"], "map": name,
                "compile_profile": {"profile": profile["profile"], "version": profile["version"]},
                "entities": entities}
    return brushset, [dict({"id": r["id"], "role": r["role"], "path": r["path"]}, output=o)
                      for r, o in provenance]


def check_content(report, profile, resolver):
    """Models, the collision each role needs, and materials exist. Returns the
    resolved files with their hashes."""
    missing = {"compile.model-missing": [], "compile.model-collision-missing": [],
               "compile.material-missing": []}
    resolved = {}

    def need(relative, code, label):
        data, where = resolver.read(relative)
        if data is None:
            missing[code].append("%s: %s" % (label, relative))
        else:
            resolved[relative] = {"from": where, "sha256": hashlib.sha256(data).hexdigest()}

    for record in report["objects"]:
        rule = profile["model_files"].get(record["role"])
        if rule:
            base = record["model"][:-len(".mdl")]
            required = list(rule["always"])
            required += rule.get("collision:%s" % record.get("collision"), [])
            for suffix in required:
                need(base + suffix, "compile.model-missing" if suffix == ".mdl" else
                     "compile.model-collision-missing", record["id"])
        if record["role"] == "world_solid":
            for surface in record["surfaces"]:
                need("materials/%s.vmt" % surface["material"], "compile.material-missing",
                     "%s surface %d" % (record["id"], surface["surface"]))
    if any(r["role"] == "entity_brush" for r in report["objects"]):
        need("materials/%s.vmt" % profile["brush_entity_material"],
             "compile.material-missing", "brush entities")
    for code, messages in missing.items():
        if messages:
            fail(profile, code, messages)
    return resolved


# ------------------------------------------------------------------ stages

class Build:
    """The stage runner. Each stage gets a private directory, declares its
    input files and JSON parameters, and returns its output files, which must
    lie in its directory. A stage may add the dependencies it resolved (such
    as USD sublayers) to its own inputs. build.json holds the records."""

    def __init__(self, work):
        self.work = Path(work)
        self.stages = []

    def run(self, name, inputs, params, action):
        directory = self.work / "stages" / name
        directory.mkdir(parents=True)
        record = {"stage": name, "params": params,
                  "inputs": {key: {"path": str(path), "sha256": sha256(path)}
                             for key, path in sorted(inputs.items())}}
        self.stages.append(record)
        started = time.monotonic()
        try:
            outputs = action(directory, dict(inputs), params, record)
        except CompileError as error:
            record.update({"status": "fail", "code": error.code,
                           "seconds": round(time.monotonic() - started, 3)})
            raise
        record["outputs"] = {}
        for key, path in sorted(outputs.items()):
            path = Path(path)
            if directory.resolve() not in path.resolve().parents:
                raise RuntimeError("stage %s wrote %s outside its directory" % (name, path))
            record["outputs"][key] = {"path": str(path), "sha256": sha256(path)}
        # The content key a cache would use: every input's hash and the params.
        key = {"stage": name, "params": params,
               "inputs": {k: v["sha256"] for k, v in record["inputs"].items()}}
        record["key"] = hashlib.sha256(json.dumps(key, sort_keys=True).encode()).hexdigest()
        record.update({"status": "pass", "seconds": round(time.monotonic() - started, 3)})
        return outputs

    def record(self):
        return {"schema": BUILD_SCHEMA, "stages": self.stages}


def run_tool(directory, name, command, timeout):
    log = directory / (name + ".log")
    with log.open("w") as handle:
        handle.write("$ " + " ".join(map(str, command)) + "\n")
        handle.flush()
        try:
            completed = subprocess.run([str(c) for c in command], stdout=handle,
                                       stderr=subprocess.STDOUT, timeout=timeout, check=False,
                                       cwd=directory)
            status = completed.returncode
        except subprocess.TimeoutExpired:
            status = "timeout"
    return status, log


def tail(log, lines=12):
    return [line for line in log.read_text(errors="replace").splitlines()[-lines:] if line]


def tool_failed(profile, tool, status, log):
    fail(profile, "compile.tool-failed", ["%s exited %s: %s" % (tool, status,
                                                                " | ".join(tail(log)))])


def stage_validate(directory, inputs, params, record):
    import usd_authoring_validate as validator
    profile = load_compile_profile(inputs["profile"])
    report = validator.validate(str(inputs["stage"]), params["previous"],
                                validator.load_profile(inputs["authoring_profile"]))
    path = directory / "authoring-report.json"
    path.write_text(json.dumps(report, indent=2, sort_keys=True))
    for layer in report["layers"]:
        if os.path.isfile(layer) and Path(layer).resolve() != Path(inputs["stage"]).resolve():
            record["inputs"]["layer:" + layer] = {"path": layer, "sha256": sha256(layer)}
    if report["errors"]:
        fail(profile, "compile.authoring-invalid",
             ["%s %s: %s" % (e["code"], e["path"], e["message"]) for e in report["errors"]])
    return {"report": path}


def runtime_indexes(runtime):
    """The runtime's VPK directory files. Their entries carry each packed
    file's CRC, so their hashes stand for the content the tools can read
    (surface properties, sky, ...) beyond the files a stage resolves itself."""
    import source_content
    return {"vpk:" + relative: Path(runtime) / relative
            for kind, relative in source_content.SEARCH_PATHS
            if kind == "vpk" and (Path(runtime) / relative).is_file()}


def stage_content(directory, inputs, params, record):
    import source_content
    profile = load_compile_profile(inputs["profile"])
    report = json.loads(Path(inputs["report"]).read_text())
    resolved = check_content(report, profile, source_content.ContentResolver(params["runtime"]))
    path = directory / "content.json"
    path.write_text(json.dumps({"runtime": params["runtime"], "files": resolved}, indent=2,
                               sort_keys=True))
    return {"content": path}


def stage_game(directory, inputs, params, record):
    """A private compile game whose search paths are the runtime's content."""
    runtime = Path(params["runtime"])
    paths = [("game+mod", "|gameinfo_path|."),
             ("game", runtime / "portal/portal_pak.vpk"), ("game", runtime / "portal"),
             ("game", runtime / "hl2/hl2_textures.vpk"), ("game", runtime / "hl2/hl2_misc.vpk"),
             ("game", runtime / "hl2"),
             ("platform", runtime / "platform/platform_misc.vpk"),
             ("platform", runtime / "platform")]
    lines = ['"GameInfo"', "{", '\tgame\t"usd_map_compile"', "\tFileSystem", "\t{",
             "\t\tSteamAppId\t400", "\t\tSearchPaths", "\t\t{"]
    lines += ['\t\t\t%s\t"%s"' % (kind, path) for kind, path in paths]
    lines += ["\t\t}", "\t}", "}", ""]
    (directory / "gameinfo.txt").write_text("\n".join(lines))
    (directory / "lights.rad").write_text(EMPTY_LIGHTS_RAD)
    return {"gameinfo": directory / "gameinfo.txt", "lights": directory / "lights.rad"}


def stage_brushset(directory, inputs, params, record):
    profile = load_compile_profile(inputs["profile"])
    report = json.loads(Path(inputs["report"]).read_text())
    brushset, objects = build_brushset(report, profile, params["map"])
    path = directory / (params["map"] + ".json")
    path.write_text(json.dumps(brushset, indent=1))
    mapping = directory / "objects.json"
    mapping.write_text(json.dumps(objects, indent=1, sort_keys=True))
    return {"brushset": path, "objects": mapping}


def stage_vbsp(directory, inputs, params, record):
    profile = load_compile_profile(inputs["profile"])
    name = params["map"]
    brushset = directory / (name + ".json")
    shutil.copyfile(inputs["brushset"], brushset)
    status, log = run_tool(directory, "vbsp",
                           [inputs["vbsp"]] + profile["tools"]["vbsp"] +
                           ["-game", Path(inputs["gameinfo"]).parent, brushset],
                           params["timeout"])
    # The leak is judged from vbsp's outputs as well as its exit status: a
    # leaked tree writes <map>.lin, and a sealed one writes a portal file.
    leaked = ["the world solids do not seal the map: %s" % " | ".join(tail(log, 6))]
    if (directory / (name + ".lin")).exists():
        fail(profile, "compile.leak", leaked)
    if status != 0 or not (directory / (name + ".bsp")).is_file():
        tool_failed(profile, "vbsp", status, log)
    if not (directory / (name + ".prt")).exists():
        fail(profile, "compile.leak", leaked)
    return {"bsp": directory / (name + ".bsp"), "portals": directory / (name + ".prt")}


def tool_stage(tool, extra_inputs=()):
    """vvis or vrad on a private copy of the previous stage's BSP."""
    def action(directory, inputs, params, record):
        profile = load_compile_profile(inputs["profile"])
        bsp = directory / (params["map"] + ".bsp")
        shutil.copyfile(inputs["bsp"], bsp)
        for key in extra_inputs:
            shutil.copyfile(inputs[key], directory / Path(inputs[key]).name)
        status, log = run_tool(directory, tool, [inputs[tool]] + profile["tools"][tool] +
                               ["-game", Path(inputs["gameinfo"]).parent, bsp],
                               params["timeout"])
        if status != 0:
            tool_failed(profile, tool, status, log)
        return {"bsp": bsp}
    return action


def stage_assemble(directory, inputs, params, record):
    profile = load_compile_profile(inputs["profile"])
    content = directory / "content" / "maps"
    content.mkdir(parents=True)
    bsp2 = content / (params["map"] + ".bsp")
    for step, command in (("bsp2-convert", [inputs["bsp2tool"], "convert", inputs["bsp"], bsp2]),
                          ("bsp2-verify", [inputs["bsp2tool"], "verify", bsp2])):
        status, log = run_tool(directory, step, command, params["timeout"])
        if status != 0:
            tool_failed(profile, step, status, log)
    report = json.loads(Path(inputs["report"]).read_text())
    provenance = {"schema": profile["provenance_schema"], "map": params["map"],
                  "compile_profile": {"profile": profile["profile"],
                                      "version": profile["version"]},
                  "authoring_profile": {"profile": report["profile"],
                                        "version": report["version"]},
                  "brushset_schema": profile["brushset_schema"], "stage": report["stage"],
                  "bsp2_sha256": sha256(bsp2), "entity_id_key": profile["entity_id_key"],
                  "objects": json.loads(Path(inputs["objects"]).read_text())}
    path = directory / "provenance.json"
    path.write_text(json.dumps(provenance, indent=2, sort_keys=True) + "\n")
    return {"bsp2": bsp2, "provenance": path}


def check_stage(checker):
    def action(directory, inputs, params, record):
        profile = load_compile_profile(inputs["profile"])
        result = checker(inputs["bsp2"], json.loads(Path(inputs["report"]).read_text()),
                         json.loads(Path(inputs["provenance"]).read_text()), profile,
                         params["runtime"])
        path = directory / "check.json"
        path.write_text(json.dumps(result, indent=2, sort_keys=True))
        record["checks"] = result["checks"]
        if result["failures"]:
            fail(profile, "compile.output-check", result["failures"])
        return {"check": path}
    return action


def compile_stage(stage, name, tools, runtime, work, previous=None, timeout=600,
                  profile_path=COMPILE_PROFILE, checker=None):
    """Build `stage` under the private directory `work` (which must not exist)
    and return {build, bsp2, content_root, provenance, check}. Raises
    CompileError; build.json is written either way. `checker` replaces the
    output checks (seeded-mutant tests only)."""
    import usd_authoring_validate as validator
    import usd_map_check

    tools, runtime = Path(tools).resolve(), str(Path(runtime).resolve())
    work = Path(work)
    work.mkdir(parents=True, exist_ok=False)
    build = Build(work)
    profile = {"profile": Path(profile_path).resolve()}
    timing = {"map": name, "timeout": timeout}
    try:
        validated = build.run(
            "validate", dict(profile, stage=Path(stage).resolve(),
                             authoring_profile=Path(validator.PROFILE_PATH),
                             **({"previous": Path(previous)} if previous and
                                os.path.isfile(previous) else {})),
            {"previous": str(previous) if previous else None}, stage_validate)
        content = build.run("content", dict(profile, report=validated["report"],
                                            **runtime_indexes(runtime)),
                            {"runtime": runtime}, stage_content)
        game = build.run("game", {}, {"runtime": runtime}, stage_game)
        brushset = build.run("brushset", dict(profile, report=validated["report"]),
                             {"map": name}, stage_brushset)
        common = dict(profile, gameinfo=game["gameinfo"], lights=game["lights"],
                      content=content["content"])
        vbsp = build.run("vbsp", dict(common, brushset=brushset["brushset"],
                                      vbsp=tools / "vbsp"), timing, stage_vbsp)
        vvis = build.run("vvis", dict(common, bsp=vbsp["bsp"], portals=vbsp["portals"],
                                      vvis=tools / "vvis"), timing,
                         tool_stage("vvis", ("portals",)))
        vrad = build.run("vrad", dict(common, bsp=vvis["bsp"], vrad=tools / "vrad"), timing,
                         tool_stage("vrad"))
        assembled = build.run("assemble", dict(profile, bsp=vrad["bsp"],
                                               report=validated["report"],
                                               objects=brushset["objects"],
                                               bsp2tool=tools / "bsp2tool"),
                              timing, stage_assemble)
        checked = build.run("check", dict(common, bsp2=assembled["bsp2"],
                                          report=validated["report"],
                                          provenance=assembled["provenance"]),
                            {"runtime": runtime}, check_stage(checker or
                                                              usd_map_check.check_map))
    finally:
        (work / "build.json").write_text(json.dumps(build.record(), indent=2, sort_keys=True))
    return {"build": work / "build.json", "bsp2": assembled["bsp2"],
            "content_root": assembled["bsp2"].parents[1],
            "provenance": json.loads(assembled["provenance"].read_text()),
            "check": json.loads(checked["check"].read_text())}


def publish(result, name, store):
    """The commit: one rename into the store (playable_maps.publish)."""
    import playable_maps
    summary = {"map": name, "status": "pass", "failed_gates": [],
               "bsp2_sha256": result["provenance"]["bsp2_sha256"],
               "content_root": str(result["content_root"])}
    return playable_maps.publish(summary, store, sidecars={
        "provenance.json": json.dumps(result["provenance"], indent=2, sort_keys=True) + "\n",
        "build.json": Path(result["build"]).read_text()})


def published_previous(store, name):
    path = Path(store) / name / "provenance.json"
    return str(path) if path.is_file() else None


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    commands = parser.add_subparsers(dest="command", required=True)
    build = commands.add_parser("compile", help="compile one authored stage")
    build.add_argument("stage")
    build.add_argument("--map", required=True, help="map name (maps/<map>.bsp)")
    build.add_argument("--tools", required=True, type=Path,
                       help="installed tools (vbsp with -authored, vvis, vrad, bsp2tool)")
    build.add_argument("--runtime", required=True, type=Path,
                       help="Portal runtime whose content the map uses (read only)")
    build.add_argument("--store", type=Path, help="published-map store (default run/maps)")
    build.add_argument("--work", type=Path, help="keep the build here (default: temporary)")
    build.add_argument("--previous", help="earlier stage, report or provenance for id "
                       "stability (default: the published package's provenance)")
    build.add_argument("--no-previous", action="store_true")
    build.add_argument("--no-publish", action="store_true")
    build.add_argument("--json", type=Path, help="write the build outcome here")
    args = parser.parse_args(argv)

    import playable_maps
    store = args.store or playable_maps.STORE
    previous = None if args.no_previous else (args.previous or published_previous(store, args.map))
    scratch = None
    if args.work:
        work = args.work
    else:
        scratch = tempfile.mkdtemp(prefix="usd-map-compile-")
        work = Path(scratch) / "work"
    outcome = {"schema": "source-usd-map-outcome/v1", "map": args.map, "stage": args.stage,
               "previous": previous, "work": str(work)}
    status = 1
    try:
        result = compile_stage(args.stage, args.map, args.tools, args.runtime, work, previous)
        outcome.update({"status": "pass", "bsp2_sha256": result["provenance"]["bsp2_sha256"],
                        "checks": result["check"]["checks"]})
        if not args.no_publish:
            publish(result, args.map, store)
            outcome["published"] = str(Path(store) / args.map)
            print("published %s; play it with ./play %s" % (outcome["published"], args.map))
        print("compiled %s: %d output checks passed" % (args.map, result["check"]["checks"]))
        status = 0
    except CompileError as error:
        outcome.update({"status": "fail", "code": error.code, "messages": error.messages})
        print("FAIL %s" % error.code)
        for message in error.messages:
            print("  " + message)
    finally:
        if args.json:
            args.json.write_text(json.dumps(outcome, indent=2, sort_keys=True, default=str) + "\n")
        if scratch and status == 0:
            shutil.rmtree(scratch, ignore_errors=True)
        elif scratch:
            print("build kept for inspection: %s" % work)
    return status


if __name__ == "__main__":
    sys.exit(main())
