#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Run and judge the VGUI fixture host (RFC 0010 V0 fixed-screen corpus).

    python3 tools/vgui/vgui_fixture_host.py run --build BUILD --renderer native-vulkan
        --out OUT [--lib-path DIR ...] [--steady N] [--display headless|desktop]
        (BUILD and the library path default to $VGUI_FIXTURE_BUILD, $VGUI_FIXTURE_LIB_PATH)
    python3 tools/vgui/vgui_fixture_host.py check --out OUT

`run` stages a runtime that holds only repository-owned content:

- the host (unittests/vguihost/vgui_fixture_host.cpp) from BUILD, a Waf output
  tree configured with SDL3 and a render backend;
- the fixture game `vguifixture`: quality/fixtures/vgui-surface/game (gameinfo,
  the fixture scheme and the packaged DejaVu Sans) plus the fixture materials
  written by tools/vgui/vgui_fixture_content.py.

It runs the host headless (SDL's offscreen driver; Vulkan through
VK_EXT_headless_surface) in a throwaway HOME, then judges the capture. `check`
judges an existing capture again. Both print one checks-v1 record and write
OUT/evidence.json.

The oracle:

- the capture is complete: four screens, each with its frame and counters;
- the empty screen is the clear color, every pixel (readback works);
- primitives are held to closed forms of UnlitGeneric's PC model
  (vertexlit_and_unlit_generic with sRGB writes: vertex colors and $color
  decoded as gamma 2.2, textures and the destination decoded as sRGB, blending
  in linear light, the result encoded as sRGB), within PIXEL_TOLERANCE;
  texture orientation, sub-rectangles, polygons, $frame frames and point
  sampling are exact;
- the controls screen draws the image panel and the frame's scheme colors;
- the text screens cover a plausible share of their text boxes;
- the counters: the empty screen draws nothing; every other screen draws, one
  paint pass per frame; after warm-up nothing is uploaded or copied; the text
  screen's draws are all text draws, and its first frame rasterizes glyphs;
- the log: the packaged font loaded, no VGUI material is missing, and
  fontconfig was asked only for the foreign-script fallback (a fontconfig
  "DejaVu Sans" means the packaged font was replaced by a system one).

Known renderer gaps (KNOWN_GAPS) are checked to still be present: a gap that
closes fails until the list is updated, so a fix is never silent.

No D3D9 reference capture exists for these screens; the closed forms are the
reference.
"""

import argparse
import datetime
import hashlib
import json
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools" / "quality"))
sys.path.insert(0, str(ROOT / "tools" / "vgui"))
import launch_sandbox  # noqa: E402
import vgui_fixture_content  # noqa: E402

SCHEMA = "vgui-fixture-frames/v1"
EVIDENCE_SCHEMA = "vgui-fixture-evidence/v1"
GAME = "vguifixture"
GAME_SOURCE = ROOT / "quality" / "fixtures" / "vgui-surface" / "game"
SCREENS = ("primitives", "controls", "text", "empty")
PIXEL_TOLERANCE = 2

# Renderer gaps the oracle expects to see, by renderer id. Each names the
# probe group it covers; those probes must still fail.
KNOWN_GAPS = {
    "native-vulkan": {
        "lines": "native Vulkan drops line and line-strip draws (shaderapivulkan.cpp "
                 "'draw dropped: line/point topology'; RFC 0010 V0, RFC 0016 binding rules: "
                 "fixed by the core UI pass, not the frozen backend)",
    },
}

# The fixture scheme's font family as the surface registers the packaged file;
# the only fontconfig request expected is FontManager's foreign fallback.
FOREIGN_FALLBACK = "WenQuanYi Zen Hei"


# ---------------------------------------------------------------------------
# The color model (UnlitGeneric, PC, sRGB writes)
# ---------------------------------------------------------------------------

def gamma_decode(value):
    """Vertex colors and $color: the shader's GammaToLinear, x^2.2."""
    return (value / 255.0) ** 2.2


def srgb_decode(value):
    c = value / 255.0
    return c / 12.92 if c <= 0.04045 else ((c + 0.055) / 1.055) ** 2.4


def srgb_encode(linear):
    linear = min(max(linear, 0.0), 1.0)
    c = linear * 12.92 if linear <= 0.0031308 else 1.055 * linear ** (1 / 2.4) - 0.055
    return int(round(c * 255))


