#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""The WebGPU adapter's native lane (RFC 0029 decision 11, gate W3): build
render.device.webgpu and a suite against the pinned Dawn release
(quality/toolchain/webgpu.json) with the host compiler, and run it on the
host GPU through Dawn.

    python3 tools/render/webgpu_lane.py run [--out DIR] [--compiler CXX]
        the render.device.v2 suite (unittests/rendertest/core/device/
        test_device_webgpu.cpp)
    python3 tools/render/webgpu_lane.py fetch
        fetch, verify and extract the pinned Dawn release (Waf's
        --render-core-webgpu links it)
    python3 tools/render/webgpu_lane.py suite --name NAME [--out DIR]
        [--compiler CXX] [--define D]... SOURCE...
        any suite: SOURCE files (repository paths) compiled with the adapter

The generated headers (SPIR-V, GLSL, MSL, HLSL and the WGSL artifacts, with
the core artifact store's table) come from tools/render/shader_artifacts.py
headers; the WGSL form and its contract with the adapter are
shader_artifacts' wgsl_compile, translated by the pinned tint. Dawn picks its
backend (Vulkan on Linux); the adapter it chose is printed with every run.
Results prove the adapter against WebGPU as the pinned Dawn implements it,
not against a browser (the browser lane is RFC 0029 W4).

The executable's stdout passes through unchanged, so a checks-v1 suite's
CONFORMANCE record reaches the conformance runner.
"""

import argparse
import os
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools" / "render"))
import shader_toolchain as st  # noqa: E402

DEFAULT_OUT = Path(os.environ.get("TMPDIR", "/tmp")) / "webgpu-lane"
DEVICE_SUITE = ["unittests/rendertest/core/device/test_device_webgpu.cpp"]


class LaneError(Exception):
    pass


def run(argv, what, **kwargs):
    result = subprocess.run(argv, capture_output=True, text=True, **kwargs)
    if result.returncode != 0:
        raise LaneError("%s failed (%d): %s" % (what, result.returncode,
                                                (result.stdout + result.stderr).strip()[-4000:]))
    return result


def adapter_sources():
    return (sorted(str(p.relative_to(ROOT)) for p in (ROOT / "render/device").glob("*.cpp")) +
            sorted(str(p.relative_to(ROOT)) for p in (ROOT / "render/device/webgpu").glob("*.cpp")))


def generated(out):
    """The generated headers (shader_artifacts headers), rebuilt in place."""
    directory = out / "generated"
    run([sys.executable, str(ROOT / "tools/render/shader_artifacts.py"), "headers", "--out",
         str(directory / "spv")], "shader_artifacts headers")
    return directory


def dependency_times(depfile):
    """Modification times of the files a -MMD dependency file lists (a
    missing file counts as new, so the object rebuilds)."""
    if not depfile.is_file():
        return [float("inf")]
    text = depfile.read_text().replace("\\\n", " ")
    _, _, listed = text.partition(":")
    return [Path(name).stat().st_mtime if Path(name).exists() else float("inf")
            for name in listed.split()]


def build(out, name, sources, compiler, defines=()):
    out.mkdir(parents=True, exist_ok=True)
    include = generated(out)
    dawn = st.webgpu_release("linux")
    files = list(dict.fromkeys(adapter_sources() + list(sources)))
    exe = out / name
    objects = out / "obj" / name
    objects.mkdir(parents=True, exist_ok=True)
    flags = ["-std=c++20", "-O2", "-g", "-Wall", "-Wextra", "-Werror", "-pthread",
             "-I" + str(ROOT / "public"), "-I" + str(ROOT),
             "-I" + str(ROOT / "unittests/rendertest/core/device"),
             "-I" + str(include), "-isystem", str(dawn / "include")] + ["-D" + d for d in defines]
    flag_text = " ".join([compiler] + flags)
    outputs, running = [], []

    def finish(entry):
        source, obj, stamp, process = entry
        output, _ = process.communicate()
        if process.returncode != 0:
            obj.unlink(missing_ok=True)
            raise LaneError("compile of %s failed: %s" % (source, output.strip()[-4000:]))
        stamp.write_text(flag_text)

    for source in files:
        obj = objects / (source.replace("/", "__") + ".o")
        stamp = obj.with_suffix(".flags")
        stale = (not obj.exists() or not stamp.exists() or stamp.read_text() != flag_text or
                 obj.stat().st_mtime < max([(ROOT / source).stat().st_mtime] +
                                           dependency_times(obj.with_suffix(".d"))))
        if stale:
            running.append((source, obj, stamp, subprocess.Popen(
                [compiler, *flags, "-MMD", "-MF", str(obj.with_suffix(".d")), "-c",
                 str(ROOT / source), "-o", str(obj)],
                stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)))
            if len(running) >= (os.cpu_count() or 4):
                finish(running.pop(0))
        outputs.append(str(obj))
    for entry in running:
        finish(entry)
    run([compiler, *outputs, "-pthread", "-o", str(exe),
         str(dawn / "lib64" / "libwebgpu_dawn.a"), "-ldl", "-lm"], "link " + name)
    return exe


def execute(exe):
    """Runs the suite with its stdout passed through; its exit status."""
    print("webgpu_lane: Dawn %s" % st.json.loads(st.WEBGPU_PIN.read_text())["release"],
          file=sys.stderr)
    return subprocess.run([str(exe)], cwd=ROOT).returncode


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    commands = parser.add_subparsers(dest="command", required=True)
    commands.add_parser("fetch")
    run_parser = commands.add_parser("run")
    run_parser.add_argument("--out", type=Path, default=DEFAULT_OUT)
    run_parser.add_argument("--compiler", default=os.environ.get("CXX", "g++"))
    suite_parser = commands.add_parser("suite")
    suite_parser.add_argument("--name", required=True)
    suite_parser.add_argument("--out", type=Path, default=DEFAULT_OUT)
    suite_parser.add_argument("--compiler", default=os.environ.get("CXX", "g++"))
    suite_parser.add_argument("--define", action="append", default=[])
    suite_parser.add_argument("sources", nargs="+")
    args = parser.parse_args(argv)
    try:
        if args.command == "fetch":
            print(st.webgpu_release("linux"))
            print(st.webgpu_release("emscripten"))
            return 0
        if args.command == "run":
            exe = build(args.out, "test_device_webgpu", DEVICE_SUITE, args.compiler)
        else:
            exe = build(args.out, args.name, args.sources, args.compiler, args.define)
    except (LaneError, st.ToolchainError) as error:
        print("webgpu_lane: %s" % error, file=sys.stderr)
        return 2
    return execute(exe)


if __name__ == "__main__":
    sys.exit(main())
