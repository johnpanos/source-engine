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
CHART_PREFIX = "_lightmap_chart_"
CHART_FACE = "_lightmap_chart_face"
CHART_OBJECT = "_lightmap_chart_object"
# Vertices this close (stage metres) are one vertex for charting.
WELD_DISTANCE = 1e-6
T_JUNCTION_PASSES = 64
# Edges sharper than this never join two charts (smart_project's angle limit
# is 66 degrees; see split_creases).
CREASE_DEGREES = 60.0
# A source corner and its charting-surface corner, both in world space, agree
# to float32 rounding of the world transform.
MATCH_DISTANCE = 1e-5


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


def chart_copy(obj, weld):
    """A charting copy of `obj` in world space, each face tagged with its
    source face index, with coincident vertices welded and T-junctions split.

    Charts are connected regions, so faces must share vertices and edges: a
    triangle soup (usd_scene's normalized stages, relit BSP faces) would
    chart every triangle alone. The source mesh itself is never modified -
    welding re-encodes Blender's custom corner normals and moves merged
    vertices - so its positions, normals and material UVs stay exactly as
    imported; only the lightmap UVs are copied back (`transfer_chart`).
    """
    mesh = obj.data.copy()
    mesh.transform(obj.matrix_world)
    tag = mesh.attributes.new(CHART_FACE, "INT", "FACE")
    tag.data.foreach_set("value", list(range(len(mesh.polygons))))
    copy = bpy.data.objects.new(CHART_PREFIX + obj.name, mesh)
    bpy.context.scene.collection.objects.link(copy)
    if weld:
        weld_mesh(mesh)
    return copy


def weld_mesh(mesh):
    import bmesh
    bm = bmesh.new()
    bm.from_mesh(mesh)
    bmesh.ops.remove_doubles(bm, verts=bm.verts, dist=WELD_DISTANCE)
    split_t_junctions(bm)
    bm.to_mesh(mesh)
    bm.free()


def split_creases(mesh):
    """Cut the charting surface along sharp edges (dihedral above
    CREASE_DEGREES). smart_project projects the faces on either side onto
    different planes, but where both projections give the shared edge the
    same UVs the packer treats the two faces as one island and packs them
    touching: lookups along the corner then read the other face's light."""
    import bmesh
    bm = bmesh.new()
    bm.from_mesh(mesh)
    limit = math.radians(CREASE_DEGREES)
    sharp = [edge for edge in bm.edges if len(edge.link_faces) == 2 and
             edge.calc_face_angle(0.0) > limit]
    if sharp:
        bmesh.ops.split_edges(bm, edges=sharp)
    bm.to_mesh(mesh)
    bm.free()


def join_chart_copies(copies, weld):
    """One charting surface from per-object copies, faces tagged with their
    object index (CHART_OBJECT), welded across objects.

    Lightmap UVs do not depend on material, so a flat wall made of several
    materials - several meshes - must chart as one island: charted apart,
    every material boundary would be a lightmap seam."""
    for index, copy in enumerate(copies):
        tag = copy.data.attributes.new(CHART_OBJECT, "INT", "FACE")
        tag.data.foreach_set("value", [index] * len(copy.data.polygons))
    bpy.ops.object.select_all(action="DESELECT")
    for copy in copies:
        copy.select_set(True)
    bpy.context.view_layer.objects.active = copies[0]
    if len(copies) > 1 and bpy.ops.object.join() != {"FINISHED"}:
        raise RuntimeError("Blender could not join the lightmap charting copies")
    joined = bpy.context.view_layer.objects.active
    joined.name = CHART_PREFIX + "surface"
    if weld:
        weld_mesh(joined.data)
    split_creases(joined.data)
    joined.data.uv_layers.active = joined.data.uv_layers["lightmap_st"]
    return joined


