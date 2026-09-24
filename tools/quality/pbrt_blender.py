"""Blender-side helpers for the PBRT map pipeline (import inside Blender only).

The PBRT reader and material translation policy live in `pbrt_scene`; this
module turns them into Blender data so the USD stage step, the Cycles reference
render and the lightmap bake build identical materials, emitters and sky.
"""

import math
from pathlib import Path

import bpy
from mathutils import Matrix

import pbrt_scene

PBRT_TO_USD = Matrix(pbrt_scene.PBRT_TO_USD)
EMITTER_PREFIXES = ("LightQuad", "LightDisk")


def matrix(values):
    return Matrix(values)


def clear_scene():
    for obj in list(bpy.data.objects):
        bpy.data.objects.remove(obj, do_unlink=True)


def build_material(scene, name):
    """Create a Principled material from the shared PBRT translation policy."""
    summary = pbrt_scene.material_summary(scene, name)
    root = Path(scene["source"]).parent
    result = bpy.data.materials.new(name)
    result.use_nodes = True
    shader = result.node_tree.nodes.get("Principled BSDF")
    if summary["base_texture"]:
        image = bpy.data.images.load(str((root / summary["base_texture"]).resolve()),
                                     check_existing=True)
        texture = result.node_tree.nodes.new("ShaderNodeTexImage")
        texture.image = image
        color = texture.outputs["Color"]
        fdr = summary["coat_internal_reflectance"]
        if fdr is not None:
            color = coated_albedo_nodes(result.node_tree, color, fdr)
        result.node_tree.links.new(color, shader.inputs["Base Color"])
    else:
        shader.inputs["Base Color"].default_value = tuple(summary["base_color"]) + (1.0,)
    shader.inputs["Metallic"].default_value = summary["metallic"]
    shader.inputs["IOR"].default_value = summary["ior"]
    shader.inputs["Transmission Weight"].default_value = summary["transmission"]
    if summary["coat_roughness"] is not None:
        shader.inputs["Roughness"].default_value = 1.0
        shader.inputs["Coat Weight"].default_value = 1.0
        shader.inputs["Coat Roughness"].default_value = summary["coat_roughness"]
    else:
        shader.inputs["Roughness"].default_value = summary["roughness"]
    if summary["diffuse_transmittance"]:
        diffuse_transmission_nodes(result.node_tree, shader, root, summary)
    return result


def scaled_color(tree, color, scale):
    if scale == 1.0:
        return color
    node = tree.nodes.new("ShaderNodeVectorMath")
    node.operation = "SCALE"
    tree.links.new(color, node.inputs[0])
    node.inputs["Scale"].default_value = scale
    return node.outputs["Vector"]


def diffuse_transmission_nodes(tree, principled, root, summary):
    """PBRT diffusetransmission exactly: Diffuse(R) + Translucent(T).

    The Principled node keeps the reflectance wiring (and stays the USD
    preview); the output is replaced by the two-lobe sum Cycles renders.
    """
    diffuse = tree.nodes.new("ShaderNodeBsdfDiffuse")
    base = principled.inputs["Base Color"]
    if base.is_linked:
        tree.links.new(scaled_color(tree, base.links[0].from_socket, summary["base_scale"]),
                       diffuse.inputs["Color"])
    else:
        diffuse.inputs["Color"].default_value = base.default_value
    translucent = tree.nodes.new("ShaderNodeBsdfTranslucent")
    transmittance = summary["diffuse_transmittance"]
    if transmittance["texture"]:
        texture = tree.nodes.new("ShaderNodeTexImage")
        texture.image = bpy.data.images.load(str((root / transmittance["texture"]).resolve()),
                                             check_existing=True)
        tree.links.new(scaled_color(tree, texture.outputs["Color"], transmittance["scale"]),
                       translucent.inputs["Color"])
    else:
        translucent.inputs["Color"].default_value = tuple(transmittance["color"]) + (1.0,)
    add = tree.nodes.new("ShaderNodeAddShader")
    tree.links.new(diffuse.outputs["BSDF"], add.inputs[0])
    tree.links.new(translucent.outputs["BSDF"], add.inputs[1])
    output = tree.nodes.get("Material Output")
    tree.links.new(add.outputs["Shader"], output.inputs["Surface"])


