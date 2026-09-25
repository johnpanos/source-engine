#!/usr/bin/env python3
"""Generate the RFC 0011 global-illumination fixtures under quality/fixtures/gi/.

Each fixture is an authored OpenUSD stage (Z-up meters) plus one override
layer per interaction state, and a `gi-fixture/v1` declaration
(`fixture.json`) that names the states, the cameras, the regions the oracles
score, the analytic values where a closed form exists, and the map manifest
that builds the fixture's baked state into a playable BSP2 map:

  furnace      closed box, uniform albedo 0.5 and uniform emission: infinite
               bounce radiosity L = Le / (1 - rho) everywhere
  thin-wall    two sealed rooms split by a 4-unit panel, only one lit
  room-states  the usd-room fixture with its ceiling panel, sun and screen
               switched, and a dynamic model under the panel
  door         a room lit through a doorway, door open and closed
  probe-grid   a large floor under a uniform sky, and under one sun
  portal-view  a lit chamber seen directly and from the two portal cameras
  swing        a bulb swinging on a rope, at rest and 40 degrees either side

plus the gallery (`gi_gallery.py`): dozens of smaller fixtures (colored
lights blending, colored and cluttered furnaces, color bleeding, leaks,
symmetry, intensity ranges) whose relational `oracles` `gi_oracles.py`
evaluates.

Materials are Lambertian (UsdPreviewSurface ior 1, so Cycles' Principled BSDF
has no specular lobe) unless a fixture says otherwise. Dynamic models are
`Xform` prims carrying `sourceEngine:model`; their child mesh is the Cycles
stand-in for the Source model, which the map pipeline places as a
`prop_dynamic` and keeps out of the world mesh and the bake. Output is
deterministic; rerun to regenerate:

    PYTHONPATH=build/toolchains/openusd-25.11/lib/python \\
        /usr/bin/python3.12 tools/quality/gi_fixtures.py [--check]

`--check` regenerates into a temporary directory and fails when any file
differs from the checked-in fixtures.
"""

import argparse
import filecmp
import json
import math
import shutil
import sys
import tempfile
from pathlib import Path

from pxr import Gf, Sdf, Usd, UsdGeom, UsdLux, UsdShade, Vt

ROOT = Path(__file__).resolve().parents[2]
DEFAULT_OUT = ROOT / "quality/fixtures/gi"
SCHEMA = "gi-fixture/v1"
SOURCE_UNITS_PER_METER = 39.37007874015748
# The dynamic model every fixture places: models/props/sphere.mdl, whose hull
# is +-20.25 Source units around its origin (read from the MDL header).
PROBE_MODEL = "models/props/sphere.mdl"
PROBE_RADIUS_M = 20.25 / SOURCE_UNITS_PER_METER
# Reference film: 4:3 with a 90 degree horizontal fov, which Source's `fov 90`
# reproduces exactly, so engine captures need no crop.
FILM = {"width": 256, "height": 192}
HORIZONTAL_FOV = 90.0
# The 4-unit panel of `thin-wall` (RFC 0011 fixture table).
THIN_WALL_M = 4.0 / SOURCE_UNITS_PER_METER
# Every dynamic model a fixture places (Author.probe_model / room_states).
DYNAMIC_MODELS = ("ProbeSphere", "ProbeA", "ProbeB", "ProbeC", "ProbeD", "ProbeS")


# ------------------------------------------------------------------ geometry

def planar_st(points, normal):
    """1 m planar UVs on the face's two tangent axes."""
    axis = max(range(3), key=lambda i: abs(normal[i]))
    u_axis, v_axis = [i for i in range(3) if i != axis]
    return [(p[u_axis], p[v_axis]) for p in points]


def quad(lo, hi, axis, facing):
    """Axis-aligned rectangle on plane `axis` = lo[axis] (= hi[axis]), with
    its front face toward `facing` (+1 or -1 along `axis`)."""
    u, v = [i for i in range(3) if i != axis]
    corners = []
    for cu, cv in ((0, 0), (1, 0), (1, 1), (0, 1)):
        point = [0.0, 0.0, 0.0]
        point[axis] = lo[axis]
        point[u] = (lo, hi)[cu][u]
        point[v] = (lo, hi)[cv][v]
        corners.append(tuple(point))
    normal = [0.0, 0.0, 0.0]
    normal[axis] = float(facing)
    # Counter-clockwise seen from the front (USD right-handed default).
    a, b, c = (Gf.Vec3d(*corners[i]) for i in range(3))
    if Gf.Dot(Gf.Cross(b - a, c - a), Gf.Vec3d(*normal)) < 0:
        corners.reverse()
    return corners, tuple(normal)


def box_faces(lo, hi, inward=True, skip=()):
    """The six faces of a box as (name, corners, normal); inward faces for a
    room interior, outward faces for a solid."""
    faces = []
    for axis, letter in enumerate("XYZ"):
        for side, sign in ((0, "n"), (1, "p")):
            name = letter + sign
            if name in skip:
                continue
            plane_lo, plane_hi = list(lo), list(hi)
            plane_lo[axis] = plane_hi[axis] = (lo, hi)[side][axis]
            outward = -1 if side == 0 else 1
            corners, normal = quad(plane_lo, plane_hi, axis, -outward if inward else outward)
            faces.append((name, corners, normal))
    return faces


def rect_with_hole(plane, axis, facing, outer, hole):
    """Faces of a wall rectangle `outer` (2D on the two non-`axis` axes) with
    a rectangular opening `hole`, as up to four quads."""
    u, v = [i for i in range(3) if i != axis]
    (u0, v0), (u1, v1) = outer
    (hu0, hv0), (hu1, hv1) = hole
    pieces = [((u0, v0), (hu0, v1)), ((hu1, v0), (u1, v1)),
              ((hu0, v0), (hu1, hv0)), ((hu0, hv1), (hu1, v1))]
    faces = []
    for (a0, b0), (a1, b1) in pieces:
        if a1 - a0 <= 1e-9 or b1 - b0 <= 1e-9:
            continue
        lo, hi = [0.0] * 3, [0.0] * 3
        lo[axis] = hi[axis] = plane
        lo[u], lo[v], hi[u], hi[v] = a0, b0, a1, b1
        faces.append(quad(lo, hi, axis, facing))
    return faces


