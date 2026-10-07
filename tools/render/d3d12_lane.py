#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""The Direct3D 12 adapter's lane (RFC 0024): build render.device.d3d12 and a
suite with MinGW-w64, and run it under Wine with vkd3d-proton.

    python3 tools/render/d3d12_lane.py run [--out DIR]
        the render.device.v2 suite (unittests/rendertest/core/device/
        test_device_d3d12.cpp)
    python3 tools/render/d3d12_lane.py sweep [--family F]... [--frames N] [--out DIR]
        RFC 0024 X5, the optimization resolution sweep (RFC 0016): each
        family's pixel cases drawn full-frame at 720p, 1080p, 1440p and 4K
        (RENDER_FAMILY_SWEEP=N, port timestamps around every draw) on native
        Vulkan (render.family.<F> through the conformance runner) and on
        D3D12 under Wine, back to back on the same tree; DIR/sweep.json and a
        table per resolution
    python3 tools/render/d3d12_lane.py suite --name NAME --out DIR
        [--define D]... [--env K=V]... [--sources-of SUITE]
        [--record-with SUITE] [SOURCE...]
        any suite: SOURCE files (repository paths), and with --sources-of
        those of a conformance-manifest suite other than its device
        adapters, with the D3D12 adapter's, compiled together with -DD for
        each define. --record-with first runs that manifest suite (a Vulkan
        family suite) with RENDER_FAMILY_RECORD_DIR, and the D3D12 suite then
        judges its frames against them (RENDER_FAMILY_REFERENCE_DIR)

