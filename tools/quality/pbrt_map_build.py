#!/usr/bin/env python3
"""Build a playable BSP2 map from a PBRT-v4 or OpenUSD scene with one command.

    python3 tools/quality/pbrt_map_build.py \\
        --manifest quality/fixtures/pbrt-maps/living-room.json \\
        --out quality-results/living-room-map [--boot]

The manifest (`pbrt-map-manifest/v1`) holds only per-scene decisions that the
scene file cannot supply: map name, lightmap settings, collision selection and
optional material exclusions. Its `scene` is a `.pbrt` file or an authored
`.usd`/`.usda`/`.usdc` stage; every later step reads either through
`map_scene`. The toolchain file (`pbrt-map-toolchain/v1`)
holds machine paths. `tools/quality/pbrt_map_toolchain.py provision` builds the
pinned tools under build/toolchains/ and writes the default toolchain file;
`--check-toolchain` validates versions and capabilities and exits. Steps:

    scene        (USD scenes) usd_scene.py extract: map-scene/v1 model + normalized stage
    environment  sky (PBRT equal-area map or USD DomeLight) -> Z-up equirect EXR +
                 display texture (if any sky)
    stage        PBRT -> USD stage in Blender (+ optional Cycles reference render)
    reference-gate  Cycles render vs the scene's reference image (manifest reference.gate)
    bake         shared lightmap UVs + Cycles diffuse irradiance atlas
    denoise      OpenImageDenoise RTLightmap filter (manifest lightmap.denoise, default on)
    probe        optional reflection probe (manifest reflection_probe): six Cycles cube
                 faces, stored as roughness mips in rows the bake reserved in the LMAP
    ktx2         atlas -> linear RGBA16F KTX2 (LMAP payload)
    sky          render stage = lighting stage + SkyDome for window views (scenes with a sky)
    collision    shell/solids/spawn VMF
    compile      vbsp2 / vvis / vrad -> collision + PVS BSP
    pack         USD triangles -> WMSH + LMAP inside BSP2
    content      VTF/VMT materials + maps/<map>.bsp content root
    boot         (with --boot) headless native Vulkan boot and screenshot at the spawn
    camera-boot  (with --boot) second boot with the camera at the PBRT reference eye
    runtime-gate camera-matched game frame vs the Cycles render (manifest runtime_gate)
    traversal-boot / traversal  drop the player onto the highest walkable tops and
                 the spawn floor; each must come to rest on its surface

Each step records the digests of its inputs, script and settings in
`<out>/steps.json`; unchanged steps are skipped, so editing collision does not
re-run the bake. `--from STEP` forces a step and everything after it.
A failing pixel gate stops the build unless `--keep-going` is given; then the
map is still finished, build.json reports `gate-failed` and the exit is nonzero.
A finished map is published to run/maps/<map> (`playable_maps.py`), so
`./play <map>` loads it; `--no-publish` skips that.
"""

import argparse
import hashlib
import json
import os
import shutil
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import map_scene  # noqa: E402
import pbrt_map_toolchain  # noqa: E402
import pbrt_traversal  # noqa: E402
import playable_maps  # noqa: E402
import reference_compare  # noqa: E402

STEPS = ("scene", "environment", "stage", "reference-gate", "bake", "denoise", "probe", "ktx2", "sky",
         "collision", "compile", "pack", "content", "boot", "camera-boot", "runtime-gate",
         "traversal-boot", "traversal")
BAKE_SCOPE = "pbrt-shared-lightmap-uv-and-cycles-bake"
GATES = ("reference-gate", "runtime-gate", "traversal")
SCENE_SCRIPTS = ["pbrt_scene.py", "map_scene.py"]
def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def digest(value):
    return hashlib.sha256(json.dumps(value, sort_keys=True, default=str).encode()).hexdigest()


def gate_summary(path):
    result = json.loads(Path(path).read_text())
    if "scored" not in result:
        return {"status": result["status"], "probes": len(result.get("probes", []))}
    return {"status": result["status"], **result[result["scored"]]}


