#!/usr/bin/env python3
"""render_lab suites (RFC 0016 K11): the render core composed alone.

    python3 tools/render/lab.py composition [--tree build-rc-lab] [--out DIR]
    python3 tools/render/lab.py selftest
    python3 tools/render/lab.py suite <name> [--tree build-rc-lab] [suite options]

`composition` is K11's "Lab composes the core alone": render_lab's link holds
no engine, material system, legacy frontend, composition root or SDL (its
NEEDED libraries are an allow list, and its defined symbols name no module
outside the core layers it may use), and it renders a BSP2 fixture's world
and a studio model through the Vulkan adapter with the Khronos validation
layer and synchronization validation reporting nothing.

`selftest` runs the link scans against seeded symbol lists: each forbidden
module must be found (the scans' negative controls).

`suite` runs one of render_lab's own suites (render/lab/suites.h) with the
tree's libraries on the path; render_lab prints the checks-v1 record.

Both print one checks-v1 record.
"""
import argparse
import math
import os
import re
import struct
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools" / "quality"))
from conformance_result import Checks  # noqa: E402

# The shared libraries render_lab may need: tier0 (the texture readers'
# legacy dependencies), the Vulkan loader, and the C and C++ runtimes.
ALLOWED_NEEDED = re.compile(
    r'^(libtier0\.so|libvulkan\.so\.1|libstdc\+\+\.so\.6|libm\.so\.6|libmvec\.so\.1|'
    r'libgcc_s\.so\.1|libc\.so\.6|ld-linux-x86-64\.so\.2)$')

# Defined symbols that would mean a forbidden module is linked in: the legacy
# frontend and composition root of the core, the engine's binding, the legacy
# material system and shader API, studiorender, and SDL.
FORBIDDEN = (
    ("render.legacy-frontend", re.compile(r'^render::legacy::')),
    ("render.composition", re.compile(r'^(render::composition::|RenderCore_[A-Za-z]+\b)')),
    ("engine", re.compile(r'^(Engine_BindRenderCore|CRender::|CEngine::|Host_[A-Za-z]+\b)')),
    ("materialsystem", re.compile(r'^(CMaterialSystem::|CShaderAPI[A-Za-z0-9]*::|CMatRenderContext)')),
    ("studiorender", re.compile(r'^CStudioRender')),
    ("sdl", re.compile(r'^SDL_[A-Za-z]')),
)

# The composition fixture: the RFC 0011 gallery's Cornell box as published by
# pbrt_map_build.py, and its probe sphere model.
CORNELL = ROOT.parent / "source-engine" / "run" / "maps" / "gi_bleed_cornell"
CORNELL_CAMERA = ["--eye", "59.055,3.937,59.055", "--forward", "0,0.99469,-0.1029",
                  "--up", "0,0.1029,0.99469", "--hfov", "73.74"]


def lab_binary(tree):
    return ROOT / tree / "render" / "lab" / "render_lab"


def lab_env(tree):
    env = dict(os.environ)
    env["LD_LIBRARY_PATH"] = str(ROOT / tree / "tier0") + (
        ":" + env["LD_LIBRARY_PATH"] if env.get("LD_LIBRARY_PATH") else "")
    return env


def defined_symbols(path):
    result = subprocess.run(["nm", "-C", "--defined-only", str(path)], capture_output=True,
                            text=True)
    names = []
    for line in result.stdout.splitlines():
        parts = line.split(" ", 2)
        if len(parts) == 3 and parts[1] in "TtDdBbRr":
            names.append(parts[2])
    return result.returncode, names


def forbidden_modules(symbols):
    """The forbidden modules whose code the symbols define."""
    found = {}
    for name in symbols:
        base = name.split("(")[0]
        for module, pattern in FORBIDDEN:
            if pattern.match(base):
                found.setdefault(module, base)
    return found


def needed_libraries(path):
    result = subprocess.run(["readelf", "-d", str(path)], capture_output=True, text=True)
    return result.returncode, re.findall(r"\(NEEDED\)\s+Shared library: \[([^\]]+)\]",
                                         result.stdout)


def read_pfm(path):
    data = Path(path).read_bytes()
    parts = data.split(b"\n", 3)
    if len(parts) < 4 or parts[0] != b"PF":
        return None
    width, height = (int(v) for v in parts[1].split())
    pixels = struct.unpack("<%df" % (width * height * 3), parts[3][:width * height * 12])
    return width, height, pixels


