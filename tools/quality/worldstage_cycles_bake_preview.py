"""Bake a linear Cycles inspection lightmap from a composed World Stage.

Run inside Blender with --stage, --out, --width, --height and --samples after --.
This is a feasibility artifact; the pinned standalone Cycles baker, SH layers,
and BSP2 runtime lighting packer remain separate RFC 0007/0008 work.
"""

import argparse
import hashlib
import json
import sys
from pathlib import Path

import bpy
import numpy as np


SOURCE_RNM_BASIS = (
    (0.81649661064147949, 0.0, 0.57735025882720947),
    (-0.40824821591377258, 0.70710676908493042, 0.57735025882720947),
    (-0.40824821591377258, -0.70710676908493042, 0.57735025882720947),
)


def normalized(vector):
    length = np.linalg.norm(vector)
    if not np.isfinite(length) or length < 1e-8:
        raise ValueError("World Stage has a degenerate tangent frame")
    return vector / length


def source_basis(obj, basis, local_direction=None):
    """Match GetBumpNormals using the Stage's face geometry and material UVs."""
    polygon = obj.data.polygons[0]
    loops = list(polygon.loop_indices[:3])
    if len(loops) != 3 or "st" not in obj.data.uv_layers:
        raise ValueError("World Stage mesh lacks triangle material UVs")
    points = [np.array(obj.matrix_world @ obj.data.vertices[
        obj.data.loops[loop].vertex_index].co, dtype=np.float64) for loop in loops]
    uv = [np.array(obj.data.uv_layers["st"].data[loop].uv, dtype=np.float64)
          for loop in loops]
    normal = normalized(np.array(obj.matrix_world.to_3x3().inverted().transposed() @
                                 polygon.normal, dtype=np.float64))
    edges = [points[index] - points[0] for index in (1, 2)]
    uv_edges = [uv[index] - uv[0] for index in (1, 2)]
    system = np.stack((edges[0], edges[1], normal))
    if abs(np.linalg.det(system)) < 1e-8:
        raise ValueError("World Stage mesh has degenerate geometry")
    s_axis = np.linalg.solve(system, [uv_edges[0][0], uv_edges[1][0], 0])
    t_axis = np.linalg.solve(system, [uv_edges[0][1], uv_edges[1][1], 0])
    if basis == "flat":
        return normal
    axis_y = normalized(np.cross(normal, s_axis))
    axis_x = normalized(np.cross(axis_y, normal))
    if np.dot(normal, np.cross(s_axis, t_axis)) < 0:
        axis_y = -axis_y
    local = local_direction if local_direction is not None else SOURCE_RNM_BASIS[int(basis[-1])]
    return normalized(local[0] * axis_x + local[1] * axis_y + local[2] * normal)


