#!/usr/bin/env python3
"""Build the F-Stop mechanics test map, fstop_mechanics.

One hall with five bays, split by half-length divider walls, that places the
F-Stop entities ported into the fstop product (game/server/fstop, FSTOP-guarded
base code) with their DATADESC keyfields and inputs:

* A, camera and photos (spawn here): weapon_camera (3 capture slots, scaling
  and zoom on) and weapon_placement pickups; capturable props at three
  scale values (canbecaptured, scalevalue); info_placement_helper targets
  (one forced and size-limited); a trigger_photo_eraser; an env_dof_controller.
* B, F-Stop props: prop_levitator, prop_geyser, prop_monopole (active,
  positive) above a func_monopole_field, prop_air_vent, prop_mousetrap (its
  trigger_callback volumes are created by the prop), prop_reflect, prop_swap,
  prop_tombstone, prop_building (targets the resized portal) and
  prop_android_dispenser.
* C, portals: a prop_portal_linked_door pair; a prop_portal pair spawned
  resized through HalfWidth/HalfHeight and resized again by the Resize input
  (a button toggles between two sizes); a prop_portal_tunnel with its
  success and fail targets.
* D, NPCs on an info_node grid: npc_android with ai_addon_shield and
  npc_android_basic with ai_addon_minigun (the CreateAddon input at map
  start), npc_chicken with a nest (info_hint of HINT_PORTAL2_NEST, 1200),
  npc_hover_turret; a filter_enemy with filter_object_size as the androids'
  enemy filter, and a filter_size trigger that lights a lamp when a
  scale-1 object enters.
* E, blobs on the same node grid: npc_blob_fountain and npc_blob_demomonster
  (HULL_TINY_FLUID). sv_blob_lennard_jones 1 turns on their cohesion.

    python3 tools/quality/fstop_mechanics_map.py          build and install
    ./play_fstop +map fstop_mechanics

The map compiles with the pinned legacy vbsp/vvis/vrad of the PBRT map
toolchain against the staged Portal runtime (tools/quality/vmf_map_build.py)
and is installed into the F-Stop runtime's game directory
(run/runtime-fstop/fstop/maps), which the Portal map store is not mounted in.
"""

import argparse
import json
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import vmf_map_build  # noqa: E402
from gyro_lab_map import Vmf, vec, BUTTON, PEDESTAL, TRIGGER  # noqa: E402

ROOT = HERE.parents[1]
TOOLCHAIN = ROOT / "build/toolchains/pbrt-map-toolchain.json"
GAME_DIR = ROOT / "run/runtime-fstop/fstop"
NAME = "fstop_mechanics"

WALL = "DEV/GRAYGRID"
FLOOR = "DEV/DEV_MEASUREGENERIC01B"
CEILING = "DEV/GRAYGRID"
DIVIDER = "DEV/DEV_MEASUREWALL01A"
FIELD = "TOOLS/TOOLSTRIGGER"

HALL = (-1280, -768, 0, 1280, 768, 384)
THICK = 16
DIVIDERS_X = (-768, -256, 256, 768)   # bays A | B | C | D | E
DIVIDER_HEIGHT = 128

# Models the staged content has (Portal and HL2 packs, the F-Stop overlay).
CUBE = "models/props/metal_box.mdl"
MELON = "models/props_junk/watermelon01.mdl"
DRUM = "models/props_c17/oildrum001.mdl"
RADIO = "models/props/radio_reference.mdl"


def box(vmf, lo, hi, material):
    center = tuple((a + b) / 2.0 for a, b in zip(lo, hi))
    half = tuple((b - a) / 2.0 for a, b in zip(lo, hi))
    return vmf.box(center, half, material)