class Author:
    """Writes one fixture stage: materials, meshes, lights, cameras, props."""

    def __init__(self, path, name):
        self.path = Path(path)
        self.path.parent.mkdir(parents=True, exist_ok=True)
        if self.path.exists():
            self.path.unlink()
        self.stage = Usd.Stage.CreateNew(str(self.path))
        self.stage.SetMetadata("comment", "RFC 0011 GI fixture %s; generated by "
                               "tools/quality/gi_fixtures.py, do not edit" % name)
        UsdGeom.SetStageUpAxis(self.stage, UsdGeom.Tokens.z)
        UsdGeom.SetStageMetersPerUnit(self.stage, 1.0)
        self.root = UsdGeom.Xform.Define(self.stage, "/" + name)
        self.stage.SetDefaultPrim(self.root.GetPrim())
        self.root_path = self.root.GetPath()
        UsdGeom.Scope.Define(self.stage, self.root_path.AppendChild("Looks"))
        UsdGeom.Xform.Define(self.stage, self.root_path.AppendChild("World"))
        UsdGeom.Xform.Define(self.stage, self.root_path.AppendChild("Lights"))
        self.materials = {}

    def material(self, name, albedo, emission=(0.0, 0.0, 0.0), roughness=1.0):
        path = self.root_path.AppendPath("Looks/" + name)
        material = UsdShade.Material.Define(self.stage, path)
        shader = UsdShade.Shader.Define(self.stage, path.AppendChild("Surface"))
        shader.CreateIdAttr("UsdPreviewSurface")
        shader.CreateInput("diffuseColor", Sdf.ValueTypeNames.Color3f).Set(Gf.Vec3f(*albedo))
        shader.CreateInput("roughness", Sdf.ValueTypeNames.Float).Set(float(roughness))
        shader.CreateInput("metallic", Sdf.ValueTypeNames.Float).Set(0.0)
        # ior 1: no Fresnel lobe, so Cycles' Principled BSDF is Lambertian.
        shader.CreateInput("ior", Sdf.ValueTypeNames.Float).Set(1.0)
        shader.CreateInput("emissiveColor", Sdf.ValueTypeNames.Color3f).Set(
            Gf.Vec3f(*emission))
        material.CreateSurfaceOutput().ConnectToSource(shader.ConnectableAPI(), "surface")
        self.materials[name] = material
        return material

    def mesh(self, path, faces, material, parent="World"):
        """A mesh of quads [(corners, normal)] with faceVarying normals and st."""
        prim_path = self.root_path.AppendPath(parent + "/" + path)
        mesh = UsdGeom.Mesh.Define(self.stage, prim_path)
        points, counts, indices, normals, st = [], [], [], [], []
        for corners, normal in faces:
            base = len(points)
            points.extend(corners)
            counts.append(len(corners))
            indices.extend(range(base, base + len(corners)))
            normals.extend([normal] * len(corners))
            st.extend(planar_st(corners, normal))
        mesh.CreatePointsAttr(Vt.Vec3fArray([Gf.Vec3f(*p) for p in points]))
        mesh.CreateFaceVertexCountsAttr(Vt.IntArray(counts))
        mesh.CreateFaceVertexIndicesAttr(Vt.IntArray(indices))
        mesh.CreateNormalsAttr(Vt.Vec3fArray([Gf.Vec3f(*n) for n in normals]))
        mesh.SetNormalsInterpolation(UsdGeom.Tokens.faceVarying)
        mesh.CreateSubdivisionSchemeAttr(UsdGeom.Tokens.none)
        mesh.CreateExtentAttr(UsdGeom.PointBased.ComputeExtent(mesh.GetPointsAttr().Get()))
        primvar = UsdGeom.PrimvarsAPI(mesh).CreatePrimvar(
            "st", Sdf.ValueTypeNames.TexCoord2fArray, UsdGeom.Tokens.faceVarying)
        primvar.Set(Vt.Vec2fArray([Gf.Vec2f(*uv) for uv in st]))
        UsdShade.MaterialBindingAPI.Apply(mesh.GetPrim()).Bind(self.materials[material])
        return mesh

    def room(self, prefix, lo, hi, material, skip=()):
        """One mesh per interior face, named <prefix>_<Xn|Xp|Yn|Yp|Zn|Zp>."""
        names = []
        for face, corners, normal in box_faces(lo, hi, inward=True, skip=skip):
            names.append(prefix + "_" + face)
            self.mesh(names[-1], [(corners, normal)], material)
        return names

    def solid(self, name, lo, hi, material, parent="World"):
        faces = [(corners, normal) for _, corners, normal in box_faces(lo, hi, inward=False)]
        self.mesh(name, faces, material, parent)
        return name

    def rect_light(self, name, center, size, intensity, color=(1.0, 1.0, 1.0)):
        """A ceiling RectLight at `center` emitting straight down (local -Z)."""
        light = UsdLux.RectLight.Define(self.stage, self.root_path.AppendPath("Lights/" + name))
        light.CreateWidthAttr(float(size[0]))
        light.CreateHeightAttr(float(size[1]))
        light.CreateIntensityAttr(float(intensity))
        light.CreateColorAttr(Gf.Vec3f(*color))
        light.CreateNormalizeAttr(False)
        light.AddTranslateOp().Set(Gf.Vec3d(*center))
        return light

    def distant_light(self, name, direction, irradiance, angle=0.53):
        """A DistantLight whose rays travel along `direction` (Z-up)."""
        light = UsdLux.DistantLight.Define(self.stage,
                                           self.root_path.AppendPath("Lights/" + name))
        light.CreateIntensityAttr(float(irradiance))
        light.CreateAngleAttr(float(angle))
        # Normalized intensity: the perpendicular irradiance, independent of the disk size.
        light.CreateNormalizeAttr(True)
        light.AddTransformOp().Set(look_rotation(direction))
        return light

    def dome_light(self, name, radiance):
        light = UsdLux.DomeLight.Define(self.stage, self.root_path.AppendPath("Lights/" + name))
        light.CreateIntensityAttr(float(radiance))
        light.CreateColorAttr(Gf.Vec3f(1.0, 1.0, 1.0))
        return light

    def camera(self, name, pose):
        camera = UsdGeom.Camera.Define(self.stage, self.root_path.AppendChild(name))
        focal = 18.0
        camera.CreateFocalLengthAttr(focal)
        horizontal = 2 * focal * math.tan(math.radians(HORIZONTAL_FOV) / 2)
        camera.CreateHorizontalApertureAttr(horizontal)
        camera.CreateVerticalApertureAttr(horizontal * FILM["height"] / FILM["width"])
        camera.CreateClippingRangeAttr(Gf.Vec2f(0.01, 1000.0))
        camera.AddTransformOp().Set(camera_matrix(pose))
        return camera

    def probe_model(self, name, center, material):
        """A dynamic model: prop Xform carrying the Source model path, with a
        Cycles stand-in sphere of the model's radius as its child."""
        path = self.root_path.AppendPath("Props/" + name)
        UsdGeom.Xform.Define(self.stage, self.root_path.AppendChild("Props"))
        xform = UsdGeom.Xform.Define(self.stage, path)
        xform.AddTranslateOp().Set(Gf.Vec3d(*center))
        prim = xform.GetPrim()
        prim.CreateAttribute("sourceEngine:model", Sdf.ValueTypeNames.String,
                             custom=True).Set(PROBE_MODEL)
        prim.CreateAttribute("sourceEngine:role", Sdf.ValueTypeNames.Token,
                             custom=True).Set("prop")
        sphere_mesh(self.stage, path.AppendChild(name + "Shape"), PROBE_RADIUS_M, 48, 24)
        UsdShade.MaterialBindingAPI.Apply(
            self.stage.GetPrimAtPath(path.AppendChild(name + "Shape"))).Bind(
                self.materials[material])
        return path

    def save(self):
        self.stage.GetRootLayer().Save()


