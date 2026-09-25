"""Author shared `lightmap_st` UVs on a PBRT-derived USD stage and bake Cycles light.

Run inside Blender; `pbrt_map_build.py` drives it. Meshes whose translated
material is transmissive or fully metallic, plus any `--exclude-material`,
keep a degenerate
lightmap UV at (0, 0) and get no atlas space. Every other source mesh is
smart-projected, area-normalized and packed into one atlas, then baked as
scene-linear diffuse irradiance (direct + indirect, albedo divided out) under
the scene's area emitters, sun lamps and sky. Materials bake on their smooth
normals (no normal maps).

With `--directional-dir` the same light is also baked for the three Source
RNM basis normals in each texel's lightmap tangent frame (T from the
`lightmap_st` tangent orthogonalized against the smooth normal N, B = N x T),
and that frame is baked too (EMIT of T and N, encoded 0.5 + 0.5 v).
`lightmap_directional.py` fits the per-texel irradiance gradient the runtime
shader applies to normal-mapped surfaces.

A scene with a distant light (sun) also gets `sun_visibility.exr`: the sun's
direct diffuse bake with shadows divided by the same bake without them, so
texels hold the sun's [0, 1] visibility (soft penumbrae included). Its diffuse
light stays in the atlas; the runtime adds the sun's specular dynamically,
shadowed by this mask, as Source 2 does for its static sun.
"""

import argparse
import hashlib
import json
import math
import os
import sys
from pathlib import Path

import bpy

sys.path.insert(0, str(Path(__file__).resolve().parent))
import bake_progress  # noqa: E402
import cycles_device  # noqa: E402
import pbrt_blender  # noqa: E402
import map_scene  # noqa: E402

SCOPE = "pbrt-shared-lightmap-uv-and-cycles-bake"
# Source's radiosity normal mapping basis (tangent space), 54.74 degrees from N.
RNM_BASIS = ((0.816496580927726, 0.0, 0.5773502691896258),
             (-0.408248290463863, 0.7071067811865475, 0.5773502691896258),
             (-0.408248290463863, -0.7071067811865475, 0.5773502691896258))
FRAME_SAMPLES = 16
SUN_SAMPLES = 256
PROJECTED_PART_LIMIT = 4096
PROXY_PREFIX = "_lightmap_footprint_"


# Bake tile edge: a 4096 atlas reports 16 tiles of progress per pass.
BAKE_TILE = 1024


def announce(label, size):
    """Name the next bake for the pipeline's progress (bake_progress.py)."""
    print(bake_progress.progress_line(event="bake", label=label, size=[size, size],
                                      samples=bpy.context.scene.cycles.samples), flush=True)


def message(text):
    print(bake_progress.progress_line(message=text), flush=True)


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def connected_parts(obj):
    """Number of vertex-connected parts of a mesh (label propagation)."""
    import numpy as np
    count = len(obj.data.vertices)
    edges = np.empty(len(obj.data.edges) * 2, dtype=np.int64)
    obj.data.edges.foreach_get("vertices", edges)
    a, b = edges[0::2], edges[1::2]
    labels = np.arange(count)
    while True:
        low = np.minimum(labels[a], labels[b])
        before = labels.copy()
        np.minimum.at(labels, a, low)
        np.minimum.at(labels, b, low)
        labels = labels[labels]
        if np.array_equal(labels, before):
            return len(np.unique(labels))


def footprint(obj):
    """World rectangle of a mesh on its two widest axes: (axes, low, high)."""
    import numpy as np
    points = np.empty(len(obj.data.vertices) * 3)
    obj.data.vertices.foreach_get("co", points)
    points = points.reshape(-1, 3)
    world = points @ np.array(obj.matrix_world)[:3, :3].T + np.array(obj.matrix_world)[:3, 3]
    axes = tuple(sorted(np.argsort(world.max(axis=0) - world.min(axis=0))[1:]))
    return axes, world[:, axes].min(axis=0), world[:, axes].max(axis=0), world