def coated_albedo_nodes(tree, color, fdr):
    """Per-texel pbrt_scene.coated_albedo: a (1 - fdr) / (1 - a fdr)."""
    def vector_math(operation):
        node = tree.nodes.new("ShaderNodeVectorMath")
        node.operation = operation
        return node
    numerator = vector_math("SCALE")
    tree.links.new(color, numerator.inputs[0])
    numerator.inputs["Scale"].default_value = 1.0 - fdr
    scaled = vector_math("SCALE")
    tree.links.new(color, scaled.inputs[0])
    scaled.inputs["Scale"].default_value = fdr
    denominator = vector_math("SUBTRACT")
    denominator.inputs[0].default_value = (1.0, 1.0, 1.0)
    tree.links.new(scaled.outputs["Vector"], denominator.inputs[1])
    divide = vector_math("DIVIDE")
    tree.links.new(numerator.outputs["Vector"], divide.inputs[0])
    tree.links.new(denominator.outputs["Vector"], divide.inputs[1])
    return divide.outputs["Vector"]


def rebind_materials(scene):
    """Replace USD preview materials with the PBRT translation policy.

    UsdPreviewSurface cannot carry coats, transmission or the coat albedo
    transform; the USD stage stays the geometry authority while Cycles steps
    shade with the same policy the game content uses.
    """
    assignments = {shape["name"]: shape["material"] for shape in scene["shapes"]}
    built = {}
    for obj in source_meshes():
        if obj.name not in assignments:
            raise ValueError("USD mesh has no PBRT material assignment: " + obj.name)
        name = assignments[obj.name]
        if name not in built:
            built[name] = build_material(scene, name)
        obj.data.materials.clear()
        obj.data.materials.append(built[name])


def emitter_name(index, shape):
    return ("LightDisk" if shape["kind"] == "disk" else "LightQuad") + "%02d" % index


def add_emitter(index, shape):
    name = emitter_name(index, shape)
    if shape["kind"] == "disk":
        radius = shape["radius"]
        vertices = [(radius * math.cos(2 * math.pi * i / 32),
                     radius * math.sin(2 * math.pi * i / 32), 0.0) for i in range(32)]
        faces = [tuple(range(32))]
    elif shape["kind"] == "trianglemesh":
        vertices = shape["points"]
        indices = shape["indices"]
        faces = [tuple(indices[i:i + 3]) for i in range(0, len(indices), 3)]
    else:
        raise ValueError("unsupported PBRT emitter shape " + shape["kind"])
    mesh = bpy.data.meshes.new(name)
    mesh.from_pydata(vertices, [], faces)
    mesh.update()
    obj = bpy.data.objects.new(name, mesh)
    bpy.context.scene.collection.objects.link(obj)
    obj.matrix_world = PBRT_TO_USD @ matrix(shape["world_from_object"])
    return obj


def emitter_material(index, shape):
    emission = shape["emission"]
    result = bpy.data.materials.new("Emitter%02d_Cycles" % index)
    result.use_nodes = True
    nodes = result.node_tree.nodes
    nodes.clear()
    output = nodes.new("ShaderNodeOutputMaterial")
    node = nodes.new("ShaderNodeEmission")
    node.inputs["Color"].default_value = tuple(emission["radiance"]) + (1.0,)
    node.inputs["Strength"].default_value = emission["scale"]
    result.node_tree.links.new(node.outputs["Emission"], output.inputs["Surface"])
    return result


