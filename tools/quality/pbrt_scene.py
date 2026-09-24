#!/usr/bin/env python3
"""Parse the PBRT-v4 scene subset used by imported reference maps.

This module owns PBRT scene knowledge for the map pipeline: tokenizing,
graphics state, named materials, textures, PLY shapes, area emitters, image
infinite lights, the reference camera and PBRT -> USD (Z-up) axis conversion.
It has no Blender or OpenUSD dependency, so Blender scripts, OpenUSD tools and
plain Python steps share one reader. Unsupported directives fail loudly rather
than being dropped.

Run directly to print a JSON inventory of a scene:

    python3 tools/quality/pbrt_scene.py living-room/scene-v4.pbrt
"""

import hashlib
import json
import math
import re
import sys
from pathlib import Path


# PBRT scenes here are Y-up; USD stages and Source are Z-up.
# usd = (x, -z, y), row-major.
PBRT_TO_USD = ((1.0, 0.0, 0.0, 0.0),
               (0.0, 0.0, -1.0, 0.0),
               (0.0, 1.0, 0.0, 0.0),
               (0.0, 0.0, 0.0, 1.0))
IDENTITY = tuple(tuple(1.0 if row == column else 0.0 for column in range(4))
                 for row in range(4))

# Named PBRT spectra reduced to RGB (eta, k) at roughly 650/550/450 nm.
NAMED_CONDUCTORS = {
    "metal-Ag": ((0.155, 0.117, 0.138), (4.828, 3.122, 2.147)),
    "metal-Al": ((1.657, 0.880, 0.521), (9.224, 6.270, 4.837)),
    "metal-Au": ((0.143, 0.374, 1.442), (3.983, 2.386, 1.603)),
    "metal-Cu": ((0.200, 0.924, 1.102), (3.912, 2.452, 2.142)),
}

IGNORED_RENDER_DIRECTIVES = {"Integrator", "Sampler", "PixelFilter", "ColorSpace", "Option"}
SUPPORTED_MATERIALS = {"diffuse", "coateddiffuse", "conductor", "dielectric",
                       "diffusetransmission"}


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def matmul(a, b):
    return tuple(tuple(sum(a[row][k] * b[k][column] for k in range(4))
                       for column in range(4)) for row in range(4))


def transform_point(matrix, point):
    return tuple(sum(matrix[row][k] * (tuple(point) + (1.0,))[k] for k in range(4))
                 for row in range(3))


def transform_vector(matrix, vector):
    return tuple(sum(matrix[row][k] * vector[k] for k in range(3)) for row in range(3))


def invert(matrix):
    """Invert a 4x4 matrix with Gauss-Jordan elimination."""
    augmented = [list(matrix[row]) + [1.0 if row == column else 0.0 for column in range(4)]
                 for row in range(4)]
    for column in range(4):
        pivot = max(range(column, 4), key=lambda row: abs(augmented[row][column]))
        if abs(augmented[pivot][column]) < 1e-12:
            raise ValueError("PBRT transform is singular")
        augmented[column], augmented[pivot] = augmented[pivot], augmented[column]
        scale = augmented[column][column]
        augmented[column] = [value / scale for value in augmented[column]]
        for row in range(4):
            if row != column:
                factor = augmented[row][column]
                augmented[row] = [value - factor * other
                                  for value, other in zip(augmented[row], augmented[column])]
    return tuple(tuple(augmented[row][4:]) for row in range(4))


def matrix_from_pbrt(values):
    """PBRT lists matrices column-major; return a row-major tuple matrix."""
    if len(values) != 16:
        raise ValueError("PBRT transform must have 16 scalars")
    return tuple(tuple(float(values[column * 4 + row]) for column in range(4))
                 for row in range(4))


def tokenize(text):
    pattern = re.compile(r'"[^"]*"|\[|\]|[^\s\[\]"#]+|#[^\n]*')
    for match in pattern.finditer(text):
        token = match.group(0)
        if not token.startswith("#"):
            yield token


