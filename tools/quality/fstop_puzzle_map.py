#!/usr/bin/env python3
"""Build the first F-Stop puzzle chamber, fstop_puzzle_01.

A Portal-style test chamber for the camera (weapon_camera photographs a
capturable prop out of the world; weapon_placement puts it back, optionally
scaled). Four beats, each teaching one rule and the last combining them:

1. Across the gap. The camera waits on Valve's F-Stop pedestal in the
   vestibule. A 224-unit pit (too wide to jump) separates the Portal cube
   from a floor button. Photograph the cube, aim at the button, place it: an
   info_placement_helper snaps it onto the plate and the bridge slides out.
   Objects are placed only within 30 feet, so it has to be done from the edge.
2. Make a step. On the far side, pick up the size scroller (Valve's
   camera_size_scroller item; it enables scaling on the camera). The ledge is
   100 units high, out of reach (a crouch jump clears 57). Photograph the hay
   bale and place it against the ledge at 2x (one scroll step up): 47 units
   tall, it is a crouch jump up and another onto the ledge. At 1x it is too
   low, at 4x too tall to climb.
3. The eraser field. The exit button is behind a trigger_photo_eraser. The
   anvil is too heavy to carry (150 kg against the player's 85 kg lift limit),
   and neither the camera nor the placement tool reaches through the field.
   Walking through it with a photo erases the photo and the anvil reappears
   where it was.
4. Combination. Photograph the anvil and place it at half size (75 kg), carry
   it through the field by hand, photograph it again on the far side and put it
   on the exit button at full size: the button takes only the full-weight
   anvil (filter_size), and a half-size anvil on it says "too light". The exit
   door opens; the test ends in the exit room with a chicken.

Falling into the pit returns the player to its near edge; anything that falls in
can be photographed from above and placed again, so the chamber cannot be
softlocked.

    python3 tools/quality/fstop_puzzle_map.py          build and install
    ./play_fstop +map fstop_puzzle_01

The solution is also written out as console input (--write-walkthrough
<game>/cfg), for a headless run in a private staged runtime:
+map fstop_puzzle_01 +wait 240 +exec fstop_puzzle_01_walk_0 with developer 2
logging the entity outputs listed in WALKTHROUGH_OUTPUTS.

Like tools/quality/fstop_mechanics_map.py, it compiles with the pinned legacy
vbsp/vvis/vrad of the PBRT map toolchain against the staged Portal runtime
(tools/quality/vmf_map_build.py) and installs into the F-Stop runtime's game
directory (run/runtime-fstop/fstop/maps). Before compiling it checks that every
model the map names resolves in the Portal runtime or in Valve's F-Stop-era
content staged next to the game directory (fstop_valve, fstop_valve_tempcontent).
"""

import argparse
import json
import os
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import vmf_map_build  # noqa: E402
from gyro_lab_map import Vmf, vec  # noqa: E402
from source_content import ContentResolver  # noqa: E402

ROOT = HERE.parents[1]
TOOLCHAIN = ROOT / "build/toolchains/pbrt-map-toolchain.json"
GAME_DIR = ROOT / "run/runtime-fstop/fstop"
NAME = "fstop_puzzle_01"

# Portal test chamber materials (testchmb_a_02/08) from the Portal runtime.
FLOOR = "CONCRETE/CONCRETE_MODULAR_FLOOR001A"
WALL = "CONCRETE/CONCRETE_MODULAR_WALL001A"
CEILING = "CONCRETE/CONCRETE_MODULAR_CEILING001A"
DARK = "METAL/METALWALL048B"
BRIDGE = "METAL/METAL_MODULAR_FLOOR001"
LIGHT_PANEL = "LIGHTS/WHITE008"
FIELD = "EFFECTS/PORTAL_CLEANSER"
NODRAW = "TOOLS/TOOLSNODRAW"
TRIGGER = "TOOLS/TOOLSTRIGGER"

# Models: Portal's (runtime packs) and Valve's F-Stop-era content (depot 852).
CUBE = "models/props/metal_box.mdl"
BUTTON_BASE = "models/props/button_base_reference.mdl"
BUTTON_TOP = "models/props/button_top_reference.mdl"
DOOR_FRAME = "models/props/door_01_frame_reference.mdl"
DOOR_LEFT = "models/props/door_01_lftdoor_reference.mdl"
DOOR_RIGHT = "models/props/door_01_rtdoor_reference.mdl"
PEDESTAL = "models/props_lab/aperture_pedestal.mdl"
MANUAL = "models/props_fstop/instruction_manual_scrap01.mdl"
FILM = "models/items/film_roll.mdl"
PHOTO = "models/items/photograph.mdl"
SCROLLER = "models/items/camera_size_scroller.mdl"
HAYBALE = "models/props_gameplay/haybale.mdl"
ANVIL = "models/props_farm/anvil.mdl"
NEST = "models/props_farm/chicken_nest.mdl"
SUNFLOWER = "models/props_farm/sunflower.mdl"
CLEANSER = "models/props/portal_cleanser_1.mdl"
MODELS = (CLEANSER, CUBE, BUTTON_BASE, BUTTON_TOP, DOOR_FRAME, DOOR_LEFT, DOOR_RIGHT, PEDESTAL,
          MANUAL, FILM, PHOTO, SCROLLER, HAYBALE, ANVIL, NEST, SUNFLOWER)
