"""Cycles references for the RFC 0016 K11 lighting fixtures, inside Blender.

Run by `lighting_fixtures.py render`; it is `gi_reference_blender.py` (the
RFC 0011 GI reference renderer: the map pipeline's normalized stage, material
and emitter policy, light paths, passes and receipt) with two additions the
lighting set needs and the USD stage cannot carry, read from the JSON file
named by the LIGHTING_EXTRAS environment variable:

  projectors  `render.projected-light.v1` lights (public/render/projected_light.h):
              a point lamp whose node tree evaluates the contract's rule,
                  color x cookie(u, v) x saturate(c + l/d + q/d^2) x endFalloff x n.l
              with u, v the point's place in the perspective frustum (nothing
              outside it, nearer than nearZ or beyond farZ) and d in Source
              units. Projected lights are never baked (RFC 0011), so the lamp
              lights only paths of depth 1 (camera-ray surface hits and single
              scattering on camera rays): its own bounce is not part of the
              reference, as it is not part of the model's baked indirect light.
  medium      a homogeneous participating medium as the world volume
              (participating_medium.py, the one form the lightmap bake's
              --medium shares; the fixture's hall is sealed): scattering and
              absorption coefficients per meter and a Henyey-Greenstein
              anisotropy. The gi-reference light paths have no volume bounces,
              so the medium adds single scattering of the lights and
              attenuates everything.

It also turns on the Position and Normal passes (for the projector's analytic
check) and writes `lighting-extras.json` beside the render receipt with this
script's digest. Everything else is gi_reference_blender.main unchanged: the
extras are applied right after the scene's emitters are restored, before
object indices are assigned.
"""

import hashlib
import json
import math
import os
import sys
from pathlib import Path

import bpy
from mathutils import Matrix, Vector

sys.path.insert(0, str(Path(__file__).resolve().parent))
import gi_reference_blender  # noqa: E402
import participating_medium  # noqa: E402
import pbrt_blender  # noqa: E402

SOURCE_UNITS_PER_METER = 39.37007874015748
PROJECTOR_POWER = 4.0 * math.pi  # a point lamp of 4 pi W radiates 1 W/sr


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def math_node(tree, operation, a=None, b=None, value_b=None):
    node = tree.nodes.new("ShaderNodeMath")
    node.operation = operation
    for index, source in ((0, a), (1, b)):
        if source is not None:
            tree.links.new(source, node.inputs[index])
    if value_b is not None:
        node.inputs[1].default_value = value_b
    return node.outputs[0]


def constant(tree, value):
    node = tree.nodes.new("ShaderNodeValue")
    node.outputs[0].default_value = value
    return node.outputs[0]