The generated headers (SPIR-V, GLSL, MSL and the HLSL artifacts, with the core
artifact store's table) come from tools/render/shader_artifacts.py headers;
the HLSL form and its contract with the adapter are shader_artifacts'
hlsl_compile. The DXC pin is quality/toolchain/dxc.json
(shader_toolchain.dxc_release). Wine and vkd3d-proton come from the host;
their versions are printed with every run. Results prove the adapter against
D3D12 as vkd3d-proton implements it, not against a Windows driver.

The executable's stdout passes through unchanged, so a checks-v1 suite's
CONFORMANCE record reaches the conformance runner.
"""

import argparse
import os
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools" / "render"))
import shader_toolchain as st  # noqa: E402

MINGW = "x86_64-w64-mingw32-g++"
DEFAULT_OUT = Path(os.environ.get("TMPDIR", "/tmp")) / "d3d12-lane"
DEVICE_SUITE = ["unittests/rendertest/core/device/test_device_d3d12.cpp"]
PROFILE = ROOT / "quality/product_profiles/render-d3d12-windows.json"


class LaneError(Exception):
    pass


def run(argv, what, **kwargs):
    result = subprocess.run(argv, capture_output=True, text=True, **kwargs)
    if result.returncode != 0:
        raise LaneError("%s failed (%d): %s" % (what, result.returncode,
                                                (result.stdout + result.stderr).strip()[-4000:]))
    return result


def sdl3():
    """The pinned SDL3 MinGW release's triplet directory (the profile's
    sdl3_mingw), fetched and verified on first use."""
    import hashlib
    import json
    pin = json.loads(PROFILE.read_text())["dependencies"]["sdl3_mingw"]
    base = ROOT / "dependencies/d3d12-lane"
    archive = base / "archives" / pin["cache_archive"]
    if not archive.exists():
        archive.parent.mkdir(parents=True, exist_ok=True)
        run(["curl", "-fsSL", "-o", str(archive), pin["url"]], "download " + pin["cache_archive"])
    digest = hashlib.sha256(archive.read_bytes()).hexdigest()
    if digest != pin["sha256"]:
        raise LaneError("%s: sha256 %s is not the pin %s" % (archive, digest, pin["sha256"]))
    target = base / pin["extracted_directory"]
    stamp = base / (pin["extracted_directory"] + ".pin")
    if not stamp.is_file() or stamp.read_text() != digest:
        shutil.rmtree(target, ignore_errors=True)
        run(["tar", "-xzf", str(archive), "-C", str(base)], "extract " + pin["cache_archive"])
        stamp.write_text(digest)
    return target / pin["triplet_directory"]


def adapter_sources():
    return (sorted(str(p.relative_to(ROOT)) for p in (ROOT / "render/device").glob("*.cpp")) +
            sorted(str(p.relative_to(ROOT)) for p in (ROOT / "render/device/d3d12").glob("*.cpp")))


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
    times = []
    for name in listed.split():
        path = Path(name)
        times.append(path.stat().st_mtime if path.exists() else float("inf"))
    return times


def finish(entry):
    source, obj, stamp, flag_text, process = entry
    output, _ = process.communicate()
    if process.returncode != 0:
        obj.unlink(missing_ok=True)
        raise LaneError("MinGW compile of %s failed: %s" % (source, output.strip()[-4000:]))
    stamp.write_text(flag_text)


def build(out, name, sources, defines=(), with_sdl3=False):
    out.mkdir(parents=True, exist_ok=True)
    include = generated(out)
    newest_header = max((h.stat().st_mtime for h in include.rglob("*.h")), default=0)
    files = list(dict.fromkeys(adapter_sources() + list(sources)))
    exe = out / (name + ".exe")
    objects = out / "obj" / name
    objects.mkdir(parents=True, exist_ok=True)
    flags = ["-std=c++20", "-O2", "-g", "-Wall", "-Wextra", "-Werror",
             "-I" + str(ROOT / "public"), "-I" + str(ROOT),
             "-I" + str(ROOT / "unittests/rendertest/core/device"),
             "-I" + str(ROOT / "unittests/rendertest/core/material"),
             "-I" + str(ROOT / "unittests/rendertest/core/graph"),
             "-I" + str(include)] + ["-D" + d for d in defines]
    libraries = []
    if with_sdl3:
        sdl = sdl3()
        flags.append("-I" + str(sdl / "include"))
        libraries = [str(sdl / "lib" / "libSDL3.dll.a")]  # the import library: SDL3.dll ships beside the executable
        (out / "SDL3.dll").unlink(missing_ok=True)
        shutil.copyfile(sdl / "bin" / "SDL3.dll", out / "SDL3.dll")
    flag_text = " ".join(flags)
    # One object per source, rebuilt when the flags change or any file it
    # included is newer (the compiler's -MMD dependency list).
    outputs, pending, running = [], [], []
    for source in files:
        obj = objects / (source.replace("/", "__") + ".o")
        stamp = obj.with_suffix(".flags")
        stale = (not obj.exists() or not stamp.exists() or stamp.read_text() != flag_text or
                 obj.stat().st_mtime < max([(ROOT / source).stat().st_mtime, newest_header] +
                                           dependency_times(obj.with_suffix(".d"))))
        if stale:
            pending.append((source, obj, stamp))
        outputs.append(str(obj))
    for source, obj, stamp in pending:
        running.append((source, obj, stamp, flag_text, subprocess.Popen(
            [MINGW, *flags, "-MMD", "-MF", str(obj.with_suffix(".d")), "-c", str(ROOT / source),
             "-o", str(obj)],
            stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)))
        if len(running) >= (os.cpu_count() or 4):
            finish(running.pop(0))
    for entry in running:
        finish(entry)
    windows = st.dxc_release("windows")
    run([MINGW, *outputs, "-static", "-pthread", "-o", str(exe), *libraries,
         "-L" + str(windows / "lib" / "x64"), "-ldxcompiler", "-ld3d12", "-ldxgi", "-ldxguid"],
        "MinGW link")
    for dll in ("dxcompiler.dll", "dxil.dll"):
        (out / dll).unlink(missing_ok=True)
        shutil.copyfile(windows / "bin" / "x64" / dll, out / dll)
    return exe


def manifest_sources(suite_id):
    """A conformance-manifest suite's sources, without device adapters (the
    D3D12 adapter's are added by build)."""
    import json
    manifest = json.loads((ROOT / "quality/conformance.manifest.json").read_text())
    for suite in manifest["suites"]:
        if suite["id"] == suite_id:
            return [s for s in suite.get("sources", []) if not s.startswith("render/device/")]
    raise LaneError("no manifest suite " + suite_id)


def record(out, suite_id):
    """Runs a Vulkan family suite with RENDER_FAMILY_RECORD_DIR; the frames'
    directory. Its own verdict is reported but does not stop the lane: the
    frames, not its other clauses, are the reference."""
    directory = out / ("reference-" + suite_id)
    shutil.rmtree(directory, ignore_errors=True)
    directory.mkdir(parents=True)
    env = dict(os.environ, RENDER_FAMILY_RECORD_DIR=str(directory))
    result = subprocess.run([sys.executable, str(ROOT / "tools/quality/conformance.py"), "check",
                             "--suite", suite_id, "--out", str(out / ("conformance-" + suite_id))],
                            cwd=ROOT, env=env, capture_output=True, text=True)
    frames = len(list(directory.glob("*.vulkan.rgba")))
    print("d3d12_lane: %s recorded %d Vulkan frames (exit %d)" % (suite_id, frames,
                                                                  result.returncode),
          file=sys.stderr)
    if frames == 0:
        raise LaneError("%s recorded no Vulkan frame: %s" % (suite_id, result.stdout[-2000:]))
    return directory