def sphere_mesh(stage, path, radius, longitudes, latitudes):
    mesh = UsdGeom.Mesh.Define(stage, path)
    points, normals, st, counts, indices = [], [], [], [], []

    def vertex(row, column):
        theta = math.pi * row / latitudes
        phi = 2 * math.pi * column / longitudes
        direction = (math.sin(theta) * math.cos(phi), math.sin(theta) * math.sin(phi),
                     math.cos(theta))
        return direction, (column / longitudes, 1.0 - row / latitudes)
    for row in range(latitudes):
        for column in range(longitudes):
            corners = [(row, column), (row + 1, column), (row + 1, column + 1),
                       (row, column + 1)]
            if row == 0:
                corners = corners[:3]
            elif row == latitudes - 1:
                corners = [corners[0], corners[1], corners[3]]
            counts.append(len(corners))
            for r, c in corners:
                direction, uv = vertex(r, c)
                indices.append(len(points))
                points.append(tuple(radius * d for d in direction))
                normals.append(direction)
                st.append(uv)
    mesh.CreatePointsAttr(Vt.Vec3fArray([Gf.Vec3f(*p) for p in points]))
    mesh.CreateFaceVertexCountsAttr(Vt.IntArray(counts))
    mesh.CreateFaceVertexIndicesAttr(Vt.IntArray(indices))
    mesh.CreateNormalsAttr(Vt.Vec3fArray([Gf.Vec3f(*n) for n in normals]))
    mesh.SetNormalsInterpolation(UsdGeom.Tokens.faceVarying)
    mesh.CreateSubdivisionSchemeAttr(UsdGeom.Tokens.none)
    mesh.CreateExtentAttr(UsdGeom.PointBased.ComputeExtent(mesh.GetPointsAttr().Get()))
    primvar = UsdGeom.PrimvarsAPI(mesh).CreatePrimvar(
        "st", Sdf.ValueTypeNames.TexCoord2fArray, UsdGeom.Tokens.faceVarying)
    primvar.Set(Vt.Vec2fArray([Gf.Vec2f(*uv) for uv in st]))
    return mesh


def normalize(v):
    length = math.sqrt(sum(c * c for c in v))
    return tuple(c / length for c in v)


def cross3(a, b):
    return (a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0])


def look_rotation(direction):
    """Row-vector matrix whose local -Z is `direction`."""
    back = normalize(tuple(-c for c in direction))
    helper = (0.0, 0.0, 1.0) if abs(back[2]) < 0.99 else (1.0, 0.0, 0.0)
    x = normalize(cross3(helper, back))
    y = cross3(back, x)
    return Gf.Matrix4d(x[0], x[1], x[2], 0, y[0], y[1], y[2], 0, back[0], back[1], back[2], 0,
                       0, 0, 0, 1)


def camera_pose(eye, target):
    forward = normalize(tuple(t - e for t, e in zip(target, eye)))
    return {"eye": [float(c) for c in eye], "forward": [float(c) for c in forward],
            "up": [0.0, 0.0, 1.0]}


def camera_matrix(pose):
    """USD camera looks down local -Z with +Y up."""
    m = look_rotation(pose["forward"])
    m.SetTranslateOnly(Gf.Vec3d(*pose["eye"]))
    return m


def copy_stage_metadata(layer, base):
    """Stage metadata (up axis, units, default prim) is read from the root
    layer only; an override layer must repeat its base's."""
    source = Sdf.Layer.FindOrOpen(str(base))
    for key in ("upAxis", "metersPerUnit", "defaultPrim"):
        if source.pseudoRoot.HasInfo(key):
            layer.pseudoRoot.SetInfo(key, source.pseudoRoot.GetInfo(key))


def state_layer(path, base, edits):
    """An override layer over `base` applying `edits` [(prim path, attribute, type, value)]."""
    path.parent.mkdir(parents=True, exist_ok=True)
    if path.exists():
        path.unlink()
    layer = Sdf.Layer.CreateNew(str(path))
    layer.subLayerPaths.append(str(Path("..") / base.name) if path.parent != base.parent
                               else base.name)
    copy_stage_metadata(layer, base)
    stage = Usd.Stage.Open(layer)
    stage.SetEditTarget(stage.GetRootLayer())
    for prim_path, attribute, value_type, value in edits:
        prim = stage.OverridePrim(prim_path)
        if attribute == "visibility":
            UsdGeom.Imageable(prim).CreateVisibilityAttr().Set(value)
        elif attribute == "xformOp:transform":
            prim.CreateAttribute(attribute, value_type).Set(value)
        else:
            prim.CreateAttribute(attribute, value_type).Set(value)
    layer.Save()
    return path


# ------------------------------------------------------------------ fixtures