# Valve's F-Stop-era content, staged next to the F-Stop game directory.
FSTOP_CONTENT_DIRS = ("fstop_valve", "fstop_valve_tempcontent")

# Layout (inches; x runs from the vestibule to the exit).
T = 16                       # wall thickness
HALF_Y = 192                 # chamber half width
BOTTOM = -256                # pit floor
TOP = 384                    # chamber ceiling
PIT = (512, 736)             # the gap: 224 wide (a running jump covers ~170)
LEDGE_X = 1152               # the step up
LEDGE_Z = 100                # out of reach: jump 21 + air duck 36 = 57
FIELD_X = 1536               # eraser wall (16 thick)
END_X = 1920                 # exit wall (16 thick)
EXIT_ROOM = (1936, 2144)
DOOR_HALF_Y = 64             # door openings: 128 wide
DOOR_H = 108                 # exit opening height (the frame model's)
FIELD_H = 128                # eraser doorway height

# Puzzle pieces.
BUTTON1 = (832, 0, 0)        # across the gap, 96 from the far edge
BUTTON2 = (1760, 0, LEDGE_Z)
BUTTON_TRAVEL = 8
ANVIL_MASS_SCALE = 0.3       # 500 kg model: 150 kg at 1x, 75 kg at 0.5x
PLAYER_LIFT_LIMIT = 85       # PORTAL_PLAYER_MAX_LIFT_MASS
ANVIL_BASE_MASS = 500
HAYBALE_HEIGHT = 23.7       # collision height at 1x (a player stood on a 4x bale at 94.8)
CUBE_HEIGHT = 40.6
JUMP_REACH = 21 + 36         # GAMEMOVEMENT_JUMP_HEIGHT + the air-duck lift
SCALES = (0.25, 0.5, 1.0, 2.0, 4.0)   # CPhotoPlacementQuery::GetSimpleScales
PLACEMENT_REACH = 360
# prop_physics "Generate output on +USE": makes +use reach props over 35 kg
# (CPhysicsProp::ObjectCaps); the player's 85 kg lift limit then decides.
USE_PICKUP = "256"        # CPhotoPlacementQuery::GetMaxPlacementDistance
EYE = 64


def box(vmf, lo, hi, material):
    center = tuple((a + b) / 2.0 for a, b in zip(lo, hi))
    half = tuple((b - a) / 2.0 for a, b in zip(lo, hi))
    return vmf.box(center, half, material)


def light(vmf, origin, brightness=320, color="255 248 236"):
    vmf.entity("light", {"origin": vec(origin), "_light": "%s %d" % (color, brightness),
                         "_quadratic_attn": "0", "_linear_attn": "1", "_constant_attn": "0"})


def prop(vmf, classname, origin, keyvalues=None, angles=(0, 0, 0), outputs=()):
    kv = {"origin": vec(origin), "angles": vec(angles)}
    kv.update(keyvalues or {})
    vmf.entity(classname, kv, outputs=outputs)


def decor(vmf, model, origin, angles=(0, 0, 0), solid=True):
    prop(vmf, "prop_dynamic", origin, {"model": model, "solid": "6" if solid else "0",
                                       "DisableBoneFollowers": "1"}, angles)


def hint(vmf, name, message, origin, hold=7.0, y=0.72):
    """A screen message shown by its Display input."""
    vmf.entity("game_text", {"targetname": name, "origin": vec(origin), "message": message.replace("\n", "\\n"),
                             "x": "-1", "y": "%g" % y, "effect": "0", "color": "236 240 255",
                             "color2": "240 110 0", "fadein": "0.4", "fadeout": "1",
                             "holdtime": "%g" % hold, "fxtime": "0.25", "channel": "1",
                             "spawnflags": "1"})


def hint_trigger(vmf, lo, hi, target):
    vmf.entity("trigger_once", {"spawnflags": "1", "StartDisabled": "0"},
               solids=[box(vmf, lo, hi, TRIGGER)], outputs=[("OnStartTouch", target, "Display",
                                                             "", 0)])


