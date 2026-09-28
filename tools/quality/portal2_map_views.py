#!/usr/bin/env python3
"""Boot a Portal 2 map headless and capture a scripted set of views.

The Portal 2 build (`build-p2`) is staged into a private runtime (never the
user's run/runtime-p2, whose cfg a headless run would overwrite) with the
map pipeline's published maps mounted, and runs offscreen (SDL's offscreen
video driver: no window reaches the desktop). A step list drives it after the
map loads:

    view NAME X Y Z PITCH YAW     noclip the player there and take a screenshot
    do COMMANDS                   run console commands (e.g. "ent_fire button PressIn")
    wait FRAMES                   let FRAMES frames pass

Each screenshot is saved as <out>/<NAME>.tga; <out>/console.log holds the
console and <out>/views.json the result: a run passes when the map loads,
every view is captured, and the console reports no rejected map lump, missing
model or material, or engine error.

    python3 tools/quality/portal2_map_views.py --map sp_gi_chamber_01 --out DIR \\
        --step "view entry -1000 0 0 0 0" --step "do ent_fire button PressIn" ...
"""

import argparse
import json
import os
import re
import shutil
import signal
import subprocess
import sys
import time
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import playable_maps  # noqa: E402
import portal_boot  # noqa: E402
import stage_portal2_runtime  # noqa: E402

ROOT = HERE.parents[1]
SCHEMA = "portal2-map-views/v1"
CFG = "qa_map_views"
# Console lines that fail a run: a rejected pipeline lump (an unlit world), a
# model that did not load, or a map that did not load.
PROBLEMS = re.compile(r"(WMSH|LMAP|PRBV|SDFV|RTRN|RPRB)\b.*rejected|Host_Error|map load failed|"
                      r"Error loading model|Model .* not found|Bad model", re.IGNORECASE)
# Lines worth reading that do not fail a run: missing files the map names
# (Portal 2's own HUD layouts and sprites are missing in every staged run).
NOTES = re.compile(r"couldn't find|can't find|unable to load|failed to load|missing",
                   re.IGNORECASE)


def parse_step(text):
    kind, _, rest = text.strip().partition(" ")
    if kind == "view":
        parts = rest.split()
        if len(parts) != 6:
            raise ValueError("view needs NAME X Y Z PITCH YAW: " + text)
        return {"kind": "view", "name": parts[0], "origin": [float(v) for v in parts[1:4]],
                "angles": [float(parts[4]), float(parts[5]), 0.0]}
    if kind == "do":
        return {"kind": "do", "commands": rest}
    if kind == "wait":
        return {"kind": "wait", "frames": int(rest)}
    raise ValueError("unknown step: " + text)


def write_cfg(path, steps, settle, shot_frames):
    """One alias per step, each chaining to the next: the console's `wait`
    only delays the rest of its own line, so every step is one line."""
    lines = ["sv_cheats 1", "cl_drawhud 0", "r_drawviewmodel 0", "crosshair 0",
             "hud_saytext_time 0", "developer 0", "god", "notarget", "noclip"]
    names = ["qa_step%d" % i for i in range(len(steps))] + ["qa_finish"]
    for index, step in enumerate(steps):
        after = names[index + 1]
        if step["kind"] == "view":
            x, y, z = step["origin"]
            pitch, yaw, _ = step["angles"]
            body = ("setpos %g %g %g; setang %g %g 0; wait %d; echo QA_VIEW %s; screenshot; "
                    "wait %d; %s" % (x, y, z, pitch, yaw, settle, step["name"], shot_frames,
                                     after))
        elif step["kind"] == "do":
            body = "%s; wait 2; %s" % (step["commands"], after)
        else:
            body = "wait %d; %s" % (step["frames"], after)
        if len(body) > 480:
            raise ValueError("step too long for one console line: " + body)
        lines.append('alias %s "%s"' % (names[index], body.replace('"', "'")))
    lines.append('alias qa_finish "echo QA_DONE; wait 30; quit"')
    lines.append(names[0])
    path.write_text("\n".join(lines) + "\n")


def stage(runtime, steam_root, build):
    stage_portal2_runtime.stage_content(steam_root, runtime, mount_custom=True)
    portal_boot.install_build(build, runtime, game="portal2")
    mounted, _ = playable_maps.mount(runtime, game="portal2")
    return sorted(mounted)


