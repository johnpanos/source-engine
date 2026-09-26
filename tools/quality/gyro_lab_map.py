#!/usr/bin/env python3
"""Build and publish playable maps for gyroscopic torque.

They stage, in game, the scenes of the VPhysics suite's `gyro.*` clauses
(unittests/physicstest/test_vphysics_gyroscopic.cpp).

`gyro_lab`:

* a zero-gravity bay (`trigger_vphysics_motion`, gravity scale 0) with five
  48x24x8 boxes, each spun up by a `phys_torque` to 1 rev/s: about 5 degrees
  off the intermediate axis (it flips over and back, the Dzhanibekov effect),
  about the major axis and the minor axis (both steady), about the body
  diagonal (the torque-free tumble: its angular velocity wanders while its
  angular momentum stays fixed; IVP's explicit gyroscopic integration gains
  energy here, so on IVP it tumbles faster and faster), and a box turned
  inside its brush and spun about the vertical (it tumbles only with the
  shape inertia model, RFC 0013 vphysics.shape-inertia.v1);
* a gyroscope: a 96x96 plate on an axle, hung from a `phys_ballsocket` on a
  post. A `phys_hinge` holds the axle level while a `phys_torque` spins the
  plate to 3000 deg/s; the hinge then lets go and the plate precesses about
  the post at m g r / (I3 s), about 10 deg/s, instead of falling.

`dzhanibekov`: the Dzhanibekov effect as Box3D's samples app shows it (the
"Gyroscopic Torque" sample, Erin Catto's Box3D walkthrough at 6:06): three
T-handles, a bar with a cylindrical handle at the sample's size (2 m bar,
0.6 m handle), float in zero gravity in front of the spawn. Each is spun at
10 rad/s: the middle one about its handle, the intermediate principal axis,
flips over and back; the left one about its bar (the minor axis) and the
right one about the axis through both (the major axis) keep spinning steadily.

Each scene restarts from a button in front of it (use key); every scene
starts once at map load. The bodies have no damping or drag
(`overridescript`), so what is left is the provider's rotation.

    python3 tools/quality/gyro_lab_map.py              both maps
    python3 tools/quality/gyro_lab_map.py --map dzhanibekov
    ./play dzhanibekov                 Box3D, shape inertia (-physics_shape_inertia)
    PHYSICS_ARGS= ./play dzhanibekov   Box3D, legacy inertia
    PHYSICS=vphysics ./play gyro_lab   IVP

The map compiles with the pinned legacy vbsp/vvis/vrad of the PBRT map
toolchain (build/toolchains/pbrt-map-toolchain.json) against the staged Portal
runtime, and publishes to run/maps/<map> (tools/quality/playable_maps.py).
"""

import argparse
import datetime
import hashlib
import json
import math
import re
import shutil
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import playable_maps  # noqa: E402
from source_content import ContentResolver  # noqa: E402

ROOT = HERE.parents[1]
TOOLCHAIN = ROOT / "build/toolchains/pbrt-map-toolchain.json"

WALL = "DEV/GRAYGRID"
FLOOR = "DEV/DEV_MEASUREGENERIC01B"
CEILING = "DEV/GRAYGRID"
PEDESTAL = "DEV/DEV_MEASUREWALL01A"
BODY = "DEV/DEV_MEASUREGENERIC01"
PLATE = "DEV/DEV_MEASUREWALL01D"
BUTTON = "DEV/DEV_MEASURESWITCH01"
TRIGGER = "TOOLS/TOOLSTRIGGER"
MATERIALS = (WALL, FLOOR, CEILING, PEDESTAL, BODY, PLATE, BUTTON, TRIGGER)
SURFACE_MANIFEST = "scripts/surfaceproperties_manifest.txt"

# Room interior and wall thickness (inches).
ROOM = (-640, -448, 0, 640, 448, 448)
THICK = 16
INCHES_PER_METER = 1.0 / 0.0254
NO_DAMPING = "damping,0,rotdamping,0,drag,0"

