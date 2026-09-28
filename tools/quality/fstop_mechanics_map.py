#!/usr/bin/env python3
"""Build the F-Stop mechanics test map, fstop_mechanics.

One hall with five bays, split by half-length divider walls, that places the
F-Stop entities ported into the fstop product (game/server/fstop, FSTOP-guarded
base code) with their DATADESC keyfields and inputs, set up so each one does
what its code does. tools/quality/fstop_mechanics_check.py boots the map
headless and checks that behavior; every output it judges is wired to a
probe.<entity>.<output> logic_relay.

* A, camera and photos (spawn here): weapon_camera (one photo, which is all
  the inventory holds; scaling and zoom on) and weapon_placement pickups;
  capturable props at three scale values (canbecaptured, scalevalue): the
  objects of Valve's F-Stop camera training photos (materials/photos:
  barrel, crate, fan, tire) and an F-Stop instruction manual scrap (the
  photos' stairs, gnome and doorway have no complete physics model in the
  staged content); an item_photo holding a photo of a crate;
  info_placement_helper targets (one forced and size-limited); a
  func_placement_clip wall; a trigger_photo_eraser; an env_dof_controller.
* B, F-Stop props: prop_levitator (the balloon), prop_geyser (pitched up; it
  pushes along its forward axis), prop_monopole above a func_monopole_field,
  prop_air_vent, prop_mousetrap (its trigger_callback volumes are created by
  the prop), prop_reflect and prop_swap (photographing them lifts the player
  or trades places), prop_tombstone over a dirt patch (it raises zombies only
  from dirt) and prop_android_dispenser (dispensing). Valve never shipped the
  tombstone model (models/props_fstop/tombstone001.mdl; the depots carry only
  its materials), so it draws as the error model; prop_building is left out,
  because without its dollhouse model (dollhouse04.mdl) it has no door
  attachment to open its portal at.
* C, portals: a prop_portal_linked_door pair; a prop_portal pair spawned
  resized through HalfWidth/HalfHeight and resized again by the Resize input
  (a button toggles between two sizes); a prop_portal_tunnel with its
  success and fail targets; an env_portal_laser burning a breakable crate;
  a talking prop_personality_sphere (CoreType 4, its sphere02 lines).
* D, NPCs on an info_node grid: npc_android (neutral), two npc_android_basic
  with the add-ons that attach to them (ai_addon_shield and ai_addon_saw
  through CreateAddon at map start; the add-on models never shipped, so the
  add-ons work unseen), npc_chicken with its nest (info_hint of
  HINT_PORTAL2_NEST, 1200, at Valve's nest model and eggs), npc_hover_turret;
  a filter_enemy with filter_object_size as the androids' enemy filter, and a
  filter_size trigger that lights a lamp when a scale-1 object enters.
* E, blobs and bots on the same node grid: npc_blob_fountain and
  npc_blob_demomonster (HULL_TINY_FLUID; sv_blob_lennard_jones 1 turns on
  their cohesion), and the companion bots npc_medicbot and npc_obot, which
  speak through the response rules library (RESPONSE_RULES_LIBRARY) and
  keep to the node grid behind the blobs.

Models come from the staged content: Portal and HL2, then Valve's F-Stop-era
content (Steam2 depot 852 version 0; tools/quality/stage_fstop_runtime.py).

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
DIRT = "NATURE/DIRTFLOOR003A"

HALL = (-1280, -768, 0, 1280, 768, 384)
THICK = 16
DIVIDERS_X = (-768, -256, 256, 768)   # bays A | B | C | D | E
DIVIDER_HEIGHT = 128

# Models the staged content has: the Portal and HL2 packs, then Valve's depot 852.
CUBE = "models/props/metal_box.mdl"
RADIO = "models/props/radio_reference.mdl"
# The objects of Valve's camera training photos (materials/photos/*.vtf), each
# with a collision model: F-Stop's own (depot 852) where one shipped, else HL2's.
BARREL = "models/props_lab/barrel.mdl"
CRATE = "models/props_junk/wood_crate001a.mdl"
FAN = "models/props_gameplay/fan.mdl"
TIRE = "models/props_vehicles/tire001c_car.mdl"
MANUAL_SCRAP = "models/props_fstop/instruction_manual_scrap01.mdl"
NEST = "models/props_farm/chicken_nest.mdl"
EGG = "models/props_farm/egg.mdl"
# Models without prop data: a plain prop_physics deletes itself ("must be used
# on a prop_static"), so these are placed as prop_physics_override.
NO_PROPDATA = {BARREL, FAN, MANUAL_SCRAP, RADIO}


def physics_class(model):
    return "prop_physics_override" if model in NO_PROPDATA else "prop_physics"


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


# Every output the map wires for tools/quality/fstop_mechanics_check.py goes to its
# own logic_relay, probe.<entity>.<output>; developer 2 logs each firing.
PROBES = []


def probed(entity, *names):
    """Outputs of entity wired to their probe relays."""
    PROBES.extend("probe.%s.%s" % (entity, name) for name in names)
    return [(name, "probe.%s.%s" % (entity, name), "Trigger", "", 0) for name in names]


def probe_relays(vmf):
    for i, name in enumerate(sorted(set(PROBES))):
        vmf.entity("logic_relay", {"targetname": name, "origin": vec((-1270, -760 + 8 * i, 8))})
    # The checker schedules console commands in game time through these
    # (ent_fire check_server Command "<command>" <delay>).
    vmf.entity("point_servercommand", {"targetname": "check_server", "origin": "-1270 760 8"})
    vmf.entity("point_clientcommand", {"targetname": "check_client", "origin": "-1262 760 8"})


def prop(vmf, classname, origin, keyvalues=None, angles=(0, 0, 0), outputs=()):
    kv = {"origin": vec(origin), "angles": vec(angles)}
    kv.update(keyvalues or {})
    vmf.entity(classname, kv, outputs=list(outputs))
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
    prop(vmf, "weapon_camera", (-1100, -120, 16), {"targetname": "camera", "captureslots": "1",
                                                  "canscale": "1", "canzoom": "1"})
    prop(vmf, "weapon_placement", (-1100, 120, 16), {"targetname": "placement"})
    # Capturable physics props at the three object scale levels.
    capturables = ((BARREL, "0"), (CRATE, "1"), (TIRE, "-1"), (FAN, "0"), (MANUAL_SCRAP, "-1"))
    for i, (model, scale) in enumerate(capturables):
        prop(vmf, physics_class(model), (-950 + 120 * (i % 2), -450 + 150 * (i // 2), 24),
             {"targetname": "capturable_%d" % i, "model": model, "canbecaptured": "1",
              "scalevalue": scale, "spawnflags": "256"},
             outputs=probed("capturable_%d" % i, "OnCameraCapture", "OnCameraRelease",
                            "OnFizzled"))
    prop(vmf, physics_class(RADIO), (-950, 250, 24), {"targetname": "not_capturable", "model": RADIO,
                                                "canbecaptured": "0", "spawnflags": "256"})
    # Placement helpers: a free one and a forced, size-limited one for cubes.
    prop(vmf, "info_placement_helper", (-850, -500, 8),
         {"targetname": "helper_free", "radius": "48", "StartDisabled": "0"},
         outputs=probed("helper_free", "OnObjectPlaced"))
    prop(vmf, "info_placement_helper", (-850, 500, 8),
         {"targetname": "helper_cube", "radius": "64", "force_placement": "1",
          "snap_to_helper_angles": "1", "usesizelimit": "1", "target_size": "0",
          "target_classname": "prop_physics", "StartDisabled": "0"},
         outputs=probed("helper_cube", "OnObjectPlaced", "OnObjectPlacedSize"))
    # A photo lying on the floor: picking it up (+use) photographs its target.
    prop(vmf, physics_class(CRATE), (-1000, 650, 24),
         {"targetname": "photo_crate", "model": CRATE, "canbecaptured": "1", "scalevalue": "0",
          "spawnflags": "256"},
         outputs=probed("photo_crate", "OnCameraCapture", "OnCameraRelease"))
    prop(vmf, "item_photo", (-1150, 650, 16), {"targetname": "photo", "target_entity": "photo_crate"},
         outputs=probed("photo", "OnPickedUp"))
    # A wall only the placement trace sees (collision group PLACEMENT_SOLID).
    vmf.entity("func_placement_clip", {"targetname": "placement_clip", "StartDisabled": "0"},
               solids=[box(vmf, (-880, 560, 0), (-860, 760, 128), FIELD)])
    vmf.entity("trigger_photo_eraser", {"targetname": "photo_eraser", "spawnflags": "9",
                                        "StartDisabled": "0"},
               solids=[box(vmf, (-1260, -760, 0), (-1200, -560, 128), TRIGGER)],
               outputs=probed("photo_eraser", "OnObjectsFizzled"))
    prop(vmf, "env_dof_controller", (-1150, 0, 64),
         {"targetname": "dof", "enabled": "1", "near_blur": "20", "near_focus": "60",
          "far_focus": "300", "far_blur": "900", "near_radius": "0", "far_radius": "5"})


def bay_props(vmf):
    """B: x -768..-256."""
    prop(vmf, "prop_levitator", (-680, -600, 16), {"targetname": "levitator"})
    # The balloon floats up; this physics-object trigger under the ceiling sees it arrive.
    vmf.entity("filter_activator_name", {"targetname": "filter_levitator",
                                         "filtername": "levitator", "origin": "-680 -600 300"})
    vmf.entity("trigger_multiple", {"targetname": "levitator_high", "spawnflags": "8",
                                    "filtername": "filter_levitator", "wait": "1",
                                    "StartDisabled": "0"},
               solids=[box(vmf, (-760, -700, 260), (-600, -500, 380), TRIGGER)],
               outputs=probed("levitator_high", "OnStartTouch"))
    prop(vmf, "prop_geyser", (-560, -600, 0), {"targetname": "geyser"}, angles=(-90, 0, 0))
    prop(vmf, "prop_air_vent", (-440, -600, 0), {"targetname": "air_vent"})
    prop(vmf, "prop_mousetrap", (-340, -600, 0), {"targetname": "mousetrap"})
    prop(vmf, "prop_monopole", (-600, 0, 64),
         {"targetname": "monopole", "StartActive": "1", "StartPositive": "1",
          "maxobjects": "3", "massScale": "1", "forcelimit": "0", "torquelimit": "0"},
         outputs=probed("monopole", "OnAttach", "OnDetach"))
    vmf.entity("func_monopole_field", {"targetname": "monopole_field", "StartActive": "1",
                                       "StartPositive": "0", "HitboxPadding": "0 0 0"},
               solids=[box(vmf, (-720, -96, 0), (-560, 96, 16), FIELD)],
               )
    prop(vmf, "prop_physics", (-600, 120, 24), {"targetname": "monopole_cube", "model": CUBE,
                                                "canbecaptured": "1", "scalevalue": "0"})
    # Photographing these is refused (TestPreCapture) and does their trick instead:
    # the reflect cube lifts the player 64 units above it, the swap cube trades
    # places with the player. Like any capturable, they need canbecaptured.
    prop(vmf, "prop_reflect", (-440, 0, 24), {"targetname": "reflect", "canbecaptured": "1"})
    prop(vmf, "prop_swap", (-340, 0, 24), {"targetname": "swap", "canbecaptured": "1"})
    # The tombstone raises zombies only over dirt (TestValidGround: game material 'D').
    vmf.world.append(box(vmf, (-760, 420, 0), (-600, 580, 2), DIRT))
    prop(vmf, "prop_tombstone", (-680, 500, 4), {"targetname": "tombstone"})
    # Dispenses one npc_android_basic at a time; the next rises 2 s after it dies.
    prop(vmf, "prop_android_dispenser", (-340, 520, 0), {"targetname": "dispenser",
                                                         "StartDisabled": "0"},
         outputs=probed("dispenser", "OnSpawnNPC", "OnChildKilled"))


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
    # The laser burns what it hits: a breakable crate in its path.
    prop(vmf, "env_portal_laser", (-220, -680, 40), {"targetname": "laser"})
    prop(vmf, "prop_physics", (180, -680, 24),
         {"targetname": "laser_target", "model": CRATE, "canbecaptured": "0"},
         outputs=probed("laser_target", "OnBreak"))
    prop(vmf, "prop_personality_sphere", (0, -300, 16),
         {"targetname": "sphere", "CoreType": "4", "DelayBetweenLines": "2"})
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
    # npc_android is neutral to the player; npc_android_basic hunts them. Add-ons
    # attach only to the front/rear ("eyes") of npc_android_basic.
    prop(vmf, "npc_android", (400, -650, 8), {"targetname": "android"}, angles=(0, 180, 0))
    prop(vmf, "npc_android_basic", (400, -400, 8),
         {"targetname": "android_shield", "enemyfilter": "filter_small_enemies"},
         angles=(0, 180, 0))
    prop(vmf, "npc_android_basic", (400, -150, 8),
         {"targetname": "android_saw", "enemyfilter": "filter_small_enemies"},
         angles=(0, 180, 0))
    vmf.entity("logic_auto", {"origin": "400 -600 16", "spawnflags": "1"},
               outputs=[("OnMapSpawn", "android_shield", "CreateAddon", "ai_addon_shield", 1.0),
                        ("OnMapSpawn", "android_saw", "CreateAddon", "ai_addon_saw", 1.0)])
    prop(vmf, "npc_chicken", (600, 250, 8), {"targetname": "chicken"},
         outputs=probed("chicken", "OnDeath"))
    prop(vmf, "info_hint", (700, 450, 8), {"targetname": "chicken_nest", "hinttype": "1200",
                                           "nodeFOV": "360", "StartHintDisabled": "0",
                                           "spawnflags": "0"})
    vmf.entity("prop_dynamic", {"origin": "700 450 0", "angles": "0 0 0", "model": NEST,
                                "solid": "6"})
    for i in range(2):
        vmf.entity("prop_physics", {"origin": vec((690 + 20 * i, 450, 16)),
                                    "angles": "0 0 0", "model": EGG})
    prop(vmf, "npc_hover_turret", (600, -400, 64), {"targetname": "hover_turret",
                                                    "ignoreclipbrushes": "0"},
         angles=(0, 180, 0))
    # A lamp that lights when a scale-1 object (the crate) enters its pad.
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
    """E: x 768..1280. Blobs and the companion bots."""
    node_grid(vmf, 768, 1280)
    prop(vmf, "npc_blob_fountain", (1000, -300, 8),
         {"targetname": "blob_fountain", "particlecount": "120", "particle_radius": "4"})
    prop(vmf, "npc_blob_demomonster", (1000, 300, 8),
         {"targetname": "blob_monster", "particlecount": "80", "particle_radius": "5"})
    # The companion bots: the medic (defensive) and offense bots.
    prop(vmf, "npc_medicbot", (900, 620, 8), {"targetname": "medicbot"}, angles=(0, 180, 0),
         outputs=probed("medicbot", "OnPlayerUse"))
    prop(vmf, "npc_obot", (1150, 620, 8), {"targetname": "obot"}, angles=(0, 180, 0),
         outputs=probed("obot", "OnPlayerUse"))


def vmf_text():
    del PROBES[:]
    vmf = Vmf()
    hall(vmf)
    bay_camera(vmf)
    bay_props(vmf)
    bay_portals(vmf)
    bay_npcs(vmf)
    bay_blobs(vmf)
    probe_relays(vmf)
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
