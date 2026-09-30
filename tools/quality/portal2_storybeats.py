#!/usr/bin/env python3
"""Portal 2 story-beat walkthroughs on the player's own input, this build vs retail.

quality/workloads/portal2-storybeats-v1 (sp_a1_wakeup) and
portal2-storybeats-laser-v1 (sp_a2_laser_intro, solved with the portal gun)
play a map from start to level change: the scenario script names the story
beats (qa_beats.nut), and qa_walker.nut walks the player between them. The walker plans each walk with A* over a grid it
measures in the running game (TraceLine), and drives the player only through
console input: +forward, +left/+right with cl_yawspeed, +lookup/+lookdown
with cl_pitchspeed, +jump. Nothing sets the player's position or angles. At
each beat it takes a named screenshot (QA_SHOT).

TraceLine does not see player clips, nor any brush entity that moves (the
trace filter skips MOVETYPE_PUSH entities for masks without
CONTENTS_MOVEABLE): doors, trains, lifts. The walker needs both to stand on a
lift or stop at a door, so this tool extracts the map's world player-clip
brushes and its solid brush entities' brushes from the BSP in the Portal 2
installation and installs them as qa/qa_clips.nut next to the scripts; the
walker places each entity's brushes at its current origin. They are derived
at run time: nothing from the map is stored in the repository.

A scenario may run on a published derivative of a shipped map (the map
pipeline's run/maps, mounted as ./play_p2 mounts it): "map" names the
published map this build plays, "retail_map" the shipped map retail plays
and whose BSP gives the collision. A relit map keeps every gameplay lump
byte-identical, so both sides play the same chamber.

  capture --side build   runs the scenarios on a Waf tree (SDL offscreen,
                         native Vulkan) in a private staged runtime
  capture --side retail  runs the same scripts on retail portal2_linux in an
                         isolated headless compositor (mutter) through a
                         mapspawn.nut hook; retail needs a Steam client, which
                         the session starts and stops if none is running
  sheet                  pairs the two captures' shots by name into one
                         side-by-side contact sheet, and draws both routes
                         over each other (top-down)
  check                  judges a build capture against a retail capture, or
                         against the recorded reference, and prints one
                         checks-v1 record
  record                 writes the reference (quality/workloads/.../
                         reference.json) from a retail capture: the beats in
                         order and where retail's player stood and looked at
                         each. Numbers only; no retail pixels are stored.
  suite                  capture --side build, then check against the
                         reference (the manifest row)
  clips                  prints the collision script for a map (diagnosis)

Checks (per scenario):

  <scenario>.<side>.run        the scenario ran to QA_DONE with every required
                               check PASS (every walk arrived, every story
                               trigger fired, every view was reached)
  <scenario>.<side>.shots      a screenshot for every beat, in order
  <scenario>.beats             both sides shot the same beats
  <scenario>.<shot>.position   the player stood within 96 units (horizontally)
                               and 160 units (in height: a shot in free fall
                               is a tick or two apart) of retail's position
                               when the shot was taken

Both sides render at 1024x768 (the SDL offscreen driver's largest mode) with
sv_cheats 1 before the map loads, so nothing prints a cheat notice over a
frame, and with keyboard look (+cl_mouselook 0 +joystick 0; cl_mouselook
cannot change while connected). Retail content and binaries are not in the repository.
"""

import argparse
import datetime
import json
import math
import os
from pathlib import Path
import re
import struct
import subprocess
import sys
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import conformance  # noqa: E402
import conformance_result  # noqa: E402
import legacy_bsp  # noqa: E402
import playable_maps  # noqa: E402
import portal2_material_shots as shots  # noqa: E402
import portal2_scenarios  # noqa: E402
import private_session  # noqa: E402
import stage_portal2_runtime  # noqa: E402


ROOT = Path(conformance.repo_root())
WORKLOAD = ROOT / "quality/workloads/portal2-storybeats-v1"
CLIP_SCRIPT = "qa_clips.nut"
REFERENCE_SCHEMA = "portal2-storybeats-reference/v1"
# public/bspflags.h. A world brush the walker's TraceLine
# (MASK_NPCWORLDSTATIC) already sees needs no clip record.
CONTENTS_PLAYERCLIP = 0x10000
TRACED_CONTENTS = 0x1 | 0x2 | 0x8 | 0x20000  # solid, window, grate, monster clip
# MASK_PLAYERSOLID's brush contents: solid, window, grate, moveable, player clip.
PLAYER_SOLID = 0x1 | 0x2 | 0x8 | 0x4000 | 0x10000
SHOT_LOG = re.compile(r"^QA_LOG (\S+) t=\S+ shot (\S+) feet \((\S+) (\S+) (\S+)\) "
                      r"pitch (\S+) yaw (\S+)\s*$")
