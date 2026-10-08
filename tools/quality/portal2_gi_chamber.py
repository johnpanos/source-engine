#!/usr/bin/env python3
"""Build `sp_gi_chamber_01`, a clean Aperture Portal 2 test chamber lit by the
GI map pipeline, with dynamic and moving lights as part of its art direction.

The chamber is authored here as a VMF with the Portal 2 SDK's own pieces
(door frames, the cube dropper, the fizzler, the floor button base, ceiling
light panels and an observation room, collapsed by `vmf_instances.py`),
compiled by `vmf_map_build.py` against the staged Portal 2 runtime and lit by
the one lighting back end (`map_lighting.py`, scene derived from the BSP, via
`vmf_map_build.light`): Cycles bakes the opaque world, the probe volume,
the radiosity transfer and the SDF volume, so every indirect-light producer
(`r_indirect_producer sdf` on desktop) brings the moving lights' bounce to the
white tiles. Gameplay lumps are carried byte for byte.

Test (a first Portal 2 chamber): the player enters through the entry door; a
weighted cube drops from the dropper onto the white lower floor. The exit
ledge is 256 units up behind a black, unportalable face; white panels on the
north wall below and above it are the way up. The floor button on the ledge
opens the exit door behind an emancipation grid.

Light, in three kinds:
  * baked (Cycles): neutral ceiling panels, a cool entry hall and a warm
    tungsten observation room whose light spills through its window;
  * switched: the exit corridor's warm panels turn on with the door
    (light_dynamic spots; see exit_lights for why not named `light`s);
  * dynamic (light_dynamic, inverse square, in the runtime light set):
      - the observer: a cool spotlight on a security camera in the
        observation room, sweeping the chamber floor back and forth
        (a looping func_door_rotating);
      - two amber beacons that spin up beside the exit when the button is
        pressed (func_rotating) and wind down when it is released;
      - the emancipation grid's pale blue glow, pulsing (light style 5);
      - the button's indicator, blue until pressed, then orange.

    python3 tools/quality/portal2_gi_chamber.py                # build, relight, publish
    python3 tools/quality/portal2_gi_chamber.py --vmf-only     # just write the VMF
    ./kiln play portal2 sp_gi_chamber_01

Coordinates are Source units: x east, y north, z up; the lower floor is z 0.
"""

import argparse
import json
import math
import os
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import vmf_instances as vi  # noqa: E402

ROOT = HERE.parents[1]
NAME = "sp_gi_chamber_01"
TOOLCHAIN = ROOT / "build/toolchains/pbrt-map-toolchain.json"
STEAM_ROOT = Path(os.environ.get(
    "P2_STEAM_ROOT", Path.home() / ".local/share/Steam/steamapps/common/Portal 2"))

# ------------------------------------------------------------- materials
# The palette of Portal 2's sp_a2_triple_laser (its compiled faces by area):
# tall white panels on the walls with 4 x 4 and single-slab accents, the
# large 2 x 2 wall tile on the ceilings, 4 x 4 black metal with fine-grid and
# brushed accents, white floor tile and black floor metal.
WHITE_WALL = "TILE/WHITE_WALL_TILE003A"
WHITE_GRID = "TILE/WHITE_WALL_TILE003F"
WHITE_SLAB = "TILE/WHITE_WALL_TILE003L"
WHITE_FLOOR = "TILE/WHITE_FLOOR_TILE002A"
WHITE_CEILING = "TILE/WHITE_WALL_TILE003C"
BLACK_WALL = "METAL/BLACK_WALL_METAL_002A"
BLACK_TRIM = "METAL/BLACK_WALL_METAL_002B"
BLACK_BRUSHED = "METAL/BLACK_WALL_METAL_002D"
BLACK_FLOOR = "METAL/BLACK_FLOOR_METAL_001C"
NODRAW = "TOOLS/TOOLSNODRAW"
TRIGGER = "TOOLS/TOOLSTRIGGER"
GLOW = "sprites/light_glow03.vmt"

# --------------------------------------------------------------- layout
T = 16                      # wall, floor and ceiling thickness
X0, X1 = -768, 512          # chamber interior
Y0, Y1 = -512, 512
Z1 = 576                    # chamber ceiling
LEDGE_X, LEDGE_Z = 128, 256
FRAME = 24                  # door frame instance depth
HALL = (-1152, -808)        # entry hall interior x
EXIT = (552, 896)           # exit corridor interior x
HALL_HALF, HALL_TOP = 128, 192
EXIT_TOP = LEDGE_Z + 192
BUTTON = (320, -256)
DROPPER = (-384, 256)
OBSERVATION = (-384, Y0, 384)          # the window's center on the south wall
FIZZLER_X = 448
CEILING_PANELS = [(-576, -256), (-192, -256), (-576, 256), (-192, 256), (320, -256),
                  (320, 256)]
