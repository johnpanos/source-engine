#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Capture two Portal 2 maps at one Source-unit camera and make a swipe comparison.

Default comparison: sp_a1_intro4 / sp_a1_intro4_relit at 3840x2160.
The new (right) game selects FSR Native AA with MSAA disabled and requests HDR
display output. HDR presentation requires a capable compositor/display; the
private headless compositor and RGB PNG comparison do not certify HDR output.
The original (left) game uses its separate DXVK Native build and legacy rendering.

Example (chamber panel view):
  python3 tools/render/map_swipe_compare.py --pose=-1552,37,-32:0,-90,0 \
      --out quality-results/map-comparisons/intro4

All retained Intro4 comparison and survey poses:
  python3 tools/render/map_swipe_compare.py --all-captures \
      --out quality-results/map-comparisons/intro4-4k-dxvk-fsr

Published maps in run/maps/<name>/published.json are mounted automatically.
The original retail map is loaded from the staged Portal 2 runtime. Each map
gets a separate portal_boot run; the view oracle must confirm the requested
camera before the HTML is written.
"""

import argparse
import datetime
import html
import json
import math
import os
from pathlib import Path
import re
import subprocess
import sys

from PIL import Image, ImageChops, ImageEnhance, ImageStat


ROOT = Path(__file__).resolve().parents[2]
BOOT = ROOT / "tools/quality/portal_boot.py"
# Retained comparison and survey cameras, deduplicated. Survey player origins
# include the 64-unit standing eye offset here.
INTRO4_CAPTURES = (
    ("panel", "Chamber panel", "-1552,37,-32:0,-90,0"),
    ("chamber-overview", "Chamber overview", "-300,100,128:0,0,0"),
    ("chamber", "Damaged chamber", "-420,70,112:-6,5,0"),
    ("lightboard", "Illuminated lightboard", "-900,180,72:0,310,0"),
    ("second-lightboard", "Second 04 room", "420,180,100:0,-15,0"),
    ("panorama", "Chamber panorama", "-360,40,124:-4,10,0"),
    ("look-up", "Chamber upward view", "-300,100,128:-6,-8,0"),
    ("oblique", "Chamber oblique view", "-450,130,112:-3,12,0"),
    ("arrival", "Chamber arrival", "-1100,176,96:-3,0,0"),
    ("instance-front", "Lightboard front", "-780,150,72:0,270,0"),
    ("instance-right", "Lightboard right", "-600,160,72:0,230,0"),
    ("instance-low", "Lightboard low view", "-800,60,40:-5,300,0"),
    ("main-front", "Second lightboard front", "450,79,100:0,0,0"),
    ("cool-front", "Cool panel front", "-1200,176,96:-20,0,0"),
    ("cool-side", "Cool panel side", "-1100,0,96:-24,55,0"),
    ("cool-exit", "Cool panel exit", "-960,176,96:-25,180,0"),
    ("warm-front", "Warm panel front", "1250,-528,180:-20,0,0"),
    ("warm-side", "Warm panel side", "1450,-700,180:-25,45,0"),
    ("warm-back", "Warm panel back", "1800,-528,180:-20,180,0"),
)
sys.path.insert(0, str(ROOT / "tools/quality"))
import private_session


def parse_pose(value):
    try:
        position, angles = ([float(part) for part in group.split(",")]
                            for group in value.split(":"))
    except ValueError as error:
        raise argparse.ArgumentTypeError("pose must be x,y,z:pitch,yaw,roll") from error
    if len(position) != 3 or len(angles) != 3 or not all(
            math.isfinite(number) for number in position + angles):
        raise argparse.ArgumentTypeError("pose must have six finite numbers")
    return position, angles


def published_content(map_name):
    receipt = ROOT / "run/maps" / map_name / "published.json"
    if not receipt.is_file():
        return None
    content = Path(json.loads(receipt.read_text())["content_root"])
    if not (content / "maps" / (map_name + ".bsp")).is_file():
        raise ValueError("published content lacks %s.bsp: %s" % (map_name, content))
    return content


def oracle_camera(path):
    lines = Path(path).read_text().splitlines()
    if not lines or json.loads(lines[0]).get("schema") != "source-view-oracle/v1":
        raise ValueError("missing source view oracle: " + str(path))
    for line in lines[1:]:
        event = json.loads(line)
        label = event.get("name", "")
        if event.get("event") != "label_begin" or not label.startswith("vieworacle "):
            continue
        view = json.loads(label[len("vieworacle "):])
        if view.get("type") == "3d" and view.get("stack") == 1 and \
                view.get("target") == "backbuffer":
            return view
    raise ValueError("no top-level world camera in " + str(path))


def verify_camera(view, position, angles, width, height):
    if view.get("viewport") != [0, 0, width, height]:
        raise ValueError("capture viewport differs from requested dimensions")
    if max(abs(float(a) - b) for a, b in zip(view["origin"], position)) > 0.25:
        raise ValueError("capture position differs from request: " + str(view["origin"]))
    if max(abs((float(a) - b + 180) % 360 - 180)
           for a, b in zip(view["angles"], angles)) > 0.25:
        raise ValueError("capture angles differ from request: " + str(view["angles"]))


def capture(map_name, side, content, args, out, startup_commands=(), engine_args=(),
            renderer="native-vulkan", build=None, shader_artifacts=None):
    position, angles = args.pose
    boot = out / (side + "-boot")
    command = [sys.executable, str(BOOT), "--runtime", str(args.runtime),
               "--build", str(build or args.build), "--game", "portal2",
               "--renderer", renderer, "--require-vulkan", "--require-wayland",
               "--view-oracle", "--map", map_name, "--out", str(boot),
               "--width", str(args.width), "--height", str(args.height),
               "--timeout", str(args.timeout), "--capture-wait", "120",
               "--engine-arg=-deterministicrender", "--engine-arg=-nosound",
               "--engine-arg=-noborder",
               "--startup-command", "host_framerate 0.015",
               "--startup-command", "mat_picmip -1",
               "--startup-command", "r_lod 0",
               "--startup-command", "mat_forceaniso 16",
               "--startup-command", "mat_trilinear 1",
               "--startup-command", "mat_antialias 4",
               "--startup-command", "mat_colorcorrection 1",
               "--startup-command", "r_core_world 1",
               "--console-command", "noclip",
               "--console-command", "cl_drawhud 0",
               "--console-command", "r_drawviewmodel 0",
               "--console-command", "con_drawnotify 0",
               "--console-command", "hud_quickinfo 0"]
    for setting in startup_commands:
        command += ["--startup-command", setting]
    for argument in engine_args:
        command += ["--engine-arg=" + argument]
    if shader_artifacts:
        command += ["--shader-artifacts", str(shader_artifacts)]
    if renderer == "vulkan-compat":
        command += ["--render-trace"]
    for relay in args.panel_relay:
        command += ["--console-command", "ent_fire " + relay + " Trigger"]
    command += [
               # setpos names the player origin. The standing Portal 2 eye is 64 units above it.
               "--console-command", "cmd setpos " + " ".join(
                   "%.6f" % n for n in (position[0], position[1], position[2] - 64)),
               "--console-command", "cmd setang " + " ".join("%.6f" % n for n in angles)]
    if content:
        command += ["--content-root", str(content)]
    result = subprocess.run(command, capture_output=True, text=True)
    (out / (side + "-boot.log")).write_text(result.stdout + result.stderr)
    evidence_path = boot / "evidence.json"
    if not evidence_path.is_file():
        raise ValueError("no boot evidence for %s; see %s" % (map_name, out / (side + "-boot.log")))
    evidence = json.loads(evidence_path.read_text())
    if result.returncode or evidence.get("status") != "pass":
        raise ValueError("%s capture failed: %s (see %s)" %
                         (map_name, evidence.get("failures"), out / (side + "-boot.log")))
    shots = evidence.get("screenshots", [])
    oracles = evidence.get("view_oracle_captures", [])
    if len(shots) != 1 or len(oracles) != 1:
        raise ValueError("%s needs exactly one screenshot and one view oracle" % map_name)
    view = oracle_camera(oracles[0]["path"])
    verify_camera(view, position, angles, args.width, args.height)
    source = Path(shots[0]["path"])
    image = Image.open(source).convert("RGB")
    if image.size != (args.width, args.height):
        raise ValueError("%s screenshot has wrong dimensions" % map_name)
    target = out / (side + ".png")
    image.save(target)
    render_confirmation = None
    if "-fsr" in engine_args:
        log = (boot / "stdout.log").read_text()
        dispatch = "FSR game: %dx%d -> %dx%d, before post/HUD" % (
            args.width, args.height, args.width, args.height)
        if dispatch not in log:
            raise ValueError("FSR Native AA dispatch was not confirmed in " + str(boot))
        hdr = re.findall(r"\[NativeVulkan\] (HDR output: [^\n]+|extended output declined: [^\n]+)", log)
        render_confirmation = {"fsr_dispatch": dispatch,
                               "hdr_presentation_log": hdr[-1] if hdr else "unconfirmed"}
    return {"map": map_name, "content_root": str(content) if content else None,
            "renderer": renderer, "build": str(build or args.build),
            "image": target.name, "boot_evidence": str(evidence_path),
            "startup_commands": list(startup_commands),
            "engine_args": list(engine_args),
            "render_confirmation": render_confirmation,
            "camera": {key: view[key] for key in ("origin", "angles", "fov", "viewport")}}


def make_html(out, left, right, metrics, position, angles):
    title = html.escape(left["map"] + " / " + right["map"])
    label_a, label_b = html.escape(left["map"]), html.escape(right["map"])
    pose = html.escape("position %s · angles %s" % (position, angles))
    page = """<!doctype html>
