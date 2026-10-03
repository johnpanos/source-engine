#!/usr/bin/env python3
"""The RFC 0016 lighting fixture set: scenes, cameras and baked map inputs.

`render_lab` and the game use the same published maps for parity checks.
They extend the RFC 0011 GI gallery (quality/fixtures/gi/, `gi_gallery.py`) with the lighting set the
K11 gate names, under quality/fixtures/lighting/:

  cornell-floors     a Cornell box with a rough and a polished floor half
  area-room          64 rectangular emitters (LTC) and an emissive sign
  projector-cookie   an env_projectedtexture with a cookie, a pillar and a fill
  sun-colonnade      a low sun through a colonnade under a sky (cascades)
  foggy-hall         a homogeneous medium, 16 spots, 240 points and a projector
  mirror-corridor    mirror to rough floor segments (SSR over probes)
  material-sweep     metal, clear-coat and dielectric spheres, roughness 0.05-1
  portal-chamber     testchmb_a_00 as relit by legacy_bsp_relight.py
  portal2-chamber    sp_gi_chamber_01 as built by portal2_gi_chamber.py

Each synthetic fixture is a `gi-fixture/v1` stage and record (so the RFC 0011
tools read it) with a `lighting` block, plus `entities.json`: the lights as
Source entities, which `build` compiles into the map's entity lump. One
definition feeds both: every light is declared once here and written as a
UsdLux light (what Cycles and the bake see) and as its entity (what the lab
reads), in the pipeline's lightmap unit. `build` checks the compiled world
lights against the stage's lights.

    PYTHONPATH=build/toolchains/openusd-25.11/lib/python \\
        /usr/bin/python3.12 tools/quality/lighting_fixtures.py generate [--check]
    python3 tools/quality/lighting_fixtures.py render [--fixture NAME]... [--samples 16]
    python3 tools/quality/lighting_fixtures.py build [--fixture NAME]...
    python3 tools/quality/lighting_fixtures.py check [--fixture NAME]...

`generate` writes the stages, records, entities, cookies and manifest.json
(deterministic; `--check` compares). `render` renders references through
`usd_scene.py extract` and `lighting_reference_blender.py` (the GI reference
renderer plus projectors and media). `build` makes each map: the scene front
end's collision VMF (`pbrt_collision_vmf.py`) plus the fixture's entities,
compiled by `vmf_map_build.py`, then lit with the authored scene by the one
lighting back end (`map_lighting.py`) and published (./play <map>); a state
that owns a map (`lighting.state_maps`, a medium state) gets the same BSP lit
again with its medium in the lightmap bake (`map_lighting.light(medium=...)`).
`check` validates fixture data and historical references. Product parity uses
`game_lab_matrix.py --require-image-parity`; Cycles is not a render_lab gate.
"""

import argparse
import datetime
import filecmp
import hashlib
import json
import math
import os
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
sys.path.insert(0, str(HERE))

FIXTURES = Path(os.environ.get("LIGHTING_FIXTURES_ROOT", ROOT / "quality/fixtures/lighting"))
WORK = ROOT / "quality-results/lighting-fixtures"
MANIFEST_SCHEMA = "lighting-fixtures/v1"
REFERENCES_SCHEMA = "lighting-references/v1"
TOLERANCES_SCHEMA = "lighting-tolerances/v1"
RESULT_SCHEMA = "lighting-comparison/v1"
ENTITIES_SCHEMA = "lighting-entities/v1"
SOURCE_UNITS_PER_METER = 39.37007874015748
FILM = {"width": 512, "height": 384}
HORIZONTAL_FOV = 90.0
PREVIEW_SAMPLES = 16
FINAL_SAMPLES = 2048
# Historical reference status; these images do not certify render_lab.
DENOISED_SAMPLES = 256
REFERENCE_STATUSES = ("preview", "denoised", "final")
SEED = 20260929
# Point and spot lights are spheres and disks of vrad's normalization radius
# (legacy_bsp_scene.py LIGHT_RADIUS_UNITS), so Cycles, the bake and the
# derived-scene convention agree on their size.
LIGHT_RADIUS_UNITS = 2.0
LIGHT_RADIUS_M = LIGHT_RADIUS_UNITS / SOURCE_UNITS_PER_METER
# vrad's light brightness is normalized at 100 units (quadratic attenuation 1).
VRAD_NORMALIZE = 100.0 * 100.0
# Portal's env_projectedtexture (c_env_projectedtexture.cpp): fixed attenuation.
PROJECTOR_ATTENUATION = (0.0, 100.0, 0.0)
EMITTER_PREFIXES = ("LightQuad", "LightDisk")
# Fixture-carrier entity classes (proposed; the render-core owner decides the
# product form). Every other light uses a stock Source entity.
# Entity classes decided by the render-core owner (source-engine-43,
# 2026-09-29, binding rule 5), with Source 2's names.
RECT_CLASS = "light_rect"
MEDIUM_CLASS = "env_volumetric_fog_volume"
FOG_CONTROLLER_CLASS = "env_volumetric_fog_controller"

# The lighting model's terms (RFC 0016 "Lighting model"); each names its
# owning definition and the negative control that must fail its fixtures.
TERMS = {
    "brdf": {
        "owner": "RFC/0007-physically-based-lighting-pipeline.md#shading-model",
        "negative_control": "the lobe seeded wrong: energy compensation off (as "
                            "cl_render_debug_brdf 3), or a Lambert lobe in place of GGX: the "
                            "rough metal spheres and the polished floor leave tolerance"},
    "filtered-roughness": {
        "owner": "RFC/0012-antialiasing-msaa-specular-alpha-coverage.md#specular-antialiasing-renderpbr-specular-aav1",
        "negative_control": "input roughness in place of the filtered roughness: the distant "
                            "mirror and polished floor texels sparkle above the p99 tolerance"},
    "runtime-lights": {
        "owner": "RFC/0011-runtime-indirect-lighting.md#runtime-light-set-renderlight-setv1",
        "negative_control": "every other clustered light dropped from its cluster lists (a "
                            "seeded assignment defect): the hall's wall pools go dark"},
    "sun": {
        "owner": "RFC/0016-render-core.md#lighting-model-renderlightingv1-amended-2026-09-28",
        "negative_control": "the cascades' splits swapped, or the sun removed: the colonnade's "
                            "far shadows and sunlit floor leave tolerance"},
    "area-lights": {
        "owner": "RFC/0011-runtime-indirect-lighting.md#area-lights-light-set-v2-amendment-2026-09-28",
        "negative_control": "each rectangle evaluated as a point at its centre (no LTC) and "
                            "without horizon clipping: the glossy floor stripes' elongated "
                            "highlights and the wall strips' grazing light fail"},
    "projected-lights": {
        "owner": "public/render/projected_light.h (RFC 0011 render.projected-light.v1)",
        "negative_control": "the cookie ignored (white) or its v axis flipped: the window "
                            "pattern on the back wall leaves tolerance"},
    "direct-visibility": {
        "owner": "RFC/0016-render-core.md#lights-and-shadows-renderlightsv1-rendershadowsv1",
        "negative_control": "visibility one (shadows off): the pillar, block and column "
                            "shadows vanish and p99 fails"},
    "moving-occluders": {
        "owner": "RFC/0016-render-core.md#runtime-direct-light-amended-2026-09-30",
        "negative_control": "the bake's total layer with the door closed (a mover that "
                            "blocks no baked light), or runtime direct light without the door "
                            "as a caster: the spot's patch on room B's floor stays lit and p99 "
                            "fails"},
    "indirect-diffuse-static": {
        "owner": "RFC/0011-runtime-indirect-lighting.md#indirect-light-policy-renderindirect-policyv1",
        "negative_control": "the lightmap's indirect layer zero (or the total layer used with "
                            "runtime direct light, doubling it): every closed room fails its mean"},
    "indirect-diffuse-dynamic": {
        "owner": "RFC/0011-runtime-indirect-lighting.md#probe-volume-contract-renderprobe-volumev1",
        "negative_control": "the probe volume replaced by a black ambient cube: the dynamic "
                            "ProbeSphere's region leaves tolerance"},
    "image-based-specular": {
        "owner": "RFC/0007-physically-based-lighting-pipeline.md#image-based-lighting",
        "negative_control": "reflectance zero, or the wrong prefiltered mip: glossy spheres and "
                            "floors lose their reflections"},
    "screen-space-reflections": {
        "owner": "RFC/0016-render-core.md#lighting-model-renderlightingv1-amended-2026-09-28 "
                 "(render.pass.ssr)",
        "negative_control": "thickness ignored, no edge fade or the wrong mip (the K11 seeded "
                            "defects), and SSR off: the mirror corridor's on-screen hits fail"},
    "ambient-occlusion": {
        "owner": "RFC/0016-render-core.md#lighting-model-renderlightingv1-amended-2026-09-28 "
                 "(render.pass.ao)",
        "negative_control": "GTAO over the baked indirect light with no rule (double "
                            "occlusion), or applied to direct light: contact regions darken "
                            "past tolerance"},
    "specular-occlusion": {
        "owner": "RFC/0016-render-core.md#lighting-model-renderlightingv1-amended-2026-09-28",
        "negative_control": "specular occlusion one: probe reflections leak into the creases "
                            "under the blocks and pillars"},
    "emission": {
        "owner": "RFC/0016-render-core.md#the-surface-model-legacy-materials-as-degenerate-cases-plan-2026-09-28",
        "negative_control": "emission zero: the area room's sign and the light it casts are "
                            "missing"},
    "participating-media": {
        "owner": "RFC/0016-render-core.md#lighting-model-renderlightingv1-amended-2026-09-28 "
                 "(render.pass.volumetric)",
        "negative_control": "density zero (the clear state's frame) judged against the fog "
                            "reference, and the projector's shaft drawn inside its shadow"},
    "portal-transport": {
        "owner": "unittests/rendertest/contracts/render.portal-lights.v1.md (RFC 0011; each "
                 "term evaluated through open portal pairs by its images; decided by the "
                 "render-core owner, source-engine-43, 2026-09-29)",
        "negative_control": "the pair closed (no images, no view through it) judged against "
                            "the open reference: room B's floor, walls and the view through "
                            "the portal fail; the opening's rectangle in place of its ellipse "
                            "(render.portal-lights.v1 P2's clip) shows as extra light at the "
                            "corners of each through-portal pool"},
    "output": {
        "owner": "RFC/0016-render-core.md#output-renderoutputv1-amended-2026-09-28",
        "negative_control": "not judged by these fixtures: every comparison is of linear scene "
                            "radiance before exposure and output; render.output owns the term",
        "judged_by": "render.output"},
}

# The per-pixel error of a lab image against a reference (linear light).
ERROR_METRIC = {
    "definition": "e(p) = max over R, G, B of |lab(p) - ref(p)| / Y_ref, with Y_ref the mean "
                  "Rec. 709 luminance of the reference over the judged pixels; mean is the "
                  "average of e over the judged pixels and p99 its 99th percentile",
    "judged_pixels": "pixels whose reference object index is a scene mesh (not the background "
                     "or sky, index 0) and not a light's own visible emitter shape "
                     "(LightQuad*/LightDisk* meshes: the lab draws no light shapes)",
    "space": "linear scene radiance in the pipeline's lightmap unit, before exposure, tone map "
             "and output encoding; the lab image is a linear PFM of the same film and pose",
}


# ================================================================== pure data

def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def digest_json(value):
    return hashlib.sha256(json.dumps(value, sort_keys=True).encode()).hexdigest()


def write_json(path, value):
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, indent=2, sort_keys=True) + "\n")


def write_exr(path, rgb, half=False):
    """An uncompressed scanline OpenEXR of R, G, B, float32 or (`half`)
    float16, which gi_reference.read_exr reads back exactly. imageio's
    writer here stores half floats with lossy block compression (it zeroed
    5% of an environment map's texels)."""
    import struct
    import numpy as np
    kind, dtype, size = (1, "<f2", 2) if half else (2, "<f4", 4)
    rgb = np.ascontiguousarray(np.asarray(rgb, np.float32)[..., :3])
    height, width = rgb.shape[:2]

    def attribute(name, kind, payload):
        return name.encode() + b"\0" + kind.encode() + b"\0" + \
            struct.pack("<i", len(payload)) + payload
    channels = b"".join(name.encode() + b"\0" + struct.pack("<iB3xii", kind, 0, 1, 1)
                        for name in "BGR") + b"\0"
    window = struct.pack("<iiii", 0, 0, width - 1, height - 1)
    header = (b"\x76\x2f\x31\x01" + struct.pack("<I", 2) +
              attribute("channels", "chlist", channels) +
              attribute("compression", "compression", b"\0") +
              attribute("dataWindow", "box2i", window) +
              attribute("displayWindow", "box2i", window) +
              attribute("lineOrder", "lineOrder", b"\0") +
              attribute("pixelAspectRatio", "float", struct.pack("<f", 1.0)) +
              attribute("screenWindowCenter", "v2f", struct.pack("<ff", 0.0, 0.0)) +
              attribute("screenWindowWidth", "float", struct.pack("<f", 1.0)) + b"\0")
    line = 8 + 3 * size * width
    start = len(header) + 8 * height
    table = struct.pack("<%dQ" % height, *[start + y * line for y in range(height)])
    rows = [struct.pack("<ii", y, 3 * size * width) + b"".join(
        rgb[y, :, c].astype(dtype).tobytes() for c in (2, 1, 0)) for y in range(height)]
    Path(path).write_bytes(header + table + b"".join(rows))


def read_rgb_exr(path):
    """R, G, B of a single-part uncompressed scanline EXR (float64 H x W x 3)."""
    import numpy as np
    import gi_reference
    channels = gi_reference.read_exr(path)
    return np.stack([channels[c] for c in "RGB"], axis=-1).astype(np.float64)


def units(meters):
    return [round(float(c) * SOURCE_UNITS_PER_METER, 4) for c in meters]


def vec_text(values):
    return " ".join("%.4f" % float(v) for v in values)


def peak_and_norm(rgb):
    peak = max(float(c) for c in rgb)
    if peak <= 0:
        raise ValueError("a light needs a positive color")
    return peak, [float(c) / peak for c in rgb]


def gamma_color(norm):
    """Source keyvalue channels whose GammaToLinear (x/255)^2.2 is `norm`."""
    return [255.0 * (c ** (1.0 / 2.2)) for c in norm]


def light_direction_keys(direction):
    """vrad's light normal (map_utils.cpp SetupLightNormalFromProps):
    (cos yaw cos pitch, sin yaw cos pitch, sin pitch) from "pitch"/"angles"."""
    x, y, z = normalize(direction)
    pitch = math.degrees(math.asin(max(-1.0, min(1.0, z))))
    yaw = math.degrees(math.atan2(y, x)) if abs(z) < 0.999999 else 0.0
    return {"pitch": "%.4f" % pitch, "angles": "%.4f %.4f 0" % (pitch, yaw)}


def normalize(v):
    length = math.sqrt(sum(float(c) * float(c) for c in v))
    return [float(c) / length for c in v]


def angle_vectors(pitch, yaw):
    """The engine's AngleVectors with roll 0 (pitch positive looks down)."""
    sp, cp = math.sin(math.radians(pitch)), math.cos(math.radians(pitch))
    sy, cy = math.sin(math.radians(yaw)), math.cos(math.radians(yaw))
    return ([cp * cy, cp * sy, -sp], [sy, -cy, 0.0], [sp * cy, sp * sy, cp])


def angle_vectors_roll(pitch, yaw, roll):
    """The engine's AngleVectors: forward, right, up."""
    sp, cp = math.sin(math.radians(pitch)), math.cos(math.radians(pitch))
    sy, cy = math.sin(math.radians(yaw)), math.cos(math.radians(yaw))
    sr, cr = math.sin(math.radians(roll)), math.cos(math.radians(roll))
    forward = [cp * cy, cp * sy, -sp]
    right = [-sr * sp * cy + cr * sy, -sr * sp * sy - cr * cy, -sr * cp]
    up = [cr * sp * cy + sr * sy, cr * sp * sy - sr * cy, cr * cp]
    return forward, right, up


def angles_for(forward, up):
    """Source angles (pitch, yaw, roll) whose forward is `forward` and whose up
    is `up` projected perpendicular to it."""
    f = normalize(forward)
    pitch = math.degrees(math.asin(max(-1.0, min(1.0, -f[2]))))
    yaw = math.degrees(math.atan2(f[1], f[0]))
    _, right0, up0 = angle_vectors_roll(pitch, yaw, 0.0)
    # up(roll) = cos(roll) up0 + sin(roll) right0
    roll = math.degrees(math.atan2(sum(a * b for a, b in zip(up, right0)),
                                   sum(a * b for a, b in zip(up, up0))))
    return pitch, yaw, roll