class Parser:
    def __init__(self, text):
        self.tokens = list(tokenize(text))
        self.position = 0

    def peek(self):
        return self.tokens[self.position] if self.position < len(self.tokens) else None

    def take(self):
        token = self.peek()
        if token is None:
            raise ValueError("unexpected end of PBRT scene")
        self.position += 1
        return token

    def string(self):
        token = self.take()
        if not token.startswith('"'):
            raise ValueError("expected PBRT string, found " + token)
        return token[1:-1]

    def numbers(self):
        token = self.take()
        if token != "[":
            return [float(token)]
        values = []
        while self.peek() != "]":
            values.append(float(self.take()))
        self.take()
        return values

    def parameters(self):
        """Read `"type name" value` pairs until the next directive."""
        result = {}
        while self.peek() is not None and self.peek().startswith('"'):
            declaration = self.string().split()
            if len(declaration) != 2:
                raise ValueError("malformed PBRT parameter declaration")
            kind, name = declaration
            token = self.take()
            values = []
            if token == "[":
                while self.peek() != "]":
                    values.append(self.take())
                self.take()
            else:
                values.append(token)
            if kind in {"string", "texture", "spectrum"} and all(
                    value.startswith('"') for value in values):
                parsed = [value[1:-1] for value in values]
            elif kind == "bool":
                parsed = [value.strip('"') == "true" for value in values]
            elif kind in {"float", "integer", "rgb", "point3", "point2", "normal",
                          "vector3", "spectrum", "blackbody", "point", "normal3"}:
                parsed = [float(value) for value in values]
            else:
                raise ValueError("unsupported PBRT parameter type " + kind)
            result[name] = {"type": kind, "value": parsed}
        return result


def scalar(parameters, name, default=None):
    if name not in parameters:
        return default
    value = parameters[name]["value"]
    if len(value) != 1:
        raise ValueError("PBRT parameter is not scalar: " + name)
    return value[0]


