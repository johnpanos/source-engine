#!/usr/bin/env python3
"""Build and publish playable physics scenario maps (RFC 0026).

Each map stages, in game, scenes that RFC 0026's program judges Box3D against
IVP with: the same scene runs on either provider, chosen at launch, so a user
can watch the difference and a headless run can count it.

    ./kiln play portal phys_tunnel                 Box3D (the ./kiln play portal default)
    ./kiln play portal phys_tunnel --set ivp   IVP, the fallback provider

Every scene starts once at map load and restarts from the button in front of
it (use key). Walking up to a scene shows what it tests and what to look for.
Scenes that count a result also print one console line per event, prefixed
`physlab`, so the console (or a headless log) holds the score:

`phys_tunnel` (RFC 0026 B1, vphysics.continuous.v1): three lanes of small
cubes launched at the 2000 in/s object speed limit. Lane A: 6-unit cubes
against a 2-unit static wall. Lane B: 2-unit cubes against a 1-unit static
wall. Lane C: 6-unit cubes against three 1-unit panes hanging from hinges
(dynamic bodies). A cube that reaches the striped floor behind its barrier
passed through it and prints `physlab tunnel <lane> through`. The benchmark's
counts for the same barriers (RFC 0013): A 0/0, B 3/0, C 24/56 (IVP/Box3D).

`phys_stack` (scoreboard: stack collapse and settling): a 2D pyramid of 55
cubes, a 16-cube tower, and 64 Portal cubes dropped into a bin. A cube that
falls off the pyramid or the tower prints `physlab stack <scene> fell`.

`phys_joints` (B5, vphysics.joint-drive.v1; the ragdoll and joint-error
rows): eight ragdolls dropped into a bin, a 12-link chain carrying a weight
50 times a link's mass (joint stretch shows as gaps), and a motor-driven
turntable carrying cubes (phys_motor).

`phys_impacts` (B4, vphysics.contact-events.v1; B7, vphysics.explosion.v1):
Portal cubes dropped from 32 to 512 units (listen to the impacts), and a ring
of drums and crates around an env_physexplosion.

`phys_rolling` (B2/B3, vphysics.capsule-shapes.v1 and
vphysics.surface-motion.v1): drums, exact 24-sided cylinders, melons and
canisters roll down a ramp onto a measured floor (with no rolling resistance
they roll far), and a func_conveyor that carries players but not physics
objects on either provider today.

    python3 tools/quality/physics_lab_maps.py               every map
    python3 tools/quality/physics_lab_maps.py --map phys_tunnel
    python3 tools/quality/physics_lab_maps.py --vmf-only    write the VMFs only

The maps compile with the pinned legacy vbsp/vvis/vrad of the PBRT map
toolchain (build/toolchains/pbrt-map-toolchain.json) through
tools/quality/vmf_map_build.py and publish to run/maps/<map>
(tools/quality/playable_maps.py), as tools/quality/gyro_lab_map.py does.
"""

import argparse
import json
import math
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import gyro_lab_map as gyro  # noqa: E402
import playable_maps  # noqa: E402
import vmf_map_build  # noqa: E402

ROOT = HERE.parents[1]
TOOLCHAIN = ROOT / "build/toolchains/pbrt-map-toolchain.json"

BODY = gyro.BODY
WALL = gyro.WALL
PLATE = gyro.PLATE
TRIGGER = gyro.TRIGGER
STRIPE = "DEV/DEV_HAZZARDSTRIPE01A"
BARRIER = "DEV/DEV_MEASUREWALL01A"

CUBE = "models/props/metal_box.mdl"
DRUM = "models/props_c17/oildrum001.mdl"
CRATE = "models/props_junk/wood_crate001a.mdl"
MELON = "models/props_junk/watermelon01.mdl"
CANISTER = "models/props_junk/propanecanister001a.mdl"
RAGDOLL = "models/kleiner.mdl"
# Every model a map references; each is in the Portal runtime's mounted VPKs
# (portal_pak for the cube, hl2_misc for the rest).
MODELS = (CUBE, DRUM, CRATE, MELON, CANISTER, RAGDOLL)

# The VPhysics object speed limit (in/s): IVP's default maximum velocity,
# which the Box3D provider applies too.
SPEED_LIMIT = 2000.0
LAUNCH_SECONDS = 0.1
COMMAND = "physlab_cmd"
TEXT_HOLD = 8.0


