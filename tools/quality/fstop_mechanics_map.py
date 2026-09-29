#!/usr/bin/env python3
"""Build the F-Stop mechanics map, fstop_mechanics, through the Hammer MCP server.

The map is a shopping mall of F-Stop mechanics: a central atrium where the
player spawns, a concourse running east and west through it, twenty shops
along the concourse and an anchor store at each end. Each shop shows off one
mechanic ported into the fstop product (game/server/fstop, FSTOP-guarded base
code), with its entities' DATADESC keyfields and inputs set up so each one
does what its code does. Every shop front is a mall entrance: glass display
windows (Portal's chamber glass) either side of a Portal 2 test chamber door
(retail Portal 2's portal_door_combined, staged by
tools/quality/stage_fstop_runtime.py), which a player trigger opens
(OnStartTouch) and closes once nobody is in it (OnEndTouchAll). Invisible
sliding leaves (func_door) carry the door's collision. Shops with something
that shoots or hunts the player (the hover turret, the androids, the
dispenser, the tombstone's zombies) have solid fronts.

The look is Valve's F-Stop-era Portal 2 test chambers (Steam2 depot 852
version 0): white wall, floor and ceiling tiles, dark metal trim, light strips,
and signage over each door (the F-Stop camera and size manuals and chicken
sign, Portal's chamber icons). Each shop also carries a point_fstop_label.

The shell is built room by room (each room has its own floor, ceiling and back
wall) so every textured face can be fitted to the part of it that shows: a
whole number of texture repeats per face, starting at the face's edge, sized
from the texture's VTF (fit_texture). Signs are fitted once.

A portal tunnel pair links the tunnel shop to the atrium: prop_portal_tunnel on
the shop's west wall builds its partner at its success target, on the atrium's
north wall.

West wing, north side (atrium outward): camera, size, reflect, swap, levitator
West wing, south side:                  photo lab, geyser, air vent,
                                        mousetrap, monopole
East wing, north side:                  resizable portals, portal tunnel,
                                        laser, personality sphere, tombstone
East wing, south side:                  chicken, companion bots, hover turret,
                                        androids, dispenser
Anchor stores:                          linked portal doors (west),
                                        blobs (east)

tools/quality/fstop_mechanics_check.py boots the map headless and checks the
mechanics; every output it judges is wired to a probe.<entity>.<output>
logic_relay, and its scenarios place the player with setpos(room, dx, dy).

The map is authored entirely with Hammer MCP tool calls (hammer_cli --mcp,
RFC 0002 R08-MCP): create_block, tie_to_entity, place_entity, set_key,
add_output, check_map, then save and build_map, which compiles it with
tools/quality/vmf_map_build.py against the staged F-Stop runtime (Portal and
HL2 plus the depot's materials) and installs it into the F-Stop game directory
(run/runtime-fstop/fstop/maps), which the Portal map store is not mounted in.

    python3 tools/quality/fstop_mechanics_map.py          author, build and install
    python3 tools/quality/fstop_mechanics_map.py --vmf-only
    ./play_fstop +map fstop_mechanics
"""

import argparse
import json
import re
import struct
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

from source_content import ContentResolver  # noqa: E402

ROOT = HERE.parents[1]
CLI = ROOT / "build-r03-tools/hammer/cli/hammer_cli"
RUNTIME = ROOT / "run/runtime-fstop"
GAME_DIR = RUNTIME / "fstop"
NAME = "fstop_mechanics"

# F-Stop-era Portal 2 materials (depot 852, mounted as fstop_valve), then
# Portal's and HL2's.
SHOP_WALL = "TILE/WHITE_WALL_TILE003A"
SHOP_FLOOR = "TILE/WHITE_FLOOR_TILE002A"
CEILING = "TILE/WHITE_CEILING_TILE002A"
CONCOURSE_FLOOR = "TILE/FLOOR_TILE_002A"
ATRIUM_FLOOR = "TILE/WHITE_FLOOR_TILE004A"
OUTER_WALL = SHOP_WALL
FRONT = SHOP_WALL
TRIM = "METAL/BLACK_WALL_METAL_002B"
GLASS = "GLASS/GLASSWINDOW_REFRACT02"
NODRAW = "TOOLS/TOOLSNODRAW"
DOOR_MODEL = "models/props/portal_door_combined.mdl"
LIGHT_STRIP = "LIGHTS/WHITE008"
PLANTER = "METAL/BLACK_FLOOR_METAL_001A"
PEDESTAL = "METAL/NONPORTABLE_WALL001A"
BUTTON = "DEV/DEV_MEASURESWITCH01"
FIELD_LOOK = "EFFECTS/PORTAL_CLEANSER"
TRIGGER = "TOOLS/TOOLSTRIGGER"
DIRT = "NATURE/DIRTFLOOR003A"

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

# The building: interior x -3712..3712, y -848..848, z 0..384.
INNER = (-3712, -848, 0, 3712, 848, 384)
T = 16
CONCOURSE_Y = 192       # the concourse is y -192..192; shop fronts are 16 thick
ATRIUM_X = 504          # the atrium is x -504..504, the whole depth
ANCHOR_X = 3064         # anchor stores' fronts face the concourse ends
ANCHOR_HALF = 448       # anchor stores are y -448..448
SHOP_WIDTH = 512
# Portal 2's door cutout: Valve's p2editor instance door_frame_white.vmf (depot
# 841 sdk_content), a 128x128 panel 24 deep with the door's arched opening, white
# tile on its face and a nodraw ring (+-88) that sits inside the wall. Valve's
# door_exit.vmf pairs it with the door: prop_testchamber_door at the panel's
# bottom centre, 13 units behind the white face, facing out of it.
DOOR_FRAME = ROOT / ("external/portal2_steam2_maps/841_24/sdk_content/maps/instances/"
                     "p2editor/door_frame_white.vmf")
DOOR_OPENING = 64       # the panel's half width and the opening's
DOOR_HEIGHT = 128
DOOR_DEPTH = 13         # the door's origin behind the frame's face
FRAME_DEPTH = 24
DOOR_WALL = 112         # the door's wall section: two frames deep, +-112 wide
WINDOW = 232            # display windows run from the door post to here