EXIT_STRIPS = [640, 808]
HALL_PANELS = [-1064, -896]

FACES = ("x-", "x+", "y-", "y+", "z-", "z+")
AXES = {"x": 0, "y": 1, "z": 2}


class Chamber:
    """The map being authored: world solids and entities as vmf_instances trees."""

    def __init__(self, sdk):
        self.sdk = sdk
        self.next = 1
        self.world = []
        self.entities = []

    def id(self):
        self.next += 1
        return self.next

    # ----------------------------------------------------------- brushes
    def solid(self, lo, hi, material, faces=None):
        """An axis-aligned box; `faces` overrides the material per face
        (x-, x+, y-, y+, z-, z+)."""
        if not all(a < b for a, b in zip(lo, hi)):
            raise ValueError("empty box %s %s" % (lo, hi))
        sides = []
        for face in FACES:
            axis, sign = AXES[face[0]], 1 if face[1] == "+" else -1
            normal = [0, 0, 0]
            normal[axis] = sign
            point = [lo[0], lo[1], lo[2]]
            if sign > 0:
                point[axis] = hi[axis]
            u, v = face_basis(normal)
            # vbsp's plane normal is cross(p0 - p1, p2 - p1) for (p1 + u, p1, p1 + v).
            p1 = tuple(point)
            p0 = tuple(a + 64 * b for a, b in zip(p1, u))
            p2 = tuple(a + 64 * b for a, b in zip(p1, v))
            uaxis, vaxis = texture_axes(normal)
            sides.append(("side", [
                ("id", str(self.id())),
                ("plane", " ".join("(%s)" % vi.vector_text(p) for p in (p0, p1, p2))),
                ("material", (faces or {}).get(face, material)),
                ("uaxis", "[%s 0] 0.25" % uaxis), ("vaxis", "[%s 0] 0.25" % vaxis),
                ("rotation", "0"), ("lightmapscale", "16"), ("smoothing_groups", "0")]))
        return [("id", str(self.id()))] + sides

    def box(self, lo, hi, material, faces=None):
        self.world.append(self.solid(lo, hi, material, faces))

    def slab(self, axis, depth, rect, material, holes=(), zones=(), face=None):
        """A wall, floor or ceiling slab `depth` = (lo, hi) thick along `axis`,
        covering `rect` = ((a0, a1), (b0, b1)) in the other two axes (in x, y,
        z order), minus `holes`. `zones` are (rect, material) overrides of the
        slab's material; `face` names the visible face ("x-", ...), which
        takes the zone material while the others are nodraw."""
        k = AXES[axis]
        others = [i for i in range(3) if i != k]
        placed = []
        for zone_rect, zone_material in zones:
            clipped = intersect(zone_rect, rect)
            if clipped:
                placed.append(clipped)
                for piece in subtract(clipped, holes):
                    self._slab_piece(k, others, depth, piece, zone_material, face)
        for piece in subtract(rect, list(holes) + placed):
            self._slab_piece(k, others, depth, piece, material, face)

    def _slab_piece(self, k, others, depth, piece, material, face):
        lo, hi = [0, 0, 0], [0, 0, 0]
        lo[k], hi[k] = depth
        for i, (a, b) in zip(others, piece):
            lo[i], hi[i] = a, b
        faces = None
        if face:
            faces = {f: NODRAW for f in FACES if f != face}
        self.box(lo, hi, material, faces)

    # ---------------------------------------------------------- entities
    def entity(self, classname, keys, solids=(), outputs=()):
        body = [("id", str(self.id())), ("classname", classname)]
        body += [(k, str(v)) for k, v in keys.items()]
        if outputs:
            body.append(("connections", [
                (event, ",".join(str(x) for x in (target, action, param, delay, -1)))
                for event, target, action, param, delay in outputs]))
        body += [("solid", s) for s in solids]
        self.entities.append(body)
        return body

    def brush_entity(self, classname, keys, lo, hi, material, outputs=()):
        return self.entity(classname, keys, [self.solid(lo, hi, material)], outputs)

    def instance(self, path, origin, angles=(0, 0, 0), name="", world=True, edit=None,
                 drop=None):
        """Collapse an SDK instance into the map; `edit(entity)` may change
        its collapsed entities, and world solids whose instance-space bounds
        `drop` accepts are left out. Returns its transform (instance -> world)."""
        if drop:
            local = vi.collapse(path, self.sdk, ids=self.id, world=True)
            dropped = [solid_bounds(s) for s in local.world if drop(solid_bounds(s))]
        collapsed = vi.collapse(path, self.sdk, origin, angles, name, 0 if name else 2,
                                ids=self.id, world=world)
        transform = vi.Transform(origin, angles)
        for solid in collapsed.world:
            if drop and any(same_box(solid_bounds(solid), transform, box) for box in dropped):
                continue
            self.world.append(solid)
        for entity in collapsed.entities:
            if edit:
                edit(entity)
            self.entities.append(entity)
        return transform

    def text(self):
        head = [("versioninfo", [("editorversion", "400"), ("editorbuild", "8000"),
                                 ("mapversion", "1"), ("formatversion", "100"),
                                 ("prefab", "0")]),
                ("visgroups", []),
                ("world", [("id", "1"), ("mapversion", "1"), ("classname", "worldspawn"),
                           ("skyname", "sky_black_nofog"), ("detailmaterial", "detail/detailsprites"),
                           ("detailvbsp", "detail.vbsp"), ("maxpropscreenwidth", "-1")] +
                 [("solid", s) for s in self.world])]
        return vi.serialize(head + [("entity", e) for e in self.entities]) + "\n"


