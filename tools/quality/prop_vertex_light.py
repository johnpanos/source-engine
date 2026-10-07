#!/usr/bin/env python3
"""Static props' baked per-vertex lighting from the map's Cycles bake.

    python3 tools/quality/prop_vertex_light.py points --bsp map.bsp \\
        --runtime run/runtime-p2 --model-tool build/mdl/mdl_mesh_export \\
        --out work/prop_vertices.npz
    (bake: pbrt_lightmap_bake.py --prop-vertices work/prop_vertices.npz
           --prop-vertex-light work/prop_vertex_light.npy)
    python3 tools/quality/prop_vertex_light.py write --bsp map.bsp \\
        --points work/prop_vertices.npz --light work/prop_vertex_light.npy \\
        --out relit.bsp --receipt relit.prop-vertex-light.json

Source 2 stores a lighting value for every vertex of a static prop (Valve
Developer Community, "Source 2 lighting": "Static props are lit statically: a
lighting value is stored for every vertex"); Source 1's vrad writes the same
data as "sp_<n>.vhv" / "sp_hdr_<n>.vhv" colour meshes in the pak lump
(utils/vrad/vradstaticprops.cpp, SerializeLighting), which the engine binds
to the prop (engine/l_studio.cpp) and the render core draws as
SurfaceVariant::staticVertexLight. A relit map's lightmaps come from Cycles
while its props kept vrad's light; this tool gives the props the bake's.

`points`: every static prop's sample points, in the engine's static prop
order (the sprp lump's): for each VTX strip group of every body part,
submodel and LOD (mdl::VertexColorGroup, `mdl_mesh_export --color-groups`),
each group vertex's world position and normal. A prop's vertices that share
a position and normal share one sample. Props flagged
STATIC_PROP_NO_PER_VERTEX_LIGHTING (0x40) get none, as in vrad.

The bake (pbrt_lightmap_bake.py --prop-vertices) gives each sample the
irradiance the lightmap atlas holds (Cycles DIFFUSE, direct and indirect,
albedo divided out): the same unit, so a prop beside a wall gets the wall's
light. That is the total light, as vrad's colour meshes are: the core skips
the baked lights' runtime diffuse on such a surface (surface_program.glsl).

`write`: a sample whose point is inside the world's solid (a solid leaf, or
no cluster) sees only the solid's inside, and takes the nearest open sample
of its prop; then one colour mesh per prop, vrad's layout (VHV_VERSION 2, the model's
checksum, VERTEX_COLOR, 4-byte vertices, a mesh header per strip group, the
vertices from offset 512, the file padded to 512), each vertex encoded as
the core decodes it, light = (2 c)^2.2 (surface_world_vertex.glsl): c =
0.5 light^(1/2.2), and a colour over 1 scaled down to 1 keeping its hue
(vrad's ColorClamp), stored b, g, r, 255. Both sp_<n>.vhv and sp_hdr_<n>.vhv
are written with the same data. Every other pak entry is kept byte for byte;
only the pak lump changes, appended at the end with its old range zeroed (as
leaf_ambient_from_prbv.py does), so every other lump keeps its offset.
"""

import argparse
import hashlib
import io
import json
import struct
import sys
import zipfile
from pathlib import Path

import numpy as np

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import legacy_bsp  # noqa: E402

SCHEMA = "prop-vertex-light/v1"
NO_PER_VERTEX_LIGHTING = 0x40
VHV_VERSION = 2
VERTEX_COLOR = 0x0004  # public/materialsystem/imaterial.h
VHV_ALIGN = 512
GAMMA = 2.2
# The sample's distance off the surface (Source units): clear of the prop's
# own triangles, close enough to keep its contact shadows.
SAMPLE_OFFSET = 0.5


def sha256(data):
    return hashlib.sha256(data).hexdigest()


def encode(light):
    """Linear lightmap-unit irradiance (N, 3) to the colour mesh's bytes (N, 4), b g r a."""
    light = np.maximum(np.asarray(light, dtype=np.float64), 0.0)
    colour = 0.5 * np.power(light, 1.0 / GAMMA)
    peak = colour.max(axis=1, keepdims=True)
    colour = np.where(peak > 1.0, colour / np.maximum(peak, 1e-30), colour)
    rgb = np.clip(np.rint(colour * 255.0), 0, 255).astype(np.uint8)
    out = np.full((len(rgb), 4), 255, dtype=np.uint8)
    out[:, 0], out[:, 1], out[:, 2] = rgb[:, 2], rgb[:, 1], rgb[:, 0]
    return out


