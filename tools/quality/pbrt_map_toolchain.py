#!/usr/bin/env python3
"""Provision and verify the pinned host tools of the PBRT map pipeline.

`quality/product_profiles/pbrt-map-linux-tools.json` owns the layout and the
pipeline-specific pins; OpenUSD/oneTBB and KTX-Software pins stay in their own
host profiles. Everything is installed under `build/toolchains/` (ignored), not
`/tmp`:

    # clone pinned sources (optionally from local mirrors), build, install,
    # and write build/toolchains/pbrt-map-toolchain.json
    python3 tools/quality/pbrt_map_toolchain.py provision \\
        --mirror openusd=/path/OpenUSD --mirror onetbb=/path/oneTBB \\
        --mirror ktx_software=/path/KTX-Software --jobs 16
    python3 tools/quality/pbrt_map_toolchain.py check

Steps are idempotent: an install whose recorded revision matches is reused.
`check` verifies versions against the profiles (Blender, OIDN, OpenUSD Python,
KTX revision), required compile tools and `bsp2tool pack-world-lit`.

The native Vulkan client needs the pinned KTX reader (a PIC static archive the
ktx-reader step builds under dependencies/, outside every Waf output tree) to
upload a map's LMAP lightmap. Enable it in a client
tree, for example the one ./play boots, with

    python3 tools/quality/pbrt_map_toolchain.py configure-client --build build

which adds `--ktx-source-root/--ktx-build-root` to the tree's stored Waf
options; the next ./play (or waf build) rebuilds the affected modules.
"""

import argparse
import ctypes
import json
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
PROFILE = ROOT / "quality/product_profiles/pbrt-map-linux-tools.json"
STEPS = ("sources", "openusd", "ktx", "ktx-reader", "compile-tools", "write")


def load_profiles(path=PROFILE):
    profile = json.loads(Path(path).read_text())
    if profile.get("schema") != "source-host-tool-profile/v1":
        raise ValueError("pbrt map tool profile schema is invalid")
    linked = {name: json.loads((ROOT / relative).read_text())
              for name, relative in profile["profiles"].items()}
    return profile, linked


def absolute(value):
    path = Path(value)
    return path if path.is_absolute() else ROOT / path


def run(command, **kwargs):
    print("$ " + " ".join(map(str, command)), flush=True)
    subprocess.run([str(part) for part in command], check=True, **kwargs)


def git_revision(path):
    result = subprocess.run(["git", "-C", str(path), "rev-parse", "HEAD"],
                            capture_output=True, text=True)
    dirty = subprocess.run(["git", "-C", str(path), "status", "--porcelain",
                            "--untracked-files=no"], capture_output=True, text=True)
    return (result.stdout.strip() if result.returncode == 0 else None,
            bool(dirty.stdout.strip()))


def pinned_sources(linked):
    openusd = linked["openusd"]["dependencies"]
    return {"openusd": openusd["openusd"], "onetbb": openusd["onetbb"],
            "ktx_software": linked["ktx"]["dependencies"]["ktx_software"]}


def provision_sources(profile, linked, mirrors):
    root = absolute(profile["layout"]["sources"])
    root.mkdir(parents=True, exist_ok=True)
    paths = {}
    for name, pin in pinned_sources(linked).items():
        destination = root / name
        if not destination.exists():
            origin = mirrors.get(name, pin["repository"])
            run(["git", "clone", "--no-checkout", origin, destination])
            run(["git", "-C", destination, "checkout", "--detach", pin["revision"]])
            if name == "ktx_software":
                run(["git", "-C", destination, "submodule", "update", "--init", "--recursive"])
        revision, dirty = git_revision(destination)
        if revision != pin["revision"] or dirty:
            raise SystemExit("%s is not the clean pinned revision %s" % (destination,
                                                                         pin["revision"]))
        paths[name] = destination
    return paths


def stamp_matches(install, revision):
    stamp = install / ".pbrt-map-provisioned.json"
    return stamp.is_file() and json.loads(stamp.read_text()).get("revision") == revision


def write_stamp(install, revision):
    (install / ".pbrt-map-provisioned.json").write_text(json.dumps({"revision": revision}) + "\n")