# Zero-g bay: box centers along x, one scene each.
BAY_Y, BAY_Z = 256, 240
BOX_HALF = (24, 12, 4)       # long x (minor axis), mid y (intermediate), thin z (major)
SPIN_DEG_PER_S = 360.0       # 1 rev/s
SPIN_SECONDS = 0.5
# The first four boxes are axis-aligned and carry the off-axis part in the
# spin axis (phys_torque snaps an axis only within 0.002 of a unit axis). The
# last is turned inside its brush entity and spun about the vertical: the
# legacy inertia model keeps only per-axis inertia in the entity frame (IVP's
# compact surface has no principal-axis rotation), so it just spins; with the
# shape inertia model (RFC 0013 vphysics.shape-inertia.v1, -physics_shape_inertia,
# the ./play default) its products of inertia make it tumble.
BAYS = [
    # name, x, world torque axis (normalized when written), box angles
    ("tilted", -440, (0.0, 0.0, 1.0), (35.0, 30.0, 20.0)),
    ("intermediate", -240, (0.06, 1.0, 0.06), (0.0, 0.0, 0.0)),   # ~5 degrees off: it flips
    ("major", -24, (0.0, 0.0, 1.0), (0.0, 0.0, 0.0)),
    ("minor", 192, (1.0, 0.0, 0.0), (0.0, 0.0, 0.0)),
    ("tumble", 408, (1.0, 1.0, 1.0), (0.0, 0.0, 0.0)),         # the conformance tumble
]

# Gyroscope: pivot, arm and spin.
PIVOT = (0, -256, 224)
ARM = 24
PLATE_HALF = 48
GYRO_SPIN_DEG_PER_S = 3000.0
GYRO_SPIN_SECONDS = 4.0

# dzhanibekov: Box3D's GyroscopicTorque sample (samples/sample_bodies.cpp) at
# its own size, with the handle along world z and the bar along world y.
DZ_ROOM = (-384, -320, 0, 384, 320, 384)
DZ_X, DZ_Z = 64, 72
DZ_BAR_HALF = (0.1 * INCHES_PER_METER, 1.0 * INCHES_PER_METER, 0.05 * INCHES_PER_METER)
DZ_HANDLE_RADIUS = 0.15 * INCHES_PER_METER
DZ_HANDLE_HEIGHT = 0.6 * INCHES_PER_METER
DZ_SPIN_DEG_PER_S = math.degrees(10.0)  # the sample's 10 rad/s
DZ_HANDLES = [
    # name, y, world spin axis (normalized when written)
    ("bar", -176, (0.06, 1.0, 0.06)),        # minor axis: steady
    ("handle", 0, (0.06, 0.06, 1.0)),        # intermediate axis: flips
    ("broad", 176, (1.0, 0.06, 0.06)),       # major axis: steady
]