def split_t_junctions(bm):
    """Split open edges at vertices lying inside them, then weld.

    Where one face's edge meets two faces' shorter edges (a T-junction) the
    faces share no edge and would chart apart. Splitting the long edge at
    the junction vertex gives them a common edge; faces keep their tags.
    Each pass splits every open edge at its first interior junction.
    """
    import bmesh
    from mathutils import kdtree
    for _ in range(T_JUNCTION_PASSES):
        open_edges = [e for e in bm.edges if len(e.link_faces) == 1]
        open_verts = list({v for e in open_edges for v in e.verts})
        if not open_verts:
            return
        tree = kdtree.KDTree(len(open_verts))
        for index, vert in enumerate(open_verts):
            tree.insert(vert.co, index)
        tree.balance()
        splits = []
        for edge in open_edges:
            a, b = edge.verts[0].co, edge.verts[1].co
            length = (b - a).length
            if length <= 2 * WELD_DISTANCE:
                continue
            first = None
            for co, _, _ in tree.find_range((a + b) / 2, length / 2 + WELD_DISTANCE):
                t = (co - a).dot(b - a) / (length * length)
                if WELD_DISTANCE / length < t < 1 - WELD_DISTANCE / length and \
                        (a + (b - a) * t - co).length <= WELD_DISTANCE:
                    first = t if first is None else min(first, t)
            if first is not None:
                splits.append((edge, first))
        if not splits:
            return
        for edge, t in splits:
            bmesh.utils.edge_split(edge, edge.verts[0], t)
        bmesh.ops.remove_doubles(bm, verts=bm.verts, dist=WELD_DISTANCE)
    raise RuntimeError("T-junction splitting did not converge in %d passes" % T_JUNCTION_PASSES)


def transfer_chart(surface, obj, index):
    """Copy the charting surface's lightmap UVs onto `obj`'s corners: each
    source corner takes the UV of the surface corner of its tagged face at
    the same world position (exactly one must match); faces the weld
    collapsed (zero area) keep a degenerate UV. Returns their corner count."""
    import numpy as np
    source, charted = obj.data, surface.data
    uv = np.zeros((len(source.loops), 2))
    matched = np.zeros(len(source.loops), dtype=bool)
    objects = np.empty(len(charted.polygons), dtype=np.int64)
    faces = np.empty(len(charted.polygons), dtype=np.int64)
    charted.attributes[CHART_OBJECT].data.foreach_get("value", objects)
    charted.attributes[CHART_FACE].data.foreach_get("value", faces)
    chart_uv = np.empty(len(charted.loops) * 2)
    charted.uv_layers["lightmap_st"].data.foreach_get("uv", chart_uv)
    chart_uv = chart_uv.reshape(-1, 2)
    chart_co = np.empty(len(charted.vertices) * 3)
    charted.vertices.foreach_get("co", chart_co)
    chart_co = chart_co.reshape(-1, 3)
    chart_vertex = np.empty(len(charted.loops), dtype=np.int64)
    charted.loops.foreach_get("vertex_index", chart_vertex)
    source_co = np.empty(len(source.vertices) * 3)
    source.vertices.foreach_get("co", source_co)
    world = np.array(obj.matrix_world)
    source_co = source_co.reshape(-1, 3) @ world[:3, :3].T + world[:3, 3]
    source_vertex = np.empty(len(source.loops), dtype=np.int64)
    source.loops.foreach_get("vertex_index", source_vertex)
    tolerance = max(MATCH_DISTANCE, 8 * np.finfo(np.float32).eps * np.abs(source_co).max())
    for polygon_index in np.flatnonzero(objects == index):
        polygon = charted.polygons[polygon_index]
        face = source.polygons[faces[polygon_index]]
        corners = np.arange(polygon.loop_start, polygon.loop_start + polygon.loop_total)
        positions = chart_co[chart_vertex[corners]]
        for loop in range(face.loop_start, face.loop_start + face.loop_total):
            distance = np.linalg.norm(positions - source_co[source_vertex[loop]], axis=1)
            close = np.flatnonzero(distance <= tolerance)
            if len(close) != 1:
                raise RuntimeError("lightmap chart corner of %s face %d matches %d corners"
                                   % (obj.name, face.index, len(close)))
            uv[loop] = chart_uv[corners[close[0]]]
            matched[loop] = True
    # A face the weld collapsed has zero area: park it on one texel.
    uv[~matched] = uv[matched][0] if matched.any() else 0.0
    source.uv_layers["lightmap_st"].data.foreach_set("uv", uv.ravel())
    return int((~matched).sum())


