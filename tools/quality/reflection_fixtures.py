#!/usr/bin/env python3
"""Generate the R50-PARALLAX reflection fixtures under quality/fixtures/reflection/.

Each fixture is an authored OpenUSD stage (Z-up meters) with mirror floors and
striped walls, a fixture record (`fixture.json`, the gi-fixture/v1 fields
`gi_reference.render_state` reads: stage, states, cameras, film, regions) and
the map manifest that builds it with automatically placed RPRB probes:

  mirror-room  one 6 x 4 x 3 m room with a mirror floor, walls and ceiling in
               0.5 m colour stripes, seen from cameras away from the probe.
               `reflection_runtime.py compare` scores the engine's mirror
               floor against Cycles renders of the same cameras; the
               direction-only mode and a deliberately wrong box must fail.
  two-rooms    two 5 x 4 x 3 m rooms, warm and cool stripes, joined by a
               1.2 m doorway, both with mirror floors. Placement must give
               each room its own probe; `reflection_runtime.py walk` walks a
               camera through the doorway and fails on a frame-to-frame jump
               in the floor's reflection, which the nearest-capture mode
               (Source 1's switch) must produce.
  mirror-lamp  R50-RELIGHT: the mirror room with a pillar and a bulb that
               the bake lacks (an unbaked, inverse-square light_dynamic in
               game). Its probes carry relight bands; `reflection_runtime.py
               relight` judges the mirror floor against Cycles renders with
               the bulb lit, which the relit probes must match and the
               probes as baked (`mat_reflection_relight 0`) must not.

Why (RFC/0007-progress.md, R50-PARALLAX): a direction-only probe is right
only at its capture point, and switching between probes pops (3kliksphilip,
"Advanced Reflections in CS:GO... and for Source 2?", 2019; Lagarde and
Zanuttini, SIGGRAPH 2012). Stripes make both errors visible in every
direction. Output is deterministic; rerun to regenerate:

    PYTHONPATH=build/toolchains/openusd-25.11/lib/python \\
        /usr/bin/python3.12 tools/quality/reflection_fixtures.py [--check]
"""

import argparse
import filecmp
import shutil
import sys
import tempfile
from pathlib import Path

from pxr import Gf, Sdf, UsdGeom, UsdShade

sys.path.insert(0, str(Path(__file__).resolve().parent))
import gi_fixtures  # noqa: E402
from gi_fixtures import Author, camera_pose, quad, rect_with_hole, write_json  # noqa: E402

ROOT = Path(__file__).resolve().parents[2]
DEFAULT_OUT = ROOT / "quality/fixtures/reflection"
STRIPE_M = 0.5
WARM = (("Red", (0.65, 0.12, 0.10)), ("Cream", (0.75, 0.70, 0.55)))
COOL = (("Blue", (0.10, 0.18, 0.65)), ("Green", (0.12, 0.55, 0.15)))
FOUR = WARM + COOL
MIRROR = (0.9, 0.9, 0.9)


def mirror_material(author, name):
    """A perfect mirror: metallic 1, roughness 0, reflectance 0.9."""
    material = author.material(name, MIRROR, roughness=0.0)
    shader = author.stage.GetPrimAtPath(material.GetPath().AppendChild("Surface"))
    shader.GetAttribute("inputs:metallic").Set(1.0)
    return material


def striped(author, prefix, lo, hi, axis, facing, along, palette):
    """A wall rectangle on plane `axis` split into STRIPE_M strips along axis
    `along`, cycling `palette` materials; returns the mesh names."""
    names = []
    start, end = lo[along], hi[along]
    count = max(1, int(round((end - start) / STRIPE_M)))
    for i in range(count):
        a = start + (end - start) * i / count
        b = start + (end - start) * (i + 1) / count
        strip_lo, strip_hi = list(lo), list(hi)
        strip_lo[along], strip_hi[along] = a, b
        name = "%s_%02d" % (prefix, i)
        author.mesh(name, [quad(strip_lo, strip_hi, axis, facing)], palette[i % len(palette)][0])
        names.append(name)
    return names


