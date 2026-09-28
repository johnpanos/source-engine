#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""RFC 0016 K0 view oracles and per-draw state fixtures.

The render core's inversion (K1-K3) must leave the legacy frame exactly as it
was. This tool freezes what a real product frame renders, before any of that
work starts, as two oracles per screenshot frame:

  views       source-view-oracle/v1 (materialsystem/vieworaclecapture.h):
              every CRender::Push3DView/Push2DView ... PopView span in
              submission order, nested as the engine nested them, and the
              draws (material, shader, pass, primitive, index range, vertex
              count, render target, viewport) and clears each view made.
  draw-state  source-draw-state/v1 (materialsystem/drawstatefixture.h): the
              state every material pass was drawn with (samplers, blend,
              depth, alpha test, material registers).

Scenarios live in quality/workloads/render-view-oracles-v1.json: camera
placements, console variables and portal placement per map, each shot naming
the features it covers. The frames are made repeatable, not filtered:

  host_framerate      fixed simulation step, so every shot lands at the same
                      game time;
  -deterministicrender particle collections seed from a sequence instead of
                      address + clock (particles/particles.cpp), and the
                      world's static batches are ordered by their first
                      material sort ID instead of heap address
                      (engine/gl_rsurf.cpp), and the engine seeds the shared
                      random stream with 0, as -random_invariant does
                      (engine/sys_dll.cpp);
  -nosound            scripted scenes that wait for a line to finish playing
                      follow game time, not the audio device's clock;
  noclip + setpos     the camera is re-placed right before every screenshot.

Declared, not hidden: view origins and angles are printed at 0.1 and compared
within POSE_TOLERANCE; PIX labels other than the engine's view events
(mat_pix_events) are ignored; DECLARED_NONDETERMINISTIC names the per-draw
state components that hold uninitialized memory in the legacy frame. No view
oracle draw field is ignored.

Commands:

  capture   run scenarios on the installed product and keep the raw captures
  record    freeze a capture as the fixture (quality/fixtures/render-views)
  check     compare a capture with the fixture (views or draw-state); checks-v1
  suite     capture then check (the manifest rows render.view-oracles and
            render.draw-state)
  selftest  the comparators against the recorded fixtures with seeded
            defects (no game): every single draw removed from every view, every
            view removed, adjacent draws swapped, one field of one draw
            changed, and one field of one per-draw state record changed;
            checks-v1
  summary   views, draws and view kinds per shot of a capture or the fixture

  view_oracle.py suite --check views --out /tmp/vo
  view_oracle.py selftest
