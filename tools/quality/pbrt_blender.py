"""Blender-side helpers for the map pipeline (import inside Blender only).

Scenes are read through `map_scene` (PBRT scenes and extracted USD scene
models); this module turns them into Blender data so the stage step, the
Cycles reference render, the lightmap bake and the reflection probe build
identical materials (every texture channel), emitters, sun lamps and sky.
"""

import math
from pathlib import Path

import bpy
from mathutils import Matrix, Vector

import cycles_device
import map_scene
import pbrt_scene

PBRT_TO_USD = Matrix(pbrt_scene.PBRT_TO_USD)
EMITTER_PREFIXES = ("LightQuad", "LightDisk")


def matrix(values):
    return Matrix(values)


def clear_scene():
    for obj in list(bpy.data.objects):
        bpy.data.objects.remove(obj, do_unlink=True)


def build_material(scene, name, normal_maps=True):
    """Create a Principled material from the shared material translation policy.

    Lightmap bakes pass `normal_maps=False`: baked irradiance is sampled at
    lightmap resolution on the smooth normal, and the runtime shader applies
    normal-map detail through the directional lightmap.
    """
    summary = map_scene.material_summary(scene, name)
    root = map_scene.material_root(scene)
    result = bpy.data.materials.new(name)
    result.use_nodes = True
    tree = result.node_tree
    shader = tree.nodes.get("Principled BSDF")
    textures = summary["textures"]
    if summary["base_texture"]:
        channel = textures.get("base") or {"file": summary["base_texture"], "channel": "rgb"}
        color = texture_channel(tree, root, channel, color=True)
        fdr = summary["coat_internal_reflectance"]
        if fdr is not None:
            color = coated_albedo_nodes(tree, color, fdr)
        tree.links.new(color, shader.inputs["Base Color"])
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
    if summary["clearcoat"] > 0:
        # UsdPreviewSurface clear coat: an IOR 1.5 GGX layer over the base.
        shader.inputs["Coat Weight"].default_value = summary["clearcoat"]
        shader.inputs["Coat Roughness"].default_value = summary["clearcoat_roughness"]
        shader.inputs["Coat IOR"].default_value = 1.5
    for channel, socket in (("roughness", "Roughness"), ("metallic", "Metallic"),
                            ("opacity", "Alpha")):
        if channel in textures:
            tree.links.new(texture_channel(tree, root, textures[channel]),
                           shader.inputs[socket])
    if not textures.get("opacity") and summary["opacity"] < 1.0 and \
            summary["transmission"] == 0.0:
        shader.inputs["Alpha"].default_value = summary["opacity"]
    if summary["opacity_threshold"] > 0 and shader.inputs["Alpha"].is_linked:
        # UsdPreviewSurface opacityThreshold: a binary cut-out mask.
        cut = tree.nodes.new("ShaderNodeMath")
        cut.operation = "GREATER_THAN"
        cut.inputs[1].default_value = summary["opacity_threshold"] - 1e-6
        tree.links.new(shader.inputs["Alpha"].links[0].from_socket, cut.inputs[0])
        tree.links.new(cut.outputs[0], shader.inputs["Alpha"])
    if "normal" in textures and normal_maps:
        normal_map = tree.nodes.new("ShaderNodeNormalMap")
        normal_map.space = "TANGENT"
        normal_map.uv_map = "st"
        tree.links.new(texture_channel(tree, root, textures["normal"], normal=True),
                       normal_map.inputs["Color"])
        tree.links.new(normal_map.outputs["Normal"], shader.inputs["Normal"])
    if "emission" in textures:
        tree.links.new(texture_channel(tree, root, textures["emission"], color=True),
                       shader.inputs["Emission Color"])
        shader.inputs["Emission Strength"].default_value = 1.0
    elif summary["emission_color"] and max(summary["emission_color"]) > 0:
        shader.inputs["Emission Color"].default_value = tuple(summary["emission_color"]) + (1.0,)
        shader.inputs["Emission Strength"].default_value = 1.0
    if summary["diffuse_transmittance"]:
        diffuse_transmission_nodes(tree, shader, root, summary)
    return result


