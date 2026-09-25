#!/usr/bin/env python3
"""Read an authored OpenUSD scene into the map pipeline's scene model.

This module owns USD scene knowledge for the map pipeline, as `pbrt_scene`
owns PBRT: stage traversal (instance proxies, point instancers, visibility and
purpose), up-axis and unit conversion, mesh triangulation, normals and `st`
selection, `GeomSubset` material splits, `UsdPreviewSurface` material
channels (constants and `UsdUVTexture` inputs with scale/bias, channel,
colour space and `UsdTransform2d`), cameras, and `UsdLux` lights with their
normative units. Unsupported inputs fail loudly or are recorded as
approximations; nothing is silently dropped.

`extract` writes two artifacts that the rest of the pipeline consumes:

  * a `map-scene/v1` JSON scene model (read by `map_scene.parse` with plain
    Python and inside Blender, neither of which has `pxr`), and
  * a normalized stage: Z-up, meters, one material per triangulated mesh with
    faceVarying `st` and normals, world transforms baked, `/root/<name>/<name>`
    layout (the layout Blender's exporter writes for PBRT scenes), and
    `LightQuadNN`/`LightDiskNN` emitter meshes for area lights.

Run with the OpenUSD Python (`pxr`) on PYTHONPATH:

    python3 tools/quality/usd_scene.py extract --scene room.usda \\
        --out-scene scene.json --out-stage stage/room.usdc
    python3 tools/quality/usd_scene.py inventory room.usda
"""

import argparse
import fnmatch
import hashlib
import json
import math
import re
import sys
from pathlib import Path

import numpy as np
from pxr import Gf, Sdf, Usd, UsdGeom, UsdLux, UsdShade, Vt

SCHEMA = "map-scene/v1"
EMITTER_PREFIXES = ("LightQuad", "LightDisk")
DEFAULT_FILM_WIDTH = 1920
# UsdPreviewSurface defaults (UsdPreviewSurface specification).
PREVIEW_DEFAULTS = {"diffuseColor": (0.18, 0.18, 0.18), "emissiveColor": (0.0, 0.0, 0.0),
                    "metallic": 0.0, "roughness": 0.5, "opacity": 1.0,
                    "opacityThreshold": 0.0, "ior": 1.5, "occlusion": 1.0,
                    "clearcoat": 0.0, "clearcoatRoughness": 0.01,
                    "useSpecularWorkflow": 0, "specularColor": (0.0, 0.0, 0.0)}
TEXTURE_CHANNELS = ("base", "roughness", "metallic", "occlusion", "normal", "emission",
                    "opacity")
NAME_LIMIT = 48


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def sanitize(name, lower=False):
    """A Blender/USD/VMT-safe identifier; long names keep a hash suffix."""
    text = re.sub(r"[^A-Za-z0-9_]+", "_", name).strip("_") or "unnamed"
    if lower:
        text = text.lower()
    if text[0].isdigit():
        text = "m_" + text
    if len(text) > NAME_LIMIT:
        text = text[:NAME_LIMIT - 9] + "_" + hashlib.sha1(name.encode()).hexdigest()[:8]
    return text


class Names:
    """Unique names; the first use keeps its stem, repeats get `_iN`."""

    def __init__(self, reserved=()):
        self.used = set(reserved)

    def take(self, stem):
        name, index = stem, 0
        while name.lower() in self.used or name.startswith(EMITTER_PREFIXES):
            index += 1
            name = "%s_i%d" % (stem, index)
        self.used.add(name.lower())
        return name


def up_rotation(stage):
    """3x3 rotation from stage axes to the pipeline's Z-up axes (row vectors)."""
    axis = UsdGeom.GetStageUpAxis(stage)
    if axis == UsdGeom.Tokens.z:
        return np.eye(3)
    if axis == UsdGeom.Tokens.y:
        # (x, y, z) -> (x, -z, y): Y-up right-handed to Z-up right-handed.
        return np.array(((1.0, 0.0, 0.0), (0.0, 0.0, 1.0), (0.0, -1.0, 0.0)))
    raise ValueError("unsupported USD up axis " + str(axis))


class Converter:
    """Stage world space -> pipeline space (Z-up meters)."""

    def __init__(self, stage):
        meters = UsdGeom.GetStageMetersPerUnit(stage)
        if not math.isfinite(meters) or meters <= 0:
            raise ValueError("USD metersPerUnit is invalid")
        self.meters = meters
        self.rotation = up_rotation(stage)  # applied as row @ rotation

    def matrix(self, gf_matrix):
        """Row-vector 4x4 numpy matrix of a Gf world transform, pipeline space."""
        m = np.array(gf_matrix, dtype=np.float64)
        out = np.eye(4)
        out[:3, :3] = m[:3, :3] @ self.rotation
        out[3, :3] = (m[3, :3] @ self.rotation) * self.meters
        out[:3, :3] *= self.meters
        return out

    def points(self, points, matrix):
        return points @ matrix[:3, :3] + matrix[3, :3]

    def directions(self, vectors, matrix):
        linear = matrix[:3, :3]
        result = vectors @ linear
        return result / np.maximum(np.linalg.norm(result, axis=1, keepdims=True), 1e-30)

    def normals(self, normals, matrix):
        inverse_transpose = np.linalg.inv(matrix[:3, :3]).T
        result = normals @ inverse_transpose
        return result / np.maximum(np.linalg.norm(result, axis=1, keepdims=True), 1e-30)


def earliest(attribute):
    if not attribute or not attribute.HasAuthoredValue() and not attribute.HasFallbackValue():
        return None
    return attribute.Get(Usd.TimeCode.EarliestTime())


# ---------------------------------------------------------------- materials

def resolved_asset(attribute_value, context):
    """Absolute path of an asset-valued input, or None."""
    if attribute_value is None:
        return None
    path = getattr(attribute_value, "resolvedPath", "") or ""
    raw = getattr(attribute_value, "path", str(attribute_value))
    if not path:
        if not raw:
            return None
        raise FileNotFoundError("USD asset does not resolve: %s (in %s)" % (raw, context))
    if "[" in path and path.endswith("]"):
        raise ValueError("packaged (.usdz) textures are not supported yet: " + path)
    if "<UDIM>" in path or "<UDIM>" in raw:
        raise ValueError("UDIM textures are not supported: " + raw)
    return str(Path(path).resolve())


def producing_attribute(shade_input):
    """The attribute that supplies an input: a shader output or a value.

    `GetValueProducingAttributes` follows interface inputs and node-graph
    outputs, as renderers do.
    """
    if not shade_input:
        return None, None
    attributes = shade_input.GetValueProducingAttributes()
    if not attributes:
        return None, None
    attribute = attributes[0]
    name, kind = UsdShade.Utils.GetBaseNameAndType(attribute.GetName())
    return attribute, (name if kind == UsdShade.AttributeType.Output else None)


def connected_source(shade_input):
    """(shader, output name) feeding an input, or (None, None)."""
    attribute, output_name = producing_attribute(shade_input)
    if attribute is None or output_name is None:
        return None, None
    return UsdShade.Shader(attribute.GetPrim()), output_name


