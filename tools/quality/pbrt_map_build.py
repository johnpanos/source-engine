#!/usr/bin/env python3
"""The one lighting back end for every map, with the scene maps' own front end.

    python3 tools/quality/pbrt_map_build.py \\
        --manifest quality/fixtures/pbrt-maps/living-room.json \\
        --out quality-results/living-room-map [--boot]

Every map is lit here. The back end takes a compiled BSP and, optionally, an
authored visual scene; with no scene it derives one from the BSP's faces,
materials and lights. Front ends differ only in how they make the BSP: a VMF
(`vmf_map_build.py --lighting`, which Hammer's build runs), a generator such as
`portal2_gi_chamber.py`, a regular compile (`vrad_cycles.py`), a shipped map
(`legacy_bsp_relight.py`), a USD-native map (`usd_map_compile.py --lighting`),
or a PBRT/USD scene manifest, whose own front end (`collision`, `compile`)
compiles a collision/PVS BSP from the scene before any bake. The others enter
through `map_lighting.light`. The back end bakes only through the one baker
seam (`light_baker.py`), and its `identity` gate checks that the packed map
carries every gameplay lump of the front end's BSP byte for byte.

The manifest (`pbrt-map-manifest/v1`) holds only per-map decisions: map name,
lightmap settings, collision selection and optional material exclusions, and
one of three combinations:

    scene        a `.pbrt` file or an authored `.usd`/`.usda`/`.usdc` stage; this
                 pipeline's front end compiles the BSP
    bsp          a front end's compiled BSP (`legacy_bsp` is its old name); the
                 scene is derived from it (`legacy-scene`), with `legacy_game`, the
                 compile's game directory searched for materials first, and
                 `legacy_runtime`, the staged game runtime the materials come from
                 (for a Portal 2 map the portal2 profile's packaged runtime,
                 out/portal2-linux-native-vulkan/dev/runtime) in place of the toolchain's
    bsp + scene  a compiled BSP lit with an authored scene in the map's space

An optional `medium` (participating_medium.py: scattering_per_m,
absorption_per_m, anisotropy, bounds_m) is a homogeneous participating
medium the lightmap bake's light paths cross (`bake --medium`, recorded in
the bake receipt and the step's cache key). It is explicit and opt-in: a
manifest without it bakes exactly as before. Only the `bake` step takes it;
the probe, probe-volume, radiosity and SDF bakes do not, and they say so in
the build log.

Every later step reads the scene through `map_scene`. The toolchain file
(`pbrt-map-toolchain/v1`) holds machine paths; `pbrt_map_toolchain.py
provision` builds the pinned tools under build/toolchains/ and writes the
default one; `--check-toolchain` validates versions and capabilities and exits.

With a derived scene, the scene's light emitters stay invisible and there is
no sky dome or traversal gate. Wherever a probe volume carries the models'
light, `pack` keeps only the world lights the bake left to the engine: a
derived scene's named lights that start dark, and none for an authored scene
(its compiled light entities are the switchable sources' zero-light stand-ins,
whose light the radiosity transfer owns). Steps:

    legacy-scene (a bsp with no scene) legacy_bsp_scene.py: the BSP's world faces,
                 static-prop meshes, materials, occluders and lights as an authored
                 USD scene; mdl_mesh_export comes from the configured client build
    scene        (USD scenes) usd_scene.py extract: map-scene/v1 model + normalized stage
    environment  sky (PBRT equal-area map or USD DomeLight) -> Z-up equirect EXR +
                 display texture (if any sky)
    stage        PBRT -> USD stage in Blender (+ optional Cycles reference render)
    reference-gate  Cycles render vs the scene's reference image (manifest reference.gate)
    collision    (front end, scene maps) shell/solids/spawn VMF from the scene's stage, its
                 switchable sources (radiosity_transfer.switchable_sources) named `light`s
    compile      (front end) vbsp2 / vvis / vrad -> collision + PVS BSP; a manifest's
                 `bsp` replaces both
    layout       (lightmap.layout "planar", the default) lightmap_layout.py: exact UVs for flat
                 geometry and xatlas charts (within a verified stretch) for curved surfaces,
                 written into a copy of the stage; the bake uses them unchanged
    bake         shared lightmap UVs (from layout; Blender charting with "layout": "blender")
                 + Cycles diffuse irradiance atlas
    prop-points  (a legacy-scene map; lightmap.prop_vertex_light, default on)
                 prop_vertex_light.py points: a sample per static-prop vertex
    prop-vertices the baker's prop-vertices operation: each sample's light in the atlas's
                 unit; `pack` writes it as the props' sp_<n>.vhv colour meshes
    noise        (lightmap.noise_target) lightmap_noise.py: the bake's measured Monte Carlo
                 noise (every light page is the mean of two half-sample bakes) must be under
                 the target, or the step reports the sample count that would meet it; with
                 lightmap.denoise it judges what remains after the per-chart denoise;
                 lightmap.noise_gate "record" measures and records without failing
    denoise      OpenImageDenoise RTLightmap filter (manifest lightmap.denoise, default on)
    seams        lightmap_seams.py extract --check: the lighting stage's chart seams, and a
                 gate on its chart invariants (no overlap, bleed, escaped UVs or split
                 flat regions); ktx2 stitches every page across those seams
    probe        optional reflection probes (manifest reflection_probe): placed per room
                 and per glossy surface (pbrt_reflection_probe.py), six Cycles cube faces
                 each with the depth pass
    rprb         the probes' parallax boxes fitted to their depth, GGX roughness mips and
                 influence volumes, encoded as the RPRB lump (reflection_probe_set.py pack)
    probe-volume optional RFC 0011 PRBV (profile/manifest probe_volume): Cycles-baked
                 irradiance and ray-traced visibility per probe (probe_volume_bake.py);
                 the map then has no vrad fallback light, and its leaf ambient is
                 derived from the volume (leaf_ambient_from_prbv.py) in `pack`
    radiosity    optional RFC 0011 G4 RTRN (profile/manifest radiosity, needs probe_volume):
                 patches, form factors, per-light injection and the probe gather
                 (radiosity_transfer_bake.py); its switchable sources are the ones the
                 front end named, and the transfer is packed beside PRBV
    sdf          optional RFC 0011 G6 SDFV (profile/manifest sdf_volume, needs radiosity):
                 the static world's signed distance, reflectance and emission per voxel
                 and its analytic lights, styled as the transfer's sources
                 (sdf_volume_bake.py), light cells culled by the BSP's PVS; packed beside RTRN
    ktx2         atlas -> linear RGBA16F KTX2 (LMAP payload)
    sky          render stage = lighting stage + SkyDome for window views (scenes with a sky)
    pack         USD triangles -> WMSH + LMAP (+ PRBV, RTRN, SDFV, RPRB) inside BSP2
    identity     gameplay_identity.py: every legacy lump of the BSP carried byte for byte
                 except the world lights and leaf ambient
    content      VTF/VMT materials + maps/<map>.bsp content root
    boot         (with --boot) headless native Vulkan boot and screenshot at the spawn
    camera-boot  (with --boot) second boot with the camera at the PBRT reference eye
    runtime-gate camera-matched game frame vs the Cycles render (manifest runtime_gate)
    traversal-boot / traversal  drop the player onto the highest walkable tops and
                 the spawn floor; each must come to rest on its surface

Each step records the digests of its inputs, script and settings in
`<out>/steps.json`; unchanged steps are skipped, so editing collision does not
re-run the bake. `--from STEP` forces a step and everything after it; `--until
STEP` stops after it. A key covers every tools/quality module the step's
scripts import, the identity of the tools it runs (Blender and OCIO, OIDN,
OpenUSD and the compile-tool prefix, KTX) and, for steps that build the
scene's materials and lights, every scene source file. A running step's
previous outputs are set aside under `.previous/<step>` and put back when it
fails or is interrupted (the next build also restores them after a crash),
so a failed rebuild never leaves a step without its last good outputs.
A failing pixel gate stops the build unless `--keep-going` is given; then the
map is still finished, build.json reports `gate-failed` and the exit is nonzero.
A finished map is published to run/maps/<map> (`playable_maps.py`), so
`./kiln play portal <map>` loads it; `--no-publish` skips that.
"""

import argparse
import ast
import functools
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
import gameplay_identity  # noqa: E402
import light_baker  # noqa: E402
import participating_medium  # noqa: E402
import radiosity_transfer  # noqa: E402
import reference_compare  # noqa: E402
import remote_blender  # noqa: E402

STEPS = ("legacy-scene", "scene", "environment", "stage", "probe-placement", "reference-gate",
         "collision", "compile",
         "layout", "bake", "prop-points", "prop-vertices", "noise", "denoise", "directional", "directional-indirect", "seams",
         "light-masks", "probe", "rprb", "probe-volume", "radiosity", "sdf", "ktx2", "sky", "pack",
         "identity",
         "content", "boot", "camera-boot", "runtime-gate", "traversal-boot", "traversal", "audit")
BAKE_SCOPE = "pbrt-shared-lightmap-uv-and-cycles-bake"
# Quality gates report, they never stop a build or block publishing (user
# decision 2026-10-05): a failed gate is recorded in build.json's
# "gate_findings" and printed. Correctness steps (compile, identity, pack)
# still fail the build.
GATES = ("reference-gate", "noise", "seams", "runtime-gate", "traversal", "audit")
PROFILES = ROOT / "quality" / "map_export_profiles"
DEFAULT_QUALITY = "source2"
LEGACY_QUALITY = DEFAULT_QUALITY
QUALITY_FIELDS = ("lightmap", "reflection_probe", "probe_volume", "radiosity", "sdf_volume",
                  "reference", "runtime_gate", "audit", "world_mesh")
