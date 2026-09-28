#!/usr/bin/env python3
"""Judge the Portal 2 client's per-frame view trace across portal crossings.

The client prints, with `cl_portal_view_trace 1`
(game/client/portal2/portal/c_portal_player.cpp):

    PVIEW f=<frame> t=<curtime> rt=<realtime> eye (x y z) ang (p y r) body (x y z) through <portal>
    PVIEW_PORTAL f=<frame> portal <index> linked <index> active <0|1> origin (x y z) angles (p y r)

A walk through a portal pair is seamless when every frame's eye follows from
the previous one, either directly or through one of the linked portals (the
portal's matrix to its partner, as CPortal_Base2D_Shared builds it: the
partner's frame, a half turn about the portal's up axis, then the inverse of
this portal's frame). `judge()` finds the worst frame-to-frame step of eye
position and view direction after that undoing, counts the crossings, and
fails a window whose view jumps, turns too fast or never crosses.

    portal_view_trace.py <console.log> <window> [--max-speed 600] [--max-turn-rate 720]

prints the verdict and each step that broke a limit; the scenario runner
(tools/quality/portal2_scenarios.py) calls judge() for `view_continuity`
console checks.
"""

import argparse
import math
import re
import sys

NUMBER = r"(-?[0-9.]+(?:e[-+]?\d+)?|-?nan|-?inf)"
VEC = r"\(%s %s %s\)" % (NUMBER, NUMBER, NUMBER)
VIEW_LINE = re.compile(r"^PVIEW f=(\d+) t=%s rt=%s eye %s ang %s body %s through (-?\d+)\s*$" % (
    NUMBER, NUMBER, VEC, VEC, VEC))
PORTAL_LINE = re.compile(r"^PVIEW_PORTAL f=(\d+) portal (\d+) linked (-?\d+) active ([01]) "
                         r"origin %s angles %s\s*$" % (VEC, VEC))

DEFAULTS = {"max_speed": 600.0, "max_turn_rate": 720.0, "slack": 1.0, "turn_slack": 1.0,
            "min_crossings": 1, "max_crossings": 1000}


def angle_axes(angles):
    """Forward, right, up for Source angles (pitch, yaw, roll), as AngleVectors."""
    pitch, yaw, roll = (math.radians(a) for a in angles)
    sp, cp = math.sin(pitch), math.cos(pitch)
    sy, cy = math.sin(yaw), math.cos(yaw)
    sr, cr = math.sin(roll), math.cos(roll)
    forward = (cp * cy, cp * sy, -sp)
    right = (-sr * sp * cy + cr * sy, -sr * sp * sy - cr * cy, -sr * cp)
    up = (cr * sp * cy + sr * sy, cr * sp * sy - sr * cy, cr * cp)
    return forward, right, up


def frame_matrix(origin, angles):
    """3x4 local-to-world: columns forward, left, up (AngleMatrix), then origin."""
    forward, right, up = angle_axes(angles)
    left = tuple(-c for c in right)
    return [[forward[i], left[i], up[i], origin[i]] for i in range(3)]


def invert(m):
    """Inverse of a rigid 3x4 matrix."""
    r = [[m[j][i] for j in range(3)] for i in range(3)]
    t = [-sum(r[i][k] * m[k][3] for k in range(3)) for i in range(3)]
    return [r[i] + [t[i]] for i in range(3)]


def multiply(a, b):
    return [[sum(a[i][k] * b[k][j] for k in range(3)) + (a[i][3] if j == 3 else 0.0)
             for j in range(4)] for i in range(3)]


def portal_transform(this, linked):
    """CPortal_Base2D_Shared's MatrixThisToLinked for two (origin, angles) poses."""
    half_turn = [[-1.0, 0.0, 0.0, 0.0], [0.0, -1.0, 0.0, 0.0], [0.0, 0.0, 1.0, 0.0]]
    return multiply(multiply(frame_matrix(*linked), half_turn), invert(frame_matrix(*this)))


def apply_point(m, p):
    return tuple(sum(m[i][k] * p[k] for k in range(3)) + m[i][3] for i in range(3))


def apply_vector(m, v):
    return tuple(sum(m[i][k] * v[k] for k in range(3)) for i in range(3))


def distance(a, b):
    return math.sqrt(sum((x - y) ** 2 for x, y in zip(a, b)))


def turn_degrees(a, b):
    dot = max(-1.0, min(1.0, sum(x * y for x, y in zip(a, b))))
    return math.degrees(math.acos(dot))


def parse(lines):
    """PVIEW frames (the last line per frame wins) and PVIEW_PORTAL events, in order."""
    frames, events = [], []
    for line in lines:
        line = line.strip()
        match = VIEW_LINE.match(line)
        if match:
            g = match.groups()
            frame = {"frame": int(g[0]), "t": float(g[1]), "rt": float(g[2]),
                     "eye": tuple(float(x) for x in g[3:6]), "angles": tuple(float(x) for x in g[6:9]),
                     "body": tuple(float(x) for x in g[9:12]), "through": int(g[12]),
                     "index": len(events)}
            if frames and frames[-1]["frame"] == frame["frame"]:
                frames[-1] = frame
            else:
                frames.append(frame)
            continue
        match = PORTAL_LINE.match(line)
        if match:
            g = match.groups()
            events.append({"frame": int(g[0]), "portal": int(g[1]), "linked": int(g[2]),
                           "active": g[3] == "1", "origin": tuple(float(x) for x in g[4:7]),
                           "angles": tuple(float(x) for x in g[7:10])})
    return frames, events