def point_entity(light):
    """A `light` whose compiled world light is the sphere's: intensity
    L r^2 (units^2, lightmap unit) with inverse-square falloff."""
    peak, norm = peak_and_norm(light["radiance"])
    intensity = peak * LIGHT_RADIUS_UNITS ** 2
    # No targetname: vbsp makes every named light switchable (at most 32).
    keys = {"classname": "light", "_fixture_light": light["name"],
            "origin": vec_text(units(light["center_m"])),
            "_light": vec_text(gamma_color(norm) + [intensity * 255.0 / VRAD_NORMALIZE]),
            "_lightHDR": "-1 -1 -1 1", "_lightscaleHDR": "1",
            "_constant_attn": "0", "_linear_attn": "0", "_quadratic_attn": "1"}
    if light["kind"] == "spot":
        keys["classname"] = "light_spot"
        keys.update(light_direction_keys(light["direction"]))
        keys.update({"_inner_cone": "%.4f" % light["inner_degrees"],
                     "_cone": "%.4f" % light["outer_degrees"],
                     "_exponent": "%.4f" % light["exponent"]})
    return keys


def portal_entity(portal):
    """The pair's map-placed prop_portal: activated, linkage group 0."""
    return {"classname": "prop_portal", "targetname": portal["name"],
            "origin": vec_text(portal["origin_units"]),
            "angles": vec_text([round(a, 4) for a in portal["angles"]]),
            "Activated": "1", "PortalTwo": "1" if portal["portal_two"] else "0",
            "LinkageGroupID": str(portal["linkage_group"])}


def environment_entity(sun, sky):
    """A light_environment: the sun's world light is E / pi (its lightmap is
    intensity x cos) and the sky ambient's is the dome's radiance."""
    peak, norm = peak_and_norm(sun["irradiance"])
    keys = {"classname": "light_environment", "_fixture_light": sun["name"],
            "origin": vec_text(units(sun.get("entity_origin_m", (0.0, 0.0, 0.0)))),
            "_light": vec_text(gamma_color(norm) + [255.0 * peak / math.pi]),
            "_lightHDR": "-1 -1 -1 1", "_lightscaleHDR": "1",
            "SunSpreadAngle": "%.4f" % sun["angle_degrees"]}
    keys.update(light_direction_keys(sun["travel"]))
    if sky:
        sky_peak, sky_norm = peak_and_norm(sky["radiance"])
        keys.update({"_ambient": vec_text(gamma_color(sky_norm) + [255.0 * sky_peak]),
                     "_ambientHDR": "-1 -1 -1 1", "_AmbientScaleHDR": "1"})
    else:
        keys.update({"_ambient": "0 0 0 0", "_ambientHDR": "0 0 0 0"})
    return keys


def rect_entity(light):
    """A `light_rect` (render.area-light.v1 Rect + radiance). The entity's
    forward is the rectangle's normal, the direction it emits along. `width`
    is the full extent along the entity's left axis (-right) and `height`
    along its up axis: halfU = -right x width / 2 and halfV = up x height / 2,
    so halfU x halfV = forward (Source's AngleVectors has right x up =
    -forward). `color` is linear 0..255 and `brightness` scales it: radiance =
    color / 255 x brightness, in the lightmap unit."""
    u, v = units(light["half_u_m"]), units(light["half_v_m"])
    front = [u[1] * v[2] - u[2] * v[1], u[2] * v[0] - u[0] * v[2], u[0] * v[1] - u[1] * v[0]]
    pitch, yaw, roll = angles_for(front, v)
    peak, norm = peak_and_norm(light["radiance"])
    length = lambda w: math.sqrt(sum(c * c for c in w))
    # No targetname: vbsp gives every named light* entity a switchable style
    # (at most 32), as it does for stock lights.
    return {"classname": RECT_CLASS, "_fixture_light": light["name"],
            "origin": vec_text(units(light["center_m"])),
            "angles": "%.4f %.4f %.4f" % (pitch, yaw, roll),
            "width": "%.4f" % (2.0 * length(u)), "height": "%.4f" % (2.0 * length(v)),
            "color": vec_text([255.0 * c for c in norm]), "brightness": "%.6f" % peak,
            "two_sided": "0"}


def projector_entity(projector):
    peak, norm = peak_and_norm(projector["color"])
    return {"classname": "env_projectedtexture", "targetname": projector["name"],
            "origin": vec_text(projector["origin_units"]),
            "angles": "%.4f %.4f 0" % (projector["pitch"], projector["yaw"]),
            "lightfov": "%.4f" % projector["horizontal_fov_degrees"],
            "nearz": "%.4f" % projector["near_z"], "farz": "%.4f" % projector["far_z"],
            "lightcolor": vec_text(gamma_color(norm) + [255.0 * peak]),
            "texturename": projector["texture"], "enableshadows": "1", "lightworld": "1",
            "lightonlytarget": "0", "cameraspace": "0", "spawnflags": "1",
            "shadowquality": "1"}


def medium_entity(medium):
    """An `env_volumetric_fog_volume`: a homogeneous box medium. `density` is
    the extinction coefficient per Source unit (scattering + absorption),
    `albedo` the single-scattering albedo (scattering / extinction),
    `anisotropy` the Henyey-Greenstein g, `emission` zero. `box_mins` and
    `box_maxs` are relative to `origin`."""
    lo, hi = units(medium["bounds_m"][0]), units(medium["bounds_m"][1])
    center = [(a + b) / 2 for a, b in zip(lo, hi)]
    scattering = medium["scattering_per_m"] / SOURCE_UNITS_PER_METER
    absorption = medium["absorption_per_m"] / SOURCE_UNITS_PER_METER
    extinction = scattering + absorption
    return {"classname": MEDIUM_CLASS, "targetname": medium["name"],
            "origin": vec_text(center),
            "box_mins": vec_text([a - c for a, c in zip(lo, center)]),
            "box_maxs": vec_text([b - c for b, c in zip(hi, center)]),
            "density": "%.8f" % extinction,
            "albedo": "%.6f" % (scattering / extinction if extinction > 0 else 0.0),
            "anisotropy": "%.4f" % medium["anisotropy"], "emission": "0 0 0"}


def fog_controller_entity():
    """The one `env_volumetric_fog_controller`: no global height fog here; the
    fixture's medium is its fog volume alone."""
    return {"classname": FOG_CONTROLLER_CLASS, "targetname": "FogController",
            "density": "0", "height_fog_density": "0", "height_fog_falloff": "0",
            "anisotropy": "0"}


def expected_world_lights(lights):
    """The world lights vrad must compile from the entities: [(kind, origin
    units or None, intensity rgb)] in the lightmap unit."""
    expected = []
    for light in lights:
        if light["kind"] in ("point", "spot"):
            expected.append((light["kind"], units(light["center_m"]),
                             [c * LIGHT_RADIUS_UNITS ** 2 for c in light["radiance"]]))
        elif light["kind"] == "sun":
            expected.append(("sky", None, [c / math.pi for c in light["irradiance"]]))
        elif light["kind"] == "sky":
            expected.append(("sky_ambient", None, list(light["radiance"])))
    return expected


def compare_world_lights(expected, compiled, tolerance=0.01):
    """Problems where vrad's compiled world lights differ from the stage's."""
    problems = []
    pool = list(compiled)
    for kind, origin, intensity in expected:
        match = None
        for record in pool:
            if record["type"] != kind:
                continue
            if origin is not None and max(abs(a - b) for a, b in
                                          zip(record["origin"], origin)) > 0.05:
                continue
            match = record
            break
        if not match:
            problems.append("no compiled %s world light at %s" % (kind, origin))
            continue
        pool.remove(match)
        for want, got in zip(intensity, match["intensity"]):
            if abs(got - want) > tolerance * max(abs(want), 1e-6):
                problems.append("%s at %s: compiled intensity %s, stage %s" % (
                    kind, origin, [round(v, 5) for v in match["intensity"]],
                    [round(v, 5) for v in intensity]))
                break
    return problems


# ============================================================ authoring (pxr)

def authoring():
    """Import the USD authoring helpers (OpenUSD Python required)."""
    from pxr import Gf, Sdf, UsdGeom, UsdShade  # noqa: F401
    import gi_fixtures as gf
    import gi_gallery
    return gf, gi_gallery


def lighting_scene_class():
    from pxr import Gf, Sdf, UsdGeom, UsdShade
    gf, gi_gallery = authoring()

    class LightingScene(gi_gallery.Scene):
        """A gallery scene with physically based materials and lights that
        carry their Source entities."""

        def __init__(self, out, name, purpose, terms):
            super().__init__(out, name, "lighting", purpose)
            unknown = [t for t in terms if t not in TERMS]
            if unknown:
                raise ValueError("%s: unknown terms %s" % (name, unknown))
            self.terms = list(terms)
            self.lights = []
            self.projectors = []
            self.medium = None
            self.media_states = {}
            # States with their own map: the base map's compiled BSP, its
            # lightmap bake crossing the state's medium (lighting.state_maps).
            self.own_map_states = []
            self.map_overrides = {}
            self.cookies = {}
            self.portals = []
            # Per state: the moving objects the bake did not see (boxes of a
            # mesh the base state hides), as render_lab --mover draws them.
            self.movers = {}
            self.author.stage.SetMetadata(
                "comment", "RFC 0016 K11 lighting fixture %s; generated by "
                "tools/quality/lighting_fixtures.py, do not edit" % name)

        # ---------------------------------------------------------- materials
        def pbr(self, name, base, roughness, metallic=0.0, ior=1.5, clearcoat=0.0,
                clearcoat_roughness=0.03, emission=(0.0, 0.0, 0.0)):
            if name in self.author.materials:
                return name
            path = self.author.root_path.AppendPath("Looks/" + name)
            material = UsdShade.Material.Define(self.author.stage, path)
            shader = UsdShade.Shader.Define(self.author.stage, path.AppendChild("Surface"))
            shader.CreateIdAttr("UsdPreviewSurface")
            for key, kind, value in (
                    ("diffuseColor", Sdf.ValueTypeNames.Color3f, Gf.Vec3f(*base)),
                    ("roughness", Sdf.ValueTypeNames.Float, float(roughness)),
                    ("metallic", Sdf.ValueTypeNames.Float, float(metallic)),
                    ("ior", Sdf.ValueTypeNames.Float, float(ior)),
                    ("clearcoat", Sdf.ValueTypeNames.Float, float(clearcoat)),
                    ("clearcoatRoughness", Sdf.ValueTypeNames.Float,
                     float(clearcoat_roughness)),
                    ("emissiveColor", Sdf.ValueTypeNames.Color3f, Gf.Vec3f(*emission))):
                shader.CreateInput(key, kind).Set(value)
            material.CreateSurfaceOutput().ConnectToSource(shader.ConnectableAPI(), "surface")
            self.author.materials[name] = material
            return name

        def smooth_sphere(self, name, center, radius, material, solid=False):
            """A smooth-shaded geodesic sphere (per-vertex normals), for
            specular sweeps where facets would show: a mesh referencing one
            class prototype per radius (the points are written once), bound
            to its own material and placed by a translate. Geodesic, not UV:
            a UV sphere's pole triangles leave xatlas charts outside the
            lightmap seams gate's density spread (1.62 against 1.5); at 3
            subdivisions (1280 triangles) the sweep's spread is 1.48, at 4
            1.51, and at 5 the stitched seams fail their gate."""
            self.layout = "planar"
            if name in self.meshes:
                raise ValueError("%s: mesh %s defined twice" % (self.name, name))
            self.meshes.add(name)
            stage = self.author.stage
            prototype = self.author.root_path.AppendPath(
                "Prototypes/Sphere%dmm" % round(radius * 1000))
            if not stage.GetPrimAtPath(prototype):
                stage.CreateClassPrim(prototype.GetParentPath())
                icosphere_mesh(stage, prototype, radius, 3)
                stage.GetPrimAtPath(prototype).SetSpecifier(Sdf.SpecifierClass)
            prim = stage.DefinePrim(self.author.root_path.AppendPath("World/" + name), "Mesh")
            prim.GetReferences().AddInternalReference(prototype)
            UsdGeom.Xformable(prim).AddTranslateOp().Set(Gf.Vec3d(*center))
            UsdShade.MaterialBindingAPI.Apply(prim).Bind(self.author.materials[material])
            if solid:
                self.solids.append(name)
            return name

        # ------------------------------------------------------------- lights
        def point_light(self, name, center, radiance):
            peak, norm = peak_and_norm(radiance)
            self.point(name, center, LIGHT_RADIUS_M, peak, norm)
            self.lights.append({"kind": "point", "name": name, "center_m": list(center),
                                "radiance": list(radiance)})

        def spot_light(self, name, center, direction, radiance, inner, outer, exponent=1.0):
            peak, norm = peak_and_norm(radiance)
            self.spot(name, center, LIGHT_RADIUS_M, peak, norm, direction, inner, outer,
                      exponent)
            self.lights.append({"kind": "spot", "name": name, "center_m": list(center),
                                "direction": normalize(direction), "radiance": list(radiance),
                                "inner_degrees": inner, "outer_degrees": outer,
                                "exponent": exponent})

        def rect_light(self, name, center, size, radiance, direction=(0.0, 0.0, -1.0)):
            peak, norm = peak_and_norm(radiance)
            light = self.rect(name, center, size, peak, norm, direction)
            m = gf.look_rotation(direction)
            x = [m[0][k] for k in range(3)]
            y = [m[1][k] for k in range(3)]
            # The RectLight spans width along local X and height along local
            # Y and emits along local -Z = X x Y reversed, so halfU = Y h/2
            # and halfV = X w/2 give U x V = direction.
            self.lights.append({"kind": "rect", "name": name, "center_m": list(center),
                                "size_m": list(size), "direction": normalize(direction),
                                "half_u_m": [c * size[1] / 2 for c in y],
                                "half_v_m": [c * size[0] / 2 for c in x],
                                "radiance": list(radiance)})
            return light

        def sun_light(self, name, azimuth, elevation, irradiance, angle=0.53):
            peak, norm = peak_and_norm(irradiance)
            a, e = math.radians(azimuth), math.radians(elevation)
            travel = (-math.cos(e) * math.cos(a), -math.cos(e) * math.sin(a), -math.sin(e))
            light = self.author.distant_light(name, travel, peak, angle)
            light.CreateColorAttr(Gf.Vec3f(*norm))
            self.lights.append({"kind": "sun", "name": name, "travel": list(travel),
                                "irradiance": list(irradiance), "angle_degrees": angle,
                                "azimuth_degrees": azimuth, "elevation_degrees": elevation})

        def sky_light(self, name, radiance):
            peak, norm = peak_and_norm(radiance)
            self.sky(name, peak, norm)
            self.lights.append({"kind": "sky", "name": name, "radiance": list(radiance)})

        def projector(self, name, origin, pitch, yaw, fov, near, far, color, cookie):
            """An env_projectedtexture at `origin` (meters) looking along
            engine angles (pitch positive down), with Portal's attenuation."""
            forward, right, up = angle_vectors(pitch, yaw)
            self.projectors.append({
                "name": name, "origin_units": units(origin), "origin_m": list(origin),
                "pitch": pitch, "yaw": yaw, "forward": forward, "right": right, "up": up,
                "horizontal_fov_degrees": fov, "vertical_fov_degrees": fov,
                "near_z": near, "far_z": far, "attenuation": list(PROJECTOR_ATTENUATION),
                "color": list(color), "cookie_image": cookie + ".png",
                "texture": "lighting/cookies/" + cookie})
            self.cookies[cookie] = True

        def portal(self, name, frame, two):
            """A map-placed, activated prop_portal of linkage group 0 at the
            portal frame's centre, facing its forward (into its room)."""
            pitch, yaw, _ = angles_for(frame["forward"], frame["up"])
            self.portals.append({
                "name": name, "portal_two": bool(two), "linkage_group": 0,
                "center_m": list(frame["origin"]), "origin_units": units(frame["origin"]),
                "forward": list(frame["forward"]), "right": list(frame["right"]),
                "up": list(frame["up"]), "angles": [pitch, yaw, 0.0],
                "half_width_units": PORTAL_HALF_WIDTH_UNITS,
                "half_height_units": PORTAL_HALF_HEIGHT_UNITS, "aperture": "ellipse"})

        def fog(self, name, lo, hi, scattering, absorption, anisotropy):
            self.medium = {"name": name, "bounds_m": [list(lo), list(hi)],
                           "scattering_per_m": scattering, "absorption_per_m": absorption,
                           "anisotropy": anisotropy}

        def mover(self, name, lo, hi, material, states):
            """A box the base state hides and `states` show: a moving object
            the bake did not see (a closed door). Each listed state's
            record names it in Source units with its world material's VMT
            (render_lab --mover); the material must also be on a world
            surface, so the map carries its VMT."""
            from pxr import UsdGeom
            self.box(name, lo, hi, material, solid=False)
            UsdGeom.Imageable(self.author.stage.GetPrimAtPath(
                "%s/World/%s" % (self.root, name))).CreateVisibilityAttr().Set(
                    UsdGeom.Tokens.invisible)
            record = {"name": name, "min": [v * SOURCE_UNITS_PER_METER for v in lo],
                      "max": [v * SOURCE_UNITS_PER_METER for v in hi],
                      "material": "lt_%s/%s" % (self.name.replace("-", "_"), material.lower())}
            for state in states:
                self.movers.setdefault(state, []).append(record)
            return [("%s/World/%s" % (self.root, name), "visibility", None,
                     UsdGeom.Tokens.inherited)]

        # ------------------------------------------------------------- output
        def entities(self):
            entities = []
            sun = next((l for l in self.lights if l["kind"] == "sun"), None)
            sky = next((l for l in self.lights if l["kind"] == "sky"), None)
            if sky and not sun:
                raise ValueError("%s: a sky needs a sun (light_environment)" % self.name)
            for light in self.lights:
                if light["kind"] in ("point", "spot"):
                    entities.append(point_entity(light))
                elif light["kind"] == "rect":
                    entities.append(rect_entity(light))
            if sun:
                entities.append(environment_entity(sun, sky))
            entities += [projector_entity(p) for p in self.projectors]
            if self.medium:
                entities.append(medium_entity(self.medium))
                entities.append(fog_controller_entity())
            entities += [portal_entity(p) for p in self.portals]
            return entities

        def finish_lighting(self, map_overrides, cameras_note=None):
            if not self.cameras:
                raise ValueError(self.name + ": no cameras")
            base_states = [s for s, v in self.states.items() if v["layer"] is None]
            if len(base_states) != 1:
                raise ValueError(self.name + ": exactly one base state is required")
            for camera, regions in self.regions.items():
                for region, entries in regions.items():
                    unknown = [e for e in entries if e not in self.meshes and
                               e not in self.models]
                    if unknown:
                        raise ValueError("%s: camera %s region %s names unknown %s" % (
                            self.name, camera, region, unknown))
            first = gf.camera_pose(*self.spawn) if self.spawn else \
                next(iter(self.cameras.values()))
            self.author.camera("Camera", first)
            self.author.save()
            stage = self.directory / (self.name + ".usda")
            states = {}
            for state, entry in self.states.items():
                if entry["layer"]:
                    gf.state_layer(self.directory / entry["layer"], stage, entry["edits"])
                states[state] = {"layer": entry["layer"], "note": entry["note"]}
            baked = base_states[0]
            media = {state: (self.medium if self.media_states.get(state, True) else None)
                     for state in states} if self.medium else {}
            map_name = "lt_" + self.name.replace("-", "_")
            state_maps = {}
            for state in self.own_map_states:
                if not media.get(state):
                    raise ValueError("%s: state %s owns a map but has no medium" % (
                        self.name, state))
                name = "%s_%s" % (map_name, state)
                state_maps[state] = {
                    "name": name, "bsp": "run/maps/%s/maps/%s.bsp" % (name, name),
                    "bake": "the base map's compiled BSP and baked state; the lightmap bake "
                            "crosses this state's medium (lighting.media)"}
            lighting = {
                "terms": self.terms, "entities": "entities.json",
                "lights": {kind: sum(l["kind"] == kind for l in self.lights)
                           for kind in ("point", "spot", "rect", "sun", "sky")},
                "projectors": self.projectors, "media": media,
                **({"portals": self.portals} if self.portals else {}),
                **({"movers": self.movers} if self.movers else {}),
                "map": {"name": map_name, "bsp": "run/maps/%s/maps/%s.bsp" % (map_name,
                                                                                map_name),
                        "baked_state": baked, "overrides": map_overrides,
                        "solid_meshes": list(self.solids), "layout": self.layout},
                **({"state_maps": state_maps} if state_maps else {})}
            extra = {"family": "lighting", "oracles": self.oracles, "film": FILM,
                     "horizontal_fov_degrees": HORIZONTAL_FOV, "lambertian": False,
                     # The joined-copy USD scene is a transport diagnostic,
                     # not a Cycles oracle for a runtime portal view.
                     "cycles_receiver_oracle": "portal-transport" not in self.terms,
                     "lighting": lighting}
            record = gf.fixture_record(self.name, self.purpose, stage.name, states,
                                       self.cameras, self.regions, baked, extra)
            record["map_manifest"] = None
            gf.write_json(self.directory / "fixture.json", record)
            write_json(self.directory / "entities.json", {
                "schema": ENTITIES_SCHEMA, "fixture": self.name, "units": "Source units",
                "lights": self.lights, "entities": self.entities()})
            for cookie in self.cookies:
                write_cookie(self.directory / (cookie + ".png"))
            return record

    return LightingScene


