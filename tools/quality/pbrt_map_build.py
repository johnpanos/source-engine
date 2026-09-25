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
`--check-toolchain` validates versions and capabilities and exits.

A manifest with `legacy_bsp` instead of `scene` relights a compiled map
(`legacy_bsp_relight.py` writes one): the scene is authored from the BSP, the
BSP itself replaces `collision` and `compile` (its gameplay lumps are carried
unchanged), its baked world lights are removed in `pack`, the scene's light
emitters stay invisible, and there is no sky dome or traversal gate. Steps:

    legacy-scene (legacy_bsp manifests) legacy_bsp_scene.py: the BSP's world faces,
                 materials, occluders and lights as an authored USD scene
    scene        (USD scenes) usd_scene.py extract: map-scene/v1 model + normalized stage
    environment  sky (PBRT equal-area map or USD DomeLight) -> Z-up equirect EXR +
                 display texture (if any sky)
    stage        PBRT -> USD stage in Blender (+ optional Cycles reference render)
    reference-gate  Cycles render vs the scene's reference image (manifest reference.gate)
    bake         shared lightmap UVs + Cycles diffuse irradiance atlas
    denoise      OpenImageDenoise RTLightmap filter (manifest lightmap.denoise, default on)
    probe        optional reflection probe (manifest reflection_probe): six Cycles cube
                 faces, stored as roughness mips in rows the bake reserved in the LMAP
    probe-volume optional RFC 0011 PRBV (profile/manifest probe_volume): Cycles-baked
                 irradiance and ray-traced visibility per probe (probe_volume_bake.py);
                 the map then has no vrad fallback light, and its leaf ambient is
                 derived from the volume (leaf_ambient_from_prbv.py) in `pack`
    radiosity    optional RFC 0011 G4 RTRN (profile/manifest radiosity, needs probe_volume):
                 patches, form factors, per-light injection and the probe gather
                 (radiosity_transfer_bake.py); its switchable lights become named
                 `light` entities in `collision` and the transfer is packed beside PRBV
    sdf          optional RFC 0011 G6 SDFV (profile/manifest sdf_volume, needs radiosity):
                 the static world's signed distance, reflectance and emission per voxel
                 and its analytic lights, styled as the transfer's sources
                 (sdf_volume_bake.py); packed beside RTRN
    ktx2         atlas -> linear RGBA16F KTX2 (LMAP payload)
    sky          render stage = lighting stage + SkyDome for window views (scenes with a sky)
    collision    shell/solids/spawn VMF
    compile      vbsp2 / vvis / vrad -> collision + PVS BSP
    pack         USD triangles -> WMSH + LMAP (+ PRBV) inside BSP2
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
import re
import shutil
import struct
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import bake_progress  # noqa: E402
import cycles_device  # noqa: E402
import map_scene  # noqa: E402
import pbrt_map_toolchain  # noqa: E402
import pbrt_traversal  # noqa: E402
import playable_maps  # noqa: E402
import reference_compare  # noqa: E402

STEPS = ("legacy-scene", "scene", "environment", "stage", "reference-gate", "bake", "denoise", "directional",
         "probe", "probe-volume", "radiosity", "sdf", "ktx2", "sky",
         "collision", "compile", "pack", "content", "boot", "camera-boot", "runtime-gate",
         "traversal-boot", "traversal", "audit")
BAKE_SCOPE = "pbrt-shared-lightmap-uv-and-cycles-bake"
GATES = ("reference-gate", "runtime-gate", "traversal", "audit")
PROFILES = ROOT / "quality" / "map_export_profiles"
DEFAULT_QUALITY = "source2"
LEGACY_QUALITY = "legacy-relight"
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
    if "legacy_bsp" in manifest:
        if "scene" in manifest:
            raise ValueError("a manifest names a scene or a legacy_bsp, not both")
        manifest["legacy_bsp"] = str((ROOT / manifest["legacy_bsp"]).resolve())
        if not Path(manifest["legacy_bsp"]).is_file():
            raise ValueError("legacy_bsp does not exist: " + manifest["legacy_bsp"])
        # The `legacy-scene` step writes the scene (Pipeline sets its path).
        manifest["scene"] = None
        manifest["scene_format"] = "usd"
        manifest.setdefault("quality", LEGACY_QUALITY)
        return manifest
    manifest["scene"] = str((ROOT / manifest["scene"]).resolve())
    manifest["scene_format"] = "usd" if map_scene.is_usd(manifest["scene"]) else "pbrt"
    manifest.setdefault("quality", DEFAULT_QUALITY)
    return manifest