"""

import argparse
import copy
import datetime
import gzip
import hashlib
import json
import os
from pathlib import Path
import random
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
QUALITY = ROOT / "tools/quality"
sys.path.insert(0, str(QUALITY))
import conformance  # noqa: E402
import conformance_result  # noqa: E402
import draw_state_diff  # noqa: E402
import legacy_ports_views  # noqa: E402

SCHEMA = "source-view-oracle/v1"
WORKLOAD_SCHEMA = "render-view-oracles/v1"
CAPTURE_SCHEMA = "render-view-oracle-capture/v1"
PROVENANCE_SCHEMA = "render-view-oracle-fixture/v1"
WORKLOAD = ROOT / "quality/workloads/render-view-oracles-v1.json"
FIXTURES = ROOT / "quality/fixtures/render-views"
PORTAL_BOOT = QUALITY / "portal_boot.py"
VIEW_PREFIX = "vieworacle "
BACKEND = "vulkan-native"
# The engine prints origins and angles with one decimal.
POSE_TOLERANCE = 0.11
VIEW_EXACT_FIELDS = ("type", "stack", "target", "explicit_target", "viewport", "fov", "ortho",
                     "flags", "nodraw")
VIEW_POSE_FIELDS = ("origin", "angles")
DRAW_FIELDS = ("material", "shader", "pass", "pass_count", "primitive", "first_index",
               "index_count", "vertex_count", "world_batch", "target", "viewport", "submitted",
               "drop_reason")
CLEAR_FIELDS = ("color", "depth", "stencil", "target")
# Per-draw state components that hold uninitialized memory in the legacy frame,
# ignored by the draw-state comparison: (shader, field) -> component indices.
# Downsample_nohdr sets c48-c51 from a float[4][4] whose z and w it never
# writes (materialsystem/stdshaders/downsample_nohdr.cpp), so c48.zw and c49.zw
# are stack garbage; its vertex shader reads only .xy.
DECLARED_NONDETERMINISTIC = {("Downsample_nohdr", "base_texture_transform"): (2, 3, 6, 7)}
# Source files whose change changes what the captures record.
CAPTURE_SOURCES = ("engine/gl_rmain.cpp", "engine/gl_rsurf.cpp", "engine/sys_dll.cpp",
                   "particles/particles.cpp", "materialsystem/cmatrendercontext.cpp",
                   "materialsystem/vieworaclecapture.h", "materialsystem/drawstatefixture.h",
                   "materialsystem/shaderapivulkan/shaderapivulkan.cpp",
                   "tools/quality/portal_boot.py", "tools/render/view_oracle.py",
                   "quality/workloads/render-view-oracles-v1.json")
DEFAULT_STEAM_P2 = Path.home() / ".local/share/Steam/steamapps/common/Portal 2"


class OracleError(ValueError):
    pass


# --------------------------------------------------------------------------
# Captures
# --------------------------------------------------------------------------

def read_lines(path):
    path = Path(path)
    opener = gzip.open if path.suffix == ".gz" else open
    with opener(path, "rt") as stream:
        return [line for line in stream.read().splitlines() if line]


def read_capture(path):
    """(header, events) of a source-view-oracle/v1 capture."""
    lines = read_lines(path)
    if not lines:
        raise OracleError("%s is empty" % path)
    header = json.loads(lines[0])
    if header.get("schema") != SCHEMA:
        raise OracleError("%s is not a %s capture" % (path, SCHEMA))
    if header.get("truncated"):
        raise OracleError("%s is truncated" % path)
    events = [json.loads(line) for line in lines[1:]]
    if len(events) != header.get("events"):
        raise OracleError("%s declares %s events but holds %d" % (path, header.get("events"),
                                                                  len(events)))
    for index, event in enumerate(events):
        if event.get("seq") != index:
            raise OracleError("%s: event %d has seq %s" % (path, index, event.get("seq")))
    return header, events


class View:
    """One view span. The root pseudo-view `frame` holds what no view encloses."""

    def __init__(self, vid, parent, path, info):
        self.id = vid
        self.parent = parent
        self.path = path
        self.info = info
        self.items = []       # ("draw", fields) | ("clear", fields) | ("view", child path)
        self.children = []
        self.kind = "frame" if parent is None else None
        self.nested_depth = 0

    def draws(self):
        return [item for item in self.items if item[0] == "draw"]


def _canonical(value):
    return tuple(value) if isinstance(value, list) else value


def build_frame(events):
    """The frame's view tree from its ordered events."""
    root = View(0, None, "frame", {"type": "frame"})
    views = [root]
    stack = []  # open labels: a View, or None for a label that is not a view

    def current():
        for entry in reversed(stack):
            if entry is not None:
                return entry
        return root

    for event in events:
        kind = event.get("event")
        if kind == "label_begin":
            name = event.get("name", "")
            if not name.startswith(VIEW_PREFIX):
                stack.append(None)
                continue
            try:
                info = json.loads(name[len(VIEW_PREFIX):])
            except json.JSONDecodeError as error:
                raise OracleError("malformed view event %d: %s" % (event["seq"], error))
            parent = current()
            view = View(len(views), parent, "%s/%d" % (parent.path, len(parent.children)), info)
            parent.children.append(view)
            parent.items.append(("view", view.path))
            views.append(view)
            stack.append(view)
        elif kind == "label_end":
            if not stack:
                raise OracleError("label end without a begin at event %d" % event["seq"])
            stack.pop()
        elif kind == "marker":
            continue
        elif kind == "clear":
            current().items.append(("clear", tuple(_canonical(event.get(f)) for f in CLEAR_FIELDS)))
        elif kind == "draw":
            current().items.append(("draw", tuple(_canonical(event.get(f, ""))
                                                  for f in DRAW_FIELDS)))
        else:
            raise OracleError("unknown event %r at %d" % (kind, event.get("seq")))
    if stack:
        raise OracleError("%d label(s) never ended" % len(stack))
    for view in views[1:]:
        view.kind = classify(view)
        parent = view.parent
        view.nested_depth = (parent.nested_depth + 1) if view.kind == "nested" else 0
    return views


def classify(view):
    """A descriptive kind from the engine's view record (summaries and coverage;
    the comparison itself is on the raw records)."""
    info, parent = view.info, view.parent
    if info.get("type") == "2d":
        return "2d"
    if info.get("nodraw"):
        return "nodraw"
    pinfo = parent.info
    if parent.kind != "frame" and pinfo.get("type") == "3d" and \
            not info.get("explicit_target") and pinfo.get("target") == info.get("target"):
        # Drawn into its parent's target: the view model, or a view through a
        # portal (or the 3D skybox) nested in it.
        if pinfo.get("origin") == info.get("origin") and pinfo.get("fov") != info.get("fov"):
            return "viewmodel"
        return "nested"
    target = str(info.get("target", "")).lower()
    for token, kind in (("waterreflection", "water_reflection"),
                        ("waterrefraction", "water_refraction"),
                        ("_rt_camera", "monitor"), ("shadowdepth", "shadow_depth")):
        if token in target:
            return kind
    if parent.kind == "frame":
        return "main" if not info.get("explicit_target") else "render_target"
    return "render_target" if info.get("explicit_target") else "3d"


def frame_summary(views):
    kinds = {}
    for view in views[1:]:
        kinds[view.kind] = kinds.get(view.kind, 0) + 1
    return {"views": len(views) - 1, "draws": sum(len(v.draws()) for v in views),
            "clears": sum(1 for v in views for item in v.items if item[0] == "clear"),
            "kinds": dict(sorted(kinds.items())),
            "max_nested_depth": max([v.nested_depth for v in views] + [0])}


def draw_key(view, ordinal, fields):
    return "%s#%d:%s/%d" % (view.path, ordinal, fields[0], fields[2])


def _item_key(view, index):
    item = view.items[index]
    if item[0] == "draw":
        ordinal = sum(1 for other in view.items[:index] if other[0] == "draw")
        return draw_key(view, ordinal, item[1])
    if item[0] == "view":
        return "%s (view)" % item[1]
    return "%s clear %d" % (view.path, index)


def _describe(item):
    if item is None:
        return None
    kind, fields = item
    if kind == "draw":
        return {"draw": dict(zip(DRAW_FIELDS, fields))}
    if kind == "clear":
        return {"clear": dict(zip(CLEAR_FIELDS, fields))}
    return {"view": fields}


