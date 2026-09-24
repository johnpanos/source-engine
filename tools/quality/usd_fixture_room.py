#!/usr/bin/env python3
"""Generate the `usd-room` fixture: a small authored USD interior for the map pipeline.

The fixture exercises what real DCC exports contain and a PBRT conversion does
not: a Y-up centimetre stage, UsdPreviewSurface materials with base colour,
packed occlusion/roughness/metallic, tangent-space normal, emissive and
alpha-cutout textures, a UsdTransform2d tiling, GeomSubset material splits,
instanceable references, a PointInstancer, a sun (DistantLight), a ceiling
RectLight, an HDR DomeLight seen through a window, a camera, and an
asymmetric poster that reveals texture orientation. Output is deterministic;
rerun to regenerate `quality/fixtures/usd-maps/room/`:

    PYTHONPATH=build/toolchains/openusd-25.11/lib/python \\
        /usr/bin/python3.12 tools/quality/usd_fixture_room.py
"""

import argparse
import math
import struct
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw
from pxr import Gf, Sdf, Usd, UsdGeom, UsdLux, UsdShade, Vt

ROOT = Path(__file__).resolve().parents[2]
DEFAULT_OUT = ROOT / "quality/fixtures/usd-maps/room"
# Interior (cm, Y-up): x 0..600, y 0..300, z 0..800; walls 20 cm thick.
WIDTH, HEIGHT, DEPTH, WALL = 600.0, 300.0, 800.0, 20.0
WINDOW = {"z": (250.0, 550.0), "y": (90.0, 240.0)}


# ---------------------------------------------------------------- textures

def srgb(linear):
    linear = np.clip(linear, 0.0, 1.0)
    return np.where(linear <= 0.0031308, 12.92 * linear,
                    1.055 * np.power(linear, 1 / 2.4) - 0.055)


def save_rgb(path, linear_or_encoded, encode=True, alpha=None):
    data = srgb(linear_or_encoded) if encode else np.clip(linear_or_encoded, 0.0, 1.0)
    pixels = np.rint(data * 255).astype(np.uint8)
    if alpha is not None:
        pixels = np.dstack((pixels, np.rint(np.clip(alpha, 0, 1) * 255).astype(np.uint8)))
    Image.fromarray(pixels).save(path, optimize=False)


def uv_grid(size):
    """u right, v up (row 0 is v = 1, as UsdUVTexture reads images)."""
    u = (np.arange(size) + 0.5) / size
    v = 1.0 - (np.arange(size) + 0.5) / size
    return np.meshgrid(u, v)


def normal_from_height(height, strength):
    """Tangent-space normal (+X = +u, +Y = +v) encoded to [0, 1]."""
    size = height.shape[0]
    # np.gradient axis 0 runs down the rows, i.e. toward -v.
    d_dv = -np.gradient(height, axis=0) * size
    d_du = np.gradient(height, axis=1) * size
    normal = np.dstack((-d_du * strength, -d_dv * strength, np.ones_like(height)))
    normal /= np.linalg.norm(normal, axis=2, keepdims=True)
    return normal * 0.5 + 0.5


def value_noise(size, cells, seed):
    rng = np.random.default_rng(seed)
    grid = rng.random((cells + 1, cells + 1))
    grid[-1, :] = grid[0, :]
    grid[:, -1] = grid[:, 0]
    coordinates = np.arange(size) / size * cells
    i = coordinates.astype(int)
    f = coordinates - i
    f = f * f * (3 - 2 * f)
    rows = grid[i][:, i] * (1 - f)[None, :] + grid[i][:, i + 1] * f[None, :]
    rows_next = grid[i + 1][:, i] * (1 - f)[None, :] + grid[i + 1][:, i + 1] * f[None, :]
    return rows * (1 - f)[:, None] + rows_next * f[:, None]