def solid_bounds(solid):
    points = [tuple(float(c) for c in m) for side in vi.children(solid, "side")
              for m in vi.PLANE.findall(vi.value(side, "plane"))]
    return tuple((min(p[i] for p in points), max(p[i] for p in points)) for i in range(3))


def same_box(bounds, transform, local):
    """Whether world `bounds` are the instance-space box `local` moved by `transform`."""
    corners = [transform.point((x, y, z)) for x in local[0] for y in local[1] for z in local[2]]
    moved = [(min(c[i] for c in corners), max(c[i] for c in corners)) for i in range(3)]
    return all(abs(a - b) < 0.01 for pair in zip(moved, bounds) for a, b in zip(*pair))


def door_back_plate(bounds):
    """The SDK door frame's back plate: it closes the linked-portal door's
    cavity, which a walk-through door replaces with its corridor."""
    return bounds[1] == (16.0, 24.0) and bounds[0] == (-96.0, 96.0)


def face_basis(normal):
    """Two in-plane directions u, v with cross(u, v) = normal."""
    axis = [i for i in range(3) if normal[i]][0]
    sign = normal[axis]
    a, b = [(1, 2), (2, 0), (0, 1)][axis]
    u, v = [0, 0, 0], [0, 0, 0]
    u[a], v[b] = 1, 1
    if sign < 0:
        u, v = v, u
    return tuple(u), tuple(v)


def texture_axes(normal):
    """World-aligned texture axes (Hammer's default alignment)."""
    axis = [i for i in range(3) if normal[i]][0]
    if axis == 0:
        return "0 1 0", "0 0 -1"
    if axis == 1:
        return "1 0 0", "0 0 -1"
    return "1 0 0", "0 -1 0"


def intersect(a, b):
    (a0, a1), (b0, b1) = a
    (c0, c1), (d0, d1) = b
    lo_a, hi_a, lo_b, hi_b = max(a0, c0), min(a1, c1), max(b0, d0), min(b1, d1)
    if lo_a >= hi_a or lo_b >= hi_b:
        return None
    return ((lo_a, hi_a), (lo_b, hi_b))


def subtract(rect, holes):
    """`rect` minus the `holes`, as a few rectangles: cells of the grid of all
    hole edges, merged along the first axis within each row, then rows with
    the same spans merged."""
    (a0, a1), (b0, b1) = rect
    holes = [h for h in (intersect(h, rect) for h in holes) if h]
    xs = sorted({a0, a1} | {v for h in holes for v in h[0]})
    ys = sorted({b0, b1} | {v for h in holes for v in h[1]})

    def open_cell(x0, x1, y0, y1):
        cx, cy = (x0 + x1) / 2, (y0 + y1) / 2
        return not any(h[0][0] < cx < h[0][1] and h[1][0] < cy < h[1][1] for h in holes)

    rows = []
    for y0, y1 in zip(ys, ys[1:]):
        spans, start = [], None
        for x0, x1 in zip(xs, xs[1:]):
            if open_cell(x0, x1, y0, y1):
                start = x0 if start is None else start
                end = x1
            elif start is not None:
                spans.append((start, end))
                start = None
        if start is not None:
            spans.append((start, end))
        rows.append(((y0, y1), spans))
    pieces, open_rows = [], {}
    for (y0, y1), spans in rows:
        next_rows = {}
        for span in spans:
            if span in open_rows and open_rows[span][1] == y0:
                next_rows[span] = (open_rows.pop(span)[0], y1)
            else:
                next_rows[span] = (y0, y1)
        for span, (s0, s1) in open_rows.items():
            pieces.append((span, (s0, s1)))
        open_rows = next_rows
    pieces += [(span, rows_) for span, rows_ in open_rows.items()]
    return pieces