def run_composition(args):
    checks = Checks()
    binary = lab_binary(args.tree)
    if not checks.check(binary.exists(), "lab.binary", "%s is not built" % binary):
        return checks.report()

    status, needed = needed_libraries(binary)
    checks.equal(status, 0, "link.readelf")
    checks.check(bool(needed), "link.needed-read", "no NEEDED entries read")
    for library in needed:
        checks.check(ALLOWED_NEEDED.match(library), "link.needed." + library,
                     "render_lab needs a library outside the allow list")

    status, symbols = defined_symbols(binary)
    checks.equal(status, 0, "link.nm")
    checks.check(any(s.startswith("render::device::") for s in symbols),
                 "link.core-present", "render_lab defines no render.device code")
    found = forbidden_modules(symbols)
    for module, _ in FORBIDDEN:
        checks.check(module not in found, "link.no-" + module,
                     "defines %s" % found.get(module))

    bsp = CORNELL / "maps" / "gi_bleed_cornell.bsp"
    if not checks.check(bsp.exists(), "fixture.present", "%s is missing" % bsp):
        return checks.report()
    out_dir = Path(args.out) if args.out else Path(tempfile.mkdtemp(prefix="render-lab-"))
    out_dir.mkdir(parents=True, exist_ok=True)
    image = out_dir / "cornell.pfm"
    command = [str(binary), "--game", str(CORNELL), "--map", str(bsp)] + CORNELL_CAMERA + [
        "--size", "320x240", "--out", str(image), "--validate",
        "--model", "models/gi_bleed_cornell/probesphere.mdl", "--model-origin", "60,60,20"]
    result = subprocess.run(command, capture_output=True, text=True, env=lab_env(args.tree),
                            timeout=120)
    (out_dir / "cornell.log").write_text(result.stdout + result.stderr)
    checks.equal(result.returncode, 0, "render.exit")
    checks.check("validation messages 0" in result.stdout, "render.validation-silent",
                 (result.stdout + result.stderr).strip()[-400:])
    drawn = re.search(r"render_lab: (\d+) draws, (\d+) vertices, (\d+) materials", result.stdout)
    checks.check(drawn is not None, "render.reported", result.stdout.strip()[-200:])
    if drawn:
        # The world's four batches and at least one model mesh.
        checks.check(int(drawn.group(1)) >= 5, "render.world-and-model",
                     "%s draws" % drawn.group(1))
    pfm = read_pfm(image) if image.exists() else None
    if checks.check(pfm is not None, "image.written"):
        width, height, pixels = pfm
        checks.equal((width, height), (320, 240), "image.size")
        checks.check(all(math.isfinite(v) and v >= 0.0 for v in pixels), "image.finite")
        lit = sum(1 for i in range(0, len(pixels), 3) if pixels[i] + pixels[i + 1] + pixels[i + 2] > 0)
        checks.check(lit > width * height // 2, "image.lit", "%d lit pixels" % lit)
    return checks.report()


def run_selftest(_args):
    checks = Checks()
    control = ["render::device::Encoder::Draw()", "render::material::Resolve()", "main"]
    checks.equal(forbidden_modules(control), {}, "selftest.control-clean")
    seeded = {
        "render.legacy-frontend": "render::legacy::FrameExecutor::Run()",
        "render.composition": "RenderCore_Create",
        "engine": "Engine_BindRenderCore",
        "materialsystem": "CMaterialSystem::Init()",
        "studiorender": "CStudioRender::DrawModel()",
        "sdl": "SDL_CreateWindow",
    }
    for module, symbol in seeded.items():
        checks.check(module in forbidden_modules(control + [symbol]), "selftest.detects." + module)
    checks.check(not ALLOWED_NEEDED.match("libSDL3.so.0"), "selftest.needed-sdl-rejected")
    checks.check(not ALLOWED_NEEDED.match("libmaterialsystem.so"),
                 "selftest.needed-materialsystem-rejected")
    checks.check(ALLOWED_NEEDED.match("libvulkan.so.1"), "selftest.needed-vulkan-allowed")
    return checks.report()


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = parser.add_subparsers(dest="command", required=True)
    composition = sub.add_parser("composition")
    composition.add_argument("--tree", default="build-rc-lab")
    composition.add_argument("--out")
    sub.add_parser("selftest")
    suite = sub.add_parser("suite")
    suite.add_argument("name")
    suite.add_argument("--tree", default="build-rc-lab")
    args, rest = parser.parse_known_args()
    if args.command == "composition":
        return run_composition(args)
    if args.command == "suite":
        binary = lab_binary(args.tree)
        if not binary.exists():
            print("FAIL lab.binary: %s is not built" % binary)
            return report_missing()
        sys.stdout.flush()
        os.execve(str(binary), [str(binary), "suite", args.name] + rest, lab_env(args.tree))
    return run_selftest(args)


def report_missing():
    checks = Checks()
    checks.check(False, "lab.binary")
    return checks.report()


if __name__ == "__main__":
    sys.exit(main())