def parse_sweep(text):
    """{resolution: {"gpu_median_ms": sum, "gpu_p95_ms": sum, "cpu_run_ms": sum,
    "cases": n}} from SWEEP lines."""
    import re
    totals = {}
    pattern = re.compile(r"^SWEEP (\S+) case (\d+) (\d+x\d+) frames (\d+) gpu_median_ms "
                         r"([\d.]+) gpu_p95_ms ([\d.]+) cpu_run_ms ([\d.]+)")
    for line in text.splitlines():
        match = pattern.match(line)
        if not match:
            continue
        entry = totals.setdefault(match.group(3), {"gpu_median_ms": 0.0, "gpu_p95_ms": 0.0,
                                                   "cpu_run_ms": 0.0, "cases": 0})
        entry["gpu_median_ms"] += float(match.group(5))
        entry["gpu_p95_ms"] += float(match.group(6))
        entry["cpu_run_ms"] += float(match.group(7))
        entry["cases"] += 1
    return totals


REMOTE_FIXTURES = ["quality/fixtures/legacy-shaders", "quality/fixtures/render-families"]


def remote_run(remote, password, out, families, frames):
    """The sweep's measurements on another machine (the benchmark host; the
    development host runs correctness only): both builds made here, copied
    with their fixtures, run there back to back (native Vulkan, then D3D12
    under that machine's newest Proton Wine and vkd3d-proton), logs fetched.
    {family: (vulkan log text, d3d12 log text)}."""
    ssh = ["sshpass", "-p", password, "ssh", "-o", "StrictHostKeyChecking=no", remote]
    bundle = out / "remote"
    shutil.rmtree(bundle, ignore_errors=True)
    bundle.mkdir(parents=True)
    for fixture in REMOTE_FIXTURES:
        shutil.copytree(ROOT / fixture, bundle / fixture)
    for family in families:
        suite = "render.family.%s" % family
        build_dir = out / "vulkan-build"
        built = subprocess.run([sys.executable, str(ROOT / "tools/quality/conformance.py"), "check",
                                "--suite", suite, "--build-dir", str(build_dir), "--out",
                                str(out / ("conformance-" + family))],
                               cwd=ROOT, capture_output=True, text=True)
        binary = build_dir / ("render_family_%s.default" % family)
        if not binary.is_file():
            raise LaneError("the Vulkan %s suite did not build: %s" % (family, built.stdout[-1500:]))
        shutil.copy2(binary, bundle / binary.name)
        exe = build(out, "family_" + family, manifest_sources(suite + ".gl"),
                    ["RENDERTEST_FAMILY_D3D12"])
        shutil.copy2(exe, bundle / exe.name)
    for dll in ("dxcompiler.dll", "dxil.dll"):
        shutil.copy2(out / dll, bundle / dll)
    script = """set -u
cd ~/d3d12-x5
proton=$(ls -d ~/.local/share/Steam/steamapps/common/'Proton 11.0' 2>/dev/null || ls -d ~/.local/share/Steam/steamapps/common/Proton* | sort | tail -1)
cp -f "$proton"/files/lib/wine/vkd3d-proton/x86_64-windows/*.dll .
# Proton's own dxgi is DXVK's (its builtin needs vkd3d libraries it does not ship).
cp -f "$proton"/files/lib/wine/dxvk/x86_64-windows/dxgi.dll .
# A fresh prefix each run: a prefix whose wineserver was killed mid-write hangs
# DXGI's factory creation in every later run.
export WINEPREFIX=$HOME/d3d12-x5/wineprefix
"$proton/files/bin/wineserver" -k 2>/dev/null; rm -rf "$WINEPREFIX"
export WINEDEBUG=-all VKD3D_DEBUG=none DXVK_LOG_LEVEL=none
export WINEDLLOVERRIDES='d3d12,d3d12core,dxgi=n'
echo "PROTON $proton"
nvidia-smi --query-gpu=name,driver_version --format=csv,noheader
# A benchmark on a GPU another job uses measures that job: wait (bounded)
# until no other compute process holds the GPU, and mark any run during
# which one appeared.
others() { nvidia-smi --query-compute-apps=pid,process_name --format=csv,noheader | grep -v "family_" ; }
for i in $(seq 1 360); do [ -z "$(others)" ] && break; [ $i = 1 ] && echo "WAITING for an idle GPU: $(others)"; sleep 10; done
if [ -n "$(others)" ]; then echo "CONTENDED the GPU stayed busy: $(others)"; exit 3; fi
for family in %s; do
  RENDER_FAMILY_SWEEP=%d ./render_family_${family}.default > vulkan-${family}.log 2>&1
  if [ -n "$(others)" ]; then echo "CONTENDED vulkan-${family}: $(others)"; fi
  # Each Wine program in a fresh prefix: a second program started in a
  # prefix another one used hangs in CreateDXGIFactory2 (Proton 11, DXVK dxgi).
  "$proton/files/bin/wineserver" -k 2>/dev/null; rm -rf "$WINEPREFIX"
  RENDER_FAMILY_SWEEP=%d "$proton/files/bin/wine" ./family_${family}.exe > d3d12-${family}.log 2>&1
  if [ -n "$(others)" ]; then echo "CONTENDED d3d12-${family}: $(others)"; fi
done
""" % (" ".join(families), frames, frames)
    (bundle / "run.sh").write_text(script)
    run(ssh + ["rm -rf ~/d3d12-x5 && mkdir -p ~/d3d12-x5"], "remote clean")
    run(["sshpass", "-p", password, "rsync", "-a", "-e", "ssh -o StrictHostKeyChecking=no",
         str(bundle) + "/", "%s:d3d12-x5/" % remote], "rsync to " + remote)
    remote_out = run(ssh + ["bash ~/d3d12-x5/run.sh"], "remote sweep")
    (out / "remote-run.txt").write_text(remote_out.stdout + remote_out.stderr)
    print(remote_out.stdout.strip(), file=sys.stderr)
    if "CONTENDED" in remote_out.stdout:
        raise LaneError("another job used the GPU during the sweep; its numbers are not kept")
    logs = {}
    for family in families:
        texts = []
        for backend in ("vulkan", "d3d12"):
            fetched = run(ssh + ["cat ~/d3d12-x5/%s-%s.log" % (backend, family)], "fetch log")
            (out / ("remote-%s-%s.log" % (backend, family))).write_text(fetched.stdout)
            texts.append(fetched.stdout)
        logs[family] = tuple(texts)
    return logs


