#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Capture physical door occlusion of an idle core fizzler on native Vulkan.

The generated room reuses the fizzler fixture's constant, known emission and
adds a retail test-chamber door through the existing read-only content resolver.
Paired emission-off/on captures freeze each pose; no animation or exposure
change participates in a receiver comparison. Disabling mover shadows with the
door closed is the live negative control. Retail asset bytes are never saved
in the source tree.
"""

import argparse
import json
import re
from pathlib import Path
import shutil
import subprocess
import sys

from conformance_result import Checks
from fizzler_light import generate, ROOT
import vmf_map_build


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--runtime", type=Path, default=ROOT / "run/runtime-p2")
    parser.add_argument("--build", type=Path, default=ROOT / "build-p2")
    args = parser.parse_args()
    out, runtime = args.out.resolve(), args.runtime.resolve()
    out.mkdir(parents=True, exist_ok=True)
    source, content, overlay = generate(out, runtime, door=True)
    name = source.stem
    tools = Path(json.loads(vmf_map_build.TOOLCHAIN.read_text())["compile_tools"])
    record = vmf_map_build.build(source, out / "map-build", tools, overlay)
    checks = Checks()
    checks.check(record["status"] == "pass", "door.compile", str(record))
    if record["status"] != "pass":
        return checks.report()
    (content / "maps").mkdir(exist_ok=True)
    shutil.copy2(out / "map-build/compile" / (name + ".bsp"), content / "maps" / (name + ".bsp"))
    commands = ["noclip", "cmd setpos -210 0 6", "cmd setang 18 0 0", "fov 90",
                "mat_force_tonemap_scale 1", "fps_max 60", "host_framerate 0.015", "r_drawviewmodel 0",
                "cl_portal_cleanser_scanline 0", "cl_portal_cleanser_default_intensity 1",
                "sv_pausable 1", "r_core_shadow_movers 1", "r_core_shadow_quality 3", "wait 180"]
    states = ["closed", "early", "partial", "open", "reclosed", "no-blockers"]
    transitions = [[], ["unpause", "ent_fire door Open", "wait 24"],
                   ["unpause", "wait 75"], ["unpause", "wait 200"],
                   ["unpause", "ent_fire door Close", "wait 200"],
                   ["r_core_shadow_movers 0"]]
    for state, transition in zip(states, transitions):
        commands += transition + ["setpause", "wait 30", "echo door_state " + state,
                                  "cl_fizzler_core_emission 0", "r_dynamic_occlusion_report 1", "wait 20", "screenshot",
                                  "cl_fizzler_core_emission 1", "r_dynamic_occlusion_report 1",
                                  "cl_fizzler_core_emission_report 1", "wait 20", "screenshot",
                                  "wait 10"]
    command = [sys.executable, str(ROOT / "tools/quality/portal_boot.py"), "--runtime", str(runtime),
               "--build", str(args.build.resolve()), "--game", "portal2", "--map", name,
               "--content-root", str(content), "--renderer", "native-vulkan", "--require-vulkan",
               "--require-sdl3", "--headless", "--width", "640", "--height", "480",
               "--out", str(out / "capture"), "--capture-wait", "900", "--timeout", "180",
               "--startup-command", "r_core_world 1"]
    for item in commands:
        command += ["--console-command", item]
    with (out / "capture.log").open("w") as log:
        done = subprocess.run(command, stdout=log, stderr=subprocess.STDOUT)
    checks.equal(done.returncode, 0, "door.boot")
    shots = sorted((out / "capture/runtime/portal2/screenshots").glob("*.tga"))
    checks.equal(len(shots), len(states) * 2 + 1, "door.all-poses-captured")
    metrics = {"schema": "fizzler-door-light/v1", "states": [],
               "receiver_region": [120, 320, 520, 440], "closed_blue_max": 0.5,
               "open_blue_min": 2.0, "capture_evidence": "capture/evidence.json"}
    if len(shots) == len(states) * 2 + 1:
        import numpy as np
        from PIL import Image
        images = [np.asarray(Image.open(path)).astype(float) for path in shots]
        for path, image in zip(shots, images):
            Image.fromarray(image.astype("uint8")).save(path.with_suffix(".png"))
        rises = []
        for index, state in enumerate(states):
            difference = images[2 * index + 1] - images[2 * index]
            receiver = difference[320:440, 120:520, :3]
            rise = float(receiver[:, :, 2].mean())
            rises.append(rise)
            metrics["states"].append({"pose": state, "mean_rgb_rise": receiver.mean(axis=(0, 1)).tolist(),
                                      "blue_lit_fraction": float((receiver[:, :, 2] > 4).mean())})
        checks.check(rises[0] < 0.5 and rises[4] < 0.5, "door.closed-blocks-light", str(rises))
        checks.check(rises[3] > 2.0, "door.open-passes-light", str(rises))
        checks.check(0 <= rises[1] < rises[2] < rises[3], "door.aperture-grows-with-pose", str(rises))
        checks.check(rises[5] > 2.0, "door.removed-blockers-control-rejected", str(rises))
        closed = images[1][320:440, 120:520, :3]
        reclosed = images[9][320:440, 120:520, :3]
        error = float(np.abs(closed - reclosed).mean())
        metrics["closed_return_error"] = error
        checks.check(error < 0.5, "door.closing-restores-shadow", str(error))
    log = (out / "capture/runtime/portal2/console.log").read_text(errors="replace")
    pose_reports = {}
    parts = re.split(r"door_state (\S+) *\n", log)
    for state, block in zip(parts[1::2], parts[2::2]):
        reports = []
        for entity, part, revision, count, bounds in re.findall(
                r"physical (-?\d+):(\d+) revision (\d+) vertices (\d+) bounds ([^\n]+)", block):
            if part == "0":
                reports.append([])
            if reports:
                reports[-1].append({"entity": int(entity), "part": int(part), "revision": int(revision),
                                    "vertices": int(count),
                                    "bounds": [float(value) for value in bounds.replace("/", "").split()]})
        pose_reports[state] = reports
    complete = all(len(pose_reports.get(state, [])) == 2 and
                   all(len(report) == 3 for report in pose_reports[state]) for state in states)
    checks.check(complete and all(pose_reports[state][0] == pose_reports[state][1] for state in states),
                 "door.fixed-pose-emission-pairs")
    gaps = []
    if complete:
        for state in states:
            leaves = {part["part"]: part["bounds"] for part in pose_reports[state][0]}
            gaps.append(max(0.0, leaves[2][1] - leaves[1][4]))
    checks.check(len(gaps) == len(states) and 0.01 < gaps[2] < gaps[3] - 0.01,
                 "door.real-partial-aperture", str(gaps))
    metrics["physical_poses"] = pose_reports
    metrics["aperture_widths"] = gaps
    checks.check(log.count("core physical occluders: 3 part(s)") >= 12,
                 "door.physical-panel-publication")
    checks.check("intensity 1.000 powerup 1.000 strength 16.000 shot 0.000" in log,
                 "door.idle-emitter-without-shot")
    checks.check("Core physical shadows:" not in log, "door.no-publication-failures")
    metrics.update(checks=checks.checks, failures=checks.failures,
                   status="pass" if checks.failures == 0 else "fail")
    (out / "receivers.json").write_text(json.dumps(metrics, indent=2) + "\n")
    return checks.report()


if __name__ == "__main__":
    sys.exit(main())