def parse(path):
    """Return the scene description as plain dictionaries and tuple matrices."""
    path = Path(path)
    parser = Parser(path.read_text())
    ctm = IDENTITY
    state = {"material": None, "area_light": None, "reverse_orientation": False,
             "medium": None}
    stack = []
    in_world = False
    scene = {"source": str(path), "source_sha256": sha256(path), "camera": None,
             "film": {}, "textures": {}, "materials": {}, "shapes": [],
             "emitters": [], "environment": None, "media": {}}
    while parser.peek() is not None:
        directive = parser.take()
        if directive in IGNORED_RENDER_DIRECTIVES:
            parser.string()
            parser.parameters()
        elif directive == "Transform":
            ctm = matrix_from_pbrt(parser.numbers())
        elif directive == "ConcatTransform":
            ctm = matmul(ctm, matrix_from_pbrt(parser.numbers()))
        elif directive == "Translate":
            x, y, z = (float(parser.take()) for _ in range(3))
            ctm = matmul(ctm, ((1, 0, 0, x), (0, 1, 0, y), (0, 0, 1, z), (0, 0, 0, 1)))
        elif directive == "Scale":
            x, y, z = (float(parser.take()) for _ in range(3))
            ctm = matmul(ctm, ((x, 0, 0, 0), (0, y, 0, 0), (0, 0, z, 0), (0, 0, 0, 1)))
        elif directive == "Film":
            parser.string()
            parameters = parser.parameters()
            scene["film"] = {"width": int(scalar(parameters, "xresolution", 1280)),
                             "height": int(scalar(parameters, "yresolution", 720))}
        elif directive == "Camera":
            kind = parser.string()
            parameters = parser.parameters()
            if kind != "perspective":
                raise ValueError("only perspective PBRT cameras are supported")
            scene["camera"] = {"camera_from_world": ctm,
                               "fov_degrees": scalar(parameters, "fov", 90.0)}
        elif directive == "WorldBegin":
            in_world = True
            ctm = IDENTITY
        elif directive == "AttributeBegin":
            stack.append((ctm, dict(state)))
        elif directive == "AttributeEnd":
            if not stack:
                raise ValueError("PBRT attribute stack underflow")
            ctm, state = stack.pop()
        elif directive == "ReverseOrientation":
            state["reverse_orientation"] = not state["reverse_orientation"]
        elif directive == "Texture":
            name = parser.string()
            value_type = parser.string()
            texture_class = parser.string()
            parameters = parser.parameters()
            if texture_class != "imagemap":
                raise ValueError("unsupported PBRT texture class " + texture_class)
            scene["textures"][name] = {
                "value_type": value_type,
                "filename": scalar(parameters, "filename"),
                "scale": scalar(parameters, "scale", 1.0)}
        elif directive == "MakeNamedMaterial":
            name = parser.string()
            parameters = parser.parameters()
            kind = scalar(parameters, "type")
            if kind not in SUPPORTED_MATERIALS:
                raise ValueError("unsupported PBRT material type %s for %s" % (kind, name))
            scene["materials"][name] = {"type": kind, "parameters": parameters}
        elif directive == "MakeNamedMedium":
            name = parser.string()
            parameters = parser.parameters()
            scene["media"][name] = {key: value["value"] for key, value in parameters.items()}
        elif directive == "MediumInterface":
            inside = parser.string()
            parser.string()
            if inside and inside not in scene["media"]:
                raise ValueError("PBRT medium interface uses undefined medium " + inside)
            state["medium"] = inside or None
        elif directive == "Material":
            kind = parser.string()
            parameters = parser.parameters()
            if kind not in SUPPORTED_MATERIALS:
                raise ValueError("unsupported anonymous PBRT material type " + kind)
            name = "Anonymous%03d" % len(scene["materials"])
            scene["materials"][name] = {"type": kind, "parameters": parameters}
            state["material"] = name
        elif directive == "NamedMaterial":
            state["material"] = parser.string()
        elif directive == "AreaLightSource":
            kind = parser.string()
            parameters = parser.parameters()
            if kind != "diffuse":
                raise ValueError("unsupported PBRT area light " + kind)
            radiance = parameters.get("L", {}).get("value")
            if not radiance or len(radiance) != 3:
                raise ValueError("PBRT area light requires RGB L")
            state["area_light"] = {"radiance": radiance,
                                   "scale": scalar(parameters, "scale", 1.0),
                                   "two_sided": bool(scalar(parameters, "twosided", False))}
        elif directive == "LightSource":
            kind = parser.string()
            parameters = parser.parameters()
            if kind != "infinite" or "filename" not in parameters:
                raise ValueError("only image infinite PBRT lights are supported")
            if scene["environment"]:
                raise ValueError("multiple PBRT environment lights")
            scene["environment"] = {"filename": scalar(parameters, "filename"),
                                    "scale": scalar(parameters, "scale", 1.0),
                                    "world_from_light": ctm}
        elif directive == "Shape":
            kind = parser.string()
            parameters = parser.parameters()
            if not in_world:
                raise ValueError("PBRT shape before WorldBegin")
            shape = {"kind": kind, "world_from_object": ctm, "material": state["material"],
                     "reverse_orientation": state["reverse_orientation"],
                     "interior_medium": state["medium"]}
            if kind == "plymesh":
                shape["filename"] = scalar(parameters, "filename")
            elif kind == "trianglemesh":
                points = parameters["P"]["value"]
                shape["points"] = [tuple(points[index:index + 3])
                                   for index in range(0, len(points), 3)]
                shape["indices"] = [int(value) for value in parameters["indices"]["value"]]
            elif kind == "disk":
                shape["radius"] = scalar(parameters, "radius", 1.0)
            else:
                raise ValueError("unsupported PBRT shape " + kind)
            if state["area_light"]:
                shape["emission"] = state["area_light"]
                scene["emitters"].append(shape)
            elif kind == "plymesh":
                if not shape["material"]:
                    raise ValueError("PBRT mesh lacks a named material: " + shape["filename"])
                scene["shapes"].append(shape)
            else:
                raise ValueError("non-emissive PBRT %s shapes are not supported" % kind)
        else:
            raise ValueError("unsupported PBRT directive " + directive)
    if stack:
        raise ValueError("PBRT attribute stack is incomplete")
    if not scene["camera"]:
        raise ValueError("PBRT scene has no camera")
    names = [Path(shape["filename"]).stem for shape in scene["shapes"]]
    if len(set(names)) != len(names):
        raise ValueError("PBRT PLY stems must be unique; they become USD mesh names")
    for shape in scene["shapes"]:
        if shape["material"] not in scene["materials"]:
            raise ValueError("PBRT mesh uses undefined material " + shape["material"])
    return scene