def sweep(out, families, frames, remote=None, password="bazzite"):
    import json
    out.mkdir(parents=True, exist_ok=True)
    report = {"frames": frames, "resolutions": ["1280x720", "1920x1080", "2560x1440",
                                               "3840x2160"], "families": {}}
    remote_logs = remote_run(remote, password, out, families, frames) if remote else None
    report["host"] = remote or "local"
    for family in families:
        if remote_logs:
            vulkan_text, d3d12_text = remote_logs[family]
            report["families"][family] = {"vulkan": parse_sweep(vulkan_text),
                                          "d3d12": parse_sweep(d3d12_text)}
            continue
        suite = "render.family.%s" % family
        env = dict(os.environ, RENDER_FAMILY_SWEEP=str(frames))
        vulkan = subprocess.run([sys.executable, str(ROOT / "tools/quality/conformance.py"),
                                 "check", "--suite", suite, "--out",
                                 str(out / ("conformance-sweep-" + family))],
                                cwd=ROOT, env=env, capture_output=True, text=True)
        log = out / ("conformance-sweep-" + family + ".logs") / suite / "run.0.log"
        vulkan_text = log.read_text() if log.is_file() else vulkan.stdout
        exe = build(out, "family_" + family, manifest_sources(suite + ".gl"),
                    ["RENDERTEST_FAMILY_D3D12"])
        execute(out, exe, ["RENDER_FAMILY_SWEEP=%d" % frames])
        d3d12_text = (out / (exe.stem + ".log")).read_text()
        report["families"][family] = {"vulkan": parse_sweep(vulkan_text),
                                      "d3d12": parse_sweep(d3d12_text)}
    (out / "sweep.json").write_text(json.dumps(report, indent=2) + "\n")
    print("family       resolution  cases  vulkan gpu ms (p95)   d3d12 gpu ms (p95)   d3d12/vulkan")
    for family, result in report["families"].items():
        for resolution in report["resolutions"]:
            v = result["vulkan"].get(resolution)
            d = result["d3d12"].get(resolution)
            if not v or not d:
                print("%-12s %-11s missing (vulkan %s, d3d12 %s)" % (family, resolution, bool(v),
                                                                    bool(d)))
                continue
            print("%-12s %-11s %5d  %8.3f (%8.3f)   %8.3f (%8.3f)   %6.2f" % (
                family, resolution, d["cases"], v["gpu_median_ms"], v["gpu_p95_ms"],
                d["gpu_median_ms"], d["gpu_p95_ms"], d["gpu_median_ms"] / max(v["gpu_median_ms"], 1e-9)))
    print("d3d12_lane: sweep written to " + str(out / "sweep.json"), file=sys.stderr)
    return report