class Vmf:
    """A VMF writer for axis-aligned boxes and entities."""

    def __init__(self):
        self.next_id = 1
        self.world = []
        self.entities = []

    def id(self):
        self.next_id += 1
        return self.next_id

    def solid(self, faces, material):
        """A convex solid from its faces, each (point on the face, u, v) with
        cross(u, v) the outward normal: vbsp's plane normal is
        cross(p0 - p1, p2 - p1) for (p1 + u, p1, p1 + v)."""
        lines = ["\tsolid", "\t{", '\t\t"id" "%d"' % self.id()]
        for point, u, v in faces:
            u, v = normalize(u), normalize(v)
            points = (add(point, scale(u, 32)), point, add(point, scale(v, 32)))
            plane = " ".join("(%.4f %.4f %.4f)" % p for p in points)
            uaxis, vaxis = texture_axes(cross(u, v))
            lines += ["\t\tside", "\t\t{", '\t\t\t"id" "%d"' % self.id(),
                      '\t\t\t"plane" "%s"' % plane,
                      '\t\t\t"material" "%s"' % material,
                      '\t\t\t"uaxis" "%s 0.25"' % uaxis, '\t\t\t"vaxis" "%s 0.25"' % vaxis,
                      '\t\t\t"rotation" "0"', '\t\t\t"lightmapscale" "16"',
                      '\t\t\t"smoothing_groups" "0"', "\t\t}"]
        return "\n".join(lines + ["\t}"])

    def box(self, center, half, material, angles=(0.0, 0.0, 0.0)):
        axes = rotation_axes(angles)
        faces = []
        for axis in range(3):
            for sign in (1, -1):
                normal = scale(axes[axis], sign)
                u = axes[(axis + 1) % 3]
                v = axes[(axis + 2) % 3]
                if dot(cross(u, v), normal) < 0:
                    u, v = v, u
                faces.append((add(center, scale(normal, half[axis])), u, v))
        return self.solid(faces, material)

    def prism(self, base, radius, height, sides, material):
        """A vertical regular prism (a cylinder) standing on `base`."""
        top = add(base, (0, 0, height))
        faces = [(top, (1, 0, 0), (0, 1, 0)), (base, (0, 1, 0), (1, 0, 0))]
        for i in range(sides):
            a0, a1 = 2 * math.pi * i / sides, 2 * math.pi * (i + 1) / sides
            p0 = add(base, (radius * math.cos(a0), radius * math.sin(a0), 0))
            p1 = add(base, (radius * math.cos(a1), radius * math.sin(a1), 0))
            faces.append((p0, sub(p1, p0), (0, 0, 1)))
        return self.solid(faces, material)

    def entity(self, classname, keyvalues, solids=(), outputs=()):
        lines = ["entity", "{", '\t"id" "%d"' % self.id(), '\t"classname" "%s"' % classname]
        lines += ['\t"%s" "%s"' % (k, v) for k, v in keyvalues.items()]
        if outputs:
            lines += ["\tconnections", "\t{"]
            lines += ['\t\t"%s" "%s,%s,%s,%s,-1"' % (event, target, action, param, delay)
                      for event, target, action, param, delay in outputs]
            lines += ["\t}"]
        lines += list(solids)
        self.entities.append("\n".join(lines + ["}"]))

    def text(self):
        head = ["versioninfo", "{", '\t"editorversion" "400"', '\t"editorbuild" "8000"',
                '\t"mapversion" "1"', '\t"formatversion" "100"', '\t"prefab" "0"', "}",
                "visgroups", "{", "}", "world", "{", '\t"id" "1"', '\t"mapversion" "1"',
                '\t"classname" "worldspawn"', '\t"skyname" "sky_day01_01"']
        return "\n".join(head + self.world + ["}"] + self.entities) + "\n"


def add(a, b):
    return tuple(x + y for x, y in zip(a, b))


def sub(a, b):
    return tuple(x - y for x, y in zip(a, b))


def normalize(a):
    length = math.sqrt(dot(a, a))
    return scale(a, 1.0 / length)


def scale(a, s):
    return tuple(x * s for x in a)


def dot(a, b):
    return sum(x * y for x, y in zip(a, b))


def cross(a, b):
    return (a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0])


def rotation_axes(angles):
    """Local x, y, z in world for Source angles (pitch, yaw, roll), as AngleMatrix."""
    pitch, yaw, roll = (math.radians(a) for a in angles)
    sp, cp = math.sin(pitch), math.cos(pitch)
    sy, cy = math.sin(yaw), math.cos(yaw)
    sr, cr = math.sin(roll), math.cos(roll)
    forward = (cp * cy, cp * sy, -sp)
    left = (sr * sp * cy - cr * sy, sr * sp * sy + cr * cy, sr * cp)
    up = (cr * sp * cy + sr * sy, cr * sp * sy - sr * cy, cr * cp)
    return forward, left, up


def texture_axes(normal):
    """Hammer's world-aligned texture axes for the normal's dominant axis."""
    axis = max(range(3), key=lambda i: abs(normal[i]))
    return (("[0 1 0 0]", "[0 0 -1 0]") if axis == 0 else
            ("[1 0 0 0]", "[0 0 -1 0]") if axis == 1 else
            ("[1 0 0 0]", "[0 -1 0 0]"))


def vec(v):
    return " ".join("%g" % c for c in v)