def pack_lightmap_uvs(meshes, margin, weld=True):
    """Chart and jointly pack `meshes`; return {name: parts} of projected meshes.

    Charting runs on one welded world-space surface of all the meshes
    (`chart_copy`, `join_chart_copies`), so coplanar regions chart as one
    island across mesh and material boundaries; `weld=False` charts the
    triangles as they are connected, for tests that show what the weld is for. A mesh
    with more than PROJECTED_PART_LIMIT separate parts after welding (fur,
    grass, foliage cards) gets one planar chart of its footprint instead of
    one chart per part: hundreds of thousands of islands make Blender's
    FRACTION margin pack collapse the whole atlas to nothing. A footprint
    proxy quad is packed with the other charts so the projected chart gets
    the same texel density and margin; overlapping parts share its texels.
    """
    copies = {obj.name: chart_copy(obj, weld) for obj in meshes}
    parts = {obj.name: connected_parts(copies[obj.name]) for obj in meshes}
    projected = [obj for obj in meshes if parts[obj.name] > PROJECTED_PART_LIMIT]
    for obj in projected:
        copy = copies.pop(obj.name)
        mesh = copy.data
        bpy.data.objects.remove(copy, do_unlink=True)
        bpy.data.meshes.remove(mesh)
    proxies = {obj.name: add_footprint_proxy(obj) for obj in projected}
    kept = [obj for obj in meshes if obj.name in copies]
    surface = join_chart_copies([copies[obj.name] for obj in kept], weld) if kept else None
    charted = ([surface] if surface else []) + list(proxies.values())
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
    if surface:
        for index, obj in enumerate(kept):
            transfer_chart(surface, obj, index)
        mesh = surface.data
        bpy.data.objects.remove(surface, do_unlink=True)
        bpy.data.meshes.remove(mesh)
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


# --noise-pair: every light page is the mean of two independent half-sample
# bakes (seeds s and s + 1); the total's halves go to this directory for the
# noise gate (lightmap_noise.py). None bakes each page once.
NOISE_PAIR = {"dir": None, "halves": {}}


