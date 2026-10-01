#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Compare a core-rendered game frame with the same lighting-fixture lab camera.

The area-room/overview profile has a declared pixel gate. Other fixture views
are diagnostics until their scene composition and image limits are reviewed.
Every score requires matching staged content and a game view-oracle camera.
"""

import argparse
import hashlib
import json
import math
from pathlib import Path
import subprocess

import numpy as np
from PIL import Image

import lighting_fixtures as lf
import lighting_gallery as gallery


PROFILE = ("area-room", "overview", (512, 384))
LIMITS = {"mean": 3.0, "p99": 25.0, "fraction_gt8": 0.03}


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def display_bytes(linear):
    if not np.isfinite(linear).all():
        raise ValueError("the lab frame contains nonfinite pixels")
    linear = np.maximum(linear, 0.0)
    srgb = np.where(linear <= 0.0031308, 12.92 * linear,
                    1.055 * np.power(linear, 1.0 / 2.4) - 0.055)
    return np.rint(np.clip(srgb, 0.0, 1.0) * 255.0).astype(np.uint8)


def output_scale(evidence):
    """The game's linear surface scale recorded beside its screenshot."""
    value = evidence.get("tonemap_scale")
    if (isinstance(value, bool) or not isinstance(value, (int, float)) or
            not math.isfinite(value) or value < 0.0):
        raise ValueError("the product has no valid tone-map scale")
    return float(value)


def camera_from_oracle(path):
    lines = Path(path).read_text().splitlines()
    if not lines or json.loads(lines[0]).get("schema") != "source-view-oracle/v1":
        raise ValueError("missing view-oracle header")
    for line in lines[1:]:
        event = json.loads(line)
        label = event.get("name", "")
        if event.get("event") != "label_begin" or not label.startswith("vieworacle "):
            continue
        view = json.loads(label[len("vieworacle "):])
        if view.get("type") == "3d" and view.get("stack") == 1 and \
                view.get("target") == "backbuffer":
            return view
    raise ValueError("the screenshot has no top-level world camera")


def camera_errors(fixture, camera, view):
    authored = fixture["cameras"][camera]
    eye = [float(value) * lf.SOURCE_UNITS_PER_METER for value in authored["eye"]]
    forward = authored["forward"]
    pitch = math.degrees(math.atan2(-forward[2], math.hypot(forward[0], forward[1])))
    yaw = math.degrees(math.atan2(forward[1], forward[0]))
    expected_angles = [pitch, yaw, 0.0]
    if any(abs(value - target) > 0.001 for value, target in
           zip(authored["up"], (0.0, 0.0, 1.0))):
        raise ValueError("this profile needs a zero-roll fixture camera")
    position = max(abs(float(actual) - target) for actual, target in zip(view["origin"], eye))
    angle = max(abs((float(actual) - target + 180.0) % 360.0 - 180.0)
                for actual, target in zip(view["angles"], expected_angles))
    fov = abs(float(view["fov"]) - float(fixture["horizontal_fov_degrees"]))
    return {"position_units": position, "angle_degrees": angle, "fov_degrees": fov}


def score(lab, game):
    if lab.shape != game.shape or lab.ndim != 3 or lab.shape[2] != 3:
        raise ValueError("the lab and game frames have different RGB dimensions")
    delta = np.abs(lab.astype(np.int16) - game.astype(np.int16))
    return {"mean": float(delta.mean()), "p99": float(np.percentile(delta, 99)),
            "fraction_gt8": float(np.any(delta > 8, axis=2).mean())}


def content_mismatches(content_root, staged):
    expected = {str(path.relative_to(content_root)) for path in content_root.rglob("*")
                if path.is_file() and path.relative_to(content_root).parts[0] in
                ("maps", "materials", "models")}
    missing = expected - set(staged)
    changed = {relative for relative, entry in staged.items()
               if not (content_root / relative).is_file() or
               sha256(content_root / relative) != entry["sha256"]}
    return sorted(missing | changed)


def render_lab(lab_binary, content_root, fixture, camera, out, state=None):
    authored = fixture["cameras"][camera]
    bsp = content_root / "maps" / (lf.map_for(fixture, state)["name"] + ".bsp")
    film = fixture["film"]
    command = [str(lab_binary), "--game", str(content_root), "--map", str(bsp),
               "--eye", ",".join("%.4f" % (value * lf.SOURCE_UNITS_PER_METER)
                                 for value in authored["eye"]),
               "--forward", ",".join("%.6f" % value for value in authored["forward"]),
               "--up", ",".join("%.6f" % value for value in authored["up"]),
               "--hfov", str(fixture["horizontal_fov_degrees"]),
               "--size", "%dx%d" % (film["width"], film["height"]),
               "--core-direct", "--out", str(out)]
    model = gallery.probe_model(fixture, state)
    if model:
        command += ["--model", model[0], "--model-origin",
                    ",".join("%.4f" % value for value in model[1]), "--model-no-shadow"]
    for mover in fixture["lighting"].get("movers", {}).get(state or "", []):
        bounds = list(mover["min"]) + list(mover["max"])
        command += ["--mover", ",".join("%.4f" % value for value in bounds) +
                    "," + mover["material"]]
    fog_scale = gallery.fog_scale(fixture, state)
    if fog_scale is not None:
        command += ["--fog-scale", "%g" % fog_scale]
    result = subprocess.run(command, env=gallery.lab_environment(lab_binary),
                            capture_output=True, text=True, timeout=600)
    if result.returncode or not out.is_file():
        raise ValueError("render_lab did not draw the fixture camera: " +
                         (result.stdout + result.stderr).strip()[-500:])
    return command, (result.stdout + result.stderr).strip().splitlines()[-1]


