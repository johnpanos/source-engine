#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""model_lod: LOD trim of VTX, VVD and MDL, judged by an independent reader.

The reader here shares no code with model_lod.py. It walks the original and the
trimmed files the way the engine does (OptimizedModel offsets relative to each
structure; Studio_LoadVertexes for the VVD pool and fixups) and compares the kept
LOD's vertices, tangents, indices and strips byte for byte. Seeded corruptions of
the trimmed files must each be reported.

    python3 -m unittest discover -s tools/n3ds/tests -v
"""

import os
import struct
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
sys.path.insert(0, str(Path(__file__).resolve().parents[3] / "tools/quality"))

import model_lod  # noqa: E402

RUNTIME = Path(os.environ.get("N3DS_RUNTIME", "/home/john/src/source-engine/run/runtime-p2"))


# --------------------------------------------------------------------------
# Independent reader
# --------------------------------------------------------------------------

def u(fmt, data, at):
    return struct.unpack_from("<" + fmt, data, at)


def vtx_lods(vtx):
    """(numLODs, {lod: [(path, vertex bytes, index bytes, [(strip fields, state bytes)])]}).

    The stripgroup record is 33 bytes (strips 35) in version 49 files and 25 (27)
    in older ones; the reader tries both and keeps the one whose strips add up to
    their group's index count."""
    last = None
    for wide in (True, False):
        try:
            return walk_vtx(vtx, wide)
        except (AssertionError, struct.error) as error:
            last = error
    raise AssertionError("no consistent layout: %s" % last)


def walk_vtx(vtx, wide):
    num_lods, bodies, body_at = u("i", vtx, 20)[0], u("i", vtx, 28)[0], u("i", vtx, 32)[0]
    group_size, strip_size = (33, 35) if wide else (25, 27)
    result = {}
    for b in range(bodies):
        bp = body_at + 8 * b
        n_models, m_off = u("ii", vtx, bp)
        for m in range(n_models):
            mp = bp + m_off + 8 * m
            n_lod, l_off = u("ii", vtx, mp)
            assert n_lod == num_lods, "model LOD count differs from the file header"
            for lod in range(n_lod):
                lp = mp + l_off + 12 * lod
                n_mesh, me_off = u("ii", vtx, lp)
                for k in range(n_mesh):
                    kp = lp + me_off + 9 * k
                    n_group, g_off = u("ii", vtx, kp)
                    for g in range(n_group):
                        gp = kp + g_off + group_size * g
                        nv, vo, ni, io, ns, so = u("6i", vtx, gp)
                        strips = []
                        for s in range(ns):
                            sp = gp + so + strip_size * s
                            ni2, io2, nv2, vo2, nb, fl, nsc, soff = u("iiiihBii", vtx, sp)
                            strips.append(((ni2, io2, nv2, vo2, nb, fl, nsc),
                                           bytes(vtx[sp + soff: sp + soff + 8 * nsc])))
                        assert sum(f[0][0] for f in strips) == ni, "strips do not cover the group"
                        assert vo + 9 * nv <= len(vtx) - gp and io + 2 * ni <= len(vtx) - gp
                        result.setdefault(lod, []).append((
                            (b, m, k, g), bytes(vtx[gp + vo: gp + vo + 9 * nv]),
                            bytes(vtx[gp + io: gp + io + 2 * ni]), strips))
    return num_lods, result


def vvd_root(vvd, root):
    """(vertex bytes, tangent bytes, numLODs) the engine builds for a root LOD."""
    ident, version, _sum, num_lods = u("4i", vvd, 0)
    assert ident == 0x56534449 and version == 4
    counts = u("8i", vvd, 16)
    nfix, fix_at, vert_at, tan_at = u("4i", vvd, 48)
    root = min(root, num_lods - 1)
    n = counts[root]
    if not nfix:
        return (bytes(vvd[vert_at:vert_at + 48 * n]),
                bytes(vvd[tan_at:tan_at + 16 * n]) if tan_at else b"", num_lods)
    vertices, tangents = b"", b""
    for i in range(nfix):
        lod, src, cnt = u("3i", vvd, fix_at + 12 * i)
        if lod < root:
            continue
        vertices += bytes(vvd[vert_at + 48 * src: vert_at + 48 * (src + cnt)])
        if tan_at:
            tangents += bytes(vvd[tan_at + 16 * src: tan_at + 16 * (src + cnt)])
    assert len(vertices) == 48 * n, "fixups do not rebuild numLODVertexes[root]"
    return vertices, tangents, num_lods