# Shop name -> (wing x of its atrium-side edge sign, side, index from the atrium).
# The wings run from x = +-512 outward; north shops open to -y, south to +y.
SHOPS = {
    "camera": (-1, 1, 0), "size": (-1, 1, 1), "reflect": (-1, 1, 2), "swap": (-1, 1, 3),
    "levitator": (-1, 1, 4),
    "photo_lab": (-1, -1, 0), "geyser": (-1, -1, 1), "air_vent": (-1, -1, 2),
    "mousetrap": (-1, -1, 3), "monopole": (-1, -1, 4),
    "portals": (1, 1, 0), "tunnel": (1, 1, 1), "laser": (1, 1, 2), "sphere": (1, 1, 3),
    "tombstone": (1, 1, 4),
    "chicken": (1, -1, 0), "bots": (1, -1, 1), "turret": (1, -1, 2), "androids": (1, -1, 3),
    "dispenser": (1, -1, 4),
}
ANCHORS = {"linked_doors": -1, "blobs": 1}
TUNNEL_Z = 60           # a tunnel's portal (56 half-height) is centred on its origin
TUNNEL_EXIT = (200, INNER[4] - 8, TUNNEL_Z)
TITLES = {
    "camera": "Camera + placement", "size": "Object size", "reflect": "Reflect",
    "swap": "Swap", "levitator": "Levitator", "photo_lab": "Photo lab: eraser, clip, photos",
    "geyser": "Geyser", "air_vent": "Air vent", "mousetrap": "Mousetrap",
    "monopole": "Monopole", "portals": "Resizable portals", "tunnel": "Portal tunnel",
    "laser": "Laser", "sphere": "Personality sphere", "tombstone": "Tombstone",
    "chicken": "Chicken + nest", "bots": "Companion bots", "turret": "Hover turret",
    "androids": "Androids + add-ons", "dispenser": "Android dispenser",
    "linked_doors": "Linked portal doors", "blobs": "Blobs",
}
# F-Stop's signs (depot 852) and Portal's chamber icons.
SIGN_MATERIAL = {
    "camera": "SIGNAGE/CAMERA_MANUAL", "photo_lab": "SIGNAGE/CAMERA_MANUAL",
    "size": "SIGNAGE/SIZE_MANUAL", "chicken": "SIGNAGE/CHICKEN_SIGN",
    "reflect": "SIGNAGE/SIGNAGE_OVERLAY_DOTS1", "swap": "SIGNAGE/SIGNAGE_OVERLAY_DOTS2",
    "levitator": "SIGNAGE/SIGNAGE_OVERLAY_MIDAIR1", "geyser": "SIGNAGE/SIGNAGE_OVERLAY_FOUNTAIN",
    "air_vent": "SIGNAGE/SIGNAGE_OVERLAY_FLING1", "mousetrap": "SIGNAGE/SIGNAGE_OVERLAY_BOXHURT",
    "monopole": "SIGNAGE/SIGNAGE_OVERLAY_CATCHER", "portals": "SIGNAGE/SIGNAGE_OVERLAY_DOTS3",
    "tunnel": "SIGNAGE/SIGNAGE_OVERLAY_FLING2", "laser": "SIGNAGE/SIGNAGE_OVERLAY_ENERGYBALL",
    "sphere": "SIGNAGE/SIGNAGE_OVERLAY_CAKE", "tombstone": "SIGNAGE/SIGNAGE_OVERLAY_TOXIC",
    "bots": "SIGNAGE/SIGNAGE_OVERLAY_COMPANIONCUBE", "turret": "SIGNAGE/SIGNAGE_OVERLAY_TURRET",
    "androids": "SIGNAGE/SIGNAGE_OVERLAY_MIDAIR2",
    "dispenser": "SIGNAGE/SIGNAGE_OVERLAY_BOXDISPENSER",
    "linked_doors": "SIGNAGE/SIGNAGE_OVERLAY_DOTS4", "blobs": "SIGNAGE/SIGNAGE_OVERLAY_LAB01",
}
OPAQUE = {"turret", "androids", "dispenser", "tombstone"}
NPC_ROOMS = {"chicken", "bots", "turret", "androids", "dispenser", "blobs"}


def room_center(room):
    """World (x, y) of a room's floor center."""
    if room in ANCHORS:
        return ANCHORS[room] * (INNER[3] + ANCHOR_X + T) / 2.0, 0.0
    wing, side, index = SHOPS[room]
    x = wing * (ATRIUM_X + 8 + SHOP_WIDTH * index + SHOP_WIDTH / 2.0)
    return x, side * (CONCOURSE_Y + T + INNER[4]) / 2.0


def room_half(room):
    """Half extents (x, y) of a room's floor."""
    if room in ANCHORS:
        return (INNER[3] - ANCHOR_X - T) / 2.0, ANCHOR_HALF
    return SHOP_WIDTH / 2.0 - 8, (INNER[4] - CONCOURSE_Y - T) / 2.0


def at(room, dx, dy, z=0):
    """A point in a room: its floor center moved by (dx, dy), at height z."""
    x, y = room_center(room)
    return (x + dx, y + dy, z)


def setpos(room, dx, dy, z=8):
    """Console text that puts the player at (dx, dy) in a room."""
    return "setpos %g %g %g" % at(room, dx, dy, z)


def vec(v):
    return " ".join("%g" % c for c in v)


# --- texture fitting ----------------------------------------------------------

# A new block's faces in the codec's order (+z, -z, -x, +x, -y, +y), each with its
# world-aligned texture axes as (world axis, sign) for u and v.
FACE_AXES = (((0, 1), (1, -1)), ((0, 1), (1, -1)), ((1, 1), (2, -1)), ((1, 1), (2, -1)),
             ((0, 1), (2, -1)), ((0, 1), (2, -1)))
TEXEL = 0.25            # world units per texel at the materials' authored scale
_TEXTURE_SIZES = {}
_RESOLVER = []


def texture_size(material):
    """The (width, height) in texels of a material's base texture, from its VTF."""
    if material not in _TEXTURE_SIZES:
        if not _RESOLVER:
            _RESOLVER.append(ContentResolver(str(RUNTIME)))
        size = (512, 512)
        vmt, _ = _RESOLVER[0].read("materials/%s.vmt" % material.lower())
        if vmt:
            text = vmt.decode("utf-8", "replace")
            for key in ("$basetexture", "$normalmap", "$dudvmap"):
                match = re.search(r'"?%s"?\s+"?([^"\s]+)' % re.escape(key), text, re.I)
                vtf = match and _RESOLVER[0].read(
                    "materials/%s.vtf" % match.group(1).replace("\\", "/").lower())[0]
                if vtf and vtf[:4] == b"VTF\0":
                    size = struct.unpack_from("<HH", vtf, 16)
                    break
        _TEXTURE_SIZES[material] = size
    return _TEXTURE_SIZES[material]


def fit_axis(lo, hi, axis, sign, texels, repeats=None):
    """(scale, shift) that put a whole number of texture repeats across [lo, hi] on
    a world axis, starting at the face's edge. A face much narrower than the texture
    keeps the authored scale, aligned at its edge."""
    length = hi[axis] - lo[axis]
    start = lo[axis] if sign > 0 else -hi[axis]
    n = repeats or round(length / (texels * TEXEL))
    scale = length / (n * texels) if n >= 1 else TEXEL
    return scale, (-start / scale) % texels