PATH_LINE = re.compile(r"^QA_PATH (\S+) t=(\S+) (\S+) (\S+) (\S+) (\S+) (\S+) (\S+)(?: .*)?$")
# Where the player stood at a beat, against retail: horizontally, and
# vertically (a beat in free fall is a tick or two apart in height).
POSITION_TOLERANCE = 96.0
HEIGHT_TOLERANCE = 160.0
# The walker's keyboard look, on both sides before the map loads:
# cl_mouselook cannot change while connected (FCVAR_NOT_CONNECTED), and
# keyboard pitch needs it 0; a connected gamepad's absolute look axis would
# set the pitch every frame.
WALKER_ENGINE_ARGS = ["+cl_mouselook", "0", "+joystick", "0"]
# Retail runs its user config (the mirror's own cfg/config.cfg; see
# stage_portal2_runtime.RETAIL_ENGINE_ARGS) at every map start, after the
# command line: the walker's settings go into that file too.
RETAIL_WALKER_CONFIG = {"cl_mouselook": "0", "cl_mouselook2": "0", "joystick": "0"}


class BeatError(Exception):
    pass


def now_iso():
    return datetime.datetime.now(datetime.timezone.utc).isoformat()


# ---------------------------------------------------------------------------
# Collision TraceLine does not see
# ---------------------------------------------------------------------------

def model_brushes(bsp, model):
    """Brush indices reachable from a model's BSP tree."""
    nodes = bsp.lump(legacy_bsp.LUMP_NODES, legacy_bsp.NODE_BYTES)
    leaves = bsp.lump(legacy_bsp.LUMP_LEAFS, legacy_bsp.LEAF_BYTES)
    leafbrushes = bsp.lump(legacy_bsp.LUMP_LEAFBRUSHES, 2)
    node_count = len(nodes) // legacy_bsp.NODE_BYTES
    found, stack, visited = set(), [bsp.models()[model]["headnode"]], 0
    while stack:
        node = stack.pop()
        visited += 1
        if visited > node_count + len(leaves) // legacy_bsp.LEAF_BYTES:
            raise ValueError("BSP node tree has a cycle")
        if node < 0:
            first, count = struct.unpack_from("<HH", leaves, (-1 - node) * legacy_bsp.LEAF_BYTES + 24)
            found.update(struct.unpack_from("<%dH" % count, leafbrushes, first * 2))
            continue
        stack.extend(struct.unpack_from("<ii", nodes, node * legacy_bsp.NODE_BYTES + 4))
    return sorted(found)


def brush_record(bsp, normals, dists, index):
    """[lo, hi, planes]: a brush's bounds and its planes (n, d; solid where
    n . x <= d), in the coordinates the BSP stores it in."""
    brushes = bsp.lump(legacy_bsp.LUMP_BRUSHES, legacy_bsp.BRUSH_BYTES)
    sides = bsp.lump(legacy_bsp.LUMP_BRUSHSIDES, legacy_bsp.BRUSHSIDE_BYTES)
    first, count, contents = struct.unpack_from("<3i", brushes, index * legacy_bsp.BRUSH_BYTES)
    brush = {"contents": contents,
             "sides": [struct.unpack_from("<Hhhh", sides, i * legacy_bsp.BRUSHSIDE_BYTES)
                       for i in range(first, first + count)]}
    points = [p for _side, winding in bsp.brush_side_polygons(brush) for p in winding]
    if not points:
        return contents, None
    lo = [min(p[a] for p in points) for a in range(3)]
    hi = [max(p[a] for p in points) for a in range(3)]
    planes = [(tuple(float(v) for v in normals[plane]), float(dists[plane]))
              for plane, _texinfo, _disp, _bevel in brush["sides"]]
    return contents, (lo, hi, planes)


def toggled_names(entities):
    """Lower-case names of entities some output switches on or off. The
    walker cannot see whether such a func_brush is on, so it leaves them to
    its stuck detection."""
    names = set()
    for entity in entities:
        for key, value in entity.items():
            if not key.startswith("On"):
                continue
            for action in value if isinstance(value, list) else [value]:
                parts = re.split("[\x1b,]", action)
                if len(parts) >= 2 and parts[1].lower() in ("enable", "disable", "toggle"):
                    names.add(parts[0].lower())
    return names


def is_mover(entity, toggled):
    """Brush entities the player collides with that TraceLine skips: every
    MOVETYPE_PUSH brush (doors, trains, func_brush) is left out by the trace
    filter's standard rules unless the mask has CONTENTS_MOVEABLE."""
    classname = entity.get("classname", "")
    spawnflags = int(entity.get("spawnflags", "0") or 0)
    if classname in ("func_door", "func_door_rotating"):
        return not spawnflags & 4           # non-solid to the player
    if classname in ("func_tracktrain", "func_train"):
        return not spawnflags & 8           # passable
    if classname == "func_brush":
        # Solidity 1 is never solid; one that starts disabled is a blocker
        # the map switches on behind the player.
        return entity.get("Solidity", "0") != "1" and entity.get("StartDisabled", "0") != "1" \
            and entity.get("targetname", "").lower() not in toggled
    return classname in ("func_movelinear", "func_plat", "func_platrot", "func_rotating",
                         "func_wall_toggle")