def material_texture(scene, material, parameter):
    """Return the image path bound to a material parameter, or None."""
    entry = material["parameters"].get(parameter)
    if not entry or entry["type"] != "texture":
        return None
    texture = scene["textures"].get(entry["value"][0])
    if not texture:
        raise ValueError("PBRT material references missing texture " + entry["value"][0])
    return texture["filename"]


def conductor_f0(parameters):
    """Normal-incidence RGB reflectance of a PBRT conductor."""
    def rgb(name):
        entry = parameters.get(name)
        if not entry:
            return None
        if entry["type"] == "spectrum" and isinstance(entry["value"][0], str):
            named = entry["value"][0]
            key = named.rsplit("-", 1)[0]
            if key not in NAMED_CONDUCTORS:
                raise ValueError("unsupported named PBRT spectrum " + named)
            return NAMED_CONDUCTORS[key][0 if named.endswith("-eta") else 1]
        if len(entry["value"]) != 3:
            raise ValueError("PBRT conductor %s must be RGB" % name)
        return tuple(entry["value"])
    eta = rgb("eta") or NAMED_CONDUCTORS["metal-Cu"][0]
    k = rgb("k") or NAMED_CONDUCTORS["metal-Cu"][1]
    return tuple(((e - 1) ** 2 + c ** 2) / ((e + 1) ** 2 + c ** 2) for e, c in zip(eta, k))


def roughness_alpha(parameters, default):
    """Microfacet alpha from PBRT roughness parameters."""
    values = [scalar(parameters, name) for name in ("roughness", "uroughness", "vroughness")]
    values = [value for value in values if value is not None]
    alpha = sum(values) / len(values) if values else default
    if scalar(parameters, "remaproughness", True) and alpha > 0:
        # PBRT-v4 RoughnessToAlpha is sqrt(roughness).
        alpha = math.sqrt(alpha)
    return alpha


def dielectric_fresnel(cos_i, eta):
    """Unpolarized Fresnel reflectance from the outside of a dielectric."""
    sin_t2 = (1.0 - cos_i * cos_i) / (eta * eta)
    if sin_t2 >= 1.0:
        return 1.0
    cos_t = math.sqrt(1.0 - sin_t2)
    parallel = (eta * cos_i - cos_t) / (eta * cos_i + cos_t)
    perpendicular = (cos_i - eta * cos_t) / (cos_i + eta * cos_t)
    return 0.5 * (parallel * parallel + perpendicular * perpendicular)


def internal_diffuse_reflectance(eta, steps=2048):
    """Cosine-weighted average reflectance seen from inside a coat of `eta`.

    External average Fdr_ext = 2 int F(mu) mu dmu; by reciprocity the internal
    average is 1 - (1 - Fdr_ext) / eta^2 (0.596 for eta = 1.5).
    """
    external = sum(2.0 * dielectric_fresnel((i + 0.5) / steps, eta) * (i + 0.5) / steps
                   for i in range(steps)) / steps
    return 1.0 - (1.0 - external) / (eta * eta)


