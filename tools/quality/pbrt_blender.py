"""Blender-side helpers for the map pipeline (import inside Blender only).

Scenes are read through `map_scene` (PBRT scenes and extracted USD scene
models); this module turns them into Blender data so the stage step, the
Cycles reference render, the lightmap bake and the reflection probe build
identical materials (every texture channel), emitters, sun lamps and sky.
"""

import contextlib
import math
from pathlib import Path

import bpy
import numpy as np
from mathutils import Matrix, Vector

import cycles_device
import map_scene
import pbrt_scene

PBRT_TO_USD = Matrix(pbrt_scene.PBRT_TO_USD)
EMITTER_PREFIXES = map_scene.EMITTER_PREFIXES
LAMP_SUFFIX = "Lamp"


def matrix(values):
    return Matrix(values)


def clear_scene():
    for obj in list(bpy.data.objects):
        bpy.data.objects.remove(obj, do_unlink=True)


def sort_collections():
    """Relink every collection's objects in name order.

    Blender's USD import links objects in a run-dependent order, and the USD
    export writes prims in collection order, so without this two exports of
    the same scene hold the same prims in different orders (different bytes).
    """
    for collection in [bpy.context.scene.collection] + list(bpy.data.collections):
        for obj in sorted(collection.objects, key=lambda item: item.name):
            collection.objects.unlink(obj)
            collection.objects.link(obj)


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


def validate_video_screen_uvs(scene):
    """Check a lifted movie frame's mapping after Blender's USD import."""
    root = map_scene.material_root(scene)
    for shape in scene["shapes"]:
        if shape.get("role") != "video_screen":
            continue
        name = shape["name"]
        obj = bpy.data.objects.get(name)
        if obj is None or obj.type != "MESH":
            raise ValueError("movie screen missing from Blender: " + name)
        layer = obj.data.uv_layers.get("st")
        if layer is None:
            raise ValueError("movie screen has no st UV layer: " + name)
        if "video_st_corners" not in shape:
            raise ValueError("movie screen needs a freshly extracted scene model: " + name)
        expected = [tuple(corner) for corner in shape["video_st_corners"]]
        actual = []
        for loop in obj.data.loops:
            point = obj.matrix_world @ obj.data.vertices[loop.vertex_index].co
            uv = layer.data[loop.index].uv
            actual.append((point.x, point.y, point.z, uv.x, uv.y))
        if len(actual) != len(expected):
            raise ValueError("movie screen corner count changed in Blender: " + name)
        for corner in expected:
            match = next((i for i, found in enumerate(actual)
                          if max(abs(a - b) for a, b in zip(corner, found)) < 1e-5), None)
            if match is None:
                raise ValueError("movie screen st UV or corner position changed: " + name)
            actual.pop(match)
        summary = map_scene.material_summary(scene, shape["material"])
        emission = summary["textures"].get("emission")
        if not emission:
            continue
        image_path = (root / emission["file"]).resolve()
        material = obj.data.materials[0]
        linked = False
        for node in material.node_tree.nodes:
            if node.type != "TEX_IMAGE" or not node.image or \
                    Path(node.image.filepath).resolve() != image_path:
                continue
            linked = any(link.to_node == node and link.to_socket == node.inputs["Vector"] and
                         link.from_node.type == "UVMAP" and link.from_node.uv_map == "st"
                         for link in material.node_tree.links)
            if linked:
                break
        if not linked:
            raise ValueError("movie frame is not sampled from st in Blender: " + name)


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


def cone_strength(tree, cone, scale, cosine):
    """A Source spot's strength: `scale` times vrad's cone multiplier of the
    cosine between the emitting normal and the direction to the lit point:
    1 inside the inner cone, 0 outside the outer one,
    ((cos - outer) / (inner - outer)) ** exponent between."""
    nodes, links = tree.nodes, tree.links
    ramp = nodes.new("ShaderNodeMapRange")
    ramp.clamp = True
    ramp.inputs["From Min"].default_value = cone["outer"]
    ramp.inputs["From Max"].default_value = max(cone["inner"], cone["outer"] + 1e-6)
    links.new(cosine, ramp.inputs["Value"])
    multiplier = ramp.outputs["Result"]
    if cone["exponent"] not in (0.0, 1.0):
        power = nodes.new("ShaderNodeMath")
        power.operation = "POWER"
        links.new(multiplier, power.inputs[0])
        power.inputs[1].default_value = cone["exponent"]
        multiplier = power.outputs["Value"]
    strength = nodes.new("ShaderNodeMath")
    strength.operation = "MULTIPLY"
    strength.inputs[0].default_value = scale
    links.new(multiplier, strength.inputs[1])
    return strength.outputs["Value"]


