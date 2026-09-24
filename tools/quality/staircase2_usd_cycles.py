"""Import the staircase2 PBRT fixture, export USD, and render that USD in Cycles.

This is a reference-scene adapter for RFC 0008 visual comparisons. It accepts
only the PBRT constructs used by staircase2 and fails if its mesh inventory or
material assignments change. Run inside Blender with ``--python ... --``.
"""

import argparse
import hashlib
import json
import math
import os
import re
import sys
from pathlib import Path

import bpy
from mathutils import Matrix, Vector

sys.path.insert(0, str(Path(__file__).resolve().parent))
from staircase2_materials import solid_linear_base_colors

PBRT_TO_USD = Matrix(((1.0, 0.0, 0.0, 0.0),
                      (0.0, 0.0, -1.0, 0.0),
                      (0.0, 1.0, 0.0, 0.0),
                      (0.0, 0.0, 0.0, 1.0)))


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def bracket_numbers(text, name):
    match = re.search(r'"' + re.escape(name) + r'"\s*\[([^\]]+)\]', text)
    if not match:
        raise ValueError("missing PBRT field " + name)
    return [float(value) for value in match.group(1).split()]


def matrix_from_pbrt(values):
    if len(values) != 16:
        raise ValueError("PBRT transform must have 16 scalars")
    return Matrix(tuple(tuple(values[column * 4 + row] for column in range(4))
                        for row in range(4)))


def parse_scene(source):
    camera_match = re.search(r'^Transform\s*\[([^\]]+)\]', source, re.M)
    if not camera_match:
        raise ValueError("PBRT camera transform missing")
    camera = matrix_from_pbrt([float(v) for v in camera_match.group(1).split()])
    fov = bracket_numbers(source, "float fov")
    if len(fov) != 1:
        raise ValueError("PBRT camera FOV is not scalar")
    textures = {}
    for name, path in re.findall(
            r'^Texture "([^"]+)" "spectrum" "imagemap"\s*'
            r'"string filter"\s*"trilinear"\s*'
            r'"string filename"\s*\[ "([^"]+)" \]', source, re.M):
        textures[name] = path
    if len(textures) != 3:
        raise ValueError("staircase2 texture inventory changed")
    geometry = source.split('\nAttributeBegin\n    AreaLightSource', 1)[0]
    assignments = []
    material = None
    material_stack = []
    for line in geometry.splitlines():
        if line.strip() == "AttributeBegin":
            material_stack.append(material)
        elif line.strip() == "AttributeEnd":
            if not material_stack:
                raise ValueError("PBRT material attribute stack underflow")
            material = material_stack.pop()
        named = re.match(r'^\s*NamedMaterial "([^"]+)"', line)
        if named:
            material = named.group(1)
        mesh = re.search(r'"string filename"\s*\[ "(models/[^"]+\.ply)" \]', line)
        if mesh:
            if not material:
                raise ValueError("PBRT mesh lacks a named material")
            assignments.append((mesh.group(1), material))
    if material_stack:
        raise ValueError("PBRT material attribute stack is incomplete")
    if len(assignments) != 19 or len({path for path, _ in assignments}) != 19:
        raise ValueError("staircase2 PLY inventory changed")
    light_blocks = re.split(r'\nAttributeBegin\n    AreaLightSource "diffuse"', source)[1:]
    if len(light_blocks) != 13:
        raise ValueError("staircase2 area-light inventory changed")
    lights = []
    for block in light_blocks:
        radiance = bracket_numbers(block, "rgb L")
        if len(radiance) != 3:
            raise ValueError("PBRT area light radiance is not RGB")
        if 'Shape "disk"' in block:
            transform = re.search(r'Transform\s*\[([^\]]+)\]', block)
            radius = bracket_numbers(block, "float radius")
            if not transform or len(radius) != 1:
                raise ValueError("PBRT disk light is incomplete")
            lights.append({"shape": "disk", "radiance": radiance,
                           "matrix": matrix_from_pbrt(
                               [float(v) for v in transform.group(1).split()]),
                           "radius": radius[0]})
        elif 'Shape "trianglemesh"' in block:
            points = bracket_numbers(block, "point3 P")
            indices = [int(v) for v in bracket_numbers(block, "integer indices")]
            if len(points) != 12 or indices != [0, 1, 2, 0, 2, 3]:
                raise ValueError("PBRT quad emitter topology changed")
            lights.append({"shape": "quad", "radiance": radiance,
                           "points": [points[index:index + 3] for index in range(0, 12, 3)]})
        else:
            raise ValueError("unsupported PBRT light shape")
    return camera, fov[0], textures, assignments, lights