def collision_data(bsp_path):
    """World player clips and mover brushes, as walker data."""
    bsp = legacy_bsp.LegacyBsp.read(bsp_path)
    normals, dists = bsp.planes()
    clips = []
    for brush in bsp.world_brushes():
        if brush["contents"] & CONTENTS_PLAYERCLIP and not brush["contents"] & TRACED_CONTENTS:
            _contents, record = brush_record(bsp, normals, dists, brush["index"])
            if record:
                clips.append(record)
    movers = []
    entities = bsp.entities()
    toggled = toggled_names(entities)
    for entity in entities:
        model = entity.get("model", "")
        if not model.startswith("*") or not is_mover(entity, toggled):
            continue
        records = []
        for index in model_brushes(bsp, int(model[1:])):
            contents, record = brush_record(bsp, normals, dists, index)
            if record and contents & PLAYER_SOLID:
                records.append(record)
        if records:
            origin = [float(v) for v in entity.get("origin", "0 0 0").split()]
            movers.append((entity.get("targetname", ""), entity["classname"], origin, records))
    return clips, movers


def format_record(record):
    lo, hi, planes = record
    values = ", ".join("%.5g, %.5g, %.5g, %.3f" % (n[0], n[1], n[2], d) for n, d in planes)
    return "[ %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, [ %s ] ]" % (*lo, *hi, values)


def clip_script(bsp_path, map_name, fault=None):
    clips, movers = collision_data(bsp_path)
    # Negative controls: the walker without the collision TraceLine misses.
    if fault == "no-clips":
        clips = []
    if fault == "no-movers":
        movers = []
    lines = ["// Generated by tools/quality/portal2_storybeats.py from %s.bsp: collision" % map_name,
             "// the player has and TraceLine does not see (qa_walker.nut). Brushes are",
             "// [ bounds lo, bounds hi, [ planes nx ny nz d ... ] ], solid where n . x <= d.",
             "// World player clips:",
             "::WALK_CLIPS <- ["]
    lines += ["\t%s," % format_record(record) for record in clips]
    lines += ["]", "// Brush entities (doors, trains, func_brush) in model coordinates, placed",
              "// at the entity's current origin: [ name, class, spawn origin, brushes ].",
              "::WALK_MOVERS <- ["]
    for name, classname, origin, records in movers:
        lines.append("\t[ \"%s\", \"%s\", Vector( %.2f, %.2f, %.2f ), [" % (
            name, classname, *origin))
        lines += ["\t\t%s," % format_record(record) for record in records]
        lines.append("\t] ],")
    lines.append("]")
    return "\n".join(lines) + "\n", len(clips) + sum(len(m[3]) for m in movers)


def map_bsp(steam_root, map_name):
    for directory in ("update/maps", "portal2/maps", "portal2_dlc1/maps", "portal2_dlc2/maps"):
        path = Path(steam_root) / directory / (map_name + ".bsp")
        if path.is_file():
            return path
    raise BeatError("no %s.bsp in %s" % (map_name, steam_root))


def install_clips(steam_root, scenarios, qa_directory, fault=None):
    # A published derivative (a relit map) keeps the shipped map's gameplay
    # lumps byte-identical, so its collision comes from the shipped BSP.
    maps = sorted({portal2_scenarios.retail_map(s) for s in scenarios})
    if len(maps) != 1:
        # qa_clips.nut is one file under qa/; a run covers one map.
        raise BeatError("a capture runs one map at a time, not %s" % ", ".join(maps))
    text, count = clip_script(map_bsp(steam_root, maps[0]), maps[0], fault)
    Path(qa_directory).mkdir(parents=True, exist_ok=True)
    (Path(qa_directory) / CLIP_SCRIPT).write_text(text)
    return count


# ---------------------------------------------------------------------------
# Capture
# ---------------------------------------------------------------------------

