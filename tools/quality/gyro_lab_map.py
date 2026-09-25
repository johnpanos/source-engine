#!/usr/bin/env python3
"""Build and publish `gyro_lab`, a playable map for gyroscopic torque.

The map stages, in game, the scenes of the VPhysics suite's `gyro.*` clauses
(unittests/physicstest/test_vphysics_gyroscopic.cpp):

* a zero-gravity bay (`trigger_vphysics_motion`, gravity scale 0) with four
  48x24x8 boxes, each spun up by a `phys_torque` to 1 rev/s: about 5 degrees
  off the intermediate axis (it flips over and back, the Dzhanibekov effect),
  about the major axis and the minor axis (both steady), and about the body
  diagonal (the
  torque-free tumble: its angular velocity wanders while its angular momentum
  stays fixed; IVP's explicit gyroscopic integration gains energy here, so on
  IVP it tumbles faster and faster);
* a gyroscope: a 96x96 plate on an axle, hung from a `phys_ballsocket` on a
  post. A `phys_hinge` holds the axle level while a `phys_torque` spins the
  plate to 3000 deg/s; the hinge then lets go and the plate precesses about
  the post at m g r / (I3 s), about 10 deg/s, instead of falling.

Each scene restarts from a button in front of it (use key); every scene
starts once at map load. The boxes and the plate have no damping or drag
(`overridescript`), so what is left is the provider's rotation.

    python3 tools/quality/gyro_lab_map.py --out quality-results/gyro-lab
    ./play gyro_lab                   Box3D
    PHYSICS=vphysics ./play gyro_lab  IVP

The map compiles with the pinned legacy vbsp/vvis/vrad of the PBRT map
toolchain (build/toolchains/pbrt-map-toolchain.json) against the staged Portal
runtime, and publishes to run/maps/gyro_lab (tools/quality/playable_maps.py).
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
MAP = "gyro_lab"
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

# Zero-g bay: box centers along x, one scene each.
BAY_Y, BAY_Z = 256, 240
BOX_HALF = (24, 12, 4)       # long x (minor axis), mid y (intermediate), thin z (major)
SPIN_DEG_PER_S = 360.0       # 1 rev/s
SPIN_SECONDS = 0.5
# Each box is axis-aligned: VPhysics takes a brush entity's inertia as
# diagonal in the entity frame (IVP's compact surface has no principal-axis
# rotation), so a rotated brush does not tilt its inertia. The spin axis
# carries the off-axis part instead; phys_torque snaps an axis only within
# 0.002 of a unit axis.
BAYS = [
    # name, x, world torque axis (normalized when written)
    ("intermediate", -240, (0.06, 1.0, 0.06)),   # ~5 degrees off: it flips
    ("major", -24, (0.0, 0.0, 1.0)),
    ("minor", 192, (1.0, 0.0, 0.0)),
    ("tumble", 408, (1.0, 1.0, 1.0)),           # the conformance tumble
]

# Gyroscope: pivot, arm and spin.
PIVOT = (0, -256, 224)
ARM = 24
PLATE_HALF = 48
GYRO_SPIN_DEG_PER_S = 3000.0
GYRO_SPIN_SECONDS = 4.0


class Vmf:
    """A VMF writer for axis-aligned boxes and entities."""

    def __init__(self):
        self.next_id = 1
        self.world = []
        self.entities = []

    def id(self):
        self.next_id += 1
        return self.next_id

    def box(self, center, half, material):
        axes = ((1, 0, 0), (0, 1, 0), (0, 0, 1))
        lines = ["\tsolid", "\t{", '\t\t"id" "%d"' % self.id()]
        for axis in range(3):
            for sign in (1, -1):
                normal = scale(axes[axis], sign)
                face_center = add(center, scale(normal, half[axis]))
                # Two in-face directions u, v with cross(u, v) = normal: vbsp's
                # plane normal is cross(p0 - p1, p2 - p1) for (p1 + u, p1, p1 + v).
                u = axes[(axis + 1) % 3]
                v = axes[(axis + 2) % 3]
                if dot(cross(u, v), normal) < 0:
                    u, v = v, u
                points = (add(face_center, scale(u, 32)), face_center, add(face_center, scale(v, 32)))
                plane = " ".join("(%.4f %.4f %.4f)" % p for p in points)
                uaxis, vaxis = texture_axes(normal)
                lines += ["\t\tside", "\t\t{", '\t\t\t"id" "%d"' % self.id(),
                          '\t\t\t"plane" "%s"' % plane,
                          '\t\t\t"material" "%s"' % material,
                          '\t\t\t"uaxis" "%s 0.25"' % uaxis, '\t\t\t"vaxis" "%s 0.25"' % vaxis,
                          '\t\t\t"rotation" "0"', '\t\t\t"lightmapscale" "16"',
                          '\t\t\t"smoothing_groups" "0"', "\t\t}"]
        return "\n".join(lines + ["\t}"])

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


def scale(a, s):
    return tuple(x * s for x in a)


def dot(a, b):
    return sum(x * y for x, y in zip(a, b))


def cross(a, b):
    return (a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0])


def texture_axes(normal):
    """Hammer's world-aligned texture axes for the normal's dominant axis."""
    axis = max(range(3), key=lambda i: abs(normal[i]))
    return (("[0 1 0 0]", "[0 0 -1 0]") if axis == 0 else
            ("[1 0 0 0]", "[0 0 -1 0]") if axis == 1 else
            ("[1 0 0 0]", "[0 -1 0 0]"))


def vec(v):
    return " ".join("%g" % c for c in v)


def build_vmf():
    vmf = Vmf()
    x0, y0, z0, x1, y1, z1 = ROOM
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
    vmf.entity("info_player_start", {"origin": vec((x0 + 80, 0, 8)), "angles": "0 0 0"})
    for x, y, z in ((-400, 0, 400), (0, 0, 400), (400, 0, 400), (-400, 300, 400),
                    (0, 300, 400), (400, 300, 400), (0, -300, 400), (-400, -300, 400),
                    (400, -300, 400)):
        vmf.entity("light", {"origin": vec((x, y, z)), "_light": "255 250 240 350",
                             "_quadratic_attn": "0", "_linear_attn": "1", "_constant_attn": "0"})

    # Zero-g bay.
    trigger_x0 = BAYS[0][1] - 112
    trigger_x1 = BAYS[-1][1] + 112
    vmf.entity("trigger_vphysics_motion",
               {"spawnflags": "8", "StartDisabled": "0", "SetGravityScale": "0",
                "SetAdditionalAirDensity": "0",
                # A near-zero speed limit holds each box where it is (it has fallen
                # for a moment before the trigger touches it); rotation is free.
                "SetVelocityLimit": "0.5", "SetVelocityLimitDelta": "0",
                "SetVelocityScale": "1", "SetAngVelocityLimit": "0", "SetAngVelocityScale": "1",
                "SetLinearForce": "0", "SetLinearForceAngles": "0 0 0",
                "ParticleTrailMaterial": "", "ParticleTrailLifetime": "4",
                "ParticleTrailStartSize": "2", "ParticleTrailEndSize": "3"},
               solids=[vmf.box(((trigger_x0 + trigger_x1) / 2, BAY_Y, BAY_Z),
                               ((trigger_x1 - trigger_x0) / 2, 144, 144), TRIGGER)])
    starts = []
    for name, x, axis in BAYS:
        length = math.sqrt(dot(axis, axis))
        axis = scale(axis, 1.0 / length)
        center = (x, BAY_Y, BAY_Z)
        body, relay = "box_" + name, "spin_" + name
        vmf.entity("func_physbox",
                   {"targetname": body, "origin": vec(center), "spawnflags": "0",
                    "overridescript": "damping,0,rotdamping,0,drag,0", "massScale": "0",
                    "Damagetype": "0", "health": "0", "material": "2", "nodamageforces": "1",
                    "preferredcarryangles": "0 0 0", "notsolid": "0", "renderamt": "255"},
                   solids=[vmf.box(center, BOX_HALF, BODY)])
        vmf.entity("point_teleport", {"targetname": "reset_" + name, "target": body,
                                      "origin": vec(center), "angles": "0 0 0", "spawnflags": "1"})
        vmf.entity("phys_torque", {"targetname": "torque_" + name, "attach1": body,
                                   "origin": vec(center), "axis": vec(add(center, scale(axis, 64))),
                                   "force": "%g" % (SPIN_DEG_PER_S / SPIN_SECONDS),
                                   "forcetime": "%g" % SPIN_SECONDS, "spawnflags": "0"})
        vmf.entity("logic_relay", {"targetname": relay, "origin": vec((x, BAY_Y, 24))},
                   outputs=[("OnTrigger", "reset_" + name, "Teleport", "", 0),
                            ("OnTrigger", "torque_" + name, "Activate", "", 0.1)])
        button_center = (x, BAY_Y - 160, 40)
        vmf.world.append(vmf.box((x, BAY_Y - 160, 16), (12, 12, 16), PEDESTAL))
        vmf.entity("func_button", {"targetname": "button_" + name, "origin": vec(button_center),
                                   "spawnflags": "1025", "speed": "5", "wait": "1", "lip": "0",
                                   "sounds": "0", "movedir": "0 0 0", "renderamt": "255"},
                   solids=[vmf.box(button_center, (8, 8, 8), BUTTON)],
                   outputs=[("OnPressed", relay, "Trigger", "", 0)])
        starts.append(relay)

    # Gyroscope.
    px, py, pz = PIVOT
    vmf.world.append(vmf.box((px, py, (pz - 12) / 2), (6, 6, (pz - 12) / 2), PEDESTAL))
    plate_center = (px + ARM, py, pz)
    vmf.entity("func_physbox",
               {"targetname": "gyro_plate", "origin": vec(plate_center), "spawnflags": "0",
                "overridescript": "damping,0,rotdamping,0,drag,0", "massScale": "0",
                "Damagetype": "0", "health": "0", "material": "2", "nodamageforces": "1",
                "preferredcarryangles": "0 0 0", "notsolid": "0", "renderamt": "255"},
               solids=[vmf.box(plate_center, (4, PLATE_HALF, PLATE_HALF), PLATE),
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
    button_center = (px - 96, py + 96, 40)
    vmf.world.append(vmf.box((px - 96, py + 96, 16), (12, 12, 16), PEDESTAL))
    vmf.entity("func_button", {"targetname": "button_gyro", "origin": vec(button_center),
                               "spawnflags": "1025", "speed": "5", "wait": "1", "lip": "0",
                               "sounds": "0", "movedir": "0 0 0", "renderamt": "255"},
               solids=[vmf.box(button_center, (8, 8, 8), BUTTON)],
               outputs=[("OnPressed", "spin_gyro", "Trigger", "", 0)])
    starts.append("spin_gyro")

    vmf.entity("logic_auto", {"origin": vec((x0 + 32, 0, 24)), "spawnflags": "1"},
               outputs=[("OnMapSpawn", relay, "Trigger", "", 1.0) for relay in starts])
    return vmf.text()


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


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--out", type=Path, default=ROOT / "quality-results/gyro-lab")
    parser.add_argument("--toolchain", type=Path, default=TOOLCHAIN)
    parser.add_argument("--runtime", type=Path, default=ROOT / "run/runtime",
                        help="staged runtime the map's materials come from")
    parser.add_argument("--vmf-only", action="store_true", help="write the VMF and stop")
    args = parser.parse_args()

    out = args.out.resolve()
    work = out / "compile"
    work.mkdir(parents=True, exist_ok=True)
    vmf = work / (MAP + ".vmf")
    vmf.write_text(build_vmf())
    print("wrote " + str(vmf))
    if args.vmf_only:
        return 0

    tools = Path(json.loads(args.toolchain.read_text())["compile_tools"])
    game = work / "game"
    stage_compile_game(game, args.runtime.resolve())
    log = out / "compile.log"
    log.write_text("")
    bsp = work / (MAP + ".bsp")
    run([tools / "vbsp", "-game", game, vmf], log)
    run([tools / "vvis", "-threads", "4", "-game", game, bsp], log)
    run([tools / "vrad", "-bounce", "2", "-threads", "4", "-game", game, bsp], log)

    content = out / "content"
    (content / "maps").mkdir(parents=True, exist_ok=True)
    target = content / "maps" / (MAP + ".bsp")
    target.write_bytes(bsp.read_bytes())
    summary = {"map": MAP, "status": "pass", "failed_gates": [], "bsp2_sha256": sha256(target),
               "content_root": str(content), "vmf_sha256": sha256(vmf),
               "built": datetime.datetime.now(datetime.timezone.utc).isoformat(timespec="seconds")}
    (out / "build.json").write_text(json.dumps(summary, indent=2) + "\n")
    print("published " + playable_maps.describe(playable_maps.publish(summary)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
