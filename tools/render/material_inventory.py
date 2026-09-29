#!/usr/bin/env python3
"""Quantitative inventory of Source material (VMT) shader-feature usage.

For the RFC 0016 "one general material model" plan: which shaders, keys,
values, key sets and proxies the Portal and Portal 2 content uses, and how
fast a sequence of general-model terms covers it.

    python3 tools/render/material_inventory.py --out DIR [--refresh] [--phases]

--phases also prints the cumulative coverage after each phase of the RFC 0016
surface-model plan (PHASES below): materials, world area and world faces.

Phase 1 loads every VMT of each game's search path (reusing the VPK reader
and search order of tools/render/vmt_corpus.py / tools/quality/source_content.py),
the pak lumps of the maps, and the world-face areas of selected maps, and
caches the parsed materials (materials.pickle). Phase 2 analyzes them and
writes inventory.json.

Material semantics follow render/material/vmt_import.cpp (itself
CMaterial::ParseMaterialVars): patch include/insert/replace, the first
fallback block the dx95/ps20b/HDR/sRGB/GPU-level-3 profile selects, "cond?$key"
variables, [$TAG] conditionals with WIN32/LINUX/POSIX held, and shader aliases
from render/material/legacy_shaders.inc (DEFINE_FALLBACK_SHADER).
"""

import argparse
import collections
import io
import json
import lzma
import os
import pickle
import re
import struct
import sys
import zipfile
from pathlib import Path

REPO = Path(os.environ.get("SOURCE_ENGINE_ROOT", Path(__file__).resolve().parents[2]))
sys.path.insert(0, str(REPO / "tools" / "quality"))
sys.path.insert(0, str(REPO / "tools" / "render"))
from source_content import VpkDirectory  # noqa: E402
import vmt_corpus  # noqa: E402  (search_paths, Game)

SYMBOLS = {"win32", "linux", "posix"}
PROFILE = dict(dx=95, ps20b=True, hdr=True, srgb=True, gpu=3)

# ---------------------------------------------------------------------------
# KeyValues


class Node:
    __slots__ = ("name", "pairs", "children")

    def __init__(self, name):
        self.name = name
        self.pairs = []      # [(key, value)]
        self.children = []   # [Node]

    def block(self, name):
        lname = name.lower()
        for c in self.children:
            if c.name.lower() == lname:
                return c
        return None

    def pair(self, key):
        lk = key.lower()
        for k, v in self.pairs:
            if k.lower() == lk:
                return v
        return None


def tokenize(text):
    """[(kind, text)] kinds: 's' string, '{', '}', 'c' conditional."""
    out = []
    i, n = 0, len(text)
    while i < n:
        ch = text[i]
        if ch in " \t\r\n﻿":
            i += 1
        elif ch == "/" and text.startswith("//", i):
            j = text.find("\n", i)
            i = n if j < 0 else j + 1
        elif ch == "{" or ch == "}":
            out.append((ch, ch))
            i += 1
        elif ch == '"':
            j = text.find('"', i + 1)
            if j < 0:
                j = n
            out.append(("s", text[i + 1:j]))
            i = j + 1
        elif ch == "[":
            j = text.find("]", i)
            if j < 0:
                j = n
            out.append(("c", text[i + 1:j]))
            i = j + 1
        else:
            j = i
            while j < n and text[j] not in " \t\r\n\"{}":
                if text.startswith("//", j):
                    break
                j += 1
            out.append(("s", text[i:j]))
            i = j
    return out


def eval_condition(expr):
    """[$WIN32 && !$X360] style; unknown symbols are false."""
    toks = re.findall(r"\$?\w+|&&|\|\||!|\(|\)", expr)
    pos = 0

    def primary():
        nonlocal pos
        if pos >= len(toks):
            return False
        t = toks[pos]
        pos += 1
        if t == "!":
            return not primary()
        if t == "(":
            v = orexpr()
            if pos < len(toks) and toks[pos] == ")":
                pos += 1
            return v
        return t.lstrip("$").lower() in SYMBOLS

    def andexpr():
        nonlocal pos
        v = primary()
        while pos < len(toks) and toks[pos] == "&&":
            pos += 1
            v = primary() and v
        return v

    def orexpr():
        nonlocal pos
        v = andexpr()
        while pos < len(toks) and toks[pos] == "||":
            pos += 1
            v = andexpr() or v
        return v

    return orexpr()


def parse_kv(text):
    """Root node; conditionals evaluated and dropped; tolerant of a missing '}'."""
    toks = tokenize(text)
    pos = 0

    def body(node, top):
        nonlocal pos
        while pos < len(toks):
            kind, val = toks[pos]
            if kind == "}":
                pos += 1
                return
            if kind == "{":  # stray block without a name
                pos += 1
                child = Node("")
                body(child, False)
                node.children.append(child)
                continue
            if kind == "c":
                pos += 1
                continue
            key = val
            pos += 1
            cond = True
            if pos < len(toks) and toks[pos][0] == "c":
                cond = eval_condition(toks[pos][1])
                pos += 1
            if pos >= len(toks):
                return
            kind2, val2 = toks[pos]
            if kind2 == "{":
                pos += 1
                child = Node(key)
                body(child, False)
                if pos < len(toks) and toks[pos][0] == "c":
                    cond = cond and eval_condition(toks[pos][1])
                    pos += 1
                if cond:
                    node.children.append(child)
            elif kind2 == "}":
                continue
            else:
                value = val2
                if kind2 == "c":  # unquoted [1 1 1] read as a vector value
                    value = "[" + val2 + "]"
                pos += 1
                if pos < len(toks) and toks[pos][0] == "c":
                    cond = cond and eval_condition(toks[pos][1])
                    pos += 1
                if cond:
                    node.pairs.append((key, value))

    root = Node("")
    body(root, True)
    return root


def decode(data):
    for enc in ("utf-8", "latin-1"):
        try:
            return data.decode(enc)
        except UnicodeDecodeError:
            continue
    return data.decode("latin-1", "replace")


# ---------------------------------------------------------------------------
# Materials


def gpu_holds(level):
    m = re.fullmatch(r"(?i)gpu(>=|<)(\d)", level)
    if not m:
        return None
    return PROFILE["gpu"] >= int(m.group(2)) if m.group(1) == ">=" else PROFILE["gpu"] < int(m.group(2))


def skip_variable(cond):
    toggle = cond.startswith("!")
    c = cond[1:] if toggle else cond
    c = c.lower()
    if c == "lowfill":
        skip = True
    elif c == "hdr":
        skip = False
    elif c in ("srgb", "srgb_pc"):
        skip = not PROFILE["srgb"]
    elif c == "ldr":
        skip = PROFILE["hdr"]
    elif c in ("sonyps3", "360", "gameconsole"):
        skip = True
    else:
        g = gpu_holds(c)
        skip = True if g is None else not g
    return skip != toggle


