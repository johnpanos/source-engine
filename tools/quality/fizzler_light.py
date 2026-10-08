#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Build and capture a controlled core fizzler scene; no retail texture bytes.

Uses the installed shared VMF compiler and Portal boot runner. A constant flow
image has a known radiance, so camera motion/flow time cannot change its mean.
The off/on/off/on sequence removes only the core emission; the field remains
visible. The disabled/enabled sequence exercises the entity state. The room
and central blocker are ordinary core world materials.

Use a fresh output directory for each run.

  python3 tools/quality/fizzler_light.py --out quality-results/fizzler-light-fixture
"""

import argparse
import json
from pathlib import Path
import shutil
import subprocess
import sys

from gyro_lab_map import Vmf
from hammer_ui_test import vtf_rgba8888
import vmf_map_build
from conformance_result import Checks

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "kiln"))
import sepipe_loader  # noqa: E402

ROOT = Path(__file__).resolve().parents[2]
NAME = "core_fizzler_light"


def generate(out, runtime, door=False):
    content = out / "content"
    materials = content / "materials/quality/fizzler"
    materials.mkdir(parents=True, exist_ok=True)
    for name, rgb in {"white": (188, 188, 188), "base": (0, 128, 0),
                      "flow": (128, 128, 0), "noise": (0, 0, 0),
                      "bounds": (255, 0, 255)}.items():
        (materials / (name + ".vtf")).write_bytes(vtf_rgba8888(16, 16, rgb))
    (materials / "white.vmt").write_text('"LightmappedGeneric" { "$basetexture" "quality/fizzler/white" }\n')
    (materials / "field.vmt").write_text('''"SolidEnergy"
{
 "$basetexture" "quality/fizzler/base"
 "$flowmap" "quality/fizzler/flow"
 "$flow_noise_texture" "quality/fizzler/noise"
 "$flowbounds" "quality/fizzler/bounds"
 "$flow_worlduvscale" "0.01"
 "$flow_normaluvscale" "0.01"
 "$flow_noise_scale" "0.01"
 "$flow_timeintervalinseconds" "1"
 "$flow_uvscrolldistance" "0.2"
 "$flow_lerpexp" "1"
 "$flow_color" "[0.08 0.25 0.8]"
 "$flow_vortex_color" "[0.08 0.25 0.8]"
 "$flow_vortex_size" "40"
 "$flow_color_intensity" "1"
 "$powerup" "1"
 "$translucent" "1"
 "$nocull" "1"
 "Proxies" { "FizzlerVortex" {} }
}
''')
    vmf = Vmf()
    for center, half in [((0, 0, -8), (256, 256, 8)), ((0, 0, 200), (256, 256, 8)),
                         ((-264, 0, 96), (8, 256, 96)), ((264, 0, 96), (8, 256, 96)),
                         ((0, -264, 96), (256, 8, 96)), ((0, 264, 96), (256, 8, 96))]:
        vmf.world.append(vmf.box(center, half, "quality/fizzler/white"))
    if door:
        for center, half in [((-56, -160, 96), (4, 100, 96)),
                             ((-56, 160, 96), (4, 100, 96)),
                             ((-56, 0, 156), (4, 60, 44))]:
            vmf.world.append(vmf.box(center, half, "quality/fizzler/white"))
        vmf.entity("prop_testchamber_door", {"targetname": "door", "origin": "-56 0 0",
                   "angles": "0 0 0", "model": "models/props/portal_door_combined.mdl"})
    else:
        vmf.world.append(vmf.box((-56, 0, 40), (4, 24, 40), "quality/fizzler/white"))
    vmf.entity("light", {"origin": "-160 -160 160", "_light": "255 255 255 48"})
    vmf.entity("info_player_start", {"origin": "-160 -160 16", "angles": "0 45 0"})
    vmf.entity("trigger_portal_cleanser", {"targetname": "field", "origin": "0 0 0",
               "Visible": "1", "StartDisabled": "0", "spawnflags": "1", "UseScanline": "0"},
               solids=[vmf.box((0, 0, 80), (1, 64, 64), "quality/fizzler/field")])
    source = out / (("core_fizzler_door" if door else NAME) + ".vmf")
    source.write_text(vmf.text())
    # Only generated materials enter this private compile overlay. External
    # content remains read-only through the resolver's existing runtime layers.
    overlay = out / "compile-runtime"
    overlay.mkdir(exist_ok=True)
    for entry in runtime.iterdir():
        if entry.name != "portal2":
            (overlay / entry.name).symlink_to(entry.resolve(), target_is_directory=entry.is_dir())
    game = overlay / "portal2"
    game.mkdir()
    for entry in (runtime / "portal2").iterdir():
        if entry.name != "materials":
            (game / entry.name).symlink_to(entry.resolve(), target_is_directory=entry.is_dir())
    shutil.copytree(content / "materials", game / "materials")
    return source, content, overlay


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--out", type=Path, required=True)
    sepipe_loader.add_arguments(parser, "portal2")
    args = parser.parse_args()
    out, runtime = args.out.resolve(), sepipe_loader.packaged_runtime(args.profile, args.flavor)
    out.mkdir(parents=True, exist_ok=True)
    source, content, overlay = generate(out, runtime)
    tools = Path(json.loads(vmf_map_build.TOOLCHAIN.read_text())["compile_tools"])
    record = vmf_map_build.build(source, out / "map-build", tools, overlay)
    checks = Checks()
    metrics = {"schema": "fizzler-light-fixture/v1", "receiver_region": [40, 300, 220, 420],
               "blue_rise_min": 2.0, "off_return_max": 0.5, "states": [],
               "capture_evidence": "capture/evidence.json"}
    checks.check(record["status"] == "pass", "fixture.compile", str(record))
    if record["status"] != "pass":
        return checks.report()
    (content / "maps").mkdir(exist_ok=True)
    shutil.copy2(out / "map-build/compile" / (NAME + ".bsp"), content / "maps" / (NAME + ".bsp"))
    commands = ["noclip", "cmd setpos -160 -210 80", "cmd setang 24 54 0", "fov 90",
                "mat_force_tonemap_scale 1", "cl_portal_cleanser_scanline 0",
                "cl_portal_cleanser_default_intensity 1",
                "wait 180", "cl_fizzler_core_emission 0", "wait 20", "screenshot",
                "cl_fizzler_core_emission 1", "cl_fizzler_core_emission_report 1",
                "r_area_lights_report 1", "wait 20", "screenshot",
                "cl_fizzler_core_emission 0", "wait 20", "screenshot",
                "cl_fizzler_core_emission 1", "wait 20", "screenshot",
                "ent_fire field Disable", "wait 90", "screenshot",
                "ent_fire field Enable", "wait 180"]
    command = [sys.executable, str(ROOT / "tools/quality/portal_boot.py"),
               *sepipe_loader.boot_arguments(args), "--map", NAME,
               "--content-root", str(content), "--renderer", "native-vulkan", "--require-vulkan",
               "--require-sdl3", "--headless", "--width", "640", "--height", "480",
               "--out", str(out / "capture"), "--capture-wait", "900", "--timeout", "120",
               "--startup-command", "r_core_world 1"]
    for c in commands:
        command += ["--console-command", c]
    with (out / "capture.log").open("w") as log:
        done = subprocess.run(command, stdout=log, stderr=subprocess.STDOUT)
    checks.equal(done.returncode, 0, "fixture.boot")
    shots = sorted((out / "capture/runtime/portal2/screenshots").glob("*.tga"))
    checks.equal(len(shots), 6, "fixture.six-states")
    # Receiver regions and independent acceptance are recorded with the capture;
    # no field pixels participate in the receiver's on/off comparison.
    if len(shots) == 6:
        import numpy as np
        from PIL import Image
        images = [np.asarray(Image.open(p)).astype(float) for p in shots]
        for p, im in zip(shots, images):
            Image.fromarray(im.astype("uint8")).save(p.with_suffix(".png"))
        for i, j in [(1, 0), (3, 2), (5, 4)]:
            change = images[i] - images[j]
            # Broad lower image excludes the field and weapon; a second control
            # verifies unchanged source-off frames in this same receiver region.
            region = change[300:420, 40:220, :3]
            metrics["states"].append({"on": i, "off": j,
                                      "mean_rgb_rise": region.mean(axis=(0, 1)).tolist()})
            checks.check(float(region[:, :, 2].mean()) > 2.0,
                         "receiver.blue-rise.%d" % i, str(region.mean(axis=(0, 1)).tolist()))
        control = images[2][300:420, 40:220, :3] - images[0][300:420, 40:220, :3]
        metrics["off_return_mean_absolute"] = float(np.abs(control).mean())
        checks.check(float(np.abs(control).mean()) < 0.5, "receiver.off-returns", str(float(np.abs(control).mean())))
    text = (out / "capture/runtime/portal2/console.log").read_text(errors="replace")
    checks.check("fizzler core emitter" in text and "powerup 1.000" in text,
                 "source.authored-emitter")
    checks.check("intensity 1.000 powerup 1.000 strength 16.000 shot 0.000" in text,
                 "source.idle-without-portal-hit")
    checks.check("0 lit (1 without a slot)" in text, "source.core-only-no-cpu-slot")
    metrics.update(checks=checks.checks, failures=checks.failures,
                   status="pass" if checks.failures == 0 else "fail")
    (out / "receivers.json").write_text(json.dumps(metrics, indent=2) + "\n")
    return checks.report()


if __name__ == "__main__":
    sys.exit(main())