def override_face_normal(obj, direction):
    material = obj.data.materials[0].copy()
    material.name = obj.name + "_Source_" + str(tuple(direction))
    material.cycles.use_bump_map_correction = False
    obj.data.materials[0] = material
    principled = next((node for node in material.node_tree.nodes
                       if node.bl_idname == "ShaderNodeBsdfPrincipled"), None)
    if principled is None:
        raise ValueError("USD material lacks a Principled shader: " + obj.name)
    socket = principled.inputs["Normal"]
    for link in list(socket.links):
        material.node_tree.links.remove(link)
    normal = material.node_tree.nodes.new("ShaderNodeVectorMath")
    normal.operation = "NORMALIZE"
    normal.inputs[0].default_value = tuple(float(value) for value in direction)
    material.node_tree.links.new(normal.outputs["Vector"], socket)


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    arguments = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--stage", type=Path, required=True)
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--width", type=int, required=True)
    parser.add_argument("--height", type=int, required=True)
    parser.add_argument("--samples", type=int, default=32)
    parser.add_argument("--manifest", type=Path,
                        help="Portal material manifest with source chart extents")
    parser.add_argument("--require-all-charts-lit", action="store_true",
                        help="fixture control: every chart must receive nonzero light")
    parser.add_argument("--disable-lights", action="store_true",
                        help="negative control: zero imported light energy")
    parser.add_argument("--basis", choices=("flat", "rnm0", "rnm1", "rnm2"),
                        help="force the Source flat or RNM normal on each world face")
    parser.add_argument("--direction-id", help="identity for an additional SH fit direction")
    parser.add_argument("--normal-local", nargs=3, type=float, metavar=("X", "Y", "Z"),
                        help="additional normal in each face's Source tangent frame")
    args = parser.parse_args(arguments)
    if (not args.stage.is_file() or args.width < 64 or args.height < 64
            or args.samples < 1 or args.out.suffix.lower() != ".exr"):
        parser.error("valid stage, EXR output, dimensions and samples are required")
    if args.require_all_charts_lit and not args.manifest:
        parser.error("--require-all-charts-lit requires --manifest")
    if bool(args.direction_id) != bool(args.normal_local) or (args.basis and args.normal_local):
        parser.error("--direction-id and --normal-local are required together, without --basis")
    local_direction = normalized(np.asarray(args.normal_local, dtype=np.float64)) if (
        args.normal_local) else None
    if local_direction is not None and local_direction[2] < 0:
        parser.error("additional directions must be in the upper hemisphere")
    direction_id = args.direction_id or args.basis
    manifest = json.loads(args.manifest.read_text()) if args.manifest else None
    if manifest:
        source = args.stage.read_text()
        if 'sourceGeometrySha256 = "' + manifest["stage_sha256"] + '"' not in source:
            raise ValueError("material stage and chart manifest use different geometry")
        asset_set = json.dumps(manifest["materials"], sort_keys=True,
                               separators=(",", ":")).encode()
        asset_hash = hashlib.sha256(asset_set).hexdigest()
        if 'sourcePortalAssetsSha256 = "' + asset_hash + '"' not in source:
            raise ValueError("material stage and manifest use different Portal assets")
        for material in manifest["materials"].values():
            for channel in ("basecolor", "mrao", "normal"):
                if channel in material and digest(Path(material[channel]["path"])) != material[
                        channel]["sha256"]:
                    raise ValueError("Portal texture changed since stage authoring")

    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    if bpy.ops.wm.usd_import(filepath=str(args.stage.resolve()), import_materials=True,
                             import_lights=True) != {"FINISHED"}:
        raise RuntimeError("could not import composed World Stage")
    meshes = [obj for obj in bpy.data.objects if obj.type == "MESH"]
    lights = [obj for obj in bpy.data.objects if obj.type == "LIGHT"]
    if not meshes or not lights:
        raise RuntimeError("USD import omitted meshes or lights")
    if args.disable_lights:
        for light in lights:
            light.data.energy = 0.0
    face_directions = {}
    for obj in meshes:
        if "lightmap:st" not in obj.data.uv_layers or len(obj.data.materials) != 1:
            raise RuntimeError("mesh lacks a lightmap chart or material: " + obj.name)
        if obj.data.materials[0] is None or not obj.data.materials[0].use_nodes:
            raise RuntimeError("mesh has no imported USD shader: " + obj.name)
        if direction_id:
            direction = source_basis(obj, args.basis, local_direction)
            override_face_normal(obj, direction)
            face_directions[obj.name] = [float(value) for value in direction]
        obj.data.uv_layers.active = obj.data.uv_layers["lightmap:st"]

    scene = bpy.context.scene
    scene.render.engine = "CYCLES"
    scene.cycles.device = "CPU"
    scene.cycles.samples = args.samples
    scene.cycles.use_denoising = False
    scene.world.use_nodes = True
    scene.world.node_tree.nodes.get("Background").inputs["Strength"].default_value = 0.0
    scene.render.bake.margin = 2
    atlas = bpy.data.images.new("WorldStage_Lightmap_Atlas", width=args.width,
                                height=args.height, alpha=False, float_buffer=True)
    atlas.generated_color = (0.0, 0.0, 0.0, 1.0)
    for material in {slot.material for obj in meshes for slot in obj.material_slots}:
        target = material.node_tree.nodes.new("ShaderNodeTexImage")
        target.name = "WorldStage_Bake_Target"
        target.image = atlas
        material.node_tree.nodes.active = target

    bpy.ops.object.select_all(action="DESELECT")
    for obj in meshes:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = meshes[0]
    if bpy.ops.object.join() != {"FINISHED"}:
        raise RuntimeError("could not join USD meshes for a shared bake atlas")
    joined = bpy.context.view_layer.objects.active
    if "lightmap:st" not in joined.data.uv_layers:
        raise RuntimeError("joined mesh lost lightmap UVs")
    joined.data.uv_layers.active = joined.data.uv_layers["lightmap:st"]
    result = bpy.ops.object.bake(type="DIFFUSE", pass_filter={"DIRECT", "INDIRECT"})
    if result != {"FINISHED"}:
        raise RuntimeError("Cycles diffuse lighting bake failed")

    pixels = np.empty(args.width * args.height * 4, dtype=np.float32)
    atlas.pixels.foreach_get(pixels)
    rgb = pixels.reshape((-1, 4))[:, :3]
    if not np.isfinite(rgb).all():
        raise RuntimeError("Cycles bake produced non-finite pixels")
    lit = np.any(rgb > 1e-5, axis=1)
    if not lit.any():
        raise RuntimeError("Cycles bake produced no lit atlas pixels")
    chart_maxima = {}
    if manifest:
        layout = manifest["lightmap_atlas"]
        base_width, base_height = layout["width"], layout["height"]
        if (base_width <= 0 or base_height <= 0 or args.width % base_width
                or args.height % base_height):
            raise ValueError("bake dimensions must be integer multiples of chart atlas")
        scale_x, scale_y = args.width // base_width, args.height // base_height
        rgb_image = rgb.reshape((args.height, args.width, 3))
        for face_id, (x, y, width, height) in layout["charts"].items():
            chart = rgb_image[y * scale_y:(y + height) * scale_y,
                              x * scale_x:(x + width) * scale_x]
            if chart.size == 0:
                raise ValueError("lightmap chart escaped the bake atlas")
            chart_maxima[face_id] = float(chart.max())
        if args.require_all_charts_lit and any(value <= 1e-5 for value in
                                                chart_maxima.values()):
            raise RuntimeError("Cycles bake left a required lightmap chart dark")
    args.out.parent.mkdir(parents=True, exist_ok=True)
    atlas.filepath_raw = str(args.out.resolve())
    atlas.file_format = "OPEN_EXR"
    atlas.save()
    evidence = {
        "status": "pass", "renderer": "Blender Cycles bake preview",
        "stage_sha256": digest(args.stage), "exr_sha256": digest(args.out),
        "width": args.width, "height": args.height, "samples": args.samples,
        "source_meshes": len(meshes), "lights": len(lights),
        "lights_disabled": args.disable_lights,
        "nonzero_texels": int(lit.sum()),
        "chart_max_linear": chart_maxima,
        "mean_linear_rgb": [float(value) for value in rgb[lit].mean(axis=0)],
        "max_linear_rgb": [float(value) for value in rgb.max(axis=0)],
        "lighting_policy": "preview-v1; direct plus indirect diffuse, no color pass",
        "source_basis": direction_id,
        "source_local_direction": local_direction.tolist() if local_direction is not None else None,
        "source_face_directions": face_directions,
    }
    args.out.with_suffix(".json").write_text(json.dumps(evidence, indent=2,
                                                        sort_keys=True) + "\n")
    print("WORLDSTAGE_CYCLES_BAKE_PREVIEW " + json.dumps(evidence, sort_keys=True))


if __name__ == "__main__":
    main()