# ============================================================== authoring
def shell(m):
    """The chamber's walls, floor, ledge and ceiling, with their cut-outs."""
    # Lower floor: white tile; the ledge: a black block with a black floor.
    m.slab("z", (-T, 0), ((X0, LEDGE_X), (Y0, Y1)), WHITE_FLOOR, face="z+")
    m.box((LEDGE_X, Y0, -T), (X1, Y1, LEDGE_Z - 8), BLACK_BRUSHED,
          {"z+": NODRAW, "z-": NODRAW, "y-": NODRAW, "y+": NODRAW, "x+": NODRAW})
    bx, by = BUTTON
    m.slab("z", (LEDGE_Z - 8, LEDGE_Z), ((LEDGE_X, X1), (Y0, Y1)), BLACK_FLOOR,
           holes=[((bx - 64, bx + 64), (by - 64, by + 64))], face="z+")
    # Ceiling: white tile, with the dropper's shaft and the light panels cut out.
    dx, dy = DROPPER
    holes = [((dx - 64, dx + 64), (dy - 64, dy + 64))]
    holes += [((x - 64, x + 64), (y - 64, y + 64)) for x, y in CEILING_PANELS]
    m.slab("z", (Z1, Z1 + T), ((X0, X1), (Y0, Y1)), WHITE_CEILING, holes, face="z-")

    span = ((X0 - FRAME, X1 + FRAME), (-T, Z1 + T))
    # North wall: white below the ledge and on the ledge (the way up), black trim.
    m.slab("y", (Y1, Y1 + T), span, BLACK_WALL, face="y-",
           zones=[(((X0, LEDGE_X), (0, 320)), WHITE_WALL),
                  (((LEDGE_X, X1), (LEDGE_Z, LEDGE_Z + 192)), WHITE_GRID)])
    # South wall: white low on the lower floor, the observation window above.
    ox, _, oz = OBSERVATION
    m.slab("y", (Y0 - T, Y0), span, BLACK_WALL, face="y+",
           holes=[((ox - 128, ox + 128), (oz - 64, oz + 64))],
           zones=[(((X0, LEDGE_X), (0, 256)), WHITE_WALL)])
    # West wall (entry door) and east wall (exit door): FRAME deep, so the SDK
    # door frames plug their 192 x 160 cut-outs flush (the frames' side
    # posts reach 16 units further, into the wall).
    wall = ((Y0 - T, Y1 + T), (-T, Z1 + T))
    m.slab("x", (X0 - FRAME, X0), wall, BLACK_WALL, face="x+",
           holes=[((-96, 96), (-16, 144))])
    m.slab("x", (X1, X1 + FRAME), wall, BLACK_WALL, face="x-",
           holes=[((-96, 96), (LEDGE_Z - 16, LEDGE_Z + 144))])
    # The exit vestibule on the ledge: two black pillars and a lintel frame a
    # 128 x 128 passage to the exit door, the emancipation grid across it.
    for y0, y1 in ((64, 160), (-160, -64)):
        m.box((384, y0, LEDGE_Z), (X1, y1, LEDGE_Z + 192), BLACK_TRIM)
    m.box((384, -64, LEDGE_Z + 128), (X1, 64, LEDGE_Z + 192), BLACK_TRIM)


def corridor(m, x0, x1, floor, top, door_side, material_floor, strips=(), panels=(),
             wall=BLACK_WALL):
    """A black hall from x0 to x1 (interior) at `floor`..`top`, joined to a
    door frame on its `door_side` ("x-" or "x+") by a layer with a 128 x 128
    opening that hides the frame's back."""
    h = HALL_HALF
    m.slab("z", (floor - T, floor), ((x0, x1), (-h, h)), material_floor, face="z+")
    holes = [((c - 64, c + 64), (-16, 16)) for c in strips]
    holes += [((c - 64, c + 64), (-64, 64)) for c in panels]
    m.slab("z", (top, top + T), ((x0, x1), (-h, h)), BLACK_WALL, holes, face="z-")
    # Side walls: `wall` up to 128, black trim above.
    for y0, y1, face in ((-h - T, -h, "y+"), (h, h + T, "y-")):
        m.slab("y", (y0, y1), ((x0 - T, x1 + T), (floor - T, top + T)), BLACK_WALL, face=face,
               zones=[(((x0, x1), (floor, floor + 128)), wall)])
    end = (x1, x1 + T) if door_side == "x+" else (x0 - T, x0)
    far = (x0 - T, x0) if door_side == "x+" else (x1, x1 + T)
    m.slab("x", far, ((-h - T, h + T), (floor - T, top + T)), BLACK_WALL,
           face="x+" if door_side == "x+" else "x-")
    m.slab("x", end, ((-h - T, h + T), (floor - T, top + T)), BLACK_WALL,
           holes=[((-64, 64), (floor, floor + 128))],
           face="x-" if door_side == "x+" else "x+")