def _pose_equal(a, b):
    if not isinstance(a, list) or not isinstance(b, list) or len(a) != len(b):
        return a == b
    return all(abs(x - y) <= POSE_TOLERANCE for x, y in zip(a, b))


def compare_frames(reference, candidate):
    """Divergences between two frames (lists of View). Empty when equal."""
    found = []
    ref_views = {view.path: view for view in reference}
    cand_views = {view.path: view for view in candidate}
    for path in sorted(set(ref_views) - set(cand_views)):
        found.append({"view": path, "reason": "view missing from the candidate"})
    for path in sorted(set(cand_views) - set(ref_views)):
        found.append({"view": path, "reason": "view only in the candidate"})
    for view in reference:
        other = cand_views.get(view.path)
        if other is None:
            continue
        for field in VIEW_EXACT_FIELDS:
            if view.info.get(field) != other.info.get(field):
                found.append({"view": view.path, "reason": "view field differs", "field": field,
                              "reference": view.info.get(field),
                              "candidate": other.info.get(field)})
        for field in VIEW_POSE_FIELDS:
            if not _pose_equal(view.info.get(field), other.info.get(field)):
                found.append({"view": view.path, "reason": "view pose differs", "field": field,
                              "reference": view.info.get(field),
                              "candidate": other.info.get(field)})
        if view.items == other.items:
            continue
        index = next((i for i, (a, b) in enumerate(zip(view.items, other.items)) if a != b),
                     min(len(view.items), len(other.items)))
        ref_item = view.items[index] if index < len(view.items) else None
        cand_item = other.items[index] if index < len(other.items) else None
        found.append({"view": view.path, "reason": "view contents differ",
                      "first_divergence": index,
                      "key": _item_key(view if ref_item else other, index),
                      "reference_items": len(view.items), "candidate_items": len(other.items),
                      "reference_draws": len(view.draws()), "candidate_draws": len(other.draws()),
                      "reference": _describe(ref_item), "candidate": _describe(cand_item)})
    return found


def load_frame(path):
    return build_frame(read_capture(path)[1])


# --------------------------------------------------------------------------
# Workload and scenarios
# --------------------------------------------------------------------------

def load_workload(path=WORKLOAD):
    workload = json.loads(Path(path).read_text())
    if workload.get("schema") != WORKLOAD_SCHEMA:
        raise OracleError("%s: schema must be %s" % (path, WORKLOAD_SCHEMA))
    ids = [scenario["id"] for scenario in workload["scenarios"]]
    if len(set(ids)) != len(ids):
        raise OracleError("scenario ids must be distinct")
    for scenario in workload["scenarios"]:
        names = [shot["name"] for shot in expand_shots(scenario)]
        if len(set(names)) != len(names) or not names:
            raise OracleError("%s: shot names must be distinct and present" % scenario["id"])
    return workload


def expand_shots(scenario):
    """The scenario's shots, with the legacy-ports view set expanded from its
    one authority (tools/quality/legacy_ports_views.py). The set's pause menu
    is the scenario's last shot: opening the menu pauses a single-player game,
    and the frame the game resumes on after it closes is not repeatable."""
    shots = []
    last = []
    for shot in scenario["shots"]:
        if shot.get("view_set") != "legacy-ports":
            shots.append(shot)
            continue
        camera = shot.get("camera")
        for name, pitch, yaw in legacy_ports_views.VIEWS:
            placement = ["cmd setpos %s %s %s" % tuple(camera[:3])] if camera else []
            shots.append({"name": "ports_" + name, "features": ["legacy_ports_view_set"],
                          "commands": placement + ["cmd setang %d %d 0" % (pitch, yaw)],
                          "settle": legacy_ports_views.SETTLE_FRAMES,
                          "after": legacy_ports_views.AFTER_FRAMES})
        last.append({"name": "ports_" + legacy_ports_views.PAUSE,
                     "features": ["legacy_ports_view_set"],
                     "commands": ["gameui_activate"],
                     "settle": legacy_ports_views.SETTLE_FRAMES * 2,
                     "after": legacy_ports_views.AFTER_FRAMES})
    return shots + last


def console_script(scenario, shots):
    """Console lines for portal_boot's command cfg.

    Every line of an exec'd cfg runs at once, and a long `wait` chain on one
    line is not reliable, so the setup runs first (at once: it may quote, as
    ent_fire parameters do) and each shot is an alias that ends by calling the
    next; one last line starts the chain after the intro."""
    lines = list(scenario.get("setup", []))
    for index, shot in enumerate(shots):
        commands = list(shot.get("commands", []))
        steps = commands + ["wait %d" % shot.get("settle", 30)]
        # Re-place the camera right before the frame: nothing may drift it.
        steps += [c for c in commands if c.startswith("cmd setpos") or c.startswith("cmd setang")]
        steps += ["wait 3", "screenshot", "wait %d" % shot.get("after", 10)]
        steps += list(shot.get("post", []))
        if index + 1 < len(shots):
            steps.append("vo_shot%d" % (index + 1))
        body = "; ".join(steps)
        if '"' in body:
            raise OracleError("%s: a shot command may not quote (move it to setup)" % shot["name"])
        lines.append('alias vo_shot%d "%s"' % (index, body))
    lines.append("wait %d; vo_shot0" % scenario.get("intro_frames", 0))
    return lines