def texture_channel(tree, root, texture, color=False, normal=False):
    """A UsdUVTexture-like record as a Blender socket.

    Reads `st` explicitly (bakes make the lightmap UVs active), applies the
    record's scale and bias, and selects its output channel. A normal record
    becomes the [0, 1] colour Blender's Normal Map node expects:
    (tex * scale + bias + 1) / 2.
    """
    path = Path(texture["file"])
    image = bpy.data.images.load(str((root / path).resolve()), check_existing=True)
    space = texture.get("colorspace", "auto")
    linear = space == "raw" or (space == "auto" and (not color or path.suffix.lower() in
                                                      (".exr", ".hdr")))
    if linear:
        image.colorspace_settings.name = "Non-Color"
    node = tree.nodes.new("ShaderNodeTexImage")
    node.image = image
    node.interpolation = "Linear"
    wraps = texture.get("wrap", ["repeat", "repeat"])
    if all(value in ("clamp",) for value in wraps):
        node.extension = "EXTEND"
    elif all(value == "black" for value in wraps):
        node.extension = "CLIP"
    uv = tree.nodes.new("ShaderNodeUVMap")
    uv.uv_map = "st"
    tree.links.new(uv.outputs["UV"], node.inputs["Vector"])
    scale = texture.get("scale", [1.0, 1.0, 1.0, 1.0])
    bias = texture.get("bias", [0.0, 0.0, 0.0, 0.0])
    channel = texture.get("channel", "rgb")
    if channel == "a":
        socket = node.outputs["Alpha"]
        return affine(tree, socket, scale[3], bias[3])
    socket = node.outputs["Color"]
    if normal:
        scale = [value / 2 for value in scale[:3]]
        bias = [(value + 1) / 2 for value in bias[:3]]
    if any(abs(v - 1) > 1e-9 for v in scale[:3]) or any(abs(v) > 1e-9 for v in bias[:3]):
        node_ma = tree.nodes.new("ShaderNodeVectorMath")
        node_ma.operation = "MULTIPLY_ADD"
        tree.links.new(socket, node_ma.inputs[0])
        node_ma.inputs[1].default_value = tuple(scale[:3])
        node_ma.inputs[2].default_value = tuple(bias[:3])
        socket = node_ma.outputs["Vector"]
    if channel in ("r", "g", "b"):
        separate = tree.nodes.new("ShaderNodeSeparateColor")
        tree.links.new(socket, separate.inputs["Color"])
        socket = separate.outputs["rgb".index(channel)]
    return socket


def affine(tree, socket, scale, bias):
    if abs(scale - 1) < 1e-9 and abs(bias) < 1e-9:
        return socket
    node = tree.nodes.new("ShaderNodeMath")
    node.operation = "MULTIPLY_ADD"
    tree.links.new(socket, node.inputs[0])
    node.inputs[1].default_value = scale
    node.inputs[2].default_value = bias
    return node.outputs[0]


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


def rebind_materials(scene, normal_maps=True):
    """Replace USD preview materials with the PBRT translation policy.

    UsdPreviewSurface cannot carry coats, transmission or the coat albedo
    transform; the USD stage stays the geometry authority while Cycles steps
    shade with the same policy the game content uses.
    """
    assignments = {shape["name"]: shape["material"] for shape in scene["shapes"]}
    built = {}
    for obj in source_meshes():
        if obj.name not in assignments:
            raise ValueError("USD mesh has no scene material assignment: " + obj.name)
        name = assignments[obj.name]
        if name not in built:
            built[name] = build_material(scene, name, normal_maps)
        obj.data.materials.clear()
        obj.data.materials.append(built[name])


emitter_name = map_scene.emitter_name


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
    obj.matrix_world = matrix(map_scene.emitter_to_stage(shape))
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
    surface = node.outputs["Emission"]
    if emission.get("one_sided"):
        # UsdLux area lights emit from their front face only.
        geometry = nodes.new("ShaderNodeNewGeometry")
        dark = nodes.new("ShaderNodeBsdfTransparent")
        mix = nodes.new("ShaderNodeMixShader")
        result.node_tree.links.new(geometry.outputs["Backfacing"], mix.inputs["Fac"])
        result.node_tree.links.new(surface, mix.inputs[1])
        result.node_tree.links.new(dark.outputs["BSDF"], mix.inputs[2])
        surface = mix.outputs["Shader"]
    result.node_tree.links.new(surface, output.inputs["Surface"])
    return result


def restore_emitters(scene):
    """Rebind emitter radiance (Blender's USD preview-surface bridge drops
    emission) and add the scene's distant lights as sun lamps."""
    for index, shape in enumerate(scene["emitters"]):
        obj = bpy.data.objects.get(emitter_name(index, shape))
        if not obj or obj.type != "MESH":
            raise ValueError("USD stage lost scene emitter " + emitter_name(index, shape))
        obj.data.materials.clear()
        obj.data.materials.append(emitter_material(index, shape))
    for index, light in enumerate(scene.get("distant_lights", [])):
        add_sun(index, light)


