#!/usr/bin/env python3
"""The RFC 0011 GI gallery: many small fixtures with relational oracles.

Each scene is an ordinary `gi-fixture/v1` fixture under quality/fixtures/gi/
(written by `gi_fixtures.py`, which calls `generate` here), so the Cycles
references (`gi_reference.py`), the map builds and the in-game captures
(`gi_runtime.py`) treat it like the hand-built fixtures. What the gallery
adds is breadth and declared `oracles` (`gi_oracles.py`): light transport
properties that hold whatever the renderer, so they judge Cycles references
now and the engine's producers later.

Families (the `family` field of each fixture):

  mix        colored lights blending: superposition (every state with several
             lights on is the sum of its parts), mirror symmetry with channel
             swaps, exact zeros in the channels a light does not emit
  furnace    closed furnaces with colored, extreme and cluttered interiors:
             L = Le / (1 - rho) per channel on every surface
  analytic   closed forms in open or large scenes: a tinted sky over a
             colored plane, emissive parallel plates, an integrating sphere
  bleed      color bleeding: Cornell boxes, colored floors, bounce filters
  leak       sealed colored rooms behind thin walls, slits and louvers
  occlusion  bounce-only corridors, colored shadows
  probe      dynamic models lit from different directions and heights
  symmetry   moved and swapped lights, mirrored suns
  range      intensity and albedo ladders, HDR pairs, color temperature
  emissive   emissive geometry as the only light
  complex    stairwells, colonnades, city blocks

Conventions: Z-up meters; every material Lambertian (ior 1); a scene's
first-listed state is the base stage (all lights on), and other states are
override layers switching lights (`inputs:intensity` 0), emission or light
transforms, so every state has identical geometry, which superposition
requires. Region names are mesh names; dynamic models (`Probe*`) are
receiver-only stand-ins, as in every GI fixture.
"""

import colorsys
import math
import random

from pxr import Gf, Sdf, UsdGeom, UsdLux, UsdShade

import gi_fixtures as gf
import gi_oracles

WHITE = (0.7, 0.7, 0.7)
GREY = (0.6, 0.6, 0.6)
RED = (1.0, 0.0, 0.0)
GREEN = (0.0, 1.0, 0.0)
BLUE = (0.0, 0.0, 1.0)
CYAN = (0.0, 1.0, 1.0)
MAGENTA = (1.0, 0.0, 1.0)
YELLOW = (1.0, 1.0, 0.0)
BOTH = ("total", "indirect")
ALL_LIGHTS = ("total", "indirect", "direct")
SWAP_RB = [2, 1, 0]
SWAP_RG = [1, 0, 2]
SWAP_GB = [0, 2, 1]


def camel(name):
    return "".join(part.capitalize() for part in name.replace("_", "-").split("-"))


def oriented(corners, normal):
    """Corners wound counter-clockwise seen from the front of `normal`."""
    a, b, c = (Gf.Vec3d(*corners[i]) for i in range(3))
    if Gf.Dot(Gf.Cross(b - a, c - a), Gf.Vec3d(*normal)) < 0:
        corners = list(reversed(corners))
    return list(corners), tuple(normal)


def grid_wall(plane, axis, facing, outer, holes):
    """Faces of a rectangle on plane `axis` = `plane` with rectangular holes,
    split on every hole edge; cells inside a hole are left out."""
    u, v = [i for i in range(3) if i != axis]
    (u0, v0), (u1, v1) = outer
    us = sorted({u0, u1} | {h[k][0] for h in holes for k in (0, 1)})
    vs = sorted({v0, v1} | {h[k][1] for h in holes for k in (0, 1)})
    faces = []
    for a0, a1 in zip(us, us[1:]):
        for b0, b1 in zip(vs, vs[1:]):
            cu, cv = (a0 + a1) / 2, (b0 + b1) / 2
            if any(h[0][0] < cu < h[1][0] and h[0][1] < cv < h[1][1] for h in holes):
                continue
            lo, hi = [0.0] * 3, [0.0] * 3
            lo[axis] = hi[axis] = plane
            lo[u], lo[v], hi[u], hi[v] = a0, b0, a1, b1
            faces.append(gf.quad(lo, hi, axis, facing))
    return faces


def sphere_faces(center, radius, inward=False, lon=48, lat=24, rows=None, columns=None):
    """Flat-shaded faces of a UV sphere (or a band of it: `rows`, `columns`
    as half-open index ranges), normals outward or inward."""
    def point(row, column):
        theta = math.pi * row / lat
        phi = 2 * math.pi * column / lon
        return (center[0] + radius * math.sin(theta) * math.cos(phi),
                center[1] + radius * math.sin(theta) * math.sin(phi),
                center[2] + radius * math.cos(theta))
    faces = []
    for row in range(*(rows or (0, lat))):
        for column in range(*(columns or (0, lon))):
            corners = [point(row, column), point(row + 1, column), point(row + 1, column + 1),
                       point(row, column + 1)]
            if row == 0:
                corners = [corners[0], corners[1], corners[2]]
            elif row == lat - 1:
                corners = [corners[0], corners[1], corners[3]]
            middle = [sum(c[i] for c in corners) / len(corners) - center[i] for i in range(3)]
            length = math.sqrt(sum(m * m for m in middle))
            normal = tuple((-m if inward else m) / length for m in middle)
            faces.append(oriented(corners, normal))
    return faces


def box_face_list(lo, hi, skip=()):
    return [(name, corners, normal)
            for name, corners, normal in gf.box_faces(lo, hi, inward=False) if name not in skip]


def hsv(hue_degrees, saturation=1.0, value=1.0):
    return tuple(round(c, 6) for c in colorsys.hsv_to_rgb((hue_degrees % 360) / 360.0,
                                                          saturation, value))


def blackbody(kelvin):
    rgb = UsdLux.BlackbodyTemperatureAsRgb(float(kelvin))
    return [round(float(c), 6) for c in rgb]


class Scene:
    """One gallery fixture: stage, state layers, cameras, regions, oracles."""

    def __init__(self, out, name, family, purpose):
        self.name, self.family, self.purpose = name, family, purpose
        self.directory = out / name
        self.author = gf.Author(self.directory / (name + ".usda"), "Gallery" + camel(name))
        self.root = str(self.author.root_path)
        self.meshes = set()
        self.models = set()
        self.states = {}
        self.cameras = {}
        self.regions = {}
        self.oracles = []
        self.solids = []
        self.probe_bounds = None
        self.extra = {}
        # The stage camera places the player spawn (pbrt_collision_vmf.py):
        # the first view unless a scene names a pose clear for the player hull.
        self.spawn = None
        # Lightmap layout: "planar" (lightmap_layout.py) where Blender's charts
        # break the seam invariants: curved meshes (texel density) and meshes of
        # coplanar boxes such as stairs (flat in-mesh seams).
        self.layout = None
        # Lightmap samples: 512 unless small, bright or sharp lights (spots,
        # a pinhole, a tiny HDR lamp, a sun) leave fireflies whose removal
        # moves the denoised mean more than lightmap_denoise.py's 5%.
        self.samples = 512

    # ------------------------------------------------------------ materials
    def mat(self, name, albedo, emission=(0.0, 0.0, 0.0)):
        if name not in self.author.materials:
            self.author.material(name, albedo, emission)
        return name

    def material_path(self, name):
        return "%s/Looks/%s/Surface" % (self.root, name)

    # ------------------------------------------------------------- geometry
    def mesh(self, name, faces, material):
        if name in self.meshes:
            raise ValueError("%s: mesh %s defined twice" % (self.name, name))
        self.meshes.add(name)
        self.author.mesh(name, faces, material)
        return name

    def room(self, prefix, lo, hi, material, skip=(), materials=None):
        """One inward mesh per face, <prefix>_<Xn|Xp|Yn|Yp|Zn|Zp>."""
        names = []
        for face, corners, normal in gf.box_faces(lo, hi, inward=True, skip=skip):
            names.append(self.mesh(prefix + "_" + face, [(corners, normal)],
                                   (materials or {}).get(face, material)))
        return names

    def box(self, name, lo, hi, material, skip=(), solid=True):
        self.mesh(name, [(c, n) for _, c, n in box_face_list(lo, hi, skip)], material)
        if solid:
            self.solids.append(name)
        return name

    def boxes(self, name, bounds, material, skip=(), solid=True):
        # Touching boxes (maze walls, stair treads) share faces in one mesh.
        if any(all(a[0][k] <= b[1][k] and b[0][k] <= a[1][k] for k in range(3))
               for i, a in enumerate(bounds) for b in bounds[i + 1:]):
            self.layout = "planar"
        faces = [(c, n) for lo, hi in bounds for _, c, n in box_face_list(lo, hi, skip)]
        self.mesh(name, faces, material)
        if solid:
            self.solids.append(name)
        return name

    def box_faces(self, prefix, lo, hi, material, skip=(), materials=None):
        """One outward mesh per face of a solid, <prefix>_<face>."""
        names = []
        for face, corners, normal in box_face_list(lo, hi, skip):
            names.append(self.mesh(prefix + "_" + face, [(corners, normal)],
                                   (materials or {}).get(face, material)))
        return names

    def panel(self, name, lo, hi, axis, facing, material):
        return self.mesh(name, [gf.quad(lo, hi, axis, facing)], material)

    def wall(self, name, plane, axis, facing, outer, holes, material):
        return self.mesh(name, grid_wall(plane, axis, facing, outer, holes), material)

    def doorway_x(self, name, x0, x1, y, z, material):
        """Jambs, lintel and sill of an opening through a wall spanning x0..x1
        (the opening's y and z ranges); the sill closes the floor gap."""
        (y0, y1), (z0, z1) = y, z
        faces = [gf.quad((x0, y0, z0), (x1, y0, z1), 1, 1),
                 gf.quad((x0, y1, z0), (x1, y1, z1), 1, -1),
                 gf.quad((x0, y0, z1), (x1, y1, z1), 2, -1),
                 gf.quad((x0, y0, z0), (x1, y1, z0), 2, 1)]
        return self.mesh(name, faces, material)

    def sphere(self, name, center, radius, material, inward=False, **band):
        self.layout = "planar"
        return self.mesh(name, sphere_faces(center, radius, inward, **band), material)

    def probe(self, name, center, material="ProbeGrey"):
        if name not in gf.DYNAMIC_MODELS:
            raise ValueError("%s: %s is not a declared dynamic model name" % (self.name, name))
        self.mat(material, GREY)
        self.author.probe_model(name, center, material)
        self.models.add(name)
        return name

    # --------------------------------------------------------------- lights
    def light_path(self, name):
        return "%s/Lights/%s" % (self.root, name)

    def place(self, light, center, direction=(0.0, 0.0, -1.0)):
        matrix = gf.look_rotation(direction)
        matrix.SetTranslateOnly(Gf.Vec3d(*center))
        light.AddTransformOp().Set(matrix)

    def finish_light(self, light, intensity, color, temperature=None):
        light.CreateIntensityAttr(float(intensity))
        light.CreateColorAttr(Gf.Vec3f(*color))
        light.CreateNormalizeAttr(False)
        if temperature:
            light.CreateEnableColorTemperatureAttr(True)
            light.CreateColorTemperatureAttr(float(temperature))
        return light

    def rect(self, name, center, size, intensity, color=(1.0, 1.0, 1.0),
             direction=(0.0, 0.0, -1.0), temperature=None):
        """A one-sided RectLight emitting along `direction`."""
        light = UsdLux.RectLight.Define(self.author.stage, self.light_path(name))
        light.CreateWidthAttr(float(size[0]))
        light.CreateHeightAttr(float(size[1]))
        self.place(light, center, direction)
        return self.finish_light(light, intensity, color, temperature)

    def point(self, name, center, radius, intensity, color=(1.0, 1.0, 1.0)):
        light = UsdLux.SphereLight.Define(self.author.stage, self.light_path(name))
        light.CreateRadiusAttr(float(radius))
        self.place(light, center)
        return self.finish_light(light, intensity, color)

    def spot(self, name, center, radius, intensity, color, direction, inner, outer,
             exponent=1.0):
        """A DiskLight with a Source cone (inner/outer half-angles in degrees)."""
        light = UsdLux.DiskLight.Define(self.author.stage, self.light_path(name))
        light.CreateRadiusAttr(float(radius))
        self.place(light, center, direction)
        prim = light.GetPrim()
        for attribute, value in (("coneInner", math.cos(math.radians(inner))),
                                 ("coneOuter", math.cos(math.radians(outer))),
                                 ("coneExponent", exponent)):
            prim.CreateAttribute("sourceEngine:" + attribute, Sdf.ValueTypeNames.Float,
                                 custom=True).Set(round(float(value), 6))
        return self.finish_light(light, intensity, color)

    def sun(self, name, azimuth, elevation, irradiance, color):
        """A DistantLight shining from `azimuth` (degrees from +X toward +Y)
        and `elevation` above the horizon."""
        a, e = math.radians(azimuth), math.radians(elevation)
        travel = (-math.cos(e) * math.cos(a), -math.cos(e) * math.sin(a), -math.sin(e))
        light = self.author.distant_light(name, travel, 1.0)
        light.CreateIntensityAttr(float(irradiance))
        light.CreateColorAttr(Gf.Vec3f(*color))
        return light

    def sky(self, name, radiance, color):
        light = self.author.dome_light(name, radiance)
        light.CreateColorAttr(Gf.Vec3f(*color))
        return light

    # --------------------------------------------------------------- states
    def base(self, state, note):
        self.states[state] = {"layer": None, "note": note, "edits": []}

    def state(self, state, note, edits):
        self.states[state] = {"layer": "states/%s.usda" % state, "note": note, "edits": edits}

    def off(self, *lights):
        return [(self.light_path(n), "inputs:intensity", Sdf.ValueTypeNames.Float, 0.0)
                for n in lights]

    def intensity(self, light, value):
        return [(self.light_path(light), "inputs:intensity", Sdf.ValueTypeNames.Float,
                 float(value))]

    def recolor(self, light, color):
        return [(self.light_path(light), "inputs:color", Sdf.ValueTypeNames.Color3f,
                 Gf.Vec3f(*color))]

    def dark(self, *materials):
        return [(self.material_path(m), "inputs:emissiveColor", Sdf.ValueTypeNames.Color3f,
                 Gf.Vec3f(0.0, 0.0, 0.0)) for m in materials]

    def moved(self, light, center, direction=(0.0, 0.0, -1.0)):
        matrix = gf.look_rotation(direction)
        matrix.SetTranslateOnly(Gf.Vec3d(*center))
        return [(self.light_path(light), "xformOp:transform", Sdf.ValueTypeNames.Matrix4d,
                 matrix)]

    def solo(self, lights, keep):
        """Edits switching off every light in `lights` except `keep`."""
        return self.off(*[n for n in lights if n not in keep])

    # -------------------------------------------------------------- cameras
    def view(self, name, eye, target, regions):
        self.cameras[name] = gf.camera_pose(eye, target)
        self.regions[name] = regions if isinstance(regions, dict) else {r: [r] for r in regions}

    # -------------------------------------------------------------- oracles
    def oracle(self, **fields):
        self.oracles.append(fields)

    def superpose(self, camera, total, parts, regions=None, lights=BOTH):
        self.oracle(kind="superposition", name="%s = %s (%s)" % (total, " + ".join(parts),
                                                                  camera),
                    camera=camera, sum=total, parts=list(parts),
                    regions=list(regions or self.regions[camera]), lights=list(lights))

    def superpose_all(self, total, parts, lights=BOTH):
        for camera in self.cameras:
            self.superpose(camera, total, parts, lights=lights)

    def mirror(self, a, b, perm=(0, 1, 2), lights=BOTH, tolerance=None, name=None,
               channels=(0, 1, 2)):
        """`a` and `b` are (state, camera, region); b's channels are `perm`
        of a's (restricted to `channels`)."""
        fields = {"kind": "equal", "lights": list(lights),
                  "name": name or "mirror %s/%s = %s/%s" % (a[0], a[2], b[0], b[2]),
                  "a": {"state": a[0], "camera": a[1], "region": a[2],
                        "channels": list(channels)},
                  "b": {"state": b[0], "camera": b[1], "region": b[2],
                        "channels": [perm[c] for c in channels]}}
        if tolerance:
            fields["tolerance"] = tolerance
        self.oracles.append(fields)

    def zero(self, state, camera, channels, regions=None, lights=ALL_LIGHTS, name=None):
        self.oracle(kind="zero", name=name or "%s: %s dark (%s)" % (
            state, "".join(gi_oracles.CHANNELS[c] for c in channels), camera),
            state=state, camera=camera, regions=list(regions or self.regions[camera]),
            channels=list(channels), lights=list(lights))

    def zero_all(self, state, channels, lights=ALL_LIGHTS):
        for camera in self.cameras:
            self.zero(state, camera, channels, lights=lights)

    def value(self, state, camera, regions, expected, name, tolerance=None):
        fields = {"kind": "value", "name": name, "state": state, "camera": camera,
                  "regions": list(regions), "lights": sorted(expected),
                  "expected": {k: [round(float(c), 9) for c in v] for k, v in expected.items()}}
        if tolerance:
            fields["tolerance"] = tolerance
        self.oracles.append(fields)

    def chroma(self, state, camera, expected, regions=None, lights=BOTH, name=None):
        self.oracle(kind="chromaticity", name=name or "%s chromaticity (%s)" % (state, camera),
                    state=state, camera=camera, regions=list(regions or self.regions[camera]),
                    expected=[round(float(c), 6) for c in expected], lights=list(lights))

    def chroma_all(self, state, expected, lights=BOTH):
        for camera in self.cameras:
            self.chroma(state, camera, expected, lights=lights)

    def dominant(self, state, camera, region, channel, over, factor, lights=BOTH, name=None):
        self.oracle(kind="dominant", name=name or "%s/%s %s dominant (%s)" % (
            state, region, gi_oracles.CHANNELS[channel], camera),
            state=state, camera=camera, region=region, channel=channel, over=list(over),
            factor=factor, lights=list(lights))

    def tint_order(self, entries, channel, over, factor, lights=BOTH, name=None):
        self.oracle(kind="tint_order", name=name or "%s/%s falls along %s" % (
            gi_oracles.CHANNELS[channel], gi_oracles.CHANNELS[over],
            ", ".join(e[2] for e in entries)),
            entries=[{"state": s, "camera": c, "region": r} for s, c, r in entries],
            channel=channel, over=over, factor=factor, lights=list(lights))

    def ordered(self, entries, factor, lights=BOTH, name=None):
        self.oracle(kind="ordered", name=name or "luminance rises along %s" % ", ".join(
            e[2] for e in entries),
            entries=[{"state": s, "camera": c, "region": r} for s, c, r in entries],
            factor=factor, lights=list(lights))

    def scaled(self, camera, base, scaled, factor, lights=BOTH):
        self.oracle(kind="scale", name="%s = %g x %s (%s)" % (scaled, factor, base, camera),
                    camera=camera, base=base, scaled=scaled, factor=factor,
                    regions=list(self.regions[camera]), lights=list(lights))

    def uniform(self, entries, lights=("indirect",), tolerance=None, name=None):
        fields = {"kind": "uniform", "name": name or "uniform over %d regions" % len(entries),
                  "entries": [{"state": s, "camera": c, "region": r} for s, c, r in entries],
                  "lights": list(lights)}
        if tolerance:
            fields["tolerance"] = tolerance
        self.oracles.append(fields)

    # --------------------------------------------------------------- output
    def finish(self, baked_state=None):
        if not self.cameras:
            raise ValueError(self.name + ": no cameras")
        base_states = [s for s, v in self.states.items() if v["layer"] is None]
        if len(base_states) != 1:
            raise ValueError(self.name + ": exactly one base state is required")
        for camera, regions in self.regions.items():
            for region, entries in regions.items():
                unknown = [e for e in entries if e not in self.meshes and e not in self.models]
                if unknown:
                    raise ValueError("%s: camera %s region %s names unknown %s" % (
                        self.name, camera, region, unknown))
        first = gf.camera_pose(*self.spawn) if self.spawn else next(iter(self.cameras.values()))
        self.author.camera("Camera", first)
        self.author.save()
        stage = self.directory / (self.name + ".usda")
        states = {}
        for state, entry in self.states.items():
            if entry["layer"]:
                gf.state_layer(self.directory / entry["layer"], stage, entry["edits"])
            states[state] = {"layer": entry["layer"], "note": entry["note"]}
        baked = baked_state or base_states[0]
        extra = {"family": self.family, "oracles": self.oracles}
        extra.update(self.extra)
        record = gf.fixture_record(self.name, self.purpose, stage.name, states, self.cameras,
                                   self.regions, baked, extra)
        problems = gi_oracles.validate(record)
        if problems:
            raise ValueError("%s: %s" % (self.name, "; ".join(problems)))
        gf.write_json(self.directory / "fixture.json", record)
        layer = states[baked]["layer"]
        scene = "quality/fixtures/gi/%s/%s" % (self.name, layer or stage.name)
        manifest = gf.map_manifest("gi_" + self.name.replace("-", "_"), scene,
                                   solid_meshes=self.solids, probe_bounds=self.probe_bounds)
        # Gallery maps bake cheaper than the gi-fixture profile, with 512
        # denoised lightmap samples and 1024 probe samples: dozens of maps at
        # 2048 samples on a shared host take a day. Their oracles run on the
        # 2048-sample references; denoised bakes are statistical
        # (cycles_device.determinism), which the runtime's 10% tolerances
        # allow. (The shared integrated GPU was slower still.)
        manifest["lightmap"] = {"samples": self.samples}
        if self.layout:
            manifest["lightmap"]["layout"] = self.layout
        manifest.setdefault("probe_volume", {})["samples"] = 1024
        gf.write_json(self.directory / "map.json", manifest)