def room(vmf, bounds, spawn, lights):
    x0, y0, z0, x1, y1, z1 = bounds
    cx, cy, cz = (x0 + x1) / 2, (y0 + y1) / 2, (z0 + z1) / 2
    hx, hy, hz = (x1 - x0) / 2 + THICK, (y1 - y0) / 2 + THICK, (z1 - z0) / 2 + THICK
    t = THICK / 2
    vmf.world += [
        vmf.box((cx, cy, z0 - t), (hx, hy, t), FLOOR),
        vmf.box((cx, cy, z1 + t), (hx, hy, t), CEILING),
        vmf.box((x0 - t, cy, cz), (t, hy, hz), WALL),
        vmf.box((x1 + t, cy, cz), (t, hy, hz), WALL),
        vmf.box((cx, y0 - t, cz), (hx - THICK, t, hz), WALL),
        vmf.box((cx, y1 + t, cz), (hx - THICK, t, hz), WALL),
    ]
    vmf.entity("info_player_start", {"origin": vec(spawn), "angles": "0 0 0"})
    for origin in lights:
        vmf.entity("light", {"origin": vec(origin), "_light": "255 250 240 350",
                             "_quadratic_attn": "0", "_linear_attn": "1", "_constant_attn": "0"})


def zero_gravity(vmf, center, half):
    """Physics objects inside float. A near-zero speed limit holds each where
    it is (it has fallen for a moment before the trigger touches it);
    rotation is free."""
    vmf.entity("trigger_vphysics_motion",
               {"spawnflags": "8", "StartDisabled": "0", "SetGravityScale": "0",
                "SetAdditionalAirDensity": "0", "SetVelocityLimit": "0.5",
                "SetVelocityLimitDelta": "0", "SetVelocityScale": "1", "SetAngVelocityLimit": "0",
                "SetAngVelocityScale": "1", "SetLinearForce": "0", "SetLinearForceAngles": "0 0 0",
                "ParticleTrailMaterial": "", "ParticleTrailLifetime": "4",
                "ParticleTrailStartSize": "2", "ParticleTrailEndSize": "3"},
               solids=[vmf.box(center, half, TRIGGER)])


def physbox(vmf, name, center, solids):
    vmf.entity("func_physbox",
               {"targetname": name, "origin": vec(center), "spawnflags": "0",
                "overridescript": NO_DAMPING, "massScale": "0", "Damagetype": "0", "health": "0",
                "material": "2", "nodamageforces": "1", "preferredcarryangles": "0 0 0",
                "notsolid": "0", "renderamt": "255"},
               solids=solids)


def button(vmf, name, center, relay):
    """A use-key button on a pedestal; pressing it triggers `relay`."""
    x, y, z = center
    vmf.world.append(vmf.box((x, y, (z - 8) / 2), (12, 12, (z - 8) / 2), PEDESTAL))
    vmf.entity("func_button", {"targetname": "button_" + name, "origin": vec(center),
                               "spawnflags": "1025", "speed": "5", "wait": "1", "lip": "0",
                               "sounds": "0", "movedir": "0 0 0", "renderamt": "255"},
               solids=[vmf.box(center, (8, 8, 8), BUTTON)],
               outputs=[("OnPressed", relay, "Trigger", "", 0)])


def spinner(vmf, name, center, solids, axis, degrees_per_second, button_center):
    """A floating physbox that its relay returns to its spawn pose and spins
    up about the world `axis` (phys_torque over SPIN_SECONDS); returns the
    relay's name."""
    body, relay = "box_" + name, "spin_" + name
    axis = normalize(axis)
    physbox(vmf, body, center, solids)
    vmf.entity("point_teleport", {"targetname": "reset_" + name, "target": body,
                                  "origin": vec(center), "angles": "0 0 0", "spawnflags": "1"})
    vmf.entity("phys_torque", {"targetname": "torque_" + name, "attach1": body,
                               "origin": vec(center), "axis": vec(add(center, scale(axis, 64))),
                               "force": "%g" % (degrees_per_second / SPIN_SECONDS),
                               "forcetime": "%g" % SPIN_SECONDS, "spawnflags": "0"})
    vmf.entity("logic_relay", {"targetname": relay, "origin": vec(add(center, (0, 0, -48)))},
               outputs=[("OnTrigger", "reset_" + name, "Teleport", "", 0),
                        ("OnTrigger", "torque_" + name, "Activate", "", 0.1)])
    button(vmf, name, button_center, relay)
    return relay


def start_on_spawn(vmf, origin, relays):
    vmf.entity("logic_auto", {"origin": vec(origin), "spawnflags": "1"},
               outputs=[("OnMapSpawn", relay, "Trigger", "", 1.0) for relay in relays])


