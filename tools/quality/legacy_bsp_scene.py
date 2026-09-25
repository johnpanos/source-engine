#!/usr/bin/env python3
"""Author an OpenUSD scene from a compiled legacy BSP, for relighting it.

    <usd python> tools/quality/legacy_bsp_scene.py --bsp maps/testchmb_a_00.bsp \\
        --runtime run/runtime-native --out <dir>

The scene is what `pbrt_map_build.py` bakes when a manifest names a
`legacy_bsp` (the relight path, `legacy_bsp_relight.py`): the map's own
geometry, materials and lights, so the new light lands on the BSP's
unchanged gameplay data. Stage units are Source units (metersPerUnit 0.0254,
Z-up), so every point keeps its BSP coordinates.

Geometry. Model 0's faces, one mesh per material, with `st` from each face's
texinfo (v flipped to USD's bottom-left origin). Left out, and still drawn by
the legacy renderer from the unchanged lumps: sky faces (they stay openings,
so the sun and sky light enter), displacements, water and translucent faces,
and every brush entity. The engine replaces only the opaque world with the
relit world mesh, so these keep their vrad lightmaps. vrad's shadow rays test
the solid brushes, not the faces, so the nodraw sides of the solid world
brushes are added as black occluders (`relight_occluder`), which the bake
gives no atlas space and the pack step leaves out.

Materials. Each face's VMT is read from the map's pak lump or the game
content (a cubemap patch resolves to its included material). The base texture
and bump map are decoded from their VTFs (`vtf_decode`); Source bump maps are
DirectX (+Y down) and are written OpenGL (+Y up). `$alphatest` becomes a
cut-out; everything else is opaque, dielectric, roughness 0.8 (or
sqrt(2 / (e + 2)) of a numeric `$phongexponent`).

Lights come from vrad's compiled world lights (the HDR lump when present),
in the engine's lightmap unit, which is the pipeline's (irradiance / pi):
  point  a SphereLight of radius 2 units whose light at 100 units (where vrad
         normalizes a light's brightness) matches vrad's falloff there;
  spot   a one-sided DiskLight of that size facing the spot's direction,
         matched on its axis, with vrad's cone (`sourceEngine:coneInner`,
         `coneOuter`, `coneExponent`), which the disk's projected area
         completes exactly;
  sun    a DistantLight of irradiance pi x intensity (vrad's lightmap is
         intensity x cos);
  sky ambient  a constant DomeLight of radiance equal to the intensity (vrad
         normalizes the visible hemisphere to it);
  surface      the emitting (SURF_LIGHT) faces' materials get an emissive
         radiance of pi x (their patches' summed intensity) / face area, per
         face; faces of one material that differ get separate materials.
A styled light keeps its light style (`sourceEngine:lightStyle`), so the
radiosity transfer follows the map's own `TurnOn`/`TurnOff` (styles 32+) and
preset animations (1-31). A named light that starts dark (spawnflag 1) is
left out of the bake: it stays a world light the engine applies when it is
switched on. The receipt lists
every approximation, exclusion and conversion.
"""

import argparse
import hashlib
import json
import math
import re
import sys
from pathlib import Path

import numpy as np

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import legacy_bsp  # noqa: E402
import source_content  # noqa: E402
import vtf_decode  # noqa: E402

SCHEMA = "legacy-relight-scene/v1"
METERS_PER_UNIT = 0.0254
LIGHT_RADIUS_UNITS = 2.0
NORMALIZE_DISTANCE = 100.0  # vrad scales a light's brightness to its value here
OCCLUDER = "relight_occluder"
DEFAULT_ROUGHNESS = 0.8
MAX_TEXTURE = 2048
# Faces of these shaders are drawn by the legacy translucent/water passes.
LEGACY_ONLY_SHADERS = {"water", "refract", "unlittwotexture", "monitorscreen",
                       "spritecard", "sprite", "cable"}
CUBEMAP_PATCH_KEYS = {"$envmap", "$envmaporigin", "$envmapmaskintintmasktexture"}
BASE_KEY = "%relight_base"  # the material a cubemap patch renders as
FIRST_SWITCHED_STYLE = 32  # below: the engine's preset animated styles
EMISSION_STEP = 0.02  # emissive radiances within 2% share a material
SURFACE_MATCH_DISTANCE = 2.0  # units: a patch light's origin lies on (or 1 unit off) its face


def sha256_bytes(data):
    return hashlib.sha256(data).hexdigest()


# ------------------------------------------------------------------ VMT

def parse_keyvalues(text):
    """A VMT's KeyValues as (root name, {key: value | dict}); keys lower-case."""
    text = re.sub(r"//[^\n]*", "", text)
    tokens = re.findall(r'"([^"]*)"|([{}])|([^\s{}"]+)', text)
    items = [quoted if quoted or not (brace or bare) else (brace or bare)
             for quoted, brace, bare in tokens]
    position = 0

    def block():
        nonlocal position
        result = {}
        while position < len(items):
            key = items[position]
            position += 1
            if key == "}":
                return result
            if position >= len(items):
                break
            value = items[position]
            position += 1
            if value == "{":
                result[key.lower()] = block()
            else:
                result.setdefault(key.lower(), value)
        return result
    if len(items) < 2 or items[1] != "{":
        raise ValueError("VMT does not start with a shader block")
    root = items[0].lower()
    position = 2
    return root, block()