def script_frames(scenario, shots):
    total = scenario.get("intro_frames", 0) + 60
    for shot in shots:
        total += shot.get("settle", 30) + 3 + shot.get("after", 10)
    return total


def _sha256(path):
    digest = hashlib.sha256()
    with Path(path).open("rb") as stream:
        for chunk in iter(lambda: stream.read(1 << 20), b""):
            digest.update(chunk)
    return digest.hexdigest()


def runtime_identity(runtime, game, map_name):
    """Digest of the content a capture reads: gameinfo, VPK directories, the map."""
    runtime = Path(runtime).resolve()
    files = [runtime / game / "gameinfo.txt"]
    for directory in [runtime / game] + [runtime / name for name in
                                         ("update", "portal2_dlc1", "portal2_dlc2")]:
        if directory.is_dir():
            files += sorted(directory.glob("*_dir.vpk"))
    files += [runtime / game / "maps" / (map_name + ".bsp")]
    entries = {str(path.relative_to(runtime)): _sha256(path) for path in files if path.is_file()}
    digest = hashlib.sha256(json.dumps(entries, sort_keys=True).encode()).hexdigest()
    return {"path": str(runtime), "sha256": digest, "files": entries}


def sources_identity():
    identity = conformance.source_identity(str(ROOT))
    identity["capture_sources"] = {name: _sha256(ROOT / name) for name in CAPTURE_SOURCES
                                   if (ROOT / name).is_file()}
    return identity


def prepare_runtime(scenario, args, out):
    """The runtime portal_boot stages from: Portal's, or a Portal 2 content runtime."""
    if scenario["game"] == "portal":
        return Path(args.runtime)
    runtime = Path(args.p2_runtime) if args.p2_runtime else out / "p2content"
    if not runtime.exists():
        import stage_portal2_runtime  # noqa: E402 (needs the Steam install only here)
        stage_portal2_runtime.stage_content(args.steam_root, runtime)
    return runtime


def capture_scenario(scenario, workload, args, out):
    """Boot the product once for the scenario; keep its captures and evidence."""
    shots = expand_shots(scenario)
    common = workload["common"]
    directory = out / scenario["id"]
    if directory.exists():
        shutil.rmtree(directory)
    directory.parent.mkdir(parents=True, exist_ok=True)
    runtime = prepare_runtime(scenario, args, out)
    build = args.p2_build if scenario["game"] == "portal2" else args.build
    width, height = scenario.get("size", common["size"])
    engine_args = common.get("engine_args", []) + scenario.get("engine_args", [])
    startup = common.get("startup_commands", []) + scenario.get("startup_commands", [])
    physics = scenario.get("physics", common["physics"])
    if scenario.get("settings") == "legacy-ports":
        # The legacy-ports view set's own settings (run.conf's), from its one
        # authority; the workload's determinism settings stay.
        width, height = legacy_ports_views.WIDTH, legacy_ports_views.HEIGHT
        engine_args = common.get("engine_args", []) + list(legacy_ports_views.ENGINE_ARGS)
        startup = common.get("startup_commands", []) + list(legacy_ports_views.STARTUP_COMMANDS)
        physics = "vphysics_box3d"
    command = [sys.executable, str(PORTAL_BOOT), "--game", scenario["game"],
               "--runtime", str(runtime), "--build", str(build), "--out", str(directory),
               "--headless", "--renderer", common["renderer"], "--map", scenario["map"],
               "--physics", physics,
               "--width", str(width), "--height", str(height),
               "--view-oracle", "--draw-state-fixtures",
               "--capture-wait", str(script_frames(scenario, shots)),
               "--timeout", str(args.timeout),
               ]
    for line in console_script(scenario, shots):
        command += ["--console-command", line]
    for argument in engine_args + list(args.engine_arg):
        command.append("--engine-arg=" + argument)
    queue_mode = args.queue_mode if args.queue_mode is not None else scenario.get("mat_queue_mode")
    if queue_mode is not None:
        startup = [line for line in startup if not line.startswith("mat_queue_mode")]
        startup.append("mat_queue_mode %d" % queue_mode)
    else:
        # portal_boot's own +mat_queue_mode 0, unless a startup line sets it.
        modes = [int(line.split()[1]) for line in startup if line.startswith("mat_queue_mode ")]
        queue_mode = modes[-1] if modes else 0
    for line in startup:
        command += ["--startup-command", line]
    started = datetime.datetime.now(datetime.timezone.utc).isoformat()
    completed = subprocess.run(command, capture_output=True, text=True)
    evidence_path = directory / "evidence.json"
    evidence = json.loads(evidence_path.read_text()) if evidence_path.is_file() else {}
    views = sorted((directory / "vo").glob("*.jsonl"), key=_capture_index)
    states = sorted((directory / "draw-state").glob("*.jsonl"), key=_capture_index)
    record = {"schema": CAPTURE_SCHEMA, "scenario": scenario["id"], "game": scenario["game"],
              "map": scenario["map"], "started_utc": started, "source": sources_identity(),
              "runtime": runtime_identity(runtime, scenario["game"], scenario["map"]),
              "build": {"path": str(Path(build).resolve()),
                        "executables_sha256": hashlib.sha256(json.dumps(
                            evidence.get("executables", {}), sort_keys=True).encode()).hexdigest()},
              "queue_mode": queue_mode,
              "command": command, "boot_status": evidence.get("status"),
              "boot_failures": evidence.get("failures", []),
              "returncode": completed.returncode,
              "boot_tail": (completed.stdout + completed.stderr)[-600:],
              "shots": [], "failures": []}
    # The final capture is portal_boot's own closing screenshot, not a shot.
    if len(views) < len(shots) or len(states) < len(shots):
        record["failures"].append("%d shots but %d view and %d draw-state captures"
                                  % (len(shots), len(views), len(states)))
    for index, shot in enumerate(shots):
        entry = {"name": shot["name"], "features": shot.get("features", []), "index": index}
        if index < len(views):
            entry["views"] = str(views[index].relative_to(directory))
            entry["summary"] = frame_summary(load_frame(views[index]))
        if index < len(states):
            entry["draw_state"] = str(states[index].relative_to(directory))
        record["shots"].append(entry)
    if completed.returncode != 0:
        record["failures"].append("boot failed: %s" % "; ".join(evidence.get("failures", [])))
    (directory / "capture.json").write_text(json.dumps(record, indent=2) + "\n")
    return record