def shell(vmf):
    """Vestibule, the chamber (lower floor, pit, ledge, eraser wall), exit room."""
    x_end = END_X + T
    y0, y1 = -HALF_Y, HALF_Y
    w = vmf.world
    # Chamber: base under the pit, side walls, ceiling.
    w.append(box(vmf, (-T, y0 - T, BOTTOM - T), (x_end, y1 + T, BOTTOM), DARK))
    w.append(box(vmf, (-T, y0 - T, BOTTOM), (x_end, y0, TOP), WALL))
    w.append(box(vmf, (-T, y1, BOTTOM), (x_end, y1 + T, TOP), WALL))
    w.append(box(vmf, (-T, y0 - T, TOP), (x_end, y1 + T, TOP + T), CEILING))
    # Lower floor: near side and far side of the pit, dark blocks below.
    for x0, x1 in ((0, PIT[0]), (PIT[1], LEDGE_X)):
        w.append(box(vmf, (x0, y0, BOTTOM), (x1, y1, -T), DARK))
        w.append(box(vmf, (x0, y0, -T), (x1, y1, 0), FLOOR))
    # The ledge and the upper floor.
    w.append(box(vmf, (LEDGE_X, y0, BOTTOM), (END_X, y1, LEDGE_Z - T), DARK))
    w.append(box(vmf, (LEDGE_X, y0, LEDGE_Z - T), (END_X, y1, LEDGE_Z), FLOOR))
    # Entry wall (x = -16..0) with the vestibule doorway.
    doorway(vmf, -T, 0, (BOTTOM, TOP), (0, 128))
    # Eraser wall and exit wall, floor to ceiling so nothing is photographed over them.
    doorway(vmf, FIELD_X, FIELD_X + T, (LEDGE_Z, TOP), (LEDGE_Z, LEDGE_Z + FIELD_H))
    doorway(vmf, END_X, END_X + T, (BOTTOM, TOP), (LEDGE_Z, LEDGE_Z + DOOR_H))
    # Vestibule.
    vx0 = -400
    w.append(box(vmf, (vx0 - T, -128 - T, -T), (-T, 128 + T, 0), FLOOR))
    w.append(box(vmf, (vx0 - T, -128 - T, 192), (-T, 128 + T, 192 + T), CEILING))
    w.append(box(vmf, (vx0 - T, -128 - T, 0), (-T, -128, 192), WALL))
    w.append(box(vmf, (vx0 - T, 128, 0), (-T, 128 + T, 192), WALL))
    w.append(box(vmf, (vx0 - T, -128, 0), (vx0, 128, 192), WALL))
    # Exit room.
    ex0, ex1 = EXIT_ROOM
    z0, z1 = LEDGE_Z, LEDGE_Z + 144
    w.append(box(vmf, (ex0, -128, z0 - T), (ex1 + T, 128, z0), FLOOR))
    w.append(box(vmf, (ex0, -128, z1), (ex1 + T, 128, z1 + T), CEILING))
    w.append(box(vmf, (ex0, -128, z0), (ex1, -112, z1), WALL))
    w.append(box(vmf, (ex0, 112, z0), (ex1, 128, z1), WALL))
    w.append(box(vmf, (ex1, -128, z0), (ex1 + T, 128, z1), WALL))
    # Light panels in the ceiling (self-illuminated; the light entities light the room).
    for x0, x1 in ((64, 448), (800, 1088), (1216, 1472), (1600, 1856)):
        for y in (-96, 96):
            w.append(box(vmf, (x0, y - 16, TOP - 4), (x1, y + 16, TOP), LIGHT_PANEL))
    # A dark frame around the eraser doorway, on the approach side.
    fz = LEDGE_Z + FIELD_H
    for lo, hi in (((FIELD_X - 8, -DOOR_HALF_Y - 8, LEDGE_Z), (FIELD_X, -DOOR_HALF_Y, fz + 8)),
                   ((FIELD_X - 8, DOOR_HALF_Y, LEDGE_Z), (FIELD_X, DOOR_HALF_Y + 8, fz + 8)),
                   ((FIELD_X - 8, -DOOR_HALF_Y, fz), (FIELD_X, DOOR_HALF_Y, fz + 8))):
        w.append(box(vmf, lo, hi, DARK))


def doorway(vmf, x0, x1, span, opening):
    """A wall across the chamber (y -HALF_Y..HALF_Y, z span) with a centered
    opening DOOR_HALF_Y wide on each side and z opening."""
    z0, z1 = span
    o0, o1 = opening
    y0, y1 = -HALF_Y, HALF_Y
    vmf.world += [box(vmf, (x0, y0, z0), (x1, -DOOR_HALF_Y, z1), WALL),
                  box(vmf, (x0, DOOR_HALF_Y, z0), (x1, y1, z1), WALL),
                  box(vmf, (x0, -DOOR_HALF_Y, o1), (x1, DOOR_HALF_Y, z1), WALL)]
    if o0 > z0:
        vmf.world.append(box(vmf, (x0, -DOOR_HALF_Y, z0), (x1, DOOR_HALF_Y, o0), WALL))


def lights(vmf):
    light(vmf, (-200, 0, 170), 220)
    for x in (160, 400):
        light(vmf, (x, 0, 330))
    light(vmf, (624, 0, 300), 260)
    light(vmf, (624, 0, -160), 90, "180 200 255")
    for x in (860, 1060):
        light(vmf, (x, 0, 330))
    light(vmf, (1340, 0, 350))
    light(vmf, (1740, 0, 350))
    light(vmf, (2040, 0, LEDGE_Z + 120), 90)


