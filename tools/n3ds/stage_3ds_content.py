#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Stage a Nintendo-3DS-sized Portal 2 content set for one map.

The 3DS PICA backend (materialsystem/shaderapipica) samples only each
material's base texture, downscales it to at most 128 px per side and encodes
ETC1 itself. The device has roughly 124-178 MB of RAM in total, so this tool's
job is to cut the bytes the engine reads and decodes:

  1. read the map BSP and compute its asset closure: world materials (texdata
     string table), static prop models, entity models/materials/sprites, each
     model's materials ($cdmaterials + texture names in the MDL, included
     models), and each material's textures ($basetexture, $basetexture2, and
     `include` patch targets);
  2. add a small, guessed boot set (cfg, scripts, resource schemes, debug and
     engine materials, models/error.mdl; see BOOT_FILES and BOOT_NOTES);
  3. write a loose-file mod directory (<out>/<mod>): the BSP, models as is,
     VMTs rewritten only to drop keys that reference textures the 3DS ignores,
     base textures re-encoded as VTF 7.2 DXT1/DXT5 (or RGBA8888) at no more than
     --max-size per side with a full mip chain, and a gameinfo.txt that mounts
     only that directory;
  4. report file count, bytes before and after, the largest remaining files
     and every missing asset.

    python3 tools/n3ds/stage_3ds_content.py --bsp sp_a1_intro1 \\
        --runtime run/runtime-p2 --out quality-results/n3ds-content/sp_a1_intro1


BSP pak lump (embedded cubemaps, per-map materials, static prop lighting) is
rewritten to hold only the closure's files, re-encoded the same way. .ani
animation blocks and editor-only models are left out (--keep-ani, --keep-pak), and
every model is cut to one LOD (--model-lod N, --no-lod-trim; see model_lod.py), and
the loose files are packed into <mod>/pak01_dir.vpk (--no-vpk to keep them loose).
"""

import argparse
import fnmatch
import io
import json
import os
import re
import struct
import sys
import zlib
import zipfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
sys.path.insert(0, str(HERE))
sys.path.insert(0, str(ROOT / "tools/quality"))

import make_bitmap_font  # noqa: E402
import model_lod  # noqa: E402
from PIL import Image  # noqa: E402

DEFAULT_MAX_SIZE = 128
DEFAULT_MOD = "portal2"

# Texture keys the 3DS never samples. Their keys are removed from the VMT so
# the material system does not load the textures at all. The first eight are
# the user's list; the rest are the same kind (other maps of a lighting or
# reflection model, and the HDR sky variants the non-HDR 3DS never reads).
DROP_KEYS = frozenset([
    "$bumpmap", "$normalmap", "$envmap", "$envmapmask", "$detail",
    "$phongexponenttexture", "$lightwarptexture", "$selfillummask",
    "$bumpmap2", "$normalmap2", "$bumpmask", "$detail2", "$phongwarptexture",
    "$ambientoccltexture", "$blendmodulatetexture", "$hdrbasetexture",
    "$hdrcompressedtexture", "$envmapmask2", "$iris", "$warptexture",
])
# Texture keys that stay: they are read, closed over and downscaled.
KEEP_TEXTURE_KEYS = frozenset(["$basetexture", "$basetexture2", "$texture2", "$texture1",
                               "$texture3", "$flowmap"])
# Keys whose value names another VMT (patch include).
INCLUDE_KEYS = frozenset(["include"])
# Blocks (inside a top-level patch material) that carry material keys.
PATCH_BLOCKS = frozenset(["insert", "replace"])

# What the engine and the GameUI load before any map is up. This is a guess
# read from the file system search the retail game does at boot; each pattern is
# a fnmatch against a lower-case logical path, and a pattern that matches nothing
# is reported in BOOT_NOTES, never an error.
BOOT_FILES = [
    # Whole trees the engine, client and VGUI read by name at startup and level
    # load (fnmatch's * crosses directories). Their textures are downscaled
    # like everything else; sounds, scenes and the commentary stay out.
    "cfg/*", "scripts/*", "resource/*", "particles/*",
    "materials/vgui/*", "materials/console/*", "materials/debug/*", "materials/engine/*",
    "materials/editor/*", "materials/gamepadui/*", "materials/dev/*",
    "models/error.*",
]
# Fonts and the 2D UI skin: the VGUI scheme loads these by name. With no fonts
# under platform/resource the font manager has nothing to rasterize.
BOOT_NOTES = [
    "boot set is a guess: cfg, scripts and resource schemes the retail game reads at "
    "startup; sounds, scenes, particles (*.pcf), vscripts and the commentary are left out",
]

# VTF image formats that Pillow can decode here.
RAW_MODES = {0: ("RGBA", "RGBA", True), 1: ("RGBA", "ABGR", True), 2: ("RGB", "RGB", False),
             3: ("RGB", "BGR", False), 11: ("RGBA", "ARGB", True), 12: ("RGBA", "BGRA", True),
             16: ("RGB", "BGRX", False), 5: ("L", "L", False), 6: ("LA", "LA", True)}
BYTES_PER_PIXEL = {0: 4, 1: 4, 2: 3, 3: 3, 4: 2, 5: 1, 6: 2, 8: 1, 11: 4, 12: 4, 16: 4,
                   17: 2, 18: 2, 19: 2, 21: 2, 22: 2}
DXT_FORMATS = {13: 8, 14: 16, 15: 16, 20: 8}
FORMAT_DXT1, FORMAT_DXT5, FORMAT_RGBA8888 = 13, 15, 0
FLAG_NOMIP, FLAG_ENVMAP = 0x100, 0x4000
FLAG_ONEBITALPHA, FLAG_EIGHTBITALPHA = 0x1000, 0x2000
RESOURCE_HIGHRES = 0x30
LUMP_ENTITIES_WORDS = (".mdl", ".vmt", ".spr")


# Steam languages other than English. Text and cache files for them under resource/
# are left out: the 3DS runs in English and each language's subtitles are ~2 MB.
OTHER_LANGUAGES = ("brazilian", "bulgarian", "czech", "danish", "dutch", "finnish", "french",
                   "german", "greek", "hungarian", "italian", "japanese", "korean", "koreana",
                   "latam", "norwegian", "polish", "portuguese", "romanian", "russian",
                   "schinese", "spanish", "swedish", "tchinese", "thai", "turkish", "ukrainian",
                   "vietnamese")
LANGUAGE_FILE = re.compile(r"_(%s)\.(txt|dat)$" % "|".join(OTHER_LANGUAGES))


def is_other_language(logical):
    """A resource/ text or cache file for a non-English language."""
    return logical.startswith("resource/") and LANGUAGE_FILE.search(logical) is not None


def norm(path):
    """Lower-case, forward-slash logical path with no leading slash."""
    return path.replace("\\", "/").strip().strip("/").lower()


# --------------------------------------------------------------------------
# Content sources
# --------------------------------------------------------------------------

class Content:
    """A staged runtime in gameinfo order, plus the BSP's pak lump on top."""

    def __init__(self, runtime, pak=None):
        import source_content
        self.resolver = source_content.ContentResolver(str(runtime))
        self.pak = {}
        if pak is not None:
            for info in pak.infolist():
                self.pak[norm(info.filename)] = pak.read(info)
        self._listing = None

    def read(self, relative):
        """(data, origin) or (None, None)."""
        relative = norm(relative)
        if relative in self.pak:
            return self.pak[relative], "bsp-pak:" + relative
        return self.resolver.read(relative)

    def exists(self, relative):
        return self.read(relative)[0] is not None

    def names(self):
        """Every logical file name (lower case) across the layers."""
        if self._listing is None:
            names = set(self.pak)
            for kind, path, vpk in self.resolver.layers:
                if kind == "vpk":
                    names.update(vpk.entries)
                else:
                    for base, _dirs, files in os.walk(path):
                        for name in files:
                            names.add(norm(os.path.relpath(os.path.join(base, name), path)))
            self._listing = names
        return self._listing