def cylinder_y(vmf, center, radius, length, sides, material):
    """A regular prism along world y (a cylinder lying on its side)."""
    half = length / 2.0
    faces = [(gyro.add(center, (0, half, 0)), (0, 0, 1), (1, 0, 0)),
             (gyro.add(center, (0, -half, 0)), (1, 0, 0), (0, 0, 1))]
    for i in range(sides):
        a0, a1 = 2 * math.pi * i / sides, 2 * math.pi * (i + 1) / sides
        p0 = gyro.add(center, (radius * math.cos(a0), -half, radius * math.sin(a0)))
        p1 = gyro.add(center, (radius * math.cos(a1), -half, radius * math.sin(a1)))
        faces.append((p0, (0, 1, 0), gyro.sub(p1, p0)))
    return vmf.solid(faces, material)


def ramp(vmf, x_top, x_bottom, y0, y1, height, material):
    """A wedge rising from z=0 at x_bottom to `height` at x_top (x_top < x_bottom)."""
    yc = (y0 + y1) / 2.0
    down = (x_bottom - x_top, 0, -height)
    faces = [((x_top, yc, 0), (0, 1, 0), (1, 0, 0)),            # bottom, -z
             ((x_top, yc, 0), (0, 0, 1), (0, 1, 0)),            # back, -x
             ((x_top, yc, height), down, (0, 1, 0)),            # slope
             ((x_top, y0, 0), (1, 0, 0), (0, 0, 1)),            # side, -y
             ((x_top, y1, 0), (0, 0, 1), (1, 0, 0))]            # side, +y
    return vmf.solid(faces, material)


def ramp_height(x, x_top, x_bottom, height):
    return height * (x_bottom - x) / (x_bottom - x_top)


def command_entity(vmf, origin):
    vmf.entity("point_servercommand", {"targetname": COMMAND, "origin": gyro.vec(origin)})


def label(vmf, name, zone_center, zone_half, lines):
    """Walking into the zone shows `lines` at the top left of the screen."""
    texts = []
    for i, line in enumerate(lines):
        text = "text_%s_%d" % (name, i)
        texts.append(text)
        vmf.entity("game_text", {"targetname": text, "origin": gyro.vec(gyro.add(zone_center, (0, 0, 8 * i))),
                                 "message": line, "x": "0.03", "y": "%.3f" % (0.06 + 0.035 * i),
                                 "effect": "0", "color": "255 255 255" if i else "255 220 120",
                                 "color2": "240 110 0", "fadein": "0.1", "fadeout": "0.5",
                                 "holdtime": "%g" % TEXT_HOLD, "fxtime": "0.25",
                                 "channel": "%d" % (i % 6 + 1), "spawnflags": "1"})
    vmf.entity("trigger_multiple", {"targetname": "zone_" + name, "origin": gyro.vec(zone_center),
                                    "spawnflags": "1", "StartDisabled": "0", "wait": "%g" % TEXT_HOLD},
               solids=[vmf.box(zone_center, zone_half, TRIGGER)],
               outputs=[("OnStartTouch", text, "Display", "", 0) for text in texts])


def counter(vmf, name, boxes, pattern, message, counted):
    """Each physics object matching `pattern` that enters the zone, the union
    of `boxes` ((center, half) pairs) in one trigger so that crossing from
    one box into another is not a second entry, prints
    `echo physlab <message>` once: it is renamed `counted` as it is counted."""
    origin = gyro.vec(boxes[0][0])
    vmf.entity("filter_activator_name", {"targetname": "filter_" + name, "origin": origin,
                                         "Negated": "0", "filtername": pattern})
    vmf.entity("trigger_multiple", {"targetname": "count_" + name, "origin": origin,
                                    "spawnflags": "8", "StartDisabled": "0", "wait": "0",
                                    "filtername": "filter_" + name},
               solids=[vmf.box(center, half, TRIGGER) for center, half in boxes],
               outputs=[("OnStartTouch", COMMAND, "Command", "echo physlab " + message, 0),
                        ("OnStartTouch", "!activator", "AddOutput", "targetname " + counted, 0)])


def scene(vmf, name, patterns, button_center, extra=(), message=None, counted=False):
    """A point_template over the entities named by `patterns` (trailing `*`)
    and a relay that kills them (with `counted`, also the copies its counters
    renamed counted_<name>) and spawns them afresh; a button in front of the
    scene fires the relay. Returns the relay's name."""
    template, relay = "tpl_" + name, "restart_" + name
    keys = {"targetname": template, "origin": gyro.vec(gyro.add(button_center, (0, 0, 48))),
            "spawnflags": "2"}
    for i, pattern in enumerate(patterns):
        keys["Template%02d" % (i + 1)] = pattern
    vmf.entity("point_template", keys)
    outputs = [("OnTrigger", pattern, "Kill", "", 0) for pattern in patterns]
    if counted:
        outputs.append(("OnTrigger", "counted_%s*" % name, "Kill", "", 0))
    outputs.append(("OnTrigger", template, "ForceSpawn", "", 0.1))
    if message:
        outputs.append(("OnTrigger", COMMAND, "Command", "echo physlab " + message, 0))
    outputs += list(extra)
    vmf.entity("logic_relay", {"targetname": relay, "origin": gyro.vec(gyro.add(button_center, (0, 0, 64)))},
               outputs=outputs)
    gyro.button(vmf, name, button_center, relay)
    return relay