def coated_albedo(albedo, internal_reflectance):
    """Diffuse albedo leaving a clear coat, before the outer Fresnel lobe.

    Light scattered by the base is trapped by internal reflection and
    re-bounces: a (1 - Fdr) / (1 - a Fdr). This is the plastic model used by
    Tungsten and matched by PBRT's coateddiffuse with a thin, clear coat.
    """
    return albedo * (1.0 - internal_reflectance) / (1.0 - albedo * internal_reflectance)


def material_summary(scene, name):
    """Reduce a PBRT material to the metal/roughness preview model.

    `roughness` is perceptual (sqrt(alpha)). Colors are linear. This is the one
    translation policy shared by the Cycles stage, the lightmap bake and the
    Source content bridge; `approximation` records what is lost.
    """
    material = scene["materials"][name]
    parameters = material["parameters"]
    kind = material["type"]
    texture = material_texture(scene, material, "reflectance")
    reflectance = parameters.get("reflectance", {}).get("value")
    color = tuple(reflectance) if reflectance and not texture else (
        (0.5, 0.5, 0.5) if not texture else None)
    result = {"name": name, "pbrt_type": kind, "base_color": color, "base_texture": texture,
              "metallic": 0.0, "roughness": 1.0, "coat_roughness": None,
              "coat_internal_reflectance": None,
              "transmission": 0.0, "ior": 1.5, "approximation": None}
    if kind == "coateddiffuse":
        if scalar(parameters, "albedo", 0.0) or parameters.get("albedo", {}).get("type") == "rgb":
            raise ValueError("absorbing PBRT coats are not supported: " + name)
        result["coat_roughness"] = math.sqrt(roughness_alpha(parameters, 0.0))
        result["roughness"] = max(result["coat_roughness"], 0.02)
        fdr = internal_diffuse_reflectance(scalar(parameters, "eta", 1.5))
        result["coat_internal_reflectance"] = fdr
        if color:
            result["base_color"] = tuple(coated_albedo(value, fdr) for value in color)
        result["approximation"] = ("clear coat over diffuse; base albedo reduced by coat "
                                   "internal reflection, single-layer preview uses coat roughness")
    elif kind == "conductor":
        result["metallic"] = 1.0
        result["base_color"] = conductor_f0(parameters)
        result["base_texture"] = None
        result["roughness"] = max(math.sqrt(roughness_alpha(parameters, 0.0)), 0.02)
        result["approximation"] = "spectral conductor reduced to RGB F0"
    elif kind == "dielectric":
        result["base_color"] = (1.0, 1.0, 1.0)
        result["transmission"] = 1.0
        result["ior"] = scalar(parameters, "eta", 1.5)
        result["roughness"] = 0.0
        result["approximation"] = "refraction unavailable in game preview"
    elif kind == "diffusetransmission":
        result["approximation"] = "diffuse transmission reduced to diffuse reflection"
    return result


def camera_pose(scene):
    """Reference camera eye and forward vector in USD (Z-up, stage units)."""
    world_from_camera = invert(scene["camera"]["camera_from_world"])
    to_usd = PBRT_TO_USD
    eye = transform_point(matmul(to_usd, world_from_camera), (0.0, 0.0, 0.0))
    forward = transform_vector(matmul(to_usd, world_from_camera), (0.0, 0.0, 1.0))
    up = transform_vector(matmul(to_usd, world_from_camera), (0.0, 1.0, 0.0))
    return {"eye": eye, "forward": forward, "up": up}


def read_pfm(path):
    """Read a PFM as (height, width, 3) float32 rows ordered top to bottom."""
    import numpy as np
    data = Path(path).read_bytes()
    header = []
    offset = 0
    while len(header) < 3:
        end = data.index(b"\n", offset)
        line = data[offset:end].strip()
        offset = end + 1
        if line:
            header.append(line.decode("ascii"))
    if header[0] not in {"PF", "Pf"}:
        raise ValueError("unsupported PFM header " + header[0])
    width, height = (int(value) for value in header[1].split())
    scale = float(header[2])
    channels = 3 if header[0] == "PF" else 1
    pixels = np.frombuffer(data[offset:offset + width * height * channels * 4],
                           dtype="<f4" if scale < 0 else ">f4")
    if pixels.size != width * height * channels:
        raise ValueError("truncated PFM " + str(path))
    pixels = pixels.reshape(height, width, channels).astype(np.float32)
    if channels == 1:
        pixels = np.repeat(pixels, 3, axis=2)
    return pixels[::-1].copy()