SCENE_SCRIPTS = ["pbrt_scene.py", "map_scene.py"]
PREVIOUS = ".previous"
BLENDER_TOOLS = ("blender", "ocio")
USD_TOOLS = ("openusd", "compile_tools")
# The tools each step runs (pbrt_map_toolchain.identity names); boot steps
# name their client build in their settings.
STEP_TOOLS = {"legacy-scene": USD_TOOLS, "scene": USD_TOOLS, "stage": BLENDER_TOOLS,
              "probe-placement": BLENDER_TOOLS,
              "layout": USD_TOOLS + ("xatlas",), "bake": BLENDER_TOOLS,
              "light-masks": BLENDER_TOOLS, "denoise": ("openimagedenoise",),
              "noise": ("openimagedenoise",),
              "directional": ("openimagedenoise",),
              "directional-indirect": ("openimagedenoise",), "probe": BLENDER_TOOLS,
              "probe-volume": BLENDER_TOOLS, "radiosity": BLENDER_TOOLS, "sdf": BLENDER_TOOLS,
              "seams": USD_TOOLS, "ktx2": ("ktx",), "sky": USD_TOOLS, "collision": USD_TOOLS,
              "compile": ("compile_tools",), "pack": USD_TOOLS, "content": ("compile_tools",)}


class StopAfter(Exception):
    """Raised after the `--until` step."""


@functools.lru_cache(maxsize=None)
def script_closure(scripts):
    """`scripts` and every tools/quality module they import, transitively."""
    pending, seen = list(scripts), set()
    while pending:
        name = pending.pop()
        if name in seen:
            continue
        seen.add(name)
        for node in ast.walk(ast.parse((HERE / name).read_text())):
            if isinstance(node, ast.Import):
                modules = [alias.name for alias in node.names]
            elif isinstance(node, ast.ImportFrom) and node.module and not node.level:
                modules = [node.module]
            else:
                continue
            pending += [module.split(".")[0] + ".py" for module in modules
                        if (HERE / (module.split(".")[0] + ".py")).is_file()]
    return tuple(sorted(seen))
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
    if "legacy_bsp" in manifest:  # the key's name before any BSP could be lit
        if "bsp" in manifest:
            raise ValueError("a manifest names one bsp (legacy_bsp is its old name)")
        manifest["bsp"] = manifest.pop("legacy_bsp")
    if "bsp" in manifest:
        manifest["bsp"] = str((ROOT / manifest["bsp"]).resolve())
        if not Path(manifest["bsp"]).is_file():
            raise ValueError("bsp does not exist: " + manifest["bsp"])
        if "collision" in manifest:
            raise ValueError("a manifest with a bsp was compiled by its front end: no collision")
        if "legacy_game" in manifest:
            manifest["legacy_game"] = str((ROOT / manifest["legacy_game"]).resolve())
            if not Path(manifest["legacy_game"]).is_dir():
                raise ValueError("legacy_game is not a directory: " + manifest["legacy_game"])
        if "legacy_runtime" in manifest:
            manifest["legacy_runtime"] = str((ROOT / manifest["legacy_runtime"]).resolve())
            if not Path(manifest["legacy_runtime"]).is_dir():
                raise ValueError("legacy_runtime is not a directory: " +
                                 manifest["legacy_runtime"])
        manifest.setdefault("quality", LEGACY_QUALITY)
        if not manifest.get("scene"):
            # No authored scene: the `legacy-scene` step derives one from the
            # BSP (Pipeline sets its path).
            manifest["scene"] = None
            manifest["scene_format"] = "usd"
            return manifest
    manifest["scene"] = str((ROOT / manifest["scene"]).resolve())
    manifest["scene_format"] = "usd" if map_scene.is_usd(manifest["scene"]) else "pbrt"
    manifest.setdefault("quality", DEFAULT_QUALITY)
    return manifest


# Published beside a map whose lightmap bake crossed a participating medium:
# the medium as the bake receipt records it.
MEDIUM_SIDECAR = "lightmap-medium.json"


def load_medium(manifest):
    """The manifest's participating medium (validated), or None."""
    medium = manifest.get("medium")
    return participating_medium.validate(medium) if medium is not None else None


def load_profile(name, preview=False):
    """A declared export-quality profile (quality/map_export_profiles/<name>.json).

    `preview` allows a declared preview rung (`PREVIEW_PROFILES`) as well as the
    production profile: the run then does every step at that profile's cost, and
    the pipeline records it as non-production. Nothing else may stand in for the
    production profile (user decision, 2026-10-02: one production map profile)."""
    path = PROFILES / (name + ".json")
    if not path.is_file():
        raise ValueError("unknown map export quality profile " + name)
    profile = json.loads(path.read_text())
    if profile.get("schema") != "map-export-profile/v1" or profile.get("name") != name:
        raise ValueError("invalid map export profile " + str(path))
    if name != DEFAULT_QUALITY and profile.get("purpose") != "fixture" and not preview:
        raise ValueError("the production map profile is source2; unsupported profile: " + name
                         + " (a preview run asks for --preview)")
    profile["path"] = str(path)
    return profile


# The three quality rungs, named after Valve's own (Source 2 level-design docs,
# Building Lighting), cheapest first:
#
#   preview        "Preview Baked Lighting": a low-quality look at the map while
#                  you work. Valve's does not bake lightmaps at all - it uses
#                  vertex lighting and many per-pixel dynamic lights, so there is
#                  no lightmap equivalent to point at; our cheapest rung is the
#                  smallest real bake (a 1024 atlas at 128 samples, no probe
#                  volume, no directional page, no reflection probes).
#   full-compile   "Full Compile" (F9's default): 1k lightmaps, "relatively fast
#                  ... a few minutes" on a simple map.
#   final-compile  "Final Compile" (the release option): 2k lightmaps, "a little
#                  while on a typical home PC". Our release rung is denser than
#                  Valve's 2k (4096, Vulkan's guaranteed max texture size; see
#                  quality/map_export_profiles/source2.json).
#
# Every rung but `final-compile` is a preview: it runs the same steps at that
# profile's cost and is recorded as non-production.
QUALITY_LADDER = {"preview": "preview",
                  "full-compile": "portal2-chamber-preview",
                  "final-compile": DEFAULT_QUALITY}
# The declared preview rung of the production profile, which `--preview` uses.
PREVIEW_PROFILES = {production: QUALITY_LADDER["full-compile"]
                    for production in QUALITY_LADDER.values() if production == DEFAULT_QUALITY}


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