def provision_openusd(profile, linked, sources, jobs):
    layout = profile["layout"]
    install = absolute(layout["openusd_install"])
    revision = linked["openusd"]["dependencies"]["openusd"]["revision"]
    if stamp_matches(install, revision):
        print("[openusd] up to date")
        return
    root = absolute(layout["root"])
    run([sys.executable, ROOT / "tools/worldstage/build_host.py",
         "--openusd-source", sources["openusd"], "--openusd-build", root / "openusd-build",
         "--openusd-install", install, "--onetbb-source", sources["onetbb"],
         "--onetbb-build", root / "onetbb-build",
         "--onetbb-install", absolute(layout["onetbb_install"]),
         "--python", profile["toolchain"]["usd_python"], "--jobs", str(jobs)])
    write_stamp(install, revision)


def provision_ktx(profile, linked, sources, jobs):
    ktx = linked["ktx"]
    build = absolute(profile["layout"]["ktx_build"])
    revision = ktx["dependencies"]["ktx_software"]["revision"]
    if stamp_matches(build, revision) and (build / ktx["build"]["binary"]).is_file():
        print("[ktx] up to date")
        return
    options = ["-D%s=%s" % item for item in ktx["build"]["cmake_options"].items()]
    run(["cmake", "-S", sources["ktx_software"], "-B", build, "-G", ktx["build"]["generator"],
         "-DCMAKE_BUILD_TYPE=" + ktx["toolchain"]["build_type"]] + options)
    run(["cmake", "--build", build, "--target", ktx["build"]["target"], "-j%d" % jobs])
    write_stamp(build, revision)


def provision_ktx_reader(profile, linked, sources, jobs):
    """The client links libktx_read.a into a shared module, so it must be PIC.

    Its source and build live outside every Waf output tree: a client tree
    cannot use headers from inside its own output directory (build/toolchains).
    The source is a local clone of the pinned checkout (the reader needs no
    submodules).
    """
    ktx = linked["ktx"]
    layout = profile["layout"]
    source = absolute(layout["ktx_reader_source"])
    build = absolute(layout["ktx_reader_build"])
    revision = ktx["dependencies"]["ktx_software"]["revision"]
    if stamp_matches(build, revision) and (build / ktx["build"]["reader_library"]).is_file():
        print("[ktx-reader] up to date")
        return
    if not source.exists():
        run(["git", "clone", "--no-checkout", sources["ktx_software"], source])
        run(["git", "-C", source, "checkout", "--detach", revision])
    if git_revision(source) != (revision, False):
        raise SystemExit("%s is not the clean pinned revision %s" % (source, revision))
    options = ["-D%s=%s" % item for item in ktx["build"]["cmake_options"].items()]
    run(["cmake", "-S", source, "-B", build, "-G", ktx["build"]["generator"],
         "-DCMAKE_BUILD_TYPE=" + ktx["toolchain"]["build_type"],
         "-DCMAKE_POSITION_INDEPENDENT_CODE=ON"] + options)
    run(["cmake", "--build", build, "--target", "ktx_read", "-j%d" % jobs])
    write_stamp(build, revision)


def configure_client(profile, linked, build):
    """Add the pinned KTX reader to a client tree's stored Waf options."""
    reader = absolute(profile["layout"]["ktx_reader_build"])
    source = absolute(profile["layout"]["ktx_reader_source"])
    revision = linked["ktx"]["dependencies"]["ktx_software"]["revision"]
    if not stamp_matches(reader, revision):
        raise SystemExit("the KTX reader is not provisioned; run: provision --steps ktx-reader")
    # Add the two options to the tree's stored configure options, then replay
    # them the way ./play does. `waf configure --reconfigure` must not be used:
    # it overwrites every stored option with the parser's truthy defaults
    # (a native Vulkan Portal tree becomes RENDER_BACKEND=legacy GAMES=hl2).
    stored = absolute(build) / "configuration.py"
    wafdirs = sorted(ROOT.glob(".waf3-*/waflib"))
    if not stored.is_file() or not wafdirs:
        raise SystemExit("%s is not a configured Waf tree" % build)
    sys.path.insert(0, str(wafdirs[-1].parent))
    from waflib import ConfigSet
    configuration = ConfigSet.ConfigSet()
    configuration.load(str(stored))
    configuration["OPTIONS"]["KTX_SOURCE_ROOT"] = str(source)
    configuration["OPTIONS"]["KTX_BUILD_ROOT"] = str(reader)
    configuration.store(str(stored))
    import ensure_configured
    ensure_configured.reconfigure(absolute(build))


