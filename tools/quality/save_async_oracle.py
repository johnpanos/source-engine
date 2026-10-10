#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""RFC 0003 J8 oracle for the save write queue (engine/host_saverestore.cpp).

The queued writes run on the blocking lane (save_async 1) or inline
(save_async 0). Both must land the same files in the same order, load to the
same position, and a quit or map change right after a save must wait for the
writes. One boot per phase, headless, in a private copy of a staged runtime:

  phase order   paused, in one run: three saves per mode. A .sav embeds the
                .hl1/.hl2/.hl3 state files the same save wrote, so the last
                save of the run must contain the on-disk state files byte for
                byte (strict; a reordered drain moves DirectoryCopy before the
                AsyncWrite it packs and fails here), run once with async last
                and once with inline last. Sizes must match across modes. The
                engine also writes uninitialised struct padding (words that
                differ between two saves of one mode), so the words that
                differ between the modes are compared with the same-mode
                noise: more than the noise itself fails.
  phase load    each mode's save loads and places the player the same.
  phase block   a save followed at once by quit (and by a map change) must be
                complete: both reload to the same position.

    python3 tools/quality/save_async_oracle.py --profile portal-linux-native-vulkan \\
        --out DIR [--runtime DIR] [--lib bin/libengine.so=FILE] [--map testchmb_a_01]