# ======================================================================
# mix: colored lights blending
# ======================================================================

def symmetric_room(scene, material="White", size=(6.0, 6.0, 3.0)):
    """A room mirror-symmetric about x = size[0] / 2."""
    scene.mat(material, WHITE)
    return scene.room("R", (0.0, 0.0, 0.0), size, material)


def centre_view(scene, name="centre", size=(6.0, 6.0, 3.0), extra=(), eye_z=1.6):
    """A camera on the symmetry plane x = size[0] / 2 looking along +Y."""
    mid = size[0] / 2
    scene.view(name, (mid, 0.2, eye_z), (mid, size[1], 0.9),
               ["R_Xn", "R_Xp", "R_Yp", "R_Zn", "R_Zp"] + list(extra))


def self_mirrors(scene, state, camera, perm, regions, lights=BOTH):
    """Regions the mirror maps onto themselves: their channels equal their
    own permuted channels (only the channels the permutation moves)."""
    moved = [c for c in range(3) if perm[c] != c]
    for region in regions:
        scene.mirror((state, camera, region), (state, camera, region), perm, lights,
                     channels=moved[:1],
                     name="self-mirror %s/%s %s = %s" % (
                         state, region, gi_oracles.CHANNELS[moved[0]],
                         gi_oracles.CHANNELS[perm[moved[0]]]))


def mix_rgb_ceiling(out):
    s = Scene(out, "mix-rgb-ceiling", "mix", "Red, green and blue ceiling panels in a white "
              "room: every mix is the sum of its parts, a pure light stays pure through "
              "grey bounces, and the red/blue mirror symmetry swaps channels")
    symmetric_room(s)
    lights = {"Red": ((1.5, 3.5, 2.99), RED), "Green": ((3.0, 3.5, 2.99), GREEN),
              "Blue": ((4.5, 3.5, 2.99), BLUE)}
    for name, (center, color) in lights.items():
        s.rect(name, center, (0.8, 0.8), 12.0, color)
    s.probe("ProbeSphere", (3.0, 3.8, 1.0))
    centre_view(s, extra=["ProbeSphere"])
    s.base("all", "red, green and blue panels on")
    for name in lights:
        s.state(name.lower(), name.lower() + " panel alone", s.solo(lights, [name]))
    s.superpose_all("all", ["red", "green", "blue"])
    for name, zero in (("red", [1, 2]), ("green", [0, 2]), ("blue", [0, 1])):
        s.zero_all(name, zero)
    s.mirror(("all", "centre", "R_Xn"), ("all", "centre", "R_Xp"), SWAP_RB)
    self_mirrors(s, "all", "centre", SWAP_RB, ["R_Yp", "R_Zn", "R_Zp", "ProbeSphere"])
    s.mirror(("red", "centre", "R_Xn"), ("blue", "centre", "R_Xp"), SWAP_RB, channels=[0])
    s.mirror(("green", "centre", "R_Xn"), ("green", "centre", "R_Xp"), channels=[1])
    s.tint_order([("all", "centre", "R_Xn"), ("all", "centre", "R_Zn"),
                  ("all", "centre", "R_Xp")], 0, 2, 1.1)
    s.finish()


def mix_cmy_ceiling(out):
    s = Scene(out, "mix-cmy-ceiling", "mix", "Cyan, magenta and yellow panels: subtractive "
              "primaries add; each two-channel light keeps its exact chromaticity on grey "
              "walls and never emits its missing channel")
    symmetric_room(s)
    lights = {"Cyan": ((1.5, 3.5, 2.99), CYAN), "Magenta": ((3.0, 3.5, 2.99), MAGENTA),
              "Yellow": ((4.5, 3.5, 2.99), YELLOW)}
    for name, (center, color) in lights.items():
        s.rect(name, center, (0.8, 0.8), 8.0, color)
    s.probe("ProbeSphere", (3.0, 3.8, 1.0))
    centre_view(s, extra=["ProbeSphere"])
    s.base("all", "cyan, magenta and yellow panels on")
    for name, (_, color) in lights.items():
        s.state(name.lower(), name.lower() + " panel alone", s.solo(lights, [name]))
    s.superpose_all("all", ["cyan", "magenta", "yellow"])
    for name, color, missing in (("cyan", CYAN, 0), ("magenta", MAGENTA, 1),
                                 ("yellow", YELLOW, 2)):
        s.zero_all(name, [missing])
        s.chroma_all(name, color)
    s.mirror(("all", "centre", "R_Xn"), ("all", "centre", "R_Xp"), SWAP_RB)
    self_mirrors(s, "all", "centre", SWAP_RB, ["R_Yp", "R_Zn", "ProbeSphere"])
    s.mirror(("cyan", "centre", "R_Xn"), ("yellow", "centre", "R_Xp"), SWAP_RB)
    s.finish()


def mix_complementary_corridor(out):
    s = Scene(out, "mix-complementary-corridor", "mix", "A 12 m corridor with a red panel "
              "at one end and a cyan panel at the other: the floor fades red to cyan "
              "segment by segment, and the mirrored ends match channel for channel")
    s.mat("White", WHITE)
    s.room("C", (0.0, 0.0, 0.0), (12.0, 2.5, 3.0), "White", skip=("Zn",))
    floors = []
    for i in range(6):
        floors.append(s.panel("F%d" % i, (2.0 * i, 0.0, 0.0), (2.0 * i + 2.0, 2.5, 0.0), 2, 1,
                              "White"))
    s.rect("Red", (1.5, 1.25, 2.99), (1.0, 1.0), 10.0, RED)
    s.rect("Cyan", (10.5, 1.25, 2.99), (1.0, 1.0), 10.0, CYAN)
    common = ["C_Yn", "C_Yp", "C_Zp"]
    s.view("west", (7.0, 1.25, 1.8), (0.0, 1.25, 0.5), ["F0", "F1", "F2", "C_Xn"] + common)
    s.view("east", (5.0, 1.25, 1.8), (12.0, 1.25, 0.5), ["F5", "F4", "F3", "C_Xp"] + common)
    s.base("all", "both panels on")
    s.state("red", "red panel alone", s.off("Cyan"))
    s.state("cyan", "cyan panel alone", s.off("Red"))
    s.superpose_all("all", ["red", "cyan"])
    s.zero_all("red", [1, 2])
    s.zero_all("cyan", [0])
    s.chroma_all("cyan", CYAN)
    for west, east in (("F0", "F5"), ("F1", "F4"), ("F2", "F3"), ("C_Xn", "C_Xp"),
                       ("C_Yn", "C_Yn"), ("C_Zp", "C_Zp")):
        for channel in (1, 2):
            s.mirror(("red", "west", west), ("cyan", "east", east), [channel, 1, 2],
                     channels=[0], name="mirror red/%s R = cyan/%s %s" % (
                         west, east, gi_oracles.CHANNELS[channel]))
    s.tint_order([("all", "west", "F0"), ("all", "west", "F1"), ("all", "west", "F2"),
                  ("all", "east", "F3"), ("all", "east", "F4"), ("all", "east", "F5")],
                 0, 1, 1.05)
    s.finish()