def add_sun(index, light):
    """A UsdLux DistantLight: Cycles sun strength is perpendicular irradiance."""
    irradiance = light["irradiance"]
    peak = max(irradiance)
    data = bpy.data.lights.new("Sun%02d" % index, "SUN")
    data.energy = peak
    data.color = tuple(value / peak for value in irradiance) if peak > 0 else (1.0, 1.0, 1.0)
    data.angle = math.radians(light["angle_degrees"])
    obj = bpy.data.objects.new(data.name, data)
    bpy.context.scene.collection.objects.link(obj)
    direction = Vector(light["direction"]).normalized()
    # A sun lamp shines along its local -Z.
    obj.rotation_euler = direction.to_track_quat("-Z", "Y").to_euler()
    return obj


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
    # Blender cameras look down -Z with +Y up; build that frame from the
    # scene's reference pose in stage space.
    pose = map_scene.camera_pose(scene)
    forward = Vector(pose["forward"]).normalized()
    up = Vector(pose["up"])
    right = forward.cross(up).normalized()
    up = right.cross(forward).normalized()
    eye = pose["eye"]
    camera.matrix_world = Matrix(((right.x, up.x, -forward.x, eye[0]),
                                  (right.y, up.y, -forward.y, eye[1]),
                                  (right.z, up.z, -forward.z, eye[2]),
                                  (0.0, 0.0, 0.0, 1.0)))
    data.type = "PERSP"
    film = scene["film"]
    # Blender's USD export derives the camera apertures from the scene's render
    # resolution, so it must be the film's before export; the default 16:9
    # turned a square 70-degree camera into a 102-degree one after re-import.
    render = bpy.context.scene.render
    render.resolution_x, render.resolution_y = film["width"], film["height"]
    render.resolution_percentage = 100
    # The reference fov spans the shorter image axis.
    data.sensor_fit = "VERTICAL" if film["width"] >= film["height"] else "HORIZONTAL"
    if data.sensor_fit == "VERTICAL":
        data.angle_y = math.radians(scene["camera"]["fov_degrees"])
    else:
        data.angle_x = math.radians(scene["camera"]["fov_degrees"])
    data.clip_start = 0.01
    return camera


def source_meshes():
    return sorted((obj for obj in bpy.data.objects if obj.type == "MESH" and
                   not obj.name.startswith(EMITTER_PREFIXES)), key=lambda obj: obj.name)


GPU_BACKENDS = ("OPTIX", "CUDA", "HIP", "ONEAPI", "METAL")
# Cycles device requests; `cycles_device` owns the policy (bakes on the GPU,
# correctness checks on the CPU).
DEVICES = cycles_device.DEVICES
DEFAULT_DEVICE = cycles_device.BAKE_DEVICE
# Cycles light-path policies, by name. `blender-default` keeps Blender's own
# defaults (4 diffuse bounces), which every map built before RFC 0011 used.
# `gi-reference` is the RFC 0011 oracle setting: enough diffuse bounces that
# the truncated series is below 1e-6 at albedo 0.5, no path clamping, and no
# caustics filter, so analytic fixtures (the furnace) converge to their
# closed form rather than to a biased estimate.
LIGHT_PATH_POLICIES = {
    "blender-default": None,
    "gi-reference": {"max_bounces": 64, "diffuse_bounces": 64, "glossy_bounces": 16,
                     "transmission_bounces": 16, "volume_bounces": 0,
                     "transparent_max_bounces": 16, "sample_clamp_direct": 0.0,
                     "sample_clamp_indirect": 0.0, "blur_glossy": 0.0,
                     "caustics_reflective": True, "caustics_refractive": True},
}


def configure_light_paths(policy):
    """Apply a named light-path policy; return the settings Cycles now uses."""
    if policy not in LIGHT_PATH_POLICIES:
        raise ValueError("unknown light-path policy " + str(policy))
    cycles = bpy.context.scene.cycles
    for key, value in (LIGHT_PATH_POLICIES[policy] or {}).items():
        setattr(cycles, key, value)
    return {"policy": policy, **{key: getattr(cycles, key) for key in (
        "max_bounces", "diffuse_bounces", "glossy_bounces", "transmission_bounces",
        "sample_clamp_direct", "sample_clamp_indirect")}}


def configure_cycles(samples, device=DEFAULT_DEVICE):
    """Select Cycles, its sample count and device; return the device used.

    `device` is "gpu" (the default; fail without one), "auto" (GPU when
    Cycles finds one, else CPU) or "cpu". GPU and CPU renders are both unbiased estimates of the same
    light; the receipt records which one produced the result.
    """
    render = bpy.context.scene
    render.render.engine = "CYCLES"
    render.cycles.samples = samples
    render.cycles.device = "CPU"
    if device not in DEVICES:
        raise ValueError("unknown Cycles device request " + str(device))
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