def restore_emitters(scene):
    """Rebind PBRT radiance: Blender's USD preview-surface bridge drops emission."""
    for index, shape in enumerate(scene["emitters"]):
        obj = bpy.data.objects.get(emitter_name(index, shape))
        if not obj or obj.type != "MESH":
            raise ValueError("USD stage lost PBRT emitter " + emitter_name(index, shape))
        obj.data.materials.clear()
        obj.data.materials.append(emitter_material(index, shape))


def apply_environment(scene, equirect_exr):
    """Light the Blender world with the PBRT sky, or with nothing."""
    world = bpy.context.scene.world or bpy.data.worlds.new("World")
    bpy.context.scene.world = world
    world.use_nodes = True
    nodes = world.node_tree.nodes
    background = nodes.get("Background")
    if not scene["environment"]:
        background.inputs["Strength"].default_value = 0.0
        return
    if not equirect_exr or not Path(equirect_exr).is_file():
        raise FileNotFoundError("PBRT environment requires its equirect EXR")
    texture = nodes.new("ShaderNodeTexEnvironment")
    texture.image = bpy.data.images.load(str(Path(equirect_exr).resolve()), check_existing=True)
    texture.image.colorspace_settings.name = "Linear Rec.709"
    texture.interpolation = "Linear"
    world.node_tree.links.new(texture.outputs["Color"], background.inputs["Color"])
    background.inputs["Strength"].default_value = 1.0


def add_camera(scene, name):
    data = bpy.data.cameras.new(name)
    camera = bpy.data.objects.new(name, data)
    bpy.context.scene.collection.objects.link(camera)
    # PBRT camera space has +Z forward; Blender cameras look down -Z. These
    # exported cameras carry a mirrored camera_from_world (det -1), so the
    # column flip leaves a proper rotation.
    world_from_camera = matrix(scene["camera"]["camera_from_world"]).inverted()
    for row in range(3):
        world_from_camera[row][2] *= -1.0
    camera.matrix_world = PBRT_TO_USD @ world_from_camera
    data.type = "PERSP"
    film = scene["film"]
    # Blender's USD export derives the camera apertures from the scene's render
    # resolution, so it must be the film's before export; the default 16:9
    # turned a square 70-degree camera into a 102-degree one after re-import.
    render = bpy.context.scene.render
    render.resolution_x, render.resolution_y = film["width"], film["height"]
    render.resolution_percentage = 100
    # PBRT's fov spans the shorter image axis.
    data.sensor_fit = "VERTICAL" if film["width"] >= film["height"] else "HORIZONTAL"
    if data.sensor_fit == "VERTICAL":
        data.angle_y = math.radians(scene["camera"]["fov_degrees"])
    else:
        data.angle_x = math.radians(scene["camera"]["fov_degrees"])
    return camera


def source_meshes():
    return sorted((obj for obj in bpy.data.objects if obj.type == "MESH" and
                   not obj.name.startswith(EMITTER_PREFIXES)), key=lambda obj: obj.name)


GPU_BACKENDS = ("OPTIX", "CUDA", "HIP", "ONEAPI", "METAL")


def configure_cycles(samples, device="cpu"):
    """Select Cycles, its sample count and device; return the device used.

    `device` is "cpu", "gpu" (fail without one) or "auto" (GPU when Cycles
    finds one). GPU and CPU renders are both unbiased estimates of the same
    light; the receipt records which one produced the result.
    """
    render = bpy.context.scene
    render.render.engine = "CYCLES"
    render.cycles.samples = samples
    render.cycles.device = "CPU"
    if device == "cpu":
        return "CPU"
    preferences = bpy.context.preferences.addons["cycles"].preferences
    for backend in GPU_BACKENDS:
        try:
            preferences.compute_device_type = backend
        except TypeError:
            continue
        preferences.get_devices()
        gpus = [item for item in preferences.devices if item.type == backend]
        if gpus:
            for item in preferences.devices:
                item.use = item.type == backend
            render.cycles.device = "GPU"
            return "%s: %s" % (backend, ", ".join(item.name for item in gpus))
    if device == "gpu":
        raise RuntimeError("no Cycles GPU device is available")
    return "CPU"