def striped_room(author, prefix, lo, hi, palette, skip=()):
    """Walls striped vertically, the ceiling striped along x, no floor."""
    names = []
    x0, y0, z0 = lo
    x1, y1, z1 = hi
    walls = {"Xn": ((x0, y0, z0), (x0, y1, z1), 0, 1, 1),
             "Xp": ((x1, y0, z0), (x1, y1, z1), 0, -1, 1),
             "Yn": ((x0, y0, z0), (x1, y0, z1), 1, 1, 0),
             "Yp": ((x0, y1, z0), (x1, y1, z1), 1, -1, 0)}
    for key, (a, b, axis, facing, along) in walls.items():
        if key not in skip:
            names += striped(author, "%s_%s" % (prefix, key), a, b, axis, facing, along, palette)
    names += striped(author, prefix + "_Ceiling", (x0, y0, z1), (x1, y1, z1), 2, -1, 0,
                     palette[::-1])
    return names


def palette_materials(author, palette):
    for name, albedo in palette:
        if name not in author.materials:
            author.material(name, albedo)


def fixture_record(name, purpose, stage, cameras, regions, extra):
    record = {"schema": "gi-fixture/v1", "name": name, "purpose": purpose, "stage": stage,
              "dynamic_models": [], "states": {"default": {"layer": None}},
              "baked_state": "default", "cameras": cameras, "film": gi_fixtures.FILM,
              "horizontal_fov_degrees": gi_fixtures.HORIZONTAL_FOV, "lambertian": False,
              "regions": regions, "map_manifest": "map.json"}
    record.update(extra)
    return record


def map_manifest(name, scene, solid_meshes=()):
    return {"schema": "pbrt-map-manifest/v1", "map": name, "scene": scene,
            "quality": "reflection-fixture", "collision": {"solid_meshes": list(solid_meshes)}}


def mirror_room(out):
    directory = out / "mirror-room"
    author = Author(directory / "mirror-room.usda", "MirrorRoom")
    palette_materials(author, FOUR)
    mirror_material(author, "Mirror")
    lo, hi = (0.0, 0.0, 0.0), (6.0, 4.0, 3.0)
    walls = striped_room(author, "Room", lo, hi, FOUR)
    author.mesh("Floor", [quad(lo, (6.0, 4.0, 0.0), 2, 1)], "Mirror")
    author.rect_light("Panel", (3.0, 2.0, 2.99), (1.5, 1.0), 12.0)
    # Cameras away from the room's probe (placement puts it near the room's
    # middle at eye height), looking down at the mirror floor.
    cameras = {"corner": camera_pose((0.5, 0.5, 1.6), (3.5, 2.8, 0.0)),
               "far-corner": camera_pose((5.5, 3.5, 1.4), (2.5, 1.0, 0.0)),
               "side": camera_pose((3.0, 0.4, 2.3), (3.0, 3.0, 0.2))}
    author.camera("Camera", cameras["corner"])
    author.save()
    regions = {camera: {"floor": ["Floor"], "walls": walls} for camera in cameras}
    write_json(directory / "fixture.json", fixture_record(
        "mirror-room", "Parallax oracle: the engine's mirror floor seen from cameras away "
        "from the probe against Cycles; direction-only sampling and a wrong box must fail",
        "mirror-room.usda", cameras, regions,
        {"room_m": [list(lo), list(hi)], "mirror_meshes": ["Floor"]}))
    write_json(directory / "map.json", map_manifest(
        "reflection_mirror_room", "quality/fixtures/reflection/mirror-room/mirror-room.usda"))