def input_value(shader, name, default=None):
    """Constant value of a shader input, following interface connections."""
    attribute, output_name = producing_attribute(shader.GetInput(name))
    if attribute is None or output_name is not None:
        return default
    value = attribute.Get()
    return default if value is None else value


def shader_id(shader):
    identifier = shader.GetShaderId()
    return str(identifier) if identifier else ""


def st_transform(shader, notes):
    """Follow a UsdUVTexture `st` input to its primvar and UsdTransform2d."""
    source, _ = connected_source(shader.GetInput("st"))
    transform = {"scale": [1.0, 1.0], "rotation": 0.0, "translation": [0.0, 0.0]}
    if source and shader_id(source) == "UsdTransform2d":
        transform = {"scale": list(map(float, input_value(source, "scale", (1.0, 1.0)))),
                     "rotation": float(input_value(source, "rotation", 0.0)),
                     "translation": list(map(float, input_value(source, "translation",
                                                                 (0.0, 0.0))))}
        source, _ = connected_source(source.GetInput("in"))
    varname = None
    if source and shader_id(source).startswith("UsdPrimvarReader"):
        value = input_value(source, "varname")
        varname = str(value) if value is not None else None
    elif source:
        notes.append("st fed by unsupported shader " + shader_id(source))
    return varname, transform


def texture_input(surface, name, notes, context):
    """A UsdUVTexture feeding `name`: file, channel, scale/bias, colour space."""
    source, output_name = connected_source(surface.GetInput(name))
    if not source:
        return None
    identifier = shader_id(source)
    if identifier != "UsdUVTexture":
        notes.append("%s fed by unsupported shader %s (constant fallback)" % (name, identifier))
        return None
    file_value = input_value(source, "file")
    path = resolved_asset(file_value, context)
    if not path:
        return None
    varname, transform = st_transform(source, notes)
    colorspace = input_value(source, "sourceColorSpace", "auto")
    file_attr = source.GetInput("file").GetAttr() if source.GetInput("file") else None
    if file_attr and file_attr.GetColorSpace():
        colorspace = str(file_attr.GetColorSpace())
    return {"file": path, "channel": str(output_name),
            "scale": list(map(float, input_value(source, "scale", (1.0, 1.0, 1.0, 1.0)))),
            "bias": list(map(float, input_value(source, "bias", (0.0, 0.0, 0.0, 0.0)))),
            "colorspace": str(colorspace),
            "wrap": [str(input_value(source, "wrapS", "useMetadata")),
                     str(input_value(source, "wrapT", "useMetadata"))],
            "fallback": list(map(float, input_value(source, "fallback", (0.0, 0.0, 0.0, 1.0)))),
            "primvar": varname, "transform": transform}


def default_summary(name, color, source, approximation):
    return {"name": name, "pbrt_type": "usdpreviewsurface", "source": source,
            "base_color": tuple(float(c) for c in color), "base_texture": None,
            "base_scale": 1.0, "metallic": 0.0, "roughness": 0.5, "coat_roughness": None,
            "coat_internal_reflectance": None, "diffuse_transmittance": None,
            "transmission": 0.0, "ior": 1.5, "emission_color": None, "opacity": 1.0,
            "opacity_threshold": 0.0, "clearcoat": 0.0, "clearcoat_roughness": 0.01,
            "textures": {}, "primvar": "st",
            "st_transform": None, "approximation": approximation}


def preview_summary(name, material):
    """Reduce a bound material to the pipeline's metal/roughness model."""
    path = str(material.GetPath())
    surface = material.ComputeSurfaceSource()[0]
    if not surface or shader_id(surface) != "UsdPreviewSurface":
        for context in ("mtlx", "mdl"):
            other = material.ComputeSurfaceSource([context])[0]
            if other:
                identifier = shader_id(other) or context
                return default_summary(name, PREVIEW_DEFAULTS["diffuseColor"], path,
                                       "unsupported %s surface %s; neutral grey fallback" %
                                       (context, identifier))
        return default_summary(name, PREVIEW_DEFAULTS["diffuseColor"], path,
                               "material has no UsdPreviewSurface; neutral grey fallback")
    notes = []
    textures = {}
    for channel, input_name in (("base", "diffuseColor"), ("roughness", "roughness"),
                                ("metallic", "metallic"), ("occlusion", "occlusion"),
                                ("normal", "normal"), ("emission", "emissiveColor"),
                                ("opacity", "opacity")):
        texture = texture_input(surface, input_name, notes, path)
        if texture:
            textures[channel] = texture

    def constant(input_name):
        value = input_value(surface, input_name, PREVIEW_DEFAULTS[input_name])
        return tuple(map(float, value)) if isinstance(value, (Gf.Vec3f, Gf.Vec3d, tuple, list)) \
            else float(value)
    summary = default_summary(name, constant("diffuseColor"), path, None)
    summary["metallic"] = constant("metallic")
    summary["roughness"] = constant("roughness")
    summary["opacity"] = constant("opacity")
    summary["opacity_threshold"] = constant("opacityThreshold")
    summary["ior"] = constant("ior")
    emission = constant("emissiveColor")
    summary["emission_color"] = emission if max(emission) > 0 or "emission" in textures else None
    if "base" in textures:
        summary["base_texture"] = textures["base"]["file"]
        summary["base_color"] = None
    if int(constant("useSpecularWorkflow")):
        notes.append("specular workflow reduced to metallic 0 with the authored roughness")
        summary["metallic"] = 0.0
        textures.pop("metallic", None)
    summary["clearcoat"] = constant("clearcoat")
    summary["clearcoat_roughness"] = constant("clearcoatRoughness")
    if surface.GetInput("displacement") and surface.GetInput("displacement").HasConnectedSource():
        notes.append("displacement ignored")
    if summary["opacity"] < 1.0 and summary["opacity_threshold"] <= 0 and \
            "opacity" not in textures:
        # A uniformly partial opacity is a thin transmissive sheet.
        summary["transmission"] = 1.0 - summary["opacity"]
        notes.append("uniform opacity %.2f treated as thin transmission" % summary["opacity"])
        if summary["opacity"] <= 0.0:
            # Fully transparent: no diffuse lobe, and UsdPreviewSurface does
            # not tint transmitted light by diffuseColor. The Principled/VMT
            # transmission tint is the base colour, so it must be white (as
            # PBRT dielectrics are); a black diffuseColor would be opaque.
            if summary["base_color"] != (1.0, 1.0, 1.0) or "base" in textures:
                notes.append("diffuseColor unused at opacity 0; transmission is untinted")
            summary["base_color"] = (1.0, 1.0, 1.0)
            textures.pop("base", None)
    elif "opacity" in textures and summary["opacity_threshold"] <= 0:
        summary["opacity_threshold"] = 0.5
        notes.append("textured opacity without opacityThreshold alpha-tested at 0.5")
    transforms = {json.dumps(t["transform"], sort_keys=True) for t in textures.values()}
    primvars = {t["primvar"] for t in textures.values() if t["primvar"]}
    if len(transforms) > 1:
        notes.append("textures use different st transforms; the base colour's is applied")
    if len(primvars) > 1:
        notes.append("textures read different primvars; %s is used" % sorted(primvars)[0])
    first = textures.get("base") or next(iter(textures.values()), None)
    if first:
        summary["st_transform"] = first["transform"]
        summary["primvar"] = first["primvar"] or "st"
    summary["textures"] = textures
    summary["approximation"] = "; ".join(notes) or None
    return summary


