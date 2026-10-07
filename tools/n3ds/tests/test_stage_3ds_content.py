#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Closure walk and VTF downscale checks for stage_3ds_content.py (stdlib + Pillow).

    python3 -m unittest discover -s tools/n3ds/tests -v
"""

import struct
import sys
import tempfile
import unittest
import zlib
from pathlib import Path

from PIL import Image

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

import stage_3ds_content as stage  # noqa: E402


def make_vtf(width, height, rgba_for, flags=0, mips=None, frames=1):
    """A VTF 7.2 RGBA8888 file; rgba_for(x, y, frame, level) gives each texel."""
    levels = mips if mips is not None else max(width, height).bit_length()
    header = bytearray(80)
    header[0:4] = b"VTF\0"
    struct.pack_into("<3I", header, 4, 7, 2, 80)
    struct.pack_into("<HHIHH", header, 16, width, height, flags, frames, 0)
    struct.pack_into("<3f", header, 32, 0.25, 0.5, 0.75)
    struct.pack_into("<f", header, 48, 1.0)
    struct.pack_into("<iBiBB", header, 52, 0, levels, -1, 0, 0)
    struct.pack_into("<H", header, 63, 1)
    body = bytearray()
    for level in range(levels - 1, -1, -1):
        w, h = max(1, width >> level), max(1, height >> level)
        for frame in range(frames):
            for y in range(h):
                for x in range(w):
                    body += bytes(rgba_for(x, y, frame, level))
    return bytes(header) + bytes(body)


def make_mdl(textures, cdmaterials, includes=()):
    """The parts of a studio header the closure walk reads."""
    header = bytearray(348)
    header[0:4] = b"IDST"
    struct.pack_into("<i", header, 4, 48)
    strings = bytearray()
    body = bytearray()
    texture_index = len(header)
    records = bytearray(64 * len(textures))
    cd_table = bytearray(4 * len(cdmaterials))
    include_records = bytearray(8 * len(includes))
    base = texture_index + len(records) + len(cd_table) + len(include_records)
    for index, name in enumerate(textures):
        struct.pack_into("<i", records, 64 * index, base + len(strings) - (texture_index + 64 * index))
        strings += name.encode() + b"\0"
    cd_index = texture_index + len(records)
    for index, name in enumerate(cdmaterials):
        struct.pack_into("<i", cd_table, 4 * index, base + len(strings))
        strings += name.encode() + b"\0"
    include_index = cd_index + len(cd_table)
    for index, name in enumerate(includes):
        struct.pack_into("<i", include_records, 8 * index + 4,
                         base + len(strings) - (include_index + 8 * index))
        strings += name.encode() + b"\0"
    struct.pack_into("<4i", header, 204, len(textures), texture_index, len(cdmaterials), cd_index)
    struct.pack_into("<2i", header, 336, len(includes), include_index)
    return bytes(header) + bytes(records) + bytes(cd_table) + bytes(include_records) + bytes(strings)


class VtfDownscale(unittest.TestCase):
    def test_downscales_with_full_mip_chain_and_keeps_header_fields(self):
        data = make_vtf(512, 256, lambda x, y, f, l: (200, 100, 50, 255), flags=0x8000 | 0x2000)
        out, info = stage.downscale_vtf(data, 128)
        header = stage.vtf_header(out)
        self.assertEqual((header["width"], header["height"]), (128, 64))
        self.assertEqual(header["mips"], 8)  # 128 down to 1 px
        self.assertEqual(header["minor"], 2)
        self.assertEqual(header["flags"], 0x8000 | 0x2000)
        self.assertEqual(header["format"], stage.FORMAT_DXT1)  # alpha all 255
        self.assertEqual(header["reflectivity"], (0.25, 0.5, 0.75))
        self.assertEqual(len(out), 80 + sum(
            stage.mip_size(13, max(1, 128 >> l), max(1, 64 >> l)) for l in range(8)))
        self.assertEqual(info["source"][:2], (512, 256))

    def test_pixels_survive_the_round_trip(self):
        def texel(x, y, f, l):
            return (255, 0, 0, 255) if x < (256 >> l) // 2 else (0, 0, 255, 255)
        data = make_vtf(256, 256, texel)
        out, _info = stage.downscale_vtf(data, 64)
        header = stage.vtf_header(out)
        image = stage.decode_frame(out, header, 0)
        self.assertEqual(image.size, (64, 64))
        left, right = image.getpixel((8, 32)), image.getpixel((56, 32))
        self.assertGreater(left[0], 230)
        self.assertLess(left[2], 25)
        self.assertGreater(right[2], 230)
        self.assertLess(right[0], 25)

    def test_alpha_selects_dxt5_and_rgba8888_is_available(self):
        data = make_vtf(16, 16, lambda x, y, f, l: (10, 20, 30, 128 if x < 8 else 255))
        out, _ = stage.downscale_vtf(data, 128)
        self.assertEqual(stage.vtf_header(out)["format"], stage.FORMAT_DXT5)
        out, _ = stage.downscale_vtf(data, 128, "rgba8888")
        header = stage.vtf_header(out)
        self.assertEqual(header["format"], stage.FORMAT_RGBA8888)
        image = stage.decode_frame(out, header, 0)
        self.assertEqual(image.getpixel((0, 0)), (10, 20, 30, 128))

    def test_small_texture_is_not_upscaled_and_nomip_keeps_one_level(self):
        data = make_vtf(8, 8, lambda x, y, f, l: (1, 2, 3, 255), flags=stage.FLAG_NOMIP, mips=1)
        out, _ = stage.downscale_vtf(data, 128)
        header = stage.vtf_header(out)
        self.assertEqual((header["width"], header["height"], header["mips"]), (8, 8, 1))

    def test_frames_are_kept(self):
        data = make_vtf(32, 32, lambda x, y, f, l: (255 * f, 0, 255 * (1 - f), 255), frames=2)
        out, _ = stage.downscale_vtf(data, 16)
        header = stage.vtf_header(out)
        self.assertEqual(header["frames"], 2)
        first = stage.decode_frame(out, header, 0).getpixel((4, 4))
        second = stage.decode_frame(out, header, 1).getpixel((4, 4))
        self.assertGreater(first[2], 200)
        self.assertGreater(second[0], 200)

    def test_cube_maps_are_refused_for_passthrough(self):
        data = make_vtf(8, 8, lambda x, y, f, l: (0, 0, 0, 255), flags=stage.FLAG_ENVMAP)
        with self.assertRaises(stage.VtfError):
            stage.downscale_vtf(data, 128)


def make_bsp(pak_first):
    """A minimal v20 BSP: entities, a game lump with one sub-lump, and a pak lump."""
    entities = b'{\n"classname" "worldspawn"\n}\n\0'
    sub = b"sub-lump-data"
    pak = stage.build_pak({"materials/a.vmt": b"x" * 1000, "junk.vhv": b"y" * 5000})
    out = bytearray(stage.BSP_HEADER_BYTES)
    lumps = {}

    def add(index, blob):
        while len(out) % 4:
            out.append(0)
        lumps[index] = (len(out), len(blob))
        out.extend(blob)

    if pak_first:
        add(stage.LUMP_PAKFILE, pak)
    add(0, entities)
    while len(out) % 4:
        out.append(0)
    game_at = len(out) + 4 + 16
    game = struct.pack("<i", 1) + struct.pack("<iHHii", 0x70727073, 0, 0, game_at, len(sub)) + sub
    add(stage.LUMP_GAME, game)
    if not pak_first:
        add(stage.LUMP_PAKFILE, pak)
    out[0:8] = struct.pack("<4si", b"VBSP", 20)
    for index, (offset, length) in lumps.items():
        struct.pack_into("<iiii", out, 8 + 16 * index, offset, length, 0, 0)
    return bytes(out), sub


class BspRewrite(unittest.TestCase):
    def check(self, pak_first):
        data, sub = make_bsp(pak_first)
        files = {"materials/a.vmt": b"small"}
        new = stage.rewrite_bsp(data, stage.build_pak(files))
        stage.verify_bsp(new, data, files)  # lumps, static props, pak contents
        self.assertLess(len(new), len(data))

        import legacy_bsp
        bsp = legacy_bsp.LegacyBsp(new)
        game = bsp.lump(stage.LUMP_GAME, 0)
        _id, _flags, _version, offset, length = struct.unpack_from("<iHHii", game, 4)
        self.assertEqual(new[offset:offset + length], sub)  # the sub-lump offset moved with it
        self.assertEqual(bsp.pakfile().namelist(), ["materials/a.vmt"])

    def test_pak_last_and_pak_first(self):
        self.check(False)
        self.check(True)

    def test_verify_rejects_a_damaged_lump(self):
        data, _sub = make_bsp(False)
        files = {"materials/a.vmt": b"small"}
        new = bytearray(stage.rewrite_bsp(data, stage.build_pak(files)))
        offset = struct.unpack_from("<i", new, 8)[0]
        new[offset + 3] ^= 0xFF
        with self.assertRaises(ValueError):
            stage.verify_bsp(bytes(new), data, files)


class VpkPacking(unittest.TestCase):
    FILES = {"materials/a/wall.vmt": b"vmt text", "materials/a/wall.vtf": bytes(range(256)) * 9,
             "models/crate.mdl": b"mdl", "models/crate.dx90.vtx": b"vtx", "readme": b"no extension",
             "top.txt": b"root file", "resource/ui/x.res": b""}

    def decode(self, directory, archives):
        """Read the bytes the way vpklib/packedstore.cpp does, not through the writer."""
        marker, version, tree, embedded, hashes, selfhash, signature = struct.unpack_from(
            "<7I", directory, 0)
        self.assertEqual((marker, version, embedded, hashes, selfhash, signature),
                         (0x55AA1234, 2, 0, 0, 0, 0))
        self.assertEqual(28 + tree, len(directory))      # the _dir file is the tree alone
        raw = directory[28:]
        pos, found = 0, {}

        def text():
            nonlocal pos
            end = raw.index(b"\0", pos)
            value = raw[pos:end].decode()
            pos = end + 1
            return value
        while True:
            ext = text()
            if not ext:
                break
            while True:
                folder = text()
                if not folder:
                    break
                while True:
                    base = text()
                    if not base:
                        break
                    crc, preload, archive, offset, length, end = struct.unpack_from(
                        "<IHHIIH", raw, pos)
                    pos += 18 + preload
                    self.assertEqual(end, 0xFFFF)
                    self.assertLess(archive, len(archives))   # pak01_%03d.vpk, absolute offset
                    name = base + ("" if ext == " " else "." + ext)
                    name = name if folder == " " else folder + "/" + name
                    blob = archives[archive][offset:offset + length]
                    self.assertEqual(len(blob), length)
                    self.assertEqual(zlib.crc32(blob) & 0xFFFFFFFF, crc)
                    found[name] = blob
        return found

    def test_engine_reader_layout(self):
        directory, archives = stage.build_vpk(self.FILES)
        self.assertEqual(len(archives), 1)
        self.assertEqual(self.decode(directory, archives), self.FILES)

    def test_archives_respect_the_limit_and_never_split_a_file(self):
        files = {"a/%d.bin" % i: bytes([i]) * 40 for i in range(7)}
        files["big.bin"] = b"B" * 100
        directory, archives = stage.build_vpk(files, limit=100)
        self.assertGreater(len(archives), 3)
        for data in archives:
            self.assertTrue(len(data) <= 100 or data == b"B" * 100)
        self.assertEqual(self.decode(directory, archives), files)

    def test_pack_moves_files_and_keeps_the_loose_set(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            for name, blob in {**self.FILES, "gameinfo.txt": b"gi", "maps/m.bsp": b"bsp",
                               "cfg/config.cfg": b"cfg", "maps/m.nav": b"nav"}.items():
                (root / name).parent.mkdir(parents=True, exist_ok=True)
                (root / name).write_bytes(blob)
            packed = stage.pack_vpk(root, limit=64)
            self.assertEqual(set(packed), set(self.FILES) | {"maps/m.nav"})
            left = sorted(p.relative_to(root).as_posix() for p in root.rglob("*") if p.is_file())
            archives = [n for n in left if n.startswith("pak01_0")]
            self.assertGreater(len(archives), 1)
            self.assertEqual(archives, ["pak01_%03d.vpk" % i for i in range(len(archives))])
            self.assertEqual([n for n in left if n not in archives],
                             ["cfg/config.cfg", "gameinfo.txt", "maps/m.bsp", "pak01_dir.vpk"])
            self.assertFalse((root / "materials").exists())   # emptied directories go too

    def test_verify_catches_a_changed_byte_and_a_missing_entry(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            directory, archives = stage.build_vpk(self.FILES)
            (root / "pak01_dir.vpk").write_bytes(directory)
            (root / "pak01_000.vpk").write_bytes(archives[0])
            stage.verify_vpk(root / "pak01_dir.vpk", self.FILES)
            damaged = bytearray(archives[0])
            damaged[5] ^= 0xFF
            (root / "pak01_000.vpk").write_bytes(bytes(damaged))
            with self.assertRaises(ValueError):
                stage.verify_vpk(root / "pak01_dir.vpk", self.FILES)
            (root / "pak01_000.vpk").write_bytes(archives[0])
            with self.assertRaises(ValueError):
                stage.verify_vpk(root / "pak01_dir.vpk", {**self.FILES, "extra.txt": b"x"})
            # data left inside the _dir file (the 120 MB heap problem) is rejected
            (root / "pak01_dir.vpk").write_bytes(directory + b"x" * 10)
            with self.assertRaises(ValueError):
                stage.verify_vpk(root / "pak01_dir.vpk", self.FILES)

    def test_gameinfo_lists_the_vpk_first(self):
        source = ('"GameInfo"\n{\n\tFileSystem\n\t{\n\t\tSearchPaths\n\t\t{\n'
                  '\t\t\tgame |gameinfo_path|x\n\t\t}\n\t}\n}\n')
        with tempfile.TemporaryDirectory() as temp:
            destination = Path(temp) / "gameinfo.txt"
            stage.write_gameinfo(source, destination, with_vpk=True)
            lines = [l.split() for l in destination.read_text().splitlines() if "gameinfo_path" in l]
            self.assertEqual(lines[0], ["game+mod", "|gameinfo_path|pak01_dir.vpk"])
            self.assertEqual(lines[2][-1], "|gameinfo_path|.")
            stage.write_gameinfo(source, destination)
            self.assertNotIn("vpk", destination.read_text())


class Languages(unittest.TestCase):
    def test_other_languages_under_resource_are_dropped(self):
        for name in ("resource/subtitles_french.txt", "resource/closecaption_schinese.dat",
                     "resource/basemodui_tchinese.txt", "resource/ui/x_koreana.txt"):
            self.assertTrue(stage.is_other_language(name), name)
        for name in ("resource/subtitles_english.txt", "resource/closecaption_english.dat",
                     "resource/clientscheme.res", "resource/ui/hud.res", "scripts/x_french.txt",
                     "resource/frenchfries.txt", "resource/a_french.res"):
            self.assertFalse(stage.is_other_language(name), name)


class EditorModels(unittest.TestCase):
    def test_editor_models_are_recognised(self):
        self.assertTrue(stage.is_editor_model("models/editor/axis_helper.mdl"))
        self.assertTrue(stage.is_editor_model("models/props_map_editor/x.mdl"))
        self.assertFalse(stage.is_editor_model("models/props/crate.mdl"))


class VmtRewrite(unittest.TestCase):
    TEXT = '''"LightmappedGeneric"
{
    "$basetexture" "tile/wall"  // base
    "$bumpmap" "tile/wall_normal"
    $envmap env_cubemap
    "$surfaceprop" "tile"
    "Proxies"
    {
        "AnimatedTexture" { "animatedtexturevar" "$bumpmap" "$bumpmap" "x" }
    }
}
'''

    def test_drops_only_ignored_texture_keys(self):
        text, dropped, textures, includes = stage.rewrite_vmt(self.TEXT)
        self.assertEqual([k for k, _ in dropped], ["$bumpmap", "$envmap"])
        self.assertEqual(textures, ["tile/wall"])
        self.assertNotIn('"$bumpmap" "tile', text)
        self.assertIn("$surfaceprop", text)
        self.assertIn("AnimatedTexture", text)  # proxy lines are not material keys
        self.assertEqual(includes, [])

    def test_patch_keys_and_include(self):
        patch = '"patch"\n{\n "include" "materials/a/b.vmt"\n "insert"\n {\n'\
                '  "$basetexture" "a/c"\n  "$normalmap" "a/n"\n }\n}\n'
        text, dropped, textures, includes = stage.rewrite_vmt(patch)
        self.assertEqual(includes, ["materials/a/b.vmt"])
        self.assertEqual(textures, ["a/c"])
        self.assertEqual([k for k, _ in dropped], ["$normalmap"])


class MdlReferences(unittest.TestCase):
    def test_reads_textures_cdmaterials_and_includes(self):
        data = make_mdl(["wood", "metal"], ["models/a/", "models/b/"], ["models/shared.mdl"])
        textures, cds, includes = stage.mdl_references(data)
        self.assertEqual(textures, ["wood", "metal"])
        self.assertEqual(cds, ["models/a", "models/b"])
        self.assertEqual(includes, ["models/shared.mdl"])


class ClosureWalk(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.runtime = Path(self.temp.name) / "runtime"
        self.out = Path(self.temp.name) / "out"
        self.addCleanup(self.temp.cleanup)
        root = self.runtime / "portal"  # a directory layer in ContentResolver's search order
        (root / "materials/tile").mkdir(parents=True)
        (root / "materials/models/a").mkdir(parents=True)
        (root / "models").mkdir()
        vtf = make_vtf(256, 256, lambda x, y, f, l: (9, 9, 9, 255))
        (root / "materials/tile/wall.vtf").write_bytes(vtf)
        (root / "materials/tile/wall_n.vtf").write_bytes(vtf)
        (root / "materials/tile/wall.vmt").write_text(
            '"LightmappedGeneric"\n{\n"$basetexture" "tile/wall"\n"$bumpmap" "tile/wall_n"\n}\n')
        (root / "materials/tile/patched.vmt").write_text(
            '"patch"\n{\n"include" "materials/tile/wall.vmt"\n}\n')
        (root / "materials/models/a/wood.vmt").write_text(
            '"VertexLitGeneric"\n{\n"$basetexture" "models/a/wood"\n}\n')
        (root / "materials/models/a/wood.vtf").write_bytes(vtf)
        (root / "models/crate.mdl").write_bytes(
            make_mdl(["wood", "gone"], ["models/a/"]))
        (root / "models/crate.vvd").write_bytes(b"vvd")
        (root / "models/crate.dx90.vtx").write_bytes(b"vtx")
        (root / "models/crate.phy").write_bytes(b"phy")
        self.content = stage.Content(self.runtime)

    def test_walk_copies_closure_and_reports_missing(self):
        stager = stage.Stager(self.content, self.out / "portal2", 64)
        stager.material("tile/patched", "test")      # patch -> include -> wall
        stager.model("models/crate.mdl", "test")
        stager.material("tile/absent", "test")
        stager.textures()
        written = set(stager.written)
        for name in ("materials/tile/patched.vmt", "materials/tile/wall.vmt",
                     "materials/tile/wall.vtf", "materials/models/a/wood.vmt",
                     "materials/models/a/wood.vtf", "models/crate.mdl", "models/crate.vvd",
                     "models/crate.dx90.vtx", "models/crate.phy"):
            self.assertIn(name, written)
        self.assertNotIn("materials/tile/wall_n.vtf", written)  # the dropped $bumpmap
        self.assertNotIn("$bumpmap", (self.out / "portal2/materials/tile/wall.vmt").read_text())
        self.assertIn("materials/tile/absent.vmt", stager.missing)
        self.assertTrue(any("gone" in name for name in stager.missing))
        header = stage.vtf_header((self.out / "portal2/materials/tile/wall.vtf").read_bytes())
        self.assertEqual((header["width"], header["height"]), (64, 64))

    def test_gameinfo_mounts_only_the_mod_directory(self):
        source = ('"GameInfo"\n{\n\ttitle "X"\n\tFileSystem\n\t{\n\t\tSteamAppId 1\n\t\tSearchPaths\n'
                  '\t\t{\n\t\t\tgame |gameinfo_path|../other\n\t\t\tplatform |gameinfo_path|../platform\n'
                  '\t\t}\n\t}\n}\n')
        destination = self.out / "gameinfo.txt"
        destination.parent.mkdir(parents=True)
        stage.write_gameinfo(source, destination)
        text = destination.read_text()
        self.assertNotIn("../other", text)
        self.assertNotIn("../platform", text)
        self.assertIn("|gameinfo_path|.", text)
        self.assertIn("SteamAppId", text)


if __name__ == "__main__":
    unittest.main()