def capture_build(args, workload, scenarios):
    out = Path(args.out).resolve()
    extra = shots.CAPTURE_ENGINE_ARGS + shots.PLAY_P2_ENGINE_ARGS + WALKER_ENGINE_ARGS + \
        list(args.extra_arg)
    capture = {"schema": shots.CAPTURE_SCHEMA, "side": "build", "status": "incomplete",
               "started_utc": now_iso(), "source": conformance.source_identity(str(ROOT)),
               "build": str(args.build), "extra_args": extra,
               "selected": [s["name"] for s in scenarios], "scenarios": {}}
    runtime = Path(args.runtime).resolve()
    published = [s["map"] for s in scenarios if s["map"] != portal2_scenarios.retail_map(s)]
    stage_portal2_runtime.stage_content(args.steam_root, runtime, mount_custom=bool(published))
    if published:
        # The map pipeline's published maps (run/maps), mounted as ./play_p2
        # mounts them: portal2/custom/pbrt-<map>.
        mounted, skipped = playable_maps.mount(runtime, game="portal2")
        missing = [name for name in published if name not in mounted]
        if missing:
            raise BeatError("published map not mounted: %s" % "; ".join(
                "%s (%s)" % (name, skipped.get(name, "not published")) for name in missing))
        capture["published_maps"] = {name: {key: mounted[name].get(key) for key in (
            "status", "failed_gates", "bsp2_sha256", "published")} for name in published}
    capture["installed"] = stage_portal2_runtime.portal_boot.install_build(
        args.build, runtime, game="portal2")
    portal2_scenarios.install_scripts(args.workload, workload, runtime)
    capture["seed_fault"] = getattr(args, "seed_fault", None)
    capture["player_clips"] = install_clips(
        args.steam_root, scenarios,
        runtime / "portal2/scripts/vscripts" / portal2_scenarios.SCRIPT_DIRECTORY,
        capture["seed_fault"])
    tools = out / "tools"
    portal2_scenarios.write_fake_zenity(tools)
    screenshots = runtime / "portal2/screenshots"
    for scenario in scenarios:
        name = scenario["name"]
        print("== build %s" % name, flush=True)
        started = time.time()
        result = portal2_scenarios.run_scenario(
            scenario, runtime, out / name, args.start_frames, shots.WIDTH, shots.HEIGHT, tools,
            extra_args=extra)
        record = shots.finish_scenario(out / name, name, result, screenshots, started,
                                       (out / name / "stdout.log").read_text(errors="replace"))
        capture["scenarios"][name] = record
        shots.write_capture(out, capture)
        print_record(record)
    capture["status"] = "complete"
    capture["finished_utc"] = now_iso()
    shots.write_capture(out, capture)
    return capture


def capture_retail(args, workload, scenarios):
    """Installs the scripts and clips in a retail mirror, then runs
    portal2_material_shots' retail session (Steam, the hook, the game) inside
    an isolated headless compositor."""
    out = Path(args.out).resolve()
    mirror = shots.make_retail_mirror(args.steam_root, args.mirror)
    shots.install_retail_scripts(args.workload, workload, mirror)
    set_retail_config(mirror)
    count = install_clips(args.steam_root, scenarios,
                          mirror / "portal2/scripts/vscripts" / portal2_scenarios.SCRIPT_DIRECTORY)
    inner = [sys.executable, os.path.abspath(shots.__file__), "_retail-session", "--out",
             str(out), "--mirror", str(mirror), "--workload", str(Path(args.workload).resolve())]
    for scenario in scenarios:
        inner += ["--scenario", scenario["name"]]
    inner += ["--engine-arg=" + arg for arg in WALKER_ENGINE_ARGS]
    config = out / "compositor-config"
    config.mkdir(parents=True, exist_ok=True)
    environment = dict(os.environ, XDG_CONFIG_HOME=str(config))
    for variable in ("DISPLAY", "WAYLAND_DISPLAY"):
        environment.pop(variable, None)
    command = private_session.dbus_run_session(out / "dbus") + [
        "mutter", "--headless", "--wayland", "--virtual-monitor", "1920x1080@60",
        "--wayland-display", "p2-storybeats-%d" % os.getpid(), "--"] + inner
    print("== retail %s" % ", ".join(s["name"] for s in scenarios), flush=True)
    with (out / "compositor.log").open("wb") as log:
        process = subprocess.run(command, env=environment, stdout=log,
                                 stderr=subprocess.STDOUT, timeout=args.session_timeout)
    path = out / "capture.json"
    if not path.is_file():
        raise BeatError("the retail session wrote no capture (exit %d); see %s"
                        % (process.returncode, out / "compositor.log"))
    capture = json.loads(path.read_text())
    capture["player_clips"] = count
    shots.write_capture(out, capture)
    for record in capture["scenarios"].values():
        print_record(record)
    return capture


def set_retail_config(mirror):
    """Writes the walker's input settings into the mirror's own user configs
    (update/ is searched first); refuses a config outside the mirror."""
    mirror = Path(mirror).resolve()
    for relative in ("update/cfg/config.cfg", "portal2/cfg/config.cfg"):
        path = mirror / relative
        if mirror not in path.parent.resolve().parents or path.is_symlink():
            raise BeatError("%s is not the mirror's own file" % path)
        lines = path.read_text(errors="replace").splitlines() if path.is_file() else []
        lines = [line for line in lines if line.split(" ", 1)[0] not in RETAIL_WALKER_CONFIG]
        lines += ['%s "%s"' % item for item in RETAIL_WALKER_CONFIG.items()]
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text("\n".join(lines) + "\n")


