#!/usr/bin/env python3
"""Capture and label the render core's lighting layers in the installed game.

    python3 tools/render/game_layer_dump.py

The default is the published sp_a2_laser_intro_source2 cache at three fixed
camera poses. --camera name:x,y,z:pitch,yaw,roll selects other poses.

The game runs through portal_boot in a private runtime. Each camera gets a
beauty frame, the core-only frame, installed pixel debug views, and the same
frame with one lighting term disabled at a time. The contact sheets retain
the actual game pixels; term difference images show the screen-space change
from disabling that term and are display aids, not linear-light contributions.
"""

import argparse
import datetime
import json
import math
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

from PIL import Image, ImageChops, ImageDraw, ImageEnhance, ImageStat


ROOT = Path(__file__).resolve().parents[2]
PORTAL_BOOT = ROOT / "tools" / "quality" / "portal_boot.py"
sys.path.insert(0, str(ROOT / "tools" / "kiln"))
import sepipe_loader  # noqa: E402
DEFAULT_MAP = "sp_a2_laser_intro_source2"
DEFAULT_CAMERAS = (
    "chamber:-440,0,-100:0,0,0",
    "laser:-160,96,-100:0,0,0",
    "lift:320,0,-100:0,180,0",
)
VIEWS = (
    ("albedo", 1), ("world-normal", 2), ("normal-map", 3),
    ("roughness", 4), ("metalness", 6), ("ao", 7),
    ("baked", 8), ("direct", 9), ("image-specular", 10),
    ("ssr", 11), ("emissive", 12), ("invalid-values", 16),
    ("over-range", 17),
)
TERMS = ("baked", "clustered", "sun", "area", "projected", "probes", "ibl",
         "ssr", "ao", "emission", "volumetric")
CONTENT_EXTENSIONS = {"maps": {".bsp"}, "materials": {".vtf", ".vmt"},
                      "models": {".mdl", ".vvd", ".vtx", ".phy", ".ani"}}


def camera_arg(value):
    parts = value.split(":")
    if len(parts) != 3 or not re.fullmatch(r"[a-z][a-z0-9_-]*", parts[0]):
        raise argparse.ArgumentTypeError("camera must be name:x,y,z:pitch,yaw,roll")
    try:
        position, angles = ([float(number) for number in triplet.split(",")]
                            for triplet in parts[1:])
    except ValueError as error:
        raise argparse.ArgumentTypeError("camera coordinates must be numbers") from error
    if len(position) != 3 or len(angles) != 3:
        raise argparse.ArgumentTypeError("camera needs three position and three angle values")
    if not all(math.isfinite(number) for number in position + angles):
        raise argparse.ArgumentTypeError("camera coordinates must be finite")
    return parts[0], position, angles


def shots_for(camera):
    name, position, angles = camera
    prefix = name + "/"
    pose = ["cmd setpos " + " ".join("%g" % value for value in position),
            "cmd setang " + " ".join("%g" % value for value in angles)]
    shots = [(prefix + "beauty", pose + ["cl_render_debug_view 0",
                                       "cl_render_debug_term ,",
                                       "cl_render_debug_legacy 0"])]
    shots += [(prefix + "view-" + label,
               ["cl_render_debug_view %d" % index]) for label, index in VIEWS]
    shots.append((prefix + "core", ["cl_render_debug_view 0", "cl_render_debug_term ,",
                                     "cl_render_debug_legacy 2"]))
    shots += [(prefix + "off-" + term,
               ["cl_render_debug_view 0", "cl_render_debug_legacy 2",
                "cl_render_debug_term " + term]) for term in TERMS]
    shots.append((prefix + "core-repeat", ["cl_render_debug_term ,"]))
    return shots


