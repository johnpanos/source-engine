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


IMMUTABLE_ASSETS = {
    ".vpk", ".bsp", ".vtf", ".vmt", ".mdl", ".vvd", ".vtx", ".phy",
    ".wav", ".mp3", ".ogg", ".webm", ".bik",
}
EXCLUDED_DIRECTORIES = {"screenshots", "save", "logs", "dumps"}


def sha256(path):
    digest = hashlib.sha256()
    with Path(path).open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def stage_runtime(runtime, stage):
    """Only immutable asset files are shared; every writable directory is private."""
    runtime, stage = Path(runtime).resolve(), Path(stage).resolve()
    if not runtime.is_dir() or not (runtime / "portal/gameinfo.txt").is_file():
        raise ValueError("runtime must contain portal/gameinfo.txt")
    if stage == runtime or runtime in stage.parents:
        raise ValueError("staging directory must be outside the original runtime")
    stage.mkdir(parents=True, exist_ok=False)
    count = {"copied": 0, "shared_assets": 0}
    for directory, directories, filenames in os.walk(runtime, followlinks=False):
        relative = Path(directory).relative_to(runtime)
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


def install_build(build, stage):
    """Overlay Waf products, keeping Portal client/server modules in gamebin."""
    build, stage = Path(build).resolve(), Path(stage)
    if not build.is_dir():
        raise ValueError("build output directory is missing")
    products = sorted(path for path in build.rglob("*.so") if path.is_file())
    launchers = sorted(path for path in build.rglob("hl2_launcher") if path.is_file())
    if not products or len(launchers) != 1:
        raise ValueError("build must contain shared libraries and exactly one hl2_launcher")
    installed = {}
    # Waf gives single-game products an unqualified game/client output path.
    # Read only its literal game selection, never execute the Python cache.
    selected_games = set()
    native_library_paths = []
    for cache in (build / "c4che").rglob("*_cache.py"):
        for line in cache.read_text().splitlines():
            if line.startswith("GAMES = "):
                selected_games.add(ast.literal_eval(line.partition(" = ")[2]))
            elif line.startswith("LIBPATH_DXVK = "):
                native_library_paths += ast.literal_eval(line.partition(" = ")[2])
    for directory in sorted(set(native_library_paths)):
        for library in sorted(Path(directory).glob("libdxvk_d3d9.so*")):
            destination = stage / "bin" / library.name
            destination.parent.mkdir(parents=True, exist_ok=True)
            destination.unlink(missing_ok=True)
            shutil.copy2(library, destination)
            installed[str(destination.relative_to(stage))] = {
                "source": str(library), "sha256": sha256(destination)}
    for source in products + launchers:
        relative = source.relative_to(build)
        if source.name == "hl2_launcher":
            destination = stage / source.name
        elif source.name in {"client.so", "libclient.so", "server.so", "libserver.so"}:
            if "portal" not in relative.parts and not (
                    relative.parts[:2] in (("game", "client"), ("game", "server"))
                    and len(relative.parts) == 3 and selected_games == {"portal"}):
                continue
            destination = stage / "portal/bin" / source.name
        else:
            destination = stage / "bin" / source.name
        key = str(destination.relative_to(stage))
        if key in installed:
            raise ValueError("ambiguous build output for " + key)
        destination.parent.mkdir(parents=True, exist_ok=True)
        destination.unlink(missing_ok=True)
        shutil.copy2(source, destination)
        installed[key] = {"source": str(source), "sha256": sha256(destination)}
    return installed


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
    visible = sum(10 < (pixels[i] + pixels[i + 1] + pixels[i + 2]) / 3 < 245
                  for i in range(0, len(pixels), stride))
    detail_fraction = visible / (width * height)
    return {"path": str(path), "width": width, "height": height,
            "midtone_fraction": detail_fraction, "has_scene_detail": detail_fraction >= 0.05,
            "bytes": path.stat().st_size, "sha256": sha256(path)}