def add_footprint_proxy(obj):
    """A quad the size of `obj`'s footprint that the joint pack sizes and places."""
    axes, low, high, _ = footprint(obj)
    corners = []
    for a, b in ((low[0], low[1]), (high[0], low[1]), (high[0], high[1]), (low[0], high[1])):
        point = [0.0, 0.0, 0.0]
        point[axes[0]], point[axes[1]] = float(a), float(b)
        corners.append(point)
    mesh = bpy.data.meshes.new(PROXY_PREFIX + obj.name)
    mesh.from_pydata(corners, [], [(0, 1, 2, 3)])
    mesh.uv_layers.new(name="lightmap_st")
    proxy = bpy.data.objects.new(mesh.name, mesh)
    bpy.context.scene.collection.objects.link(proxy)
    return proxy


def apply_footprint_chart(obj, proxy):
    """Map `obj`'s footprint coordinates affinely onto its packed proxy quad."""
    import numpy as np
    axes, low, high, world = footprint(obj)
    uv = {proxy.data.loops[i].vertex_index: np.array(proxy.data.uv_layers["lightmap_st"].data[i].uv)
          for i in range(4)}
    s = (world[:, axes[0]] - low[0]) / max(high[0] - low[0], 1e-12)
    t = (world[:, axes[1]] - low[1]) / max(high[1] - low[1], 1e-12)
    vertex_uv = uv[0] + np.outer(s, uv[1] - uv[0]) + np.outer(t, uv[3] - uv[0])
    loops = np.empty(len(obj.data.loops), dtype=np.int64)
    obj.data.loops.foreach_get("vertex_index", loops)
    obj.data.uv_layers["lightmap_st"].data.foreach_set("uv", vertex_uv[loops].ravel())


def pack_lightmap_uvs(meshes, margin):
    """Chart and jointly pack `meshes`; return {name: parts} of projected meshes.

    A mesh with more than PROJECTED_PART_LIMIT separate parts (fur, grass,
    foliage cards) gets one planar chart of its footprint instead of one
    chart per part: hundreds of thousands of islands make Blender's FRACTION
    margin pack collapse the whole atlas to nothing. A footprint proxy quad
    is packed with the other charts so the projected chart gets the same
    texel density and margin; overlapping parts share its texels.
    """
    parts = {obj.name: connected_parts(obj) for obj in meshes}
    projected = [obj for obj in meshes if parts[obj.name] > PROJECTED_PART_LIMIT]
    proxies = {obj.name: add_footprint_proxy(obj) for obj in projected}
    charted = [obj for obj in meshes if obj not in projected] + list(proxies.values())
    bpy.ops.object.select_all(action="DESELECT")
    for obj in charted:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = charted[0]
    bpy.ops.object.mode_set(mode="EDIT")
    # Headless there is no UV editor selection; sync makes every selected face
    # a selected UV face so projection and packing see the whole atlas.
    bpy.context.scene.tool_settings.use_uv_select_sync = True
    bpy.ops.mesh.select_all(action="SELECT")
    if bpy.ops.uv.smart_project(angle_limit=math.radians(66), island_margin=margin,
                                area_weight=0.0, correct_aspect=True,
                                scale_to_bounds=False) != {"FINISHED"}:
        raise RuntimeError("Blender could not project lightmap UVs")
    # One texel density everywhere, then a joint pack. A FRACTION margin is a
    # share of atlas space; the default scaled margin collapses scenes with
    # thousands of small islands (foliage, cushions) to ~10% coverage.
    bpy.ops.uv.select_all(action="SELECT")
    bpy.ops.uv.average_islands_scale()
    if bpy.ops.uv.pack_islands(margin_method="FRACTION", margin=margin,
                               rotate=True) != {"FINISHED"}:
        raise RuntimeError("Blender could not pack the shared lightmap atlas")
    bpy.ops.object.mode_set(mode="OBJECT")
    for obj in projected:
        apply_footprint_chart(obj, proxies[obj.name])
    return {obj.name: {"parts": parts[obj.name], "proxy": proxies[obj.name]} for obj in projected}