def attenuation_factor(tree, attenuation, centre):
    """A Source light's falloff relative to an inverse square, as nodes:
    d^2 / (c + l d + q d^2), and 0 at or beyond a positive hard radius -
    map_scene.vrad_falloff's expression. d is vrad's distance, from the
    shaded point to the light's centre (meters): the emitting point plus the
    ray back to the shaded point (Incoming x Ray Length), less `centre`.
    Cycles already applies the inverse square, so the light lands at
    intensity / (c + l d + q d^2), vrad's."""
    nodes, links = tree.nodes, tree.links

    def math_node(operation, a, b):
        node = nodes.new("ShaderNodeMath")
        node.operation = operation
        for socket, value in zip(node.inputs, (a, b)):
            if isinstance(value, (int, float)):
                socket.default_value = value
            else:
                links.new(value, socket)
        return node.outputs["Value"]

    def vector_node(operation, a, b, value=None):
        node = nodes.new("ShaderNodeVectorMath")
        node.operation = operation
        for socket, item in zip(node.inputs, (a, b)):
            if isinstance(item, tuple):
                socket.default_value = item
            elif item is not None:
                links.new(item, socket)
        if value is not None:
            node.inputs["Scale"].default_value = value
        return node

    geometry = nodes.new("ShaderNodeNewGeometry")
    ray = nodes.new("ShaderNodeLightPath").outputs["Ray Length"]
    back = vector_node("SCALE", geometry.outputs["Incoming"], None)
    links.new(ray, back.inputs["Scale"])
    shaded = vector_node("ADD", geometry.outputs["Position"], back.outputs["Vector"])
    offset = vector_node("SUBTRACT", shaded.outputs["Vector"], tuple(float(v) for v in centre))
    distance = vector_node("LENGTH", offset.outputs["Vector"], None).outputs["Value"]
    squared = math_node("MULTIPLY", distance, distance)
    denominator = math_node("ADD", math_node("ADD", attenuation["constant"],
                                             math_node("MULTIPLY", distance,
                                                       attenuation["linear"])),
                            math_node("MULTIPLY", squared, attenuation["quadratic"]))
    factor = math_node("DIVIDE", squared, denominator)
    if attenuation.get("radius_m", 0.0) > 0:
        factor = math_node("MULTIPLY", factor,
                           math_node("LESS_THAN", distance, attenuation["radius_m"]))
    return factor


def scaled_strength(tree, strength, attenuation, centre):
    """`strength` (a socket or a number) times the emitter's attenuation."""
    node = tree.nodes.new("ShaderNodeMath")
    node.operation = "MULTIPLY"
    if isinstance(strength, (int, float)):
        node.inputs[0].default_value = strength
    else:
        tree.links.new(strength, node.inputs[0])
    tree.links.new(attenuation_factor(tree, attenuation, centre), node.inputs[1])
    return node.outputs["Value"]


def emitter_centre(shape):
    """Where a Source light's falloff distance is measured from: its analytic
    centre, else its mesh's vertex mean (stage space)."""
    analytic = shape.get("shape") or {}
    if "centre" in analytic:
        return analytic["centre"]
    world = matrix(map_scene.emitter_to_stage(shape))
    points = [world @ Vector(point) for point in shape["points"]]
    return tuple(sum(p[i] for p in points) / len(points) for i in range(3))


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
    cone = shape.get("cone")
    strength = emission["scale"]
    if cone:
        # The disk's projected area supplies the cosine vrad also applies.
        geometry = nodes.new("ShaderNodeNewGeometry")
        cosine = nodes.new("ShaderNodeVectorMath")
        cosine.operation = "DOT_PRODUCT"
        result.node_tree.links.new(geometry.outputs["Incoming"], cosine.inputs[0])
        result.node_tree.links.new(geometry.outputs["True Normal"], cosine.inputs[1])
        strength = cone_strength(result.node_tree, cone, emission["scale"],
                                 cosine.outputs["Value"])
    # A lamp carries the falloff when there is one (the mesh is then seen by
    # camera rays only, whose bulb must not dim with distance).
    if shape.get("attenuation") and not lamp_kind(shape):
        strength = scaled_strength(result.node_tree, strength, shape["attenuation"],
                                   emitter_centre(shape))
    if not isinstance(strength, (int, float)):
        result.node_tree.links.new(strength, node.inputs["Strength"])
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


