"""A homogeneous participating medium as the Cycles bakes and references use it.

There is one definition of the medium's Cycles form, and this module holds it.
The medium is a world volume: a Volume Scatter of `scattering_per_m` with the
Henyey-Greenstein `anisotropy`, plus a black Volume Absorption of
`absorption_per_m` (absorbing all of it). It is homogeneous, and it fills
the world, so `bounds_m` is a record of where the author placed it. The
lighting fixtures' media fill their sealed rooms, so the two agree there.

Two users:
- lighting_reference_blender.py, the Cycles references of the K11 lighting
  fixtures (RFC 0016);
- pbrt_lightmap_bake.py's `--medium`, an explicit, opt-in input. It reaches
  the bake through the map back end's manifest key `medium`
  (pbrt_map_build.py) and is recorded in the bake receipt. A lightmap baked
  with it carries the medium's transmittance on every light path it
  integrates.

The light-path policies keep volume bounces at 0 (pbrt_blender), so the
medium attenuates light and scatters camera rays once; it does not relight
surfaces by scattering.

`validate` is plain Python. `apply_world_medium` runs inside Blender only;
it imports bpy when called.
"""

import math

KEYS = ("absorption_per_m", "anisotropy", "bounds_m", "name", "scattering_per_m")


def validate(medium):
    """The medium as a canonical dict; raises ValueError when it is not one."""
    if not isinstance(medium, dict):
        raise ValueError("a medium is an object")
    unknown = sorted(set(medium) - set(KEYS))
    if unknown:
        raise ValueError("unknown medium keys: " + ", ".join(unknown))
    out = {}
    for key in ("scattering_per_m", "absorption_per_m"):
        value = medium.get(key)
        if not isinstance(value, (int, float)) or not math.isfinite(value) or value < 0:
            raise ValueError("medium %s must be a finite number >= 0" % key)
        out[key] = float(value)
    if out["scattering_per_m"] + out["absorption_per_m"] <= 0:
        raise ValueError("a medium needs scattering or absorption")
    g = medium.get("anisotropy", 0.0)
    if not isinstance(g, (int, float)) or not -1.0 < g < 1.0:
        raise ValueError("medium anisotropy must lie in (-1, 1)")
    out["anisotropy"] = float(g)
    bounds = medium.get("bounds_m")
    if bounds is not None:
        if (not isinstance(bounds, list) or len(bounds) != 2 or
                any(not isinstance(c, list) or len(c) != 3 for c in bounds) or
                any(not isinstance(v, (int, float)) or not math.isfinite(v)
                    for c in bounds for v in c) or
                any(lo > hi for lo, hi in zip(bounds[0], bounds[1]))):
            raise ValueError("medium bounds_m must be [[x, y, z], [x, y, z]], low then high")
        out["bounds_m"] = [[float(v) for v in c] for c in bounds]
    if "name" in medium:
        out["name"] = str(medium["name"])
    return out


def apply_world_medium(medium):
    """Make `medium` (validated) the Blender scene's world volume."""
    import bpy
    medium = validate(medium)
    world = bpy.context.scene.world or bpy.data.worlds.new("LightingWorld")
    bpy.context.scene.world = world
    world.use_nodes = True
    tree = world.node_tree
    output = next(node for node in tree.nodes if node.type == "OUTPUT_WORLD")
    scatter = tree.nodes.new("ShaderNodeVolumeScatter")
    scatter.inputs["Color"].default_value = (1.0, 1.0, 1.0, 1.0)
    scatter.inputs["Density"].default_value = medium["scattering_per_m"]
    scatter.inputs["Anisotropy"].default_value = medium["anisotropy"]
    shader = scatter.outputs[0]
    if medium["absorption_per_m"] > 0:
        absorb = tree.nodes.new("ShaderNodeVolumeAbsorption")
        absorb.inputs["Color"].default_value = (0.0, 0.0, 0.0, 1.0)
        # Absorption color c absorbs density x (1 - c): black absorbs it all.
        absorb.inputs["Density"].default_value = medium["absorption_per_m"]
        add = tree.nodes.new("ShaderNodeAddShader")
        tree.links.new(shader, add.inputs[0])
        tree.links.new(absorb.outputs[0], add.inputs[1])
        shader = add.outputs[0]
    tree.links.new(shader, output.inputs["Volume"])
    if hasattr(world.cycles, "homogeneous_volume"):
        world.cycles.homogeneous_volume = True
    return medium