def load_profile(name):
    """A declared export-quality profile (quality/map_export_profiles/<name>.json)."""
    path = PROFILES / (name + ".json")
    if not path.is_file():
        raise ValueError("unknown map export quality profile " + name)
    profile = json.loads(path.read_text())
    if profile.get("schema") != "map-export-profile/v1" or profile.get("name") != name:
        raise ValueError("invalid map export profile " + str(path))
    profile["path"] = str(path)
    return profile


def bsp_entities(path):
    """The entity lump of a legacy VBSP as a list of key-value dicts."""
    data = Path(path).read_bytes()
    offset, length = struct.unpack_from("<ii", data, 8)
    text = data[offset:offset + length].decode("latin-1").rstrip("\0")
    entities, current = [], None
    for match in re.finditer(r'"([^"]*)"\s*"([^"]*)"|([{}])', text):
        if match.group(3) == "{":
            current = {}
        elif match.group(3) == "}":
            entities.append(current)
            current = None
        elif current is not None:
            current[match.group(1)] = match.group(2)
    return entities


def check_legacy_light_styles(bsp, controls):
    """Each styled source of a relit map is a style of the map's own lights
    (vbsp writes a named light's switchable style, 32+, into its entity; the
    preset animated styles 1-31 are authored there)."""
    styles = {entity.get("style") for entity in bsp_entities(bsp)
              if entity.get("classname", "").startswith("light")}
    wrong = [(c["name"], c["style"]) for c in controls if str(c["style"]) not in styles]
    if wrong:
        raise ValueError("styled sources with no light entity of their style: %s" % wrong)


def check_light_styles(bsp, controls):
    """Each switchable source's `light` entity compiled with its RTRN style."""
    styles = {entity.get("targetname"): entity.get("style") for entity in bsp_entities(bsp)
              if entity.get("classname") == "light" and entity.get("targetname")}
    wrong = [(c["name"], c["style"], styles.get(c["name"])) for c in controls
             if styles.get(c["name"]) != str(c["style"])]
    if wrong:
        raise ValueError("compiled light styles differ from the radiosity transfer's: %s" %
                         wrong)


def with_defaults(manifest, profile, key):
    """Manifest value over the profile default; dicts merge, null disables."""
    default = profile.get(key)
    if key not in manifest:
        return default
    value = manifest[key]
    if isinstance(value, dict) and isinstance(default, dict):
        return dict(default, **value)
    return value or None