def lamp_kind(shape):
    """The Cycles lamp that lights the scene in place of an emitter mesh, or
    None to light with the mesh itself.

    A USD sphere, disk or rect light carries its analytic `shape`, and Cycles
    has the exact lamp for each: a point lamp with a radius is a true sphere,
    and a disk or rectangle area lamp emits from its front face, as a
    one-sided UsdLux light does. A lamp is sampled by the solid angle it
    covers; the tessellated mesh is sampled triangle by triangle, half of
    them facing away, which on the sphere fixture is some 300 times the
    noise at equal samples. Two-sided flat emitters and PBRT meshes keep
    their mesh."""
    kind = (shape.get("shape") or {}).get("kind")
    if kind == "sphere":
        return kind
    if kind in ("disk", "rect") and shape["emission"].get("one_sided"):
        return kind
    return None


def emitter_objects(index, shape):
    """Names of the Blender objects that emit for scene emitter `index`: its
    mesh and, when it has one, its lamp. Switch them together."""
    name = emitter_name(index, shape)
    return [name] + ([name + LAMP_SUFFIX] if lamp_kind(shape) else [])


def add_lamp(index, shape, mesh):
    """The emitter's Cycles lamp, with the same radiance, size and placement.

    Cycles gives a lamp of power P the radiance P / (pi * A) over its
    emitting area A (both faces of a sphere count: A = 4 pi r^2). The mesh
    stays for camera rays only - probes and reference views still see the
    fixture - and every other ray (diffuse, glossy, transmission, shadow)
    sees the lamp alone, so nothing is lit twice or shadowed by its own
    bulb."""
    kind = lamp_kind(shape)
    analytic = shape["shape"]
    emission = shape["emission"]
    radiance = [value * emission["scale"] for value in emission["radiance"]]
    peak = max(radiance)
    centre = Vector(analytic["centre"])
    name = emitter_name(index, shape) + LAMP_SUFFIX
    if kind == "sphere":
        radius = analytic["radius_m"]
        data = bpy.data.lights.new(name, "POINT")
        data.shadow_soft_size = radius
        area = 4 * math.pi * radius * radius
        rotation = Matrix.Identity(3)
    elif kind == "disk":
        radius = analytic["radius_m"]
        data = bpy.data.lights.new(name, "AREA")
        data.shape = "DISK"
        data.size = 2 * radius
        area = math.pi * radius * radius
        # An area lamp emits along its local -Z.
        rotation = Vector(analytic["normal"]).normalized().to_track_quat("-Z", "Y").to_matrix()
    else:
        world = matrix(map_scene.emitter_to_stage(shape))
        corners = [world @ Vector(point) for point in shape["points"][:4]]
        u, v = corners[1] - corners[0], corners[3] - corners[0]
        data = bpy.data.lights.new(name, "AREA")
        data.shape = "RECTANGLE"
        data.size, data.size_y = u.length, v.length
        area = u.length * v.length
        # usd_scene winds a rect so it emits along -(u x v): the lamp's -Z.
        rotation = Matrix((u.normalized(), v.normalized(),
                           u.cross(v).normalized())).transposed()
    data.energy = peak * math.pi * area
    data.color = tuple(value / peak for value in radiance) if peak > 0 else (1.0, 1.0, 1.0)
    cone = shape.get("cone")
    attenuation = shape.get("attenuation")
    if cone or attenuation:
        data.use_nodes = True
        tree = data.node_tree
        node = next(node for node in tree.nodes if node.type == "EMISSION")
        strength = 1.0
        if cone:
            geometry = tree.nodes.new("ShaderNodeNewGeometry")
            cosine = tree.nodes.new("ShaderNodeVectorMath")
            cosine.operation = "DOT_PRODUCT"
            tree.links.new(geometry.outputs["Incoming"], cosine.inputs[0])
            cosine.inputs[1].default_value = tuple(-rotation.col[2])
            strength = cone_strength(tree, cone, 1.0, cosine.outputs["Value"])
        if attenuation:
            strength = scaled_strength(tree, strength, attenuation, analytic["centre"])
        tree.links.new(strength, node.inputs["Strength"])
    obj = bpy.data.objects.new(name, data)
    bpy.context.scene.collection.objects.link(obj)
    obj.matrix_world = Matrix.Translation(centre) @ rotation.to_4x4()
    obj.visible_camera = False
    low = mesh.matrix_world @ Vector([min(v.co[i] for v in mesh.data.vertices) for i in range(3)])
    high = mesh.matrix_world @ Vector([max(v.co[i] for v in mesh.data.vertices)
                                       for i in range(3)])
    if ((low + high) / 2 - centre).length > 1e-3 * (high - low).length + 1e-6:
        raise ValueError("%s: the analytic light sits off its mesh" % name)
    mesh.visible_diffuse = mesh.visible_glossy = mesh.visible_transmission = False
    mesh.visible_shadow = mesh.visible_volume_scatter = False
    return obj


