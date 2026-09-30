#!/usr/bin/env python3
"""RFC 0014 D0 in the product: the debug view catalog on native Vulkan.

    python3 tools/render/debug_views_product.py run --out DIR [--build DIR]
        [--queue-mode 0|2] [--map testchmb_a_01]
    python3 tools/render/debug_views_product.py run --platform android
        --device SERIAL --out DIR [--queue-mode 0|2]
    python3 tools/render/debug_views_product.py selftest

`run` boots the Portal client headless on native Vulkan with the render core
drawing the world (r_core_world 1), takes a chain of screenshots with the
cl_render_debug_* ConVars set, and judges each:

- uv-checker (cl_render_debug_view 13): every pixel is black, white, the
  hatch or a black/white edge blend; the core drew at least a third of the
  frame; no other color, so the legacy stream (HUD, models, particles) drew
  nothing;
- hatch (r_core_world 0 and view 13): the core draws nothing and the legacy
  stream is off, so every pixel is the not-applicable hatch at its own
  coordinates (period 8 along the diagonal, sRGB 137 and 188);
- filter (view 1, cl_render_debug_view_program unlit): the lightmapped world
  outside the filter is flat 18% grey (sRGB 118);
- refused (view 18, reserved until its term is on the core): the console
  names the refusal and the frame keeps the previous view (13's checker);
- neutral (every control back at its default): the frame is a normal frame,
  which the uv-checker judge must reject (the judge's negative control);
- legacy-skip (cl_render_debug_legacy 2, no view): the core world keeps its
  shading and the legacy stream is off (the checker judge rejects it, and no
  pixel is the HUD's);
- legacy-tint (cl_render_debug_legacy 1): what the core did not draw (the
  legacy glass, the HUD) is tinted magenta, the core's world is not; with the
  core drawing nothing (r_core_world 0) the whole frame is tinted;
- furnace (cl_render_debug_furnace 1): the core's world is albedo 1 under a
  radiance of 1, so white (env-mapped surfaces add their specular over it);
- term-baked (cl_render_debug_term baked): the core's lightmapped world takes
  a zero lightmap, so it goes dark; cl_render_debug_claims names the program
  of each claimed material.

`selftest` runs the judges on synthetic frames: each must accept its own
frame and reject the others' (checks-v1).

Both print one checks-v1 record. The render sequence runs on the main thread
(--queue-mode 0) or its own (2); D0 requires both.
"""
import argparse
import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools" / "quality"))
from conformance_result import Checks  # noqa: E402

PORTAL_BOOT = ROOT / "tools" / "quality" / "portal_boot.py"
RUNTIME = ROOT.parent / "source-engine" / "run" / "runtime"

HATCH_DARK = 137   # 0.25 linear through the sRGB view
HATCH_LIGHT = 188  # 0.5
FILTER_GREY = 118  # 0.18


def srgb(linear):
    value = linear * 12.92 if linear <= 0.0031308 else 1.055 * linear ** (1 / 2.4) - 0.055
    return int(round(max(0.0, min(1.0, value)) * 255))


# The shots, in order: (name, console commands before the frame).
SHOTS = [
    ("uv-checker", ["mat_queue_mode", "cl_render_debug_view 13"]),
    ("refused", ["cl_render_debug_view 18"]),
    ("filter", ["cl_render_debug_view 1", "cl_render_debug_view_program unlit"]),
    ("hatch", ["cl_render_debug_view_program \"\"", "cl_render_debug_view 13", "r_core_world 0"]),
    ("neutral", ["r_core_world 1", "cl_render_debug_view 0"]),
    ("legacy-skip", ["cl_render_debug_legacy 2"]),
    ("legacy-tint-yaw000", ["cl_render_debug_legacy 1", "cmd setang 0 0 0"]),
    ("legacy-tint-yaw090", ["cmd setang 0 90 0"]),
    ("legacy-tint-yaw180", ["cmd setang 0 180 0"]),
    ("legacy-tint-yaw270", ["cmd setang 0 270 0"]),
    ("legacy-tint-no-core", ["r_core_world 0"]),
    ("furnace", ["r_core_world 1", "cl_render_debug_legacy 0", "cl_render_debug_furnace 1"]),
    ("term-baked", ["cl_render_debug_furnace 0", "cl_render_debug_term baked",
                    "cl_render_debug_claims"]),
]


def console_script(shots, settle=40):
    """Aliases that each set a shot's controls, wait, screenshot and call the
    next (view_oracle.py's pattern: one exec'd line runs at once)."""
    lines = []
    for index, (name, commands) in enumerate(shots):
        steps = list(commands) + ["wait %d" % settle, "echo DVP_SHOT %s" % name, "screenshot",
                                  "wait 10"]
        if index + 1 < len(shots):
            steps.append("dvp_shot%d" % (index + 1))
        lines.append('alias dvp_shot%d "%s"' % (index, "; ".join(
            step.replace('"', "") for step in steps)))
    lines.append("wait 120; dvp_shot0")
    return lines