def bake_light(image, pass_filter, label, size, render, keep=None):
    """DIFFUSE-bake the light into `image` (its BakeTarget nodes already set):
    once, or as the mean of two half-sample halves; `keep` names halves to
    save as <dir>/<keep>-a.exr and -b.exr. Returns the halves (None when
    baked once)."""
    import numpy as np
    if NOISE_PAIR["dir"] is None:
        announce(label, size)
        bpy.ops.object.bake(type="DIFFUSE", pass_filter=pass_filter)
        return None
    samples, seed = render.cycles.samples, render.cycles.seed
    halves = []
    try:
        render.cycles.samples = max(1, (samples + 1) // 2)
        for index in range(2):
            render.cycles.seed = seed + index
            announce("%s half %d" % (label, index + 1), size)
            bpy.ops.object.bake(type="DIFFUSE", pass_filter=pass_filter)
            halves.append(np.array(image.pixels[:], dtype=np.float32))
    finally:
        render.cycles.samples, render.cycles.seed = samples, seed
    if keep:
        save_halves(halves, keep, size, render)
    image.pixels.foreach_set((halves[0] + halves[1]) / 2)
    return halves


def save_halves(halves, keep, size, render):
    """Write a page's two half-sample bakes as <dir>/<keep>-a.exr and -b.exr."""
    for index, half in enumerate(halves):
        copy = bpy.data.images.new("%s_half%d" % (keep, index), width=size, height=size,
                                   alpha=True, float_buffer=True)
        copy.pixels.foreach_set(half)
        path = NOISE_PAIR["dir"] / ("%s-%s.exr" % (keep, "ab"[index]))
        copy.save_render(filepath=str(path.resolve()), scene=render)
        if not path.is_file():
            raise RuntimeError("Cycles did not save noise half " + path.name)
        NOISE_PAIR["halves"][path.name] = sha256(path)


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
# one Cycles pass filter. When both are baked the total is their sum
# (summed_total) instead of a third bake, so total = direct + indirect exactly.
SEPARATED_PASSES = {"direct": {"DIRECT"}, "indirect": {"INDIRECT"}}


def summed_total(layers):
    """Whether the total atlas is the sum of the separated layers."""
    return set(SEPARATED_PASSES) <= set(layers)


def bake_separated_layers(merged, layers, out_dir, size, render, parts=None):
    """Bake each separated layer into <out_dir>/<role>.exr; returns receipts.
    `parts`, when given, receives each role's (pixels, halves)."""
    import numpy as np
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
        halves = bake_light(image, SEPARATED_PASSES[role], role, size, render)
        if parts is not None:
            parts[role] = (np.array(image.pixels[:], dtype=np.float32), halves)
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
        bake_light(image, {"DIRECT", "INDIRECT"}, "RNM basis %d" % index, size, render)
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
    parser.add_argument("--noise-pair-dir", type=Path,
                        help="bake every light page as the mean of two half-sample halves "
                             "and keep the total's halves here (lightmap_noise.py)")
    parser.add_argument("--layout", choices=("blender", "authored"), default="blender",
                        help="authored: use the stage's lightmap_st (lightmap_layout.py) "
                             "unchanged; blender: chart and pack in Blender")
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
    excluded, unbaked = map_scene.lightmap_exclusions(scene, args.exclude_material)
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
    authored = args.layout == "authored"
    for obj in meshes:
        if not obj.data.uv_layers.get("st"):
            raise ValueError("source mesh lacks material UVs: " + obj.name)
        if authored:
            # lightmap_layout.py wrote every mesh's lightmap UVs, parked ones
            # included; the bake uses them as they are.
            if not obj.data.uv_layers.get("lightmap_st"):
                raise ValueError("stage mesh lacks authored lightmap_st: " + obj.name)
            obj.data.uv_layers.active = obj.data.uv_layers["lightmap_st"]
            if obj.name not in unbaked:
                baked.append(obj)
            continue
        obj.data.uv_layers.active = obj.data.uv_layers.new(name="lightmap_st")
        if obj.name in unbaked:
            # One texel at the far corner, away from a reflection probe band.
            corner = 1.0 - 0.5 / args.size
            for loop in obj.data.uv_layers.active.data:
                loop.uv = (corner, corner)
        else:
            baked.append(obj)
    if not baked:
        raise ValueError("no meshes selected for baking")
    if authored:
        projected = {}
    else:
        message("packing lightmap UVs for %d meshes" % len(baked))
        # Keep roughly four texels between charts at the requested atlas size.
        projected = pack_lightmap_uvs(baked, margin=args.margin_texels / args.size)
    if args.reserve_rows and not authored:
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
    if args.noise_pair_dir:
        args.noise_pair_dir.mkdir(parents=True, exist_ok=True)
        NOISE_PAIR["dir"] = args.noise_pair_dir
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
    render.render.image_settings.file_format = "OPEN_EXR"
    render.render.image_settings.color_depth = "32"
    import numpy as np
    separated, parts = {}, {}
    if summed_total(layers):
        separated = bake_separated_layers(merged, layers, args.layers_dir, args.size, render,
                                          parts)
        (direct, direct_halves), (indirect, indirect_halves) = \
            parts["direct"], parts["indirect"]
        total = direct + indirect
        total[3::4] = np.maximum(direct[3::4], indirect[3::4])
        atlas.pixels.foreach_set(total)
        if direct_halves is not None:
            halves = [a + b for a, b in zip(direct_halves, indirect_halves)]
            for half, a, b in zip(halves, direct_halves, indirect_halves):
                half[3::4] = np.maximum(a[3::4], b[3::4])
            save_halves(halves, "total", args.size, render)
    else:
        bake_light(atlas, {"DIRECT", "INDIRECT"}, "total", args.size, render, keep="total")
    args.out_exr.parent.mkdir(parents=True, exist_ok=True)
    # Image.save() applies the display transform to generated images even for
    # EXR; save_render() keeps scene-linear texels.
    render.render.image_settings.file_format = "OPEN_EXR"
    render.render.image_settings.color_depth = "32"
    atlas.save_render(filepath=str(args.out_exr.resolve()), scene=render)
    if not args.out_exr.is_file():
        raise RuntimeError("Cycles did not save the lightmap atlas")
    if layers and not separated:
        separated = bake_separated_layers(merged, layers, args.layers_dir, args.size, render)
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
                "noise_pair": {"half_samples": max(1, (args.samples + 1) // 2),
                               "seeds": [args.seed, args.seed + 1],
                               "halves": NOISE_PAIR["halves"]} if args.noise_pair_dir else None,
                "reserved_rows": args.reserve_rows,
                "mesh_count": len(meshes), "baked_mesh_count": len(baked),
                "projected_meshes": {name: {"parts": value["parts"]}
                                     for name, value in projected.items()},
                "excluded_materials": sorted(excluded), "layout": args.layout,
                "excluded_dynamic_models": sorted(props),
                "layers": separated,
                "total": "direct + indirect" if summed_total(layers) else "baked",
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