class GameDirectory:
    """The compile's game directory (vrad's `-game`) ahead of the installed
    content, so a map's own loose materials resolve as they did for vrad.
    Lookups ignore case, like the Source filesystem's."""

    def __init__(self, path, resolver):
        self.path = Path(path)
        self.resolver = resolver
        self.files = {str(f.relative_to(self.path)).replace("\\", "/").lower(): f
                      for f in sorted((self.path / "materials").rglob("*")) if f.is_file()}

    def read(self, relative):
        found = self.files.get(relative.replace("\\", "/").lower())
        if found is not None:
            return found.read_bytes(), "game:" + relative.lower()
        return self.resolver.read(relative)


class Materials:
    """VMT lookup through the map's pak lump, then the game content."""

    def __init__(self, bsp, resolver):
        self.pak = bsp.pakfile()
        self.pak_names = {name.lower(): name for name in self.pak.namelist()} if self.pak else {}
        self.resolver = resolver
        self.cache = {}

    def read(self, relative):
        relative = relative.replace("\\", "/").lower()
        if relative in self.pak_names:
            return self.pak.read(self.pak_names[relative]), "pak:" + relative
        return self.resolver.read(relative)

    def vmt(self, name, depth=0):
        """(shader, params, source) with patches resolved."""
        key = name.lower()
        if key in self.cache:
            return self.cache[key]
        if depth > 8:
            raise ValueError("VMT patch chain too deep at " + name)
        data, source = self.read("materials/%s.vmt" % key)
        if data is None:
            result = (None, {}, None)
        else:
            shader, params = parse_keyvalues(data.decode("latin-1"))
            if shader == "patch":
                include = params.get("include", "").replace("\\", "/").lower()
                include = re.sub(r"^materials/", "", re.sub(r"\.vmt$", "", include))
                base_shader, base, _ = self.vmt(include, depth + 1)
                merged = dict(base)
                changed = set()
                for section in ("insert", "replace"):
                    for k, v in (params.get(section) or {}).items():
                        if not isinstance(v, dict):
                            merged[k] = v
                            changed.add(k)
                # vbsp's cubemap patches only add the probe's $envmap; they
                # render as their base material here.
                if not changed <= CUBEMAP_PATCH_KEYS:
                    merged[BASE_KEY] = key
                result = (base_shader, merged, "%s (patch of %s)" % (source, include))
            else:
                result = (shader, dict({k: v for k, v in params.items()
                                        if not isinstance(v, dict)}, **{BASE_KEY: key}), source)
        self.cache[key] = result
        return result

    def texture(self, name):
        """(RGBA8 image, info, source) of materials/<name>.vtf, or None."""
        name = name.replace("\\", "/").lower()
        name = re.sub(r"\.vtf$", "", re.sub(r"^materials/", "", name))
        data, source = self.read("materials/%s.vtf" % name)
        if data is None:
            return None
        info = vtf_decode.header(data)
        mip = 0
        while max(info["width"], info["height"]) >> mip > MAX_TEXTURE and \
                mip + 1 < info["mips"]:
            mip += 1
        image, info = vtf_decode.decode(data, mip)
        return image, info, source


def truthy(value):
    try:
        return float(value) != 0
    except (TypeError, ValueError):
        return False


# ------------------------------------------------------------------ model

def sanitize(name):
    text = re.sub(r"[^a-z0-9_]+", "_", name.lower()).strip("_") or "unnamed"
    if text[0].isdigit():
        text = "m_" + text
    if len(text) > 40:
        text = text[:31] + "_" + hashlib.sha1(name.lower().encode()).hexdigest()[:8]
    return text


def face_st(points, texinfo, mapping):
    """USD `st` of face points: the engine's texture coordinate (texinfo
    vectors over the material's mapping size, gl_matsysiface.cpp), with v
    flipped to USD's bottom-left origin. `mapping` is {width, height}."""
    vectors = texinfo["vectors"]
    u = (points @ vectors[0, :3] + vectors[0, 3]) / mapping["width"]
    v = (points @ vectors[1, :3] + vectors[1, 3]) / mapping["height"]
    return np.stack([u, 1.0 - v], axis=1)


# CMaterial::FindRepresentativeTexture's order; the engine divides texture
# coordinates by that texture's mapping (full-resolution VTF) size.
PLANE_PRIMVAR = "sourceEngine:plane"
REPRESENTATIVE_TEXTURES = ("$basetexture", "$envmapmask", "$bumpmap", "$dudvmap", "$normalmap")
ERROR_TEXTURE_SIZE = 32  # texturemanager.cpp: a missing texture maps as the error texture


def mapping_size(materials, name):
    """({width, height}, source) the engine maps material `name`'s surfaces with."""
    _, params, _ = materials.vmt(name)
    for key in REPRESENTATIVE_TEXTURES:
        texture = params.get(key)
        if isinstance(texture, str) and texture:
            texture = re.sub(r"\.vtf$", "", re.sub(r"^materials/", "",
                                                    texture.replace("\\", "/").lower()))
            data, _ = materials.read("materials/%s.vtf" % texture)
            if data is None:
                return {"width": ERROR_TEXTURE_SIZE, "height": ERROR_TEXTURE_SIZE}, \
                    "%s %s missing: error texture" % (key, texture)
            header = vtf_decode.header(data)
            return {"width": header["width"], "height": header["height"]}, \
                "%s %s" % (key, texture)
    return {"width": ERROR_TEXTURE_SIZE, "height": ERROR_TEXTURE_SIZE}, \
        "no representative texture: error texture"