def floor_button(vmf, name, center, trigger_filter, on_press, on_release, extra_outputs=()):
    """A Portal floor button: the base, a plate (func_door, nodraw collision)
    carrying the top model down BUTTON_TRAVEL units, and a physics trigger."""
    x, y, z = center
    decor(vmf, BUTTON_BASE, center)
    plate = name + "_plate"
    vmf.entity("func_door", {"targetname": plate, "origin": vec((x, y, z + 7)),
                             "movedir": "90 0 0", "speed": "40", "lip": "%d" % (10 - BUTTON_TRAVEL),
                             "wait": "-1", "spawnflags": "0", "dmg": "0", "forceclosed": "1",
                             "ignoredebris": "1", "renderamt": "255",
                             "noise1": "doors/vent_open1.wav"},
               solids=[box(vmf, (x - 30, y - 30, z + 2), (x + 30, y + 30, z + 12), NODRAW)])
    prop(vmf, "prop_dynamic", center, {"targetname": name + "_top", "model": BUTTON_TOP,
                                        "solid": "0", "parentname": plate,
                                        "DisableBoneFollowers": "1"})
    kv = {"targetname": name + "_trigger", "spawnflags": "8", "wait": "0.2", "StartDisabled": "0"}
    if trigger_filter:
        kv["filtername"] = trigger_filter
    press = [("OnStartTouch", plate, "Open", "", 0)] + list(on_press)
    release = [("OnEndTouchAll", plate, "Close", "", 0)] + list(on_release)
    vmf.entity("trigger_multiple", kv,
               solids=[box(vmf, (x - 40, y - 40, z + 10), (x + 40, y + 40, z + 64), TRIGGER)],
               outputs=press + release + list(extra_outputs))


def vestibule(vmf):
    """Spawn, the camera on Valve's F-Stop pedestal, some photo lab clutter."""
    prop(vmf, "info_player_start", (-330, 0, 8))
    decor(vmf, PEDESTAL, (-200, -64, 0), angles=(0, 180, 0))
    prop(vmf, "weapon_camera", (-200, -64, 60),
         {"targetname": "camera", "captureslots": "1", "canscale": "0", "canzoom": "1",
          "spawnflags": "1"},
         angles=(0, 90, 0),
         outputs=[("OnPlayerPickup", "hint_camera", "Display", "", 0.5)])
    # These models have no prop data, so they are physics props by override.
    prop(vmf, "prop_physics_override", (-150, 70, 2), {"model": MANUAL, "canbecaptured": "0",
                                                      "health": "0"}, angles=(0, 25, 0))
    prop(vmf, "prop_physics_override", (-240, -60, 6), {"model": FILM, "canbecaptured": "1",
                                                       "health": "0"})
    prop(vmf, "prop_physics_override", (-250, 80, 2), {"model": PHOTO, "canbecaptured": "0",
                                                      "health": "0"}, angles=(0, 0, 90))
    hint(vmf, "hint_intro", "F-STOP  TEST 01\n\nPick up the camera.", (-300, 0, 40), hold=5)
    hint(vmf, "hint_camera",
         "PRIMARY FIRE: raise the camera.  PRIMARY FIRE again: take the photo.\n"
         "What you photograph leaves the world and goes into the picture.",
         (-300, 0, 48), hold=9)
    vmf.entity("logic_auto", {"origin": "-300 0 56", "spawnflags": "1"},
               outputs=[("OnMapSpawn", "hint_intro", "Display", "", 1.5)])


def beat_gap(vmf):
    """1: the cube goes onto the button across the pit; the bridge slides out."""
    prop(vmf, "prop_physics", (400, -110, 24), {"targetname": "cube", "model": CUBE,
                                               "canbecaptured": "1", "scalevalue": "0",
                                               "skin": "0", "spawnflags": USE_PICKUP},
         angles=(0, 20, 0),
         outputs=[("OnCameraCapture", "hint_place", "Display", "", 0.3)])
    hint(vmf, "hint_place",
         "PRIMARY FIRE: aim the photo.  PRIMARY FIRE again: put the object back.\n"
         "SECONDARY FIRE: stop aiming.  Photos are placed within 30 feet.",
         (400, 0, 48), hold=9)
    hint(vmf, "hint_gap", "The button is across the gap.", (300, 0, 48), hold=5)
    hint_trigger(vmf, (360, -HALF_Y, 0), (400, HALF_Y, 128), "hint_gap")
    x, y, z = BUTTON1
    floor_button(vmf, "button1", BUTTON1, None,
                 on_press=[("OnStartTouch", "bridge", "Open", "", 0.2)],
                 on_release=[("OnEndTouchAll", "bridge", "Close", "", 0.2)])
    prop(vmf, "info_placement_helper", (x, y, z + 38),
         {"targetname": "helper_button1", "radius": "40", "hide_until_placed": "1",
          "snap_to_helper_angles": "1", "force_placement": "0", "usesizelimit": "0",
          "StartDisabled": "0"})
    # The bridge waits inside the far floor and slides out across the pit.
    vmf.entity("func_door", {"targetname": "bridge", "origin": vec((848, 0, -9)),
                             "movedir": "0 180 0", "speed": "180", "lip": "0", "wait": "-1",
                             "spawnflags": "0", "dmg": "0", "forceclosed": "0",
                             "renderamt": "255", "noise1": "doors/heavy_metal_move1.wav",
                             "noise2": "doors/heavy_metal_stop1.wav"},
               solids=[box(vmf, (PIT[1], -64, -17), (PIT[1] + (PIT[1] - PIT[0]), 64, -1), BRIDGE)])
    # The pit returns the player to its near edge.
    vmf.entity("trigger_teleport", {"spawnflags": "1", "target": "pit_return",
                                    "StartDisabled": "0"},
               solids=[box(vmf, (PIT[0], -HALF_Y, BOTTOM), (PIT[1], HALF_Y, BOTTOM + 96), TRIGGER)])
    prop(vmf, "info_teleport_destination", (440, 0, 8), {"targetname": "pit_return"})