def material(name, textures, root, solid_colors):
    result = bpy.data.materials.new(name)
    result.use_nodes = True
    shader = result.node_tree.nodes.get("Principled BSDF")
    shader.inputs["Roughness"].default_value = 1.0
    if name in {"Wood", "FloorTiles", "Wallpaper"}:
        key = {"Wood": "Texture01", "FloorTiles": "Texture02",
               "Wallpaper": "Texture03"}[name]
        image = bpy.data.images.load(str((root / textures[key]).resolve()),
                                     check_existing=True)
        texture = result.node_tree.nodes.new("ShaderNodeTexImage")
        texture.image = image
        result.node_tree.links.new(texture.outputs["Color"], shader.inputs["Base Color"])
    elif name == "Wall":
        shader.inputs["Base Color"].default_value = solid_colors["wall"] + (1.0,)
    elif name == "SpotHolder":
        shader.inputs["Base Color"].default_value = (1.0, 1.0, 1.0, 1.0)
    if name in {"Wood", "FloorTiles"}:
        shader.inputs["Roughness"].default_value = 0.35 if name == "Wood" else 0.15
        shader.inputs["Coat Weight"].default_value = 0.65
        shader.inputs["Coat Roughness"].default_value = 0.1 if name == "Wood" else 0.01
    elif name in {"Metal", "Chrome"}:
        shader.inputs["Base Color"].default_value = solid_colors[name.lower()] + (1.0,)
        shader.inputs["Metallic"].default_value = 1.0
        shader.inputs["Roughness"].default_value = 0.1
    elif name == "Glass":
        shader.inputs["Transmission Weight"].default_value = 1.0
        shader.inputs["IOR"].default_value = 1.5
        shader.inputs["Roughness"].default_value = 0.0
    return result


def add_emitter(index, spec, gain):
    if spec["shape"] == "quad":
        vertices = spec["points"]
        mesh = bpy.data.meshes.new("LightQuad%02d" % index)
        mesh.from_pydata(vertices, [], [(0, 1, 2, 3)])
        mesh.update()
        obj = bpy.data.objects.new(mesh.name, mesh)
    else:
        radius = spec["radius"]
        vertices = [(radius * math.cos(2 * math.pi * i / 32),
                     radius * math.sin(2 * math.pi * i / 32), 0.0)
                    for i in range(32)]
        mesh = bpy.data.meshes.new("LightDisk%02d" % index)
        mesh.from_pydata(vertices, [], [tuple(range(32))])
        mesh.update()
        obj = bpy.data.objects.new(mesh.name, mesh)
        obj.matrix_world = spec["matrix"]
    bpy.context.scene.collection.objects.link(obj)
    obj.matrix_world = PBRT_TO_USD @ obj.matrix_world
    return obj


def emitter_material(index, spec, gain):
    light_material = bpy.data.materials.new("Emitter%02d_Cycles" % index)
    light_material.use_nodes = True
    nodes = light_material.node_tree.nodes
    nodes.clear()
    output = nodes.new("ShaderNodeOutputMaterial")
    emission = nodes.new("ShaderNodeEmission")
    emission.inputs["Color"].default_value = tuple(spec["radiance"]) + (1.0,)
    emission.inputs["Strength"].default_value = gain
    light_material.node_tree.links.new(emission.outputs["Emission"],
                                       output.inputs["Surface"])
    return light_material


