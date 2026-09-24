"""Author shared `lightmap_st` UVs on a PBRT-derived USD stage and bake Cycles light.

Run inside Blender; `pbrt_map_build.py` drives it. Meshes whose translated
material is transmissive or fully metallic, plus any `--exclude-material`,
keep a degenerate
lightmap UV at (0, 0) and get no atlas space. Every other source mesh is
smart-projected, area-normalized and packed into one atlas, then baked as
scene-linear diffuse irradiance (direct + indirect, albedo divided out) under
the scene's area emitters and sky.
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
import pbrt_blender  # noqa: E402
import pbrt_scene  # noqa: E402

SCOPE = "pbrt-shared-lightmap-uv-and-cycles-bake"
PROJECTED_PART_LIMIT = 4096
PROXY_PREFIX = "_lightmap_footprint_"


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
    parser.add_argument("--device", choices=("cpu", "gpu", "auto"), default="auto")
    parser.add_argument("--reserve-rows", type=int, default=0,
                        help="keep the atlas's first N texel rows (lightmap v < N/size) "
                             "empty for a reflection probe band")
    parser.add_argument("--margin-texels", type=int, default=2,
                        help="gap between packed charts; bake dilation uses half")
    args = parser.parse_args(arguments)
    if args.size < 256 or args.samples < 1 or not os.environ.get("OCIO"):
        parser.error("valid atlas size, samples and OCIO are required")
    if args.stage.resolve() == args.out_stage.resolve():
        parser.error("output stage must differ from input stage")
    scene = pbrt_scene.parse(args.scene)
    unknown = set(args.exclude_material) - set(scene["materials"])
    if unknown:
        parser.error("excluded materials are not in the PBRT scene: " + ", ".join(sorted(unknown)))
    # Transmissive previews are unlit, and the WMSH PBR shader weights baked
    # diffuse by (1 - metalness), so neither reads the atlas.
    excluded = set(args.exclude_material) | {
        name for name in scene["materials"]
        if pbrt_scene.material_summary(scene, name)["transmission"] > 0 or
        pbrt_scene.material_summary(scene, name)["metallic"] >= 1.0}
    pbrt_blender.clear_scene()
    if bpy.ops.wm.usd_import(filepath=str(args.stage.resolve()), import_materials=True,
                             import_lights=True, import_cameras=True) != {"FINISHED"}:
        raise RuntimeError("could not import the USD stage")
    meshes = pbrt_blender.source_meshes()
    assignments = {shape["name"]: shape["material"] for shape in scene["shapes"]}
    if sorted(obj.name for obj in meshes) != sorted(assignments):
        raise ValueError("USD stage meshes differ from the PBRT scene")
    baked = []
    for obj in meshes:
        if not obj.data.uv_layers.get("st"):
            raise ValueError("source mesh lacks material UVs: " + obj.name)
        obj.data.uv_layers.active = obj.data.uv_layers.new(name="lightmap_st")
        if assignments[obj.name] in excluded:
            # One texel at the far corner, away from a reflection probe band.
            corner = 1.0 - 0.5 / args.size
            for loop in obj.data.uv_layers.active.data:
                loop.uv = (corner, corner)
        else:
            baked.append(obj)
    if not baked:
        raise ValueError("no meshes selected for baking")
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
    if bpy.ops.wm.usd_export(filepath=str(args.out_stage.resolve()), export_materials=True,
                             export_uvmaps=True, rename_uvmaps=False, export_normals=True,
                             export_cameras=True, export_lights=True,
                             triangulate_meshes=True,
                             export_textures_mode="NEW") != {"FINISHED"}:
        raise RuntimeError("could not export the lighting USD stage")
    pbrt_blender.rebind_materials(scene)
    pbrt_blender.restore_emitters(scene)
    pbrt_blender.apply_environment(scene, args.environment)
    device = pbrt_blender.configure_cycles(args.samples, args.device)
    render = bpy.context.scene
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
    bpy.ops.object.bake(type="DIFFUSE", pass_filter={"DIRECT", "INDIRECT"})
    args.out_exr.parent.mkdir(parents=True, exist_ok=True)
    # Image.save() applies the display transform to generated images even for
    # EXR; save_render() keeps scene-linear texels.
    render.render.image_settings.file_format = "OPEN_EXR"
    render.render.image_settings.color_depth = "32"
    atlas.save_render(filepath=str(args.out_exr.resolve()), scene=render)
    if not args.out_exr.is_file():
        raise RuntimeError("Cycles did not save the lightmap atlas")
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
        if bpy.ops.object.bake(type="EMIT") != {"FINISHED"}:
            raise RuntimeError("Cycles could not bake the UV footprint")
        args.out_coverage_exr.parent.mkdir(parents=True, exist_ok=True)
        coverage.save_render(filepath=str(args.out_coverage_exr.resolve()), scene=render)
        if not args.out_coverage_exr.is_file():
            raise RuntimeError("Cycles did not save the UV footprint")
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
                "reserved_rows": args.reserve_rows,
                "mesh_count": len(meshes), "baked_mesh_count": len(baked),
                "projected_meshes": {name: {"parts": value["parts"]}
                                     for name, value in projected.items()},
                "excluded_materials": sorted(excluded),
                "emitter_count": len(scene["emitters"]),
                "atlas_coverage_estimate": covered,
                "coverage_exr_sha256": sha256(args.out_coverage_exr)
                if args.out_coverage_exr else None,
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