def load_tga(path):
    from PIL import Image
    image = Image.open(path).convert("RGB")
    return image.size[0], image.size[1], image.tobytes()


def classify(width, height, pixels):
    """Counts of black, white, hatch-correct, edge blends and other pixels."""
    counts = {"black": 0, "white": 0, "hatch": 0, "hatch-wrong": 0, "blend": 0, "grey": 0,
              "other": 0, "tinted": 0, "dark": 0}
    for y in range(height):
        row = y * width * 3
        for x in range(width):
            r, g, b = pixels[row + x * 3: row + x * 3 + 3]
            if max(r, g, b) <= 12:
                counts["dark"] += 1
            if r >= 188 and b >= 188 and r - g >= 30 and b - g >= 30:
                counts["tinted"] += 1
            if r != g or g != b:
                counts["other"] += 1
                continue
            expected = HATCH_DARK if ((x + y) & 7) < 4 else HATCH_LIGHT
            if r == 0:
                counts["black"] += 1
            elif r == 255:
                counts["white"] += 1
            elif r == expected:
                counts["hatch"] += 1
            elif r in (HATCH_DARK, HATCH_LIGHT):
                counts["hatch-wrong"] += 1
            elif r == FILTER_GREY:
                counts["grey"] += 1
            else:
                counts["blend"] += 1
    return counts