def write_textures(out):
    size = 512
    u, v = uv_grid(size)
    # Wood planks: 4 planks across u per tile; grooves between them.
    plank = np.floor(u * 4)
    offset = (plank * 0.37) % 1.0
    along = (v + offset) % 1.0
    grain = 0.5 + 0.5 * np.sin((u * 4 % 1.0) * 40 + value_noise(size, 16, 1) * 6)
    tone = 0.75 + 0.25 * ((plank * 0.61) % 1.0)
    base = np.dstack((0.38 * tone * (0.8 + 0.2 * grain), 0.20 * tone * (0.8 + 0.2 * grain),
                      0.09 * tone * (0.85 + 0.15 * grain)))
    groove = np.minimum(np.abs(u * 4 - np.round(u * 4)), np.abs(along - np.round(along)) * 0.25)
    height = np.clip(groove * 60, 0, 1) + 0.02 * value_noise(size, 32, 2)
    base *= (0.55 + 0.45 * np.clip(groove * 60, 0, 1))[..., None]
    save_rgb(out / "wood_basecolor.png", base)
    save_rgb(out / "wood_normal.png", normal_from_height(height, 0.004 * 16), encode=False)
    roughness = 0.35 + 0.25 * value_noise(size, 8, 3)
    occlusion = 0.6 + 0.4 * np.clip(groove * 60, 0, 1)
    save_rgb(out / "wood_orm.png", np.dstack((occlusion, roughness, np.zeros_like(u))),
             encode=False)
    # Plaster: fine bumps only.
    bumps = value_noise(size, 64, 4) * 0.7 + value_noise(size, 16, 5) * 0.3
    save_rgb(out / "plaster_normal.png", normal_from_height(bumps, 0.0006 * 64), encode=False)
    # Screen: emissive test pattern with an orientation marker.
    screen = Image.new("RGB", (256, 256), (10, 10, 30))
    draw = ImageDraw.Draw(screen)
    for k, color in enumerate(((230, 60, 40), (60, 200, 80), (60, 110, 230), (240, 220, 60))):
        draw.rectangle((16 + k * 58, 150, 64 + k * 58, 240), fill=color)
    draw.polygon(((128, 20), (80, 110), (176, 110)), fill=(250, 250, 250))  # arrow up = +v
    screen.save(out / "screen_emissive.png", optimize=False)
    # Poster: an asymmetric "F" (top = +v, left = -u).
    poster = Image.new("RGB", (256, 256), (235, 225, 200))
    draw = ImageDraw.Draw(poster)
    draw.rectangle((60, 30, 100, 226), fill=(160, 30, 30))
    draw.rectangle((60, 30, 200, 70), fill=(160, 30, 30))
    draw.rectangle((60, 110, 170, 145), fill=(160, 30, 30))
    poster.save(out / "poster_basecolor.png", optimize=False)
    # Leaves: a cut-out frond with alpha.
    leaf_u, leaf_v = uv_grid(256)
    rib = np.abs(leaf_u - 0.5)
    leaflets = 0.33 * np.abs(np.sin(leaf_v * math.pi * 7)) * (1 - leaf_v) + 0.05
    alpha = ((rib < leaflets) & (leaf_v > 0.03) & (leaf_v < 0.97)).astype(float)
    color = np.dstack((0.05 + 0.05 * leaf_v, 0.25 + 0.15 * leaf_v, 0.04 + 0.03 * leaf_v))
    save_rgb(out / "leaf_basecolor.png", color, alpha=alpha)


def write_sky(path, width=256):
    """A Radiance RGBE latlong sky: blue zenith, bright horizon, dim ground."""
    height = width // 2
    latitude = math.pi / 2 - (np.arange(height) + 0.5) / height * math.pi
    rows = []
    for lat in latitude:
        if lat >= 0:
            t = math.sin(lat)
            color = np.array((0.75, 0.85, 1.0)) * (1 - t) + np.array((0.22, 0.38, 0.85)) * t
            color *= 1.4
        else:
            color = np.array((0.30, 0.27, 0.24))
        rows.append(np.tile(color, (width, 1)))
    pixels = np.stack(rows).astype(np.float64)
    header = b"#?RADIANCE\nFORMAT=32-bit_rle_rgbe\n\n-Y %d +X %d\n" % (height, width)
    body = bytearray()
    for row in pixels:
        for rgb in row:
            peak = max(rgb)
            if peak < 1e-32:
                body += b"\0\0\0\0"
                continue
            mantissa, exponent = math.frexp(peak)
            scale = mantissa * 256.0 / peak
            body += struct.pack("BBBB", *(int(c * scale) for c in rgb), exponent + 128)
    path.write_bytes(header + bytes(body))