def provision_compile_tools(profile, jobs):
    layout, tools = profile["layout"], profile["compile_tools"]
    prefix = absolute(layout["compile_tools"])
    env = dict(os.environ, WAFLOCK=tools["waf_lock"])
    sources = absolute(layout["sources"])
    root = absolute(layout["root"])
    run(["./waf", "configure", "-o", tools["waf_output"]] + tools["waf_options"] + [
        "--openusd-source-root=%s" % (sources / "openusd"),
        "--openusd-build-root=%s" % (root / "openusd-build"),
        "--openusd-install-root=%s" % absolute(layout["openusd_install"]),
        "--onetbb-source-root=%s" % (sources / "onetbb"),
        "--onetbb-build-root=%s" % (root / "onetbb-build"),
        "--onetbb-install-root=%s" % absolute(layout["onetbb_install"]),
        "--prefix=%s" % prefix], cwd=ROOT, env=env)
    run(["./waf", "install", "-j%d" % jobs], cwd=ROOT, env=env)
    revision, dirty = git_revision(ROOT)
    (prefix / ".pbrt-map-provisioned.json").write_text(json.dumps(
        {"revision": revision, "dirty_checkout": dirty}) + "\n")


def toolchain_document(profile, linked):
    layout = profile["layout"]
    ktx = linked["ktx"]["build"]["binary"]
    compile_tools = absolute(layout["compile_tools"])
    return {"schema": "pbrt-map-toolchain/v1", "profile": profile["id"],
            "blender": profile["host_packages"]["blender"]["executable"],
            "ocio": profile["fixtures"]["ocio"]["path"],
            "usd_python": profile["toolchain"]["usd_python"],
            "usd_pythonpath": str(absolute(layout["openusd_install"]) / "lib/python"),
            "compile_tools": str(compile_tools),
            "bsp2tool": str(compile_tools / "bsp2tool"),
            "ktx": str(absolute(layout["ktx_build"]) / ktx),
            "runtime": profile["runtime"]["runtime"],
            "client_build": profile["runtime"]["client_build"]}