def print_record(record):
    for check, value in sorted(record.get("checks", {}).items()):
        print("  %-4s %s %s" % (value["outcome"], check, value["detail"][:140]))
    for failure in record.get("failures", []):
        print("  FAILURE " + failure[:200])
    print("  %s: %d shots%s" % (record["status"], len(record.get("shots", {})),
                                "" if not record.get("error") else " (%s)" % record["error"]),
          flush=True)


# ---------------------------------------------------------------------------
# Reading a capture
# ---------------------------------------------------------------------------

def load_capture(directory, side):
    directory = Path(directory).resolve()
    path = directory / "capture.json"
    if not path.is_file():
        raise BeatError("%s holds no capture" % directory)
    capture = json.loads(path.read_text())
    if capture.get("side") != side:
        raise BeatError("%s is a %s capture, not %s" % (directory, capture.get("side"), side))
    return directory, capture


def shot_positions(directory, scenario):
    log = (Path(directory) / scenario / "console.log").read_text(errors="replace")
    positions = {}
    for match in map(SHOT_LOG.match, log.splitlines()):
        if match and match.group(1) == scenario:
            positions[match.group(2)] = tuple(float(match.group(i)) for i in range(3, 8))
    return positions


def route(directory, scenario):
    log = (Path(directory) / scenario / "console.log").read_text(errors="replace")
    return [tuple(float(m.group(i)) for i in range(2, 8))
            for m in map(PATH_LINE.match, log.splitlines()) if m and m.group(1) == scenario]


# ---------------------------------------------------------------------------
# Sheet
# ---------------------------------------------------------------------------

def sheet(args):
    from PIL import Image, ImageDraw
    build_dir, build = load_capture(args.build_capture, "build")
    retail_dir, retail = load_capture(args.retail_capture, "retail")
    out = Path(args.out)
    out.mkdir(parents=True, exist_ok=True)
    thumb = (512, 384)
    for scenario in sorted(set(build["scenarios"]) & set(retail["scenarios"])):
        names = list(retail["scenarios"][scenario].get("shots", {}))
        extra = [n for n in build["scenarios"][scenario].get("shots", {}) if n not in names]
        names += extra
        label_h = 22
        image = Image.new("RGB", (thumb[0] * 2 + 12, (thumb[1] + label_h) * len(names) + 30),
                          (24, 24, 24))
        draw = ImageDraw.Draw(image)
        draw.text((6, 8), "%s   retail (left)   this build (right)" % scenario,
                  fill=(230, 230, 230))
        for row, name in enumerate(names):
            y = 30 + row * (thumb[1] + label_h)
            draw.text((6, y + 4), name, fill=(230, 230, 120))
            for column, (directory, capture) in enumerate(((retail_dir, retail),
                                                           (build_dir, build))):
                record = capture["scenarios"][scenario].get("shots", {}).get(name)
                x = column * (thumb[0] + 12)
                if record is None:
                    draw.text((x + 200, y + label_h + 180), "missing", fill=(255, 80, 80))
                    continue
                shot = Image.open(directory / scenario / "shots" / record["file"]).convert("RGB")
                image.paste(shot.resize(thumb, Image.BILINEAR), (x, y + label_h))
        path = out / ("%s-sheet.png" % scenario)
        image.save(path)
        print("wrote %s (%d beats)" % (path, len(names)))
        draw_routes(out / ("%s-route.png" % scenario), scenario,
                    route(retail_dir, scenario), route(build_dir, scenario),
                    shot_positions(retail_dir, scenario), shot_positions(build_dir, scenario))
    return 0


def draw_routes(path, scenario, retail_route, build_route, retail_shots, build_shots):
    from PIL import Image, ImageDraw
    points = [(p[1], p[2]) for p in retail_route + build_route]
    if not points:
        return
    x0, x1 = min(p[0] for p in points) - 64, max(p[0] for p in points) + 64
    y0, y1 = min(p[1] for p in points) - 64, max(p[1] for p in points) + 64
    scale = min(1600.0 / (x1 - x0), 900.0 / (y1 - y0))
    image = Image.new("RGB", (int((x1 - x0) * scale) + 1, int((y1 - y0) * scale) + 40), (16, 16, 16))
    draw = ImageDraw.Draw(image)

    def xy(x, y):
        return ((x - x0) * scale, 40 + (y1 - y) * scale)

    for name, colour, trace, marks in (("retail", (80, 160, 255), retail_route, retail_shots),
                                       ("build", (255, 140, 60), build_route, build_shots)):
        line = [xy(p[1], p[2]) for p in trace]
        if len(line) > 1:
            draw.line(line, fill=colour, width=2)
        for label, (x, y, _z, _pitch, _yaw) in marks.items():
            cx, cy = xy(x, y)
            draw.ellipse([cx - 4, cy - 4, cx + 4, cy + 4], outline=colour, width=2)
            if name == "retail":
                draw.text((cx + 6, cy - 6), label.split("_", 1)[0], fill=(220, 220, 220))
    draw.text((8, 8), "%s top-down route: retail (blue), this build (orange); circles are shots"
              % scenario, fill=(230, 230, 230))
    image.save(path)
    print("wrote %s" % path)