# ------------------------------------------------------------------ meshes

def primvar_corners(primvar, counts, indices, point_count):
    """Expand a primvar to one value per face corner (faceVarying order)."""
    values = primvar.ComputeFlattened(Usd.TimeCode.EarliestTime())
    if values is None:
        return None
    values = np.asarray(values, dtype=np.float64)
    interpolation = primvar.GetInterpolation()
    corners = len(indices)
    if interpolation == UsdGeom.Tokens.faceVarying and len(values) == corners:
        return values
    if interpolation in (UsdGeom.Tokens.vertex, UsdGeom.Tokens.varying) and \
            len(values) >= point_count:
        return values[indices]
    if interpolation == UsdGeom.Tokens.uniform and len(values) == len(counts):
        return np.repeat(values, counts, axis=0)
    if interpolation == UsdGeom.Tokens.constant and len(values) >= 1:
        return np.repeat(values[:1], corners, axis=0)
    raise ValueError("primvar %s has %d values for %s interpolation" %
                     (primvar.GetPrimvarName(), len(values), interpolation))


def smooth_vertex_normals(points, counts, indices):
    """Area-weighted (Newell) vertex normals, expanded to corners."""
    triangles, _ = triangulate(points, counts, indices)
    corners = points[indices[triangles]]
    area_normals = np.cross(corners[:, 1] - corners[:, 0], corners[:, 2] - corners[:, 0])
    normals = np.zeros_like(points)
    for k in range(3):
        np.add.at(normals, indices[triangles[:, k]], area_normals)
    lengths = np.linalg.norm(normals, axis=1, keepdims=True)
    normals = np.divide(normals, lengths, out=np.zeros_like(normals), where=lengths > 0)
    return normals[indices]


def triangulate(points, counts, indices):
    """Corner indices (into the faceVarying corner list) of triangles, plus face ids.

    Triangles and convex polygons fan from their first corner; polygons whose
    fan would fold (concave) are ear-clipped in their Newell plane.
    """
    counts = np.asarray(counts)
    starts = np.concatenate(([0], np.cumsum(counts)[:-1]))
    triangles, faces = [], []
    for size in np.unique(counts):
        if size < 3:
            continue
        selected = np.nonzero(counts == size)[0]
        first = starts[selected]
        fan = np.stack([np.stack((first, first + k, first + k + 1), axis=1)
                        for k in range(1, size - 1)], axis=1)  # (faces, size-2, 3)
        if size > 3:
            corners = points[indices[first[:, None] + np.arange(size)]]  # (faces, size, 3)
            shifted = np.roll(corners, -1, axis=1)
            newell = np.stack((
                np.sum((corners[..., 1] - shifted[..., 1]) * (corners[..., 2] + shifted[..., 2]), 1),
                np.sum((corners[..., 2] - shifted[..., 2]) * (corners[..., 0] + shifted[..., 0]), 1),
                np.sum((corners[..., 0] - shifted[..., 0]) * (corners[..., 1] + shifted[..., 1]), 1)),
                axis=1)
            previous = np.roll(corners, 1, axis=1)
            turns = np.einsum("fkc,fc->fk", np.cross(corners - previous, shifted - corners),
                              newell)
            concave = np.nonzero((turns < -1e-12 * np.abs(turns).max(initial=1.0)).any(axis=1))[0]
            for face in concave:
                start = first[face]
                clipped = [[start + a, start + b, start + c]
                           for a, b, c in ear_clip(corners[face])][:size - 2]
                # Degenerate fillers are dropped below.
                fan[face] = clipped + [[start, start, start]] * (size - 2 - len(clipped))
        triangles.append(fan.reshape(-1, 3))
        faces.append(np.repeat(selected, size - 2))
    if not triangles:
        return np.zeros((0, 3), dtype=np.int64), np.zeros(0, dtype=np.int64)
    triangles = np.concatenate(triangles)
    faces = np.concatenate(faces)
    keep = ~((triangles[:, 0] == triangles[:, 1]) & (triangles[:, 1] == triangles[:, 2]))
    order = np.argsort(faces[keep], kind="stable")
    return triangles[keep][order].astype(np.int64), faces[keep][order]


def ear_clip(polygon):
    count = len(polygon)
    shifted = np.roll(polygon, -1, axis=0)
    normal = np.array([
        np.sum((polygon[:, 1] - shifted[:, 1]) * (polygon[:, 2] + shifted[:, 2])),
        np.sum((polygon[:, 2] - shifted[:, 2]) * (polygon[:, 0] + shifted[:, 0])),
        np.sum((polygon[:, 0] - shifted[:, 0]) * (polygon[:, 1] + shifted[:, 1]))])
    length = np.linalg.norm(normal)
    fan = [(0, k, k + 1) for k in range(1, count - 1)]
    if length < 1e-20:
        return fan
    normal /= length

    def cross_z(a, b, c):
        return float(np.dot(np.cross(b - a, c - a), normal))
    if all(cross_z(polygon[0], polygon[k], polygon[k + 1]) >= 0 for k in range(1, count - 1)) \
            and all(cross_z(polygon[k - 1], polygon[k], polygon[(k + 1) % count]) >= -1e-12
                    for k in range(count)):
        return fan
    remaining = list(range(count))
    result = []
    guard = 0
    while len(remaining) > 3 and guard < count * count:
        guard += 1
        clipped = False
        for i in range(len(remaining)):
            a, b, c = remaining[i - 1], remaining[i], remaining[(i + 1) % len(remaining)]
            if cross_z(polygon[a], polygon[b], polygon[c]) <= 1e-14:
                continue
            inside = False
            for other in remaining:
                if other in (a, b, c):
                    continue
                p = polygon[other]
                if (cross_z(polygon[a], polygon[b], p) >= 0 and
                        cross_z(polygon[b], polygon[c], p) >= 0 and
                        cross_z(polygon[c], polygon[a], p) >= 0):
                    inside = True
                    break
            if not inside:
                result.append((a, b, c))
                remaining.pop(i)
                clipped = True
                break
        if not clipped:
            break
    if len(remaining) == 3:
        result.append(tuple(remaining))
        return result
    return fan


