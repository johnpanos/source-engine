#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Boot every retail map of Portal or Portal 2 with the render core drawing the world.

RFC 0016 K5 (row R89): `r_core_world 1` makes the render core draw the BSP
world surfaces its material model claims, and `r_core_world_strict 1` (the
default) turns any view or claimed material the core fails to draw into a
fatal Sys_Error. This smoke test loads each retail map in turn, headless (the
SDL offscreen driver, real GPU rendering), and judges the core per map:

    python3 tools/quality/core_world_smoke.py run --game portal --out <dir>
    python3 tools/quality/core_world_smoke.py run --game portal2 --out <dir> \\
        --build build-p2 --jobs 2
    python3 tools/quality/core_world_smoke.py list --game portal2
    python3 tools/quality/core_world_smoke.py self-test [--seed-fault NAME]

The maps come from the content, not a hand list: every top-level maps/*.bsp
of the game's own search path (VPKs and loose directories) in the Portal
runtime (run/runtime) or the Portal 2 Steam installation. The workload
(quality/workloads/core-world-smoke-v1.json) excludes non-playable maps, each
with its reason, and records the known failures, each with its owner.

One engine process loads many maps. A chain of generated cfg files runs, per
map: `echo CWS_BEGIN`, `map <name>`, `echo CWS_MAPPED`, a settle wait,
`status`, `r_core_world_stats`, `echo CWS_END`, then the next map's cfg. When
a process dies (Sys_Error, crash) or a map stops making progress (timeout),
the map it was on is recorded and a new process continues from the next map,
so one failure hides no other.

Verdicts per map:

- pass: loaded, no fatal error, the core drew views (drawn > 0), failed 0;
- no-claims: loaded, no fatal error, the core queued no view (it claimed no
  surface, or none of its claimed surfaces was in a back-buffer view);
- core-failure: fails with r_core_world 1 but loads with r_core_world 0 (a
  control process runs every failing map again without the core);
- engine-failure: fails in the control run too: the engine's own problem.

Evidence: evidence.json (per-map records, processes, commands) and
summary.txt in --out. The run ends with one checks-v1 record: unexpected
results (a new failure, a known failure that changed or now passes) print
first as `FAIL unexpected.<game>.<map>`, then each known failure as
`FAIL known-failure.<game>.<map>`, so a conformance row that expects the
known failures names the first of them and any new one breaks the match.
"""

import argparse
import datetime
import json
import os
from pathlib import Path
import re
import shutil
import signal
import subprocess
import sys
import time

import conformance
import conformance_result
import launch_sandbox
import portal_boot
import source_content
import stage_portal2_runtime

ROOT = Path(__file__).resolve().parents[2]
DEFAULT_WORKLOAD = ROOT / "quality/workloads/core-world-smoke-v1.json"
EVIDENCE_SCHEMA = "core-world-smoke-evidence/v1"
WORKLOAD_SCHEMA = "core-world-smoke-workload/v1"
DEFAULT_P2_STEAM_ROOT = Path.home() / ".local/share/Steam/steamapps/common/Portal 2"
# Loose maps linked from the map pipeline's published tree are not retail.
PUBLISHED_MAPS = ROOT / "run/maps"
GAME_LAYERS = {
    # The game's own search path: its VPK and loose directory (Portal's HL2
    # layers hold no Portal map).
    "portal": source_content.SEARCH_PATHS[:2],
    "portal2": source_content.PORTAL2_SEARCH_PATHS,
}
STEAM_APP_IDS = {"portal": "400", "portal2": "620"}
SCRIPT_PREFIX = "cws_"
MAP_NAME = re.compile(r"[A-Za-z0-9_]+")

STATS_LINE = re.compile(
    r"r_core_world_stats: materials (?P<materials>\d+) claimed (?P<claimed_materials>\d+) "
    r"surfaces (?P<surfaces>\d+) claimed (?P<claimed_surfaces>\d+) views queued (?P<queued>\d+) "
    r"drawn (?P<drawn>\d+) failed (?P<failed>\d+) skipped (?P<skipped>\d+) "
    r"surfaces drawn (?P<surfaces_drawn>\d+) last failure '(?P<last_failure>[^'\n]*)'")
NO_CORE_LINE = "r_core_world_stats: no render core"
STATS_SECTION = re.compile(r"r_core_world_stats: (gaps|drawn by the core):$")
# One token each: echo writes each argument separately, and a console line
# without its newline (a truncated stats list) can run into the next write.
MARKER = re.compile(r"CWS_(BEGIN|MAPPED|END)\.(\d+)\.([A-Za-z0-9_]+)")
# Sys_Error prints its text to the console, "Engine error: <text>" to the
# engine log and " ##### Sys_Error: <text>" to stdout; a strict core failure's
# console text is recognised by its own wording.
SYS_ERROR = re.compile(r"(?:Sys_Error:|Engine error:)\s*(.+)|"
                       r"^(r_core_world: the render core failed \d+ claimed view\(s\): .+"
                       r"\(r_core_world_strict 0 reports.*)$")
CORE_SYS_ERROR = re.compile(r"r_core_world: the render core failed \d+ claimed view\(s\): (.+?)"
                            r"(?: \(r_core_world_strict 0 reports|$)")
CORE_WARNING = re.compile(r"r_core_world: the render core failed \d+ claimed view\(s\), "
                          r"surfaces left undrawn: (.+)")
# engine.log stamps each console write with the engine's clock: "[12.3456] ".
LOG_STAMP = re.compile(r"\[(\d+\.\d+)\] ?")
# The end of a level load: the core's level-init summary, else the server's
# activation (the control run has no core world).
LEVEL_LOADED = re.compile(r"^r_core_world: \d+ of \d+ surfaces|^SV_ActivateServer:")
STATS_ENTRY = re.compile(r"^\s*\d+ \S")
STATUS_MAP = re.compile(r"\bmap\s*:\s*([A-Za-z0-9_]+)\s+at:")
ACTIVE_PLAYER = re.compile(r"#\s*\d+\s+\"[^\n]*\"[^\n]*\bactive\b")
# Engine load failures a log names without dying.
LOAD_FAILURE = re.compile(r"(?:Map load failed|map load failed|Couldn't spawn server|"
                          r"Can't find map|Host_Error: .+)[^\n]*")

COUNTERS = ("queued", "drawn", "failed", "skipped", "surfaces_drawn")
PASSING = ("pass", "no-claims")


# ---------------------------------------------------------------------------
# Content


def enumerate_maps(game, content_root):
    """Every top-level maps/*.bsp of the game's own search path, first layer
    wins. Returns (maps, skipped) where maps is a sorted list of
    {"map", "source"} and skipped lists published (non-retail) maps."""
    content_root = Path(content_root)
    found, skipped = {}, []
    for kind, relative in GAME_LAYERS[game]:
        path = content_root / relative
        names = []
        if kind == "vpk" and path.is_file():
            vpk = source_content.VpkDirectory(str(path))
            for entry in vpk.entries:
                parts = entry.split("/")
                if len(parts) == 2 and parts[0] == "maps" and parts[1].endswith(".bsp"):
                    names.append((parts[1][:-4], "%s:%s" % (relative, entry), None))
        elif kind == "dir" and (path / "maps").is_dir():
            for entry in sorted((path / "maps").iterdir()):
                if entry.suffix.lower() != ".bsp" or not entry.is_file():
                    continue
                names.append((entry.stem, "%s/maps/%s" % (relative, entry.name), entry))
        for name, source, file_path in names:
            key = name.lower()
            if key in found:
                continue
            if file_path is not None and PUBLISHED_MAPS.exists() and \
                    PUBLISHED_MAPS.resolve() in file_path.resolve().parents:
                skipped.append({"map": name, "source": source,
                                "reason": "published by the map pipeline (run/maps), not retail"})
                continue
            if not MAP_NAME.fullmatch(name):
                skipped.append({"map": name, "source": source,
                                "reason": "not a simple map name"})
                continue
            found[key] = {"map": name, "source": source}
    return sorted(found.values(), key=lambda item: item["map"].lower()), skipped


# ---------------------------------------------------------------------------
# Scripts


def map_commands(index, name, settle_frames, first=False, views=(), view_frames=0):
    """A map's commands. The process's first map loads from the command line
    (`+map`, which also keeps the menu's background map from loading over a
    scripted `map`), so its script starts after the load. After the settle
    wait, each of `views` (e.g. `setang 0 90 0`) turns the player and waits
    view_frames, so the core draws more of the level than one spawn view."""
    begin = [] if first else ["echo CWS_BEGIN.%d.%s" % (index, name), "map %s" % name]
    look = []
    for view in views:
        look += [view, "wait %d" % view_frames]
    return begin + ["echo CWS_MAPPED.%d.%s" % (index, name), "wait %d" % settle_frames] + \
        look + ["status", "r_core_world_stats", "echo CWS_END.%d.%s" % (index, name)]


def write_scripts(cfg_dir, maps, settle_frames, startup, views=(), view_frames=0):
    """One cfg per map, each a single semicolon-joined line (a cfg's separate
    lines run at once; one line honors `wait`) ending in `exec` of the next.
    The startup cfg ends with the first map's CWS_BEGIN. Returns the first
    map's script name."""
    cfg_dir = Path(cfg_dir)
    cfg_dir.mkdir(parents=True, exist_ok=True)
    for stale in cfg_dir.glob(SCRIPT_PREFIX + "*.cfg"):
        stale.unlink()
    first_index, first_name = maps[0]
    (cfg_dir / (SCRIPT_PREFIX + "start.cfg")).write_text(
        "".join(line + "\n" for line in startup) +
        "echo CWS_BEGIN.%d.%s\n" % (first_index, first_name))
    for position, (index, name) in enumerate(maps):
        following = (SCRIPT_PREFIX + "%d" % maps[position + 1][0]
                     if position + 1 < len(maps) else SCRIPT_PREFIX + "done")
        line = "; ".join(map_commands(index, name, settle_frames, first=position == 0,
                                      views=views, view_frames=view_frames) +
                         ["exec " + following])
        (cfg_dir / ("%s%d.cfg" % (SCRIPT_PREFIX, index))).write_text(line + "\n")
    (cfg_dir / (SCRIPT_PREFIX + "done.cfg")).write_text("echo CWS_DONE; wait 5; quit\n")
    return SCRIPT_PREFIX + "%d" % first_index


# ---------------------------------------------------------------------------
# Log parsing (pure: the self-test drives these with synthetic logs)


def parse_stats(line):
    match = STATS_LINE.search(line)
    if not match:
        return None
    stats = {key: int(value) for key, value in match.groupdict().items() if key != "last_failure"}
    stats["last_failure"] = match.group("last_failure")
    return stats


def parse_process_log(text):
    """Split one process's console output into per-map sections.

    Returns {"maps": {index: section}, "order": [index...], "done": bool,
    "sys_error": text or None, "tail_index": index of a BEGIN without END}."""
    sections, order = {}, []
    current = None
    section_kind = None
    sys_error = None
    done = False
    clock = None
    for raw in text.splitlines():
        # A continuation line (a multi-line write) carries the last stamp.
        stamp = LOG_STAMP.search(raw)
        clock = float(stamp.group(1)) if stamp else clock
        line = LOG_STAMP.sub("", raw.rstrip("\r"))
        marker = MARKER.search(line)
        if marker:
            kind, index, name = marker.group(1), int(marker.group(2)), marker.group(3)
            if kind == "BEGIN":
                current = sections.setdefault(index, new_section(index, name))
                if index not in order:
                    order.append(index)
                section_kind = None
            elif current is not None and current["index"] == index:
                current["mapped" if kind == "MAPPED" else "ended"] = True
            if current is not None and current["index"] == index:
                current["clock"][kind.lower()] = clock
                if kind == "END":
                    current = None
            continue
        if "CWS_DONE" in line:
            done = True
            continue
        error = SYS_ERROR.search(line)
        if error:
            text_error = (error.group(1) or error.group(2)).strip()
            if sys_error is None:
                sys_error = text_error
            if current is not None and current["sys_error"] is None:
                current["sys_error"] = text_error
            continue
        if current is None:
            continue
        if LEVEL_LOADED.match(line) and clock is not None and (
                "loaded" not in current["clock"] or line.startswith("r_core_world:")):
            current["clock"]["loaded"] = clock
        stats = parse_stats(line)
        if stats is not None:
            current["stats"] = stats
            section_kind = None
            continue
        if NO_CORE_LINE in line:
            current["no_core"] = True
            continue
        section = STATS_SECTION.search(line)
        if section:
            section_kind = "gaps" if section.group(1) == "gaps" else "claimed"
            continue
        status = STATUS_MAP.search(line)
        if status:
            current["status_map"] = status.group(1)
            section_kind = None
            continue
        if ACTIVE_PLAYER.search(line):
            current["active_player"] = True
            continue
        warning = CORE_WARNING.search(line)
        if warning:
            current["core_warnings"].append(warning.group(1).strip())
            continue
        failure = LOAD_FAILURE.search(line)
        if failure:
            current["load_errors"].append(failure.group(0).strip())
            continue
        if section_kind and STATS_ENTRY.match(line):
            bucket = current[section_kind]
            if len(bucket) < 40:
                bucket.append(line.strip())
            continue
        section_kind = None
    tail = next((index for index in reversed(order) if not sections[index]["ended"]), None)
    return {"maps": sections, "order": order, "done": done, "sys_error": sys_error,
            "tail_index": tail}


def new_section(index, name):
    return {"index": index, "map": name, "mapped": False, "ended": False, "stats": None,
            "no_core": False, "status_map": None, "active_player": False, "sys_error": None,
            "core_warnings": [], "load_errors": [], "gaps": [], "claimed": [], "clock": {}}


def core_reason(text):
    """The core's own failure text, from a strict-mode Sys_Error, or None."""
    match = CORE_SYS_ERROR.search(text or "")
    return match.group(1).strip() if match else None


def judge(section, previous_counters, died=None, timed_out=False):
    """One map's record and verdict before the control run.

    previous_counters: the cumulative view counters of the last stats line in
    the same process (the core's counters span the process), or None.
    died: the process's exit description when it died on this map."""
    record = {"map": section["map"], "index": section["index"], "mapped": section["mapped"],
              "ended": section["ended"], "status_map": section["status_map"],
              "active_player": section["active_player"], "sys_error": section["sys_error"],
              "core_warnings": section["core_warnings"], "load_errors": section["load_errors"],
              "timed_out": timed_out, "died": died, "stats": section["stats"],
              "gaps": section["gaps"], "claimed": section["claimed"]}
    stats = section["stats"]
    if stats is not None:
        delta = {}
        for key in COUNTERS:
            before = (previous_counters or {}).get(key, 0)
            delta[key] = stats[key] - before if stats[key] >= before else stats[key]
        record["views"] = delta
    else:
        record["views"] = None
    loaded = bool(section["status_map"] and
                  section["status_map"].lower() == section["map"].lower())
    record["loaded"] = loaded
    reason = None
    if section["sys_error"]:
        core = core_reason(section["sys_error"])
        reason = ("core: " + core) if core else ("Sys_Error: " + section["sys_error"])
    elif timed_out:
        reason = "timeout: no progress"
    elif died:
        reason = "process died: " + died
    elif not section["ended"]:
        reason = "incomplete: the map's script did not finish"
    elif not loaded:
        reason = "not loaded: " + ("; ".join(section["load_errors"]) or
                                   "status names another map or none")
    elif section["no_core"]:
        reason = "no render core"
    elif stats is None:
        reason = "no r_core_world_stats line"
    elif record["views"]["failed"] > 0:
        reason = "core: failed %d view(s): %s" % (record["views"]["failed"],
                                                  stats["last_failure"] or "(no reason)")
    elif section["core_warnings"]:
        reason = "core: " + section["core_warnings"][0]
    elif record["views"]["queued"] == 0:
        # The engine queues a view only for a back-buffer view that sees a
        # claimed surface, so claimed surfaces can stay out of every view.
        record["verdict"] = "no-claims"
        record["note"] = ("the core claimed no surface" if stats["claimed_surfaces"] == 0 else
                          "the core claimed %d surface(s), none in a queued view"
                          % stats["claimed_surfaces"])
    elif record["views"]["drawn"] == 0:
        reason = "core: queued %d view(s) but drew none" % record["views"]["queued"]
    else:
        record["verdict"] = "pass"
    if reason is not None:
        record["verdict"] = "fail"
        record["reason"] = reason
    return record


def judge_control(section, died=None, timed_out=False):
    """Whether a map loads with r_core_world 0: loaded, finished, no fatal error."""
    loaded = bool(section and section["ended"] and section["status_map"] and
                  section["status_map"].lower() == section["map"].lower())
    problem = None
    if section is None:
        problem = "control never reached the map"
    elif section["sys_error"]:
        problem = "Sys_Error: " + section["sys_error"]
    elif timed_out:
        problem = "timeout"
    elif died:
        problem = "process died: " + died
    elif not loaded:
        problem = "not loaded: " + ("; ".join(section["load_errors"]) or "status names another map")
    return {"loads": problem is None, "problem": problem,
            "active_player": bool(section and section["active_player"])}


def classify(record, control):
    """Final verdict of a failing map, from its control run."""
    if record["verdict"] in PASSING:
        return record
    record["control"] = control
    record["verdict"] = "core-failure" if control and control["loads"] else "engine-failure"
    return record


def reason_group(reason):
    """A grouping key: the reason without counts and hex addresses."""
    text = re.sub(r"0x[0-9a-fA-F]+", "0x…", reason or "")
    text = re.sub(r"\b\d+\b", "N", text)
    return text[:200]


# ---------------------------------------------------------------------------
# Expectations


def matches(entry, record):
    return (record["verdict"] == entry["verdict"] and
            entry["reason"] in (record.get("reason") or ""))


def expectations(results, known, game, checks, ran_all, intermittent=()):
    """Count one check per map against the workload's known failures;
    unexpected results print first, then the known failures.

    An intermittent entry names a failure reason (and optionally the maps it
    is seen on): a map failing with it is a known failure, and the same map
    passing is no news."""
    known_by_map = {item["map"].lower(): item for item in known if item["game"] == game}
    flaky = [item for item in intermittent if item["game"] == game]
    known_failing = []
    for record in results:
        name = record["map"]
        entry = known_by_map.get(name.lower())
        ok = record["verdict"] in PASSING
        if entry is None:
            entry = next((item for item in flaky if not ok and matches(item, record) and
                          name.lower() in [m.lower() for m in item.get("maps", [name])]), None)
            if entry is not None:
                known_failing.append((record, entry))
                continue
            checks.check(ok, "unexpected.%s.%s" % (game, name),
                         "%s: %s" % (record["verdict"], record.get("reason", "")))
            continue
        if ok:
            checks.check(False, "unexpected.%s.%s" % (game, name),
                         "known failure (%s, owner %s) now %s: remove it from the workload"
                         % (entry["verdict"], entry["owner"], record["verdict"]))
            continue
        same = matches(entry, record)
        checks.check(same, "unexpected.%s.%s" % (game, name),
                     "known %s (%s) but got %s: %s" % (entry["verdict"], entry["reason"],
                                                      record["verdict"], record.get("reason")))
        if same:
            known_failing.append((record, entry))
    if ran_all:
        ran = {record["map"].lower() for record in results}
        for key, entry in sorted(known_by_map.items()):
            checks.check(key in ran, "unexpected.%s.%s" % (game, entry["map"]),
                         "known failure names a map that is not in the content")
    for record, entry in known_failing:
        checks.check(False, "known-failure.%s.%s" % (game, record["map"]),
                     "%s%s (owner %s): %s" % (record["verdict"],
                                              " (intermittent)" if "map" not in entry else "",
                                              entry["owner"], record["reason"]))


# ---------------------------------------------------------------------------
# Processes


def staged_runtime(game, args, root):
    """Stage an isolated runtime with the build's products under root."""
    if game == "portal":
        info = {"staging": portal_boot.stage_runtime(args.runtime, root, game="portal")}
    else:
        stage_portal2_runtime.stage_content(args.steam_root, root)
        info = {"staging": "stage_portal2_runtime.stage_content"}
    info["build_overrides"] = sorted(portal_boot.install_build(args.build, root, game=game))
    if not (root / "hl2_launcher").is_file():
        raise ValueError("staged runtime lacks hl2_launcher")
    return info


def write_fake_zenity(directory):
    """Error() dialogs go through zenity; never let one open or block a run."""
    directory.mkdir(parents=True, exist_ok=True)
    zenity = directory / "zenity"
    zenity.write_text("#!/bin/sh\nif [ \"$1\" = --version ]; then echo 4.0.2; exit 0; fi\n"
                      "echo \"zenity $*\" >&2\nexit 1\n")
    zenity.chmod(0o755)


def describe_exit(code):
    if code is None:
        return None
    if code < 0:
        try:
            return "signal %d (%s)" % (-code, signal.Signals(-code).name)
        except ValueError:
            return "signal %d" % -code
    return "exit %d" % code


def run_process(game, stage, maps, workload, core_world, out_dir, tools, extra_startup,
                width, height):
    """Run one engine process over maps [(index, name)]; return its parse and exit."""
    cfg_dir = stage / game / "cfg"
    startup = ["sv_cheats 1", "r_core_world %d" % (1 if core_world else 0)]
    startup += list(workload["startup_commands"]) + list(extra_startup)
    first = write_scripts(cfg_dir, maps, workload["settle_frames"], startup,
                          workload.get("views", ()), workload.get("view_frames", 0))
    # The launcher's engine.log holds every console line, stamped with the
    # engine's clock; stdout adds the " ##### Sys_Error:" line of a fatal error.
    log = stage / "engine.log"
    log.unlink(missing_ok=True)
    (stage / game / "console.log").unlink(missing_ok=True)
    sandbox = launch_sandbox.Sandbox(out_dir / "sandbox", write_paths=[stage])
    environment = sandbox.environment(os.environ)
    for variable in ("DISPLAY", "WAYLAND_DISPLAY"):
        environment.pop(variable, None)
    environment.update({
        "SteamAppId": STEAM_APP_IDS[game], "SteamGameId": STEAM_APP_IDS[game],
        "LD_LIBRARY_PATH": str(stage / "bin"),
        # SDL3's offscreen driver: real GPU rendering, no window or display.
        "SDL_VIDEODRIVER": "offscreen", "SDL_VIDEO_DRIVER": "offscreen",
        "PATH": str(tools) + os.pathsep + environment.get("PATH", ""),
    })
    shared = portal_boot.user_display_in_use(environment, portal_boot.login_session_displays())
    if shared:
        raise ValueError("refusing a run on the user's display: " + ", ".join(shared))
    command = ["./hl2_launcher", "-game", game, "-windowed", "-w", str(width), "-h", str(height),
               "-multirun", "-novid", "-insecure", "-nomessagebox", *workload["engine_args"],
               "+exec", SCRIPT_PREFIX + "start", "+map", maps[0][1], "+exec", first]
    out_dir.mkdir(parents=True, exist_ok=True)
    started = time.monotonic()
    markers = 0
    timed_out = False
    last_progress = started
    offset, partial = 0, ""
    with (out_dir / "stdout.log").open("wb") as stream:
        process = subprocess.Popen(command, cwd=stage, env=environment, stdout=stream,
                                   stderr=subprocess.STDOUT, start_new_session=True)
        while process.poll() is None:
            time.sleep(0.25)
            now = time.monotonic()
            if log.is_file():
                with log.open("rb") as handle:
                    handle.seek(offset)
                    chunk = handle.read()
                offset += len(chunk)
                lines = (partial + chunk.decode("utf-8", "replace")).split("\n")
                partial = lines.pop()
                for line in lines:
                    if MARKER.search(LOG_STAMP.sub("", line)):
                        markers += 1
                        last_progress = now
            limit = workload["startup_timeout_seconds"] if not markers else \
                workload["map_timeout_seconds"]
            if now - last_progress > limit:
                timed_out = True
                os.killpg(process.pid, signal.SIGTERM)
                try:
                    process.wait(timeout=10)
                except subprocess.TimeoutExpired:
                    os.killpg(process.pid, signal.SIGKILL)
                    process.wait()
                break
        code = process.wait()
    seconds = time.monotonic() - started
    log_text = log.read_text(errors="replace") if log.is_file() else ""
    (out_dir / "engine.log").write_text(log_text)
    stdout_text = (out_dir / "stdout.log").read_text(errors="replace")
    parsed = parse_process_log(log_text)
    if parsed["sys_error"] is None or "Sys_Error" in stdout_text:
        # The stdout line is the whole message, even when the log stops short.
        tail_parse = parse_process_log(stdout_text)
        if tail_parse["sys_error"]:
            parsed["sys_error"] = tail_parse["sys_error"]
            if parsed["tail_index"] is not None:
                parsed["maps"][parsed["tail_index"]]["sys_error"] = tail_parse["sys_error"]
    return {"command": command, "returncode": code, "exit": describe_exit(code),
            "timed_out": timed_out, "seconds": round(seconds, 1), "parsed": parsed,
            "sandbox": sandbox.finish(), "out": str(out_dir)}


def sweep(game, stage, maps, workload, core_world, out_root, tools, extra_startup, width,
          height, label):
    """Run maps [(index, name)] with restarts; return (records by index, processes)."""
    records, processes = {}, []
    pending = list(maps)
    previous_startup_failure = False
    number = 0
    while pending:
        number += 1
        result = run_process(game, stage, pending, workload, core_world,
                             out_root / ("%s-%03d" % (label, number)), tools, extra_startup,
                             width, height)
        parsed = result.pop("parsed")
        processes.append(result)
        counters = None
        progressed = False
        finished = parsed["done"] and result["returncode"] == 0 and not result["timed_out"]
        consumed = set()
        for index in parsed["order"]:
            section = parsed["maps"][index]
            is_tail = index == parsed["tail_index"]
            died = None
            if is_tail and not finished and not result["timed_out"]:
                died = result["exit"] or "exited"
            record = judge(section, counters, died=died,
                           timed_out=is_tail and result["timed_out"])
            # How the same section reads as a control (r_core_world 0) run.
            record["as_control"] = judge_control(section, died=died,
                                                 timed_out=is_tail and result["timed_out"])
            begin = section["clock"].get("begin")
            mapped = section["clock"].get("loaded")
            end = section["clock"].get("end")
            record["load_seconds"] = round(mapped - begin, 2) if begin is not None and \
                mapped is not None else None
            record["map_seconds"] = round(end - begin, 2) if begin is not None and \
                end is not None else None
            record["process"] = len(processes)
            records[index] = record
            consumed.add(index)
            progressed = True
            if section["stats"] is not None:
                counters = {key: section["stats"][key] for key in COUNTERS}
            if is_tail:
                break
        pending = [(index, name) for index, name in pending if index not in consumed]
        if not pending:
            break
        if not progressed:
            # The process failed before its first map: record it once, then stop.
            if previous_startup_failure:
                for index, name in pending:
                    problem = "not run: the engine did not start (%s)" % (
                        parsed["sys_error"] or result["exit"])
                    records[index] = {"map": name, "index": index, "verdict": "fail",
                                      "reason": problem, "process": len(processes),
                                      "as_control": {"loads": False, "problem": problem,
                                                     "active_player": False}}
                break
            previous_startup_failure = True
        else:
            previous_startup_failure = False
    return records, processes


# ---------------------------------------------------------------------------
# Run


def load_workload(path):
    workload = json.loads(Path(path).read_text())
    if workload.get("schema") != WORKLOAD_SCHEMA:
        raise ValueError("workload schema must be " + WORKLOAD_SCHEMA)
    for key in ("settle_frames", "map_timeout_seconds", "startup_timeout_seconds",
                "engine_args", "startup_commands", "exclusions", "known_failures",
                "intermittent_failures"):
        if key not in workload:
            raise ValueError("workload lacks " + key)
    for item in workload["exclusions"]:
        if not item.get("reason") or item.get("game") not in GAME_LAYERS:
            raise ValueError("every exclusion needs a game and a written reason")
    for item in workload["intermittent_failures"]:
        if not all(item.get(key) for key in ("game", "verdict", "reason", "owner")) or \
                "map" in item:
            raise ValueError("every intermittent failure needs game, verdict, reason and owner, "
                             "and names no single map")
    for item in workload["known_failures"]:
        if not all(item.get(key) for key in ("game", "map", "verdict", "reason", "owner")):
            raise ValueError("every known failure needs game, map, verdict, reason and owner")
        if item["verdict"] not in ("core-failure", "engine-failure"):
            raise ValueError("a known failure's verdict is core-failure or engine-failure")
    return workload


def summarize(game, results, excluded):
    counts = {}
    for record in results:
        counts[record["verdict"]] = counts.get(record["verdict"], 0) + 1
    groups = {}
    for record in results:
        if record["verdict"] in PASSING:
            continue
        key = (record["verdict"], reason_group(record.get("reason")))
        groups.setdefault(key, []).append(record["map"])
    lines = ["core-world smoke: %s, %d maps run, %d excluded" % (game, len(results), len(excluded)),
             "  " + ", ".join("%s %d" % (key, counts[key]) for key in sorted(counts))]
    for (verdict, reason), names in sorted(groups.items(), key=lambda item: -len(item[1])):
        lines.append("  %s (%d): %s" % (verdict, len(names), reason))
        lines.append("    " + " ".join(names))
    return counts, [{"verdict": verdict, "reason": reason, "maps": names}
                    for (verdict, reason), names in groups.items()], lines


def control_sequence(failing, records):
    """The control run's maps: each failing map after the map its core-run
    process loaded before it (the same level change), if any. Returns the
    groups, each kept whole in one control process."""
    sequence, seen = [], set()
    for index, name in failing:
        record = records.get(index, {})
        before = [other for other in records.values()
                  if other.get("shard") == record.get("shard") and
                  other.get("process") == record.get("process") and other["index"] < index]
        pair = []
        if before:
            previous = max(before, key=lambda other: other["index"])
            pair.append((previous["index"], previous["map"]))
        pair.append((index, name))
        group = [item for item in pair if item[0] not in seen]
        seen.update(item[0] for item in group)
        if group:
            sequence.append(group)
    return sequence


def split(items, parts):
    parts = max(1, min(parts, len(items)))
    size, extra = divmod(len(items), parts)
    shards, start = [], 0
    for part in range(parts):
        end = start + size + (1 if part < extra else 0)
        shards.append(items[start:end])
        start = end
    return shards


def command_run(args):
    import concurrent.futures

    workload = load_workload(args.workload)
    if args.settle_frames:
        workload["settle_frames"] = args.settle_frames
    game = args.game
    output = args.out.resolve()
    output.mkdir(parents=True, exist_ok=True)
    if (output / "evidence.json").exists():
        print("core_world_smoke: evidence already exists in %s" % output, file=sys.stderr)
        return 2
    content = Path(args.runtime if game == "portal" else args.steam_root)
    maps, skipped = enumerate_maps(game, content)
    exclusions = {item["map"].lower(): item for item in workload["exclusions"]
                  if item["game"] == game}
    selected = [item["map"] for item in maps]
    if args.map:
        wanted = {name.lower() for name in args.map}
        unknown = wanted - {name.lower() for name in selected}
        if unknown:
            print("core_world_smoke: not a retail map of %s: %s" % (game, ", ".join(sorted(unknown))),
                  file=sys.stderr)
            return 2
        selected = [name for name in selected if name.lower() in wanted]
    excluded = [dict(exclusions[name.lower()], map=name) for name in selected
                if name.lower() in exclusions]
    run_maps = [name for name in selected if name.lower() not in exclusions]
    evidence = {
        "schema": EVIDENCE_SCHEMA, "game": game, "status": "fail",
        "started_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
        "source": conformance.source_identity(str(ROOT)),
        "workload": {"path": str(args.workload), "version": workload.get("version"),
                     "settle_frames": workload["settle_frames"]},
        "content_root": str(content.resolve()), "build": str(args.build.resolve()),
        "content_maps": len(maps), "skipped_content": skipped, "excluded": excluded,
        "selected": len(selected), "seed_failure": args.seed_failure,
    }
    checks = conformance_result.Checks()
    print("core_world_smoke: %s: %d retail maps in %s, %d selected, %d excluded"
          % (game, len(maps), content, len(selected), len(excluded)), flush=True)
    checks.check(len(maps) > 0, "content.maps", "no retail map found in " + str(content))
    for name, item in sorted(exclusions.items()):
        checks.check(any(entry["map"].lower() == name for entry in maps),
                     "content.exclusion.%s" % item["map"],
                     "the workload excludes a map the content does not hold")
    if not run_maps:
        evidence["failures"] = ["no map to run"]
        (output / "evidence.json").write_text(json.dumps(evidence, indent=2) + "\n")
        checks.check(False, "run.maps", "no map to run")
        return checks.report()
    tools = output / "tools"
    write_fake_zenity(tools)
    extra = ["r_core_world_seed_failure %s" % args.seed_failure] if args.seed_failure else []
    indexed = list(enumerate(run_maps))
    shards = split(indexed, args.jobs)

    def run_shard(number, shard, core_world, label):
        stage = output / ("%s-%d" % ("runtime" if core_world else "control-runtime", number))
        if stage.exists():
            shutil.rmtree(stage)
        info = staged_runtime(game, args, stage)
        records, processes = sweep(game, stage, shard, workload, core_world, output, tools,
                                   extra, args.width, args.height, "%s%d" % (label, number))
        for record in records.values():
            record["shard"] = number
        if not args.keep_runtime:
            shutil.rmtree(stage, ignore_errors=True)
        return info, records, processes

    records, processes, staging = {}, [], []
    started = time.monotonic()
    try:
        with concurrent.futures.ThreadPoolExecutor(max_workers=len(shards)) as pool:
            futures = [pool.submit(run_shard, number, shard, True, "core")
                       for number, shard in enumerate(shards)]
            for future in futures:
                info, shard_records, shard_processes = future.result()
                staging.append(info)
                records.update(shard_records)
                processes.extend(shard_processes)
        failing = [(index, records[index]["map"]) for index, _ in indexed
                   if records.get(index, {}).get("verdict") not in PASSING]
        control = {}
        if failing and not args.no_control:
            print("core_world_smoke: control run (r_core_world 0) of %d failing map(s)"
                  % len(failing), flush=True)
            control_shards = [[item for group in groups for item in group]
                              for groups in split(control_sequence(failing, records), args.jobs)]
            with concurrent.futures.ThreadPoolExecutor(max_workers=len(control_shards)) as pool:
                futures = [pool.submit(run_shard, number, shard, False, "control")
                           for number, shard in enumerate(control_shards)]
                for future in futures:
                    _, shard_records, shard_processes = future.result()
                    processes.extend(shard_processes)
                    control.update(shard_records)
    except (OSError, ValueError, subprocess.SubprocessError) as error:
        evidence["failures"] = [str(error)]
        (output / "evidence.json").write_text(json.dumps(evidence, indent=2) + "\n")
        checks.check(False, "run.harness", str(error))
        return checks.report()
    results = []
    for index, name in indexed:
        record = records.get(index) or {"map": name, "index": index, "verdict": "fail",
                                        "reason": "not run"}
        if record["verdict"] not in PASSING:
            if args.no_control:
                record["control"] = None
            else:
                controlled = control.get(index)
                record = classify(record, controlled and controlled.get(
                    "as_control", {"loads": False, "problem": controlled.get("reason")}))
        results.append(record)
    counts, groups, lines = summarize(game, results, excluded)
    evidence.update(results=results, counts=counts, failure_groups=groups, processes=processes,
                    staging=staging, elapsed_seconds=round(time.monotonic() - started, 1))
    expectations(results, workload["known_failures"], game, checks,
                 intermittent=workload["intermittent_failures"],
                 ran_all=not args.map)
    evidence["checks"] = {"checks": checks.checks, "failures": checks.failures}
    evidence["status"] = "pass" if checks.failures == 0 else "fail"
    (output / "evidence.json").write_text(json.dumps(evidence, indent=2) + "\n")
    (output / "summary.txt").write_text("\n".join(lines) + "\n")
    for line in lines:
        print(line)
    print("core_world_smoke: evidence %s" % (output / "evidence.json"), flush=True)
    return checks.report()


def command_list(args):
    content = Path(args.runtime if args.game == "portal" else args.steam_root)
    maps, skipped = enumerate_maps(args.game, content)
    workload = load_workload(args.workload)
    excluded = {item["map"].lower(): item["reason"] for item in workload["exclusions"]
                if item["game"] == args.game}
    for item in maps:
        note = excluded.get(item["map"].lower())
        print("%-40s %s%s" % (item["map"], item["source"], ("  EXCLUDED: " + note) if note else ""))
    for item in skipped:
        print("%-40s %s  SKIPPED: %s" % (item["map"], item["source"], item["reason"]))
    print("%d retail maps, %d excluded" % (len(maps), sum(item["map"].lower() in excluded
                                                          for item in maps)))
    return 0


# ---------------------------------------------------------------------------
# Self-test: the parser, verdicts and expectations on synthetic logs


def synthetic_stats(queued, drawn, failed=0, skipped=0, claimed=12, last=""):
    return ("r_core_world_stats: materials 40 claimed 3 surfaces 900 claimed %d views queued %d "
            "drawn %d failed %d skipped %d surfaces drawn %d last failure '%s'"
            % (claimed, queued, drawn, failed, skipped, drawn * 10, last))


def synthetic_map(index, name, stats, status=True, end=True, extra=()):
    lines = ["CWS_BEGIN.%d.%s" % (index, name), "Loading map \"%s\"" % name,
             "CWS_MAPPED.%d.%s" % (index, name)]
    lines += list(extra)
    if status:
        lines += ["hostname: Portal", "map     : %s at: 0 x, 0 y, 64 z" % name,
                  "#  2 \"player\" STEAM_1:0:1 00:10 5 0 active"]
    if stats is not None:
        lines += [stats, "r_core_world_stats: gaps:", "\t12 LightmappedGeneric $envmap",
                  "r_core_world_stats: drawn by the core:", "\t40 metal/black_wall_metal_002c"]
    if end:
        lines.append("CWS_END.%d.%s" % (index, name))
    return lines


def self_test_cases():
    """(name, log text, exit description, timed out, expected verdicts by map)."""
    core_error = ("Sys_Error: r_core_world: the render core failed 2 claimed view(s): a view named "
                  "surfaces the pass does not draw (r_core_world_strict 0 reports failures "
                  "instead; legacy never draws what the core claimed)")
    cases = [
        ("clean", synthetic_map(0, "a", synthetic_stats(10, 10)) +
         synthetic_map(1, "b", synthetic_stats(25, 25)) + ["CWS_DONE"], None, False,
         {"a": "pass", "b": "pass"}),
        # The core claims nothing on b: no views queued in its section.
        ("no-claims", synthetic_map(0, "a", synthetic_stats(10, 10)) +
         synthetic_map(1, "b", synthetic_stats(10, 10, claimed=0)) + ["CWS_DONE"], None, False,
         {"a": "pass", "b": "no-claims"}),
        # A strict-mode Sys_Error on the second of three maps: the first
        # passes, the second fails with the core's reason, the third never ran.
        ("sys-error-mid-list", synthetic_map(0, "a", synthetic_stats(10, 10)) +
         synthetic_map(1, "b", None, status=True, end=False, extra=[core_error]),
         "signal 5 (SIGTRAP)", False, {"a": "pass", "b": "fail"}),
        # The stats line is missing: never a pass.
        ("missing-stats", synthetic_map(0, "a", None) + ["CWS_DONE"], None, False, {"a": "fail"}),
        # failed > 0 without strict mode.
        ("failed-views", synthetic_map(0, "a", synthetic_stats(10, 8, failed=2,
                                                                last="a view failed")) +
         ["CWS_DONE"], None, False, {"a": "fail"}),
        # The counters span the process: b's own views drew nothing.
        ("cumulative-counters", synthetic_map(0, "a", synthetic_stats(10, 10)) +
         synthetic_map(1, "b", synthetic_stats(14, 10)) + ["CWS_DONE"], None, False,
         {"a": "pass", "b": "fail"}),
        # The map never loaded: status names no map.
        ("not-loaded", synthetic_map(0, "a", synthetic_stats(0, 0, claimed=0), status=False,
                                     extra=["Map load failed: a not found or invalid"]) +
         ["CWS_DONE"], None, False, {"a": "fail"}),
        # A crash (no Sys_Error) mid-map.
        ("crash", synthetic_map(0, "a", synthetic_stats(4, 4)) +
         synthetic_map(1, "b", None, status=False, end=False), "signal 11 (SIGSEGV)", False,
         {"a": "pass", "b": "fail"}),
        # A hang: the watchdog killed the process on b.
        ("timeout", synthetic_map(0, "a", None, status=False, end=False), None, True,
         {"a": "fail"}),
        # Claimed surfaces outside every queued view: no-claims too.
        ("claimed-unqueued", synthetic_map(0, "a", synthetic_stats(0, 0, claimed=5)) +
         ["CWS_DONE"], None, False, {"a": "no-claims"}),
    ]
    return cases


def run_self_test_case(log_lines, died, timed_out, seed_fault=None):
    parsed = parse_process_log("\n".join(log_lines) + "\n")
    records = {}
    counters = None
    for index in parsed["order"]:
        section = parsed["maps"][index]
        is_tail = index == parsed["tail_index"]
        if seed_fault == "ignore-sys-error":
            section = dict(section, sys_error=None)
        record = judge(section, None if seed_fault == "absolute-counters" else counters,
                       died=died if is_tail and not timed_out else None,
                       timed_out=is_tail and timed_out)
        if seed_fault == "ignore-failed" and record["verdict"] == "fail" and \
                record.get("reason", "").startswith("core: failed"):
            record["verdict"] = "pass"
        if seed_fault == "ignore-sys-error" and record["verdict"] == "fail" and \
                record.get("reason", "").startswith(("incomplete", "process died")):
            record["verdict"] = "pass"
        records[section["map"]] = record
        if section["stats"] is not None:
            counters = {key: section["stats"][key] for key in COUNTERS}
        if is_tail:
            break
    return parsed, records


def command_self_test(args):
    checks = conformance_result.Checks()
    fault = args.seed_fault
    for name, lines, died, timed_out, expected in self_test_cases():
        parsed, records = run_self_test_case(lines, died, timed_out, fault)
        got = {key: value["verdict"] for key, value in records.items()}
        checks.equal(got, expected, "self_test.%s" % name)
        if name == "sys-error-mid-list":
            record = records.get("b", {})
            checks.check(record.get("reason", "").startswith(
                "core: a view named surfaces the pass does not draw") or fault is not None,
                "self_test.sys-error-reason", "reason %r" % record.get("reason"))
            checks.equal(parsed["tail_index"], 1, "self_test.sys-error-tail")
        if name == "clean":
            record = records.get("a", {})
            checks.equal(record.get("gaps"), ["12 LightmappedGeneric $envmap"],
                         "self_test.gaps-parsed")
            checks.equal(record.get("claimed"), ["40 metal/black_wall_metal_002c"],
                         "self_test.claimed-parsed")
            checks.equal(record.get("views"), {"queued": 10, "drawn": 10, "failed": 0,
                                               "skipped": 0, "surfaces_drawn": 100},
                         "self_test.views-parsed")
    # Classification: control loads -> core-failure; control fails -> engine.
    failing = {"verdict": "fail", "reason": "core: x"}
    checks.equal(classify(dict(failing), {"loads": True})["verdict"], "core-failure",
                 "self_test.classify-core")
    checks.equal(classify(dict(failing), {"loads": False})["verdict"], "engine-failure",
                 "self_test.classify-engine")
    checks.equal(classify(dict(failing), None)["verdict"], "engine-failure",
                 "self_test.classify-no-control")
    # Expectations: a known failure prints after the unexpected results, and a
    # known failure that now passes is unexpected.
    import io
    stream = io.StringIO()
    inner = conformance_result.Checks(stream)
    known = [{"game": "portal", "map": "b", "verdict": "core-failure", "reason": "core: x",
              "owner": "R89"}, {"game": "portal", "map": "c", "verdict": "core-failure",
                                "reason": "core: y", "owner": "R89"}]
    results = [{"map": "a", "verdict": "core-failure", "reason": "core: new"},
               {"map": "b", "verdict": "core-failure", "reason": "core: x"},
               {"map": "c", "verdict": "pass"}]
    expectations(results, known, "portal", inner, ran_all=True)
    lines = [line for line in stream.getvalue().splitlines() if line.startswith("FAIL")]
    checks.equal([line.split(":")[0] for line in lines],
                 ["FAIL unexpected.portal.a", "FAIL unexpected.portal.c",
                  "FAIL known-failure.portal.b"], "self_test.expectation-order")
    # An intermittent reason: a failing map with it is known, a passing map
    # is no news, a failing map with another reason is unexpected.
    stream = io.StringIO()
    inner = conformance_result.Checks(stream)
    flaky = [{"game": "portal", "verdict": "core-failure", "reason": "core: flake",
              "owner": "R89"}]
    expectations([{"map": "d", "verdict": "pass"},
                  {"map": "e", "verdict": "core-failure", "reason": "core: flake here"},
                  {"map": "f", "verdict": "core-failure", "reason": "core: other"}],
                 [], "portal", inner, ran_all=True, intermittent=flaky)
    lines = [line for line in stream.getvalue().splitlines() if line.startswith("FAIL")]
    checks.equal([line.split(":")[0] for line in lines],
                 ["FAIL unexpected.portal.f", "FAIL known-failure.portal.e"],
                 "self_test.expectation-intermittent")
    stream = io.StringIO()
    inner = conformance_result.Checks(stream)
    expectations([{"map": "b", "verdict": "core-failure", "reason": "core: x"}], known[:1],
                 "portal", inner, ran_all=True)
    checks.check(stream.getvalue().startswith("FAIL known-failure.portal.b"),
                 "self_test.expectation-known-only", stream.getvalue().strip())
    return checks.report()


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    commands = parser.add_subparsers(dest="command", required=True)
    steam = Path(os.environ.get("SOURCE_PORTAL2_STEAM_ROOT") or
                 os.environ.get("P2_STEAM_ROOT") or DEFAULT_P2_STEAM_ROOT)
    for name in ("run", "list"):
        sub = commands.add_parser(name)
        sub.add_argument("--game", choices=sorted(GAME_LAYERS), required=True)
        sub.add_argument("--workload", type=Path, default=DEFAULT_WORKLOAD)
        sub.add_argument("--runtime", type=Path, default=ROOT / "run/runtime",
                         help="Portal runtime holding the retail content (portal/)")
        sub.add_argument("--steam-root", type=Path, default=steam,
                         help="Portal 2 installation (SOURCE_PORTAL2_STEAM_ROOT)")
        if name == "run":
            sub.add_argument("--build", type=Path,
                             help="Waf output tree (default build for portal, build-p2 for portal2)")
            sub.add_argument("--out", type=Path, required=True)
            sub.add_argument("--map", action="append", default=[],
                             help="run only this map (repeatable)")
            sub.add_argument("--jobs", type=int, default=1,
                             help="engine processes in parallel, each on its own runtime")
            sub.add_argument("--seed-failure", metavar="MATERIAL",
                             help="negative control: r_core_world_seed_failure MATERIAL")
            sub.add_argument("--no-control", action="store_true",
                             help="skip the r_core_world 0 control run of failing maps")
            sub.add_argument("--keep-runtime", action="store_true")
            sub.add_argument("--settle-frames", type=int,
                             help="override the workload's frames between a load and the stats")
            sub.add_argument("--width", type=int, default=1024)
            sub.add_argument("--height", type=int, default=768)
    selftest = commands.add_parser("self-test")
    selftest.add_argument("--seed-fault", choices=("ignore-failed", "ignore-sys-error",
                                                   "absolute-counters"))
    args = parser.parse_args(argv)
    if args.command == "self-test":
        return command_self_test(args)
    if args.command == "list":
        return command_list(args)
    if args.build is None:
        args.build = ROOT / ("build" if args.game == "portal" else "build-p2")
    if args.jobs < 1:
        parser.error("--jobs must be positive")
    if args.seed_failure and not re.fullmatch(r"[A-Za-z0-9_/.\-]+", args.seed_failure):
        parser.error("--seed-failure must be a material name")
    return command_run(args)


if __name__ == "__main__":
    sys.exit(main())