# ---------------------------------------------------------------------------
# Check
# ---------------------------------------------------------------------------

def side_beats(results, name, side, directory, capture):
    """Reports <scenario>.<side>.run and .shots; returns (shot names, positions)."""
    record = capture["scenarios"].get(name)
    if record is None:
        results.report("%s.%s.run" % (name, side), False, "not captured")
        return None
    failed = [c for c, v in record.get("checks", {}).items() if v["outcome"] != "PASS"]
    results.report("%s.%s.run" % (name, side), record["status"] == "pass",
                   "%d checks, failed %s; %s" % (len(record.get("checks", {})), failed or "none",
                                                 "; ".join(record.get("failures", []))[:300]))
    names = list(record.get("shots", {}))
    positions = shot_positions(directory, name)
    results.report("%s.%s.shots" % (name, side),
                   not record.get("error") and names == sorted(positions) and len(names) > 0,
                   "%d shots%s" % (len(names), "; " + record["error"] if record.get("error") else ""))
    return names, positions


def compare_beats(results, name, retail, build):
    (retail_names, retail_positions), (build_names, build_positions) = retail, build
    results.report("%s.beats" % name, retail_names == build_names,
                   "retail %d, build %d; only retail %s, only build %s" % (
                       len(retail_names), len(build_names),
                       sorted(set(retail_names) - set(build_names)) or "none",
                       sorted(set(build_names) - set(retail_names)) or "none"))
    for shot in retail_names:
        if shot not in build_positions or shot not in retail_positions:
            results.report("%s.%s.position" % (name, shot), False, "no position logged")
            continue
        r, b = retail_positions[shot], build_positions[shot]
        across, height = math.dist(r[:2], b[:2]), abs(r[2] - b[2])
        results.report("%s.%s.position" % (name, shot),
                       across <= POSITION_TOLERANCE and height <= HEIGHT_TOLERANCE,
                       "retail (%.0f %.0f %.0f) build (%.0f %.0f %.0f): %.0f apart, %.0f in height"
                       % (*r[:3], *b[:3], across, height))


def check(args):
    results = shots.Checks(args.only)
    build_dir, build = load_capture(args.build_capture, "build")
    reference = None
    if args.retail_capture:
        retail_dir, retail = load_capture(args.retail_capture, "retail")
    else:
        reference = json.loads(Path(args.reference).read_text())
        if reference.get("schema") != REFERENCE_SCHEMA:
            raise BeatError("%s is not a %s reference" % (args.reference, REFERENCE_SCHEMA))
    workload = portal2_scenarios.load_workload(args.workload)
    for scenario in workload["scenarios"]:
        name = scenario["name"]
        if name not in build["scenarios"]:
            continue
        build_beats = side_beats(results, name, "build", build_dir, build)
        if reference is not None:
            entry = reference["scenarios"].get(name)
            if entry is None:
                results.report("%s.beats" % name, False, "no reference")
                continue
            retail_beats = ([b["shot"] for b in entry["beats"]],
                            {b["shot"]: tuple(b["feet"] + b["view"]) for b in entry["beats"]})
        else:
            retail_beats = side_beats(results, name, "retail", retail_dir, retail)
        if build_beats and retail_beats:
            compare_beats(results, name, retail_beats, build_beats)
    return results.finish()


def record_reference(args):
    retail_dir, retail = load_capture(args.retail_capture, "retail")
    if retail.get("status") != "complete":
        raise BeatError("%s is not a complete retail capture" % retail_dir)
    path = Path(args.reference)
    reference = json.loads(path.read_text()) if path.is_file() else {
        "schema": REFERENCE_SCHEMA, "scenarios": {}}
    reference["recorded_utc"] = now_iso()
    for name, record in retail["scenarios"].items():
        if record["status"] != "pass":
            raise BeatError("retail %s did not pass; nothing recorded" % name)
        positions = shot_positions(retail_dir, name)
        reference["scenarios"][name] = {
            "retail_capture_started_utc": retail["started_utc"],
            "beats": [{"shot": shot, "feet": [round(v, 1) for v in positions[shot][:3]],
                       "view": [round(v, 1) for v in positions[shot][3:]]}
                      for shot in record["shots"]]}
    path.write_text(json.dumps(reference, indent=2) + "\n")
    print("wrote %s" % path)
    return 0


# ---------------------------------------------------------------------------
# Self-test: the comparator on synthetic captures
# ---------------------------------------------------------------------------