def icosphere_mesh(stage, path, radius, subdivisions):
    """A geodesic sphere: an icosahedron split `subdivisions` times, corners
    on the sphere, smooth (per-corner) normals and spherical st."""
    from pxr import Gf, Sdf, UsdGeom, Vt
    t = (1.0 + 5 ** 0.5) / 2
    vertices = [normalize(v) for v in (
        (-1, t, 0), (1, t, 0), (-1, -t, 0), (1, -t, 0), (0, -1, t), (0, 1, t),
        (0, -1, -t), (0, 1, -t), (t, 0, -1), (t, 0, 1), (-t, 0, -1), (-t, 0, 1))]
    faces = [(0, 11, 5), (0, 5, 1), (0, 1, 7), (0, 7, 10), (0, 10, 11), (1, 5, 9), (5, 11, 4),
             (11, 10, 2), (10, 7, 6), (7, 1, 8), (3, 9, 4), (3, 4, 2), (3, 2, 6), (3, 6, 8),
             (3, 8, 9), (4, 9, 5), (2, 4, 11), (6, 2, 10), (8, 6, 7), (9, 8, 1)]
    for _ in range(subdivisions):
        middle = {}

        def mid(a, b):
            key = (min(a, b), max(a, b))
            if key not in middle:
                middle[key] = len(vertices)
                vertices.append(normalize([(x + y) / 2 for x, y in zip(vertices[a],
                                                                      vertices[b])]))
            return middle[key]
        faces = [f for a, b, c in faces for f in (
            (a, mid(a, b), mid(c, a)), (b, mid(b, c), mid(a, b)), (c, mid(c, a), mid(b, c)),
            (mid(a, b), mid(b, c), mid(c, a)))]
    mesh = UsdGeom.Mesh.Define(stage, path)
    indices = [i for face in faces for i in face]
    mesh.CreatePointsAttr(Vt.Vec3fArray([Gf.Vec3f(*(radius * c for c in v)) for v in vertices]))
    mesh.CreateFaceVertexCountsAttr(Vt.IntArray([3] * len(faces)))
    mesh.CreateFaceVertexIndicesAttr(Vt.IntArray(indices))
    mesh.CreateNormalsAttr(Vt.Vec3fArray([Gf.Vec3f(*vertices[i]) for i in indices]))
    mesh.SetNormalsInterpolation(UsdGeom.Tokens.faceVarying)
    mesh.CreateSubdivisionSchemeAttr(UsdGeom.Tokens.none)
    mesh.CreateExtentAttr(UsdGeom.PointBased.ComputeExtent(mesh.GetPointsAttr().Get()))
    st = [(0.5 + math.atan2(vertices[i][1], vertices[i][0]) / (2 * math.pi),
           0.5 + math.asin(max(-1.0, min(1.0, vertices[i][2]))) / math.pi) for i in indices]
    primvar = UsdGeom.PrimvarsAPI(mesh).CreatePrimvar(
        "st", Sdf.ValueTypeNames.TexCoord2fArray, UsdGeom.Tokens.faceVarying)
    primvar.Set(Vt.Vec2fArray([Gf.Vec2f(*uv) for uv in st]))
    return mesh


