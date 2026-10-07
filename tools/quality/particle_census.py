#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Census of Portal 2 particle draws on the render core (RFC 0016 K8 particles).

Boots one retail map headless (portal_boot.py, native Vulkan, r_core_world 1),
spawns a fixed set of particle effects with ent_create info_particle_system,
and reads two counts once they have drawn:

- the core's named refusals (r_core_world_stats' gaps): each dynamic draw
  the core refused, by material and reason;
- the native backend's drop census (`dropped material draws=` at each
  screenshot): each draw the backend dropped without the core drawing it.

Only SpriteCard and Refract materials and particle/effects paths are kept.

    python3 tools/quality/particle_census.py run --runtime run/runtime-p2-particles \\
        --map sp_a1_intro4 --out <dir> [--setpos "120 272 0" --setang "0 0 0"]
    python3 tools/quality/particle_census.py summarize <dir>...

`run` writes <dir>/census.json; `summarize` prints one line per run.
`--dynamic-draws 1` also routes the experimental r_core_dynamic_draws handoff
(the baseline census used it to name SpriteCard refusals before particles had
their own handoff kind).
"""

import argparse
import json
from pathlib import Path
import re
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
SCHEMA = "particle-census/v1"
# Effects whose materials cover the cohort: sparks, fizzler edges, the portal
# close flash, gel and case bubbles (warp), both fling trails (warp rope),
# dissolve glow and water wash (warp), the GLaDOS stream (warp rope), rain
# (warp trail), smoke, steam, fire, confetti, laser-cutter sparks and paint.
EFFECTS = ("impact_physics_sparks", "sparks_generic_random", "portal_close_flash",
           "Cleanser_edge_1", "portal_cleanser", "gel_bubbles", "case_bubbles",
           "bot_fling_trail", "bot_fling_trail_rainbow", "dissolve_glow", "water_wash_c",
           "glados_stream_01", "rain", "human_cleanser", "steam_long", "coop_spawntube_steam",
           "Dust_Ceiling_Rumble_256Line", "br_fire_line", "confetti", "laser_cutter_sparks",
           "paint_bomb_speed")
DROPPED = re.compile(r"dropped material draws=(\d+)\s+(\S+) \[(\w+)\]")
GAP = re.compile(r"^(\d+) material (\S+): (.*)$", re.M)


def particle(material, why=""):
    return ("particle" in material or material.startswith("effects/") or
            "spritecard" in why.lower())


def run(args):
    out = args.out.resolve()
    if out.exists():
        sys.exit("particle_census: %s exists; use a fresh directory" % out)
    commands = ["cmd noclip", "r_drawviewmodel 0"]
    if args.setpos:
        commands += ["cmd setpos " + args.setpos, "cmd setang " + (args.setang or "0 0 0")]
    commands.append("wait 60")
    for index, effect in enumerate(EFFECTS):
        commands += ["cmd ent_create info_particle_system effect_name %s start_active 1 "
                     "targetname pc_%d" % (effect, index), "wait 4"]
    commands += ["wait 90", "screenshot", "wait 6", "cl_render_debug_claims",
                 "r_core_world_stats"]
    boot = [sys.executable, str(ROOT / "tools/quality/portal_boot.py"),
            "--runtime", str(args.runtime.resolve()), "--out", str(out),
            "--game", "portal2", "--renderer", "native-vulkan", "--require-vulkan",
            "--headless", "--map", args.map, "--timeout", str(args.timeout),
            "--capture-wait", "60", "--no-mouse", "--physics", "vphysics_box3d"]
    for setting in ("sv_cheats 1", "mat_queue_mode 2", "r_core_world 1",
                    "r_core_world_strict 0", "r_core_dynamic_draws %d" % args.dynamic_draws,
                    "r_indirect_producer baked"):
        boot += ["--startup-command", setting]
    for command in commands:
        boot += ["--console-command", command]
    with open(str(out) + ".log", "w") as log:
        status = subprocess.run(boot, cwd=ROOT, stdout=log, stderr=subprocess.STDOUT).returncode
    record = {"schema": SCHEMA, "map": args.map, "boot_exit": status,
              "dynamic_draws": args.dynamic_draws, "effects": list(EFFECTS),
              "command": boot}
    record.update(read(out))
    (out / "census.json").write_text(json.dumps(record, indent=1) + "\n")
    print("%s: %s" % (args.map, line(record)))
    return 0 if status == 0 and record.get("complete") else 1


def read(out):
    console = out / "runtime/portal2/console.log"
    stdout = out / "stdout.log"
    if not console.is_file() or not stdout.is_file():
        return {"complete": False}
    text = console.read_text(errors="replace")
    if "r_core_world_stats: gaps:" not in text:
        refusals = []
    else:
        gaps = text.split("r_core_world_stats: gaps:")[-1].split("r_core_world_stats: drawn")[0]
        refusals = [[m, why, int(n)] for n, m, why in GAP.findall(gaps) if particle(m, why)]
    drops = {}
    for n, material, shader in DROPPED.findall(stdout.read_text(errors="replace")):
        if shader.lower().startswith(("spritecard", "refract")):
            drops[material] = [shader, int(n)]
    return {"complete": "r_core_world_stats: dynamic draws" in text,
            "core_refusals": sorted(refusals, key=lambda r: -r[2]),
            "backend_drops": drops}


def line(record):
    refusals = record.get("core_refusals", [])
    drops = record.get("backend_drops", {})
    card = sum(n for shader, n in drops.values() if shader.lower().startswith("spritecard"))
    warp = sum(n for shader, n in drops.values() if shader.lower().startswith("refract"))
    return ("core refusals %d materials / %d draws; backend drops SpriteCard %d, Refract %d" %
            (len(refusals), sum(r[2] for r in refusals), card, warp))


def summarize(args):
    for directory in args.dirs:
        record = json.loads((directory / "census.json").read_text())
        print("%s: %s" % (record["map"], line(record)))
        for material, why, count in record.get("core_refusals", []):
            print("    %6d %s: %s" % (count, material, why))
    return 0


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    sub = parser.add_subparsers(dest="command", required=True)
    r = sub.add_parser("run")
    r.add_argument("--runtime", type=Path, required=True)
    r.add_argument("--map", required=True)
    r.add_argument("--out", type=Path, required=True)
    r.add_argument("--setpos")
    r.add_argument("--setang")
    r.add_argument("--dynamic-draws", type=int, choices=(0, 1), default=0)
    r.add_argument("--timeout", type=int, default=400)
    s = sub.add_parser("summarize")
    s.add_argument("dirs", type=Path, nargs="+")
    args = parser.parse_args(argv)
    return run(args) if args.command == "run" else summarize(args)


if __name__ == "__main__":
    raise SystemExit(main())