def write_json(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, indent=2, sort_keys=True) + "\n")


def map_manifest(name, scene, solid_meshes=(), envelope=(), probe_bounds=None):
    """The fixture's baked state as a playable map, in the `gi-fixture` export
    profile (quality/map_export_profiles/gi-fixture.json). `probe_bounds`
    (meters, min xyz then max xyz) replaces the probe grid's default bounds,
    the world meshes' own, for an open scene."""
    manifest = {"schema": "pbrt-map-manifest/v1", "map": name, "scene": scene,
                "quality": "gi-fixture",
                "collision": {"solid_meshes": list(solid_meshes)}}
    if envelope:
        manifest["collision"]["envelope_meshes"] = list(envelope)
    if probe_bounds:
        manifest["probe_volume"] = {"bounds_m": list(probe_bounds)}
    return manifest


def fixture_record(name, purpose, stage, states, cameras, regions, baked_state, extra=None):
    # Region entries naming a dynamic model (rather than a mesh) cover the
    # model's stand-in mesh, `<name>Shape`.
    models = sorted({entry for view in regions.values() for entries in view.values()
                     for entry in entries if entry in DYNAMIC_MODELS})
    record = {"schema": SCHEMA, "name": name, "purpose": purpose, "stage": stage,
              "dynamic_models": [{"name": model, "stand_in_mesh": model + "Shape"}
                                 for model in models],
              "states": states, "baked_state": baked_state, "cameras": cameras,
              "film": FILM, "horizontal_fov_degrees": HORIZONTAL_FOV,
              # Every generated fixture material is Lambertian (ior 1); room-states
              # reuses the usd-room's authored materials.
              "lambertian": name != "room-states",
              "regions": regions, "map_manifest": "map.json",
              "probe_model": {"model": PROBE_MODEL, "radius_m": PROBE_RADIUS_M}}
    record.update(extra or {})
    return record


def furnace(out):
    """Closed 3 m box; every wall albedo 0.6 and emission 0.3 (linear): L = 0.75,
    direct light 0.3 and indirect 0.45, so swapped layers are also detected."""
    directory = out / "furnace"
    author = Author(directory / "furnace.usda", "Furnace")
    rho, emission = 0.6, 0.3
    author.material("FurnaceWall", (rho,) * 3, (emission,) * 3)
    author.material("ProbeGrey", (rho,) * 3, (emission,) * 3)
    walls = author.room("Box", (0.0, 0.0, 0.0), (3.0, 3.0, 3.0), "FurnaceWall")
    # An emissive grey model is part of the furnace, so it changes nothing.
    author.probe_model("ProbeSphere", (1.5, 1.5, 1.2), "ProbeGrey")
    pose = camera_pose((0.35, 0.35, 1.6), (3.0, 3.0, 1.0))
    author.camera("Camera", pose)
    author.save()
    total = round(emission / (1 - rho), 9)
    write_json(directory / "fixture.json", fixture_record(
        "furnace", "Analytic infinite-bounce radiosity: detects missing or double-counted "
        "bounce", "furnace.usda", {"default": {"layer": None}},
        {"inside": pose}, {"inside": {"walls": walls, "model": ["ProbeSphere"]}}, "default",
        {"analytic": {
            "albedo": rho, "emission": emission,
            "radiance": total,
            "diffuse_light_total": total,
            "diffuse_light_direct": emission,
            "diffuse_light_indirect": round(total - emission, 9),
            "diffuse_light_one_bounce_indirect": round(rho * emission, 9),
            "note": "Every surface sees radiance L = Le / (1 - rho) in every direction. "
                    "Cycles diffuse light passes (irradiance / pi, no albedo): direct = Le "
                    "(emission seen directly), indirect = L - Le. A probe anywhere inside has "
                    "irradiance / pi = L on every normal."}}))
    write_json(directory / "map.json", map_manifest(
        "gi_furnace", "quality/fixtures/gi/furnace/furnace.usda"))


def thin_wall(out):
    """Rooms A (lit) and B (dark), each 3 x 3 x 3 m, split by a 4-unit panel."""
    directory = out / "thin-wall"
    author = Author(directory / "thin-wall.usda", "ThinWall")
    author.material("Wall", (0.6, 0.6, 0.6))
    author.material("ProbeGrey", (0.6, 0.6, 0.6))
    split = 3.0
    b0 = split + THIN_WALL_M
    room_a = author.room("A", (0.0, 0.0, 0.0), (split, 3.0, 3.0), "Wall")
    room_b = author.room("B", (b0, 0.0, 0.0), (b0 + 3.0, 3.0, 3.0), "Wall")
    author.rect_light("PanelA", (1.5, 1.5, 2.99), (1.0, 1.0), 8.0)
    # Models 0.6 m from each side of the panel, where an 0.5 m probe spacing
    # puts probes of both rooms in one trilinear cell.
    author.probe_model("ProbeA", (split - 0.6, 1.5, 1.2), "ProbeGrey")
    author.probe_model("ProbeB", (b0 + 0.6, 1.5, 1.2), "ProbeGrey")
    pose_a = camera_pose((0.3, 0.3, 1.6), (split, 2.0, 1.0))
    pose_b = camera_pose((b0 + 2.7, 2.7, 1.6), (b0, 1.0, 1.0))
    author.camera("Camera", pose_b)
    author.save()
    write_json(directory / "fixture.json", fixture_record(
        "thin-wall", "Probe leak control: indirect light in the dark room B must stay below "
        "the leak bound; a visibility-disabled build must exceed it", "thin-wall.usda",
        {"default": {"layer": None}}, {"lit": pose_a, "dark": pose_b},
        {"lit": {"walls": room_a, "model": ["ProbeA"]},
         "dark": {"walls": room_b, "model": ["ProbeB"], "panel_face": ["B_Xn"]}},
        "default",
        {"leak_bound": {"provisional": 0.02, "measure": "dark-room mean indirect / lit-room "
                        "mean indirect"},
         "panel": {"x_min_m": split, "x_max_m": b0, "thickness_source_units": 4.0}}))
    write_json(directory / "map.json", map_manifest(
        "gi_thin_wall", "quality/fixtures/gi/thin-wall/thin-wall.usda"))