def vkd3d_proton():
    """The host's vkd3d-proton d3d12.dll directory (newest Proton build)."""
    roots = sorted(Path.home().glob(
        ".local/share/Steam/compatibilitytools.d/*/files/lib/wine/vkd3d-proton/x86_64-windows"))
    if not roots:
        raise LaneError("no vkd3d-proton found (install a Proton build with vkd3d-proton)")
    return roots[-1]


def execute(out, exe, extra_env=(), session=False):
    source = vkd3d_proton()
    for dll in source.glob("*.dll"):
        target = out / dll.name
        target.unlink(missing_ok=True)  # Proton's files are read-only
        shutil.copyfile(dll, target)
    env = dict(os.environ, WINEPREFIX=str(out / "wineprefix"), WINEDEBUG="-all",
               WINEDLLOVERRIDES="d3d12,d3d12core=n;dxgi=b", VKD3D_DEBUG="none",
               WINEPATH=str(out))
    for item in extra_env:
        key, _, value = item.partition("=")
        env[key] = value
    wine = run(["wine", "--version"], "wine --version").stdout.strip()
    print("d3d12_lane: %s, vkd3d-proton from %s" % (wine, source), file=sys.stderr)
    # The suite runs from the repository root (fixtures are repository paths);
    # its DLLs are found beside the executable.
    command = ["wine", str(exe)]
    if session:
        # A private compositor (and Xwayland for Wine's X11 driver) with its
        # own D-Bus session: windows never reach the user's display.
        sys.path.insert(0, str(ROOT / "tools/quality"))
        import private_session
        for key in ("DISPLAY", "WAYLAND_DISPLAY"):
            env.pop(key, None)
        command = (private_session.dbus_run_session(out / "dbus") +
                   ["mutter", "--headless", "--virtual-monitor", "1920x1080@60",
                    "--wayland-display", "d3d12-lane-%d" % os.getpid(), "--"] + command)
    result = subprocess.run(command, cwd=ROOT, env=env, capture_output=True, text=True,
                            timeout=1800)
    log = out / (exe.stem + ".log")
    log.write_text(result.stdout + result.stderr)
    for line in result.stdout.splitlines():
        if not line.startswith(("info:", "warn:", "err:")):
            print(line)
    print("d3d12_lane: exit %d, log %s" % (result.returncode, log), file=sys.stderr)
    return result.returncode


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    sub = parser.add_subparsers(dest="command", required=True)
    for name in ("build", "run"):
        p = sub.add_parser(name)
        p.add_argument("--out", type=Path, default=DEFAULT_OUT)
    p = sub.add_parser("sweep")
    p.add_argument("--family", action="append", default=[])
    p.add_argument("--frames", type=int, default=30)
    p.add_argument("--out", type=Path, default=DEFAULT_OUT)
    p.add_argument("--remote", help="user@host to measure on (the benchmark machine)")
    p.add_argument("--password", default="bazzite")
    p = sub.add_parser("suite")
    p.add_argument("--name", required=True)
    p.add_argument("--out", type=Path, default=DEFAULT_OUT)
    p.add_argument("--define", action="append", default=[])
    p.add_argument("--env", action="append", default=[])
    p.add_argument("--sources-of")
    p.add_argument("--record-with")
    p.add_argument("--sdl3", action="store_true", help="link the pinned SDL3 (MinGW)")
    p.add_argument("--session", action="store_true",
                   help="run inside a private headless compositor (window tests)")
    p.add_argument("sources", nargs="*")
    args = parser.parse_args()
    try:
        if args.command == "sweep":
            sweep(args.out, args.family or ["lightmapped", "pbr"], args.frames, args.remote,
                  args.password)
            return 0
        if args.command == "suite":
            sources = list(args.sources)
            if args.sources_of:
                sources += manifest_sources(args.sources_of)
            env = list(args.env)
            if args.record_with:
                env.append("RENDER_FAMILY_REFERENCE_DIR=" + str(record(args.out, args.record_with)))
            exe = build(args.out, args.name, sources, args.define, args.sdl3)
            return execute(args.out, exe, env, args.session)
        exe = build(args.out, "render_device_d3d12", DEVICE_SUITE)
        print("d3d12_lane: built " + str(exe), file=sys.stderr)
        return execute(args.out, exe) if args.command == "run" else 0
    except (LaneError, st.ToolchainError) as error:
        print("d3d12_lane: " + str(error), file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
