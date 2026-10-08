#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""RFC 0016 K9 "First-party use is zero", runtime half: the legacy stream on
native Portal 2, per map and view.

Boots each map headless (portal_boot.py, native Vulkan, the product's core
settings: r_core_world 1, queued material system), lets it settle, runs the
view's console commands, takes a screenshot and records:

- legacy_stream_draws: the native backend's draws issued from the legacy
  stream after the core's slots and suppression (vulkan-frame-stats/v1),
  over the last settled frames;
- the captured frame's legacy draws by material (the backend's record dump
  at the screenshot, `frame draw ... material=`);
- the backend's dropped materials (`dropped material draws=`): draws neither
  the core nor the stream drew, which are missing effects, not zero use.

    python3 tools/render/legacy_stream_census.py run --out <dir> [--view NAME ...]

Each view boots through tools/quality/portal_boot.py on its kiln profile
(portal2, or portal for the p1-* views; build them with ./kiln build).
    python3 tools/render/legacy_stream_census.py summarize <dir>

`run` writes <dir>/census.json (schema legacy-stream-census/v1) and exits 1
when any view drew from the stream, its core received no mesh draws
(r_core_world_stats) or a boot failed; `summarize` prints it.
--dump-materials also writes each view's mat_dump_material_state (the
load-time state of every material) for comparing shader-library changes.
"""
import argparse
import collections
import json
from pathlib import Path
import re
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
SCHEMA = "legacy-stream-census/v1"
SETTLED = 60

# name: (map, console commands after the map settles)
VIEWS = {
    "intro4-spawn": ("sp_a1_intro4", []),
    "intro4-menu": ("sp_a1_intro4", ["gameui_activate", "wait 30"]),
    "laser-intro": ("sp_a2_laser_intro", []),
    "fizzler-intro": ("sp_a2_fizzler_intro", []),
    "laser-over-goo": ("sp_a2_laser_over_goo", []),
    "jump-intro": ("sp_a3_jump_intro", []),
    "coop-multifling": ("mp_coop_multifling_1", []),
    "intro2-spawn": ("sp_a1_intro2", []),
    # In front of the first fizzler (trigger_portal_cleanser at 128 -432 64).
    "fizzler-close": ("sp_a2_fizzler_intro", ["cmd noclip", "cmd setpos 128 -250 0",
                                             "cmd setang 0 -90 0", "wait 60"]),
    "intro3-spawn": ("sp_a1_intro3", []),
    "wakeup-spawn": ("sp_a1_wakeup", []),
    # The portal fixture room (tools/quality/portal2_portal_map.py): blue on
    # the north wall, orange on the east, looking into blue.
    "portal-walk": ("qa_portal_walk", ["wait 30", "+attack", "wait 10", "-attack", "wait 60",
                                       "cmd setang 0 0 0", "wait 30", "+attack2", "wait 10",
                                       "-attack2", "wait 60", "cmd setang 0 90 0", "wait 60"]),
    # A Wheatley monitor (tools/quality/portal2_monitors.py's placement:
    # 190 units in front of monitor1's screen, after its deploy relay).
    "monitor-tb-intro": ("sp_a4_tb_intro", ["cmd noclip",
                                             "ent_fire monitor1-relay_deploy_straight trigger",
                                             "wait 360", "cmd setpos 1880.62 386 -320.9",
                                             "cmd setang 0 0 0", "wait 120"]),
    # The observation window's glass (static props; the matched game/lab
    # frame of RFC/0016-progress.md's glass cohort).
    "glass-intro4": ("sp_a1_intro4", ["cmd noclip", "cmd setpos -60 260 134",
                                       "cmd setang 0 90 0", "wait 60"]),
    "intro4-portals": ("sp_a1_intro4", ["give weapon_portalgun", "upgrade_portalgun",
                                         "wait 20", "+attack", "wait 5", "-attack", "wait 20",
                                         "cmd setang 0 90 0", "wait 5", "+attack2", "wait 5",
                                         "-attack2", "wait 40"]),
    # Native Portal (RFC 0016 K9 covers both games): the K0 views and one pass
    # of the frame-pacing workload (quality/workloads/portal-frame-pacing-v1).
    "p1-testchmb-a-00": ("testchmb_a_00", []),
    "p1-testchmb-a-08": ("testchmb_a_08", []),
    "p1-testchmb-a-01": ("testchmb_a_01", []),
    "p1-frame-pacing": ("testchmb_a_02", ["god", "notarget", "give weapon_portalgun", "upgrade_portalgun", "upgrade_portalgun", "wait 120", "ent_fire prop_portal fizzle", "setpos -448 150 0", "setang 0 90 0", "wait 90", "wait 120", "+attack", "wait 2", "-attack", "wait 120", "setang 0 270 0", "wait 30", "+attack2", "wait 2", "-attack2", "wait 120", "wait 60", "setang 0 90 0", "wait 60", "+forward", "wait 300", "-forward", "wait 30", "setpos -448 150 0", "setang 0 270 0", "+forward", "wait 300", "-forward", "wait 60"]),
}

# Views on native Portal (the portal kiln profile) rather than Portal 2.
PORTAL1 = {"p1-testchmb-a-00", "p1-testchmb-a-08", "p1-testchmb-a-01", "p1-frame-pacing"}
# Views whose map is a repository fixture rather than game content: its built
# content root (tools/quality/portal2_portal_map.py writes it).
CONTENT = {"portal-walk": "quality-results/portal2-maps/qa_portal_walk/content"}
# Views judged over every frame after the map settles, not the last ones.
ALL_FRAMES = {"p1-frame-pacing"}

DRAW = re.compile(r"\[vulkan\]\s+frame draw tgt=(-?\d+) blend=(\d+)\s+verts=\d+ material=(\S+)")
DROPPED = re.compile(r"dropped material draws=(\d+)\s+(\S+) \[(\w+)\]")
# r_core_world_stats' dynamic line: the mesh draws the core received and refused.
CORE_DYNAMIC = re.compile(r"r_core_world_stats: dynamic draws (\d+) refused (\d+)")


def boot(args, name, out):
    level, commands = VIEWS[name]
    portal1 = name in PORTAL1
    stats = out / (name + ".jsonl")
    cmd = [sys.executable, str(ROOT / "tools/quality/portal_boot.py"),
           "--profile", "portal" if portal1 else "portal2", "--out", str(out / name),
           "--renderer", "native-vulkan",
           "--require-vulkan",
           "--headless", "--map", level, "--timeout", str(args.timeout),
           "--capture-wait", "60", "--no-mouse", "--physics", "vphysics_box3d",
           "--engine-arg=-vkframestats", "--engine-arg=" + str(stats)]
    if name in CONTENT:
        cmd += ["--content-root", str(ROOT / CONTENT[name])]
    for setting in ("sv_cheats 1", "mat_queue_mode 2", "r_core_world 1",
                    "r_indirect_producer baked"):
        cmd += ["--startup-command", setting]
    # "=" keeps commands such as -attack from reading as options.
    tail = ["wait %d" % SETTLED, "r_core_world_stats"]
    if args.dump_materials:
        tail.append("mat_dump_material_state census_materials.jsonl")
    for command in ["wait 120"] + commands + tail:
        cmd.append("--console-command=" + command)
    with open(out / (name + ".log"), "w") as log:
        status = subprocess.run(cmd, cwd=ROOT, stdout=log, stderr=subprocess.STDOUT).returncode
    record = {"view": name, "map": level, "commands": commands, "boot_exit": status}
    try:
        frames = [json.loads(line) for line in stats.read_text().splitlines()[1:]]
    except OSError:
        frames = []
    # Past the map's settling wait (120 frames) for a workload judged whole.
    window = frames[120:] if name in ALL_FRAMES else frames[-SETTLED:]
    settled = [f["legacy_stream_draws"] for f in window]
    record["frames"] = len(frames)
    record["legacy_stream_draws"] = {"max": max(settled, default=None),
                                     "min": min(settled, default=None)}
    stdout = out / name / "stdout.log"
    text = stdout.read_text(errors="replace") if stdout.is_file() else ""
    draws = collections.Counter(m for _, _, m in DRAW.findall(text))
    record["captured_legacy_draws"] = dict(draws.most_common())
    dropped = {}
    for count, material, shader in DROPPED.findall(text):
        dropped[material] = [shader, max(int(count), dropped.get(material, [shader, 0])[1])]
    record["dropped"] = dropped
    # The meshes the engine drew (models, particles, sprites, UI) reach the
    # core: a view with none lost them (e32f58361's DynamicMeshReady regression
    # left this count at 0 while the stream counts stayed 0 too).
    engine_log = out / name / "runtime" / "engine.log"
    log_text = engine_log.read_text(errors="replace") if engine_log.is_file() else text
    dynamic = CORE_DYNAMIC.findall(log_text) or CORE_DYNAMIC.findall(text)
    record["core_dynamic_draws"] = int(dynamic[-1][0]) if dynamic else None
    record["core_dynamic_refused"] = int(dynamic[-1][1]) if dynamic else None
    if args.dump_materials:
        game = "portal" if portal1 else "portal2"
        dumped = out / name / "runtime" / game / "census_materials.jsonl"
        if dumped.is_file():
            shutil.copyfile(dumped, out / (name + ".materials.jsonl"))
        record["materials_dumped"] = dumped.is_file()
    record["zero"] = (status == 0 and bool(settled) and max(settled) == 0 and
                      bool(record["core_dynamic_draws"]))
    return record


def line(record):
    draws = record["legacy_stream_draws"]
    return "%-16s %-22s boot=%d legacy/frame %s..%s core meshes %s captured %s dropped %s" % (
        record["view"], record["map"], record["boot_exit"], draws["min"], draws["max"],
        record.get("core_dynamic_draws"),
        ", ".join("%s x%d" % kv for kv in list(record["captured_legacy_draws"].items())[:6]) or "-",
        ", ".join(sorted(record["dropped"])) or "-")


def run(args):
    out = args.out.resolve()
    out.mkdir(parents=True, exist_ok=True)
    views = args.view or list(VIEWS)
    records = []
    for name in views:
        record = boot(args, name, out)
        records.append(record)
        print(line(record), flush=True)
    census = {"schema": SCHEMA, "views": records}
    (out / "census.json").write_text(json.dumps(census, indent=1) + "\n")
    return 0 if all(r["zero"] for r in records) else 1


def summarize(args):
    census = json.loads((args.dir / "census.json").read_text())
    for record in census["views"]:
        print(line(record))
    return 0


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    sub = parser.add_subparsers(dest="command", required=True)
    r = sub.add_parser("run")
    r.add_argument("--out", type=Path, required=True)
    r.add_argument("--dump-materials", action="store_true",
                   help="write each view's mat_dump_material_state to <out>/<view>.materials.jsonl")
    r.add_argument("--view", action="append", choices=sorted(VIEWS))
    r.add_argument("--timeout", type=int, default=400)
    s = sub.add_parser("summarize")
    s.add_argument("dir", type=Path)
    args = parser.parse_args(argv)
    return run(args) if args.command == "run" else summarize(args)


if __name__ == "__main__":
    raise SystemExit(main())