def console_script(shots, settle, view_scale):
    lines = ["cl_drawhud 0", "con_drawnotify 0", "hud_quickinfo 0",
             "cl_render_debug_view_scale %g" % view_scale]
    # `exec` runs a cfg's lines at once; an alias per shot lets `wait` delay
    # the next alias until the screenshot has actually reached a frame.
    for index, (name, commands) in enumerate(shots):
        steps = commands + ["wait %d" % settle, "echo LAYER_SHOT " + name,
                            "screenshot", "wait 8"]
        steps.append("ld_shot%d" % (index + 1) if index + 1 < len(shots)
                     else "echo LAYER_DUMP_DONE")
        lines.append('alias ld_shot%d "%s"' % (index, "; ".join(steps)))
    lines.append("wait 120; ld_shot0")
    return lines


def sheet(images, labels, path, columns=3):
    thumb_width = 512
    thumb_height = round(images[0].height * thumb_width / images[0].width)
    tile_height = thumb_height + 28
    rows = (len(images) + columns - 1) // columns
    result = Image.new("RGB", (thumb_width * columns, tile_height * rows), (28, 28, 28))
    draw = ImageDraw.Draw(result)
    for index, (frame, label) in enumerate(zip(images, labels)):
        x = index % columns * thumb_width
        y = index // columns * tile_height
        result.paste(frame.resize((thumb_width, thumb_height)), (x, y + 28))
        draw.text((x + 8, y + 7), label, fill="white")
    result.save(path)


def content_overlay(source, out):
    """Give portal_boot just installable game assets from a working build."""
    overlay = out / "content-overlay"
    skipped = []
    count = 0
    for item in sorted(source.rglob("*")):
        if not item.is_file():
            continue
        relative = item.relative_to(source)
        if item.suffix.lower() not in CONTENT_EXTENSIONS.get(relative.parts[0], set()):
            skipped.append(str(relative))
            continue
        target = overlay / relative
        target.parent.mkdir(parents=True, exist_ok=True)
        try:
            os.link(item.resolve(), target)
        except OSError:
            shutil.copy2(item, target)
        count += 1
    if not any((overlay / "maps").glob("*.bsp")):
        raise ValueError("content root has no installable BSP")
    (out / "content-overlay.json").write_text(json.dumps({
        "source": str(source), "installable_files": count, "skipped": skipped
    }, indent=2) + "\n")
    return overlay