def over(src_linear, alpha, dst):
    """Straight alpha over the destination, in linear light."""
    return tuple(srgb_encode(s * alpha + srgb_decode(d) * (1 - alpha)) for s, d in zip(src_linear, dst))


def expected_probes(clear):
    """{name: ((x, y), rgb or None, tolerance)} for the primitives screen. The
    positions are those vgui_fixture_host.cpp draws at."""
    c = clear[:3]
    fill = tuple(srgb_encode(gamma_decode(v)) for v in (200, 40, 40))
    half = 128 / 255.0
    blended = over([gamma_decode(v) for v in (40, 200, 40)], half, c)
    additive = tuple(srgb_encode(srgb_decode(t) + srgb_decode(d)) for t, d in zip((128, 64, 32), c))
    tinted = tuple(srgb_encode(srgb_decode(t) * gamma_decode(m * 255))
                   for t, m in zip((255, 0, 0), (0.5, 0.5, 1.0)))
    ramp = over([1.0, 1.0, 1.0], half, c)
    exact = 0
    return {
        "filled rectangle": ((52, 52), fill, PIXEL_TOLERANCE),
        "alpha rectangle": ((132, 52), blended, PIXEL_TOLERANCE),
        "outlined rectangle edge": ((180, 52), (255, 255, 255), exact),
        "outlined rectangle interior": ((212, 52), c, exact),
        "quadrants top-left": ((36, 116), (255, 0, 0), exact),
        "quadrants top-right": ((68, 116), (0, 255, 0), exact),
        "quadrants bottom-left": ((36, 148), (0, 0, 255), exact),
        "quadrants bottom-right": ((68, 148), (255, 255, 255), exact),
        "polygon top-left": ((116, 116), (255, 0, 0), exact),
        "polygon bottom-right": ((148, 148), (255, 255, 255), exact),
        "atlas cell (2, 1)": ((212, 132), (160, 96, 128), exact),
        "additive": ((292, 132), additive, PIXEL_TOLERANCE),
        "opaque ignores texture alpha": ((372, 132), (40, 120, 200), exact),
        "$color tint": ((436, 116), tinted, PIXEL_TOLERANCE),
        "alpha ramp column 128": ((148, 188), ramp, PIXEL_TOLERANCE),
        "frame 0": ((308, 188), (255, 0, 0), exact),
        "frame 1": ((328, 188), (0, 255, 0), exact),
        "frame 2": ((348, 188), (0, 0, 255), exact),
    }


# Line primitives: a window that must hold the line's color somewhere.
LINE_PROBES = {
    "outlined circle": ((344, 52), 3, (255, 255, 255)),
    "polyline": ((420, 52), 2, (255, 255, 0)),
    "line": ((500, 188), 2, (255, 255, 0)),
}

# Controls screen: the frame at (40, 40); its image panel at (260, 40) inside.
CONTROLS_PROBES = {
    "image panel top-left": ((316, 96), (255, 0, 0)),
    "image panel top-right": ((348, 96), (0, 255, 0)),
    "image panel bottom-left": ((316, 128), (0, 0, 255)),
    "image panel bottom-right": ((348, 128), (255, 255, 255)),
}


# ---------------------------------------------------------------------------
# Capture access
# ---------------------------------------------------------------------------

class Frame:
    def __init__(self, data, width, height):
        self.data, self.width, self.height = data, width, height

    def rgb(self, x, y):
        i = (y * self.width + x) * 4
        return tuple(self.data[i:i + 3])

    def coverage(self, x0, y0, x1, y1, clear):
        """Share of pixels in [x0, x1) x [y0, y1) that differ from the clear color."""
        hits = total = 0
        for y in range(y0, y1):
            for x in range(x0, x1):
                total += 1
                hits += self.rgb(x, y) != tuple(clear[:3])
        return hits / total if total else 0.0

    def window_has(self, center, radius, color):
        cx, cy = center
        for y in range(cy - radius, cy + radius + 1):
            for x in range(cx - radius, cx + radius + 1):
                if 0 <= x < self.width and 0 <= y < self.height and \
                        max(abs(a - b) for a, b in zip(self.rgb(x, y), color)) <= 40:
                    return True
        return False


def close(actual, expected, tolerance):
    return max(abs(a - b) for a, b in zip(actual, expected)) <= tolerance


