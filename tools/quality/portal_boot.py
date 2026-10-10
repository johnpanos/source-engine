#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Run the installed Portal product in an isolated, evidenced writable tree."""

import argparse
import datetime
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import struct
import subprocess
import sys
import time

import conformance
import launch_sandbox
import render_trace

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "kiln"))
import sepipe_loader  # noqa: E402


SHADER_REGEN = (Path(__file__).resolve().parents[2] /
                "render/shaders/legacy/regen_material_spv.py")
# The sizes exercise grow, shrink, aspect changes and non-aligned dimensions.
# Version 2 fixes the frame time and waits several frames per step (see
# resize_commands) and adds the queued settle sweep.
RESIZE_WORKLOAD_VERSION = 2
RESIZE_WORKLOAD = ((640, 480), (801, 601), (1024, 576), (1279, 719),
                   (960, 720), (641, 479), (1280, 800), (1001, 701),
                   (800, 600), (1200, 675), (721, 541), (1024, 768))
# Queued mode: a screenshot leaves queued rendering for its frame
# (AllowThreading), so the main thread runs the render worker's queue. Taken at
# each command-buffer pass from 0 to 24 after a resize (about 12 frames, past
# the 60 ms settle), one screenshot lands in the frame that queues the settled
# resize. Each size must still complete: a completion lost there once stopped
# rendering for good.
RESIZE_SETTLE_SWEEP = tuple(((860 + 8 * offset, 540 + 6 * offset), offset) for offset in range(25))



def sha256(path):
    digest = hashlib.sha256()
    with Path(path).open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def content_files(content_root, require_map=True):
    """Validated (source, relative) files of a compiled map content root, or
    (require_map False) of a materials-only root."""
    source_root = Path(content_root).resolve()
    if not source_root.is_dir():
        raise ValueError("content root is missing")
    files = []
    for source in sorted(source_root.rglob("*")):
        if source.is_dir():
            continue
        relative = source.relative_to(source_root)
        # Maps, materials and map-scoped dynamic models (map_scene props).
        allowed = {"maps": {".bsp"}, "materials": {".vtf", ".vmt"},
                   "models": {".mdl", ".vvd", ".vtx", ".phy", ".ani"}}
        if (source.is_symlink() or not source.is_file()
                or source.suffix.lower() not in allowed.get(relative.parts[0], set())):
            raise ValueError("content root has an unsupported file: " + str(relative))
        files.append((source, relative))
    if require_map and not any(relative.parts[0] == "maps" for _, relative in files):
        raise ValueError("content root has no map")
    if not require_map and any(relative.parts[0] != "materials" for _, relative in files):
        raise ValueError("a material root holds only materials/")
    return files


def install_content(content_root, stage, game="portal", require_map=True, mount=None):
    """Overlay a private compiled map and its materials (or, require_map False,
    private materials alone) into a staged game."""
    plan = []
    destination_root = stage / game / mount if mount else stage / game
    for source, relative in content_files(content_root, require_map):
        target = destination_root / relative
        if target.exists() or target.is_symlink():
            raise ValueError("private content would replace installed content: " + str(relative))
        plan.append((source, relative, target))
    installed = {}
    for source, relative, target in plan:
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source, target)
        installed[str(relative)] = {"source": str(source), "sha256": sha256(target)}
    return installed


def prepend_game_search_path(gameinfo, path):
    """Put a private overlay before packaged and published content in staged GameInfo."""
    tokens = [match for match in re.finditer(r'"(?:\\.|[^"\\])*"|//[^\r\n]*|[{}]|[^\s{}"]+', gameinfo)
              if not match.group().startswith("//")]
    sections, positions = [], []
    for index, token in enumerate(tokens):
        value = token.group()
        if value == "{":
            if not index or tokens[index - 1].group() in {"{", "}"}:
                raise ValueError("malformed staged GameInfo sections")
            name = tokens[index - 1].group().strip('"').lower()
            if name == "searchpaths" and sections and sections[-1] == "filesystem":
                positions.append(token.end())
            sections.append(name)
        elif value == "}":
            if not sections:
                raise ValueError("malformed staged GameInfo sections")
            sections.pop()
    if sections or len(positions) != 1:
        raise ValueError("staged GameInfo must contain exactly one FileSystem/SearchPaths section")
    position = positions[0]
    entry = "\n\t\t\tgame+mod\t\t" + path + "\n"
    return gameinfo[:position] + entry + gameinfo[position:]


def shader_search_path(gameinfo, game="portal"):
    return prepend_game_search_path(gameinfo, game + "/custom/source-engine-shaders")