def add_projector(index, projector, directory):
    """A lamp evaluating render.projected-light.v1 for `projector` (Source
    units for positions and distances, as the entity lump holds them)."""
    meters = 1.0 / SOURCE_UNITS_PER_METER
    origin = Vector(projector["origin_units"]) * meters
    forward = Vector(projector["forward"]).normalized()
    right = Vector(projector["right"]).normalized()
    up = Vector(projector["up"]).normalized()
    data = bpy.data.lights.new("Projector%02d" % index, "POINT")
    data.shadow_soft_size = 0.0
    data.energy = PROJECTOR_POWER
    data.color = (1.0, 1.0, 1.0)
    data.use_nodes = True
    tree = data.node_tree
    emission = next(node for node in tree.nodes if node.type == "EMISSION")
    # Normal: the unit direction from the lamp to the shaded point, in the
    # lamp's local frame, whose axes are right, up and -forward.
    coordinates = tree.nodes.new("ShaderNodeTexCoord")
    split = tree.nodes.new("ShaderNodeSeparateXYZ")
    tree.links.new(coordinates.outputs["Normal"], split.inputs[0])
    along = math_node(tree, "MULTIPLY", split.outputs["Z"], value_b=-1.0)  # cos to forward
    falloff = tree.nodes.new("ShaderNodeLightFalloff")
    falloff.inputs["Strength"].default_value = 1.0
    d_units = math_node(tree, "MULTIPLY", falloff.outputs["Linear"],
                        value_b=SOURCE_UNITS_PER_METER)
    z_units = math_node(tree, "MULTIPLY", d_units, along)
    tan_h = math.tan(math.radians(projector["horizontal_fov_degrees"]) / 2)
    tan_v = math.tan(math.radians(projector["vertical_fov_degrees"]) / 2)
    safe_along = math_node(tree, "MAXIMUM", along, value_b=1e-6)
    x = math_node(tree, "DIVIDE", split.outputs["X"],
                  math_node(tree, "MULTIPLY", safe_along, value_b=tan_h))
    y = math_node(tree, "DIVIDE", split.outputs["Y"],
                  math_node(tree, "MULTIPLY", safe_along, value_b=tan_v))
    # Cookie coordinates: u = 0.5 + 0.5 x along right, v = 0.5 - 0.5 y down
    # from up; Blender images start at the bottom row, so it reads (u, 1 - v).
    combine = tree.nodes.new("ShaderNodeCombineXYZ")
    tree.links.new(math_node(tree, "ADD", math_node(tree, "MULTIPLY", x, value_b=0.5),
                             constant(tree, 0.5)), combine.inputs["X"])
    tree.links.new(math_node(tree, "ADD", math_node(tree, "MULTIPLY", y, value_b=0.5),
                             constant(tree, 0.5)), combine.inputs["Y"])
    image = tree.nodes.new("ShaderNodeTexImage")
    image.image = bpy.data.images.load(str(directory / projector["cookie_image"]))
    image.image.colorspace_settings.name = "Non-Color"
    image.extension = "CLIP"
    image.interpolation = "Linear"
    tree.links.new(combine.outputs["Vector"], image.inputs["Vector"])
    # Frustum depth: nearZ < z <= farZ (the image's CLIP covers |x|, |y| > 1).
    inside = math_node(tree, "MULTIPLY",
                       math_node(tree, "GREATER_THAN", z_units, value_b=projector["near_z"]),
                       math_node(tree, "LESS_THAN", z_units,
                                 value_b=projector["far_z"] + 1e-4))
    c, l, q = projector["attenuation"]
    safe_d = math_node(tree, "MAXIMUM", d_units, value_b=1e-6)
    atten = math_node(tree, "ADD", constant(tree, c),
                      math_node(tree, "ADD",
                                math_node(tree, "DIVIDE", constant(tree, l), safe_d),
                                math_node(tree, "DIVIDE", constant(tree, q),
                                          math_node(tree, "MULTIPLY", safe_d, safe_d))))
    atten = math_node(tree, "MINIMUM", math_node(tree, "MAXIMUM", atten, value_b=0.0),
                      value_b=1.0)
    far = projector["far_z"]
    end = math_node(tree, "DIVIDE", math_node(tree, "SUBTRACT", d_units, constant(tree, far)),
                    constant(tree, 0.6 * far - far))
    end = math_node(tree, "MINIMUM", math_node(tree, "MAXIMUM", end, value_b=0.0), value_b=1.0)
    # Depth-1 paths only (see the module note).
    path = tree.nodes.new("ShaderNodeLightPath")
    direct = math_node(tree, "LESS_THAN", path.outputs["Ray Depth"], value_b=1.5)
    # Emission strength S gives irradiance S n.l with the Constant falloff
    # output (it cancels 1 / d^2), and the pipeline's unit is E / pi.
    scale = math_node(tree, "MULTIPLY",
                      math_node(tree, "MULTIPLY", math_node(tree, "MULTIPLY", atten, end),
                                math_node(tree, "MULTIPLY", inside, direct)),
                      value_b=math.pi)
    strength = tree.nodes.new("ShaderNodeLightFalloff")
    tree.links.new(scale, strength.inputs["Strength"])
    tree.links.new(strength.outputs["Constant"], emission.inputs["Strength"])
    tint = tree.nodes.new("ShaderNodeMix")
    tint.data_type = "RGBA"
    tint.blend_type = "MULTIPLY"
    tint.inputs["Factor"].default_value = 1.0
    tint.inputs[6].default_value = tuple(projector["color"]) + (1.0,)
    tree.links.new(image.outputs["Color"], tint.inputs[7])
    tree.links.new(tint.outputs[2], emission.inputs["Color"])
    lamp = bpy.data.objects.new(data.name, data)
    bpy.context.scene.collection.objects.link(lamp)
    rotation = Matrix((right, up, -forward)).transposed()
    lamp.matrix_world = Matrix.Translation(origin) @ rotation.to_4x4()
    lamp.visible_camera = False
    return lamp


def main():
    extras_path = Path(os.environ["LIGHTING_EXTRAS"])
    extras = json.loads(extras_path.read_text())
    directory = Path(extras["fixture_directory"])
    applied = {"projectors": [], "medium": None}
    restore = pbrt_blender.restore_emitters

    def restore_with_extras(scene):
        restore(scene)
        for index, projector in enumerate(extras.get("projectors", [])):
            applied["projectors"].append(add_projector(index, projector, directory).name)
        if extras.get("medium"):
            participating_medium.apply_world_medium(extras["medium"])
            applied["medium"] = extras["medium"]
        layer = bpy.context.scene.view_layers[0]
        layer.use_pass_position = True
        layer.use_pass_normal = True
        layer.cycles.use_pass_volume_direct = True
        layer.cycles.use_pass_volume_indirect = True
    pbrt_blender.restore_emitters = restore_with_extras
    gi_reference_blender.main()
    out_dir = Path(sys.argv[sys.argv.index("--out-dir") + 1])
    (out_dir / "lighting-extras.json").write_text(json.dumps({
        "schema": "lighting-extras-receipt/v1", "extras_sha256": sha256(extras_path),
        "renderer_sha256": sha256(__file__), "base_renderer_sha256":
        sha256(gi_reference_blender.__file__), "applied": applied}, indent=2,
        sort_keys=True) + "\n")


if __name__ == "__main__":
    main()