# ---------------------------------------------------------------------------
# The oracle
# ---------------------------------------------------------------------------

def evaluate(out_dir):
    """[(ok, label)] for the capture in out_dir, and a summary."""
    checks = []

    def check(ok, label):
        checks.append((bool(ok), label))
        return ok

    summary_path = out_dir / "frames.json"
    try:
        report = json.loads(summary_path.read_text())
    except (OSError, ValueError) as error:
        check(False, "the host wrote %s: %s" % (summary_path.name, error))
        return checks, {}
    check(report.get("schema") == SCHEMA, "capture schema " + SCHEMA)
    width, height = report.get("width", 0), report.get("height", 0)
    clear = report.get("clear", [0, 0, 0, 255])
    renderer = report.get("renderer", "")
    gaps = KNOWN_GAPS.get(renderer, {})
    screens = {s.get("name"): s for s in report.get("screens", [])}
    check(sorted(screens) == sorted(SCREENS), "the capture has the screens %s" % ", ".join(SCREENS))
    fonts = report.get("fonts", {})
    check(all(fonts.get(name, 0) > 0 for name in ("Default", "DefaultLarge", "DefaultOutline")),
          "the scheme's three fonts exist (tall %s)" % fonts)

    frames = {}
    for name in SCREENS:
        screen = screens.get(name, {})
        path = out_dir / screen.get("frame", name + ".rgba")
        data = path.read_bytes() if path.is_file() else b""
        complete = (screen.get("frame_written") and screen.get("have_first") and
                    screen.get("have_steady") and len(data) == width * height * 4)
        check(complete, "%s: frame and first/steady counters captured" % name)
        if complete:
            frames[name] = Frame(data, width, height)

    # Readback: the empty screen is the clear color everywhere.
    empty = frames.get("empty")
    if empty:
        off = sum(1 for y in range(height) for x in range(width) if empty.rgb(x, y) != tuple(clear[:3]))
        check(off == 0, "empty: every pixel is the clear color %s (%d differ)" % (tuple(clear[:3]), off))

    primitives = frames.get("primitives")
    if primitives:
        for label, (point, expected, tolerance) in expected_probes(clear).items():
            actual = primitives.rgb(*point)
            check(close(actual, expected, tolerance), "primitives: %s at %s is %s +-%d (read %s)"
                  % (label, point, expected, tolerance, actual))
        fade_left, fade_right = primitives.rgb(261, 52), primitives.rgb(322, 52)
        check(fade_left[0] >= 245 and fade_right[0] <= 60,
              "primitives: the horizontal fade runs from opaque to clear (read %s .. %s)"
              % (fade_left, fade_right))
        row = [primitives.rgb(x, 100) for x in range(500, 564)]
        column = [primitives.rgb(500, y) for y in range(100, 164)]
        alternates = all(px in ((255, 255, 255), (0, 0, 0)) for px in row + column) and \
            all(row[i] != row[i + 1] for i in range(len(row) - 1)) and \
            all(column[i] != column[i + 1] for i in range(len(column) - 1))
        check(alternates, "primitives: the point-sampled checker maps one texel to one pixel")
        text = primitives.coverage(20, 220, 620, 240, clear)
        check(0.03 <= text <= 0.6, "primitives: text covers %.3f of its box (0.03..0.6)" % text)
        for label, (center, radius, color) in LINE_PROBES.items():
            drawn = primitives.window_has(center, radius, color)
            if "lines" in gaps:
                check(not drawn, "primitives: %s: known gap still present (%s)" % (label, gaps["lines"]))
            else:
                check(drawn, "primitives: %s is drawn near %s" % (label, center))

    controls = frames.get("controls")
    if controls:
        for label, (point, expected) in CONTROLS_PROBES.items():
            actual = controls.rgb(*point)
            check(actual == expected, "controls: %s at %s is %s (read %s)" % (label, point, expected, actual))
        title = controls.rgb(240, 56)
        check(title == (96, 96, 96), "controls: the title bar is the scheme's FixtureGray (read %s)" % (title,))
        check(controls.coverage(40, 40, 440, 340, clear) > 0.95,
              "controls: the frame covers its bounds")

    text = frames.get("text")
    if text:
        share = text.coverage(10, 10, 620, 300, clear)
        check(0.03 <= share <= 0.6, "text: glyphs cover %.3f of the text area (0.03..0.6)" % share)

    # Counters.
    for name in SCREENS:
        screen = screens.get(name)
        if not screen:
            continue
        first, steady = screen.get("first", {}), screen.get("steady", {})
        check(steady.get("frames") == report.get("steady_frames") and first.get("frames") == 1,
              "%s: counters span 1 first frame and %s steady frames" % (name, report.get("steady_frames")))
        check(steady.get("paintPasses") == 1.0, "%s: one paint pass per frame (read %s)"
              % (name, steady.get("paintPasses")))
        if name == "empty":
            check(steady.get("draws") == 0.0, "empty: draws nothing (read %s)" % steady.get("draws"))
        else:
            check(steady.get("draws", 0) > 0, "%s: draws (read %s per frame)" % (name, steady.get("draws")))
        check(float(steady.get("draws", -1)).is_integer() and
              float(steady.get("vertices", -1)).is_integer(),
              "%s: every steady frame submits the same draws and vertices" % name)
        for counter in ("textureUploads", "glyphUploads", "cpuCopies"):
            check(steady.get(counter) == 0.0, "%s: no %s after warm-up (read %s)"
                  % (name, counter, steady.get(counter)))
    text_screen = screens.get("text", {})
    if text_screen:
        steady, first = text_screen.get("steady", {}), text_screen.get("first", {})
        check(steady.get("draws", 0) > 0 and steady.get("textDraws") == steady.get("draws"),
              "text: every draw is a text draw (%s of %s)" % (steady.get("textDraws"), steady.get("draws")))
        check(first.get("glyphUploads", 0) > 0 and
              first.get("textureUploads", 0) >= first.get("glyphUploads", 0),
              "text: its first frame rasterizes glyphs (%s uploads, %s glyphs)"
              % (first.get("textureUploads"), first.get("glyphUploads")))

    # The log.
    log_path = out_dir / "host.log"
    log = log_path.read_text(errors="replace") if log_path.is_file() else ""
    check(log != "", "the host log exists")
    check("Failed to load custom font file" not in log and "Couldn't find custom font file" not in log,
          "the packaged font loaded")
    missing = sorted(set(re.findall(r"Missing Vgui material (\S+)", log)))
    check(not missing, "no VGUI material is missing (%s)" % (", ".join(missing) or "none"))
    requested = sorted(set(re.findall(r"^font fc: (.+?) - ", log, re.M)))
    check(set(requested) <= {FOREIGN_FALLBACK},
          "fontconfig was asked only for the foreign fallback (asked: %s)" % (", ".join(requested) or "none"))

    summary = {
        "renderer": renderer,
        "known_gaps": gaps,
        "fonts": fonts,
        "foreign_fallback": re.findall(r"^font fc: .+$", log, re.M),
        "counters": {name: {"first": screens[name].get("first"), "steady": screens[name].get("steady")}
                     for name in SCREENS if name in screens},
    }
    return checks, summary