def doors(m):
    """The SDK door frames and the doors: entry (west) and exit (east)."""
    frame = "instances/door/portal_door_entities/portal_door_frame_white.vmf"
    m.instance(frame, (X0, 0, 0), (0, 90, 0), drop=door_back_plate)
    m.instance(frame, (X1, 0, LEDGE_Z), (0, 270, 0), drop=door_back_plate)
    # The frames' door sits 16 units into the frame, facing the chamber.
    m.entity("prop_testchamber_door", {"targetname": "entry_door",
                                       "origin": "%d 0 0" % (X0 - 16), "angles": "0 0 0"})
    m.entity("prop_testchamber_door", {"targetname": "exit_door",
                                       "origin": "%d 0 %d" % (X1 + 16, LEDGE_Z),
                                       "angles": "0 180 0"})
    corridor(m, HALL[0], HALL[1], 0, HALL_TOP, "x+", BLACK_FLOOR, panels=HALL_PANELS,
             wall=WHITE_SLAB)
    corridor(m, EXIT[0], EXIT[1], LEDGE_Z, EXIT_TOP, "x-", BLACK_FLOOR, strips=EXIT_STRIPS)


def gameplay(m):
    # Player, gun and the way in.
    m.entity("info_player_start", {"origin": "%d 0 0" % (HALL[0] + 48), "angles": "0 0 0"})
    m.entity("weapon_portalgun", {"origin": "%d 0 16" % (HALL[0] + 112), "angles": "0 0 0",
                                  "CanFirePortal1": "1", "CanFirePortal2": "1"})
    m.brush_entity("trigger_multiple", {"spawnflags": "1", "wait": "1", "StartDisabled": "0"},
                   (HALL[1] - 160, -HALL_HALF, 0), (HALL[1], HALL_HALF, 128), TRIGGER,
                   outputs=[("OnStartTouch", "entry_door", "Open", "", 0)])
    m.brush_entity("trigger_once", {"spawnflags": "1", "StartDisabled": "0"},
                   (X0 + 32, -128, 0), (X0 + 160, 128, 128), TRIGGER,
                   outputs=[("OnStartTouch", "@cube_dropper", "Trigger", "", 0.5),
                            ("OnStartTouch", "entry_door", "Close", "", 1.5)])

    # The cube dropper, hanging from its shaft in the ceiling.
    dx, dy = DROPPER
    m.instance("instances/gameplay/cube_dropper_normal.vmf", (dx, dy, Z1 + 440), (0, 0, 0))

    # The floor button on the ledge; its base sits in the ledge's recess.
    bx, by = BUTTON
    m.instance("instances/buttons/button_pieces/floor_button_base_intact_black.vmf",
               (bx, by, LEDGE_Z - 8))
    pressed = [("exit_door", "Open", "", 0), ("button_glow_blue", "TurnOff", "", 0),
               ("button_glow_orange", "TurnOn", "", 0), ("exit_lights", "TurnOn", "", 0.4),
               ("beacon_spin", "Start", "", 0), ("beacon_light", "TurnOn", "", 0),
               ("beacon_glow", "ShowSprite", "", 0)]
    released = [("exit_door", "Close", "", 0), ("button_glow_orange", "TurnOff", "", 0),
                ("button_glow_blue", "TurnOn", "", 0), ("exit_lights", "TurnOff", "", 0.6),
                ("beacon_spin", "Stop", "", 0), ("beacon_light", "TurnOff", "", 1.5),
                ("beacon_glow", "HideSprite", "", 1.5)]
    m.entity("prop_floor_button", {"targetname": "button", "model": "models/props/portal_button.mdl",
                                   "origin": "%d %d %d" % (bx, by, LEDGE_Z), "angles": "0 0 0"},
             outputs=[("OnPressed",) + o for o in pressed] +
             [("OnUnPressed",) + o for o in released])

    # The emancipation grid across the exit vestibule (its emitters stand
    # against the pillars, so the instance's wall housings are left out).
    m.instance("instances/gameplay/fizzler_black_clean_128x128.vmf",
               (FIZZLER_X, 0, LEDGE_Z), (0, 90, 0), name="exit_fizzler", world=False)

    # The end of the test.
    m.brush_entity("trigger_once", {"spawnflags": "1", "StartDisabled": "0"},
                   (EXIT[1] - 96, -HALL_HALF, LEDGE_Z), (EXIT[1], HALL_HALF, LEDGE_Z + 128),
                   TRIGGER, outputs=[("OnStartTouch", "complete_text", "Display", "", 0)])
    m.entity("game_text", {"targetname": "complete_text", "origin": "%d 0 %d" % (EXIT[1] - 48,
                                                                                LEDGE_Z + 64),
                           "message": "TEST CHAMBER COMPLETE", "x": "-1", "y": "0.35",
                           "channel": "1", "color": "240 240 255", "color2": "255 160 60",
                           "effect": "0", "fadein": "0.5", "fadeout": "1", "holdtime": "4",
                           "spawnflags": "1"})
    for origin in ((-320, 0, 128), (320, 0, LEDGE_Z + 96), (-980, 0, 96), (720, 0, LEDGE_Z + 96)):
        # Built by `buildcubemaps` in a runtime; until then the materials
        # fall back to the default cubemap.
        m.entity("env_cubemap", {"origin": "%d %d %d" % origin, "cubemapsize": "0"})