def cluster_simplify(points, corner_triangles, cell):
    """Vertex-clustering simplification for a manifest-selected mesh: snap
    every used corner to a `cell`-metre grid cell, move it to the mean of its
    cell's corner positions, and drop triangles that collapse or repeat
    another triangle of the same cells and winding. Corner normals and UVs
    are kept, so texturing and shading follow the original surface.
    Returns (corner_triangles, points) with `points` a modified copy."""
    used = np.unique(corner_triangles)
    keys = np.floor(points[used] / cell).astype(np.int64)
    _, cluster = np.unique(keys, axis=0, return_inverse=True)
    cluster = cluster.reshape(-1)
    means = np.zeros((cluster.max() + 1, 3))
    np.add.at(means, cluster, points[used])
    means /= np.bincount(cluster)[:, None]
    corner_cluster = np.full(len(points), -1, dtype=np.int64)
    corner_cluster[used] = cluster
    cells = corner_cluster[corner_triangles]
    keep = ((cells[:, 0] != cells[:, 1]) & (cells[:, 1] != cells[:, 2]) &
            (cells[:, 0] != cells[:, 2]))
    triangles, cells = corner_triangles[keep], cells[keep]
    # Rotate each cell triple to start at its smallest cell: same winding,
    # one key per oriented triangle, so two-sided shells keep both sides.
    rotation = np.argmin(cells, axis=1)
    order = (rotation[:, None] + np.arange(3)[None, :]) % 3
    rotated = np.take_along_axis(cells, order, axis=1)
    _, first = np.unique(rotated, axis=0, return_index=True)
    simplified = points.copy()
    simplified[used] = means[cluster]
    return triangles[np.sort(first)], simplified


def apply_st_transform(uv, transform):
    """UsdTransform2d: scale, then rotate (degrees, CCW), then translate."""
    if not transform:
        return uv
    scale = np.asarray(transform["scale"], dtype=np.float64)
    angle = math.radians(transform["rotation"])
    rotation = np.array(((math.cos(angle), -math.sin(angle)),
                         (math.sin(angle), math.cos(angle))))
    return (uv * scale) @ rotation.T + np.asarray(transform["translation"])


def mesh_st(prim, preferred, counts, indices, point_count):
    api = UsdGeom.PrimvarsAPI(prim)
    names = [preferred] if preferred else []
    names += ["st", "st0", "UVMap", "uv", "map1", "UVW"]
    for name in names:
        primvar = api.GetPrimvar(name) if name else None
        if primvar and primvar.HasValue():
            return name, primvar_corners(primvar, counts, indices, point_count)[:, :2]
    for primvar in api.GetPrimvarsWithValues():
        if primvar.GetTypeName() in (Sdf.ValueTypeNames.TexCoord2fArray,
                                     Sdf.ValueTypeNames.Float2Array,
                                     Sdf.ValueTypeNames.TexCoord2dArray):
            return primvar.GetPrimvarName(), primvar_corners(primvar, counts, indices,
                                                             point_count)[:, :2]
    return None, None


def display_color(prim, counts, indices, point_count):
    primvar = UsdGeom.PrimvarsAPI(prim).GetPrimvar("displayColor")
    if not primvar or not primvar.HasValue():
        return None
    values = primvar_corners(primvar, counts, indices, point_count)
    return tuple(float(v) for v in values.mean(axis=0)) if len(values) else None


# --------------------------------------------------------------- traversal