class HammerMcp:
    """An MCP client of hammer_cli --mcp: newline-delimited JSON-RPC on stdio."""

    def __init__(self, cli, root, builds, runtime, game_dir, log):
        argv = [str(cli), "--mcp", "--root", str(root), "--repo", str(ROOT),
                "--builds", str(builds)]
        # build_map compiles against the F-Stop runtime and installs the map.
        if runtime:
            argv += ["--runtime", str(runtime)]
        if game_dir:
            argv += ["--install-game-dir", str(game_dir)]
        self.proc = subprocess.Popen(argv, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                                     stderr=log, text=True, bufsize=1)
        self.next_id = 0
        self.calls = 0
        reply = self.request("initialize", {"protocolVersion": "2024-11-05", "capabilities": {},
                                            "clientInfo": {"name": NAME, "version": "1"}})
        if "result" not in reply:
            raise RuntimeError("initialize failed: %r" % reply)
        self.proc.stdin.write(json.dumps({"jsonrpc": "2.0",
                                          "method": "notifications/initialized"}) + "\n")
        self.proc.stdin.flush()

    def request(self, method, params=None):
        self.next_id += 1
        message = {"jsonrpc": "2.0", "id": self.next_id, "method": method}
        if params is not None:
            message["params"] = params
        self.proc.stdin.write(json.dumps(message) + "\n")
        self.proc.stdin.flush()
        line = self.proc.stdout.readline()
        if not line:
            raise RuntimeError("hammer_cli closed the stream during " + method)
        reply = json.loads(line)
        if reply.get("id") != self.next_id:
            raise RuntimeError("reply id %r for request %d" % (reply.get("id"), self.next_id))
        return reply

    def __call__(self, tool, **arguments):
        """Call one tool; returns its text output, raises on an error result."""
        self.calls += 1
        args = {k: (vec(v) if isinstance(v, tuple) else str(v)) for k, v in arguments.items()}
        reply = self.request("tools/call", {"name": tool, "arguments": args})
        if "error" in reply:
            raise RuntimeError("%s %r: %s" % (tool, args, reply["error"]))
        result = reply["result"]
        text = "".join(c.get("text", "") for c in result.get("content", []))
        if result.get("isError"):
            raise RuntimeError("%s %r: %s" % (tool, args, text))
        return text.strip()

    def close(self):
        self.proc.stdin.close()
        return self.proc.wait(timeout=60)