def mix_rgb_venn(out):
    s = Scene(out, "mix-rgb-venn", "mix", "Three cone spots (red, green, blue) overlap on the "
              "floor as an additive Venn diagram; mirror symmetry swaps green and blue")
    s.samples = 2048
    size = (6.0, 6.0, 4.0)
    symmetric_room(s, size=size)
    for name, angle, color in (("Red", 90, RED), ("Green", 210, GREEN), ("Blue", 330, BLUE)):
        center = (3.0 + 0.6 * math.cos(math.radians(angle)),
                  3.0 + 0.6 * math.sin(math.radians(angle)), 3.95)
        s.spot(name, [round(c, 6) for c in center], 0.1, 900.0, color, (0.0, 0.0, -1.0), 12.0,
               18.0)
    s.view("centre", (3.0, 0.2, 3.0), (3.0, 3.5, 0.0), ["R_Xn", "R_Xp", "R_Yp", "R_Zn"])
    s.base("all", "three spots on")
    lights = ("Red", "Green", "Blue")
    for name in lights:
        s.state(name.lower(), name.lower() + " spot alone", s.solo(lights, [name]))
    s.superpose_all("all", ["red", "green", "blue"])
    for name, zero in (("red", [1, 2]), ("green", [0, 2]), ("blue", [0, 1])):
        s.zero_all(name, zero)
    s.mirror(("all", "centre", "R_Xn"), ("all", "centre", "R_Xp"), SWAP_GB)
    self_mirrors(s, "all", "centre", SWAP_GB, ["R_Zn", "R_Yp"])
    s.mirror(("green", "centre", "R_Xn"), ("blue", "centre", "R_Xp"), SWAP_GB, channels=[1])
    s.finish()


def mix_spot_cross(out):
    s = Scene(out, "mix-spot-cross", "mix", "A red spot from the left and a blue spot from "
              "the right cross on the back wall: overlap adds, mirror swaps red and blue")
    s.samples = 2048
    symmetric_room(s)
    s.spot("Red", (0.3, 1.0, 2.6), 0.08, 1500.0, RED, (0.55, 1.0, -0.25), 10.0, 16.0)
    s.spot("Blue", (5.7, 1.0, 2.6), 0.08, 1500.0, BLUE, (-0.55, 1.0, -0.25), 10.0, 16.0)
    centre_view(s)
    s.base("both", "both spots on")
    s.state("red", "red spot alone", s.off("Blue"))
    s.state("blue", "blue spot alone", s.off("Red"))
    s.superpose_all("both", ["red", "blue"])
    s.zero_all("both", [1])
    s.zero_all("red", [1, 2])
    s.zero_all("blue", [0, 1])
    s.mirror(("both", "centre", "R_Xn"), ("both", "centre", "R_Xp"), SWAP_RB)
    self_mirrors(s, "both", "centre", SWAP_RB, ["R_Yp", "R_Zn", "R_Zp"])
    s.mirror(("red", "centre", "R_Yp"), ("blue", "centre", "R_Yp"), SWAP_RB, channels=[0])
    s.finish()


def mix_sun_sky_pillar(out):
    s = Scene(out, "mix-sun-sky-pillar", "mix", "An orange sun and a blue sky over a grey "
              "ground and pillar: sun and sky add, each keeps its color on the grey world, "
              "the sun's shadow is blue and the lit face orange")
    s.samples = 2048
    s.mat("Ground", (0.5, 0.5, 0.5))
    s.mat("Stone", (0.6, 0.6, 0.6))
    half = 20.0
    shadow = ((-3.2, -0.45), (-0.5, 0.45))
    s.wall("Ground", 0.0, 2, 1, ((-half, -half), (half, half)), [shadow], "Ground")
    s.panel("GroundShadow", (shadow[0][0], shadow[0][1], 0.0), (shadow[1][0], shadow[1][1], 0.0),
            2, 1, "Ground")
    s.box_faces("Pillar", (-0.5, -0.5, 0.0), (0.5, 0.5, 2.0), "Stone", skip=("Zn",))
    sun, sky = (1.0, 0.75, 0.45), (0.35, 0.55, 1.0)
    s.sun("Sun", 0.0, 35.0, 3.0, sun)
    s.sky("Sky", 0.5, sky)
    s.view("shade", (-3.0, -4.0, 2.0), (0.0, 0.0, 0.8),
           ["Ground", "GroundShadow", "Pillar_Xn", "Pillar_Yn"])
    s.view("lit", (3.0, -4.0, 2.0), (0.0, 0.0, 0.8), ["Ground", "Pillar_Xp", "Pillar_Yn"])
    s.base("both", "sun and sky")
    s.state("sun", "sun alone", s.off("Sky"))
    s.state("sky", "sky alone", s.off("Sun"))
    s.superpose_all("both", ["sun", "sky"])
    s.chroma_all("sun", sun)
    s.chroma_all("sky", sky)
    s.zero("sun", "shade", [0, 1, 2], ["GroundShadow"], ["direct"],
           name="sun: the pillar's umbra has no direct light")
    s.dominant("both", "shade", "GroundShadow", 2, [0], 1.5)
    s.dominant("both", "shade", "Pillar_Xn", 2, [0], 1.1)
    s.dominant("both", "lit", "Pillar_Xp", 0, [2], 1.2, lights=["total"])
    s.probe_bounds = (-5.0, -5.0, 0.25, 5.0, 5.0, 3.0)
    s.finish()


def mix_disco(out):
    s = Scene(out, "mix-disco", "mix", "Twenty-four small lights of seeded random hue and "
              "position among four pillars: three random subsets add to the whole, and with "
              "every light off the room is exactly black")
    s.mat("White", WHITE)
    s.room("R", (0.0, 0.0, 0.0), (8.0, 8.0, 4.0), "White")
    pillars = [((x - 0.3, y - 0.3, 0.0), (x + 0.3, y + 0.3, 4.0))
               for x, y in ((2.0, 2.0), (6.0, 2.0), (2.0, 6.0), (6.0, 6.0))]
    s.boxes("Pillars", pillars, "White", skip=("Zn", "Zp"))
    s.probe("ProbeSphere", (4.0, 4.0, 1.2))
    rng = random.Random(20260925)
    names = []
    while len(names) < 24:
        p = (round(rng.uniform(0.6, 7.4), 3), round(rng.uniform(0.6, 7.4), 3),
             round(rng.uniform(0.5, 3.5), 3))
        if any(abs(p[0] - x) < 0.6 and abs(p[1] - y) < 0.6 for x, y in
               ((2, 2), (6, 2), (2, 6), (6, 6))):
            continue
        if math.dist(p, (4.0, 4.0, 1.2)) < 0.9:
            continue
        name = "Light%02d" % len(names)
        s.point(name, p, 0.06, 300.0, hsv(rng.uniform(0.0, 360.0), rng.uniform(0.6, 1.0)))
        names.append(name)
    regions = ["R_Zn", "R_Zp", "Pillars", "ProbeSphere"]
    # Wall-centre cameras: a corner camera's diagonal runs behind a pillar.
    s.view("south", (4.0, 0.4, 1.8), (4.0, 8.0, 0.8), regions + ["R_Yp", "R_Xn", "R_Xp"])
    s.view("north", (4.0, 7.6, 1.8), (4.0, 0.0, 0.8), regions + ["R_Yn", "R_Xn", "R_Xp"])
    s.base("all", "all 24 lights on")
    subsets = {"set-a": names[0::3], "set-b": names[1::3], "set-c": names[2::3]}
    for state, keep in subsets.items():
        s.state(state, "lights %s on" % ", ".join(keep), s.solo(names, keep))
    s.state("off", "every light off", s.off(*names))
    s.superpose_all("all", list(subsets))
    s.zero_all("off", [0, 1, 2])
    s.finish()


def mix_hue_ring(out):
    s = Scene(out, "mix-hue-ring", "mix", "Twelve lights round a pedestal at every 30 degrees "
              "of hue; four triads (RGB, their halves, CMY, the rest) add to the full ring")
    s.mat("White", WHITE)
    s.room("R", (0.0, 0.0, 0.0), (8.0, 8.0, 3.5), "White")
    s.box("Pedestal", (3.6, 3.6, 0.0), (4.4, 4.4, 0.8), "White", skip=("Zn",))
    s.probe("ProbeSphere", (4.0, 4.0, 1.35))
    names = []
    for k in range(12):
        angle = math.radians(30 * k)
        name = "Hue%03d" % (30 * k)
        s.point(name, (round(4.0 + 2.5 * math.cos(angle), 6), round(4.0 + 2.5 * math.sin(angle),
                                                                    6), 2.2),
                0.08, 180.0, hsv(30 * k))
        names.append(name)
    s.view("corner", (0.4, 0.4, 1.7), (4.0, 4.0, 1.0),
           ["R_Zn", "R_Xp", "R_Yp", "Pedestal", "ProbeSphere"])
    s.view("above", (7.5, 4.0, 3.3), (4.0, 4.0, 0.6), ["R_Zn", "R_Xn", "Pedestal", "ProbeSphere"])
    s.base("all", "the full ring on")
    triads = {}
    for t in range(4):
        keep = names[t::4]
        triads["triad-%d" % t] = keep
        s.state("triad-%d" % t, "hues %s on" % ", ".join(k[3:] for k in keep),
                s.solo(names, keep))
    s.superpose_all("all", list(triads))
    s.finish()


def mix_color_cube(out):
    s = Scene(out, "mix-color-cube", "mix", "Every wall of a closed cube glows a different "
              "color (red, cyan, green, magenta, blue, yellow): warm and cool halves add")
    colors = {"Xn": RED, "Xp": CYAN, "Yn": GREEN, "Yp": MAGENTA, "Zn": BLUE, "Zp": YELLOW}
    materials = {face: s.mat("Glow" + face, (0.5, 0.5, 0.5), tuple(0.25 * c for c in color))
                 for face, color in colors.items()}
    s.room("Cube", (0.0, 0.0, 0.0), (4.0, 4.0, 4.0), "GlowXn", materials=materials)
    s.probe("ProbeSphere", (2.0, 2.0, 2.0))
    s.view("low", (0.3, 0.3, 0.6), (4.0, 4.0, 3.2), ["Cube_Xp", "Cube_Yp", "Cube_Zp",
                                                     "ProbeSphere"])
    s.view("high", (3.7, 3.7, 3.4), (0.0, 0.0, 0.8), ["Cube_Xn", "Cube_Yn", "Cube_Zn",
                                                      "ProbeSphere"])
    s.base("all", "every wall glowing")
    warm, cool = ["GlowXn", "GlowYp", "GlowZp"], ["GlowXp", "GlowYn", "GlowZn"]
    s.state("warm", "red, magenta and yellow walls glowing", s.dark(*cool))
    s.state("cool", "cyan, green and blue walls glowing", s.dark(*warm))
    s.superpose_all("all", ["warm", "cool"], lights=ALL_LIGHTS)
    s.finish()


def mix_neon_strips(out):
    s = Scene(out, "mix-neon-strips", "mix", "Six neon strips (red to violet) on one wall of "
              "a grey room: warm and cool strips add, and the warm side reads warmer")
    s.mat("Grey", (0.5, 0.5, 0.5))
    s.room("R", (0.0, 0.0, 0.0), (8.0, 8.0, 3.0), "Grey")
    colors = [("Red", RED), ("Orange", (1.0, 0.5, 0.0)), ("Yellow", YELLOW),
              ("Green", GREEN), ("Blue", (0.0, 0.3, 1.0)), ("Violet", (0.6, 0.0, 1.0))]
    neon = []
    for i, (name, color) in enumerate(colors):
        material = s.mat("Neon" + name, (0.0, 0.0, 0.0), tuple(6.0 * c for c in color))
        x0 = 0.6 + 1.2 * i
        s.box("Neon" + name, (x0, 7.9, 2.3), (x0 + 1.0, 7.98, 2.4), material, solid=False)
        neon.append(material)
    s.view("centre", (4.0, 0.3, 1.6), (4.0, 8.0, 1.2),
           ["R_Xn", "R_Xp", "R_Yp", "R_Zn", "R_Zp"])
    s.base("all", "all six strips on")
    s.state("warm", "red, orange and yellow strips on", s.dark(*neon[3:]))
    s.state("cool", "green, blue and violet strips on", s.dark(*neon[:3]))
    s.superpose_all("all", ["warm", "cool"])
    s.tint_order([("all", "centre", "R_Xn"), ("all", "centre", "R_Xp")], 0, 2, 1.2)
    s.finish()


def mix_rgb_probes(out):
    s = Scene(out, "mix-rgb-probes", "mix", "Red, green and blue point lights with a dynamic "
              "model beside each and one between: each model takes its neighbour's color, "
              "and the red/green mirror swaps the models' channels")
    size = (6.0, 8.0, 3.0)
    s.mat("White", WHITE)
    s.room("R", (0.0, 0.0, 0.0), size, "White")
    # Each lamp stands on the camera's side of its model, which the camera sees.
    lights = {"Red": ((1.0, 2.0, 2.0), RED), "Green": ((5.0, 2.0, 2.0), GREEN),
              "Blue": ((3.0, 5.3, 2.4), BLUE)}
    for name, (center, color) in lights.items():
        s.point(name, center, 0.08, 250.0, color)
    s.probe("ProbeA", (1.5, 2.8, 1.0))
    s.probe("ProbeB", (4.5, 2.8, 1.0))
    # ProbeD sits low and ProbeC high so the centred camera sees both.
    s.probe("ProbeC", (3.0, 6.3, 1.7))
    s.probe("ProbeD", (3.0, 4.2, 0.6))
    s.view("centre", (3.0, 0.2, 2.0), (3.0, 5.0, 0.8),
           ["R_Zn", "R_Yp", "R_Xn", "R_Xp", "ProbeA", "ProbeB", "ProbeC", "ProbeD"])
    s.base("all", "all three lights on")
    for name in lights:
        s.state(name.lower(), name.lower() + " light alone", s.solo(lights, [name]))
    s.superpose_all("all", ["red", "green", "blue"])
    s.dominant("all", "centre", "ProbeA", 0, [1, 2], 1.3)
    s.dominant("all", "centre", "ProbeB", 1, [0, 2], 1.3)
    s.dominant("all", "centre", "ProbeC", 2, [0, 1], 1.3)
    s.mirror(("all", "centre", "ProbeA"), ("all", "centre", "ProbeB"), SWAP_RG)
    s.mirror(("all", "centre", "R_Xn"), ("all", "centre", "R_Xp"), SWAP_RG)
    self_mirrors(s, "all", "centre", SWAP_RG, ["ProbeC", "ProbeD", "R_Zn", "R_Yp"])
    s.finish()


def mix_light_swap(out):
    s = Scene(out, "mix-light-swap", "symmetry", "An orange and a teal panel trade places: the "
              "swapped state is the mirror image, and orange/teal are each other's red/blue "
              "swap, so one state is also its own mirror")
    symmetric_room(s)
    orange, teal = (1.0, 0.5, 0.1), (0.1, 0.5, 1.0)
    s.rect("Left", (1.5, 3.5, 2.99), (0.8, 0.8), 12.0, orange)
    s.rect("Right", (4.5, 3.5, 2.99), (0.8, 0.8), 12.0, teal)
    s.probe("ProbeSphere", (3.0, 3.8, 1.0))
    centre_view(s, extra=["ProbeSphere"])
    s.base("orange-teal", "orange left, teal right")
    s.state("teal-orange", "teal left, orange right",
            s.recolor("Left", teal) + s.recolor("Right", orange))
    s.state("orange-left", "orange left alone", s.off("Right"))
    s.state("teal-right", "teal right alone", s.off("Left"))
    s.superpose_all("orange-teal", ["orange-left", "teal-right"])
    for a, b in (("R_Xn", "R_Xp"), ("R_Xp", "R_Xn"), ("R_Zn", "R_Zn"), ("R_Yp", "R_Yp"),
                 ("ProbeSphere", "ProbeSphere")):
        s.mirror(("orange-teal", "centre", a), ("teal-orange", "centre", b))
    s.mirror(("orange-teal", "centre", "R_Xn"), ("orange-teal", "centre", "R_Xp"), SWAP_RB)
    s.chroma_all("orange-left", orange)
    s.chroma_all("teal-right", teal)
    s.finish()