def beat_step(vmf):
    """2: the size scroller, then the hay bale at 2x as a step onto the ledge."""
    vmf.world.append(box(vmf, (880, -176, 0), (912, -144, 36), WALL))
    prop(vmf, "prop_dynamic", (896, -160, 36), {"targetname": "scroller", "model": SCROLLER,
                                                "solid": "0", "DisableBoneFollowers": "1"},
         angles=(0, 180, 0))
    vmf.entity("trigger_once", {"spawnflags": "1", "StartDisabled": "0"},
               solids=[box(vmf, (856, -192, 0), (936, -120, 96), TRIGGER)],
               outputs=[("OnStartTouch", "camera", "SetScaleAbility", "1", 0),
                        ("OnStartTouch", "scroller", "Kill", "", 0),
                        ("OnStartTouch", "hint_scroller", "Display", "", 0)])
    hint(vmf, "hint_scroller",
         "SIZE SCROLLER\n\nWhile placing a photo, MOUSE WHEEL makes the object bigger or smaller.",
         (896, -160, 64), hold=9)
    prop(vmf, "prop_physics", (1010, 120, 10), {"targetname": "haybale", "model": HAYBALE,
                                               "canbecaptured": "1", "scalevalue": "0",
                                               "spawnflags": USE_PICKUP},
         angles=(0, 90, 0))
    hint(vmf, "hint_ledge", "Too high to climb.\nCrouch in mid-air to jump higher.",
         (1000, 0, 48), hold=4)
    hint_trigger(vmf, (1040, -HALF_Y, 0), (1100, HALF_Y, 96), "hint_ledge")
    decor(vmf, SUNFLOWER, (760, 170, 0))


