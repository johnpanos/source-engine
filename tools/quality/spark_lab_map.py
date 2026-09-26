#!/usr/bin/env python3
"""Build and publish `spark_lab`, the test map for spark-burst lights
(RFC 0011, render.spark-light.v2).

A dim room (so the sparks' light reads against the bake) with four scenes, each
driven by one logic_timer so every burst starts at a known time:

* `fall`: an env_spark 512 units up; its sparks rise, fall to the floor and
  slide. Most of their life is spent well beyond the light's base radius from
  their source, where the first policy stopped counting them.
* `wall`: a directional env_spark 16 units off a wall, spraying into the room.
* `pcf`: an info_particle_system (`ricochet_sparks`, drawn with
  effects/spark) restarted on the timer: the particle-system path.
* `budget`: six env_sparks in a row, fired together, more than the
  fx_spark_lights budget (4). The row runs away from the player start, so the
  nearest bursts must be the lit ones.

The fall, wall and pcf scenes fire at the start of each 8 s cycle; the budget
row fires 5 s in, once the other sparks have died.

    python3 tools/quality/spark_lab_map.py          build and publish
    ./play spark_lab                                play it
    python3 tools/quality/spark_light_scene.py run  the in-engine fixtures

The map compiles with the pinned legacy vbsp/vvis/vrad of the PBRT map
toolchain (build/toolchains/pbrt-map-toolchain.json) against the staged Portal
runtime, and publishes to run/maps/spark_lab (tools/quality/playable_maps.py).
"""

import argparse
import json
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import playable_maps  # noqa: E402
import vmf_map_build  # noqa: E402
from gyro_lab_map import Vmf, room, vec  # noqa: E402  (the VMF writer)

ROOT = HERE.parents[1]
TOOLCHAIN = ROOT / "build/toolchains/pbrt-map-toolchain.json"
NAME = "spark_lab"

ROOM = (-1024, -512, 0, 1024, 512, 576)
SPAWN = (1000, 380, 8)
SPAWN_ANGLES = "0 180 0"
CYCLE = 8.0
BUDGET_DELAY = 5.0

# Scenes: name -> (classname, origin, keyvalues). Env_spark spawnflags: 512
# directional (along its angles), 256 silent.
SCENES = {
    "fall": ("env_spark", (-768, 0, 512),
             {"Magnitude": "2", "TrailLength": "2", "MaxDelay": "0", "spawnflags": "256"}),
    "wall": ("env_spark", (-256, -496, 192),
             {"Magnitude": "2", "TrailLength": "2", "MaxDelay": "0", "spawnflags": "768",
              "angles": "0 90 0"}),
    "pcf": ("info_particle_system", (256, 0, 64),
            {"effect_name": "ricochet_sparks", "start_active": "0", "angles": "-90 0 0"}),
}
# The budget row: magnitude 1 (base radius 128), 112 units apart along -x.
BUDGET = [(960 - 112 * i, 300, 96) for i in range(6)]


def scene_layout():
    """Every burst source, for the fixtures: name -> origin."""
    layout = {name: origin for name, (_, origin, _) in SCENES.items()}
    layout.update({"budget%d" % i: origin for i, origin in enumerate(BUDGET)})
    return layout


def build_vmf():
    vmf = Vmf()
    # Dim: the bake is a faint fill, so the sparks' light is plain to see.
    room(vmf, ROOM, SPAWN, [])
    for origin in [(-600, 0, 540), (0, 0, 540), (600, 0, 540)]:
        vmf.entity("light", {"origin": vec(origin), "_light": "200 200 210 40",
                             "_quadratic_attn": "0", "_linear_attn": "1", "_constant_attn": "0"})
    # The player start faces down the room.
    vmf.entities = [e.replace('"angles" "0 0 0"', '"angles" "%s"' % SPAWN_ANGLES)
                    if '"info_player_start"' in e else e for e in vmf.entities]

    outputs = []
    for name, (classname, origin, keys) in SCENES.items():
        target = "spark_" + name
        vmf.entity(classname, dict({"targetname": target, "origin": vec(origin)}, **keys))
        if classname == "env_spark":
            outputs.append(("OnTimer", target, "SparkOnce", "", 0))
        else:
            # A restart is a new burst.
            outputs += [("OnTimer", target, "Stop", "", 0), ("OnTimer", target, "Start", "", 0.05)]
    for i, origin in enumerate(BUDGET):
        target = "spark_budget%d" % i
        vmf.entity("env_spark", {"targetname": target, "origin": vec(origin), "Magnitude": "1",
                                 "TrailLength": "1", "MaxDelay": "0", "spawnflags": "256"})
        outputs.append(("OnTimer", target, "SparkOnce", "", BUDGET_DELAY))
    vmf.entity("logic_timer", {"targetname": "spark_cycle", "origin": "0 0 32",
                               "RefireTime": "%g" % CYCLE, "StartDisabled": "0",
                               "UseRandomTime": "0", "spawnflags": "0"},
               outputs=outputs)
    return vmf.text()


def build(out, tools, runtime):
    work = out / "compile"
    work.mkdir(parents=True, exist_ok=True)
    vmf = work / (NAME + ".vmf")
    vmf.write_text(build_vmf())
    (out / "layout.json").write_text(json.dumps(
        {"map": NAME, "cycle": CYCLE, "budget_delay": BUDGET_DELAY, "spawn": SPAWN,
         "sources": scene_layout()}, indent=2) + "\n")
    print("wrote " + str(vmf))
    if tools is None:
        return
    # The shared VMF compile owns staging, vbsp/vvis/vrad, leak detection and
    # packaging (tools/quality/vmf_map_build.py).
    record = vmf_map_build.build(vmf, out, tools, runtime, quality="full", name=NAME)
    if record["status"] != "pass":
        raise RuntimeError("%s: %s (%s)" % (NAME, record["status"], out / "build.json"))
    print("published " + playable_maps.describe(playable_maps.publish(record)))


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--out", type=Path, default=ROOT / "quality-results/spark-light/map",
                        help="output directory")
    parser.add_argument("--toolchain", type=Path, default=TOOLCHAIN)
    parser.add_argument("--runtime", type=Path, default=ROOT / "run/runtime",
                        help="staged runtime the map's materials come from")
    parser.add_argument("--vmf-only", action="store_true", help="write the VMF and stop")
    args = parser.parse_args()
    tools = None if args.vmf_only else Path(json.loads(args.toolchain.read_text())["compile_tools"])
    build(args.out.resolve(), tools, args.runtime.resolve())
    return 0


if __name__ == "__main__":
    sys.exit(main())