def room_states(out):
    """The usd-room fixture's lights switched, plus a dynamic model."""
    directory = out / "room-states"
    directory.mkdir(parents=True, exist_ok=True)
    base = directory / "room-states.usda"
    if base.exists():
        base.unlink()
    layer = Sdf.Layer.CreateNew(str(base))
    layer.comment = ("RFC 0011 GI fixture room-states; generated by "
                     "tools/quality/gi_fixtures.py, do not edit")
    layer.subLayerPaths.append("../../usd-maps/room/room.usda")
    copy_stage_metadata(layer, directory / "../../usd-maps/room/room.usda")
    stage = Usd.Stage.Open(layer)
    stage.SetEditTarget(layer)
    # Y-up centimeters (room.usda's units): the model hangs under the ceiling
    # panel (centre (300, 299, 400)), clear of the metal sphere and the table.
    looks = "/Room/Looks/GiProbeGrey"
    material = UsdShade.Material.Define(stage, looks)
    shader = UsdShade.Shader.Define(stage, looks + "/Surface")
    shader.CreateIdAttr("UsdPreviewSurface")
    shader.CreateInput("diffuseColor", Sdf.ValueTypeNames.Color3f).Set(Gf.Vec3f(0.5, 0.5, 0.5))
    shader.CreateInput("roughness", Sdf.ValueTypeNames.Float).Set(1.0)
    shader.CreateInput("ior", Sdf.ValueTypeNames.Float).Set(1.0)
    material.CreateSurfaceOutput().ConnectToSource(shader.ConnectableAPI(), "surface")
    UsdGeom.Xform.Define(stage, "/Room/GiProps")
    xform = UsdGeom.Xform.Define(stage, "/Room/GiProps/ProbeSphere")
    xform.AddTranslateOp().Set(Gf.Vec3d(270.0, 140.0, 450.0))
    xform.GetPrim().CreateAttribute("sourceEngine:model", Sdf.ValueTypeNames.String,
                                    custom=True).Set(PROBE_MODEL)
    xform.GetPrim().CreateAttribute("sourceEngine:role", Sdf.ValueTypeNames.Token,
                                    custom=True).Set("prop")
    shape = sphere_mesh(stage, "/Room/GiProps/ProbeSphere/ProbeSphereShape",
                        PROBE_RADIUS_M * 100.0, 48, 24)
    UsdShade.MaterialBindingAPI.Apply(shape.GetPrim()).Bind(material)
    layer.Save()
    states = {"default": {"layer": None, "note": "sun, ceiling panel, sky and screen on"}}
    edits = {
        "panel-off": [("/Room/Lights/CeilingPanel", "inputs:intensity",
                       Sdf.ValueTypeNames.Float, 0.0)],
        "screen-off": [("/Room/Looks/Screen/Surface", "inputs:emissiveColor",
                        Sdf.ValueTypeNames.Color3f, Gf.Vec3f(0.0, 0.0, 0.0))],
        "sun-low": [("/Room/Lights/Sun", "xformOp:transform", Sdf.ValueTypeNames.Matrix4d,
                     sun_low_matrix())],
    }
    notes = {"panel-off": "ceiling RectLight intensity 0",
             "screen-off": "emissive screen texture disconnected (emission 0)",
             "sun-low": "sun lowered to 12 degrees elevation, rotated 25 degrees in azimuth"}
    for state, state_edits in edits.items():
        if state == "screen-off":
            state_edits = state_edits + [("/Room/Looks/Screen/Surface",
                                          "inputs:emissiveColor:connect", None, None)]
        path = directory / "states" / (state + ".usda")
        write_state_with_connections(path, base, state_edits)
        states[state] = {"layer": "states/%s.usda" % state, "note": notes[state]}
    # The moved sun's direction of travel in stage (Z-up) axes, which Source
    # shares: the runtime's light-direction override (G6.3).
    elevation, azimuth = math.radians(SUN_LOW_ELEVATION), math.radians(SUN_LOW_AZIMUTH)
    states["sun-low"]["sun_direction"] = [
        round(math.cos(elevation) * math.cos(azimuth), 6),
        round(-math.cos(elevation) * math.sin(azimuth), 6), round(-math.sin(elevation), 6)]
    # Room interior (Z-up meters after conversion): x 0..6, y -8..0, z 0..3.
    # (Y-up cm (x, y, z) -> Z-up m (x, -z, y) / 100.)
    model = (2.7, -4.5, 1.4)
    pose = camera_pose((1.2, -1.4, 1.5), (model[0], model[1], 1.0))
    write_json(directory / "fixture.json", fixture_record(
        "room-states", "Light-state changes (panel, sun, screen) in the usd-room interior; a "
        "dynamic model under the ceiling panel measures model lighting against the bake",
        "room-states.usda", states, {"model": pose},
        {"model": {"model": ["ProbeSphere"],
                   "floor": ["Floor"], "ceiling": ["Ceiling"],
                   "walls": ["WallWest", "WallNorth", "WallSouth"]}},
        "default", {"model_center_m": list(model)}))
    manifest = map_manifest("gi_room_states", "quality/fixtures/gi/room-states/room-states.usda")
    manifest["collision"]["solid_meshes"] = ["Table_Top", "Table_Legs", "MetalSphere", "Body",
                                             "Body_i1"]
    write_json(directory / "map.json", manifest)


def write_state_with_connections(path, base, edits):
    """state_layer with support for removing a connection (value None)."""
    path.parent.mkdir(parents=True, exist_ok=True)
    if path.exists():
        path.unlink()
    layer = Sdf.Layer.CreateNew(str(path))
    layer.subLayerPaths.append("../" + base.name)
    copy_stage_metadata(layer, base)
    stage = Usd.Stage.Open(layer)
    stage.SetEditTarget(layer)
    for prim_path, attribute, value_type, value in edits:
        prim = stage.OverridePrim(prim_path)
        if attribute.endswith(":connect"):
            name = attribute[:-len(":connect")]
            attr = prim.GetAttribute(name) or prim.CreateAttribute(
                name, Sdf.ValueTypeNames.Color3f)
            # An empty explicit connection list blocks the base layer's source.
            attr.SetConnections([])
            continue
        prim.CreateAttribute(attribute, value_type).Set(value)
    layer.Save()


SUN_LOW_ELEVATION, SUN_LOW_AZIMUTH = 12.0, 183.0  # degrees, room.usda's Y-up frame