def _capture_index(path):
    return int(path.name.split("-")[-1].split(".")[0])


def select_scenarios(workload, names):
    scenarios = workload["scenarios"]
    if names:
        unknown = set(names) - {scenario["id"] for scenario in scenarios}
        if unknown:
            raise OracleError("unknown scenario(s): " + ", ".join(sorted(unknown)))
        scenarios = [scenario for scenario in scenarios if scenario["id"] in names]
    return scenarios


# --------------------------------------------------------------------------
# Fixtures
# --------------------------------------------------------------------------

def write_gzip(source, destination):
    """Deterministic gzip (no name, no time) so a re-record of equal data is equal."""
    data = Path(source).read_bytes()
    with open(destination, "wb") as raw:
        with gzip.GzipFile(filename="", mode="wb", fileobj=raw, mtime=0, compresslevel=9) as out:
            out.write(data)


def record_fixture(scenario, capture_dir, fixtures=FIXTURES):
    capture = json.loads((capture_dir / "capture.json").read_text())
    if capture["failures"]:
        raise OracleError("%s: the capture failed: %s" % (scenario["id"],
                                                          "; ".join(capture["failures"])))
    target = fixtures / scenario["id"]
    if target.exists():
        shutil.rmtree(target)
    target.mkdir(parents=True)
    shots = []
    for shot in capture["shots"]:
        views = target / ("%s.views.jsonl.gz" % shot["name"])
        states = target / ("%s.draw-state.jsonl.gz" % shot["name"])
        write_gzip(capture_dir / shot["views"], views)
        write_gzip(capture_dir / shot["draw_state"], states)
        shots.append({"name": shot["name"], "features": shot["features"],
                      "views": views.name, "views_sha256": _sha256(views),
                      "draw_state": states.name, "draw_state_sha256": _sha256(states),
                      "summary": shot["summary"],
                      "draw_state_draws": read_draw_state(states)[0]["draws"]})
    provenance = {"schema": PROVENANCE_SCHEMA, "scenario": scenario["id"],
                  "game": scenario["game"], "map": scenario["map"],
                  "recorded_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
                  "captured_utc": capture["started_utc"], "source": capture["source"],
                  "runtime": capture["runtime"], "build": capture["build"],
                  "queue_mode": capture["queue_mode"], "command": capture["command"],
                  "shots": shots}
    (target / "provenance.json").write_text(json.dumps(provenance, indent=2) + "\n")
    return provenance


def read_draw_state(path):
    lines = read_lines(path)
    header = json.loads(lines[0])
    if header.get("schema") != draw_state_diff.SCHEMA:
        raise OracleError("%s is not a %s fixture" % (path, draw_state_diff.SCHEMA))
    draws = [json.loads(line) for line in lines[1:]]
    if len(draws) != header.get("draws") or header.get("truncated"):
        raise OracleError("%s: incomplete draw-state fixture" % path)
    return header, draws


def fixture_provenance(scenario_id, fixtures=FIXTURES):
    path = fixtures / scenario_id / "provenance.json"
    if not path.is_file():
        raise OracleError("no fixture recorded for %s" % scenario_id)
    return json.loads(path.read_text())


# --------------------------------------------------------------------------
# Checks
# --------------------------------------------------------------------------

def coverage_problems(shot, views):
    """What a shot's frame must contain for the features it claims: view kinds,
    the deepest nested (portal) view, materials drawn."""
    problems = []
    expect = shot.get("expect", {})
    summary = frame_summary(views)
    for kind, minimum in expect.get("kinds", {}).items():
        if summary["kinds"].get(kind, 0) < minimum:
            problems.append("%d %s view(s), expected at least %d"
                            % (summary["kinds"].get(kind, 0), kind, minimum))
    if "nested_depth" in expect and summary["max_nested_depth"] != expect["nested_depth"]:
        problems.append("deepest nested view %d, expected %d"
                        % (summary["max_nested_depth"], expect["nested_depth"]))
    materials = {item[1][0].lower() for view in views for item in view.draws()}
    for token in expect.get("materials", []):
        if not any(token in material for material in materials):
            problems.append("no draw of a material containing %r" % token)
    if summary["draws"] < expect.get("min_draws", 1):
        problems.append("%d draws" % summary["draws"])
    return problems