def synthetic_capture(root, side, beats, status="pass", drop=None, move=None):
    """A capture directory as capture_* writes it: capture.json and the
    scenario's console.log with its shot lines."""
    name = "synthetic_walk"
    directory = Path(root) / side
    (directory / name).mkdir(parents=True)
    lines, shots_record = [], {}
    for index, (shot, feet) in enumerate(beats):
        label = "%02d_%s" % (index + 1, shot)
        if shot == drop:
            continue
        x, y, z = feet
        if shot == move:
            x += 200.0
        lines.append("QA_SHOT %s %s" % (name, label))
        lines.append("QA_LOG %s t=%d shot %s feet (%.1f %.1f %.1f) pitch 0.0 yaw 90.0"
                     % (name, index, label, x, y, z))
        lines.append("QA_PATH %s t=%d.00 %.1f %.1f %.1f 90.0 0.0 walking body=90.0"
                     % (name, index, x, y, z))
        shots_record[label] = {"file": label + ".png", "size": [shots.WIDTH, shots.HEIGHT]}
    (directory / name / "console.log").write_text("\n".join(lines) + "\n")
    capture = {"schema": shots.CAPTURE_SCHEMA, "side": side, "status": "complete",
               "started_utc": now_iso(), "scenarios": {name: {
                   "status": status, "failures": [] if status == "pass" else ["seeded"],
                   "checks": {"walk.reached": {"outcome": "PASS" if status == "pass" else "FAIL",
                                               "detail": ""}},
                   "shots": shots_record}}}
    (directory / "capture.json").write_text(json.dumps(capture))
    return directory


def self_test(args):
    """The clean pair must pass the check and every seeded defect fail it."""
    import io
    import contextlib
    import tempfile
    beats = [("arrival", (0.0, 0.0, 0.0)), ("hall", (400.0, 0.0, 0.0)),
             ("drop", (400.0, 300.0, -500.0))]
    workload = {"schema": portal2_scenarios.SCHEMA, "driver": "driver.nut", "scenarios": [
        {"name": "synthetic_walk", "map": "sp_a1_wakeup", "script": "walk.nut",
         "timeout_seconds": 10, "required_checks": ["walk.reached"]}]}
    cases = {"clean": {}, "missing-beat": {"drop": "hall"}, "moved-beat": {"move": "drop"},
             "failed-run": {"status": "fail"}}
    # With --seed-fault the clean pair carries that defect and must fail (a
    # manifest sensitivity row); without it every case runs and is judged.
    if args.seed_fault:
        cases = {"clean": cases[args.seed_fault]}
    results = shots.Checks()
    for case in cases:
        with tempfile.TemporaryDirectory() as root:
            (Path(root) / "driver.nut").write_text("")
            (Path(root) / "walk.nut").write_text("")
            (Path(root) / "scenarios.json").write_text(json.dumps(workload))
            retail = synthetic_capture(root, "retail", beats)
            build = synthetic_capture(root, "build", beats, **cases[case])
            quiet = io.StringIO()
            with contextlib.redirect_stdout(quiet):
                code = check(argparse.Namespace(
                    build_capture=build, retail_capture=retail, reference=None,
                    workload=Path(root) / "scenarios.json", only=None))
            failed = [line for line in quiet.getvalue().splitlines() if line.startswith("FAIL")]
            expected_pass = case == "clean"  # the others carry a seeded defect
            results.report("self_test.%s" % case, (code == 0) == expected_pass,
                           "check %s%s" % ("passed" if code == 0 else "failed",
                                           ": " + "; ".join(failed)[:200] if failed else ""))
    return results.finish()


# ---------------------------------------------------------------------------