def load_manifest(path):
    manifest = json.loads(Path(path).read_text())
    if manifest.get("schema") != "pbrt-map-manifest/v1":
        raise ValueError("manifest schema must be pbrt-map-manifest/v1")
    name = manifest.get("map", "")
    if not name.replace("_", "").isalnum() or name.lower() != name:
        raise ValueError("manifest map must be lowercase [a-z0-9_]")
    manifest["scene"] = str((ROOT / manifest["scene"]).resolve())
    manifest["scene_format"] = "usd" if map_scene.is_usd(manifest["scene"]) else "pbrt"
    return manifest


class Pipeline:
    def __init__(self, manifest, toolchain, out, force_from, boot, keep_going=False,
                 publish=True):
        self.manifest = manifest
        self.tools = toolchain
        self.out = out.resolve()
        self.map = manifest["map"]
        self.usd = manifest["scene_format"] == "usd"
        # A USD scene is read through the model its `scene` step extracts.
        self.scene_file = (self.out / "scene" / "scene.json") if self.usd else manifest["scene"]
        self.scene = None if self.usd else map_scene.parse(self.scene_file)
        self.state_path = self.out / "steps.json"
        self.state = json.loads(self.state_path.read_text()) if self.state_path.is_file() else {}
        self.force_from = STEPS.index(force_from) if force_from else len(STEPS)
        self.boot = boot
        self.keep_going = keep_going
        self.publish = publish
        self.failed_gates = []
        lightmap = manifest.get("lightmap", {})
        self.lightmap = {"size": lightmap.get("size", 2048),
                         "samples": lightmap.get("samples", 64),
                         "preview_gain": lightmap.get("preview_gain", 1.0),
                         "denoise": lightmap.get("denoise", True),
                         "device": lightmap.get("device", "auto"),
                         "exclude_materials": lightmap.get("exclude_materials", [])}
        world_mesh = manifest.get("world_mesh", {})
        self.world_mesh = {"weld_materials": world_mesh.get("weld_materials", []),
                           "weld_distance_source_units":
                               world_mesh.get("weld_distance_source_units", 0.0)}
        self.paths = {
            "environment": self.out / "environment.exr",
            "stage": self.out / "stage" / (self.map + ".usdc"),
            "stage_receipt": self.out / "stage.json",
            "reference": self.out / "reference" / "cycles.png",
            "lighting_stage": self.out / "lighting" / (self.map + "_lighting.usdc"),
            "atlas": self.out / "lighting" / "atlas.exr",
            "coverage": self.out / "lighting" / "uv-coverage.exr",
            "atlas_receipt": self.out / "lighting" / "atlas.exr.json",
            "denoised": self.out / "lighting" / "atlas-denoised.exr",
            "denoised_receipt": self.out / "lighting" / "atlas-denoised.exr.json",
            "ktx2": self.out / "lighting" / "atlas.ktx2",
            "probe": self.out / "lighting" / "probe",
            "sky_texture": self.out / "sky.png",
            "render_stage": self.out / "lighting" / (self.map + "_render.usda"),
            "collision": self.out / "collision",
            "bsp": self.out / "collision" / (self.map + "_collision.bsp"),
            "wmsh": self.out / (self.map + ".wmsh"),
            "bsp2": self.out / (self.map + ".bsp2"),
            "content": self.out / "content",
            "boot": self.out / "boot",
            "reference_gate": self.out / "reference" / "gate.json",
            "camera_boot": self.out / "camera-boot",
            "runtime_gate": self.out / "camera-boot" / "gate.json",
            "traversal_boot": self.out / "traversal-boot",
            "traversal": self.out / "traversal-boot" / "result.json",
        }
        self.logs = self.out / "logs"

    def run(self, step, command, env=None):
        self.logs.mkdir(parents=True, exist_ok=True)
        log = self.logs / (step + ".log")
        started = time.monotonic()
        with log.open("a") as handle:
            handle.write("$ " + " ".join(map(str, command)) + "\n")
            handle.flush()
            result = subprocess.run([str(part) for part in command], stdout=handle,
                                    stderr=subprocess.STDOUT, cwd=ROOT,
                                    env=dict(os.environ, **(env or {})))
        if result.returncode:
            tail = log.read_text().splitlines()[-15:]
            raise SystemExit("step %s failed (exit %d); log %s:\n  %s" %
                             (step, result.returncode, log, "\n  ".join(tail)))
        return time.monotonic() - started

    def blender(self, step, script, arguments):
        return self.run(step, [self.tools["blender"], "-b", "--factory-startup",
                               "--python-exit-code", "9", "--python", HERE / script,
                               "--"] + arguments, env={"OCIO": self.tools["ocio"]})

    def usd_python(self, step, script, arguments):
        return self.run(step, [self.tools["usd_python"], HERE / script] + arguments,
                        env={"PYTHONPATH": self.tools["usd_pythonpath"],
                             "PXR_PLUGINPATH_NAME": str(Path(self.tools["compile_tools"]) /
                                                        "share/sourceWorld")})

    def step(self, name, inputs, settings, scripts, outputs, action):
        """Run `action` unless its inputs, settings and scripts are unchanged."""
        key = digest({"inputs": {str(p): sha256(p) if Path(p).is_file() else None
                                 for p in inputs},
                      "settings": settings,
                      "scripts": {s: sha256(HERE / s) for s in scripts}})
        index = STEPS.index(name)
        previous = self.state.get(name, {})
        # Keys hash every input file, so a rebuilt upstream output invalidates
        # exactly the steps that read it; only --from forces a cascade.
        fresh = (previous.get("key") == key and index < self.force_from and
                 all(Path(p).exists() for p in outputs))
        if fresh:
            print("[%s] up to date" % name)
            return
        for path in outputs:
            path = Path(path)
            if path.is_dir():
                shutil.rmtree(path)
            elif path.exists():
                path.unlink()
        (self.logs / (name + ".log")).unlink(missing_ok=True)
        print("[%s] running..." % name, flush=True)
        try:
            seconds = action()
        except SystemExit as failure:
            if not (self.keep_going and name in GATES):
                raise
            # Not recorded in steps.json, so the gate runs again next build.
            print("[%s] FAILED, continuing (--keep-going): %s" % (name, failure), flush=True)
            self.failed_gates.append(name)
            return
        missing = [str(p) for p in outputs if not Path(p).exists()]
        if missing:
            raise SystemExit("step %s did not produce %s" % (name, ", ".join(missing)))
        self.state[name] = {"key": key, "seconds": round(seconds or 0, 1),
                            "outputs": {str(Path(p).relative_to(self.out)):
                                        sha256(p) if Path(p).is_file() else "directory"
                                        for p in outputs}}
        self.state_path.write_text(json.dumps(self.state, indent=2, sort_keys=True) + "\n")
        print("[%s] done in %.1fs" % (name, seconds or 0), flush=True)

    def extract_usd_scene(self):
        """`scene` step: the USD scene's model and normalized stage."""
        p = self.paths
        model = self.scene_file
        previous = json.loads(model.read_text()).get("source_files", []) \
            if model.is_file() else []
        # The model lists every layer and texture it read; any change reruns.
        inputs = [self.manifest["scene"]] + [path for path in previous if Path(path).is_file()]
        self.step("scene", inputs, {"missing_inputs": [path for path in previous
                                                       if not Path(path).is_file()]},
                  ["usd_scene.py"], [model, p["stage"]],
                  lambda: self.usd_python("scene", "usd_scene.py", [
                      "extract", "--scene", self.manifest["scene"], "--out-scene", model,
                      "--out-stage", p["stage"]]))
        self.scene = map_scene.parse(model)

    def build(self):
        self.out.mkdir(parents=True, exist_ok=True)
        p = self.paths
        if self.usd:
            self.extract_usd_scene()
        scene = str(self.scene_file)
        environment = p["environment"] if self.scene["environment"] else None
        if environment:
            def write_environment():
                import imageio.v3 as iio
                import numpy as np
                started = time.monotonic()
                pixels = map_scene.environment_equirect(self.scene, 2048)
                iio.imwrite(environment, pixels.astype(np.float32))
                iio.imwrite(p["sky_texture"], map_scene.sky_display(pixels))
                return time.monotonic() - started
            sky_inputs = [scene] if self.usd else \
                [scene, Path(scene).parent / self.scene["environment"]["filename"]]
            self.step("environment", sky_inputs, {"width": 2048},
                      ["pbrt_scene.py", "map_scene.py"], [environment, p["sky_texture"]],
                      write_environment)
        env_args = ["--environment", environment] if environment else []
        reference = self.manifest.get("reference", {}).get("render")
        stage_args = ["--scene", scene, "--stage", p["stage"], "--receipt",
                      p["stage_receipt"]] + env_args
        if reference:
            stage_args += ["--render", p["reference"], "--samples",
                           str(reference.get("samples", 64)),
                           "--scale", str(reference.get("scale", 1.0)),
                           "--device", reference.get("device", "auto")]
        # A USD scene's stage is the `scene` step's output: an input here.
        self.step("stage", [scene] + ([environment] if environment else []) +
                  ([p["stage"]] if self.usd else []),
                  {"reference": reference}, SCENE_SCRIPTS + ["pbrt_blender.py",
                                                             "pbrt_usd_stage.py"],
                  ([] if self.usd else [p["stage"]]) + [p["stage_receipt"]] +
                  ([p["reference"]] if reference else []),
                  lambda: self.blender("stage", "pbrt_usd_stage.py", stage_args))
        supplied = self.manifest.get("reference", {})
        if reference and supplied.get("gate"):
            gate_args = ["render", "--reference", ROOT / supplied["png"],
                         "--candidate", p["reference"], "--gate", json.dumps(supplied["gate"]),
                         "--out", p["reference_gate"]]
            if supplied.get("exr"):
                gate_args += ["--reference-exr", ROOT / supplied["exr"], "--candidate-exr",
                              p["reference"].with_suffix(".exr"), "--display-candidate",
                              p["reference"].with_name("cycles-reference-display.png")]
            self.step("reference-gate", [p["reference"], ROOT / supplied["png"]],
                      supplied["gate"], ["reference_compare.py"], [p["reference_gate"]],
                      lambda: self.run("reference-gate", [sys.executable,
                                                          HERE / "reference_compare.py"] +
                                       gate_args))
        probe = self.manifest.get("reflection_probe")
        probe_width = probe.get("width", 512) if probe else 0
        bake_args = ["--reserve-rows", str(probe_width // 2),
                     "--scene", scene, "--stage", p["stage"], "--out-stage", p["lighting_stage"],
                     "--out-exr", p["atlas"], "--out-coverage-exr", p["coverage"],
                     "--size", str(self.lightmap["size"]),
                     "--samples", str(self.lightmap["samples"]),
                     "--device", self.lightmap["device"]] + env_args
        for material in self.lightmap["exclude_materials"]:
            bake_args += ["--exclude-material", material]
        self.step("bake", [p["stage"]] + ([environment] if environment else []),
                  dict({k: self.lightmap[k] for k in ("size", "samples", "exclude_materials",
                                                      "device")},
                       reserve_rows=probe_width // 2),
                  SCENE_SCRIPTS + ["pbrt_blender.py", "pbrt_lightmap_bake.py"],
                  [p["lighting_stage"], p["atlas"], p["coverage"], p["atlas_receipt"]],
                  lambda: self.blender("bake", "pbrt_lightmap_bake.py", bake_args))
        atlas, atlas_receipt, scope = p["atlas"], p["atlas_receipt"], BAKE_SCOPE
        self.step("denoise", [p["atlas"], p["coverage"], p["atlas_receipt"]],
                  {"uv_coverage": True, "denoise": self.lightmap["denoise"]},
                  ["lightmap_denoise.py"], [p["denoised"], p["denoised_receipt"]],
                  lambda: self.run("denoise", [sys.executable, HERE / "lightmap_denoise.py",
                                               "--exr", p["atlas"], "--bake-evidence",
                                               p["atlas_receipt"], "--coverage-exr",
                                               p["coverage"], "--out", p["denoised"]] +
                                   ([] if self.lightmap["denoise"] else ["--skip-denoise"])))
        atlas, atlas_receipt = p["denoised"], p["denoised_receipt"]
        scope = BAKE_SCOPE + ("-denoised" if self.lightmap["denoise"] else "-gutter-filled")
        probe_args = []
        if probe:
            face_args = ["--scene", scene, "--stage", p["lighting_stage"], "--out-dir", p["probe"],
                         "--face-size", str(probe.get("face_size", 256)),
                         "--samples", str(probe.get("samples", 512)),
                         "--device", self.lightmap["device"]] + env_args
            if probe.get("position"):
                face_args += ["--position"] + [str(value) for value in probe["position"]]
            self.step("probe", [p["lighting_stage"]] + ([environment] if environment else []),
                      probe, SCENE_SCRIPTS + ["pbrt_reflection_probe.py",
                                              "reflection_probe.py", "pbrt_blender.py"],
                      [p["probe"]],
                      lambda: self.blender("probe", "pbrt_reflection_probe.py", face_args))
            probe_args = ["--probe-dir", p["probe"], "--probe-width", str(probe_width)]
        self.step("ktx2", [atlas, atlas_receipt, p["lighting_stage"]] +
                  ([p["probe"] / "probe.json"] if probe else []),
                  {"preview_gain": self.lightmap["preview_gain"], "scope": scope,
                   "probe_width": probe_width},
                  ["lightmap_ktx2.py", "reflection_probe.py"], [p["ktx2"]],
                  lambda: self.run("ktx2", [sys.executable, HERE / "lightmap_ktx2.py",
                                            "--exr", atlas, "--bake-evidence", atlas_receipt,
                                            "--lighting-stage", p["lighting_stage"],
                                            "--ktx-tool", self.tools["ktx"],
                                            "--preview-gain", str(self.lightmap["preview_gain"]),
                                            "--expected-scope", scope, "--out", p["ktx2"]] +
                                           probe_args))
        pack_stage = p["lighting_stage"]
        if environment:
            pack_stage = p["render_stage"]
            self.step("sky", [p["lighting_stage"], p["sky_texture"]], {},
                      ["pbrt_sky_dome.py"],
                      [p["render_stage"], p["render_stage"].with_name(p["render_stage"].name + ".json")],
                      lambda: self.usd_python("sky", "pbrt_sky_dome.py", [
                          "--stage", p["lighting_stage"], "--sky-texture", p["sky_texture"],
                          "--out-stage", p["render_stage"]]))
        collision = self.manifest.get("collision", {})
        collision_args = ["--scene", scene, "--stage", p["lighting_stage"], "--map-name",
                          self.map, "--out-dir", p["collision"]]
        for flag, key in (("--envelope-mesh", "envelope_meshes"),
                          ("--solid-material", "solid_materials"),
                          ("--solid-mesh", "solid_meshes")):
            for value in collision.get(key, []):
                collision_args += [flag, value]
        self.step("collision", [scene, p["lighting_stage"]], collision,
                  SCENE_SCRIPTS + ["pbrt_collision_vmf.py"], [p["collision"]],
                  lambda: self.usd_python("collision", "pbrt_collision_vmf.py", collision_args))
        vmf = p["collision"] / (self.map + "_collision.vmf")
        tools = Path(self.tools["compile_tools"])
        game = p["collision"] / "game"

        def compile_map():
            env = {"PXR_PLUGINPATH_NAME": str(tools / "share/sourceWorld")}
            seconds = self.run("compile", [tools / "vbsp2", "-game", game, vmf], env)
            seconds += self.run("compile", [tools / "vvis", "-threads", "4", "-game", game,
                                            p["bsp"]])
            seconds += self.run("compile", [tools / "vrad", "-bounce", "0", "-threads", "4",
                                            "-game", game, p["bsp"]])
            return seconds
        self.step("compile", [vmf], {"tools": str(tools)}, [], [p["bsp"]], compile_map)
        self.step("pack", [pack_stage, p["lighting_stage"], p["bsp"], p["ktx2"]],
                  {"prefix": self.map, **self.world_mesh},
                  ["usd_worldmesh_pack.py", "worldmesh_seam_weld.py"],
                  [p["wmsh"], p["wmsh"].with_name(p["wmsh"].name + ".json"), p["bsp2"]],
                  lambda: self.usd_python("pack", "usd_worldmesh_pack.py", [
                      "--stage", pack_stage, "--bsp", p["bsp"], "--material-prefix",
                      self.map, "--require-lightmap-uv"] +
                      (["--include-emitters"] if self.scene["emitters"] else []) + [
                      "--lightmap-ktx2", p["ktx2"], "--bsp2tool", self.tools["bsp2tool"],
                      "--out", p["wmsh"], "--out-bsp2", p["bsp2"]] +
                      (["--weld-distance-source-units",
                        str(self.world_mesh["weld_distance_source_units"])]
                       if self.world_mesh["weld_materials"] else []) +
                      [item for material in self.world_mesh["weld_materials"]
                       for item in ("--weld-material", material)]))
        sky_args = ["--sky-texture", p["sky_texture"]] if environment else []
        self.step("content", [scene, p["stage_receipt"], p["bsp2"]] +
                  ([p["sky_texture"]] if environment else []), {},
                  SCENE_SCRIPTS + ["pbrt_playable_content.py", "vtf_content.py"],
                  [p["content"], p["content"].with_suffix(".json")],
                  lambda: self.run("content", [sys.executable, HERE / "pbrt_playable_content.py",
                                               "--scene", scene, "--stage-receipt",
                                               p["stage_receipt"], "--bsp2", p["bsp2"],
                                               "--vtex", tools / "vtex", "--map-name", self.map,
                                               "--out", p["content"]] + sky_args))
        if self.boot:
            content_files = sorted(f for f in p["content"].rglob("*") if f.is_file())
            self.step("boot", content_files, {"build": self.tools["client_build"]},
                      ["portal_boot.py"], [p["boot"]],
                      lambda: self.run("boot", [sys.executable, HERE / "portal_boot.py",
                                                "--runtime", self.tools["runtime"],
                                                "--build", self.tools["client_build"],
                                                "--content-root", p["content"],
                                                "--renderer", "native-vulkan", "--headless",
                                                "--map", self.map, "--console-command",
                                                "r_worldmesh_draw 2", "--out", p["boot"]]))
        runtime_gate = self.manifest.get("runtime_gate")
        if self.boot and reference:
            commands, _ = reference_compare.camera_commands(self.scene)
            boot_args = ["--runtime", self.tools["runtime"], "--build",
                         self.tools["client_build"], "--content-root", p["content"],
                         "--renderer", "native-vulkan", "--headless", "--map", self.map,
                         "--out", p["camera_boot"]]
            for command in commands:
                boot_args += ["--console-command", command]
            content_files = sorted(f for f in p["content"].rglob("*") if f.is_file())
            self.step("camera-boot", content_files,
                      {"build": self.tools["client_build"], "commands": commands},
                      ["portal_boot.py", "reference_compare.py"], [p["camera_boot"]],
                      lambda: self.run("camera-boot", [sys.executable, HERE / "portal_boot.py"] +
                                       boot_args))
            gate_args = ["runtime", "--scene", scene, "--reference", p["reference"],
                         "--boot-evidence", p["camera_boot"] / "evidence.json",
                         "--matched-frame", p["camera_boot"] / "matched.png",
                         "--out", p["runtime_gate"]]
            if runtime_gate:
                gate_args += ["--gate", json.dumps(runtime_gate)]
            self.step("runtime-gate", [p["camera_boot"] / "evidence.json", p["reference"]],
                      runtime_gate or {}, ["reference_compare.py"], [p["runtime_gate"]],
                      lambda: self.run("runtime-gate", [sys.executable,
                                                        HERE / "reference_compare.py"] +
                                       gate_args))
        if self.boot:
            receipt_path = p["collision"] / "collision-receipt.json"
            probe_commands = pbrt_traversal.commands(json.loads(receipt_path.read_text()))
            content_files = sorted(f for f in p["content"].rglob("*") if f.is_file())
            self.step("traversal-boot", content_files + [receipt_path],
                      {"build": self.tools["client_build"], "commands": probe_commands},
                      ["portal_boot.py", "pbrt_traversal.py"], [p["traversal_boot"]],
                      lambda: self.run("traversal-boot", [
                          sys.executable, HERE / "portal_boot.py", "--runtime",
                          self.tools["runtime"], "--build", self.tools["client_build"],
                          "--content-root", p["content"], "--renderer", "native-vulkan",
                          "--headless", "--map", self.map, "--console-command",
                          probe_commands[0], "--out", p["traversal_boot"]]))
            self.step("traversal", [p["traversal_boot"] / "evidence.json", receipt_path], {},
                      ["pbrt_traversal.py"], [p["traversal"]],
                      lambda: self.run("traversal", [
                          sys.executable, HERE / "pbrt_traversal.py", "--collision-receipt",
                          receipt_path, "--boot-evidence", p["traversal_boot"] / "evidence.json",
                          "--out", p["traversal"]]))
        summary = {"status": "gate-failed" if self.failed_gates else "pass",
                   "failed_gates": self.failed_gates, "map": self.map, "manifest_scene": scene,
                   "content_root": str(p["content"]),
                   "bsp2_sha256": sha256(p["bsp2"]),
                   "gates": {name: gate_summary(path) for name, path in
                             (("reference", p["reference_gate"]), ("runtime", p["runtime_gate"]),
                              ("traversal", p["traversal"]))
                             if path.is_file()},
                   "steps": {name: self.state[name]["seconds"] for name in STEPS
                             if name in self.state}}
        (self.out / "build.json").write_text(json.dumps(summary, indent=2) + "\n")
        print(json.dumps(summary, indent=2))
        if self.publish:
            playable_maps.publish(summary)
            print("published to %s; play it with ./play %s" %
                  (playable_maps.STORE / self.map, self.map))
        if self.failed_gates:
            raise SystemExit("gates failed: " + ", ".join(self.failed_gates))


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--manifest", type=Path, required=True)
    parser.add_argument("--toolchain", type=Path,
                        help="pbrt-map-toolchain/v1 file (default: the provisioned "
                             "build/toolchains/pbrt-map-toolchain.json)")
    parser.add_argument("--out", type=Path)
    parser.add_argument("--from", dest="force_from", choices=STEPS,
                        help="rebuild this step and every later step")
    parser.add_argument("--boot", action="store_true",
                        help="boot the map headless in native Vulkan and screenshot it")
    parser.add_argument("--keep-going", action="store_true",
                        help="finish the map when a pixel gate fails; build.json records "
                             "status gate-failed and the exit status stays nonzero")
    parser.add_argument("--no-publish", action="store_true",
                        help="do not publish the finished map to run/maps for ./play")
    parser.add_argument("--check-toolchain", action="store_true")
    args = parser.parse_args()
    profile, _ = pbrt_map_toolchain.load_profiles()
    toolchain = pbrt_map_toolchain.load(
        args.toolchain or ROOT / profile["layout"]["toolchain_file"])
    manifest = load_manifest(args.manifest)
    if args.check_toolchain:
        print("toolchain ok")
        return
    if not args.out:
        parser.error("--out is required")
    Pipeline(manifest, toolchain, args.out, args.force_from, args.boot,
             args.keep_going, not args.no_publish).build()


if __name__ == "__main__":
    main()