def vector_math(tree, operation, *inputs, scale=None):
    node = tree.nodes.new("ShaderNodeVectorMath")
    node.operation = operation
    for index, value in enumerate(inputs):
        if isinstance(value, bpy.types.NodeSocket):
            tree.links.new(value, node.inputs[index])
        else:
            node.inputs[index].default_value = value
    if scale is not None:
        if isinstance(scale, bpy.types.NodeSocket):
            tree.links.new(scale, node.inputs["Scale"])
        else:
            node.inputs["Scale"].default_value = scale
    return node.outputs["Value" if operation == "DOT_PRODUCT" else "Vector"]


def lightmap_frame(tree):
    """(N, T, B) sockets: smooth normal, lightmap tangent orthogonal to N, N x T."""
    geometry = tree.nodes.new("ShaderNodeNewGeometry")
    tangent = tree.nodes.new("ShaderNodeTangent")
    tangent.direction_type = "UV_MAP"
    tangent.uv_map = "lightmap_st"
    normal = geometry.outputs["Normal"]
    along = vector_math(tree, "DOT_PRODUCT", tangent.outputs["Tangent"], normal)
    projected = vector_math(tree, "SCALE", normal, scale=along)
    t = vector_math(tree, "NORMALIZE", vector_math(tree, "SUBTRACT", tangent.outputs["Tangent"],
                                                   projected))
    b = vector_math(tree, "CROSS_PRODUCT", normal, t)
    return normal, t, b


def bake_sun_visibility(merged, scene, path, size, render):
    """Sun-only direct diffuse with / without shadows; their ratio is visibility."""
    import numpy as np
    light = scene["distant_lights"][0]
    suns = sorted((obj for obj in bpy.data.objects if obj.type == "LIGHT" and
                   obj.data.type == "SUN"), key=lambda obj: obj.name)
    if not suns:
        raise ValueError("scene has a distant light but Blender has no sun lamp")
    sun = suns[0]
    # Only the first sun lights these bakes: hide every other light source.
    hidden = [obj for obj in bpy.data.objects if obj is not sun and obj.hide_render is False and
              (obj.type == "LIGHT" or obj.name.startswith(pbrt_blender.EMITTER_PREFIXES))]
    for obj in hidden:
        obj.hide_render = True
    world = render.world
    background = world.node_tree.nodes.get("Background") if world and world.use_nodes else None
    strength = background.inputs["Strength"].default_value if background else None
    if background:
        background.inputs["Strength"].default_value = 0.0
    samples = render.cycles.samples
    render.cycles.samples = SUN_SAMPLES
    render.render.bake.use_pass_indirect = False
    planes = []
    for shadows in (True, False):
        sun.data.use_shadow = shadows
        image = bpy.data.images.new("PbrtSun%d" % shadows, width=size, height=size,
                                    alpha=True, float_buffer=True)
        for material in {slot.material for slot in merged.material_slots}:
            target = material.node_tree.nodes["BakeTarget"]
            target.image = image
            material.node_tree.nodes.active = target
        announce("sun %s" % ("shadowed" if shadows else "unshadowed"), size)
        bpy.ops.object.bake(type="DIFFUSE", pass_filter={"DIRECT"})
        planes.append(np.array(image.pixels[:], dtype=np.float64).reshape(size, size, 4))
    sun.data.use_shadow = True
    render.cycles.samples = samples
    render.render.bake.use_pass_indirect = True
    if background:
        background.inputs["Strength"].default_value = strength
    for obj in hidden:
        obj.hide_render = False
    shadowed, open_sky = (plane[..., :3].mean(axis=2) for plane in planes)
    lit = open_sky > 1e-6
    visibility = np.where(lit, np.clip(shadowed / np.maximum(open_sky, 1e-6), 0.0, 1.0), 0.0)
    image = bpy.data.images.new("PbrtSunVisibility", width=size, height=size, alpha=True,
                                float_buffer=True)
    image.pixels.foreach_set(np.dstack([visibility] * 3 + [np.ones_like(visibility)])
                             .astype(np.float32).ravel())
    image.save_render(filepath=str(path.resolve()), scene=render)
    if not path.is_file():
        raise RuntimeError("Cycles did not save the sun visibility")
    for material in {slot.material for slot in merged.material_slots}:
        material.node_tree.nodes["BakeTarget"].image = bpy.data.images["PbrtLightmap"]
    return {"source": light.get("source"), "direction": light["direction"],
            "irradiance": light["irradiance"], "angle_degrees": light["angle_degrees"],
            "samples": SUN_SAMPLES, "visibility_exr_sha256": sha256(path),
            "lit_fraction": float(lit.mean()),
            "ignored_distant_lights": len(scene["distant_lights"]) - 1}