def run(args):
    out = args.out.resolve()
    out.mkdir(parents=True, exist_ok=True)
    runtime = args.runtime.resolve()
    steps = [parse_step(s) for s in args.step]
    mounted = stage(runtime, args.steam_root, args.build.resolve())
    game = runtime / "portal2"
    shots = game / "screenshots"
    shutil.rmtree(shots, ignore_errors=True)
    write_cfg(game / "cfg" / (CFG + ".cfg"), steps, args.settle, args.shot_frames)
    console = game / "console.log"
    console.unlink(missing_ok=True)
    environment = dict(os.environ)
    for variable in ("DISPLAY", "WAYLAND_DISPLAY"):
        environment.pop(variable, None)
    environment.update({"SteamAppId": "620", "SteamGameId": "620",
                        "LD_LIBRARY_PATH": str(runtime / "bin"),
                        "SDL_VIDEODRIVER": "offscreen", "SDL_VIDEO_DRIVER": "offscreen"})
    command = [str(runtime / "hl2_launcher"), "-game", "portal2", "-multirun", "-novid",
               "-insecure", "-windowed", "-w", str(args.width), "-h", str(args.height),
               "-condebug", "-physics", args.physics, "+volume", "0", *args.engine_arg,
               "+map", args.map, "+wait", str(args.start_frames), "+exec", CFG]
    started = time.monotonic()
    timed_out = False
    with (out / "stdout.log").open("wb") as stream:
        process = subprocess.Popen(command, cwd=runtime, env=environment, stdout=stream,
                                   stderr=subprocess.STDOUT, start_new_session=True)
        try:
            process.wait(timeout=args.timeout)
        except subprocess.TimeoutExpired:
            timed_out = True
            os.killpg(process.pid, signal.SIGTERM)
            try:
                process.wait(timeout=10)
            except subprocess.TimeoutExpired:
                os.killpg(process.pid, signal.SIGKILL)
                process.wait()
    log = console.read_text(errors="replace") if console.is_file() else ""
    (out / "console.log").write_text(log)
    names = [s["name"] for s in steps if s["kind"] == "view"]
    files = sorted(shots.glob("*.tga"), key=lambda p: p.stat().st_mtime) if shots.is_dir() else []
    captured = {}
    for name, source in zip(names, files):
        target = out / (name + ".tga")
        shutil.copy2(source, target)
        captured[name] = str(target)
    problems = sorted({line.strip() for line in log.splitlines() if PROBLEMS.search(line)})
    notes = sorted({line.strip() for line in log.splitlines()
                    if NOTES.search(line) and ("maps/" + args.map in line.lower() or
                                               "models/" in line.lower())})
    failures = []
    if timed_out:
        failures.append("timed out after %ds" % args.timeout)
    if "QA_DONE" not in log:
        failures.append("the step chain did not finish")
    if len(files) != len(names):
        failures.append("%d of %d views captured" % (len(files), len(names)))
    failures += ["console: " + p for p in problems]
    result = {"schema": SCHEMA, "map": args.map, "status": "fail" if failures else "pass",
              "failures": failures, "notes": notes, "views": captured, "steps": steps,
              "mounted_maps": len(mounted), "seconds": round(time.monotonic() - started, 1),
              "returncode": process.returncode, "command": command}
    (out / "views.json").write_text(json.dumps(result, indent=2) + "\n")
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--map", required=True)
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--step", action="append", default=[], help="view, do or wait step")
    parser.add_argument("--build", type=Path, default=ROOT / "build-p2")
    parser.add_argument("--runtime", type=Path, default=ROOT / "run/runtime-p2-views",
                        help="private staged runtime (created on first use)")
    parser.add_argument("--steam-root", type=Path, default=Path(os.environ.get(
        "P2_STEAM_ROOT", Path.home() / ".local/share/Steam/steamapps/common/Portal 2")))
    parser.add_argument("--physics", default="vphysics_box3d")
    parser.add_argument("--engine-arg", action="append", default=[])
    parser.add_argument("--width", type=int, default=1600)
    parser.add_argument("--height", type=int, default=900)
    parser.add_argument("--start-frames", type=int, default=200)
    parser.add_argument("--settle", type=int, default=90,
                        help="frames between placing the camera and the screenshot")
    parser.add_argument("--shot-frames", type=int, default=60,
                        help="frames after a screenshot (the native writer drops overlapping "
                             "requests)")
    parser.add_argument("--timeout", type=int, default=420)
    args = parser.parse_args()
    result = run(args)
    print(json.dumps({k: result[k] for k in ("status", "failures", "seconds")}, indent=2))
    return 0 if result["status"] == "pass" else 1


if __name__ == "__main__":
    sys.exit(main())