def fallback_block(block, shader):
    def find(suffix):
        return block.block(suffix) or block.block(shader + "_" + suffix)
    for level in ("GPU<1", "GPU<2", "GPU>=1", "GPU>=2"):
        if gpu_holds(level):
            f = find(level)
            if f:
                return f
    dx, ps = PROFILE["dx"], PROFILE["ps20b"]
    levels = [(dx < 90, "<DX90"), (dx < 95, "<DX95"), (dx < 90 or not ps, "<DX90_20b"),
              (dx >= 90 and ps, ">=DX90_20b"), (dx <= 90, "<=DX90"), (dx >= 90, ">=DX90"),
              (dx > 90, ">DX90"), (True, "hdr_dx9"), (True, "hdr"), (not PROFILE["hdr"], "ldr"),
              (PROFILE["srgb"], "srgb"), (dx >= 90, "dx9")]
    for holds, suffix in levels:
        if holds:
            f = find(suffix)
            if f:
                return f
    return None


def first_block(text):
    root = parse_kv(text)
    for c in root.children:
        return c
    return None


def merge_into(src, dest):
    for k, v in src.pairs:
        set_value(dest, k, v)
    for c in src.children:
        d = dest.block(c.name)
        if d is None:
            d = Node(c.name)
            dest.children.append(d)
        merge_into(c, d)


def set_value(node, key, value):
    lk = key.lower()
    for i, (k, _) in enumerate(node.pairs):
        if k.lower() == lk:
            node.pairs[i] = (k, value)
            return
    node.pairs.append((key, value))


def insert_into(dest, src, only_existing):
    for k, v in src.pairs:
        if not only_existing or dest.pair(k) is not None or dest.block(k) is not None:
            set_value(dest, k, v)
    for c in src.children:
        if not only_existing or dest.pair(c.name) is not None or dest.block(c.name) is not None:
            d = dest.block(c.name)
            if d is None:
                d = Node(c.name)
                dest.children.append(d)
            insert_into(d, c, only_existing)


def norm_path(p):
    p = p.replace("\\", "/").strip().lower().lstrip("/")
    while "//" in p:
        p = p.replace("//", "/")
    return p


def load_material(path, read):
    """dict describing the material, or {'error': ...}."""
    data = read(path)
    if data is None:
        return {"error": "missing"}
    block = first_block(decode(data))
    if block is None:
        return {"error": "no material block"}
    includes = []
    insert, replace = Node("insert"), Node("replace")
    depth = 0
    while block.name.lower() == "patch" and depth < 10:
        s = block.block("insert")
        if s:
            merge_into(s, insert)
        s = block.block("replace")
        if s:
            merge_into(s, replace)
        inc = block.pair("include")
        if not inc or not inc.strip():
            return {"error": "patch without include"}
        ip = norm_path(inc)
        includes.append(ip)
        d = read(ip)
        if d is None:
            return {"error": "missing include " + ip, "includes": includes}
        block = first_block(decode(d))
        if block is None:
            return {"error": "include without block", "includes": includes}
        depth += 1
    if block.name.lower() == "patch":
        return {"error": "patch depth"}
    insert_into(block, insert, False)
    insert_into(block, replace, True)
    shader_raw = block.name
    fb = fallback_block(block, shader_raw)
    variables = collections.OrderedDict()
    conditional = {}
    editor = {}

    def visit(k, v):
        q = k.find("?")
        is_cond = q > 0
        if is_cond:
            if skip_variable(k[:q]):
                return
            k = k[q + 1:]
        if k.startswith("%"):
            editor[k.lower()] = v
            return
        lk = k.strip().lower()
        if lk in variables:
            if is_cond:
                variables[lk] = v
                conditional[lk] = True
            return
        variables[lk] = v
        conditional[lk] = is_cond

    if fb is not None:
        for k, v in fb.pairs:
            visit(k, v)
    for k, v in block.pairs:
        visit(k, v)
    proxies = []
    pblock = block.block("proxies") or (fb.block("proxies") if fb is not None else None)
    if pblock is not None:
        for c in pblock.children:
            proxies.append((c.name.lower(), {k.lower(): v for k, v in c.pairs}))
    other_blocks = [c.name for c in block.children
                    if c.name.lower() not in ("proxies",) and c is not fb]
    return {"shader_raw": shader_raw, "vars": dict(variables), "proxies": proxies,
            "editor": editor, "includes": includes,
            "fallback": fb.name if fb is not None else None, "other_blocks": other_blocks}


# ---------------------------------------------------------------------------
# BSP (v20 Portal, v21 Portal 2): pak lump and world-face areas

LUMP_TEXDATA, LUMP_TEXINFO, LUMP_FACES, LUMP_PAK = 2, 6, 7, 40
LUMP_STRDATA, LUMP_STRTABLE = 43, 44
SURF_SKY2D, SURF_SKY, SURF_NODRAW, SURF_TRIGGER, SURF_HINT, SURF_SKIP = 0x2, 0x4, 0x80, 0x40, 0x100, 0x200