def point_in_polygon(point, polygon, normal, tolerance):
    """Whether `point`, projected onto the convex polygon's plane, lies inside
    it (at most `tolerance` units outside any edge), for either winding."""
    area = np.zeros(3)
    for i in range(1, len(polygon) - 1):
        area += np.cross(polygon[i] - polygon[0], polygon[i + 1] - polygon[0])
    sign = 1.0 if np.dot(area, normal) >= 0 else -1.0
    count = len(polygon)
    for i in range(count):
        a, b = polygon[i], polygon[(i + 1) % count]
        inward = sign * np.cross(normal, b - a)
        length = np.linalg.norm(inward)
        if length < 1e-9:
            continue
        if np.dot(point - a, inward) / length < -tolerance:
            return False
    return True


def match_surface_lights(lights, faces):
    """Assign each emit_surface world light to the emitting face under it.
    Returns ({face index: summed intensity}, [unmatched light indices])."""
    candidates = [(f, f["points"].mean(axis=0)) for f in faces]
    sums, unmatched = {}, []
    for light in lights:
        origin = np.asarray(light["origin"])
        normal = np.asarray(light["normal"])
        best, best_distance = None, None
        for face, _centre in candidates:
            if np.dot(face["plane_normal"], normal) < 0.99:
                continue
            distance = abs(np.dot(origin - face["points"][0], face["plane_normal"]))
            if distance > SURFACE_MATCH_DISTANCE:
                continue
            if not point_in_polygon(origin, face["points"], face["plane_normal"], 0.5):
                continue
            if best is None or distance < best_distance:
                best, best_distance = face, distance
        if best is None:
            unmatched.append(light["index"])
            continue
        total = sums.setdefault(best["index"], np.zeros(3))
        total += np.asarray(light["intensity"])
    return sums, unmatched


def falloff_match(light):
    """vrad's falloff 1 / (c + l d + q d^2) matched by an inverse square at
    the normalization distance: the effective intensity I' with I'/d^2 = I/(...)."""
    c, l, q = light["attenuation"]
    d = NORMALIZE_DISTANCE
    denominator = c + l * d + q * d * d
    if denominator <= 0:
        raise ValueError("world light %d has no falloff" % light["index"])
    return d * d / denominator, not (c == 0 and l == 0)


def entity_for_light(entities, light):
    origin = np.asarray(light["origin"])
    best = None
    for entity in entities:
        if not entity.get("classname", "").startswith("light") or "origin" not in entity:
            continue
        try:
            position = np.array([float(v) for v in entity["origin"].split()])
        except ValueError:
            continue
        if position.shape == (3,) and np.abs(position - origin).max() < 0.05:
            best = entity
            break
    return best


def starting_dark_styles(lights, owners):
    """Switchable styles (32+) that any light of starts dark (spawnflag 1)."""
    return {light["style"] for light in lights
            if light["style"] >= FIRST_SWITCHED_STYLE and owners.get(light["index"]) and
            int(owners[light["index"]].get("spawnflags", "0") or 0) & 1}