def build_gyro_lab():
    vmf = Vmf()
    x0 = ROOM[0]
    room(vmf, ROOM, (x0 + 80, 0, 8),
         [(x, y, 400) for x in (-400, 0, 400) for y in (-300, 0, 300)])

    # Zero-g bay.
    trigger_x0 = BAYS[0][1] - 112
    trigger_x1 = BAYS[-1][1] + 112
    zero_gravity(vmf, ((trigger_x0 + trigger_x1) / 2, BAY_Y, BAY_Z),
                 ((trigger_x1 - trigger_x0) / 2, 144, 144))
    starts = []
    for name, x, axis, angles in BAYS:
        center = (x, BAY_Y, BAY_Z)
        starts.append(spinner(vmf, name, center, [vmf.box(center, BOX_HALF, BODY, angles)], axis,
                              SPIN_DEG_PER_S, (x, BAY_Y - 160, 40)))

    # Gyroscope.
    px, py, pz = PIVOT
    vmf.world.append(vmf.box((px, py, (pz - 12) / 2), (6, 6, (pz - 12) / 2), PEDESTAL))
    plate_center = (px + ARM, py, pz)
    physbox(vmf, "gyro_plate", plate_center,
            [vmf.box(plate_center, (4, PLATE_HALF, PLATE_HALF), PLATE),
             vmf.box((px + (ARM - 4) / 2, py, pz), ((ARM - 4) / 2, 2, 2), BODY)])
    vmf.entity("phys_ballsocket", {"targetname": "gyro_pivot", "attach1": "gyro_plate",
                                   "origin": vec(PIVOT), "spawnflags": "0",
                                   "forcelimit": "0", "torquelimit": "0"})
    vmf.entity("phys_hinge", {"targetname": "gyro_hinge", "attach1": "gyro_plate",
                              "origin": vec(PIVOT), "hingeaxis": vec((px + 64, py, pz)),
                              "spawnflags": "0", "forcelimit": "0", "torquelimit": "0",
                              "hingefriction": "0"})
    vmf.entity("point_teleport", {"targetname": "reset_gyro", "target": "gyro_plate",
                                  "origin": vec(plate_center), "angles": "0 0 0", "spawnflags": "1"})
    vmf.entity("phys_torque", {"targetname": "torque_gyro", "attach1": "gyro_plate",
                               "origin": vec(plate_center), "axis": vec((px + ARM + 64, py, pz)),
                               "force": "%g" % (GYRO_SPIN_DEG_PER_S / GYRO_SPIN_SECONDS),
                               "forcetime": "%g" % GYRO_SPIN_SECONDS, "spawnflags": "0"})
    vmf.entity("logic_relay", {"targetname": "spin_gyro", "origin": vec((px, py, 24))},
               outputs=[("OnTrigger", "reset_gyro", "Teleport", "", 0),
                        ("OnTrigger", "gyro_hinge", "TurnOn", "", 0.05),
                        ("OnTrigger", "torque_gyro", "Activate", "", 0.5),
                        ("OnTrigger", "gyro_hinge", "TurnOff", "", 0.5 + GYRO_SPIN_SECONDS + 0.5)])
    button(vmf, "gyro", (px - 96, py + 96, 40), "spin_gyro")
    starts.append("spin_gyro")

    start_on_spawn(vmf, (x0 + 32, 0, 24), starts)
    return vmf.text()


def build_dzhanibekov():
    vmf = Vmf()
    x0 = DZ_ROOM[0]
    room(vmf, DZ_ROOM, (x0 + 80, 0, 8),
         [(x, y, 340) for x in (-240, 160) for y in (-200, 0, 200)])
    zero_gravity(vmf, (DZ_X, 0, DZ_Z + 24), (112, 300, DZ_Z + 24))
    starts = []
    for name, y, axis in DZ_HANDLES:
        center = (DZ_X, y, DZ_Z)
        handle = vmf.prism(center, DZ_HANDLE_RADIUS, DZ_HANDLE_HEIGHT, 24, PLATE)
        bar = vmf.box(center, DZ_BAR_HALF, BODY)
        starts.append(spinner(vmf, name, center, [bar, handle], axis, DZ_SPIN_DEG_PER_S,
                              (DZ_X - 176, y, 40)))
    start_on_spawn(vmf, (x0 + 32, 0, 24), starts)
    return vmf.text()