def check_views(scenario, capture_dir, checks, fixtures=FIXTURES, report=None):
    provenance = fixture_provenance(scenario["id"], fixtures)
    capture = json.loads((capture_dir / "capture.json").read_text())
    checks.check(not capture["failures"], "%s.run" % scenario["id"],
                 "; ".join(capture["failures"]))
    shots = {shot["name"]: shot for shot in capture["shots"]}
    declared = {shot["name"]: shot for shot in expand_shots(scenario)}
    for fixed in provenance["shots"]:
        name = "%s.%s.views" % (scenario["id"], fixed["name"])
        taken = shots.get(fixed["name"])
        if not taken or "views" not in taken:
            checks.check(False, name, "no capture")
            continue
        reference = load_frame(fixtures / scenario["id"] / fixed["views"])
        candidate = load_frame(capture_dir / taken["views"])
        found = compare_frames(reference, candidate)
        if report is not None:
            report.append({"scenario": scenario["id"], "shot": fixed["name"],
                           "divergences": found[:20], "divergence_count": len(found)})
        checks.check(not found, name, "%d divergence(s); first %s"
                     % (len(found), json.dumps(found[0]) if found else ""))
        problems = coverage_problems(declared.get(fixed["name"], {}), candidate)
        checks.check(not problems, "%s.%s.coverage" % (scenario["id"], fixed["name"]),
                     "; ".join(problems))


def check_draw_state(scenario, capture_dir, checks, fixtures=FIXTURES, report=None):
    provenance = fixture_provenance(scenario["id"], fixtures)
    capture = json.loads((capture_dir / "capture.json").read_text())
    checks.check(not capture["failures"], "%s.run" % scenario["id"],
                 "; ".join(capture["failures"]))
    shots = {shot["name"]: shot for shot in capture["shots"]}
    for fixed in provenance["shots"]:
        name = "%s.%s.draw-state" % (scenario["id"], fixed["name"])
        taken = shots.get(fixed["name"])
        if not taken or "draw_state" not in taken:
            checks.check(False, name, "no capture")
            continue
        _, reference = read_draw_state(fixtures / scenario["id"] / fixed["draw_state"])
        _, candidate = read_draw_state(capture_dir / taken["draw_state"])
        found = draw_state_diff.compare_exact(reference, candidate, DECLARED_NONDETERMINISTIC)
        if report is not None:
            report.append({"scenario": scenario["id"], "shot": fixed["name"],
                           "divergences": found[:20], "divergence_count": len(found)})
        checks.check(not found and reference, name, "%d draws differ; first %s"
                     % (len(found), json.dumps(found[0]) if found else "(empty fixture)"))


# --------------------------------------------------------------------------
# Self-test: seeded defects against the recorded fixtures
# --------------------------------------------------------------------------

def _with_items(views, position, items):
    """The frame with one view's item list replaced (the others shared)."""
    mutated = list(views)
    view = copy.copy(views[position])
    view.items = items
    mutated[position] = view
    return mutated


def remove_view(views, path):
    """The frame without one view and its subtree (and the parent's reference)."""
    kept = [copy.copy(v) for v in views if not (v.path == path or v.path.startswith(path + "/"))]
    for view in kept:
        view.items = [item for item in view.items if item != ("view", path)]
    return kept


SEEDED_FAULTS = ("none", "keep-draw", "ignore-view", "ignore-state")