def build_model(bsp, resolver, texture_dir):
    """Everything the USD writer needs, as plain data, plus the receipt."""
    texinfo = bsp.texinfo()
    texdata = bsp.texdata()
    entities = bsp.entities()
    faces = bsp.world_faces()
    materials = Materials(bsp, resolver)
    lights, light_lump = bsp.world_lights()
    excluded = {}
    notes = []

    def exclude(reason, count=1):
        excluded[reason] = excluded.get(reason, 0) + count

    # Faces -> relit world faces grouped by material.
    groups = {}
    material_info = {}
    for face in faces:
        info = texinfo[face["texinfo"]]
        flags = info["flags"]
        if info["texdata"] < 0:
            exclude("no texdata")
            continue
        if flags & (legacy_bsp.SURF_SKY | legacy_bsp.SURF_SKY2D):
            exclude("sky (an opening)")
            continue
        if flags & (legacy_bsp.SURF_NODRAW | legacy_bsp.SURF_SKIP | legacy_bsp.SURF_HINT |
                    legacy_bsp.SURF_TRIGGER):
            exclude("tool face")
            continue
        if face["dispinfo"] != -1:
            exclude("displacement (legacy-lit)")
            continue
        if flags & legacy_bsp.SURF_WARP:
            exclude("water (legacy-lit)")
            continue
        name = texdata[info["texdata"]]["name"]
        if name not in material_info:
            shader, params, source = materials.vmt(name)
            material_info[name] = {"shader": shader, "params": params, "source": source}
        shader = material_info[name]["shader"]
        params = material_info[name]["params"]
        translucent = (flags & legacy_bsp.SURF_TRANS or truthy(params.get("$translucent")) or
                       truthy(params.get("$additive")) or
                       ("$alpha" in params and float(params["$alpha"]) < 1.0))
        if shader is None:
            exclude("missing VMT (legacy-lit)")
            notes.append("material %s has no VMT; its faces stay legacy-lit" % name)
            continue
        if translucent or shader in LEGACY_ONLY_SHADERS:
            exclude("translucent or special shader (legacy-lit)")
            continue
        groups.setdefault(name, []).append(face)

    # Surface lights: per emitting face, the radiance its patches imply.
    surface_lights = [light for light in lights if light["type"] == "surface"]
    emitting_faces = [face for group in groups.values() for face in group
                      if texinfo[face["texinfo"]]["flags"] & legacy_bsp.SURF_LIGHT]
    sums, unmatched = match_surface_lights(surface_lights, emitting_faces)
    if unmatched:
        notes.append("%d of %d surface lights lie on no relit face (brush entities or "
                     "left-out faces); their light is not baked" %
                     (len(unmatched), len(surface_lights)))
    radiance_of = {}
    for face in emitting_faces:
        if face["index"] in sums:
            area = legacy_bsp.polygon_area(face["points"])
            if area > 1e-6:
                radiance_of[face["index"]] = math.pi * sums[face["index"]] / area

    # Material records, split where emissive faces of one material differ.
    records = {}
    meshes = {}
    for name, group in sorted(groups.items()):
        params = material_info[name]["params"]
        base = sanitize(params.get(BASE_KEY, name))
        for face in group:
            radiance = radiance_of.get(face["index"])
            if radiance is not None:
                # Quantize on a log scale so equal lights share a material.
                steps = np.round(np.log(np.maximum(radiance, 1e-6)) / math.log1p(EMISSION_STEP))
                radiance = np.exp(steps * math.log1p(EMISSION_STEP))
            key = base if radiance is None else "%s_e%s" % (
                base, hashlib.sha1(steps.astype(np.int64).tobytes()).hexdigest()[:6])
            if key not in records:
                records[key] = {"source_material": name, "shader": material_info[name]["shader"],
                                "vmt": material_info[name]["source"], "params": params,
                                "emission": None if radiance is None else
                                [float(v) for v in radiance]}
            meshes.setdefault(key, []).append(face)

    # Textures.
    texture_dir.mkdir(parents=True, exist_ok=True)
    decoded = {}

    def texture_file(texture, kind):
        if (texture, kind) in decoded:
            return decoded[(texture, kind)]
        found = materials.texture(texture)
        if found is None:
            decoded[(texture, kind)] = None
            notes.append("texture %s is missing" % texture)
            return None
        from PIL import Image
        image, info, source = found
        if kind == "normal":
            image = image.copy()
            image[..., 1] = 255 - image[..., 1]  # DirectX (+Y down) -> OpenGL (+Y up)
        path = texture_dir / (sanitize(texture) + ("_normal" if kind == "normal" else "") +
                              ".png")
        Image.fromarray(image if kind != "normal" else image[..., :3],
                        "RGBA" if kind != "normal" else "RGB").save(path)
        decoded[(texture, kind)] = {"file": str(path), "source": source,
                                    "size": [int(image.shape[1]), int(image.shape[0])],
                                    "alpha": bool(kind == "base" and (image[..., 3] < 255).any())}
        return decoded[(texture, kind)]

    approximations = {}
    for key, record in records.items():
        params = record["params"]
        base = params.get("$basetexture")
        record["base"] = texture_file(base, "base") if base else None
        bump = params.get("$bumpmap") or params.get("$normalmap")
        if bump and truthy(params.get("$ssbump")):
            approximations.setdefault(key, []).append("self-shadowed bump map dropped")
            bump = None
        record["normal"] = texture_file(bump, "normal") if bump else None
        record["alphatest"] = truthy(params.get("$alphatest")) and bool(
            record["base"] and record["base"]["alpha"])
        record["alphatest_reference"] = float(params.get("$alphatestreference", 0.5) or 0.5)
        exponent = params.get("$phongexponent")
        try:
            record["roughness"] = math.sqrt(2.0 / (float(exponent) + 2.0)) if exponent \
                else DEFAULT_ROUGHNESS
        except ValueError:
            record["roughness"] = DEFAULT_ROUGHNESS
        if not record["base"]:
            record["base_color"] = [float(v) for v in texdata_reflectivity(texdata, record)]
            approximations.setdefault(key, []).append(
                "no base texture: the compiled reflectivity is the colour")
        if record["shader"] not in ("lightmappedgeneric", "worldvertextransition",
                                    "lightmapped_4wayblend"):
            approximations.setdefault(key, []).append(
                "%s drawn as a lit PBR surface" % record["shader"])
        if record["shader"] == "worldvertextransition":
            approximations.setdefault(key, []).append("second blend layer dropped")
        if params.get("$envmap"):
            approximations.setdefault(key, []).append("legacy $envmap not carried")

    # Occluders: the non-bevel nodraw sides of the solid world brushes.
    occluders = []
    normals, _dists = bsp.planes()
    for brush in bsp.world_brushes():
        if not brush["contents"] & (legacy_bsp.CONTENTS_SOLID | legacy_bsp.CONTENTS_OPAQUE):
            continue
        for side, polygon in bsp.brush_side_polygons(brush):
            texinfo_index = brush["sides"][side][1]
            flags = texinfo[texinfo_index]["flags"] if texinfo_index >= 0 else \
                legacy_bsp.SURF_NODRAW
            if flags & (legacy_bsp.SURF_SKY | legacy_bsp.SURF_SKY2D):
                continue
            if flags & legacy_bsp.SURF_NODRAW:
                # Brush planes face out of the brush.
                occluders.append((polygon, normals[brush["sides"][side][0]]))

    # Analytic lights. A switchable style (32+: vbsp gives every light of one
    # targetname one) any of whose lights starts dark is left out whole; the
    # engine ignores the start-dark flag of the preset styles (1-31).
    light_records = []
    left_out = []
    owners = {light["index"]: entity_for_light(entities, light) for light in lights
              if light["type"] in ("point", "spot")}
    dark_styles = starting_dark_styles(lights, owners)
    for light in lights:
        kind = light["type"]
        if kind == "surface":
            if light["style"] in dark_styles:
                raise ValueError("surface light %d has the dark style %d" %
                                 (light["index"], light["style"]))
            continue
        entity = owners.get(light["index"])
        style = light["style"]
        record = {"world_light": light["index"], "type": kind, "style": style,
                  "entity": (entity or {}).get("targetname") or (entity or {}).get("classname"),
                  "hammerid": (entity or {}).get("hammerid")}
        if style in dark_styles:
            record["excluded"] = "style starts dark: stays a world light, not baked"
            left_out.append(record)
            continue
        intensity = np.asarray(light["intensity"], np.float64)
        if kind in ("point", "spot"):
            scale, approximated = falloff_match(light)
            effective = intensity * scale
            record["radiance"] = (effective / LIGHT_RADIUS_UNITS ** 2).tolist()
            record["origin"] = list(light["origin"])
            record["normal"] = list(light["normal"])
            record["radius_units"] = LIGHT_RADIUS_UNITS
            if approximated:
                record["approximation"] = ("constant/linear falloff %s matched at %g units" %
                                           (list(light["attenuation"]), NORMALIZE_DISTANCE))
            if kind == "spot":
                # vrad's cone: cosines of the inner and outer cones.
                inner = float(np.clip(light["stopdot"], -1.0, 1.0))
                record["cone"] = {"inner": inner,
                                  "outer": float(np.clip(light["stopdot2"], -1.0, inner)),
                                  "exponent": max(0.0, float(light["exponent"]))}
            if light["radius"] > 0:
                record["approximation"] = "; ".join(filter(None, [
                    record.get("approximation"), "hard falloff radius %g ignored" %
                    light["radius"]]))
        elif kind == "sky":
            record["irradiance"] = (math.pi * intensity).tolist()
            record["direction"] = list(light["normal"])
            environment = next((e for e in entities
                                if e.get("classname") == "light_environment"), {})
            record["angle_degrees"] = float(environment.get("SunSpreadAngle", 0) or 0)
        elif kind == "sky_ambient":
            record["radiance"] = intensity.tolist()
        else:
            record["excluded"] = "%s lights are not supported" % kind
            left_out.append(record)
            continue
        light_records.append(record)
    if sum(r["type"] == "sky" for r in light_records) > 1 or \
            sum(r["type"] == "sky_ambient" for r in light_records) > 1:
        raise ValueError("more than one sun or sky ambient light")

    # Texture coordinates follow the engine's mapping size, which can differ
    # from the compile-time texdata size (content revised after compiling).
    mappings, mapping_differs = {}, []
    used = {face["texinfo"] for faces_of in meshes.values() for face in faces_of}
    for index in sorted({texinfo[i]["texdata"] for i in used}):
        record = texdata[index]
        try:
            size, source = mapping_size(materials, record["name"])
        except (OSError, ValueError) as error:
            size, source = {"width": record["width"], "height": record["height"]}, \
                "material unreadable (%s): texdata size" % error
        mappings[index] = size
        if (size["width"], size["height"]) != (record["width"], record["height"]):
            mapping_differs.append({"material": record["name"], "source": source,
                                    "texdata": [record["width"], record["height"]],
                                    "mapping": [size["width"], size["height"]]})
    model = {"materials": records, "meshes": meshes, "occluders": occluders,
             "lights": light_records, "texinfo": texinfo, "texdata": texdata,
             "mappings": mappings}
    receipt = {"schema": SCHEMA, "world_light_lump": light_lump,
               "relit_faces": sum(len(v) for v in meshes.values()),
               "excluded_faces": excluded, "materials": len(records),
               "emissive_materials": sorted(k for k, r in records.items() if r["emission"]),
               "occluder_polygons": len(occluders),
               "surface_lights": {"total": len(surface_lights),
                                  "matched": len(surface_lights) - len(unmatched),
                                  "emitting_faces": len(radiance_of)},
               "lights": light_records, "left_out_lights": left_out,
               "switchable_styles": sorted({r["style"] for r in light_records if r["style"]}),
               # The world lights the engine still needs: every other one's
               # light is baked (bsp_worldlights.py --keep-only).
               "kept_world_light_styles": sorted(dark_styles),
               "unbaked_surface_lights": len(unmatched),
               "approximations": approximations, "notes": notes,
               "mapping_differs_from_texdata": mapping_differs,
               "textures": {"%s:%s" % key: value for key, value in decoded.items()}}
    return model, receipt