def physbox(vmf, name, center, solids, mass_scale=0, script=""):
    keys = {"targetname": name, "origin": gyro.vec(center), "spawnflags": "0",
            "massScale": "%g" % mass_scale, "Damagetype": "0", "health": "0", "material": "2",
            "nodamageforces": "1", "preferredcarryangles": "0 0 0", "notsolid": "0",
            "renderamt": "255"}
    if script:
        keys["overridescript"] = script
    vmf.entity("func_physbox", keys, solids=solids)


def prop(vmf, name, model, origin, angles=(0, 0, 0)):
    vmf.entity("prop_physics", {"targetname": name, "model": model, "origin": gyro.vec(origin),
                                "angles": gyro.vec(angles), "spawnflags": "0", "skin": "0",
                                "massScale": "0", "inertiaScale": "1", "physdamagescale": "0",
                                "minhealthdmg": "0", "fademindist": "-1", "fadescale": "1"})


def start_on_spawn(vmf, origin, relays):
    gyro.start_on_spawn(vmf, origin, relays)


# phys_tunnel ---------------------------------------------------------------

TUNNEL_ROOM = (-640, -480, 0, 640, 480, 256)
TUNNEL_LAUNCH_X = -480
TUNNEL_ZONE = (112, 600)          # x range counted as "through", behind every barrier
TUNNEL_LANES = [
    # lane, y, projectile half size, barrier kind, barrier thickness, note
    ("a", -300, 3, "wall", 2, "Lane A: 6-unit cubes vs a 2-unit static wall"),
    ("b", 0, 1, "wall", 1, "Lane B: 2-unit cubes vs a 1-unit static wall"),
    ("c", 300, 3, "panes", 1, "Lane C: 6-unit cubes vs three 1-unit hanging panes (dynamic)"),
]
TUNNEL_SHOTS = 8
TUNNEL_SPACING = 20
TUNNEL_HEIGHT = 40
TUNNEL_DECK = 64