def mix_moving_light(out):
    s = Scene(out, "mix-moving-light", "symmetry", "One pink point light at five positions "
              "across a symmetric room: mirrored positions give mirrored light, the centre is "
              "its own mirror, and the pink stays pink on grey")
    s.mat("Grey", GREY)
    s.room("R", (0.0, 0.0, 0.0), (6.0, 6.0, 3.0), "Grey")
    pink = (1.0, 0.3, 0.6)
    s.point("Lamp", (3.0, 3.5, 2.2), 0.08, 300.0, pink)
    s.probe("ProbeSphere", (3.0, 3.8, 0.9))
    centre_view(s, extra=["ProbeSphere"])
    s.base("x3", "lamp at the centre, x = 3")
    for x in (1, 2, 4, 5):
        s.state("x%d" % x, "lamp at x = %d" % x, s.moved("Lamp", (float(x), 3.5, 2.2)))
    for left, right in (("x1", "x5"), ("x2", "x4")):
        for a, b in (("R_Xn", "R_Xp"), ("R_Zn", "R_Zn"), ("R_Yp", "R_Yp"), ("R_Zp", "R_Zp"),
                     ("ProbeSphere", "ProbeSphere")):
            s.mirror((left, "centre", a), (right, "centre", b))
    s.mirror(("x3", "centre", "R_Xn"), ("x3", "centre", "R_Xp"))
    for state in ("x1", "x2", "x3", "x4", "x5"):
        s.chroma(state, "centre", pink)
    s.ordered([("x3", "centre", "R_Xn"), ("x2", "centre", "R_Xn"), ("x1", "centre", "R_Xn")],
              1.1, lights=["total"])
    s.finish()


# ======================================================================
# furnace: L = Le / (1 - rho) on every surface of a closed uniform box
# ======================================================================

def furnace_values(albedo, emission):
    total = [e / (1 - r) for r, e in zip(albedo, emission)]
    return {"total": total, "direct": list(emission),
            "indirect": [t - e for t, e in zip(total, emission)]}


def colored_furnace(out, name, purpose, albedo, emission, clutter=None, size=(3.0, 3.0, 3.0),
                    eye=(0.35, 0.35, 1.6), target=(3.0, 3.0, 1.0), probe=(1.5, 1.5, 1.2),
                    extra_views=(), spawn=None):
    s = Scene(out, name, "furnace", purpose)
    s.spawn = spawn
    s.mat("Furnace", albedo, emission)
    s.mat("ProbeFurnace", albedo, emission)
    walls = s.room("Box", (0.0, 0.0, 0.0), size, "Furnace")
    s.probe("ProbeSphere", probe, "ProbeFurnace")
    regions = {"walls": walls, "model": ["ProbeSphere"]}
    if clutter:
        regions.update(clutter(s))
    s.view("inside", eye, target, regions)
    for view_name, view_eye, view_target, view_regions in extra_views:
        s.view(view_name, view_eye, view_target, view_regions)
    s.base("default", "uniform emission %s, albedo %s" % (list(emission), list(albedo)))
    values = furnace_values(albedo, emission)
    for camera in s.cameras:
        s.value("default", camera, list(s.regions[camera]), values,
                "furnace L = Le / (1 - rho) per channel (%s)" % camera)
    s.extra["analytic"] = {"albedo": list(albedo), "emission": list(emission),
                           **{"diffuse_light_" + k: v for k, v in values.items()}}
    s.finish()


def furnace_family(out):
    # L peaks at 0.8, not 1: the bake validates its linear EXR on texels away
    # from 1, where a gamma error would be invisible (pbrt_lightmap_bake.py).
    colored_furnace(out, "furnace-rgb", "Colored furnace: albedo (0.8, 0.5, 0.2) and emission "
                    "(0.16, 0.24, 0.32) give L = (0.8, 0.48, 0.4); channels must bounce "
                    "independently", (0.8, 0.5, 0.2), (0.16, 0.24, 0.32))
    colored_furnace(out, "furnace-high-albedo", "Albedo 0.9 furnace: 90% of the light is "
                    "indirect, reached only after dozens of bounces; bounce truncation shows",
                    (0.9, 0.9, 0.9), (0.05, 0.05, 0.05))
    colored_furnace(out, "furnace-low-albedo", "Albedo 0.1 furnace: almost all light is "
                    "direct; a producer adding ambient light shows", (0.1, 0.1, 0.1),
                    (0.5, 0.5, 0.5))
    # Albedos stay above 0: Cycles' diffuse light passes divide by the diffuse
    # color, and a channel of albedo 0 splits into meaningless direct and
    # indirect parts (measured: a (0.9, 0, 0.5) furnace read green direct 0.07
    # and indirect 0.23 for a true 0.3 and 0), so its reference would be wrong.
    colored_furnace(out, "furnace-near-black", "Albedo 0.02 furnace: almost no bounce, "
                    "indirect light 2% of the direct", (0.02, 0.02, 0.02), (0.4, 0.4, 0.4))
    colored_furnace(out, "furnace-channel-extremes", "Albedo (0.9, 0.05, 0.5): red bounces "
                    "ten times its emission, green barely bounces, blue doubles",
                    (0.9, 0.05, 0.5), (0.04, 0.3, 0.1))

    def clutter(s):
        m = "Furnace"
        s.box("Block", (2.4, 0.6, 0.5), (3.4, 1.4, 1.3), m)
        s.box("Slab", (0.5, 2.4, 1.8), (2.0, 3.5, 1.9), m)
        s.boxes("Fins", [((2.6 + 0.3 * i, 2.3, 0.3), (2.62 + 0.3 * i, 3.6, 2.4))
                         for i in range(4)], m)
        s.box("Pillar", (1.0, 1.0, 0.02), (1.4, 1.4, 2.98), m)
        s.boxes("Crevice", [((3.0, 0.2, 1.8), (3.4, 1.2, 2.6)),
                            ((3.41, 0.2, 1.8), (3.8, 1.2, 2.6))], m)
        s.sphere("Orb", (2.2, 2.0, 2.5), 0.35, m)
        return {"block": ["Block"], "slab": ["Slab"], "fins": ["Fins"], "pillar": ["Pillar"]}
    colored_furnace(out, "furnace-clutter", "Colored furnace full of occluders (block, slab, "
                    "thin fins, pillar, a 1 cm crevice, a sphere): the furnace property holds "
                    "on every surface regardless of occlusion", (0.7, 0.5, 0.3),
                    (0.1, 0.2, 0.3), clutter, size=(4.0, 4.0, 3.0), eye=(0.3, 3.7, 1.7),
                    target=(3.5, 0.8, 1.1), probe=(0.6, 2.2, 0.9),
                    spawn=((1.9, 1.9, 1.7), (3.9, 3.9, 1.0)),
                    extra_views=[("crevice", (3.9, 3.9, 2.8), (3.2, 0.8, 1.6),
                                  {"crevice": ["Crevice"], "fins": ["Fins"], "orb": ["Orb"],
                                   "walls": ["Box_Xn", "Box_Yn", "Box_Zn"]})])

    def maze(s):
        walls = [((1.9, 0.0, 0.02), (2.0, 5.5, 2.2)), ((4.0, 2.5, 0.02), (4.1, 8.0, 2.2)),
                 ((6.0, 0.0, 0.02), (6.1, 5.5, 2.2)), ((2.0, 2.4, 0.02), (3.2, 2.5, 2.2)),
                 ((4.1, 5.4, 0.02), (5.2, 5.5, 2.2)), ((0.0, 6.0, 0.02), (1.2, 6.1, 2.2))]
        s.boxes("Maze", walls, "Furnace")
        return {"maze": ["Maze"]}
    colored_furnace(out, "furnace-maze", "Furnace maze: light must turn many corners to reach "
                    "the dead ends, yet every surface still sees L", (0.6, 0.6, 0.6),
                    (0.2, 0.2, 0.2), maze, size=(8.0, 8.0, 2.5), eye=(1.0, 0.3, 1.5),
                    target=(1.0, 8.0, 0.8), probe=(1.0, 4.0, 1.0))

    s = Scene(out, "furnace-sphere", "furnace", "Spherical furnace (faceted, inward): curved "
              "walls reach the same L = Le / (1 - rho) per channel")
    albedo, emission = (0.5, 0.5, 0.5), (0.1, 0.15, 0.2)
    s.mat("Furnace", albedo, emission)
    s.mat("ProbeFurnace", albedo, emission)
    s.sphere("ShellUpper", (0.0, 0.0, 2.0), 2.0, "Furnace", inward=True, rows=(0, 12))
    s.sphere("ShellLower", (0.0, 0.0, 2.0), 2.0, "Furnace", inward=True, rows=(12, 24))
    s.probe("ProbeSphere", (0.6, 0.3, 2.0), "ProbeFurnace")
    s.view("inside", (-1.3, 0.0, 2.2), (1.0, 0.0, 1.9), ["ShellUpper", "ShellLower",
                                                          "ProbeSphere"])
    s.base("default", "uniform emission and albedo")
    s.value("default", "inside", ["ShellUpper", "ShellLower", "ProbeSphere"],
            furnace_values(albedo, emission), "spherical furnace L = Le / (1 - rho)")
    s.finish()


# ======================================================================
# analytic: open and large scenes with closed forms
# ======================================================================

def tinted_sky_plane(out):
    s = Scene(out, "tinted-sky-plane", "analytic", "A blue-tinted sky, then an ivory sun, over "
              "an orange plane: floor radiance is albedo x sky (or albedo x E sin e / pi) per "
              "channel, and an open plane has no indirect light at all")
    albedo = (0.9, 0.6, 0.3)
    sky, sun, irradiance, elevation = (0.2, 0.5, 0.9), (1.0, 0.85, 0.6), 3.0, 60.0
    s.mat("Ground", albedo)
    half = 20.0
    s.panel("Floor", (-half, -half, 0.0), (half, half, 0.0), 2, 1, "Ground")
    s.sky("Sky", 1.0, sky)
    # Authored dark: the base state is the sky alone.
    s.sun("Sun", 180.0, elevation, 0.0, sun)
    s.view("floor", (-3.0, 0.0, 1.7), (2.0, 0.0, 0.0), ["Floor"])
    s.base("sky", "tinted sky alone")
    s.state("sun", "sun alone", s.off("Sky") + s.intensity("Sun", irradiance))
    direct_sun = [irradiance * math.sin(math.radians(elevation)) / math.pi * c for c in sun]
    s.value("sky", "floor", ["Floor"], {"total": [r * k for r, k in zip(albedo, sky)],
                                        "direct": list(sky)},
            "sky: floor radiance = rho x sky, direct light = sky")
    s.value("sun", "floor", ["Floor"], {"total": [r * d for r, d in zip(albedo, direct_sun)],
                                        "direct": direct_sun},
            "sun: floor radiance = rho E sin(e) / pi per channel")
    s.zero("sky", "floor", [0, 1, 2], lights=["indirect"],
           name="sky: an open plane receives no indirect light")
    s.zero("sun", "floor", [0, 1, 2], lights=["indirect"],
           name="sun: an open plane receives no indirect light")
    s.probe_bounds = (-5.0, -5.0, 0.25, 5.0, 5.0, 2.75)
    s.finish()


def parallel_plates(out):
    s = Scene(out, "emissive-parallel-plates", "analytic", "Two 80 m plates 2.5 m apart: a "
              "glowing floor under a colored ceiling. Multiple bounces between them give "
              "B_floor = Le / (1 - rho_f rho_c) and B_ceiling = rho_c B_floor per channel")
    rho_f, rho_c, le = (0.5, 0.5, 0.5), (0.2, 0.6, 0.9), (0.4, 0.2, 0.05)
    s.mat("Glow", rho_f, le)
    s.mat("Canopy", rho_c)
    s.mat("Black", (0.0, 0.0, 0.0))
    # 80 m plates 2.5 m apart: the view factor to the black walls from the
    # centre is 0.4%; the cameras stand at eye height (a camera 0.5 m up in a
    # 1 m slot, looking straight up or down, read half the light in game).
    half, height = 40.0, 2.5
    s.room("P", (-half, -half, 0.0), (half, half, height), "Black",
           materials={"Zn": "Glow", "Zp": "Canopy"})
    s.view("up", (0.0, 0.0, 1.7), (1.0, 0.0, 1.7 + math.sqrt(3.0)), ["P_Zp"])
    s.view("down", (0.0, 0.0, 1.7), (1.0, 0.0, 1.7 - math.sqrt(3.0)), ["P_Zn"])
    # The centre views see one uniform plate each, which portal_boot's scene
    # detail gate rejects; their shaded proof frame comes from this view.
    s.view("overview", (-35.0, 0.0, 1.7), (0.0, 3.0, 1.2), ["P_Zn", "P_Zp"])
    s.extra["proof_cameras"] = {"up": "overview", "down": "overview"}
    s.base("default", "glowing floor, colored ceiling, black side walls 40 m away")
    floor = [e / (1 - f * c) for e, f, c in zip(le, rho_f, rho_c)]
    ceiling = [c * b for c, b in zip(rho_c, floor)]
    s.value("default", "up", ["P_Zp"], {"total": ceiling, "direct": list(le),
                                        "indirect": [b - e for b, e in zip(floor, le)]},
            "ceiling: rho_c Le / (1 - rho_f rho_c); incoming = floor radiance", tolerance=0.015)
    s.value("default", "down", ["P_Zn"], {"total": floor, "indirect": ceiling},
            "floor: Le / (1 - rho_f rho_c); incoming = ceiling radiance", tolerance=0.015)
    s.zero("default", "down", [0, 1, 2], lights=["direct"],
           name="floor sees no emitter directly")
    s.finish()