def restore_emitters(scene):
    """Rebind emitter radiance (Blender's USD preview-surface bridge drops
    emission), add each analytic emitter's lamp, and add the scene's
    distant lights as sun lamps."""
    for index, shape in enumerate(scene["emitters"]):
        obj = bpy.data.objects.get(emitter_name(index, shape))
        if not obj or obj.type != "MESH":
            raise ValueError("USD stage lost scene emitter " + emitter_name(index, shape))
        obj.data.materials.clear()
        obj.data.materials.append(emitter_material(index, shape))
        if lamp_kind(shape):
            add_lamp(index, shape, obj)
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
    # Diffuse lightmaps: light moves between surfaces by diffuse bounces only
    # (as production lightmappers do; glossy and caustic paths only add rare,
    # very bright samples to a diffuse irradiance bake), eight of them (the
    # truncated series is under 1% at albedo 0.6), unclamped.
    "lightmap": {"max_bounces": 16, "diffuse_bounces": 8, "glossy_bounces": 0,
                 "transmission_bounces": 8, "volume_bounces": 0,
                 "transparent_max_bounces": 16, "sample_clamp_direct": 0.0,
                 "sample_clamp_indirect": 0.0, "blur_glossy": 1.0,
                 "caustics_reflective": False, "caustics_refractive": False},
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



# Bounce limits of a DIRECT-only bake. Cycles' DIRECT bake pass (light
# reaching the surface straight from a lamp, emitter, sun or sky) is
# bit-identical at every bounce limit: 0, 1, 4, 8 and 64 bounces gave
# byte-equal EXRs on sp_gi_chamber_01's atlas (19 emitters) and on
# gi_room_states' sun, latlong sky and emitter (2026-09-29). The configured
# limit still traces every path's continuation, which cost the chamber's
# direct layer 1.8x. Transparent bounces stay: shadow rays pass cut-outs.
DIRECT_ONLY_BOUNCES = {"max_bounces": 0, "diffuse_bounces": 0, "glossy_bounces": 0,
                       "transmission_bounces": 0}


@contextlib.contextmanager
def direct_only_light_paths():
    """Run the enclosed DIRECT-only bakes without bounces, then restore the
    scene's light-path policy."""
    cycles = bpy.context.scene.cycles
    saved = {key: getattr(cycles, key) for key in DIRECT_ONLY_BOUNCES}
    try:
        for key, value in DIRECT_ONLY_BOUNCES.items():
            setattr(cycles, key, value)
        yield
    finally:
        for key, value in saved.items():
            setattr(cycles, key, value)

# Bakes pin their sampling rather than inherit Blender's defaults, and record
# it: a fixed seed, no adaptive stopping (every texel or pixel takes all its
# samples), and Cycles' own denoiser only where the caller asks for it, as
# OpenImageDenoise on the CPU. With the CPU device the result is then a pure
# function of scene, settings and toolchain (cycles_device.determinism).
SEED = 0


def pin_sampling(seed=SEED, denoise=False):
    """Pin the scene's Cycles sampling and denoising; return what is now used."""
    cycles = bpy.context.scene.cycles
    cycles.seed = seed
    cycles.use_animated_seed = False
    cycles.sample_offset = 0
    cycles.use_adaptive_sampling = False
    cycles.auto_scrambling_distance = False
    cycles.scrambling_distance = 1.0
    cycles.use_denoising = denoise
    if denoise:
        cycles.denoiser = "OPENIMAGEDENOISE"
        cycles.denoising_use_gpu = False
    return {"seed": seed, "adaptive_sampling": False, "sampling_pattern": cycles.sampling_pattern,
            "light_tree": cycles.use_light_tree,
            "denoise": {"denoiser": cycles.denoiser, "gpu": False,
                        "input_passes": cycles.denoising_input_passes,
                        "prefilter": cycles.denoising_prefilter,
                        "quality": cycles.denoising_quality} if denoise else None}


def configure_cycles(samples, device=DEFAULT_DEVICE):
    """Select Cycles, its sample count and device; return the device used.

    `device` is "gpu" (the default; fail without one), "auto" (GPU when
    Cycles finds one, else CPU) or "cpu". GPU and CPU renders are both unbiased estimates of the same
    light; the receipt records which one produced the result.
    """
    render = bpy.context.scene
    render.render.engine = "CYCLES"
    # Render results carry stamp metadata (date, render time, host) into
    # saved EXRs; outputs must be a function of the scene and settings.
    for name in dir(render.render):
        if name.startswith("use_stamp"):
            setattr(render.render, name, False)
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