def oidn_version(library):
    handle = ctypes.CDLL(library)
    handle.oidnNewDevice.restype = ctypes.c_void_p
    handle.oidnNewDevice.argtypes = [ctypes.c_int]
    handle.oidnGetDeviceInt.restype = ctypes.c_int
    handle.oidnGetDeviceInt.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
    handle.oidnReleaseDevice.argtypes = [ctypes.c_void_p]
    device = handle.oidnNewDevice(1)
    if not device:
        raise RuntimeError("OIDN could not create a CPU device")
    try:
        value = handle.oidnGetDeviceInt(device, b"version")
    finally:
        handle.oidnReleaseDevice(device)
    return "%d.%d.%d" % (value // 10000, value // 100 % 100, value % 100)


def load(path, profile_path=PROFILE):
    """Resolve and verify a toolchain file; raise SystemExit listing every problem."""
    toolchain = json.loads(Path(path).read_text())
    if toolchain.get("schema") != "pbrt-map-toolchain/v1":
        raise SystemExit("toolchain schema must be pbrt-map-toolchain/v1")
    profile, linked = load_profiles(profile_path)
    problems = []
    keys = ("blender", "ocio", "usd_python", "usd_pythonpath", "compile_tools", "ktx",
            "runtime", "client_build")
    for key in keys:
        value = toolchain.get(key)
        if not value:
            problems.append("%s: missing" % key)
            continue
        resolved = shutil.which(value) if key == "blender" and "/" not in value else str(
            absolute(value))
        if not resolved or not Path(resolved).exists():
            problems.append("%s: %s does not exist" % (key, value))
        toolchain[key] = resolved
    toolchain["bsp2tool"] = str(absolute(toolchain.get("bsp2tool") or
                                         Path(toolchain["compile_tools"] or "") / "bsp2tool"))
    if problems:
        raise SystemExit("toolchain problems:\n  " + "\n  ".join(problems))
    tools = Path(toolchain["compile_tools"])
    for name in profile["compile_tools"]["required"]:
        if name != "bsp2tool" and not (tools / name).exists():
            problems.append("compile_tools: %s lacks %s" % (tools, name))
    usage = subprocess.run([toolchain["bsp2tool"]], capture_output=True, text=True)
    for command in profile["compile_tools"]["bsp2tool_commands"]:
        if command not in usage.stdout + usage.stderr:
            problems.append("bsp2tool: %s lacks %s" % (toolchain["bsp2tool"], command))
    blender = subprocess.run([toolchain["blender"], "--version"], capture_output=True, text=True)
    match = re.search(r"^Blender (\S+)", blender.stdout, re.M)
    expected = profile["host_packages"]["blender"]["version"]
    if not match or match.group(1) != expected:
        problems.append("blender: version %s, profile requires %s" %
                        (match.group(1) if match else "unknown", expected))
    tag = linked["openusd"]["dependencies"]["openusd"]["tag"]
    usd = subprocess.run([toolchain["usd_python"], "-c",
                          "from pxr import Usd; print('%d.%d.%d' % Usd.GetVersion())"],
                         env=dict(os.environ, PYTHONPATH=toolchain["usd_pythonpath"]),
                         capture_output=True, text=True)
    if usd.returncode or usd.stdout.strip() != "0." + tag.lstrip("v"):
        problems.append("openusd: python reports %r, profile requires %s" %
                        (usd.stdout.strip() or usd.stderr.strip()[-120:], tag))
    revision = linked["ktx"]["dependencies"]["ktx_software"]["revision"]
    ktx_version = subprocess.run([toolchain["ktx"], "--version"], capture_output=True, text=True)
    if revision[:7] not in ktx_version.stdout + ktx_version.stderr:
        problems.append("ktx: %s is not revision %s" % (toolchain["ktx"], revision[:7]))
    oidn = profile["host_packages"]["openimagedenoise"]
    try:
        found = oidn_version(oidn["library"])
        if found != oidn["version"]:
            problems.append("openimagedenoise: %s, profile requires %s" % (found, oidn["version"]))
    except OSError as error:
        problems.append("openimagedenoise: %s" % error)
    if problems:
        raise SystemExit("toolchain problems:\n  " + "\n  ".join(problems))
    toolchain["profile_id"] = profile["id"]
    return toolchain


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    commands = parser.add_subparsers(dest="command", required=True)
    provision = commands.add_parser("provision")
    provision.add_argument("--mirror", action="append", default=[],
                           help="NAME=PATH local clone to fetch a pinned source from")
    provision.add_argument("--jobs", type=int, default=os.cpu_count() or 8)
    provision.add_argument("--steps", default=",".join(STEPS))
    check = commands.add_parser("check")
    check.add_argument("--toolchain", type=Path)
    client = commands.add_parser("configure-client",
                                 help="enable the pinned KTX lightmap reader in a client tree")
    client.add_argument("--build", type=Path, default=Path("build"),
                        help="configured native Vulkan Waf output tree (default: build)")
    args = parser.parse_args()
    profile, linked = load_profiles()
    if args.command == "configure-client":
        configure_client(profile, linked, args.build)
        return
    toolchain_file = absolute(profile["layout"]["toolchain_file"])
    if args.command == "check":
        load(args.toolchain or toolchain_file)
        print("toolchain ok (%s)" % profile["id"])
        return
    steps = args.steps.split(",")
    unknown = set(steps) - set(STEPS)
    if unknown:
        parser.error("unknown steps: " + ", ".join(sorted(unknown)))
    mirrors = dict(item.split("=", 1) for item in args.mirror)
    sources = provision_sources(profile, linked, mirrors) if (
        {"sources", "openusd", "ktx", "ktx-reader"} & set(steps)) else None
    if "openusd" in steps:
        provision_openusd(profile, linked, sources, args.jobs)
    if "ktx" in steps:
        provision_ktx(profile, linked, sources, args.jobs)
    if "ktx-reader" in steps:
        provision_ktx_reader(profile, linked, sources, args.jobs)
    if "compile-tools" in steps:
        provision_compile_tools(profile, args.jobs)
    if "write" in steps:
        toolchain_file.parent.mkdir(parents=True, exist_ok=True)
        toolchain_file.write_text(json.dumps(toolchain_document(profile, linked), indent=2) + "\n")
        load(toolchain_file)
        print("wrote and verified " + str(toolchain_file))


if __name__ == "__main__":
    main()