class Extractor:
    def __init__(self, stage, source, simplify=()):
        self.stage = stage
        self.source = source
        # (prim path pattern, cell metres) rules; each must match a mesh.
        self.simplify = [(pattern, float(cell)) for pattern, cell in simplify]
        self.simplify_used = set()
        self.convert = Converter(stage)
        self.time = Usd.TimeCode.EarliestTime()
        self.xforms = UsdGeom.XformCache(self.time)
        self.shape_names = Names()
        self.material_names = Names()
        self.materials = {}          # material prim path -> summary name
        self.summaries = {}          # name -> summary
        self.shapes = []             # dicts with geometry arrays
        self.emitters = []
        self.distant = []
        self.props = {}              # prop prim path -> dynamic model placement
        self.environment = None
        self.notes = []
        self.skipped = {"invisible": 0, "guide_or_proxy": 0, "empty": 0}
        self.textures = set()
        self.exposure_scale = 1.0

    # materials -----------------------------------------------------------
    def material_name(self, material, prim, counts, indices, point_count):
        if material:
            key = str(material.GetPath())
            if key not in self.materials:
                name = self.material_names.take(sanitize(material.GetPrim().GetName(), True))
                self.materials[key] = name
                summary = preview_summary(name, material)
                self.summaries[name] = summary
                self.textures.update(t["file"] for t in summary["textures"].values())
            return self.materials[key]
        color = display_color(prim, counts, indices, point_count) or \
            PREVIEW_DEFAULTS["diffuseColor"]
        key = "displayColor:%.3f,%.3f,%.3f" % color
        if key not in self.materials:
            stem = "displaycolor_%02x%02x%02x" % tuple(
                min(255, round(max(0.0, c) * 255)) for c in color)
            name = self.material_names.take(stem)
            self.materials[key] = name
            self.summaries[name] = default_summary(
                name, color, "displayColor", "unbound mesh uses its displayColor")
        return self.materials[key]

    # meshes --------------------------------------------------------------
    def add_mesh(self, prim, world, stem):
        mesh = UsdGeom.Mesh(prim)
        points = earliest(mesh.GetPointsAttr())
        counts = earliest(mesh.GetFaceVertexCountsAttr())
        indices = earliest(mesh.GetFaceVertexIndicesAttr())
        if not points or not counts or not indices:
            self.skipped["empty"] += 1
            return
        points = np.asarray(points, dtype=np.float64)
        counts = np.asarray(counts, dtype=np.int64)
        indices = np.asarray(indices, dtype=np.int64)
        if counts.sum() != len(indices) or indices.min() < 0 or indices.max() >= len(points):
            raise ValueError("USD mesh has inconsistent topology: " + str(prim.GetPath()))
        binding = UsdShade.MaterialBindingAPI(prim)
        face_material = [None] * len(counts)
        mesh_material = binding.ComputeBoundMaterial(UsdShade.Tokens.full)[0]
        for subset in binding.GetMaterialBindSubsets():
            material = UsdShade.MaterialBindingAPI(subset.GetPrim()).ComputeBoundMaterial(
                UsdShade.Tokens.full)[0]
            for face in earliest(subset.GetIndicesAttr()) or []:
                if 0 <= face < len(counts):
                    face_material[face] = (subset.GetPrim().GetName(), material)
        groups = {}
        for face, entry in enumerate(face_material):
            subset_name, material = entry if entry else (None, mesh_material)
            key = (subset_name, str(material.GetPath()) if material else None)
            groups.setdefault(key, (subset_name, material, []))[2].append(face)
        orientation = mesh.GetOrientationAttr().Get()
        flip = (orientation == UsdGeom.Tokens.leftHanded) != (np.linalg.det(world[:3, :3]) < 0)
        normals_primvar = UsdGeom.PrimvarsAPI(prim).GetPrimvar("normals")
        if normals_primvar and normals_primvar.HasValue():
            normals = primvar_corners(normals_primvar, counts, indices, len(points))
        elif mesh.GetNormalsAttr().HasAuthoredValue():
            values = np.asarray(earliest(mesh.GetNormalsAttr()), dtype=np.float64)
            interpolation = mesh.GetNormalsInterpolation()
            if interpolation == UsdGeom.Tokens.faceVarying:
                normals = values
            elif interpolation == UsdGeom.Tokens.uniform:
                normals = np.repeat(values, counts, axis=0)
            else:
                normals = values[indices]
        else:
            normals = smooth_vertex_normals(points, counts, indices)
        if len(normals) != len(indices):
            raise ValueError("USD mesh normals do not cover its corners: " + str(prim.GetPath()))
        world_points = self.convert.points(points, world)
        world_normals = self.convert.normals(normals, world)
        triangles, triangle_faces = triangulate(points, counts, indices)
        if flip:
            triangles = triangles[:, ::-1]
        double_sided = bool(mesh.GetDoubleSidedAttr().Get())
        subdivision = mesh.GetSubdivisionSchemeAttr().Get()
        for (subset_name, material, faces) in groups.values():
            name_stem = sanitize(stem + ("_" + subset_name if subset_name else ""))
            material_name = self.material_name(material, prim, counts, indices, len(points))
            summary = self.summaries[material_name]
            primvar_name, uv = mesh_st(prim, summary["primvar"], counts, indices, len(points))
            if uv is None:
                uv = np.zeros((len(indices), 2))
                self.notes.append("%s has no st primvar; material UVs are zero" %
                                  prim.GetPath())
            uv = apply_st_transform(uv, summary["st_transform"])
            selected = np.isin(triangle_faces, np.asarray(faces))
            corner_triangles = triangles[selected]
            if not len(corner_triangles):
                continue
            corner_points = world_points[indices]
            for pattern, cell in self.simplify:
                if fnmatch.fnmatchcase(str(prim.GetPath()), pattern):
                    self.simplify_used.add(pattern)
                    before = len(corner_triangles)
                    corner_triangles, corner_points = cluster_simplify(
                        corner_points, corner_triangles, cell)
                    self.notes.append("%s simplified by %g m vertex clustering: %d -> %d "
                                      "triangles" % (prim.GetPath(), cell, before,
                                                     len(corner_triangles)))
                    break
            prop = self.prop_for(prim)
            self.shapes.append({
                "name": self.shape_names.take(name_stem), "material": material_name,
                "role": "prop" if prop else "world", "prop": prop,
                "prim": str(prim.GetPath()), "subset": subset_name,
                "double_sided": double_sided, "subdivision": str(subdivision),
                "st_primvar": primvar_name,
                "corner_triangles": corner_triangles, "points": corner_points,
                "normals": world_normals, "uv": uv})

    def prop_for(self, prim):
        """Name of the dynamic model (`sourceEngine:model` Xform) a mesh
        belongs to, or None for world geometry. The mesh is the model's
        Cycles stand-in; the placement is recorded once per prop."""
        current = prim
        while current and not current.IsPseudoRoot():
            attribute = current.GetAttribute("sourceEngine:model")
            if attribute and attribute.HasAuthoredValue():
                key = str(current.GetPath())
                if key not in self.props:
                    local_to_stage = self.xforms.GetLocalToWorldTransform(current)
                    # Upright in its stage (translation only), so it is upright
                    # in the Z-up pipeline frame once the stage axes convert.
                    if not np.allclose(np.array(local_to_stage)[:3, :3], np.eye(3), atol=1e-6):
                        raise ValueError("dynamic model %s must be translated only" % key)
                    world = self.convert.matrix(local_to_stage)
                    self.props[key] = {"name": sanitize(current.GetName()),
                                       "prim": key, "model": str(attribute.Get()),
                                       "origin_m": world[3, :3].tolist()}
                return self.props[key]["name"]
            current = current.GetParent()
        return None

    def add_point_instancer(self, prim, instancer_world):
        instancer = UsdGeom.PointInstancer(prim)
        transforms = instancer.ComputeInstanceTransformsAtTime(
            self.time, self.time, UsdGeom.PointInstancer.IncludeProtoXform)
        proto_indices = earliest(instancer.GetProtoIndicesAttr()) or []
        prototypes = instancer.GetPrototypesRel().GetTargets()
        count = 0
        for index, (proto, local) in enumerate(zip(proto_indices, transforms or [])):
            if proto < 0 or proto >= len(prototypes):
                continue
            root = self.stage.GetPrimAtPath(prototypes[proto])
            instance_world = np.array(local) @ instancer_world
            root_world_inverse = np.linalg.inv(np.array(
                self.xforms.GetLocalToWorldTransform(root)))
            for child in Usd.PrimRange(root, Usd.TraverseInstanceProxies()):
                if not child.IsA(UsdGeom.Mesh) or not self.visible_below(child, root):
                    continue
                # The mesh in its prototype root's frame; `local` already
                # includes the root's own transform (IncludeProtoXform).
                relative = np.array(self.xforms.GetLocalToWorldTransform(child)) @ \
                    root_world_inverse
                self.add_mesh(child, self.convert.matrix(Gf.Matrix4d(
                    (relative @ instance_world).tolist())),
                    sanitize("%s_%s_%d" % (prim.GetName(), child.GetName(), index)))
                count += 1
        if count > 20000:
            raise ValueError("point instancer %s expands to %d meshes" % (prim.GetPath(), count))

    def visible_below(self, prim, root):
        """Prototype visibility: only opinions at or below the prototype root
        count, because instancers commonly hide prototypes in place."""
        current = prim
        while True:
            imageable = UsdGeom.Imageable(current)
            if imageable.GetVisibilityAttr().Get(self.time) == UsdGeom.Tokens.invisible or \
                    imageable.GetPurposeAttr().Get() in (UsdGeom.Tokens.guide,
                                                         UsdGeom.Tokens.proxy):
                self.skipped["invisible"] += 1
                return False
            if current == root:
                return True
            current = current.GetParent()

    def visible(self, prim):
        imageable = UsdGeom.Imageable(prim)
        if imageable.ComputeVisibility(self.time) == UsdGeom.Tokens.invisible:
            self.skipped["invisible"] += 1
            return False
        if imageable.ComputePurpose() in (UsdGeom.Tokens.guide, UsdGeom.Tokens.proxy):
            self.skipped["guide_or_proxy"] += 1
            return False
        return True

    # lights --------------------------------------------------------------
    def light_radiance(self, light, area):
        api = UsdLux.LightAPI(light)
        intensity = float(api.GetIntensityAttr().Get(self.time) or 0.0)
        exposure = float(api.GetExposureAttr().Get(self.time) or 0.0)
        color = np.asarray(api.GetColorAttr().Get(self.time) or (1.0, 1.0, 1.0))
        if api.GetEnableColorTemperatureAttr().Get(self.time):
            color = color * np.asarray(UsdLux.BlackbodyTemperatureAsRgb(
                float(api.GetColorTemperatureAttr().Get(self.time))))
        radiance = color * intensity * 2.0 ** exposure
        if api.GetNormalizeAttr().Get(self.time) and area:
            radiance = radiance / area
        return radiance, bool(api.GetNormalizeAttr().Get(self.time))

    def add_area_light(self, prim, world):
        kind = prim.GetTypeName()
        if kind == "RectLight":
            light = UsdLux.RectLight(prim)
            width = float(light.GetWidthAttr().Get(self.time))
            height = float(light.GetHeightAttr().Get(self.time))
            if light.GetTextureFileAttr().Get(self.time):
                self.notes.append("%s texture ignored" % prim.GetPath())
            local = np.array(((-width / 2, -height / 2, 0), (width / 2, -height / 2, 0),
                              (width / 2, height / 2, 0), (-width / 2, height / 2, 0)))
            # Emits toward local -Z: wind so the geometric normal is -Z.
            faces = [(0, 2, 1), (0, 3, 2)]
            prefix = "LightQuad"
        elif kind == "DiskLight":
            radius = float(UsdLux.DiskLight(prim).GetRadiusAttr().Get(self.time))
            local = np.array([(0.0, 0.0, 0.0)] + [
                (radius * math.cos(2 * math.pi * k / 32), radius * math.sin(2 * math.pi * k / 32),
                 0.0) for k in range(32)])
            faces = [(0, 1 + (k + 1) % 32, 1 + k) for k in range(32)]
            prefix = "LightDisk"
        elif kind == "SphereLight":
            light = UsdLux.SphereLight(prim)
            radius = float(light.GetRadiusAttr().Get(self.time))
            if light.GetTreatAsPointAttr().Get(self.time) or radius <= 0:
                radius = 0.01 / self.convert.meters
                self.notes.append("%s treated as a 1 cm sphere" % prim.GetPath())
            local, faces = uv_sphere(radius, 16, 8)
            prefix = "LightQuad"
        else:
            raise ValueError("unsupported area light " + kind)
        world_points = self.convert.points(local, world)
        area = sum(0.5 * np.linalg.norm(np.cross(world_points[b] - world_points[a],
                                                 world_points[c] - world_points[a]))
                   for a, b, c in faces)
        # Normalization divides by the light's area in the stage's own units.
        radiance, normalized = self.light_radiance(
            prim, area / (self.convert.meters ** 2))
        one_sided = kind != "SphereLight"
        if UsdLux.ShapingAPI(prim) and prim.HasAPI(UsdLux.ShapingAPI):
            self.notes.append("%s shaping (cone/IES) ignored" % prim.GetPath())
        self.emitters.append({
            "kind": "trianglemesh", "space": "stage", "prefix": prefix,
            "source": str(prim.GetPath()), "light_type": kind, "points": world_points.tolist(),
            "indices": [v for face in faces for v in face],
            "world_from_object": np.eye(4).tolist(),
            "emission": {"radiance": radiance.tolist(), "scale": 1.0,
                         "two_sided": not one_sided, "one_sided": one_sided},
            "normalized": normalized, "area_m2": float(area)})

    def add_distant_light(self, prim, world):
        light = UsdLux.DistantLight(prim)
        angle = float(light.GetAngleAttr().Get(self.time) or 0.0)
        radiance, normalized = self.light_radiance(prim, None)
        theta = min(max(math.radians(angle) / 2, 0.0), math.pi)
        if normalized or theta == 0:
            irradiance = radiance
        elif theta <= math.pi / 2:
            irradiance = radiance * math.pi * math.sin(theta) ** 2
        else:
            irradiance = radiance * (2 - math.sin(theta) ** 2) * math.pi
        direction = self.convert.directions(np.array(((0.0, 0.0, -1.0),)), world)[0]
        self.distant.append({"source": str(prim.GetPath()), "direction": direction.tolist(),
                             "irradiance": np.asarray(irradiance).tolist(),
                             "angle_degrees": angle})

    def add_dome_light(self, prim, world):
        if self.environment:
            raise ValueError("more than one DomeLight: %s and %s" %
                             (self.environment["source"], prim.GetPath()))
        radiance, _ = self.light_radiance(prim, None)
        texture_attr = prim.GetAttribute("inputs:texture:file")
        texture = resolved_asset(texture_attr.Get(self.time), str(prim.GetPath())) \
            if texture_attr and texture_attr.Get(self.time) else None
        texture_format = prim.GetAttribute("inputs:texture:format")
        texture_format = str(texture_format.Get(self.time)) if texture_format and \
            texture_format.Get(self.time) else "automatic"
        if texture_format not in ("automatic", "latlong"):
            raise ValueError("DomeLight texture format %s is not supported" % texture_format)
        # Dome local axes: +Y is the pole for DomeLight and poleAxis Y; poleAxis
        # Z (or "scene" on a Z-up stage) makes +Z the pole with longitude 0 at -Y.
        pole = "Y"
        if prim.GetTypeName() == "DomeLight_1":
            axis = str(prim.GetAttribute("poleAxis").Get() or "scene")
            if axis == "Z" or (axis == "scene" and
                               UsdGeom.GetStageUpAxis(self.stage) == UsdGeom.Tokens.z):
                pole = "Z"
        rotation = world[:3, :3] / np.linalg.norm(world[:3, :3], axis=1, keepdims=True)
        self.environment = {"kind": "latlong" if texture else "constant",
                            "source": str(prim.GetPath()), "filename": texture,
                            "radiance": radiance.tolist(), "pole": pole,
                            # Row vectors: pipeline direction = local @ rotation.
                            "local_to_pipeline": rotation.tolist(), "scale": 1.0}
        if texture:
            self.textures.add(texture)

    # camera --------------------------------------------------------------
    def camera(self):
        cameras = [prim for prim in self.stage.Traverse() if prim.IsA(UsdGeom.Camera)]
        chosen = None
        for prim in self.stage.Traverse():
            if prim.GetTypeName() == "RenderSettings":
                targets = prim.GetRelationship("camera").GetTargets() \
                    if prim.GetRelationship("camera") else []
                resolution = prim.GetAttribute("resolution").Get() \
                    if prim.GetAttribute("resolution") else None
                if targets:
                    chosen = self.stage.GetPrimAtPath(targets[0])
                if resolution:
                    self.film = {"width": int(resolution[0]), "height": int(resolution[1])}
                break
        chosen = chosen or (sorted(cameras, key=lambda p: str(p.GetPath()))[0]
                            if cameras else None)
        if not chosen:
            return None
        camera = UsdGeom.Camera(chosen)
        # Photographic exposure (exposure, exposure:iso/time/fStop/responsivity)
        # scales the image; `run` folds it into the UsdLux light units.
        self.exposure_scale = float(camera.ComputeLinearExposureScale(self.time))
        world = self.convert.matrix(self.xforms.GetLocalToWorldTransform(chosen))
        eye = self.convert.points(np.zeros((1, 3)), world)[0]
        forward = self.convert.directions(np.array(((0.0, 0.0, -1.0),)), world)[0]
        up = self.convert.directions(np.array(((0.0, 1.0, 0.0),)), world)[0]
        focal = float(camera.GetFocalLengthAttr().Get(self.time))
        horizontal = float(camera.GetHorizontalApertureAttr().Get(self.time))
        vertical = float(camera.GetVerticalApertureAttr().Get(self.time))
        if not hasattr(self, "film"):
            self.film = {"width": DEFAULT_FILM_WIDTH,
                         "height": max(16, round(DEFAULT_FILM_WIDTH * vertical / horizontal))}
        aspect = self.film["width"] / self.film["height"]
        # Conform the aperture to the film (UsdGeom "fit": horizontal/vertical
        # apertures whose ratio differs from the image are cropped by the
        # renderer; use the axis that the image fills).
        if horizontal / vertical > aspect:
            horizontal = vertical * aspect
        else:
            vertical = horizontal / aspect
        shorter = min(horizontal, vertical)
        fov = math.degrees(2 * math.atan(shorter / (2 * focal)))
        return {"source": str(chosen.GetPath()), "fov_degrees": fov,
                "pose": {"eye": eye.tolist(), "forward": forward.tolist(), "up": up.tolist()}}

    def synthesized_camera(self, bounds):
        low, high = bounds
        center = (low + high) / 2
        extent = high - low
        axis = 0 if extent[0] >= extent[1] else 1
        eye = center.copy()
        eye[2] = low[2] + min(1.6, extent[2] * 0.5)
        eye[axis] = low[axis] + extent[axis] * 0.15
        forward = np.zeros(3)
        forward[axis] = 1.0
        self.film = {"width": DEFAULT_FILM_WIDTH, "height": 1080}
        self.notes.append("no USD camera: synthesized a 60 degree camera at eye height")
        return {"source": "synthesized", "fov_degrees": 60.0,
                "pose": {"eye": eye.tolist(), "forward": forward.tolist(),
                         "up": [0.0, 0.0, 1.0]}}

    # traversal -----------------------------------------------------------
    def run(self):
        iterator = iter(Usd.PrimRange(self.stage.GetPseudoRoot(), Usd.TraverseInstanceProxies()))
        for prim in iterator:
            if prim.IsA(UsdGeom.Imageable) and not self.visible(prim):
                iterator.PruneChildren()
                continue
            if prim.IsA(UsdGeom.PointInstancer):
                iterator.PruneChildren()
                self.add_point_instancer(prim, np.array(
                    self.xforms.GetLocalToWorldTransform(prim)))
                continue
            world = None
            type_name = prim.GetTypeName()
            if prim.IsA(UsdGeom.Mesh):
                world = self.convert.matrix(self.xforms.GetLocalToWorldTransform(prim))
                self.add_mesh(prim, world, sanitize(prim.GetName()))
            elif type_name in ("RectLight", "DiskLight", "SphereLight"):
                self.add_area_light(prim, self.convert.matrix(
                    self.xforms.GetLocalToWorldTransform(prim)))
            elif type_name == "DistantLight":
                self.add_distant_light(prim, self.convert.matrix(
                    self.xforms.GetLocalToWorldTransform(prim)))
            elif type_name in ("DomeLight", "DomeLight_1"):
                self.add_dome_light(prim, self.convert.matrix(
                    self.xforms.GetLocalToWorldTransform(prim)))
            elif type_name in ("CylinderLight", "GeometryLight", "PortalLight"):
                raise ValueError("unsupported light %s at %s" % (type_name, prim.GetPath()))
            elif prim.IsA(UsdGeom.Gprim) and not prim.IsA(UsdGeom.Mesh):
                self.notes.append("%s %s is not a mesh and was skipped" %
                                  (type_name, prim.GetPath()))
            if prim.HasAPI(UsdLux.MeshLightAPI):
                raise ValueError("MeshLightAPI emitters are not supported: " + str(prim.GetPath()))
        if not self.shapes:
            raise ValueError("USD scene has no visible meshes")
        unused = [pattern for pattern, _ in self.simplify if pattern not in self.simplify_used]
        if unused:
            raise ValueError("simplify patterns match no mesh: " + ", ".join(unused))
        points = np.concatenate([shape["points"] for shape in self.shapes])
        bounds = (points.min(axis=0), points.max(axis=0))
        camera = self.camera() or self.synthesized_camera(bounds)
        self.apply_exposure()
        return camera, bounds

    def apply_exposure(self):
        """Scale UsdLux light units by the camera's linear exposure, so the
        baked radiance is the image the camera would record. UsdPreviewSurface
        emission is left as authored (preview fallbacks are display-level)."""
        scale = self.exposure_scale
        if not math.isfinite(scale) or scale <= 0:
            raise ValueError("camera exposure scale %r is invalid" % scale)
        if scale == 1.0:
            return
        for emitter in self.emitters:
            emitter["emission"]["radiance"] = [v * scale for v in emitter["emission"]["radiance"]]
        for light in self.distant:
            light["irradiance"] = [v * scale for v in light["irradiance"]]
        if self.environment:
            self.environment["radiance"] = [v * scale for v in self.environment["radiance"]]
        self.notes.append("camera exposure scale %g applied to UsdLux lights; material "
                          "emission kept as authored" % scale)