# Separated diffuse light (RFC 0011): the same bake as the total atlas with
# one Cycles pass filter, so total = direct + indirect up to sampling noise.
SEPARATED_PASSES = {"direct": {"DIRECT"}, "indirect": {"INDIRECT"}}


def bake_separated_layers(merged, layers, out_dir, size, render):
    """Bake each separated layer into <out_dir>/<role>.exr; returns receipts."""
    out_dir.mkdir(parents=True, exist_ok=True)
    targets = [slot.material.node_tree.nodes["BakeTarget"] for slot in merged.material_slots
               if slot.material and "BakeTarget" in slot.material.node_tree.nodes]
    original = targets[0].image
    result = {}
    for role in layers:
        image = bpy.data.images.new("PbrtLightmap_" + role, width=size, height=size,
                                    alpha=True, float_buffer=True)
        for node in targets:
            node.image = image
        bpy.ops.object.select_all(action="DESELECT")
        merged.select_set(True)
        bpy.context.view_layer.objects.active = merged
        announce(role, size)
        bpy.ops.object.bake(type="DIFFUSE", pass_filter=SEPARATED_PASSES[role])
        path = out_dir / (role + ".exr")
        image.save_render(filepath=str(path.resolve()), scene=render)
        if not path.is_file():
            raise RuntimeError("Cycles did not save the %s lightmap layer" % role)
        result[role] = {"exr": path.name, "exr_sha256": sha256(path),
                        "pass_filter": sorted(SEPARATED_PASSES[role])}
    for node in targets:
        node.image = original
    return result


def bake_rnm(merged, out_dir, size, render):
    """Bake diffuse irradiance for each RNM basis normal; return EXR hashes."""
    out_dir.mkdir(parents=True, exist_ok=True)
    hashes = []
    for index, basis in enumerate(RNM_BASIS):
        image = bpy.data.images.new("PbrtLightmapRnm%d" % index, width=size, height=size,
                                    alpha=True, float_buffer=True)
        for material in {slot.material for slot in merged.material_slots}:
            tree = material.node_tree
            material.cycles.use_bump_map_correction = False
            normal, t, b = lightmap_frame(tree)
            direction = vector_math(tree, "NORMALIZE", vector_math(
                tree, "ADD", vector_math(tree, "ADD", vector_math(tree, "SCALE", t,
                                                                  scale=basis[0]),
                                         vector_math(tree, "SCALE", b, scale=basis[1])),
                vector_math(tree, "SCALE", normal, scale=basis[2])))
            for node in list(tree.nodes):
                if node.type.startswith("BSDF") and "Normal" in node.inputs:
                    tree.links.new(direction, node.inputs["Normal"])
            target = tree.nodes["BakeTarget"]
            target.image = image
            tree.nodes.active = target
        announce("RNM basis %d" % index, size)
        bpy.ops.object.bake(type="DIFFUSE", pass_filter={"DIRECT", "INDIRECT"})
        path = out_dir / ("rnm%d.exr" % index)
        image.save_render(filepath=str(path.resolve()), scene=render)
        if not path.is_file():
            raise RuntimeError("Cycles did not save RNM bake %d" % index)
        hashes.append(sha256(path))
    return {"basis": [list(basis) for basis in RNM_BASIS], "rnm_exr_sha256": hashes}