def selftest(args):
    """Every seeded defect must be detected by the comparators; `--seed-fault`
    breaks the comparator on purpose, which the self-test must then report."""
    checks = conformance_result.Checks()
    workload = load_workload(args.workload)
    rng = random.Random(20260926)
    fault = args.seed_fault
    compare = compare_frames
    if fault == "keep-draw":
        # A comparator that only counts views misses removed draws.
        def compare(reference, candidate):
            return [] if [v.path for v in reference] == [v.path for v in candidate] else ["views"]
    elif fault == "ignore-view":
        # A comparator that only compares draw lists misses a removed empty view.
        def compare(reference, candidate):
            return [] if [d for v in reference for d in v.draws()] == \
                [d for v in candidate for d in v.draws()] else ["draws"]
    def state_compare(reference, candidate):
        return draw_state_diff.compare_exact(reference, candidate, DECLARED_NONDETERMINISTIC)
    if fault == "ignore-state":
        def state_compare(reference, candidate):
            return [] if len(reference) == len(candidate) else ["count"]
    totals = {"frames": 0, "draw_removals": 0, "view_removals": 0, "swaps": 0,
              "field_changes": 0, "state_changes": 0, "event_removals": 0}
    for scenario in select_scenarios(workload, args.scenario):
        provenance = fixture_provenance(scenario["id"], args.fixtures)
        for shot in provenance["shots"]:
            prefix = "%s.%s" % (scenario["id"], shot["name"])
            _, events = read_capture(args.fixtures / scenario["id"] / shot["views"])
            views = build_frame(events)
            totals["frames"] += 1
            checks.check(not compare(views, build_frame(events)), prefix + ".identical",
                         "an unchanged capture diverges")
            missed = []
            # Every single draw of every view removed, one at a time.
            for position, view in enumerate(views):
                for index, item in enumerate(view.items):
                    if item[0] != "draw":
                        continue
                    mutated = _with_items(views, position,
                                          view.items[:index] + view.items[index + 1:])
                    totals["draw_removals"] += 1
                    if not compare(views, mutated):
                        missed.append(_item_key(view, index))
            checks.check(not missed, prefix + ".remove-each-draw",
                         "%d undetected, first %s" % (len(missed), missed[:3]))
            # Every view removed.
            missed = []
            for view in views[1:]:
                totals["view_removals"] += 1
                if not compare(views, remove_view(views, view.path)):
                    missed.append(view.path)
            checks.check(not missed, prefix + ".remove-each-view", "undetected %s" % missed[:3])
            # Adjacent distinct draws swapped, and one field of one draw changed
            # (seeded sample, every view with draws at least once).
            missed = []
            for position, view in enumerate(views):
                pairs = [i for i in range(len(view.items) - 1)
                         if view.items[i][0] == "draw" and view.items[i + 1][0] == "draw"
                         and view.items[i] != view.items[i + 1]]
                for i in rng.sample(pairs, min(len(pairs), 8)):
                    items = list(view.items)
                    items[i], items[i + 1] = items[i + 1], items[i]
                    mutated = _with_items(views, position, items)
                    totals["swaps"] += 1
                    if not compare(views, mutated):
                        missed.append("swap %s" % _item_key(view, i))
                draws = [i for i, item in enumerate(view.items) if item[0] == "draw"]
                for i in rng.sample(draws, min(len(draws), 8)):
                    field = rng.randrange(len(DRAW_FIELDS))
                    fields = list(view.items[i][1])
                    value = fields[field]
                    if isinstance(value, bool):
                        fields[field] = not value
                    elif isinstance(value, int):
                        fields[field] = value + 1
                    elif isinstance(value, tuple):
                        fields[field] = value[:-1] + (value[-1] + 1,)
                    else:
                        fields[field] = str(value) + "_seeded"
                    items = list(view.items)
                    items[i] = ("draw", tuple(fields))
                    mutated = _with_items(views, position, items)
                    totals["field_changes"] += 1
                    if not compare(views, mutated):
                        missed.append("%s %s" % (DRAW_FIELDS[field], _item_key(view, i)))
            checks.check(not missed, prefix + ".swap-and-field", "undetected %s" % missed[:3])
            # End to end: a seeded sample of draw events removed from the raw
            # capture, rebuilt and compared.
            draw_events = [i for i, event in enumerate(events) if event["event"] == "draw"]
            missed = []
            for i in rng.sample(draw_events, min(len(draw_events), 12)):
                mutated_events = events[:i] + events[i + 1:]
                totals["event_removals"] += 1
                if not compare(views, build_frame(mutated_events)):
                    missed.append(i)
            checks.check(not missed, prefix + ".remove-draw-event", "undetected events %s"
                         % missed[:3])
            # Draw state: one field of one draw changed, for a seeded sample of
            # draws that covers every state field.
            _, states = read_draw_state(args.fixtures / scenario["id"] / shot["draw_state"])
            checks.check(not state_compare(states, states), prefix + ".draw-state.identical",
                         "an unchanged fixture diverges")
            missed = []
            fields = draw_state_diff.STATE_FIELDS
            sample = rng.sample(range(len(states)), min(len(states), 4 * len(fields)))
            for position, index in enumerate(sample):
                draw = states[index]
                field = fields[position % len(fields)]
                mutated = list(states)
                mutated[index] = seed_state_change(draw, field)
                totals["state_changes"] += 1
                if not state_compare(states, mutated):
                    missed.append("%d:%s" % (index, field))
            checks.check(bool(states) and not missed, prefix + ".draw-state.change-one-field",
                         "%d draws, undetected %s" % (len(states), missed[:3]))
    print("selftest: %s" % json.dumps(totals))
    return checks.report()


def seed_state_change(draw, field):
    """A copy of a draw-state record with `field` changed to a value it does not have."""
    changed = json.loads(json.dumps(draw))
    value = changed.get(field)
    if isinstance(value, bool):
        changed[field] = not value
    elif isinstance(value, (int, float)):
        changed[field] = value + 0.5
    elif value is None:
        changed[field] = 0.5
    elif isinstance(value, list):
        if value and isinstance(value[0], dict):
            changed[field] = [dict(value[0], texture="seeded")] + value[1:]
        elif value:
            changed[field] = [value[0] + 0.5] + value[1:]
        else:
            changed[field] = [{"stage": 0, "texture": "seeded"}]
    else:
        changed[field] = str(value) + "_seeded"
    return changed


# --------------------------------------------------------------------------
# Command line
# --------------------------------------------------------------------------

def common_arguments(parser):
    parser.add_argument("--workload", type=Path, default=WORKLOAD)
    parser.add_argument("--fixtures", type=Path, default=FIXTURES)
    parser.add_argument("--scenario", action="append", default=[],
                        help="only this scenario (repeatable)")


def run_arguments(parser):
    parser.add_argument("--runtime", type=Path,
                        default=Path(os.environ.get("SOURCE_PORTAL_RUNTIME",
                                                    ROOT / "run/runtime")),
                        help="installed Portal runtime (immutable assets are shared)")
    parser.add_argument("--build", type=Path,
                        default=Path(os.environ.get("SOURCE_VIEW_ORACLE_BUILD", ROOT / "build")),
                        help="Portal client Waf tree or install (native Vulkan, SDL3)")
    parser.add_argument("--p2-build", type=Path,
                        default=Path(os.environ.get("SOURCE_PORTAL2_BUILD", ROOT / "build-p2")),
                        help="Portal 2 client Waf tree or install (native Vulkan, SDL3)")
    parser.add_argument("--steam-root", type=Path,
                        default=Path(os.environ.get("SOURCE_PORTAL2_STEAM_ROOT",
                                                    DEFAULT_STEAM_P2)))
    parser.add_argument("--p2-runtime", type=Path,
                        help="private staged Portal 2 content runtime (default <out>/p2content)")
    parser.add_argument("--queue-mode", type=int, choices=(0, 2),
                        help="override every scenario's mat_queue_mode")
    parser.add_argument("--engine-arg", action="append", default=[],
                        help="extra launcher argument for every scenario (diagnosis; a "
                             "capture compared with the fixture should not need one)")
    parser.add_argument("--timeout", type=int, default=900)
    parser.add_argument("--out", type=Path, required=True,
                        help="new output directory (keep it short: the engine refuses "
                             "command lines over 512 characters)")