def install_shader_artifacts(artifacts, stage, source_root=None, game="portal"):
    """Validate source-matched compiler output before replacing staged shaders."""
    if game not in {"portal", "portal2"}:
        raise ValueError("unsupported shader artifact game")
    artifacts, stage = Path(artifacts).resolve(), Path(stage).resolve()
    source_root = Path(source_root or conformance.repo_root()).resolve()
    manifest_path = artifacts / "manifest.json"
    manifest = json.loads(manifest_path.read_text())
    if not isinstance(manifest, dict) or manifest.get("schema") != 1 or manifest.get("status") != "passed":
        raise ValueError("shader artifact manifest must be schema 1 with passed status")
    shaders = manifest.get("shaders")
    if not isinstance(shaders, list) or not shaders:
        raise ValueError("shader artifact manifest must enumerate compiled shaders")

    def verified_file(root, relative, digest):
        if not isinstance(relative, str) or not relative or "\\" in relative:
            raise ValueError("invalid shader artifact/source path")
        path = Path(relative)
        if path.is_absolute() or ".." in path.parts or str(path) != relative:
            raise ValueError("shader artifact/source path must be relative and canonical")
        absolute = (root / path).resolve()
        if root not in absolute.parents or not absolute.is_file():
            raise ValueError("shader artifact/source is missing or outside its root: " + relative)
        if not isinstance(digest, str) or not re.fullmatch(r"[a-f0-9]{64}", digest) or sha256(absolute) != digest:
            raise ValueError("shader artifact/source hash differs: " + relative)
        return absolute

    compiler = manifest.get("compiler")
    if not isinstance(compiler, dict):
        raise ValueError("shader artifact must record its compiler hash")
    verified_file(source_root, compiler.get("path"), compiler.get("sha256"))
    gameinfo = stage / game / "gameinfo.txt"
    staged_gameinfo = shader_search_path(gameinfo.read_text(), game)
    planned = {}
    for shader in shaders:
        if not isinstance(shader, dict):
            raise ValueError("invalid shader artifact entry")
        name = shader.get("name")
        if not isinstance(name, str) or not re.fullmatch(r"[A-Za-z0-9_]+", name):
            raise ValueError("invalid shader artifact name")
        if name in planned:
            raise ValueError("duplicate shader artifact: " + name)
        relative = "shaders/fxc/%s.vcs" % name
        if shader.get("path") != relative:
            raise ValueError("shader artifact path does not match its name: " + name)
        sources = shader.get("sources")
        if not isinstance(sources, dict) or not sources:
            raise ValueError("shader artifact must record its source hashes: " + name)
        for source, digest in sources.items():
            verified_file(source_root, source, digest)
        source = verified_file(artifacts, relative, shader.get("sha256"))
        destination = stage / game / "custom/source-engine-shaders" / relative
        if stage not in destination.parent.resolve().parents:
            raise ValueError("shader staging directory escapes the private runtime")
        planned[name] = (source, destination)
    installed = {}
    for name, (source, destination) in planned.items():
        destination.parent.mkdir(parents=True, exist_ok=True)
        destination.unlink(missing_ok=True)
        shutil.copy2(source, destination)
        installed[name] = {"path": str(destination.relative_to(stage)), "sha256": sha256(destination)}
    gameinfo.write_text(staged_gameinfo)
    return {"manifest": str(manifest_path), "manifest_sha256": sha256(manifest_path),
            "coverage": manifest.get("coverage"), "shaders": installed,
            "gameinfo_sha256": sha256(gameinfo),
            "search_path": game + "/custom/source-engine-shaders"}


