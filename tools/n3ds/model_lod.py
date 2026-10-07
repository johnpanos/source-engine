#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Trim a Source model to one level of detail: .dx90.vtx, .vvd and .mdl together.

`trim_model(mdl, vvd, vtx, lod)` keeps LOD `lod` (0 = the most detailed) as the
model's only LOD. Formats, as the engine reads them:

  VTX (public/optimize.h, OptimizedModel, pack 1; every offset is relative to the
  start of the structure that holds it, except strip/stripgroup vertex and index
  offsets, which are element indices): FileHeader -> bodyparts -> models ->
  ModelLODHeader per LOD -> meshes -> stripgroups -> (vertices, indices, strips ->
  bone state changes). Trimming rebuilds the file from the chosen LOD alone.

  VVD (public/studio.h, vertexFileHeader_t): the vertex pool is sorted so that
  root LOD N uses the first numLODVertexes[N] vertices; the fixup table
  (vertexFileFixup_t: lod, sourceVertexID, numVertexes) re-establishes mesh order,
  skipping fixups whose lod is below the root LOD (Studio_LoadVertexes). A model
  trimmed to LOD N keeps that prefix of vertices and of tangents, the fixups with
  lod >= N, numLODs = 1 and every numLODVertexes[] = numLODVertexes[N].

  MDL (studiohdr_t): each mstudiomesh_t carries numLODVertexes[8], which
  Studio_SetRootLOD turns into mesh/model vertex counts and offsets at load. They
  are rewritten to the kept LOD's counts, and numAllowedRootLODs = 1 so no later
  root LOD request can reach a LOD that is gone.