def two_rooms(out):
    directory = out / "two-rooms"
    author = Author(directory / "two-rooms.usda", "TwoRooms")
    palette_materials(author, FOUR)
    author.material("Plaster", (0.6, 0.6, 0.6))
    mirror_material(author, "Mirror")
    wall0, wall1 = 5.0, 5.2
    hole_y, hole_z = (1.4, 2.6), (0.0, 2.2)
    a_lo, a_hi = (0.0, 0.0, 0.0), (wall0, 4.0, 3.0)
    b_lo, b_hi = (wall1, 0.0, 0.0), (wall1 + 5.0, 4.0, 3.0)
    walls_a = striped_room(author, "A", a_lo, a_hi, WARM, skip=("Xp",))
    walls_b = striped_room(author, "B", b_lo, b_hi, COOL, skip=("Xn",))
    outer = ((0.0, 0.0), (4.0, 3.0))
    hole = ((hole_y[0], hole_z[0]), (hole_y[1], hole_z[1]))
    author.mesh("A_Xp", rect_with_hole(wall0, 0, -1, outer, hole), "Cream")
    author.mesh("B_Xn", rect_with_hole(wall1, 0, 1, outer, hole), "Blue")
    jambs = [quad((wall0, hole_y[0], hole_z[0]), (wall1, hole_y[0], hole_z[1]), 1, 1),
             quad((wall0, hole_y[1], hole_z[0]), (wall1, hole_y[1], hole_z[1]), 1, -1),
             quad((wall0, hole_y[0], hole_z[1]), (wall1, hole_y[1], hole_z[1]), 2, -1)]
    author.mesh("Doorway", jambs, "Plaster")
    author.mesh("FloorA", [quad(a_lo, (wall0, 4.0, 0.0), 2, 1)], "Mirror")
    author.mesh("FloorB", [quad(b_lo, (wall1 + 5.0, 4.0, 0.0), 2, 1)], "Mirror")
    author.mesh("DoorwayFloor", [quad((wall0, hole_y[0], 0.0), (wall1, hole_y[1], 0.0), 2, 1)],
                "Mirror")
    author.rect_light("PanelA", (2.5, 2.0, 2.99), (1.2, 1.0), 12.0)
    author.rect_light("PanelB", (wall1 + 2.5, 2.0, 2.99), (1.2, 1.0), 12.0)
    # The walk: along the rooms' axis through the doorway, looking ahead and
    # down at the floor. 25 stations from room A's middle to room B's.
    walk = []
    for i in range(25):
        x = 2.2 + (wall1 + 2.8 - 2.2) * i / 24
        walk.append(camera_pose((x, 2.0, 1.6), (x + 1.0, 2.0, 0.9)))
    cameras = {"walk%02d" % i: pose for i, pose in enumerate(walk)}
    author.camera("Camera", walk[0])
    author.save()
    write_json(directory / "fixture.json", fixture_record(
        "two-rooms", "Blend oracle: one probe per room, and a walk through the doorway "
        "whose mirror-floor reflection must not jump; the nearest-capture mode must",
        "two-rooms.usda", cameras, {}, {
            "walk": sorted(cameras), "rooms_m": {"A": [list(a_lo), list(a_hi)],
                                                  "B": [list(b_lo), list(b_hi)]},
            "mirror_meshes": ["FloorA", "FloorB", "DoorwayFloor"]}))
    manifest = map_manifest(
        "reflection_two_rooms", "quality/fixtures/reflection/two-rooms/two-rooms.usda",
        solid_meshes=["A_Xp", "B_Xn"])
    # One probe per room and no glossy probes: the walk reads each room's
    # probe weight from the weight view's two palette colours.
    manifest["reflection_probe"] = {"placement": {"glossy_roughness": -1.0}}
    write_json(directory / "map.json", manifest)


# mirror-lamp: the bulb (gi_fixtures' swing bulb: 2 Source units of radius and
# radiance 500, which the engine's inverse-square light gives with colour
# L (R / 100 units)^2), 1.1 m from the west wall, and a pillar between it and
# the north wall, whose shadow the relit probes must carry.
LAMP_BULB_M = (1.1, 2.0, 1.7)
LAMP_RADIUS_M = gi_fixtures.SWING_BULB_RADIUS_M
LAMP_RADIANCE = gi_fixtures.SWING_RADIANCE
LAMP_PILLAR_M = ((1.0, 2.9, 0.0), (1.4, 3.3, 2.4))