def baked_lights(m):
    """Lights Cycles bakes into the lightmaps, probe volume and SDF."""
    for x, y in CEILING_PANELS:
        m.instance("instances/lights/light_panel_128_neutral_med.vmf", (x, y, Z1), (90, 0, 0))
    for x in HALL_PANELS:
        m.instance("instances/lights/light_panel_128_cool_med.vmf", (x, 0, HALL_TOP), (90, 0, 0))

    # The observation room, relit warm: tungsten against the cool chamber.
    def tungsten(entity):
        if vi.value(entity, "classname") in ("light", "light_spot"):
            parts = vi.value(entity, "_light").split()
            vi.set_value(entity, "_light", "255 178 110 %s" % parts[3])

    ox, oy, oz = OBSERVATION
    room = m.instance("instances/labs/observation_room_256x128_1.vmf", (ox, oy, oz), (0, 90, 0),
                      edit=tungsten)
    # Its furniture, in the room's frame (x toward the window, z = 0 at the
    # window's center; the floor is at z = -64).
    for model, local, yaw in (("models/props/lab_desk05/lab_desk05.mdl", (-72, -48, -64), 0),
                              ("models/props_office/office_chair_1970.mdl", (-112, -44, -64), 20),
                              ("models/props_office/office_lamp_1970.mdl", (-80, -88, -30), 200)):
        origin = room.point(local)
        m.entity("prop_static", {"model": model, "origin": vi.vector_text(origin),
                                 "angles": "0 %g 0" % (90 + yaw), "solid": "6", "skin": "0",
                                 "disableshadows": "0", "fademindist": "-1"})


def exit_lights(m):
    """The exit corridor's warm panels, dark until the door opens. They are
    light_dynamic, not named `light`s: the relight leaves a switchable world
    light out of the bake, and the runtime light set counts every world light
    as baked, so on the relit world mesh such a light would give only its
    producers' indirect light and no direct light."""
    for x in EXIT_STRIPS:
        m.instance("instances/lights/light_panel_32x128_warm_med.vmf",
                   (x, 0, EXIT_TOP), (90, 0, 0))
        dynamic_light(m, "exit_lights", (x, 0, EXIT_TOP - 12), (7.0, 4.6, 2.6), (40, 75),
                      (90, 0, 0), distance=900)


def dynamic_light(m, name, origin, rgb, cone=None, angles=(0, 0, 0), style=0, parent=None,
                  distance=2048):
    """An inverse-square light_dynamic of linear color `rgb` (the light set's
    unit: rgb at 100 units), a spot when `cone` = (inner, outer) degrees."""
    peak = max(rgb)
    exponent = math.ceil(math.log2(peak)) if peak > 1 else 0
    mantissa = [max(0, min(255, round(c * 255.0 / 2 ** exponent))) for c in rgb]
    # `angles` are engine angles (pitch down positive); the light's own
    # `pitch` key counts up positive (CDynamicLight::KeyValue negates it).
    keys = {"targetname": name, "origin": vi.vector_text(origin),
            "angles": vi.vector_text(angles), "pitch": vi.fmt(-angles[0]),
            # 16: DLIGHT_INVERSE_SQUARE (a physical source).
            "spawnflags": "16", "_light": "%d %d %d 255" % tuple(mantissa),
            "brightness": str(exponent), "distance": str(distance),
            "_inner_cone": str(cone[0] if cone else 0), "_cone": str(cone[1] if cone else 0),
            "spotlight_radius": "0", "style": str(style)}
    if parent:
        keys["parentname"] = parent
    return m.entity("light_dynamic", keys)