def cmd_capture(args):
    workload = load_workload(args.workload)
    out = args.out.resolve()
    out.mkdir(parents=True, exist_ok=True)
    status = 0
    for scenario in select_scenarios(workload, args.scenario):
        record = capture_scenario(scenario, workload, args, out)
        print("%s: %s" % (scenario["id"], "; ".join(record["failures"]) or "captured"))
        for shot in record["shots"]:
            summary = shot.get("summary", {})
            print("  %-22s views %4s draws %5s kinds %s" % (
                shot["name"], summary.get("views"), summary.get("draws"),
                json.dumps(summary.get("kinds"))))
        status |= 1 if record["failures"] else 0
    return status


def cmd_record(args):
    workload = load_workload(args.workload)
    for scenario in select_scenarios(workload, args.scenario):
        provenance = record_fixture(scenario, args.capture.resolve() / scenario["id"],
                                    args.fixtures)
        print("%s: %d shots recorded" % (scenario["id"], len(provenance["shots"])))
    return 0


def cmd_check(args):
    workload = load_workload(args.workload)
    checks = conformance_result.Checks()
    report = []
    for scenario in select_scenarios(workload, args.scenario):
        capture_dir = args.capture.resolve() / scenario["id"]
        if args.check in ("views", "both"):
            check_views(scenario, capture_dir, checks, args.fixtures, report)
        if args.check in ("draw-state", "both"):
            check_draw_state(scenario, capture_dir, checks, args.fixtures, report)
    if args.report:
        args.report.write_text(json.dumps(report, indent=2) + "\n")
    return checks.report()


def cmd_suite(args):
    workload = load_workload(args.workload)
    out = args.out.resolve()
    out.mkdir(parents=True, exist_ok=True)
    checks = conformance_result.Checks()
    report = []
    for scenario in select_scenarios(workload, args.scenario):
        capture_scenario(scenario, workload, args, out)
        if args.check in ("views", "both"):
            check_views(scenario, out / scenario["id"], checks, args.fixtures, report)
        if args.check in ("draw-state", "both"):
            check_draw_state(scenario, out / scenario["id"], checks, args.fixtures, report)
    (out / "report.json").write_text(json.dumps(report, indent=2) + "\n")
    return checks.report()


def cmd_summary(args):
    workload = load_workload(args.workload)
    for scenario in select_scenarios(workload, args.scenario):
        if args.capture:
            capture = json.loads((args.capture / scenario["id"] / "capture.json").read_text())
            shots = [(shot["name"], args.capture / scenario["id"] / shot["views"])
                     for shot in capture["shots"] if "views" in shot]
        else:
            provenance = fixture_provenance(scenario["id"], args.fixtures)
            shots = [(shot["name"], args.fixtures / scenario["id"] / shot["views"])
                     for shot in provenance["shots"]]
        print(scenario["id"])
        for name, path in shots:
            summary = frame_summary(load_frame(path))
            print("  %-22s views %4d draws %5d clears %3d depth %2d kinds %s" % (
                name, summary["views"], summary["draws"], summary["clears"],
                summary["max_nested_depth"], json.dumps(summary["kinds"])))
    return 0


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    commands = parser.add_subparsers(dest="command", required=True)
    capture = commands.add_parser("capture", help="run scenarios and keep the raw captures")
    common_arguments(capture)
    run_arguments(capture)
    record = commands.add_parser("record", help="freeze a capture as the fixture")
    common_arguments(record)
    record.add_argument("--capture", type=Path, required=True)
    check = commands.add_parser("check", help="compare a capture with the fixture")
    common_arguments(check)
    check.add_argument("--capture", type=Path, required=True)
    check.add_argument("--check", choices=("views", "draw-state", "both"), default="both")
    check.add_argument("--report", type=Path)
    suite = commands.add_parser("suite", help="capture, then check against the fixture")
    common_arguments(suite)
    run_arguments(suite)
    suite.add_argument("--check", choices=("views", "draw-state", "both"), default="both")
    test = commands.add_parser("selftest", help="seeded defects against the fixtures")
    common_arguments(test)
    test.add_argument("--seed-fault", choices=SEEDED_FAULTS, default="none",
                      help="break the comparator on purpose (negative control)")
    summary = commands.add_parser("summary", help="views and draws per shot")
    common_arguments(summary)
    summary.add_argument("--capture", type=Path)
    args = parser.parse_args(argv)
    try:
        return {"capture": cmd_capture, "record": cmd_record, "check": cmd_check,
                "suite": cmd_suite, "selftest": selftest, "summary": cmd_summary}[args.command](args)
    except (OSError, OracleError, json.JSONDecodeError) as error:
        print("view oracle: %s" % error, file=sys.stderr)
        if args.command in ("check", "suite", "selftest"):
            return conformance_result.report_conformance(1, 1)
        return 2


if __name__ == "__main__":
    sys.exit(main())