def uv_sphere(radius, longitudes, latitudes):
    points = [(0.0, 0.0, radius), (0.0, 0.0, -radius)]
    for row in range(1, latitudes):
        theta = math.pi * row / latitudes
        for column in range(longitudes):
            phi = 2 * math.pi * column / longitudes
            points.append((radius * math.sin(theta) * math.cos(phi),
                           radius * math.sin(theta) * math.sin(phi), radius * math.cos(theta)))

    def ring(row, column):
        return 2 + (row - 1) * longitudes + column % longitudes
    faces = []
    for column in range(longitudes):
        faces.append((0, ring(1, column), ring(1, column + 1)))
        faces.append((1, ring(latitudes - 1, column + 1), ring(latitudes - 1, column)))
    for row in range(1, latitudes - 1):
        for column in range(longitudes):
            a, b = ring(row, column), ring(row, column + 1)
            c, d = ring(row + 1, column + 1), ring(row + 1, column)
            faces += [(a, d, c), (a, c, b)]
    return np.array(points), faces


# ------------------------------------------------------------------ output

def source_digest(stage, textures):
    """Digest of every layer file the stage used and every texture it reads."""
    entries = []
    for layer in stage.GetUsedLayers():
        path = layer.realPath
        if path and Path(path).is_file():
            entries.append((path, sha256(path)))
    for texture in sorted(textures):
        entries.append((texture, sha256(texture)))
    return hashlib.sha256(json.dumps(sorted(entries)).encode()).hexdigest(), \
        [path for path, _ in sorted(entries)]