def bake_frame(merged, out_dir, size, render):
    """EMIT-bake the lightmap tangent frame (T and N, 0.5 + 0.5 v)."""
    samples = render.cycles.samples
    render.cycles.samples = FRAME_SAMPLES
    render.render.bake.margin = 0
    hashes = {}
    for axis in ("t", "n"):
        image = bpy.data.images.new("PbrtLightmapFrame" + axis, width=size, height=size,
                                    alpha=True, float_buffer=True)
        material = bpy.data.materials.new("PbrtLightmapFrame" + axis)
        material.use_nodes = True
        tree = material.node_tree
        tree.nodes.clear()
        output = tree.nodes.new("ShaderNodeOutputMaterial")
        emission = tree.nodes.new("ShaderNodeEmission")
        normal, t, _ = lightmap_frame(tree)
        encoded = vector_math(tree, "MULTIPLY_ADD", t if axis == "t" else normal,
                              (0.5, 0.5, 0.5), (0.5, 0.5, 0.5))
        tree.links.new(encoded, emission.inputs["Color"])
        tree.links.new(emission.outputs[0], output.inputs[0])
        target = tree.nodes.new("ShaderNodeTexImage")
        target.image = image
        tree.nodes.active = target
        merged.data.materials.clear()
        merged.data.materials.append(material)
        announce("frame %s" % axis, size)
        if bpy.ops.object.bake(type="EMIT") != {"FINISHED"}:
            raise RuntimeError("Cycles could not bake the lightmap frame")
        path = out_dir / ("frame_%s.exr" % axis)
        image.save_render(filepath=str(path.resolve()), scene=render)
        if not path.is_file():
            raise RuntimeError("Cycles did not save the lightmap frame")
        hashes[axis] = sha256(path)
    render.cycles.samples = samples
    return hashes


