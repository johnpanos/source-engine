#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Run the installed Portal product in an isolated, evidenced writable tree."""

import argparse
import ast
import datetime
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import signal
import struct
import subprocess
import sys
import time

import conformance
import render_trace
import product_profile


IMMUTABLE_ASSETS = {
    ".vpk", ".bsp", ".vtf", ".vmt", ".mdl", ".vvd", ".vtx", ".phy",
    ".wav", ".mp3", ".ogg", ".webm", ".bik",
}
EXCLUDED_DIRECTORIES = {"screenshots", "save", "logs", "dumps"}
# Version 1 exercises grow, shrink, aspect changes and non-aligned dimensions.
RESIZE_WORKLOAD = ((640, 480), (801, 601), (1024, 576), (1279, 719),
                   (960, 720), (641, 479), (1280, 800), (1001, 701),
                   (800, 600), (1200, 675), (721, 541), (1024, 768))



def sha256(path):
    digest = hashlib.sha256()
    with Path(path).open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def stage_runtime(runtime, stage, game="portal", content_only=False):
    """Only immutable asset files are shared; every writable directory is private."""
    runtime, stage = Path(runtime).resolve(), Path(stage).resolve()
    if not runtime.is_dir() or not (runtime / game / "gameinfo.txt").is_file():
        raise ValueError("runtime must contain %s/gameinfo.txt" % game)
    if stage == runtime or runtime in stage.parents:
        raise ValueError("staging directory must be outside the original runtime")
    stage.mkdir(parents=True, exist_ok=False)
    count = {"copied": 0, "shared_assets": 0}
    for directory, directories, filenames in os.walk(runtime, followlinks=False):
        relative = Path(directory).relative_to(runtime)
        if content_only:
            if relative == Path("."):
                directories[:] = [name for name in directories if name in {game, "platform"}]
                filenames = []
            elif relative == Path(game):
                directories[:] = [name for name in directories if name != "bin"]
        directories[:] = sorted(name for name in directories
                                if name.lower() not in EXCLUDED_DIRECTORIES)
        destination = stage / relative
        destination.mkdir(parents=True, exist_ok=True)
        for name in sorted(filenames):
            source = Path(directory) / name
            if source.suffix.lower() in {".log", ".dmp"}:
                continue
            target = destination / name
            if source.suffix.lower() in IMMUTABLE_ASSETS:
                target.symlink_to(source.resolve())
                count["shared_assets"] += 1
            else:
                shutil.copy2(source, target, follow_symlinks=True)
                count["copied"] += 1
    return count


def content_files(content_root):
    """Validated (source, relative) files of a compiled map content root."""
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
    if not any(relative.parts[0] == "maps" for _, relative in files):
        raise ValueError("content root has no map")
    return files


def install_content(content_root, stage, game="portal"):
    """Overlay a private compiled map and its materials into a staged game."""
    plan = []
    for source, relative in content_files(content_root):
        target = stage / game / relative
        if target.exists() or target.is_symlink():
            raise ValueError("private content would replace installed content: " + str(relative))
        plan.append((source, relative, target))
    installed = {}
    for source, relative, target in plan:
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source, target)
        installed[str(relative)] = {"source": str(source), "sha256": sha256(target)}
    return installed


def host_tool_roots(source_root=None):
    """Install roots that host-tool profiles declare; they hold no game products."""
    source_root = Path(source_root or conformance.repo_root()).resolve()
    roots = set()
    for path in sorted((source_root / "quality/product_profiles").glob("*.json")):
        profile = json.loads(path.read_text())
        if profile.get("schema") != "source-host-tool-profile/v1":
            continue
        root = profile.get("layout", {}).get("root")
        if root is not None:
            if not isinstance(root, str) or Path(root).is_absolute():
                raise ValueError("host-tool layout root must be repository-relative: " + path.name)
            roots.add((source_root / root).resolve())
    return roots


