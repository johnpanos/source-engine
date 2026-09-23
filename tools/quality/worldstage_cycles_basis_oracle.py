"""Blender Cycles feasibility oracle for Source's four RNM bake directions."""

import json
from pathlib import Path
import sys

import bpy
import numpy as np
from mathutils import Vector


BASIS = (
    (0.0, 0.0, 1.0),
    (0.81649661064147949, 0.0, 0.57735025882720947),
    (-0.40824821591377258, 0.70710676908493042, 0.57735025882720947),
    (-0.40824821591377258, -0.70710676908493042, 0.57735025882720947),
)


def main():
    arguments = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    if len(arguments) != 1:
        raise ValueError("usage: blender --python worldstage_cycles_basis_oracle.py -- OUT.json")
    output = Path(arguments[0])
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    bpy.ops.mesh.primitive_plane_add(size=2.0)
    plane = bpy.context.object
    material = bpy.data.materials.new("Source_RNM_Basis_Oracle")
    material.use_nodes = True
    material.cycles.use_bump_map_correction = False
    material.node_tree.nodes.clear()
    shader = material.node_tree.nodes.new("ShaderNodeBsdfDiffuse")
    shader.inputs["Color"].default_value = (1, 1, 1, 1)
    normal = material.node_tree.nodes.new("ShaderNodeVectorMath")
    normal.operation = "NORMALIZE"
    target = material.node_tree.nodes.new("ShaderNodeTexImage")
    material.node_tree.nodes.active = target
    surface = material.node_tree.nodes.new("ShaderNodeOutputMaterial")
    material.node_tree.links.new(normal.outputs["Vector"], shader.inputs["Normal"])
    material.node_tree.links.new(shader.outputs["BSDF"], surface.inputs["Surface"])
    plane.data.materials.append(material)
    bpy.ops.object.light_add(type="SUN", location=(0, 0, 2))
    sun = bpy.context.object
    light_direction = Vector(BASIS[1])
    sun.rotation_euler = (-light_direction).to_track_quat("-Z", "Y").to_euler()
    sun.data.energy = 2.0
    sun.data.angle = 0.0
    scene = bpy.context.scene
    scene.render.engine = "CYCLES"
    scene.cycles.device = "CPU"
    scene.cycles.samples = 64
    scene.cycles.max_bounces = 0
    scene.cycles.use_denoising = False
    scene.world.use_nodes = True
    scene.world.node_tree.nodes.get("Background").inputs["Strength"].default_value = 0.0
    scene.render.bake.margin = 0
    bpy.ops.object.select_all(action="DESELECT")
    plane.select_set(True)
    bpy.context.view_layer.objects.active = plane
    measured = []
    for index, direction in enumerate(BASIS):
        normal.inputs[0].default_value = direction
        image = bpy.data.images.new("RNM_" + str(index), width=64, height=64,
                                    alpha=False, float_buffer=True)
        image.generated_color = (0, 0, 0, 1)
        target.image = image
        if bpy.ops.object.bake(type="DIFFUSE", pass_filter={"DIRECT"}) != {"FINISHED"}:
            raise RuntimeError("Cycles direct bake failed for basis " + str(index))
        pixels = np.empty(64 * 64 * 4, dtype=np.float32)
        image.pixels.foreach_get(pixels)
        rgb = pixels.reshape((64, 64, 4))[8:56, 8:56, :3]
        measured.append(float(np.mean(rgb)))
    if measured[0] <= 0 or measured[1] <= 0:
        raise ValueError("Cycles basis oracle produced no flat or aligned light")
    expected = [max(0.0, sum(a * b for a, b in zip(direction, BASIS[1])))
                for direction in BASIS]
    ratios = [value / measured[0] for value in measured]
    expected_ratios = [value / expected[0] for value in expected]
    errors = [abs(actual - predicted) for actual, predicted in zip(ratios, expected_ratios)]
    evidence = {"status": "pass" if max(errors) < 0.08 else "fail",
                "renderer": "Blender Cycles direct diffuse bake", "samples": 64,
                "light_direction": BASIS[1], "basis": BASIS,
                "measured_linear": measured, "measured_ratios": ratios,
                "expected_clamped_cosine_ratios": expected_ratios,
                "absolute_ratio_errors": errors}
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(evidence, indent=2, sort_keys=True) + "\n")
    print("WORLDSTAGE_CYCLES_BASIS_ORACLE " + json.dumps(evidence, sort_keys=True))
    if evidence["status"] != "pass":
        raise RuntimeError("Cycles normal override differs from analytic RNM basis")


if __name__ == "__main__":
    main()