# -------------------------------------------------------------------- stage

def box(stage, path, low, high, material, st_scale=0.01):
    """A closed box mesh with per-face planar st in metres (cm * st_scale)."""
    (x0, y0, z0), (x1, y1, z1) = low, high
    corners = [(x0, y0, z0), (x1, y0, z0), (x1, y1, z0), (x0, y1, z0),
               (x0, y0, z1), (x1, y0, z1), (x1, y1, z1), (x0, y1, z1)]
    faces = [(0, 3, 2, 1), (4, 5, 6, 7), (0, 1, 5, 4), (2, 3, 7, 6), (1, 2, 6, 5), (0, 4, 7, 3)]
    axes = [(0, 1), (0, 1), (0, 2), (0, 2), (2, 1), (2, 1)]
    mesh = UsdGeom.Mesh.Define(stage, path)
    mesh.CreatePointsAttr([Gf.Vec3f(*c) for c in corners])
    mesh.CreateFaceVertexCountsAttr([4] * 6)
    mesh.CreateFaceVertexIndicesAttr([i for face in faces for i in face])
    mesh.CreateSubdivisionSchemeAttr(UsdGeom.Tokens.none)
    st = []
    for face, (a, b) in zip(faces, axes):
        for index in face:
            st.append(Gf.Vec2f(corners[index][a] * st_scale, corners[index][b] * st_scale))
    UsdGeom.PrimvarsAPI(mesh).CreatePrimvar(
        "st", Sdf.ValueTypeNames.TexCoord2fArray, UsdGeom.Tokens.faceVarying).Set(st)
    if material:
        UsdShade.MaterialBindingAPI.Apply(mesh.GetPrim()).Bind(material)
    return mesh


def quad(stage, path, corners, material, double_sided=False):
    mesh = UsdGeom.Mesh.Define(stage, path)
    mesh.CreatePointsAttr([Gf.Vec3f(*c) for c in corners])
    mesh.CreateFaceVertexCountsAttr([4])
    mesh.CreateFaceVertexIndicesAttr([0, 1, 2, 3])
    mesh.CreateSubdivisionSchemeAttr(UsdGeom.Tokens.none)
    mesh.CreateDoubleSidedAttr(double_sided)
    # st as vertex interpolation (exercises primvar expansion).
    UsdGeom.PrimvarsAPI(mesh).CreatePrimvar(
        "st", Sdf.ValueTypeNames.TexCoord2fArray, UsdGeom.Tokens.vertex).Set(
        [Gf.Vec2f(0, 0), Gf.Vec2f(1, 0), Gf.Vec2f(1, 1), Gf.Vec2f(0, 1)])
    UsdShade.MaterialBindingAPI.Apply(mesh.GetPrim()).Bind(material)
    return mesh