def install_build(build, stage, game="portal", launcher_name="hl2_launcher", tool_roots=None):
    """Overlay Waf products, keeping game modules in the selected gamebin."""
    if launcher_name not in {"hl2_launcher", "dedicated_launcher"}:
        raise ValueError("unsupported launcher: " + launcher_name)
    build, stage = Path(build).resolve(), Path(stage)
    if not build.is_dir():
        raise ValueError("build output directory is missing")
    # Some workflows keep independent Waf profiles below the primary output
    # tree (for example build/pbr-native), and host-tool profiles may install
    # below it (build/toolchains). Neither holds products of this build, so
    # neither may participate in its staging plan.
    excluded = {cache.parent for cache in build.rglob("c4che")
                if cache.is_dir() and cache.parent != build}
    excluded |= {Path(root).resolve() for root in
                 (host_tool_roots() if tool_roots is None else tool_roots)}

    def belongs_to_active_build(path):
        return not any(root in path.parents for root in excluded)

    products = sorted(path for path in build.rglob("*.so")
                      if path.is_file() and belongs_to_active_build(path))
    launchers = sorted(path for path in build.rglob(launcher_name)
                       if path.is_file() and belongs_to_active_build(path))
    if not products or len(launchers) != 1:
        raise ValueError("build must contain shared libraries and exactly one " + launcher_name)
    sources = {}
    # Waf gives single-game products an unqualified game/client output path.
    # Read only its literal game selection, never execute the Python cache.
    selected_games = set()
    native_library_paths, native_roots, profiles = set(), set(), set()
    for cache in (build / "c4che").rglob("*_cache.py"):
        for line in cache.read_text().splitlines():
            key, separator, literal = line.partition(" = ")
            if not separator or key not in {"GAMES", "LIBPATH_DXVK", "DXVK_ROOT", "PRODUCT_PROFILE"}:
                continue
            try:
                value = ast.literal_eval(literal)
            except (ValueError, SyntaxError) as error:
                raise ValueError("invalid literal Waf setting: " + key) from error
            if key == "LIBPATH_DXVK":
                if not isinstance(value, list) or not all(isinstance(item, str) for item in value):
                    raise ValueError("invalid Waf DXVK search paths")
                native_library_paths.update(Path(item).resolve() for item in value)
            else:
                if not isinstance(value, str):
                    raise ValueError("invalid literal Waf setting: " + key)
                {"GAMES": selected_games, "DXVK_ROOT": native_roots,
                 "PRODUCT_PROFILE": profiles}[key].add(value)
    if native_library_paths:
        if len(native_roots) != 1 or len(profiles) != 1:
            raise ValueError("DXVK staging requires one configured product profile and dependency root")
        profile = product_profile.load_profile(next(iter(profiles)))
        prefix = Path(next(iter(native_roots)))
        dependency = profile["dependencies"]["dxvk_native"]
        product_profile.verify_dependency(profile, "dxvk_native", prefix)
        directory = (prefix / dependency["library_directory"]).resolve()
        if native_library_paths != {directory}:
            raise ValueError("DXVK search paths differ from the verified product profile")
        for library in sorted(directory.glob("lib%s.so*" % dependency["link_library"])):
            sources["bin/" + library.name] = library
    for source in products + launchers:
        relative = source.relative_to(build)
        if source.name == launcher_name:
            destination = stage / source.name
        elif source.name in {"client.so", "libclient.so", "server.so", "libserver.so"}:
            if game not in relative.parts and not (
                    relative.parts[:2] in (("game", "client"), ("game", "server"))
                    and len(relative.parts) == 3 and selected_games == {game}):
                continue
            destination = stage / game / "bin" / source.name
        else:
            destination = stage / "bin" / source.name
        key = str(destination.relative_to(stage))
        if key in sources:
            raise ValueError("ambiguous build output for " + key)
        sources[key] = source
    # Validate the full plan before replacing any staged product.
    installed = {}
    for key, source in sources.items():
        destination = stage / key
        destination.parent.mkdir(parents=True, exist_ok=True)
        destination.unlink(missing_ok=True)
        shutil.copy2(source, destination)
        installed[key] = {"source": str(source), "sha256": sha256(destination)}
    return installed


def shader_search_path(gameinfo):
    """Put the named shader overlay before packaged content in staged GameInfo."""
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
    entry = "\n\t\t\tgame+mod\t\tportal/custom/source-engine-shaders\n"
    return gameinfo[:position] + entry + gameinfo[position:]