# --------------------------------------------------------------------------
# KeyValues (VMT) handling
# --------------------------------------------------------------------------

def tokenize_line(line):
    """Tokens of one VMT line: quoted or bare words, braces, // comments dropped."""
    tokens = []
    index, length = 0, len(line)
    while index < length:
        char = line[index]
        if char.isspace():
            index += 1
        elif char == "/" and line[index:index + 2] == "//":
            break
        elif char == '"':
            end = line.find('"', index + 1)
            if end < 0:
                end = length
            tokens.append(line[index + 1:end])
            index = end + 1
        elif char in "{}":
            tokens.append(char)
            index += 1
        else:
            start = index
            while index < length and not line[index].isspace() and line[index] not in '{}"':
                index += 1
            tokens.append(line[start:index])
    return tokens


def scan_vmt(text):
    """[(line, key, value, scope)] for every two-token key/value line.

    scope is "top" for a key directly in the material block or in a patch
    insert/replace block, and "other" for proxies, fallback blocks and the
    like. Other lines are returned with key None."""
    result = []
    stack = []  # block names
    pending = None  # last bare token, the name of a block opened by `{`
    for line in text.splitlines(keepends=True):
        tokens = tokenize_line(line)
        entry_scope = stack[:]
        if len(tokens) == 2 and tokens[0] not in "{}" and tokens[1] not in "{}":
            top = len(stack) == 1 or (len(stack) == 2 and stack[1] in PATCH_BLOCKS)
            result.append((line, tokens[0].lower(), tokens[1], "top" if top else "other"))
            pending = None
            continue
        for token in tokens:
            if token == "{":
                stack.append((pending or "").lower() if len(stack) else "material")
                pending = None
            elif token == "}":
                if stack:
                    stack.pop()
                pending = None
            else:
                pending = token
        result.append((line, None, None, "other"))
    return result


def rewrite_vmt(text):
    """(new text, dropped keys list, kept texture names, included vmt names)."""
    out, dropped, textures, includes = [], [], [], []
    for line, key, value, scope in scan_vmt(text):
        if key is not None and scope == "top":
            if key in DROP_KEYS:
                dropped.append((key, value))
                continue
            if key in KEEP_TEXTURE_KEYS and value.strip():
                textures.append(norm(value))
            elif key in INCLUDE_KEYS and value.strip():
                includes.append(norm(value))
        out.append(line)
    return "".join(out), dropped, textures, includes


# --------------------------------------------------------------------------
# MDL
# --------------------------------------------------------------------------

def cstring(data, offset):
    end = data.find(b"\0", offset)
    return data[offset:end if end >= 0 else len(data)].decode("latin-1")


def mdl_references(data):
    """(texture names, cdmaterials dirs, included model names) of one MDL."""
    if len(data) < 348 or data[:4] != b"IDST":
        raise ValueError("not a studio model")
    numtextures, textureindex, numcd, cdindex = struct.unpack_from("<4i", data, 204)
    numinclude, includeindex = struct.unpack_from("<2i", data, 336)
    textures = []
    for index in range(numtextures):
        base = textureindex + 64 * index
        textures.append(norm(cstring(data, base + struct.unpack_from("<i", data, base)[0])))
    cdmaterials = []
    for index in range(numcd):
        offset = struct.unpack_from("<i", data, cdindex + 4 * index)[0]
        cdmaterials.append(norm(cstring(data, offset)))
    includes = []
    for index in range(max(numinclude, 0)):
        base = includeindex + 8 * index
        includes.append(norm(cstring(data, base + struct.unpack_from("<i", data, base + 4)[0])))
    return textures, cdmaterials, includes


MODEL_SIDECARS = (".vvd", ".dx90.vtx", ".phy")
# External animation blocks are the largest model files; static props never read them.
ANIMATION_SIDECAR = ".ani"
# Models that exist for the map editor only.
EDITOR_MODEL_MARKERS = ("models/editor/", "_map_editor/")


def is_editor_model(logical):
    return any(marker in logical for marker in EDITOR_MODEL_MARKERS)


# --------------------------------------------------------------------------
# VTF
# --------------------------------------------------------------------------

class VtfError(Exception):
    pass