def integrating_sphere(out):
    s = Scene(out, "integrating-sphere", "analytic", "Integrating sphere: three colored lights "
              "at arbitrary positions inside a grey spherical cavity. Every wall point sees "
              "every other with the same form factor, so wall indirect light is uniform, with "
              "the lights' flux ratio as its color, wherever the lights are")
    s.mat("Shell", (0.6, 0.6, 0.6))
    center, radius = (0.0, 0.0, 2.0), 2.0
    quarters = {"ShellUE": ((0, 12), (36, 60)), "ShellUW": ((0, 12), (12, 36)),
                "ShellLE": ((12, 24), (36, 60)), "ShellLW": ((12, 24), (12, 36))}
    for name, (rows, columns) in quarters.items():
        faces = []
        for band in ((columns[0], min(columns[1], 48)), (0, max(columns[1] - 48, 0))):
            if band[1] > band[0]:
                faces += sphere_faces(center, radius, True, rows=rows, columns=band)
        s.mesh(name, faces, "Shell")
    s.layout = "planar"
    lights = {"Red": ((0.8, 0.5, 2.6), RED, 400.0), "Green": ((-0.9, -0.4, 1.3), GREEN, 200.0),
              "Blue": ((0.2, -1.1, 3.1), BLUE, 100.0)}
    for name, (position, color, intensity) in lights.items():
        s.point(name, position, 0.05, intensity, color)
    s.view("east", (0.0, 0.0, 2.0), (1.0, 0.0, 2.0), ["ShellUE", "ShellLE"])
    s.view("west", (0.0, 0.0, 2.0), (-1.0, 0.0, 2.0), ["ShellUW", "ShellLW"])
    s.base("all", "all three lights on")
    for name in lights:
        s.state(name.lower(), name.lower() + " light alone", s.solo(lights, [name]))
    s.superpose_all("all", ["red", "green", "blue"])
    entries = [("all", "east", "ShellUE"), ("all", "east", "ShellLE"),
               ("all", "west", "ShellUW"), ("all", "west", "ShellLW")]
    s.uniform(entries, name="all: wall indirect light is uniform")
    for state in ("red", "green", "blue"):
        s.uniform([(state, c, r) for _, c, r in entries],
                  name="%s: wall indirect light is uniform" % state)
    for camera in ("east", "west"):
        s.chroma("all", camera, (1.0, 0.5, 0.25), lights=["indirect"],
                 name="all: indirect color is the lights' flux ratio 4:2:1 (%s)" % camera)
    s.finish()


# ======================================================================
# bleed: color bleeding
# ======================================================================

def cornell(out):
    s = Scene(out, "bleed-cornell", "bleed", "The Cornell box: red and green side walls tint "
              "the white surfaces nearest them; the block faces facing each wall take its "
              "color")
    s.mat("White", (0.73, 0.73, 0.73))
    s.mat("Red", (0.63, 0.065, 0.05))
    s.mat("Green", (0.14, 0.45, 0.09))
    s.room("C", (0.0, 0.0, 0.0), (3.0, 3.0, 3.0), "White", skip=("Yp", "Zn"),
           materials={"Xn": "Red", "Xp": "Green"})
    s.panel("BackLeft", (0.0, 3.0, 0.0), (1.5, 3.0, 3.0), 1, -1, "White")
    s.panel("BackRight", (1.5, 3.0, 0.0), (3.0, 3.0, 3.0), 1, -1, "White")
    s.panel("FloorLeft", (0.0, 0.0, 0.0), (1.5, 3.0, 0.0), 2, 1, "White")
    s.panel("FloorRight", (1.5, 0.0, 0.0), (3.0, 3.0, 0.0), 2, 1, "White")
    s.box_faces("Tall", (0.4, 1.7, 0.0), (1.1, 2.4, 1.8), "White", skip=("Zn",))
    s.box_faces("Short", (1.9, 0.8, 0.0), (2.6, 1.5, 0.9), "White", skip=("Zn",))
    s.rect("Lamp", (1.5, 1.5, 2.99), (0.75, 0.75), 12.0)
    s.probe("ProbeSphere", (1.5, 2.5, 2.1))
    s.view("front", (1.5, 0.1, 1.5), (1.5, 3.0, 1.2),
           ["C_Xn", "C_Xp", "BackLeft", "BackRight", "FloorLeft", "FloorRight", "C_Zp",
            "Tall_Yn", "Tall_Xp", "Short_Yn", "Short_Xn", "ProbeSphere"])
    s.base("default", "ceiling lamp on")
    for a, b in (("BackLeft", "BackRight"), ("FloorLeft", "FloorRight"),
                 ("Short_Xn", "Tall_Xp")):
        s.tint_order([("default", "front", a), ("default", "front", b)], 0, 1, 1.05,
                     lights=["indirect"])
    s.finish()


def cornell_mirror(out):
    s = Scene(out, "bleed-cornell-mirror", "bleed", "A symmetric Cornell box: walls (0.7, "
              "0.1, 0.1) and (0.1, 0.7, 0.1). Mirroring swaps red and green, so every centred "
              "surface has equal red and green light")
    s.mat("White", (0.73, 0.73, 0.73))
    s.mat("Red", (0.7, 0.1, 0.1))
    s.mat("Green", (0.1, 0.7, 0.1))
    s.room("C", (0.0, 0.0, 0.0), (3.0, 3.0, 3.0), "White",
           materials={"Xn": "Red", "Xp": "Green"})
    s.rect("Lamp", (1.5, 1.5, 2.99), (0.75, 0.75), 12.0)
    s.probe("ProbeSphere", (1.5, 1.8, 1.2))
    s.view("front", (1.5, 0.1, 1.5), (1.5, 3.0, 1.2),
           ["C_Xn", "C_Xp", "C_Yp", "C_Zn", "C_Zp", "ProbeSphere"])
    s.base("default", "ceiling lamp on")
    s.mirror(("default", "front", "C_Xn"), ("default", "front", "C_Xp"), SWAP_RG)
    self_mirrors(s, "default", "front", SWAP_RG, ["C_Yp", "C_Zn", "C_Zp", "ProbeSphere"])
    s.dominant("default", "front", "C_Zn", 0, [2], 1.1, lights=["indirect"])
    s.finish()


def blue_carpet(out):
    s = Scene(out, "bleed-blue-carpet", "bleed", "A deep blue carpet under a downward lamp: "
              "the ceiling gets no direct light and its bounce light is blue")
    s.mat("White", WHITE)
    s.mat("Carpet", (0.1, 0.2, 0.75))
    s.room("R", (0.0, 0.0, 0.0), (5.0, 5.0, 3.0), "White", materials={"Zn": "Carpet"})
    s.rect("Lamp", (2.5, 2.5, 2.9), (1.0, 1.0), 12.0)
    s.probe("ProbeSphere", (2.5, 1.2, 2.2))
    s.view("up", (0.3, 0.3, 1.2), (5.0, 5.0, 3.0), ["R_Zp", "R_Xp", "R_Yp", "ProbeSphere"])
    s.view("down", (0.3, 0.3, 2.6), (4.0, 4.0, 0.0), ["R_Zn", "R_Xp", "R_Yp"])
    s.base("default", "lamp on")
    s.zero("default", "up", [0, 1, 2], ["R_Zp"], ["direct"],
           name="the ceiling sees no emitter")
    s.dominant("default", "up", "R_Zp", 2, [0], 1.3, lights=["indirect"])
    s.dominant("default", "up", "R_Zp", 2, [1], 1.1, lights=["indirect"])
    s.dominant("default", "up", "ProbeSphere", 2, [0], 1.1, lights=["indirect"])
    s.finish()


def two_bounce_filter(out):
    s = Scene(out, "bleed-two-bounce-filter", "bleed", "White light reaches the third room "
              "only off yellow walls and then cyan walls: yellow removes blue, cyan removes "
              "red, and green alone arrives. No direct light passes either doorway")
    s.mat("Yellow", (0.9, 0.9, 0.1))
    s.mat("Cyan", (0.1, 0.9, 0.9))
    s.mat("White", WHITE)
    outer = ((0.0, 0.0), (3.0, 3.0))
    door1, door2 = ((0.2, 0.0), (1.2, 2.0)), ((1.8, 0.0), (2.8, 2.0))
    s.room("A", (0.0, 0.0, 0.0), (3.0, 3.0, 3.0), "Yellow", skip=("Xp",))
    s.wall("A_Xp", 3.0, 0, -1, outer, [door1], "Yellow")
    s.doorway_x("Door1", 3.0, 3.2, (0.2, 1.2), (0.0, 2.0), "Yellow")
    s.room("B", (3.2, 0.0, 0.0), (6.2, 3.0, 3.0), "Cyan", skip=("Xn", "Xp"))
    s.wall("B_Xn", 3.2, 0, 1, outer, [door1], "Cyan")
    s.wall("B_Xp", 6.2, 0, -1, outer, [door2], "Cyan")
    s.doorway_x("Door2", 6.2, 6.4, (1.8, 2.8), (0.0, 2.0), "Cyan")
    s.room("C", (6.4, 0.0, 0.0), (9.4, 3.0, 3.0), "White", skip=("Xn",))
    s.wall("C_Xn", 6.4, 0, 1, outer, [door2], "White")
    s.rect("Lamp", (2.6, 2.2, 1.5), (0.8, 0.8), 25.0, direction=(-1.0, 0.0, 0.0))
    s.view("room-a", (0.2, 2.8, 1.8), (3.0, 0.3, 0.5), ["A_Zn", "A_Yn", "A_Xp"])
    s.view("room-b", (6.0, 0.2, 2.0), (3.2, 2.8, 0.6), ["B_Zn", "B_Yp", "B_Xn", "B_Zp"])
    s.view("room-c", (9.2, 0.2, 1.8), (6.6, 2.6, 0.6), ["C_Zn", "C_Yp", "C_Xn"])
    s.base("default", "lamp in room A facing away from the doorway")
    s.zero("default", "room-b", [0, 1, 2], lights=["direct"],
           name="room B receives no direct light")
    s.zero("default", "room-c", [0, 1, 2], lights=["direct"],
           name="room C receives no direct light")
    for region in ("C_Zn", "C_Yp", "C_Xn"):
        s.dominant("default", "room-c", region, 1, [0, 2], 3.0, lights=["indirect"])
    s.tint_order([("default", "room-a", "A_Zn"), ("default", "room-b", "B_Zn"),
                  ("default", "room-c", "C_Zn")], 0, 1, 1.5, lights=["indirect"])
    s.finish()


def checker_hall(out):
    s = Scene(out, "bleed-checker-hall", "bleed", "A red and blue checkerboard floor: "
              "reflecting across the hall's midline swaps red and blue tiles, so the two "
              "tile sets and the two side walls match with channels swapped")
    s.mat("White", WHITE)
    s.mat("TileRed", (0.7, 0.1, 0.1))
    s.mat("TileBlue", (0.1, 0.1, 0.7))
    s.room("R", (0.0, 0.0, 0.0), (8.0, 8.0, 3.0), "White", skip=("Zn",))
    red, blue = [], []
    for i in range(8):
        for j in range(8):
            face = gf.quad((float(i), float(j), 0.0), (i + 1.0, j + 1.0, 0.0), 2, 1)
            (red if (i + j) % 2 == 0 else blue).append(face)
    s.mesh("FloorRed", red, "TileRed")
    s.mesh("FloorBlue", blue, "TileBlue")
    s.rect("Lamp", (4.0, 4.0, 2.99), (2.0, 2.0), 6.0)
    regions = ["R_Xn", "R_Xp", "R_Yp", "R_Zp", "FloorRed", "FloorBlue"]
    s.view("mid", (4.0, 0.3, 1.8), (4.0, 8.0, 0.3), regions)
    s.view("diagonal", (0.3, 0.3, 1.8), (8.0, 8.0, 0.3),
           ["R_Xp", "R_Yp", "FloorRed", "FloorBlue"])
    s.base("default", "lamp on")
    s.mirror(("default", "mid", "R_Xn"), ("default", "mid", "R_Xp"), SWAP_RB)
    s.mirror(("default", "mid", "FloorRed"), ("default", "mid", "FloorBlue"), SWAP_RB)
    self_mirrors(s, "default", "mid", SWAP_RB, ["R_Yp", "R_Zp"])
    s.mirror(("default", "diagonal", "R_Xp"), ("default", "diagonal", "R_Yp"))
    s.dominant("default", "mid", "R_Zp", 0, [1], 1.5, lights=["indirect"])
    s.dominant("default", "mid", "R_Zp", 2, [1], 1.5, lights=["indirect"])
    s.finish()


def hue_pillars(out):
    s = Scene(out, "bleed-hue-pillars", "bleed", "Red, green and blue pillars on a white "
              "floor: each floor cell is tinted by its pillar, and mirroring swaps the red "
              "and blue sides")
    s.mat("White", WHITE)
    colors = {"Red": (0.8, 0.1, 0.1), "Green": (0.1, 0.8, 0.1), "Blue": (0.1, 0.1, 0.8)}
    s.room("R", (0.0, 0.0, 0.0), (9.0, 9.0, 3.0), "White", skip=("Zn",))
    cells = []
    for i in range(3):
        cells.append(s.panel("Cell%d" % i, (3.0 * i, 0.0, 0.0), (3.0 * i + 3.0, 9.0, 0.0), 2, 1,
                             "White"))
    for i, (name, albedo) in enumerate(colors.items()):
        s.mat(name, albedo)
        x = 1.5 + 3.0 * i
        s.box("Pillar" + name, (x - 0.4, 4.1, 0.0), (x + 0.4, 4.9, 3.0), name,
              skip=("Zn", "Zp"))
    s.rect("Lamp", (4.5, 4.5, 2.99), (6.0, 6.0), 1.5)
    s.view("front", (4.5, 0.3, 2.5), (4.5, 6.0, 0.0),
           cells + ["PillarRed", "PillarGreen", "PillarBlue", "R_Yp"])
    s.base("default", "lamp on")
    s.mirror(("default", "front", "Cell0"), ("default", "front", "Cell2"), SWAP_RB)
    s.mirror(("default", "front", "PillarRed"), ("default", "front", "PillarBlue"), SWAP_RB)
    self_mirrors(s, "default", "front", SWAP_RB, ["Cell1", "PillarGreen", "R_Yp"])
    s.tint_order([("default", "front", "Cell0"), ("default", "front", "Cell1"),
                  ("default", "front", "Cell2")], 0, 2, 1.05, lights=["indirect"])
    s.tint_order([("default", "front", "Cell1"), ("default", "front", "Cell0")], 1, 0, 1.05,
                 lights=["indirect"])
    s.finish()


# ======================================================================
# leak: sealed rooms and narrow openings
# ======================================================================