def sun_low_matrix():
    """room.usda's Sun (Y-up) re-aimed: rays at 12 degrees elevation (the
    default is 32) and 25 degrees further round in azimuth (default 158)."""
    elevation, azimuth = math.radians(SUN_LOW_ELEVATION), math.radians(SUN_LOW_AZIMUTH)
    # Y-up direction of travel: down (-Y) and across the XZ plane.
    travel = (math.cos(elevation) * math.cos(azimuth), -math.sin(elevation),
              math.cos(elevation) * math.sin(azimuth))
    back = normalize(tuple(-c for c in travel))
    helper = (0.0, 1.0, 0.0)
    x = normalize(cross3(helper, back))
    y = cross3(back, x)
    return Gf.Matrix4d(x[0], x[1], x[2], 0, y[0], y[1], y[2], 0, back[0], back[1], back[2], 0,
                       0, 0, 0, 1)


def door(out):
    """Room A (lit, 4 x 4 x 3 m) and room B joined by a 1 x 2 m doorway in a
    0.2 m wall; the door slab fills the doorway only in state `closed`."""
    directory = out / "door"
    author = Author(directory / "door.usda", "Door")
    author.material("Wall", (0.6, 0.6, 0.6))
    author.material("DoorPaint", (0.6, 0.6, 0.6))
    author.material("ProbeGrey", (0.6, 0.6, 0.6))
    wall0, wall1 = 4.0, 4.2
    hole_y, hole_z = (1.5, 2.5), (0.0, 2.0)
    room_a = author.room("A", (0.0, 0.0, 0.0), (wall0, 4.0, 3.0), "Wall", skip=("Xp",))
    room_b = author.room("B", (wall1, 0.0, 0.0), (wall1 + 4.0, 4.0, 3.0), "Wall", skip=("Xn",))
    outer = ((0.0, 0.0), (4.0, 3.0))
    hole = ((hole_y[0], hole_z[0]), (hole_y[1], hole_z[1]))
    author.mesh("A_Xp", rect_with_hole(wall0, 0, -1, outer, hole), "Wall")
    author.mesh("B_Xn", rect_with_hole(wall1, 0, 1, outer, hole), "Wall")
    jambs = [quad((wall0, hole_y[0], hole_z[0]), (wall1, hole_y[0], hole_z[1]), 1, 1),
             quad((wall0, hole_y[1], hole_z[0]), (wall1, hole_y[1], hole_z[1]), 1, -1),
             quad((wall0, hole_y[0], hole_z[1]), (wall1, hole_y[1], hole_z[1]), 2, -1)]
    author.mesh("Doorway", jambs, "Wall")
    author.solid("DoorSlab", (wall0 + 0.075, hole_y[0], hole_z[0]),
                 (wall0 + 0.125, hole_y[1], hole_z[1]), "DoorPaint")
    author.rect_light("PanelA", (2.0, 2.0, 2.99), (1.2, 1.2), 10.0)
    author.probe_model("ProbeB", (wall1 + 1.5, 2.0, 1.2), "ProbeGrey")
    pose = camera_pose((wall1 + 3.7, 3.7, 1.6), (wall1 + 1.0, 1.0, 0.8))
    author.camera("Camera", pose)
    author.save()
    # The base stage has the door closed; `open` hides the slab.
    state_layer(directory / "states" / "open.usda", directory / "door.usda",
                [("/Door/World/DoorSlab", "visibility", None, UsdGeom.Tokens.invisible)])
    write_json(directory / "fixture.json", fixture_record(
        "door", "Geometry-motion oracle: closing the door darkens room B's indirect light",
        "door.usda",
        {"closed": {"layer": None, "note": "door slab fills the doorway"},
         "open": {"layer": "states/open.usda", "note": "door slab hidden"}},
        {"far": pose},
        {"far": {"walls": [n for n in room_b if n != "B_Xn"] + ["B_Xn"],
                 "model": ["ProbeB"]}},
        "open", {"door_mesh": "DoorSlab",
                 "door_bounds_m": [[wall0 + 0.075, hole_y[0], hole_z[0]],
                                   [wall0 + 0.125, hole_y[1], hole_z[1]]]}))
    manifest = map_manifest("gi_door", "quality/fixtures/gi/door/states/open.usda",
                            solid_meshes=["A_Xp", "B_Xn"])
    # The slab as a moving brush entity (open at spawn): the geometry change
    # the SDF-traced producer must follow and radiosity cannot (G6).
    # It draws with the map's wall material (the same 0.6 grey as DoorPaint),
    # in its legacy lightmapped form: a brush entity is not world mesh.
    manifest["collision"]["doors"] = [{"name": "Door", "bounds_m": [
        [wall0 + 0.075, hole_y[0], hole_z[0]], [wall0 + 0.125, hole_y[1], hole_z[1]]],
        "material": "gi_door_fallback/wall"}]
    write_json(directory / "map.json", manifest)