def hall(vmf):
    x0, y0, z0, x1, y1, z1 = HALL
    t = THICK
    vmf.world += [
        box(vmf, (x0 - t, y0 - t, z0 - t), (x1 + t, y1 + t, z0), FLOOR),
        box(vmf, (x0 - t, y0 - t, z1), (x1 + t, y1 + t, z1 + t), CEILING),
        box(vmf, (x0 - t, y0 - t, z0), (x0, y1 + t, z1), WALL),
        box(vmf, (x1, y0 - t, z0), (x1 + t, y1 + t, z1), WALL),
        box(vmf, (x0, y0 - t, z0), (x1, y0, z1), WALL),
        box(vmf, (x0, y1, z0), (x1, y1 + t, z1), WALL),
    ]
    # Divider walls along y with a 512-wide gap in the middle of each.
    for x in DIVIDERS_X:
        vmf.world += [box(vmf, (x - 8, y0, z0), (x + 8, -256, DIVIDER_HEIGHT), DIVIDER),
                      box(vmf, (x - 8, 256, z0), (x + 8, y1, DIVIDER_HEIGHT), DIVIDER)]
    for x in (-1024, -512, 0, 512, 1024):
        for y in (-384, 384):
            vmf.entity("light", {"origin": vec((x, y, 320)), "_light": "255 250 240 300",
                                 "_quadratic_attn": "0", "_linear_attn": "1",
                                 "_constant_attn": "0"})


def prop(vmf, classname, origin, keyvalues=None, angles=(0, 0, 0)):
    kv = {"origin": vec(origin), "angles": vec(angles)}
    kv.update(keyvalues or {})
    vmf.entity(classname, kv)
    if (classname.startswith(("npc_", "prop_", "weapon_")) and
            kv.get("targetname")):
        x, y, z = origin
        vmf.entity("point_fstop_label",
                   {"origin": vec((x, y, z)),
                    "label_target": kv["targetname"],
                    "offset": "0 0 72",
                     "label": "%s [%s]" % (kv["targetname"], classname)})


def button(vmf, name, center, outputs):
    x, y, z = center
    vmf.world.append(vmf.box((x, y, (z - 8) / 2), (12, 12, (z - 8) / 2), PEDESTAL))
    vmf.entity("func_button", {"targetname": name, "origin": vec(center), "spawnflags": "1025",
                               "speed": "5", "wait": "1", "lip": "0", "sounds": "0",
                               "movedir": "0 0 0", "renderamt": "255"},
               solids=[vmf.box(center, (8, 8, 8), BUTTON)], outputs=outputs)


def bay_camera(vmf):
    """A: x -1280..-768. The player spawns here."""
    vmf.entity("info_player_start", {"origin": "-1150 0 8", "angles": "0 0 0"})
    prop(vmf, "weapon_camera", (-1100, -120, 16), {"targetname": "camera", "captureslots": "3",
                                                  "canscale": "1", "canzoom": "1"})
    prop(vmf, "weapon_placement", (-1100, 120, 16), {"targetname": "placement"})
    # Capturable physics props at three object scale levels.
    for i, (model, scale) in enumerate(((CUBE, "0"), (MELON, "1"), (DRUM, "-1"))):
        prop(vmf, "prop_physics", (-950, -300 + 150 * i, 24),
             {"targetname": "capturable_%d" % i, "model": model, "canbecaptured": "1",
              "scalevalue": scale, "spawnflags": "256"})
    prop(vmf, "prop_physics", (-950, 250, 24), {"targetname": "not_capturable", "model": RADIO,
                                                "canbecaptured": "0", "spawnflags": "256"})
    # Placement helpers: a free one and a forced, size-limited one for cubes.
    prop(vmf, "info_placement_helper", (-850, -500, 8),
         {"targetname": "helper_free", "radius": "48", "StartDisabled": "0"})
    prop(vmf, "info_placement_helper", (-850, 500, 8),
         {"targetname": "helper_cube", "radius": "64", "force_placement": "1",
          "snap_to_helper_angles": "1", "usesizelimit": "1", "target_size": "0",
          "target_classname": "prop_physics", "StartDisabled": "0"})
    vmf.entity("trigger_photo_eraser", {"targetname": "photo_eraser", "spawnflags": "9",
                                        "StartDisabled": "0"},
               solids=[box(vmf, (-1260, -760, 0), (-1200, -560, 128), TRIGGER)])
    prop(vmf, "env_dof_controller", (-1150, 0, 64),
         {"targetname": "dof", "enabled": "1", "near_blur": "20", "near_focus": "60",
          "far_focus": "300", "far_blur": "900", "near_radius": "0", "far_radius": "5"})