def compare(fixture_name, camera, lab_path, content_root, evidence_path, state=None):
    fixture_path = lf.ROOT / "quality/fixtures/lighting" / fixture_name / "fixture.json"
    fixture = json.loads(fixture_path.read_text())
    evidence = json.loads(Path(evidence_path).read_text())
    if evidence.get("status") != "pass" or evidence.get("failures"):
        raise ValueError("the product boot did not pass")
    if evidence.get("map") != lf.map_for(fixture, state)["name"]:
        raise ValueError("the game and lab name different maps")
    content_root = Path(content_root).resolve()
    bsp = content_root / "maps" / (evidence["map"] + ".bsp")
    staged_bsp = evidence.get("content_overrides", {}).get("maps/" + evidence["map"] + ".bsp")
    if not staged_bsp or staged_bsp.get("sha256") != sha256(bsp):
        raise ValueError("the game's staged BSP differs from the lab content snapshot")
    mismatches = content_mismatches(content_root, evidence["content_overrides"])
    resolution = [fixture["film"]["width"], fixture["film"]["height"]]
    if evidence.get("requested_resolution") != resolution:
        raise ValueError("the game capture is not at the profile resolution")
    scale = output_scale(evidence)
    command = evidence.get("command", [])
    if "-renderer" not in command or command[-1] == "-renderer" or \
            command[command.index("-renderer") + 1] != "native-vulkan":
        raise ValueError("the product capture is not native Vulkan")
    screenshots = evidence.get("screenshots", [])
    oracles = evidence.get("view_oracle_captures", [])
    if len(screenshots) != 1 or len(oracles) != 1:
        raise ValueError("the product needs one screenshot and one view oracle")
    screenshot, oracle = Path(screenshots[0]["path"]), Path(oracles[0]["path"])
    if sha256(screenshot) != screenshots[0]["sha256"] or sha256(oracle) != oracles[0]["sha256"]:
        raise ValueError("a product capture changed after its boot")
    view = camera_from_oracle(oracle)
    camera_error = camera_errors(fixture, camera, view)
    if camera_error["position_units"] > 0.25 or camera_error["angle_degrees"] > 0.25 or \
            camera_error["fov_degrees"] > 0.1:
        raise ValueError("the product camera differs from the lab fixture: " + str(camera_error))
    lab_path = Path(lab_path)
    lab = display_bytes(lf.read_pfm(lab_path) * scale)
    game = np.asarray(Image.open(screenshot).convert("RGB"), dtype=np.uint8)
    metrics = score(lab, game)
    screened = all(metrics[key] <= limit for key, limit in LIMITS.items())
    declared = (fixture_name, camera, tuple(resolution)) == PROFILE and state in (None, "default")
    if mismatches:
        status = "fail"
    elif declared:
        status = "pass" if screened else "fail"
    else:
        status = "diagnostic"
    return {"schema": "render-game-lab-comparison/v2", "status": status,
            "fixture": fixture_name, "state": state, "camera": camera,
            "resolution": resolution, "output_scale": scale, "declared_gate": declared,
            "lab": {"path": str(lab_path.resolve()), "sha256": sha256(lab_path),
                    "content_root": str(content_root), "bsp_sha256": sha256(bsp)},
            "product": {"evidence": str(Path(evidence_path).resolve()),
                        "screenshot_sha256": screenshots[0]["sha256"],
                        "view_oracle_sha256": oracles[0]["sha256"],
                        "source": evidence.get("source")},
            "camera_error": camera_error, "content_mismatches": mismatches,
            "metrics": metrics, "screened_by_first_profile_limits": screened,
            "first_profile_limits": LIMITS}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--fixture", default="area-room")
    parser.add_argument("--camera", default="overview")
    parser.add_argument("--state", help="fixture state; defaults to its baked state")
    parser.add_argument("--lab-bin", type=Path, required=True,
                        help="render_lab executable; this command renders the fixture camera")
    parser.add_argument("--lab-content-root", type=Path, required=True,
                        help="frozen map/material snapshot that the game staged")
    parser.add_argument("--boot-evidence", type=Path, required=True)
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()
    try:
        fixture = lf.load_fixture(args.fixture)
        lab_binary = args.lab_bin.resolve()
        content_root = args.lab_content_root.resolve()
        lab_frame = args.out.with_suffix(".pfm").resolve()
        lab_frame.parent.mkdir(parents=True, exist_ok=True)
        state = args.state or fixture["lighting"]["map"]["baked_state"]
        command, message = render_lab(
            lab_binary, content_root, fixture, args.camera, lab_frame, state)
        result = compare(args.fixture, args.camera, lab_frame, content_root,
                         args.boot_evidence, state)
        result["lab"]["binary"] = str(lab_binary)
        result["lab"]["binary_sha256"] = sha256(lab_binary)
        result["lab"]["core_direct"] = True
        result["lab"]["command"] = command
        result["lab"]["message"] = message
    except (OSError, KeyError, TypeError, ValueError, subprocess.TimeoutExpired) as error:
        result = {"schema": "render-game-lab-comparison/v2", "status": "error",
                  "reason": str(error)}
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n")
    print("game/lab comparison:", result["status"], result.get("metrics", result.get("reason")))
    return 0 if result["status"] in ("pass", "diagnostic") else 1


if __name__ == "__main__":
    raise SystemExit(main())