def leak_rgb_rooms(out):
    s = Scene(out, "leak-rgb-rooms", "leak", "Three sealed rooms in a row behind 4-unit walls, "
              "lit red, green and blue: no room may contain another room's color")
    s.mat("Wall", GREY)
    gap = gf.THIN_WALL_M
    rooms = [("Red", RED), ("Green", GREEN), ("Blue", BLUE)]
    for i, (name, color) in enumerate(rooms):
        x0 = i * (3.0 + gap)
        s.room(name, (x0, 0.0, 0.0), (x0 + 3.0, 3.0, 3.0), "Wall")
        s.rect("Lamp" + name, (x0 + 1.5, 1.5, 2.99), (1.0, 1.0), 8.0, color)
        s.probe(("ProbeA", "ProbeB", "ProbeC")[i],
                (x0 + (2.4 if i < 2 else 0.6), 1.5, 1.2))
    x1, x2 = 3.0 + gap, 2 * (3.0 + gap)
    s.view("red", (0.3, 0.3, 1.6), (3.0, 2.0, 1.0), ["Red_Xp", "Red_Yp", "Red_Zn", "ProbeA"])
    s.view("green", (x1 + 1.5, 0.2, 1.6), (x1 + 1.5, 3.0, 1.0),
           ["Green_Xn", "Green_Xp", "Green_Yp", "Green_Zn", "ProbeB"])
    s.view("blue", (x2 + 2.7, 2.7, 1.6), (x2, 1.0, 1.0),
           ["Blue_Xn", "Blue_Yn", "Blue_Zn", "ProbeC"])
    s.base("default", "each room lit by its own color")
    for name, zero in (("red", [1, 2]), ("green", [0, 2]), ("blue", [0, 1])):
        s.zero("default", name, zero)
    s.finish()


def leak_quad_rooms(out):
    s = Scene(out, "leak-quad-rooms", "leak", "Four sealed rooms around one corner (red, "
              "green, blue, white), each with a model beside the shared corner where a probe "
              "cell spans all four: colors stay in their rooms, and white stays neutral")
    s.mat("Wall", GREY)
    gap = gf.THIN_WALL_M
    rooms = {"Red": (0, 0, RED, "ProbeA"), "Green": (1, 0, GREEN, "ProbeB"),
             "Blue": (0, 1, BLUE, "ProbeC"), "White": (1, 1, (1.0, 1.0, 1.0), "ProbeD")}
    for name, (i, j, color, probe) in rooms.items():
        x0, y0 = i * (3.0 + gap), j * (3.0 + gap)
        s.room(name, (x0, y0, 0.0), (x0 + 3.0, y0 + 3.0, 3.0), "Wall")
        s.rect("Lamp" + name, (x0 + 1.5, y0 + 1.5, 2.99), (1.0, 1.0), 8.0, color)
        near = (x0 + (2.4 if i == 0 else 0.6), y0 + (2.4 if j == 0 else 0.6), 1.2)
        s.probe(probe, near)
        far = (x0 + (0.2 if i == 0 else 2.8), y0 + (0.2 if j == 0 else 2.8), 1.7)
        s.view(name.lower(), far, (near[0], near[1], 0.9),
               [name + "_Zn", name + ("_Xp" if i == 0 else "_Xn"),
                name + ("_Yp" if j == 0 else "_Yn"), probe])
    s.base("default", "each room lit by its own color")
    for name, zero in (("red", [1, 2]), ("green", [0, 2]), ("blue", [0, 1])):
        s.zero("default", name, zero)
    s.chroma("default", "white", (1.0, 1.0, 1.0), name="the white room stays neutral")
    s.finish()


def leak_slit(out):
    s = Scene(out, "leak-slit", "leak", "Orange light enters a dark room only through a 5 cm "
              "slit: the dark room is lit, far less than the lit one, and exactly orange")
    s.mat("Wall", GREY)
    outer = ((0.0, 0.0), (3.0, 3.0))
    slit = ((1.475, 0.5), (1.525, 2.5))
    s.room("A", (0.0, 0.0, 0.0), (3.0, 3.0, 3.0), "Wall", skip=("Xp",))
    s.wall("A_Xp", 3.0, 0, -1, outer, [slit], "Wall")
    s.doorway_x("Slit", 3.0, 3.2, (1.475, 1.525), (0.5, 2.5), "Wall")
    s.room("B", (3.2, 0.0, 0.0), (6.2, 3.0, 3.0), "Wall", skip=("Xn",))
    s.wall("B_Xn", 3.2, 0, 1, outer, [slit], "Wall")
    orange = (1.0, 0.5, 0.1)
    s.rect("Lamp", (1.5, 1.5, 2.99), (1.0, 1.0), 10.0, orange)
    s.probe("ProbeB", (4.2, 1.5, 1.2))
    s.view("lit", (0.2, 0.2, 1.7), (3.0, 2.0, 0.8), ["A_Zn", "A_Xp", "A_Yp"])
    s.view("dark", (6.0, 0.2, 1.7), (3.2, 2.2, 0.9), ["B_Zn", "B_Xn", "B_Yp", "B_Zp",
                                                      "ProbeB"])
    s.base("default", "lamp in room A")
    # The dim room's shaded frame shows too little detail to prove a render.
    s.extra["proof_cameras"] = {"dark": "lit"}
    s.chroma("default", "dark", orange, name="light through the slit stays orange")
    s.chroma("default", "lit", orange)
    s.ordered([("default", "dark", "B_Zn"), ("default", "lit", "A_Zn")], 10.0,
              lights=["total"], name="the slit passes under a tenth of room A's light")
    s.finish()


def leak_stacked(out):
    s = Scene(out, "leak-stacked-floors", "leak", "A green-lit room above a red-lit room, "
              "separated by a 4-unit slab: neither color crosses the slab")
    s.mat("Wall", GREY)
    gap = gf.THIN_WALL_M
    top = 3.0 + gap
    s.room("Low", (0.0, 0.0, 0.0), (4.0, 4.0, 3.0), "Wall")
    s.room("High", (0.0, 0.0, top), (4.0, 4.0, top + 3.0), "Wall")
    s.rect("LampLow", (2.0, 2.0, 2.99), (1.0, 1.0), 4.0, RED)
    s.rect("LampHigh", (2.0, 2.0, top + 2.99), (1.0, 1.0), 8.0, GREEN)
    s.probe("ProbeA", (2.8, 2.8, 2.4))
    s.probe("ProbeB", (2.8, 2.8, top + 0.6))
    s.view("low", (0.3, 0.3, 1.2), (3.0, 3.0, 2.4), ["Low_Zp", "Low_Xp", "Low_Yp", "ProbeA"])
    s.view("high", (0.3, 0.3, top + 2.2), (3.0, 3.0, top + 0.3),
           ["High_Zn", "High_Xp", "High_Yp", "ProbeB"])
    s.base("default", "red below, green above")
    s.zero("default", "low", [1, 2])
    s.zero("default", "high", [0, 2])
    s.finish()


def louver(out):
    s = Scene(out, "leak-louver", "leak", "An orange-lit room and a blue-lit room joined by a "
              "louvered opening of thin slats: light crosses in both directions and adds, "
              "each color unchanged by the grey slats")
    s.mat("Wall", GREY)
    outer = ((0.0, 0.0), (4.0, 3.0))
    hole = ((0.5, 0.3), (3.5, 2.7))
    s.room("A", (0.0, 0.0, 0.0), (4.0, 4.0, 3.0), "Wall", skip=("Xp",))
    s.wall("A_Xp", 4.0, 0, -1, outer, [hole], "Wall")
    s.doorway_x("Opening", 4.0, 4.2, (0.5, 3.5), (0.3, 2.7), "Wall")
    s.room("B", (4.2, 0.0, 0.0), (8.2, 4.0, 3.0), "Wall", skip=("Xn",))
    s.wall("B_Xn", 4.2, 0, 1, outer, [hole], "Wall")
    slats, z = [], 0.36
    while z + 0.04 < 2.7:
        slats.append(((4.0, 0.5, z), (4.2, 3.5, z + 0.04)))
        z += 0.12
    s.boxes("Louvers", slats, "Wall", skip=("Yn", "Yp"))
    orange, blue = (1.0, 0.6, 0.2), (0.2, 0.4, 1.0)
    s.rect("Orange", (2.0, 2.0, 2.99), (1.0, 1.0), 10.0, orange)
    s.point("Blue", (7.2, 2.0, 2.2), 0.08, 200.0, blue)
    s.probe("ProbeA", (3.2, 2.0, 1.4))
    s.probe("ProbeB", (5.0, 2.0, 1.4))
    s.view("a", (0.2, 0.3, 1.7), (4.0, 2.5, 1.0), ["A_Zn", "A_Yp", "A_Xp", "ProbeA"])
    s.view("b", (8.0, 0.3, 1.7), (4.2, 2.5, 1.0), ["B_Zn", "B_Yp", "B_Xn", "ProbeB"])
    s.base("both", "both rooms lit")
    s.state("orange", "orange room alone", s.off("Blue"))
    s.state("blue", "blue room alone", s.off("Orange"))
    s.superpose_all("both", ["orange", "blue"])
    s.chroma_all("orange", orange)
    s.chroma_all("blue", blue)
    s.finish()


# ======================================================================
# occlusion
# ======================================================================

def l_corridor(out):
    s = Scene(out, "occlusion-l-corridor", "occlusion", "A magenta lamp at one end of an L "
              "corridor: the far leg is lit only by bounces, around an orange corner, so it "
              "has no direct light, no green, and more red than blue")
    s.mat("White", WHITE)
    s.mat("Orange", (0.9, 0.5, 0.1))
    s.panel("Floor1", (0.0, 0.0, 0.0), (8.0, 2.0, 0.0), 2, 1, "White")
    s.panel("Ceil1", (0.0, 0.0, 3.0), (8.0, 2.0, 3.0), 2, -1, "White")
    s.panel("FloorNear", (6.0, 2.0, 0.0), (8.0, 6.0, 0.0), 2, 1, "White")
    s.panel("CeilNear", (6.0, 2.0, 3.0), (8.0, 6.0, 3.0), 2, -1, "White")
    s.panel("FloorFar", (6.0, 6.0, 0.0), (8.0, 10.0, 0.0), 2, 1, "White")
    s.panel("CeilFar", (6.0, 6.0, 3.0), (8.0, 10.0, 3.0), 2, -1, "White")
    s.panel("South", (0.0, 0.0, 0.0), (8.0, 0.0, 3.0), 1, 1, "White")
    s.panel("North1", (0.0, 2.0, 0.0), (6.0, 2.0, 3.0), 1, -1, "White")
    s.panel("West", (0.0, 0.0, 0.0), (0.0, 2.0, 3.0), 0, 1, "White")
    s.panel("Corner", (8.0, 0.0, 0.0), (8.0, 6.0, 3.0), 0, -1, "Orange")
    s.panel("EastFar", (8.0, 6.0, 0.0), (8.0, 10.0, 3.0), 0, -1, "White")
    s.panel("WestNear", (6.0, 2.0, 0.0), (6.0, 6.0, 3.0), 0, 1, "White")
    s.panel("WestFar", (6.0, 6.0, 0.0), (6.0, 10.0, 3.0), 0, 1, "White")
    s.panel("End", (6.0, 10.0, 0.0), (8.0, 10.0, 3.0), 1, -1, "White")
    magenta = MAGENTA
    s.rect("Lamp", (0.01, 1.0, 1.5), (1.0, 1.0), 15.0, magenta, direction=(1.0, 0.0, 0.0))
    s.probe("ProbeSphere", (7.0, 7.5, 1.2))
    s.view("far", (7.0, 9.8, 1.6), (7.0, 4.0, 0.8),
           ["FloorFar", "EastFar", "WestFar", "CeilFar", "ProbeSphere"])
    s.view("near", (1.0, 1.0, 1.6), (8.0, 1.5, 1.0), ["Floor1", "Corner", "South", "North1"])
    s.base("default", "magenta lamp at the west end")
    s.zero_all("default", [1])
    s.zero("default", "far", [0, 1, 2], lights=["direct"],
           name="the far leg receives no direct light")
    for region in ("FloorFar", "EastFar", "WestFar", "ProbeSphere"):
        s.dominant("default", "far", region, 0, [2], 1.2, lights=["indirect"])
    s.tint_order([("default", "far", "FloorFar"), ("default", "near", "Floor1")], 0, 2, 1.05,
                 lights=["indirect"])
    s.finish()


def colored_shadows(out):
    s = Scene(out, "occlusion-colored-shadows", "occlusion", "Red, green and blue lamps round "
              "a pillar: each face is lit by the lamp it faces, the face turned from all three "
              "gets only bounce light, and the red/blue mirror swaps faces")
    s.mat("White", WHITE)
    s.room("R", (0.0, 0.0, 0.0), (8.0, 8.0, 4.0), "White")
    s.box_faces("Pillar", (3.7, 3.7, 0.0), (4.3, 4.3, 4.0), "White", skip=("Zn", "Zp"))
    lights = {"Red": ((6.5, 4.0, 2.0), RED), "Green": ((4.0, 6.5, 2.0), GREEN),
              "Blue": ((1.5, 4.0, 2.0), BLUE)}
    for name, (center, color) in lights.items():
        s.point(name, center, 0.08, 300.0, color)
    s.view("south-east", (7.6, 0.4, 2.0), (4.0, 4.0, 1.5), ["Pillar_Xp", "Pillar_Yn", "R_Zn"])
    s.view("north-west", (0.4, 7.6, 2.0), (4.0, 4.0, 1.5), ["Pillar_Xn", "Pillar_Yp", "R_Zn"])
    s.view("south", (4.0, 0.3, 2.0), (4.0, 4.0, 1.2), ["Pillar_Yn", "R_Zn", "R_Xn", "R_Xp"])
    s.base("all", "three lamps on")
    for name in lights:
        s.state(name.lower(), name.lower() + " lamp alone", s.solo(lights, [name]))
    s.superpose_all("all", ["red", "green", "blue"])
    s.dominant("all", "south-east", "Pillar_Xp", 0, [1, 2], 3.0, lights=["direct"])
    s.dominant("all", "north-west", "Pillar_Xn", 2, [0, 1], 3.0, lights=["direct"])
    s.dominant("all", "north-west", "Pillar_Yp", 1, [0, 2], 3.0, lights=["direct"])
    s.zero("all", "south", [0, 1, 2], ["Pillar_Yn"], ["direct"],
           name="the south face sees no lamp")
    s.mirror(("all", "south", "R_Xn"), ("all", "south", "R_Xp"), SWAP_RB)
    self_mirrors(s, "all", "south", SWAP_RB, ["Pillar_Yn", "R_Zn"])
    s.finish()


# ======================================================================
# probe: dynamic-model lighting by direction and height
# ======================================================================