def vtf_header(data):
    if len(data) < 64 or data[:4] != b"VTF\0":
        raise VtfError("not a VTF")
    major, minor, header_size = struct.unpack_from("<3I", data, 4)
    if major != 7 or minor > 5:
        raise VtfError("VTF %d.%d is not handled" % (major, minor))
    width, height, flags, frames, first_frame = struct.unpack_from("<HHIHH", data, 16)
    reflectivity = struct.unpack_from("<3f", data, 32)
    bump_scale = struct.unpack_from("<f", data, 48)[0]
    fmt, mips, low_fmt, low_w, low_h = struct.unpack_from("<iBiBB", data, 52)
    depth = struct.unpack_from("<H", data, 63)[0] if minor >= 2 else 1
    return {"minor": minor, "header_size": header_size, "width": width, "height": height,
            "flags": flags, "frames": max(frames, 1), "first_frame": first_frame,
            "reflectivity": reflectivity, "bump_scale": bump_scale, "format": fmt, "mips": mips,
            "low_format": low_fmt, "low_w": low_w, "low_h": low_h, "depth": max(depth, 1)}


def mip_size(fmt, width, height):
    if fmt in DXT_FORMATS:
        return max(1, (width + 3) // 4) * max(1, (height + 3) // 4) * DXT_FORMATS[fmt]
    if fmt not in BYTES_PER_PIXEL:
        raise VtfError("unknown VTF format %d" % fmt)
    return width * height * BYTES_PER_PIXEL[fmt]


def image_offset(data, info):
    """Offset of the high-resolution image data."""
    if info["minor"] >= 3:
        count = struct.unpack_from("<I", data, 68)[0]
        for index in range(count):
            tag, offset = struct.unpack_from("<II", data, 80 + 8 * index)
            if tag & 0xFFFFFF == RESOURCE_HIGHRES and not tag >> 24 & 0x2:
                return offset
        raise VtfError("no plain high-resolution image resource")
    offset = info["header_size"]
    if info["low_format"] >= 0 and info["low_w"]:
        offset += mip_size(info["low_format"], info["low_w"], info["low_h"])
    return offset


def dds_wrap(fourcc, width, height, blocks):
    """A one-surface DDS around block-compressed data, so Pillow can decode it."""
    header = struct.pack("<4s7I44s", b"DDS ", 124, 0x81007, height, width, len(blocks), 0, 1,
                         b"\0" * 44)
    header += struct.pack("<2I4s5I", 32, 4, fourcc, 0, 0, 0, 0, 0)   # pixel format: FOURCC
    header += struct.pack("<5I", 0x1000, 0, 0, 0, 0)                  # caps, caps2-4, reserved
    return header + blocks


def decode_frame(data, info, frame):
    """Mip 0 of one frame as an RGBA Pillow image."""
    fmt, width, height = info["format"], info["width"], info["height"]
    offset = image_offset(data, info)
    for level in range(info["mips"] - 1, 0, -1):
        offset += mip_size(fmt, max(1, width >> level), max(1, height >> level)) * info["frames"]
    size = mip_size(fmt, width, height)
    start = offset + size * frame
    raw = data[start:start + size]
    if len(raw) != size:
        raise VtfError("truncated image data")
    if fmt in DXT_FORMATS:
        code = {13: b"DXT1", 20: b"DXT1", 14: b"DXT3", 15: b"DXT5"}[fmt]
        return Image.open(io.BytesIO(dds_wrap(code, width, height, raw))).convert("RGBA")
    if fmt in RAW_MODES:
        mode, raw_mode, _alpha = RAW_MODES[fmt]
        return Image.frombytes(mode, (width, height), raw, "raw", raw_mode).convert("RGBA")
    raise VtfError("VTF format %d is not decoded" % fmt)


def format_has_alpha(fmt):
    return fmt in (14, 15, 20, 1, 0, 11, 12, 6, 8, 19, 21)


def encode_blocks(image, fmt):
    """DXT1 or DXT5 block data of an RGBA image (edge-padded to whole blocks)."""
    width, height = image.size
    padded = (-(-width // 4) * 4, -(-height // 4) * 4)
    if padded != image.size:
        canvas = Image.new("RGBA", padded)
        canvas.paste(image, (0, 0))
        # Replicate the last row and column so the border blocks do not darken.
        for x in range(width, padded[0]):
            canvas.paste(image.crop((width - 1, 0, width, height)), (x, 0))
        for y in range(height, padded[1]):
            canvas.paste(canvas.crop((0, height - 1, padded[0], height)), (0, y))
        image = canvas
    out = io.BytesIO()
    image.save(out, "DDS", pixel_format="DXT1" if fmt == FORMAT_DXT1 else "DXT5")
    return out.getvalue()[128:]


def encode_level(image, fmt):
    if fmt == FORMAT_RGBA8888:
        return image.tobytes("raw", "RGBA")
    return encode_blocks(image, fmt)


def target_size(width, height, max_size):
    while max(width, height) > max_size and max(width, height) > 1:
        width, height = max(1, width >> 1), max(1, height >> 1)
    return width, height


def mip_chain(base, count):
    chain = [base]
    for _ in range(count - 1):
        previous = chain[-1]
        size = (max(1, previous.width >> 1), max(1, previous.height >> 1))
        chain.append(previous.resize(size, Image.BOX))
    return chain


def downscale_vtf(data, max_size, out_format="auto"):
    """(new VTF bytes, info dict). Raises VtfError when the file is not handled."""
    info = vtf_header(data)
    if info["flags"] & FLAG_ENVMAP or info["depth"] != 1:
        raise VtfError("cube and volume textures are copied as is")
    frames = [decode_frame(data, info, frame) for frame in range(info["frames"])]
    width, height = target_size(info["width"], info["height"], max_size)
    if (width, height) != frames[0].size:
        frames = [frame.resize((width, height), Image.BOX) for frame in frames]
    alpha = format_has_alpha(info["format"]) and any(
        frame.getchannel("A").getextrema()[0] < 255 for frame in frames)
    fmt = {"rgba8888": FORMAT_RGBA8888, "dxt": FORMAT_DXT5 if alpha else FORMAT_DXT1,
           "auto": FORMAT_DXT5 if alpha else FORMAT_DXT1}[out_format]
    flags = info["flags"]
    mips = 1 if flags & FLAG_NOMIP else max(width, height).bit_length()
    chains = [mip_chain(frame, mips) for frame in frames]
    header = bytearray(80)
    header[0:4] = b"VTF\0"
    struct.pack_into("<3I", header, 4, 7, 2, 80)
    struct.pack_into("<HHIHH", header, 16, width, height, flags, len(frames), info["first_frame"])
    struct.pack_into("<3f", header, 32, *info["reflectivity"])
    struct.pack_into("<f", header, 48, info["bump_scale"])
    struct.pack_into("<iBiBB", header, 52, fmt, mips, -1, 0, 0)
    struct.pack_into("<H", header, 63, 1)
    body = bytearray()
    for level in range(mips - 1, -1, -1):
        for chain in chains:
            body += encode_level(chain[level], fmt)
    result = bytes(header) + bytes(body)
    return result, {"width": width, "height": height, "mips": mips, "format": fmt,
                    "source": (info["width"], info["height"], info["format"])}


# --------------------------------------------------------------------------
# BSP
# --------------------------------------------------------------------------

def read_bsp(path):
    import legacy_bsp
    return legacy_bsp.LegacyBsp.read(path)


def bsp_references(bsp):
    """(materials, models, entity vmts, skyname, vscripts) from a LegacyBsp."""
    materials = {norm(name) for name in bsp.material_names()}
    _version, dictionary, _props = bsp.static_props()
    models = {norm(name) for name in dictionary}
    vmts, vscripts, sky = set(), set(), None
    for entity in bsp.entities():
        for key, value in entity.items():
            for text in value if isinstance(value, list) else [value]:
                lowered = norm(text)
                if key == "skyname":
                    sky = lowered
                elif key == "vscripts":
                    vscripts.update(norm(v) for v in text.split())
                elif lowered.endswith(".mdl") and not lowered.startswith("*"):
                    models.add(lowered)
                elif lowered.endswith(".vmt"):
                    vmts.add(lowered if lowered.startswith("materials/") else "materials/" + lowered)
                elif lowered.endswith(".spr"):
                    vmts.add("materials/" + lowered[:-4] + ".vmt")
    return materials, models, vmts, sky, vscripts


BSP_HEADER_BYTES = 8 + 64 * 16 + 4
LUMP_GAME, LUMP_PAKFILE = 35, 40


def build_pak(files):
    """A stored (uncompressed) zip of {name: bytes}, the form the engine's pak reader takes."""
    buffer = io.BytesIO()
    with zipfile.ZipFile(buffer, "w", zipfile.ZIP_STORED) as archive:
        for name in sorted(files):
            archive.writestr(zipfile.ZipInfo(name, (1980, 1, 1, 0, 0, 0)), files[name])
    return buffer.getvalue()


def rewrite_bsp(data, pak_bytes):
    """The BSP with its pak lump replaced by pak_bytes; every other lump is carried verbatim.

    Lumps are written in their original file order, 4-byte aligned. The game lump's
    directory holds absolute file offsets for its sub-lumps, so they move with it."""
    ident, version = struct.unpack_from("<4si", data, 0)
    lumps = [list(struct.unpack_from("<iiii", data, 8 + 16 * i)) for i in range(64)]
    revision = struct.unpack_from("<i", data, 8 + 16 * 64)[0]
    out = bytearray(BSP_HEADER_BYTES)
    order = sorted((lump[0], index) for index, lump in enumerate(lumps) if lump[1])
    for _offset, index in order:
        offset, length, lump_version, uncompressed = lumps[index]
        blob = bytes(data[offset:offset + length])
        if index == LUMP_PAKFILE:
            blob, uncompressed = pak_bytes, 0
        while len(out) % 4:
            out.append(0)
        new_offset = len(out)
        if index == LUMP_GAME and new_offset != offset:
            if uncompressed:
                raise ValueError("cannot move a compressed game lump")
            blob = bytearray(blob)
            count = struct.unpack_from("<i", blob, 0)[0]
            for entry in range(count):
                at = 4 + 16 * entry
                lump_id, flags, sub_version, sub_offset, sub_length = struct.unpack_from(
                    "<iHHii", blob, at)
                struct.pack_into("<iHHii", blob, at, lump_id, flags, sub_version,
                                 sub_offset + new_offset - offset, sub_length)
            blob = bytes(blob)
        out += blob
        lumps[index] = [new_offset, len(blob), lump_version, uncompressed]
    out[0:8] = struct.pack("<4si", ident, version)
    for index, lump in enumerate(lumps):
        if not lump[1]:
            lump[0] = 0
        struct.pack_into("<iiii", out, 8 + 16 * index, *lump)
    struct.pack_into("<i", out, 8 + 16 * 64, revision)
    return bytes(out)


def game_sublumps(bsp):
    """[(id, flags, version, bytes)] of the game lump, read through its absolute offsets."""
    raw = bsp.lump(LUMP_GAME, 0)
    result = []
    for entry in range(struct.unpack_from("<i", raw, 0)[0] if raw else 0):
        lump_id, flags, version, offset, length = struct.unpack_from("<iHHii", raw, 4 + 16 * entry)
        result.append((lump_id, flags, version, bsp.data[offset:offset + length]))
    return result


def verify_bsp(new_data, old_data, expected_pak):
    """Re-read the rewritten BSP with legacy_bsp and compare it with the original."""
    import legacy_bsp
    new, old = legacy_bsp.LegacyBsp(new_data), legacy_bsp.LegacyBsp(old_data)
    for index in range(64):
        if index == LUMP_PAKFILE:
            continue
        if index == LUMP_GAME:
            if game_sublumps(new) != game_sublumps(old):
                raise ValueError("rewritten BSP differs in the game lump")
        elif new.lump(index, 0) != old.lump(index, 0):
            raise ValueError("rewritten BSP differs in lump %d" % index)
        if new.lump_version(index) != old.lump_version(index):
            raise ValueError("rewritten BSP changed the version of lump %d" % index)
    if new.static_props() != old.static_props():
        raise ValueError("rewritten BSP static props differ")
    archive = new.pakfile()
    names = sorted(archive.namelist()) if archive else []
    if names != sorted(expected_pak) or (archive and archive.testzip() is not None):
        raise ValueError("rewritten BSP pak lump does not hold the expected files")
    for name in names:
        if archive.read(name) != expected_pak[name]:
            raise ValueError("pak file %s changed" % name)


SKY_SIDES = ("up", "dn", "lf", "rt", "ft", "bk")


# --------------------------------------------------------------------------
# Closure and staging
# --------------------------------------------------------------------------

class Stager:
    def __init__(self, content, out_mod, max_size, out_format="auto", keep_ani=False, lod=0):
        self.lod = lod   # the one LOD kept in model files; None keeps them whole
        self.keep_ani = keep_ani
        self.pak_out = {}   # logical path -> bytes that stay in the rewritten BSP pak lump
        self.skipped_editor = []
        self.content = content
        self.out = Path(out_mod)
        self.max_size = max_size
        self.out_format = out_format
        self.written = {}   # logical path -> {"before", "after", "kind", "origin"}
        self.missing = {}   # logical path -> referrer
        self.notes = []
        self.seen_materials = set()
        self.seen_models = set()
        self.pending_textures = {}  # logical vtf path -> referrer

    # -- output ---------------------------------------------------------
    def put(self, logical, data, before, kind, origin):
        destination = self.out / logical
        destination.parent.mkdir(parents=True, exist_ok=True)
        destination.write_bytes(data)
        self.written[logical] = {"before": before, "after": len(data), "kind": kind,
                                 "origin": origin}

    def put_pak(self, logical, data, before, kind):
        """A file whose source is the BSP's pak lump: it stays in the (rewritten) pak lump."""
        self.pak_out[logical] = data
        self.written[logical] = {"before": before, "after": len(data), "kind": "pak-" + kind,
                                 "origin": "bsp-pak"}

    def copy(self, logical, referrer, kind="copy"):
        """Copy one file as is. False (and a missing record) when no layer has it."""
        if logical in self.written:
            return True
        data, origin = self.content.read(logical)
        if data is None:
            self.missing.setdefault(logical, referrer)
            return False
        if origin.startswith("bsp-pak"):
            self.put_pak(logical, data, len(data), kind)
            return True
        self.put(logical, data, len(data), kind, origin)
        return True

    # -- materials ------------------------------------------------------
    def material(self, name, referrer):
        """Stage materials/<name>.vmt and its textures; name has no extension."""
        name = norm(name)
        if name.endswith(".vmt"):
            name = name[:-4]
        if name.startswith("materials/"):
            name = name[len("materials/"):]
        return self.vmt("materials/%s.vmt" % name, referrer)

    def vmt(self, logical, referrer):
        if logical in self.seen_materials:
            return True
        self.seen_materials.add(logical)
        data, origin = self.content.read(logical)
        if data is None:
            self.missing.setdefault(logical, referrer)
            return False
        text = data.decode("utf-8", "replace")
        new_text, dropped, textures, includes = rewrite_vmt(text)
        encoded = new_text.encode("utf-8")
        if origin.startswith("bsp-pak"):
            self.put_pak(logical, encoded, len(data), "vmt")
        else:
            self.put(logical, encoded, len(data), "vmt", origin)
        self.written[logical]["dropped"] = [key for key, _ in dropped]
        for texture in textures:
            if texture.startswith("_rt_"):
                continue  # render targets are created at run time
            path = "materials/%s.vtf" % texture.removeprefix("materials/").removesuffix(".vtf")
            self.pending_textures.setdefault(path, logical)
        for include in includes:
            target = include if include.endswith(".vmt") else include + ".vmt"
            self.vmt(target, logical)
        return True

    def textures(self):
        for logical, referrer in sorted(self.pending_textures.items()):
            if logical in self.written:
                continue
            data, origin = self.content.read(logical)
            if data is None:
                self.missing.setdefault(logical, referrer)
                continue
            in_pak = origin.startswith("bsp-pak")
            try:
                converted, info = downscale_vtf(data, self.max_size, self.out_format)
            except (VtfError, struct.error, OSError, ValueError) as error:
                self.notes.append("%s copied as is: %s" % (logical, error))
                if in_pak:
                    self.put_pak(logical, data, len(data), "vtf-passthrough")
                else:
                    self.put(logical, data, len(data), "vtf-passthrough", origin)
                continue
            if in_pak:
                self.put_pak(logical, converted, len(data), "vtf")
            else:
                self.put(logical, converted, len(data), "vtf", origin)
            self.written[logical]["size"] = [info["width"], info["height"]]

    # -- models ---------------------------------------------------------
    def model(self, logical, referrer):
        if logical in self.seen_models:
            return
        self.seen_models.add(logical)
        if is_editor_model(logical):
            self.skipped_editor.append(logical)
            return
        data, origin = self.content.read(logical)
        if data is None:
            self.missing.setdefault(logical, referrer)
            return
        stem = logical[:-4]
        trimmed = self.trim_lod(logical, data, referrer) if self.lod is not None else None
        out_mdl = trimmed[0] if trimmed else data
        if origin.startswith("bsp-pak"):
            self.put_pak(logical, out_mdl, len(data), "model")
        else:
            self.put(logical, out_mdl, len(data), "model", origin)
        for suffix in MODEL_SIDECARS + ((ANIMATION_SIDECAR,) if self.keep_ani else ()):
            side = stem + suffix
            if trimmed and suffix in (".vvd", ".dx90.vtx"):
                original = self.content.read(side)[0]
                self.put(side, trimmed[1 if suffix == ".vvd" else 2], len(original), "model",
                         "trimmed:" + side)
            elif self.content.exists(side):
                self.copy(side, logical, "model")
            elif suffix in (".vvd", ".dx90.vtx"):
                self.missing.setdefault(side, logical)
        try:
            textures, cdmaterials, includes = mdl_references(data)
        except (ValueError, struct.error) as error:
            self.notes.append("%s: cannot read the MDL (%s)" % (logical, error))
            return
        for texture in textures:
            self.model_material(texture, cdmaterials, logical)
        for include in includes:
            self.model(include, logical)

    def trim_lod(self, logical, mdl, referrer):
        """(mdl, vvd, vtx) cut to one LOD, or None (no VTX/VVD, or not safely rewritable)."""
        stem = logical[:-4]
        vvd = self.content.read(stem + ".vvd")[0]
        vtx = self.content.read(stem + ".dx90.vtx")[0]
        if vvd is None or vtx is None:
            return None
        try:
            return model_lod.trim_model(mdl, vvd, vtx, self.lod)
        except (model_lod.LodError, struct.error) as error:
            self.notes.append("%s: LOD trim skipped, copied whole (%s)" % (logical, error))
            return None

    def model_material(self, texture, cdmaterials, referrer):
        """A model texture name resolves against each $cdmaterials directory."""
        for directory in cdmaterials or [""]:
            name = norm("%s/%s" % (directory, texture))
            if self.content.exists("materials/%s.vmt" % name):
                self.material(name, referrer)
                return
        self.missing.setdefault("materials/%s/%s.vmt (searched %d cdmaterials)" % (
            cdmaterials[0] if cdmaterials else "", texture, len(cdmaterials)), referrer)

    # -- boot set -------------------------------------------------------
    def boot(self, extra):
        names = sorted(self.content.names())
        for pattern in list(BOOT_FILES) + list(extra):
            matched = [n for n in names if fnmatch.fnmatchcase(n, pattern)
                       and not is_other_language(n)]
            if not matched:
                self.notes.append("boot pattern matches nothing: " + pattern)
            for name in matched:
                if name.endswith(".vmt"):
                    self.material(name, "boot")
                elif name.endswith(".mdl"):
                    self.model(name, "boot")
                elif name.endswith(".vtf"):
                    self.pending_textures.setdefault(name, "boot")
                else:
                    self.copy(name, "boot")


BITMAP_FONT_NAME = "n3ds_small"
VPK_NAME = "pak01_dir.vpk"
VPK_MARKER, VPK_VERSION, VPK_EMBEDDED = 0x55AA1234, 2, 0x7FFF
VPK_ARCHIVE_LIMIT = 32 * 1024 * 1024
# Stays loose: the engine opens the BSP itself, writes under cfg/, and reads gameinfo.txt
# before any search path exists.
LOOSE_PATTERNS = ("gameinfo.txt", "maps/*.bsp", "cfg/*")


def write_gameinfo(source_text, destination, title_suffix=" (3DS)", with_vpk=False):
    """The source gameinfo with its SearchPaths replaced by this directory alone.

    The engine does not mount pak01_dir.vpk by itself (AddPackFiles looks only for
    zipN.zip), so a packed tree lists it, ahead of the directory, as the retail
    gameinfo does."""
    vpk = ""
    if with_vpk:
        vpk = ('\t\t\tgame+mod\t\t\t\t|gameinfo_path|%s\n'
               '\t\t\tplatform\t\t\t|gameinfo_path|%s\n' % (VPK_NAME, VPK_NAME))
    paths = ('\t\tSearchPaths\n\t\t{\n' + vpk +
             '\t\t\tgame+mod+mod_write+game_write+default_write_path\t|gameinfo_path|.\n'
             '\t\t\tgamebin\t\t\t\t|gameinfo_path|bin\n'
             '\t\t\tplatform\t\t\t|gameinfo_path|.\n\t\t}\n')
    text, count = re.subn(r"[ \t]*SearchPaths\s*\{[^}]*\}\n", lambda _m: paths, source_text,
                          flags=re.S)
    if not count:
        raise ValueError("gameinfo.txt has no SearchPaths block")
    text = re.sub(r'(?m)^(\s*title\s+"[^"]*)"', lambda m: m.group(1) + title_suffix + '"', text,
                  count=1)
    destination.write_text(text)


def build_vpk(files, limit=VPK_ARCHIVE_LIMIT):
    """(directory bytes, [archive bytes]) from {logical path: data}.

    Layout as vpklib/packedstore.cpp reads it: pak01_dir.vpk is a 28-byte
    VPKDirHeader_t (marker, version 2, tree size, embedded size 0, three zero
    hash/signature sizes) and the tree (extension / directory / file name,
    NUL-terminated; " " for none). The engine reads that whole file into memory,
    so file data lives in numbered archives (pak01_000.vpk, ...): an entry holds
    the archive number and the file's absolute offset in that archive. A file is
    never split; an archive closes before it would pass `limit`."""
    tree = {}
    for logical in sorted(files):
        directory, _slash, name = logical.rpartition("/")
        base, dot, ext = name.rpartition(".")
        if not dot:
            base, ext = name, " "
        tree.setdefault(ext, {}).setdefault(directory or " ", []).append((base, logical))
    index, archives = bytearray(), [bytearray()]
    for ext in sorted(tree):
        index += ext.encode("latin-1") + b"\0"
        for directory in sorted(tree[ext]):
            index += directory.encode("latin-1") + b"\0"
            for base, logical in tree[ext][directory]:
                blob = files[logical]
                if archives[-1] and len(archives[-1]) + len(blob) > limit:
                    archives.append(bytearray())
                if len(archives) > 0x7FFE:
                    raise ValueError("too many VPK archives")
                index += base.encode("latin-1") + b"\0"
                index += struct.pack("<IHHIIH", zlib.crc32(blob) & 0xFFFFFFFF, 0,
                                     len(archives) - 1, len(archives[-1]), len(blob), 0xFFFF)
                archives[-1] += blob
            index += b"\0"
        index += b"\0"
    index += b"\0"
    header = struct.pack("<7I", VPK_MARKER, VPK_VERSION, len(index), 0, 0, 0, 0)
    return header + bytes(index), [bytes(a) for a in archives if a]


def archive_name(number):
    return "pak01_%03d.vpk" % number


def verify_vpk(path, files):
    """Read the VPK back with source_content.VpkDirectory and compare every entry."""
    import source_content
    directory = source_content.VpkDirectory(str(path))
    # VpkDirectory spells an extension-less file "name. "; the file is "name".
    keys = {key[:-2] if key.endswith(". ") else key: key for key in directory.entries}
    if set(keys) != set(files):
        raise ValueError("VPK entries differ from the packed files: %s" % sorted(
            set(keys) ^ set(files))[:5])
    if directory.entries and len(directory.dir_data) != directory.data_base:
        raise ValueError("pak01_dir.vpk holds data beyond its tree")
    for logical, blob in files.items():
        if directory.entries[keys[logical]][0] == VPK_EMBEDDED:
            raise ValueError("VPK entry %s is embedded in the _dir file" % logical)
        if directory.read(keys[logical]) != blob:
            raise ValueError("VPK entry %s differs from its source" % logical)


def pack_vpk(out_mod, limit=VPK_ARCHIVE_LIMIT):
    """Move every loose file except LOOSE_PATTERNS into pak01_dir.vpk and its numbered
    archives; returns the packed entries."""
    out_mod = Path(out_mod)
    files = {}
    for path in sorted(out_mod.rglob("*")):
        if not path.is_file():
            continue
        logical = path.relative_to(out_mod).as_posix()
        if logical == VPK_NAME or re.fullmatch(r"pak01_\d{3}\.vpk", logical) or any(
                fnmatch.fnmatchcase(logical, pattern) for pattern in LOOSE_PATTERNS):
            continue
        if logical != logical.lower():
            raise ValueError("%s is not lower case; VPK lookups fold case" % logical)
        files[logical] = path.read_bytes()
    destination = out_mod / VPK_NAME
    directory, archives = build_vpk(files, limit)
    destination.write_bytes(directory)
    for number, data in enumerate(archives):
        (out_mod / archive_name(number)).write_bytes(data)
    verify_vpk(destination, files)
    for logical in files:
        (out_mod / logical).unlink()
    for path in sorted((p for p in out_mod.rglob("*") if p.is_dir()), reverse=True):
        if not any(path.iterdir()):
            path.rmdir()
    return files


def stage(bsp_path, runtime, out, max_size=DEFAULT_MAX_SIZE, mod=DEFAULT_MOD, out_format="auto",
          extra_boot=(), with_boot=True, keep_ani=False, keep_pak=False, model_lod_index=0,
          use_vpk=True, bitmap_font=True):
    """Run the closure walk and write the content tree; returns the report dict."""
    bsp_path = Path(bsp_path)
    runtime = Path(runtime)
    if not bsp_path.is_file():
        candidate = runtime / mod / "maps" / (bsp_path.name + ".bsp" if bsp_path.suffix != ".bsp"
                                              else bsp_path.name)
        bsp_path = candidate if candidate.is_file() else bsp_path
    bsp = read_bsp(bsp_path)
    pak = bsp.pakfile()
    content = Content(runtime, pak)
    out_mod = Path(out) / mod
    stager = Stager(content, out_mod, max_size, out_format, keep_ani, model_lod_index)
    map_name = bsp_path.stem
    materials, models, vmts, sky, vscripts = bsp_references(bsp)

    bsp_bytes = bsp_path.read_bytes()
    pak_bytes = len(bsp.lump(LUMP_PAKFILE)) if pak else 0
    for extra in ("maps/%s.txt" % map_name, "maps/%s.nav" % map_name):
        if content.exists(extra):
            stager.copy(extra, "map")

    for name in sorted(materials):
        stager.material(name, "bsp texdata")
    for name in sorted(vmts):
        stager.vmt(name, "bsp entity")
    if sky:
        for side in SKY_SIDES:
            stager.material("skybox/" + sky + side, "bsp skyname")
    for model in sorted(models):
        stager.model(model, "bsp model")
    for script in sorted(vscripts):
        stager.copy("scripts/vscripts/" + script, "bsp entity vscripts")
    if with_boot:
        stager.boot(extra_boot)
    stager.textures()
    if bitmap_font:
        # The 3DS's own tiny bitmap font (make_bitmap_font.py); the VTF is already page-sized.
        for logical, data in make_bitmap_font.build_font(BITMAP_FONT_NAME).items():
            stager.put(logical, data, 0, "bitmap-font", "make_bitmap_font")
        # Point every scheme font at it: the 3DS has no TrueType rasterizer budget.
        for logical in sorted(stager.written):
            if not (logical.startswith("resource/") and logical.endswith(".res")):
                continue
            before = (stager.out / logical).read_bytes()
            hooked = make_bitmap_font.hook_scheme(before.decode("latin-1"), BITMAP_FONT_NAME)
            if hooked is not None:
                stager.put(logical, hooked.encode("latin-1"), len(before), "scheme-bitmap-font",
                           stager.written[logical]["origin"])

    # The BSP goes last: its pak lump is rewritten to the files the closure used.
    if keep_pak or not pak:
        new_bsp = bsp_bytes
    else:
        new_pak = build_pak(stager.pak_out) if stager.pak_out else b""
        new_bsp = rewrite_bsp(bsp_bytes, new_pak)
        verify_bsp(new_bsp, bsp_bytes, stager.pak_out)
    stager.put("maps/%s.bsp" % map_name, new_bsp, len(bsp_bytes), "bsp", str(bsp_path))
    dropped_pak = sorted(set(content.pak) - set(stager.pak_out))

    gameinfo_path = runtime / mod / "gameinfo.txt"
    gameinfo = gameinfo_path.read_bytes() if gameinfo_path.is_file() else \
        content.resolver.read("gameinfo.txt")[0]
    if gameinfo is None:
        raise FileNotFoundError("no gameinfo.txt under %s" % runtime)
    out_mod.mkdir(parents=True, exist_ok=True)
    write_gameinfo(gameinfo.decode("utf-8", "replace").replace("\r\n", "\n"),
                   out_mod / "gameinfo.txt", with_vpk=use_vpk)
    stager.written.pop("gameinfo.txt", None)

    files = [(logical, entry) for logical, entry in stager.written.items()]
    loose = [(p, p.stat().st_size) for p in out_mod.rglob("*") if p.is_file()]
    packed = pack_vpk(out_mod) if use_vpk else {}
    on_disk = [(p, p.stat().st_size) for p in out_mod.rglob("*") if p.is_file()]
    report = {
        "map": map_name, "mod_dir": str(out_mod), "max_size": max_size,
        "file_count": len(on_disk),
        "loose_files_before_packing": len(loose),
        "vpk_entries": len(packed),
        "vpk_bytes": (out_mod / VPK_NAME).stat().st_size if packed else 0,
        "vpk_archives": {p.name: size for p, size in sorted(on_disk)
                         if re.fullmatch(r"pak01_\d{3}\.vpk", p.name)},
        "loose_files": sorted(p.relative_to(out_mod).as_posix() for p, _s in on_disk),
        "bytes_before": sum(e["before"] for _l, e in files if not e["kind"].startswith("pak-")),
        "bytes_after": sum(size for _p, size in on_disk),
        "bsp_pak_bytes": pak_bytes,
        "in_bsp_pak": sorted(stager.pak_out),
        "bsp_pak_bytes_after": sum(len(v) for v in stager.pak_out.values()),
        "dropped_pak_files": dropped_pak,
        "editor_models_skipped": sorted(stager.skipped_editor),
        "largest": [(str(p.relative_to(out_mod)), size)
                    for p, size in sorted(loose, key=lambda item: -item[1])[:12]],
        "by_kind": {},
        "missing": dict(sorted(stager.missing.items())),
        "notes": stager.notes + BOOT_NOTES,
        "counts": {"materials": len(stager.seen_materials), "models": len(stager.seen_models),
                   "textures": sum(1 for e in stager.written.values()
                                   if e["kind"] in ("vtf", "vtf-passthrough")),
                   "world_materials": len(materials), "static_prop_and_entity_models": len(models)},
    }
    for logical, entry in files:
        kind = report["by_kind"].setdefault(entry["kind"], {"files": 0, "before": 0, "after": 0})
        kind["files"] += 1
        kind["before"] += entry["before"]
        kind["after"] += entry["after"]
    (out_mod.parent / "stage_report.json").write_text(json.dumps(report, indent=1, default=str))
    return report


def format_report(report):
    lines = ["map %s -> %s" % (report["map"], report["mod_dir"]),
             "files on disk: %d (%d before packing)" % (report["file_count"],
                                                        report["loose_files_before_packing"]),
             "VPK: %d entries; %s %d bytes (tree only); %d archives, %d bytes" % (
                 report["vpk_entries"], VPK_NAME, report["vpk_bytes"], len(report["vpk_archives"]),
                 sum(report["vpk_archives"].values())),
             "bytes before: %d (%.1f MB)" % (report["bytes_before"], report["bytes_before"] / 1e6),
             "bytes after:  %d (%.1f MB)" % (report["bytes_after"], report["bytes_after"] / 1e6),
             "BSP pak lump: %d bytes before; %d files (%d bytes) kept, %d dropped" % (
                 report["bsp_pak_bytes"], len(report["in_bsp_pak"]),
                 report["bsp_pak_bytes_after"], len(report["dropped_pak_files"])),
             "editor-only models skipped: %d" % len(report["editor_models_skipped"]),
             "closure: " + ", ".join("%s %d" % item for item in report["counts"].items()),
             "by kind:"]
    for kind, entry in sorted(report["by_kind"].items()):
        lines.append("  %-16s %5d files  %12d -> %12d" % (kind, entry["files"], entry["before"],
                                                           entry["after"]))
    lines.append("largest remaining files:")
    lines += ["  %10d  %s" % (size, name) for name, size in report["largest"]]
    lines.append("missing assets: %d" % len(report["missing"]))
    for name, referrer in list(report["missing"].items())[:60]:
        lines.append("  %s  (from %s)" % (name, referrer))
    if len(report["missing"]) > 60:
        lines.append("  ... %d more in stage_report.json" % (len(report["missing"]) - 60))
    for note in report["notes"][:30]:
        lines.append("note: " + note)
    return "\n".join(lines)


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--bsp", required=True,
                        help="BSP path, or a map name found under <runtime>/<mod>/maps")
    parser.add_argument("--runtime", type=Path, default=ROOT / "run/runtime-p2")
    parser.add_argument("--out", type=Path, required=True,
                        help="output root; the content goes in <out>/<mod>")
    parser.add_argument("--mod", default=DEFAULT_MOD)
    parser.add_argument("--max-size", type=int, default=DEFAULT_MAX_SIZE)
    parser.add_argument("--format", choices=("auto", "dxt", "rgba8888"), default="auto",
                        help="texture encoding (auto: DXT1, or DXT5 when alpha is used)")
    parser.add_argument("--boot-extra", action="append", default=[],
                        help="extra fnmatch pattern for the boot set (repeatable)")
    parser.add_argument("--keep-ani", action="store_true",
                        help="copy .ani animation blocks (dropped by default)")
    parser.add_argument("--model-lod", type=int, default=0, metavar="N",
                        help="keep only LOD N of every model (default 0, the most detailed; "
                             "a higher N uses distant-quality meshes and shrinks the VVD too)")
    parser.add_argument("--no-lod-trim", action="store_true",
                        help="copy .vtx/.vvd/.mdl whole")
    parser.add_argument("--keep-pak", action="store_true",
                        help="leave the BSP pak lump as is (default: keep only closure files)")
    parser.add_argument("--vpk", action=argparse.BooleanOptionalAction, default=True,
                        help="pack everything except gameinfo.txt, maps/*.bsp and cfg/ into "
                             "<mod>/pak01_dir.vpk (default; --no-vpk leaves loose files)")
    parser.add_argument("--no-bitmap-font", action="store_true",
                        help="do not add resource/n3ds_small.vbf and its page texture, nor point the "
                             "schemes' fonts at it")
    parser.add_argument("--no-boot", action="store_true", help="closure only, no boot set")
    args = parser.parse_args(argv)
    if args.max_size < 1 or args.max_size & (args.max_size - 1):
        parser.error("--max-size must be a power of two")
    report = stage(args.bsp, args.runtime, args.out, args.max_size, args.mod, args.format,
                   args.boot_extra, not args.no_boot, args.keep_ani, args.keep_pak,
                   None if args.no_lod_trim else args.model_lod, args.vpk,
                   not args.no_bitmap_font)
    stage_platform_fonts(args.runtime, args.out)
    print(format_report(report))
    return 0


# Fonts VGUI's Linux font manager loads by literal path,
# "platform/resource/linux_fonts/<name>.ttf", relative to every search path.
# Missing, it re-probes them on every font request (hundreds of slow SD card
# opens at startup).
PLATFORM_FONTS = "platform/resource/linux_fonts"


def stage_platform_fonts(runtime, out):
    source = Path(runtime) / PLATFORM_FONTS
    target = Path(out) / PLATFORM_FONTS
    if not source.is_dir():
        print("note: no %s in the runtime; VGUI fonts will be missing" % source)
        return
    target.mkdir(parents=True, exist_ok=True)
    # Lowercase: the font manager asks for lowercase names, and an emulator's
    # SD card (Azahar maps it to the host's file system) is case-sensitive.
    for font in sorted(source.glob("*.ttf")):
        (target / font.name.lower()).write_bytes(font.read_bytes())


if __name__ == "__main__":
    sys.exit(main())