def judge(before, window, limits=None):
    """Judge the frames in `window` (lines), with portal state from `before`
    (every line up to the window's start) and the window's own events.
    Returns (ok, detail, report) where report lists each bad step."""
    limits = dict(DEFAULTS, **(limits or {}))
    _, prior_events = parse(before)
    frames, events = parse(window)
    portals = {}
    for event in prior_events:
        portals[event["portal"]] = event
    if len(frames) < 2:
        return False, "only %d traced frames in the window" % len(frames), []

    def transforms():
        result = []
        for index, portal in portals.items():
            linked = portals.get(portal["linked"])
            if portal["active"] and linked and linked["active"] and linked["linked"] == index:
                result.append((index, portal_transform((portal["origin"], portal["angles"]),
                                                       (linked["origin"], linked["angles"]))))
        return result

    applied = 0
    bad, crossings = [], []
    worst_step, worst_turn = (0.0, None), (0.0, None)
    for previous, current in zip(frames, frames[1:]):
        while applied < current["index"]:
            event = events[applied]
            portals[event["portal"]] = event
            applied += 1
        dt = max(current["rt"] - previous["rt"], current["t"] - previous["t"], 0.0)
        step_limit = limits["max_speed"] * dt + limits["slack"]
        turn_limit = limits["max_turn_rate"] * dt + limits["turn_slack"]
        forward0 = angle_axes(previous["angles"])[0]
        forward1 = angle_axes(current["angles"])[0]
        candidates = [(distance(current["eye"], previous["eye"]), turn_degrees(forward1, forward0), None)]
        for index, matrix in transforms():
            candidates.append((distance(current["eye"], apply_point(matrix, previous["eye"])),
                               turn_degrees(forward1, apply_vector(matrix, forward0)), index))
        # The explanation that moves the eye least, then turns it least.
        step, turn, through = min(candidates, key=lambda c: (c[0] > step_limit, c[0] + c[1]))
        if through is not None:
            crossings.append({"frame": current["frame"], "portal": through,
                              "from": previous["eye"], "to": current["eye"]})
        record = {"frame": current["frame"], "dt": round(dt, 5), "step": round(step, 3),
                  "step_limit": round(step_limit, 3), "turn": round(turn, 3),
                  "turn_limit": round(turn_limit, 3), "through": through,
                  "from": previous["eye"], "to": current["eye"],
                  "from_angles": previous["angles"], "to_angles": current["angles"]}
        if step > worst_step[0]:
            worst_step = (step, record)
        if turn > worst_turn[0]:
            worst_turn = (turn, record)
        if step > step_limit or turn > turn_limit:
            bad.append(record)
    ok = not bad and limits["min_crossings"] <= len(crossings) <= limits["max_crossings"]
    detail = "%d frames, %d crossings, %d bad steps; worst step %.2f, worst turn %.2f deg" % (
        len(frames), len(crossings), len(bad), worst_step[0], worst_turn[0])
    if bad:
        first = bad[0]
        detail += "; first bad frame %d: eye %s -> %s (step %.2f > %.2f or turn %.2f > %.2f)" % (
            first["frame"], fmt(first["from"]), fmt(first["to"]), first["step"],
            first["step_limit"], first["turn"], first["turn_limit"])
    return ok, detail, bad


def fmt(v):
    return "(" + " ".join("%.1f" % x for x in v) + ")"


def window_lines(lines, label):
    """(lines before the window, the window's lines) for QA_WINDOW <label>."""
    begin = end = None
    for index, line in enumerate(lines):
        if line.strip() == "QA_WINDOW %s BEGIN" % label:
            begin = index
        elif line.strip() == "QA_WINDOW %s END" % label and begin is not None:
            end = index
            break
    if begin is None or end is None:
        raise ValueError("window %s was not bracketed" % label)
    return lines[:begin], lines[begin + 1:end]


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("log")
    parser.add_argument("window")
    parser.add_argument("--max-speed", type=float, default=DEFAULTS["max_speed"])
    parser.add_argument("--max-turn-rate", type=float, default=DEFAULTS["max_turn_rate"])
    parser.add_argument("--show", type=int, default=20, help="bad steps to print")
    args = parser.parse_args(argv)
    lines = open(args.log, errors="replace").read().splitlines()
    before, window = window_lines(lines, args.window)
    ok, detail, bad = judge(before, window, {"max_speed": args.max_speed,
                                             "max_turn_rate": args.max_turn_rate})
    print(("PASS " if ok else "FAIL ") + detail)
    for record in bad[:args.show]:
        print("  frame %d dt %.4f step %.2f/%.2f turn %.2f/%.2f through %s  %s %s -> %s %s" % (
            record["frame"], record["dt"], record["step"], record["step_limit"], record["turn"],
            record["turn_limit"], record["through"], fmt(record["from"]), fmt(record["from_angles"]),
            fmt(record["to"]), fmt(record["to_angles"])))
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