def main():
    arguments = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--scene", type=Path, required=True)
    parser.add_argument("--stage", type=Path, required=True)
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--samples", type=int, default=16)
    parser.add_argument("--size", type=int, default=512)
    parser.add_argument("--light-gain", type=float, default=1.0)
    parser.add_argument("--exposure", type=float, default=0.0)
    args = parser.parse_args(arguments)
    if args.samples < 1 or args.size < 64 or args.light_gain <= 0:
        parser.error("samples, size and light gain must be positive")
    if not os.environ.get("OCIO") or not Path(os.environ["OCIO"]).is_file():
        parser.error("a readable OCIO configuration is required for color fidelity")
    root = args.scene.resolve().parent
    source = args.scene.read_text()
    camera_matrix, fov, textures, assignments, lights = parse_scene(source)
    solid_colors = solid_linear_base_colors(source)
    for path in list(textures.values()) + [path for path, _ in assignments]:
        if not (root / path).is_file():
            raise FileNotFoundError(root / path)
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    materials = {name: material(name, textures, root, solid_colors)
                 for name in sorted({name for _, name in assignments})}
    model_hashes = {}
    for path, name in assignments:
        mesh_path = root / path
        model_hashes[path] = sha256(mesh_path)
        if bpy.ops.wm.ply_import(filepath=str(mesh_path.resolve())) != {"FINISHED"}:
            raise RuntimeError("Blender could not import " + path)
        obj = bpy.context.object
        obj.name = Path(path).stem
        mesh = obj.data
        attrs = {attr.name: attr for attr in mesh.attributes}
        if "u" not in attrs or "v" not in attrs:
            raise ValueError("PLY mesh lacks PBRT UVs: " + path)
        layer = mesh.uv_layers.new(name="st")
        for loop in mesh.loops:
            vertex = loop.vertex_index
            layer.data[loop.index].uv = (attrs["u"].data[vertex].value,
                                         attrs["v"].data[vertex].value)
        mesh.materials.append(materials[name])
        obj.matrix_world = PBRT_TO_USD
    for index, light in enumerate(lights):
        add_emitter(index, light, args.light_gain)
    camera_data = bpy.data.cameras.new("Staircase2Camera")
    camera = bpy.data.objects.new("Staircase2Camera", camera_data)
    bpy.context.scene.collection.objects.link(camera)
    # The PBRT camera transform is world-to-camera and has +Z forward. Its
    # inverse supplies the camera location and basis; Blender uses -Z forward.
    camera_world = camera_matrix.inverted()
    for row in range(3):
        camera_world[row][2] *= -1.0
    camera.matrix_world = PBRT_TO_USD @ camera_world
    camera_data.type = "PERSP"
    camera_data.angle = math.radians(fov)
    bpy.context.scene.camera = camera
    args.stage.parent.mkdir(parents=True, exist_ok=True)
    if bpy.ops.wm.usd_export(filepath=str(args.stage.resolve()),
                             export_materials=True, export_uvmaps=True,
                             export_cameras=True, export_lights=True,
                             export_textures_mode="NEW") != {"FINISHED"}:
        raise RuntimeError("Blender could not export staircase2 USD")
    for texture_path in textures.values():
        if sha256(args.stage.parent / texture_path) != sha256(root / texture_path):
            raise ValueError("USD texture copy differs from PBRT source: " + texture_path)
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    if bpy.ops.wm.usd_import(filepath=str(args.stage.resolve()),
                             import_materials=True, import_lights=True,
                             import_cameras=True) != {"FINISHED"}:
        raise RuntimeError("Blender could not import staircase2 USD")
    meshes = [obj for obj in bpy.data.objects if obj.type == "MESH"]
    if len(meshes) != len(assignments) + len(lights):
        raise ValueError("USD round trip lost staircase2 geometry or emitters")
    # Blender's USD preview-surface bridge does not carry emissive mesh
    # materials. Keep the USD geometry and restore the source light radiance
    # explicitly for the Cycles reference render.
    for index, spec in enumerate(lights):
        name = ("LightDisk" if spec["shape"] == "disk" else "LightQuad") + "%02d" % index
        obj = bpy.data.objects.get(name)
        if not obj or obj.type != "MESH":
            raise ValueError("USD round trip lost PBRT emitter " + name)
        obj.data.materials.clear()
        obj.data.materials.append(emitter_material(index, spec, args.light_gain))
    camera = bpy.data.objects.get("Staircase2Camera")
    if not camera:
        raise ValueError("USD round trip lost staircase2 camera")
    scene = bpy.context.scene
    scene.camera = camera
    scene.render.engine = "CYCLES"
    scene.cycles.device = "CPU"
    scene.cycles.samples = args.samples
    scene.cycles.use_denoising = True
    scene.render.resolution_x = args.size
    scene.render.resolution_y = args.size
    scene.render.resolution_percentage = 100
    scene.render.image_settings.file_format = "PNG"
    scene.render.film_transparent = False
    scene.world.use_nodes = True
    scene.world.node_tree.nodes.get("Background").inputs["Strength"].default_value = 0.0
    scene.view_settings.exposure = args.exposure
    scene.view_settings.view_transform = "Standard"
    args.out.parent.mkdir(parents=True, exist_ok=True)
    scene.render.filepath = str(args.out.resolve())
    bpy.ops.render.render(write_still=True)
    if not args.out.is_file():
        raise RuntimeError("Cycles did not render staircase2 USD")
    exr_path = args.out.with_suffix(".exr")
    scene.render.image_settings.file_format = "OPEN_EXR"
    scene.render.image_settings.color_depth = "32"
    bpy.data.images["Render Result"].save_render(filepath=str(exr_path.resolve()),
                                                  scene=scene)
    if not exr_path.is_file():
        raise RuntimeError("Cycles did not write linear staircase2 EXR")
    evidence = {"status": "pass", "fixture": "staircase2", "renderer": "Blender Cycles",
                "adapter_sha256": sha256(Path(__file__)),
                "source_sha256": sha256(args.scene), "stage_sha256": sha256(args.stage),
                "image_sha256": sha256(args.out), "exr_sha256": sha256(exr_path),
                "reference_sha256": sha256(root / "TungstenRender.png"),
                "reference_exr_sha256": sha256(root / "TungstenRender.exr"),
                "models": model_hashes,
                "material_assignments": dict(assignments),
                "solid_linear_base_colors": solid_colors,
                "textures": {path: sha256(root / path) for path in textures.values()},
                "material_policy": "PBRT named materials approximated by Blender Principled; "
                                   "mesh emitters restored from PBRT radiance after USD import",
                "mesh_count": len(assignments), "emitter_count": len(lights),
                "camera_fov_degrees": fov, "samples": args.samples, "size": args.size,
                "light_gain": args.light_gain, "exposure": args.exposure,
                "blender": bpy.app.version_string,
                "ocio_configuration_sha256": sha256(Path(os.environ["OCIO"]))
                if "OCIO" in os.environ else None}
    args.out.with_suffix(".json").write_text(json.dumps(evidence, indent=2, sort_keys=True) + "\n")
    print("STAIRCASE2_USD_CYCLES " + json.dumps(evidence, sort_keys=True))


if __name__ == "__main__":
    main()