<html lang="en"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>MAP_TITLE</title>
<style>
  :root { color-scheme: dark; font: 15px system-ui, sans-serif; background: #141719; color: #e9eef0; }
  body { max-width: 1320px; margin: 0 auto; padding: 24px; }
  h1 { font-size: 1.3rem; margin: 0 0 8px; }
  p { color: #b9c6ca; margin: 8px 0 18px; }
  .comparison { position: relative; width: 100%; aspect-ratio: WIDTH / HEIGHT; overflow: hidden;
                cursor: ew-resize; touch-action: none; background: #000; user-select: none; }
  .comparison img { display: block; position: absolute; width: 100%; height: 100%; object-fit: fill; }
  #top { clip-path: inset(0 50% 0 0); }
  #line { position: absolute; top: 0; bottom: 0; left: 50%; border-left: 2px solid white;
          pointer-events: none; filter: drop-shadow(0 0 3px black); }
  #line::after { content: "↔"; position: absolute; top: 50%; left: -19px; width: 36px;
                 height: 36px; border-radius: 50%; background: white; color: #15191b;
                 display: grid; place-items: center; font-size: 22px; transform: translateY(-50%); }
  .labels { display: flex; justify-content: space-between; gap: 16px; margin: 12px 0; }
  .controls { display: flex; align-items: center; gap: 14px; flex-wrap: wrap; }
  input[type=range] { flex: 1; min-width: 180px; }
  button { color: inherit; background: #303b40; border: 1px solid #6a7a80; padding: 8px 12px; cursor: pointer; }
  #diff { display: none; width: 100%; margin-top: 18px; }
  #diff.visible { display: block; }
</style>
<h1>MAP_TITLE</h1><p>POSE · WIDTH×HEIGHT · LEFT_RENDERER / RIGHT_RENDERER.
  Drag the divider or use the slider.
  Mean absolute RGB difference: MEAN / 255.</p>
<p>Right game: FSR Native AA (100%), MSAA off, HDR output requested.
  These RGB PNGs do not verify HDR display presentation.</p>
<div class="comparison" id="comparison" aria-label="Image comparison">
  <img src="b.png" alt="RIGHT_LABEL"><img id="top" src="a.png" alt="LEFT_LABEL"><div id="line"></div>
</div>
<div class="labels"><strong>LEFT_LABEL</strong><strong>RIGHT_LABEL</strong></div>
<div class="controls"><label for="slider">Divider</label><input id="slider" type="range" min="0" max="100" value="50">
  <button id="showdiff" type="button">Show absolute difference (4×)</button></div>
<img id="diff" src="difference-4x.png" alt="Absolute RGB difference, brightened four times">
<script>
  const box = document.getElementById('comparison');
  const slider = document.getElementById('slider');
  const topImage = document.getElementById('top');
  const line = document.getElementById('line');
  function setDivider(value) {
    value = Math.max(0, Math.min(100, Number(value)));
    slider.value = value;
    topImage.style.clipPath = `inset(0 ${100 - value}% 0 0)`;
    line.style.left = `${value}%`;
  }
  slider.addEventListener('input', () => setDivider(slider.value));
  function move(event) {
    const rect = box.getBoundingClientRect();
    setDivider(100 * (event.clientX - rect.left) / rect.width);
  }
  box.addEventListener('pointerdown', event => { box.setPointerCapture(event.pointerId); move(event); });
  box.addEventListener('pointermove', event => { if (box.hasPointerCapture(event.pointerId)) move(event); });
  document.getElementById('showdiff').addEventListener('click', event => {
    const diff = document.getElementById('diff');
    diff.classList.toggle('visible');
    event.currentTarget.textContent = diff.classList.contains('visible') ? 'Hide absolute difference' :
      'Show absolute difference (4×)';
  });
</script></html>
"""
    for marker, value in {"MAP_TITLE": title, "LEFT_LABEL": label_a,
                          "RIGHT_LABEL": label_b, "POSE": pose,
                          "LEFT_RENDERER": html.escape(left.get("renderer", "native-vulkan")),
                          "RIGHT_RENDERER": html.escape(right.get("renderer", "native-vulkan")),
                          "WIDTH": str(metrics["size"][0]), "HEIGHT": str(metrics["size"][1]),
                          "MEAN": "%.2f" % metrics["mean_absolute_rgb"]}.items():
        page = page.replace(marker, value)
    (out / "compare.html").write_text(page)


def capture_gallery(argv, args, parser):
    if (args.map_a, args.map_b) != ("sp_a1_intro4", "sp_a1_intro4_relit"):
        parser.error("--all-captures uses the Intro4 gallery cameras and map pair")
    out = args.out.resolve()
    if out.exists() and any(out.iterdir()):
        parser.error("output directory is not empty: " + str(out))
    out.mkdir(parents=True, exist_ok=True)
    # Preserve the caller's build, assets, resolution and content overrides.
    shared = []
    arguments = iter(argv)
    for argument in arguments:
        if argument == "--all-captures":
            continue
        if argument == "--out":
            next(arguments)
            continue
        if argument.startswith("--out="):
            continue
        shared.append(argument)
    cards = []
    for name, label, pose in INTRO4_CAPTURES:
        print("Capturing Intro4: " + label, flush=True)
        result = main(shared + ["--pose=" + pose, "--out", str(out / name),
                                "--panel-relay", "info_sign-info_panel_activate_rl",
                                "--panel-relay", "InstanceAuto63-info_panel_activate_rl"])
        if result:
            return result
        cards.append('<a href="%s/compare.html"><img src="%s/b.png" alt="%s">%s</a>' %
                     (name, name, html.escape(label), html.escape(label)))
    write_gallery(out, cards, args.width, args.height)
    print(out / "index.html")
    return 0


def write_gallery(out, cards, width, height):
    page = '''<!doctype html><html lang="en"><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Intro4 map comparisons</title><style>
:root { color-scheme: dark; font: 16px system-ui; }
body { max-width: 1320px; margin: auto; padding: 24px; }
.grid { display: grid; grid-template-columns: repeat(auto-fit,minmax(300px,1fr)); gap: 20px; }
a { color: inherit; } img { display: block; width: 100%%; margin-bottom: 8px; }
</style><h1>Intro4 map comparisons</h1>
<p>sp_a1_intro4 / sp_a1_intro4_relit · %d×%d · right game: FSR Native AA,
MSAA off, HDR output requested. RGB PNGs do not verify HDR display presentation.</p>
<p>Original: DXVK Native legacy renderer. Relit: native Vulkan render core.</p>
<div class="grid">%s</div></html>''' % (width, height, "".join(cards))
    (out / "index.html").write_text(page)


def main(argv=None):
    argv = list(argv if argv is not None else sys.argv[1:])
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("map_a", nargs="?", default="sp_a1_intro4")
    parser.add_argument("map_b", nargs="?", default="sp_a1_intro4_relit")
    capture_selection = parser.add_mutually_exclusive_group(required=True)
    capture_selection.add_argument("--pose", type=parse_pose,
                                   help="shared Source-unit camera: x,y,z:pitch,yaw,roll")
    capture_selection.add_argument("--all-captures", action="store_true",
                                   help="capture all 19 retained Intro4 comparison/survey poses")
    parser.add_argument("--content-root-a", type=Path)
    parser.add_argument("--content-root-b", type=Path)
    parser.add_argument("--panel-relay", action="append", default=[],
                        help="activate a named test chamber panel relay on both maps; repeatable")
    parser.add_argument("--runtime", type=Path, default=ROOT / "run/runtime-p2-fsr")
    parser.add_argument("--build", type=Path, default=ROOT / "build-p2-fsr")
    parser.add_argument("--build-a", type=Path, default=ROOT / "build-p2-dxvk",
                        help="original game's separate Portal 2 DXVK Native build")
    parser.add_argument("--shader-artifacts-a", type=Path,
                        default=ROOT / "build-p2-dxvk/shaders/intro4",
                        help="source-matched D3D9 shader pack for the original game")
    parser.add_argument("--fsr-assets", type=Path, default=ROOT / "external/fsr411/assets")
    parser.add_argument("--width", type=int, default=3840)
    parser.add_argument("--height", type=int, default=2160)
    parser.add_argument("--timeout", type=int, default=900)
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--in-compositor", action="store_true", help=argparse.SUPPRESS)
    args = parser.parse_args(argv)
    if args.map_a == args.map_b:
        parser.error("choose two different maps")
    if args.timeout < 1:
        parser.error("timeout must be positive")
    if not (64 <= args.width <= 8192 and 64 <= args.height <= 8192):
        parser.error("capture dimensions must be between 64 and 8192")
    cache = args.build / "c4che/_cache.py"
    if not cache.is_file() or not re.search(r"^RENDER_FSR411 = True$", cache.read_text(), re.M):
        parser.error("--build must name an FSR-capable Portal 2 build (./play_p2_fsr)")
    if not any(re.search(r"^GAMES = ['\"]portal2['\"]$", path.read_text(), re.M)
               for path in (args.build / "c4che").glob("*_cache.py")):
        parser.error("--build must be configured for Portal 2")
    if not args.fsr_assets.is_dir():
        parser.error("FSR assets are missing: " + str(args.fsr_assets))
    original_caches = list((args.build_a / "c4che").glob("*_cache.py"))
    if not any(re.search(r"^DXVK = True$", path.read_text(), re.M)
               for path in original_caches):
        parser.error("--build-a must name a DXVK Native build")
    if not any(re.search(r"^GAMES = ['\"]portal2['\"]$", path.read_text(), re.M)
               for path in original_caches):
        parser.error("--build-a must be configured for Portal 2")
    if any(not re.fullmatch(r"[A-Za-z0-9_@-]+", name) for name in args.panel_relay):
        parser.error("panel relay names may contain only letters, digits, _, @, and -")
    if args.all_captures:
        return capture_gallery(argv, args, parser)
    out = args.out.resolve()
    if not args.in_compositor:
        out.mkdir(parents=True, exist_ok=True)
        environment = dict(os.environ)
        environment["SDL_VIDEODRIVER"] = "wayland"
        environment["SDL_VIDEO_DRIVER"] = "wayland"
        for name in ("DISPLAY", "WAYLAND_DISPLAY"):
            environment.pop(name, None)
        command = private_session.dbus_run_session(out / "dbus") + [
            "mutter", "--headless", "--wayland", "--virtual-monitor",
            "%dx%d@60" % (args.width, args.height),
            "--wayland-display", "map-swipe-%d" % os.getpid(), "--",
            sys.executable, str(Path(__file__).resolve()), *(argv if argv is not None else sys.argv[1:]),
            "--in-compositor"]
        with (out / "compositor.log").open("w") as log:
            result = subprocess.run(command, env=environment, stdout=log,
                                    stderr=subprocess.STDOUT, timeout=args.timeout * 2 + 120)
        if result.returncode:
            print("map_swipe_compare: compositor run failed; see " + str(out / "compositor.log"),
                  file=sys.stderr)
        elif (out / "compare.html").is_file():
            print(out / "compare.html")
        return result.returncode
    if out.exists() and any(out.iterdir()):
        allowed = {"dbus", "compositor.log"}
        if any(item.name not in allowed for item in out.iterdir()):
            parser.error("output directory is not empty: " + str(out))
    out.mkdir(parents=True, exist_ok=True)
    try:
        content_a = args.content_root_a or published_content(args.map_a)
        content_b = args.content_root_b or published_content(args.map_b)
        left = capture(args.map_a, "a", content_a, args, out,
                       ("r_temporal_scale 0", "r_core_world 0"), ("-norendercore",),
                       renderer="vulkan-compat", build=args.build_a,
                       shader_artifacts=args.shader_artifacts_a)
        right = capture(args.map_b, "b", content_b, args, out,
                        ("mat_antialias 0", "r_temporal_scale 1", "mat_hdr_output 1"),
                        ("-fsr", "-fsr-assets", str(args.fsr_assets.resolve())))
        if abs(left["camera"]["fov"] - right["camera"]["fov"]) > 0.1:
            raise ValueError("captured camera FOV differs between maps")
        a, b = Image.open(out / "a.png"), Image.open(out / "b.png")
        difference = ImageChops.difference(a, b)
        ImageEnhance.Brightness(difference).enhance(4).save(out / "difference-4x.png")
        metrics = {"size": list(a.size),
                   "mean_absolute_rgb": sum(ImageStat.Stat(difference).mean) / 3}
        position, angles = args.pose
        receipt = {"schema": "map-swipe-comparison/v1", "left": left, "right": right,
                   "requested_camera": {"origin": position, "angles": angles},
                   "activated_panel_relays": args.panel_relay,
                   "metrics": metrics,
                   "captured_utc": datetime.datetime.now(datetime.timezone.utc).isoformat()}
        (out / "comparison.json").write_text(json.dumps(receipt, indent=2) + "\n")
        make_html(out, left, right, metrics, position, angles)
        print(out / "compare.html")
        return 0
    except (OSError, ValueError, KeyError, subprocess.SubprocessError) as error:
        print("map_swipe_compare: " + str(error), file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