def directional_probe(out):
    s = Scene(out, "probe-directional-rgb", "probe", "A dynamic model lit red from the east, "
              "green from the north and blue from above, seen from five sides: each side "
              "shows its light's color, and a lamp's far side is exactly unlit")
    s.mat("Dim", (0.3, 0.3, 0.3))
    s.room("R", (0.0, 0.0, 0.0), (8.0, 8.0, 4.0), "Dim")
    center = (4.0, 4.0, 1.6)
    lights = {"Red": ((7.0, 4.0, 1.6), RED), "Green": ((4.0, 7.0, 1.6), GREEN),
              "Blue": ((4.0, 4.0, 3.6), BLUE)}
    for name, (position, color) in lights.items():
        s.point(name, position, 0.1, 200.0, color)
    s.probe("ProbeSphere", center)
    for name, eye in (("east", (6.5, 4.0, 1.6)), ("west", (1.5, 4.0, 1.6)),
                      ("north", (4.0, 6.5, 1.6)), ("south", (4.0, 1.5, 1.6)),
                      ("top", (4.0, 3.3, 3.2))):
        s.view(name, eye, center, ["ProbeSphere"])
    s.base("all", "three lamps on")
    for name in lights:
        s.state(name.lower(), name.lower() + " lamp alone", s.solo(lights, [name]))
    s.superpose_all("all", ["red", "green", "blue"], lights=ALL_LIGHTS)
    s.zero("red", "west", [0, 1, 2], lights=["direct"],
           name="red: the model's west side is unlit")
    s.zero("green", "south", [0, 1, 2], lights=["direct"],
           name="green: the model's south side is unlit")
    s.tint_order([("all", "east", "ProbeSphere"), ("all", "west", "ProbeSphere")], 0, 1, 2.0,
                  lights=["total"])
    s.tint_order([("all", "north", "ProbeSphere"), ("all", "south", "ProbeSphere")], 1, 0,
                  2.0, lights=["total"])
    s.tint_order([("all", "top", "ProbeSphere"), ("all", "south", "ProbeSphere")], 2, 1, 1.2,
                  lights=["total"])
    s.dominant("all", "east", "ProbeSphere", 0, [1, 2], 1.2, lights=["direct"])
    s.finish()


def vertical_gradient(out):
    s = Scene(out, "probe-vertical-gradient", "probe", "A tall room with a green floor and a "
              "magenta ceiling: dynamic models low, middle and high read greener near the "
              "floor and more magenta near the ceiling")
    s.mat("Grey", (0.5, 0.5, 0.5))
    s.mat("Green", (0.1, 0.7, 0.1))
    s.mat("Magenta", (0.7, 0.1, 0.7))
    s.room("R", (0.0, 0.0, 0.0), (4.0, 4.0, 8.0), "Grey",
           materials={"Zn": "Green", "Zp": "Magenta"})
    for name, center in (("N", (2.0, 3.5, 4.0)), ("S", (2.0, 0.5, 4.0)),
                         ("E", (3.5, 2.0, 4.0)), ("W", (0.5, 2.0, 4.0))):
        s.point("Lamp" + name, center, 0.1, 200.0)
    s.probe("ProbeA", (2.0, 2.0, 1.0))
    s.probe("ProbeB", (2.0, 2.0, 4.0))
    s.probe("ProbeC", (2.0, 2.0, 7.0))
    # One level camera per model, so each sees its model's upper and lower
    # halves alike (a camera above a model mostly sees the ceiling's light).
    for camera, probe, z in (("low", "ProbeA", 1.0), ("mid", "ProbeB", 4.0),
                             ("high", "ProbeC", 7.0)):
        s.view(camera, (0.3, 0.3, z), (2.0, 2.0, z), [probe, "R_Xp", "R_Yp"])
    s.base("default", "four white lamps at mid height")
    s.tint_order([("default", "low", "ProbeA"), ("default", "mid", "ProbeB"),
                  ("default", "high", "ProbeC")], 1, 0, 1.1, lights=["indirect"])
    s.tint_order([("default", "high", "ProbeC"), ("default", "mid", "ProbeB"),
                  ("default", "low", "ProbeA")], 2, 1, 1.1, lights=["indirect"])
    s.finish()


# ======================================================================
# symmetry
# ======================================================================

def courtyard_sun(out):
    s = Scene(out, "symmetry-courtyard-sun", "symmetry", "An open courtyard under a warm sun "
              "at azimuth 30 degrees, then 150: the two are mirror images, wall for wall")
    s.mat("Stone", GREY)
    s.room("Y", (-4.0, -4.0, 0.0), (4.0, 4.0, 4.0), "Stone", skip=("Zp",))
    s.sun("Sun", 30.0, 40.0, 3.0, (1.0, 0.8, 0.55))
    s.sky("Sky", 0.4, (0.4, 0.6, 1.0))
    s.probe("ProbeSphere", (0.0, 1.0, 1.0))
    s.view("centre", (0.0, -3.8, 2.5), (0.0, 2.0, 0.5),
           ["Y_Xn", "Y_Xp", "Y_Yp", "Y_Zn", "ProbeSphere"])
    s.base("az30", "sun from azimuth 30 degrees")
    a, e = math.radians(150.0), math.radians(40.0)
    travel = (-math.cos(e) * math.cos(a), -math.cos(e) * math.sin(a), -math.sin(e))
    s.state("az150", "sun from azimuth 150 degrees",
            [(s.light_path("Sun"), "xformOp:transform", Sdf.ValueTypeNames.Matrix4d,
              gf.look_rotation(travel))])
    for a_region, b_region in (("Y_Xn", "Y_Xp"), ("Y_Xp", "Y_Xn"), ("Y_Yp", "Y_Yp"),
                               ("Y_Zn", "Y_Zn"), ("ProbeSphere", "ProbeSphere")):
        s.mirror(("az30", "centre", a_region), ("az150", "centre", b_region),
                 lights=ALL_LIGHTS)
    s.probe_bounds = (-4.0, -4.0, 0.25, 4.0, 4.0, 3.75)
    s.finish()


# ======================================================================
# range: intensity, albedo, HDR and color temperature
# ======================================================================

def intensity_ladder(out):
    s = Scene(out, "range-intensity-ladder", "range", "One orange lamp from 0.01x to 100x: "
              "light scales exactly with emission over four decades and keeps its color")
    s.mat("Grey", GREY)
    s.room("R", (0.0, 0.0, 0.0), (4.0, 4.0, 3.0), "Grey")
    orange = (1.0, 0.55, 0.2)
    base = 10.0
    s.rect("Lamp", (2.0, 2.0, 2.99), (1.0, 1.0), base, orange)
    s.probe("ProbeSphere", (2.0, 2.0, 1.0))
    s.view("corner", (0.3, 0.3, 1.7), (4.0, 4.0, 0.8), ["R_Zn", "R_Xp", "R_Yp", "ProbeSphere"])
    s.base("x1", "lamp at intensity %g" % base)
    for label, factor in (("x0p01", 0.01), ("x0p1", 0.1), ("x10", 10.0), ("x100", 100.0)):
        s.state(label, "lamp at %g x" % factor, s.intensity("Lamp", base * factor))
        s.scaled("corner", "x1", label, factor)
    for state in ("x0p01", "x1", "x100"):
        s.chroma(state, "corner", orange)
    s.finish()


def albedo_ladder(out):
    s = Scene(out, "range-albedo-ladder", "range", "Five sealed booths with the same lamp and "
              "albedo 0.1, 0.3, 0.5, 0.7, 0.9: indirect light rises steeply with albedo")
    entries = []
    for i, rho in enumerate((0.1, 0.3, 0.5, 0.7, 0.9)):
        name = "Booth%d" % i
        s.mat("Albedo%d" % i, (rho, rho, rho))
        x0 = 2.2 * i
        s.room(name, (x0, 0.0, 0.0), (x0 + 2.0, 2.0, 2.5), "Albedo%d" % i)
        s.rect("Lamp%d" % i, (x0 + 1.0, 1.0, 2.49), (0.6, 0.6), 8.0)
        camera = "booth%d" % i
        s.view(camera, (x0 + 0.2, 0.2, 1.8), (x0 + 2.0, 2.0, 0.6),
               [name + "_Zn", name + "_Xp", name + "_Yp"])
        entries.append(("default", camera, name + "_Zn"))
    s.base("default", "five booths lit alike")
    s.ordered(entries, 1.3, lights=["indirect"])
    s.ordered(entries, 1.5, lights=["total"])
    s.finish()


def hdr_pair(out):
    s = Scene(out, "range-hdr-pair", "range", "A tiny, very bright red lamp and a wide, dim "
              "blue panel: two hundred times apart in level, they still add exactly and keep "
              "to their own channels")
    s.samples = 2048
    s.mat("Grey", (0.5, 0.5, 0.5))
    s.room("R", (0.0, 0.0, 0.0), (6.0, 6.0, 3.0), "Grey")
    s.point("Red", (1.5, 3.0, 2.5), 0.02, 150000.0, RED)
    s.rect("Blue", (3.0, 3.0, 2.99), (3.0, 3.0), 0.05, BLUE)
    s.probe("ProbeSphere", (3.5, 3.5, 1.0))
    s.view("corner", (5.7, 0.3, 1.7), (1.0, 5.0, 0.8), ["R_Zn", "R_Xn", "R_Yp", "ProbeSphere"])
    s.base("both", "both on")
    s.state("red", "red lamp alone", s.off("Blue"))
    s.state("blue", "blue panel alone", s.off("Red"))
    s.superpose_all("both", ["red", "blue"])
    s.zero_all("red", [1, 2])
    s.zero_all("blue", [0, 1])
    s.zero_all("both", [1])
    s.finish()


def color_temperature(out):
    s = Scene(out, "range-color-temperature", "range", "Four sealed white booths lit at "
              "1900 K, 3200 K, 5600 K and 10000 K: grey walls take each blackbody color "
              "exactly, and red over blue falls as the temperature rises")
    s.mat("White", WHITE)
    entries = []
    for i, kelvin in enumerate((1900, 3200, 5600, 10000)):
        name = "K%d" % kelvin
        x0 = 2.2 * i
        s.room(name, (x0, 0.0, 0.0), (x0 + 2.0, 2.0, 2.5), "White")
        s.rect("Lamp" + name, (x0 + 1.0, 1.0, 2.49), (0.6, 0.6), 8.0, temperature=kelvin)
        camera = "k%d" % kelvin
        s.view(camera, (x0 + 0.2, 0.2, 1.8), (x0 + 2.0, 2.0, 0.6),
               [name + "_Zn", name + "_Xp", name + "_Yp"])
        s.chroma("default", camera, blackbody(kelvin),
                 name="%d K booth has the blackbody color" % kelvin)
        entries.append(("default", camera, name + "_Zn"))
    s.base("default", "four booths")
    s.tint_order(entries, 0, 2, 1.1)
    s.finish()


# ======================================================================
# emissive geometry
# ======================================================================

def emissive_orbs(out):
    s = Scene(out, "emissive-orbs", "emissive", "Two glowing orbs, magenta and cyan, are the "
              "only light: they add, a mirror swaps red and green, and with both dark the "
              "room is exactly black")
    symmetric_room(s)
    s.mat("GlowMagenta", (0.0, 0.0, 0.0), tuple(3.0 * c for c in MAGENTA))
    s.mat("GlowCyan", (0.0, 0.0, 0.0), tuple(3.0 * c for c in CYAN))
    s.sphere("OrbMagenta", (1.5, 3.5, 1.5), 0.3, "GlowMagenta", lon=32, lat=16)
    s.sphere("OrbCyan", (4.5, 3.5, 1.5), 0.3, "GlowCyan", lon=32, lat=16)
    s.probe("ProbeSphere", (3.0, 4.6, 1.0))
    centre_view(s, extra=["ProbeSphere"])
    s.base("both", "both orbs glowing")
    s.state("magenta", "magenta orb alone", s.dark("GlowCyan"))
    s.state("cyan", "cyan orb alone", s.dark("GlowMagenta"))
    s.state("off", "both orbs dark", s.dark("GlowMagenta", "GlowCyan"))
    s.superpose_all("both", ["magenta", "cyan"], lights=ALL_LIGHTS)
    s.zero_all("magenta", [1])
    s.zero_all("cyan", [0])
    s.zero_all("off", [0, 1, 2])
    s.mirror(("both", "centre", "R_Xn"), ("both", "centre", "R_Xp"), SWAP_RG)
    self_mirrors(s, "both", "centre", SWAP_RG, ["R_Yp", "R_Zn", "R_Zp", "ProbeSphere"])
    s.chroma_all("magenta", MAGENTA)
    s.finish()


# ======================================================================
# complex scenes
# ======================================================================

def stairwell(out):
    s = Scene(out, "complex-stairwell", "complex", "A two-level hall with stairs up to a "
              "mezzanine: a warm lamp above and a cool lamp below add, each keeps its color on "
              "the grey building, and the lower floor reads cooler than the mezzanine")
    s.mat("Grey", GREY)
    s.room("H", (0.0, 0.0, 0.0), (6.0, 8.0, 6.0), "Grey")
    s.box_faces("Mezz", (0.0, 4.0, 2.8), (6.0, 8.0, 3.0), "Grey", skip=("Xn", "Xp", "Yp"))
    steps = [((0.0, 0.5 + 0.3 * k, 0.0), (1.5, 0.8 + 0.3 * k, 0.28 * (k + 1)))
             for k in range(11)]
    s.boxes("Stairs", steps, "Grey", skip=("Zn", "Xn"))
    warm, cool = (1.0, 0.7, 0.4), (0.4, 0.6, 1.0)
    s.point("Warm", (4.0, 6.0, 5.0), 0.1, 400.0, warm)
    s.point("Cool", (4.0, 6.0, 1.5), 0.1, 300.0, cool)
    s.view("lower", (5.5, 0.3, 1.5), (2.0, 6.0, 1.2), ["H_Zn", "Mezz_Zn", "Stairs", "H_Xn"])
    s.view("upper", (5.5, 0.3, 5.0), (2.0, 6.0, 3.2), ["Mezz_Zp", "H_Zp", "H_Yp", "Mezz_Yn"])
    s.base("both", "both lamps on")
    s.state("warm", "upper lamp alone", s.off("Cool"))
    s.state("cool", "lower lamp alone", s.off("Warm"))
    s.superpose_all("both", ["warm", "cool"])
    s.chroma_all("warm", warm)
    s.chroma_all("cool", cool)
    s.tint_order([("both", "lower", "H_Zn"), ("both", "upper", "Mezz_Zp")], 2, 0, 1.5)
    s.finish()