def lzma_lump(data):
    if data[:4] != b"LZMA":
        return data
    actual, lsize = struct.unpack_from("<II", data, 4)
    props = data[12:17]
    d = props[0]
    lc, lp, pb = d % 9, (d // 9) % 5, (d // 45)
    dict_size = struct.unpack_from("<I", props, 1)[0]
    dec = lzma.LZMADecompressor(format=lzma.FORMAT_RAW, filters=[
        {"id": lzma.FILTER_LZMA1, "dict_size": dict_size, "lc": lc, "lp": lp, "pb": pb}])
    return dec.decompress(data[17:17 + lsize])[:actual]


def bsp_lumps(path):
    data = Path(path).read_bytes()
    ident, version = struct.unpack_from("<4sI", data, 0)
    if ident != b"VBSP":
        return None, None
    lumps = []
    for i in range(64):
        a, b, c, d = struct.unpack_from("<iiii", data, 8 + 16 * i)
        lumps.append((a, b, c, d))

    def get(i):
        ofs, length, _, _ = lumps[i]
        if ofs <= 0 or length <= 0 or ofs + length > len(data):
            return b""
        return lzma_lump(data[ofs:ofs + length])
    return version, get


def bsp_pak(path):
    version, get = bsp_lumps(path)
    if get is None:
        return {}
    raw = get(LUMP_PAK)
    out = {}
    if not raw:
        return out
    try:
        z = zipfile.ZipFile(io.BytesIO(raw))
        for info in z.infolist():
            name = norm_path(info.filename)
            try:
                out[name] = z.read(info)
            except Exception:
                pass
    except Exception:
        pass
    return out


def bsp_face_areas(path):
    """{material name (lower): area} of drawn world faces, plus skipped totals."""
    version, get = bsp_lumps(path)
    faces, texinfo, texdata = get(LUMP_FACES), get(LUMP_TEXINFO), get(LUMP_TEXDATA)
    strdata, strtable = get(LUMP_STRDATA), get(LUMP_STRTABLE)
    names = []
    for i in range(len(strtable) // 4):
        ofs = struct.unpack_from("<i", strtable, 4 * i)[0]
        end = strdata.index(b"\0", ofs)
        names.append(strdata[ofs:end].decode("latin-1").lower().replace("\\", "/"))
    tdnames = []
    for i in range(len(texdata) // 32):
        sid = struct.unpack_from("<i", texdata, 32 * i + 12)[0]
        tdnames.append(names[sid] if 0 <= sid < len(names) else "?")
    areas = collections.Counter()
    counts = collections.Counter()
    skipped = collections.Counter()
    for i in range(len(faces) // 56):
        ti = struct.unpack_from("<h", faces, 56 * i + 10)[0]
        area = struct.unpack_from("<f", faces, 56 * i + 24)[0]
        if ti < 0 or ti * 72 + 72 > len(texinfo):
            continue
        flags, td = struct.unpack_from("<ii", texinfo, 72 * ti + 64)
        name = tdnames[td] if 0 <= td < len(tdnames) else "?"
        if flags & (SURF_NODRAW | SURF_SKIP | SURF_HINT | SURF_TRIGGER):
            skipped["nodraw"] += area
            continue
        if flags & (SURF_SKY | SURF_SKY2D):
            skipped["sky"] += area
            continue
        areas[name] += area
        counts[name] += 1
    return {"version": version, "areas": dict(areas), "faces": dict(counts), "skipped": dict(skipped)}


# ---------------------------------------------------------------------------
# Phase 1


def game_maps(game):
    if game == "portal":
        base = vmt_corpus.portal_runtime() / "portal" / "maps"
        return sorted(base.glob("*.bsp"))
    base = vmt_corpus.portal2_root()
    maps = []
    for folder in ("portal2", "portal2_dlc1", "portal2_dlc2", "update"):
        maps += sorted((base / folder / "maps").glob("*.bsp"))
    return maps


WEIGHT_MAPS = {
    "portal": ["testchmb_a_%02d" % i for i in range(12)],
    "portal2": None,  # every sp_a* single-player map (filled in load())
}


def load(cache):
    if cache.is_file():
        with open(cache, "rb") as f:
            return pickle.load(f)
    result = {}
    for game in ("portal", "portal2"):
        g = vmt_corpus.Game(game)
        cache_read = {}

        def read(p, g=g, cache_read=cache_read):
            p = norm_path(p)
            if p not in cache_read:
                cache_read[p] = g.read(p)
            return cache_read[p]
        mats = {}
        for p in g.vmts():
            mats[p] = load_material(p, read)
        # Pak lumps and world faces.
        pak_mats = {}
        weights = {}
        maps = game_maps(game)
        wanted = WEIGHT_MAPS[game]
        for mp in maps:
            name = mp.stem.lower()
            pak = bsp_pak(mp)
            pak_vmts = {k: v for k, v in pak.items() if k.endswith(".vmt")}

            def pread(p, pak=pak):
                p = norm_path(p)
                return pak[p] if p in pak else read(p)
            for p in pak_vmts:
                pak_mats[name + ":" + p] = load_material(p, pread)
            if (wanted is None and name.startswith("sp_a")) or (wanted and name in wanted):
                fa = bsp_face_areas(mp)
                # Resolve each face material through the pak lump (patched cubemap variants).
                resolved = {}
                for m in fa["areas"]:
                    p = "materials/" + m + ".vmt"
                    if p in pak:
                        resolved[m] = ("pak", load_material(p, pread))
                    elif read(p) is not None:
                        resolved[m] = ("game", p)
                    else:
                        resolved[m] = ("missing", None)
                weights[name] = {"faces": fa, "resolved": resolved}
        result[game] = {"materials": mats, "pak": pak_mats, "weights": weights,
                        "layers": [str(p) for _, p, _ in g.layers]}
        print("%s: %d VMTs, %d pak VMTs from %d maps, %d weighted maps" % (
            game, len(mats), len(pak_mats), len(maps), len(weights)), file=sys.stderr)
    with open(cache, "wb") as f:
        pickle.dump(result, f)
    return result




# ---------------------------------------------------------------------------
# Shader facts from the product source


def shader_aliases():
    """{name: fallback} from render/material/legacy_shaders.inc."""
    text = (REPO / "render/material/legacy_shaders.inc").read_text()
    return {a: b for a, b in re.findall(r'\{\s*"([^"]+)",\s*"([^"]*)"\s*\}', text)}


def declared_params():
    """{shader (lower): {param}} from the stdshader_dx9 BEGIN_SHADER_PARAMS blocks."""
    decl = collections.defaultdict(set)
    for f in (REPO / "materialsystem/stdshaders").glob("*.cpp"):
        t = f.read_text(errors="replace")
        for m in re.finditer(r"BEGIN_(?:VS_)?SHADER(?:_FLAGS)?\s*\(\s*(\w+)(.*?)END_SHADER_PARAMS", t, re.S):
            decl[m.group(1).lower()] |= {"$" + p.lower() for p in re.findall(r"SHADER_PARAM\s*\(\s*(\w+)", m.group(2))}
    return decl


STANDARD = {"$flags", "$flags_defined", "$flags2", "$flags_defined2", "$color", "$alpha", "$basetexture",
            "$frame", "$basetexturetransform", "$flashlighttexture", "$flashlighttextureframe", "$color2",
            "$srgbtint"}
FLAGS = {"$debug", "$no_fullbright", "$no_draw", "$use_in_fillrate_mode", "$vertexcolor", "$vertexalpha",
         "$selfillum", "$additive", "$alphatest", "$multipass", "$znearer", "$model", "$flat", "$nocull",
         "$nofog", "$ignorez", "$decal", "$envmapsphere", "$noalphamod", "$envmapcameraspace",
         "$basealphaenvmapmask", "$translucent", "$normalmapalphaenvmapmask", "$softwareskin",
         "$opaquetexture", "$envmapmode", "$nodecal", "$halflambert", "$wireframe", "$allowalphatocoverage"}

# Canonical shader: the alias chain's end with _dx9/_dx90/_hdr suffixes folded.
DISPLAY = {"sky_hdr_dx9": "sky", "water_dx9_hdr": "water", "water_dx90": "water",
           "spritecard_dx8": "spritecard", "vertexlitgeneric_dx6": "vertexlitgeneric"}


def canonical_shader(raw, aliases):
    s = raw.lower()
    seen = set()
    while s in aliases and aliases[s] and s not in seen:
        seen.add(s)
        s = aliases[s]
    s = DISPLAY.get(s, s)
    s = re.sub(r"_(dx9|dx90|dx8|dx6)$", "", s)
    return s


# ---------------------------------------------------------------------------
# Key classification

# Keys with no effect on shading: other systems' metadata, tool keys, console
# (X360/PS3) keys and the material system's LOD/debug flags.
IGNORED = {
    "$surfaceprop", "$surfaceprop2", "$decalscale", "$reflectivity", "$fallbackmaterial", "$modelmaterial",
    "$nodecal", "$no_fullbright", "$nofullbright", "$detailtype", "$minsize", "$maxsize", "$maxdistance",
    "$farfadeinterval", "$keywords", "$shadersrgbread360", "$x360appchooser", "$nolod", "$model",
    "$use_in_fillrate_mode", "$debug", "$softwareskin", "$lowqualityflashlightshadows", "$decalfadeduration",
    "$decalfadetime", "$surfaceprop_override", "$forceexpensive", "$forcecheap", "$clientshader",
    "$flashlighttexture", "$flashlighttextureframe", "$bottommaterial", "$underwateroverlay", "$abovewater",
    "$multipass", "$flat", "$noalphamod", "$opaquetexture", "$groupbame", "$albedo", "$worldimposter",
    "$nomip", "$nosrgb", "$compress", "$stretch", "$ambientocclusion", "$vertextcolor",
}

# Parent feature keys: a child is dead (no effect) unless its parent is effective.
PARENT = {}
for _k in ("$envmapmask", "$basealphaenvmapmask", "$normalmapalphaenvmapmask", "$envmaptint", "$envmapcontrast",
           "$envmapsaturation", "$fresnelreflection", "$envmapfresnel", "$envmapframe", "$envmapmasktransform",
           "$envmapmaskframe", "$envmaplightscale", "$envmapmaskscale", "$basetexturenoenvmap",
           "$basetexture2noenvmap", "$envmapfresnelminmaxexp", "$envmapmode", "$envmapcameraspace",
           "$envmapsphere", "$alphaenvmapmask", "$basemapalphaenvmapmask", "$envmaplightscaleminmax"):
    PARENT[_k] = "$envmap"
for _k in ("$detailscale", "$detailblendmode", "$detailblendfactor", "$detailtint", "$detailframe",
           "$detailtexturetransform", "$detail_alpha_mask_base_texture"):
    PARENT[_k] = "$detail"
for _k in ("$selfillumtint", "$selfillummask", "$selfillumfresnel", "$selfillum_envmapmask_alpha",
           "$selfillumfresnelminmaxexp", "$selfillummaskscale"):
    PARENT[_k] = "$selfillum"
for _k in ("$phongexponent", "$phongboost", "$phongfresnelranges", "$phongtint", "$phongexponenttexture",
           "$basemapalphaphongmask", "$invertphongmask", "$phongalbedotint", "$phongdisablehalflambert",
           "$rimlight", "$phongwarptexture", "$phongexponentfactor", "$phongalbedoboost"):
    PARENT[_k] = "$phong"
for _k in ("$rimlightexponent", "$rimlightboost", "$rimmask"):
    PARENT[_k] = "$rimlight"
for _k in ("$bumpframe", "$bumptransform", "$ssbump", "$nodiffusebumplighting", "$ssbumpmathfix", "$bumpscale"):
    PARENT[_k] = "$bumpmap"
for _k in ("$frame2", "$basetexturetransform2", "$bumpmap2", "$bumpframe2", "$blendmodulatetexture",
           "$blendmasktransform", "$bumptransform2"):
    PARENT[_k] = "$basetexture2"
for _k in ("$alphatestreference", "$allowalphatocoverage"):
    PARENT[_k] = "$alphatest"
for _k in ("$parallaxmapscale",):
    PARENT[_k] = "$parallaxmap"
for _k in ("$depthblendscale",):
    PARENT[_k] = "$depthblend"
for _k in ("$seamless_base", "$seamless_detail"):
    PARENT[_k] = "$seamless_scale"

WHITE = {"[1 1 1]", "{255 255 255}", "[1.0 1.0 1.0]", "[1 1 1 1]"}
NEUTRAL = {
    "$color": WHITE, "$color2": WHITE, "$envmaptint": WHITE, "$selfillumtint": WHITE, "$detailtint": WHITE,
    "$phongtint": WHITE, "$srgbtint": WHITE, "$alpha": {"1"}, "$envmapcontrast": {"0"},
    "$envmapsaturation": {"1"}, "$fresnelreflection": {"1"}, "$detailblendfactor": {"1"},
    "$frame": {"0"}, "$bumpframe": {"0"}, "$frame2": {"0"}, "$detailframe": {"0"}, "$envmapframe": {"0"},
    "$detailscale": set(),  # any value is meaningful with $detail
    "$seamless_scale": {"0"}, "$phongboost": set(), "$alphatestreference": set(),
}


def numbers(v):
    v = v.strip()
    scale = 1.0 / 255 if v.startswith("{") else 1.0
    out = []
    for x in re.findall(r"[-+]?(?:\d+\.?\d*|\.\d+)(?:[eE][-+]?\d+)?", v):
        try:
            out.append(float(x) * scale)
        except ValueError:
            pass
    return out


def is_neutral(key, value):
    v = value.strip()
    if key in NEUTRAL and NEUTRAL[key]:
        if v.lower() in NEUTRAL[key]:
            return True
        ns = numbers(v)
        target = numbers(next(iter(NEUTRAL[key])))
        if ns and target and all(abs(a - target[min(i, len(target) - 1)]) < 1e-3 for i, a in enumerate(ns)):
            return True
        return False
    if key in FLAGS or key in BOOLISH:
        ns = numbers(v)
        return (not ns and v.lower() in ("", "false", "0")) or (bool(ns) and ns[0] == 0)
    if key in TEXTURE_KEYS:
        return v == ""
    return False


BOOLISH = {"$phong", "$rimlight", "$ssbump", "$depthblend", "$treesway", "$seamless_base", "$seamless_detail",
           "$phongalbedotint", "$invertphongmask", "$basemapalphaphongmask", "$selfillumfresnel",
           "$nodiffusebumplighting", "$rimmask", "$mod2x", "$ssbumpmathfix", "$vertexalphatest",
           "$blendtintbybasealpha", "$linearwrite", "$gammacolorread", "$ambientonly", "$flashlightnolambert",
           "$lightmapwaterfog", "$phongdisablehalflambert", "$basetexturenoenvmap", "$basetexture2noenvmap",
           "$selfillum_envmapmask_alpha", "$distancealpha", "$softedges", "$outline", "$glow",
           "$vertexcolormodulate", "$linearread_basetexture", "$writez", "$reflectonlymarkedentities",
           "$reflectskyboxonly", "$forcerefract", "$forceenvmap", "$vertexfog", "$ignorevertexcolors"}
TEXTURE_KEYS = {"$basetexture", "$basetexture2", "$detail", "$envmap", "$envmapmask", "$bumpmap", "$bumpmap2",
                "$selfillummask", "$phongexponenttexture", "$lightwarptexture", "$blendmodulatetexture",
                "$parallaxmap", "$hdrcompressedtexture", "$hdrbasetexture", "$texture2", "$phongwarptexture",
                "$normalmap", "$ambientoccltexture", "$alphamasktexture"}

# ---------------------------------------------------------------------------
# General-model terms. Each effective key maps to one term; a material is
# covered once every term it needs is supported. Keys absent here map to
# "key:<name>" (an unplanned term, which blocks coverage until added).

BASE_SHADERS = {"lightmappedgeneric": "lightmap", "vertexlitgeneric": "model lighting",
                "unlitgeneric": "unlit", "worldvertextransition": "lightmap",
                "lightmappedgeneric_decal": "lightmap"}

TERM_OF_KEY = {}


def _term(name, *keys):
    for k in keys:
        TERM_OF_KEY[k] = name


_term("base", "$basetexture", "$frame", "$color", "$alpha", "$vertexcolor", "$vertexalpha", "$alphatest",
      "$alphatestreference", "$allowalphatocoverage", "$no_draw", "$srgbtint")
_term("blend: translucent", "$translucent")
_term("blend: additive", "$additive")
_term("render state (nocull/ignorez/nofog/decal/znearer)", "$nocull", "$ignorez", "$nofog", "$decal",
      "$znearer", "$wireframe", "$writez", "$vertexfog")
_term("texture transforms", "$basetexturetransform", "$detailtexturetransform", "$bumptransform",
      "$envmapmasktransform", "$basetexturetransform2", "$blendmasktransform", "$texture2transform")
_term("envmap", "$envmap", "$envmapframe", "$envmapmode", "$envmapcameraspace", "$envmapsphere",
      "$forceenvmap")
_term("envmap mask", "$envmapmask", "$basealphaenvmapmask", "$normalmapalphaenvmapmask", "$envmapmaskframe",
      "$envmapmaskscale", "$basetexturenoenvmap", "$basetexture2noenvmap", "$alphaenvmapmask",
      "$basemapalphaenvmapmask")
_term("envmap tint", "$envmaptint")
_term("envmap contrast/saturation", "$envmapcontrast", "$envmapsaturation")
_term("envmap fresnel", "$fresnelreflection", "$envmapfresnel", "$envmapfresnelminmaxexp")
_term("envmap lightmap scale (P2)", "$envmaplightscale", "$envmaplightscaleminmax")
_term("selfillum", "$selfillum", "$selfillumtint", "$selfillummask", "$selfillum_envmapmask_alpha",
      "$selfillummaskscale")
_term("selfillum fresnel", "$selfillumfresnel", "$selfillumfresnelminmaxexp")
_term("bump (RNM lightmap / per-pixel normal)", "$bumpmap", "$bumpframe", "$nodiffusebumplighting",
      "$bumpscale")
_term("ssbump", "$ssbump", "$ssbumpmathfix")
_term("two-layer blend (WVT $basetexture2)", "$basetexture2", "$frame2", "$bumpmap2", "$bumpframe2",
      "$bumptransform2")
_term("two-layer blend modulate", "$blendmodulatetexture")
_term("triplanar ($seamless)", "$seamless_scale", "$seamless_base", "$seamless_detail")
_term("phong", "$phong", "$phongexponent", "$phongboost", "$phongfresnelranges", "$phongtint",
      "$basemapalphaphongmask", "$invertphongmask", "$phongdisablehalflambert", "$phongexponentfactor")
_term("phong exponent texture / albedo tint", "$phongexponenttexture", "$phongalbedotint", "$phongalbedoboost")
_term("rim light", "$rimlight", "$rimlightexponent", "$rimlightboost", "$rimmask")
_term("light warp", "$lightwarptexture", "$phongwarptexture")
_term("half-lambert / ambient-only", "$halflambert", "$ambientonly")
_term("parallax", "$parallaxmap", "$parallaxmapscale")
_term("tree sway", *["$treesway" + s for s in ("", "height", "startheight", "radius", "startradius", "speed",
                                               "strength", "scrumblespeed", "scrumblestrength",
                                               "scrumblefrequency", "falloffexp", "scrumblefalloffexp",
                                               "speedhighwindmultiplier", "speedlerpstart", "speedlerpend",
                                               "static")])
_term("soft depth blend", "$depthblend", "$depthblendscale")
_term("color2 / tint by base alpha", "$color2", "$blendtintbybasealpha", "$blendtintcoloroverbase")
_term("flashlight response", "$flashlighttint", "$flashlightnolambert")
_term("color-space overrides", "$linearwrite", "$gammacolorread", "$linearread_basetexture",
      "$linearread_texture1", "$linearread_texture2", "$linearread_texture3")
_term("lightmap water fog (P2)", "$lightmapwaterfog")
_term("sprite orientation/render mode", "$spriteorientation", "$spriteorigin", "$spriterendermode",
      "$spritesize", "$spritescale", "$ignorevertexcolors", "$vertexalphatest", "$hdrcolorscale")
_term("modulate blend ($mod2x)", "$mod2x")
_term("second texture (UnlitTwoTexture)", "$texture2", "$frame2_", "$texture2scale")
_term("HDR sky encodings", "$hdrcompressedtexture", "$hdrbasetexture", "$hdrcompressedtexture0",
      "$hdrcompressedtexture1", "$hdrcompressedtexture2")
_term("distance-field alpha (text/decals)", "$distancealpha", "$softedges", "$edgesoftnessstart",
      "$edgesoftnessend", "$scaleedgesoftnessbasedonscreenres", "$outline", "$outlinecolor", "$outlinestart0",
      "$outlinestart1", "$outlineend0", "$outlineend1", "$scaleoutlinesoftnessbasedonscreenres", "$glow",
      "$glowalpha", "$glowstart", "$glowend", "$glowx", "$glowy", "$distancealphafromdetail",
      "$vertexalphatest")
_term("vertex color modulate", "$vertexcolormodulate")
_term("glow color (P2 VertexLitGeneric)", "$glowcolor")


def detail_term(mode):
    return "detail mode %s" % mode


# Shaders a base family covers with an extra term; the rest are the long tail
# ("shader:<name>"). Category "internal" is engine-internal post/debug/tool.
SHADER_TERM = {
    "worldvertextransition": None,  # its $basetexture2 needs the two-layer term
    "sprite": "sprite orientation/render mode",
    "decalmodulate": "modulate blend ($mod2x)",
    "modulate": "modulate blend ($mod2x)",
    "unlittwotexture": "second texture (UnlitTwoTexture)",
    "sky": "HDR sky encodings",
    "decalbasetimeslightmapalphablendselfillum": "selfillum",
    "black": None,
}
INTERNAL_SHADERS = {
    "screenspace_general", "wireframe", "debugluxels", "showdestalpha", "writez", "vr_distort_texture",
    "blurfiltery", "blurfilterx", "debugtextureview", "shadow", "sample4x4", "sample4x4_blend", "floattoscreen",
    "downsample_nohdr", "downsample", "volumetricfog", "vr_distort_hud", "debuglightingonly", "vertexnormals",
    "hsv", "showz", "engine_post", "shadowmodel", "depthoffield", "floatcombine", "motionblur", "lightshafts",
    "occlusion", "setz", "shadowbuild", "accumbuff4sample", "accumbuff5sample", "bloom", "colorcorrection",
    "compositor", "bufferclearobeystencil", "debugmrttexture", "debugmorphaccumulator", "debugdepth",
    "debugdrawenvmapmask", "debugsoftwarevertexshader", "fillrate", "floattoscreen_vanilla", "introscreenspaceeffect",
    "modulate_hdr", "portal_refract_mask", "rendertargetblit", "sample4x4delog", "screenspace_general_8tex",
    "unlitgeneric_nobaseblend", "yuv", "bik", "color_projection", "tonemap", "luminance_compare", "passthru",
    "filmgrain", "filmdust", "apply_fog", "writestencil", "decalbasetimeslightmapalphablendselfillum_nope",
    "wireframe_dx9", "debugnormalmap", "debugtangentspace", "fxaa", "copyfb", "downsample_nohdr",
}


def shader_category(shader):
    if shader in BASE_SHADERS:
        return "base"
    if shader in SHADER_TERM:
        return "foldable"
    if shader in INTERNAL_SHADERS or shader.startswith("debug"):
        return "internal"
    return "long-tail"


# ---------------------------------------------------------------------------
# Phase 2


class Analyzer:
    def __init__(self):
        self.aliases = shader_aliases()
        self.decl = declared_params()

    def declared(self, shader_raw):
        s = shader_raw.lower()
        names = {s}
        seen = set()
        while s in self.aliases and self.aliases[s] and s not in seen:
            seen.add(s)
            s = self.aliases[s]
            names.add(s)
        out = set()
        for n in names:
            out |= self.decl.get(n, set())
        return out

    def analyze(self, mat, game="portal"):
        """Effective keys, terms and diagnostics of a loaded material."""
        if "error" in mat:
            return None
        shader = canonical_shader(mat["shader_raw"], self.aliases)
        declared = self.declared(mat["shader_raw"])
        known_shader = bool(declared)
        allowed = declared | STANDARD | FLAGS
        scratch_refs = set()
        implied = {}
        proxy_names = []
        for name, params in mat["proxies"]:
            proxy_names.append(name)
            for pk, pv in params.items():
                if pv.strip().startswith("$"):
                    ref = pv.strip().split("[")[0].lower()
                    scratch_refs.add(ref)
                    if pk in ("resultvar", "texturescrollvar", "texturerotatevar", "animatedtexturevar",
                              "animatedtextureframenumvar") and ref in allowed:
                        implied[ref] = name
            if name == "texturescroll" and "texturescrollvar" not in params:
                implied["$basetexturetransform"] = name
            if name == "animatedtexture" and "animatedtextureframenumvar" not in params:
                implied["$frame"] = name
        raw = mat["vars"]
        eff, info = {}, collections.Counter()
        undeclared = []
        for k, v in raw.items():
            if not k.startswith("$"):
                info["malformed key"] += 1
                continue
            if k in IGNORED:
                info["ignored"] += 1
                continue
            if known_shader and k not in allowed:
                if k in scratch_refs:
                    info["proxy scratch"] += 1
                    continue
                undeclared.append(k)
                # The product's stdshaders are Portal's own, so an undeclared
                # key has no effect there. Retail Portal 2 shaders declare more
                # (e.g. $envmaplightscale, $treesway): keep them for portal2.
                if game != "portal2":
                    info["undeclared (no effect)"] += 1
                    continue
                if k not in TERM_OF_KEY and k not in PARENT and not re.fullmatch(r"\$[a-z0-9_]+", k):
                    info["undeclared junk"] += 1
                    continue
                info["undeclared kept (portal2)"] += 1
            if is_neutral(k, v):
                info["neutral"] += 1
                continue
            eff[k] = v
        for k, name in implied.items():
            if k not in eff and k in allowed:
                eff[k] = "<proxy %s>" % name
        changed = True
        while changed:
            changed = False
            for k in list(eff):
                p = PARENT.get(k)
                if p and p not in eff:
                    del eff[k]
                    info["dead (parent off)"] += 1
                    changed = True
        # $detailblendmode is a term selector, not its own feature.
        terms = set()
        cat = shader_category(shader)
        if cat == "base":
            terms.add("base")
        elif cat == "foldable":
            terms.add("base")
            if SHADER_TERM.get(shader):
                terms.add(SHADER_TERM[shader])
        else:
            terms.add("shader:" + shader)
        if shader in BASE_SHADERS or cat == "foldable":
            for k in eff:
                if k == "$detail":
                    mode = eff.get("$detailblendmode", "0").strip() or "0"
                    try:
                        mode = str(int(float(mode)))
                    except ValueError:
                        mode = "?"
                    terms.add(detail_term(mode))
                elif k in ("$detailscale", "$detailblendmode", "$detailblendfactor", "$detailtint", "$detailframe"):
                    continue
                else:
                    terms.add(TERM_OF_KEY.get(k, "key:" + k))
        return {"shader": shader, "category": cat, "eff": eff, "terms": terms, "info": info,
                "undeclared": undeclared, "proxies": proxy_names, "known_shader": known_shader}


def greedy_order(items, fixed_first):
    """items: [(weight, terms set)]. Each step adds the single term, or the pair
    of terms, with the most newly covered materials per term added (a pair only
    when it beats every single term, e.g. $envmap with $envmaptint).
    Returns [(terms tuple, gain)]."""
    supported = set(fixed_first)
    order = []
    while True:
        missing = collections.Counter()
        for w, t in items:
            m = frozenset(t - supported)
            if m:
                missing[m] += w
        if not missing:
            return order
        demand = collections.Counter()
        for m, w in missing.items():
            for term in m:
                demand[term] += w
        candidates = {frozenset([x]) for x in demand}
        candidates |= {m for m in missing if len(m) == 2}
        best, best_key = None, None
        for c in candidates:
            gain = sum(w for m, w in missing.items() if m <= c)
            key = (gain / len(c), -len(c), sum(demand[x] for x in c), sorted(c))
            if best_key is None or key[:3] > best_key[:3] or (key[:3] == best_key[:3] and key[3] < best_key[3]):
                best, best_key = c, key
        gain = sum(w for m, w in missing.items() if m <= best)
        supported |= best
        order.append((tuple(sorted(best)), gain))


# The RFC 0016 surface-model plan's phases ("The surface model: legacy
# materials as degenerate cases"), as the terms each adds.
PHASES = [
    ("S0 current model", ["base"]),
    ("S1 coverage and state", ["blend: translucent", "blend: additive",
                               "render state (nocull/ignorez/nofog/decal/znearer)",
                               "modulate blend ($mod2x)"]),
    ("S2 specular image", ["envmap", "envmap mask", "envmap tint", "envmap contrast/saturation",
                           "envmap fresnel", "envmap lightmap scale (P2)"]),
    ("S3 normal and basis light", ["bump (RNM lightmap / per-pixel normal)", "ssbump",
                                   "texture transforms"]),
    ("S4 detail", ["detail mode %d" % mode for mode in range(13)]),
    ("S5 emission", ["selfillum", "selfillum fresnel"]),
    ("S6 layers", ["two-layer blend (WVT $basetexture2)", "two-layer blend modulate",
                   "triplanar ($seamless)"]),
    ("S7 model surfaces", ["phong", "phong exponent texture / albedo tint", "rim light", "light warp",
                           "half-lambert / ambient-only", "color2 / tint by base alpha",
                           "glow color (P2 VertexLitGeneric)", "flashlight response"]),
    ("S8 unlit points", ["sprite orientation/render mode", "second texture (UnlitTwoTexture)",
                         "HDR sky encodings", "distance-field alpha (text/decals)",
                         "vertex color modulate", "color-space overrides"]),
]


def print_phases(per, weights):
    supported = set()
    for name, terms in PHASES:
        supported |= set(terms)
        row = []
        for g in per:
            covered = sum(1 for a in per[g].values() if a["terms"] <= supported)
            wl = weights[g]
            area = sum(x[2] for x in wl) or 1
            faces = sum(x[3] for x in wl) or 1
            area_covered = sum(x[2] for x in wl if x[4] is not None and x[4]["terms"] <= supported)
            faces_covered = sum(x[3] for x in wl if x[4] is not None and x[4]["terms"] <= supported)
            row.append("%s %.1f%% materials, %.1f%% area, %.1f%% faces" % (
                g, 100.0 * covered / len(per[g]), 100.0 * area_covered / area,
                100.0 * faces_covered / faces))
        print("%-28s %s" % (name, " | ".join(row)))
    rest = collections.Counter()
    for g in per:
        for a in per[g].values():
            for term in a["terms"] - supported:
                rest[g + " " + term] += 1
    print("outside the phases:", ", ".join("%s %d" % kv for kv in rest.most_common(20)))


def main(argv=None):
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", required=True)
    ap.add_argument("--phases", action="store_true",
                    help="print the coverage after each phase of the RFC 0016 surface-model plan")
    ap.add_argument("--refresh", action="store_true")
    args = ap.parse_args(argv)
    out = Path(args.out)
    out.mkdir(parents=True, exist_ok=True)
    cache = out / "materials.pickle"
    if args.refresh and cache.exists():
        cache.unlink()
    data = load(cache)
    an = Analyzer()
    report = {"schema": "material-inventory/v1", "profile": PROFILE, "symbols": sorted(SYMBOLS),
              "games": {}, "notes": []}
    per = {}  # game -> path -> analysis
    for game, g in data.items():
        per[game] = {}
        for p, m in g["materials"].items():
            a = an.analyze(m, game)
            if a is not None:
                per[game][p] = a
        # subrect materials take the terms of their target
        for p, a in per[game].items():
            if a["shader"] == "subrect":
                tgt = g["materials"][p]["vars"].get("$material", "")
                tp = "materials/" + norm_path(tgt).replace("materials/", "", 1)
                if not tp.endswith(".vmt"):
                    tp += ".vmt"
                if tp in per[game]:
                    a["terms"] = set(per[game][tp]["terms"])
                    a["subrect_target"] = tp
                    a["category"] = "subrect:" + per[game][tp]["category"]
    games = list(per)

    def counter_table(fn):
        res = {}
        for game in games:
            c = collections.Counter()
            for p, a in per[game].items():
                for x in fn(p, a):
                    c[x] += 1
            res[game] = dict(c.most_common())
        return res

    # 1. shaders per game
    report["shaders"] = counter_table(lambda p, a: [a["shader"]])
    report["categories"] = counter_table(lambda p, a: [a["category"]])
    report["load_errors"] = {game: {p: m["error"] for p, m in data[game]["materials"].items() if "error" in m}
                             for game in games}

    # 2. shader x key: raw set count and effective count
    sk = {}
    for game in games:
        rawc = collections.defaultdict(collections.Counter)
        effc = collections.defaultdict(collections.Counter)
        undc = collections.defaultdict(collections.Counter)
        for p, a in per[game].items():
            m = data[game]["materials"][p]
            for k in m["vars"]:
                rawc[a["shader"]][k] += 1
            for k in a["eff"]:
                effc[a["shader"]][k] += 1
            for k in a["undeclared"]:
                undc[a["shader"]][k] += 1
        sk[game] = {s: {"set": dict(rawc[s].most_common()), "effective": dict(effc[s].most_common()),
                        "undeclared_by_product_shader": dict(undc[s].most_common())}
                    for s in sorted(rawc, key=lambda s: -sum(1 for _ in per[game]))}
    report["shader_keys"] = sk

    # value distributions
    def vdist(key, shaders=None, norm=lambda v: v.strip().lower()):
        res = {}
        for game in games:
            c = collections.Counter()
            for p, a in per[game].items():
                if shaders and a["shader"] not in shaders:
                    continue
                if key in a["eff"]:
                    c[norm(a["eff"][key])] += 1
            res[game] = dict(c.most_common(25))
        return res

    def envmap_target(v):
        v = v.strip().lower().replace("\\", "/")
        if v == "env_cubemap":
            return "env_cubemap"
        if v.startswith("maps/"):
            return "maps/<map baked cubemap>"
        return "named: " + v
    report["values"] = {
        "$envmap": vdist("$envmap", norm=envmap_target),
        "$detailblendmode(with $detail)": {g: dict(collections.Counter(
            str(t) for p, a in per[g].items() for t in a["terms"] if t.startswith("detail mode")).most_common())
            for g in games},
        "$phongexponent": vdist("$phongexponent"),
        "$phongboost": vdist("$phongboost"),
        "$phongfresnelranges": vdist("$phongfresnelranges"),
        "$alphatestreference": vdist("$alphatestreference"),
        "$detailscale": vdist("$detailscale"),
        "$detail": vdist("$detail"),
        "$color": vdist("$color"),
        "$alpha": vdist("$alpha"),
        "$envmaptint": vdist("$envmaptint"),
        "$envmapcontrast": vdist("$envmapcontrast"),
        "$envmapsaturation": vdist("$envmapsaturation"),
        "$fresnelreflection": vdist("$fresnelreflection"),
        "$selfillumtint": vdist("$selfillumtint"),
        "$spriterendermode": vdist("$spriterendermode"),
        "$spriteorientation": vdist("$spriteorientation"),
        "$seamless_scale": vdist("$seamless_scale"),
        "$envmaplightscale": vdist("$envmaplightscale"),
    }
    report["proxies"] = counter_table(lambda p, a: a["proxies"])
    report["proxies_by_shader"] = {g: {} for g in games}
    for g in games:
        c = collections.defaultdict(collections.Counter)
        for p, a in per[g].items():
            for n in a["proxies"]:
                c[a["shader"]][n] += 1
        report["proxies_by_shader"][g] = {s: dict(v.most_common()) for s, v in c.items()}
    report["materials_with_proxies"] = {g: sum(1 for a in per[g].values() if a["proxies"]) for g in games}
    report["key_info"] = {g: dict(sum((a["info"] for a in per[g].values()), collections.Counter())) for g in games}

    # 4. signatures per shader (effective keys, $detailblendmode folded into detail:N)
    def signature(a):
        keys = []
        for k in sorted(a["eff"]):
            if k in ("$detailblendmode",):
                continue
            if k == "$detail":
                keys.append("$detail:" + a["eff"].get("$detailblendmode", "0").strip())
            else:
                keys.append(k)
        return " ".join(keys) if keys else "(none)"
    sigs = {}
    for g in games:
        c = collections.defaultdict(collections.Counter)
        for p, a in per[g].items():
            c[a["shader"]][signature(a)] += 1
        sigs[g] = {s: {"materials": sum(v.values()), "distinct": len(v), "top": v.most_common(20)}
                   for s, v in sorted(c.items(), key=lambda kv: -sum(kv[1].values())) if sum(v.values()) >= 5}
    report["signatures"] = sigs

    # 3. coverage curve (greedy over both games' materials, equal weight)
    items = []
    for g in games:
        for p, a in per[g].items():
            items.append((1, a["terms"]))
    order = greedy_order(items, ["base"])
    totals = {g: len(per[g]) for g in games}
    surface = {g: sum(1 for a in per[g].values() if "internal" not in a["category"]) for g in games}

    # World-area weights
    weights = {}
    for g in games:
        wl = []
        for mapname, w in data[g]["weights"].items():
            fa = w["faces"]
            for mname, area in fa["areas"].items():
                kind, ref = w["resolved"][mname]
                if kind == "pak":
                    a = an.analyze(ref, g)
                elif kind == "game":
                    a = per[g].get(ref)
                else:
                    a = None
                wl.append((mapname, mname, area, fa["faces"][mname], a))
        weights[g] = wl

    if args.phases:
        print_phases(per, weights)

    def coverage_after(supported):
        row = {}
        for g in games:
            cov = sum(1 for a in per[g].values() if a["terms"] <= supported)
            cov_s = sum(1 for a in per[g].values() if a["terms"] <= supported and "internal" not in a["category"])
            row[g] = {"materials_pct": round(100.0 * cov / totals[g], 1),
                      "surface_materials_pct": round(100.0 * cov_s / max(1, surface[g]), 1)}
            wl = weights[g]
            if wl:
                tot = sum(x[2] for x in wl)
                c = sum(x[2] for x in wl if x[4] is not None and x[4]["terms"] <= supported)
                row[g]["world_area_pct"] = round(100.0 * c / tot, 1)
                totf = sum(x[3] for x in wl)
                cf = sum(x[3] for x in wl if x[4] is not None and x[4]["terms"] <= supported)
                row[g]["world_faces_pct"] = round(100.0 * cf / totf, 1)
        return row
    curve = [{"term": "base (lightmap/model light/unlit; texture, vertex color/alpha, $color/$alpha, alpha test; opaque)",
              "gain": sum(1 for _, t in items if t <= {"base"}), "cumulative": coverage_after({"base"})}]
    sup = {"base"}
    for terms, gain in order:
        sup |= set(terms)
        curve.append({"term": " + ".join(terms), "gain": gain, "cumulative": coverage_after(sup)})
    report["coverage_curve"] = curve
    report["coverage_denominators"] = {"materials": totals, "surface_materials": surface}

    # Fixed-order curve in the plan's proposed sequence (for comparison)
    proposed = ["blend: translucent", "blend: additive", "render state (nocull/ignorez/nofog/decal/znearer)",
                "envmap", "envmap mask", "envmap tint", "envmap contrast/saturation", "envmap fresnel",
                "detail mode 0", "detail mode 1", "detail mode 2", "detail mode 3", "detail mode 4",
                "detail mode 5", "detail mode 6", "detail mode 7", "detail mode 8", "detail mode 9",
                "detail mode 10", "detail mode 11", "detail mode 12", "selfillum",
                "bump (RNM lightmap / per-pixel normal)", "ssbump", "two-layer blend (WVT $basetexture2)",
                "texture transforms", "phong"]
    sup = {"base"}
    fixed = []
    for term in proposed:
        sup.add(term)
        fixed.append({"term": term, "cumulative": coverage_after(sup)})
    report["coverage_proposed_order"] = fixed

    # World-area term usage and uncovered at the end of the curve
    world = {}
    for g in games:
        wl = weights[g]
        tot = sum(x[2] for x in wl)
        by_shader = collections.Counter()
        by_term = collections.Counter()
        by_mat = collections.Counter()
        by_sig = collections.Counter()
        for mapname, mname, area, nf, a in wl:
            if a:
                by_sig[" + ".join(sorted(a["terms"] - {"base"})) or "base only"] += area
            by_shader[a["shader"] if a else "<unresolved>"] += area
            by_mat[mname] += area
            if a:
                for t in a["terms"]:
                    by_term[t] += area
        world[g] = {"maps": sorted(data[g]["weights"]), "total_area": round(tot),
                    "shader_area_pct": {k: round(100 * v / tot, 2) for k, v in by_shader.most_common()},
                    "term_area_pct": {k: round(100 * v / tot, 2) for k, v in by_term.most_common()},
                    "top_materials_area_pct": {k: round(100 * v / tot, 2) for k, v in by_mat.most_common(25)},
                    "distinct_materials": len(by_mat),
                    "term_sets_area_pct": {k: round(100 * v / tot, 2) for k, v in by_sig.most_common(20)}}
    report["world_surfaces"] = world

    # 5. long tail: materials needing a shader:* or key:* term
    tail = {}
    for g in games:
        c = collections.Counter()
        keyterms = collections.Counter()
        examples = collections.defaultdict(list)
        for p, a in per[g].items():
            for t in a["terms"]:
                if t.startswith("shader:"):
                    c[t[7:] + " (" + a["category"] + ")"] += 1
                    if len(examples[t]) < 3:
                        examples[t].append(p)
                elif t.startswith("key:"):
                    keyterms[a["shader"] + " " + t[4:]] += 1
        tail[g] = {"shaders": dict(c.most_common()), "unplanned_keys": dict(keyterms.most_common(60)),
                   "examples": dict(examples)}
    report["long_tail"] = tail

    # term demand: how many materials need each term (regardless of coverage)
    report["term_demand"] = counter_table(lambda p, a: sorted(a["terms"]))

    # pak lumps
    pak = {}
    for g in games:
        c = collections.Counter()
        patched = 0
        pkeys = collections.Counter()
        for name, m in data[g]["pak"].items():
            a = an.analyze(m, g)
            c[a["shader"] if a else "<error>"] += 1
            if m.get("includes"):
                patched += 1
        pak[g] = {"vmts": len(data[g]["pak"]), "patch_materials": patched, "shaders": dict(c.most_common(20))}
    report["pak_lumps"] = pak
    report["notes"] = [
        "Effective keys drop: keys without '$' (malformed), metadata/tool/console keys (IGNORED), keys the "
        "product shader does not declare (listed per shader as undeclared_by_product_shader; proxy scratch "
        "variables separately), neutral values ($color [1 1 1], flags 0, empty textures, ...), and child keys "
        "whose parent feature is off (e.g. $envmaptint without $envmap).",
        "Proxy result variables that are shader parameters count as effective keys (e.g. TextureScroll -> "
        "$basetexturetransform).",
        "Search order and parsing follow tools/render/vmt_corpus.py and render/material/vmt_import.cpp; the "
        "profile is dx95/ps20b/HDR/sRGB/GPU level 3, conditionals with WIN32/LINUX/POSIX held.",
        "World area: dface_t.area of drawn brush faces (nodraw/skip/hint/trigger and sky excluded) of the "
        "listed maps, each face's material resolved through the map's pak lump (cubemap patches). Static and "
        "dynamic props are not in the face lump.",
    ]
    with open(out / "inventory.json", "w") as f:
        json.dump(report, f, indent=1, default=lambda o: sorted(o) if isinstance(o, set) else str(o))
    # compact console summary
    for g in games:
        print("==", g, totals[g], "materials;", "surface", surface[g])
    for row in curve:
        print("%-55s %+5d  %s" % (row["term"][:55], row["gain"], "  ".join(
            "%s %5.1f/%5.1f/%s/%s" % (g[:2] if g == "portal" else "p2", row["cumulative"][g]["materials_pct"],
                                   row["cumulative"][g]["surface_materials_pct"],
                                   row["cumulative"][g].get("world_area_pct", "-"),
                                   row["cumulative"][g].get("world_faces_pct", "-")) for g in games)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