def _unit_rows(v):
    """Rows of `v` (float32) scaled to unit length; zero rows stay zero."""
    squared = v[:, 0] * v[:, 0] + v[:, 1] * v[:, 1] + v[:, 2] * v[:, 2]
    ok = squared > 1e-35
    scale = np.float32(1.0) / np.sqrt(np.where(ok, squared, np.float32(1.0)))
    return np.where(ok[:, None], v * scale[:, None], np.float32(0.0))


def receiver_mesh(name, uv_name, positions, normals, quad_half, bake_width, normalize=True):
    """One tiny quad per (position, normal) receiver, each mapped to its own
    bake texel of a `bake_width`-wide image, as (object, BakeTarget node,
    image height). The object is invisible to light-transport rays (the
    quads cannot occlude one another) but bakeable (camera visibility).

    Built with numpy (float32, as mathutils): the probe volume and radiosity
    bakes build one per probe direction, millions on a whole retail map,
    where a Python loop of mathutils Vectors ran for over half an hour. The
    corners match that loop's to within one float32 rounding. Each quad is
    counter-clockwise about a x b = d."""
    d = np.asarray(normals, dtype=np.float32).reshape(-1, 3)
    if normalize:
        d = _unit_rows(d)
    count = len(d)
    # mathutils' Vector.orthogonal (Blender's ortho_v3_v3): built from the
    # dominant axis.
    x, y, z = d[:, 0], d[:, 1], d[:, 2]
    magnitude = np.abs(d)
    axis = np.where(magnitude[:, 0] > magnitude[:, 1],
                    np.where(magnitude[:, 0] > magnitude[:, 2], 0, 2),
                    np.where(magnitude[:, 1] > magnitude[:, 2], 1, 2))
    other = np.stack([z, z, -x - y], axis=1)
    first, second = axis == 0, axis == 1
    other[first] = np.stack([-y[first] - z[first], x[first], x[first]], axis=1)
    other[second] = np.stack([y[second], -x[second] - z[second], y[second]], axis=1)
    a = _unit_rows(other)
    b = np.cross(d, a).astype(np.float32)
    origin = np.asarray(positions, dtype=np.float32).reshape(-1, 3)
    height = max(1, (count + bake_width - 1) // bake_width)
    texel = np.arange(count)
    tx, ty = texel % bake_width, texel // bake_width
    half = np.float32(quad_half)
    corners, uvs = [], []
    for su, sv in ((-1, -1), (1, -1), (1, 1), (-1, 1)):
        corners.append(origin + (a * np.float32(su) + b * np.float32(sv)) * half)
        uvs.append(np.stack([(tx + 0.5 + su * 0.5) / bake_width,
                             (ty + 0.5 + sv * 0.5) / height], axis=1))
    vertices = np.stack(corners, axis=1).reshape(-1, 3)
    loop_uvs = np.stack(uvs, axis=1).reshape(-1, 2).astype(np.float32)
    mesh = bpy.data.meshes.new(name)
    mesh.vertices.add(len(vertices))
    mesh.vertices.foreach_set("co", vertices.ravel())
    mesh.loops.add(len(vertices))
    mesh.loops.foreach_set("vertex_index", np.arange(len(vertices), dtype=np.int32))
    mesh.polygons.add(count)
    mesh.polygons.foreach_set("loop_start", np.arange(0, len(vertices), 4, dtype=np.int32))
    mesh.update()
    layer = mesh.uv_layers.new(name=uv_name)
    layer.data.foreach_set("uv", loop_uvs.ravel())
    mesh.polygons.foreach_set("use_smooth", np.zeros(count, dtype=bool))
    obj = bpy.data.objects.new(name, mesh)
    bpy.context.scene.collection.objects.link(obj)
    obj.visible_diffuse = obj.visible_glossy = obj.visible_transmission = False
    obj.visible_shadow = obj.visible_volume_scatter = False
    obj.visible_camera = True
    material = bpy.data.materials.new(name.rstrip("s"))
    material.use_nodes = True
    tree = material.node_tree
    tree.nodes.clear()
    output = tree.nodes.new("ShaderNodeOutputMaterial")
    diffuse = tree.nodes.new("ShaderNodeBsdfDiffuse")
    diffuse.inputs["Color"].default_value = (1.0, 1.0, 1.0, 1.0)
    tree.links.new(diffuse.outputs["BSDF"], output.inputs["Surface"])
    target = tree.nodes.new("ShaderNodeTexImage")
    target.name = "BakeTarget"
    tree.nodes.active = target
    mesh.materials.append(material)
    return obj, target, height