def install_shader_artifacts(artifacts, stage, source_root=None):
    """Validate source-matched compiler output before replacing staged shaders."""
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
    gameinfo = stage / "portal/gameinfo.txt"
    staged_gameinfo = shader_search_path(gameinfo.read_text())
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
        destination = stage / "portal/custom/source-engine-shaders" / relative
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
            "search_path": "portal/custom/source-engine-shaders"}


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
             renderer=None, expect_dark=False):
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
    elif expect_dark:
        # A legitimately dark view (an unlit room) cannot show scene detail;
        # the engine must instead attest that it drew the world mesh.
        if not re.search(r"WMSH draw path active \([1-9][0-9]* material batches", log):
            failures.append("dark capture has no WMSH draw attestation")
    elif not any(frame.get("has_scene_detail", False) for frame in screenshots):
        failures.append("engine capture lacks scene detail (blank or almost entirely black/white)")
    if re.search(r"Couldn't load (?:combo|vertex shader|pixel shader)|Using invalid shader combo", log):
        failures.append("required shader artifact or permutation was unavailable")
    markers = {
        "vulkan": (r"\[NativeVulkan\] IShaderAPI::SetMode: device [^\n]+ up\b"
                   if renderer == "native-vulkan" else
                   r"RFC0001 renderer: provider=vulkan-compat\b"),
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
# frame for resizing to stay smooth.
RESIZE_REQUEST_BUDGET_US = {"queued": 2000, "sync": 16667}


def inspect_resize(log, screenshots, expected=RESIZE_WORKLOAD, trace_records=(), mode="queued",
                   require_unscaled=False):
    """Require exact logical/drawable convergence and nonblocking full-frame presents.

    `require_unscaled` also requires the renderer's census to report that no
    present scaled its back buffer to the window (`[vulkan] presents=N scaled=0`)."""
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
    if queued and max(item[3] for item in queued) > budget:
        failures.append("main-thread resize request exceeded %dus" % budget)
    presents = re.findall(r"\[vulkan\] presents=(\d+) scaled=(\d+)", log)
    scaled = None
    if require_unscaled:
        if not presents:
            failures.append("renderer reported no present census")
        else:
            scaled = int(presents[-1][1])
            if scaled:
                failures.append("%d presents scaled the back buffer to the window" % scaled)
    if any(item[3] != 0 for item in completed):
        failures.append("main thread waited for a renderer resize")
    presents = [record for record in trace_records if record.get("event") == "present"]
    if trace_records and not presents:
        failures.append("resize trace contains no presentation events")
    if any(record.get("cropped") is not False for record in presents):
        failures.append("resize used cropped or unclassified presentation")
    return {"schema": "source-resize-evidence/v1", "status": "fail" if failures else "pass",
            "mode": mode, "request_budget_us": budget, "scaled_presents": scaled,
            # Main-thread UI relayout after each resize; measured, not budgeted.
            "max_ui_relayout_us": max(ui_micros) if ui_micros else None,
            "requested_logical_sizes": list(expected), "observed_extents": observed,
            "queued_resizes": queued, "completed_resizes": completed, "failures": failures,
            "coverage": "SDL logical and drawable extents, lock-free main-thread publication, render-worker completion, exact nonblank backbuffers, and uncropped presentation."}


def resize_commands(workload=RESIZE_WORKLOAD, mode="queued"):
    """Return one-line script commands so the engine command buffer honors `wait`.

    `mode` selects the material system's threading: "queued" resizes on the
    render worker once the extent settles, "sync" on the frame it changes."""
    commands = ["mat_queue_mode %d" % (2 if mode == "queued" else 0), "wait 120"]
    for width, height in workload:
        commands += ["mat_resizewindow %d %d" % (width, height),
                     "wait 12", "screenshot", "wait 1"]
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


def install_resize_script(stage, workload=RESIZE_WORKLOAD, mode="queued"):
    """Install one parsed script line, preserving delayed commands after `exec`.

    A workload longer than one line continues in chained scripts, each line
    ending in `exec` of the next, which runs only when its turn comes."""
    directory = Path(stage) / "portal/cfg"
    directory.mkdir(parents=True, exist_ok=True)
    commands = resize_commands(workload, mode)
    chunks, current = [], []
    for command in commands:
        candidate = current + [command, "exec rfc0001_resize_e2e_%d" % (len(chunks) + 1)]
        if current and len("; ".join(candidate)) > RESIZE_SCRIPT_LINE_LIMIT:
            chunks.append(current)
            current = []
        current.append(command)
    chunks.append(current)
    paths = []
    for index, chunk in enumerate(chunks):
        name = "rfc0001_resize_e2e" + ("_%d" % index if index else "")
        if index + 1 < len(chunks):
            chunk = chunk + ["exec rfc0001_resize_e2e_%d" % (index + 1)]
        path = directory / (name + ".cfg")
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


def run_product(command, stage, environment, timeout, output):
    loaded = set()
    started = time.monotonic()
    timed_out = False
    with Path(output).open("wb") as stream:
        process = subprocess.Popen(command, cwd=stage, env=environment, stdout=stream,
                                   stderr=subprocess.STDOUT, start_new_session=True)
        while process.poll() is None:
            try:
                for line in Path("/proc/%d/maps" % process.pid).read_text().splitlines():
                    fields = line.split(None, 5)
                    if len(fields) == 6 and fields[5].startswith("/"):
                        loaded.add(fields[5])
            except OSError:
                pass
            if time.monotonic() - started > timeout:
                timed_out = True
                os.killpg(process.pid, signal.SIGTERM)
                try:
                    process.wait(timeout=5)
                except subprocess.TimeoutExpired:
                    os.killpg(process.pid, signal.SIGKILL)
                break
            time.sleep(0.1)
        returncode = process.wait()
    return returncode, timed_out, sorted(loaded), time.monotonic() - started


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--runtime", type=Path, required=True)
    parser.add_argument("--build", type=Path)
    parser.add_argument("--content-root", type=Path,
                        help="private maps/ and materials/ files added to the staged game")
    parser.add_argument("--shader-artifacts", type=Path,
                        help="overlay source-matched shader artifacts into the private runtime")
    parser.add_argument("--render-trace", action="store_true",
                        help="retain renderer diagnostics in render-trace.jsonl")
    parser.add_argument("--draw-state-fixtures", action="store_true",
                        help="write the screenshot frame's per-draw state (source-draw-state/v1) "
                             "to draw-state/ for cross-backend comparison")
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
    parser.add_argument("--expect-dark-frame", action="store_true",
                        help="the view is expected to be dark: instead of requiring scene "
                             "detail in the capture, require the engine's WMSH draw attestation")
    parser.add_argument("--capture-wait", type=int, default=600,
                        help="frames between the console commands and the final screenshot and "
                             "quit (a longer cfg sequence, e.g. several screenshots, needs more)")
    parser.add_argument("--headless", action="store_true",
                        help="render offscreen on the GPU (SDL offscreen driver) with the "
                             "volume muted")
    parser.add_argument("--console-command", action="append", default=[],
                        help="console command to run after the map loads and before the "
                             "screenshot (repeatable). Player commands such as setpos and "
                             "setang need the client prefix: 'cmd setpos 0 0 64'")
    parser.add_argument("--map", default="testchmb_a_00")
    parser.add_argument("--physics", default="vphysics",
                        help="physics provider module name (e.g. vphysics, vphysics_box3d)")
    parser.add_argument("--renderer", default=None,
                        help="render provider id (e.g. vulkan-compat, native-vulkan); engine default if unset")
    parser.add_argument("--no-mouse", action="store_true",
                        help="disable mouse input for repeatable windowed camera captures")
    parser.add_argument("--width", type=int, default=1024)
    parser.add_argument("--height", type=int, default=768)
    for name in ("vulkan", "sdl3", "wayland"):
        parser.add_argument("--require-" + name, action="store_true")
    args = parser.parse_args(argv)
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
                "runtime": str(args.runtime.resolve()), "map": args.map,
                "requested_resolution": [args.width, args.height]}
    try:
        stage = output / "runtime"
        evidence["staging"] = stage_runtime(args.runtime, stage)
        evidence["build_overrides"] = install_build(args.build, stage) if args.build else {}
        if args.content_root:
            evidence["content_overrides"] = install_content(args.content_root, stage)
        if args.shader_artifacts:
            evidence["shader_overrides"] = install_shader_artifacts(args.shader_artifacts, stage)
        executable = stage / "hl2_launcher"
        if not executable.is_file():
            raise ValueError("runtime is missing hl2_launcher")
        evidence["executables"] = {str(path.relative_to(stage)): sha256(path)
                                   for path in [executable] + sorted((stage / "bin").glob("*.so"))
                                   + sorted((stage / "portal/bin").glob("*.so"))}
        command = [str(executable), "-game", "portal", "-windowed", "-w", str(args.width), "-h", str(args.height),
                   # -multirun: an isolated boot must not collide with (or be refused
                   # by) a game the user is running.
                   "-multirun",
                   "-novid", "-insecure", "-console", "-condebug", "-dev", "-physics", args.physics,
                   # mat_vsync 0: a real present-mode wait must not stretch the boot.
                   "+sv_cheats", "1", "+mat_queue_mode", "0", "+mat_vsync", "0", "+fps_max", "60",
                   "+map", args.map,
                   "+wait", "180", "+status", "+hideconsole", "+developer", "0"]
        # Extra console commands run once the map has loaded, before the capture
        # (e.g. "setpos X Y Z" / "setang P Y R" to frame the same view on every
        # backend).
        if args.console_command:
            # Written to a cfg and exec'd: the command line splits arguments (and
            # reads a negative number as an option), a cfg keeps each line whole.
            # The local player exists only after spawn, hence the wait.
            (stage / "portal/cfg").mkdir(parents=True, exist_ok=True)
            (stage / "portal/cfg/portal_boot_commands.cfg").write_text(
                "".join(line + "\n" for line in args.console_command))
            command += ["+wait", "300", "+exec", "portal_boot_commands.cfg"]
        command += ["+wait", str(args.capture_wait), "+screenshot", "+mat_spewvertexandpixelshaders",
                   # The exposure the client's auto-exposure settled on for the
                   # screenshot frame (it writes its goal here every frame).
                   "+mat_hdr_tonemapscale",
                   "+wait", "10", "+quit"]
        if args.renderer:
            command[1:1] = ["-renderer", args.renderer]
        if args.no_mouse:
            command[1:1] = ["-nomouse"]
        if args.draw_state_fixtures:
            fixture_dir = output / "draw-state"
            fixture_dir.mkdir()
            command[1:1] = ["-drawstatefixture", str(fixture_dir)]
        if args.resize_stress:
            # Cmd_Exec_f evaluates separate file lines immediately. A single
            # semicolon-delimited line is parsed as one delayed command sequence,
            # so later sizes remain queued behind each `wait`.
            tail = command.index("+wait", command.index("+developer"))
            resize_script = install_resize_script(stage, mode=args.resize_mode)
            command = command[:tail] + ["-resizetelemetry", "+exec", "rfc0001_resize_e2e"]
            evidence["resize_workload"] = {"version": 1, "sizes": RESIZE_WORKLOAD,
                                           **resize_script}
        if args.require_provider_catalog:
            command += ["-moduleloadtelemetry"]
        environment = os.environ.copy()
        environment["LD_LIBRARY_PATH"] = str(stage / "bin") + ":" + environment.get("LD_LIBRARY_PATH", "")
        environment["SteamAppId"] = "400"
        environment["SteamGameId"] = "400"
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
        if args.require_vulkan:
            environment["DXVK_WSI_DRIVER"] = "SDL3"
        if args.headless:
            # SDL3's offscreen driver: real GPU rendering through
            # VK_EXT_headless_surface, no window, compositor or display.
            environment["SDL_VIDEODRIVER"] = environment["SDL_VIDEO_DRIVER"] = "offscreen"
            for variable in ("WAYLAND_DISPLAY", "DISPLAY"):
                environment.pop(variable, None)
        if environment.get("SDL_VIDEODRIVER") == "offscreen":
            # Nobody is watching or listening to a headless run.
            command[command.index("+map"):command.index("+map")] = ["+volume", "0"]
        evidence["command"] = command
        evidence["requirements"] = requirements
        evidence["display_environment"] = {key: environment.get(key) for key in
                                           ("DISPLAY", "WAYLAND_DISPLAY", "SDL_VIDEODRIVER", "GDK_BACKEND")}
        code, timed_out, loaded, seconds = run_product(command, stage, environment,
                                                       args.timeout, output / "stdout.log")
        log_paths = [output / "stdout.log", stage / "engine.log", stage / "portal/console.log"]
        log = "\n".join(path.read_text(errors="replace") for path in log_paths if path.is_file())
        screenshots = [info for path in sorted(stage.rglob("screenshots/*.tga"))
                       if (info := screenshot_info(path))]
        failures = evaluate(log, screenshots, code, timed_out, args.map, requirements, loaded,
                            args.renderer, args.expect_dark_frame)
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
                require_unscaled=args.renderer == "native-vulkan")
            failures.extend(evidence["resize"]["failures"])
        if args.require_provider_catalog:
            evidence["provider_catalog"] = inspect_provider_catalog(log)
            failures.extend(evidence["provider_catalog"]["failures"])
        if args.draw_state_fixtures:
            fixtures = sorted(fixture_dir.glob("*.jsonl"))
            evidence["draw_state_fixtures"] = [{"path": str(path), "sha256": sha256(path),
                                                "bytes": path.stat().st_size} for path in fixtures]
            if not fixtures:
                failures.append("requested draw-state fixtures were not written")
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