def beat_eraser(vmf):
    """3 and 4: the anvil, the eraser field, the exit button that wants the full anvil."""
    # The anvil model has no prop data either.
    prop(vmf, "prop_physics_override", (1360, -120, LEDGE_Z + 1),
         {"targetname": "anvil", "model": ANVIL, "canbecaptured": "1", "scalevalue": "0",
          "massScale": "%g" % ANVIL_MASS_SCALE, "health": "0",
          "spawnflags": USE_PICKUP}, angles=(0, 0, 0))
    vmf.entity("trigger_photo_eraser", {"targetname": "eraser", "spawnflags": "1",
                                        "StartDisabled": "0"},
               solids=[box(vmf, (FIELD_X, -DOOR_HALF_Y, LEDGE_Z),
                           (FIELD_X + T, DOOR_HALF_Y, LEDGE_Z + FIELD_H), TRIGGER)],
               outputs=[("OnObjectsFizzled", "hint_erased", "Display", "", 0.2)])
    vmf.entity("func_brush", {"targetname": "eraser_field", "Solidity": "1", "spawnflags": "2",
                              "disableshadows": "1", "rendermode": "0", "renderamt": "255",
                              "origin": vec((FIELD_X + 8, 0, LEDGE_Z + FIELD_H / 2))},
               solids=[box(vmf, (FIELD_X + 7, -DOOR_HALF_Y, LEDGE_Z),
                           (FIELD_X + 9, DOOR_HALF_Y, LEDGE_Z + FIELD_H), FIELD)])
    # Portal's cleanser look: its particle field and emitter posts.
    prop(vmf, "info_particle_system", (FIELD_X + 8, 0, LEDGE_Z + FIELD_H / 2),
         {"effect_name": "portal_cleanser", "start_active": "1"}, angles=(0, 270, 0))
    for y in (-DOOR_HALF_Y - 4, DOOR_HALF_Y + 4):
        decor(vmf, CLEANSER, (FIELD_X - 12, y, LEDGE_Z + 66), solid=False)
    hint(vmf, "hint_eraser",
         "ERASER FIELD\n\nPhotos do not survive it, and the camera cannot see through it.",
         (1450, 0, LEDGE_Z + 48), hold=7)
    hint_trigger(vmf, (1420, -HALF_Y, LEDGE_Z), (1460, HALF_Y, LEDGE_Z + 128), "hint_eraser")
    hint(vmf, "hint_erased", "The photo was erased: the anvil is back where it was.",
         (1560, 0, LEDGE_Z + 48), hold=6)
    hint(vmf, "hint_heavy", "The anvil is too heavy to carry.", (1400, 0, LEDGE_Z + 48),
         hold=4, y=0.8)
    hint(vmf, "hint_light", "Too light. The button needs the full-size anvil.",
         (1700, 0, LEDGE_Z + 48), hold=5)

    # Filters: the exit button takes the anvil at full size or larger; a
    # shrunk anvil on it only earns a hint.
    x, y, z = BUTTON2
    vmf.entity("filter_activator_name", {"targetname": "filter_is_anvil", "filtername": "anvil",
                                         "Negated": "0", "origin": vec((x, y - 160, z + 16))})
    for level in (-2, -1, 0, 1, 2):
        vmf.entity("filter_size", {"targetname": "filter_size_%s" % str(level).replace("-", "m"),
                                   "filtersize": "%d" % level, "Negated": "0",
                                   "origin": vec((x + 16 * level, y - 176, z + 16))})
    multi(vmf, "filter_full_size", 1, ["filter_size_0", "filter_size_1", "filter_size_2"],
          (x, y - 192, z + 16))
    multi(vmf, "filter_small_size", 1, ["filter_size_m1", "filter_size_m2"],
          (x + 16, y - 192, z + 16))
    multi(vmf, "filter_heavy_anvil", 0, ["filter_is_anvil", "filter_full_size"],
          (x, y - 208, z + 16))
    multi(vmf, "filter_light_anvil", 0, ["filter_is_anvil", "filter_small_size"],
          (x + 16, y - 208, z + 16))
    floor_button(vmf, "button2", BUTTON2, "filter_heavy_anvil",
                 on_press=[("OnStartTouch", "exit_door_left", "Open", "", 0.3),
                           ("OnStartTouch", "exit_door_right", "Open", "", 0.3)],
                 on_release=[("OnEndTouchAll", "exit_door_left", "Close", "", 0.3),
                             ("OnEndTouchAll", "exit_door_right", "Close", "", 0.3)])
    vmf.entity("trigger_multiple", {"targetname": "button2_light", "spawnflags": "8",
                                    "wait": "3", "StartDisabled": "0",
                                    "filtername": "filter_light_anvil"},
               solids=[box(vmf, (x - 40, y - 40, z + 10), (x + 40, y + 40, z + 64), TRIGGER)],
               outputs=[("OnStartTouch", "hint_light", "Display", "", 0.3)])
    prop(vmf, "info_placement_helper", (x, y, z + 13),
         {"targetname": "helper_button2", "radius": "40", "hide_until_placed": "1",
          "snap_to_helper_angles": "1", "force_placement": "0", "usesizelimit": "0",
          "target_classname": "anvil", "StartDisabled": "0"})
    # The anvil's hint comes on the first approach (a use trigger would not follow it).
    hint_trigger(vmf, (1300, -HALF_Y, LEDGE_Z), (1330, 0, LEDGE_Z + 96), "hint_heavy")


def multi(vmf, name, filter_type, filters, origin):
    kv = {"targetname": name, "filtertype": "%d" % filter_type, "Negated": "0",
          "origin": vec(origin)}
    for i, f in enumerate(filters):
        kv["Filter%02d" % (i + 1)] = f
    vmf.entity("filter_multi", kv)


def exit_door(vmf):
    """Portal's chamber door in the exit wall: two func_door halves (nodraw
    collision) carrying the door models, and the frame."""
    x = END_X + T / 2
    z = LEDGE_Z
    decor(vmf, DOOR_FRAME, (x, 0, z), solid=False)
    for side, model, sign, yaw in (("left", DOOR_LEFT, 1, 90), ("right", DOOR_RIGHT, -1, 270)):
        name = "exit_door_" + side
        y0, y1 = (0, 57) if sign > 0 else (-57, 0)
        vmf.entity("func_door", {"targetname": name, "origin": vec((x, 0, z)),
                                 "movedir": "0 %d 0" % yaw, "speed": "120", "lip": "4",
                                 "wait": "-1", "spawnflags": "0", "dmg": "0",
                                 "forceclosed": "0", "renderamt": "255",
                                 "noise1": "plats/hall_elev_door.wav"},
                   solids=[box(vmf, (x - 6, y0, z), (x + 6, y1, z + 101), NODRAW)])
        prop(vmf, "prop_dynamic", (x, 0, z), {"targetname": name + "_model", "model": model,
                                              "solid": "0", "parentname": name,
                                              "DisableBoneFollowers": "1"})