def bay_props(vmf):
    """B: x -768..-256."""
    prop(vmf, "prop_levitator", (-680, -600, 16), {"targetname": "levitator"})
    prop(vmf, "prop_geyser", (-560, -600, 0), {"targetname": "geyser"})
    prop(vmf, "prop_air_vent", (-440, -600, 0), {"targetname": "air_vent"})
    prop(vmf, "prop_mousetrap", (-340, -600, 0), {"targetname": "mousetrap"})
    prop(vmf, "prop_monopole", (-600, 0, 64),
         {"targetname": "monopole", "StartActive": "1", "StartPositive": "1",
          "maxobjects": "3", "massScale": "1", "forcelimit": "0", "torquelimit": "0"})
    vmf.entity("func_monopole_field", {"targetname": "monopole_field", "StartActive": "1",
                                       "StartPositive": "0", "HitboxPadding": "0 0 0"},
               solids=[box(vmf, (-720, -96, 0), (-560, 96, 16), FIELD)])
    prop(vmf, "prop_physics", (-600, 120, 24), {"targetname": "monopole_cube", "model": CUBE,
                                                "canbecaptured": "1", "scalevalue": "0"})
    prop(vmf, "prop_reflect", (-440, 0, 24), {"targetname": "reflect"})
    prop(vmf, "prop_swap", (-340, 0, 24), {"targetname": "swap"})
    prop(vmf, "prop_tombstone", (-680, 500, 0), {"targetname": "tombstone"})
    prop(vmf, "prop_building", (-500, 500, 0), {"targetname": "building",
                                                "target_portal": "portal_resize_a"})
    prop(vmf, "prop_android_dispenser", (-340, 520, 0), {"targetname": "dispenser",
                                                         "StartDisabled": "1"})


def bay_portals(vmf):
    """C: x -256..256. Resized pair on the back wall (y = 768), facing -y."""
    y = HALL[4] - 1
    prop(vmf, "prop_portal", (-128, y, 96),
         {"targetname": "portal_resize_a", "LinkageGroupID": "2", "Activated": "1",
          "PortalTwo": "0", "HalfWidth": "48", "HalfHeight": "72"}, angles=(0, 270, 0))
    prop(vmf, "prop_portal", (128, y, 96),
         {"targetname": "portal_resize_b", "LinkageGroupID": "2", "Activated": "1",
          "PortalTwo": "1", "HalfWidth": "48", "HalfHeight": "72"}, angles=(0, 270, 0))
    # Toggle between the spawned size and a small one.
    button(vmf, "resize_small", (-64, 560, 40),
           [("OnPressed", "portal_resize_a", "Resize", "24 36", 0),
            ("OnPressed", "portal_resize_b", "Resize", "24 36", 0)])
    button(vmf, "resize_large", (64, 560, 40),
           [("OnPressed", "portal_resize_a", "Resize", "48 72", 0),
            ("OnPressed", "portal_resize_b", "Resize", "48 72", 0)])
    # A linked door pair facing each other across the bay.
    prop(vmf, "prop_portal_linked_door", (-180, -480, 0),
         {"targetname": "door_a", "partnername": "door_b", "IsPortal2": "0"}, angles=(0, 0, 0))
    prop(vmf, "prop_portal_linked_door", (180, -480, 0),
         {"targetname": "door_b", "partnername": "door_a", "IsPortal2": "1"}, angles=(0, 180, 0))
    vmf.entity("logic_auto", {"origin": "0 -600 16", "spawnflags": "1"},
               outputs=[("OnMapSpawn", "door_a", "Open", "", 1.0),
                        ("OnMapSpawn", "door_b", "Open", "", 1.0)])
    prop(vmf, "prop_portal_tunnel", (0, 0, 0),
         {"targetname": "tunnel", "successtarget": "tunnel_success",
          "failtarget": "tunnel_fail", "failtarget_floor": "tunnel_fail_floor",
          "failtarget_ceiling": "tunnel_fail_ceiling"}, angles=(0, 90, 0))
    for name, origin in (("tunnel_success", (0, 200, 16)), ("tunnel_fail", (0, -200, 16)),
                         ("tunnel_fail_floor", (-80, -200, 16)),
                         ("tunnel_fail_ceiling", (80, -200, 16))):
        prop(vmf, "info_target", origin, {"targetname": name})


def node_grid(vmf, x0, x1):
    for x in range(x0 + 64, x1, 128):
        for y in range(-704, 705, 128):
            if abs(y) < 64 and x in DIVIDERS_X:
                continue
            prop(vmf, "info_node", (x, y, 8))


