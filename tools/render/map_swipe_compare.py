#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Capture two Portal 2 maps at one Source-unit camera and make a swipe comparison.

Default comparison: sp_a1_intro4 / sp_a1_intro4_relit at 7680x4320.
The new (right) game selects native 8K with FSR off and 4x MSAA, and requests HDR
display output with exposure 3 and a 10000-nit peak. B also exports the unexposed
linear scene as a floating-point PFM, with colour-space metadata for offline conversion.
HDR presentation requires a capable compositor/display;
the private headless compositor and RGB PNG comparison do not certify HDR output.
The original (left) game uses native Vulkan with the render core and FSR disabled.

Example (chamber panel view):
  python3 tools/render/map_swipe_compare.py --pose=-1552,37,-32:0,-90,0 \
      --out quality-results/map-comparisons/intro4

All retained Intro4 comparison and survey poses except the panel close-up:
  python3 tools/render/map_swipe_compare.py --all-captures \
      --out quality-results/map-comparisons/intro4-8k-native-hdr

Published maps in run/maps/<name>/published.json are mounted automatically.
The original retail map is loaded from the staged Portal 2 runtime. Each map
gets one portal_boot run; all-captures boots A and B in parallel and captures
every pose without rebooting. The view oracle must confirm the requested
camera before the HTML is written.
"""

import argparse
import array
from concurrent.futures import ThreadPoolExecutor
import datetime
import html
import json
import math
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys

from PIL import Image, ImageChops, ImageEnhance, ImageStat


ROOT = Path(__file__).resolve().parents[2]
BOOT = ROOT / "tools/quality/portal_boot.py"
sys.path.insert(0, str(ROOT / "tools/kiln"))
import sepipe_loader  # noqa: E402
sys.path.insert(0, str(Path(__file__).resolve().parent))
from view_oracle import console_script, script_frames
# Retained comparison and survey cameras, deduplicated. Survey player origins
# include the 64-unit standing eye offset here.
INTRO4_CAPTURES = (
    ("fizzler-door", "First fizzler doorway", "400.03,223.97,66.48:0,-45.04,0"),
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

FIZZLER_SETTLE_FRAMES = 134  # At host_framerate 0.015, allow just over two seconds.
FIZZLER_CHECK_FRAMES = 10  # ent_dump replies arrive through the server/client channel.
FIZZLER_BEGIN = "swipe_fizzler_check_begin"
FIZZLER_END = "swipe_fizzler_check_end"


def fizzler_commands():
    return ["ent_fire fizzler1_disable_rl Trigger", "wait %d" % FIZZLER_SETTLE_FRAMES,
            "echo " + FIZZLER_BEGIN, "ent_dump fizzler_brush",
            "wait %d" % FIZZLER_CHECK_FRAMES, "echo " + FIZZLER_END]


def verify_fizzler(boot):
    logs = [boot / "stdout.log", boot / "runtime/portal2/console.log"]
    for path in logs:
        if not path.is_file():
            continue
        text = path.read_text(errors="replace")
        for dump in re.findall(FIZZLER_BEGIN + r"\s*\n(.*?)" + FIZZLER_END, text, re.S):
            states = re.findall(r"^\s*StartDisabled:\s*([01])\s*$", dump, re.M)
            if states == ["1"]:
                return {"entity": "fizzler_brush", "disabled": True,
                        "settle_frames": FIZZLER_SETTLE_FRAMES, "log": str(path)}
    raise ValueError("first fizzler did not confirm disabled before capture")
sys.path.insert(0, str(ROOT / "tools/kiln"))
import sepipe_loader


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


def camera_commands(pose):
    position, angles = pose
    # setpos names the player origin; Portal 2's standing eye is 64 units above it.
    return ["cmd setpos " + " ".join("%.6f" % n for n in
                                   (position[0], position[1], position[2] - 64)),
            "cmd setang " + " ".join("%.6f" % n for n in angles)]


def side_settings(side, args):
    if side == "a":
        return (("r_temporal_scale 0", "r_core_world 0", "mat_hdr_output 0"),
                ("-norendercore",))
    return (("r_core_world_strict 1", "mat_antialias 4", "r_temporal_scale 0", "mat_hdr_output 1",
             "mat_hdr_exposure 3", "mat_hdr_peak_nits 10000"),
            ())


def capture(map_name, side, content, args, out, poses=None):
    startup_commands, engine_args = side_settings(side, args)
    boot = out / (side + "-boot")
    command = [sys.executable, str(BOOT), *sepipe_loader.boot_arguments(args),
               "--renderer", "core", "--require-vulkan", "--require-wayland",
               "--view-oracle", "--map", map_name, "--out", str(boot),
               "--width", str(args.width), "--height", str(args.height),
               "--timeout", str(args.timeout), "--capture-wait", "120",
               "--engine-arg=-deterministicrender", "--engine-arg=-nosound",
               "--engine-arg=-noborder", "--no-mouse",
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
    intro4 = map_name in ("sp_a1_intro4", "sp_a1_intro4_relit")
    if intro4:
        for line in fizzler_commands():
            command += ["--console-command", line]
    for relay in args.panel_relay:
        command += ["--console-command", "ent_fire " + relay + " Trigger"]
    if poses is None:
        for line in camera_commands(args.pose):
            command += ["--console-command", line]
        if side == "b":
            command += ["--console-command", "wait 120"]
            for line in camera_commands(args.pose):
                command += ["--console-command", line]
            command += ["--console-command", "wait 3"]
            command += ["--console-command", "screenshot_hdr swipe_single_hdr; wait 3"]
    else:
        shots = [{"name": name, "commands": camera_commands(pose), "settle": 120}
                 for name, pose in poses]
        scenario = {"setup": [], "intro_frames": 0}
        for line in console_script(scenario, shots,
                                   frame=lambda index, shot:
                                   ("screenshot_hdr swipe_" + shot["name"] + "_hdr; wait 3; "
                                    if side == "b" else "") +
                                   "screenshot swipe_" + shot["name"]):
            command += ["--console-command", line]
        # Exec schedules the closing commands independently of future alias waits.
        # Keep the runner alive for the entire chain, including each HDR export.
        frames = script_frames(scenario, shots) + (3 * len(shots) if side == "b" else 0)
        command[command.index("--capture-wait") + 1] = str(frames)
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
    fizzler = verify_fizzler(boot) if intro4 else None
    shots = evidence.get("screenshots", [])
    oracles = evidence.get("view_oracle_captures", [])
    expected = len(poses) + 1 if poses is not None else 1
    expected_oracles = expected + (len(poses) if poses is not None else 1) if side == "b" else expected
    if len(shots) != expected or len(oracles) != expected_oracles:
        raise ValueError("%s needs %d SDR screenshots and %d view oracles; got %d and %d" %
                         (map_name, expected, expected_oracles, len(shots), len(oracles)))
    oracles = sorted(oracles, key=lambda item: int(
        Path(item["path"]).stem.rsplit("-", 1)[-1]))
    render_confirmation = None
    if side == "b":
        log = (boot / "stdout.log").read_text()
        hdr = re.findall(r"\[NativeVulkan\] (HDR output: [^\n]+|extended output declined: [^\n]+)", log)
        render_confirmation = {"hdr_presentation_log": hdr[-1] if hdr else "unconfirmed"}
    records = {}
    for index, (name, pose) in enumerate(poses or [("single", args.pose)]):
        position, angles = pose
        oracle_index = index * 2 + 1 if side == "b" else index
        oracle = oracles[oracle_index]["path"]
        view = oracle_camera(oracle)
        verify_camera(view, position, angles, args.width, args.height)
        if poses is None:
            source = Path(shots[0]["path"])
            target = out / (side + ".png")
        else:
            matches = [Path(shot["path"]) for shot in shots
                       if Path(shot["path"]).stem == "swipe_" + name]
            if len(matches) != 1:
                raise ValueError("missing or duplicate named screenshot: " + name)
            source = matches[0]
            target = out / name / (side + ".png")
            target.parent.mkdir(exist_ok=True)
        with Image.open(source) as image:
            if image.size != (args.width, args.height):
                raise ValueError("%s screenshot has wrong dimensions" % name)
            image.convert("RGB").save(target)
        records[name] = {"map": map_name, "content_root": str(content) if content else None,
            "renderer": "native-vulkan", "client": sepipe_loader.boot_arguments(args),
            "image": target.name, "boot_evidence": str(evidence_path),
            "source_image": str(source), "view_oracle": str(oracle),
            "startup_commands": list(startup_commands),
            "engine_args": list(engine_args),
            "render_confirmation": render_confirmation,
            "fizzler_confirmation": fizzler,
            "camera": {key: view[key] for key in ("origin", "angles", "fov", "viewport")}}
        if side == "b":
            hdr_oracle = oracles[oracle_index - 1]["path"]
            verify_camera(oracle_camera(hdr_oracle), position, angles, args.width, args.height)
            hdr = boot / "runtime/portal2/screenshots" / ("swipe_" + name + "_hdr.pfm")
            records[name]["hdr_export"] = verify_hdr_export(hdr, args)
            records[name]["hdr_export"]["view_oracle"] = hdr_oracle
    return records if poses is not None else records["single"]


def verify_hdr_export(path, args):
    metadata = json.loads(Path(str(path) + ".json").read_text())
    if metadata.get("schema") != "source-hdr-capture/v1" or \
            metadata.get("stage") != "scene-before-output" or \
            metadata.get("primaries") != "Rec.709" or metadata.get("encoding") != "linear" or \
            metadata.get("exposure") != 3 or metadata.get("display_peak_nits") != 10000 or \
            [metadata.get("width"), metadata.get("height")] != [args.width, args.height]:
        raise ValueError("HDR export metadata differs from requested capture: " + str(path))
    with path.open("rb") as file:
        if file.readline().strip() != b"PF" or \
                file.readline().strip() != ("%d %d" % (args.width, args.height)).encode() or \
                float(file.readline()) != -1.0:
            raise ValueError("HDR export has an invalid PFM header: " + str(path))
        offset = file.tell()
        count = args.width * args.height * 3
        if path.stat().st_size != offset + count * 4:
            raise ValueError("HDR export is incomplete: " + str(path))
        pixels = array.array("f")
        pixels.fromfile(file, count)
    if sys.byteorder != "little":
        pixels.byteswap()
    if not all(math.isfinite(value) for value in pixels):
        raise ValueError("HDR export contains non-finite pixels: " + str(path))
    peak = max(pixels)
    if not math.isclose(peak, metadata["rgb_max"], rel_tol=1e-6, abs_tol=1e-8):
        raise ValueError("HDR pixel range differs from metadata: " + str(path))
    if metadata.get("reference_white_nits") != 203:
        raise ValueError("HDR export has an unknown scene luminance scale: " + str(path))
    if not metadata.get("hdr_output_active"):
        raise ValueError("HDR display output was not active: " + str(path))
    if not metadata.get("output_pass_recorded"):
        raise ValueError("HDR export did not record the required output pass: " + str(path))
    return {"path": str(path), "metadata": str(path) + ".json", "raw_rgb_peak": peak,
            "reference_white_nits": metadata["reference_white_nits"],
            "hdr_output_active": metadata["hdr_output_active"]}


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
<p>Right game: native resolution, FSR off, 4× MSAA, HDR exposure 3, 10000-nit output peak.
  These RGB PNGs do not verify HDR display presentation.</p>
PROCESSING_NOTE
HDR_LINKS
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
                          "PROCESSING_NOTE": (
                              '<p>B’s PNG uses offline SDR tone mapping fitted to the original: '
                              'shared contrast %.3f, exposure %.3f×. '
                              '<a href="b-engine.png">Previous engine PNG</a> · '
                              '<a href="../tone-map.json">Mapping settings</a>.</p>' % (
                                  right["offline_sdr"]["contrast"], right["offline_sdr"]["exposure"])
                              if right.get("offline_sdr") else ""),
                          "HDR_LINKS": ('<p>Download B’s unexposed linear HDR: '
                                        '<a href="%s">floating-point PFM</a> · '
                                        '<a href="%s">colour-space metadata</a>.</p>' % (
                                            html.escape(os.path.relpath(right["hdr_export"]["path"], out)),
                                            html.escape(os.path.relpath(right["hdr_export"]["metadata"], out)))
                                        if right.get("hdr_export") else ""),
                          "WIDTH": str(metrics["size"][0]), "HEIGHT": str(metrics["size"][1]),
                          "MEAN": "%.2f" % metrics["mean_absolute_rgb"]}.items():
        page = page.replace(marker, value)
    (out / "compare.html").write_text(page)


def write_comparison(args, out, left, right, pose, captured_utc=None):
    if abs(left["camera"]["fov"] - right["camera"]["fov"]) > 0.1:
        raise ValueError("captured camera FOV differs between maps")
    with Image.open(out / "a.png") as a, Image.open(out / "b.png") as b:
        difference = ImageChops.difference(a, b)
        ImageEnhance.Brightness(difference).enhance(4).save(out / "difference-4x.png")
        metrics = {"size": list(a.size),
                   "mean_absolute_rgb": sum(ImageStat.Stat(difference).mean) / 3}
    position, angles = pose
    receipt = {"schema": "map-swipe-comparison/v1", "left": left, "right": right,
               "requested_camera": {"origin": position, "angles": angles},
               "activated_panel_relays": args.panel_relay,
               "disabled_fizzler_relay": "fizzler1_disable_rl",
               "metrics": metrics,
               "captured_utc": captured_utc or datetime.datetime.now(datetime.timezone.utc).isoformat()}
    (out / "comparison.json").write_text(json.dumps(receipt, indent=2) + "\n")
    make_html(out, left, right, metrics, position, angles)
    print(out / "compare.html", flush=True)


def capture_gallery(args, out, content_a, content_b):
    # Each worker owns one private game runtime. Both games run once, concurrently.
    poses = [(name, parse_pose(pose)) for name, _, pose in INTRO4_CAPTURES]
    for relay in ("info_sign-info_panel_activate_rl", "InstanceAuto63-info_panel_activate_rl"):
        if relay not in args.panel_relay:
            args.panel_relay.append(relay)
    if args.rerun_b:
        return refresh_b(args, out, content_b, poses)
    print("Capturing %d Intro4 poses on two parallel game hosts" % len(poses), flush=True)
    with ThreadPoolExecutor(max_workers=2) as hosts:
        a = hosts.submit(capture, args.map_a, "a", content_a, args, out, poses)
        b = hosts.submit(capture, args.map_b, "b", content_b, args, out, poses)
        left, right = a.result(), b.result()
    cards = []
    for name, label, pose in INTRO4_CAPTURES:
        write_comparison(args, out / name, left[name], right[name], parse_pose(pose))
        cards.append('<a href="%s/compare.html"><img src="%s/b.png" alt="%s">%s</a>' %
                     (name, name, html.escape(label), html.escape(label)))
    write_gallery(out, cards, args.width, args.height)
    print(out / "index.html", flush=True)
    return 0


def existing_gallery(args, out):
    if not (out / "index.html").is_file():
        raise ValueError("--rerun-b needs an existing complete gallery")
    receipts = {}
    for name, _, pose in INTRO4_CAPTURES:
        directory = out / name
        receipt = json.loads((directory / "comparison.json").read_text())
        if receipt["left"]["map"] != args.map_a or receipt["right"]["map"] != args.map_b:
            raise ValueError("existing gallery map pair differs")
        position, angles = parse_pose(pose)
        verify_camera(receipt["left"]["camera"], position, angles, args.width, args.height)
        with Image.open(directory / "a.png") as image:
            if image.size != (args.width, args.height):
                raise ValueError("existing A capture dimensions differ")
        receipts[name] = receipt
    return receipts


def refresh_b(args, out, content, poses):
    previous = existing_gallery(args, out)
    tag = datetime.datetime.now(datetime.timezone.utc).strftime("%Y%m%dT%H%M%S.%fZ")
    staged = out / "b-reruns" / tag
    staged.mkdir(parents=True)
    print("Refreshing B's %d poses; reusing A from %s" % (len(poses), out), flush=True)
    right = capture(args.map_b, "b", content, args, staged, poses)
    # All camera/HDR/boot checks pass before changing the live gallery.
    for name, pose in poses:
        directory = staged / name
        shutil.copy2(out / name / "a.png", directory / "a.png")
        write_comparison(args, directory, previous[name]["left"], right[name], pose)
    styled = (out / "tone-map.json").is_file()
    if styled:
        import hdr_swipe_tonemap
        hdr_swipe_tonemap.main(["--gallery", str(staged)])
    backup = staged / "previous-gallery"
    backup.mkdir()
    for filename in ("index.html", "tone-map.json"):
        if (out / filename).is_file():
            shutil.copy2(out / filename, backup / filename)
    cards = []
    for name, label, _ in INTRO4_CAPTURES:
        directory, source = out / name, staged / name
        old = backup / name
        old.mkdir()
        for filename in ("b.png", "b-engine.png", "difference-4x.png", "compare.html",
                         "comparison.json", "comparison-engine.json"):
            if (directory / filename).is_file():
                shutil.copy2(directory / filename, old / filename)
            if (source / filename).is_file():
                shutil.copy2(source / filename, directory / filename)
        receipt = json.loads((directory / "comparison.json").read_text())
        make_html(directory, receipt["left"], receipt["right"], receipt["metrics"],
                  receipt["requested_camera"]["origin"], receipt["requested_camera"]["angles"])
        cards.append('<a href="%s/compare.html"><img loading="lazy" src="%s/b.png" alt="%s">%s</a>' %
                     (name, name, html.escape(label), html.escape(label)))
    if styled:
        shutil.copy2(staged / "tone-map.json", out / "tone-map.json")
    write_gallery(out, cards, args.width, args.height,
                  processing_note=("PNG previews use offline tone mapping fitted to the original A images."
                                   if styled else None))
    (out / "latest-b-rerun.json").write_text(json.dumps({
        "schema": "map-swipe-b-refresh/v1", "capture": str(staged),
        "previous_gallery": str(backup), "poses": len(poses), "left_reused": True,
        "styled_pngs": styled, "boot_evidence": str(staged / "b-boot/evidence.json")}, indent=2) + "\n")
    print(out / "index.html", flush=True)
    return 0


def write_gallery(out, cards, width, height, processing_note=None):
    page = '''<!doctype html><html lang="en"><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Intro4 map comparisons</title><style>
:root { color-scheme: dark; font: 16px system-ui; }
body { max-width: 1320px; margin: auto; padding: 24px; }
.grid { display: grid; grid-template-columns: repeat(auto-fit,minmax(300px,1fr)); gap: 20px; }
a { color: inherit; } img { display: block; width: 100%%; margin-bottom: 8px; }
</style><h1>Intro4 map comparisons</h1>
<p>sp_a1_intro4 / sp_a1_intro4_relit · %d×%d · right game: native resolution, FSR off,
4× MSAA, HDR exposure 3, 10000-nit output peak. Each swipe links B’s unexposed
floating-point HDR export. RGB PNGs do not verify HDR display presentation.</p>
<p>Original: native Vulkan with render core disabled. Relit: native Vulkan render core.</p>
%s
<div class="grid">%s</div></html>''' % (
        width, height, '<p>%s</p>' % html.escape(processing_note) if processing_note else "",
        "".join(cards))
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
                                   help="capture all 19 selected Intro4 poses excluding panel")
    parser.add_argument("--rerun-b", action="store_true",
                        help="refresh only B in an existing all-captures gallery, reusing A")
    parser.add_argument("--content-root-a", type=Path)
    parser.add_argument("--content-root-b", type=Path)
    parser.add_argument("--panel-relay", action="append", default=[],
                        help="activate a named test chamber panel relay on both maps; repeatable")
    sepipe_loader.add_arguments(parser, "portal2-fsr")
    parser.add_argument("--width", type=int, default=7680)
    parser.add_argument("--height", type=int, default=4320)
    parser.add_argument("--timeout", type=int, default=1800)
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--in-compositor", action="store_true", help=argparse.SUPPRESS)
    args = parser.parse_args(argv)
    if args.map_a == args.map_b:
        parser.error("choose two different maps")
    if args.timeout < 1:
        parser.error("timeout must be positive")
    if not (64 <= args.width <= 8192 and 64 <= args.height <= 8192):
        parser.error("capture dimensions must be between 64 and 8192")
    try:
        if sepipe_loader.game_of(args.profile) != "portal2":
            parser.error("--profile must launch Portal 2")
    except sepipe_loader.LoadError as error:
        parser.error("kiln: %s" % error)
    if any(not re.fullmatch(r"[A-Za-z0-9_@-]+", name) for name in args.panel_relay):
        parser.error("panel relay names may contain only letters, digits, _, @, and -")
    if args.all_captures and (args.map_a, args.map_b) != ("sp_a1_intro4", "sp_a1_intro4_relit"):
        parser.error("--all-captures uses the Intro4 gallery cameras and map pair")
    if args.rerun_b and not args.all_captures:
        parser.error("--rerun-b requires --all-captures")
    out = args.out.resolve()
    if args.rerun_b:
        try:
            existing_gallery(args, out)
        except (OSError, ValueError, KeyError) as error:
            parser.error(str(error))
    if not args.in_compositor:
        if not args.rerun_b and out.exists() and any(out.iterdir()):
            parser.error("output directory is not empty: " + str(out))
        out.mkdir(parents=True, exist_ok=True)
        # kiln's private display session: a private bus and headless mutter.
        command = [sys.executable, str(Path(__file__).resolve()),
                   *(argv if argv is not None else sys.argv[1:]), "--in-compositor"]
        returncode = sepipe_loader.run_under_display(
            "private", out / "display", (args.width, args.height, 60), command, os.environ,
            out / "compositor.log", args.timeout * 2 + 120, append=args.rerun_b)
        if returncode:
            print("map_swipe_compare: compositor run failed; see " + str(out / "compositor.log"),
                  file=sys.stderr)
        else:
            print(out / ("index.html" if args.all_captures else "compare.html"))
        return returncode
    if not args.rerun_b and out.exists() and any(out.iterdir()):
        allowed = {"display", "compositor.log"}
        if any(item.name not in allowed for item in out.iterdir()):
            parser.error("output directory is not empty: " + str(out))
    out.mkdir(parents=True, exist_ok=True)
    try:
        content_a = args.content_root_a or published_content(args.map_a)
        content_b = args.content_root_b or published_content(args.map_b)
        if args.all_captures:
            return capture_gallery(args, out, content_a, content_b)
        left = capture(args.map_a, "a", content_a, args, out)
        right = capture(args.map_b, "b", content_b, args, out)
        write_comparison(args, out, left, right, args.pose)
        return 0
    except (OSError, ValueError, KeyError, subprocess.SubprocessError) as error:
        print("map_swipe_compare: " + str(error), file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