def assemble(boot, out, shots, cameras, map_name, game, view_scale):
    console = boot / "runtime" / game / "console.log"
    log = console.read_text(errors="replace") if console.exists() else ""
    if "LAYER_DUMP_DONE" not in log:
        raise RuntimeError("game did not finish the layer script; see " + str(console))
    for name, _ in shots:
        if "LAYER_SHOT " + name not in log:
            raise RuntimeError("game missed layer " + name)
    screenshots = sorted((boot / "runtime" / game / "screenshots").glob("*.tga"))
    if len(screenshots) != len(shots) + 1:
        raise RuntimeError("expected %d scripted shots and the boot's final shot, got %d" %
                           (len(shots), len(screenshots)))
    frames = {}
    for (name, _), source in zip(shots, screenshots):
        camera, label = name.split("/", 1)
        target = out / camera / (label + ".png")
        target.parent.mkdir(parents=True, exist_ok=True)
        frame = Image.open(source).convert("RGB")
        frame.save(target)
        frames[name] = frame
    for camera, _, _ in cameras:
        prefix = camera + "/"
        view_labels = ["beauty", "core"] + ["view-" + label for label, _ in VIEWS]
        sheet([frames[prefix + label] for label in view_labels], view_labels,
              out / camera / "views.png")
        core = frames[prefix + "core"]
        repeat = ImageChops.difference(core, frames[prefix + "core-repeat"])
        drift_max = max(channel[1] for channel in repeat.getextrema())
        metrics = {"core_repeat_max_channel_difference": drift_max,
                   "stable": drift_max <= 2, "terms": {}}
        if not metrics["stable"]:
            print("warning: %s changed between core captures; term differences may include "
                  "animation or exposure drift" % camera, file=sys.stderr)
        off_labels = ["core"] + ["off-" + term for term in TERMS] + ["core-repeat"]
        sheet([frames[prefix + label] for label in off_labels], off_labels,
              out / camera / "terms-off.png")
        differences = []
        for term in TERMS:
            difference = ImageChops.difference(core, frames[prefix + "off-" + term])
            metrics["terms"][term] = {
                "mean_channel_difference": sum(ImageStat.Stat(difference).mean) / 3.0,
                "max_channel_difference": max(channel[1] for channel in difference.getextrema())
            }
            differences.append(ImageEnhance.Brightness(difference).enhance(4.0))
        sheet(differences, ["change: " + term + " (4x)" for term in TERMS],
              out / camera / "term-changes.png")
        (out / camera / "metrics.json").write_text(json.dumps(metrics, indent=2) + "\n")
    (out / "shots.json").write_text(json.dumps({
        "schema": "game-layer-dump/v1", "map": map_name,
        "cameras": [{"name": name, "position": position, "angles": angles}
                    for name, position, angles in cameras],
        "view_scale": view_scale,
        "shots": [name for name, _ in shots],
        "note": "Term changes are absolute display-space differences at 4x brightness."
    }, indent=2) + "\n")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--map", default=DEFAULT_MAP)
    parser.add_argument("--content-root", type=Path,
                        help="defaults to run/maps/<map>/published.json's content_root")
    sepipe_loader.add_arguments(parser, "portal2")
    parser.add_argument("--camera", action="append", type=camera_arg,
                        help="repeat for each pose; laser intro defaults to three poses")
    parser.add_argument("--out", type=Path,
                        help="defaults to a timestamped quality-results/game-layer-dump run")
    parser.add_argument("--settle", type=int, default=24)
    parser.add_argument("--view-scale", type=float, default=8.0,
                        help="in-engine radiometric debug-view gain (default 8)")
    parser.add_argument("--timeout", type=int, default=900)
    args = parser.parse_args()
    if not args.camera:
        if args.map != DEFAULT_MAP:
            parser.error("--camera is required for maps other than " + DEFAULT_MAP)
        args.camera = [camera_arg(value) for value in DEFAULT_CAMERAS]
    if not args.content_root:
        published = ROOT / "run" / "maps" / args.map / "published.json"
        if not published.is_file():
            parser.error("--content-root is required: no published map at " + str(published))
        args.content_root = Path(json.loads(published.read_text())["content_root"])
    if not math.isfinite(args.view_scale) or args.view_scale <= 0:
        parser.error("--view-scale must be finite and positive")
    if args.settle < 1 or args.timeout < 1:
        parser.error("--settle and --timeout must be positive")
    if len({camera[0] for camera in args.camera}) != len(args.camera):
        parser.error("camera names must be unique")
    shots = [shot for camera in args.camera for shot in shots_for(camera)]
    out = (args.out or ROOT / "quality-results" / "game-layer-dump" /
           datetime.datetime.now().strftime("%Y%m%d-%H%M%S")).resolve()
    out.mkdir(parents=True, exist_ok=True)
    boot = out / "boot"
    if boot.exists():
        parser.error("output boot already exists: " + str(boot))
    content = content_overlay(args.content_root.resolve(), out)
    command = [sys.executable, str(PORTAL_BOOT), *sepipe_loader.boot_arguments(args),
               "--content-root", str(content),
               "--renderer", "native-vulkan", "--require-vulkan", "--headless",
               "--map", args.map, "--out", str(boot), "--timeout", str(args.timeout),
               "--capture-wait", str(120 + len(shots) * (args.settle + 8) + 80),
               "--startup-command", "sv_cheats 1",
               "--startup-command", "r_core_world 1"]
    for line in console_script(shots, args.settle, args.view_scale):
        command += ["--console-command", line]
    completed = subprocess.run(command, text=True, capture_output=True)
    (out / "boot.log").write_text(completed.stdout + completed.stderr)
    if completed.returncode:
        raise SystemExit("game boot failed; see " + str(out / "boot.log"))
    assemble(boot, out, shots, args.camera, args.map, args.game, args.view_scale)
    print("Layer images and contact sheets: " + str(out))


if __name__ == "__main__":
    main()