def equal_area_sphere_to_square(directions):
    """PBRT-v4 EqualAreaSphereToSquare for (N, 3) unit vectors."""
    import numpy as np
    x, y, z = (np.abs(directions[:, axis]) for axis in range(3))
    r = np.sqrt(np.maximum(0.0, 1.0 - z))
    a = np.maximum(x, y)
    b = np.divide(np.minimum(x, y), a, out=np.zeros_like(a), where=a > 0)
    phi = np.arctan(b) * 2.0 / np.pi
    phi = np.where(x < y, 1.0 - phi, phi)
    v = phi * r
    u = r - v
    lower = directions[:, 2] < 0
    u, v = np.where(lower, 1.0 - v, u), np.where(lower, 1.0 - u, v)
    u = np.copysign(u, directions[:, 0])
    v = np.copysign(v, directions[:, 1])
    return 0.5 * (u + 1.0), 0.5 * (v + 1.0)


def environment_equirect(scene, width=2048):
    """Resample a PBRT-v4 equal-area environment into a Blender Z-up equirect.

    Row 0 is the top (zenith). Columns follow Blender's environment
    convention: u = -atan2(y, x) / 2pi + 0.5 in USD/Blender world space.
    """
    import numpy as np
    environment = scene["environment"]
    source = read_pfm(Path(scene["source"]).parent / environment["filename"])
    if source.shape[0] != source.shape[1]:
        raise ValueError("PBRT-v4 image infinite lights require square equal-area maps")
    height = width // 2
    columns = (np.arange(width) + 0.5) / width
    rows = (np.arange(height) + 0.5) / height
    u, v = np.meshgrid(columns, 1.0 - rows)
    phi = (0.5 - u) * 2.0 * np.pi
    elevation = (v - 0.5) * np.pi
    usd = np.stack((np.cos(elevation) * np.cos(phi), np.cos(elevation) * np.sin(phi),
                    np.sin(elevation)), axis=-1).reshape(-1, 3)
    # USD (x, y, z) -> PBRT world (x, z, -y) -> light space.
    pbrt = np.stack((usd[:, 0], usd[:, 2], -usd[:, 1]), axis=-1)
    light_from_world = np.array(invert(environment["world_from_light"]))[:3, :3]
    local = pbrt @ light_from_world.T
    local /= np.linalg.norm(local, axis=1, keepdims=True)
    square_u, square_v = equal_area_sphere_to_square(local)
    size = source.shape[0]
    column = np.clip((square_u * size).astype(np.int64), 0, size - 1)
    row = np.clip((square_v * size).astype(np.int64), 0, size - 1)
    return (source[row, column] * environment["scale"]).reshape(height, width, 3)


def inventory(scene):
    materials = {}
    for shape in scene["shapes"]:
        materials.setdefault(shape["material"], []).append(Path(shape["filename"]).stem)
    return {"source_sha256": scene["source_sha256"], "film": scene["film"],
            "camera_fov_degrees": scene["camera"]["fov_degrees"],
            "camera_usd": camera_pose(scene),
            "mesh_count": len(scene["shapes"]), "emitter_count": len(scene["emitters"]),
            "environment": scene["environment"]["filename"] if scene["environment"] else None,
            "textures": {name: texture["filename"] for name, texture in scene["textures"].items()},
            "materials": {name: material_summary(scene, name)
                          for name in sorted(materials)},
            "meshes_by_material": materials}


if __name__ == "__main__":
    print(json.dumps(inventory(parse(sys.argv[1])), indent=2, sort_keys=True))