def mirror_lamp(out):
    directory = out / "mirror-lamp"
    author = Author(directory / "mirror-lamp.usda", "MirrorLamp")
    palette_materials(author, FOUR)
    author.material("Plaster", (0.6, 0.6, 0.6))
    author.material("Bulb", (0.0, 0.0, 0.0), (LAMP_RADIANCE,) * 3)
    mirror_material(author, "Mirror")
    lo, hi = (0.0, 0.0, 0.0), (6.0, 4.0, 3.0)
    walls = striped_room(author, "Room", lo, hi, FOUR)
    author.mesh("Floor", [quad(lo, (6.0, 4.0, 0.0), 2, 1)], "Mirror")
    author.solid("Pillar", *LAMP_PILLAR_M, "Plaster")
    # A dim baked panel, so the unbaked bulb dominates the change.
    author.rect_light("Panel", (4.0, 2.0, 2.99), (1.0, 1.0), 4.0)
    bulb = author.root_path.AppendPath("World/Bulb")
    xform = UsdGeom.Xform.Define(author.stage, bulb)
    xform.AddTranslateOp().Set(Gf.Vec3d(*LAMP_BULB_M))
    gi_fixtures.sphere_mesh(author.stage, bulb.AppendChild("BulbShape"), LAMP_RADIUS_M, 24, 12)
    UsdShade.MaterialBindingAPI.Apply(author.stage.GetPrimAtPath(
        bulb.AppendChild("BulbShape"))).Bind(author.materials["Bulb"])
    # Cameras away from the probes, looking down at the mirror floor's
    # reflection of the bulb-lit west and north walls and the pillar's shadow.
    cameras = {"east": camera_pose((5.5, 1.0, 1.6), (1.5, 2.5, 0.0)),
               "south": camera_pose((3.5, 0.3, 2.0), (2.0, 3.0, 0.0)),
               "far": camera_pose((5.6, 3.6, 1.5), (1.8, 1.5, 0.0))}
    author.camera("Camera", cameras["east"])
    author.save()
    base = directory / "mirror-lamp.usda"
    gi_fixtures.state_layer(directory / "states" / "baked.usda", base,
                            [(str(bulb), "visibility", None, UsdGeom.Tokens.invisible)])
    regions = {camera: {"floor": ["Floor"], "walls": walls + ["Pillar"]} for camera in cameras}
    record = fixture_record(
        "mirror-lamp", "Relight oracle: the mirror floor's reflection of walls lit by a bulb "
        "the bake lacks, against Cycles with the bulb lit; the probes as baked must fail",
        "mirror-lamp.usda", cameras, regions,
        {"room_m": [list(lo), list(hi)], "mirror_meshes": ["Floor"],
         "lamp": {"bulb_m": list(LAMP_BULB_M), "bulb_radius_m": LAMP_RADIUS_M,
                  "radiance": LAMP_RADIANCE,
                  "engine_color": LAMP_RADIANCE * (2.0 / 100.0) ** 2}})
    record["states"] = {"default": {"layer": None, "note": "the bulb lit (the references)"},
                        "baked": {"layer": "states/baked.usda",
                                  "note": "the bulb hidden: the map's bake"}}
    record["baked_state"] = "baked"
    write_json(directory / "fixture.json", record)
    manifest = map_manifest("reflection_mirror_lamp",
                            "quality/fixtures/reflection/mirror-lamp/states/baked.usda",
                            solid_meshes=["Pillar"])
    manifest["quality"] = "reflection-relight-fixture"
    manifest["collision"]["dynamic_lights"] = [{
        "name": "Bulb", "position_m": list(LAMP_BULB_M),
        "color_linear": LAMP_RADIANCE * (2.0 / 100.0) ** 2}]
    write_json(directory / "map.json", manifest)


def generate(out):
    mirror_room(out)
    two_rooms(out)
    mirror_lamp(out)
    write_json(out / "index.json", {"fixtures": ["mirror-room", "two-rooms", "mirror-lamp"]})


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--out", type=Path, default=DEFAULT_OUT)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    if not args.check:
        generate(args.out)
        return
    with tempfile.TemporaryDirectory() as name:
        fresh = Path(name) / "reflection"
        generate(fresh)
        differences = []
        for path in sorted(fresh.rglob("*")):
            if path.is_file():
                relative = path.relative_to(fresh)
                stored = args.out / relative
                if not stored.is_file() or not filecmp.cmp(path, stored, shallow=False):
                    differences.append(str(relative))
        if differences:
            raise SystemExit("fixtures differ from the generator: " + ", ".join(differences))
    print("reflection fixtures match the generator")


if __name__ == "__main__":
    main()