def glow(m, name, origin, rgb, scale=0.25, parent=None, on=True):
    keys = {"targetname": name, "origin": vi.vector_text(origin), "model": GLOW,
            "scale": str(scale), "rendermode": "9", "renderamt": "255", "GlowProxySize": "4",
            "rendercolor": "%d %d %d" % tuple(rgb), "spawnflags": "1" if on else "0"}
    if parent:
        keys["parentname"] = parent
    m.entity("env_sprite", keys)


def dynamic_lights(m):
    """The moving and switching light the art direction is built on."""
    spawn = []
    # The observer: a security camera behind the observation window with a
    # cool spot, swept 60 degrees back and forth across the chamber floor.
    ox, oy, oz = OBSERVATION
    pivot = (ox - 40, oy - 36, oz + 44)
    m.brush_entity("func_door_rotating",
                   {"targetname": "observer", "origin": vi.vector_text(pivot),
                    "angles": "0 0 0", "distance": "60", "speed": "9", "wait": "-1",
                    "spawnflags": str(8 | 4096), "rendermode": "10", "disableshadows": "1",
                    "lip": "0", "dmg": "0", "forceclosed": "0", "ignoredebris": "1"},
                   tuple(c - 4 for c in pivot), tuple(c + 4 for c in pivot), NODRAW,
                   outputs=[("OnFullyOpen", "!self", "Close", "", 1.2),
                            ("OnFullyClosed", "!self", "Open", "", 1.2)])
    aim = (32, 90 - 30, 0)          # pitched down, starting at the sweep's west end
    m.entity("prop_dynamic", {"targetname": "observer_camera", "parentname": "observer",
                              "model": "models/props/security_camera.mdl",
                              "origin": vi.vector_text(pivot), "angles": vi.vector_text(aim),
                              "solid": "0", "disableshadows": "1", "DefaultAnim": "idle"})
    lens = (pivot[0] + 14 * 0.47, pivot[1] + 14 * 0.67, pivot[2] - 8)
    dynamic_light(m, "observer_light", lens, (26.0, 30.0, 36.0), (10, 20), aim,
                  parent="observer", distance=3000)
    glow(m, "observer_glow", lens, (210, 225, 255), 0.18, parent="observer")
    spawn.append(("observer", "Open", "", 1.0))

    # Beacons beside the exit: amber spots that spin while the button is down.
    for side, y in (("a", 112), ("b", -112)):
        centre = (370, y, LEDGE_Z + 172)
        m.box((384 - 14, y - 8, LEDGE_Z + 156), (384, y + 8, LEDGE_Z + 164), BLACK_TRIM)
        m.brush_entity("func_rotating",
                       {"targetname": "beacon_spin", "origin": vi.vector_text(centre),
                        "maxspeed": "220", "fanfriction": "35", "spawnflags": str(4 | 64),
                        "rendermode": "10", "disableshadows": "1", "volume": "0"},
                       tuple(c - 3 for c in centre), tuple(c + 3 for c in centre), NODRAW)
        heading = 180 if side == "a" else 0
        dynamic_light(m, "beacon_light", centre, (9.0, 3.6, 0.35), (14, 28), (8, heading, 0),
                      parent="beacon_spin", distance=1400)
        glow(m, "beacon_glow", centre, (255, 150, 30), 0.3, parent="beacon_spin", on=False)
    spawn.append(("beacon_light", "TurnOff", "", 0))

    # The emancipation grid's glow, pulsing (style 5, "gentle pulse").
    dynamic_light(m, "fizzler_glow", (FIZZLER_X - 16, 0, LEDGE_Z + 64), (0.4, 0.8, 1.45),
                  style=5, distance=900)

    # The button's indicator: blue at rest, orange while pressed.
    bx, by = BUTTON
    dynamic_light(m, "button_glow_blue", (bx, by, LEDGE_Z + 40), (0.12, 0.55, 1.4), distance=500)
    dynamic_light(m, "button_glow_orange", (bx, by, LEDGE_Z + 40), (1.6, 0.6, 0.06), distance=500)
    spawn.append(("button_glow_orange", "TurnOff", "", 0))
    spawn.append(("exit_lights", "TurnOff", "", 0))

    m.entity("logic_auto", {"spawnflags": "1", "origin": "%d 0 64" % (HALL[0] + 64)},
             outputs=[("OnMapSpawn",) + o for o in spawn])


def build_vmf(sdk):
    m = Chamber(sdk)
    shell(m)
    doors(m)
    gameplay(m)
    baked_lights(m)
    exit_lights(m)
    dynamic_lights(m)
    return m.text()