def colonnade(out):
    s = Scene(out, "complex-colonnade", "complex", "A long hall of columns lit through three "
              "roof openings by a warm sun and a blue sky: sun and sky add, and each keeps its "
              "color throughout the grey interior")
    s.mat("Stone", GREY)
    size = (16.0, 8.0, 8.0)
    s.room("N", (0.0, 0.0, 0.0), size, "Stone", skip=("Zp",))
    holes = [((3.0 + 5.0 * k, 3.0), (5.0 + 5.0 * k, 5.0)) for k in range(3)]
    s.wall("Roof", 8.0, 2, -1, ((0.0, 0.0), (16.0, 8.0)), holes, "Stone")
    columns = [((x - 0.3, y - 0.3, 0.0), (x + 0.3, y + 0.3, 8.0))
               for x in (3.0, 6.0, 9.0, 12.0, 15.0) for y in (1.8, 6.2)]
    s.boxes("Columns", columns, "Stone", skip=("Zn", "Zp"))
    sun, sky = (1.0, 0.85, 0.6), (0.35, 0.5, 1.0)
    s.sun("Sun", 20.0, 60.0, 4.0, sun)
    s.sky("Sky", 0.6, sky)
    s.probe("ProbeSphere", (8.0, 4.0, 1.5))
    s.view("nave", (0.5, 4.0, 1.8), (16.0, 4.0, 3.0),
           ["N_Zn", "N_Xp", "N_Yn", "N_Yp", "Roof", "Columns", "ProbeSphere"])
    s.base("both", "sun and sky")
    s.state("sun", "sun alone", s.off("Sky"))
    s.state("sky", "sky alone", s.off("Sun"))
    s.superpose_all("both", ["sun", "sky"])
    s.chroma_all("sun", sun)
    s.chroma_all("sky", sky)
    s.finish()


def city_block(out):
    s = Scene(out, "complex-city-block", "complex", "Nine colored buildings at sunset: low "
              "orange sun and blue sky add along a street, and the street reads far redder "
              "under the sun than under the sky")
    s.mat("Asphalt", (0.4, 0.4, 0.4))
    s.panel("Ground", (-20.0, -20.0, 0.0), (20.0, 20.0, 0.0), 2, 1, "Asphalt")
    rng = random.Random(9)
    names = []
    for i, cx in enumerate((-6.0, 0.0, 6.0)):
        for j, cy in enumerate((-6.0, 0.0, 6.0)):
            name = "Bldg%d%d" % (i, j)
            albedo = hsv(40 * (3 * i + j), 0.5, 0.75)
            s.mat(name, albedo)
            height = round(rng.uniform(3.0, 10.0), 2)
            s.box(name, (cx - 2.0, cy - 2.0, 0.0), (cx + 2.0, cy + 2.0, height), name,
                  skip=("Zn",))
            names.append(name)
    sun, sky = (1.0, 0.6, 0.3), (0.3, 0.45, 0.9)
    s.sun("Sun", 200.0, 15.0, 3.0, sun)
    s.sky("Sky", 0.5, sky)
    s.view("street", (3.0, -9.0, 1.7), (3.0, 9.0, 1.0),
           ["Ground", "Bldg10", "Bldg11", "Bldg20", "Bldg21"])
    s.base("both", "sun and sky")
    s.state("sun", "sun alone", s.off("Sky"))
    s.state("sky", "sky alone", s.off("Sun"))
    s.superpose_all("both", ["sun", "sky"])
    s.tint_order([("sun", "street", "Ground"), ("sky", "street", "Ground")], 0, 2, 2.0)
    s.dominant("sky", "street", "Ground", 2, [0], 1.5)
    s.probe_bounds = (-10.0, -10.0, 0.25, 10.0, 10.0, 6.0)
    s.finish()


# ======================================================================
# second batch
# ======================================================================

def pinhole(out):
    s = Scene(out, "occlusion-pinhole", "occlusion", "A camera obscura: a red lamp up and to "
              "the left and a blue lamp down and to the right shine through a 20 cm pinhole, "
              "and each lands inverted in the opposite quadrant of the back wall")
    s.samples = 2048
    s.mat("Wall", GREY)
    s.room("R", (0.0, 0.0, 0.0), (4.0, 4.0, 3.0), "Wall", skip=("Yn", "Yp"))
    s.wall("Front", 0.0, 1, 1, ((0.0, 0.0), (4.0, 3.0)), [((1.9, 1.4), (2.1, 1.6))], "Wall")
    quadrants = {"BackUL": ((0.0, 1.5), (2.0, 3.0)), "BackUR": ((2.0, 1.5), (4.0, 3.0)),
                 "BackLL": ((0.0, 0.0), (2.0, 1.5)), "BackLR": ((2.0, 0.0), (4.0, 1.5))}
    for name, ((x0, z0), (x1, z1)) in quadrants.items():
        s.panel(name, (x0, 4.0, z0), (x1, 4.0, z1), 1, -1, "Wall")
    # Through (2, 0, 1.5): the red lamp lands near (3.3, 4, 0.6), the blue near (0.7, 4, 2.4).
    s.point("Red", (1.0, -3.0, 2.2), 0.05, 100000.0, RED)
    s.point("Blue", (3.0, -3.0, 0.8), 0.05, 100000.0, BLUE)
    s.view("inside", (2.0, 0.6, 1.5), (2.0, 4.0, 1.5), list(quadrants) + ["R_Zn", "R_Zp"])
    s.base("both", "both lamps outside the pinhole")
    s.state("red", "red lamp alone", s.off("Blue"))
    s.state("blue", "blue lamp alone", s.off("Red"))
    s.superpose_all("both", ["red", "blue"])
    s.zero("red", "inside", [0, 1, 2], ["BackUL", "BackUR", "BackLL"], ["direct"],
           name="red: direct light only in the lower right quadrant")
    s.zero("blue", "inside", [0, 1, 2], ["BackUR", "BackLL", "BackLR"], ["direct"],
           name="blue: direct light only in the upper left quadrant")
    s.dominant("both", "inside", "BackLR", 0, [2], 3.0, lights=["total"])
    s.dominant("both", "inside", "BackUL", 2, [0], 3.0, lights=["total"])
    s.zero_all("both", [1])
    s.finish()


def long_hall(out):
    s = Scene(out, "range-long-hall", "range", "A 24 m hall lit by one warm lamp at one end: "
              "floor light falls with distance segment by segment, and stays the lamp's color "
              "on the grey hall")
    s.mat("Grey", GREY)
    s.room("H", (0.0, 0.0, 0.0), (24.0, 3.0, 3.0), "Grey", skip=("Zn",))
    for i in range(8):
        s.panel("F%d" % i, (3.0 * i, 0.0, 0.0), (3.0 * i + 3.0, 3.0, 0.0), 2, 1, "Grey")
    warm = (1.0, 0.8, 0.6)
    s.point("Lamp", (1.0, 1.5, 2.5), 0.1, 800.0, warm)
    s.view("near", (9.0, 1.5, 2.7), (0.0, 1.5, 0.0), ["F0", "F1", "H_Yn", "H_Yp"])
    s.view("far", (21.0, 1.5, 2.7), (12.0, 1.5, 0.0), ["F4", "F5", "H_Yn", "H_Yp"])
    s.base("default", "one lamp at the west end")
    s.ordered([("default", "far", "F5"), ("default", "far", "F4"), ("default", "near", "F1"),
               ("default", "near", "F0")], 1.2)
    s.chroma_all("default", warm)
    s.finish()


def corner_seam(out):
    s = Scene(out, "leak-corner-seam", "leak", "Two sealed rooms that meet only at one corner "
              "edge, 4 units apart, lit red and green, with models by the shared corner: a "
              "probe cell spanning the corner must not carry either color across")
    s.mat("Wall", GREY)
    gap = gf.THIN_WALL_M
    b0 = 3.0 + gap
    s.room("A", (0.0, 0.0, 0.0), (3.0, 3.0, 3.0), "Wall")
    s.room("B", (b0, b0, 0.0), (b0 + 3.0, b0 + 3.0, 3.0), "Wall")
    s.rect("LampA", (1.5, 1.5, 2.99), (1.0, 1.0), 8.0, RED)
    s.rect("LampB", (b0 + 1.5, b0 + 1.5, 2.99), (1.0, 1.0), 8.0, GREEN)
    s.probe("ProbeA", (2.4, 2.4, 1.2))
    s.probe("ProbeB", (b0 + 0.6, b0 + 0.6, 1.2))
    s.view("a", (0.2, 0.2, 1.7), (2.4, 2.4, 0.9), ["A_Zn", "A_Xp", "A_Yp", "ProbeA"])
    s.view("b", (b0 + 2.8, b0 + 2.8, 1.7), (b0 + 0.6, b0 + 0.6, 0.9),
           ["B_Zn", "B_Xn", "B_Yn", "ProbeB"])
    s.base("default", "red room A, green room B")
    s.zero("default", "a", [1, 2])
    s.zero("default", "b", [0, 2])
    s.finish()


def colored_quadrants(out):
    s = Scene(out, "analytic-colored-quadrants", "analytic", "Four colored ground quadrants "
              "under a white sky: each quadrant's radiance is its albedo times the sky "
              "exactly, with no indirect light")
    colors = {"NE": (0.8, 0.3, 0.2), "NW": (0.2, 0.7, 0.3), "SW": (0.2, 0.3, 0.8),
              "SE": (0.8, 0.8, 0.2)}
    signs = {"NE": (1, 1), "NW": (-1, 1), "SW": (-1, -1), "SE": (1, -1)}
    probes = {"NE": "ProbeA", "NW": "ProbeB", "SW": "ProbeC", "SE": "ProbeD"}
    sky = 0.8
    half = 20.0
    for quadrant, albedo in colors.items():
        s.mat("Ground" + quadrant, albedo)
        sx, sy = signs[quadrant]
        s.panel("Ground" + quadrant, (min(0.0, sx * half), min(0.0, sy * half), 0.0),
                (max(0.0, sx * half), max(0.0, sy * half), 0.0), 2, 1, "Ground" + quadrant)
        s.probe(probes[quadrant], (2.0 * sx, 2.0 * sy, 0.8))
    s.sky("Sky", sky, (1.0, 1.0, 1.0))
    s.view("above", (0.0, -0.3, 6.0), (0.0, 0.0, 0.0),
           ["Ground" + q for q in colors] + list(probes.values()))
    s.base("default", "white sky alone")
    for quadrant, albedo in colors.items():
        s.value("default", "above", ["Ground" + quadrant],
                {"total": [a * sky for a in albedo], "direct": [sky] * 3},
                "%s quadrant radiance = albedo x sky" % quadrant)
    s.zero("default", "above", [0, 1, 2], ["Ground" + q for q in colors], ["indirect"],
           name="the ground receives no indirect light")
    # The models are regions for runtime comparison only: seen from above, a
    # model's upper half gathers ground light from every quadrant near the
    # horizon, so no per-model color relation holds (measured).
    s.probe_bounds = (-5.0, -5.0, 0.25, 5.0, 5.0, 2.75)
    s.finish()


def four_fold(out):
    s = Scene(out, "symmetry-four-fold", "symmetry", "A square room with a centred square "
              "lamp and four models at off-grid positions related by quarter turns: four "
              "quarter-turned cameras must see the same light on each model, floor and wall")
    s.mat("White", WHITE)
    s.room("R", (0.0, 0.0, 0.0), (6.0, 6.0, 3.0), "White")
    warm = (1.0, 0.8, 0.6)
    s.rect("Lamp", (3.0, 3.0, 2.99), (1.0, 1.0), 12.0, warm)

    def turn(point, k):
        x, y = point[0] - 3.0, point[1] - 3.0
        for _ in range(k):
            x, y = -y, x
        return (round(3.0 + x, 6), round(3.0 + y, 6)) + tuple(point[2:])
    walls = ["R_Xp", "R_Yp", "R_Xn", "R_Yn"]
    probes = ["ProbeA", "ProbeB", "ProbeC", "ProbeD"]
    probe0, eye0 = (4.3, 3.4, 1.0), (0.5, 1.2, 1.6)
    for k in range(4):
        s.probe(probes[k], turn(probe0, k))
    for k in range(4):
        s.view("turn%d" % k, turn(eye0, k), turn(probe0, k),
               {"model": [probes[k]], "floor": ["R_Zn"], "wall-a": [walls[k]],
                "wall-b": [walls[(k + 1) % 4]]})
    s.base("default", "centred lamp")
    for region in ("model", "floor", "wall-a", "wall-b"):
        s.uniform([("default", "turn%d" % k, region) for k in range(4)], lights=BOTH,
                  name="%s is the same under quarter turns" % region)
    s.chroma_all("default", warm)
    s.finish()


def stripe_floor(out):
    s = Scene(out, "emissive-stripe-floor", "emissive", "A floor of glowing red, green and "
              "blue stripes twice over: each color adds, stays in its own channel, and the "
              "mirror maps red stripes onto blue")
    # The stripes are the floor: a room floor under them would be coplanar and
    # z-fight (the mirror oracle caught an 11% direct-light asymmetry).
    s.mat("White", WHITE)
    s.room("R", (0.0, 0.0, 0.0), (6.0, 6.0, 3.0), "White", skip=("Zn",))
    colors = [("Red", RED), ("Green", GREEN), ("Blue", BLUE)]
    faces = {name: [] for name, _ in colors}
    for i in range(6):
        name = colors[i % 3][0]
        faces[name].append(gf.quad((float(i), 0.0, 0.0), (i + 1.0, 6.0, 0.0), 2, 1))
    for name, color in colors:
        s.mat("Glow" + name, (0.3, 0.3, 0.3), tuple(0.5 * c for c in color))
        s.mesh("Stripes" + name, faces[name], "Glow" + name)
    s.view("centre", (3.0, 0.2, 1.8), (3.0, 6.0, 1.2), ["R_Xn", "R_Xp", "R_Yp", "R_Zp"])
    s.base("all", "every stripe glowing")
    materials = ["GlowRed", "GlowGreen", "GlowBlue"]
    for name, _ in colors:
        s.state(name.lower(), name.lower() + " stripes alone",
                s.dark(*[m for m in materials if m != "Glow" + name]))
    s.superpose_all("all", ["red", "green", "blue"], lights=ALL_LIGHTS)
    for name, zero in (("red", [1, 2]), ("green", [0, 2]), ("blue", [0, 1])):
        s.zero_all(name, zero)
    s.mirror(("all", "centre", "R_Xn"), ("all", "centre", "R_Xp"), SWAP_RB)
    self_mirrors(s, "all", "centre", SWAP_RB, ["R_Yp", "R_Zp"])
    s.finish()


GALLERY = (mix_rgb_ceiling, mix_cmy_ceiling, mix_complementary_corridor, mix_rgb_venn,
           mix_spot_cross, mix_sun_sky_pillar, mix_disco, mix_hue_ring, mix_color_cube,
           mix_neon_strips, mix_rgb_probes, mix_light_swap, mix_moving_light, furnace_family,
           tinted_sky_plane, parallel_plates, integrating_sphere, cornell, cornell_mirror,
           blue_carpet, two_bounce_filter, checker_hall, hue_pillars, leak_rgb_rooms,
           leak_quad_rooms, leak_slit, leak_stacked, louver, l_corridor, colored_shadows,
           directional_probe, vertical_gradient, courtyard_sun, intensity_ladder,
           albedo_ladder, hdr_pair, color_temperature, emissive_orbs, stairwell, colonnade,
           city_block, pinhole, long_hall, corner_seam, colored_quadrants, four_fold,
           stripe_floor)


def generate(out):
    for build in GALLERY:
        build(out)