def probe_grid(out):
    """A 40 x 40 m floor of albedo 0.5 under a uniform sky (default) or one
    sun (state `sun`), with known probe positions."""
    directory = out / "probe-grid"
    author = Author(directory / "probe-grid.usda", "ProbeGrid")
    # Sky radiance 0.8, not 1: the bake's linear-EXR check needs light values
    # away from 1, where a gamma error would be invisible.
    rho, sky = 0.5, 0.8
    author.material("Ground", (rho,) * 3)
    half = 20.0
    author.mesh("Floor", [quad((-half, -half, 0.0), (half, half, 0.0), 2, 1)], "Ground")
    author.dome_light("Sky", sky)
    elevation = 50.0
    sun_irradiance = 3.0
    travel = (-math.cos(math.radians(elevation)), 0.0, -math.sin(math.radians(elevation)))
    author.distant_light("Sun", travel, sun_irradiance)
    UsdGeom.Imageable(author.stage.GetPrimAtPath("/ProbeGrid/Lights/Sun")).CreateVisibilityAttr(
        UsdGeom.Tokens.invisible)
    pose = camera_pose((-3.0, 0.0, 1.7), (2.0, 0.0, 0.0))
    author.camera("Camera", pose)
    author.save()
    state_layer(directory / "states" / "sun.usda", directory / "probe-grid.usda",
                [("/ProbeGrid/Lights/Sky", "inputs:intensity", Sdf.ValueTypeNames.Float, 0.0),
                 ("/ProbeGrid/Lights/Sun", "visibility", None, UsdGeom.Tokens.inherited)])
    probes = [[0.0, 0.0, 1.0], [2.0, -2.0, 0.5], [-3.0, 4.0, 2.0]]
    sin_e = math.sin(math.radians(elevation))
    to_sun = [-c for c in travel]
    write_json(directory / "fixture.json", fixture_record(
        "probe-grid", "Analytic probe irradiance: uniform sky over an albedo-0.5 floor, and one "
        "sun; irradiance and visibility references at known probes", "probe-grid.usda",
        {"sky": {"layer": None, "note": "uniform dome radiance 0.8"},
         "sun": {"layer": "states/sun.usda", "note": "sun only, 50 degrees elevation"}},
        {"floor": pose}, {"floor": {"floor": ["Floor"]}}, "sky",
        {"analytic": {
            "probes_m": probes, "albedo": rho, "sky_radiance": sky,
            "sun": {"to_sun": to_sun, "irradiance": sun_irradiance,
                    "elevation_degrees": elevation},
            "floor_half_extent_m": half,
            "note": "Irradiance / pi (diffuse light units) on a probe normal n: the sky "
                    "(uniform over the whole sphere) where a ray misses the floor square, "
                    "and the floor's radiance, rho * sky (sky state) or rho * E_sun * "
                    "sin(elevation) / pi (sun state), where it hits; the sun state adds "
                    "the direct term E_sun * max(0, n . l) / pi. Floor-hit light is the "
                    "indirect layer. gi_probes.py integrates this over the finite square: "
                    "from 1-2 m up, the sky below the horizon past the floor's edge carries "
                    "about 6% of a horizontal normal's cosine weight, which an infinite-"
                    "floor formula leaves out.",
            "visibility_note": "probe mean hit distance downward = probe height; upward = "
                               "no hit (sky)"}}))
    write_json(directory / "map.json", map_manifest(
        "gi_probe_grid", "quality/fixtures/gi/probe-grid/probe-grid.usda",
        probe_bounds=(-5.0, -5.0, 0.25, 5.0, 5.0, 2.75)))


def portal_view(out):
    """A lit 8 x 5 x 3 m chamber with a red west wall. The portal pair sits on
    the south (y = 0) and north (y = 5) walls; `through` is the camera a
    player sees through the south portal, which is the north portal's view."""
    directory = out / "portal-view"
    author = Author(directory / "portal-view.usda", "PortalView")
    author.material("White", (0.7, 0.7, 0.7))
    author.material("Red", (0.7, 0.1, 0.1))
    author.material("ProbeGrey", (0.6, 0.6, 0.6))
    walls = author.room("C", (0.0, 0.0, 0.0), (8.0, 5.0, 3.0), "White", skip=("Xn",))
    author.mesh("C_Xn", [box_faces((0.0, 0.0, 0.0), (8.0, 5.0, 3.0), inward=True)[0][1:]],
                "Red")
    author.rect_light("Panel", (5.0, 2.5, 2.99), (1.5, 1.5), 10.0)
    author.probe_model("ProbeC", (2.0, 2.5, 1.2), "ProbeGrey")
    direct = camera_pose((6.5, 1.0, 1.6), (2.0, 2.5, 1.0))
    # The north portal's view: from its centre (4, 4.95, 1.4) into the room.
    through = camera_pose((4.0, 4.95, 1.4), (2.0, 2.5, 1.0))
    # The same view in game, through the portal pair: the portals face each
    # other on opposite walls, so looking into the south one is a translation
    # of +5 m in y. A camera 5 cm in front of it sees the `through` view from
    # 10 cm further back, and is judged against the `through` reference.
    portal = {"eye": [4.0, 0.05, 1.4], "forward": through["forward"], "up": through["up"]}
    author.camera("Camera", direct)
    author.save()
    regions = {"model": ["ProbeC"], "red_wall": ["C_Xn"], "floor": ["C_Zn"]}
    write_json(directory / "fixture.json", fixture_record(
        "portal-view", "The same indirect contribution seen directly and through a portal "
        "pair", "portal-view.usda", {"default": {"layer": None}},
        {"direct": direct, "through": through, "portal": portal},
        {"direct": regions, "through": regions, "portal": regions},
        "default", {"portals": {"south": {"center_m": [4.0, 0.0, 1.4], "normal": [0, 1, 0]},
                                "north": {"center_m": [4.0, 5.0, 1.4], "normal": [0, -1, 0]},
                                "size_m": [1.3, 2.3]},
                    "reference_cameras": {"portal": "through"}}))
    manifest = map_manifest("gi_portal_view", "quality/fixtures/gi/portal-view/portal-view.usda")
    # The linked pair as map-placed, activated prop_portal entities.
    manifest["collision"]["portals"] = [
        {"center_m": [4.0, 0.0, 1.4], "normal": [0, 1, 0], "portal_two": False},
        {"center_m": [4.0, 5.0, 1.4], "normal": [0, -1, 0], "portal_two": True}]
    write_json(directory / "map.json", manifest)


# `swing` (RFC 0011 G9): the bulb, its rope and the swing it is released into.
SWING_ANCHOR_M = (2.5, 2.5, 3.0)
SWING_LENGTH_M = 1.5
SWING_ANGLE_DEGREES = 40.0
# The bulb is a sphere of the light set's inverse-square source radius
# (light_set::kInverseSquareSourceRadius, 2 units) emitting radiance
# SWING_RADIANCE: its diffuse light at distance d is L (R / d)^2, which the
# engine's inverse-square light gives with color L (R / 100 units)^2.
SWING_BULB_RADIUS_M = 2.0 / SOURCE_UNITS_PER_METER
SWING_RADIANCE = 500.0


def swing_position(angle_degrees):
    """The bulb at `angle_degrees` from straight down, swinging in the xz plane."""
    angle = math.radians(angle_degrees)
    ax, ay, az = SWING_ANCHOR_M
    return (round(ax + SWING_LENGTH_M * math.sin(angle), 6), ay,
            round(az - SWING_LENGTH_M * math.cos(angle), 6))