# ================================================================ building
def build(out, toolchain_path, runtime, sdk, quality, relight_quality, device=None,
          force_from=None, publish=True):
    """The generator front end: write the VMF, compile it (vmf_map_build.py)
    and hand the BSP to the lighting back end (map_lighting.py) with the scene
    derived from the BSP. A published build is mounted by ./kiln play portal2
    (its kiln package's published-maps mount)."""
    import vmf_map_build

    out.mkdir(parents=True, exist_ok=True)
    vmf = out / (NAME + ".vmf")
    vmf.write_text(build_vmf(sdk))
    tools = Path(json.loads(toolchain_path.read_text())["compile_tools"])
    record = vmf_map_build.build(vmf, out / "compile", tools, runtime, quality=quality, name=NAME)
    if record["status"] != "pass":
        raise SystemExit("%s: compile %s (%s)" % (NAME, record["status"],
                                                  out / "compile" / "build.json"))
    identity = vmf_map_build.light(record, out / "compile", relight_quality, runtime,
                                   toolchain_path, device, publish, force_from)
    vmf_map_build.finish(out / "compile", record)
    return identity


# The review views (feet position; the eye is 64 above) and the states between
# them: the chamber at rest, the observer's sweep over time, then with the
# button held down (the exit opens, its lights come on, the beacons spin).
VIEWS = [
    "do ent_fire @cube_dropper Trigger",
    "wait 240",
    "view hall -1140 0 0 0 0",
    "view overview -700 -420 120 12 32",
    "view dropper -520 -60 40 -18 60",
    "view observation -300 -80 150 -22 250",
    "view ledge 440 380 256 14 215",
    "view vestibule 180 -40 256 4 2",
    "view sweep_a -620 380 180 24 -40",
    "wait 150",
    "view sweep_b -620 380 180 24 -40",
    "wait 150",
    "view sweep_c -620 380 180 24 -40",
    "do ent_fire button PressIn",
    "wait 300",
    "view pressed_vestibule 180 -40 256 4 2",
    "view pressed_overview -700 -420 120 12 32",
    "view pressed_ledge 440 380 256 14 215",
    "view exit_hall 470 0 256 0 0",
]


def capture(out, profile, flavor):
    """Boot the published chamber in Portal 2 and capture the review views."""
    import portal2_map_views
    argv = ["--map", NAME, "--out", str(out), "--profile", profile, "--flavor", flavor]
    for step in VIEWS:
        argv += ["--step", step]
    sys.argv = ["portal2_map_views.py"] + argv
    return portal2_map_views.main()


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--out", type=Path, default=ROOT / "quality-results/portal2-maps" / NAME)
    parser.add_argument("--toolchain", type=Path, default=TOOLCHAIN)
    parser.add_argument("--runtime", type=Path,
                        help="Portal 2 runtime the content comes from (default: the portal2 "
                             "profile's packaged runtime)")
    parser.add_argument("--sdk", type=Path, default=STEAM_ROOT / "sdk_content/maps",
                        help="the Portal 2 SDK's maps directory (holds instances/)")
    parser.add_argument("--quality", choices=("fast", "full"), default="full",
                        help="vbsp/vvis/vrad quality")
    parser.add_argument("--relight-quality", default="source2",
                        help="production map profile (source2)")
    parser.add_argument("--device", help="Cycles device override (cpu, gpu, auto)")
    parser.add_argument("--from", dest="force_from", help="force this relight step and later")
    parser.add_argument("--no-publish", action="store_true")
    parser.add_argument("--vmf-only", action="store_true", help="write the VMF and stop")
    parser.add_argument("--capture", type=Path, metavar="DIR",
                        help="only boot the published map and capture the review views")
    parser.add_argument("--profile", default="portal2", help="kiln profile for --capture")
    parser.add_argument("--flavor", default="dev", help="its build flavor")
    args = parser.parse_args()
    if args.capture:
        return capture(args.capture.resolve(), args.profile, args.flavor)
    out = args.out.resolve()
    if not (args.sdk / "instances").is_dir():
        parser.error("no Portal 2 SDK instances under %s (set --sdk or P2_STEAM_ROOT)" % args.sdk)
    if args.vmf_only:
        out.mkdir(parents=True, exist_ok=True)
        (out / (NAME + ".vmf")).write_text(build_vmf(args.sdk))
        print("wrote " + str(out / (NAME + ".vmf")))
        return 0
    if args.runtime is None:
        import vmf_map_build
        args.runtime = vmf_map_build.sepipe_loader.packaged_runtime("portal2")
    build(out, args.toolchain.resolve(), args.runtime.resolve(), args.sdk, args.quality,
          args.relight_quality, args.device, args.force_from, not args.no_publish)
    return 0


if __name__ == "__main__":
    sys.exit(main())