def main():
    arguments = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--scene", type=Path, required=True)
    parser.add_argument("--stage", type=Path, required=True)
    parser.add_argument("--environment", type=Path)
    parser.add_argument("--out-stage", type=Path, required=True)
    parser.add_argument("--out-exr", type=Path, required=True)
    parser.add_argument("--out-coverage-exr", type=Path,
                        help="undilated white UV footprint for safe gutter filling")
    parser.add_argument("--size", type=int, default=2048)
    parser.add_argument("--samples", type=int, default=64)
    parser.add_argument("--exclude-material", action="append", default=[])
    parser.add_argument("--device", choices=pbrt_blender.DEVICES,
                        default=pbrt_blender.DEFAULT_DEVICE)
    parser.add_argument("--light-paths", choices=sorted(pbrt_blender.LIGHT_PATH_POLICIES),
                        default="blender-default",
                        help="Cycles bounce/clamp policy (pbrt_blender.LIGHT_PATH_POLICIES)")
    parser.add_argument("--reserve-rows", type=int, default=0,
                        help="keep the atlas's first N texel rows (lightmap v < N/size) "
                             "empty for a reflection probe band")
    parser.add_argument("--directional-dir", type=Path,
                        help="also bake RNM-basis irradiance and the tangent frame here")
    parser.add_argument("--layers", default="",
                        help="comma-separated separated-light layers to bake beside the total "
                             "atlas (direct, indirect): LMAP v2 layers (RFC 0011)")
    parser.add_argument("--layers-dir", type=Path,
                        help="directory for the separated layer EXRs (<role>.exr)")
    parser.add_argument("--seed", type=int, default=pbrt_blender.SEED,
                        help="Cycles seed for every bake pass (recorded in the receipt)")
    parser.add_argument("--margin-texels", type=int, default=2,
                        help="gap between packed charts; bake dilation uses half")
    args = parser.parse_args(arguments)
    if args.size < 256 or args.samples < 1 or not os.environ.get("OCIO"):
        parser.error("valid atlas size, samples and OCIO are required")
    if args.stage.resolve() == args.out_stage.resolve():
        parser.error("output stage must differ from input stage")
    layers = [role for role in args.layers.split(",") if role]
    if any(role not in SEPARATED_PASSES for role in layers) or len(set(layers)) != len(layers):
        parser.error("--layers takes distinct roles from: " + ", ".join(SEPARATED_PASSES))
    if layers and not args.layers_dir:
        parser.error("--layers needs --layers-dir")
    scene = map_scene.parse(args.scene)
    unknown = set(args.exclude_material) - set(scene["materials"])
    if unknown:
        parser.error("excluded materials are not in the PBRT scene: " + ", ".join(sorted(unknown)))
    # Transmissive previews are unlit, and the WMSH PBR shader weights baked
    # diffuse by (1 - metalness), so neither reads the atlas.
    excluded = set(args.exclude_material) | {
        name for name in scene["materials"]
        if map_scene.material_summary(scene, name)["transmission"] > 0 or
        map_scene.material_summary(scene, name)["metallic"] >= 1.0}
    pbrt_blender.clear_scene()
    message("importing the stage")
    if bpy.ops.wm.usd_import(filepath=str(args.stage.resolve()), import_materials=True,
                             import_lights=True, import_cameras=True) != {"FINISHED"}:
        raise RuntimeError("could not import the USD stage")
    meshes = pbrt_blender.source_meshes()
    assignments = {shape["name"]: shape["material"] for shape in scene["shapes"]}
    if sorted(obj.name for obj in meshes) != sorted(assignments):
        raise ValueError("USD stage meshes differ from the PBRT scene")
    baked = []
    # Dynamic models are not static lighting: their stand-ins neither get
    # atlas space nor take part in the bake's light transport.
    props = map_scene.prop_shape_names(scene)
    for obj in meshes:
        if not obj.data.uv_layers.get("st"):
            raise ValueError("source mesh lacks material UVs: " + obj.name)
        obj.data.uv_layers.active = obj.data.uv_layers.new(name="lightmap_st")
        if obj.name in props or assignments[obj.name] in excluded:
            # One texel at the far corner, away from a reflection probe band.
            corner = 1.0 - 0.5 / args.size
            for loop in obj.data.uv_layers.active.data:
                loop.uv = (corner, corner)
        else:
            baked.append(obj)
    if not baked:
        raise ValueError("no meshes selected for baking")
    message("packing lightmap UVs for %d meshes" % len(baked))
    # Keep roughly four texels between charts at the requested atlas size.
    projected = pack_lightmap_uvs(baked, margin=args.margin_texels / args.size)
    if args.reserve_rows:
        # Atlas texel row r holds lightmap v = (r + 0.5) / size after the KTX2
        # packager's row flip, so rows [0, N) are v < N / size. Squeeze every
        # chart into v >= N / size; lightmap_ktx2 asserts those rows are empty.
        reserved = (args.reserve_rows + args.margin_texels) / args.size
        if not 0 < reserved < 0.5:
            parser.error("reserved rows must be a small part of the atlas")
        for obj in baked + [value["proxy"] for value in projected.values()]:
            for loop in obj.data.uv_layers.get("lightmap_st").data:
                loop.uv = (loop.uv[0], reserved + loop.uv[1] * (1.0 - reserved))
    extents = {}
    density = {}
    for obj in baked:
        # A projected mesh's parts overlap in its chart; its footprint proxy
        # carries the chart's texel density.
        measured = projected[obj.name]["proxy"] if obj.name in projected else obj
        layer = measured.data.uv_layers.get("lightmap_st").data
        uv_area = 0.0
        for polygon in measured.data.polygons:
            uv = [layer[index].uv for index in polygon.loop_indices]
            for k in range(1, len(uv) - 1):
                uv_area += abs((uv[k].x - uv[0].x) * (uv[k + 1].y - uv[0].y) -
                               (uv[k + 1].x - uv[0].x) * (uv[k].y - uv[0].y)) / 2
        world_area = sum(polygon.area for polygon in measured.data.polygons)
        if world_area > 1e-6:
            density[obj.name] = uv_area * args.size * args.size / world_area
    if min(density.values()) <= 0:
        raise ValueError("lightmap packing left meshes without atlas area: " +
                         ", ".join(sorted(n for n, v in density.items() if v <= 0)))
    spread = max(density.values()) / min(density.values())
    if spread > 1.5:
        raise ValueError("lightmap texel density varies %.2fx across meshes" % spread)
    for value in projected.values():
        bpy.data.meshes.remove(value["proxy"].data)
    for obj in meshes:
        layer = obj.data.uv_layers.get("lightmap_st")
        values = [tuple(loop.uv) for loop in layer.data]
        if any(not (0 <= u <= 1 and 0 <= v <= 1) for u, v in values):
            raise ValueError("packed lightmap UVs escaped their atlas: " + obj.name)
        extents[obj.name] = [min(v[0] for v in values), min(v[1] for v in values),
                             max(v[0] for v in values), max(v[1] for v in values)]
        # Export the material UVs as the default `st` layer.
        obj.data.uv_layers.active = obj.data.uv_layers.get("st")
    args.out_stage.parent.mkdir(parents=True, exist_ok=True)
    pbrt_blender.sort_collections()
    if bpy.ops.wm.usd_export(filepath=str(args.out_stage.resolve()), export_materials=True,
                             export_uvmaps=True, rename_uvmaps=False, export_normals=True,
                             export_cameras=True, export_lights=True,
                             triangulate_meshes=True,
                             export_textures_mode="NEW") != {"FINISHED"}:
        raise RuntimeError("could not export the lighting USD stage")
    pbrt_blender.rebind_materials(scene, normal_maps=False)
    pbrt_blender.restore_emitters(scene)
    pbrt_blender.apply_environment(scene, args.environment)
    for obj in meshes:
        if obj.name in props:
            obj.hide_render = True
    device = pbrt_blender.configure_cycles(args.samples, args.device)
    light_paths = pbrt_blender.configure_light_paths(args.light_paths)
    sampling = pbrt_blender.pin_sampling(seed=args.seed)
    render = bpy.context.scene
    render.cycles.use_auto_tile = True
    render.cycles.tile_size = BAKE_TILE
    render.render.bake.use_clear = True
    render.render.bake.margin = max(1, args.margin_texels // 2)
    render.render.bake.use_pass_color = False
    render.render.bake.use_pass_direct = True
    render.render.bake.use_pass_indirect = True
    atlas = bpy.data.images.new("PbrtLightmap", width=args.size, height=args.size,
                                alpha=True, float_buffer=True)
    for obj in baked:
        obj.data.uv_layers.active = obj.data.uv_layers.get("lightmap_st")
    # Blender bakes each selected object as its own render job, re-syncing the
    # whole scene every time (54 jobs for the living room). Bake one merged,
    # bake-only copy instead and hide the originals from render so light
    # transport sees the same geometry once.
    bpy.ops.object.select_all(action="DESELECT")
    for obj in baked:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = baked[0]
    bpy.ops.object.duplicate()
    bpy.ops.object.join()
    merged = bpy.context.view_layer.objects.active
    merged.data.uv_layers.active = merged.data.uv_layers.get("lightmap_st")
    for obj in baked:
        obj.hide_render = True
    for material in {slot.material for slot in merged.material_slots}:
        if not material or not material.use_nodes:
            raise ValueError("baked mesh has no Cycles material")
        node = material.node_tree.nodes.new("ShaderNodeTexImage")
        node.name = "BakeTarget"
        node.image = atlas
        material.node_tree.nodes.active = node
    bpy.ops.object.select_all(action="DESELECT")
    merged.select_set(True)
    bpy.context.view_layer.objects.active = merged
    message("baking a %d x %d atlas at %d samples: total%s%s%s%s" % (
        args.size, args.size, args.samples, "".join(", " + role for role in layers),
        ", sun visibility" if scene.get("distant_lights") else "",
        ", RNM basis" if args.directional_dir else "",
        ", coverage" if args.out_coverage_exr else ""))
    announce("total", args.size)
    bpy.ops.object.bake(type="DIFFUSE", pass_filter={"DIRECT", "INDIRECT"})
    args.out_exr.parent.mkdir(parents=True, exist_ok=True)
    # Image.save() applies the display transform to generated images even for
    # EXR; save_render() keeps scene-linear texels.
    render.render.image_settings.file_format = "OPEN_EXR"
    render.render.image_settings.color_depth = "32"
    atlas.save_render(filepath=str(args.out_exr.resolve()), scene=render)
    if not args.out_exr.is_file():
        raise RuntimeError("Cycles did not save the lightmap atlas")
    separated = bake_separated_layers(merged, layers, args.layers_dir, args.size, render) \
        if layers else {}
    sun = None
    if scene.get("distant_lights"):
        sun = bake_sun_visibility(merged, scene, args.out_exr.with_name("sun_visibility.exr"),
                                  args.size, render)
    directional = None
    if args.directional_dir:
        directional = bake_rnm(merged, args.directional_dir, args.size, render)
    if args.out_coverage_exr:
        coverage = bpy.data.images.new("PbrtUvCoverage", width=args.size, height=args.size,
                                       alpha=True, float_buffer=True)
        material = bpy.data.materials.new("PbrtUvCoverageWhite")
        material.use_nodes = True
        nodes = material.node_tree.nodes
        nodes.clear()
        output = nodes.new("ShaderNodeOutputMaterial")
        emission = nodes.new("ShaderNodeEmission")
        emission.inputs["Color"].default_value = (1.0, 1.0, 1.0, 1.0)
        material.node_tree.links.new(emission.outputs[0], output.inputs[0])
        target = nodes.new("ShaderNodeTexImage")
        target.image = coverage
        nodes.active = target
        merged.data.materials.clear()
        merged.data.materials.append(material)
        render.render.bake.margin = 0
        announce("coverage", args.size)
        if bpy.ops.object.bake(type="EMIT") != {"FINISHED"}:
            raise RuntimeError("Cycles could not bake the UV footprint")
        args.out_coverage_exr.parent.mkdir(parents=True, exist_ok=True)
        coverage.save_render(filepath=str(args.out_coverage_exr.resolve()), scene=render)
        if not args.out_coverage_exr.is_file():
            raise RuntimeError("Cycles did not save the UV footprint")
    if directional is not None:
        directional["frame_exr_sha256"] = bake_frame(merged, args.directional_dir, args.size,
                                                     render)
        directional["frame_samples"] = FRAME_SAMPLES
    pixels = list(atlas.pixels)
    sampled = []
    stride = max(1, args.size // 16)
    for y in range(stride // 2, args.size, stride):
        for x in range(stride // 2, args.size, stride):
            offset = (y * args.size + x) * 4
            rgb = tuple(pixels[offset:offset + 3])
            peak = max(rgb)
            if all(math.isfinite(v) for v in rgb) and 0.05 < peak < 5 and abs(peak - 1) > 0.1:
                sampled.append((offset, rgb))
    if not sampled:
        raise ValueError("Cycles lightmap has no nontrivial linear validation texels")
    saved = list(bpy.data.images.load(str(args.out_exr.resolve()), check_existing=False).pixels)
    error = max(abs(saved[offset + c] - rgb[c]) for offset, rgb in sampled for c in range(3))
    if error > 1e-4:
        raise ValueError("Cycles lightmap EXR differs from its linear bake buffer")
    covered = sum(1 for i in range(3, len(pixels), 4 * 7) if pixels[i] > 0) / (len(pixels) / 28)
    evidence = {"status": "pass", "scope": SCOPE,
                "source_stage_sha256": sha256(args.stage),
                "lighting_stage_sha256": sha256(args.out_stage),
                "scene_sha256": scene["source_sha256"], "atlas_exr_sha256": sha256(args.out_exr),
                "environment_sha256": sha256(args.environment) if args.environment else None,
                "size": args.size, "samples": args.samples, "device": device,
                "sampling": sampling, "determinism": cycles_device.determinism(device, False),
                "reserved_rows": args.reserve_rows,
                "mesh_count": len(meshes), "baked_mesh_count": len(baked),
                "projected_meshes": {name: {"parts": value["parts"]}
                                     for name, value in projected.items()},
                "excluded_materials": sorted(excluded),
                "excluded_dynamic_models": sorted(props),
                "layers": separated,
                "light_paths": light_paths,
                "emitter_count": len(scene["emitters"]),
                "atlas_coverage_estimate": covered,
                "coverage_exr_sha256": sha256(args.out_coverage_exr)
                if args.out_coverage_exr else None,
                "directional": directional, "sun": sun,
                "texels_per_square_meter": {"min": min(density.values()),
                                            "max": max(density.values())},
                "linear_exr_sample_count": len(sampled), "linear_exr_max_sample_error": error,
                "uv_extents": extents, "blender": bpy.app.version_string,
                "ocio_configuration_sha256": sha256(os.environ["OCIO"])}
    args.out_exr.with_name(args.out_exr.name + ".json").write_text(
        json.dumps(evidence, indent=2, sort_keys=True) + "\n")
    print("PBRT_LIGHTMAP_BAKE " + json.dumps({k: evidence[k] for k in
                                              ("status", "baked_mesh_count", "size")}))


if __name__ == "__main__":
    main()