def report_checks(checks):
    failures = 0
    for ok, label in checks:
        print("%s %s" % ("PASS" if ok else "FAIL", label))
        failures += not ok
    print("CONFORMANCE %d %d" % (len(checks), failures))
    sys.stdout.flush()
    return 0 if checks and not failures else 1


def write_evidence(out_dir, evidence, checks, summary):
    evidence.update({"checks": len(checks), "failures": [label for ok, label in checks if not ok],
                     "summary": summary})
    evidence["status"] = "pass" if checks and not evidence["failures"] else "fail"
    (out_dir / "evidence.json").write_text(json.dumps(evidence, indent=2) + "\n")


# ---------------------------------------------------------------------------
# Staging and running
# ---------------------------------------------------------------------------

def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def find_host(build):
    matches = sorted(Path(build).rglob("vgui_fixture_host"))
    matches = [m for m in matches if m.is_file() and os.access(m, os.X_OK)]
    if not matches:
        raise RuntimeError("no vgui_fixture_host under %s (build the target first)" % build)
    return matches[0]


def library_path(build, extra):
    directories = sorted({str(p.parent) for p in Path(build).rglob("*.so")})
    return ":".join([str(Path(e).resolve()) for e in extra] + directories)


def stage(out_dir, build):
    runtime = out_dir / "runtime"
    if runtime.exists():
        shutil.rmtree(runtime)
    game = runtime / GAME
    shutil.copytree(GAME_SOURCE, game)
    for path, data in vgui_fixture_content.files().items():
        target = game / path
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(data)
    host = runtime / "vgui_fixture_host"
    shutil.copy2(find_host(build), host)
    staged = {str(p.relative_to(runtime)): sha256(p) for p in sorted(game.rglob("*")) if p.is_file()}
    return runtime, host, staged


