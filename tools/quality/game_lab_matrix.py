#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Capture baked-state lighting fixtures in the game and compare their lab views.

Each map is copied once into a private snapshot. The product stages that exact
snapshot, and render_lab reads it. Every selected camera needs a declared,
passing image gate by default; --diagnostic collects exploratory scores.
Portal scenes are opt-in because their live
portal/player view composition is not yet represented by the lab camera.
The product's Vulkan frame stats count actual legacy-stream draws over the
last 30 presented frames; --require-zero-legacy turns that count into a gate.
"""

import argparse
import json
import math
from pathlib import Path
import shutil
import statistics
import subprocess
import sys

import game_lab_compare as comparison
import lighting_fixtures as lf

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "kiln"))
import sepipe_loader  # noqa: E402


PORTAL_SCENES = {"portal-chamber", "portal2-chamber"}
LEGACY_SAMPLE_FRAMES = 30


def legacy_census(path):
    """Count draws actually issued from the backend stream in settled frames."""
    lines = Path(path).read_text().splitlines()
    if not lines or json.loads(lines[0]).get("schema") != "vulkan-frame-stats/v1":
        raise ValueError("legacy census has no Vulkan frame-stats header: " + str(path))
    frames = [json.loads(line) for line in lines[1:]]
    if len(frames) < LEGACY_SAMPLE_FRAMES:
        raise ValueError("legacy census has only %d frames: %s" % (len(frames), path))
    settled = frames[-LEGACY_SAMPLE_FRAMES:]
    for index, frame in enumerate(settled):
        if (type(frame.get("f")) is not int or
                (index and frame["f"] != settled[index - 1]["f"] + 1)):
            raise ValueError("legacy census has missing or unordered frames: " + str(path))
        if (type(frame.get("legacy_stream_draws")) is not int or
                type(frame.get("legacy_program_draws")) is not int or
                frame["legacy_stream_draws"] < 0 or frame["legacy_program_draws"] < 0 or
                frame["legacy_program_draws"] > frame["legacy_stream_draws"]):
            raise ValueError("legacy census has invalid draw counts at frame %s" %
                             frame.get("f", "?"))
    draws = [frame["legacy_stream_draws"] for frame in settled]
    programs = [frame["legacy_program_draws"] for frame in settled]
    return {"frames": len(settled), "first_frame": settled[0]["f"],
            "last_frame": settled[-1]["f"],
            "stream_draws_median": statistics.median(draws),
            "stream_draws_max": max(draws),
            "legacy_program_draws_median": statistics.median(programs),
            "legacy_program_draws_max": max(programs),
            "zero_legacy_stream": max(draws) == 0}


def camera_commands(fixture, name):
    camera = fixture["cameras"][name]
    eye = [value * lf.SOURCE_UNITS_PER_METER for value in camera["eye"]]
    forward = camera["forward"]
    pitch = math.degrees(math.atan2(-forward[2], math.hypot(forward[0], forward[1])))
    yaw = math.degrees(math.atan2(forward[1], forward[0]))
    place = ["cmd setpos %.3f %.3f %.3f" % (eye[0], eye[1], eye[2] - 64.0),
             "cmd setang %.4f %.4f 0" % (pitch, yaw)]
    commands = (["r_drawvgui 0", "r_worldmesh_draw 2",
                 "mat_hdr_tonemapscale 1", "cmd noclip",
                 "cmd fov %d" % fixture["horizontal_fov_degrees"], "wait 60"] +
                place + ["wait 120"] + place + ["wait 60"])
    if fixture["name"] == "portal-chamber":
        # The map's scripted point_viewcontrol keeps the game camera at its
        # entry sequence despite setpos/setang. Release it for the authored
        # fixture camera before placing the player.
        commands.insert(5, "ent_fire point_viewcontrol Disable")
        commands.insert(6, "wait 30")
    if fixture["name"] == "portal-pair" and \
            fixture["lighting"]["map"]["baked_state"] == "closed":
        # The placed pair activates at spawn although its baked world is closed.
        # Remove both live portals so their idle ring does not cover the plug.
        commands += ["ent_fire PortalA Kill", "ent_fire PortalB Kill", "wait 60"]
    commands.append("r_core_world_stats")
    return "; ".join(commands)


def snapshot_content(map_name, destination):
    source = lf.ROOT / "run/maps" / map_name
    if destination.exists():
        raise ValueError("snapshot already exists: " + str(destination))
    if not (source / "maps" / (map_name + ".bsp")).is_file():
        raise ValueError("published BSP missing for " + map_name)
    copied = []
    for group in ("maps", "materials", "models"):
        for path in (source / group).rglob("*"):
            if path.is_file():
                relative = path.relative_to(source)
                target = destination / relative
                target.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(path, target)
                copied.append(str(relative))
    return sorted(copied)


def capture(args, fixture_name):
    fixture = lf.load_fixture(fixture_name)
    state = fixture["lighting"]["map"]["baked_state"]
    map_name = lf.map_for(fixture, state)["name"]
    root = args.out / fixture_name
    content = root / "content"
    files = snapshot_content(map_name, content)
    records = {}
    for camera in sorted(fixture["cameras"]):
        out = root / camera
        out.mkdir(parents=True)
        film = fixture["film"]
        boot = out / "boot"
        stats = boot / "runtime/portal2/legacy-stream.jsonl"
        command = [sys.executable, str(lf.ROOT / "tools/quality/portal_boot.py"),
                   *sepipe_loader.boot_arguments(args), "--content-root", str(content),
                   "--renderer", "core", "--headless", "--view-oracle",
                   "--startup-command", "r_core_world 1",
                   "--engine-arg=-vkframestats",
                   "--engine-arg=portal2/legacy-stream.jsonl",
                   "--timeout", str(args.boot_timeout),
                   "--map", map_name, "--width", str(film["width"]),
                   "--height", str(film["height"]), "--capture-wait", "420",
                   "--console-command", camera_commands(fixture, camera),
                   "--out", str(boot)]
        try:
            with (out / "boot.log").open("w") as log:
                boot_result = subprocess.run(command, cwd=lf.ROOT, stdout=log,
                                             stderr=subprocess.STDOUT,
                                             timeout=args.process_timeout)
            boot_returncode = boot_result.returncode
        except subprocess.TimeoutExpired:
            boot_returncode = None
        record = {"boot_command": command, "boot_returncode": boot_returncode,
                  "boot_evidence": str(boot / "evidence.json")}
        if boot_returncode is None:
            record["error"] = "product boot process exceeded %d seconds" % args.process_timeout
        if boot_returncode == 0 and (boot / "evidence.json").is_file():
            lab = out / "lab.pfm"
            try:
                record["legacy_census"] = legacy_census(stats)
                lab_command, message = comparison.render_lab(
                    args.lab, content, fixture, camera, lab, state)
                result = comparison.compare(fixture_name, camera, lab, content,
                                            boot / "evidence.json", state)
                result["lab"]["command"] = lab_command
                result["lab"]["message"] = message
                (out / "comparison.json").write_text(
                    json.dumps(result, indent=2, sort_keys=True) + "\n")
                record["comparison"] = result
            except (OSError, KeyError, TypeError, ValueError, subprocess.TimeoutExpired) as error:
                record["error"] = str(error)
        records[camera] = record
        outcome = record.get("comparison", {}).get("metrics", record.get("error", "boot failed"))
        print("%s/%s: %s" % (fixture_name, camera, outcome), flush=True)
    return {"map": map_name, "state": state, "snapshot": str(content),
            "files": files, "scene_comparable": fixture_name not in PORTAL_SCENES,
            "scene_note": ("live portal and player composition is outside the lab camera"
                           if fixture_name in PORTAL_SCENES else "baked state staged in both"),
            "cameras": records}


def matrix_failures(record, require_zero_legacy=False, require_image_parity=False):
    failures = []
    if not record.get("fixtures"):
        failures.append("no fixtures ran")
    for fixture, entry in record.get("fixtures", {}).items():
        if not entry.get("cameras"):
            failures.append(fixture + ": no cameras ran")
        for camera, result in entry.get("cameras", {}).items():
            label = fixture + "/" + camera
            if result.get("boot_returncode") != 0:
                failures.append(label + ": product boot failed")
            elif result.get("error") or not result.get("comparison"):
                failures.append(label + ": comparison did not complete")
            elif result["comparison"]["status"] not in ("pass", "diagnostic"):
                failures.append(label + ": comparison failed")
            elif require_image_parity and result["comparison"]["status"] != "pass":
                failures.append(label + ": image parity is diagnostic, not a pass")
            elif require_zero_legacy and not result.get("legacy_census", {}).get(
                    "zero_legacy_stream"):
                failures.append(label + ": legacy stream still drew")
    return failures


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    sepipe_loader.add_arguments(parser, "portal2")
    parser.add_argument("--lab", type=Path, required=True)
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--fixture", action="append",
                        help="select a fixture; by default run every scene without live portals")
    parser.add_argument("--include-portal-scenes", action="store_true")
    parser.add_argument("--require-zero-legacy", action="store_true",
                        help="fail if any settled captured frame issued a legacy-stream draw")
    parser.add_argument("--require-image-parity", action="store_true",
                        help="explicitly select the default image gate: every camera must pass")
    parser.add_argument("--diagnostic", action="store_true",
                        help="collect ungated image scores; exit status does not certify parity")
    parser.add_argument("--boot-timeout", type=int, default=180,
                        help="seconds allowed for the game process after staging")
    parser.add_argument("--process-timeout", type=int, default=240,
                        help="seconds allowed for the complete staged boot")
    args = parser.parse_args()
    args.lab = args.lab.resolve()
    args.out = args.out.resolve()
    if args.diagnostic and args.require_image_parity:
        parser.error("--diagnostic and --require-image-parity are mutually exclusive")
    if args.boot_timeout <= 0 or args.process_timeout <= args.boot_timeout:
        parser.error("process timeout must be greater than positive boot timeout")
    if args.out.exists():
        parser.error("output directory already exists")
    names = args.fixture or sorted(path.parent.name for path in
                                   (lf.ROOT / "quality/fixtures/lighting").glob("*/fixture.json")
                                   if args.include_portal_scenes or path.parent.name not in PORTAL_SCENES)
    args.out.mkdir(parents=True)
    record = {"schema": "render-game-lab-matrix/v1",
              "client": sepipe_loader.boot_arguments(args), "lab": str(args.lab),
              "lab_sha256": comparison.sha256(args.lab),
              "image_gate": "diagnostic" if args.diagnostic else "required", "fixtures": {}}
    try:
        for name in names:
            record["fixtures"][name] = capture(args, name)
            (args.out / "matrix.json").write_text(json.dumps(record, indent=2,
                                                             sort_keys=True) + "\n")
    except (OSError, KeyError, TypeError, ValueError, subprocess.TimeoutExpired) as error:
        record["error"] = str(error)
    cameras = [camera for fixture in record["fixtures"].values()
               for camera in fixture.get("cameras", {}).values()]
    record["zero_legacy_stream"] = bool(cameras) and all(
        camera.get("legacy_census", {}).get("zero_legacy_stream") for camera in cameras)
    record["failures"] = matrix_failures(record, args.require_zero_legacy,
                                          not args.diagnostic)
    (args.out / "matrix.json").write_text(json.dumps(record, indent=2, sort_keys=True) + "\n")
    return 1 if record.get("error") or record["failures"] else 0


if __name__ == "__main__":
    raise SystemExit(main())