def write_stage(path, shapes, emitters, summaries):
    path.parent.mkdir(parents=True, exist_ok=True)
    if path.exists():
        path.unlink()
    stage = Usd.Stage.CreateNew(str(path))
    UsdGeom.SetStageUpAxis(stage, UsdGeom.Tokens.z)
    UsdGeom.SetStageMetersPerUnit(stage, 1.0)
    root = UsdGeom.Xform.Define(stage, "/root")
    stage.SetDefaultPrim(root.GetPrim())
    materials = {}
    for name, summary in summaries.items():
        material = UsdShade.Material.Define(stage, "/root/_materials/" + name)
        shader = UsdShade.Shader.Define(stage, material.GetPath().AppendChild("Principled_BSDF"))
        shader.CreateIdAttr("UsdPreviewSurface")
        color = summary["base_color"] or (0.8, 0.8, 0.8)
        shader.CreateInput("diffuseColor", Sdf.ValueTypeNames.Color3f).Set(Gf.Vec3f(*color))
        shader.CreateInput("roughness", Sdf.ValueTypeNames.Float).Set(float(summary["roughness"]))
        shader.CreateInput("metallic", Sdf.ValueTypeNames.Float).Set(float(summary["metallic"]))
        material.CreateSurfaceOutput().ConnectToSource(shader.ConnectableAPI(), "surface")
        materials[name] = material
    for shape in shapes:
        xform = UsdGeom.Xform.Define(stage, "/root/" + shape["name"])
        mesh = UsdGeom.Mesh.Define(stage, xform.GetPath().AppendChild(shape["name"]))
        corners = shape["corner_triangles"].reshape(-1)
        points = shape["points"][corners]
        count = len(corners)
        mesh.CreatePointsAttr(Vt.Vec3fArray.FromNumpy(points.astype(np.float32)))
        mesh.CreateFaceVertexCountsAttr(Vt.IntArray([3] * (count // 3)))
        mesh.CreateFaceVertexIndicesAttr(Vt.IntArray.FromNumpy(
            np.arange(count, dtype=np.int32)))
        mesh.CreateSubdivisionSchemeAttr(UsdGeom.Tokens.none)
        mesh.CreateDoubleSidedAttr(shape["double_sided"])
        mesh.CreateNormalsAttr(Vt.Vec3fArray.FromNumpy(
            shape["normals"][corners].astype(np.float32)))
        mesh.SetNormalsInterpolation(UsdGeom.Tokens.faceVarying)
        st = UsdGeom.PrimvarsAPI(mesh).CreatePrimvar("st", Sdf.ValueTypeNames.TexCoord2fArray,
                                                     UsdGeom.Tokens.faceVarying)
        st.Set(Vt.Vec2fArray.FromNumpy(shape["uv"][corners].astype(np.float32)))
        extent = UsdGeom.PointBased.ComputeExtent(mesh.GetPointsAttr().Get())
        mesh.CreateExtentAttr(extent)
        UsdShade.MaterialBindingAPI.Apply(mesh.GetPrim()).Bind(materials[shape["material"]])
    counters = {}
    for emitter in emitters:
        index = len(counters.setdefault("all", []))
        counters["all"].append(emitter)
        name = "%s%02d" % (emitter["prefix"], index)
        emitter["name"] = name
        xform = UsdGeom.Xform.Define(stage, "/root/" + name)
        mesh = UsdGeom.Mesh.Define(stage, xform.GetPath().AppendChild(name))
        mesh.CreatePointsAttr([Gf.Vec3f(*p) for p in emitter["points"]])
        mesh.CreateFaceVertexCountsAttr([3] * (len(emitter["indices"]) // 3))
        mesh.CreateFaceVertexIndicesAttr(emitter["indices"])
        mesh.CreateSubdivisionSchemeAttr(UsdGeom.Tokens.none)
        points = np.asarray(emitter["points"])
        normals = []
        for k in range(0, len(emitter["indices"]), 3):
            a, b, c = (points[i] for i in emitter["indices"][k:k + 3])
            normal = np.cross(b - a, c - a)
            normal /= max(np.linalg.norm(normal), 1e-30)
            normals += [Gf.Vec3f(*normal)] * 3
        mesh.CreateNormalsAttr(normals)
        mesh.SetNormalsInterpolation(UsdGeom.Tokens.faceVarying)
    stage.GetRootLayer().Save()


def shape_points(shape):
    return shape["points"][shape["corner_triangles"].reshape(-1)]


def scene_model(extractor, camera, bounds, digest, inputs, usd_path):
    shapes = [{key: shape[key] for key in ("name", "material", "prim", "subset",
                                           "double_sided", "subdivision", "st_primvar",
                                           "role", "prop")}
              | {"triangles": int(len(shape["corner_triangles"])),
                 "bounds": [shape_points(shape).min(axis=0).tolist(),
                            shape_points(shape).max(axis=0).tolist()]}
              for shape in extractor.shapes]
    return {"schema": SCHEMA, "format": "usd", "source": str(Path(usd_path).resolve()),
            "source_sha256": digest, "source_files": inputs,
            "up_axis": str(UsdGeom.GetStageUpAxis(extractor.stage)),
            "meters_per_unit": extractor.convert.meters,
            "film": extractor.film, "camera": camera,
            "camera_exposure_scale": extractor.exposure_scale,
            "bounds_m": {"min": bounds[0].tolist(), "max": bounds[1].tolist()},
            "shapes": shapes, "materials": extractor.summaries,
            "emitters": [{k: v for k, v in emitter.items() if k != "prefix"}
                         for emitter in extractor.emitters],
            "distant_lights": extractor.distant, "environment": extractor.environment,
            "props": [dict(prop, shapes=[shape["name"] for shape in extractor.shapes
                                         if shape["prop"] == prop["name"]])
                      for prop in sorted(extractor.props.values(), key=lambda p: p["name"])],
            "textures": {Path(t).name + ":" + hashlib.sha1(t.encode()).hexdigest()[:8]:
                         {"filename": t} for t in sorted(extractor.textures)},
            "skipped": extractor.skipped, "approximations": extractor.notes}


def extract(usd_path, out_scene, out_stage, simplify=()):
    stage = Usd.Stage.Open(str(usd_path))
    if not stage:
        raise ValueError("could not open USD scene " + str(usd_path))
    extractor = Extractor(stage, usd_path, simplify)
    camera, bounds = extractor.run()
    digest, inputs = source_digest(stage, extractor.textures)
    write_stage(Path(out_stage), extractor.shapes, extractor.emitters, extractor.summaries)
    model = scene_model(extractor, camera, bounds, digest, inputs, usd_path)
    Path(out_scene).parent.mkdir(parents=True, exist_ok=True)
    Path(out_scene).write_text(json.dumps(model, indent=2, sort_keys=True) + "\n")
    return model


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    commands = parser.add_subparsers(dest="command", required=True)
    run = commands.add_parser("extract", help="write the scene model and normalized stage")
    run.add_argument("--scene", type=Path, required=True)
    run.add_argument("--out-scene", type=Path, required=True)
    run.add_argument("--out-stage", type=Path, required=True)
    run.add_argument("--simplify", action="append", default=[], metavar="PRIM_GLOB=CELL_M",
                     help="vertex-cluster meshes whose prim path matches the glob to a grid "
                          "of CELL_M metres (repeatable; each pattern must match a mesh)")
    show = commands.add_parser("inventory", help="print the scene model without stage output")
    show.add_argument("scene", type=Path)
    args = parser.parse_args()
    if args.command == "extract":
        rules = [rule.rsplit("=", 1) for rule in args.simplify]
        if any(len(rule) != 2 for rule in rules):
            parser.error("--simplify takes PRIM_GLOB=CELL_M")
        model = extract(args.scene, args.out_scene, args.out_stage, rules)
        print("USD_SCENE " + json.dumps({"status": "pass", "meshes": len(model["shapes"]),
                                         "materials": len(model["materials"]),
                                         "emitters": len(model["emitters"]),
                                         "distant_lights": len(model["distant_lights"]),
                                         "environment": bool(model["environment"])}))
    else:
        stage = Usd.Stage.Open(str(args.scene))
        extractor = Extractor(stage, args.scene)
        camera, bounds = extractor.run()
        digest, inputs = source_digest(stage, extractor.textures)
        print(json.dumps(scene_model(extractor, camera, bounds, digest, inputs, args.scene),
                         indent=2, sort_keys=True))


if __name__ == "__main__":
    main()