def run(args):
    out_dir = args.out.resolve()
    out_dir.mkdir(parents=True, exist_ok=True)
    if (out_dir / "evidence.json").exists():
        print("error: %s already holds a capture; use a new directory" % out_dir, file=sys.stderr)
        return 2
    evidence = {"schema": EVIDENCE_SCHEMA,
                "started_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
                "renderer": args.renderer, "display": args.display, "steady_frames": args.steady}
    try:
        runtime, host, staged = stage(out_dir, args.build)
    except (OSError, RuntimeError) as error:
        print("error: staging failed: %s" % error, file=sys.stderr)
        return 2
    evidence["host_sha256"] = sha256(host)
    evidence["staged_game"] = staged
    capture = out_dir / "capture"
    capture.mkdir(exist_ok=True)
    command = [str(host), "-game", GAME, "-renderer", args.renderer, "-out", str(capture),
               "-steady", str(args.steady)]
    sandbox = launch_sandbox.Sandbox(out_dir / "sandbox", write_paths=[runtime, capture])
    environment = sandbox.environment(os.environ)
    environment["LD_LIBRARY_PATH"] = library_path(args.build, args.lib_path) + \
        (":" + os.environ["LD_LIBRARY_PATH"] if os.environ.get("LD_LIBRARY_PATH") else "")
    if args.display == "headless":
        environment["SDL_VIDEO_DRIVER"] = "offscreen"
        for variable in ("WAYLAND_DISPLAY", "DISPLAY"):
            environment.pop(variable, None)
    evidence["command"] = command
    log_path = capture / "host.log"
    with open(log_path, "w") as log:
        try:
            completed = subprocess.run(command, cwd=runtime, env=environment, stdout=log,
                                       stderr=subprocess.STDOUT, timeout=args.timeout)
            evidence["returncode"] = completed.returncode
        except subprocess.TimeoutExpired:
            evidence["returncode"] = None
    evidence["sandbox"] = sandbox.finish()
    checks, summary = evaluate(capture)
    checks.insert(0, (evidence["returncode"] == 0, "the host exited 0 (exit %s)" % evidence["returncode"]))
    write_evidence(out_dir, evidence, checks, summary)
    return report_checks(checks)


def check_command(args):
    out_dir = args.out.resolve()
    checks, summary = evaluate(out_dir / "capture")
    evidence_path = out_dir / "evidence.json"
    evidence = json.loads(evidence_path.read_text()) if evidence_path.is_file() else {"schema": EVIDENCE_SCHEMA}
    write_evidence(out_dir, evidence, checks, summary)
    return report_checks(checks)


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    sub = parser.add_subparsers(dest="command", required=True)
    run_parser = sub.add_parser("run", help="stage, run the host and judge")
    run_parser.add_argument("--build", type=Path, default=os.environ.get("VGUI_FIXTURE_BUILD"),
                            help="Waf output tree (default $VGUI_FIXTURE_BUILD)")
    run_parser.add_argument("--renderer", required=True)
    run_parser.add_argument("--out", type=Path, required=True)
    run_parser.add_argument("--lib-path", action="append",
                            default=[p for p in os.environ.get("VGUI_FIXTURE_LIB_PATH", "").split(":") if p],
                            help="extra library directory searched first, e.g. a private SDL3 "
                                 "(default $VGUI_FIXTURE_LIB_PATH, colon-separated)")
    run_parser.add_argument("--steady", type=int, default=10)
    run_parser.add_argument("--display", choices=("headless", "desktop"), default="headless")
    run_parser.add_argument("--timeout", type=float, default=300)
    check_parser = sub.add_parser("check", help="judge an existing capture again")
    check_parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args(argv)
    if args.command == "run" and not args.build:
        parser.error("run needs --build or $VGUI_FIXTURE_BUILD")
    return run(args) if args.command == "run" else check_command(args)


if __name__ == "__main__":
    sys.exit(main())