def evaluate(log, screenshots, returncode, timed_out, map_name, requirements, loaded):
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
        failures.append("engine capture is blank or almost entirely black/white")
    markers = {
        "vulkan": r"RFC0001 renderer: provider=vulkan-compat\b",
        "sdl3": r"RFC0001 window: provider=sdl3\b",
        "wayland": r"RFC0001 window: provider=sdl3 driver=wayland\b",
    }
    for requirement in requirements:
        if not re.search(markers[requirement], log):
            failures.append("actual %s provider was not attested by the running engine" % requirement)
    for requirement, library in (("vulkan", "libvulkan"), ("sdl3", "libSDL3")):
        if requirement in requirements and not any(library in path for path in loaded):
            failures.append("running process did not map " + library)
    return failures


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
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--timeout", type=float, default=180)
    parser.add_argument("--map", default="testchmb_a_00")
    for name in ("vulkan", "sdl3", "wayland"):
        parser.add_argument("--require-" + name, action="store_true")
    args = parser.parse_args(argv)
    if args.timeout <= 0 or not re.fullmatch(r"[a-zA-Z0-9_]+", args.map):
        parser.error("timeout must be positive and map must be a simple map name")
    output = args.out.resolve()
    output.mkdir(parents=True, exist_ok=True)
    if (output / "evidence.json").exists():
        parser.error("evidence already exists; use a new output directory")
    evidence = {"schema": "portal-boot-evidence/v1", "status": "fail",
                "started_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
                "source": conformance.source_identity(conformance.repo_root()),
                "runtime": str(args.runtime.resolve()), "map": args.map}
    try:
        stage = output / "runtime"
        evidence["staging"] = stage_runtime(args.runtime, stage)
        evidence["build_overrides"] = install_build(args.build, stage) if args.build else {}
        executable = stage / "hl2_launcher"
        if not executable.is_file():
            raise ValueError("runtime is missing hl2_launcher")
        evidence["executables"] = {str(path.relative_to(stage)): sha256(path)
                                   for path in [executable] + sorted((stage / "bin").glob("*.so"))
                                   + sorted((stage / "portal/bin").glob("*.so"))}
        command = [str(executable), "-game", "portal", "-windowed", "-w", "1024", "-h", "768",
                   "-novid", "-insecure", "-console", "-condebug", "-dev",
                   "+sv_cheats", "1", "+mat_queue_mode", "0", "+fps_max", "60", "+map", args.map,
                   "+wait", "180", "+status", "+hideconsole", "+developer", "0",
                   "+wait", "600", "+screenshot", "+mat_spewvertexandpixelshaders",
                   "+wait", "10", "+quit"]
        environment = os.environ.copy()
        environment["LD_LIBRARY_PATH"] = str(stage / "bin") + ":" + environment.get("LD_LIBRARY_PATH", "")
        environment["SteamAppId"] = "400"
        environment["SteamGameId"] = "400"
        requirements = [name for name in ("vulkan", "sdl3", "wayland") if getattr(args, "require_" + name)]
        if args.require_wayland:
            environment["SDL_VIDEO_DRIVER"] = "wayland"
            environment["SDL_VIDEODRIVER"] = "wayland"
        if args.require_vulkan:
            environment["DXVK_WSI_DRIVER"] = "SDL3"
        evidence["command"] = command
        evidence["requirements"] = requirements
        evidence["display_environment"] = {key: environment.get(key) for key in
                                           ("DISPLAY", "WAYLAND_DISPLAY", "SDL_VIDEODRIVER")}
        code, timed_out, loaded, seconds = run_product(command, stage, environment,
                                                       args.timeout, output / "stdout.log")
        log_paths = [output / "stdout.log", stage / "engine.log", stage / "portal/console.log"]
        log = "\n".join(path.read_text(errors="replace") for path in log_paths if path.is_file())
        screenshots = [info for path in sorted(stage.rglob("screenshots/*.tga"))
                       if (info := screenshot_info(path))]
        failures = evaluate(log, screenshots, code, timed_out, args.map, requirements, loaded)
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
