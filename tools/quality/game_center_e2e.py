#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Game Center achievements end to end on a connected iOS or tvOS device.

Checks the installed product app (ios-deploy.sh) against what GameKit
promises, with Game Center itself as the oracle:

  1. audit: the app launches with SOURCE_GAME_CENTER_AUDIT set, and its
     platform bridge (platform/apple, AuditGameCenter) logs what Game
     Center holds without starting the engine. A player is signed in
     (GKLocalPlayer.isAuthenticated), Game Center recognizes the app
     (achievement descriptions load; GKErrorDomain 15 means it is not
     registered in App Store Connect), and every achievement the game
     declares (DECLARE_*ACHIEVEMENT in its source) has a description with
     the same identifier.
  2. play: a scripted run loads a map and fires a logic_achievement for the
     tested achievement, the game's own award path. Game Center accepts the
     report (the bridge's "game-center: submit ... result=accepted"), whether
     it was made at the award or, for an achievement the player already had,
     at sign-in. Then the achievements menu command (gamemenucommand
     OpenAchievementsDialog) asks Game Center for its screen, and
     GKAccessPoint reports it presenting.
  3. audit again: Game Center holds the achievement at 100% and completed.

Results are checks-v1 ("CONFORMANCE <checks> <failures>"), and each failure
names its cause (no signed-in player, app not registered, an identifier
missing from App Store Connect, a refused report).

    python3 tools/quality/game_center_e2e.py --out quality-results/game-center-e2e

The run changes Game Center state for the signed-in player: the tested
achievement becomes complete (Game Center never un-completes one; use a
sandbox account).
"""

import argparse
import datetime
import json
from pathlib import Path
import re
import sys

import conformance
import ios_device
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "kiln"))
from sepipe_loader import resolve_profile as load_profile  # noqa: E402

REPO = Path(__file__).resolve().parents[2]
DEFAULT_PROFILE = REPO / "quality/product_profiles/portal-tvos-native-vulkan.json"
# The game source that declares each game's achievements (their names are the
# Game Center identifiers; public/game/game_platform_services.h).
ACHIEVEMENT_SOURCES = {"portal": REPO / "game/shared/portal/achievements_portal.cpp"}
DECLARATION = re.compile(r'DECLARE_(?:MAP_EVENT_)?ACHIEVEMENT\w*\s*\(([^;]*?)\)\s*;', re.S)
DEVICE_PLATFORMS = {"ios": "iOS", "tvos": "tvOS"}
CFG_DIRECTORY = "portal/custom/game_center_e2e/cfg"
PLAY_CFG = "game_center_e2e.cfg"
LINE = re.compile(r"Source: (game-center(?:-audit)?): (\S+)(.*)$")
KEY_VALUE = re.compile(r'(\w+)=("[^"]*"|\S+)')
# GameKit error codes the checks name (GKErrorCode).
NOT_AUTHENTICATED = 6
GAME_UNRECOGNIZED = 15


def declared_achievements(game):
    source = ACHIEVEMENT_SOURCES[game].read_text(errors="replace")
    names = []
    for arguments in DECLARATION.findall(source):
        quoted = re.findall(r'"([^"]+)"', arguments)
        if quoted:
            names.append(quoted[0])
    return names


def parse_events(console):
    """The bridge's structured lines: [(channel, event, {key: value})]."""
    events = []
    for line in console.splitlines():
        match = LINE.search(line)
        if not match:
            continue
        fields = {key: value.strip('"') for key, value in KEY_VALUE.findall(match.group(3))}
        if "=" in match.group(2):
            key, value = match.group(2).split("=", 1)
            fields[key] = value
            event = key
        else:
            event = match.group(2)
        events.append((match.group(1), event, fields))
    return events


def error_code(fields):
    """GameKit's error code from error=<domain>:<code>, or None."""
    error = fields.get("error", "none")
    if error == "none" or ":" not in error:
        return None
    return int(error.rsplit(":", 1)[1])


class Checks:
    def __init__(self):
        self.results = []

    def check(self, name, ok, detail=""):
        self.results.append({"check": name, "ok": bool(ok), "detail": detail})
        print("  [%s] %s%s" % ("ok  " if ok else "FAIL", name, (": " + detail) if detail else ""),
              flush=True)
        return ok

    @property
    def failures(self):
        return sum(1 for r in self.results if not r["ok"])


def audit(device, executable, timeout):
    run = device.launch(executable, timeout=timeout,
                        environment={"SOURCE_GAME_CENTER_AUDIT": "1"})
    events = [e for e in parse_events(run["console"]) if e[0] == "game-center-audit"]
    report = {"run": run, "authenticated": None, "descriptions": [], "descriptions_error": None,
              "achievements": {}, "achievements_error": None, "done": False}
    for _, event, fields in events:
        if event == "authenticated":
            report["authenticated"] = fields.get("authenticated") == "1"
        elif event == "description":
            report["descriptions"].append(fields.get("id"))
        elif event == "descriptions":
            report["descriptions_error"] = error_code(fields)
            report["descriptions_error_text"] = fields.get("error")
        elif event == "achievement":
            report["achievements"][fields.get("id")] = {
                "percent": float(fields.get("percent", 0)),
                "completed": fields.get("completed") == "1"}
        elif event == "achievements":
            report["achievements_error"] = error_code(fields)
            report["achievements_error_text"] = fields.get("error")
        elif event == "done":
            report["done"] = True
    return report


def check_audit(checks, label, report, expected):
    checks.check("%s: the audit ran to completion" % label, report["done"],
                 "" if report["done"] else "no 'game-center-audit: done' line; exit %s" %
                 report["run"]["exit_code"])
    if not checks.check("%s: a Game Center player is signed in on the device" % label,
                        report["authenticated"],
                        "" if report["authenticated"] else
                        "sign in under Settings > Users and Accounts > Game Center"):
        return False
    recognized = report["descriptions_error"] is None
    detail = ""
    if report["descriptions_error"] == GAME_UNRECOGNIZED:
        detail = ("Game Center does not recognize the app: register its bundle id in App Store "
                  "Connect with Game Center on and its achievements defined (%s)" %
                  report.get("descriptions_error_text"))
    elif not recognized:
        detail = "descriptions failed: %s" % report.get("descriptions_error_text")
    if not checks.check("%s: Game Center recognizes the app" % label, recognized, detail):
        return False
    missing = [name for name in expected if name not in report["descriptions"]]
    checks.check("%s: every declared achievement is defined in Game Center" % label, not missing,
                 "missing from App Store Connect: %s" % ", ".join(missing) if missing else
                 "%d of %d" % (len(expected), len(expected)))
    checks.check("%s: the player's achievements load" % label,
                 report["achievements_error"] is None, report.get("achievements_error_text") or "")
    return True


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--profile", type=Path, default=DEFAULT_PROFILE)
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--achievement", default="PORTAL_GET_PORTALGUNS",
                        help="a map-event achievement the play run fires")
    parser.add_argument("--map", default="testchmb_a_00")
    parser.add_argument("--timeout", type=float, default=300)
    parser.add_argument("--host", default=ios_device.DEFAULT_HOST)
    parser.add_argument("--device", help="devicectl identifier (default: the connected one)")
    args = parser.parse_args(argv)

    output = args.out.resolve()
    output.mkdir(parents=True, exist_ok=True)
    profile = load_profile(args.profile)
    platform = profile["target"]["os"]
    app = profile[platform]
    content = profile["content"]["container_directory"]
    game = profile.get("game", "portal")
    expected = declared_achievements(game)
    device = ios_device.Device(app["bundle_id"], host=args.host, identifier=args.device,
                               platform=DEVICE_PLATFORMS[platform])
    checks = Checks()
    evidence = {"schema": "game-center-e2e/v1",
                "started_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
                "source": conformance.source_identity(conformance.repo_root()),
                "profile": str(args.profile), "device": device.describe(),
                "declared": expected, "achievement": args.achievement}
    checks.check("the game declares achievements, including the tested one",
                 expected and args.achievement in expected,
                 "%d declared in %s" % (len(expected), ACHIEVEMENT_SOURCES[game].name))

    print("audit before play:", flush=True)
    before = audit(device, app["executable"], args.timeout)
    (output / "audit-before.log").write_text(before["run"]["console"])
    ready = check_audit(checks, "before", before, expected)

    if ready:
        print("play:", flush=True)
        event = "ACHIEVEMENT_EVENT_" + args.achievement
        cfg = "".join(line + "\n" for line in [
            "achievement_debug 1",
            "ent_create logic_achievement AchievementEvent %s targetname gc_e2e_probe" % event,
            "ent_fire gc_e2e_probe FireEvent",
            "wait 180",
            "gamemenucommand OpenAchievementsDialog",
            "wait 300",
            "quit"])
        device.put_text(cfg, "%s/%s/%s" % (content, CFG_DIRECTORY, PLAY_CFG))
        engine_args = ["-novid", "-console", "+sv_cheats", "1", "+map", args.map,
                       "+wait", "120", "+exec", PLAY_CFG]
        device.put_text(" ".join(engine_args) + "\n", "%s/commandline.txt" % content)
        try:
            play = device.launch(app["executable"], timeout=args.timeout)
        finally:
            device.put_text("", "%s/commandline.txt" % content)
        (output / "play.log").write_text(play["console"])
        events = parse_events(play["console"])
        awarded = ("Achievement awarded: %s" % args.achievement) in play["console"]
        submits = [f for c, e, f in events if e == "submit"
                   and args.achievement in f.get("ids", "").split(",")]
        accepted = [f for f in submits if f.get("result") == "accepted"]
        checks.check("play: the run exited cleanly", play["exit_code"] == 0 and
                     not play["timed_out"], "exit %s, signal %s, timed out %s" % (
                         play["exit_code"], play["signal"], play["timed_out"]))
        checks.check("play: the game reported the achievement to Game Center",
                     submits, "awarded this run" if awarded else
                     "already earned; reported at sign-in" if submits else
                     "no 'game-center: submit' line names %s" % args.achievement)
        refused = submits[-1].get("error") if submits and not accepted else ""
        detail = refused
        if submits and not accepted and error_code(submits[-1]) == GAME_UNRECOGNIZED:
            detail = "Game Center does not recognize the app (App Store Connect): " + refused
        checks.check("play: Game Center accepted the report", accepted, detail)
        presents = [f for c, e, f in events if e == "present"]
        presenting = [f for c, e, f in events if e == "presenting"]
        checks.check("play: the achievements menu asked Game Center for its screen",
                     any(f.get("result") == "requested" for f in presents),
                     "results: %s" % [f.get("result") for f in presents])
        checks.check("play: Game Center presented its achievements screen",
                     any(f.get("presenting") == "1" for f in presenting),
                     "GKAccessPoint.isPresentingGameCenter: %s" %
                     [f.get("presenting") for f in presenting])

        print("audit after play:", flush=True)
        after = audit(device, app["executable"], args.timeout)
        (output / "audit-after.log").write_text(after["run"]["console"])
        if check_audit(checks, "after", after, expected):
            held = after["achievements"].get(args.achievement)
            checks.check("after: Game Center holds %s complete at 100%%" % args.achievement,
                         held and held["completed"] and held["percent"] >= 100.0,
                         json.dumps(held) if held else "not among the player's achievements")
        evidence["after"] = {k: v for k, v in after.items() if k != "run"}
    evidence["before"] = {k: v for k, v in before.items() if k != "run"}
    evidence["checks"] = checks.results
    evidence["status"] = "pass" if checks.failures == 0 else "fail"
    (output / "evidence.json").write_text(json.dumps(evidence, indent=2) + "\n")
    print("game center e2e: %s (%d checks, %d failed) -> %s" % (
        evidence["status"], len(checks.results), checks.failures, output / "evidence.json"))
    print("CONFORMANCE %d %d" % (len(checks.results), checks.failures))
    return 0 if checks.failures == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