def texdata_reflectivity(texdata, record):
    for item in texdata:
        if item["name"] == record["source_material"]:
            return item["reflectivity"]
    return (0.5, 0.5, 0.5)


# ------------------------------------------------------------------ USD

def widest_triangulation(polygon, normal):
    """Triangles (index triples, counter-clockwise about `normal`) of a
    convex polygon whose thinnest triangle is as wide as possible.

    vbsp adds vertices along face edges so neighbouring faces meet without
    cracks; a fan from the first corner then makes zero-area slivers through
    those collinear vertices. Among all triangulations of the polygon, this
    picks the one maximizing the smallest triangle altitude (dynamic
    programming over diagonals; ties go to the lowest corner index), so no
    triangle is thinner than the face forces. Triangles facing against
    `normal` score as unusable.
    """
    count = len(polygon)
    if count == 3:
        return [(0, 1, 2)]
    points = np.asarray(polygon, dtype=np.float64)

    def quality(a, b, c):
        cross = np.cross(points[b] - points[a], points[c] - points[a])
        signed = np.dot(cross, normal)
        longest = max(np.linalg.norm(points[b] - points[a]),
                      np.linalg.norm(points[c] - points[b]),
                      np.linalg.norm(points[a] - points[c]))
        return signed / longest if signed > 0 and longest > 0 else -np.inf

    best = {}
    choice = {}
    for span in range(2, count):
        for i in range(count - span):
            j = i + span
            top = None
            for k in range(i + 1, j):
                score = min(quality(i, k, j), best.get((i, k), np.inf),
                            best.get((k, j), np.inf))
                if top is None or score > top:
                    top, choice[(i, j)] = score, k
            best[(i, j)] = top
    triangles = []
    stack = [(0, count - 1)]
    while stack:
        i, j = stack.pop()
        if j - i < 2:
            continue
        k = choice[(i, j)]
        triangles.append((i, k, j))
        stack += [(i, k), (k, j)]
    return sorted(triangles)