Exit 0 on pass. --lib overrides a file in the runtime copy (a baseline or a
deliberately broken engine).
"""

import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import time

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "kiln"))
import sepipe_loader  # noqa: E402

SETPOS = re.compile(r"^setpos (\S+ \S+ \S+);setang (\S+ \S+ \S+)$", re.M)


def boot(runtime, out, game, map_name, commands, timeout=300):
    """Run one headless session; returns (returncode, console log text)."""
    out.mkdir(parents=True, exist_ok=True)
    cfg = runtime / game / "cfg"
    cfg.mkdir(parents=True, exist_ok=True)
    (cfg / "save_oracle_start.cfg").write_text("sv_autosave 1\n")
    # One line: a multi-line cfg ignores `wait`.
    (cfg / "save_oracle.cfg").write_text("; ".join(commands + ["quit"]) + "\n")
    log = runtime / game / "console.log"
    log.unlink(missing_ok=True)
    home = out / "home"
    home.mkdir(exist_ok=True)
    env = dict(os.environ, HOME=str(home), XDG_RUNTIME_DIR=str(home),
               SDL_VIDEODRIVER="offscreen", SDL_VIDEO_DRIVER="offscreen",
               SDL_JOYSTICK_ALLOW_BACKGROUND_EVENTS="1", MALLOC_PERTURB_="85")
    for name in ("DISPLAY", "WAYLAND_DISPLAY"):
        env.pop(name, None)
    command = ["./hl2_launcher", "-game", game, "-windowed", "-w", "1024", "-h", "768",
               "-multirun", "-novid", "-insecure", "-console", "-condebug", "-dev",
               "-physics", "vphysics", "+sv_cheats", "1", "+mat_queue_mode", "0",
               "+mat_vsync", "0", "+fps_max", "60", "+exec", "save_oracle_start.cfg",
               "+volume", "0", "+map", map_name, "+wait", "300", "+exec", "save_oracle.cfg"]
    try:
        run = subprocess.run(command, cwd=runtime, env=env, timeout=timeout,
                             capture_output=True, text=True)
        code = run.returncode
    except subprocess.TimeoutExpired:
        code = "timeout"
    text = log.read_text(errors="replace") if log.is_file() else ""
    (out / "console.log").write_text(text)
    return code, text


def words_that_differ(first, second):
    return {i // 4 for i in range(min(len(first), len(second))) if first[i] != second[i]}


def positions(text):
    return SETPOS.findall(text)


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--profile", default="portal-linux-native-vulkan")
    parser.add_argument("--flavor", default="dev")
    parser.add_argument("--runtime", type=Path, help="a staged runtime to copy (else packaged)")
    parser.add_argument("--map", default="testchmb_a_01")
    parser.add_argument("--map-after", default="testchmb_a_02")
    parser.add_argument("--lib", action="append", default=[], metavar="REL=FILE")
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args(argv)
    game = sepipe_loader.game_of(args.profile)
    out = args.out.resolve()
    shutil.rmtree(out, ignore_errors=True)
    out.mkdir(parents=True)
    template = out / "template"
    if args.runtime:
        subprocess.check_call(["cp", "-a", "--reflink=auto", str(args.runtime), str(template)])
    else:
        session = sepipe_loader.load().Session(str(Path(__file__).resolve().parents[2]))
        session.build(args.profile, flavor=args.flavor, up_to="package", runtime=str(template))
    for override in args.lib:
        rel, source = override.split("=", 1)
        shutil.copy2(source, template / rel)
    shutil.rmtree(template / game / "save", ignore_errors=True)

    def fresh(name):
        path = out / name / "runtime"
        path.parent.mkdir(parents=True, exist_ok=True)
        subprocess.check_call(["cp", "-a", "--reflink=auto", str(template), str(path)])
        return path

    failures = []
    report = {"schema": "save-async-oracle/v1", "map": args.map, "game": game}

    # phase order + load, one boot: paused, three saves per mode, then reload.
    def order_commands(modes):
        commands = ["wait 100", "pause", "wait 40"]
        for mode in modes:
            commands += ["save_async %d" % mode]
            for take in "abc":
                commands += ["save s%d%s" % (mode, take), "wait 100"]
        return commands

    def embedded_in_last(runtime, last):
        saves = runtime / game / "save"
        container = (saves / ("s%dc.sav" % last)).read_bytes()
        files = sorted(saves.glob(args.map + ".hl*"))
        if not files:
            return ["no state files for %s on disk" % args.map]
        return ["s%dc.sav does not contain %s" % (last, f.name) for f in files
                if f.read_bytes() not in container]

    runtime = fresh("order")
    commands = order_commands((1, 0))
    # quicksave and autosave in each mode: they land and are non-empty.
    for mode in (1, 0):
        commands += ["save_async %d" % mode, "save quick", "wait 100", "autosave", "wait 100"]
    commands += ["load s1a", "wait 400", "getpos", "wait 30",
                 "load s0a", "wait 400", "getpos", "wait 30"]
    code, text = boot(runtime, out / "order", game, args.map, commands)
    report["order"] = {"returncode": code}
    saves = runtime / game / "save"
    names = ["s%d%s" % (mode, take) for mode in (1, 0) for take in "abc"]
    data = {name: (saves / (name + ".sav")).read_bytes()
            if (saves / (name + ".sav")).is_file() else b"" for name in names}
    if code != 0:
        failures.append("order boot returned %r" % (code,))
    for name, content in data.items():
        if not content:
            failures.append("%s.sav is missing or empty" % name)
    if all(data.values()):
        noise = set()
        for mode in (1, 0):
            for first, second in (("a", "b"), ("a", "c"), ("b", "c")):
                noise |= words_that_differ(data["s%d%s" % (mode, first)], data["s%d%s" % (mode, second)])
        cross = set()
        for first in ("s1a", "s1b", "s1c"):
            for second in ("s0a", "s0b", "s0c"):
                if len(data[first]) != len(data[second]):
                    failures.append("%s and %s differ in size (%d, %d)" % (
                        first, second, len(data[first]), len(data[second])))
                else:
                    cross |= words_that_differ(data[first], data[second])
        report["order"].update(noise_words=len(noise), cross_words=len(cross),
                               unexplained_words=len(cross - noise),
                               sizes={name: len(content) for name, content in data.items()})
        # Modes add no systematic difference: what differs between them is
        # no more than the padding noise itself.
        if len(cross - noise) > len(noise):
            failures.append("a1/a0 saves differ at %d word(s) beyond the %d noise word(s)" % (
                len(cross - noise), len(noise)))
    for name in ("quick", "autosave"):
        path = saves / (name + ".sav")
        if not path.is_file() or path.stat().st_size == 0:
            failures.append("%s.sav is missing or empty" % name)
    found = positions(text)
    report["load"] = {"positions": found}
    if len(found) != 2 or found[0] != found[1]:
        failures.append("loaded positions differ or are missing: %r" % (found,))

    # Last save inline, then last save on the lane: the embedded state files
    # must match the disk's, with no load in between to overwrite them.
    for label, modes, last in (("inline-last", (1, 0), 0), ("async-last", (0, 1), 1)):
        runtime = fresh("order-" + label)
        code, _ = boot(runtime, out / ("order-" + label), game, args.map, order_commands(modes))
        if code != 0:
            failures.append("order-%s boot returned %r" % (label, code))
        elif (runtime / game / "save" / ("s%dc.sav" % last)).is_file():
            failures += ["order-%s: %s" % (label, text) for text in embedded_in_last(runtime, last)]
        else:
            failures.append("order-%s: s%dc.sav is missing" % (label, last))

    # phase block: quit, and a map change, right after an async save.
    for label, tail in (("quit", ["quit"]), ("map", ["map " + args.map_after, "wait 300"])):
        runtime = fresh("block-" + label)
        code, _ = boot(runtime, out / ("block-" + label), game, args.map,
                       ["wait 100", "pause", "wait 40", "save_async 1", "save blk"] + tail)
        path = runtime / game / "save" / "blk.sav"
        if not path.is_file() or path.stat().st_size == 0:
            failures.append("block-%s: blk.sav missing after the exit/map change" % label)
            continue
        reload = fresh("reload-" + label)
        shutil.copytree(path.parent, reload / game / "save", dirs_exist_ok=True)
        code, text = boot(reload, out / ("reload-" + label), game, args.map,
                          ["wait 100", "load blk", "wait 400", "getpos", "wait 30"])
        found = positions(text)
        report["block-" + label] = {"size": path.stat().st_size, "positions": found}
        if code != 0 or not found:
            failures.append("block-%s: blk.sav did not load (rc %r, positions %r)" % (label, code, found))
    report["failures"] = failures
    report["status"] = "fail" if failures else "pass"
    (out / "report.json").write_text(json.dumps(report, indent=1))
    for failure in failures:
        print("FAIL save-async-oracle: " + failure)
    print("save-async-oracle: " + report["status"] + " (" + str(out / "report.json") + ")")
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