class Mall:
    """Authoring helpers over the MCP tools."""

    def __init__(self, mcp):
        self.mcp = mcp
        self.probes = []

    def block(self, lo, hi, material, uv=None, sign_face=None):
        """A box solid with its textures fitted to `uv` (default: the box), the part
        of it that shows. With sign_face (a FACE_AXES index), that face shows the
        material once and the others are nodraw."""
        lo, hi = tuple(map(min, lo, hi)), tuple(map(max, lo, hi))
        solid = self.mcp("create_block", mins=lo, maxs=hi, material=material)
        if material.startswith("TOOLS/"):
            return solid
        uv_lo, uv_hi = (lo, hi) if uv is None else (tuple(map(min, *uv)), tuple(map(max, *uv)))
        vmfid = int(re.search(r"vmfid=(\d+)", self.mcp("describe", id=solid)).group(1))
        width, height = texture_size(material)
        for k, ((ua, us), (va, vs)) in enumerate(FACE_AXES):
            face = "%s:%d" % (solid, vmfid + 1 + k)
            if sign_face is not None and k != sign_face:
                self.mcp("apply_material", material=NODRAW, faces=face)
                continue
            once = 1 if sign_face is not None else None
            su, hu = fit_axis(uv_lo, uv_hi, ua, us, width, once)
            sv, hv = fit_axis(uv_lo, uv_hi, va, vs, height, once)
            self.mcp("set_texture", faces=face, scale_u="%.6g" % su, scale_v="%.6g" % sv,
                     shift_u="%.6g" % hu, shift_v="%.6g" % hv)
        return solid

    def keys(self, entity, keyvalues, outputs=()):
        for key, value in keyvalues.items():
            self.mcp("set_key", key=key, value=value, ids=entity)
        for output, target, inp, parameter, delay in outputs:
            self.mcp("add_output", output=output, target=target, input=inp,
                     parameter=parameter, delay=delay, ids=entity)
        return entity

    def point(self, classname, origin, keyvalues=None, outputs=()):
        entity = self.mcp("place_entity", classname=classname, origin=tuple(origin))
        return self.keys(entity, keyvalues or {}, outputs)

    def brush_entity(self, classname, boxes, keyvalues=None, outputs=()):
        solids = [self.block(lo, hi, material) for lo, hi, material in boxes]
        entity = self.mcp("tie_to_entity", classname=classname, ids=" ".join(solids))
        return self.keys(entity, keyvalues or {}, outputs)

    def prop(self, classname, origin, keyvalues=None, angles=(0, 0, 0), outputs=()):
        """An entity of the mechanics, labelled in the world with its name and class."""
        kv = {"angles": vec(angles)}
        kv.update(keyvalues or {})
        self.point(classname, origin, kv, outputs)
        if classname.startswith(("npc_", "prop_", "weapon_")) and kv.get("targetname"):
            self.point("point_fstop_label", origin,
                       {"label_target": kv["targetname"], "offset": "0 0 72",
                        "label": "%s [%s]" % (kv["targetname"], classname)})

    def probed(self, entity, *names):
        """Outputs of entity wired to their probe relays (tools/quality/fstop_mechanics_check.py
        watches them; developer 2 logs each firing)."""
        self.probes.extend("probe.%s.%s" % (entity, name) for name in names)
        return [(name, "probe.%s.%s" % (entity, name), "Trigger", "", 0) for name in names]

    def load_door_frame(self, work):
        """Copy Valve's door frame instance to the clipboard, turned upright with its
        white face on y = 0 facing -y and the opening's bottom centre at the origin.
        Replaces the document; call it before new_map."""
        text = DOOR_FRAME.read_text(errors="replace")
        # The codec reads no quickhide block (Hammer's quick-hide state).
        (work / "door_frame_white.vmf").write_text(
            re.sub(r"quickhide\s*\{[^{}]*\}\s*", "", text))
        self.mcp("open", path="door_frame_white.vmf")
        self.mcp("unhide_all")
        self.mcp("set_texture_lock", on="1")
        self.mcp("set_granularity", mode="objects")
        self.mcp("select_all")
        # Instance space: the white face is z = -64 facing +z, up is +x.
        self.mcp("rotate", axis="1", degrees="-90", pivot=(0, 0, 0))
        self.mcp("rotate", axis="2", degrees="90", pivot=(0, 0, 0))
        self.mcp("move", delta=(0, -DOOR_OPENING, DOOR_OPENING))
        self.mcp("copy")

    def door_frame(self, origin, turn):
        """Paste the door frame with its face's bottom centre at origin, turned about z."""
        ids = self.mcp("paste")
        if turn % 360:
            self.mcp("rotate", axis="2", degrees=str(turn % 360), pivot=(0, 0, 0), ids=ids)
        self.mcp("move", delta=tuple(origin), ids=ids)

    def light(self, origin, color="255 250 240 160"):
        self.point("light", origin, {"_light": color, "_constant_attn": "0",
                                     "_linear_attn": "1", "_quadratic_attn": "0"})

    def physics(self, origin, model, keyvalues=None, outputs=()):
        kv = {"model": model, "spawnflags": "256"}
        kv.update(keyvalues or {})
        classname = "prop_physics_override" if model in NO_PROPDATA else "prop_physics"
        self.prop(classname, origin, kv, outputs=outputs)

    def button(self, name, center, outputs):
        x, y, z = center
        self.block((x - 12, y - 12, 0), (x + 12, y + 12, z - 8), PEDESTAL)
        self.brush_entity("func_button", [((x - 8, y - 8, z - 8), (x + 8, y + 8, z + 8), BUTTON)],
                          {"targetname": name, "spawnflags": "1025", "speed": "5", "wait": "1",
                           "lip": "0", "sounds": "0", "movedir": "0 0 0", "renderamt": "255"},
                          outputs)

    def node_grid(self, room, step=96, margin=48):
        hx, hy = room_half(room)
        nx, ny = int((2 * (hx - margin)) // step), int((2 * (hy - margin)) // step)
        for i in range(nx + 1):
            for j in range(ny + 1):
                self.point("info_node", at(room, -hx + margin + i * step,
                                           -hy + margin + j * step, 8))


# --- the building -------------------------------------------------------------


def room_rect(room):
    """A room's floor rectangle (x0, y0, x1, y1)."""
    if room == "atrium":
        return (-ATRIUM_X, INNER[1], ATRIUM_X, INNER[4])
    if room in ("concourse_west", "concourse_east"):
        wing = -1 if room.endswith("west") else 1
        xs = sorted((wing * ATRIUM_X, wing * ANCHOR_X))
        return (xs[0], -CONCOURSE_Y, xs[1], CONCOURSE_Y)
    (x, y), (hx, hy) = room_center(room), room_half(room)
    return (x - hx, y - hy, x + hx, y + hy)


def shell(m):
    """Every room's floor, ceiling and outer wall, the walls between shops and the
    solid behind the anchor stores; the shop fronts are storefront()'s."""
    x0, y0, z0, x1, y1, z1 = INNER
    rooms = list(SHOPS) + list(ANCHORS) + ["atrium", "concourse_west", "concourse_east"]
    for room in rooms:
        rx0, ry0, rx1, ry1 = room_rect(room)
        floor = (ATRIUM_FLOOR if room == "atrium" else
                 CONCOURSE_FLOOR if room.startswith("concourse") else SHOP_FLOOR)
        m.block((rx0, ry0, z0 - T), (rx1, ry1, z0), floor)
        m.block((rx0, ry0, z1), (rx1, ry1, z1 + T), CEILING)
    # Outer walls behind the shops and the atrium, one per room.
    for room in list(SHOPS) + ["atrium"]:
        rx0, _, rx1, _ = room_rect(room)
        for side in ((1, -1) if room == "atrium" else (SHOPS[room][1],)):
            wall = sorted((side * y1, side * (y1 + T)))
            m.block((rx0 - 8, wall[0], z0 - T), (rx1 + 8, wall[1], z1 + T), OUTER_WALL,
                    uv=((rx0, wall[0], z0), (rx1, wall[1], z1)))
    for side in (1, -1):
        # The anchor store's end wall, and the solid beside it (behind the end shops).
        end = sorted((side * x1, side * (x1 + T)))
        m.block((end[0], -ANCHOR_HALF - T, z0 - T), (end[1], ANCHOR_HALF + T, z1 + T), OUTER_WALL,
                uv=((end[0], -ANCHOR_HALF, z0), (end[1], ANCHOR_HALF, z1)))
        for sy in (1, -1):
            xs = sorted((side * (x1 + T), side * ANCHOR_X))
            ys = sorted((sy * ANCHOR_HALF, sy * (y1 + T)))
            m.block((xs[0], ys[0], z0 - T), (xs[1], ys[1], z1 + T), OUTER_WALL,
                    uv=((xs[0], ys[0], z0), (xs[1], ys[1], z1)))
    # Walls between shops, the atrium's edges included.
    for wing in (1, -1):
        for i in range(0, 5):
            x = wing * (ATRIUM_X + 8 + SHOP_WIDTH * i)
            for side in (1, -1):
                ys = sorted((side * CONCOURSE_Y, side * y1))
                shown = sorted((side * (CONCOURSE_Y + T), side * y1))
                m.block((x - 8, ys[0], z0 - T), (x + 8, ys[1], z1 + T), SHOP_WALL,
                        uv=((x - 8, shown[0], z0), (x + 8, shown[1], z1)))
    # Light strips down the concourse and round the atrium.
    for x in range(-2816, 2817, 512):
        m.block((x - 96, -24, 376), (x + 96, 24, z1), LIGHT_STRIP)
        m.light((x, 0, 340))
    for x in (-320, 320):
        for y in (-560, 560):
            m.block((x - 24, y - 96, 376), (x + 24, y + 96, z1), LIGHT_STRIP)
            m.light((x, y, 340))


def face_index(direction):
    """The FACE_AXES index of a block face with this outward normal."""
    axis = max(range(3), key=lambda i: abs(direction[i]))
    return {0: (2, 3), 1: (4, 5), 2: (1, 0)}[axis][direction[axis] > 0]


def storefront(m, room, index):
    """The front of a shop: a Portal 2 test chamber door opened by a player trigger,
    display windows either side, a sign and a label over the door."""
    if room in ANCHORS:
        wing = ANCHORS[room]
        half = ANCHOR_HALF

        # u runs along the front, v from the concourse face into the store.
        def world(u, v, z):
            return (wing * (ANCHOR_X + v), -wing * u, z)
    else:
        _, side, _ = SHOPS[room]
        cx, _ = room_center(room)
        half = SHOP_WIDTH / 2.0 - 8

        def world(u, v, z):
            return (cx + u, side * (CONCOURSE_Y + v), z)

    def box(u0, u1, v0, v1, z0, z1):
        return world(u0, v0, z0), world(u1, v1, z1)

    def along(u, v):
        a, b = world(0, 0, 0), world(u, v, 0)
        return tuple(round(q - p) for p, q in zip(a, b))

    top, bottom = INNER[5] + T, -T
    w, d, h = WINDOW, DOOR_WALL, DOOR_HEIGHT
    o, deep = DOOR_OPENING, 2 * FRAME_DEPTH
    for s in (-1, 1):
        m.block(*box(s * w, s * half, 0, T, bottom, top), FRONT, uv=box(s * w, s * half, 0, T, 0,
                                                                         INNER[5]))
        m.block(*box(s * (d + 8), s * w, 0, T, bottom, 16), TRIM,
                uv=box(s * (d + 8), s * w, 0, T, 0, 16))
        if room in OPAQUE:
            m.block(*box(s * (d + 8), s * w, 0, T, 16, h), FRONT)
        else:
            m.block(*box(s * (d + 8), s * w, 6, 10, 16, h), GLASS)
        m.block(*box(s * d, s * (d + 8), 0, T, bottom, h), TRIM, uv=box(s * d, s * (d + 8), 0, T,
                                                                        0, h))
        # The door's wall section beside the opening, two frames deep.
        m.block(*box(s * o, s * d, 0, deep, bottom, h), FRONT, uv=box(s * o, s * d, 0, deep, 0, h))
    m.block(*box(-w, w, 0, T, h, top), FRONT, uv=box(-w, w, 0, T, h, INNER[5]))
    m.block(*box(-d, d, 0, deep, h, top), FRONT, uv=box(-d, d, 0, deep, h, INNER[5]))
    m.block(*box(-o, o, 0, deep, bottom, 0), TRIM)
    # Valve's door frame in the opening: one facing the concourse, one the shop.
    concourse = along(0, -1)
    turn = {(0, -1): 0, (1, 0): 90, (0, 1): 180, (-1, 0): 270}[(concourse[0], concourse[1])]
    m.door_frame(world(0, 0, 0), turn)
    m.door_frame(world(0, deep, 0), turn + 180)
    # The sign over the door, shown once on the concourse face.
    m.block(*box(-72, 72, -2, 0, 170, 314), SIGN_MATERIAL[room],
            sign_face=face_index(along(0, -1)))
    # The door faces the concourse, as Valve's faces out of the frame; invisible
    # sliding leaves carry its collision.
    yaw = {(1, 0): 0, (0, 1): 90, (-1, 0): 180, (0, -1): 270}[(concourse[0], concourse[1])]
    door = "door.%s.prop" % room
    m.point("prop_dynamic", world(0, DOOR_DEPTH, 0),
            {"targetname": door, "model": DOOR_MODEL, "angles": "0 %d 0" % yaw, "solid": "0",
             "DefaultAnim": "idleclose", "HoldAnimation": "1", "disableshadows": "1"})
    leaves = []
    for s in (-1, 1):
        name = "door.%s.%s" % (room, "left" if s < 0 else "right")
        leaves.append(name)
        slide = along(s, 0)
        m.brush_entity("func_door", [(*box(0, s * o, DOOR_DEPTH - 4, DOOR_DEPTH + 4, 0, h),
                                      NODRAW)],
                       {"targetname": name,
                        "movedir": "0 %d 0" % {(1, 0): 0, (0, 1): 90, (-1, 0): 180,
                                               (0, -1): 270}[(slide[0], slide[1])],
                        "speed": "200", "wait": "-1", "lip": "4", "spawnflags": "0",
                        "dmg": "0", "forceclosed": "0", "rendermode": "10"},
                       m.probed(name, "OnFullyOpen", "OnFullyClosed"))
    opens = [("OnStartTouch", leaf, "Open", "", 0) for leaf in leaves] + [
        ("OnStartTouch", door, "SetAnimation", "open", 0)]
    closes = [("OnEndTouchAll", leaf, "Close", "", 0) for leaf in leaves] + [
        ("OnEndTouchAll", door, "SetAnimation", "close", 0)]
    m.brush_entity("trigger_multiple", [(*box(-96, 96, -128, deep + 128, 0, h), TRIGGER)],
                   {"targetname": "doortrigger." + room, "spawnflags": "1", "wait": "0.1",
                    "StartDisabled": "0"}, opens + closes)
    sign_at = world(0, -8, 330)
    m.point("info_target", sign_at, {"targetname": "sign." + room})
    m.point("point_fstop_label", sign_at, {"label_target": "sign." + room, "offset": "0 0 0",
                                           "label": TITLES[room]})
    # Room lights.
    hx, hy = room_half(room)
    for dx in (-hx / 2, hx / 2):
        m.light(at(room, dx, 0, 320))
        m.block(at(room, dx - 24, -96, 376), at(room, dx + 24, 96, INNER[5]), LIGHT_STRIP)


def atrium(m):
    """The mall's court: the player starts here, by a planter."""
    m.point("info_player_start", (-300, 0, 8), {"angles": "0 180 0"})
    planter = m.mcp("create_primitive", kind="cylinder", mins=(-96, -96, 0), maxs=(96, 96, 24),
                    sides="16", axis="2", material=PLANTER)
    m.point("point_fstop_label", (0, 0, 8), {"label_target": "mall.directory", "offset": "0 0 96",
                                             "label": "F-Stop mall: one mechanic per shop"})
    m.point("info_target", (0, 0, 8), {"targetname": "mall.directory"})
    # The portal tunnel's far end: its partner is built here, on the north wall.
    m.prop("info_target", TUNNEL_EXIT, {"targetname": "tunnel_success"}, angles=(0, 270, 0))
    m.point("point_fstop_label", TUNNEL_EXIT, {"label_target": "tunnel_success",
                                               "offset": "0 0 80",
                                               "label": "Portal tunnel (from the tunnel shop)"})
    return planter


# --- the shops ----------------------------------------------------------------


def camera(m):
    r = "camera"
    for name, cls, dy in (("camera", "weapon_camera", 180), ("placement", "weapon_placement", 260)):
        x, y, _ = at(r, -200, dy)
        m.block((x - 16, y - 16, 0), (x + 16, y + 16, 32), PEDESTAL)
        kv = {"targetname": name}
        if cls == "weapon_camera":
            # One photo, which is all the inventory holds; scaling and zoom on.
            kv.update({"captureslots": "1", "canscale": "1", "canzoom": "1"})
        m.prop(cls, (x, y, 48), kv)
    # Capturable physics props at the three object scale levels: the objects of
    # Valve's camera training photos and an F-Stop instruction manual scrap.
    capturables = (((-40, 0), BARREL, "0"), ((80, 120), CRATE, "1"), ((180, 120), TIRE, "-1"),
                   ((80, -100), FAN, "0"), ((180, -100), MANUAL_SCRAP, "-1"))
    for i, ((dx, dy), model, scale) in enumerate(capturables):
        name = "capturable_%d" % i
        m.physics(at(r, dx, dy, 24), model, {"targetname": name, "canbecaptured": "1",
                                             "scalevalue": scale},
                  outputs=m.probed(name, "OnCameraCapture", "OnCameraRelease", "OnFizzled"))
    m.physics(at(r, 180, 0, 24), RADIO, {"targetname": "not_capturable", "canbecaptured": "0"})
    # Placement helpers: a free one and a forced, size-limited one for cubes.
    m.prop("info_placement_helper", at(r, 0, -180, 8),
           {"targetname": "helper_free", "radius": "48", "StartDisabled": "0"},
           outputs=m.probed("helper_free", "OnObjectPlaced"))
    m.prop("info_placement_helper", at(r, 150, 250, 8),
           {"targetname": "helper_cube", "radius": "64", "force_placement": "1",
            "snap_to_helper_angles": "1", "usesizelimit": "1", "target_size": "0",
            "target_classname": "prop_physics", "StartDisabled": "0"},
           outputs=m.probed("helper_cube", "OnObjectPlaced", "OnObjectPlacedSize"))
    m.prop("env_dof_controller", at(r, 0, 0, 64),
           {"targetname": "dof", "enabled": "1", "near_blur": "20", "near_focus": "60",
            "far_focus": "300", "far_blur": "900", "near_radius": "0", "far_radius": "5"})


def photo_lab(m):
    r = "photo_lab"
    # A capturable to carry through the eraser field.
    m.physics(at(r, -100, -100, 24), BARREL,
              {"targetname": "capturable_lab", "canbecaptured": "1", "scalevalue": "0"},
              outputs=m.probed("capturable_lab", "OnCameraCapture", "OnCameraRelease",
                               "OnFizzled"))
    # The eraser field: walking through it with a photo erases the photo and the
    # object returns. Drawn as a fizzler; the trigger is the entity.
    field = (at(r, 40, -312, 0), at(r, 56, 150, 128))
    m.brush_entity("func_brush", [(field[0], field[1], FIELD_LOOK)],
                   {"targetname": "eraser_look", "Solidity": "1", "rendermode": "0"})
    m.brush_entity("trigger_photo_eraser", [(field[0], field[1], TRIGGER)],
                   {"targetname": "photo_eraser", "spawnflags": "9", "StartDisabled": "0"},
                   m.probed("photo_eraser", "OnObjectsFizzled"))
    # A photo lying on the floor: picking it up (+use) photographs its target.
    m.physics(at(r, 180, 250, 24), CRATE,
              {"targetname": "photo_crate", "canbecaptured": "1", "scalevalue": "0"},
              outputs=m.probed("photo_crate", "OnCameraCapture", "OnCameraRelease"))
    m.prop("item_photo", at(r, -150, 250, 16), {"targetname": "photo",
                                                "target_entity": "photo_crate"},
           outputs=m.probed("photo", "OnPickedUp"))
    # A wall only the placement trace sees (collision group PLACEMENT_SOLID).
    m.brush_entity("func_placement_clip",
                   [(at(r, 120, -300, 0), at(r, 136, -100, 128), TRIGGER)],
                   {"targetname": "placement_clip", "StartDisabled": "0"})


def size_room(m):
    r = "size"
    # A lamp that lights when a scale-1 object (the crate) enters its pad.
    m.point("filter_size", at(r, -200, -250, 16), {"targetname": "filter_size_1",
                                                  "filtersize": "1"})
    m.prop("light", at(r, 150, 180, 96), {"targetname": "size_lamp", "spawnflags": "1",
                                          "_light": "80 255 80 200", "_constant_attn": "0",
                                          "_linear_attn": "1", "_quadratic_attn": "0"})
    pad = (at(r, 100, 100, 0), at(r, 200, 260, 2))
    m.block(pad[0], pad[1], PEDESTAL)
    m.brush_entity("trigger_multiple", [(at(r, 100, 100, 2), at(r, 200, 260, 64), TRIGGER)],
                   {"targetname": "size_pad", "spawnflags": "8", "filtername": "filter_size_1",
                    "wait": "1", "StartDisabled": "0"},
                   [("OnStartTouch", "size_lamp", "TurnOn", "", 0),
                    ("OnEndTouchAll", "size_lamp", "TurnOff", "", 0)])
    for i, (dx, scale) in enumerate(((-150, "1"), (-60, "0"), (30, "-1"))):
        m.physics(at(r, dx, 0, 24), CRATE if scale == "1" else CUBE,
                  {"targetname": "size_object_%d" % i, "canbecaptured": "1",
                   "scalevalue": scale})


def reflect(m):
    # Photographing it is refused (TestPreCapture) and lifts the player 64 units above it.
    m.prop("prop_reflect", at("reflect", 40, 0, 24), {"targetname": "reflect",
                                                      "canbecaptured": "1"})


def swap(m):
    # Photographing it is refused and trades places between the player and the cube.
    m.prop("prop_swap", at("swap", 40, 0, 24), {"targetname": "swap", "canbecaptured": "1"})


def levitator(m):
    r = "levitator"
    m.prop("prop_levitator", at(r, 0, 0, 16), {"targetname": "levitator"})
    # The balloon floats up; this physics-object trigger under the ceiling sees it arrive.
    m.point("filter_activator_name", at(r, 0, 0, 300), {"targetname": "filter_levitator",
                                                       "filtername": "levitator"})
    m.brush_entity("trigger_multiple",
                   [(at(r, -80, -100, 260), at(r, 80, 100, 376), TRIGGER)],
                   {"targetname": "levitator_high", "spawnflags": "8",
                    "filtername": "filter_levitator", "wait": "1", "StartDisabled": "0"},
                   m.probed("levitator_high", "OnStartTouch"))


def geyser(m):
    # Pitched up: it pushes along its forward axis.
    m.prop("prop_geyser", at("geyser", 0, 0, 0), {"targetname": "geyser"}, angles=(-90, 0, 0))


def air_vent(m):
    m.prop("prop_air_vent", at("air_vent", 0, 0, 0), {"targetname": "air_vent"})


def mousetrap(m):
    # Its trigger_callback volumes are created by the prop. Only a scaled-up trap
    # (object scale level 1 or 2) catapults the player who steps on its arm (at
    # its back, -x); at normal size it just snaps.
    m.prop("prop_mousetrap", at("mousetrap", 0, 0, 0), {"targetname": "mousetrap",
                                                        "scalevalue": "1"})


def monopole(m):
    r = "monopole"
    m.prop("prop_monopole", at(r, 0, 0, 64),
           {"targetname": "monopole", "StartActive": "1", "StartPositive": "1",
            "maxobjects": "3", "massScale": "1", "forcelimit": "0", "torquelimit": "0"},
           outputs=m.probed("monopole", "OnAttach", "OnDetach"))
    m.brush_entity("func_monopole_field", [(at(r, -120, -96, 0), at(r, 40, 96, 16), TRIGGER)],
                   {"targetname": "monopole_field", "StartActive": "1", "StartPositive": "0",
                    "HitboxPadding": "0 0 0"})
    m.physics(at(r, 0, 120, 24), CUBE, {"targetname": "monopole_cube", "canbecaptured": "1",
                                        "scalevalue": "0", "spawnflags": "0"})


def tombstone(m):
    r = "tombstone"
    # It raises zombies only over dirt (TestValidGround: game material 'D'). Valve
    # never shipped its model (models/props_fstop/tombstone001.mdl), so it draws
    # as the error model.
    m.block(at(r, -96, -96, 0), at(r, 96, 96, 2), DIRT)
    m.prop("prop_tombstone", at(r, 0, 0, 4), {"targetname": "tombstone"})


def portals(m):
    r = "portals"
    # A pair on the back wall (y = 848), facing -y, spawned resized through
    # HalfWidth/HalfHeight; the buttons toggle between two sizes.
    back = INNER[4] - 1
    for name, dx, two in (("portal_resize_a", -128, "0"), ("portal_resize_b", 128, "1")):
        x, _, _ = at(r, dx, 0)
        m.prop("prop_portal", (x, back, 96),
               {"targetname": name, "LinkageGroupID": "2", "Activated": "1", "PortalTwo": two,
                "HalfWidth": "48", "HalfHeight": "72"}, angles=(0, 270, 0))
    m.button("resize_small", at(r, -64, 110, 40),
             [("OnPressed", "portal_resize_a", "Resize", "24 36", 0),
              ("OnPressed", "portal_resize_b", "Resize", "24 36", 0)])
    m.button("resize_large", at(r, 64, 110, 40),
             [("OnPressed", "portal_resize_a", "Resize", "48 72", 0),
              ("OnPressed", "portal_resize_b", "Resize", "48 72", 0)])


def tunnel(m):
    r = "tunnel"
    # On the shop's west wall, facing into it (it attaches to the wall within 17
    # units behind it, the portal centred on its origin). Its partner, which it
    # builds, goes to the success target on the atrium's north wall; walking into
    # one comes out of the other. The fail targets, on this shop's walls, take the
    # partner when a photo of the tunnel is placed where it cannot stand.
    hx, hy = room_half(r)
    m.prop("prop_portal_tunnel", at(r, -hx + 1, 0, TUNNEL_Z),
           {"targetname": "tunnel", "successtarget": "tunnel_success",
            "failtarget": "tunnel_fail", "failtarget_floor": "tunnel_fail_floor",
            "failtarget_ceiling": "tunnel_fail_ceiling"})
    m.prop("info_target", at(r, hx - 8, 0, TUNNEL_Z), {"targetname": "tunnel_fail"},
           angles=(0, 180, 0))
    m.prop("info_target", at(r, 0, 0, 8), {"targetname": "tunnel_fail_floor"},
           angles=(-90, 0, 0))
    m.prop("info_target", at(r, 0, 0, INNER[5] - 8), {"targetname": "tunnel_fail_ceiling"},
           angles=(90, 0, 0))


def laser(m):
    r = "laser"
    # The laser burns what it hits: a breakable crate in its path.
    m.prop("env_portal_laser", at(r, -200, 0, 40), {"targetname": "laser"})
    m.prop("prop_physics", at(r, 180, 0, 24),
           {"targetname": "laser_target", "model": CRATE, "canbecaptured": "0"},
           outputs=m.probed("laser_target", "OnBreak"))


def sphere(m):
    # A talking core: CoreType 4 plays its sphere02 lines.
    m.prop("prop_personality_sphere", at("sphere", 0, 0, 16),
           {"targetname": "sphere", "CoreType": "4", "DelayBetweenLines": "2"})


def chicken(m):
    r = "chicken"
    m.node_grid(r)
    m.prop("npc_chicken", at(r, 0, 100, 8), {"targetname": "chicken"},
           outputs=m.probed("chicken", "OnDeath"))
    # Its nest: an info_hint of HINT_PORTAL2_NEST (1200) at Valve's nest model and eggs.
    m.prop("info_hint", at(r, 100, -150, 8), {"targetname": "chicken_nest", "hinttype": "1200",
                                              "nodeFOV": "360", "StartHintDisabled": "0",
                                              "spawnflags": "0"})
    m.point("prop_dynamic", at(r, 100, -150, 0), {"angles": "0 0 0", "model": NEST, "solid": "6"})
    for i in range(2):
        m.point("prop_physics", at(r, 90 + 20 * i, -150, 16), {"angles": "0 0 0", "model": EGG})


def bots(m):
    r = "bots"
    m.node_grid(r)
    # The companion bots, which speak through the response rules library: the
    # medic (defensive) and offense bots.
    m.prop("npc_medicbot", at(r, -100, -150, 8), {"targetname": "medicbot"}, angles=(0, 180, 0),
           outputs=m.probed("medicbot", "OnPlayerUse"))
    m.prop("npc_obot", at(r, 150, -150, 8), {"targetname": "obot"}, angles=(0, 180, 0),
           outputs=m.probed("obot", "OnPlayerUse"))


def turret(m):
    r = "turret"
    m.node_grid(r)
    m.prop("npc_hover_turret", at(r, 100, -50, 64), {"targetname": "hover_turret",
                                                     "ignoreclipbrushes": "0"},
           angles=(0, 180, 0))


def androids(m):
    r = "androids"
    m.node_grid(r)
    m.point("filter_enemy", at(r, -200, -250, 16),
            {"targetname": "filter_small_enemies", "filtername": "!player",
             "filter_object_size": "0"})
    # npc_android is neutral to the player; npc_android_basic hunts them. Add-ons
    # attach only to the front/rear ("eyes") of npc_android_basic; the add-on
    # models never shipped, so they work unseen.
    m.prop("npc_android", at(r, 150, -230, 8), {"targetname": "android"}, angles=(0, 180, 0))
    m.prop("npc_android_basic", at(r, 60, -50, 8),
           {"targetname": "android_shield", "enemyfilter": "filter_small_enemies"},
           angles=(0, 180, 0))
    m.prop("npc_android_basic", at(r, 60, 150, 8),
           {"targetname": "android_saw", "enemyfilter": "filter_small_enemies"},
           angles=(0, 180, 0))
    m.point("logic_auto", at(r, -200, -200, 16), {"spawnflags": "1"},
            [("OnMapSpawn", "android_shield", "CreateAddon", "ai_addon_shield", 1.0),
             ("OnMapSpawn", "android_saw", "CreateAddon", "ai_addon_saw", 1.0)])


def dispenser(m):
    r = "dispenser"
    m.node_grid(r)
    # Dispenses one npc_android_basic at a time; the next rises 2 s after it dies.
    m.prop("prop_android_dispenser", at(r, 0, -150, 0), {"targetname": "dispenser",
                                                         "StartDisabled": "0"},
           outputs=m.probed("dispenser", "OnSpawnNPC", "OnChildKilled"))


def linked_doors(m):
    r = "linked_doors"
    # A linked door pair facing each other: walk into one, out of the other.
    m.prop("prop_portal_linked_door", at(r, -180, -200, 0),
           {"targetname": "door_a", "partnername": "door_b", "IsPortal2": "0"})
    m.prop("prop_portal_linked_door", at(r, 180, -200, 0),
           {"targetname": "door_b", "partnername": "door_a", "IsPortal2": "1"},
           angles=(0, 180, 0))
    m.point("logic_auto", at(r, 0, -350, 16), {"spawnflags": "1"},
            [("OnMapSpawn", "door_a", "Open", "", 1.0),
             ("OnMapSpawn", "door_b", "Open", "", 1.0)])


def blobs(m):
    r = "blobs"
    m.node_grid(r)
    # HULL_TINY_FLUID; sv_blob_lennard_jones 1 turns on their cohesion.
    m.prop("npc_blob_fountain", at(r, 0, -200, 8),
           {"targetname": "blob_fountain", "particlecount": "120", "particle_radius": "4"})
    # The demo monster is not kept in here: roaming_blob puts it in the concourse.


def roaming_blob(m):
    """The demo monster, big and in arm mode, pours along the concourse after the player
    wherever the concourse and atrium reach (it only moves toward its MoveToPosition target;
    it starts down the west concourse, in front of the player start)."""
    m.prop("npc_blob_demomonster", (-1100, 0, 8),
           {"targetname": "blob_monster", "particlecount": "400", "particle_radius": "8"})
    # !player exists only once the client is in, so the target is set again every 2 s.
    m.point("logic_timer", (-1100, 64, 8), {"targetname": "blob_monster_follow",
                                            "RefireTime": "2", "StartDisabled": "0"},
            [("OnTimer", "blob_monster", "MoveToPosition", "!player", 0)])
    m.point("logic_auto", (-1100, 96, 8), {"spawnflags": "1"},
            [("OnMapSpawn", "blob_monster", "DoArms", "1", 0.5)])
    # An NPC-only pad round the player start: the monster reaches it following
    # the player (the blobs check waits there).
    m.brush_entity("trigger_multiple", [((-420, -120, 0), (-180, 120, 96), TRIGGER)],
                   {"targetname": "blob_monster_pad", "spawnflags": "2", "wait": "1",
                    "StartDisabled": "0"},
                   m.probed("blob_monster_pad", "OnStartTouch"))


ROOMS = {"camera": camera, "photo_lab": photo_lab, "size": size_room, "reflect": reflect,
         "swap": swap, "levitator": levitator, "geyser": geyser, "air_vent": air_vent,
         "mousetrap": mousetrap, "monopole": monopole, "tombstone": tombstone,
         "portals": portals, "tunnel": tunnel, "laser": laser, "sphere": sphere,
         "chicken": chicken, "bots": bots, "turret": turret, "androids": androids,
         "dispenser": dispenser, "linked_doors": linked_doors, "blobs": blobs}


def probe_relays(m):
    # In rows along the atrium's south-west corner.
    for i, name in enumerate(sorted(set(m.probes))):
        m.point("logic_relay", (-480 + 16 * (i // 64), -820 + 8 * (i % 64), 8),
                {"targetname": name})
    # The checker schedules console commands in game time through these
    # (ent_fire check_server Command "<command>" <delay>).
    m.point("point_servercommand", (480, -820, 8), {"targetname": "check_server"})
    m.point("point_clientcommand", (480, -800, 8), {"targetname": "check_client"})


def author(mcp, work):
    m = Mall(mcp)
    m.load_door_frame(work)
    mcp("new_map")
    shell(m)
    atrium(m)
    roaming_blob(m)
    for index, room in enumerate(list(SHOPS) + list(ANCHORS)):
        storefront(m, room, index)
        ROOMS[room](m)
    probe_relays(m)
    return m


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--out", type=Path, default=ROOT / "quality-results/fstop-maps" / NAME)
    parser.add_argument("--cli", type=Path, default=CLI, help="the built hammer_cli")
    parser.add_argument("--runtime", type=Path, default=RUNTIME,
                        help="staged F-Stop runtime the map's materials come from")
    parser.add_argument("--game-dir", type=Path, default=GAME_DIR,
                        help="staged F-Stop game directory the map is installed into")
    parser.add_argument("--quality", choices=("fast", "full"), default="full")
    parser.add_argument("--vmf-only", action="store_true", help="save the VMF and stop")
    args = parser.parse_args()

    out = args.out.resolve()
    work = out / "hammer"
    work.mkdir(parents=True, exist_ok=True)
    with open(out / "hammer_mcp.log", "w") as log:
        building = not args.vmf_only
        mcp = HammerMcp(args.cli.resolve(), work, out / "builds",
                        args.runtime.resolve() if building else None,
                        args.game_dir.resolve() if building else None, log)
        try:
            author(mcp, work)
            problems = mcp("check_map")
            if problems != "ok":
                print("check_map:\n" + problems)
            print("info: " + mcp("info").replace("\n", "; "))
            mcp("save", path=NAME + ".vmf")
            print("saved %s (%d MCP tool calls)" % (work / (NAME + ".vmf"), mcp.calls))
            if args.vmf_only:
                return 0
            built = mcp("build_map", path=NAME + ".vmf", quality=args.quality, publish="0")
            print("build_map: " + built)
        finally:
            mcp.close()
    record = json.loads((out / "builds" / NAME / "build.json").read_text())
    if record.get("status") != "pass":
        print("build failed: %s (%s)" % (record.get("status"), record.get("failed_gates")))
        return 1
    print("installed " + str(args.game_dir.resolve() / "maps" / (NAME + ".bsp")))
    return 0


if __name__ == "__main__":
    sys.exit(main())