def clean_winding(keys):
    """Positions of a winding to keep: consecutive repeats and spikes
    (a b a, where the winding folds back on itself) removed, as vbsp's folded
    windings need; fewer than 3 left means the face has no area."""
    keep = list(range(len(keys)))
    changed = True
    while changed and len(keep) >= 3:
        changed = False
        for index in range(len(keep)):
            before, here, after = (keep[index - 1], keep[index],
                                   keep[(index + 1) % len(keep)])
            if keys[here] == keys[after]:
                del keep[index]
                changed = True
                break
            if keys[before] == keys[after]:
                # Spike: drop the tip and one copy of the base.
                for position in sorted({index, (index + 1) % len(keep)}, reverse=True):
                    del keep[position]
                changed = True
                break
    return keep if len(keep) >= 3 else []


def indexed_triangles(polygons, uvs, plane_normals, vertex_ids=None):
    """Triangulate polygons into one indexed mesh (widest_triangulation).

    Faces share points the way the BSP shares vertices: by BSP vertex index
    when `vertex_ids` gives each polygon's, otherwise by exact float32
    position. Copying every corner instead made each triangle a separate
    piece, which the lightmap bake then charted alone - a seam along every
    triangle edge. Normals and material UVs stay per corner (faceVarying).
    Returns (points, counts, indices, normals, st, sources): sources[i] is the
    polygon triangle i came from (faces with no area give none).
    """
    points, counts, indices, normals, st, sources = [], [], [], [], [], []
    shared = {}
    ids = vertex_ids if vertex_ids is not None else [None] * len(polygons)
    for source, (polygon, uv, normal, corner_ids) in enumerate(
            zip(polygons, uvs, plane_normals, ids)):
        keys = [int(corner_ids[k]) if corner_ids is not None else
                np.asarray(polygon[k], dtype=np.float32).tobytes() for k in range(len(polygon))]
        kept = clean_winding(keys)
        if not kept:
            continue
        polygon, uv = np.asarray(polygon)[kept], np.asarray(uv)[kept]
        corner_ids = [keys[k] for k in kept]
        area = np.zeros(3)
        for i in range(1, len(polygon) - 1):
            area += np.cross(polygon[i] - polygon[0], polygon[i + 1] - polygon[0])
        # Counter-clockwise about the plane normal (USD's front face).
        order = list(range(len(polygon)) if np.dot(area, normal) >= 0 else
                     range(len(polygon) - 1, -1, -1))
        ordered = np.asarray(polygon, dtype=np.float64)[order]
        for triangle in widest_triangulation(ordered, np.asarray(normal, dtype=np.float64)):
            for corner in (order[i] for i in triangle):
                key = corner_ids[corner]
                if key not in shared:
                    shared[key] = len(points)
                    points.append(polygon[corner])
                indices.append(shared[key])
                st.append(uv[corner])
                normals.append(normal)
            counts.append(3)
            sources.append(source)
    return points, counts, indices, normals, st, sources