def sphere(stage, path, center, radius, material, rings=24, segments=48):
    points, counts, indices = [], [], []
    for ring in range(rings + 1):
        theta = math.pi * ring / rings
        for segment in range(segments):
            phi = 2 * math.pi * segment / segments
            points.append(Gf.Vec3f(center[0] + radius * math.sin(theta) * math.cos(phi),
                                   center[1] + radius * math.cos(theta),
                                   center[2] - radius * math.sin(theta) * math.sin(phi)))
    for ring in range(rings):
        for segment in range(segments):
            a = ring * segments + segment
            b = ring * segments + (segment + 1) % segments
            counts.append(4)
            indices += [a, a + segments, b + segments, b]
    mesh = UsdGeom.Mesh.Define(stage, path)
    mesh.CreatePointsAttr(points)
    mesh.CreateFaceVertexCountsAttr(counts)
    mesh.CreateFaceVertexIndicesAttr(indices)
    # Catmull-Clark default and no authored normals: the reader smooths them.
    st = [Gf.Vec2f((i % segments) / segments, 1 - (i // segments) / rings)
          for i in range(len(points))]
    UsdGeom.PrimvarsAPI(mesh).CreatePrimvar(
        "st", Sdf.ValueTypeNames.TexCoord2fArray, UsdGeom.Tokens.vertex).Set(st)
    UsdShade.MaterialBindingAPI.Apply(mesh.GetPrim()).Bind(material)
    return mesh


class Materials:
    def __init__(self, stage):
        self.stage = stage
        self.scope = UsdGeom.Scope.Define(stage, "/Room/Looks")

    def surface(self, name, **constants):
        material = UsdShade.Material.Define(self.stage, "/Room/Looks/" + name)
        shader = UsdShade.Shader.Define(self.stage, material.GetPath().AppendChild("Surface"))
        shader.CreateIdAttr("UsdPreviewSurface")
        for key, value in constants.items():
            kind = Sdf.ValueTypeNames.Color3f if isinstance(value, tuple) else \
                Sdf.ValueTypeNames.Float
            shader.CreateInput(key, kind).Set(Gf.Vec3f(*value) if isinstance(value, tuple)
                                              else value)
        material.CreateSurfaceOutput().ConnectToSource(shader.ConnectableAPI(), "surface")
        stname = material.CreateInput("frame:stPrimvarName", Sdf.ValueTypeNames.Token)
        stname.Set("st")
        reader = UsdShade.Shader.Define(self.stage, material.GetPath().AppendChild("StReader"))
        reader.CreateIdAttr("UsdPrimvarReader_float2")
        reader.CreateInput("varname", Sdf.ValueTypeNames.Token).ConnectToSource(stname)
        return material, shader, reader

    def texture(self, material, name, file, colorspace, st_source, **inputs):
        texture = UsdShade.Shader.Define(self.stage, material.GetPath().AppendChild(name))
        texture.CreateIdAttr("UsdUVTexture")
        texture.CreateInput("file", Sdf.ValueTypeNames.Asset).Set(file)
        texture.CreateInput("sourceColorSpace", Sdf.ValueTypeNames.Token).Set(colorspace)
        texture.CreateInput("wrapS", Sdf.ValueTypeNames.Token).Set("repeat")
        texture.CreateInput("wrapT", Sdf.ValueTypeNames.Token).Set("repeat")
        texture.CreateInput("st", Sdf.ValueTypeNames.Float2).ConnectToSource(
            st_source.ConnectableAPI(), "result")
        for key, value in inputs.items():
            texture.CreateInput(key, Sdf.ValueTypeNames.Float4).Set(Gf.Vec4f(*value))
        return texture


def connect(shader, input_name, kind, texture, output):
    shader.CreateInput(input_name, kind).ConnectToSource(texture.ConnectableAPI(), output)


def look_at(eye, target, up=(0.0, 1.0, 0.0)):
    eye, target, up = (np.asarray(v, dtype=float) for v in (eye, target, up))
    back = eye - target
    back /= np.linalg.norm(back)
    right = np.cross(up, back)
    right /= np.linalg.norm(right)
    true_up = np.cross(back, right)
    return Gf.Matrix4d(*right, 0, *true_up, 0, *back, 0, *eye, 1)


def build(out):
    out.mkdir(parents=True, exist_ok=True)
    textures = out / "textures"
    textures.mkdir(exist_ok=True)
    write_textures(textures)
    write_sky(textures / "sky.hdr")
    path = out / "room.usda"
    if path.exists():
        path.unlink()
    stage = Usd.Stage.CreateNew(str(path))
    UsdGeom.SetStageUpAxis(stage, UsdGeom.Tokens.y)
    UsdGeom.SetStageMetersPerUnit(stage, 0.01)
    room = UsdGeom.Xform.Define(stage, "/Room")
    stage.SetDefaultPrim(room.GetPrim())
    looks = Materials(stage)

    wood, wood_surface, reader = looks.surface("WoodFloor")
    tiling = UsdShade.Shader.Define(stage, wood.GetPath().AppendChild("Tiling"))
    tiling.CreateIdAttr("UsdTransform2d")
    tiling.CreateInput("in", Sdf.ValueTypeNames.Float2).ConnectToSource(
        reader.ConnectableAPI(), "result")
    tiling.CreateInput("scale", Sdf.ValueTypeNames.Float2).Set(Gf.Vec2f(0.5, 0.5))
    base = looks.texture(wood, "BaseColor", "./textures/wood_basecolor.png", "sRGB", tiling)
    orm = looks.texture(wood, "ORM", "./textures/wood_orm.png", "raw", tiling)
    normal = looks.texture(wood, "Normal", "./textures/wood_normal.png", "raw", tiling,
                           scale=(2, 2, 2, 1), bias=(-1, -1, -1, 0))
    connect(wood_surface, "diffuseColor", Sdf.ValueTypeNames.Color3f, base, "rgb")
    connect(wood_surface, "occlusion", Sdf.ValueTypeNames.Float, orm, "r")
    connect(wood_surface, "roughness", Sdf.ValueTypeNames.Float, orm, "g")
    connect(wood_surface, "metallic", Sdf.ValueTypeNames.Float, orm, "b")
    connect(wood_surface, "normal", Sdf.ValueTypeNames.Normal3f, normal, "rgb")

    plaster, plaster_surface, plaster_reader = looks.surface(
        "Plaster", diffuseColor=(0.62, 0.58, 0.52), roughness=0.85)
    plaster_normal = looks.texture(plaster, "Normal", "./textures/plaster_normal.png", "raw",
                                   plaster_reader, scale=(2, 2, 2, 1), bias=(-1, -1, -1, 0))
    connect(plaster_surface, "normal", Sdf.ValueTypeNames.Normal3f, plaster_normal, "rgb")
    ceiling, _, _ = looks.surface("Ceiling", diffuseColor=(0.8, 0.8, 0.78), roughness=0.9)
    metal, _, _ = looks.surface("BrushedMetal", diffuseColor=(0.91, 0.92, 0.92),
                                metallic=1.0, roughness=0.22)
    legs, _, _ = looks.surface("PaintedSteel", diffuseColor=(0.05, 0.05, 0.06),
                               metallic=1.0, roughness=0.45)
    screen, screen_surface, screen_reader = looks.surface(
        "Screen", diffuseColor=(0.02, 0.02, 0.02), roughness=0.15)
    screen_texture = looks.texture(screen, "Emissive", "./textures/screen_emissive.png", "sRGB",
                                   screen_reader, scale=(3, 3, 3, 1))
    connect(screen_surface, "emissiveColor", Sdf.ValueTypeNames.Color3f, screen_texture, "rgb")
    poster, poster_surface, poster_reader = looks.surface("Poster", roughness=0.7)
    poster_texture = looks.texture(poster, "BaseColor", "./textures/poster_basecolor.png",
                                   "sRGB", poster_reader)
    connect(poster_surface, "diffuseColor", Sdf.ValueTypeNames.Color3f, poster_texture, "rgb")
    leaves, leaf_surface, leaf_reader = looks.surface("Leaves", roughness=0.6,
                                                      opacityThreshold=0.5)
    leaf_texture = looks.texture(leaves, "BaseColor", "./textures/leaf_basecolor.png", "sRGB",
                                 leaf_reader)
    connect(leaf_surface, "diffuseColor", Sdf.ValueTypeNames.Color3f, leaf_texture, "rgb")
    connect(leaf_surface, "opacity", Sdf.ValueTypeNames.Float, leaf_texture, "a")
    crate_look, _, _ = looks.surface("CratePaint", diffuseColor=(0.55, 0.12, 0.08),
                                     roughness=0.5)

    shell = UsdGeom.Xform.Define(stage, "/Room/Shell")
    box(stage, "/Room/Shell/Floor", (-WALL, -WALL, -WALL), (WIDTH + WALL, 0, DEPTH + WALL), wood)
    box(stage, "/Room/Shell/Ceiling", (-WALL, HEIGHT, -WALL),
        (WIDTH + WALL, HEIGHT + WALL, DEPTH + WALL), ceiling)
    box(stage, "/Room/Shell/WallWest", (-WALL, 0, 0), (0, HEIGHT, DEPTH), plaster)
    box(stage, "/Room/Shell/WallNorth", (-WALL, 0, -WALL), (WIDTH + WALL, HEIGHT, 0), plaster)
    box(stage, "/Room/Shell/WallSouth", (-WALL, 0, DEPTH), (WIDTH + WALL, HEIGHT, DEPTH + WALL),
        plaster)
    (z0, z1), (y0, y1) = WINDOW["z"], WINDOW["y"]
    box(stage, "/Room/Shell/WallEastA", (WIDTH, 0, 0), (WIDTH + WALL, HEIGHT, z0), plaster)
    box(stage, "/Room/Shell/WallEastB", (WIDTH, 0, z1), (WIDTH + WALL, HEIGHT, DEPTH), plaster)
    box(stage, "/Room/Shell/WallEastSill", (WIDTH, 0, z0), (WIDTH + WALL, y0, z1), plaster)
    box(stage, "/Room/Shell/WallEastHead", (WIDTH, y1, z0), (WIDTH + WALL, HEIGHT, z1), plaster)
    del shell

    UsdGeom.Xform.Define(stage, "/Room/Props")
    sphere(stage, "/Room/Props/MetalSphere", (380.0, 45.0, 380.0), 45.0, metal)
    # Table: one mesh, two GeomSubsets bound to different materials.
    table = box(stage, "/Room/Props/Table", (120, 70, 300), (260, 76, 420), None)
    top_faces, leg_faces = [0, 1, 2, 3, 4, 5], []
    points = list(table.GetPointsAttr().Get())
    counts = list(table.GetFaceVertexCountsAttr().Get())
    indices = list(table.GetFaceVertexIndicesAttr().Get())
    st = list(UsdGeom.PrimvarsAPI(table).GetPrimvar("st").Get())
    for leg, (x, z) in enumerate(((125, 305), (250, 305), (125, 410), (250, 410))):
        base_index = len(points)
        (lx0, ly0, lz0), (lx1, ly1, lz1) = (x, 0, z), (x + 5, 70, z + 5)
        points += [Gf.Vec3f(*c) for c in [(lx0, ly0, lz0), (lx1, ly0, lz0), (lx1, ly1, lz0),
                                         (lx0, ly1, lz0), (lx0, ly0, lz1), (lx1, ly0, lz1),
                                         (lx1, ly1, lz1), (lx0, ly1, lz1)]]
        for face in [(0, 3, 2, 1), (4, 5, 6, 7), (0, 1, 5, 4), (2, 3, 7, 6), (1, 2, 6, 5),
                     (0, 4, 7, 3)]:
            leg_faces.append(len(counts))
            counts.append(4)
            indices += [base_index + i for i in face]
            st += [Gf.Vec2f(0, 0), Gf.Vec2f(1, 0), Gf.Vec2f(1, 1), Gf.Vec2f(0, 1)]
    table.GetPointsAttr().Set(points)
    table.GetFaceVertexCountsAttr().Set(counts)
    table.GetFaceVertexIndicesAttr().Set(indices)
    UsdGeom.PrimvarsAPI(table).GetPrimvar("st").Set(st)
    binding = UsdShade.MaterialBindingAPI.Apply(table.GetPrim())
    top = binding.CreateMaterialBindSubset("Top", Vt.IntArray(top_faces), UsdGeom.Tokens.face)
    UsdShade.MaterialBindingAPI.Apply(top.GetPrim()).Bind(wood)
    leg_subset = binding.CreateMaterialBindSubset("Legs", Vt.IntArray(leg_faces),
                                                  UsdGeom.Tokens.face)
    UsdShade.MaterialBindingAPI.Apply(leg_subset.GetPrim()).Bind(legs)
    # Wall-mounted emissive screen and the orientation poster (west wall, facing +x).
    quad(stage, "/Room/Props/Screen", [(1, 110, 560), (1, 110, 420), (1, 190, 420),
                                       (1, 190, 560)], screen)
    quad(stage, "/Room/Props/Poster", [(1, 100, 300), (1, 100, 220), (1, 180, 220),
                                       (1, 180, 300)], poster)
    # Two crossed alpha-cut frond cards in a corner.
    for k, angle in enumerate((0.6, 2.2)):
        dx, dz = 45 * math.cos(angle), 45 * math.sin(angle)
        cx, cz = 520, 90
        quad(stage, "/Room/Props/Frond%d" % k, [(cx - dx, 0, cz - dz), (cx + dx, 0, cz + dz),
                                                (cx + dx, 150, cz + dz),
                                                (cx - dx, 150, cz - dz)], leaves,
             double_sided=True)
    # Instanceable crates referencing an in-file prototype.
    prototype = stage.CreateClassPrim("/_CratePrototype")
    box(stage, "/_CratePrototype/Body", (-25, 0, -25), (25, 50, 25), crate_look)
    for k, (x, z) in enumerate(((80, 700), (140, 720))):
        crate = UsdGeom.Xform.Define(stage, "/Room/Props/Crate%d" % k)
        crate.GetPrim().GetReferences().AddInternalReference("/_CratePrototype")
        crate.GetPrim().SetInstanceable(True)
        crate.AddTranslateOp().Set(Gf.Vec3d(x, 0, z))
        crate.AddRotateYOp().Set(15.0 * k)
    del prototype
    # A point instancer of three small blocks on the floor.
    instancer = UsdGeom.PointInstancer.Define(stage, "/Room/Props/Blocks")
    UsdGeom.Scope.Define(stage, "/Room/Props/Blocks/Prototypes")
    box(stage, "/Room/Props/Blocks/Prototypes/Block", (-10, 0, -10), (10, 20, 10), crate_look)
    instancer.CreatePrototypesRel().SetTargets(["/Room/Props/Blocks/Prototypes/Block"])
    instancer.CreateProtoIndicesAttr([0, 0, 0])
    instancer.CreatePositionsAttr([Gf.Vec3f(300, 0, 650), Gf.Vec3f(330, 0, 690),
                                   Gf.Vec3f(360, 0, 640)])
    UsdGeom.Imageable(stage.GetPrimAtPath("/Room/Props/Blocks/Prototypes")).MakeInvisible()

    lights = UsdGeom.Xform.Define(stage, "/Room/Lights")
    del lights
    sun = UsdLux.DistantLight.Define(stage, "/Room/Lights/Sun")
    direction = np.array((-0.62, -0.42, 0.25))
    direction /= np.linalg.norm(direction)
    sun.AddTransformOp().Set(look_at((0, 0, 0), tuple(direction)))
    sun.CreateIntensityAttr(4.0)
    sun.CreateNormalizeAttr(True)
    sun.CreateAngleAttr(0.53)
    sun.CreateColorAttr(Gf.Vec3f(1.0, 0.95, 0.86))
    panel = UsdLux.RectLight.Define(stage, "/Room/Lights/CeilingPanel")
    panel.AddTranslateOp().Set(Gf.Vec3d(300, HEIGHT - 1, 400))
    panel.AddRotateXOp().Set(-90.0)
    panel.CreateWidthAttr(80.0)
    panel.CreateHeightAttr(80.0)
    panel.CreateIntensityAttr(6.0)
    panel.CreateColorAttr(Gf.Vec3f(1.0, 0.9, 0.75))
    dome = UsdLux.DomeLight.Define(stage, "/Room/Lights/Sky")
    dome.CreateTextureFileAttr("./textures/sky.hdr")
    dome.CreateTextureFormatAttr(UsdLux.Tokens.latlong)
    dome.CreateIntensityAttr(1.0)

    camera = UsdGeom.Camera.Define(stage, "/Room/Camera")
    camera.AddTransformOp().Set(look_at((70.0, 160.0, 760.0), (420.0, 110.0, 250.0)))
    camera.CreateFocalLengthAttr(20.0)
    camera.CreateHorizontalApertureAttr(36.0)
    camera.CreateVerticalApertureAttr(20.25)
    camera.CreateClippingRangeAttr(Gf.Vec2f(1.0, 100000.0))
    stage.GetRootLayer().Save()
    return path


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--out", type=Path, default=DEFAULT_OUT)
    args = parser.parse_args()
    print(build(args.out))


if __name__ == "__main__":
    main()