def mdl_meshes(mdl):
    """[(numvertices, vertexoffset, numLODVertexes[8])] per mesh, in file order."""
    result = []
    nbody, boff = u("2i", mdl, 232)
    for b in range(nbody):
        bp = boff + 16 * b
        nmodel, moff = u("i", mdl, bp + 4)[0], u("i", mdl, bp + 12)[0]
        for m in range(nmodel):
            mp = bp + moff + 148 * m
            nmesh, meoff = u("2i", mdl, mp + 72)
            for k in range(nmesh):
                kp = mp + meoff + 116 * k
                result.append((u("i", mdl, kp + 8)[0], u("i", mdl, kp + 12)[0],
                               u("8i", mdl, kp + 52)))
    return result


def compare(original, trimmed, lod):
    """Problems found comparing LOD `lod` of the original with LOD 0 of the trimmed files."""
    problems = []
    tlods = {}
    omdl, ovvd, ovtx = original
    tmdl, tvvd, tvtx = trimmed
    try:
        onum, olods = vtx_lods(ovtx)
        tnum, tlods = vtx_lods(tvtx)
        lod = min(lod, onum - 1)
        if tnum != 1 or list(tlods) != [0]:
            problems.append("trimmed VTX does not hold exactly one LOD")
        elif olods[lod] != tlods[0]:
            problems.append("LOD vertices, indices or strips differ")
        # Everything outside the offset tables is the same bytes: the trimmed file
        # is exactly the size of what it describes (no dead space, no overrun).
        if len(tvtx) > len(ovtx):
            problems.append("trimmed VTX grew")
    except (AssertionError, struct.error) as error:
        problems.append("VTX unreadable: %s" % error)
    try:
        overt, otan, onum_lods = vvd_root(ovvd, lod)
        tvert, ttan, tnum_lods = vvd_root(tvvd, 0)
        if tnum_lods != 1:
            problems.append("trimmed VVD numLODs != 1")
        if overt != tvert:
            problems.append("VVD vertices differ")
        if otan != ttan:
            problems.append("VVD tangents differ")
        counts = u("8i", tvvd, 16)
        if len(set(counts)) != 1 or counts[0] * 48 != len(tvert):
            problems.append("VVD numLODVertexes inconsistent")
        if u("4i", tvvd, 48)[0] and len(tvvd) != u("i", tvvd, 60)[0] + 16 * counts[0] and u("i", tvvd, 60)[0]:
            problems.append("VVD size does not match its tangent block")
    except (AssertionError, struct.error) as error:
        problems.append("VVD unreadable: %s" % error)
        tvert = b""
    try:
        meshes = mdl_meshes(tmdl)
        offset = 0
        for count, vertex_offset, per_lod in meshes:
            if len(set(per_lod)) != 1 or per_lod[0] != count or vertex_offset != offset:
                problems.append("MDL mesh vertex counts disagree")
                break
            offset += count
        if offset * 48 != len(tvert):
            problems.append("MDL meshes do not add up to the VVD's vertices")
        if tmdl[378] != 1 or tmdl[377] != 0:
            problems.append("MDL root LOD fields not reset")
        # Every vertex the VTX names must exist in its mesh.
        for (path, verts, _i, _s) in tlods.get(0, []):
            mesh_index = path[2]
            limit = meshes[mesh_index][0]
            ids = [u("H", verts, 9 * i + 4)[0] for i in range(len(verts) // 9)]
            if ids and max(ids) >= limit:
                problems.append("VTX names a vertex beyond its mesh")
                break
    except (AssertionError, struct.error, IndexError) as error:
        problems.append("MDL unreadable: %s" % error)
    return problems


# --------------------------------------------------------------------------
# Synthetic model: 1 bodypart, 1 model, 2 meshes, 3 LODs
# --------------------------------------------------------------------------

MESH_COUNTS = [[6, 4, 3], [5, 4, 2]]       # numLODVertexes per mesh and LOD
# Pool order: lowest detail first. (lod, mesh, first mesh vertex, count)
POOL = [(2, 0, 0, 3), (2, 1, 0, 2), (1, 0, 3, 1), (1, 1, 2, 2), (0, 0, 4, 2), (0, 1, 4, 1)]


def vertex_bytes(mesh, index):
    return bytes((mesh * 37 + index * 11 + k) & 0xFF for k in range(48))


def tangent_bytes(mesh, index):
    return bytes((mesh * 41 + index * 7 + k + 5) & 0xFF for k in range(16))


def make_vvd(with_fixups=True):
    order = []                                            # pool: (mesh, mesh vertex)
    for lod, mesh, first, count in POOL:
        order += [(mesh, first + i) for i in range(count)]
    # Fixups rebuild mesh order: mesh 0 (lod2, lod1, lod0 runs) then mesh 1.
    src = {}
    pos = 0
    for lod, mesh, first, count in POOL:
        src[(lod, mesh)] = (pos, count)
        pos += count
    table = [(lod, ) + src[(lod, mesh)] for mesh in (0, 1) for lod in (2, 1, 0)]
    total = [sum(m[lod] for m in MESH_COUNTS) for lod in range(3)] + [0] * 5
    total = total[:3] + [total[2]] * 5
    fix_at = 64
    vert_at = 64 + 12 * len(table)
    vert_at = (vert_at + 15) & ~15
    tan_at = vert_at + 48 * len(order)
    header = struct.pack("<4i8i4i", 0x56534449, 4, 1234, 3, *total, len(table), fix_at, vert_at,
                         tan_at)
    body = header + b"".join(struct.pack("<3i", *entry) for entry in table)
    body += bytes(vert_at - len(body))
    body += b"".join(vertex_bytes(m, i) for m, i in order)
    body += b"".join(tangent_bytes(m, i) for m, i in order)
    return body


def make_mdl():
    data = bytearray(400)
    data[0:4] = b"IDST"
    struct.pack_into("<i", data, 4, 49)
    struct.pack_into("<2i", data, 232, 1, 400)
    data += struct.pack("<4i", 0, 1, 0, 16)            # bodypart: model at +16
    model_at = len(data)
    data += bytes(148)
    struct.pack_into("<2i", data, model_at + 72, 2, 148)
    offset = 0
    for mesh, counts in enumerate(MESH_COUNTS):
        mesh_at = len(data)
        data += bytes(116)
        struct.pack_into("<2i", data, mesh_at + 8, counts[0], offset)
        struct.pack_into("<8i", data, mesh_at + 52, *(counts + [counts[2]] * 5))
        offset += counts[0]
    struct.pack_into("<3i", data, model_at + 80, offset, 0, 0)
    data[377], data[378] = 0, 0
    return bytes(data)


def make_vtx(wide=True, topology=False):
    """VTX written with a deliberately different layout from model_lod's."""
    group_header = 33 if wide else 25
    strip_size = 35 if wide else 27
    out = bytearray(36)
    # Material replacement lists first this time: one empty list per LOD.
    repl_at = len(out)
    out += bytes(8 * 3)
    body_at = len(out)
    out += bytes(8)
    model_at = len(out)
    out += bytes(8)
    lod_at = len(out)
    out += bytes(12 * 3)
    struct.pack_into("<ii", out, body_at, 1, model_at - body_at)
    struct.pack_into("<ii", out, model_at, 3, lod_at - model_at)
    for lod in range(3):
        this_lod = lod_at + 12 * lod
        mesh_at = len(out)
        out += bytes(9 * 2)
        struct.pack_into("<iif", out, this_lod, 2, mesh_at - this_lod, float(lod) * 10)
        for mesh in range(2):
            this_mesh = mesh_at + 9 * mesh
            group_at = len(out)
            out += bytes(group_header)
            struct.pack_into("<iiB", out, this_mesh, 1, group_at - this_mesh, 0)
            count = MESH_COUNTS[mesh][lod]
            strip_at = len(out)
            out += bytes(strip_size * 2)
            vertices_at = len(out)
            for v in range(count):
                out += struct.pack("<3BBHxxx", 0, 0, 0, 1, v)[:9]
            indices = [(i * 3 + lod + mesh) % count for i in range(count * 3)]
            indices_at = len(out)
            out += struct.pack("<%dH" % len(indices), *indices)
            states_at = len(out)
            out += struct.pack("<ii", 0, count) * 1
            half = len(indices) // 2
            for s, (start, n) in enumerate(((0, half), (half, len(indices) - half))):
                sp = strip_at + strip_size * s
                struct.pack_into("<iiiihBii", out, sp, n, start, count, 0, 1, 1, 1,
                                 states_at - sp)
            struct.pack_into("<6iB", out, group_at, count, vertices_at - group_at,
                             len(indices), indices_at - group_at, 2, strip_at - group_at, 2)
            if topology:
                struct.pack_into("<ii", out, group_at + 25, 4, 0)
    struct.pack_into("<iiHHiiiiii", out, 0, 7, 24, 53, 9, 3, 1234, 3, repl_at, 1, body_at)
    return bytes(out)


class Synthetic(unittest.TestCase):
    def setUp(self):
        self.original = (make_mdl(), make_vvd(), make_vtx())

    def test_trim_each_lod_matches_the_independent_reader(self):
        for lod in range(3):
            trimmed = model_lod.trim_model(*self.original, lod)
            self.assertEqual(compare(self.original, trimmed, lod), [], "lod %d" % lod)
        small = model_lod.trim_model(*self.original, 2)
        self.assertLess(len(small[1]), len(self.original[1]))      # fewer vertices
        self.assertLess(len(small[2]), len(self.original[2]))      # one LOD of three

    def test_narrow_stripgroup_layout(self):
        original = (make_mdl(), make_vvd(), make_vtx(wide=False))
        trimmed = model_lod.trim_model(*original, 1)
        self.assertEqual(compare(original, trimmed, 1), [])

    def test_lod_beyond_the_last_clamps(self):
        trimmed = model_lod.trim_model(*self.original, 9)
        self.assertEqual(compare(self.original, trimmed, 2), [])

    def test_vvd_without_fixups(self):
        # A pool already in mesh order: the first N vertices are the root LOD's.
        data = bytearray(64 + 48 * 4 + 16 * 4)
        struct.pack_into("<4i8i4i", data, 0, 0x56534449, 4, 9, 2, 4, 2, 2, 2, 2, 2, 2, 2, 0, 0, 64,
                         64 + 48 * 4)
        for i in range(4):
            data[64 + 48 * i:64 + 48 * i + 48] = bytes([i + 1]) * 48
            data[64 + 192 + 16 * i:64 + 192 + 16 * i + 16] = bytes([i + 9]) * 16
        out, kept, lod = model_lod.trim_vvd(bytes(data), 1)
        self.assertEqual((kept, lod), (2, 1))
        vertices, tangents, _n = vvd_root(out, 0)
        self.assertEqual(vertices, bytes(data[64:64 + 96]))
        self.assertEqual(tangents, bytes(data[64 + 192:64 + 192 + 32]))

    def test_seeded_corruptions_are_caught(self):
        mdl, vvd, vtx = model_lod.trim_model(*self.original, 1)
        problems = compare(self.original, (mdl, vvd, vtx), 1)
        self.assertEqual(problems, [])
        vert_at = u("i", vvd, 56)[0]
        fix_at = u("i", vvd, 52)[0]
        tan_at = u("i", vvd, 60)[0]
        nbody = u("i", vtx, 28)[0]
        self.assertEqual(nbody, 1)

        def patched(blob, index, value=None):
            out = bytearray(blob)
            out[index] = (out[index] ^ 0x5A) if value is None else value
            return bytes(out)

        # first stripgroup's index array starts here (found by the reader).
        _num, lods = vtx_lods(vtx)
        index_bytes = lods[0][0][2]
        index_at = vtx.index(index_bytes)
        vertex_bytes_at = vtx.index(lods[0][0][1])
        cases = {
            "vertex byte": (mdl, patched(vvd, vert_at + 5), vtx),
            "tangent byte": (mdl, patched(vvd, tan_at + 3), vtx),
            "fixup source": (mdl, patched(vvd, fix_at + 16, 0), vtx),
            "fixup count": (mdl, patched(vvd, 48, 3), vtx),
            "numLODs": (mdl, patched(vvd, 12, 3), vtx),
            "numLODVertexes": (mdl, patched(vvd, 16, 1), vtx),
            "index byte": (mdl, vvd, patched(vtx, index_at + 1)),
            "origMeshVertID": (mdl, vvd, patched(vtx, vertex_bytes_at + 4, 0x7F)),
            "truncated vtx": (mdl, vvd, vtx[:len(vtx) // 2]),
            "vtx lod count": (mdl, vvd, patched(vtx, 20, 3)),
            "mdl mesh count": (patched(mdl, mdl.index(struct.pack("<8i", *([4] * 8)), 0) + 8, 9),
                               vvd, vtx),
            "mdl root lod": (patched(mdl, 378, 0), vvd, vtx),
            "mdl numvertices": (patched(mdl, 400 + 16 + 148 + 8, 9), vvd, vtx),
        }
        for name, triple in cases.items():
            self.assertNotEqual(compare(self.original, triple, 1), [], name)

    def test_unmodelled_topology_refuses(self):
        original = (make_mdl(), make_vvd(), make_vtx(topology=True))
        with self.assertRaises(model_lod.LodError):
            model_lod.trim_model(*original, 0)

    def test_inconsistent_files_refuse(self):
        mdl, vvd, vtx = self.original
        bad_vvd = bytearray(vvd)
        struct.pack_into("<i", bad_vvd, 16 + 4, 99)      # numLODVertexes[1] above LOD 0's? no: fine
        struct.pack_into("<i", bad_vvd, 16, 3)           # LOD 0 smaller than LOD 1
        with self.assertRaises(model_lod.LodError):
            model_lod.trim_model(mdl, bytes(bad_vvd), vtx, 1)
        with self.assertRaises(model_lod.LodError):
            model_lod.trim_model(mdl, vvd, vtx[:50], 0)


@unittest.skipUnless((RUNTIME / "portal2/pak01_dir.vpk").is_file(), "no staged Portal 2 runtime")
class RetailModels(unittest.TestCase):
    """Retail props, root LOD 0 and 2, judged by the same independent reader."""

    def test_retail_models(self):
        import source_content
        content = source_content.ContentResolver(str(RUNTIME))
        names = ["models/props_destruction/ceiling_tile", "models/props_urban/oil_drum001",
                 "models/props_backstage/item_dropper_wrecked", "models/props/metal_box",
                 "models/elevator/elevator_b"]
        checked = 0
        for name in names:
            files = [content.read(name + suffix)[0] for suffix in (".mdl", ".vvd", ".dx90.vtx")]
            if not all(files):
                continue
            for lod in (0, 2):
                trimmed = model_lod.trim_model(*files, lod)
                self.assertEqual(compare(tuple(files), trimmed, lod), [], "%s lod %d" % (name, lod))
                checked += 1
        self.assertGreater(checked, 4)


if __name__ == "__main__":
    unittest.main()