def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = parser.add_subparsers(dest="command", required=True)
    steam = Path(os.environ.get("SOURCE_PORTAL2_STEAM_ROOT") or
                 os.environ.get("P2_STEAM_ROOT") or shots.DEFAULT_STEAM_ROOT)

    p = sub.add_parser("capture", help="play the scenarios on this build or on retail")
    p.add_argument("--side", choices=("build", "retail"), required=True)
    p.add_argument("--workload", type=Path, default=WORKLOAD / "scenarios.json")
    p.add_argument("--scenario", action="append", default=[],
                   help="only this scenario (repeatable)")
    p.add_argument("--out", type=Path, required=True, help="new capture directory")
    p.add_argument("--steam-root", type=Path, default=steam)
    p.add_argument("--build", type=Path,
                   default=Path(os.environ.get("SOURCE_PORTAL2_BUILD") or ROOT / "build-p2"))
    p.add_argument("--runtime", type=Path, default=ROOT / "run/runtime-p2-storybeats")
    p.add_argument("--mirror", type=Path, default=ROOT / "run/retail-p2-storybeats")
    p.add_argument("--start-frames", type=int, default=300)
    p.add_argument("--session-timeout", type=int, default=3600)
    p.add_argument("--extra-arg", action="append", default=[],
                   help="build side: extra engine argument before +map (repeatable)")
    p.add_argument("--seed-fault", choices=("no-clips", "no-movers"),
                   help="build side, negative control: install no player clips, or no "
                        "brush entities, for the walker; the walk must then fail")

    p = sub.add_parser("sheet", help="side-by-side shots and routes")
    p.add_argument("--build-capture", type=Path, required=True)
    p.add_argument("--retail-capture", type=Path, required=True)
    p.add_argument("--out", type=Path, required=True)

    p = sub.add_parser("check", help="judge a build capture")
    p.add_argument("--build-capture", type=Path, required=True)
    p.add_argument("--retail-capture", type=Path,
                   help="judge against this retail capture instead of the reference")
    p.add_argument("--reference", type=Path,
                   help="default: reference.json next to the workload")
    p.add_argument("--workload", type=Path, default=WORKLOAD / "scenarios.json")
    p.add_argument("--only", help="count only the checks this regular expression matches")

    p = sub.add_parser("record", help="write the reference from a retail capture")
    p.add_argument("--retail-capture", type=Path, required=True)
    p.add_argument("--workload", type=Path, default=WORKLOAD / "scenarios.json")
    p.add_argument("--reference", type=Path,
                   help="default: reference.json next to the workload")

    p = sub.add_parser("suite", help="capture this build, then check (manifest row)")
    p.add_argument("--workload", type=Path, default=WORKLOAD / "scenarios.json")
    p.add_argument("--reference", type=Path,
                   help="default: reference.json next to the workload")
    p.add_argument("--out", type=Path, help="capture directory (default: $CONFORMANCE_OUT or "
                                            "quality-results/portal2-storybeats-<time>)")
    p.add_argument("--steam-root", type=Path, default=steam)
    p.add_argument("--build", type=Path,
                   default=Path(os.environ.get("SOURCE_PORTAL2_BUILD") or ROOT / "build-p2"))
    p.add_argument("--runtime", type=Path, default=ROOT / "run/runtime-p2-storybeats")
    p.add_argument("--start-frames", type=int, default=300)
    p.add_argument("--extra-arg", action="append", default=[])
    p.add_argument("--only", help="count only the checks this regular expression matches")

    p = sub.add_parser("self-test", help="the comparator on synthetic captures")
    p.add_argument("--seed-fault", choices=("missing-beat", "moved-beat", "failed-run"),
                   help="run only this seeded case")

    p = sub.add_parser("clips", help="print a map's player-clip script")
    p.add_argument("map")
    p.add_argument("--steam-root", type=Path, default=steam)

    args = parser.parse_args(argv)
    if getattr(args, "workload", None) is not None and "reference" in args and \
            args.reference is None:
        args.reference = Path(args.workload).parent / "reference.json"
    try:
        if args.command == "clips":
            text, _count = clip_script(map_bsp(args.steam_root, args.map), args.map)
            sys.stdout.write(text)
            return 0
        if args.command == "sheet":
            return sheet(args)
        if args.command == "self-test":
            return self_test(args)
        if args.command == "check":
            return check(args)
        if args.command == "record":
            return record_reference(args)
        workload = portal2_scenarios.load_workload(args.workload)
        if args.command == "suite":
            out = Path(args.out or os.environ.get("CONFORMANCE_OUT") or ROOT / (
                "quality-results/portal2-storybeats-%s"
                % datetime.datetime.now().strftime("%Y%m%d-%H%M%S"))) / "build"
            out.mkdir(parents=True, exist_ok=True)
            args.out = out
            capture_build(args, workload, workload["scenarios"])
            return check(argparse.Namespace(build_capture=out, retail_capture=None,
                                            reference=args.reference, workload=args.workload,
                                            only=args.only))
        scenarios = [s for s in workload["scenarios"]
                     if not args.scenario or s["name"] in args.scenario]
        unknown = set(args.scenario) - {s["name"] for s in workload["scenarios"]}
        if unknown:
            parser.error("unknown scenario: " + ", ".join(sorted(unknown)))
        out = Path(args.out)
        if (out / "capture.json").exists():
            parser.error("%s already holds a capture; use a new directory" % out)
        out.mkdir(parents=True, exist_ok=True)
        if args.side == "build":
            capture = capture_build(args, workload, scenarios)
        else:
            capture = capture_retail(args, workload, scenarios)
        failed = [n for n, r in capture["scenarios"].items() if r["status"] != "pass"]
        return 1 if failed or len(capture["scenarios"]) != len(scenarios) else 0
    except (OSError, ValueError, KeyError, BeatError, shots.ShotError,
            portal2_scenarios.ScenarioError, subprocess.SubprocessError) as error:
        print("FAIL portal2_storybeats: %s" % error, flush=True)
        if args.command in ("check", "suite"):
            conformance_result.report_conformance(1, 1)
        return 1


if __name__ == "__main__":
    sys.exit(main())
