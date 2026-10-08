#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Portal 2's chamber sign as an emissive surface on the render core.

RFC 0016 render.pass.panels and render.world-panel.v1 (render/world_panel.h):
the sign (sp_a1_intro6's info_panel, a vgui_screen drawing sp_progress_sign)
is recorded once per frame into a draw list at the resolution its footprint
needs, drawn by the core as a PBRMetalRough surface whose emission is that
image, and casts the image's light as a grid of RFC 0011 area lights. Its
grime (the lightboard's dirt overlays) is a coating: it dims and tints the
board's light, catches the light around it, reflects the room's, and never
emits light of its own.

The suite boots the installed product headless on native Vulkan
(portal_boot.py) twice, with r_core_world 1:
  core     the sign switched on (SetActive), a burst of one screenshot per
           frame through its power-on flicker (host_framerate 0.02,
           mat_force_tonemap_scale 1, cl_world_panel_report printing each
           frame's recorded list, its screen corners and its tile lights),
           then a close shot of its text, a mid shot, and the area-light
           report of a steady lit frame;
  legacy   the same close shot with r_core_panels 0 (the legacy 2D path),
           the sharpness control.
It judges:
  core.takes-sign             the core drew the sign (lists, rasters, views)
                              and failed and refused none
  resolution.texel-per-pixel  the close shot's image holds 1 to 1.5 texels per
                              screen pixel along the sign (world_panel::
                              ChooseResolution's ladder and hold)
  sharp.core                  the text's edges in the close shot span at most
                              1.6 pixels (10 to 90 percent)
  sharp.legacy-blurred        the legacy 2D path's edges span more (its 28
                              pixel font magnified): the control the core
                              check must reject
  flicker.one-state           every burst frame: the sign's pixels are one
                              linear function of the light it publishes in
                              the same frame (slope 0.9 to 1.2: its bloom;
                              offset: the room's reflection), residual at
                              most 0.0015 + 2 percent, across flicker states
                              whose light differs at least 2 times; the image
                              paired with its neighbour frame's light breaks
                              it (the control)
  flicker.one-list            one list and one raster per frame drawn
  dirt.every-state            the dirt overlay is in every burst frame's list,
                              at the flicker state's overlay alpha
  dirt.dims-cast-light        every burst frame's tile lights are dimmer than
                              the same list without the grime
  area-light.tiles            the steady frame publishes the sign's 8 tile
                              lights (keys (entity << 8) | 0x80 | tile) with
                              the report's radiances

    portal2_sign_panel.py suite --out <dir>       # checks-v1
    portal2_sign_panel.py selftest                # the judge on synthetic data

Needs numpy and Pillow, a native Vulkan Portal 2 build (build-p2/,
SOURCE_PORTAL2_BUILD) and a licensed Portal 2 install
(SOURCE_PORTAL2_STEAM_ROOT), from which a content runtime is staged under
--out (or pass --p2-runtime). Keep --out short: deep paths break the boot.
"""

import argparse
import datetime
import json
from pathlib import Path
import re
import subprocess
import sys

import numpy
from PIL import Image

QUALITY = Path(__file__).resolve().parent
sys.path.insert(0, str(QUALITY))

import conformance_result  # noqa: E402

sys.path.insert(0, str(QUALITY.parent / "kiln"))
import sepipe_loader  # noqa: E402

ROOT = QUALITY.parents[1]
PORTAL_BOOT = QUALITY / "portal_boot.py"
SCHEMA = "portal2-sign-panel-evidence/v1"
MAP = "sp_a1_intro6"
SIGN = "info_panel-info_panel"
WIDTH, HEIGHT = 1024, 720
MARK = "SIGNPANEL"
BURST = 90
# The cameras (setpos, setang; setpos places the player, whose eye is 64
# units higher): head-on to the whole sign, and close to its progress label
# ("06/19"), about 3 screen pixels per panel unit.
MID = ((368, -480, -32), (0, 90, 0))
CLOSE = ((349, -293, -99), (0, 90, 0))
# The sign's progress label, in panel units (sp_progress_sign.res
# LevelProgressLabel: x 81, y 386, 400 x 100; the text is left aligned).
LABEL = (81.0, 390.0, 200.0, 440.0)
DIRT = "elevator_video_overlay"

# Thresholds, set before the first run.
SHARP_CORE_MAX = 1.6
SHARP_LEGACY_MARGIN = 0.4
TEXEL_PER_PIXEL = (1.0, 1.5)
FIT_RESIDUAL = ( 0.0015, 0.02 )  # linear: absolute, and relative to the light
FIT_SLOPE = ( 0.9, 1.2 )
MIN_STATE_RATIO = 2.0


# ---------------------------------------------------------------------------
# Boots.

def console_lines(path):
    """The cfg lines of one boot (each line's waits delay only that line, so
    the burst chains aliases)."""
    lines = []
    common = "sv_cheats 1; r_core_world 1; mat_force_tonemap_scale 1; noclip; crosshair 0; " \
             "r_drawviewmodel 0"
    if path == "legacy":
        lines.append(common + "; r_core_panels 0; wait 120; ent_fire %s SetActive; "
                     "cmd setpos %g %g %g; cmd setang %g %g %g; wait 300; echo %s close; screenshot; "
                     "wait 10; echo %s end" % ((SIGN,) + CLOSE[0] + CLOSE[1] + (MARK, MARK)))
        return lines
    for i in range(1, BURST + 1):
        after = "b%d" % (i + 1) if i < BURST else "bend"
        lines.append('alias b%d "wait 1; echo %s burst %d; screenshot; %s"' % (i, MARK, i, after))
    lines.append('alias bend "wait 5; host_framerate 0; cl_world_panel_report 0; bshots"')
    # A frame can run several ticks, and waits count ticks: each shot keeps
    # its marker, report and screenshot apart from the next by 10.
    lines.append(('alias bshots "wait 200; cmd setpos %g %g %g; cmd setang %g %g %g; wait 60; '
                  'echo %s close; cl_world_panel_report 1; screenshot; wait 10; '
                  'cl_world_panel_report 0; wait 10; bmid"') % (CLOSE[0] + CLOSE[1] + (MARK,)))
    lines.append(('alias bmid "cmd setpos %g %g %g; cmd setang %g %g %g; wait 60; echo %s mid; '
                  'cl_world_panel_report 1; r_area_lights_report 1; screenshot; wait 10; '
                  'cl_world_panel_report 0; r_core_panels_stats; echo %s end"')
                 % (MID[0] + MID[1] + (MARK, MARK)))
    lines.append(common + "; wait 120; cmd setpos %g %g %g; cmd setang %g %g %g; wait 60; "
                 "cl_world_panel_report 1; host_framerate 0.02; ent_fire %s SetActive; b1"
                 % (MID[0] + MID[1] + (SIGN,)))
    return lines


def boot(args, path, out):
    command = [sys.executable, str(PORTAL_BOOT), *sepipe_loader.boot_arguments(args),
               "--out", str(out),
               "--headless", "--map", MAP, "--renderer", "native-vulkan", "--require-vulkan",
               "--physics", "vphysics_box3d", "--width", str(WIDTH), "--height", str(HEIGHT),
               "--capture-wait", "1500", "--timeout", str(args.timeout)]
    for line in console_lines(path):
        command += ["--console-command", line]
    completed = subprocess.run(command, capture_output=True, text=True)
    console = out / "runtime/portal2/console.log"
    return {"path": path, "returncode": completed.returncode,
            "boot_tail": (completed.stdout[-600:] + completed.stderr[-600:]).strip(),
            "console": console.read_text(errors="replace") if console.is_file() else "",
            "screenshots": [str(p) for p in sorted((out / "runtime/portal2/screenshots").glob("*.tga"))]}


# ---------------------------------------------------------------------------
# Parsing: the report lines and which frame each screenshot shows.

REPORT = re.compile(r"^cl_world_panel_report: frame (\d+) ent (\d+) res (\d+)x(\d+) quads (\d+) "
                    r"textures(.*) placement (.*) units (\d+) (\d+) screen (.*) tiles (\d+)x(\d+) "
                    r"(.*) without (.*)$")


def parse_report(line):
    m = REPORT.match(line.strip())
    if not m:
        return None
    textures = [(name, int(count), int(alpha)) for name, count, alpha in
                re.findall(r" (\S+?):(\d+):(\d+)", m.group(6))]
    placement = [float(v) for v in m.group(7).split()]
    across, down = int(m.group(11)), int(m.group(12))
    return {"frame": int(m.group(1)), "ent": int(m.group(2)),
            "res": (int(m.group(3)), int(m.group(4))), "quads": int(m.group(5)),
            "textures": textures, "origin": placement[0:3], "right": placement[3:6],
            "down": placement[6:9], "units": (int(m.group(8)), int(m.group(9))),
            "screen": [float(v) for v in m.group(10).split()], "tiles": (across, down),
            "lit": numpy.array([float(v) for v in m.group(13).split()]).reshape(-1, 3),
            "without": numpy.array([float(v) for v in m.group(14).split()]).reshape(-1, 3)}


def shot_frames(console):
    """[(label, report)] per screenshot, in order: a screenshot command's
    frame is the report printed after it (the frame's pre-render) and before
    the next frame's commands. Several commands in one frame make one shot."""
    shots = []
    pending = None
    for line in console.splitlines():
        marker = re.match(r"^%s (\S+)(?: (\d+))?" % MARK, line)
        if marker:
            if marker.group(1) != "end":
                pending = marker.group(1)
            continue
        if line.startswith("cl_world_panel_report: frame") and pending is not None:
            shots.append((pending, parse_report(line)))
            pending = None
    return shots


AREA = re.compile(r"^\s+(?:slotless )?area \d+ key (-?\d+)(?: v\d+ slot -?\d+)? at (\S+) (\S+) (\S+)"
                  r".*? radiance (\S+) (\S+) (\S+) reach (\S+)")


def area_lights_after(console, label):
    """The published area lights (slotted and slotless) of the first
    r_area_lights_report after a marker; None without one."""
    seen = False
    lights = None
    for line in console.splitlines():
        if line.startswith("%s %s" % (MARK, label)):
            seen = True
            continue
        if not seen:
            continue
        if line.startswith("area lights generation"):
            if lights is not None:
                break
            lights = []
            continue
        m = AREA.match(line)
        if m and lights is not None:
            values = [float(v) for v in m.groups()]
            lights.append({"key": int(m.group(1)), "center": values[1:4], "radiance": values[4:7],
                           "reach": values[7]})
        elif lights is not None and not line.startswith("  "):
            break
    return lights


def stats(console):
    m = re.findall(r"r_core_panels_stats: lists (\d+) refused (\d+) rasterized (\d+) views queued "
                   r"(\d+) drawn (\d+) failed (\d+) panels drawn (\d+)", console)
    if not m:
        return None
    keys = ("lists", "refused", "rasterized", "queued", "drawn", "failed", "panels")
    return dict(zip(keys, map(int, m[-1])))


# ---------------------------------------------------------------------------
# Measurements.

def linear(image):
    a = numpy.asarray(image.convert("RGB"), dtype=numpy.float64) / 255.0
    return numpy.where(a <= 0.04045, a / 12.92, ((a + 0.055) / 1.055) ** 2.4)


def screen_points(report, width, height):
    """The panel's corners on screen (top-left, top-right, bottom-right,
    bottom-left), in pixels."""
    pts = numpy.array(report["screen"]).reshape(4, 2)
    return numpy.stack([(pts[:, 0] * 0.5 + 0.5) * width, (0.5 - pts[:, 1] * 0.5) * height], axis=1)


def to_screen(report, width, height, u, v):
    """A panel point (u, v in units) on screen, bilinear between the corners
    (the shots are head-on)."""
    c = screen_points(report, width, height)
    fu, fv = u / report["units"][0], v / report["units"][1]
    top = c[0] + (c[1] - c[0]) * fu
    bottom = c[3] + (c[2] - c[3]) * fu
    return top + (bottom - top) * fv


def pixels_per_unit(report, width, height):
    c = screen_points(report, width, height)
    return max(numpy.linalg.norm(c[1] - c[0]) / report["units"][0],
               numpy.linalg.norm(c[3] - c[0]) / report["units"][1])


def tile_means(image_linear, report):
    """The image's mean linear color over each of the report's tiles."""
    h, w = image_linear.shape[:2]
    across, down = report["tiles"]
    units = report["units"]
    means = []
    for ty in range(down):
        for tx in range(across):
            values = []
            for j in range(12):
                for i in range(12):
                    u = (tx + (i + 0.5) / 12) / across * units[0]
                    v = (ty + (j + 0.5) / 12) / down * units[1]
                    x, y = to_screen(report, w, h, u, v)
                    if 0 <= int(x) < w and 0 <= int(y) < h:
                        values.append(image_linear[int(y), int(x)])
            means.append(numpy.mean(values, axis=0) if values else numpy.full(3, numpy.nan))
    return numpy.array(means)


def edge_widths(image_linear, region):
    """The 10 to 90 percent widths, in pixels, of the strong horizontal
    transitions in a region (x0, y0, x1, y1): each row's steepest steps, with
    the plateaus read 5 pixels to either side."""
    x0, y0, x1, y1 = [int(round(v)) for v in region]
    luma = image_linear[..., :3] @ numpy.array([0.2126, 0.7152, 0.0722])
    patch = luma[max(y0, 0):y1, max(x0, 0):x1]
    if patch.size == 0:
        return []
    span = numpy.percentile(patch, 95) - numpy.percentile(patch, 5)
    widths = []
    for row in patch:
        grad = numpy.abs(numpy.diff(row))
        for x in numpy.argsort(grad)[::-1][:6]:
            if grad[x] < 0.25 * span or x < 6 or x + 7 > len(row):
                continue
            lo, hi = row[x - 5], row[x + 6]
            if abs(hi - lo) < 0.5 * span:
                continue
            window = row[x - 5:x + 7]
            levels = [lo + (hi - lo) * f for f in (0.1, 0.9)]
            crossings = []
            for level in levels:
                for k in range(len(window) - 1):
                    a, b = window[k] - level, window[k + 1] - level
                    if a == 0 or (a < 0) != (b < 0):
                        crossings.append(k + (a / (a - b) if a != b else 0.0))
                        break
            if len(crossings) == 2:
                widths.append(abs(crossings[1] - crossings[0]))
    return widths


# ---------------------------------------------------------------------------
# The judge: pure functions over parsed runs (the self-test feeds it
# synthetic runs).

def judge(core, legacy):
    """[(check, passed, detail)] over the core run and the legacy control."""
    verdicts = []

    def add(name, passed, detail=""):
        verdicts.append((name, bool(passed), detail))

    s = core["stats"]
    add("core.takes-sign", s is not None and s["lists"] > 0 and s["rasterized"] == s["lists"] and
        s["drawn"] > 0 and s["failed"] == 0 and s["refused"] == 0, "r_core_panels_stats %s" % s)

    close = core["shots"].get("close")
    if close:
        report, image = close
        h, w = image.shape[:2]
        ratio = report["res"][0] / report["units"][0] / pixels_per_unit(report, w, h)
        add("resolution.texel-per-pixel", TEXEL_PER_PIXEL[0] <= ratio <= TEXEL_PER_PIXEL[1],
            "%.3f texels per screen pixel (%dx%d image)" % (ratio, report["res"][0], report["res"][1]))
    else:
        add("resolution.texel-per-pixel", False, "no close shot")

    sharp = core.get("sharp")
    legacy_sharp = legacy.get("sharp") if legacy else None
    add("sharp.core", sharp is not None and sharp <= SHARP_CORE_MAX,
        "median edge width %s pixels" % sharp)
    add("sharp.legacy-blurred", legacy_sharp is not None and sharp is not None and
        legacy_sharp > sharp + SHARP_LEGACY_MARGIN and legacy_sharp > SHARP_CORE_MAX,
        "legacy %s pixels, core %s" % (legacy_sharp, sharp))

    burst = core["burst"]  # [(report, tile means of the image)]
    lights = [float(numpy.mean(rep["lit"][:, 1])) for rep, _ in burst]

    def one_state( images ):
        """The fit of the images to the lights: slope, offset, worst excess
        residual over the band (<= 0 holds)."""
        a = numpy.vstack( [lights, numpy.ones( len( lights ) )] ).T
        slope, offset = numpy.linalg.lstsq( a, numpy.array( images ), rcond=None )[0]
        excess = max( abs( slope * l + offset - i ) - ( FIT_RESIDUAL[0] + FIT_RESIDUAL[1] * l )
                      for l, i in zip( lights, images ) )
        return slope, offset, excess

    if len(burst) >= 10 and lights and min(lights) > 0:
        images = [float(numpy.mean(img[:, 1])) for _, img in burst]
        slope, offset, excess = one_state(images)
        span = max(lights) / min(lights)
        add("flicker.one-state", excess <= 0 and FIT_SLOPE[0] <= slope <= FIT_SLOPE[1] and
            span >= MIN_STATE_RATIO,
            "%d frames, light %.4f to %.4f (%.1fx): image = %.3f light + %.4f, worst excess %.5f" % (
                len(burst), min(lights), max(lights), span, slope, offset, excess))
        # The control: each image against the next frame's light.
        shifted = images[1:] + images[-1:]
        _, _, control_excess = one_state(shifted)
        add("flicker.control-misaligned-rejected", control_excess > 0,
            "images against their neighbour frame's light: worst excess %.5f" % control_excess)
    else:
        add("flicker.one-state", False, "%d burst frames" % len(burst))
        add("flicker.control-misaligned-rejected", False, "no burst")

    frames = [rep["frame"] for rep, _ in burst]
    add("flicker.one-list", frames == sorted(set(frames)) and s is not None and
        s["rasterized"] == s["lists"], "frames %s, stats %s" % (frames[:5], s))

    dirt = [max([alpha for name, count, alpha in rep["textures"] if DIRT in name] or [0])
            for rep, _ in burst]
    add("dirt.every-state", burst and all(a > 0 for a in dirt),
        "dirt alpha per frame: %s" % sorted(set(dirt)))
    dims = [float(numpy.sum(rep["lit"])) < float(numpy.sum(rep["without"])) for rep, _ in burst]
    add("dirt.dims-cast-light", burst and all(dims),
        "%d of %d frames dimmer with the grime" % (sum(dims), len(dims)))

    mid = core["shots"].get("mid")
    lights_report = core.get("area_lights") or []
    if mid:
        report = mid[0]
        keys = {((report["ent"] & 0xffff) << 8) | 0x80 | t: t
                for t in range(report["tiles"][0] * report["tiles"][1])}
        found = {keys[l["key"]]: l for l in lights_report if l["key"] in keys}
        match = len(found) == len(keys) and all(
            numpy.allclose(found[t]["radiance"], report["lit"][t], rtol=0.02, atol=0.002)
            for t in found)
        add("area-light.tiles", match, "%d of %d tile lights published%s" % (
            len(found), len(keys), "" if match else ": %s" % {t: found[t]["radiance"] for t in found}))
    else:
        add("area-light.tiles", False, "no mid shot")
    return verdicts


def analyse(run):
    """A boot's parsed shots, burst and measurements."""
    shots = shot_frames(run["console"])
    images = run["screenshots"]
    parsed = {"shots": {}, "burst": [], "stats": stats(run["console"]),
              "area_lights": None, "sharp": None}
    for (label, report), path in zip(shots, images):
        image = linear(Image.open(path))
        if label == "burst":
            parsed["burst"].append((report, tile_means(image, report)))
        else:
            parsed["shots"][label] = (report, image)
    parsed["area_lights"] = area_lights_after(run["console"], "mid")
    close = parsed["shots"].get("close")
    if close:
        report, image = close
        h, w = image.shape[:2]
        a = to_screen(report, w, h, LABEL[0], LABEL[1])
        b = to_screen(report, w, h, LABEL[2], LABEL[3])
        widths = edge_widths(image, (min(a[0], b[0]), min(a[1], b[1]), max(a[0], b[0]), max(a[1], b[1])))
        parsed["sharp"] = float(numpy.median(widths)) if widths else None
        parsed["edges"] = len(widths)
    return parsed


def legacy_close(run, core_close_report):
    """The legacy run's close shot, measured in the core run's label region
    (the same camera; the legacy path prints no report)."""
    images = run["screenshots"]
    if not images or core_close_report is None:
        return {"sharp": None}
    image = linear(Image.open(images[-1]))
    h, w = image.shape[:2]
    a = to_screen(core_close_report, w, h, LABEL[0], LABEL[1])
    b = to_screen(core_close_report, w, h, LABEL[2], LABEL[3])
    widths = edge_widths(image, (min(a[0], b[0]), min(a[1], b[1]), max(a[0], b[0]), max(a[1], b[1])))
    return {"sharp": float(numpy.median(widths)) if widths else None, "edges": len(widths)}


def cmd_suite(args):
    out = args.out.resolve()
    out.mkdir(parents=True, exist_ok=True)
    checks = conformance_result.Checks()
    evidence = {"schema": SCHEMA, "started_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
                "client": sepipe_loader.boot_arguments(args), "runs": []}
    core_run = boot(args, "core", out / "core")
    legacy_run = boot(args, "legacy", out / "legacy")
    for run in (core_run, legacy_run):
        checks.check(run["returncode"] == 0, "boot.%s" % run["path"], run["boot_tail"][-300:])
    core = analyse(core_run)
    close_report = core["shots"].get("close", (None,))[0]
    legacy = legacy_close(legacy_run, close_report)
    verdicts = judge(core, legacy)
    for name, passed, detail in verdicts:
        checks.check(passed, name, detail)
        if args.verbose and passed:
            print("ok   %s: %s" % (name, detail))
    evidence["runs"] = [{k: v for k, v in run.items() if k != "console"} for run in (core_run, legacy_run)]
    evidence["verdicts"] = verdicts
    evidence["burst_frames"] = len(core["burst"])
    (out / "evidence.json").write_text(json.dumps(evidence, indent=2, default=str) + "\n")
    print("evidence: %s" % (out / "evidence.json"))
    return checks.report()


# ---------------------------------------------------------------------------
# Self-test: the judge on synthetic runs; each seeded defect is rejected on
# its own check.

def synthetic(defect=None):
    rng = numpy.random.default_rng(7)
    units = (400, 808)
    base = {"ent": 109, "units": units, "tiles": (2, 4), "quads": 20,
            "origin": [321, -255, 63], "right": [94, 0, 0], "down": [0, 0, -190],
            "screen": [-0.2, 0.2, 0.2, 0.2, 0.2, -0.9, -0.2, -0.9]}
    burst = []
    states = [(0.023, 127), (0.008, 63), (0.008, 63), (0.023, 127)] * 5
    for i, (light, alpha) in enumerate(states):
        lit = numpy.full((8, 3), light) * (1 + 0.1 * rng.standard_normal((8, 1)))
        without = lit * 1.2
        if defect == "grime-adds-light":
            without = lit * 0.8
        textures = [("vgui/screens/vgui_coop_progress_board", 1, 255)]
        if not (defect == "dirt-missing" and i == 3):
            textures.append(("vgui/elevator_video_overlay2", 1, alpha))
        report = dict(base, frame=300 + i, res=(238, 481), textures=textures, lit=lit, without=without)
        image = lit + 0.003
        if defect == "stale-frame" and i in (4, 9, 14):
            image = burst[-1][1] if burst else image
        burst.append((report, image))
    # The close shot: 400 units over 1024 * 0.8 = 819.2 pixels, 2.048 pixels
    # per unit; the image 1.19 texels per pixel (0.8 when seeded low).
    close_report = dict(base, frame=500, textures=[], lit=numpy.zeros((8, 3)),
                        without=numpy.zeros((8, 3)),
                        screen=[-0.8, 0.9, 0.8, 0.9, 0.8, -0.9, -0.8, -0.9])
    density = 2.048 * (0.8 if defect == "low-resolution" else 1.19)
    close_report["res"] = (int(400 * density), int(808 * density))
    close_image = numpy.zeros((HEIGHT, WIDTH, 3))
    mid_report = dict(base, frame=600, res=(238, 481), textures=[], lit=numpy.full((8, 3), 0.06),
                      without=numpy.full((8, 3), 0.07))
    lights = [{"key": ((109 << 8) | 0x80 | t), "radiance": list(mid_report["lit"][t])} for t in range(8)]
    if defect == "no-tile-lights":
        lights = lights[:1]
    s = {"lists": 40, "refused": 0, "rasterized": 40, "queued": 40, "drawn": 150,
         "failed": 1 if defect == "core-failed" else 0, "panels": 150}
    core = {"stats": s, "burst": burst, "shots": {"close": (close_report, close_image), "mid": (mid_report, None)},
            "area_lights": lights, "sharp": 2.4 if defect == "blurred" else 1.0}
    legacy = {"sharp": 1.1 if defect == "legacy-sharp" else 2.5}
    return core, legacy


def cmd_selftest(args):
    checks = conformance_result.Checks()
    verdicts = judge(*synthetic())
    for name, passed, detail in verdicts:
        checks.check(passed, "selftest.good." + name, detail)
    seeded = {"core-failed": "core.takes-sign", "low-resolution": "resolution.texel-per-pixel",
              "blurred": "sharp.core", "legacy-sharp": "sharp.legacy-blurred",
              "stale-frame": "flicker.one-state", "dirt-missing": "dirt.every-state",
              "grime-adds-light": "dirt.dims-cast-light", "no-tile-lights": "area-light.tiles"}
    for defect, check in seeded.items():
        failed = {name for name, passed, _ in judge(*synthetic(defect)) if not passed}
        checks.check(check in failed, "selftest.rejects.%s" % defect,
                     "the judge accepted %s (failed: %s)" % (defect, sorted(failed)))
    return checks.report()


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    sub = parser.add_subparsers(dest="command", required=True)
    suite = sub.add_parser("suite")
    suite.add_argument("--out", type=Path, required=True)
    sepipe_loader.add_arguments(suite, "portal2")
    suite.add_argument("--timeout", type=int, default=900)
    suite.add_argument("--verbose", action="store_true")
    sub.add_parser("selftest")
    args = parser.parse_args(argv)
    if args.command == "suite":
        return cmd_suite(args)
    return cmd_selftest(args)


if __name__ == "__main__":
    sys.exit(main())