def judge_checker(counts, total):
    """The checker view: the core's black and white, the hatch and edge blends only."""
    core = counts["black"] + counts["white"]
    return (counts["other"] == 0 and counts["hatch-wrong"] <= total // 200 and
            core >= total // 3 and counts["blend"] <= total // 50)


def judge_hatch(counts, total):
    return counts["hatch"] >= total - total // 1000 and counts["other"] == 0


def judge_filter(counts, total):
    return counts["grey"] >= total // 3 and counts["other"] == 0


def boot_desktop(args, out):
    """Portal on native Vulkan, headless; returns (console text, screenshots)."""
    command = [sys.executable, str(PORTAL_BOOT), "--runtime", str(args.runtime),
               "--build", str(args.build), "--renderer", "native-vulkan", "--headless",
               "--map", args.map, "--out", str(out / "boot"),
               "--capture-wait", str(len(SHOTS) * 60 + 200), "--timeout", "600",
               "--startup-command", "sv_cheats 1", "--startup-command", "r_core_world 1",
               "--startup-command", "mat_queue_mode %d" % args.queue_mode]
    if args.content_root:
        command += ["--content-root", str(args.content_root)]
    for line in console_script(SHOTS):
        command += ["--console-command", line]
    completed = subprocess.run(command, capture_output=True, text=True)
    (out / "boot.log").write_text(completed.stdout + completed.stderr)
    console = out / "boot" / "runtime" / "portal" / "console.log"
    text = console.read_text(errors="replace") if console.exists() else ""
    shots_dir = out / "boot" / "runtime" / "portal" / "screenshots"
    return text, sorted(shots_dir.glob("*.tga")) if shots_dir.is_dir() else []


ANDROID_CONTENT = "/sdcard/Android/data/%s/files"
ANDROID_ACTIVITY = "org.libsdl.app.SDLActivity"
ANDROID_CFG = "portal/custom/debug_views/cfg"


def adb(device, arguments, timeout=120, check=True):
    command = ["adb"] + (["-s", device] if device else []) + list(arguments)
    result = subprocess.run(command, capture_output=True, text=True, timeout=timeout)
    if check and result.returncode != 0:
        raise RuntimeError("adb %s failed: %s" % (" ".join(arguments)[:100], result.stderr[-300:]))
    return result.stdout


def boot_android(args, out):
    """The installed APK on a device (the Fold7): the shots' cfg pushed into
    the app's custom folder, the run started through commandline.txt (read by
    the app root), the console log and screenshots pulled back."""
    import time
    content = ANDROID_CONTENT % args.package
    local = out / "stage"
    (local / "cfg").mkdir(parents=True, exist_ok=True)
    (local / "cfg" / "debug_views.cfg").write_text(
        "".join(line + "\n" for line in console_script(SHOTS) + ["wait 200; quit"]))
    arguments = ["-dev", "-condebug", "+sv_cheats", "1", "+r_core_world", "1",
                 "+mat_queue_mode", str(args.queue_mode), "+map", args.map, "+wait", "300",
                 "+exec", "debug_views.cfg"]
    (local / "commandline.txt").write_text(" ".join(arguments) + "\n")
    (local / "empty.txt").write_text("")
    adb(args.device, ["shell", "mkdir", "-p", "%s/%s" % (content, ANDROID_CFG)])
    adb(args.device, ["push", str(local / "cfg" / "debug_views.cfg"),
                      "%s/%s/debug_views.cfg" % (content, ANDROID_CFG)])
    adb(args.device, ["push", str(local / "commandline.txt"), "%s/commandline.txt" % content])
    adb(args.device, ["shell", "rm", "-rf", "%s/portal/screenshots" % content], check=False)
    adb(args.device, ["shell", "rm", "-f", "%s/portal/console.log" % content], check=False)
    # adb-pushed directories are shell-owned 2770: the app could not read them.
    adb(args.device, ["shell", "chmod", "a+rwX", "%s/portal/custom/debug_views" % content,
                      "%s/%s" % (content, ANDROID_CFG),
                      "%s/%s/debug_views.cfg" % (content, ANDROID_CFG),
                      "%s/commandline.txt" % content])
    try:
        adb(args.device, ["shell", "am", "force-stop", args.package])
        adb(args.device, ["logcat", "-c"], check=False)
        adb(args.device, ["shell", "am", "start", "-W", "-n",
                          "%s/%s" % (args.package, ANDROID_ACTIVITY)])
        deadline = time.monotonic() + 600
        started = False
        while time.monotonic() < deadline:
            running = adb(args.device, ["shell", "pidof", args.package], check=False).strip() != ""
            started |= running
            if started and not running:
                break
            time.sleep(2)
        else:
            adb(args.device, ["shell", "am", "force-stop", args.package], check=False)
    finally:
        (out / "logcat.txt").write_text(adb(args.device, ["logcat", "-d"], timeout=120,
                                            check=False))
        # The next ordinary launch must not rerun the shots.
        adb(args.device, ["push", str(local / "empty.txt"), "%s/commandline.txt" % content],
            check=False)
    pulled = out / "device"
    pulled.mkdir(exist_ok=True)
    adb(args.device, ["pull", "%s/portal/console.log" % content, str(pulled / "console.log")],
        check=False)
    adb(args.device, ["pull", "%s/portal/screenshots" % content, str(pulled)], check=False)
    console = pulled / "console.log"
    text = console.read_text(errors="replace") if console.exists() else ""
    frames = sorted((pulled / "screenshots").glob("*.tga")) if (pulled / "screenshots").is_dir() \
        else sorted(pulled.glob("*.tga"))
    return text, frames


def run(args):
    checks = Checks()
    out = Path(args.out)
    out.mkdir(parents=True, exist_ok=True)
    text, frames = boot_android(args, out) if args.platform == "android" else \
        boot_desktop(args, out)
    names = [line.split()[1] for line in text.splitlines() if line.startswith("DVP_SHOT ")]
    checks.check(names == [name for name, _ in SHOTS], "boot.every-shot-ran",
                 "shots in the console: %s" % names)
    checks.check(len(frames) >= len(SHOTS), "boot.every-shot-captured",
                 "%d screenshots" % len(frames))
    checks.check("Sys_Error" not in text and "Engine error" not in text, "boot.no-fatal-error")
    results = {}
    for index, (name, _) in enumerate(SHOTS):
        if index >= len(frames):
            break
        width, height, pixels = load_tga(frames[index])
        counts = classify(width, height, pixels)
        results[name] = counts
        total = width * height
        if name == "uv-checker":
            checks.check(judge_checker(counts, total), "view-13.core-only-checker", str(counts))
        elif name == "refused":
            checks.check(judge_checker(counts, total), "refused.keeps-the-previous-view",
                         str(counts))
        elif name == "filter":
            checks.check(judge_filter(counts, total), "filter.lightmapped-greyed", str(counts))
        elif name == "hatch":
            checks.check(judge_hatch(counts, total), "hatch.every-pixel-without-a-core-draw",
                         str(counts))
        elif name == "neutral":
            checks.check(not judge_checker(counts, total) and counts["other"] > total // 3,
                         "neutral.normal-frame-and-the-judge-rejects-it", str(counts))
        elif name == "legacy-skip":
            checks.check(not judge_checker(counts, total) and counts["other"] > total // 3,
                         "legacy-skip.core-keeps-its-shading", str(counts))
        elif name.startswith("legacy-tint-yaw"):
            checks.check(total - counts["tinted"] >= total * 3 // 10,
                         "legacy-tint.%s.core-untinted" % name[len("legacy-tint-"):], str(counts))
        elif name == "legacy-tint-no-core":
            checks.check(counts["tinted"] >= total * 97 // 100,
                         "legacy-tint.everything-legacy-without-the-core", str(counts))
        elif name == "furnace":
            checks.check(counts["white"] >= total // 3, "furnace.core-world-white", str(counts))
        elif name == "term-baked":
            checks.check(counts["dark"] >= total // 5, "term-baked.core-world-unlit",
                         str(counts))
    tinted = [results[name]["tinted"] for name in results if name.startswith("legacy-tint-yaw")]
    # A legacy-drawn prop is in view at one of the yaws (testchmb_a_01: the
    # next room's model behind the glass, about 500 pixels).
    checks.check(bool(tinted) and max(tinted) >= 200,
                 "legacy-tint.legacy-drawn-surfaces-magenta",
                 "tinted pixels per yaw: %s" % tinted)
    checks.check("its terms are not on the core yet" in text, "refused.console-names-why")
    checks.check("cl_render_debug_claims: program lightmapped:" in text,
                 "claims.names-the-programs")
    checks.check('"mat_queue_mode" = "%d"' % args.queue_mode in text,
                 "boot.render-sequence-mode-%d" % args.queue_mode, "the console does not show it")
    (out / "judged.json").write_text(json.dumps(results, indent=2) + "\n")
    return checks.report()


def synthetic(kind, width=64, height=48):
    pixels = bytearray()
    for y in range(height):
        for x in range(width):
            if kind == "checker":
                v = 0 if ((x // 8) + (y // 8)) % 2 else 255
                if x > 50:
                    v = HATCH_DARK if ((x + y) & 7) < 4 else HATCH_LIGHT
                pixels += bytes((v, v, v))
            elif kind == "hatch":
                v = HATCH_DARK if ((x + y) & 7) < 4 else HATCH_LIGHT
                pixels += bytes((v, v, v))
            elif kind == "grey":
                pixels += bytes((FILTER_GREY,) * 3)
            elif kind == "normal":
                pixels += bytes(((x * 4) % 256, (y * 5) % 256, 90))
            elif kind == "checker-with-hud":
                v = 0 if ((x // 8) + (y // 8)) % 2 else 255
                pixels += bytes((255, 200, 0)) if 30 < x < 34 and 20 < y < 24 else bytes((v, v, v))
            elif kind == "shifted-hatch":
                v = HATCH_DARK if ((x + y + 1) & 7) < 4 else HATCH_LIGHT
                pixels += bytes((v, v, v))
    return width, height, bytes(pixels)


def selftest(_args):
    checks = Checks()
    total = 64 * 48
    frames = {kind: classify(*synthetic(kind)) for kind in
              ("checker", "hatch", "grey", "normal", "checker-with-hud", "shifted-hatch")}
    checks.check(judge_checker(frames["checker"], total), "selftest.checker-accepted")
    checks.check(not judge_checker(frames["normal"], total), "selftest.normal-frame-rejected")
    checks.check(not judge_checker(frames["checker-with-hud"], total),
                 "selftest.legacy-hud-rejected")
    checks.check(judge_hatch(frames["hatch"], total), "selftest.hatch-accepted")
    checks.check(not judge_hatch(frames["shifted-hatch"], total),
                 "selftest.hatch-at-the-wrong-phase-rejected")
    checks.check(not judge_hatch(frames["checker"], total), "selftest.checker-is-not-hatch")
    checks.check(judge_filter(frames["grey"], total), "selftest.filter-grey-accepted")
    checks.check(not judge_filter(frames["hatch"], total), "selftest.hatch-is-not-filter-grey")
    checks.equal(srgb(0.25), HATCH_DARK, "selftest.hatch-dark-encoding")
    checks.equal(srgb(0.5), HATCH_LIGHT, "selftest.hatch-light-encoding")
    checks.equal(srgb(0.18), FILTER_GREY, "selftest.filter-grey-encoding")
    return checks.report()


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = parser.add_subparsers(dest="command", required=True)
    run_parser = sub.add_parser("run")
    run_parser.add_argument("--out", required=True)
    run_parser.add_argument("--build", default=str(ROOT / "build-rc-client" / "install"))
    run_parser.add_argument("--runtime", default=str(RUNTIME))
    run_parser.add_argument("--content-root",
                            help="a published map's tree (run/maps/<map>) staged into the game")
    run_parser.add_argument("--map", default="testchmb_a_01")
    run_parser.add_argument("--queue-mode", type=int, choices=(0, 2), default=0)
    run_parser.add_argument("--platform", choices=("linux", "android"), default="linux")
    run_parser.add_argument("--device", help="the adb serial (android)")
    run_parser.add_argument("--package", default="org.sourceengine.portal",
                            help="the installed APK's package (android)")
    sub.add_parser("selftest")
    args = parser.parse_args()
    return run(args) if args.command == "run" else selftest(args)


if __name__ == "__main__":
    sys.exit(main())