def bay_npcs(vmf):
    """D: x 256..768."""
    node_grid(vmf, 256, 768)
    vmf.entity("filter_enemy", {"targetname": "filter_small_enemies", "filtername": "!player",
                                "filter_object_size": "0", "origin": "300 -700 16"})
    prop(vmf, "npc_android", (400, -400, 8),
         {"targetname": "android_shield", "enemyfilter": "filter_small_enemies"},
         angles=(0, 180, 0))
    prop(vmf, "npc_android_basic", (400, -150, 8),
         {"targetname": "android_minigun", "enemyfilter": "filter_small_enemies"},
         angles=(0, 180, 0))
    vmf.entity("logic_auto", {"origin": "400 -600 16", "spawnflags": "1"},
               outputs=[("OnMapSpawn", "android_shield", "CreateAddon", "ai_addon_shield", 1.0),
                        ("OnMapSpawn", "android_minigun", "CreateAddon", "ai_addon_minigun", 1.0)])
    prop(vmf, "npc_chicken", (600, 250, 8), {"targetname": "chicken"})
    prop(vmf, "info_hint", (700, 450, 8), {"targetname": "chicken_nest", "hinttype": "1200",
                                           "nodeFOV": "360", "StartHintDisabled": "0",
                                           "spawnflags": "0"})
    prop(vmf, "npc_hover_turret", (600, -400, 64), {"targetname": "hover_turret",
                                                    "ignoreclipbrushes": "0"},
         angles=(0, 180, 0))
    # A lamp that lights when a scale-1 object (the melon) enters its pad.
    vmf.entity("filter_size", {"targetname": "filter_size_1", "filtersize": "1",
                               "origin": "300 600 16"})
    prop(vmf, "light", (330, 650, 96), {"targetname": "size_lamp", "spawnflags": "1",
                                        "_light": "80 255 80 200", "_constant_attn": "0",
                                        "_linear_attn": "1", "_quadratic_attn": "0"})
    vmf.entity("trigger_multiple", {"targetname": "size_pad", "spawnflags": "8",
                                    "filtername": "filter_size_1", "wait": "1",
                                    "StartDisabled": "0"},
               solids=[box(vmf, (280, 560, 0), (380, 740, 64), TRIGGER)],
               outputs=[("OnStartTouch", "size_lamp", "TurnOn", "", 0),
                        ("OnEndTouchAll", "size_lamp", "TurnOff", "", 0)])


def bay_blobs(vmf):
    """E: x 768..1280."""
    node_grid(vmf, 768, 1280)
    prop(vmf, "npc_blob_fountain", (1000, -300, 8),
         {"targetname": "blob_fountain", "particlecount": "120", "particle_radius": "4"})
    prop(vmf, "npc_blob_demomonster", (1000, 300, 8),
         {"targetname": "blob_monster", "particlecount": "80", "particle_radius": "5"})


def vmf_text():
    vmf = Vmf()
    hall(vmf)
    bay_camera(vmf)
    bay_props(vmf)
    bay_portals(vmf)
    bay_npcs(vmf)
    bay_blobs(vmf)
    return vmf.text()


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--out", type=Path, default=ROOT / "quality-results/fstop-maps" / NAME)
    parser.add_argument("--toolchain", type=Path, default=TOOLCHAIN)
    parser.add_argument("--runtime", type=Path, default=ROOT / "run/runtime",
                        help="staged Portal runtime the map's materials come from")
    parser.add_argument("--game-dir", type=Path, default=GAME_DIR,
                        help="staged F-Stop game directory the map is installed into")
    parser.add_argument("--quality", choices=("fast", "full"), default="full")
    parser.add_argument("--vmf-only", action="store_true", help="write the VMF and stop")
    args = parser.parse_args()

    out = args.out.resolve()
    (out / "compile").mkdir(parents=True, exist_ok=True)
    vmf = out / "compile" / (NAME + ".vmf")
    vmf.write_text(vmf_text())
    print("wrote " + str(vmf))
    if args.vmf_only:
        return 0
    tools = Path(json.loads(args.toolchain.read_text())["compile_tools"])
    record = vmf_map_build.build(vmf, out, tools, args.runtime.resolve(), args.quality, NAME)
    print("vmf_map_build: %s %s (%s)" % (NAME, record["status"], out / "build.json"))
    if record["status"] != "pass":
        return 1
    print("installed " + str(vmf_map_build.install(record, args.game_dir.resolve())))
    return 0


if __name__ == "__main__":
    sys.exit(main())