def write_usd(model, path, map_name):
    from pxr import Gf, Sdf, Usd, UsdGeom, UsdLux, UsdShade, Vt
    if path.exists():
        path.unlink()
    stage = Usd.Stage.CreateNew(str(path))
    stage.SetMetadata("comment", "Relight scene of legacy map %s; generated by "
                      "tools/quality/legacy_bsp_scene.py, do not edit" % map_name)
    UsdGeom.SetStageUpAxis(stage, UsdGeom.Tokens.z)
    UsdGeom.SetStageMetersPerUnit(stage, METERS_PER_UNIT)
    # Every light's style is the map's own: an unstyled light is fixed, and
    # the radiosity transfer must not number the sources itself.
    stage.GetRootLayer().customLayerData = {"sourceEngine:authoredLightStyles": True}
    root = UsdGeom.Xform.Define(stage, "/" + sanitize(map_name))
    stage.SetDefaultPrim(root.GetPrim())
    looks = root.GetPath().AppendChild("Looks")
    UsdGeom.Scope.Define(stage, looks)
    world = root.GetPath().AppendChild("World")
    UsdGeom.Xform.Define(stage, world)
    lights_path = root.GetPath().AppendChild("Lights")
    UsdGeom.Xform.Define(stage, lights_path)
    bound = {}

    def texture(shader_path, name, record, output, colorspace, scale=None, bias=None):
        node = UsdShade.Shader.Define(stage, shader_path.AppendChild(name))
        node.CreateIdAttr("UsdUVTexture")
        node.CreateInput("file", Sdf.ValueTypeNames.Asset).Set(
            Sdf.AssetPath(Path(record["file"]).resolve().as_posix()))
        node.CreateInput("sourceColorSpace", Sdf.ValueTypeNames.Token).Set(colorspace)
        node.CreateInput("wrapS", Sdf.ValueTypeNames.Token).Set("repeat")
        node.CreateInput("wrapT", Sdf.ValueTypeNames.Token).Set("repeat")
        if scale:
            node.CreateInput("scale", Sdf.ValueTypeNames.Float4).Set(Gf.Vec4f(*scale))
            node.CreateInput("bias", Sdf.ValueTypeNames.Float4).Set(Gf.Vec4f(*bias))
        reader = shader_path.AppendChild("st_reader")
        if not stage.GetPrimAtPath(reader):
            primvar = UsdShade.Shader.Define(stage, reader)
            primvar.CreateIdAttr("UsdPrimvarReader_float2")
            primvar.CreateInput("varname", Sdf.ValueTypeNames.Token).Set("st")
            primvar.CreateOutput("result", Sdf.ValueTypeNames.Float2)
        node.CreateInput("st", Sdf.ValueTypeNames.Float2).ConnectToSource(
            UsdShade.Shader(stage.GetPrimAtPath(reader)).ConnectableAPI(), "result")
        return node.CreateOutput(output, Sdf.ValueTypeNames.Float3 if output == "rgb"
                                 else Sdf.ValueTypeNames.Float)

    def material(name, record):
        path = looks.AppendChild(name)
        mat = UsdShade.Material.Define(stage, path)
        surface = UsdShade.Shader.Define(stage, path.AppendChild("Surface"))
        surface.CreateIdAttr("UsdPreviewSurface")
        surface.CreateInput("metallic", Sdf.ValueTypeNames.Float).Set(0.0)
        surface.CreateInput("roughness", Sdf.ValueTypeNames.Float).Set(
            float(record.get("roughness", 1.0)))
        if record.get("base"):
            surface.CreateInput("diffuseColor", Sdf.ValueTypeNames.Color3f).ConnectToSource(
                texture(path, "base", record["base"], "rgb", "sRGB"))
            if record.get("alphatest"):
                surface.CreateInput("opacity", Sdf.ValueTypeNames.Float).ConnectToSource(
                    texture(path, "base", record["base"], "a", "sRGB"))
                surface.CreateInput("opacityThreshold", Sdf.ValueTypeNames.Float).Set(
                    float(record["alphatest_reference"]))
        else:
            surface.CreateInput("diffuseColor", Sdf.ValueTypeNames.Color3f).Set(
                Gf.Vec3f(*record.get("base_color", (0.0, 0.0, 0.0))))
        if record.get("normal"):
            surface.CreateInput("normal", Sdf.ValueTypeNames.Normal3f).ConnectToSource(
                texture(path, "normal", record["normal"], "rgb", "raw",
                        (2.0, 2.0, 2.0, 1.0), (-1.0, -1.0, -1.0, 0.0)))
        if record.get("emission"):
            surface.CreateInput("emissiveColor", Sdf.ValueTypeNames.Color3f).Set(
                Gf.Vec3f(*record["emission"]))
        mat.CreateSurfaceOutput().ConnectToSource(surface.ConnectableAPI(), "surface")
        if record.get("source_material"):
            mat.GetPrim().CreateAttribute("sourceEngine:material", Sdf.ValueTypeNames.String,
                                          custom=True).Set(record["source_material"])
        bound[name] = mat
        return mat

    def mesh(name, polygons, uvs, plane_normals, mat, vertex_ids=None, planes=None):
        prim = UsdGeom.Mesh.Define(stage, world.AppendChild(name))
        points, counts, indices, normals, st, sources = indexed_triangles(
            polygons, uvs, plane_normals, vertex_ids)
        if planes is not None:
            # The BSP plane of every triangle: lightmap_layout.py charts only
            # triangles on one plane together (exact, no angle tolerance).
            UsdGeom.PrimvarsAPI(prim).CreatePrimvar(
                PLANE_PRIMVAR, Sdf.ValueTypeNames.IntArray, UsdGeom.Tokens.uniform).Set(
                Vt.IntArray([int(planes[source]) for source in sources]))
        prim.CreatePointsAttr(Vt.Vec3fArray([Gf.Vec3f(*map(float, p)) for p in points]))
        prim.CreateFaceVertexCountsAttr(Vt.IntArray(counts))
        prim.CreateFaceVertexIndicesAttr(Vt.IntArray(indices))
        prim.CreateNormalsAttr(Vt.Vec3fArray([Gf.Vec3f(*map(float, n)) for n in normals]))
        prim.SetNormalsInterpolation(UsdGeom.Tokens.faceVarying)
        prim.CreateSubdivisionSchemeAttr(UsdGeom.Tokens.none)
        prim.CreateExtentAttr(UsdGeom.PointBased.ComputeExtent(prim.GetPointsAttr().Get()))
        UsdGeom.PrimvarsAPI(prim).CreatePrimvar(
            "st", Sdf.ValueTypeNames.TexCoord2fArray, UsdGeom.Tokens.faceVarying).Set(
            Vt.Vec2fArray([Gf.Vec2f(*map(float, uv)) for uv in st]))
        UsdShade.MaterialBindingAPI.Apply(prim.GetPrim()).Bind(mat)
        return prim

    texinfo = model["texinfo"]
    for name, faces in sorted(model["meshes"].items()):
        mat = material(name, model["materials"][name])
        polygons = [face["points"] for face in faces]
        uvs = [face_st(face["points"], texinfo[face["texinfo"]],
                       model["mappings"][texinfo[face["texinfo"]]["texdata"]]) for face in faces]
        mesh(name, polygons, uvs, [face["plane_normal"] for face in faces], mat,
             [face["vertices"] for face in faces], [face["plane"] for face in faces])
    if model["occluders"]:
        mat = material(OCCLUDER, {"base_color": (0.0, 0.0, 0.0), "roughness": 1.0})
        polygons = [polygon for polygon, _normal in model["occluders"]]
        mesh(OCCLUDER, polygons, [np.zeros((len(p), 2)) for p in polygons],
             [normal for _polygon, normal in model["occluders"]], mat)

    def look_at(prim, origin, direction):
        """Local -Z along `direction`, at `origin`."""
        forward = -np.asarray(direction, np.float64)
        forward /= np.linalg.norm(forward)
        helper = np.array((0.0, 0.0, 1.0)) if abs(forward[2]) < 0.9 else np.array((1.0, 0.0, 0.0))
        x = np.cross(helper, forward)
        x /= np.linalg.norm(x)
        y = np.cross(forward, x)
        matrix = Gf.Matrix4d(*x, 0.0, *y, 0.0, *forward, 0.0, *origin, 1.0)
        UsdGeom.Xformable(prim).AddTransformOp().Set(matrix)

    def colour(light, rgb):
        rgb = np.asarray(rgb, np.float64)
        peak = float(rgb.max())
        light.CreateIntensityAttr(peak)
        light.CreateColorAttr(Gf.Vec3f(*(rgb / peak if peak > 0 else rgb)))
        light.CreateExposureAttr(0.0)

    for record in model["lights"]:
        label = sanitize("%s_%d" % (record.get("entity") or record["type"],
                                    record["world_light"]))
        path = lights_path.AppendChild(label)
        if record["type"] == "point":
            light = UsdLux.SphereLight.Define(stage, path)
            light.CreateRadiusAttr(float(record["radius_units"]))
            light.CreateTreatAsPointAttr(False)
            light.CreateNormalizeAttr(False)
            colour(light, record["radiance"])
            UsdGeom.Xformable(light).AddTranslateOp().Set(Gf.Vec3d(*record["origin"]))
        elif record["type"] == "spot":
            light = UsdLux.DiskLight.Define(stage, path)
            light.CreateRadiusAttr(float(record["radius_units"]))
            light.CreateNormalizeAttr(False)
            colour(light, record["radiance"])
            look_at(light, record["origin"], record["normal"])
            for key, value in (("Inner", "inner"), ("Outer", "outer"), ("Exponent", "exponent")):
                light.GetPrim().CreateAttribute("sourceEngine:cone" + key, Sdf.ValueTypeNames.Float,
                                                custom=True).Set(float(record["cone"][value]))
        elif record["type"] == "sky":
            light = UsdLux.DistantLight.Define(stage, path)
            light.CreateAngleAttr(float(record["angle_degrees"]))
            light.CreateNormalizeAttr(True)
            colour(light, record["irradiance"])
            look_at(light, (0.0, 0.0, 0.0), record["direction"])
        else:
            light = UsdLux.DomeLight.Define(stage, path)
            colour(light, record["radiance"])
        record["prim"] = str(path)
        if record["style"]:
            light.GetPrim().CreateAttribute("sourceEngine:lightStyle", Sdf.ValueTypeNames.Int,
                                            custom=True).Set(int(record["style"]))
    stage.GetRootLayer().Save()


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--bsp", type=Path, required=True)
    parser.add_argument("--runtime", type=Path, required=True,
                        help="installed game runtime (portal/ and hl2/ content)")
    parser.add_argument("--game-dir", type=Path,
                        help="the compile's game directory (vrad -game), searched first")
    parser.add_argument("--map-name", required=True)
    parser.add_argument("--out", type=Path, required=True,
                        help="output directory: scene.usda, textures/, scene-receipt.json")
    args = parser.parse_args()
    bsp = legacy_bsp.LegacyBsp.read(args.bsp)
    resolver = source_content.ContentResolver(str(args.runtime))
    if args.game_dir:
        resolver = GameDirectory(args.game_dir, resolver)
    args.out.mkdir(parents=True, exist_ok=True)
    model, receipt = build_model(bsp, resolver, args.out / "textures")
    write_usd(model, args.out / "scene.usda", args.map_name)
    receipt.update(status="pass", bsp=str(args.bsp.resolve()),
                   bsp_sha256=sha256_bytes(bsp.data), map=args.map_name,
                   exporter_sha256=sha256_bytes(Path(__file__).read_bytes()))
    (args.out / "scene-receipt.json").write_text(json.dumps(receipt, indent=2, sort_keys=True) +
                                                 "\n")
    print(json.dumps({k: receipt[k] for k in ("relit_faces", "materials", "occluder_polygons",
                                              "surface_lights", "switchable_styles",
                                              "excluded_faces")}))


if __name__ == "__main__":
    main()