MAPS = {"gyro_lab": build_gyro_lab, "dzhanibekov": build_dzhanibekov}


def stage_compile_game(game, runtime):
    """A game root for the compilers: the vbsp fixture's gameinfo and lights,
    and the map's materials (with base textures, for vrad's reflectivity) and
    surface properties extracted from the staged runtime's packs."""
    shutil.rmtree(game, ignore_errors=True)
    fixture = ROOT / "quality/fixtures/vbsp-host/game"
    game.mkdir(parents=True)
    for name in ("gameinfo.txt", "lights.rad"):
        shutil.copy2(fixture / name, game / name)
    resolver = ContentResolver(runtime)

    def extract(relative, required=True):
        data, _ = resolver.read(relative)
        if data is None:
            if required:
                raise FileNotFoundError("missing from the runtime: " + relative)
            return None
        target = game / relative
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(data)
        return data

    manifest = extract(SURFACE_MANIFEST).decode("utf-8", "replace")
    for name in re.findall(r'"file"\s+"([^"]+)"', manifest):
        extract(name)
    for material in MATERIALS:
        vmt = extract("materials/%s.vmt" % material.lower()).decode("utf-8", "replace")
        texture = re.search(r'"?\$basetexture"?\s+"?([^"\s]+)', vmt, re.IGNORECASE)
        if texture:
            extract("materials/%s.vtf" % texture.group(1).lower().replace("\\", "/"), required=False)


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def run(command, log):
    print("+ " + " ".join(str(c) for c in command), flush=True)
    with open(log, "a") as stream:
        stream.write("+ " + " ".join(str(c) for c in command) + "\n")
        stream.flush()
        subprocess.run([str(c) for c in command], check=True, stdout=stream, stderr=subprocess.STDOUT)


def build_map(name, out, tools, runtime):
    work = out / "compile"
    work.mkdir(parents=True, exist_ok=True)
    vmf = work / (name + ".vmf")
    vmf.write_text(MAPS[name]())
    print("wrote " + str(vmf))
    if tools is None:
        return
    game = work / "game"
    stage_compile_game(game, runtime)
    log = out / "compile.log"
    log.write_text("")
    bsp = work / (name + ".bsp")
    run([tools / "vbsp", "-game", game, vmf], log)
    run([tools / "vvis", "-threads", "4", "-game", game, bsp], log)
    run([tools / "vrad", "-bounce", "2", "-threads", "4", "-game", game, bsp], log)

    content = out / "content"
    (content / "maps").mkdir(parents=True, exist_ok=True)
    target = content / "maps" / (name + ".bsp")
    target.write_bytes(bsp.read_bytes())
    summary = {"map": name, "status": "pass", "failed_gates": [], "bsp2_sha256": sha256(target),
               "content_root": str(content), "vmf_sha256": sha256(vmf),
               "built": datetime.datetime.now(datetime.timezone.utc).isoformat(timespec="seconds")}
    (out / "build.json").write_text(json.dumps(summary, indent=2) + "\n")
    print("published " + playable_maps.describe(playable_maps.publish(summary)))


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--map", action="append", choices=sorted(MAPS),
                        help="map to build (repeatable; default all)")
    parser.add_argument("--out", type=Path, default=ROOT / "quality-results/gyro-maps",
                        help="output root; each map builds in <out>/<map>")
    parser.add_argument("--toolchain", type=Path, default=TOOLCHAIN)
    parser.add_argument("--runtime", type=Path, default=ROOT / "run/runtime",
                        help="staged runtime the map's materials come from")
    parser.add_argument("--vmf-only", action="store_true", help="write the VMFs and stop")
    args = parser.parse_args()

    tools = None if args.vmf_only else Path(json.loads(args.toolchain.read_text())["compile_tools"])
    for name in args.map or sorted(MAPS):
        build_map(name, args.out.resolve() / name, tools, args.runtime.resolve())
    return 0


if __name__ == "__main__":
    sys.exit(main())