def exit_room(vmf):
    ex0, ex1 = EXIT_ROOM
    z = LEDGE_Z
    hint(vmf, "hint_done", "TEST COMPLETE\n\nThank you for participating.", (ex1 - 64, 0, z + 48),
         hold=10, y=0.4)
    vmf.entity("trigger_once", {"targetname": "finish", "spawnflags": "1", "StartDisabled": "0"},
               solids=[box(vmf, (ex0 + 64, -112, z), (ex1, 112, z + 128), TRIGGER)],
               outputs=[("OnStartTouch", "hint_done", "Display", "", 0),
                        ("OnStartTouch", "finish_sound", "PlaySound", "", 0)])
    vmf.entity("ambient_generic", {"targetname": "finish_sound",
                                   "origin": vec((ex1 - 64, 0, z + 64)),
                                   "message": "plats/elevbell1.wav", "health": "10",
                                   "pitch": "100", "pitchstart": "100", "radius": "1250",
                                   "spawnflags": "49"})
    decor(vmf, NEST, (ex1 - 70, -60, z))
    for i, (nx, ny) in enumerate((nx, ny) for nx in (ex0 + 48, ex0 + 112, ex1 - 48)
                                 for ny in (-64, 64)):
        prop(vmf, "info_node", (nx, ny, z + 8), {"nodeid": "%d" % (i + 1)})
    prop(vmf, "npc_chicken", (ex1 - 90, 50, z + 8), {"targetname": "reward_chicken"},
         angles=(0, 180, 0))


def vmf_text():
    vmf = Vmf()
    shell(vmf)
    lights(vmf)
    vestibule(vmf)
    beat_gap(vmf)
    beat_step(vmf)
    beat_eraser(vmf)
    exit_door(vmf)
    exit_room(vmf)
    return vmf.text()


def solvability():
    """The numbers the intended solution relies on; test_fstop_puzzle_map.py
    checks them against the movement and placement limits."""
    near_edge_eye = (PIT[0], 0, EYE)
    reach = sum((a - b) ** 2 for a, b in zip(BUTTON1, near_edge_eye)) ** 0.5
    return {
        "pit_width": PIT[1] - PIT[0],
        "ledge_height": LEDGE_Z,
        "jump_reach": JUMP_REACH,
        "bale_heights": {scale: HAYBALE_HEIGHT * scale for scale in SCALES},
        "cube_height": CUBE_HEIGHT,
        "button1_reach_from_edge": reach,
        "anvil_mass_full": ANVIL_BASE_MASS * ANVIL_MASS_SCALE * SCALES[2],
        "anvil_mass_half": ANVIL_BASE_MASS * ANVIL_MASS_SCALE * SCALES[1],
        "lift_limit": PLAYER_LIFT_LIMIT,
        "placement_reach": PLACEMENT_REACH,
    }


def referenced_models(text):
    models = set()
    for line in text.splitlines():
        line = line.strip()
        if line.startswith('"model" "') and line.endswith('.mdl"'):
            models.add(line[len('"model" "'):-1].lower())
    return sorted(models)


def missing_models(models, runtime, game_dir):
    """Models found neither in the Portal runtime's packs nor in Valve's F-Stop
    content staged next to the game directory."""
    resolver = ContentResolver(str(runtime))
    roots = [Path(game_dir)] + [Path(game_dir).parent / d for d in FSTOP_CONTENT_DIRS]
    missing = []
    for model in models:
        if resolver.read(model)[0] is not None:
            continue
        if any(os.path.isfile(root / model) for root in roots):
            continue
        missing.append(model)
    return missing