def decode(bgra):
    """The core's decode of encode()'s bytes (the oracle's inverse)."""
    rgb = np.asarray(bgra, dtype=np.float64)[:, [2, 1, 0]] / 255.0
    return np.power(rgb * 2.0, GAMMA)


def vhv_file(checksum, groups, colours):
    """vrad's colour mesh: groups [(lod, count)], colours (sum(count), 4) b g r a."""
    header = struct.calcsize("<iIIIIi4I")
    mesh = struct.calcsize("<III4I")
    start = -(-(header + mesh * len(groups)) // VHV_ALIGN) * VHV_ALIGN
    total = sum(count for _lod, count in groups)
    if len(colours) != total:
        raise ValueError("colour count %d differs from the groups' %d" % (len(colours), total))
    end = -(-(start + 4 * total) // VHV_ALIGN) * VHV_ALIGN
    data = bytearray(end)
    struct.pack_into("<iIIIIi4I", data, 0, VHV_VERSION, checksum & 0xffffffff, VERTEX_COLOR, 4,
                     total, len(groups), 0, 0, 0, 0)
    offset = start
    for index, (lod, count) in enumerate(groups):
        struct.pack_into("<III4I", data, header + mesh * index, lod, count, offset, 0, 0, 0, 0)
        offset += 4 * count
    data[start:start + 4 * total] = np.ascontiguousarray(colours, dtype=np.uint8).tobytes()
    return bytes(data)


def read_vhv(data):
    """(checksum, [(lod, count)], colours) of a colour mesh (the oracle's reader)."""
    version, checksum, flags, size, total, meshes = struct.unpack_from("<iIIIIi", data, 0)
    if version != VHV_VERSION or size != 4 or flags != VERTEX_COLOR:
        raise ValueError("not a version 2 colour mesh")
    groups, colours = [], []
    header = struct.calcsize("<iIIIIi4I")
    for index in range(meshes):
        lod, count, offset = struct.unpack_from("<III", data, header + 28 * index)
        groups.append((lod, count))
        colours.append(np.frombuffer(data, dtype=np.uint8, count=4 * count,
                                     offset=offset).reshape(-1, 4))
    colours = np.concatenate(colours) if colours else np.zeros((0, 4), np.uint8)
    if len(colours) != total:
        raise ValueError("colour mesh vertex count disagrees with its meshes")
    return checksum, groups, colours


def placements_digest(bsp):
    """The static props as the engine numbers them (model, origin, angles,
    skin, flags): what the colour meshes are keyed to."""
    _version, _names, placements = bsp.static_props()
    keyed = [[prop["model"], [round(v, 4) for v in prop["origin"]],
              [round(v, 4) for v in prop["angles"]], prop["skin"], prop["flags"]]
             for prop in placements]
    return sha256(json.dumps(keyed).encode())


def cmd_points(args):
    import legacy_bsp_scene
    import source_content
    bsp = legacy_bsp.LegacyBsp.read(args.bsp)
    resolver = source_content.ContentResolver(str(args.runtime), ())
    if args.game_dir:
        resolver = legacy_bsp_scene.GameDirectory(args.game_dir, resolver)
    _version, _names, placements = bsp.static_props()
    models = {}
    props, positions, normals, sample_of = [], [], [], []
    for index, prop in enumerate(placements):
        if prop["flags"] & NO_PER_VERTEX_LIGHTING:
            continue
        if prop["model"] not in models:
            models[prop["model"]] = legacy_bsp_scene.studio_mesh(
                resolver, args.model_tool, prop["model"], extra=("--color-groups",))
        model = models[prop["model"]]
        rotation = legacy_bsp_scene.static_prop_matrix(prop["angles"])
        origin = np.asarray(prop["origin"], dtype=np.float64)
        groups, keys = [], {}
        first_vertex = len(sample_of)
        for group in model["groups"]:
            p = np.asarray(group["p"], dtype=np.float64).reshape(-1, 3)
            n = np.asarray(group["n"], dtype=np.float64).reshape(-1, 3)
            world = p @ rotation.T + origin
            normal = n @ rotation.T
            length = np.linalg.norm(normal, axis=1, keepdims=True)
            normal = np.where(length > 1e-6, normal / np.maximum(length, 1e-12), [0.0, 0.0, 1.0])
            for w, d in zip(world, normal):
                key = (tuple(np.round(w, 3)), tuple(np.round(d, 3)))
                if key not in keys:
                    keys[key] = len(positions)
                    positions.append(w)
                    normals.append(d)
                sample_of.append(keys[key])
            groups.append([int(group["lod"]), len(p)])
        props.append({"index": index, "model": prop["model"], "checksum": int(model["checksum"]),
                      "groups": groups, "first_vertex": first_vertex,
                      "vertices": len(sample_of) - first_vertex})
    args.out.parent.mkdir(parents=True, exist_ok=True)
    np.savez_compressed(args.out, positions=np.asarray(positions, np.float64).reshape(-1, 3),
                        normals=np.asarray(normals, np.float64).reshape(-1, 3),
                        sample_of=np.asarray(sample_of, np.int64),
                        layout=np.frombuffer(json.dumps(
                            {"schema": SCHEMA, "bsp_sha256": sha256(bsp.data),
                             "placements_sha256": placements_digest(bsp),
                             "props": props}).encode(), dtype=np.uint8))
    print(json.dumps({"props": len(props), "vertices": len(sample_of),
                      "samples": len(positions), "models": len(models)}))


def load_points(path):
    with np.load(path) as data:
        layout = json.loads(bytes(data["layout"]).decode())
        return layout, data["positions"], data["normals"], data["sample_of"]


def replace_pak(data, pak):
    """The BSP with its pak lump (40) replaced: appended, old range zeroed."""
    out = bytearray(data)
    offset, length = struct.unpack_from("<ii", out, 8 + 16 * legacy_bsp.LUMP_PAKFILE)
    out[offset:offset + length] = bytes(length)
    while len(out) % 4:
        out.append(0)
    start = len(out)
    out += pak
    struct.pack_into("<ii", out, 8 + 16 * legacy_bsp.LUMP_PAKFILE, start, len(pak))
    return bytes(out)


def is_colour_mesh(name):
    base = name.lower().rsplit("/", 1)[-1]
    return base.startswith("sp_") and base.endswith(".vhv")


def fill_enclosed(light, positions, samples, enclosed):
    """A prop's samples inside solid (`enclosed`: per sample, its point is in
    a solid leaf or outside the world; the bake there sees only the solid's
    inside) take their nearest open sample's light, within the prop. A
    sample in an open but unlit place keeps its darkness. Returns the light
    and the count filled; a prop with no open sample is unchanged."""
    unique = np.unique(samples)
    inside = unique[enclosed[unique]]
    open_ = unique[~enclosed[unique]]
    if not len(inside) or not len(open_):
        return light, 0
    filled = light.copy()
    for sample in inside:
        nearest = open_[np.argmin(np.sum((positions[open_] - positions[sample]) ** 2, axis=1))]
        filled[sample] = light[nearest]
    return filled, len(inside)


def enclosed_samples(bsp, positions, normals):
    """Per sample, whether its baked point (off the surface) is in a solid
    leaf of the world or in no cluster (outside it)."""
    contents, cluster, _mins, _maxs = bsp.leaves()
    points = positions + normals * SAMPLE_OFFSET
    out = np.zeros(len(points), dtype=bool)
    for index, point in enumerate(points):
        leaf = bsp.point_leaf(point)
        out[index] = bool(contents[leaf] & 1) or cluster[leaf] < 0
    return out


def with_legacy(bsp2_data, legacy):
    """A BSP2 carrying `legacy` as its legacy map, every other lump kept."""
    import bsp2_reader
    container = bsp2_reader.Bsp2File(bsp2_data, allow_unknown_required=True)
    carried = bsp2_reader.convert_legacy(legacy)
    converted = bsp2_reader.Bsp2File(carried, allow_unknown_required=True)

    def entries(data, parsed, keep):
        return [(e["fourcc"], e["version"], e["flags"], e["alignment"],
                 data[e["offset"]:e["offset"] + e["size"]]) for e in parsed.entries if keep(e)]

    def is_legacy(entry):
        return bsp2_reader.legacy_index(entry["fourcc"]) is not None or \
            entry["fourcc"] in (bsp2_reader.LHDR, bsp2_reader.LGAP)
    lumps = entries(carried, converted, is_legacy) + \
        entries(bsp2_data, container, lambda e: not is_legacy(e))
    return bsp2_reader.write_bsp2(container.revision, lumps)


def cmd_write(args):
    import bsp2_reader
    source = args.bsp.read_bytes()
    bsp2_data = source if source[:8] == bsp2_reader.MAGIC else None
    if bsp2_data is not None:
        source = bsp2_reader.Bsp2File(bsp2_data, allow_unknown_required=True).export_legacy()
    bsp = legacy_bsp.LegacyBsp(source)
    layout, positions, normals, sample_of = load_points(args.points)
    if layout["placements_sha256"] != placements_digest(bsp):
        raise ValueError("the points were made from a BSP with other static props")
    light = np.load(args.light)
    if light.shape != (len(positions), 3) or not np.all(np.isfinite(light)):
        raise ValueError("bake light must be finite (%d, 3)" % len(positions))
    files = {}
    inside = enclosed_samples(bsp, positions, normals)
    enclosed = 0
    for prop in layout["props"]:
        samples = sample_of[prop["first_vertex"]:prop["first_vertex"] + prop["vertices"]]
        light, filled = fill_enclosed(light, positions, samples, inside)
        enclosed += filled
        colours = encode(light[samples])
        groups = [tuple(group) for group in prop["groups"]]
        data = vhv_file(prop["checksum"], groups, colours)
        files["sp_%d.vhv" % prop["index"]] = data
        files["sp_hdr_%d.vhv" % prop["index"]] = data
    source = bsp.pakfile()
    buffer = io.BytesIO()
    kept = replaced = 0
    with zipfile.ZipFile(buffer, "w", zipfile.ZIP_STORED) as target:
        for info in source.infolist():
            if is_colour_mesh(info.filename):
                replaced += 1
                continue
            target.writestr(info, source.read(info.filename))
            kept += 1
        for name in sorted(files):
            info = zipfile.ZipInfo(name, date_time=(1980, 1, 1, 0, 0, 0))
            info.compress_type = zipfile.ZIP_STORED
            target.writestr(info, files[name])
    out = replace_pak(bsp.data, buffer.getvalue())
    if bsp2_data is not None:
        # A BSP2 (a published map): its legacy map replaced, every other lump
        # (world mesh, lightmaps, probes, masks) byte for byte.
        out = with_legacy(bsp2_data, out)
    args.out.write_bytes(out)
    luminance = light @ np.array([0.2126, 0.7152, 0.0722])
    receipt = {"schema": SCHEMA, "status": "pass", "bsp_sha256": sha256(bsp.data),
               "out_sha256": sha256(out), "props": len(layout["props"]),
               "colour_meshes": len(files), "pak_entries_kept": kept,
               "vrad_colour_meshes_replaced": replaced, "samples": len(positions),
               "enclosed_samples_filled": enclosed,
               "luminance": {"mean": float(luminance.mean()) if len(luminance) else 0.0,
                             "p99": float(np.percentile(luminance, 99)) if len(luminance)
                             else 0.0},
               "clamped_fraction": float(np.mean(
                   (0.5 * np.power(np.maximum(light, 0), 1 / GAMMA)).max(axis=1) > 1.0))
               if len(light) else 0.0}
    if args.receipt:
        args.receipt.write_text(json.dumps(receipt, indent=2, sort_keys=True) + "\n")
    print(json.dumps(receipt, sort_keys=True))


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = parser.add_subparsers(dest="command", required=True)
    points = sub.add_parser("points")
    points.add_argument("--bsp", type=Path, required=True)
    points.add_argument("--runtime", type=Path, required=True)
    points.add_argument("--game-dir", type=Path)
    points.add_argument("--model-tool", type=Path, required=True)
    points.add_argument("--out", type=Path, required=True)
    write = sub.add_parser("write")
    write.add_argument("--bsp", type=Path, required=True, help="a legacy BSP or a BSP2")
    write.add_argument("--points", type=Path, required=True)
    write.add_argument("--light", type=Path, required=True)
    write.add_argument("--out", type=Path, required=True)
    write.add_argument("--receipt", type=Path)
    args = parser.parse_args()
    {"points": cmd_points, "write": cmd_write}[args.command](args)


if __name__ == "__main__":
    main()