def check_derived_light_styles(bsp, controls):
    """Each styled source of a derived scene is a style of the map's own lights
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
    """Resolve authored data while keeping production quality owned by the profile."""
    default = profile.get(key)
    if key not in manifest:
        return default
    value = manifest[key]
    if profile.get("purpose") != "fixture":
        # Authored capture locations remain content; bake and acceptance policy
        # has exactly one owner. A matching CLI device is harmless, but cannot
        # introduce a CPU/auto production variant.
        authored = {"lightmap": {"exclude_materials"},
                    "reflection_probe": {"positions", "volumes"},
                    "world_mesh": {"exclude_materials", "weld_materials",
                                   "weld_distance_source_units"}}.get(key, set())
        if not isinstance(value, dict) or not isinstance(default, dict):
            if value != default:
                raise ValueError("%s is owned by the source2 profile" % key)
            return default
        for field, setting in value.items():
            if field not in authored and setting != default.get(field):
                raise ValueError("%s.%s is owned by the source2 profile" % (key, field))
    if isinstance(value, dict) and isinstance(default, dict):
        return dict(default, **value)
    return value or None


def applicable_exclusions(excluded, authored, scene_materials):
    """The lightmap exclusions to bake with. A manifest's own must be in the
    scene (the bake rejects a typo); a profile's apply to the scenes that
    have them (a relit map with no nodraw brush sides has no occluder)."""
    return [name for name in excluded if name in authored or name in scene_materials]


# Cycles logs every sample batch, so a Blender step silent this long is stuck
# (a HIP queue lost to a GPU fault waits forever); scene import and CPU
# tracing between bakes stay well under it.
BLENDER_SILENCE = 30 * 60
# The same bound for every other step, so no step can wait forever: a USD
# import, a host tool or a Python step that stops printing is stopped and
# reported rather than hanging the build. It is the default, not a per-call
# argument, so a new step is bounded without having to remember.
DEFAULT_STEP_SILENCE = 10 * 60


def run_logged(command, handle, env=None, on_line=None, silence=None):
    """Run `command` (in its own process group), writing its output to
    `handle` and passing each line to `on_line`. Returns (exit status, None),
    or (None, seconds) when it printed nothing for `silence` seconds and its
    whole process group was killed. `silence` defaults to
    DEFAULT_STEP_SILENCE: a step that stops talking is stopped, so no step can
    hang the build. Pass `silence=0` to wait for a command that is meant to be
    silent until it exits."""
    import queue
    import signal
    import threading
    process = subprocess.Popen([str(part) for part in command], stdout=subprocess.PIPE,
                               stderr=subprocess.STDOUT, cwd=ROOT, text=True, errors="replace",
                               bufsize=1, env=dict(os.environ, **(env or {})),
                               start_new_session=True)
    silence = DEFAULT_STEP_SILENCE if silence is None else silence
    lines = queue.Queue()

    def pump():
        for line in process.stdout:
            lines.put(line)
        lines.put(None)
    reader = threading.Thread(target=pump, daemon=True)
    reader.start()
    while True:
        try:
            line = lines.get(timeout=silence or None)
        except queue.Empty:
            os.killpg(process.pid, signal.SIGKILL)
            process.wait()
            handle.write("[stopped: no output for %d s]\n" % silence)
            handle.flush()
            return None, silence
        if line is None:
            break
        handle.write(line)
        if on_line:
            on_line(line)
    reader.join()
    return process.wait(), None


class Pipeline:
    def __init__(self, manifest, toolchain, out, force_from, boot, keep_going=False,
                 publish=True, until=None):
        # A preview run does every step at its profile's preview cost and says so
        # in its manifest and summary; it is not a production export.
        self.preview = bool(manifest.get("preview"))
        self.profile = load_profile(manifest["quality"], self.preview)
        if self.preview and self.profile["name"] == DEFAULT_QUALITY:
            raise ValueError("--preview runs the declared preview rung of %s (%s), not %s itself"
                             % (DEFAULT_QUALITY, PREVIEW_PROFILES[DEFAULT_QUALITY], DEFAULT_QUALITY))
        if publish and self.profile.get("purpose") == "fixture":
            raise ValueError("a fixture profile cannot publish production content")
        for key in QUALITY_FIELDS:
            with_defaults(manifest, self.profile, key)
        self.profile_hash = sha256(self.profile["path"])
        self.manifest = manifest
        self.tools = toolchain
        self.out = out.resolve()
        self.until = until
        self.digests = {}
        self.identities = {}
        self.map = manifest["map"]
        self.usd = manifest["scene_format"] == "usd"
        # The back end lights a compiled BSP: a front end's (`bsp`), or the
        # one this pipeline's front end compiles from the scene (`collision`,
        # `compile`). Its scene is authored (`scene`) or derived from the BSP.
        self.bsp_input = manifest.get("bsp")
        self.derived = manifest.get("scene") is None
        if self.derived:
            manifest["scene"] = str(out.resolve() / "legacy-scene" / "scene.usda")
        # A USD scene is read through the model its `scene` step extracts.
        self.scene_file = (self.out / "scene" / "scene.json") if self.usd else manifest["scene"]
        self.scene = None if self.usd else map_scene.parse(self.scene_file)
        # Cycles steps on another host's GPU (toolchain `remote_blender`).
        self.remote = remote_blender.from_toolchain(toolchain)
        # Every light-transport product comes from this one baker (R48-BAKER seam).
        self.baker = light_baker.CyclesBaker(self.blender)
        self.current = (None, ())
        self.state_path = self.out / "steps.json"
        self.state = json.loads(self.state_path.read_text()) if self.state_path.is_file() else {}
        self.force_from = STEPS.index(force_from) if force_from else len(STEPS)
        # A crash or kill mid-step left the step's last good outputs aside.
        previous = self.out / PREVIOUS
        for name in sorted(p.name for p in previous.iterdir()) if previous.is_dir() else []:
            self.restore(name)
        self.boot = boot
        self.keep_going = keep_going
        self.publish = publish
        self.failed_gates = []
        # One production policy; private fixtures cannot publish a map.
        lightmap = with_defaults(manifest, self.profile, "lightmap")
        self.lightmap = {"size": lightmap.get("size", 2048),
                         "samples": lightmap.get("samples", 64),
                         "preview_gain": lightmap.get("preview_gain", 1.0),
                         "denoise": lightmap.get("denoise", True),
                         "directional": lightmap.get("directional", False),
                         # Samples of the directional page's RNM-basis bakes
                         # (None: the atlas's). They feed only its denoised
                         # gradient relative to the flat atlas.
                         "directional_samples": lightmap.get("directional_samples"),
                         "device": lightmap.get("device", cycles_device.BAKE_DEVICE),
                         "light_paths": lightmap.get("light_paths", "blender-default"),
                         # RFC 0011 separated light: LMAP v2 layers beside the total.
                         "layers": list(lightmap.get("layers", [])),
                         # The direct layer's samples (None: a quarter of the
                         # atlas's, at least 64): under runtime direct light
                         # (RFC 0016) the core draws the runtime lights' direct
                         # light itself; the layer serves the total page, the
                         # fallback, and converges faster than the bounces.
                         "direct_samples": lightmap.get("direct_samples"),
                         # Cycles seed of the bake and probe (pbrt_blender.pin_sampling).
                         "seed": lightmap.get("seed", 0),
                         "exclude_materials": lightmap.get("exclude_materials", []),
                         # "planar" (default): lightmap_layout.py, exact planar
                         # charts plus xatlas for curved surfaces; "blender":
                         # Blender's smart-project charting, kept for comparison.
                         "layout": lightmap.get("layout", "planar"),
                         # Largest relative bake noise allowed (lightmap_noise.py), or None.
                         "noise_target": lightmap.get("noise_target"),
                         # "enforce" fails the build above the target; "record"
                         # measures and records the noise and the samples that
                         # would meet it.
                         "noise_gate": lightmap.get("noise_gate", "enforce"),
                         # Stitched seam gate {"p99", "max"} (relative) in place
                         # of lightmap_ktx2.py's defaults, or None for those.
                         "seam_gate": lightmap.get("seam_gate")}
        if self.lightmap["noise_gate"] not in ("enforce", "record"):
            raise SystemExit("lightmap.noise_gate must be \"enforce\" or \"record\"")
        seam_gate = self.lightmap["seam_gate"]
        if seam_gate is not None and (
                not isinstance(seam_gate, dict) or not seam_gate or
                set(seam_gate) - {"p99", "max"} or
                not all(isinstance(v, (int, float)) and v > 0 for v in seam_gate.values())):
            raise SystemExit("lightmap.seam_gate must be {\"p99\": x, \"max\": y} with "
                             "positive limits (either may be left out)")
        # The lightmap bake's participating medium (opt-in; see the docstring).
        self.medium = load_medium(manifest)
        if self.lightmap["layout"] not in ("blender", "planar"):
            raise ValueError("unknown lightmap layout " + str(self.lightmap["layout"]))
        self.authored_exclusions = set(
            (manifest.get("lightmap") or {}).get("exclude_materials", []))
        self.probe = with_defaults(manifest, self.profile, "reflection_probe")
        self.probe_volume = with_defaults(manifest, self.profile, "probe_volume")
        self.radiosity = with_defaults(manifest, self.profile, "radiosity")
        if self.radiosity and not self.probe_volume:
            raise ValueError("radiosity needs the probe_volume it gathers into")
        self.sdf_volume = with_defaults(manifest, self.profile, "sdf_volume")
        if self.derived and not self.probe_volume:
            raise ValueError("a relit map needs a probe_volume: its world lights move there")
        if self.sdf_volume and not self.radiosity:
            raise ValueError("sdf_volume needs the radiosity transfer its light styles follow")
        reference = with_defaults(manifest, self.profile, "reference") or {}
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
            "identity": self.out / "gameplay-identity.json",
            "environment": self.out / "environment.exr",
            "stage": self.out / "stage" / (self.map + ".usdc"),
            "stage_receipt": self.out / "stage.json",
            "layout_stage": self.out / "stage" / (self.map + "_layout.usdc"),
            "layout_receipt": self.out / "stage" / "layout.json",
            "noise_pair": self.out / "lighting" / "noise-pair",
            "noise_receipt": self.out / "lighting" / "noise.json",
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
            "light_masks": self.out / "lighting" / "light_masks.exr",
            "light_mask_ids": self.out / "lighting" / "light_masks-ids.exr",
            "lsmk": self.out / "lighting" / "light_masks.lsmk",
            "directional": self.out / "lighting" / "atlas-directional.exr",
            "directional_indirect": self.out / "lighting" / "indirect-directional.exr",
            "audit": self.out / "audit.json",
            "ktx2": self.out / "lighting" / "atlas.ktx2",
            "lmap": self.out / "lighting" / "atlas.lmap",
            "seams": self.out / "lighting" / "seams.npz",
            "probe": self.out / "lighting" / "probe",
            "probe_placement": self.out / "lighting" / "probe-placement.json",
            "rprb": self.out / "lighting" / "reflection_probes.rprb",
            "prbv": self.out / "lighting" / "probe_volume.prbv",
            "prbv_work": self.out / "lighting" / "probe_volume",
            "rtrn": self.out / "lighting" / "radiosity.rtrn",
            "rtrn_work": self.out / "lighting" / "radiosity",
            "sdfv": self.out / "lighting" / "field.sdfv",
            "sdfv_work": self.out / "lighting" / "sdf",
            "bsp_ambient": self.out / (self.map + "_leaf_ambient.bsp"),
            "prop_points": self.out / "lighting" / "prop_vertices.npz",
            "prop_light": self.out / "lighting" / "prop_vertex_light.npy",
            "prop_stage": self.out / "lighting" / "prop_vertices_stage.usda",
            "prop_scratch_exr": self.out / "lighting" / "prop_vertices_unused.exr",
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

    def run(self, step, command, env=None, progress=None, silence=None):
        """Run a step's command, its output going to logs/<step>.log; the
        lines `progress.feed` returns for its output are shown as they come.
        With `silence` (seconds), a command that prints nothing for that long
        is killed and the step fails (run_logged). A Python tool step the
        toolchain's remote_blender block takes (remote_blender.TOOL_STEPS)
        runs on that host, as the Blender steps do."""
        if (self.remote and step in remote_blender.TOOL_STEPS and self.remote.applies(step)
                and str(command[0]) == sys.executable):
            print("[%s] on %s" % (step, self.remote.host), flush=True)
            self.remote.push([a for a in command[1:] if str(a).startswith("/")], self.out)
            seconds = self.run_command(step, self.remote.tool_command(command[1:], env or {}),
                                       None, progress, silence)
            self.remote.pull(self.out)
            return seconds
        return self.run_command(step, command, env, progress, silence)

    def run_command(self, step, command, env=None, progress=None, silence=None):
        self.logs.mkdir(parents=True, exist_ok=True)
        log = self.logs / (step + ".log")
        started = time.monotonic()
        with log.open("a") as handle:
            handle.write("$ " + " ".join(map(str, command)) + "\n")
            handle.flush()
            returncode, quiet = run_logged(
                command, handle, env,
                (lambda line: [print("[%s] %s" % (step, message), flush=True)
                               for message in progress.feed(line)]) if progress else None,
                silence)
        if quiet is not None:
            raise SystemExit("step %s printed nothing for %d minutes and was stopped (a stalled "
                             "GPU does this: check the kernel log for amdgpu faults); log %s"
                             % (step, quiet // 60, log))
        if returncode:
            tail = log.read_text().splitlines()[-15:]
            raise SystemExit("step %s failed (exit %d); log %s:\n  %s" %
                             (step, returncode, log, "\n  ".join(tail)))
        return time.monotonic() - started

    def light_controls(self):
        """The radiosity transfer's switchable sources, in style order."""
        receipt = self.paths["rtrn_work"] / "rtrn-bake.json"
        sources = json.loads(receipt.read_text())["sources"]
        return [source for source in sources if source["style"] >= 0]

    def blender(self, step, script, arguments):
        # Cycles' debug log reports each bake's tiles and sample batches;
        # bake_progress turns it (and the script's PROGRESS lines) into progress.
        options = ["-b", "--factory-startup", "--log-level", "debug", "--log", "cycles",
                   "--python-exit-code", "9", "--python", HERE / script, "--"] + arguments
        # BLAS on one thread: a script's forked workers (probe tracing) inherit
        # no OpenMP pool, so an OpenMP BLAS call in them waits forever on the
        # parent's barrier. Cycles and OIDN use TBB, not OpenMP.
        env = {"OCIO": self.tools["ocio"], "OMP_NUM_THREADS": "1", "OPENBLAS_NUM_THREADS": "1"}
        if self.remote and self.remote.applies(step):
            # remote_blender.py: mirror the inputs to the host, run there, pull
            # the build directory back.
            inputs = self.current[1] if self.current[0] == step else ()
            paths = list(inputs) + [a for a in arguments if str(a).startswith("/")]
            # The scene's layers and textures, which may live outside the build
            # directory (an extracted USD model stands for them in cache keys).
            scene = self.scene
            if scene is None and Path(self.scene_file).is_file():
                scene = map_scene.parse(self.scene_file)
            if scene is not None:
                paths += map_scene.input_files(scene)
            print("[%s] on %s" % (step, self.remote.host), flush=True)
            self.remote.push(paths, self.out)
            seconds = self.run(step, self.remote.command(options, env),
                               progress=bake_progress.CyclesProgress(), silence=BLENDER_SILENCE)
            self.remote.pull(self.out)
            return seconds
        return self.run(step, [self.tools["blender"]] + options, env=env,
                        progress=bake_progress.CyclesProgress(), silence=BLENDER_SILENCE)

    def usd_python(self, step, script, arguments):
        return self.run(step, [self.tools["usd_python"], HERE / script] + arguments,
                        env={"PYTHONPATH": self.tools["usd_pythonpath"],
                             "PXR_PLUGINPATH_NAME": str(Path(self.tools["compile_tools"]) /
                                                        "share/sourceWorld")})

    def file_sha256(self, path):
        """sha256 of a file, computed once per build for unchanged files."""
        status = os.stat(path)
        memo = (str(Path(path).resolve()), status.st_mtime_ns, status.st_size)
        if memo not in self.digests:
            self.digests[memo] = sha256(path)
        return self.digests[memo]

    def identity(self, name):
        if name not in self.identities:
            self.identities[name] = pbrt_map_toolchain.identity(self.tools, name)
        return self.identities[name]

    def scene_sources(self):
        """Every scene file the material- and light-building steps read."""
        return map_scene.source_files(self.scene)

    def step_key(self, name, inputs, settings, scripts):
        return digest({"inputs": {str(p): self.file_sha256(p) if Path(p).is_file() else None
                                  for p in inputs},
                       "settings": settings,
                       "scripts": {s: self.file_sha256(HERE / s)
                                   for s in script_closure(tuple(scripts))},
                       "tools": {t: self.step_tool_identity(name, t)
                                 for t in STEP_TOOLS.get(name, ())}})

    def step_tool_identity(self, step, tool):
        if tool == "blender" and self.remote and self.remote.applies(step):
            return dict(self.remote.cache_identity(), remote=True)
        if self.remote and step in remote_blender.TOOL_STEPS and self.remote.applies(step):
            remote = self.remote.tool_identity(tool)
            if remote is None:
                raise SystemExit("remote_blender runs %s but names no identity for its %s "
                                 "(the block's `tools`)" % (step, tool))
            return remote
        return self.identity(tool)

    def set_aside(self, name, outputs):
        """Move a step's existing outputs to .previous/<name> before it runs."""
        backup = self.out / PREVIOUS / name
        shutil.rmtree(backup, ignore_errors=True)
        backup.mkdir(parents=True)
        moved = {}
        for index, path in enumerate(map(Path, outputs)):
            if path.exists() or path.is_symlink():
                shutil.move(str(path), str(backup / str(index)))
                moved[str(index)] = str(path)
        (backup / "outputs.json").write_text(json.dumps(moved, indent=2, sort_keys=True) + "\n")

    def restore(self, name):
        """Put a step's set-aside outputs back, replacing any partial new ones."""
        backup = self.out / PREVIOUS / name
        manifest = backup / "outputs.json"
        moved = json.loads(manifest.read_text()) if manifest.is_file() else {}
        for index, original in sorted(moved.items()):
            original = Path(original)
            if original.is_dir() and not original.is_symlink():
                shutil.rmtree(original)
            elif original.exists() or original.is_symlink():
                original.unlink()
            original.parent.mkdir(parents=True, exist_ok=True)
            shutil.move(str(backup / index), str(original))
        if moved:
            print("[%s] restored the previous outputs" % name, flush=True)
        shutil.rmtree(backup, ignore_errors=True)

    def step(self, name, inputs, settings, scripts, outputs, action):
        """Run `action` unless its inputs, settings, scripts and tools are unchanged."""
        key = self.step_key(name, inputs, settings, scripts)
        index = STEPS.index(name)
        previous = self.state.get(name, {})
        # Keys hash every input file, so a rebuilt upstream output invalidates
        # exactly the steps that read it; only --from forces a cascade.
        fresh = (previous.get("key") == key and index < self.force_from and
                 all(Path(p).exists() for p in outputs))
        if fresh:
            print("[%s] up to date" % name)
            self.stop_after(name)
            return
        self.set_aside(name, outputs)
        (self.logs / (name + ".log")).unlink(missing_ok=True)
        print("[%s] running..." % name, flush=True)
        self.current = (name, tuple(inputs))
        try:
            seconds = action()
            missing = [str(p) for p in outputs if not Path(p).exists()]
            if missing:
                raise SystemExit("step %s did not produce %s" % (name, ", ".join(missing)))
        except SystemExit as failure:
            if name not in GATES:
                self.restore(name)
                raise
            # A failed gate's own verdict replaces the old one; it is not
            # recorded in steps.json, so the gate runs again next build.
            shutil.rmtree(self.out / PREVIOUS / name, ignore_errors=True)
            print("[%s] GATE FINDING (reported, not fatal): %s" % (name, failure), flush=True)
            self.failed_gates.append(name)
            self.stop_after(name)
            return
        except BaseException:
            self.restore(name)
            raise
        shutil.rmtree(self.out / PREVIOUS / name)
        self.state[name] = {"key": key, "seconds": round(seconds or 0, 1),
                            "outputs": {str(Path(p).relative_to(self.out)):
                                        sha256(p) if Path(p).is_file() else "directory"
                                        for p in outputs}}
        self.state_path.write_text(json.dumps(self.state, indent=2, sort_keys=True) + "\n")
        print("[%s] done in %.1fs" % (name, seconds or 0), flush=True)
        self.stop_after(name)

    def stop_after(self, name):
        if name == self.until:
            raise StopAfter(name)

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

    def boot_target(self):
        """portal_boot.py's target: the toolchain's kiln client profile (RFC 0027)."""
        return ["--profile", self.tools["client_profile"], "--flavor", self.tools["client_flavor"]]

    def boot_key(self):
        """The boot steps' cache key for that target."""
        return {"client": [self.tools["client_profile"], self.tools["client_flavor"]],
                "runtime": self.tools["runtime"]}

    def legacy_scene(self):
        """`legacy-scene` step: the compiled map's world as an authored scene."""
        p = self.paths
        runtime = Path(self.manifest.get("legacy_runtime") or self.tools["runtime"])
        runtime = runtime if runtime.is_absolute() else ROOT / runtime
        # The game content the materials come from (VPK directories), after
        # the compile's own game directory (vrad's -game) when it has one.
        content = sorted(runtime.resolve().glob("*/*_dir.vpk"))
        model_tool = Path(self.tools["model_tool"])
        settings = {"runtime": str(runtime), "model_tool": str(model_tool)}
        game_args = []
        texture_decoder = self.tools.get("texture_decoder")
        if texture_decoder:
            content.append(Path(texture_decoder))
            game_args += ["--texture-decoder", texture_decoder]
        if self.manifest.get("material_overrides"):
            import source_content
            override_path = (ROOT / self.manifest["material_overrides"]).resolve()
            overrides = source_content.material_overrides(override_path)
            if not texture_decoder or not Path(texture_decoder).is_file():
                raise ValueError("material overrides require the built native texture_decoder")
            content.append(override_path)
            for archive in overrides["archives"]:
                archive_path = Path(archive["path"])
                content.extend(sorted(archive_path.parent.glob(archive_path.name.replace("_dir.vpk", "*.vpk"))))
            settings["material_overrides"] = overrides
            game_args += ["--material-overrides", str(override_path)]
        game = self.manifest.get("legacy_game")
        if game:
            content += sorted(f for directory in ("materials", "models")
                              for f in (Path(game) / directory).rglob("*") if f.is_file())
            settings["game"] = game
            game_args += ["--game-dir", game]
        self.step("legacy-scene", [self.bsp_input, model_tool] + content, settings,
                  ["legacy_bsp_scene.py", "legacy_bsp.py", "vtf_decode.py", "source_content.py",
                   "bsp2_reader.py"], [p["legacy_scene"]],
                  lambda: self.usd_python("legacy-scene", "legacy_bsp_scene.py", [
                      "--bsp", self.bsp_input, "--runtime", runtime, "--map-name", self.map,
                      "--out", p["legacy_scene"], "--model-tool", model_tool] + game_args))
        # Static models take part in transport and probes but have no world
        # atlas or WMSH surface: the game already draws them as instances.
        # The static props' per-vertex light samples (prop_vertex_light.py):
        # baked after the atlas, written as colour meshes when the map packs.
        self.prop_points_args = ["--bsp", self.bsp_input, "--runtime", runtime,
                                 "--model-tool", model_tool] + \
            (["--game-dir", game] if game else [])
        self.prop_points_inputs = [self.bsp_input, model_tool] + content
        receipt = json.loads((p["legacy_scene"] / "scene-receipt.json").read_text())
        prop_materials = receipt["static_props"]["materials"]
        self.lightmap["exclude_materials"] += prop_materials
        self.hidden_materials += prop_materials
        transport_materials = receipt.get("static_transport", {}).get("materials", [])
        self.lightmap["exclude_materials"] += transport_materials
        self.hidden_materials += transport_materials

    def probe_arguments(self, scene, env_args):
        p, probe = self.paths, self.probe
        args = ["--scene", scene, "--stage", p["stage"],
                "--placement-cache", p["probe_placement"],
                "--face-size", str(probe.get("face_size", 256)),
                "--samples", str(probe.get("samples", 512)),
                "--device", self.lightmap["device"], "--seed", str(self.lightmap["seed"]),
                "--placement", json.dumps(probe.get("placement", {}), sort_keys=True),
                "--coverage-rules", json.dumps(self.profile.get("audit") or {}, sort_keys=True),
                "--volumes", json.dumps(probe.get("volumes", []), sort_keys=True),
                "--light-paths", probe.get("light_paths", "blender-default"),
                "--denoise" if probe.get("denoise", True) else "--no-denoise"] + env_args
        if probe.get("relight"):
            args.append("--gbuffer")
        seeds = probe.get("positions", []) + ([probe["position"]] if probe.get("position") else [])
        for position in seeds:
            args += ["--position"] + [str(value) for value in position]
        # Reflection placement and PRBV must see the same authored playable
        # envelope. Otherwise placement can choose exterior captures whose
        # fitted boxes can never satisfy the shipped parallax contract.
        bounds = probe.get("bounds_m") or self.probe_volume_bounds()
        if bounds:
            args += ["--bounds"] + [str(value) for value in bounds]
        return args

    def probe_volume_bounds(self):
        """Return the authored playable envelope for the fixed-resolution PRBV.

        Reflection-volume proxy boxes already own the map's playable spatial
        extent. Reusing their union keeps a two-metre Source2 probe volume out
        of imported sky/exterior geometry without adding a second bounds list
        that map authors could let drift. An explicit PRBV bounds_m remains an
        authored override for maps without reflection volumes.
        """
        explicit = (self.probe_volume or {}).get("bounds_m")
        if explicit:
            return explicit
        volumes = self.probe.get("volumes", []) if self.probe else []
        if not volumes:
            return None
        try:
            lows = [[float(value) for value in volume["box_min"]] for volume in volumes]
            highs = [[float(value) for value in volume["box_max"]] for volume in volumes]
        except (KeyError, TypeError, ValueError) as error:
            raise ValueError("reflection probe volumes need box_min and box_max for PRBV bounds") from error
        if any(len(value) != 3 for value in lows + highs):
            raise ValueError("reflection probe volume bounds need three coordinates")
        low = [min(value[axis] for value in lows) for axis in range(3)]
        high = [max(value[axis] for value in highs) for axis in range(3)]
        if any(high[axis] <= low[axis] for axis in range(3)):
            raise ValueError("reflection probe volume bounds are empty")
        return low + high

    def build(self):
        self.out.mkdir(parents=True, exist_ok=True)
        p = self.paths
        if self.derived:
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
        self.step("stage", [scene] + self.scene_sources() +
                  ([environment] if environment else []) + ([p["stage"]] if self.usd else []),
                  {"reference": reference}, SCENE_SCRIPTS + ["pbrt_blender.py",
                                                             "pbrt_usd_stage.py"],
                  ([] if self.usd else [p["stage"]]) + [p["stage_receipt"]] +
                  ([p["reference"]] if reference else []),
                  lambda: self.blender("stage", "pbrt_usd_stage.py", stage_args))
        if self.probe:
            # Placement depends on geometry/materials, not baked irradiance.
            # Use this same immutable stage for preflight and face capture so
            # capture consumes the validated placement cache after the bake.
            probe_args = self.probe_arguments(scene, env_args)
            probe_bounds = self.probe.get("bounds_m") or self.probe_volume_bounds()
            placement_dir = self.out / "placement"
            self.step("probe-placement", [p["stage"]] + self.scene_sources() +
                      ([environment] if environment else []),
                      {"probe": self.probe, "bounds_m": probe_bounds,
                       "rules": self.profile.get("audit") or {}, "coverage": "reported"},
                      SCENE_SCRIPTS + self.baker.scripts("probe"),
                      [p["probe_placement"], placement_dir],
                      lambda: self.baker.place(probe_args + ["--out-dir", placement_dir,
                                                             "--record-coverage-failure"]))
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
        # Front end: a scene map compiles its own collision/PVS BSP from the
        # scene (its switchable sources named, so vbsp styles them); any other
        # map arrives compiled. Everything after this lights that BSP.
        if self.bsp_input:
            self.state.pop("collision", None)
            self.state.pop("compile", None)
            p["bsp"] = Path(self.bsp_input)
        else:
            self.collision_and_compile(scene, self.probe_volume,
                                       radiosity_transfer.switchable_sources(self.scene)
                                       if self.radiosity else [])
        self.lightmap["exclude_materials"] = applicable_exclusions(
            self.lightmap["exclude_materials"], self.authored_exclusions,
            self.scene["materials"])
        probe = self.probe
        bake_stage = p["stage"]
        if self.lightmap["layout"] == "planar":
            _, unbaked = map_scene.lightmap_exclusions(self.scene,
                                                       self.lightmap["exclude_materials"])
            layout_settings = {"size": self.lightmap["size"], "unbaked": sorted(unbaked)}
            self.step("layout", [p["stage"]] + self.scene_sources(), layout_settings,
                      ["lightmap_layout.py"], [p["layout_stage"], p["layout_receipt"]],
                      lambda: self.usd_python("layout", "lightmap_layout.py", [
                          "author", "--stage", p["stage"], "--out", p["layout_stage"],
                          "--size", str(self.lightmap["size"]),
                          "--xatlas", self.tools["xatlas"],
                          "--receipt", p["layout_receipt"]] +
                          [item for name in sorted(unbaked) for item in ("--exclude-mesh", name)]))
            bake_stage = p["layout_stage"]
        bake_args = ["--layout", "authored" if bake_stage != p["stage"] else "blender",
                     "--scene", scene, "--stage", bake_stage, "--out-stage", p["lighting_stage"],
                     "--out-exr", p["atlas"], "--out-coverage-exr", p["coverage"],
                     "--size", str(self.lightmap["size"]),
                     "--samples", str(self.lightmap["samples"]),
                     "--device", self.lightmap["device"], "--seed", str(self.lightmap["seed"]),
                     "--light-paths", self.lightmap["light_paths"]] + env_args
        for material in self.lightmap["exclude_materials"]:
            bake_args += ["--exclude-material", material]
        directional = self.lightmap["directional"]
        if directional:
            bake_args += ["--directional-dir", p["directional_bakes"]]
            if self.lightmap["directional_samples"]:
                bake_args += ["--directional-samples", str(self.lightmap["directional_samples"])]
        layers = self.lightmap["layers"]
        if layers:
            bake_args += ["--layers", ",".join(layers), "--layers-dir", p["layers"]]
        direct_samples = None
        if "direct" in layers:
            direct_samples = self.lightmap["direct_samples"] or \
                max(64, self.lightmap["samples"] // 4)
            bake_args += ["--direct-samples", str(direct_samples)]
        noise_target = self.lightmap["noise_target"]
        if noise_target:
            bake_args += ["--noise-pair-dir", p["noise_pair"]]
        bake_settings = {k: self.lightmap[k] for k in ("size", "samples", "exclude_materials",
                                                       "device", "directional", "light_paths",
                                                       "layers", "seed", "layout",
                                                       "noise_target")}
        if self.lightmap["directional_samples"]:
            bake_settings["directional_samples"] = self.lightmap["directional_samples"]
        if direct_samples:
            bake_settings["direct_samples"] = direct_samples
        if self.medium:
            # Only a map with a medium names one, so every other map's cache
            # key is unchanged.
            bake_args += ["--medium", json.dumps(self.medium, sort_keys=True)]
            bake_settings["medium"] = self.medium
            print("[bake] participating medium %s: scattering %g/m, absorption %g/m, g %g "
                  "(the lightmap bake only; probe, probe-volume, radiosity and SDF bakes take "
                  "no medium)" % (self.medium.get("name", "(unnamed)"),
                                  self.medium["scattering_per_m"],
                                  self.medium["absorption_per_m"], self.medium["anisotropy"]))
        self.step("bake", [bake_stage] + self.scene_sources() +
                  ([environment] if environment else []),
                  bake_settings,
                  SCENE_SCRIPTS + self.baker.scripts("bake"),
                  [p["lighting_stage"], p["atlas"], p["coverage"], p["atlas_receipt"]] +
                  ([p["directional_bakes"]] if directional else []) +
                  ([p["layers"]] if layers else []) +
                  ([p["noise_pair"]] if noise_target else []),
                  lambda: self.baker.bake("bake", bake_args))
        if getattr(self, "prop_points_args", None) and self.lightmap.get("prop_vertex_light",
                                                                          True):
            # Static props' baked per-vertex light (Source 2's static-prop
            # lighting; prop_vertex_light.py): the samples, then their light,
            # baked in the atlas's scene and unit. Its own stage and atlas
            # paths, so the lightmap bake's outputs are untouched.
            self.step("prop-points", self.prop_points_inputs, {},
                      ["prop_vertex_light.py", "legacy_bsp_scene.py", "legacy_bsp.py",
                       "source_content.py"], [p["prop_points"]],
                      lambda: self.run("prop-points", [
                          sys.executable, HERE / "prop_vertex_light.py", "points"] +
                          self.prop_points_args + ["--out", p["prop_points"]]))
            prop_args = []
            skip = {"--out-stage": p["prop_stage"], "--out-exr": p["prop_scratch_exr"]}
            dropped = {"--noise-pair-dir", "--out-coverage-exr", "--directional-dir"}
            arguments = iter(bake_args)
            for item in arguments:
                if item in dropped:
                    next(arguments)
                    continue
                prop_args.append(item)
                if item in skip:
                    next(arguments)
                    prop_args.append(skip[item])
            prop_args += ["--prop-vertices", p["prop_points"], "--prop-vertex-light",
                          p["prop_light"], "--prop-vertices-only"]
            self.step("prop-vertices", [bake_stage, p["prop_points"]] + self.scene_sources() +
                      ([environment] if environment else []),
                      dict(bake_settings, prop_vertices=True),
                      SCENE_SCRIPTS + self.baker.scripts("prop-vertices"),
                      [p["prop_light"], p["prop_light"].with_name(p["prop_light"].name + ".json")],
                      lambda: self.baker.bake("prop-vertices", prop_args))
        if noise_target:
            halves = [p["noise_pair"] / "total-a.exr", p["noise_pair"] / "total-b.exr"]
            mean_samples = 2 * max(1, (self.lightmap["samples"] + 1) // 2)
            denoised = ["--after-denoise"] if self.lightmap["denoise"] else []
            noise_settings = {"target": noise_target, "samples": mean_samples,
                              "after_denoise": bool(denoised)}
            record = []
            if self.lightmap["noise_gate"] == "record":
                # Only a recording map names it, so enforcing maps keep their keys.
                noise_settings["gate"] = "record"
                record = ["--record-only"]
            self.step("noise", halves + [p["coverage"]], noise_settings, ["lightmap_noise.py"],
                      [p["noise_receipt"]],
                      lambda: self.run("noise", [sys.executable, HERE / "lightmap_noise.py",
                                                 "--first", halves[0], "--second", halves[1],
                                                 "--coverage", p["coverage"],
                                                 "--samples", str(mean_samples),
                                                 "--target", str(noise_target),
                                                 "--out", p["noise_receipt"]] + denoised +
                                                record))
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
        frames = [p["directional_bakes"] / name for name in ("frame_t.exr", "frame_n.exr")]
        if directional and "indirect" in denoised_layers:
            # The indirect layer's own gradient (its RNM bakes), for the
            # core's runtime direct light; the total page stays flat.
            indirect = denoised_layers["indirect"]
            bakes = [p["directional_bakes"] / ("rnm_indirect%d.exr" % i) for i in range(3)]
            out = p["directional_indirect"]
            self.step("directional-indirect",
                      [indirect, indirect.with_name(indirect.name + ".json"),
                       p["atlas_receipt"], p["coverage"]] + bakes + frames,
                      {"denoise": self.lightmap["denoise"], "layer": "indirect"},
                      ["lightmap_directional.py", "lightmap_denoise.py"],
                      [out, out.with_name(out.name + ".json")],
                      lambda: self.run("directional", [
                          sys.executable, HERE / "lightmap_directional.py",
                          "--layer", "indirect", "--flat-exr", indirect,
                          "--flat-evidence", indirect.with_name(indirect.name + ".json"),
                          "--bake-evidence", p["atlas_receipt"], "--directional-dir",
                          p["directional_bakes"], "--coverage-exr", p["coverage"],
                          "--out", out] +
                          ([] if self.lightmap["denoise"] else ["--skip-denoise"])))
            directional_args = ["--layer-directional", "indirect=%s" % out]
        elif directional:
            bakes = [p["directional_bakes"] / ("rnm%d.exr" % i) for i in range(3)] + frames
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
        # Chart seams of the lighting stage, gated on the chart invariants.
        self.step("seams", [p["lighting_stage"]], {"size": self.lightmap["size"]},
                  ["lightmap_seams.py"], [p["seams"], p["seams"].with_suffix(".json")],
                  lambda: self.usd_python("seams", "lightmap_seams.py", [
                      "extract", "--check", "--stage", p["lighting_stage"],
                      "--size", str(self.lightmap["size"]), "--out", p["seams"]]))
        # The radiance seam invariant needs the baked pages, but no secondary
        # lighting product. Run it before probes and volumes so a bad chart
        # seam cannot consume a remote render lease on unrelated work.
        sun_args = []
        sun_inputs = []
        raw_receipt = p["atlas"].with_name(p["atlas"].name + ".json")
        if p["sun_visibility"].is_file() and raw_receipt.is_file() and \
                json.loads(raw_receipt.read_text()).get("sun"):
            sun_args = ["--sun-visibility", p["sun_visibility"],
                        "--sun-bake-evidence", raw_receipt]
            sun_inputs = [p["sun_visibility"], raw_receipt]
        # Static lights' baked shadow masks (LSMK) where the runtime draws
        # direct light (an indirect layer): their static shadows leave PCSS.
        mask_args = []
        if self.lightmap.get("light_masks", "indirect" in self.lightmap["layers"]):
            mask_settings = {k: self.lightmap[k] for k in ("size", "device", "seed",
                                                           "exclude_materials")}
            self.step("light-masks", [p["lighting_stage"]] + self.scene_sources(), mask_settings,
                      SCENE_SCRIPTS + self.baker.scripts("light-masks"),
                      [p["light_masks"], p["light_mask_ids"], p["light_masks"].with_name(
                          p["light_masks"].name + ".json")],
                      lambda: self.baker.bake("light-masks", [
                          "--scene", scene, "--stage", p["lighting_stage"],
                          "--out-exr", p["light_masks"],
                          "--size", str(self.lightmap["size"]),
                          "--device", self.lightmap["device"],
                          "--seed", str(self.lightmap["seed"])] +
                          [item for material in self.lightmap["exclude_materials"]
                           for item in ("--exclude-material", material)]))
            masks_receipt = json.loads(p["light_masks"].with_name(
                p["light_masks"].name + ".json").read_text())
            if masks_receipt.get("status") == "pass":
                mask_args = ["--light-masks", p["light_masks"], "--lsmk-out", p["lsmk"]]
                sun_inputs = sun_inputs + [p["light_masks"], p["light_mask_ids"]]
        layer_args = [item for role, out in denoised_layers.items()
                      for item in ("--layer", "%s=%s" % (role, out))]
        seam_args = ["--seams", p["seams"], "--buried-exr", p["atlas"], "--coverage-exr",
                     p["coverage"]]
        ktx2_settings = {"preview_gain": self.lightmap["preview_gain"], "scope": scope}
        seam_gate = self.lightmap["seam_gate"]
        if seam_gate:
            ktx2_settings["seam_gate"] = dict(seam_gate)
            if "p99" in seam_gate:
                seam_args += ["--max-seam-p99", str(seam_gate["p99"])]
            if "max" in seam_gate:
                seam_args += ["--max-seam", str(seam_gate["max"])]
        self.step("ktx2", [atlas, atlas_receipt, p["lighting_stage"], p["seams"],
                           p["coverage"], p["atlas"]] +
                  list(denoised_layers.values()) +
                  ([p["directional_indirect"]] if directional and "indirect" in denoised_layers
                   else [p["directional"]] if directional else []) + sun_inputs,
                  ktx2_settings,
                  ["lightmap_ktx2.py", "world_lightmap_v3.py", "light_shadow_masks.py",
                   "../texture/bc_codec.py"],
                  [p["ktx2"], p["lmap"]] + ([p["lsmk"]] if mask_args else []),
                  lambda: self.run("ktx2", [sys.executable, HERE / "lightmap_ktx2.py",
                                            "--exr", atlas, "--bake-evidence", atlas_receipt,
                                            "--lighting-stage", p["lighting_stage"],
                                            "--ktx-tool", self.tools["ktx"],
                                            "--preview-gain", str(self.lightmap["preview_gain"]),
                                            "--expected-scope", scope, "--out", p["ktx2"],
                                            "--lmap-out", p["lmap"],
                                            "--record-seam-failure"] +
                                           directional_args + layer_args + seam_args + sun_args +
                                           mask_args))
        if probe:
            # Reflection probes (R50-PARALLAX): placed per room and per glossy
            # surface, each rendered with its depth pass, then fitted to a
            # parallax box, prefiltered and encoded as the RPRB lump.
            coverage_rules = self.profile.get("audit") or {}
            face_args = self.probe_arguments(scene, env_args) + ["--out-dir", p["probe"]]
            face_args.append("--record-coverage-failure")  # reported, not fatal
            self.step("probe", [p["stage"], p["probe_placement"]] + self.scene_sources() +
                      ([environment] if environment else []),
                      dict(probe, device=self.lightmap["device"], seed=self.lightmap["seed"],
                           denoise=probe.get("denoise", True),
                           coverage_rules=coverage_rules, record_coverage_failure=True),
                      SCENE_SCRIPTS + self.baker.scripts("probe"),
                      [p["probe"]],
                      lambda: self.baker.bake("probe", face_args))
            # The candidate grid covers the playable envelope, as the PRBV
            # does: a probe box fitted past the world must not stretch it.
            envelope = self.probe_volume_bounds()
            self.step("rprb", [p["probe"] / "probes.json"],
                      {"cube_size": probe.get("cube_size", probe.get("face_size", 256)),
                       "preview_gain": self.lightmap["preview_gain"],
                       "max_mean_relative_residual": coverage_rules.get(
                           "max_reflection_probe_residual"),
                       "candidate_bounds_m": envelope},
                      ["reflection_probe_set.py", "reflection_probe.py", "gi_reference.py",
                       "../texture/bc_codec.py"],
                      [p["rprb"], p["rprb"].with_name(p["rprb"].name + ".json")],
                      lambda: self.run("rprb", [sys.executable, HERE / "reflection_probe_set.py",
                                                "pack", "--probes-dir", p["probe"],
                                                "--ktx-tool", self.tools["ktx"],
                                                "--face", str(probe.get("cube_size", probe.get("face_size", 256))),
                                                "--preview-gain",
                                                str(self.lightmap["preview_gain"]),
                                                "--out", p["rprb"]] +
                                               (["--candidate-bounds-m"] +
                                                [str(float(v)) for v in envelope]
                                                if envelope else [])))
        volume = self.probe_volume
        if volume:
            volume_bounds = self.probe_volume_bounds()
            volume_args = ["--scene", scene, "--stage", p["stage"],
                           "--spacing", str(volume["spacing_m"]),
                           "--samples", str(volume.get("samples", 4096)),
                           "--device", self.lightmap["device"],
                           "--light-paths", self.lightmap["light_paths"],
                           "--out", p["prbv"], "--work", p["prbv_work"]] + env_args
            if volume_bounds:
                volume_args += ["--bounds"] + [str(value) for value in volume_bounds]
            if volume.get("max_probes"):
                volume_args += ["--max-probes", str(volume["max_probes"])]
            if volume.get("fit_limit"):
                volume_args += ["--fit-limit"]
            self.step("probe-volume", [p["stage"]] + self.scene_sources() +
                      ([environment] if environment else []),
                      dict(volume, bounds_m=volume_bounds, light_paths=self.lightmap["light_paths"]),
                      SCENE_SCRIPTS + self.baker.scripts("probe-volume"),
                      [p["prbv"], p["prbv_work"]],
                      lambda: self.baker.bake("probe-volume", volume_args))
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
            if radiosity.get("fit_limit") and radiosity.get("max_patches"):
                radiosity_args += ["--max-patches", str(radiosity["max_patches"])]
            self.step("radiosity", [p["stage"], p["prbv"]] + self.scene_sources() +
                      ([environment] if environment else []),
                      dict(radiosity, light_paths=self.lightmap["light_paths"]),
                      SCENE_SCRIPTS + self.baker.scripts("radiosity"),
                      [p["rtrn"], p["rtrn_work"]],
                      lambda: self.baker.bake("radiosity", radiosity_args))
        field = self.sdf_volume
        if field:
            field_args = ["--scene", scene, "--stage", p["stage"],
                          "--voxel", str(field["voxel_m"]),
                          "--transfer-receipt", p["rtrn_work"] / "rtrn-bake.json",
                          "--out", p["sdfv"], "--work", p["sdfv_work"]] + env_args
            # Light cells (profile sdf_volume.light_cell_m / light_cutoff),
            # culled by the map's PVS too.
            if "light_cell_m" in field:
                field_args += ["--light-cell", str(field["light_cell_m"])]
            if "light_cutoff" in field:
                field_args += ["--light-cutoff", str(field["light_cutoff"])]
            field_args += ["--bsp", p["bsp"]]
            self.step("sdf", [p["stage"], p["rtrn"], p["bsp"]] + self.scene_sources() +
                      ([environment] if environment else []),
                      dict(field),
                      SCENE_SCRIPTS + self.baker.scripts("sdf"),
                      [p["sdfv"], p["sdfv_work"]],
                      lambda: self.baker.bake("sdf", field_args))
        pack_stage = p["lighting_stage"]
        # A relit map keeps its own skybox; no sky dome joins its world mesh.
        if environment and not self.derived:
            pack_stage = p["render_stage"]
            self.step("sky", [p["lighting_stage"], p["sky_texture"]], {},
                      ["pbrt_sky_dome.py"],
                      [p["render_stage"], p["render_stage"].with_name(p["render_stage"].name + ".json")],
                      lambda: self.usd_python("sky", "pbrt_sky_dome.py", [
                          "--stage", p["lighting_stage"], "--sky-texture", p["sky_texture"],
                          "--out-stage", p["render_stage"]]))
        # The transfer's switchable sources as the bake numbered them.
        controls = self.light_controls() if radiosity else []
        pack_bsp = p["bsp_ambient"] if volume else p["bsp"]
        scene_receipt = p["legacy_scene"] / "scene-receipt.json"

        def pack():
            seconds = 0.0
            if controls:
                (check_derived_light_styles if self.derived else check_light_styles)(
                    p["bsp"], controls)
            if volume:
                seconds += self.run("pack", [sys.executable, HERE / "leaf_ambient_from_prbv.py",
                                             "--bsp", p["bsp"], "--prbv", p["prbv"],
                                             "--out", p["bsp_ambient"], "--receipt",
                                             p["bsp_ambient"].with_suffix(".json")])
            worldlights = p["bsp_ambient"].with_name(p["bsp_ambient"].stem + "_worldlights.json")
            if volume:
                # The bake owns every light it baked, and the probe volume
                # carries the models' share: only the lights the bake left to
                # the engine stay world lights. A derived scene leaves its
                # named lights that start dark; an authored scene none (its
                # compiled light entities are the switchable sources' zero-light
                # stand-ins, whose light the transfer owns).
                kept = (json.loads(scene_receipt.read_text())["kept_world_light_styles"]
                        if self.derived else [])
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
            if p["prop_light"].is_file():
                # The props' baked per-vertex light replaces vrad's colour
                # meshes in the pak (prop_vertex_light.py write).
                seconds += self.run("pack", [sys.executable, HERE / "prop_vertex_light.py",
                                             "write", "--bsp", pack_bsp, "--points",
                                             p["prop_points"], "--light", p["prop_light"],
                                             "--out", pack_bsp, "--receipt",
                                             pack_bsp.with_name(pack_bsp.stem +
                                                                "_prop_vertex_light.json")])
            return seconds + self.usd_python("pack", "usd_worldmesh_pack.py", pack_args)
        hidden = {shape["name"] for shape in self.scene["shapes"]
                  if shape["material"] in self.hidden_materials}
        pack_args = ([
                      "--stage", pack_stage, "--bsp", pack_bsp, "--material-prefix",
                      self.map, "--require-lightmap-uv"] +
                      # A derived scene's lights are invisible, as the entities were.
                      (["--include-emitters"] if self.scene["emitters"] and not self.derived
                       else []) + [
                      "--lightmap-ktx2", p["lmap"], "--bsp2tool", self.tools["bsp2tool"],
                      "--out", p["wmsh"], "--out-bsp2", p["bsp2"]] +
                      (["--weld-distance-source-units",
                        str(self.world_mesh["weld_distance_source_units"])]
                       if self.world_mesh["weld_materials"] else []) +
                      [item for material in self.world_mesh["weld_materials"]
                       for item in ("--weld-material", material)] +
                      # Dynamic models are placed as entities, not world mesh.
                      [item for name in sorted(map_scene.nonstatic_shape_names(self.scene) | hidden)
                       for item in ("--exclude-mesh", name)] +
                      (["--probe-volume", p["prbv"]] if volume else []) +
                      (["--radiosity-transfer", p["rtrn"]] if radiosity else []) +
                      (["--sdf-volume", p["sdfv"]] if self.sdf_volume else []) +
                      (["--reflection-probes", p["rprb"]] if probe else []) +
                      (["--light-masks", p["lsmk"]] if p["lsmk"].is_file() else []))
        self.step("pack", [pack_stage, p["lighting_stage"], p["bsp"], p["lmap"]] +
                  ([p["lsmk"]] if p["lsmk"].is_file() else []) +
                  ([p["prbv"]] if volume else []) + ([p["rtrn"]] if radiosity else []) +
                  ([p["sdfv"]] if self.sdf_volume else []) +
                  ([p["rprb"]] if probe else []) +
                  ([scene_receipt] if self.derived else []) +
                  ([p["prop_light"], p["prop_points"]] if p["prop_light"].is_file() else []),
                  dict({"prefix": self.map, **self.world_mesh},
                       **({"probe_volume": True} if volume else {}),
                       **({"radiosity_transfer": True} if radiosity else {}),
                       **({"sdf_volume": True} if self.sdf_volume else {}),
                       **({"reflection_probes": True} if probe else {}),
                       **({"hidden_meshes": sorted(hidden)} if hidden else {}),
                       **({"derived_scene": True} if self.derived else {})),
                  ["usd_worldmesh_pack.py", "worldmesh_seam_weld.py", "worldstage_mesh_pack.py"] +
                  (["leaf_ambient_from_prbv.py", "probe_volume.py"] if volume else []) +
                  (["bsp_worldlights.py"] if controls or volume else []) +
                  (["prop_vertex_light.py"] if p["prop_light"].is_file() else []),
                  [p["wmsh"], p["wmsh"].with_name(p["wmsh"].name + ".json"), p["bsp2"]] +
                  ([p["bsp_ambient"], p["bsp_ambient"].with_suffix(".json")] if volume else []),
                  pack)
        # Lighting changes only lighting: the packed map carries every
        # gameplay lump of the front end's BSP byte for byte.
        self.step("identity", [p["bsp"], p["bsp2"]],
                  {"relit_lumps": sorted(gameplay_identity.RELIT_LUMPS)},
                  ["gameplay_identity.py", "bsp2_reader.py"], [p["identity"]],
                  lambda: self.run("identity", [sys.executable, HERE / "gameplay_identity.py",
                                                "--bsp", p["bsp"], "--bsp2", p["bsp2"],
                                                "--out", p["identity"]]))
        self.finish(scene, environment, reference)

    def collision_and_compile(self, scene, volume, controls):
        """`collision` and `compile`: a collision/PVS BSP for a scene map."""
        p = self.paths
        collision = self.manifest.get("collision", {})
        collision_args = ["--scene", scene, "--stage", p["stage"], "--map-name",
                          self.map, "--out-dir", p["collision"]]
        if volume:
            collision_args.append("--no-fallback-light")
        for control in controls:
            collision_args += ["--light-control", control["name"]]
        for door in collision.get("doors", []):
            collision_args += ["--door", ",".join([door["name"]] + [
                "%g" % v for corner in door["bounds_m"] for v in corner] +
                ([door["material"]] if door.get("material") else []))]
        for lamp in collision.get("lamps", []):
            collision_args += ["--lamp", ",".join([lamp["name"]] + [
                "%g" % v for v in list(lamp["anchor_m"]) +
                [lamp["length_m"], lamp["release_degrees"], lamp["color_linear"]] +
                list(lamp.get("holds_degrees", []))])]
        for bulb in collision.get("dynamic_lights", []):
            collision_args += ["--dynamic-light", ",".join([bulb["name"]] + [
                "%g" % v for v in list(bulb["position_m"]) + [bulb["color_linear"]]])]
        for orbit in collision.get("orbits", []):
            collision_args += ["--orbit", ",".join([orbit["name"]] + [
                "%g" % v for v in list(orbit["center_m"]) + [orbit["degrees_per_second"]] +
                list(orbit.get("holds_degrees", []))])]
            for light in orbit["lights"]:
                collision_args += ["--orbit-light", ",".join([orbit["name"], light["name"]] + [
                    "%g" % v for v in list(light["position_m"]) + list(light["color_linear"])])]
        for portal in collision.get("portals", []):
            collision_args += ["--portal", ",".join(
                ["%g" % v for v in list(portal["center_m"]) + list(portal["normal"]) +
                 [1 if portal.get("portal_two") else 0]] +
                ([portal["name"]] if portal.get("name") else []))]
        for cycle in collision.get("portal_cycles", []):
            collision_args += ["--portal-cycle", ",".join(
                [cycle["name"], cycle["portal"], "%g" % cycle["open_s"],
                 "%g" % cycle["closed_s"]])]
            for stop in cycle["stops"]:
                collision_args += ["--portal-stop", ",".join(
                    [cycle["name"]] + ["%g" % v for v in list(stop["center_m"]) +
                                       list(stop["normal"])])]
        for flag, key in (("--envelope-mesh", "envelope_meshes"),
                          ("--solid-material", "solid_materials"),
                          ("--solid-mesh", "solid_meshes")):
            for value in collision.get(key, []):
                collision_args += [flag, value]
        self.step("collision", [scene, p["stage"]],
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
        sky = environment and not self.derived
        sky_args = ["--sky-texture", p["sky_texture"]] if sky else []
        self.step("content", [scene, p["stage_receipt"], p["bsp2"]] + self.scene_sources() +
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
            self.step("boot", content_files, self.boot_key(),
                      ["portal_boot.py"], [p["boot"]],
                      lambda: self.run("boot", [sys.executable, HERE / "portal_boot.py",
                                                *self.boot_target(),
                                                "--content-root", p["content"],
                                                "--renderer", "native-vulkan", "--headless",
                                                "--map", self.map, "--console-command",
                                                "r_worldmesh_draw 2", "--out", p["boot"]]))
        runtime_gate = self.runtime_gate
        if self.boot and reference:
            commands, _ = reference_compare.camera_commands(self.scene)
            boot_args = [*self.boot_target(), "--content-root", p["content"],
                         "--renderer", "native-vulkan", "--headless", "--map", self.map,
                         "--out", p["camera_boot"]]
            for command in commands:
                boot_args += ["--console-command", command]
            content_files = sorted(f for f in p["content"].rglob("*") if f.is_file())
            self.step("camera-boot", content_files,
                      dict(self.boot_key(), commands=commands),
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
        # The drop test reads the collision receipt a scene map's front end wrote.
        if self.boot and not self.bsp_input:
            receipt_path = p["collision"] / "collision-receipt.json"
            probe_commands = pbrt_traversal.commands(json.loads(receipt_path.read_text()))
            content_files = sorted(f for f in p["content"].rglob("*") if f.is_file())
            self.step("traversal-boot", content_files + [receipt_path],
                      dict(self.boot_key(), commands=probe_commands),
                      ["portal_boot.py", "pbrt_traversal.py"], [p["traversal_boot"]],
                      lambda: self.run("traversal-boot", [
                          sys.executable, HERE / "portal_boot.py", *self.boot_target(),
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
                p["directional"].with_name(p["directional"].name + ".json"),
                p["directional_indirect"].with_name(p["directional_indirect"].name + ".json"))
                if path.is_file()]
            self.step("audit", receipts, {"profile": self.profile["name"],
                                         "profile_sha256": self.profile_hash, "boot": self.boot,
                                         "preview": self.preview},
                      ["map_export_audit.py"], [p["audit"]],
                      lambda: self.run("audit", [
                          sys.executable, HERE / "map_export_audit.py", "--build", self.out,
                          "--profile", self.profile["path"], "--out", p["audit"]] +
                          (["--booted"] if self.boot else [])))
        summary = {"status": "pass", "gate_findings": list(self.failed_gates),
                   "quality": self.profile["name"],
                   "preview": self.preview,
                   "production": not self.preview,
                   "profile_revision": self.profile.get("revision"),
                   "profile_sha256": self.profile_hash,
                   "failed_gates": self.failed_gates, "map": self.map, "manifest_scene": scene,
                   "content_root": str(p["content"]),
                   "bsp2_sha256": sha256(p["bsp2"]),
                   "gates": {name: gate_summary(path) for name, path in
                             (("reference", p["reference_gate"]), ("runtime", p["runtime_gate"]),
                              ("traversal", p["traversal"]))
                             if path.is_file()},
                   "steps": {name: self.state[name]["seconds"] for name in STEPS
                             if name in self.state}}
        # The medium the lightmap bake applied, from its receipt (the bake's
        # own evidence), published beside the map; a map baked without one
        # carries no such record.
        baked_medium = json.loads(p["atlas_receipt"].read_text()).get("medium") \
            if p["atlas_receipt"].is_file() else None
        if baked_medium != self.medium:
            raise SystemExit("the bake receipt's medium %r is not the manifest's %r" %
                             (baked_medium, self.medium))
        if baked_medium:
            summary["lightmap_medium"] = baked_medium
        (self.out / "build.json").write_text(json.dumps(summary, indent=2) + "\n")
        print(json.dumps(summary, indent=2))
        if self.publish:
            playable_maps.publish(summary, sidecars={
                MEDIUM_SIDECAR: json.dumps(baked_medium, indent=2, sort_keys=True) + "\n"}
                if baked_medium else None)
            print("published to %s; play it with ./kiln play portal %s" %
                  (playable_maps.STORE / self.map, self.map))
        if self.failed_gates:
            print("gate findings (reported, not fatal): " + ", ".join(self.failed_gates))


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
    parser.add_argument("--until", choices=STEPS,
                        help="stop after this step (no later step, no publish)")
    parser.add_argument("--boot", action="store_true",
                        help="boot the map headless in native Vulkan and screenshot it")
    parser.add_argument("--keep-going", action="store_true",
                        help="finish the map when a pixel gate fails; build.json records "
                             "status gate-failed and the exit status stays nonzero")
    parser.add_argument("--no-publish", action="store_true",
                        help="do not publish the finished map to run/maps for ./kiln play portal")
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
    try:
        Pipeline(manifest, toolchain, args.out, args.force_from, args.boot,
                 args.keep_going, not args.no_publish, args.until).build()
    except StopAfter as stop:
        print("stopped after %s (--until)" % stop)


if __name__ == "__main__":
    main()