def build_tunnel():
    vmf = gyro.Vmf()
    x0, y0, z0, x1, y1, z1 = TUNNEL_ROOM
    gyro.room(vmf, TUNNEL_ROOM, (x0 + 40, 0, TUNNEL_DECK + 8),
              [(x, y, 230) for x in (-400, 0, 400) for y in (-320, 0, 320)])
    # A viewing deck along the back wall, with stairs down between lanes A
    # and B, so the first view overlooks all three lanes.
    vmf.world.append(vmf.box((x0 + 40, 0, TUNNEL_DECK / 2.0), (40, y1 - 8, TUNNEL_DECK / 2.0), gyro.PEDESTAL))
    for step in range(TUNNEL_DECK // 16):
        top = TUNNEL_DECK - 16 * (step + 1)
        if top > 0:
            vmf.world.append(vmf.box((x0 + 88 + 16 * step, -150, top / 2.0), (8, 32, top / 2.0), gyro.PEDESTAL))
    command_entity(vmf, (x0 + 32, 0, TUNNEL_DECK + 64))
    label(vmf, "tunnel", (x0 + 40, 0, TUNNEL_DECK + 64), (40, 200, 63), [
        "phys_tunnel: fast cubes at the 2000 in/s speed limit against thin barriers.",
        "A cube on the striped floor went THROUGH its barrier (console: physlab tunnel).",
        "RFC 0013 bench, IVP/Box3D: A 0/0, B 3/0, C 24/56 of 64. Box3D bullets (RFC 0026 B1) fix C.",
        "Compare: ./kiln play portal phys_tunnel (Box3D)   ./kiln play portal phys_tunnel --set ivp (IVP)",
    ])
    relays = []
    for lane, y, half, kind, thick, note in TUNNEL_LANES:
        # Dividers keep a lane's cubes in their lane; behind the launch row
        # the lanes are open, so the player can walk between them.
        dx0 = TUNNEL_LAUNCH_X + 40
        for side in (-1, 1):
            vmf.world.append(vmf.box(((dx0 + x1) / 2.0, y + side * 112, 32), ((x1 - dx0) / 2.0, 4, 32), WALL))
        # The barrier.
        if kind == "wall":
            vmf.world.append(vmf.box((0, y, 64), (thick / 2.0, 104, 64), BARRIER))
        else:
            for i in range(3):
                x = i * 32
                pane = "pane_%s_%d" % (lane, i)
                physbox(vmf, pane, (x, y, 72), [vmf.box((x, y, 72), (thick / 2.0, 88, 56), PLATE)])
                vmf.entity("phys_hinge", {"targetname": "hinge_%s_%d" % (lane, i), "attach1": pane,
                                          "origin": gyro.vec((x, y, 128)),
                                          "hingeaxis": gyro.vec((x, y + 64, 128)), "spawnflags": "1",
                                          "forcelimit": "0", "torquelimit": "0", "hingefriction": "0"})
        # The striped "through" floor and its counter.
        zx0, zx1 = TUNNEL_ZONE
        zc = ((zx0 + zx1) / 2.0, y, 0.5)
        vmf.world.append(vmf.box(zc, ((zx1 - zx0) / 2.0, 104, 0.5), STRIPE))
        counter(vmf, "tunnel_" + lane, [(((zx0 + zx1) / 2.0, y, 64), ((zx1 - zx0) / 2.0, 104, 63))],
                "proj_%s_*" % lane, "tunnel %s through" % lane, "counted_tunnel_%s" % lane)
        # Projectiles, each with a thruster that fires once as it spawns: a
        # mass-independent force (an acceleration) that reaches the speed
        # limit within LAUNCH_SECONDS.
        for i in range(TUNNEL_SHOTS):
            py = y + (i - (TUNNEL_SHOTS - 1) / 2.0) * TUNNEL_SPACING
            center = (TUNNEL_LAUNCH_X, py, TUNNEL_HEIGHT)
            shot = "proj_%s_%d" % (lane, i)
            physbox(vmf, shot, center, [vmf.box(center, (half, half, half), BODY)],
                    script="damping,0,rotdamping,0,drag,0")
            vmf.entity("phys_thruster", {"targetname": "thr_%s_%d" % (lane, i), "attach1": shot,
                                         "origin": gyro.vec(center), "angles": "0 0 0",
                                         "force": "%g" % (1.2 * SPEED_LIMIT / LAUNCH_SECONDS),
                                         "forcetime": "%g" % LAUNCH_SECONDS,
                                         "spawnflags": "%d" % (0x01 | 0x02 | 0x10 | 0x20)})
        vmf.world.append(vmf.box((TUNNEL_LAUNCH_X, y, (TUNNEL_HEIGHT - half) / 2.0 - 1),
                                 (12, 96, (TUNNEL_HEIGHT - half) / 2.0 - 1), gyro.PEDESTAL))
        label(vmf, "lane_" + lane, (TUNNEL_LAUNCH_X - 40, y, 64), (24, 100, 63), [
            note, "Use the button to fire this lane again; count the cubes on the stripes."])
        relays.append(scene(vmf, "tunnel_" + lane, ["proj_%s_*" % lane, "thr_%s_*" % lane],
                            (TUNNEL_LAUNCH_X - 48, y - 80, 40), message="tunnel %s fired" % lane, counted=True))
    start_on_spawn(vmf, (x0 + 32, 64, 24), relays)
    return vmf.text()


# phys_stack ----------------------------------------------------------------

STACK_ROOM = (-640, -480, 0, 640, 480, 448)
STACK_CUBE = 12.0                  # half size of the pyramid and tower cubes
STACK_GAP = 0.25
PYRAMID_BASE = 10
PYRAMID = (-320, -200)
TOWER = (-320, 220)
TOWER_HEIGHT = 16
BIN = (300, 0)
BIN_HALF = 160
BIN_GRID = 4


def build_stack():
    vmf = gyro.Vmf()
    x0, y0, z0, x1, y1, z1 = STACK_ROOM
    gyro.room(vmf, STACK_ROOM, (x0 + 64, 0, 8),
              [(x, y, 420) for x in (-400, 0, 400) for y in (-320, 0, 320)])
    command_entity(vmf, (x0 + 32, 0, 64))
    label(vmf, "stack", (x0 + 96, 0, 64), (64, 200, 64), [
        "phys_stack: solver stability and settling.",
        "Pyramid (55 cubes) and tower (16): a cube that falls prints 'physlab stack ... fell'.",
        "RFC 0013 bench, 20-wide pyramid: IVP collapsed 38 cubes, Box3D 0. Bin: 64 Portal cubes.",
        "Compare: ./kiln play portal phys_stack (Box3D)   ./kiln play portal phys_stack --set ivp (IVP)",
    ])
    step = 2 * STACK_CUBE + STACK_GAP
    relays = []

    # Pyramid: rows along x, one cube deep.
    px, py = PYRAMID
    for row in range(PYRAMID_BASE):
        count = PYRAMID_BASE - row
        for i in range(count):
            x = px + (i - (count - 1) / 2.0) * step
            z = STACK_CUBE + STACK_GAP + row * step
            physbox(vmf, "pyr_%d_%d" % (row, i), (x, py, z),
                    [vmf.box((x, py, z), (STACK_CUBE,) * 3, BODY)])
    base = PYRAMID_BASE * step / 2.0
    fall_zones(vmf, "pyramid", (px, py), base, STACK_CUBE + 4, "pyr_*")
    label(vmf, "pyramid", (px, py - 120, 64), (180, 40, 63), [
        "Pyramid: 10 rows of 24-unit cubes. It should stand still and fall asleep."])
    relays.append(scene(vmf, "pyramid", ["pyr_*"], (px - base - 48, py - 64, 40),
                        message="stack pyramid built", counted=True))

    # Tower.
    tx, ty = TOWER
    for level in range(TOWER_HEIGHT):
        z = STACK_CUBE + STACK_GAP + level * step
        physbox(vmf, "tower_%d" % level, (tx, ty, z), [vmf.box((tx, ty, z), (STACK_CUBE,) * 3, BODY)])
    fall_zones(vmf, "tower", (tx, ty), STACK_CUBE + 2, STACK_CUBE + 4, "tower_*")
    label(vmf, "tower", (tx, ty - 100, 64), (120, 40, 63), [
        "Tower: 16 cubes on top of each other. It should not sway, creep or topple."])
    relays.append(scene(vmf, "tower", ["tower_*"], (tx - 64, ty - 64, 40), message="stack tower built",
                        counted=True))

    # Bin of Portal cubes dropped from a grid.
    bx, by = BIN
    for side in (-1, 1):
        vmf.world.append(vmf.box((bx + side * (BIN_HALF + 8), by, 48), (8, BIN_HALF + 16, 48), WALL))
        vmf.world.append(vmf.box((bx, by + side * (BIN_HALF + 8), 48), (BIN_HALF, 8, 48), WALL))
    spacing = 2 * BIN_HALF / (BIN_GRID + 1)
    n = 0
    for layer in range(BIN_GRID):
        for i in range(BIN_GRID):
            for j in range(BIN_GRID):
                origin = (bx - BIN_HALF + (i + 1) * spacing, by - BIN_HALF + (j + 1) * spacing,
                          160 + layer * 56)
                prop(vmf, "bincube_%d" % n, CUBE, origin, (0, 15 * n % 90, 0))
                n += 1
    label(vmf, "bin", (bx - BIN_HALF - 80, by, 64), (40, 160, 63), [
        "Bin: 64 Portal cubes dropped in a heap. Watch jitter, creep and how soon they sleep."])
    relays.append(scene(vmf, "bin", ["bincube_*"], (bx - BIN_HALF - 48, by - 96, 40),
                        message="stack bin dropped"))
    start_on_spawn(vmf, (x0 + 32, 64, 24), relays)
    return vmf.text()


def fall_zones(vmf, name, center, half_x, half_y, pattern):
    """Floor-level zones around a footprint (4 strips): a cube that lands there fell."""
    cx, cy = center
    # The strips start `margin` outside the footprint, so a cube that only
    # settles at the edge of the base row is not counted.
    reach, height, margin = 200, 8, 16
    half_x, half_y = half_x + margin, half_y + margin
    strips = [((cx, cy - half_y - reach / 2.0), (half_x + reach, reach / 2.0)),
              ((cx, cy + half_y + reach / 2.0), (half_x + reach, reach / 2.0)),
              ((cx - half_x - reach / 2.0, cy), (reach / 2.0, half_y)),
              ((cx + half_x + reach / 2.0, cy), (reach / 2.0, half_y))]
    counter(vmf, name, [((sx, sy, height), (hx, hy, height)) for (sx, sy), (hx, hy) in strips],
            pattern, "stack %s fell" % name, "counted_%s" % name)


# phys_joints ---------------------------------------------------------------

JOINT_ROOM = (-640, -480, 0, 640, 480, 448)
CHAIN = (-200, 240)
CHAIN_TOP = 400
CHAIN_LINKS = 12
CHAIN_LINK = (3, 3, 10)            # half size; links meet at their ends
CHAIN_WEIGHT_RATIO = 50            # the weight's mass in link masses
CHAIN_WEIGHT_HALF = 12
RAG_BIN = (240, -200)
RAG_BIN_HALF = 128
TABLE = (-260, -220)


def build_joints():
    vmf = gyro.Vmf()
    x0, y0, z0, x1, y1, z1 = JOINT_ROOM
    gyro.room(vmf, JOINT_ROOM, (x0 + 64, 0, 8),
              [(x, y, 420) for x in (-400, 0, 400) for y in (-320, 0, 320)])
    command_entity(vmf, (x0 + 32, 0, 64))
    label(vmf, "joints", (x0 + 96, 0, 64), (64, 200, 64), [
        "phys_joints: joints under load and contact.",
        "Ragdolls: limbs should stay attached (RFC 0013 bench joint error, IVP 57 / Box3D 0.08).",
        "Chain: a weight 50x a link's mass; gaps between links are joint stretch.",
        "Turntable: a phys_motor drives a hinge; cubes ride it.",
    ])
    relays = []

    # Ragdoll bin.
    bx, by = RAG_BIN
    for side in (-1, 1):
        vmf.world.append(vmf.box((bx + side * (RAG_BIN_HALF + 8), by, 40), (8, RAG_BIN_HALF + 16, 40), WALL))
        vmf.world.append(vmf.box((bx, by + side * (RAG_BIN_HALF + 8), 40), (RAG_BIN_HALF, 8, 40), WALL))
    n = 0
    for layer in range(2):
        for i in range(2):
            for j in range(2):
                origin = (bx - 56 + 112 * i, by - 56 + 112 * j, 140 + 110 * layer)
                vmf.entity("prop_ragdoll", {"targetname": "rag_%d" % n, "model": RAGDOLL,
                                            "origin": gyro.vec(origin),
                                            "angles": gyro.vec((0, 45 * n, 90 if layer else 0)),
                                            "spawnflags": "0", "skin": "0", "fademindist": "-1",
                                            "fadescale": "1"})
                n += 1
    label(vmf, "ragdolls", (bx - RAG_BIN_HALF - 72, by, 64), (40, 140, 63), [
        "Ragdolls: eight dropped into a bin. Look for stretched or jittering limbs."])
    relays.append(scene(vmf, "ragdolls", ["rag_*"], (bx - RAG_BIN_HALF - 48, by - 96, 40),
                        message="joints ragdolls dropped"))

    # Chain with a heavy weight.
    cx, cy = CHAIN
    vmf.world.append(vmf.box((cx, cy, CHAIN_TOP + 8), (24, 24, 8), gyro.PEDESTAL))
    hx, hy, hz = CHAIN_LINK
    previous = None
    top = CHAIN_TOP
    for i in range(CHAIN_LINKS):
        center = (cx, cy, top - hz)
        link = "link_%d" % i
        physbox(vmf, link, center, [vmf.box(center, CHAIN_LINK, BODY)])
        ball = {"targetname": "ball_%d" % i, "attach1": link, "origin": gyro.vec((cx, cy, top)),
                "spawnflags": "1", "forcelimit": "0", "torquelimit": "0"}
        if previous:
            ball["attach2"] = previous
        vmf.entity("phys_ballsocket", ball)
        previous, top = link, top - 2 * hz
    weight = (cx, cy, top - CHAIN_WEIGHT_HALF)
    # Same material as the links, so mass goes with volume: scale the
    # weight's own mass to CHAIN_WEIGHT_RATIO link masses.
    link_volume = 8.0 * hx * hy * hz
    weight_volume = 8.0 * CHAIN_WEIGHT_HALF ** 3
    physbox(vmf, "chain_weight", weight, [vmf.box(weight, (CHAIN_WEIGHT_HALF,) * 3, BODY)],
            mass_scale=CHAIN_WEIGHT_RATIO * link_volume / weight_volume)
    vmf.entity("phys_ballsocket", {"targetname": "ball_weight", "attach1": "chain_weight",
                                   "attach2": previous, "origin": gyro.vec((cx, cy, top)),
                                   "spawnflags": "1", "forcelimit": "0", "torquelimit": "0"})
    vmf.entity("env_physexplosion", {"targetname": "kick_chain",
                                     "origin": gyro.vec((cx - 40, cy, top - CHAIN_WEIGHT_HALF)),
                                     "magnitude": "400", "radius": "96", "inner_radius": "0",
                                     "spawnflags": "1", "targetentityname": "chain_weight"})
    vmf.entity("logic_relay", {"targetname": "kick", "origin": gyro.vec((cx - 96, cy, 96))},
               outputs=[("OnTrigger", "kick_chain", "Explode", "", 0)])
    gyro.button(vmf, "chain", (cx - 96, cy - 64, 40), "kick")
    label(vmf, "chain", (cx - 120, cy, 64), (60, 100, 63), [
        "Chain: 12 links holding a 50x weight. Use the button to kick the weight.",
        "Links should stay joined; visible gaps are the solver's joint error."])

    # Motor-driven turntable with cubes riding it.
    tx, ty = TABLE
    vmf.world.append(vmf.box((tx, ty, 12), (8, 8, 12), gyro.PEDESTAL))
    table = (tx, ty, 30)
    physbox(vmf, "turntable", table, [vmf.box(table, (96, 96, 6), PLATE)])
    vmf.entity("phys_motor", {"targetname": "motor", "attach1": "turntable", "origin": gyro.vec(table),
                              "axis": gyro.vec((tx, ty, 94)), "speed": "90", "spinup": "1",
                              "inertiafactor": "1.0", "spawnflags": "5"})
    for i in range(4):
        a = math.pi / 2 * i
        prop(vmf, "ride_%d" % i, CUBE, (tx + 56 * math.cos(a), ty + 56 * math.sin(a), 64), (0, 90 * i, 0))
    label(vmf, "table", (tx, ty + 140, 64), (120, 30, 63), [
        "Turntable: phys_motor at 90 deg/s. Cubes should ride it, not slide off or jitter."])
    relays.append(scene(vmf, "table", ["ride_*"], (tx + 140, ty - 60, 40), message="joints table loaded"))
    start_on_spawn(vmf, (x0 + 32, 64, 24), relays)
    return vmf.text()


# phys_impacts --------------------------------------------------------------

IMPACT_ROOM = (-640, -480, 0, 640, 480, 640)
DROP_HEIGHTS = (32, 64, 128, 256, 512)
DROP_Y = -240
RING = (200, 200)
RING_RADIUS = 128
RING_COUNT = 16


def build_impacts():
    vmf = gyro.Vmf()
    x0, y0, z0, x1, y1, z1 = IMPACT_ROOM
    gyro.room(vmf, IMPACT_ROOM, (x0 + 64, 0, 8),
              [(x, y, 600) for x in (-400, 0, 400) for y in (-320, 0, 320)])
    command_entity(vmf, (x0 + 32, 0, 64))
    label(vmf, "impacts", (x0 + 96, 0, 64), (64, 200, 64), [
        "phys_impacts: impact events and explosions.",
        "Drops: Portal cubes from 32 to 512 units. Each landing should sound once per real bounce.",
        "Ring: drums and crates around an explosion. The blast should push them evenly outward.",
        "Compare: ./kiln play portal phys_impacts (Box3D)   ./kiln play portal phys_impacts --set ivp (IVP)",
    ])
    relays = []
    for i, height in enumerate(DROP_HEIGHTS):
        x = -320 + 128 * i
        prop(vmf, "drop_%d" % i, CUBE, (x, DROP_Y, height + 20))
        vmf.world.append(vmf.box((x, DROP_Y + 48, 1), (32, 8, 1), STRIPE))
    label(vmf, "drops", (-64, DROP_Y + 120, 64), (320, 40, 63), [
        "Drops: cubes from 32, 64, 128, 256 and 512 units. Listen to each impact."])
    relays.append(scene(vmf, "drops", ["drop_*"], (-420, DROP_Y, 40), message="impacts dropped"))

    rx, ry = RING
    for i in range(RING_COUNT):
        a = 2 * math.pi * i / RING_COUNT
        origin = (rx + RING_RADIUS * math.cos(a), ry + RING_RADIUS * math.sin(a), 32)
        prop(vmf, "ring_%d" % i, DRUM if i % 2 == 0 else CRATE, origin, (0, math.degrees(a), 0))
    vmf.entity("env_physexplosion", {"targetname": "blast", "origin": gyro.vec((rx, ry, 24)),
                                     "magnitude": "600", "radius": "320", "inner_radius": "0",
                                     "spawnflags": "1", "targetentityname": ""})
    label(vmf, "ring", (rx - RING_RADIUS - 120, ry, 64), (40, 140, 63), [
        "Ring: 8 drums and 8 crates. The button rebuilds the ring and sets off the blast."])
    relays.append(scene(vmf, "ring", ["ring_*"], (rx - RING_RADIUS - 96, ry - 96, 40),
                        extra=[("OnTrigger", "blast", "Explode", "", 1.5)],
                        message="impacts ring blast"))
    start_on_spawn(vmf, (x0 + 32, 64, 24), relays)
    return vmf.text()


# phys_rolling --------------------------------------------------------------

ROLL_ROOM = (-640, -480, 0, 1280, 480, 320)
RAMP_TOP, RAMP_BOTTOM, RAMP_HEIGHT = -600, -300, 150
RAMP_Y = (-240, 240)
CONVEYOR_Y = 360


def build_rolling():
    vmf = gyro.Vmf()
    x0, y0, z0, x1, y1, z1 = ROLL_ROOM
    spawn = (RAMP_TOP + 40, RAMP_Y[0] - 120, 8)
    gyro.room(vmf, ROLL_ROOM, spawn,
              [(x, y, 290) for x in (-400, 100, 600, 1100) for y in (-320, 0, 320)], (0, 25, 0))
    command_entity(vmf, (x0 + 32, 0, 64))
    label(vmf, "rolling", gyro.add(spawn, (0, 0, 56)), (64, 64, 63), [
        "phys_rolling: round bodies and moving surfaces.",
        "Ramp: drums, 24-sided cylinders, melons and canisters roll onto the measured floor.",
        "No provider has rolling resistance today, so they roll far (RFC 0026 B3 adds it to Box3D).",
        "Conveyor: carries players, not physics objects, on IVP and Box3D (B3: surface velocity).",
    ])
    vmf.world.append(ramp(vmf, RAMP_TOP, RAMP_BOTTOM, RAMP_Y[0], RAMP_Y[1], RAMP_HEIGHT, gyro.PEDESTAL))
    rollers = [("drum", DRUM, 16), ("cyl", None, 16), ("melon", MELON, 8), ("can", CANISTER, 6)]
    n = 0
    for row, x in enumerate((-560, -500)):
        for i in range(8):
            y = RAMP_Y[0] + 40 + 56 * i + (28 if row else 0)
            if y > RAMP_Y[1] - 32:
                continue
            kind, model, radius = rollers[(i + row) % len(rollers)]
            z = ramp_height(x, RAMP_TOP, RAMP_BOTTOM, RAMP_HEIGHT) + radius + 6
            name = "roll_%d" % n
            if model is None:
                physbox(vmf, name, (x, y, z), [cylinder_y(vmf, (x, y, z), radius, 40, 24, BODY)])
            else:
                prop(vmf, name, model, (x, y, z), (0, 0, 90) if kind in ("drum", "can") else (0, 0, 0))
            n += 1
    label(vmf, "ramp", (-240, 0, 64), (40, 240, 63), [
        "Ramp: two rows of rolling bodies. Note how far each rolls and whether it ever stops."])
    relays = [scene(vmf, "ramp", ["roll_*"], (RAMP_TOP + 40, RAMP_Y[0] - 64, 40), message="rolling released")]

    # Conveyor along +x with a stop wall, and cubes dropped onto it.
    cx0, cx1 = -200, 400
    vmf.entity("func_conveyor", {"targetname": "belt", "speed": "150", "movedir": "0 0 0",
                                 "spawnflags": "0", "_minlight": "0", "renderamt": "255"},
               solids=[vmf.box(((cx0 + cx1) / 2.0, CONVEYOR_Y, 4), ((cx1 - cx0) / 2.0, 48, 4), STRIPE)])
    vmf.world.append(vmf.box((cx1 + 24, CONVEYOR_Y, 32), (8, 56, 32), WALL))
    for i in range(4):
        prop(vmf, "belt_cube_%d" % i, CUBE, (cx0 + 40 + 72 * i, CONVEYOR_Y, 48))
    label(vmf, "belt", ((cx0 + cx1) / 2.0, CONVEYOR_Y - 100, 64), ((cx1 - cx0) / 2.0, 40, 63), [
        "Conveyor (func_conveyor, 150 in/s): stand on it and it carries you; the cubes stay put."])
    relays.append(scene(vmf, "belt", ["belt_cube_*"], (cx0 - 48, CONVEYOR_Y - 64, 40),
                        message="rolling belt loaded"))
    start_on_spawn(vmf, (x0 + 32, 64, 24), relays)
    return vmf.text()


MAPS = {"phys_tunnel": build_tunnel, "phys_stack": build_stack, "phys_joints": build_joints,
        "phys_impacts": build_impacts, "phys_rolling": build_rolling}


def build_map(name, out, tools, runtime):
    work = out / "compile"
    work.mkdir(parents=True, exist_ok=True)
    vmf = work / (name + ".vmf")
    vmf.write_text(MAPS[name]())
    print("wrote " + str(vmf))
    if tools is None:
        return
    record = vmf_map_build.build(vmf, out, tools, runtime, quality="full", name=name)
    if record["status"] != "pass":
        raise RuntimeError("%s: %s (%s)" % (name, record["status"], out / "build.json"))
    print("published " + playable_maps.describe(playable_maps.publish(record)))


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--map", action="append", choices=sorted(MAPS),
                        help="map to build (repeatable; default all)")
    parser.add_argument("--out", type=Path, default=ROOT / "quality-results/physics-lab-maps",
                        help="output root; each map builds in <out>/<map>")
    parser.add_argument("--toolchain", type=Path, default=TOOLCHAIN)
    parser.add_argument("--runtime", type=Path, default=ROOT / "run/runtime",
                        help="staged runtime the map's materials and models come from")
    parser.add_argument("--vmf-only", action="store_true", help="write the VMFs and stop")
    args = parser.parse_args()

    tools = None if args.vmf_only else Path(json.loads(args.toolchain.read_text())["compile_tools"])
    for name in args.map or sorted(MAPS):
        build_map(name, args.out.resolve() / name, tools, args.runtime.resolve())
    return 0


if __name__ == "__main__":
    sys.exit(main())