Vertex bytes of the kept LOD are copied unchanged. LOD 0 keeps every vertex, so a
LOD 0 trim shrinks the VTX (and drops the fixup table's unused runs) but not the
VVD's vertex block; a distant LOD as the sole LOD shrinks both.
"""

import struct

MODEL_VERTEX_FILE_ID = 0x56534449      # "IDSV"
MODEL_VERTEX_FILE_VERSION = 4
MAX_NUM_LODS = 8
VVD_HEADER_BYTES = 64
VERTEX_BYTES = 48                      # mstudiovertex_t
TANGENT_BYTES = 16                     # Vector4D

# studiohdr_t byte offsets (32-bit layout, as stored in files).
MDL_NUMBODYPARTS = 232
MDL_ROOT_LOD = 377
MDL_NUM_ALLOWED_ROOT_LODS = 378
# mstudiomodel_t / mstudiomesh_t layouts as stored in files.
MODEL_BYTES, MESH_BYTES = 148, 116
MODEL_NUMMESHES, MODEL_MESHINDEX = 72, 76
MODEL_NUMVERTICES, MODEL_VERTEXINDEX, MODEL_TANGENTSINDEX = 80, 84, 88
MESH_NUMVERTICES, MESH_VERTEXOFFSET, MESH_NUMLODVERTEXES = 8, 12, 52

MESH_IS_MDL49 = 0x80
STRIPGROUP_IS_MDL49 = 0x80


class LodError(ValueError):
    """The files are not in a shape this trim can rewrite safely."""


def _unpack(fmt, data, offset):
    try:
        return struct.unpack_from(fmt, data, offset)
    except struct.error as error:
        raise LodError("truncated or out-of-range structure at %d" % offset) from error


def _bytes(data, offset, length):
    if offset < 0 or length < 0 or offset + length > len(data):
        raise LodError("array at %d (+%d) is outside the file" % (offset, length))
    return bytes(data[offset:offset + length])


def _cstring(data, offset):
    end = data.find(b"\0", offset)
    if offset < 0 or end < 0:
        raise LodError("unterminated string at %d" % offset)
    return bytes(data[offset:end])


# --------------------------------------------------------------------------
# VTX
# --------------------------------------------------------------------------

def mdl_version(mdl):
    return _unpack("<i", mdl, 4)[0]


def parse_vtx(data):
    """The VTX as plain dictionaries and byte strings (see module docstring).

    Version 49 content stores stripgroups as 33-byte and strips as 35-byte records
    (OptimizedModel::StripGroupHeader_v49_t); older content uses 25 and 27. The
    file does not say which: shipped Portal 2 MDL 49 models contain both (CMDLCache
    sets MESH_IS_MDL49 from the MDL version alone). The layout is therefore found
    by parsing both ways and keeping the one whose strips are consistent with
    their stripgroup (ranges inside the group, indices adding up)."""
    errors = []
    for wide in (True, False):
        try:
            return _parse_vtx(data, wide)
        except LodError as error:
            errors.append(str(error))
    raise LodError("no consistent stripgroup layout (%s)" % "; ".join(errors))


def _parse_vtx(data, v49):
    (version, cache, max_strip, max_tri, max_vert, checksum, num_lods, repl_offset,
     num_bodyparts, bodypart_offset) = _unpack("<iiHHiiiiii", data, 0)
    if version != 7:
        raise LodError("VTX version %d is not handled" % version)
    vtx = {"v49": v49, "version": version, "cache": cache, "max_strip": max_strip, "max_tri": max_tri,
           "max_vert": max_vert, "checksum": checksum, "num_lods": num_lods,
           "replacements": [], "bodyparts": []}
    for lod in range(num_lods):
        base = repl_offset + 8 * lod
        count, offset = _unpack("<ii", data, base)
        entries = []
        for index in range(count):
            entry = base + offset + 6 * index
            material, name_offset = _unpack("<hi", data, entry)
            entries.append((material, _cstring(data, entry + name_offset)))
        vtx["replacements"].append(entries)
    for body in range(num_bodyparts):
        base = bodypart_offset + 8 * body
        count, offset = _unpack("<ii", data, base)
        models = []
        for model in range(count):
            model_at = base + offset + 8 * model
            lod_count, lod_offset = _unpack("<ii", data, model_at)
            if lod_count != num_lods:
                raise LodError("model LOD count %d differs from the file's %d" % (
                    lod_count, num_lods))
            lods = []
            for lod in range(lod_count):
                lod_at = model_at + lod_offset + 12 * lod
                mesh_count, mesh_offset, switch = _unpack("<iif", data, lod_at)
                meshes = []
                for mesh in range(mesh_count):
                    mesh_at = lod_at + mesh_offset + 9 * mesh
                    group_count, group_offset, mesh_flags = _unpack("<iiB", data, mesh_at)
                    group_stride = 33 if v49 else 25
                    groups = []
                    for group in range(group_count):
                        group_at = mesh_at + group_offset + group_stride * group
                        groups.append(_parse_group(data, group_at, group_stride))
                    meshes.append({"flags": mesh_flags, "groups": groups})
                lods.append({"switch": switch, "meshes": meshes})
            models.append(lods)
        vtx["bodyparts"].append(models)
    return vtx


def _parse_group(data, at, stride):
    (vert_count, vert_offset, index_count, index_offset, strip_count, strip_offset,
     flags) = _unpack("<6iB", data, at)
    group = {"flags": flags,
             "vertices": _bytes(data, at + vert_offset, vert_count * 9),
             "indices": _bytes(data, at + index_offset, index_count * 2),
             "vert_count": vert_count, "index_count": index_count, "strips": []}
    strip_stride = 35 if stride == 33 else 27
    for strip in range(strip_count):
        strip_at = at + strip_offset + strip_stride * strip
        (indices, index_start, verts, vert_start, bones, strip_flags, state_count,
         state_offset) = _unpack("<iiiihBii", data, strip_at)
        if strip_stride == 35 and _unpack("<i", data, strip_at + 27)[0]:
            raise LodError("strip topology indices are not modelled")
        if (indices < 0 or verts < 0 or index_start < 0 or vert_start < 0 or
                index_start + indices > index_count or vert_start + verts > vert_count or
                bones < 0 or state_count < 0 or state_count > 4096):
            raise LodError("strip leaves its stripgroup")
        group["strips"].append({
            "indices": indices, "index_start": index_start, "verts": verts,
            "vert_start": vert_start, "bones": bones, "flags": strip_flags,
            "states": _bytes(data, strip_at + state_offset, state_count * 8),
            "state_count": state_count})
    if sum(strip["indices"] for strip in group["strips"]) != index_count:
        raise LodError("strip indices do not add up to the stripgroup's")
    if stride == 33 and _unpack("<i", data, at + 25)[0]:
        raise LodError("stripgroup topology indices are not modelled")
    group["strip_stride"] = strip_stride
    group["stride"] = stride
    return group


def build_vtx(vtx, lod):
    """A VTX holding LOD `lod` only. All offsets are recomputed."""
    out = bytearray(36)

    def alloc(size):
        offset = len(out)
        out.extend(bytes(size))
        return offset

    repl_at = alloc(8)
    entries = vtx["replacements"][lod] if lod < len(vtx["replacements"]) else []
    entries_at = alloc(6 * len(entries))
    for index, (material, name) in enumerate(entries):
        string_at = len(out)
        out.extend(name + b"\0")
        struct.pack_into("<hi", out, entries_at + 6 * index, material,
                         string_at - (entries_at + 6 * index))
    struct.pack_into("<ii", out, repl_at, len(entries), entries_at - repl_at)

    bodyparts_at = alloc(8 * len(vtx["bodyparts"]))
    for body, models in enumerate(vtx["bodyparts"]):
        body_at = bodyparts_at + 8 * body
        models_at = alloc(8 * len(models))
        struct.pack_into("<ii", out, body_at, len(models), models_at - body_at)
        for model, lods in enumerate(models):
            model_at = models_at + 8 * model
            kept = lods[lod]
            lod_at = alloc(12)
            struct.pack_into("<ii", out, model_at, 1, lod_at - model_at)
            meshes_at = alloc(9 * len(kept["meshes"]))
            struct.pack_into("<iif", out, lod_at, len(kept["meshes"]), meshes_at - lod_at,
                             kept["switch"])
            for mesh, mesh_data in enumerate(kept["meshes"]):
                mesh_at = meshes_at + 9 * mesh
                groups = mesh_data["groups"]
                stride = 33 if vtx["v49"] else 25
                groups_at = alloc(stride * len(groups))
                struct.pack_into("<iiB", out, mesh_at, len(groups), groups_at - mesh_at,
                                 mesh_data["flags"])
                for group, group_data in enumerate(groups):
                    _build_group(out, alloc, groups_at + stride * group, group_data)
    struct.pack_into("<iiHHiiiiii", out, 0, vtx["version"], vtx["cache"], vtx["max_strip"],
                     vtx["max_tri"], vtx["max_vert"], vtx["checksum"], 1, repl_at,
                     len(vtx["bodyparts"]), bodyparts_at)
    return bytes(out)


def _build_group(out, alloc, at, group):
    vertices_at = alloc(len(group["vertices"]))
    out[vertices_at:vertices_at + len(group["vertices"])] = group["vertices"]
    indices_at = alloc(len(group["indices"]))
    out[indices_at:indices_at + len(group["indices"])] = group["indices"]
    stride = group["strip_stride"]
    strips_at = alloc(stride * len(group["strips"]))
    for index, strip in enumerate(group["strips"]):
        strip_at = strips_at + stride * index
        states_at = alloc(len(strip["states"]))
        out[states_at:states_at + len(strip["states"])] = strip["states"]
        struct.pack_into("<iiiihBii", out, strip_at, strip["indices"], strip["index_start"],
                         strip["verts"], strip["vert_start"], strip["bones"], strip["flags"],
                         strip["state_count"], states_at - strip_at)
    struct.pack_into("<6iB", out, at, group["vert_count"], vertices_at - at,
                     group["index_count"], indices_at - at, len(group["strips"]),
                     strips_at - at, group["flags"])
    if group["stride"] == 33:
        struct.pack_into("<2i", out, at + 25, 0, 0)


# --------------------------------------------------------------------------
# VVD
# --------------------------------------------------------------------------

def trim_vvd(data, lod):
    """(new VVD bytes, vertex count of the kept LOD, lod actually kept)."""
    (ident, version, checksum, num_lods) = _unpack("<4i", data, 0)
    if ident != MODEL_VERTEX_FILE_ID or version != MODEL_VERTEX_FILE_VERSION:
        raise LodError("not a version 4 VVD")
    counts = list(_unpack("<8i", data, 16))
    fixup_count, fixup_start, vertex_start, tangent_start = _unpack("<4i", data, 48)
    if not 1 <= num_lods <= MAX_NUM_LODS:
        raise LodError("bad VVD LOD count %d" % num_lods)
    lod = min(lod, num_lods - 1)
    keep = counts[lod]
    if keep <= 0 or keep > counts[0] or vertex_start < VVD_HEADER_BYTES:
        raise LodError("bad VVD vertex counts")
    fixups = [_unpack("<3i", data, fixup_start + 12 * index) for index in range(fixup_count)]
    kept = [(0, source, count) for (fixup_lod, source, count) in fixups if fixup_lod >= lod]
    for _lod, source, count in kept:
        if source < 0 or count < 0 or source + count > keep:
            raise LodError("a fixup run leaves the kept vertex prefix")
    if fixups and sum(count for _l, _s, count in kept) != keep:
        raise LodError("the kept fixups do not rebuild %d vertices" % keep)
    vertices = _bytes(data, vertex_start, keep * VERTEX_BYTES)
    tangents = _bytes(data, tangent_start, keep * TANGENT_BYTES) if tangent_start else b""
    table = b"".join(struct.pack("<3i", *entry) for entry in kept)
    vertex_at = (VVD_HEADER_BYTES + len(table) + 15) & ~15
    header = struct.pack("<4i8i4i", ident, version, checksum, 1, *([keep] * MAX_NUM_LODS),
                         len(kept), VVD_HEADER_BYTES if kept else 0, vertex_at,
                         vertex_at + keep * VERTEX_BYTES if tangents else 0)
    body = header + table
    body += bytes(vertex_at - len(body)) + vertices + tangents
    return body, keep, lod


# --------------------------------------------------------------------------
# MDL
# --------------------------------------------------------------------------

def patch_mdl(data, lod, vertex_counts):
    """The MDL with its per-mesh LOD vertex counts reduced to the kept LOD.

    vertex_counts is unused for validation beyond the mesh walk; the kept counts
    come from each mesh's own numLODVertexes[lod], exactly as Studio_SetRootLOD."""
    out = bytearray(data)
    if bytes(out[:4]) != b"IDST":
        raise LodError("not a studio model")
    body_count, body_offset = _unpack("<2i", out, MDL_NUMBODYPARTS)
    vertex_index = tangent_index = 0
    for body in range(body_count):
        body_at = body_offset + 16 * body
        model_count = _unpack("<i", out, body_at + 4)[0]
        model_offset = _unpack("<i", out, body_at + 12)[0]  # sznameindex, nummodels, base, modelindex
        for model in range(model_count):
            model_at = body_at + model_offset + MODEL_BYTES * model
            mesh_count, mesh_offset = _unpack("<2i", out, model_at + MODEL_NUMMESHES)
            total = 0
            for mesh in range(mesh_count):
                mesh_at = model_at + mesh_offset + MESH_BYTES * mesh
                per_lod = _unpack("<8i", out, mesh_at + MESH_NUMLODVERTEXES)
                kept = per_lod[lod]
                struct.pack_into("<8i", out, mesh_at + MESH_NUMLODVERTEXES, *([kept] * 8))
                struct.pack_into("<2i", out, mesh_at + MESH_NUMVERTICES, kept, total)
                total += kept
            struct.pack_into("<3i", out, model_at + MODEL_NUMVERTICES, total, vertex_index,
                             tangent_index)
            vertex_index += total * VERTEX_BYTES
            tangent_index += total * TANGENT_BYTES
    if vertex_counts is not None and vertex_index != vertex_counts * VERTEX_BYTES:
        raise LodError("the MDL meshes add up to %d vertices, the VVD keeps %d" % (
            vertex_index // VERTEX_BYTES, vertex_counts))
    out[MDL_ROOT_LOD] = 0
    out[MDL_NUM_ALLOWED_ROOT_LODS] = 1
    return bytes(out)


def trim_model(mdl, vvd, vtx, lod=0):
    """(mdl, vvd, vtx) trimmed to one LOD; raises LodError when it cannot be done safely."""
    parsed = parse_vtx(vtx)
    lod = min(lod, parsed["num_lods"] - 1)
    new_vvd, keep, vvd_lod = trim_vvd(vvd, lod)
    if vvd_lod != lod:
        raise LodError("the VVD has fewer LODs (%d) than the VTX" % (vvd_lod + 1))
    new_mdl = patch_mdl(mdl, lod, keep)
    return new_mdl, new_vvd, build_vtx(parsed, lod)