def write_cookie(path):
    """A 128 x 128 window gobo: 3 x 3 colored panes behind black mullions and
    a black border, values linear (pixel / 255)."""
    from PIL import Image
    size, border, bar = 128, 8, 6
    panes = [(255, 244, 214), (190, 214, 255), (255, 200, 150),
             (230, 255, 220), (255, 255, 255), (255, 220, 235),
             (210, 200, 255), (255, 236, 170), (200, 245, 255)]
    image = Image.new("RGB", (size, size), (0, 0, 0))
    pixels = image.load()
    cell = (size - 2 * border) / 3.0
    for y in range(size):
        for x in range(size):
            u, v = x - border, y - border
            if not (0 <= u < size - 2 * border and 0 <= v < size - 2 * border):
                continue
            cu, cv = int(u // cell), int(v // cell)
            if abs(u - cell * round(u / cell)) < bar / 2 or \
                    abs(v - cell * round(v / cell)) < bar / 2:
                continue
            pixels[x, y] = panes[3 * min(cv, 2) + min(cu, 2)]
    image.save(path, optimize=False)


# ------------------------------------------------------------------ fixtures

PREVIEW_BAKE = {"lightmap": {"samples": 64}, "probe_volume": {"samples": 256},
                "radiosity": None, "sdf_volume": None}
PREVIEW_PROBES = {"reflection_probe": {"width": 512, "face_size": 128, "samples": 16,
                                       "light_paths": "gi-reference"}}


def with_probes():
    return dict(PREVIEW_BAKE, **PREVIEW_PROBES)


def cornell_floors(Scene, out):
    s = Scene(out, "cornell-floors", "The Cornell box with its floor split into a rough half "
              "(roughness 0.5) and a polished half (0.05): the ceiling rectangle's direct "
              "light and soft shadows, the colored bleed, and on the polished half the "
              "reflections of the walls and blocks (image-based and screen-space specular)",
              ["brdf", "filtered-roughness", "area-lights", "direct-visibility",
               "indirect-diffuse-static", "indirect-diffuse-dynamic", "image-based-specular",
               "screen-space-reflections", "ambient-occlusion", "specular-occlusion"])
    s.pbr("White", (0.73, 0.73, 0.73), 0.9)
    s.pbr("Red", (0.63, 0.065, 0.05), 0.9)
    s.pbr("Green", (0.14, 0.45, 0.09), 0.9)
    s.pbr("FloorRough", (0.55, 0.55, 0.55), 0.5)
    s.pbr("FloorPolished", (0.55, 0.55, 0.55), 0.05)
    s.room("C", (0.0, 0.0, 0.0), (3.0, 3.0, 3.0), "White", skip=("Zn",),
           materials={"Xn": "Red", "Xp": "Green"})
    s.panel("FloorRoughHalf", (0.0, 0.0, 0.0), (1.5, 3.0, 0.0), 2, 1, "FloorRough")
    s.panel("FloorPolishedHalf", (1.5, 0.0, 0.0), (3.0, 3.0, 0.0), 2, 1, "FloorPolished")
    s.box_faces("Tall", (0.4, 1.7, 0.0), (1.1, 2.4, 1.8), "White", skip=("Zn",))
    s.box_faces("Short", (1.9, 0.8, 0.0), (2.6, 1.5, 0.9), "White", skip=("Zn",))
    s.rect_light("Lamp", (1.5, 1.5, 2.99), (0.75, 0.75), (12.0, 12.0, 12.0))
    s.probe("ProbeSphere", (1.5, 2.5, 2.1))
    walls = ["C_Xn", "C_Xp", "C_Yp", "C_Zp", "FloorRoughHalf", "FloorPolishedHalf",
             "Tall_Yn", "Tall_Xp", "Short_Yn", "Short_Xn", "ProbeSphere"]
    s.view("front", (1.5, 0.12, 1.5), (1.5, 3.0, 1.2), walls)
    s.view("floor", (2.8, 0.15, 0.45), (1.1, 2.7, 0.2),
           ["C_Xn", "C_Yp", "FloorRoughHalf", "FloorPolishedHalf", "Short_Yn", "Tall_Yn",
            "Tall_Xp", "Short_Xn"])
    s.base("default", "ceiling rectangle on")
    return s.finish_lighting(with_probes())


def area_room(Scene, out):
    import gi_gallery
    s = Scene(out, "area-room", "An 8 x 6 m room lit by 64 rectangular lights: 48 colored "
              "ceiling panels and 16 low wall strips whose light grazes the floor (horizon "
              "clipping), over floor stripes of roughness 0.1, 0.3 and 0.6 that stretch each "
              "panel's highlight, with pedestals casting soft shadows and an emissive sign "
              "that is not a light",
              ["area-lights", "brdf", "direct-visibility", "indirect-diffuse-static",
               "emission", "image-based-specular"])
    s.pbr("Wall", (0.7, 0.7, 0.7), 0.9)
    s.pbr("Pedestal", (0.6, 0.55, 0.5), 0.6)
    for k, rough in enumerate((0.1, 0.3, 0.6)):
        s.pbr("Floor%d" % k, (0.45, 0.45, 0.45), rough)
        s.panel("FloorStripe%d" % k, (0.0, 2.0 * k, 0.0), (8.0, 2.0 * k + 2.0, 0.0), 2, 1,
                "Floor%d" % k)
    s.room("A", (0.0, 0.0, 0.0), (8.0, 6.0, 3.0), "Wall", skip=("Zn",))
    s.box_faces("PedestalA", (2.0, 1.2, 0.0), (2.8, 2.0, 1.0), "Pedestal", skip=("Zn",))
    s.box_faces("PedestalB", (4.6, 3.6, 0.0), (5.2, 4.2, 1.4), "Pedestal", skip=("Zn",))
    s.box_faces("PedestalC", (6.2, 1.0, 0.0), (7.0, 1.6, 0.6), "Pedestal", skip=("Zn",))
    s.pbr("Sign", (0.05, 0.05, 0.05), 0.8, emission=(0.4, 2.4, 0.9))
    s.panel("EmissiveSign", (7.99, 2.4, 1.4), (7.99, 3.6, 2.0), 0, -1, "Sign")
    index = 0
    for row in range(8):
        for column in range(6):
            hue = 45.0 * row + 60.0 * column
            color = gi_gallery.hsv(hue, 0.45, 1.0)
            s.rect_light("Panel%02d" % index, (0.6 + 1.0 * row + 0.0, 0.5 + 1.0 * column, 2.99),
                         (0.4, 0.4), tuple(5.0 * c for c in color))
            index += 1
    for k in range(8):
        color = gi_gallery.hsv(30.0 * k, 0.3, 1.0)
        s.rect_light("StripS%d" % k, (0.5 + 1.0 * k, 0.01, 0.35), (0.6, 0.1),
                     tuple(10.0 * c for c in color), direction=(0.0, 1.0, 0.0))
        s.rect_light("StripN%d" % k, (0.5 + 1.0 * k, 5.99, 0.35), (0.6, 0.1),
                     tuple(10.0 * c for c in color), direction=(0.0, -1.0, 0.0))
    if len(s.lights) != 64:
        raise ValueError("area-room needs 64 area lights, has %d" % len(s.lights))
    floor = ["FloorStripe0", "FloorStripe1", "FloorStripe2"]
    s.view("overview", (0.4, 3.0, 2.4), (8.0, 3.0, 0.4),
           floor + ["A_Xp", "A_Yn", "A_Yp", "A_Zp", "PedestalA_Yn", "PedestalB_Xn",
                    "EmissiveSign"])
    s.view("grazing", (7.6, 0.8, 0.35), (0.5, 4.0, 0.05),
           floor + ["A_Xn", "A_Yp", "PedestalA_Xp", "PedestalB_Xp"])
    s.view("wall", (4.0, 3.3, 1.1), (4.0, 0.0, 0.2), floor + ["A_Yn"])
    s.base("default", "all 64 rectangles and the sign on")
    return s.finish_lighting(with_probes())


def projector_cookie(Scene, out):
    s = Scene(out, "projector-cookie", "A dark room where an env_projectedtexture shines a "
              "window cookie onto a Lambertian floor and back wall past a pillar that shadows "
              "it and a glossy sphere that reflects it, under a dim fill rectangle; Portal's "
              "attenuation (linear 100 / d, clamped) and end falloff past 0.6 far",
              ["projected-lights", "direct-visibility", "brdf", "indirect-diffuse-static"])
    s.pbr("Dark", (0.12, 0.12, 0.12), 0.9)
    s.pbr("Screen", (0.6, 0.6, 0.6), 1.0, ior=1.0)
    s.pbr("Pillar", (0.5, 0.5, 0.5), 0.8)
    s.pbr("Gloss", (0.2, 0.25, 0.35), 0.15)
    s.room("P", (0.0, 0.0, 0.0), (6.0, 6.0, 3.5), "Dark", skip=("Zn", "Xp"))
    s.panel("Floor", (0.0, 0.0, 0.0), (6.0, 6.0, 0.0), 2, 1, "Screen")
    s.panel("BackWall", (6.0, 0.0, 0.0), (6.0, 6.0, 3.5), 0, -1, "Screen")
    s.box_faces("Pillar", (3.2, 2.2, 0.0), (3.6, 2.6, 2.2), "Pillar", skip=("Zn",))
    s.smooth_sphere("GlossSphere", (4.4, 4.0, 0.5), 0.5, "Gloss")
    s.rect_light("Fill", (1.0, 3.0, 3.49), (1.0, 1.0), (1.5, 1.5, 1.5))
    s.projector("Projector", (0.6, 3.0, 2.6), 18.0, 0.0, 55.0, 4.0, 330.0,
                (2.5, 2.3, 2.0), "window_gobo")
    s.view("room", (0.3, 0.5, 1.9), (6.0, 3.6, 0.9),
           ["Floor", "BackWall", "Pillar_Xn", "Pillar_Yn", "GlossSphere", "P_Yp"])
    s.view("wall", (1.8, 3.0, 1.6), (6.0, 3.0, 1.2), ["Floor", "BackWall", "Pillar_Xn"])
    s.base("default", "projector and fill")
    s.state("projector", "projector alone (fill off): the direct projected light, judged "
            "against projected_light::IrradianceAt", s.off("Fill"))
    return s.finish_lighting(dict(PREVIEW_BAKE))


def sun_colonnade(Scene, out):
    s = Scene(out, "sun-colonnade", "A 30 m portico: ten columns under a roof slab, a back "
              "wall and a sunlit yard, lit by a low warm sun (35 degrees) and a blue sky, "
              "seen along the colonnade so column shadows run from near to far (cascades) "
              "and from the yard",
              ["sun", "direct-visibility", "indirect-diffuse-static",
               "indirect-diffuse-dynamic", "ambient-occlusion", "brdf"])
    s.pbr("Stone", (0.55, 0.52, 0.48), 0.8)
    s.pbr("Paving", (0.45, 0.43, 0.4), 0.6)
    s.panel("Yard", (0.0, 0.0, 0.0), (30.0, 10.0, 0.0), 2, 1, "Paving")
    s.box_faces("BackWall", (0.0, 10.0, 0.0), (30.0, 10.4, 6.0), "Stone", skip=("Zn",))
    s.box_faces("Roof", (0.0, 5.5, 5.0), (30.0, 10.0, 5.4), "Stone")
    columns = [((x - 0.25, 5.75, 0.0), (x + 0.25, 6.25, 5.0))
               for x in [1.5 + 3.0 * k for k in range(10)]]
    s.boxes("Columns", columns, "Stone", skip=("Zn", "Zp"))
    s.sun_light("Sun", -55.0, 35.0, (5.0, 4.4, 3.6))
    s.sky_light("Sky", (0.25, 0.33, 0.5))
    s.probe("ProbeSphere", (15.0, 8.0, 1.2))
    s.view("along", (0.8, 7.6, 1.7), (30.0, 6.4, 0.8),
           ["Yard", "BackWall_Yn", "Roof_Zn", "Columns", "ProbeSphere"])
    s.view("yard", (15.0, -5.0, 2.5), (15.0, 8.0, 1.8),
           ["Yard", "BackWall_Yn", "Columns", "Roof_Yn", "ProbeSphere"])
    s.spawn = ((3.0, 8.0, 1.7), (30.0, 8.0, 1.2))
    s.base("default", "sun and sky")
    return s.finish_lighting(dict(PREVIEW_BAKE))


def foggy_hall(Scene, out):
    import gi_gallery
    s = Scene(out, "foggy-hall", "A 24 m hall filled with a homogeneous medium (scattering "
              "0.06/m, absorption 0.01/m, anisotropy 0.3): 16 ceiling spots draw shafts, "
              "240 colored bulbs line the walls (256 clustered lights), and a projector "
              "at the far end throws a shaft that a pillar shadows; the clear state is the "
              "same hall with density zero",
              ["participating-media", "runtime-lights", "projected-lights",
               "direct-visibility", "indirect-diffuse-static", "brdf"])
    s.pbr("Hall", (0.5, 0.5, 0.5), 0.85)
    s.pbr("HallFloor", (0.4, 0.4, 0.4), 0.5)
    s.room("H", (0.0, 0.0, 0.0), (24.0, 8.0, 6.0), "Hall", skip=("Zn",))
    s.panel("Floor", (0.0, 0.0, 0.0), (24.0, 8.0, 0.0), 2, 1, "HallFloor")
    pillars = [((x - 0.3, y - 0.3, 0.0), (x + 0.3, y + 0.3, 6.0))
               for x in (4.0, 8.0, 12.0, 16.0, 20.0) for y in (2.2, 5.8)]
    s.boxes("Pillars", pillars, "Hall", skip=("Zn", "Zp"))
    for k in range(16):
        x, y = 1.5 + 3.0 * (k // 2), (2.8 if k % 2 == 0 else 5.2)
        s.spot_light("Spot%02d" % k, (x, y, 5.9), (0.0, 0.0, -1.0), (26900.0, 26000.0, 24000.0),
                     14.0, 24.0, 1.0)
    index = 0
    for side, y, direction in ((0, 0.25, 1.0), (1, 7.75, -1.0)):
        for height in (1.0, 2.2, 3.4):
            for k in range(40):
                color = gi_gallery.hsv(9.0 * k + 120.0 * side + 40.0 * height, 0.55, 1.0)
                s.point_light("Bulb%03d" % index, (0.3 + 0.6 * k, y, height),
                              tuple(45.0 * c for c in color))
                index += 1
    if sum(l["kind"] in ("point", "spot") for l in s.lights) != 256:
        raise ValueError("foggy-hall needs 256 clustered lights")
    s.projector("Projector", (23.4, 4.0, 3.0), 8.0, 180.0, 40.0, 4.0, 1000.0,
                (3.5, 3.5, 3.2), "window_gobo")
    s.fog("Fog", (0.0, 0.0, 0.0), (24.0, 8.0, 6.0), 0.06, 0.01, 0.3)
    s.view("nave", (1.0, 4.0, 1.7), (24.0, 4.0, 2.6), ["Floor", "H_Xp", "H_Yn", "H_Yp", "H_Zp",
                                                      "Pillars"])
    s.view("side", (10.0, 0.7, 1.5), (10.0, 8.0, 3.2), ["Floor", "H_Yp", "H_Zp", "Pillars"])
    s.base("fog", "the medium on (scattering 0.06/m, absorption 0.01/m, g 0.3)")
    s.state("clear", "density zero: the same hall and lights with no medium", [])
    s.media_states = {"fog": True, "clear": False}
    # The fog state's surfaces receive light through the medium, as Cycles'
    # do: its own map, baked with the medium (the clear state keeps the base).
    s.own_map_states = ["fog"]
    return s.finish_lighting(dict(PREVIEW_BAKE))


def mirror_corridor(Scene, out):
    s = Scene(out, "mirror-corridor", "An 18 m corridor whose floor runs from a mirror "
              "(metal, roughness 0.02) through gloss 0.1 and 0.25 to 0.5 (above the SSR "
              "cutoff), between colored wall panels, under ceiling strips, with a block near "
              "the camera whose reflection leaves the screen (probe fallback)",
              ["screen-space-reflections", "image-based-specular", "filtered-roughness",
               "specular-occlusion", "brdf", "area-lights", "indirect-diffuse-static"])
    s.pbr("Mirror", (0.95, 0.95, 0.95), 0.02, metallic=1.0)
    s.pbr("Gloss10", (0.08, 0.08, 0.08), 0.1)
    s.pbr("Gloss25", (0.08, 0.08, 0.08), 0.25)
    s.pbr("Rough50", (0.3, 0.3, 0.3), 0.5)
    s.pbr("Wall", (0.65, 0.65, 0.65), 0.8)
    for name, color in (("PanelRed", (0.7, 0.08, 0.06)), ("PanelTeal", (0.05, 0.5, 0.5)),
                        ("PanelYellow", (0.75, 0.6, 0.05))):
        s.pbr(name, color, 0.7)
    s.pbr("Block", (0.15, 0.3, 0.7), 0.4)
    for k, (material, x0, x1) in enumerate((("Mirror", 0.0, 4.5), ("Gloss10", 4.5, 9.0),
                                            ("Gloss25", 9.0, 13.5), ("Rough50", 13.5, 18.0))):
        s.panel("Floor%d" % k, (x0, 0.0, 0.0), (x1, 3.0, 0.0), 2, 1, material)
    s.room("K", (0.0, 0.0, 0.0), (18.0, 3.0, 3.2), "Wall", skip=("Zn",))
    colors = ("PanelRed", "PanelTeal", "PanelYellow")
    for k in range(6):
        x = 1.0 + 3.0 * k
        s.panel("PanelS%d" % k, (x, 0.01, 0.5), (x + 1.6, 0.01, 2.4), 1, 1, colors[k % 3])
        s.panel("PanelN%d" % k, (x + 0.8, 2.99, 0.5), (x + 2.4, 2.99, 2.4), 1, -1,
                colors[(k + 1) % 3])
    s.box_faces("Block", (1.2, 0.2, 0.0), (1.8, 0.8, 1.2), "Block", skip=("Zn",))
    s.smooth_sphere("Ball", (10.5, 1.9, 0.45), 0.45, "PanelYellow")
    for k in range(6):
        s.rect_light("Strip%d" % k, (1.5 + 3.0 * k, 1.5, 3.19), (0.3, 1.6), (9.0, 9.0, 8.5))
    floor = ["Floor0", "Floor1", "Floor2", "Floor3"]
    s.view("down", (0.4, 1.5, 1.6), (18.0, 1.5, 0.5),
           floor + ["K_Xp", "K_Zp", "PanelS1", "PanelN1", "Ball"])
    s.view("low", (2.2, 0.5, 0.4), (12.0, 2.2, 0.9), floor + ["K_Yp", "PanelN2", "Ball"])
    s.base("default", "ceiling strips on")
    return s.finish_lighting(with_probes())


def material_sweep(Scene, out):
    s = Scene(out, "material-sweep", "Three rows of eight smooth spheres at roughness 0.05, "
              "0.1, 0.2, 0.3, 0.45, 0.6, 0.8 and 1.0: gold metal, a clear coat (coat "
              "roughness swept) over a red base, and a white dielectric, in a studio with "
              "colored walls under a key rectangle and a spot",
              ["brdf", "image-based-specular", "filtered-roughness", "area-lights",
               "runtime-lights", "indirect-diffuse-static", "specular-occlusion"])
    roughness = (0.05, 0.1, 0.2, 0.3, 0.45, 0.6, 0.8, 1.0)
    s.pbr("Studio", (0.5, 0.5, 0.5), 0.9)
    s.pbr("Backdrop", (0.1, 0.3, 0.55), 0.9)
    s.pbr("SideWarm", (0.7, 0.35, 0.1), 0.9)
    s.room("S", (0.0, 0.0, 0.0), (10.0, 7.0, 4.0), "Studio",
           materials={"Yp": "Backdrop", "Xn": "SideWarm"})
    rows = []
    for k, r in enumerate(roughness):
        x = 1.5 + 1.0 * k
        s.pbr("Gold%d" % k, (1.0, 0.78, 0.34), r, metallic=1.0)
        s.pbr("Coat%d" % k, (0.6, 0.05, 0.05), 0.5, clearcoat=1.0, clearcoat_roughness=r)
        s.pbr("Dielectric%d" % k, (0.8, 0.8, 0.8), r)
        rows += [s.smooth_sphere("GoldSphere%d" % k, (x, 4.5, 2.3), 0.32, "Gold%d" % k),
                 s.smooth_sphere("CoatSphere%d" % k, (x, 4.5, 1.5), 0.32, "Coat%d" % k),
                 s.smooth_sphere("DielectricSphere%d" % k, (x, 4.5, 0.7), 0.32,
                                 "Dielectric%d" % k)]
    s.rect_light("Key", (3.0, 0.4, 3.4), (1.6, 1.0), (14.0, 13.5, 12.5),
                 direction=(0.3, 0.8, -0.5))
    s.spot_light("Rim", (8.5, 6.5, 3.8), (-0.4, -0.6, -0.7), (4.0e4, 4.2e4, 4.6e4), 18.0, 30.0)
    s.view("front", (5.0, 0.35, 1.5), (5.0, 4.5, 1.5), rows + ["S_Yp", "S_Zn"])
    s.view("grazing", (0.5, 2.2, 1.5), (9.0, 4.6, 1.5), rows + ["S_Yp", "S_Xp"])
    s.base("default", "key rectangle and rim spot on")
    return s.finish_lighting(with_probes())


# ------------------------------------------------------------- portal pair

def portal_frame(center, normal):
    """A portal's frame: origin at the opening's centre, forward its normal
    (into its room), up world Z, right = forward x up."""
    forward = normalize(normal)
    up = (0.0, 0.0, 1.0)
    right = normalize((forward[1] * up[2] - forward[2] * up[1],
                       forward[2] * up[0] - forward[0] * up[2],
                       forward[0] * up[1] - forward[1] * up[0]))
    return {"origin": tuple(center), "forward": forward, "right": right, "up": up}


def portal_map_vector(source, target, v):
    """The linked pair's rotation: entering `source` along +v leaves `target`
    along the image of v (local forward and right negated, up kept), as
    Source's MatrixThisToLinked."""
    f = sum(a * b for a, b in zip(v, source["forward"]))
    r = sum(a * b for a, b in zip(v, source["right"]))
    u = sum(a * b for a, b in zip(v, source["up"]))
    return tuple(-f * a - r * b + u * c for a, b, c in
                 zip(target["forward"], target["right"], target["up"]))


def portal_map_point(source, target, p):
    local = tuple(a - b for a, b in zip(p, source["origin"]))
    moved = portal_map_vector(source, target, local)
    return tuple(a + b for a, b in zip(target["origin"], moved))


def oriented(corners, normal):
    """`corners` wound counter-clockwise about `normal` (Newell's normal)."""
    n = [0.0, 0.0, 0.0]
    for a, b in zip(corners, corners[1:] + corners[:1]):
        n[0] += (a[1] - b[1]) * (a[2] + b[2])
        n[1] += (a[2] - b[2]) * (a[0] + b[0])
        n[2] += (a[0] - b[0]) * (a[1] + b[1])
    return corners if sum(x * y for x, y in zip(n, normal)) >= 0 else corners[::-1]


def portal_wall_faces(frame, r_range, u_range, half_width, half_height, segments):
    """The wall a portal sits on, as (wall faces, plug faces) in meters: the
    wall rectangle (r_range, u_range, relative to the portal's centre along
    its right and up) with the portal's opening cut out, and the plug that
    fills the opening. The opening is the portal's visible shape, the
    ellipse inscribed in its half_width x half_height rectangle."""
    o, r, u, f = frame["origin"], frame["right"], frame["up"], frame["forward"]

    def at(a, b):
        return tuple(o[k] + a * r[k] + b * u[k] for k in range(3))

    rs = (r_range[0], -half_width, half_width, r_range[1])
    us = (u_range[0], -half_height, half_height, u_range[1])
    if any(b < a for a, b in zip(rs, rs[1:])) or any(b < a for a, b in zip(us, us[1:])):
        raise ValueError("the portal's opening does not fit its wall")
    wall = []
    for i in range(3):
        for j in range(3):
            # The centre cell holds the opening; an opening flush with the
            # wall's edge leaves an empty row or column, which has no face.
            if (i, j) == (1, 1) or rs[i + 1] - rs[i] < 1e-9 or us[j + 1] - us[j] < 1e-9:
                continue
            wall.append((oriented([at(rs[i], us[j]), at(rs[i + 1], us[j]),
                                   at(rs[i + 1], us[j + 1]), at(rs[i], us[j + 1])], f), f))
    ring = [(half_width * math.cos(2 * math.pi * k / segments),
             half_height * math.sin(2 * math.pi * k / segments)) for k in range(segments)]
    quarter = segments // 4
    for q, (sr, su) in enumerate(((1, 1), (-1, 1), (-1, -1), (1, -1))):
        corner = at(sr * half_width, su * half_height)
        for k in range(q * quarter, (q + 1) * quarter):
            a, b = ring[k], ring[(k + 1) % segments]
            wall.append((oriented([corner, at(*a), at(*b)], f), f))
    plug = [(oriented([at(*p) for p in ring], f), f)]
    return wall, plug


PORTAL_HALF_WIDTH_UNITS = 32.0   # Portal's prop_portal (portal_shareddefs.h)
PORTAL_HALF_HEIGHT_UNITS = 54.0
PORTAL_SEGMENTS = 64
# The portals' glow (Portal 2's r_portal_use_dlights as an area light): the
# game's default portal colours (s_defaultPortalColors, portal_util_shared.cpp:
# the first portal blue, the second orange), linearized, at this peak
# radiance (lightmap unit), on a rectangle the size of the opening just in
# front of it.
PORTAL_GLOW_COLORS = {False: (64, 160, 255), True: (255, 160, 32)}
PORTAL_GLOW_RADIANCE = 2.0
PORTAL_GLOW_OFFSET_M = 0.002


def portal_pair(Scene, out):
    """Two sealed rooms joined only by a linked portal pair on perpendicular
    walls. `closed` is the portal world the map is built from; `open` is the
    Cycles reference: each room joined, through the opening, to a copy of the
    other placed behind its portal wall by the pair's transform (for one pair
    this is every path's exact equivalent, any number of crossings)."""
    import gi_fixtures as gf
    from pxr import UsdGeom
    s = Scene(out, "portal-pair", "Light through a linked portal pair on perpendicular walls: "
              "room A's ceiling rectangle and a spot aimed through portal A past a post light "
              "room B, which has only a dim lamp of its own, through the pair's elliptical "
              "opening; B's polished floor reflects what comes through, its probe sphere and "
              "block take the light, and from A the view through portal A shows B lit by A. "
              "Judged open (the rooms joined through the opening) and closed (sealed)",
              ["brdf", "area-lights", "runtime-lights", "direct-visibility",
               "indirect-diffuse-static", "indirect-diffuse-dynamic", "image-based-specular",
               "portal-transport"])
    s.pbr("Wall", (0.7, 0.7, 0.7), 0.9)
    s.pbr("Red", (0.63, 0.065, 0.05), 0.9)
    s.pbr("Teal", (0.08, 0.4, 0.38), 0.9)
    s.pbr("Post", (0.5, 0.5, 0.5), 0.7)
    s.pbr("FloorRough", (0.5, 0.5, 0.5), 0.6)
    s.pbr("FloorPolished", (0.5, 0.5, 0.5), 0.06)
    s.layout = "planar"
    half_w = PORTAL_HALF_WIDTH_UNITS / SOURCE_UNITS_PER_METER
    half_h = PORTAL_HALF_HEIGHT_UNITS / SOURCE_UNITS_PER_METER
    centre_z = 1.4
    # Room A: x 0..5, y 0..4; portal A in its east wall (x = 5) facing -X.
    # Room B: x 11..15, y 0..5; portal B in its south wall (y = 0) facing +Y.
    # Both portal walls are 4 x 3 m with the portal at their centre, so the
    # pair's transform maps one wall onto the other: the copy of B joined
    # behind A (x 5..10) and of A behind B (y -5..0) meet no other room.
    a = portal_frame((5.0, 2.0, centre_z), (-1.0, 0.0, 0.0))
    b = portal_frame((13.0, 0.0, centre_z), (0.0, 1.0, 0.0))
    wall_r, wall_u = (-2.0, 2.0), (-centre_z, 3.0 - centre_z)
    specs = {"A": [], "B": []}

    def room_faces(room, prefix, lo, hi, skip, materials):
        for face, corners, normal in gf.box_faces(lo, hi, inward=True, skip=skip):
            specs[room].append((prefix + "_" + face, [(corners, normal)],
                                materials.get(face, "Wall")))

    room_faces("A", "A", (0.0, 0.0, 0.0), (5.0, 4.0, 3.0), ("Xp",), {"Yn": "Red"})
    room_faces("B", "B", (11.0, 0.0, 0.0), (15.0, 5.0, 3.0), ("Yn", "Zn"), {"Xp": "Teal"})
    specs["B"] += [("FloorRoughB", [gf.quad((11.0, 0.0, 0.0), (15.0, 2.5, 0.0), 2, 1)],
                    "FloorRough"),
                   ("FloorPolishedB", [gf.quad((11.0, 2.5, 0.0), (15.0, 5.0, 0.0), 2, 1)],
                    "FloorPolished")]
    # A post between A's spot and portal A: its shadow crosses the pair.
    specs["A"].append(("PostA", [(c, n) for _, c, n in gi_gallery_box((2.95, 2.02, 0.0),
                                                                        (3.07, 2.14, 3.0))],
                       "Post"))
    specs["B"].append(("BlockB", [(c, n) for _, c, n in gi_gallery_box((13.8, 3.2, 0.0),
                                                                         (14.4, 3.8, 0.8))],
                       "Wall"))
    walls = {}
    for name, frame in (("A", a), ("B", b)):
        wall, plug = portal_wall_faces(frame, wall_r, wall_u, half_w, half_h, PORTAL_SEGMENTS)
        walls[name] = ("PortalWall" + name, "Plug" + name)
        s.mesh("PortalWall" + name, wall, "Wall")
        s.mesh("Plug" + name, plug, "Wall")
    for room, target, source in (("A", b, a), ("B", a, b)):
        for name, faces, material in specs[room]:
            s.mesh(name, faces, material)
    s.solids += ["PostA", "BlockB"]
    # The joined copies: room A behind portal B, room B behind portal A.
    # Each room's own portal wall separates it from the copy (they coincide
    # under the transform), so copies carry no portal wall.
    stage = s.author.stage
    copies = []
    for room, source, target in (("A", a, b), ("B", b, a)):
        for name, faces, material in specs[room]:
            moved = [([portal_map_point(source, target, c) for c in corners],
                      portal_map_vector(source, target, normal)) for corners, normal in faces]
            copies.append(s.mesh("Joined" + name, moved, material))
    s.probe("ProbeSphere", (12.2, 2.9, 0.9))
    copies.append(s.sphere("JoinedProbeSphere", portal_map_point(b, a, (12.2, 2.9, 0.9)),
                           gf.PROBE_RADIUS_M, "ProbeGrey"))
    for name in copies:
        UsdGeom.Imageable(stage.GetPrimAtPath("%s/World/%s" % (s.root, name))) \
            .CreateVisibilityAttr().Set(UsdGeom.Tokens.invisible)
    # Lights: room A's, room B's dim lamp, and the copies of A's behind B
    # (B's lamp lights B' behind A the same way).
    lamp_a = ("LampA", (2.5, 2.0, 2.99), (1.0, 1.0), (10.0, 10.0, 10.0))
    lamp_b = ("LampB", (13.0, 4.2, 2.99), (0.5, 0.5), (1.5, 1.5, 1.5))
    spot = ("SpotA", (1.0, 2.0, 2.8), normalize((4.0, 0.0, -1.9)),
            (20000.0, 19000.0, 17000.0), 9.0, 14.0)
    for name, center, size, radiance in (lamp_a, lamp_b):
        s.rect_light(name, center, size, radiance)
    s.spot_light(spot[0], spot[1], spot[2], spot[3], spot[4], spot[5])
    joined_lights = []
    for (name, center, size, radiance), source, target in ((lamp_a, a, b), (lamp_b, b, a)):
        peak, norm = peak_and_norm(radiance)
        s.rect("Joined" + name, portal_map_point(source, target, center), size, 0.0, norm,
               portal_map_vector(source, target, (0.0, 0.0, -1.0)))
        joined_lights.append(("Joined" + name, peak))
    peak, norm = peak_and_norm(spot[3])
    s.spot("Joined" + spot[0], portal_map_point(a, b, spot[1]), LIGHT_RADIUS_M, 0.0, norm,
           portal_map_vector(a, b, spot[2]), spot[4], spot[5])
    joined_lights.append(("Joined" + spot[0], peak))
    for name, _ in joined_lights:
        UsdGeom.Imageable(stage.GetPrimAtPath(s.light_path(name))).CreateVisibilityAttr() \
            .Set(UsdGeom.Tokens.invisible)
    # The portals' glow: a one-sided rectangle the size of each opening, just
    # in front of it, facing into its own room, in the portal's colour. It is
    # a runtime light (unbaked): declared as a light_rect entity, dark in
    # `closed` (the bake) and lit only in the glow states. It lights its own
    # side only, as the engine keeps it out of portal images
    # (DLIGHT_NO_PORTAL_IMAGE); the joined copy of each glow lights the copy
    # of its room behind the other portal.
    glows, glow_copies = [], []
    for name, frame, two, other in (("GlowA", a, False, b), ("GlowB", b, True, a)):
        radiance = tuple(PORTAL_GLOW_RADIANCE * (c / 255.0) ** 2.2
                         for c in PORTAL_GLOW_COLORS[two])
        center = tuple(o + PORTAL_GLOW_OFFSET_M * f
                       for o, f in zip(frame["origin"], frame["forward"]))
        s.rect_light(name, center, (2.0 * half_w, 2.0 * half_h), radiance,
                     direction=frame["forward"])
        stage.GetPrimAtPath(s.light_path(name)).GetAttribute("inputs:intensity").Set(0.0)
        peak, norm = peak_and_norm(radiance)
        glows.append((name, peak))
        s.rect("Joined" + name, portal_map_point(frame, other, center),
               (2.0 * half_w, 2.0 * half_h), 0.0, norm,
               portal_map_vector(frame, other, frame["forward"]))
        UsdGeom.Imageable(stage.GetPrimAtPath(s.light_path("Joined" + name))) \
            .CreateVisibilityAttr().Set(UsdGeom.Tokens.invisible)
        glow_copies.append(("Joined" + name, peak))
    s.portal("PortalA", a, False)
    s.portal("PortalB", b, True)
    joined_a = [n for n in copies if n.startswith("JoinedA") or n == "JoinedPostA"]
    joined_b = [n for n in copies if n not in joined_a]
    s.view("b-portal", (14.6, 4.7, 1.7), (12.5, 0.5, 0.7), {
        "floor_rough": ["FloorRoughB"], "floor_polished": ["FloorPolishedB"],
        "portal_wall": ["PortalWallB"], "walls": ["B_Xn", "B_Xp", "B_Zp"],
        "block": ["BlockB"], "probe": ["ProbeSphere"], "through": joined_a + ["PlugB"]})
    s.view("b-floor", (12.6, 4.8, 0.35), (13.4, 0.2, 1.0), {
        "floor_polished": ["FloorPolishedB"], "floor_rough": ["FloorRoughB"],
        "portal_wall": ["PortalWallB"], "walls": ["B_Xn", "B_Xp", "B_Zp"],
        "block": ["BlockB"], "through": joined_a + ["PlugB"]})
    s.view("a-portal", (0.5, 3.6, 1.7), (5.0, 1.6, 1.0), {
        "portal_wall": ["PortalWallA"], "walls": ["A_Xn", "A_Yn", "A_Yp", "A_Zn", "A_Zp"],
        "post": ["PostA"], "through": joined_b + ["PlugA"]})
    s.spawn = ((14.0, 4.2, 1.7), (12.5, 0.5, 0.7))
    s.base("closed", "the portal world: the pair closed (plugs in), the joined copies hidden; "
                     "the map is built and baked from it")
    opened = ([(s.author.root_path.AppendPath("World/" + walls[r][1]).pathString,
                "visibility", None, UsdGeom.Tokens.invisible) for r in ("A", "B")] +
              [("%s/World/%s" % (s.root, n), "visibility", None, UsdGeom.Tokens.inherited)
               for n in copies] +
              [(s.light_path(n), "visibility", None, UsdGeom.Tokens.inherited)
               for n, _ in joined_lights] +
              [e for n, peak in joined_lights for e in s.intensity(n, peak)])
    glowing = ([(s.light_path(n), "visibility", None, UsdGeom.Tokens.inherited)
                for n, _ in glow_copies] +
               [e for n, peak in glows + glow_copies for e in s.intensity(n, peak)])
    s.state("open", "the pair open: the plugs out, each room joined through the opening to "
                    "the copy of the other behind its portal wall", opened)
    s.state("open-glow", "the pair open with the portals' glow: each opening's rectangle "
                         "lights its own room (and its copy lights the copy of its room "
                         "behind the other portal)", opened + glowing)
    s.state("glow-only", "the pair open with only the portals' glow lit: the glow term "
                         "alone, and none of it through the pair",
            opened + glowing + s.off(lamp_a[0], lamp_b[0], spot[0]) +
            s.off(*[n for n, _ in joined_lights]))
    return s.finish_lighting(with_probes())


def door_room(Scene, out):
    """Two rooms joined by a doorway, and a door the bake never saw. The map
    is built and baked with the doorway open (the door hidden); the state
    `closed` shows the door in it. Room A's spot shines through the doorway
    onto room B's floor, and its ceiling lamp lights A. Closed, the door
    must take that light off B's floor with every runtime light's shadow,
    while the lightmap holds only indirect light (RFC 0016's runtime direct
    light): the case DirectOcclusion recomposed on the CPU."""
    s = Scene(out, "door-room", "A door the bake never saw: room A's spot shines through a "
              "doorway onto room B's floor. Baked open; judged open and with the door "
              "closed, when every runtime light's shadow must take the patch off B's floor "
              "and leave the lightmap's indirect light",
              ["runtime-lights", "area-lights", "direct-visibility", "moving-occluders",
               "indirect-diffuse-static", "brdf"])
    s.pbr("Wall", (0.7, 0.7, 0.7), 0.9)
    s.pbr("Floor", (0.45, 0.45, 0.45), 0.7)
    s.pbr("Blue", (0.1, 0.2, 0.55), 0.9)
    # Dark, so the door's own shading (the probe volume baked open) weighs
    # little; the doorway's reveals carry the material into the map.
    s.pbr("DoorDark", (0.05, 0.05, 0.05), 0.8)
    s.layout = "planar"
    outer = ((0.0, 0.0), (4.0, 3.0))
    hole = ((1.4, 0.0), (2.6, 2.2))
    s.room("A", (0.0, 0.0, 0.0), (4.0, 4.0, 3.0), "Wall", skip=("Xp", "Zn"),
           materials={"Yn": "Blue"})
    s.panel("FloorA", (0.0, 0.0, 0.0), (4.0, 4.0, 0.0), 2, 1, "Floor")
    s.wall("A_Xp", 4.0, 0, -1, outer, [hole], "Wall")
    s.doorway_x("Doorway", 4.0, 4.2, (1.4, 2.6), (0.0, 2.2), "DoorDark")
    s.room("B", (4.2, 0.0, 0.0), (8.2, 4.0, 3.0), "Wall", skip=("Xn", "Zn"))
    s.panel("FloorB", (4.2, 0.0, 0.0), (8.2, 4.0, 0.0), 2, 1, "Floor")
    s.wall("B_Xn", 4.2, 0, 1, outer, [hole], "Wall")
    closed = s.mover("Door", (4.05, 1.4, 0.0), (4.15, 2.6, 2.2), "DoorDark", ["closed"])
    # Room B's own lamp lights it: closing the door takes the spot's direct
    # light off its floor, and changes B's indirect light (which the baked-
    # open lightmap keeps) by much less. A's lamp is dim.
    s.rect_light("LampA", (1.5, 2.0, 2.99), (0.6, 0.6), (3.0, 3.0, 3.0))
    s.rect_light("LampB", (7.2, 2.0, 2.99), (1.2, 1.2), (6.0, 6.0, 6.0))
    # Aimed from high in A through the doorway's middle onto B's floor.
    s.spot_light("SpotA", (1.0, 2.0, 2.5), normalize((4.6, 0.0, -2.5)),
                 (30000.0, 28000.0, 25000.0), 10.0, 16.0)
    s.view("b-door", (7.9, 3.6, 1.7), (4.4, 1.6, 0.4), {
        "floor": ["FloorB"], "walls": ["B_Xn", "B_Yn", "B_Yp", "B_Xp", "B_Zp"],
        "doorway": ["Doorway"], "door": ["Door"]})
    s.view("a-door", (0.3, 0.4, 1.7), (4.0, 2.2, 0.8), {
        "floor": ["FloorA"], "walls": ["A_Xp", "A_Yn", "A_Yp", "A_Xn", "A_Zp"],
        "doorway": ["Doorway"], "door": ["Door"]})
    s.spawn = ((7.9, 3.6, 1.7), (4.4, 1.6, 0.4))
    s.base("open", "the doorway open (the door hidden): the map is built and baked from it")
    s.state("closed", "the door closed in the doorway: a moving object the bake never saw",
            closed)
    overrides = with_probes()
    overrides["lightmap"] = dict(overrides["lightmap"], directional=True,
                                 directional_samples=64)
    return s.finish_lighting(overrides)


def gi_gallery_box(lo, hi):
    """The outward faces of a solid box, as (name, corners, normal)."""
    import gi_gallery
    return gi_gallery.box_face_list(lo, hi, ())


SYNTHETIC = (cornell_floors, area_room, projector_cookie, sun_colonnade, foggy_hall,
             mirror_corridor, material_sweep, portal_pair, door_room)

# The two chambers reuse existing maps and their derived relight scenes; the
# stages are build products (untracked), so the fixture records their paths,
# digests at generation and the commands that rebuild them.
CHAMBERS = (
    {"name": "portal-chamber", "map": "testchmb_a_00_relit",
     "purpose": "Portal's testchmb_a_00 relit by the one lighting back end: its world "
                "faces, VMT materials and vrad world lights as a derived USD scene",
     "stage": "quality-results/relight/testchmb_a_00_relit-preview/legacy-scene/scene.usda",
     "rebuild": "python3 tools/quality/legacy_bsp_relight.py --bsp "
                "<portal install>/portal/maps/testchmb_a_00.bsp --map testchmb_a_00_relit "
                "--quality legacy-relight-preview --out "
                "quality-results/relight/testchmb_a_00_relit-preview",
     # Eye (Source units) and engine angles (pitch down, yaw): the K0 view
     # oracles' poses (quality/workloads/render-view-oracles-v1.json) at eye
     # height: the relaxation vault's glass from the spawn, and the second
     # room where the recursion shots open their portals.
     "cameras": {"vault": ((-538.0, -367.0, 225.0), (4.0, 8.0)),
                 "room2": ((-1088.0, -560.0, 644.0), (12.0, 0.0))},
     "terms": ["brdf", "runtime-lights", "direct-visibility", "indirect-diffuse-static",
               "image-based-specular", "emission"]},
    {"name": "portal2-chamber", "map": "sp_gi_chamber_01",
     "purpose": "The clean-Aperture Portal 2 test chamber sp_gi_chamber_01 built by "
                "portal2_gi_chamber.py: triple-laser tiles ($ssbump as normal and AO), "
                "light_spot panels and switchable light_dynamic accents",
     "stage": "quality-results/portal2-maps/sp_gi_chamber_01-preview/relight/legacy-scene/"
              "scene.usda",
     "rebuild": "python3 tools/quality/portal2_gi_chamber.py (profile portal2-chamber-preview)",
     # The spawn (info_player_start at eye height) looking into the chamber,
     # and a view across it toward the exit.
     "cameras": {"spawn": ((-1104.0, 0.0, 64.0), (2.2, 0.0)),
                 "chamber": ((-640.0, -150.0, 150.0), (13.6, 41.4))},
     "terms": ["brdf", "runtime-lights", "direct-visibility", "indirect-diffuse-static",
               "image-based-specular", "emission", "ambient-occlusion"]},
)


def chamber_fixture(out, chamber):
    import gi_fixtures as gf
    directory = out / chamber["name"]
    directory.mkdir(parents=True, exist_ok=True)
    meters = 1.0 / SOURCE_UNITS_PER_METER
    cameras = {}
    for name, (eye, (pitch, yaw)) in chamber["cameras"].items():
        forward = angle_vectors(pitch, yaw)[0]
        target = [e + 400.0 * f for e, f in zip(eye, forward)]
        cameras[name] = gf.camera_pose([c * meters for c in eye], [c * meters for c in target])
    record = {"schema": "gi-fixture/v1", "name": chamber["name"], "family": "lighting",
              "purpose": chamber["purpose"], "stage": None,
              "cycles_receiver_oracle": True,
              "external_stage": {"path": chamber["stage"], "rebuild": chamber["rebuild"],
                                 "units": "Source units (metersPerUnit 0.0254)"},
              "states": {"default": {"layer": None, "note": "the map's lights as compiled"}},
              "baked_state": "default", "cameras": cameras, "film": FILM,
              "horizontal_fov_degrees": HORIZONTAL_FOV, "lambertian": False,
              "regions": {name: {} for name in cameras}, "dynamic_models": [],
              "map_manifest": None, "oracles": [],
              "lighting": {"terms": chamber["terms"], "entities": None,
                           "entity_lump": "the map's own (stock Source lights)",
                           "projectors": [], "media": {},
                           "map": {"name": chamber["map"],
                                   "bsp": "run/maps/%s/maps/%s.bsp" % (chamber["map"],
                                                                       chamber["map"]),
                                   "baked_state": "default", "overrides": None,
                                   "reused": True}}}
    write_json(directory / "fixture.json", record)
    return record


def manifest_record(records):
    fixtures = []
    for record in records:
        lighting = record["lighting"]
        fixtures.append({
            "name": record["name"], "directory": record["name"], "purpose": record["purpose"],
            "cycles_receiver_oracle": record["cycles_receiver_oracle"],
            "terms": lighting["terms"], "states": sorted(record["states"]),
            "cameras": sorted(record["cameras"]), "map": lighting["map"]["name"],
            "bsp": lighting["map"]["bsp"], "reused_map": bool(lighting["map"].get("reused")),
            **({"state_maps": {state: entry["name"] for state, entry in
                               sorted(lighting["state_maps"].items())}}
               if lighting.get("state_maps") else {}),
            "entities": lighting.get("entities"), "lights": lighting.get("lights"),
            "projectors": len(lighting.get("projectors", [])),
            "media": sorted(s for s, m in lighting.get("media", {}).items() if m)})
    used = {t for f in fixtures for t in f["terms"]}
    terms = {}
    for term, entry in TERMS.items():
        terms[term] = dict(entry, fixtures=[f["name"] for f in fixtures if term in f["terms"]])
    missing = [t for t in TERMS if t not in used and "judged_by" not in TERMS[t]]
    if missing:
        raise ValueError("terms no fixture exercises: %s" % missing)
    return {"schema": MANIFEST_SCHEMA, "version": 1,
            "gate": "RFC/0016-render-core.md#k11-lighting-model-proven-in-render_lab",
            "generator": "tools/quality/lighting_fixtures.py",
            "extends": "quality/fixtures/gi/ (RFC 0011 gallery)",
            "units": {"stage": "meters, Z up (metersPerUnit 1) for the synthetic fixtures",
                      "map": "Source units (%.8f per meter)" % SOURCE_UNITS_PER_METER,
                      "light": "the pipeline's lightmap unit: a white Lambertian surface under "
                               "irradiance E shows E / pi (Cycles diffuse passes)"},
            "film": FILM, "horizontal_fov_degrees": HORIZONTAL_FOV,
            "reference_policy": {
                "renderer": "tools/quality/lighting_reference_blender.py over "
                            "gi_reference_blender.py (usd_scene.py extract, pbrt_blender "
                            "material and emitter policy)",
                "light_paths": "gi-reference (64 diffuse, 16 glossy bounces, no clamping, no "
                               "volume bounces: single scattering)",
                "device": "cpu", "seed": SEED, "denoising": False,
                "preview_samples": PREVIEW_SAMPLES, "final_samples": FINAL_SAMPLES,
                "preview_is_diagnostic": "a comparison against a preview reference is "
                                         "recorded but certifies nothing"},
            "tolerances": "tolerances.json", "error_metric": ERROR_METRIC,
            "carrier_entities": {
                RECT_CLASS: "a rectangle light: origin (centre), angles (forward = the "
                            "emission direction, the rectangle's normal), width along -right and height "
                            "along up (halfU = -right x width/2, halfV = up x height/2) "
                            "in Source units, color (linear 0..255) and brightness "
                            "(radiance = color / 255 x brightness, lightmap unit), two_sided; "
                            "render.area-light.v1's Rect and AreaLight",
                MEDIUM_CLASS: "a homogeneous box medium: origin, box_mins and box_maxs "
                              "(relative), density (extinction per Source unit), albedo "
                              "(scattering / extinction), anisotropy (Henyey-Greenstein g), "
                              "emission",
                FOG_CONTROLLER_CLASS: "the one global volumetric fog controller: density, "
                                      "height_fog_density, height_fog_falloff, anisotropy",
                "status": "decided by the render-core owner (source-engine-43, 2026-09-29, "
                          "binding rule 5) with Source 2's names; the game entities are "
                          "RFC 0016 K12's"},
            "terms": terms, "fixtures": fixtures}


def generate(out):
    Scene = lighting_scene_class()
    records = [build(Scene, out) for build in SYNTHETIC]
    records += [chamber_fixture(out, chamber) for chamber in CHAMBERS]
    write_json(out / "index.json", {"schema": "gi-fixture-index/v1",
                                    "fixtures": sorted(r["name"] for r in records),
                                    "generator": "tools/quality/lighting_fixtures.py"})
    write_json(out / "manifest.json", manifest_record(records))
    return records


# Hand-kept (tolerances.json, README.md) or written by other commands
# (references/, results/): not compared by `generate --check`.
NOT_GENERATED = {"tolerances.json", "README.md", "references", "results"}


def compare_trees(expected, actual):
    differences = []
    names = {p.relative_to(expected) for p in expected.rglob("*") if p.is_file()} | \
        {p.relative_to(actual) for p in actual.rglob("*") if p.is_file()}
    for name in sorted(names):
        if NOT_GENERATED & set(name.parts):
            continue
        a, b = expected / name, actual / name
        if not a.is_file() or not b.is_file() or not filecmp.cmp(a, b, shallow=False):
            differences.append(str(name))
    return differences


def cmd_generate(args):
    if args.check:
        with tempfile.TemporaryDirectory() as temporary:
            generated = Path(temporary) / "lighting"
            generate(generated)
            differences = compare_trees(args.out, generated)
        if differences:
            print("lighting fixtures differ from the generator: " + ", ".join(differences))
            return 1
        print("lighting fixtures match the generator")
        return 0
    generate(args.out)
    print("wrote lighting fixtures to " + str(args.out))
    return 0


# ================================================================== fixtures

def load_manifest(root=None):
    root = Path(root or FIXTURES)
    manifest = json.loads((root / "manifest.json").read_text())
    if manifest.get("schema") != MANIFEST_SCHEMA:
        raise ValueError("%s: not a %s manifest" % (root / "manifest.json", MANIFEST_SCHEMA))
    return manifest


def cycles_oracle_names(requested=None, root=None):
    """Names with meaningful Cycles receiver references for the K11 gallery."""
    eligible = {entry["name"] for entry in load_manifest(root)["fixtures"]
                if entry.get("cycles_receiver_oracle", True)}
    if requested:
        excluded = sorted(set(requested) - eligible)
        if excluded:
            raise ValueError("not Cycles receiver oracles: %s" % ", ".join(excluded))
        return requested
    return sorted(eligible)


def reference_status(samples, denoised):
    """preview, denoised (OpenImageDenoise at DENOISED_SAMPLES or more) or final
    (unbiased at FINAL_SAMPLES or more)."""
    if samples >= FINAL_SAMPLES and not denoised:
        return "final"
    if denoised and samples >= DENOISED_SAMPLES:
        return "denoised"
    return "preview"


def load_fixture(name, root=None):
    root = Path(root or FIXTURES)
    path = root / name / "fixture.json"
    fixture = json.loads(path.read_text())
    if fixture.get("name") != name or "lighting" not in fixture:
        raise ValueError("invalid lighting fixture " + str(path))
    fixture["directory"] = path.parent
    return fixture


REFERENCE_FIELDS = ("stage", "external_stage", "states", "cameras", "film",
                    "horizontal_fov_degrees", "dynamic_models")


def reference_digest(fixture):
    fields = {key: fixture.get(key) for key in REFERENCE_FIELDS}
    lighting = fixture["lighting"]
    fields["projectors"] = lighting.get("projectors")
    fields["media"] = lighting.get("media")
    return digest_json(fields)


def map_for(fixture, state):
    """The map a state renders from: its own (lighting.state_maps), else the
    fixture's."""
    lighting = fixture["lighting"]
    return (lighting.get("state_maps") or {}).get(state) or lighting["map"]


def stage_path(fixture, state):
    if fixture.get("external_stage"):
        return ROOT / fixture["external_stage"]["path"]
    layer = fixture["states"][state]["layer"]
    return fixture["directory"] / (layer or fixture["stage"])


# ================================================================== render

def emitter_indices(object_index):
    return sorted(number for name, number in object_index.items()
                  if name.startswith(EMITTER_PREFIXES))


def judged_mask(index_image, emitters):
    import numpy as np
    return (index_image > 0) & ~np.isin(index_image, emitters)


def emitter_mask(index_image, emitters):
    import numpy as np
    return (index_image > 0) & np.isin(index_image, emitters)


def luminance(rgb):
    return 0.2126 * rgb[..., 0] + 0.7152 * rgb[..., 1] + 0.0722 * rgb[..., 2]


def projector_irradiance(projector, positions_units, normals, cookie):
    """render.projected-light.v1 IrradianceAt (unshadowed) per pixel, the
    cookie sampled bilinearly with clamp (numpy mirror of the contract)."""
    import numpy as np
    origin = np.asarray(projector["origin_units"], np.float64)
    forward, right, up = (np.asarray(projector[k], np.float64) for k in ("forward", "right",
                                                                           "up"))
    d = positions_units - origin
    z = d @ forward
    tan_h = math.tan(math.radians(projector["horizontal_fov_degrees"]) / 2)
    tan_v = math.tan(math.radians(projector["vertical_fov_degrees"]) / 2)
    with np.errstate(divide="ignore", invalid="ignore"):
        x = (d @ right) / (z * tan_h)
        y = (d @ up) / (z * tan_v)
    inside = (z > projector["near_z"]) & (z <= projector["far_z"]) & \
        (np.abs(x) <= 1) & (np.abs(y) <= 1)
    distance = np.linalg.norm(d, axis=-1)
    to_light = -d / np.maximum(distance, 1e-9)[..., None]
    lambert = np.sum(to_light * normals, axis=-1)
    c, l, q = projector["attenuation"]
    with np.errstate(divide="ignore", invalid="ignore"):
        atten = np.clip(c + l / distance + q / distance ** 2, 0.0, 1.0)
    far = projector["far_z"]
    end = np.clip((distance - far) / (0.6 * far - far), 0.0, 1.0)
    u = np.nan_to_num(0.5 + 0.5 * x)
    v = np.nan_to_num(0.5 - 0.5 * y)
    height, width = cookie.shape[:2]
    fx = np.clip(u * width - 0.5, 0, width - 1)
    fy = np.clip(v * height - 0.5, 0, height - 1)
    x0, y0 = np.floor(fx).astype(int), np.floor(fy).astype(int)
    x1, y1 = np.minimum(x0 + 1, width - 1), np.minimum(y0 + 1, height - 1)
    ax, ay = (fx - x0)[..., None], (fy - y0)[..., None]
    sample = ((cookie[y0, x0] * (1 - ax) + cookie[y0, x1] * ax) * (1 - ay) +
              (cookie[y1, x0] * (1 - ax) + cookie[y1, x1] * ax) * ay)
    scale = np.where(inside & (lambert > 0), atten * end * lambert, 0.0)
    return np.asarray(projector["color"])[None, None, :] * sample * scale[..., None]


def projector_check(fixture, passes, object_index, projector):
    """The rendered direct projected light against the contract's formula on
    the Lambertian floor and back wall: the median ratio over the lit
    unshadowed pixels within 3% of 1 and 80% of them within 10%."""
    import numpy as np
    from PIL import Image
    cookie = np.asarray(Image.open(fixture["directory"] / projector["cookie_image"]),
                        np.float64)[..., :3] / 255.0
    positions = passes["Position"][..., :3].astype(np.float64) * SOURCE_UNITS_PER_METER
    normals = passes["Normal"][..., :3].astype(np.float64)
    expected = projector_irradiance(projector, positions, normals, cookie)
    index = np.rint(passes["IndexOB"][..., 0]).astype(np.int32)
    receivers = [object_index[name] for name in ("Floor", "BackWall") if name in object_index]
    observed = passes["DiffDir"][..., :3].astype(np.float64)
    mask = np.isin(index, receivers) & (luminance(expected) > 0.05) & \
        (luminance(observed) > 0.02)
    ratio = luminance(observed[mask]) / luminance(expected[mask])
    count = int(mask.sum())
    median = float(np.median(ratio)) if count else 0.0
    within = float(np.mean(np.abs(ratio - 1.0) < 0.1)) if count else 0.0
    return {"check": "projector direct light = render.projected-light.v1 IrradianceAt",
            "pixels": count, "median_ratio": median, "within_10_percent": within,
            "ok": count >= 500 and abs(median - 1.0) <= 0.03 and within >= 0.8}


def cmd_render(args):
    import numpy as np
    import gi_reference
    import map_scene
    tools = gi_reference.Tools(args.toolchain)
    status = 0
    names = cycles_oracle_names(args.fixture)
    for name in names:
        fixture = load_fixture(name)
        results = {}
        for state in sorted(fixture["states"]):
            print("[%s/%s] rendering at %d samples..." % (name, state, args.samples),
                  flush=True)
            results[state] = render_state(tools, fixture, state, args.work / name,
                                          args.samples, args.seed, args.device, map_scene, np,
                                          denoise=args.denoise)
        record = write_references(fixture, results, args.samples, args.seed, np, gi_reference)
        print("[%s] %s (%s)" % (name, record["status"], "; ".join(
            "%s: %s" % (c["check"], "ok" if c["ok"] else "FAIL") for c in record["analytic"])
            or "no closed form"), flush=True)
        status |= record["status"] not in REFERENCE_STATUSES
    return status


def render_state(tools, fixture, state, work, samples, seed, device, map_scene, np,
                 denoise=False):
    directory = work / state
    if directory.exists():
        shutil.rmtree(directory)
    directory.mkdir(parents=True)
    log = directory / "log.txt"
    scene_path, normalized = directory / "scene.json", directory / "stage.usdc"
    tools.usd_python("usd_scene.py", ["extract", "--scene", stage_path(fixture, state),
                                      "--out-scene", scene_path, "--out-stage", normalized],
                     log)
    scene = map_scene.parse(scene_path)
    arguments = ["--scene", scene_path, "--stage", normalized]
    if scene["environment"]:
        environment = directory / "environment.exr"
        # Exact float32: imageio stores lossy half floats (blocks of zeros seen).
        write_exr(environment, map_scene.environment_equirect(scene, 2048))
        arguments += ["--environment", environment]
    cameras = directory / "cameras.json"
    write_json(cameras, {"film": fixture["film"],
                         "horizontal_fov_degrees": fixture["horizontal_fov_degrees"],
                         "cameras": fixture["cameras"]})
    extras = directory / "extras.json"
    lighting = fixture["lighting"]
    write_json(extras, {"fixture_directory": str(fixture["directory"]),
                        "projectors": lighting.get("projectors", []),
                        "medium": (lighting.get("media") or {}).get(state)})
    # Tools.blender passes the environment on; the wrapper reads its extras here.
    os.environ["LIGHTING_EXTRAS"] = str(extras)
    try:
        tools.blender("lighting_reference_blender.py", arguments + [
            "--cameras", cameras, "--out-dir", directory / "render", "--samples", str(samples),
            "--seed", str(seed), "--device", device, "--light-paths", "gi-reference"] + (
            ["--denoise"] if denoise else []), log)
    finally:
        os.environ.pop("LIGHTING_EXTRAS", None)
    receipt = json.loads((directory / "render" / "render.json").read_text())
    receipt["lighting_extras"] = json.loads(
        (directory / "render" / "lighting-extras.json").read_text())
    return receipt, scene, directory / "render"


def denoise_direct(color, albedo, normal):
    """OpenImageDenoise's RT filter (HDR, with the albedo and normal passes as
    auxiliary images) over a camera image: the direct diffuse reference of a
    denoised render, whose Combined pass Cycles denoised the same way."""
    import ctypes
    import numpy as np
    import lightmap_denoise
    library = lightmap_denoise.load_oidn("libOpenImageDenoise.so.2")
    library.oidnSetFilterBool.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_bool]
    height, width, _ = color.shape
    images = {name: np.ascontiguousarray(image[..., :3], dtype=np.float32)
              for name, image in (("color", color), ("albedo", albedo), ("normal", normal))}
    images["output"] = np.empty_like(images["color"])
    device = library.oidnNewDevice(lightmap_denoise.OIDN_DEVICE_TYPE_CPU)
    if not device:
        raise RuntimeError("OIDN could not create a CPU device")
    try:
        library.oidnCommitDevice(device)
        oidn_filter = library.oidnNewFilter(device, b"RT")
        try:
            for name, image in images.items():
                library.oidnSetSharedFilterImage(oidn_filter, name.encode(), image.ctypes.data,
                                                 lightmap_denoise.OIDN_FORMAT_FLOAT3, width,
                                                 height, 0, 0, 0)
            library.oidnSetFilterBool(oidn_filter, b"hdr", True)
            library.oidnCommitFilter(oidn_filter)
            library.oidnExecuteFilter(oidn_filter)
            message = ctypes.c_char_p()
            if library.oidnGetDeviceError(device, ctypes.byref(message)):
                raise RuntimeError("OIDN failed: " + (message.value or b"").decode())
        finally:
            library.oidnReleaseFilter(oidn_filter)
    finally:
        library.oidnReleaseDevice(device)
    return np.maximum(images["output"].astype(np.float64), 0.0)


def write_references(fixture, results, samples, seed, np, gi_reference):
    from PIL import Image
    references = fixture["directory"] / "references"
    if references.exists():
        shutil.rmtree(references)
    references.mkdir()
    views, renders, analytic = {}, {}, []
    media = fixture["lighting"].get("media") or {}
    clear = next((s for s in sorted(results) if not media.get(s)), None)
    # Object indices come from the camera's first surface hit; in a medium
    # samples scatter first and leave the index at 0, so a medium state takes
    # the index of a medium-free state of the same geometry and cameras.
    clear_index = {}
    if clear and any(media.get(s) for s in results):
        receipt, _, render_dir = results[clear]
        for camera, output in receipt["renders"].items():
            passes = gi_reference.render_passes(render_dir / output["exr"])
            clear_index[camera] = (np.rint(passes["IndexOB"][..., 0]).astype(np.int32),
                                   receipt["object_index"])
    for state, (receipt, scene, render_dir) in sorted(results.items()):
        renders[state] = {key: receipt[key] for key in (
            "blender", "cycles_device", "samples", "seed", "denoising", "light_paths",
            "normal_maps", "renderer_sha256", "ocio_configuration_sha256")}
        renders[state]["lighting_renderer_sha256"] = \
            receipt["lighting_extras"]["renderer_sha256"]
        renders[state]["extras_applied"] = receipt["lighting_extras"]["applied"]
        renders[state]["composed_stage_sha256"] = scene["source_sha256"]
        renders[state]["stage_layers"] = {
            (str(Path(path).relative_to(ROOT)) if Path(path).is_relative_to(ROOT)
             else str(path)): sha256(path)
            for path in scene["source_files"] if Path(path).is_file() and
            Path(path).suffix.startswith(".usd")}
        for camera, output in sorted(receipt["renders"].items()):
            passes = gi_reference.render_passes(render_dir / output["exr"])
            index_image = np.rint(passes["IndexOB"][..., 0]).astype(np.int32)
            index_from = state
            if media.get(state) and camera in clear_index:
                index_image, clear_objects = clear_index[camera]
                if clear_objects != receipt["object_index"]:
                    raise ValueError("%s: states %s and %s index different objects" % (
                        fixture["name"], clear, state))
                index_from = clear
            if index_image.max() > 65535:
                raise ValueError("more than 65535 objects in a view")
            stem = "%s.%s" % (state, camera)
            total = references / ("%s.total.exr" % stem)
            # Half floats: relative precision 5e-4, far below any tolerance.
            combined = passes["Combined"][..., :3]
            if np.abs(combined).max() > 65000:
                raise ValueError("%s exceeds the half-float range" % total)
            write_exr(total, combined, half=True)
            if not np.array_equal(read_rgb_exr(total), combined.astype(np.float16)
                                  .astype(np.float64)):
                raise ValueError("%s does not read back exactly" % total)
            # Historical offline direct-diffuse reference (Cycles DiffDir x DiffCol).
            # It is not a render_lab comparison or gate.
            direct_path = references / ("%s.direct.exr" % stem)
            direct = (passes["DiffDir"][..., :3] * passes["DiffCol"][..., :3]).astype(np.float64)
            if receipt.get("denoising") and "Normal" in passes:
                direct = denoise_direct(direct, passes["DiffCol"], passes["Normal"])
            write_exr(direct_path, direct, half=True)
            index_path = references / ("%s.index.png" % stem)
            Image.fromarray(index_image.astype(np.uint16)).save(index_path, optimize=False)
            emitters = emitter_indices(receipt["object_index"])
            mask = judged_mask(index_image, emitters)
            combined = passes["Combined"][..., :3].astype(np.float64)
            views[stem] = {
                "state": state, "camera": camera,
                "files": {"total": {"file": total.name, "sha256": sha256(total)},
                          "direct": {"file": direct_path.name, "sha256": sha256(direct_path)},
                          "index": {"file": index_path.name, "sha256": sha256(index_path)}},
                "object_index": receipt["object_index"], "emitter_indices": emitters,
                "index_from_state": index_from,
                "judged_pixels": int(mask.sum()),
                "mean_luminance": float(luminance(combined[mask]).mean()) if mask.any()
                else 0.0,
                "finite": bool(np.isfinite(combined).all())}
            lighting = fixture["lighting"]
            if lighting.get("projectors") and state == "projector":
                check = projector_check(fixture, passes, receipt["object_index"],
                                        lighting["projectors"][0])
                check["view"] = stem
                analytic.append(check)
    denoised = all(bool(r.get("denoising")) for r in renders.values()) if renders else False
    status = reference_status(samples, denoised)
    if not all(c["ok"] for c in analytic) or not all(v["finite"] for v in views.values()):
        status = "fail"
    record = {"schema": REFERENCES_SCHEMA, "fixture": fixture["name"],
              "fixture_reference_digest": reference_digest(fixture), "samples": samples,
              "seed": seed, "denoised": denoised, "status": status, "renders": renders,
              "views": views,
              "analytic": analytic,
              "light_units": "the pipeline's lightmap unit (Cycles E / pi for a white "
                             "Lambertian); total = Combined, linear scene radiance"}
    write_json(references / "references.json", record)
    return record


# ================================================================== build

def cmd_build(args):
    import gi_reference
    import legacy_bsp
    import map_lighting
    import vmf_map_build
    tools = gi_reference.Tools(args.toolchain)
    toolchain = map_lighting.load_toolchain(args.toolchain)
    compile_tools = Path(toolchain["compile_tools"])
    names = args.fixture or [f["name"] for f in load_manifest()["fixtures"]
                             if not f["reused_map"]]
    status = 0
    for name in names:
        fixture = load_fixture(name)
        lighting = fixture["lighting"]
        if lighting["map"].get("reused"):
            print("[%s] reuses %s; nothing to build" % (name, lighting["map"]["name"]))
            continue
        work = (args.work / name).resolve()
        if work.exists() and not args.keep:
            shutil.rmtree(work)
        work.mkdir(parents=True, exist_ok=True)
        log = work / "front-end.log"
        entities = json.loads((fixture["directory"] / "entities.json").read_text())
        map_name = lighting["map"]["name"]
        baked = stage_path(fixture, lighting["map"]["baked_state"])
        # Front end: the scene front end's collision VMF plus the fixture's
        # entities (the lights the lab reads), compiled by vmf_map_build.
        scene_json, normalized = work / "scene.json", work / "stage.usdc"
        tools.usd_python("usd_scene.py", ["extract", "--scene", baked, "--out-scene",
                                          scene_json, "--out-stage", normalized], log)
        collision = work / "collision"
        if collision.exists():
            shutil.rmtree(collision)
        solid = [item for mesh in lighting["map"]["solid_meshes"]
                 for item in ("--solid-mesh", mesh)]
        tools.usd_python("pbrt_collision_vmf.py", ["--scene", scene_json, "--stage", normalized,
                                                   "--map-name", map_name, "--out-dir",
                                                   collision, "--no-fallback-light"] + solid,
                         log)
        vmf = work / (map_name + ".vmf")
        vmf.write_text(append_entities((collision / (map_name + "_collision.vmf")).read_text(),
                                       entities["entities"]))
        record = vmf_map_build.build(vmf, work / "compile", compile_tools,
                                     (ROOT / "run/runtime").resolve(), "fast", map_name)
        if record["status"] != "pass":
            print("[%s] compile %s (%s)" % (name, record["status"], work / "compile"))
            status = 1
            continue
        bsp = Path(record["content_root"]) / "maps" / (map_name + ".bsp")
        compiled, _ = legacy_bsp.LegacyBsp.read(bsp).world_lights()
        problems = compare_world_lights(expected_world_lights(entities["lights"]), compiled)
        write_json(work / "world-lights.json", {"compiled": len(compiled),
                                                "problems": problems})
        if problems:
            print("[%s] compiled world lights differ from the stage:\n  %s" % (
                name, "\n  ".join(problems[:10])))
            status = 1
            continue
        # Back end: the authored scene lights the compiled map (bsp + scene).
        overrides = final_overrides(lighting["map"]["overrides"]) if args.final else \
            dict(lighting["map"]["overrides"] or {})
        if lighting["map"].get("layout"):
            overrides.setdefault("lightmap", {})
            overrides["lightmap"] = dict(overrides["lightmap"], layout=lighting["map"]["layout"])
        identity = map_lighting.light(bsp, map_name, work / "lighting", toolchain, scene=baked,
                                      quality="gi-fixture", extra=overrides,
                                      keep_going=args.keep_going, device=args.device)
        cookies = install_cookies(fixture, map_name, tools, toolchain)
        # States with their own map: the same compiled BSP and baked state, the
        # lightmap bake crossing the state's medium (map_lighting's `medium`).
        state_maps = {}
        for state, entry in sorted((lighting.get("state_maps") or {}).items()):
            medium = (lighting.get("media") or {}).get(state)
            state_identity = map_lighting.light(
                bsp, entry["name"], work / ("lighting-" + state), toolchain, scene=baked,
                quality="gi-fixture", extra=overrides, keep_going=args.keep_going,
                medium=medium, device=args.device)
            state_maps[state] = {"map": entry["name"], "medium": medium,
                                 "identity": state_identity.get("status"),
                                 "cookies": install_cookies(fixture, entry["name"], tools,
                                                            toolchain)}
        write_json(work / "build.json", {"schema": "lighting-fixture-build/v1",
                                         "fixture": name, "map": map_name,
                                         "bake": "final" if args.final else "preview",
                                         "identity": identity.get("status"),
                                         "world_lights": len(compiled), "cookies": cookies,
                                         "overrides": overrides, "state_maps": state_maps})
        print("[%s] built and published %s%s (identity %s)" % (
            name, map_name, "".join(", " + e["map"] for e in state_maps.values()),
            identity.get("status")))
    return status


# The final bakes: the gi-fixture profile's samples (lightmap 2048, probe
# volume 4096), the reflection-fixture profile's probes, and the profile's
# radiosity transfer (RTRN) and SDF volume (SDFV), which the previews switch off.
FINAL_PROBES = {"width": 1024, "face_size": 256, "samples": 64, "light_paths": "gi-reference"}


def final_overrides(preview):
    overrides = {}
    for key, value in (preview or {}).items():
        if isinstance(value, dict) and key in ("lightmap", "probe_volume"):
            value = {k: v for k, v in value.items() if k != "samples"}
            if not value:
                continue
        elif key == "reflection_probe" and value:
            value = dict(FINAL_PROBES)
        elif key in ("radiosity", "sdf_volume") and value is None:
            continue  # the profile's own setting
        overrides[key] = value
    return overrides


def append_entities(vmf_text, entities, first_id=1000):
    lines = [vmf_text.rstrip("\n")]
    for offset, keys in enumerate(entities):
        lines += ["entity", "{", '\t"id" "%d"' % (first_id + offset)]
        lines += ['\t"%s" "%s"' % (key, value) for key, value in keys.items()]
        lines += ["}"]
    return "\n".join(lines) + "\n"


def install_cookies(fixture, map_name, tools, toolchain):
    """Cookie textures in the published map's content (VTEX from the PNG,
    linear, as the texture the entity names)."""
    installed = []
    for projector in fixture["lighting"].get("projectors", []):
        source = fixture["directory"] / projector["cookie_image"]
        vtex = Path(toolchain["compile_tools"]) / "vtex"
        target = ROOT / "run/maps" / map_name / "materials" / (projector["texture"] + ".vtf")
        if not Path(vtex).is_file():
            installed.append({"texture": projector["texture"], "status": "no vtex in the "
                              "toolchain; the cookie PNG is the fixture's authority"})
            continue
        from PIL import Image
        from vtf_content import compile_texture
        target.parent.mkdir(parents=True, exist_ok=True)
        # Uncompressed: the mullions' hard edges are part of the pattern.
        target.with_suffix(".txt").write_text("nocompress 1\n")
        compile_texture(Image.open(source).convert("RGB"), target, vtex)
        target.with_suffix(".txt").unlink()
        installed.append({"texture": projector["texture"], "vtf": str(target.relative_to(ROOT)),
                          "sha256": sha256(target)})
    return installed


# ================================================================== check

def load_tolerances(root=None):
    root = Path(root or FIXTURES)
    ledger = json.loads((root / "tolerances.json").read_text())
    if ledger.get("schema") != TOLERANCES_SCHEMA:
        raise ValueError("tolerances.json: not a %s ledger" % TOLERANCES_SCHEMA)
    return ledger


def tolerance_digest(entry):
    return digest_json({k: entry[k] for k in ("fixture", "version", "mean", "p99", "fixed")})


def current_tolerance(ledger, fixture):
    entries = [e for e in ledger["entries"] if e["fixture"] == fixture]
    return max(entries, key=lambda e: e["version"]) if entries else None


def parse_time(text):
    return datetime.datetime.fromisoformat(text)


def map_entities(bsp):
    """The entity lump of a published BSP2 (or legacy BSP) as key-value dicts."""
    import re
    import bsp2_reader
    data = Path(bsp).read_bytes()
    kind, parsed = bsp2_reader.open_any(data)
    if kind == "bsp2":
        text = parsed.legacy_lump(0)
    else:
        offset, length = parsed["lumps"][0][:2]
        text = data[offset:offset + length]
    entities, current = [], None
    for match in re.finditer(r'"([^"]*)"\s*"([^"]*)"|([{}])',
                             text.decode("latin-1").rstrip("\0")):
        if match.group(3) == "{":
            current = {}
        elif match.group(3) == "}":
            entities.append(current)
            current = None
        elif current is not None:
            current[match.group(1)] = match.group(2)
    return entities


LIGHT_CLASSES = ("light", "light_spot", "light_environment", "env_projectedtexture",
                 RECT_CLASS, MEDIUM_CLASS, FOG_CONTROLLER_CLASS)
# The entities a fixture declares in entities.json: its lights, and the
# portal pairs light travels through (portal-transport).
FIXTURE_CLASSES = LIGHT_CLASSES + ("prop_portal",)


def check_map(fixture, root=None):
    """Every published map of the fixture (its own and its states') exists,
    carries exactly the fixture's light and portal entities (for reused maps:
    that it exists), and was baked with the medium it should have: a state
    map with its state's medium, recorded by the bake receipt and published
    beside the map (pbrt_map_build.MEDIUM_SIDECAR); the base map with none."""
    lighting = fixture["lighting"]
    maps = [(lighting["map"], None)] + [
        (entry, (lighting.get("media") or {}).get(state))
        for state, entry in sorted((lighting.get("state_maps") or {}).items())]
    problems = []
    for entry, medium in maps:
        problems += check_one_map(fixture, entry, medium, Path(root or ROOT))
    return problems


def check_one_map(fixture, entry, medium, root):
    import participating_medium
    import pbrt_map_build
    name, lighting = fixture["name"], fixture["lighting"]
    bsp = root / entry["bsp"]
    if not bsp.is_file():
        return ["%s: map %s is not built (run lighting_fixtures.py build --fixture %s)" % (
            name, entry["bsp"], name)]
    sidecar = bsp.parent.parent / pbrt_map_build.MEDIUM_SIDECAR
    baked = json.loads(sidecar.read_text()) if sidecar.is_file() else None
    want_medium = participating_medium.validate(medium) if medium else None
    if baked != want_medium:
        return ["%s: map %s was baked with medium %s, not %s" % (
            name, entry.get("name", entry["bsp"]), baked and baked.get("name", baked), want_medium and
            want_medium.get("name", want_medium))]
    if not lighting.get("entities"):
        return []
    want = json.loads((fixture["directory"] / lighting["entities"]).read_text())["entities"]
    have = [e for e in map_entities(bsp) if e.get("classname") in FIXTURE_CLASSES]
    strip = lambda e: {k: v for k, v in e.items() if k not in ("hammerid", "id")}
    key = lambda e: (e.get("classname"), e.get("targetname") or e.get("_fixture_light"))
    want_by, have_by = {key(e): e for e in want}, {key(e): strip(e) for e in have}
    problems = ["%s: map lacks entity %s %s" % ((name,) + k) for k in want_by
                if k not in have_by]
    problems += ["%s: map has an undeclared fixture entity %s %s" % ((name,) + k)
                 for k in have_by if k not in want_by]
    problems += ["%s: map entity %s %s differs from entities.json" % ((name,) + k)
                 for k, e in want_by.items() if k in have_by and have_by[k] != e]
    return problems


def check(root=None, names=None, maps=True):
    """Problems with the fixture set: manifest, fixtures, references,
    tolerances, recorded comparisons and (with `maps`) the published maps.
    Empty when everything holds."""
    root = Path(root or FIXTURES)
    problems = []
    try:
        manifest = load_manifest(root)
    except (OSError, ValueError) as error:
        return ["manifest: %s" % error]
    for term, entry in manifest.get("terms", {}).items():
        if term not in TERMS:
            problems.append("manifest: unknown term %s" % term)
        if not entry.get("negative_control"):
            problems.append("term %s has no negative control" % term)
        if not entry.get("fixtures") and "judged_by" not in entry:
            problems.append("term %s is exercised by no fixture" % term)
    for term in TERMS:
        if term not in manifest.get("terms", {}):
            problems.append("manifest lacks term %s" % term)
    try:
        ledger = load_tolerances(root)
    except (OSError, ValueError) as error:
        problems.append("tolerances: %s" % error)
        ledger = {"entries": []}
    seen = set()
    for entry in ledger["entries"]:
        key = (entry.get("fixture"), entry.get("version"))
        if key in seen:
            problems.append("tolerances: %s version %s defined twice" % key)
        seen.add(key)
        if entry.get("digest") != tolerance_digest(entry):
            problems.append("tolerances: %s version %s was edited after it was fixed (digest "
                            "mismatch); add a new version instead" % key)
        for bound in ("mean", "p99"):
            if not isinstance(entry.get(bound), (int, float)) or entry[bound] <= 0:
                problems.append("tolerances: %s version %s has no positive %s" % (key + (bound,)))
    fixtures = [f for f in manifest["fixtures"] if not names or f["name"] in names]
    for entry in fixtures:
        name = entry["name"]
        for term in entry["terms"]:
            if term not in TERMS:
                problems.append("%s: unknown term %s" % (name, term))
        try:
            fixture = load_fixture(name, root)
        except (OSError, ValueError) as error:
            problems.append("%s: %s" % (name, error))
            continue
        for camera in entry["cameras"]:
            if camera not in fixture["cameras"]:
                problems.append("%s: manifest camera %s is not in the fixture" % (name, camera))
        if not current_tolerance(ledger, name):
            problems.append("%s: no tolerance fixed in tolerances.json" % name)
        problems += check_references(fixture, root)
        if maps:
            problems += check_map(fixture)
    problems += check_results(root, ledger, {f["name"] for f in fixtures})
    return problems


def check_references(fixture, root):
    name = fixture["name"]
    path = fixture["directory"] / "references" / "references.json"
    if not path.is_file():
        return ["%s: no references (run lighting_fixtures.py render --fixture %s)" % (name,
                                                                                      name)]
    record = json.loads(path.read_text())
    problems = []
    if record.get("schema") != REFERENCES_SCHEMA:
        return ["%s: references schema" % name]
    if record.get("status") not in REFERENCE_STATUSES:
        problems.append("%s: references status %s" % (name, record.get("status")))
    if record.get("fixture_reference_digest") != reference_digest(fixture):
        problems.append("%s: the fixture's stage, states, cameras, projectors or media changed "
                        "since its references were rendered" % name)
    expected_status = reference_status(record.get("samples", 0), record.get("denoised", False))
    if record.get("status") in REFERENCE_STATUSES and record["status"] != expected_status:
        problems.append("%s: status %s does not match %s samples" % (
            name, record["status"], record.get("samples")))
    for state in fixture["states"]:
        render = record.get("renders", {}).get(state)
        if not render:
            problems.append("%s: state %s has no reference" % (name, state))
            continue
        if render.get("normal_maps") is not True:
            problems.append("%s/%s: lighting reference omitted authored normal maps" %
                            (name, state))
        for layer, digest in render.get("stage_layers", {}).items():
            if not (ROOT / layer).is_file() or sha256(ROOT / layer) != digest:
                problems.append("%s/%s: stage layer %s changed or missing" % (name, state,
                                                                             layer))
        for camera in fixture["cameras"]:
            view = record.get("views", {}).get("%s.%s" % (state, camera))
            if not view:
                problems.append("%s: reference %s.%s missing" % (name, state, camera))
                continue
            for entry in view["files"].values():
                file = path.parent / entry["file"]
                if not file.is_file() or sha256(file) != entry["sha256"]:
                    problems.append("%s: %s missing or differs from its record" % (
                        name, entry["file"]))
            if view.get("judged_pixels", 0) < 1000:
                problems.append("%s: %s.%s judges only %d pixels" % (
                    name, state, camera, view.get("judged_pixels", 0)))
    for analytic in record.get("analytic", []):
        if not analytic["ok"]:
            problems.append("%s: %s failed (%s)" % (name, analytic["check"], analytic))
    return problems


def check_results(root, ledger, names):
    """Recorded comparisons must use the tolerance fixed before them."""
    problems = []
    directory = Path(root) / "results"
    for path in sorted(directory.glob("*.json")) if directory.is_dir() else []:
        result = json.loads(path.read_text())
        if result.get("schema") != RESULT_SCHEMA:
            problems.append("results/%s: schema" % path.name)
            continue
        if result["fixture"] not in names:
            continue
        entry = next((e for e in ledger["entries"] if e["fixture"] == result["fixture"] and
                      e["version"] == result["tolerance_version"]), None)
        if not entry:
            problems.append("results/%s: tolerance version %s of %s does not exist" % (
                path.name, result["tolerance_version"], result["fixture"]))
            continue
        if result["tolerance_digest"] != tolerance_digest(entry):
            problems.append("results/%s: its tolerance was changed after the comparison" %
                            path.name)
        if parse_time(entry["fixed"]) > parse_time(result["compared"]):
            problems.append("results/%s: tolerance %s version %s was fixed at %s, after the "
                            "comparison at %s" % (path.name, result["fixture"],
                                                  entry["version"], entry["fixed"],
                                                  result["compared"]))
        references = Path(root) / result["fixture"] / "references" / "references.json"
        if references.is_file():
            view = json.loads(references.read_text()).get("views", {}).get(
                "%s.%s" % (result["state"], result["camera"]))
            if not view or view["files"]["total"]["sha256"] != result["reference_sha256"]:
                problems.append("results/%s: its reference has changed since (stale); "
                                "compare again and remove it" % path.name)
        later = [e for e in ledger["entries"] if e["fixture"] == result["fixture"] and
                 parse_time(e["fixed"]) > parse_time(result["compared"]) and
                 not e.get("reason")]
        for e in later:
            problems.append("tolerances: %s version %s was fixed after a recorded comparison "
                            "(%s) without a stated reason" % (e["fixture"], e["version"],
                                                               path.name))
    return problems


def cmd_check(args):
    problems = check(names=args.fixture, maps=not args.no_maps)
    for problem in problems:
        print("FAIL " + problem)
    print("lighting fixtures: %s (%d problems)" % ("FAIL" if problems else "pass",
                                                   len(problems)))
    return 1 if problems else 0


# ================================================================== compare

def read_pfm(path):
    import numpy as np
    with open(path, "rb") as stream:
        kind = stream.readline().strip()
        if kind not in (b"PF", b"Pf"):
            raise ValueError("%s is not a PFM" % path)
        width, height = (int(v) for v in stream.readline().split())
        scale = float(stream.readline())
        data = np.frombuffer(stream.read(), "<f4" if scale < 0 else ">f4")
    channels = 3 if kind == b"PF" else 1
    image = data.reshape(height, width, channels)[::-1].astype(np.float64)
    return np.repeat(image, 3, axis=2) if channels == 1 else image


def read_image(path):
    import numpy as np
    if str(path).lower().endswith(".pfm"):
        return read_pfm(path)
    if str(path).lower().endswith(".exr"):
        return read_rgb_exr(path)
    raise ValueError("%s: the lab image must be a linear PFM or EXR" % path)


def error_stats(test, reference, index_image, emitters):
    import numpy as np
    if test.shape[:2] != reference.shape[:2]:
        raise ValueError("image is %dx%d, reference %dx%d" % (
            test.shape[1], test.shape[0], reference.shape[1], reference.shape[0]))
    mask = judged_mask(index_image, emitters)
    if not mask.any():
        raise ValueError("no judged pixels")
    scale = float(luminance(reference[mask]).mean())
    if scale <= 0:
        raise ValueError("the reference is black over its judged pixels")
    error = np.max(np.abs(test[mask] - reference[mask]), axis=-1) / scale
    if not np.isfinite(error).all():
        return {"pixels": int(mask.sum()), "mean": float("inf"), "p99": float("inf")}
    return {"pixels": int(mask.sum()), "mean": float(error.mean()),
            "p99": float(np.percentile(error, 99))}


def emitter_error_stats(test, reference, index_image, emitters):
    """Visible-emitter diagnostic. Its own reference luminance is the scale;
    the fixture's receiver tolerance does not certify emitter pixels yet."""
    import numpy as np
    if test.shape[:2] != reference.shape[:2] or index_image.shape != reference.shape[:2]:
        raise ValueError("emitter images and object indices have different sizes")
    mask = emitter_mask(index_image, emitters)
    if not mask.any():
        return None
    scale = float(luminance(reference[mask]).mean())
    if not np.isfinite(scale) or scale <= 0:
        raise ValueError("the emitter reference is black or non-finite")
    error = np.max(np.abs(test[mask] - reference[mask]), axis=-1) / scale
    black = np.max(np.abs(reference[mask]), axis=-1) / scale
    if not np.isfinite(error).all():
        return {"pixels": int(mask.sum()), "mean": float("inf"), "p99": float("inf"),
                "black_control_mean": float(black.mean())}
    return {"pixels": int(mask.sum()), "mean": float(error.mean()),
            "p99": float(np.percentile(error, 99)),
            "black_control_mean": float(black.mean())}


def compare(fixture_name, state, camera, image_path, root=None, now=None, kind="total"):
    """Score a lab image against the fixture's reference (`kind` total, or
    direct: the direct diffuse light alone, judged by the tolerance entry
    "<fixture>/direct"); returns the result."""
    import numpy as np
    from PIL import Image
    root = Path(root or FIXTURES)
    fixture = load_fixture(fixture_name, root)
    record = json.loads((fixture["directory"] / "references" / "references.json").read_text())
    view = record["views"]["%s.%s" % (state, camera)]
    references = fixture["directory"] / "references"
    if kind not in view["files"]:
        raise ValueError("%s %s.%s: no %s reference" % (fixture_name, state, camera, kind))
    reference = read_rgb_exr(references / view["files"][kind]["file"])
    index_image = np.asarray(Image.open(references / view["files"]["index"]["file"]),
                             np.int64)
    ledger_name = fixture_name if kind == "total" else "%s/%s" % (fixture_name, kind)
    entry = current_tolerance(load_tolerances(root), ledger_name)
    if not entry:
        raise ValueError("%s: no tolerance fixed; fix one before comparing" % ledger_name)
    stats = error_stats(read_image(image_path), reference, index_image, view["emitter_indices"])
    passed = stats["mean"] <= entry["mean"] and stats["p99"] <= entry["p99"]
    return {"schema": RESULT_SCHEMA, "fixture": fixture_name, "state": state,
            "camera": camera,
            "compared": (now or datetime.datetime.now(datetime.timezone.utc)).isoformat(
                timespec="seconds"),
            "kind": kind,
            "tolerance_version": entry["version"], "tolerance_digest": tolerance_digest(entry),
            "tolerance": {"mean": entry["mean"], "p99": entry["p99"]},
            "reference_sha256": view["files"][kind]["sha256"],
            "reference_status": record["status"], "image": str(image_path),
            "image_sha256": sha256(image_path), **stats,
            "pass": passed, "certifies": passed and record["status"] in ("denoised", "final")}


def cmd_compare(args):
    result = compare(args.fixture, args.state, args.camera, args.image)
    print(json.dumps(result, indent=2, sort_keys=True))
    if args.record:
        directory = FIXTURES / "results"
        stamp = result["compared"].replace(":", "").replace("-", "")
        write_json(directory / ("%s.%s.%s.%s.json" % (args.fixture, args.state, args.camera,
                                                      stamp)), result)
    if not result["pass"]:
        return 1
    if result["reference_status"] not in ("denoised", "final"):
        print("note: the reference is a preview; this comparison certifies nothing")
    return 0


# ================================================================== main

def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    commands = parser.add_subparsers(dest="command", required=True)
    g = commands.add_parser("generate")
    g.add_argument("--out", type=Path, default=FIXTURES)
    g.add_argument("--check", action="store_true")
    r = commands.add_parser("render")
    r.add_argument("--fixture", action="append")
    r.add_argument("--samples", type=int, default=PREVIEW_SAMPLES)
    r.add_argument("--seed", type=int, default=SEED)
    r.add_argument("--device", choices=("cpu", "gpu", "auto"), default="cpu")
    r.add_argument("--work", type=Path, default=WORK / "references")
    r.add_argument("--toolchain", type=Path)
    r.add_argument("--denoise", action="store_true",
                   help="OpenImageDenoise the references (status denoised at %d+ samples)"
                        % DENOISED_SAMPLES)
    b = commands.add_parser("build")
    b.add_argument("--fixture", action="append")
    b.add_argument("--work", type=Path, default=WORK / "maps")
    b.add_argument("--toolchain", type=Path)
    b.add_argument("--keep", action="store_true", help="keep the work directory's cached steps")
    b.add_argument("--keep-going", action="store_true")
    b.add_argument("--final", action="store_true",
                   help="bake at the profiles' full sample counts (not the preview overrides)")
    b.add_argument("--device", choices=("cpu", "gpu", "auto"),
                   help="Cycles device of every bake (default: the gi-fixture profile's)")
    c = commands.add_parser("check")
    c.add_argument("--fixture", action="append")
    c.add_argument("--no-maps", action="store_true",
                   help="skip the published maps (for a tree without builds)")
    args = parser.parse_args()
    return {"generate": cmd_generate, "render": cmd_render, "build": cmd_build,
            "check": cmd_check}[args.command](args)


if __name__ == "__main__":
    sys.exit(main())