# The intended solution as console input, for a headless check in a private
# staged runtime (see --write-walkthrough). setpos/setang stand in for walking
# between beats; the camera, placement, scaling, carrying, eraser and climbing
# steps are the player's own inputs. The engine ignores `wait` in multi-line
# cfgs and truncates long lines, so the steps become a chain of one-line cfgs.
_SHOOT = "+attack; wait 5; -attack"
_PHOTO = _SHOOT + "; wait 40; " + _SHOOT     # raise the camera, take the photo
_JUMP = "+jump; wait 3; -jump; +duck; wait 60; -duck; wait 30"   # crouch jump
_UP = LEDGE_Z + 20
WALKTHROUGH = [
    "sv_cheats 1; developer 2; con_drawnotify 0",
    "echo WALK_camera", "setpos -178 -64 8; setang 0 180 0", "wait 60", "screenshot",
    "echo WALK_photo_cube", "setpos 300 -110 8; setang 22 0 0", "wait 30", _PHOTO, "wait 60",
    "echo WALK_place_cube", "setpos 500 0 8; setang 5 0 0", "wait 30", _SHOOT, "wait 40",
    _SHOOT, "wait 120", "screenshot",
    "echo WALK_bridge", "setpos 480 0 8; setang 0 0 0; +forward", "wait 250", "-forward; getpos",
    "echo WALK_scroller", "setpos 896 -120 8; setang 10 -60 0", "wait 60", "screenshot",
    "echo WALK_photo_bale", "setpos 900 120 8; setang 26 0 0", "wait 30", _PHOTO, "wait 90",
    "echo WALK_place_bale_2x", "setpos 900 -40 8; setang 17 0 0", "wait 30", _SHOOT, "wait 30",
    "mwheel_down", "wait 40", _SHOOT, "wait 150", "setpos 780 -170 8; setang 0 25 0", "wait 30",
    "screenshot",
    "echo WALK_climb", "setpos 960 -40 8; setang 0 0 0; +forward", "wait 60", _JUMP, _JUMP, _JUMP,
    _JUMP, _JUMP, "-forward; getpos",
    "echo WALK_lift_anvil_1x", "setpos 1330 -120 %d; setang 60 0 0" % _UP, "wait 40",
    "+use; wait 5; -use", "wait 60",
    "echo WALK_photo_anvil", "setpos 1310 -120 %d; setang 45 0 0" % _UP, "wait 30", _PHOTO,
    "wait 60",
    "echo WALK_eraser", "setpos 1480 0 %d; setang 0 0 0; +forward" % _UP, "wait 200",
    "-forward; getpos", "wait 60", "screenshot",
    "echo WALK_photo_anvil_again", "setpos 1310 -120 %d; setang 45 0 0" % _UP, "wait 30", _PHOTO,
    "wait 60",
    "echo WALK_place_anvil_half", "setpos 1290 0 %d; setang 45 0 0" % _UP, "wait 30", _SHOOT,
    "wait 30", "mwheel_up", "wait 30", _SHOOT, "wait 90",
    "echo WALK_carry", "setpos 1322 0 %d; setang 60 0 0" % _UP, "wait 20", "+use; wait 5; -use",
    "wait 30", "setang 10 0 0", "wait 20", "+forward", "wait 220", "-forward", "wait 20",
    "screenshot", "+use; wait 5; -use", "wait 90", "getpos",
    "echo WALK_photo_small_anvil", "setang 41 0 0", "wait 20", _PHOTO, "wait 60",
    "echo WALK_place_anvil_full", "setpos 1640 0 %d; setang 23 0 0" % _UP, "wait 40", _SHOOT,
    "wait 30", "mwheel_down", "wait 30", _SHOOT, "wait 150", "setang 5 0 0", "wait 10",
    "screenshot",
    "echo WALK_exit", "setpos 1840 0 %d; setang 0 0 0; +forward" % _UP, "wait 300",
    "-forward; getpos", "wait 30", "screenshot", "echo WALK_done", "quit",
]
# The outputs the walkthrough must fire, in order (developer 2 logs them).
WALKTHROUGH_OUTPUTS = [
    "(weapon_camera,camera) -> (hint_camera,Display",
    "(prop_physics,cube) -> (hint_place,Display",
    "(trigger_multiple,button1_trigger) -> (bridge,Open",
    "-> (camera,SetScaleAbility)(1)",
    "(trigger_photo_eraser,eraser) -> (hint_erased,Display",
    "(trigger_multiple,button2_trigger) -> (exit_door_left,Open",
    "(trigger_once,finish) -> (hint_done,Display",
]


def walkthrough_cfgs(prefix=NAME + "_walk", limit=400):
    """The walkthrough as {file name: one-line cfg}; each ends by exec'ing the next."""
    parts, current = [], []
    for step in WALKTHROUGH:
        if current and len("; ".join(current + [step])) > limit:
            parts.append(current)
            current = []
        current.append(step)
    parts.append(current)
    cfgs = {}
    for i, part in enumerate(parts):
        chain = ["exec %s_%d" % (prefix, i + 1)] if i + 1 < len(parts) else []
        cfgs["%s_%d.cfg" % (prefix, i)] = "; ".join(part + chain) + "\n"
    return cfgs


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
    parser.add_argument("--no-install", action="store_true",
                        help="compile into --out only; do not install into the game directory")
    parser.add_argument("--write-walkthrough", type=Path, metavar="CFG_DIR",
                        help="write the solution as chained cfgs into CFG_DIR (a staged game's "
                             "cfg/) and stop; run with +map %s +wait 240 +exec %s_walk_0"
                             % (NAME, NAME))
    args = parser.parse_args()

    if args.write_walkthrough:
        for name, line in walkthrough_cfgs().items():
            (args.write_walkthrough / name).write_text(line)
        print("wrote %d walkthrough cfgs to %s" % (len(walkthrough_cfgs()),
                                                   args.write_walkthrough))
        return 0

    out = args.out.resolve()
    (out / "compile").mkdir(parents=True, exist_ok=True)
    text = vmf_text()
    vmf = out / "compile" / (NAME + ".vmf")
    vmf.write_text(text)
    print("wrote " + str(vmf))
    missing = missing_models(referenced_models(text), args.runtime.resolve(),
                             args.game_dir.resolve())
    if missing:
        print("missing models: " + ", ".join(missing))
        return 1
    if args.vmf_only:
        return 0
    tools = Path(json.loads(args.toolchain.read_text())["compile_tools"])
    record = vmf_map_build.build(vmf, out, tools, args.runtime.resolve(), args.quality, NAME)
    print("vmf_map_build: %s %s (%s)" % (NAME, record["status"], out / "build.json"))
    if record["status"] != "pass":
        return 1
    if args.no_install:
        return 0
    print("installed " + str(vmf_map_build.install(record, args.game_dir.resolve())))
    return 0


if __name__ == "__main__":
    sys.exit(main())