def swing(out):
    """A 5 x 5 x 3 m room (red west wall, a pillar, a dim baked ceiling panel)
    with a bulb hanging on a 1.5 m rope from the ceiling's centre. The bulb
    is a dynamic light, absent from the bake (`baked` hides it); the states
    place it at rest and 40 degrees either side of it, where the in-game
    lamp is held for the moved-light oracle (G9)."""
    directory = out / "swing"
    author = Author(directory / "swing.usda", "Swing")
    author.material("White", (0.7, 0.7, 0.7))
    author.material("Red", (0.7, 0.1, 0.1))
    author.material("ProbeGrey", (0.6, 0.6, 0.6))
    author.material("Bulb", (0.0, 0.0, 0.0), (SWING_RADIANCE,) * 3)
    walls = author.room("S", (0.0, 0.0, 0.0), (5.0, 5.0, 3.0), "White", skip=("Xn",))
    author.mesh("S_Xn", [box_faces((0.0, 0.0, 0.0), (5.0, 5.0, 3.0), inward=True)[0][1:]],
                "Red")
    author.solid("Pillar", (3.7, 2.2, 0.0), (4.2, 2.8, 1.4), "White")
    author.rect_light("Panel", (4.3, 0.7, 2.99), (0.6, 0.6), 2.0)
    author.probe_model("ProbeS", (1.2, 3.9, 0.6), "ProbeGrey")
    bulb = author.root_path.AppendPath("World/Bulb")
    xform = UsdGeom.Xform.Define(author.stage, bulb)
    xform.AddTranslateOp().Set(Gf.Vec3d(*swing_position(0.0)))
    sphere_mesh(author.stage, bulb.AppendChild("BulbShape"), SWING_BULB_RADIUS_M, 24, 12)
    UsdShade.MaterialBindingAPI.Apply(author.stage.GetPrimAtPath(
        bulb.AppendChild("BulbShape"))).Bind(author.materials["Bulb"])
    pose = camera_pose((4.7, 0.3, 1.8), (1.5, 3.5, 0.8))
    author.camera("Camera", pose)
    author.save()
    base = directory / "swing.usda"
    states = {"rest": {"layer": None, "note": "bulb hanging straight down",
                       "bulb_m": list(swing_position(0.0))}}
    for name, angle in (("left", -SWING_ANGLE_DEGREES), ("right", SWING_ANGLE_DEGREES)):
        position = swing_position(angle)
        state_layer(directory / "states" / (name + ".usda"), base,
                    [(str(bulb), "xformOp:translate", Sdf.ValueTypeNames.Double3,
                      Gf.Vec3d(*position))])
        states[name] = {"layer": "states/%s.usda" % name,
                        "note": "bulb %g degrees from rest" % angle, "bulb_m": list(position)}
    state_layer(directory / "states" / "baked.usda", base,
                [(str(bulb), "visibility", None, UsdGeom.Tokens.invisible)])
    states["baked"] = {"layer": "states/baked.usda", "note": "bulb hidden: the map's bake"}
    write_json(directory / "fixture.json", fixture_record(
        "swing", "Moved-light oracle: a bulb on a rope lights the room directly, through its "
        "shadows and by its bounce, wherever it swings", "swing.usda", states,
        {"room": pose},
        {"room": {"floor": ["S_Zn"], "red_wall": ["S_Xn"], "back_wall": ["S_Yp"],
                  "pillar": ["Pillar"], "model": ["ProbeS"]}},
        "baked",
        {"swing": {"anchor_m": list(SWING_ANCHOR_M), "length_m": SWING_LENGTH_M,
                   "angle_degrees": SWING_ANGLE_DEGREES, "bulb_radius_m": SWING_BULB_RADIUS_M,
                   "radiance": SWING_RADIANCE,
                   "engine_color": SWING_RADIANCE * (2.0 / 100.0) ** 2}}))
    manifest = map_manifest("gi_swing", "quality/fixtures/gi/swing/states/baked.usda",
                            solid_meshes=["Pillar"])
    # The lamp: a physics prop on a rope constraint from the anchor, carrying
    # an inverse-square light_dynamic of the bulb's light, released from the
    # left extreme so it swings.
    manifest["collision"]["lamps"] = [{
        "name": "Lamp", "anchor_m": list(SWING_ANCHOR_M), "length_m": SWING_LENGTH_M,
        "release_degrees": -SWING_ANGLE_DEGREES,
        "color_linear": SWING_RADIANCE * (2.0 / 100.0) ** 2}]
    write_json(directory / "map.json", manifest)


FIXTURES = (furnace, thin_wall, room_states, door, probe_grid, portal_view, swing)


def generate(out):
    for build in FIXTURES:
        build(out)
    # The gallery (gi_gallery.py) builds on this module's authoring helpers.
    import gi_gallery
    gi_gallery.generate(out)
    index = {"schema": "gi-fixture-index/v1",
             "fixtures": sorted(p.parent.name for p in out.glob("*/fixture.json")),
             "generator": "tools/quality/gi_fixtures.py"}
    write_json(out / "index.json", index)


OTHER_WRITERS = {"prbv", "rtrn", "sdfv"}


def compare_trees(expected, actual):
    differences = []
    names = {p.relative_to(expected) for p in expected.rglob("*") if p.is_file()} | \
        {p.relative_to(actual) for p in actual.rglob("*") if p.is_file()}
    for name in sorted(names):
        # Rendered references (gi_reference.py) and the probe-volume fixtures
        # (probe_volume.py fixture) have their own writers and checks.
        if "references" in name.parts or name.parts[0] in OTHER_WRITERS:
            continue
        a, b = expected / name, actual / name
        if not a.is_file() or not b.is_file() or not filecmp.cmp(a, b, shallow=False):
            differences.append(str(name))
    return differences


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--out", type=Path, default=DEFAULT_OUT)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    if args.check:
        with tempfile.TemporaryDirectory() as temporary:
            generated = Path(temporary) / "gi"
            # room-states sublayers ../../usd-maps/room/room.usda.
            (Path(temporary) / "usd-maps").symlink_to(args.out.parent / "usd-maps")
            generate(generated)
            differences = compare_trees(args.out, generated)
        if differences:
            print("GI fixtures differ from the generator: " + ", ".join(differences))
            return 1
        print("GI fixtures match the generator")
        return 0
    generate(args.out)
    print("wrote GI fixtures to " + str(args.out))
    return 0


if __name__ == "__main__":
    sys.exit(main())