def screenshot_info(path):
    """Validate the engine's uncompressed TGA output, not merely its filename."""
    path = Path(path)
    data = path.read_bytes()
    header = data[:18]
    if len(header) != 18:
        return None
    image_id, color_map, image_type = header[:3]
    width, height = struct.unpack_from("<HH", header, 12)
    depth = header[16]
    if color_map or image_type != 2 or depth not in (24, 32) or width < 64 or height < 64:
        return None
    required = 18 + image_id + width * height * (depth // 8)
    if path.stat().st_size < required:
        return None
    pixels = data[18 + image_id:required]
    stride = depth // 8
    # A valid image container alone cannot prove a rendered scene. Reject the
    # blank black/white frames observed during real startup failures. This is a
    # coarse sensitivity check; visual review still establishes scene fidelity.
    visible = 0
    colors = set()
    for i in range(0, len(pixels), stride):
        visible += 10 < (pixels[i] + pixels[i + 1] + pixels[i + 2]) / 3 < 245
        if len(colors) < 32:
            colors.add(pixels[i:i + 3])
    detail_fraction = visible / (width * height)
    return {"path": str(path), "width": width, "height": height,
            "midtone_fraction": detail_fraction, "distinct_colors_capped": len(colors),
            "has_scene_detail": detail_fraction >= 0.05 and len(colors) >= 16,
            "bytes": path.stat().st_size, "sha256": sha256(path)}


def tonemap_scale(log):
    """The last value the console printed for mat_hdr_tonemapscale, or None."""
    values = re.findall(r'"mat_hdr_tonemapscale" = "([-0-9.eE+]+)"', log)
    return float(values[-1]) if values else None


def evaluate(log, screenshots, returncode, timed_out, map_name, requirements, loaded,
             renderer=None):
    failures = []
    if timed_out:
        failures.append("product timed out")
    elif returncode != 0:
        failures.append("product did not exit cleanly: %s" % returncode)
    if not re.search(r"\bmap\s*:\s*" + re.escape(map_name) + r"\s+at:", log):
        failures.append("requested map was not active in engine status output")
    if not re.search(r"#\s*\d+\s+\"[^\n]*\"[^\n]*\bactive\b", log):
        failures.append("no fully active player in engine status output")
    if not screenshots:
        failures.append("no fresh complete engine screenshot")
    elif not any(frame.get("has_scene_detail", False) for frame in screenshots):
        failures.append("engine capture lacks scene detail (blank or almost entirely black/white)")
    if re.search(r"Couldn't load (?:combo|vertex shader|pixel shader)|Using invalid shader combo", log):
        failures.append("required shader artifact or permutation was unavailable")
    markers = {
        "vulkan": r"\[NativeVulkan\] IShaderAPI::SetMode: device [^\n]+ up\b",
        "sdl3": r"RFC0001 window: provider=sdl3\b",
        "wayland": r"RFC0001 window: provider=sdl3 driver=wayland\b",
    }
    for requirement in requirements:
        if not re.search(markers[requirement], log):
            failures.append("actual %s provider was not attested by the running engine" % requirement)
    for requirement, library in (("vulkan", "libvulkan"), ("sdl3", "libSDL3")):
        if requirement in requirements and not any(
                re.fullmatch(re.escape(library) + r"\.so(?:\.\d+)*",
                             Path(path.removesuffix(" (deleted)")).name) for path in loaded):
            failures.append("running process did not map " + library)
    return failures


# A continuous drag: one logical size per frame, as an interactive resize
# delivers them. Only the synchronous mode resizes on every one of these frames.
RESIZE_DRAG_WORKLOAD = tuple((800 + 20 * step, 600 + 10 * step) for step in range(24))

# Main-thread cost of one resize request. Queued: publication only, the render
# worker does the work. Sync: the resize itself, which must fit in one 60 Hz
# frame for resizing to stay smooth. The engine reports the path each request
# took (`path=worker|main`): in queued mode a frame without the worker (after a
# screenshot) applies the resize on the main thread, under the sync budget.
RESIZE_REQUEST_BUDGET_US = {"queued": 2000, "sync": 16667}
RESIZE_PATH_MODE = {"worker": "queued", "main": "sync"}

# Longest run of consecutive scaled presents a queued resize may show. Until the
# render worker applies a settled resize, frames of the old size are scaled to
# the new drawable: the engine's 60 ms settle (UpdateWindowSize) is 4 frames at
# the workload's fps_max 60, plus the frame that requests the resize, the frame
# the worker renders at the old size, and one frame of margin. A back buffer
# that never follows its drawable scales every frame at that size and fails.
QUEUED_SCALED_PRESENT_RUN_LIMIT = 7

# Window systems where the server resizes the window before the client learns
# its new size. A frame rendered in between is scaled to the new window: during
# a continuous drag every frame can be, however promptly the renderer resizes.
# A settled queued resize there also scales until the client sees the new size
# (one frame) and its swapchain follows it (one frame): measured 7 on X11
# against 5 on Wayland (2026-09-26).
SERVER_SIZED_WINDOW_DRIVERS = ("x11",)
SERVER_SIZED_SCALED_PRESENT_RUN_ALLOWANCE = 2
PRESENT_CENSUS = r"\[vulkan\] presents=(\d+) scaled=(\d+)(?: longest_scaled_run=(\d+))?"


def settled_present_census(log, expected):
    """The renderer's present census at the last settled step, before any drag.

    The renderer prints its census (on stderr, not interleaved with the console
    log) with every native screenshot, and the workload's first screenshots are
    one per settled step."""
    census = re.findall(PRESENT_CENSUS, log)
    return census[len(expected) - 1] if expected and len(census) >= len(expected) else None


def inspect_resize(log, screenshots, expected=RESIZE_WORKLOAD, trace_records=(), mode="queued",
                   require_unscaled=False, sweep=()):
    """Require exact logical/drawable convergence and nonblocking full-frame presents.

    `require_unscaled` also requires the renderer's present census. A synchronous
    resize applies before the frame renders, so no present up to the last
    settled step may scale its back buffer to the window, nor during the drag
    unless the window system sizes windows itself (`SERVER_SIZED_WINDOW_DRIVERS`).
    A queued resize settles first, so frames of the old size are scaled until
    the worker applies it; no run of consecutive scaled presents may outlast
    that (`QUEUED_SCALED_PRESENT_RUN_LIMIT`)."""
    observed = [(int(lw), int(lh), int(dw), int(dh)) for lw, lh, dw, dh in re.findall(
        r"RFC0001 resize observed: logical=(\d+)x(\d+) drawable=(\d+)x(\d+)", log)]
    queued = [(int(serial), int(width), int(height), int(micros))
              for serial, width, height, micros in re.findall(
                  r"RFC0001 resize queued: serial=(\d+) drawable=(\d+)x(\d+) request_us=(\d+)", log)]
    ui_micros = [int(value) for value in re.findall(
        r"RFC0001 resize queued: serial=\d+ drawable=\d+x\d+ request_us=\d+ ui_us=(\d+)", log)]
    completed = [(int(serial), int(width), int(height), int(wait))
                 for serial, width, height, wait in re.findall(
                     r"RFC0001 resize complete: serial=(\d+) drawable=(\d+)x(\d+) main_wait_us=(\d+)", log)]
    failures = []
    if not observed:
        failures.append("no SDL logical/drawable resize was observed")
    for logical_width, logical_height in expected:
        extents = [(dw, dh) for lw, lh, dw, dh in observed
                   if (lw, lh) == (logical_width, logical_height)]
        if not extents:
            failures.append("SDL window did not reach logical %dx%d" %
                            (logical_width, logical_height))
            continue
        drawable = extents[-1]
        requests = [item for item in queued if item[1:3] == drawable]
        if not requests:
            failures.append("renderer did not queue drawable %dx%d" % drawable)
            continue
        serial = requests[-1][0]
        if not any(item[:3] == (serial,) + drawable for item in completed):
            failures.append("renderer did not complete drawable %dx%d" % drawable)
        frames = [frame for frame in screenshots
                  if (frame.get("width"), frame.get("height")) == drawable]
        if not frames or not all(frame.get("has_scene_detail", False) for frame in frames):
            failures.append("missing or blank resize image at drawable %dx%d" % drawable)
    if mode == "queued" and sweep:
        for (width, height), _offset in sweep:
            extents = [(dw, dh) for lw, lh, dw, dh in observed if (lw, lh) == (width, height)]
            if not extents:
                failures.append("SDL window did not reach sweep size %dx%d" % (width, height))
            elif not any(item[1:3] == extents[-1] for item in completed):
                failures.append("renderer did not complete sweep drawable %dx%d "
                                "(a screenshot during the settle)" % extents[-1])
    if mode == "sync":
        # Every drawable seen, the drag's included, is resized on its frame, and
        # every image the run captured, the ones taken mid-drag included, is a
        # complete frame of exactly a size the renderer switched to.
        completed_sizes = {item[1:3] for item in completed}
        for _lw, _lh, dw, dh in observed:
            if (dw, dh) not in completed_sizes:
                failures.append("renderer did not resize to observed drawable %dx%d" % (dw, dh))
        for frame in screenshots:
            size = (frame.get("width"), frame.get("height"))
            if size not in completed_sizes or not frame.get("has_scene_detail", False):
                failures.append("image %dx%d is blank or not a size the renderer resized to"
                                % size)
    budget = RESIZE_REQUEST_BUDGET_US[mode]
    paths = {int(serial): path for serial, path in re.findall(
        r"RFC0001 resize queued: serial=(\d+) .*? path=(\w+)", log)}
    for serial, width, height, micros in sorted(set(queued)):
        path_budget = RESIZE_REQUEST_BUDGET_US[RESIZE_PATH_MODE.get(paths.get(serial), mode)]
        if micros > path_budget:
            failures.append("main-thread resize request %dx%d exceeded %dus (%s path)"
                            % (width, height, path_budget, paths.get(serial, mode)))
    presents = re.findall(PRESENT_CENSUS, log)
    driver = re.search(r"RFC0001 window: provider=\w+ driver=(\w+)", log)
    driver = driver.group(1) if driver else None
    scaled = scaled_run = None
    if require_unscaled:
        if not presents:
            failures.append("renderer reported no present census")
        elif mode == "sync":
            scaled = int(presents[-1][1])
            settled = settled_present_census(log, expected)
            if settled is None:
                failures.append("renderer reported no present census at the settled sizes")
            elif int(settled[1]):
                failures.append("%d presents scaled the back buffer to the window at settled "
                                "sizes" % int(settled[1]))
            if scaled and driver not in SERVER_SIZED_WINDOW_DRIVERS:
                failures.append("%d presents scaled the back buffer to the window" % scaled)
        elif not presents[-1][2]:
            scaled = int(presents[-1][1])
            failures.append("renderer reported no longest scaled-present run")
        else:
            scaled, scaled_run = int(presents[-1][1]), int(presents[-1][2])
            limit = QUEUED_SCALED_PRESENT_RUN_LIMIT
            if driver in SERVER_SIZED_WINDOW_DRIVERS:
                limit += SERVER_SIZED_SCALED_PRESENT_RUN_ALLOWANCE
            if scaled_run > limit:
                failures.append("%d consecutive presents scaled the back buffer to the window "
                                "(a settled queued resize allows %d)" % (scaled_run, limit))
    if any(item[3] != 0 for item in completed):
        failures.append("main thread waited for a renderer resize")
    presents = [record for record in trace_records if record.get("event") == "present"]
    if trace_records and not presents:
        failures.append("resize trace contains no presentation events")
    if any(record.get("cropped") is not False for record in presents):
        failures.append("resize used cropped or unclassified presentation")
    return {"schema": "source-resize-evidence/v1", "status": "fail" if failures else "pass",
            "mode": mode, "window_driver": driver, "request_budget_us": budget,
            "scaled_presents": scaled,
            "longest_scaled_present_run": scaled_run,
            # Main-thread UI relayout after each resize; measured, not budgeted.
            "max_ui_relayout_us": max(ui_micros) if ui_micros else None,
            "requested_logical_sizes": list(expected), "observed_extents": observed,
            "queued_resizes": queued, "resize_paths": paths, "completed_resizes": completed,
            "failures": failures,
            "coverage": "SDL logical and drawable extents, lock-free main-thread publication, render-worker completion, exact nonblank backbuffers, and uncropped presentation."}


def resize_commands(workload=RESIZE_WORKLOAD, mode="queued"):
    """Return one-line script commands so the engine command buffer honors `wait`.

    `mode` selects the material system's threading: "queued" resizes on the
    render worker once the extent settles, "sync" on the frame it changes."""
    # `wait` counts command-buffer passes, not rendered frames. A frame runs at
    # least two passes, and after a slow frame (a native screenshot renders and
    # reads back a whole frame) the host catches up with a tick, and its passes,
    # for each 15 ms it fell behind. A step's whole wait could then pass in one
    # frame: the next size arrived before the last one settled, or in the frame
    # that took the screenshot, which then captured the next size. A fixed frame
    # time (one 15 ms tick per frame) removes the catch-up, and each wait keeps a
    # margin of several frames: the settle and the render worker after a resize,
    # and a frame boundary between a screenshot and the next resize.
    commands = ["mat_queue_mode %d" % (2 if mode == "queued" else 0), "host_framerate 0.015",
                "wait 120"]
    for width, height in workload:
        commands += ["mat_resizewindow %d %d" % (width, height),
                     "wait 30", "screenshot", "wait 6"]
    if mode == "queued":
        for (width, height), offset in RESIZE_SETTLE_SWEEP:
            commands += ["mat_resizewindow %d %d" % (width, height)]
            commands += ["wait %d" % offset] if offset else []
            commands += ["screenshot", "wait 30"]
    if mode == "sync":
        # Screenshots taken mid-drag are frames the window really presented.
        for step, (width, height) in enumerate(RESIZE_DRAG_WORKLOAD):
            commands += ["mat_resizewindow %d %d" % (width, height), "wait 1"]
            if step % 4 == 3:
                commands += ["screenshot"]
        commands += ["wait 12", "screenshot"]
    return commands + ["wait 10", "quit"]


# `exec` reads a script line into com_token (1024 bytes); the rest of a longer
# line would run at once as a separate line, `quit` included.
RESIZE_SCRIPT_LINE_LIMIT = 900


def install_resize_script(stage, workload=RESIZE_WORKLOAD, mode="queued", game="portal"):
    """Install one parsed script line, preserving delayed commands after `exec`.

    A workload longer than one line continues in chained scripts, each line
    ending in `exec` of the next, which runs only when its turn comes."""
    commands = resize_commands(workload, mode)
    return install_command_script(stage, "rfc0001_resize_e2e", commands, game)


def install_command_script(stage, name, commands, game="portal"):
    """Keep waits ordered even when an exec workload exceeds one parser line."""
    directory = Path(stage) / game / "cfg"
    directory.mkdir(parents=True, exist_ok=True)
    chunks, current = [], []
    for command in commands:
        if "\n" in command or "\r" in command or len(command) > RESIZE_SCRIPT_LINE_LIMIT - 80:
            raise ValueError("console command exceeds the parser line limit")
        candidate = current + [command, "exec %s_%d" % (name, len(chunks) + 1)]
        if current and len("; ".join(candidate)) > RESIZE_SCRIPT_LINE_LIMIT:
            chunks.append(current)
            current = []
        current.append(command)
    chunks.append(current)
    paths = []
    for index, chunk in enumerate(chunks):
        script_name = name + ("_%d" % index if index else "")
        if index + 1 < len(chunks):
            chunk = chunk + ["exec %s_%d" % (name, index + 1)]
        path = directory / (script_name + ".cfg")
        path.write_text("; ".join(chunk) + "\n")
        paths.append(path)
    return {"path": str(paths[0].relative_to(stage)), "sha256": sha256(paths[0]),
            "chained": [{"path": str(path.relative_to(stage)), "sha256": sha256(path)}
                        for path in paths[1:]],
            "commands": commands}


def inspect_provider_catalog(log):
    """Require executed providers and loader telemetry, not requested CLI flags."""
    required = {
        "input": r"RFC0001 input: provider=sdl3\b",
        "audio": r"RFC0001 audio: provider=sdl3\b",
        "shaders": r"RFC0001 shaders: provider=source-standard-materials shaders=[1-9][0-9]*\b",
        "video": r"RFC0001 video: providers=0\b",
    }
    failures = ["selected %s catalog provider did not initialize" % name
                for name, marker in required.items() if not re.search(marker, log)]
    records = [line for line in log.splitlines() if line.startswith("ModuleLoadTelemetry: ")]
    if not records:
        failures.append("provider retirement requires runtime loader telemetry")
    # These were first-party backend discovery names. Ordinary ELF/PE linkage
    # maps their libraries without passing through the instrumented module loader.
    forbidden = re.compile(r"(?:^|[/\\])(?:lib)?(?:stdshader_[^/\\ ]*|shaderapi[^/\\ ]*|"
                           r"video_(?:bink|webm|quicktime)|vaudio_(?:minimp3|opus|speex|celt))(?:\.[^/\\ ]*)?$", re.IGNORECASE)
    attempted = []
    for record in records:
        match = re.search(r"\brequested=(.*?) resolved=", record)
        if not match:
            failures.append("malformed runtime loader telemetry")
            continue
        if forbidden.search(match.group(1)):
            attempted.append(match.group(1))
    if attempted:
        failures.append("first-party backend runtime discovery: " + ", ".join(sorted(set(attempted))))
    return {"status": "fail" if failures else "pass", "failures": failures,
            "telemetry_records": len(records), "forbidden_attempts": sorted(set(attempted))}


def inspect_render_trace(path, report_path):
    """A produced trace must also be complete and free of observed render failures."""
    try:
        events, digest = render_trace.read_events(path)
        report = render_trace.analyze(events)
        report["input"] = {"path": str(Path(path).resolve()), "sha256": digest}
    except render_trace.TraceError as error:
        report = {"schema": render_trace.REPORT_SCHEMA, "status": "invalid",
                  "error": str(error), "visibility_verified": False,
                  "limitation": render_trace.LIMITATION}
    Path(report_path).write_text(json.dumps(report, indent=2, sort_keys=True) + "\n")
    return report


def run_kiln(args, exact_arguments, stage, environment, timeout, output):
    """The kiln.api run of the profile from the private runtime (RFC 0027):
    the harness owns its test command (exact arguments) and sandbox
    variables; kiln owns the program, display session and run provider.
    Watches the game's mapped files (the loaded-module evidence)."""
    wrapper = []
    if exact_arguments[0] != "./hl2_launcher":
        index = exact_arguments.index("./hl2_launcher")
        wrapper, exact_arguments = exact_arguments[:index], exact_arguments[index:]
    loaded = set()

    def watch(pid):
        try:
            for line in Path("/proc/%d/maps" % pid).read_text().splitlines():
                fields = line.split(None, 5)
                if len(fields) == 6 and fields[5].startswith("/"):
                    loaded.add(fields[5])
        except OSError:
            pass
    code, timed_out, seconds, error = sepipe_loader.run_test(
        args.profile, args.flavor, stage, exact_arguments[1:], output, timeout,
        environment=environment, wrapper=wrapper, display="none" if args.headless else "user",
        watch=watch)
    if error:
        with Path(output).open("a") as stream:
            stream.write("\nkiln: %s\n" % error)
    return code, timed_out, sorted(loaded), seconds


def login_session_displays():
    """The login session's WAYLAND_DISPLAY and DISPLAY, from the systemd user
    manager; empty where there is no user manager (CI, containers)."""
    try:
        result = subprocess.run(["systemctl", "--user", "show-environment"],
                                capture_output=True, text=True, timeout=5)
    except (OSError, subprocess.SubprocessError):
        return {}
    if result.returncode != 0:
        return {}
    shown = dict(line.split("=", 1) for line in result.stdout.splitlines() if "=" in line)
    return {key: shown[key] for key in ("WAYLAND_DISPLAY", "DISPLAY") if shown.get(key)}


def user_display_in_use(environment, login):
    """The login-session display variables a windowed run would use: a window
    there opens on the user's live desktop. Empty for offscreen runs and for
    a private compositor's own displays."""
    if environment.get("SDL_VIDEODRIVER") == "offscreen":
        return []
    return ["%s=%s" % (key, environment[key]) for key, value in sorted(login.items())
            if environment.get(key) == value]


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--profile", required=True,
                        help="kiln profile (e.g. portal, portal2): kiln packages it into the "
                             "private runtime and the game runs through kiln.api (RFC 0027)")
    parser.add_argument("--flavor", default="dev", help="the kiln profile's build flavor")
    parser.add_argument("--content-root", type=Path,
                        help="private maps/ and materials/ files added to the staged game")
    parser.add_argument("--material-root", type=Path,
                        help="private materials/ files added to the staged game without a map "
                             "(e.g. the proxy corpus fixture materials)")
    parser.add_argument("--shader-artifacts", type=Path,
                        help="overlay source-matched shader artifacts into the private runtime")
    parser.add_argument("--render-trace", action="store_true",
                        help="retain renderer diagnostics in render-trace.jsonl")
    parser.add_argument("--draw-state-fixtures", action="store_true",
                        help="write the screenshot frame's per-draw state (source-draw-state/v1) "
                             "to draw-state/ for cross-backend comparison")
    parser.add_argument("--view-oracle", action="store_true",
                        help="write each screenshot frame's views and draws "
                             "(source-view-oracle/v1, RFC 0016 K0) to vo/; "
                             "tools/render/view_oracle.py compares them")
    parser.add_argument("--renderdoc", action="store_true",
                        help="run under RenderDoc (renderdoccmd) and capture the frame after "
                             "the final screenshot command into renderdoc/*.rdc "
                             "(tools/renderdoc/rdc.py inspects it)")
    parser.add_argument("--shader-debug", action="store_true",
                        help="run the native backend's debug shader variants: GLSL names and "
                             "source-level debug information for capture tools "
                             "(regen_material_spv.py --debug-out into shader-debug/, "
                             "SOURCE_VK_SHADER_DIR)")
    parser.add_argument("--resize-stress", action="store_true",
                        help="resize the actual native game window through a versioned workload")
    parser.add_argument("--resize-mode", choices=("queued", "sync"), default="queued",
                        help="material-system threading for --resize-stress: queued (render "
                             "worker, settled resizes) or sync (resized on the frame it changes)")
    parser.add_argument("--require-gtk-decoration", action="store_true",
                        help="require the native libdecor GTK plugin mapped by the product")
    parser.add_argument("--require-provider-catalog", action="store_true",
                        help="require initialized profile providers and no first-party backend loading")
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--timeout", type=float, default=180)
    parser.add_argument("--capture-wait", type=int, default=600,
                        help="frames between the console commands and the final screenshot and "
                             "quit (a longer cfg sequence, e.g. several screenshots, needs more)")
    parser.add_argument("--allow-user-display", action="store_true",
                        help="permit a window on the login session's own display; without it a "
                             "windowed run must be inside a private compositor (kiln's private "
                             "display session, sepipe_loader.Display)")
    parser.add_argument("--headless", action="store_true",
                        help="render offscreen on the GPU (SDL offscreen driver) with the "
                             "volume muted")
    parser.add_argument("--console-command", action="append", default=[],
                        help="console command to run after the map loads and before the "
                             "screenshot (repeatable). Player commands such as setpos and "
                             "setang need the client prefix: 'cmd setpos 0 0 64'")
    parser.add_argument("--map", default="testchmb_a_00")
    parser.add_argument("--map-after-start", action="store_true",
                        help="wait in the running menu before loading the map; exercises "
                             "renderer state retained across a menu-to-game transition")
    parser.add_argument("--engine-arg", action="append", default=[],
                        help="extra launcher argument placed before +map (repeatable), e.g. "
                             "'-hostframetrace' '<file>'")
    parser.add_argument("--startup-command", action="append", default=[],
                        help="console command run at startup, before +map (repeatable). Written "
                             "to a cfg, which keeps the launcher's 512-character command line short")
    parser.add_argument("--physics", default="vphysics",
                        help="physics provider module name (e.g. vphysics, vphysics_box3d)")
    parser.add_argument("--renderer", default=None,
                        help="render provider id (e.g. native-vulkan, null); engine default if unset")
    parser.add_argument("--no-mouse", action="store_true",
                        help="disable mouse input for repeatable windowed camera captures")
    parser.add_argument("--width", type=int, default=1024)
    parser.add_argument("--height", type=int, default=768)
    for name in ("vulkan", "sdl3", "wayland"):
        parser.add_argument("--require-" + name, action="store_true")
    args = parser.parse_args(argv)
    try:
        args.sepipe = sepipe_loader.load()
        args.session = args.sepipe.Session(str(conformance.repo_root()))
        args.game = sepipe_loader.game_of(args.profile)
    except (sepipe_loader.LoadError, KeyError) as error:
        parser.error("kiln: %s" % error)
    except Exception as error:  # sepipe.KilnError
        parser.error("kiln: %s" % error)
    if args.timeout <= 0 or not re.fullmatch(r"[a-zA-Z0-9_]+", args.map):
        parser.error("timeout must be positive and map must be a simple map name")
    if not re.fullmatch(r"[a-zA-Z0-9_]+", args.physics):
        parser.error("physics must be a simple module name")
    if args.renderer is not None and not re.fullmatch(r"[a-zA-Z0-9_-]+", args.renderer):
        parser.error("renderer must be a simple provider id")
    if not (64 <= args.width <= 8192 and 64 <= args.height <= 8192):
        parser.error("capture dimensions must be between 64 and 8192")
    output = args.out.resolve()
    output.mkdir(parents=True, exist_ok=True)
    if (output / "evidence.json").exists():
        parser.error("evidence already exists; use a new output directory")
    evidence = {"schema": "portal-boot-evidence/v1", "status": "fail",
                "started_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
                "source": conformance.source_identity(conformance.repo_root()),
                "kiln": {"profile": args.profile, "flavor": args.flavor},
                "map": args.map, "game": args.game,
                "requested_resolution": [args.width, args.height]}
    try:
        stage = output / "runtime"
        game = args.game
        # The profile's own package (RFC 0027 linux-dir) in this private runtime.
        shutil.rmtree(stage, ignore_errors=True)
        built = args.session.build(args.profile, flavor=args.flavor, up_to="package",
                                   runtime=str(stage))
        evidence["staging"] = {"kiln": {stage_["name"]: stage_["summary"]
                                        for stage_ in built["stages"]},
                               "tree": built["tree"]}
        private_content = "custom/portal-boot-content"
        if args.content_root or args.material_root:
            gameinfo = stage / game / "gameinfo.txt"
            content_gameinfo = prepend_game_search_path(
                gameinfo.read_text(), game + "/" + private_content)
            if args.content_root:
                evidence["content_overrides"] = install_content(
                    args.content_root, stage, game=game, mount=private_content)
            if args.material_root:
                evidence["material_overrides"] = install_content(
                    args.material_root, stage, game=game, require_map=False,
                    mount=private_content)
            gameinfo.write_text(content_gameinfo)
            evidence["content_mount"] = game + "/" + private_content
        if args.shader_artifacts:
            evidence["shader_overrides"] = install_shader_artifacts(args.shader_artifacts, stage, game=game)
        executable = stage / "hl2_launcher"
        if not executable.is_file():
            raise ValueError("runtime is missing hl2_launcher")
        evidence["executables"] = {str(path.relative_to(stage)): sha256(path)
                                   for path in [executable] + sorted((stage / "bin").glob("*.so"))
                                   + sorted((stage / game / "bin").glob("*.so"))}
        # Relative to the working directory (the staged runtime): the engine
        # refuses command lines over 512 characters, the program path included.
        command = ["./" + executable.name, "-game", game, "-windowed", "-w", str(args.width), "-h", str(args.height),
                   # -multirun: an isolated boot must not collide with (or be refused
                   # by) a game the user is running.
                   "-multirun",
                   "-novid", "-insecure", "-console", "-condebug", "-dev", "-physics", args.physics,
                   # mat_vsync 0: a real present-mode wait must not stretch the boot.
                   "+sv_cheats", "1", "+mat_queue_mode", "0", "+mat_vsync", "0", "+fps_max", "60",
                   *args.engine_arg,
                   *( ["+exec", "portal_boot_startup.cfg"] if args.startup_command else [] ),
                   *( ["+wait", "300"] if args.map_after_start else [] ),
                   "+map", args.map,
                   "+wait", "180", "+status", "+hideconsole", "+developer", "0"]
        if args.startup_command:
            (stage / game / "cfg").mkdir(parents=True, exist_ok=True)
            (stage / game / "cfg/portal_boot_startup.cfg").write_text(
                "".join(line + "\n" for line in args.startup_command))
        # Extra console commands run once the map has loaded, before the capture
        # (e.g. "setpos X Y Z" / "setang P Y R" to frame the same view on every
        # backend).
        if args.console_command:
            # Written to a cfg and exec'd: the command line splits arguments (and
            # reads a negative number as an option), a cfg keeps each line whole.
            # The local player exists only after spawn, hence the wait.
            evidence["console_script"] = install_command_script(
                stage, "portal_boot_commands", args.console_command +
                ["wait %d" % args.capture_wait] +
                (["vk_renderdoc_capture"] if args.renderdoc else []) +
                ["screenshot", "mat_spewvertexandpixelshaders", "mat_hdr_tonemapscale",
                 "wait 10", "quit"], game)
            command += ["+wait", "300", "+exec", "portal_boot_commands.cfg"]
        if not args.console_command:
            command += ["+wait", str(args.capture_wait),
                       # vk_renderdoc_capture: RenderDoc records the next presented
                       # frame, the scene the screenshot shows.
                       *( ["+vk_renderdoc_capture"] if args.renderdoc else [] ),
                       "+screenshot", "+mat_spewvertexandpixelshaders",
                       # The exposure the client's auto-exposure settled on for the
                       # screenshot frame (it writes its goal here every frame).
                       "+mat_hdr_tonemapscale",
                       "+wait", "10", "+quit"]
        if args.renderer:
            command[1:1] = ["-renderer", args.renderer]
        if args.shader_debug:
            shader_dir = output / "shader-debug"
            subprocess.run([sys.executable, str(SHADER_REGEN), "--debug-out", str(shader_dir)],
                           check=True, capture_output=True, text=True)
            manifest = json.loads((shader_dir / "manifest.json").read_text())
            evidence["shader_debug"] = {
                "directory": str(shader_dir),
                "written": sum(entry["status"] == "written" for entry in manifest["entries"]),
                "skipped": [entry["array"] for entry in manifest["entries"]
                            if entry["status"] != "written"]}
            shader_environment = str(shader_dir)
        if args.no_mouse:
            command[1:1] = ["-nomouse"]
        if args.draw_state_fixtures:
            fixture_dir = output / "draw-state"
            fixture_dir.mkdir()
            # Relative to the product's working directory (the staged runtime):
            # the engine refuses command lines over 512 characters.
            command[1:1] = ["-drawstatefixture", os.path.relpath(fixture_dir, stage)]
        if args.view_oracle:
            oracle_dir = output / "vo"  # short: see the 512-character limit above
            oracle_dir.mkdir()
            command[1:1] = ["-vieworacle", os.path.relpath(oracle_dir, stage)]
        if args.resize_stress:
            # Cmd_Exec_f evaluates separate file lines immediately. A single
            # semicolon-delimited line is parsed as one delayed command sequence,
            # so later sizes remain queued behind each `wait`.
            tail = command.index("+wait", command.index("+developer"))
            resize_script = install_resize_script(stage, mode=args.resize_mode, game=game)
            command = command[:tail] + ["-resizetelemetry", "+exec", "rfc0001_resize_e2e"]
            evidence["resize_workload"] = {"version": RESIZE_WORKLOAD_VERSION,
                                           "sizes": RESIZE_WORKLOAD, **resize_script}
        if args.require_provider_catalog:
            command += ["-moduleloadtelemetry"]
        # A throwaway HOME/XDG for the product; the stage is its only game write path.
        sandbox = launch_sandbox.Sandbox(output / "sandbox", write_paths=[stage])
        environment = sandbox.environment(os.environ)
        if args.shader_debug:
            environment["SOURCE_VK_SHADER_DIR"] = shader_environment
        environment["LD_LIBRARY_PATH"] = str(stage / "bin") + ":" + environment.get("LD_LIBRARY_PATH", "")
        environment["SteamAppId"] = environment["SteamGameId"] = \
            "620" if game == "portal2" else "400"
        if args.render_trace:
            trace = output / "render-trace.jsonl"
            if trace.exists():
                raise ValueError("render trace already exists; use a new output directory")
            environment["SOURCE_RENDER_TRACE"] = str(trace)
        requirements = [name for name in ("vulkan", "sdl3", "wayland") if getattr(args, "require_" + name)]
        if args.require_wayland:
            environment["GDK_BACKEND"] = "wayland"
            environment["SDL_VIDEO_DRIVER"] = "wayland"
            environment["SDL_VIDEODRIVER"] = "wayland"
        if args.headless:
            # SDL3's offscreen driver: real GPU rendering through
            # VK_EXT_headless_surface, no window, compositor or display.
            environment["SDL_VIDEODRIVER"] = environment["SDL_VIDEO_DRIVER"] = "offscreen"
            # An offscreen window never has focus; controllers (real or uinput_pad.py) must still be read.
            environment["SDL_JOYSTICK_ALLOW_BACKGROUND_EVENTS"] = "1"
            for variable in ("WAYLAND_DISPLAY", "DISPLAY"):
                environment.pop(variable, None)
        if environment.get("SDL_VIDEODRIVER") == "offscreen":
            # Nobody is watching or listening to a headless run.
            command[command.index("+map"):command.index("+map")] = ["+volume", "0"]
        if args.renderdoc:
            renderdoccmd = shutil.which("renderdoccmd")
            if not renderdoccmd:
                raise ValueError("--renderdoc needs renderdoccmd on PATH")
            (output / "renderdoc").mkdir(exist_ok=True)
            # The launcher re-executes itself to set its library path, so
            # RenderDoc must follow children to reach the game process.
            command = [renderdoccmd, "capture", "--opt-hook-children", "-w", "-c",
                       str(output / "renderdoc" / args.map)] + command
        shared = user_display_in_use(environment, login_session_displays())
        if shared and not args.allow_user_display:
            raise ValueError("this run would open a window on the user's live desktop (%s); use "
                             "--headless, or run inside a private compositor "
                             "(kiln's private display session, sepipe_loader.Display), or pass "
                             "--allow-user-display" % ", ".join(shared))
        evidence["command"] = command
        evidence["requirements"] = requirements
        evidence["display_environment"] = {key: environment.get(key) for key in
                                           ("DISPLAY", "WAYLAND_DISPLAY", "SDL_VIDEODRIVER", "GDK_BACKEND")}
        code, timed_out, loaded, seconds = run_kiln(args, command, stage, environment,
                                                    args.timeout, output / "stdout.log")
        evidence["sandbox"] = sandbox.finish()
        log_paths = [output / "stdout.log", stage / "engine.log", stage / game / "console.log"]
        log = "\n".join(path.read_text(errors="replace") for path in log_paths if path.is_file())
        screenshots = [info for path in sorted(stage.rglob("screenshots/*.tga"))
                       if (info := screenshot_info(path))]
        failures = evaluate(log, screenshots, code, timed_out, args.map, requirements, loaded,
                            args.renderer)
        if args.require_gtk_decoration:
            decorated = any(Path(path).name == "libdecor-gtk.so" for path in loaded)
            evidence["gtk_decoration"] = {"plugin_mapped": decorated, "gdk_backend": environment.get("GDK_BACKEND")}
            if not decorated:
                failures.append("native GNOME GTK decoration plugin was not mapped")
        if args.resize_stress:
            trace_records = []
            if args.render_trace and trace.is_file():
                trace_records = [json.loads(line) for line in trace.read_text().splitlines() if line]
            evidence["resize"] = inspect_resize(
                log, screenshots, trace_records=trace_records, mode=args.resize_mode,
                require_unscaled=args.renderer == "core", sweep=RESIZE_SETTLE_SWEEP)
            failures.extend(evidence["resize"]["failures"])
        if args.require_provider_catalog:
            evidence["provider_catalog"] = inspect_provider_catalog(log)
            failures.extend(evidence["provider_catalog"]["failures"])
        if args.renderdoc:
            captures = sorted((output / "renderdoc").glob("*.rdc"))
            evidence["renderdoc"] = {"captures": [str(path) for path in captures]}
            if not captures:
                failures.append("requested RenderDoc capture was not written")
        if args.draw_state_fixtures:
            fixtures = sorted(fixture_dir.glob("*.jsonl"))
            evidence["draw_state_fixtures"] = [{"path": str(path), "sha256": sha256(path),
                                                "bytes": path.stat().st_size} for path in fixtures]
            if not fixtures:
                failures.append("requested draw-state fixtures were not written")
        if args.view_oracle:
            captures = sorted(oracle_dir.glob("*.jsonl"))
            evidence["view_oracle_captures"] = [{"path": str(path), "sha256": sha256(path),
                                                 "bytes": path.stat().st_size}
                                                for path in captures]
            if not captures:
                failures.append("requested view oracle captures were not written")
        if args.render_trace:
            if not trace.is_file() or trace.stat().st_size == 0:
                failures.append("requested renderer trace was not produced")
            else:
                evidence["render_trace"] = {"path": str(trace), "sha256": sha256(trace),
                                            "bytes": trace.stat().st_size}
                report_path = output / "render-report.json"
                report = inspect_render_trace(trace, report_path)
                evidence["render_trace"].update(report=str(report_path), status=report["status"])
                if report["status"] != "complete":
                    failures.append("renderer diagnostic capture: " + report["status"])
        evidence["tonemap_scale"] = tonemap_scale(log)
        evidence.update(returncode=code, timed_out=timed_out, elapsed_seconds=seconds,
                        loaded_files=loaded, screenshots=screenshots, failures=failures,
                        logs=[str(path) for path in log_paths if path.is_file()],
                        status="fail" if failures else "pass")
    except (OSError, ValueError) as error:
        evidence["failures"] = [str(error)]
    (output / "evidence.json").write_text(json.dumps(evidence, indent=2) + "\n")
    print("Portal boot: %s (%s)" % (evidence["status"], output / "evidence.json"))
    for failure in evidence.get("failures", []):
        print("  " + failure)
    return 0 if evidence["status"] == "pass" else 1


if __name__ == "__main__":
    sys.exit(main())