class Pipeline:
    def __init__(self, manifest, toolchain, out, force_from, boot, keep_going=False,
                 publish=True):
        self.manifest = manifest
        self.tools = toolchain
        self.out = out.resolve()
        self.map = manifest["map"]
        self.usd = manifest["scene_format"] == "usd"
        self.legacy = manifest.get("legacy_bsp")
        if self.legacy:
            manifest["scene"] = str(out.resolve() / "legacy-scene" / "scene.usda")
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
        # Export quality comes from a declared profile (default `source2`);
        # the manifest overrides individual settings.
        self.profile = load_profile(manifest["quality"])
        lightmap = with_defaults(manifest, self.profile, "lightmap")
        self.lightmap = {"size": lightmap.get("size", 2048),
                         "samples": lightmap.get("samples", 64),
                         "preview_gain": lightmap.get("preview_gain", 1.0),
                         "denoise": lightmap.get("denoise", True),
                         "directional": lightmap.get("directional", False),
                         "device": lightmap.get("device", cycles_device.BAKE_DEVICE),
                         "light_paths": lightmap.get("light_paths", "blender-default"),
                         # RFC 0011 separated light: LMAP v2 layers beside the total.
                         "layers": list(lightmap.get("layers", [])),
                         "exclude_materials": lightmap.get("exclude_materials", [])}
        self.probe = with_defaults(manifest, self.profile, "reflection_probe")
        self.probe_volume = with_defaults(manifest, self.profile, "probe_volume")
        self.radiosity = with_defaults(manifest, self.profile, "radiosity")
        if self.radiosity and not self.probe_volume:
            raise ValueError("radiosity needs the probe_volume it gathers into")
        self.sdf_volume = with_defaults(manifest, self.profile, "sdf_volume")
        if self.legacy and not self.probe_volume:
            raise ValueError("a relit map needs a probe_volume: its world lights move there")
        if self.sdf_volume and not self.radiosity:
            raise ValueError("sdf_volume needs the radiosity transfer its light styles follow")
        reference = dict(self.profile.get("reference") or {}, **manifest.get("reference", {}))
        self.reference_render = reference.get("render")
        self.runtime_gate = with_defaults(manifest, self.profile, "runtime_gate")
        world_mesh = with_defaults(manifest, self.profile, "world_mesh") or {}
        self.world_mesh = {"weld_materials": world_mesh.get("weld_materials", []),
                           "weld_distance_source_units":
                               world_mesh.get("weld_distance_source_units", 0.0)}
        # Materials baked and occluding but never drawn (a relit map's nodraw
        # brush sides): left out of the WMSH.
        self.hidden_materials = list(world_mesh.get("exclude_materials", []))
        self.paths = {
            "legacy_scene": self.out / "legacy-scene",
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
            "layers": self.out / "lighting" / "layers",
            "directional_bakes": self.out / "lighting" / "directional",
            "sun_visibility": self.out / "lighting" / "sun_visibility.exr",
            "directional": self.out / "lighting" / "atlas-directional.exr",
            "audit": self.out / "audit.json",
            "ktx2": self.out / "lighting" / "atlas.ktx2",
            "probe": self.out / "lighting" / "probe",
            "prbv": self.out / "lighting" / "probe_volume.prbv",
            "prbv_work": self.out / "lighting" / "probe_volume",
            "rtrn": self.out / "lighting" / "radiosity.rtrn",
            "rtrn_work": self.out / "lighting" / "radiosity",
            "sdfv": self.out / "lighting" / "field.sdfv",
            "sdfv_work": self.out / "lighting" / "sdf",
            "bsp_ambient": self.out / (self.map + "_leaf_ambient.bsp"),
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

    def run(self, step, command, env=None, progress=None):
        """Run a step's command, its output going to logs/<step>.log; the
        lines `progress.feed` returns for its output are shown as they come."""
        self.logs.mkdir(parents=True, exist_ok=True)
        log = self.logs / (step + ".log")
        started = time.monotonic()
        with log.open("a") as handle:
            handle.write("$ " + " ".join(map(str, command)) + "\n")
            handle.flush()
            process = subprocess.Popen([str(part) for part in command], stdout=subprocess.PIPE,
                                       stderr=subprocess.STDOUT, cwd=ROOT, text=True,
                                       errors="replace", bufsize=1,
                                       env=dict(os.environ, **(env or {})))
            for line in process.stdout:
                handle.write(line)
                for message in (progress.feed(line) if progress else []):
                    print("[%s] %s" % (step, message), flush=True)
            process.wait()
        if process.returncode:
            tail = log.read_text().splitlines()[-15:]
            raise SystemExit("step %s failed (exit %d); log %s:\n  %s" %
                             (step, process.returncode, log, "\n  ".join(tail)))
        return time.monotonic() - started

    def light_controls(self):
        """The radiosity transfer's switchable sources, in style order."""
        receipt = self.paths["rtrn_work"] / "rtrn-bake.json"
        sources = json.loads(receipt.read_text())["sources"]
        return [source for source in sources if source["style"] >= 0]

    def blender(self, step, script, arguments):
        # Cycles' debug log reports each bake's tiles and sample batches;
        # bake_progress turns it (and the script's PROGRESS lines) into progress.
        return self.run(step, [self.tools["blender"], "-b", "--factory-startup",
                               "--log-level", "debug", "--log", "cycles",
                               "--python-exit-code", "9", "--python", HERE / script,
                               "--"] + arguments, env={"OCIO": self.tools["ocio"]},
                        progress=bake_progress.CyclesProgress())

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
        # Manifest `simplify`: {prim path glob: vertex-cluster cell in metres}.
        simplify = self.manifest.get("simplify", {})
        simplify_args = [item for pattern, cell in sorted(simplify.items())
                         for item in ("--simplify", "%s=%g" % (pattern, cell))]
        settings = {"missing_inputs": [path for path in previous if not Path(path).is_file()]}
        if simplify:
            settings["simplify"] = simplify
        self.step("scene", inputs, settings,
                  ["usd_scene.py"], [model, p["stage"]],
                  lambda: self.usd_python("scene", "usd_scene.py", [
                      "extract", "--scene", self.manifest["scene"], "--out-scene", model,
                      "--out-stage", p["stage"]] + simplify_args))
        self.scene = map_scene.parse(model)

    def legacy_scene(self):
        """`legacy-scene` step: the compiled map's world as an authored scene."""
        p = self.paths
        runtime = Path(self.tools["runtime"])
        runtime = runtime if runtime.is_absolute() else ROOT / runtime
        # The game content the materials come from (VPK directories).
        content = sorted(runtime.resolve().glob("*/*_dir.vpk"))
        self.step("legacy-scene", [self.legacy] + content, {"runtime": str(runtime)},
                  ["legacy_bsp_scene.py", "legacy_bsp.py", "vtf_decode.py", "source_content.py",
                   "bsp2_reader.py"], [p["legacy_scene"]],
                  lambda: self.usd_python("legacy-scene", "legacy_bsp_scene.py", [
                      "--bsp", self.legacy, "--runtime", runtime, "--map-name", self.map,
                      "--out", p["legacy_scene"]]))

    def build(self):
        self.out.mkdir(parents=True, exist_ok=True)
        p = self.paths
        if self.legacy:
            self.legacy_scene()
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
        reference = self.reference_render
        stage_args = ["--scene", scene, "--stage", p["stage"], "--receipt",
                      p["stage_receipt"]] + env_args
        if reference:
            stage_args += ["--render", p["reference"], "--samples",
                           str(reference.get("samples", 64)),
                           "--scale", str(reference.get("scale", 1.0)),
                           "--device", reference.get("device", cycles_device.CHECK_DEVICE)]
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
        probe = self.probe
        probe_width = probe.get("width", 512) if probe else 0
        bake_args = ["--reserve-rows", str(probe_width // 2),
                     "--scene", scene, "--stage", p["stage"], "--out-stage", p["lighting_stage"],
                     "--out-exr", p["atlas"], "--out-coverage-exr", p["coverage"],
                     "--size", str(self.lightmap["size"]),
                     "--samples", str(self.lightmap["samples"]),
                     "--device", self.lightmap["device"],
                     "--light-paths", self.lightmap["light_paths"]] + env_args
        for material in self.lightmap["exclude_materials"]:
            bake_args += ["--exclude-material", material]
        directional = self.lightmap["directional"]
        if directional:
            bake_args += ["--directional-dir", p["directional_bakes"]]
        layers = self.lightmap["layers"]
        if layers:
            bake_args += ["--layers", ",".join(layers), "--layers-dir", p["layers"]]
        self.step("bake", [p["stage"]] + ([environment] if environment else []),
                  dict({k: self.lightmap[k] for k in ("size", "samples", "exclude_materials",
                                                      "device", "directional", "light_paths",
                                                      "layers")},
                       reserve_rows=probe_width // 2),
                  SCENE_SCRIPTS + ["pbrt_blender.py", "pbrt_lightmap_bake.py"],
                  [p["lighting_stage"], p["atlas"], p["coverage"], p["atlas_receipt"]] +
                  ([p["directional_bakes"]] if directional else []) +
                  ([p["layers"]] if layers else []),
                  lambda: self.blender("bake", "pbrt_lightmap_bake.py", bake_args))
        atlas, atlas_receipt, scope = p["atlas"], p["atlas_receipt"], BAKE_SCOPE
        denoised_layers = {role: p["layers"] / (role + "-denoised.exr") for role in layers}

        def denoise():
            skip = [] if self.lightmap["denoise"] else ["--skip-denoise"]
            seconds = self.run("denoise", [sys.executable, HERE / "lightmap_denoise.py",
                                           "--exr", p["atlas"], "--bake-evidence",
                                           p["atlas_receipt"], "--coverage-exr",
                                           p["coverage"], "--out", p["denoised"]] + skip)
            for role, out in denoised_layers.items():
                seconds += self.run("denoise", [sys.executable, HERE / "lightmap_denoise.py",
                                                "--exr", p["layers"] / (role + ".exr"),
                                                "--layer", role, "--bake-evidence",
                                                p["atlas_receipt"], "--coverage-exr",
                                                p["coverage"], "--out", out] + skip)
            return seconds
        self.step("denoise", [p["atlas"], p["coverage"], p["atlas_receipt"]] +
                  [p["layers"] / (role + ".exr") for role in layers],
                  {"uv_coverage": True, "denoise": self.lightmap["denoise"], "layers": layers},
                  ["lightmap_denoise.py"], [p["denoised"], p["denoised_receipt"]] +
                  [path for out in denoised_layers.values()
                   for path in (out, out.with_name(out.name + ".json"))],
                  denoise)
        atlas, atlas_receipt = p["denoised"], p["denoised_receipt"]
        scope = BAKE_SCOPE + ("-denoised" if self.lightmap["denoise"] else "-gutter-filled")
        directional_args = []
        if directional:
            bakes = [p["directional_bakes"] / name for name in
                     ("rnm0.exr", "rnm1.exr", "rnm2.exr", "frame_t.exr", "frame_n.exr")]
            self.step("directional", [atlas, atlas_receipt, p["atlas_receipt"], p["coverage"]] +
                      bakes, {"denoise": self.lightmap["denoise"]},
                      ["lightmap_directional.py", "lightmap_denoise.py"],
                      [p["directional"], p["directional"].with_name(p["directional"].name +
                                                                    ".json")],
                      lambda: self.run("directional", [
                          sys.executable, HERE / "lightmap_directional.py",
                          "--flat-exr", atlas, "--flat-evidence", atlas_receipt,
                          "--bake-evidence", p["atlas_receipt"], "--directional-dir",
                          p["directional_bakes"], "--coverage-exr", p["coverage"],
                          "--out", p["directional"]] +
                          ([] if self.lightmap["denoise"] else ["--skip-denoise"])))
            directional_args = ["--directional-exr", p["directional"]]
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
        volume = self.probe_volume
        if volume:
            volume_args = ["--scene", scene, "--stage", p["stage"],
                           "--spacing", str(volume["spacing_m"]),
                           "--samples", str(volume.get("samples", 4096)),
                           "--device", self.lightmap["device"],
                           "--light-paths", self.lightmap["light_paths"],
                           "--out", p["prbv"], "--work", p["prbv_work"]] + env_args
            if volume.get("bounds_m"):
                volume_args += ["--bounds"] + [str(value) for value in volume["bounds_m"]]
            self.step("probe-volume", [p["stage"]] + ([environment] if environment else []),
                      dict(volume, light_paths=self.lightmap["light_paths"]),
                      SCENE_SCRIPTS + ["probe_volume_bake.py", "probe_volume.py",
                                       "pbrt_blender.py"],
                      [p["prbv"], p["prbv_work"]],
                      lambda: self.blender("probe-volume", "probe_volume_bake.py", volume_args))
        radiosity = self.radiosity
        if radiosity:
            radiosity_args = ["--scene", scene, "--stage", p["stage"], "--prbv", p["prbv"],
                              "--patch-size", str(radiosity["patch_size_m"]),
                              "--transfer-rays", str(radiosity.get("transfer_rays", 256)),
                              "--gather-rays", str(radiosity.get("gather_rays", 4096)),
                              "--samples", str(radiosity.get("samples", 1024)),
                              "--device", self.lightmap["device"],
                              "--light-paths", self.lightmap["light_paths"],
                              "--out", p["rtrn"], "--work", p["rtrn_work"]] + env_args
            self.step("radiosity", [p["stage"], p["prbv"]] + ([environment] if environment
                                                               else []),
                      dict(radiosity, light_paths=self.lightmap["light_paths"]),
                      SCENE_SCRIPTS + ["radiosity_transfer_bake.py", "radiosity_transfer.py",
                                       "probe_volume.py", "pbrt_blender.py"],
                      [p["rtrn"], p["rtrn_work"]],
                      lambda: self.blender("radiosity", "radiosity_transfer_bake.py",
                                           radiosity_args))
        field = self.sdf_volume
        if field:
            field_args = ["--scene", scene, "--stage", p["stage"],
                          "--voxel", str(field["voxel_m"]),
                          "--transfer-receipt", p["rtrn_work"] / "rtrn-bake.json",
                          "--out", p["sdfv"], "--work", p["sdfv_work"]] + env_args
            self.step("sdf", [p["stage"], p["rtrn"]] + ([environment] if environment else []),
                      dict(field),
                      SCENE_SCRIPTS + ["sdf_volume_bake.py", "sdf_volume.py",
                                       "radiosity_transfer_bake.py", "pbrt_blender.py"],
                      [p["sdfv"], p["sdfv_work"]],
                      lambda: self.blender("sdf", "sdf_volume_bake.py", field_args))
        # A scene sun: baked visibility + marker texels for dynamic specular.
        sun_args = []
        if self.scene.get("distant_lights") and probe:
            sun_args = ["--sun-visibility", p["sun_visibility"], "--coverage-exr", p["coverage"],
                        "--sun-bake-evidence", p["atlas_receipt"]]
        layer_args = [item for role, out in denoised_layers.items()
                      for item in ("--layer", "%s=%s" % (role, out))]
        self.step("ktx2", [atlas, atlas_receipt, p["lighting_stage"]] +
                  list(denoised_layers.values()) +
                  ([p["sun_visibility"], p["coverage"]] if sun_args else []) +
                  ([p["probe"] / "probe.json"] if probe else []) +
                  ([p["directional"]] if directional else []),
                  {"preview_gain": self.lightmap["preview_gain"], "scope": scope,
                   "probe_width": probe_width},
                  ["lightmap_ktx2.py", "reflection_probe.py"], [p["ktx2"]],
                  lambda: self.run("ktx2", [sys.executable, HERE / "lightmap_ktx2.py",
                                            "--exr", atlas, "--bake-evidence", atlas_receipt,
                                            "--lighting-stage", p["lighting_stage"],
                                            "--ktx-tool", self.tools["ktx"],
                                            "--preview-gain", str(self.lightmap["preview_gain"]),
                                            "--expected-scope", scope, "--out", p["ktx2"]] +
                                           probe_args + directional_args + sun_args + layer_args))
        pack_stage = p["lighting_stage"]
        # A relit map keeps its own skybox; no sky dome joins its world mesh.
        if environment and not self.legacy:
            pack_stage = p["render_stage"]
            self.step("sky", [p["lighting_stage"], p["sky_texture"]], {},
                      ["pbrt_sky_dome.py"],
                      [p["render_stage"], p["render_stage"].with_name(p["render_stage"].name + ".json")],
                      lambda: self.usd_python("sky", "pbrt_sky_dome.py", [
                          "--stage", p["lighting_stage"], "--sky-texture", p["sky_texture"],
                          "--out-stage", p["render_stage"]]))
        controls = self.light_controls() if radiosity else []
        if self.legacy:
            self.state.pop("collision", None)
            self.state.pop("compile", None)
            p["bsp"] = Path(self.legacy)
        else:
            self.collision_and_compile(scene, volume, controls)
        pack_bsp = p["bsp_ambient"] if volume else p["bsp"]
        scene_receipt = p["legacy_scene"] / "scene-receipt.json"

        def pack():
            seconds = 0.0
            if controls:
                if self.legacy:
                    check_legacy_light_styles(p["bsp"], controls)
                else:
                    check_light_styles(p["bsp"], controls)
            if volume:
                seconds += self.run("pack", [sys.executable, HERE / "leaf_ambient_from_prbv.py",
                                             "--bsp", p["bsp"], "--prbv", p["prbv"],
                                             "--out", p["bsp_ambient"], "--receipt",
                                             p["bsp_ambient"].with_suffix(".json")])
            worldlights = p["bsp_ambient"].with_name(p["bsp_ambient"].stem + "_worldlights.json")
            if self.legacy:
                # vrad's lights are in the bake now; only the lights it left
                # out (named lights that start dark) stay world lights.
                kept = json.loads(scene_receipt.read_text())["kept_world_light_styles"]
                seconds += self.run("pack", [sys.executable, HERE / "bsp_worldlights.py",
                                             "--bsp", pack_bsp, "--out", pack_bsp,
                                             "--keep-only", "--receipt", worldlights] +
                                    [item for style in kept for item in ("--style", str(style))])
            elif controls:
                # The switchable lights' zero-light vrad world lights: the
                # transfer owns their light.
                seconds += self.run("pack", [sys.executable, HERE / "bsp_worldlights.py",
                                             "--bsp", p["bsp_ambient"], "--out",
                                             p["bsp_ambient"], "--receipt", worldlights] +
                                    [item for control in controls
                                     for item in ("--style", str(control["style"]))])
            return seconds + self.usd_python("pack", "usd_worldmesh_pack.py", pack_args)
        hidden = {shape["name"] for shape in self.scene["shapes"]
                  if shape["material"] in self.hidden_materials}
        pack_args = ([
                      "--stage", pack_stage, "--bsp", pack_bsp, "--material-prefix",
                      self.map, "--require-lightmap-uv"] +
                      # A relit map's lights are invisible, as the entities were.
                      (["--include-emitters"] if self.scene["emitters"] and not self.legacy
                       else []) + [
                      "--lightmap-ktx2", p["ktx2"], "--bsp2tool", self.tools["bsp2tool"],
                      "--out", p["wmsh"], "--out-bsp2", p["bsp2"]] +
                      (["--weld-distance-source-units",
                        str(self.world_mesh["weld_distance_source_units"])]
                       if self.world_mesh["weld_materials"] else []) +
                      [item for material in self.world_mesh["weld_materials"]
                       for item in ("--weld-material", material)] +
                      # Dynamic models are placed as entities, not world mesh.
                      [item for name in sorted(map_scene.prop_shape_names(self.scene) | hidden)
                       for item in ("--exclude-mesh", name)] +
                      (["--probe-volume", p["prbv"]] if volume else []) +
                      (["--radiosity-transfer", p["rtrn"]] if radiosity else []) +
                      (["--sdf-volume", p["sdfv"]] if self.sdf_volume else []))
        self.step("pack", [pack_stage, p["lighting_stage"], p["bsp"], p["ktx2"]] +
                  ([p["prbv"]] if volume else []) + ([p["rtrn"]] if radiosity else []) +
                  ([p["sdfv"]] if self.sdf_volume else []) +
                  ([scene_receipt] if self.legacy else []),
                  dict({"prefix": self.map, **self.world_mesh},
                       **({"probe_volume": True} if volume else {}),
                       **({"radiosity_transfer": True} if radiosity else {}),
                       **({"sdf_volume": True} if self.sdf_volume else {}),
                       **({"hidden_meshes": sorted(hidden)} if hidden else {}),
                       **({"legacy": True} if self.legacy else {})),
                  ["usd_worldmesh_pack.py", "worldmesh_seam_weld.py", "worldstage_mesh_pack.py"] +
                  (["leaf_ambient_from_prbv.py", "probe_volume.py"] if volume else []) +
                  (["bsp_worldlights.py"] if controls or self.legacy else []),
                  [p["wmsh"], p["wmsh"].with_name(p["wmsh"].name + ".json"), p["bsp2"]] +
                  ([p["bsp_ambient"], p["bsp_ambient"].with_suffix(".json")] if volume else []),
                  pack)
        self.finish(scene, environment, reference)

    def collision_and_compile(self, scene, volume, controls):
        """`collision` and `compile`: a collision/PVS BSP for a scene map."""
        p = self.paths
        collision = self.manifest.get("collision", {})
        collision_args = ["--scene", scene, "--stage", p["lighting_stage"], "--map-name",
                          self.map, "--out-dir", p["collision"]]
        if volume:
            collision_args.append("--no-fallback-light")
        for control in controls:
            collision_args += ["--light-control", control["name"]]
        for door in collision.get("doors", []):
            collision_args += ["--door", ",".join([door["name"]] + [
                "%g" % v for corner in door["bounds_m"] for v in corner])]
        for portal in collision.get("portals", []):
            collision_args += ["--portal", ",".join(
                "%g" % v for v in list(portal["center_m"]) + list(portal["normal"]) +
                [1 if portal.get("portal_two") else 0])]
        for flag, key in (("--envelope-mesh", "envelope_meshes"),
                          ("--solid-material", "solid_materials"),
                          ("--solid-mesh", "solid_meshes")):
            for value in collision.get(key, []):
                collision_args += [flag, value]
        self.step("collision", [scene, p["lighting_stage"]],
                  dict(collision, **({"fallback_light": False} if volume else {}),
                       **({"light_controls": [c["name"] for c in controls]} if controls
                          else {})),
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

    def finish(self, scene, environment, reference):
        """`content` and the boot, gate and audit steps of a packed map."""
        p = self.paths
        tools = Path(self.tools["compile_tools"])
        sky = environment and not self.legacy
        sky_args = ["--sky-texture", p["sky_texture"]] if sky else []
        self.step("content", [scene, p["stage_receipt"], p["bsp2"]] +
                  ([p["sky_texture"]] if sky else []), {},
                  SCENE_SCRIPTS + ["pbrt_playable_content.py", "vtf_content.py"],
                  [p["content"], p["content"].with_suffix(".json")],
                  lambda: self.run("content", [sys.executable, HERE / "pbrt_playable_content.py",
                                               "--scene", scene, "--stage-receipt",
                                               p["stage_receipt"], "--bsp2", p["bsp2"],
                                               "--vtex", tools / "vtex", "--map-name", self.map,
                                               "--runtime", self.tools["runtime"],
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
        runtime_gate = self.runtime_gate
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
        # A relit map keeps its compiled collision; its drop test is the game's.
        if self.boot and not self.legacy:
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
        if self.profile.get("audit"):
            receipts = [path for path in (
                p["atlas_receipt"], p["denoised_receipt"], p["ktx2"].with_name(
                    p["ktx2"].name + ".json"), p["content"].with_suffix(".json"),
                p["wmsh"].with_name(p["wmsh"].name + ".json"), p["runtime_gate"],
                p["directional"].with_name(p["directional"].name + ".json"))
                if path.is_file()]
            self.step("audit", receipts, {"profile": self.profile["name"], "boot": self.boot},
                      ["map_export_audit.py"], [p["audit"]],
                      lambda: self.run("audit", [
                          sys.executable, HERE / "map_export_audit.py", "--build", self.out,
                          "--profile", self.profile["path"], "--out", p["audit"]] +
                          (["--booted"] if self.boot else [])))
        summary = {"status": "gate-failed" if self.failed_gates else "pass",
                   "quality": self.profile["name"],
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
