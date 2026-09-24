#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Build the pinned protobuf/abseil prefix for the engine debug API.

The engine links C++ with _GLIBCXX_USE_CXX11_ABI=0 (wscript), so system protobuf
packages cannot be used. This script fetches the archives pinned in
quality/product_profiles/protobuf-linux-deps.json (verified by the shared
product_profile helpers), builds static libraries plus a host protoc with the
profile's flags, and installs them into a prefix keyed by every build input.

  build        fetch, verify and build (cached by stamp), then publish the
               stable view build-deps/debugapi/: prefix symlink, generated C++
               and compile.rsp/link.rsp for direct compiles (conformance suites)
  gen-python   regenerate tools/debugapi/generated/ with the pinned protoc;
               --check fails instead of writing when the checked-in copy drifts
"""

import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile


ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools" / "quality"))
import product_profile  # noqa: E402

DEFAULT_PROFILE = ROOT / "quality/product_profiles/protobuf-linux-deps.json"
DEFAULT_OUT = ROOT / "build-deps"
PROTO_ROOT = ROOT / "debugapi" / "proto"
PROTO_FILES = ("source/debug/v1/debug_api.proto",)
STAMP_SCHEMA = "debugapi-deps-stamp/v1"
VIEW_SCHEMA = "debugapi-deps-view/v1"
PYTHON_OUT = ROOT / "tools" / "debugapi" / "generated"
# Flags every consumer of the pinned prefix must share with it.
CONSUMER_DEFINES = ("-D_GLIBCXX_USE_CXX11_ABI=0",)


class DepsError(RuntimeError):
    pass


def load_profile(path):
    profile = json.loads(Path(path).read_text())
    if profile.get("schema") != "source-host-tool-profile/v1":
        raise DepsError("unsupported dependency profile schema")
    for name in ("protobuf", "abseil"):
        if name not in profile["dependencies"]:
            raise DepsError("profile does not pin %s" % name)
    return profile


def compiler_version(cxx):
    try:
        return subprocess.run([cxx, "-dumpfullversion"], check=True, capture_output=True,
                              text=True).stdout.strip()
    except (OSError, subprocess.CalledProcessError) as error:
        raise DepsError("cannot query %s: %s" % (cxx, error)) from error


def build_key(profile, cxx_version):
    """Every input that changes the produced prefix participates in the key."""
    material = {
        "recipe": profile["build"]["recipe"],
        "archives": {name: dep["sha256"] for name, dep in sorted(profile["dependencies"].items())},
        "cxx_flags": profile["build"]["cxx_flags"],
        "cmake_options": profile["build"]["cmake_options"],
        "cxx_version": cxx_version,
    }
    encoded = json.dumps(material, sort_keys=True).encode()
    return hashlib.sha256(encoded).hexdigest(), material


def prefix_for(profile, out, key):
    version = profile["dependencies"]["protobuf"]["prefix"]
    return Path(out) / ("%s-abi0-%s" % (version, key[:12]))


def read_stamp(prefix):
    stamp = prefix / "debugapi-deps-stamp.json"
    if not stamp.is_file():
        return None
    try:
        return json.loads(stamp.read_text())
    except ValueError:
        return None


def verify_prefix(profile, prefix, key):
    stamp = read_stamp(prefix)
    if not stamp or stamp.get("schema") != STAMP_SCHEMA or stamp.get("key") != key:
        return False
    return all((prefix / path).is_file() for path in profile["build"]["required_outputs"])


def run(command, cwd=None, env=None):
    print("+ " + " ".join(str(part) for part in command), file=sys.stderr, flush=True)
    result = subprocess.run([str(part) for part in command], cwd=cwd, env=env)
    if result.returncode != 0:
        raise DepsError("command failed (%d): %s" % (result.returncode, command[0]))


def build(profile, out, jobs=None, cxx="g++", cc="gcc"):
    out = Path(out).absolute()
    key, material = build_key(profile, compiler_version(cxx))
    prefix = prefix_for(profile, out, key)
    if verify_prefix(profile, prefix, key):
        return prefix
    if prefix.exists():
        raise DepsError("prefix exists but its stamp does not match; remove it: %s" % prefix)

    cache = out / "cache"
    sources = {name: product_profile.fetch_dependency(profile, cache, name)
               for name in ("abseil", "protobuf")}

    # Installed pkg-config/CMake files embed the install prefix, so install in
    # place; the stamp is written last and is the only completion marker.
    work = Path(tempfile.mkdtemp(prefix=".debugapi-deps-", dir=out))
    completed = False
    try:
        options = dict(profile["build"]["cmake_options"])
        options["ABSL_ROOT_DIR"] = str(sources["abseil"])
        options["CMAKE_INSTALL_PREFIX"] = str(prefix)
        options["CMAKE_CXX_FLAGS"] = " ".join(profile["build"]["cxx_flags"])
        options["CMAKE_C_FLAGS"] = " ".join(flag for flag in profile["build"]["cxx_flags"]
                                            if not flag.startswith("-fvisibility-inlines"))
        options["CMAKE_CXX_COMPILER"] = shutil.which(cxx) or cxx
        options["CMAKE_C_COMPILER"] = shutil.which(cc) or cc
        configure = ["cmake", "-S", sources["protobuf"], "-B", work / "build",
                     "-G", profile["build"]["generator"]]
        configure += ["-D%s=%s" % item for item in sorted(options.items())]
        run(configure)
        build_command = ["cmake", "--build", work / "build"]
        if jobs:
            build_command += ["--parallel", str(jobs)]
        run(build_command)
        run(["cmake", "--install", work / "build"])
        for path in profile["build"]["required_outputs"]:
            if not (prefix / path).is_file():
                raise DepsError("build did not produce %s" % path)
        stamp = {"schema": STAMP_SCHEMA, "key": key, "inputs": material,
                 "profile": profile["id"]}
        (prefix / "debugapi-deps-stamp.json").write_text(json.dumps(stamp, indent=2,
                                                                    sort_keys=True) + "\n")
        completed = True
    finally:
        shutil.rmtree(work, ignore_errors=True)
        if not completed and prefix.exists():
            shutil.rmtree(prefix, ignore_errors=True)
    return prefix


def run_protoc(prefix, language_flag, out):
    """Generate code for the schema with the pinned protoc into a fresh `out`."""
    protoc = Path(prefix) / "bin" / "protoc"
    if not protoc.is_file():
        raise DepsError("pinned protoc missing: %s" % protoc)
    out.mkdir(parents=True)
    run([protoc, "--proto_path", PROTO_ROOT, "%s=%s" % (language_flag, out)]
        + [PROTO_ROOT / path for path in PROTO_FILES])


def pkg_config_libs(prefix):
    env = dict(os.environ, PKG_CONFIG_PATH=str(Path(prefix) / "lib" / "pkgconfig"),
               PKG_CONFIG_LIBDIR=str(Path(prefix) / "lib" / "pkgconfig"))
    try:
        result = subprocess.run(["pkg-config", "--static", "--libs", "protobuf"], check=True,
                                capture_output=True, text=True, env=env)
    except (OSError, subprocess.CalledProcessError) as error:
        raise DepsError("pkg-config could not resolve the pinned protobuf: %s" % error) from error
    return result.stdout.split()


def publish(prefix, out):
    """Atomically replace build-deps/debugapi/ with a view of `prefix`."""
    out = Path(out).absolute()
    view = out / "debugapi"
    staging = Path(tempfile.mkdtemp(prefix=".debugapi-view-", dir=out))
    try:
        (staging / "prefix").symlink_to(os.path.relpath(prefix, view))
        run_protoc(prefix, "--cpp_out", staging / "gen" / "cpp")
        compile_flags = list(CONSUMER_DEFINES) + [
            "-I%s" % (Path(prefix) / "include"), "-I%s" % (view / "gen" / "cpp")]
        (staging / "compile.rsp").write_text("\n".join(compile_flags) + "\n")
        (staging / "link.rsp").write_text("\n".join(pkg_config_libs(prefix)) + "\n")
        (staging / "view.json").write_text(json.dumps(
            {"schema": VIEW_SCHEMA, "prefix": str(prefix),
             "stamp": read_stamp(Path(prefix))["key"],
             "proto_files": list(PROTO_FILES)}, indent=2, sort_keys=True) + "\n")
        retired = None
        if view.exists() or view.is_symlink():
            retired = Path(tempfile.mkdtemp(prefix=".debugapi-retired-", dir=out))
            view.rename(retired / "view")
        staging.rename(view)
        if retired is not None:
            shutil.rmtree(retired, ignore_errors=True)
    finally:
        if staging.exists():
            shutil.rmtree(staging, ignore_errors=True)
    return view


def gen_python(prefix, check=False):
    """Checked-in Python bindings; --check compares instead of writing."""
    staging = Path(tempfile.mkdtemp(prefix=".debugapi-python-"))
    try:
        run_protoc(prefix, "--python_out", staging / "out")
        generated = {path.relative_to(staging / "out").as_posix(): path.read_bytes()
                     for path in sorted((staging / "out").rglob("*.py"))}
        existing = {}
        if PYTHON_OUT.is_dir():
            existing = {path.relative_to(PYTHON_OUT).as_posix(): path.read_bytes()
                        for path in sorted(PYTHON_OUT.rglob("*.py"))
                        if path.name != "__init__.py" or path.parent != PYTHON_OUT}
        if check:
            if generated != existing:
                changed = sorted(set(generated) ^ set(existing)
                                 | {k for k in generated if existing.get(k) != generated[k]})
                raise DepsError("generated Python bindings drifted: %s" % ", ".join(changed))
            return PYTHON_OUT
        for relative in existing:
            if relative not in generated:
                (PYTHON_OUT / relative).unlink()
        for relative, content in generated.items():
            target = PYTHON_OUT / relative
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(content)
        return PYTHON_OUT
    finally:
        shutil.rmtree(staging, ignore_errors=True)


def verify_consumer_prefix(prefix, cxx, profile_path=DEFAULT_PROFILE):
    """What a build configuration needs from a verified pinned prefix.

    The prefix must carry a stamp whose key matches the profile and `cxx`;
    otherwise it was built from other inputs and is refused.
    """
    profile = load_profile(profile_path)
    key, _ = build_key(profile, compiler_version(cxx))
    prefix = Path(prefix).resolve()
    if not verify_prefix(profile, prefix, key):
        raise DepsError("%s is not the pinned protobuf prefix for %s with %s; run "
                        "python3 tools/debugapi/build_deps.py build" % (prefix, profile["id"], cxx))
    libs = [flag[2:] for flag in pkg_config_libs(prefix) if flag.startswith("-l")]
    return {"prefix": str(prefix), "protoc": str(prefix / "bin" / "protoc"),
            "include": str(prefix / "include"), "libdir": str(prefix / "lib"), "libs": libs}


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("action", choices=("build", "gen-python"))
    parser.add_argument("--profile", type=Path, default=DEFAULT_PROFILE)
    parser.add_argument("--out-dir", type=Path, default=DEFAULT_OUT,
                        help="dependency cache and prefix root (default: build-deps/)")
    parser.add_argument("--jobs", type=int)
    parser.add_argument("--cxx", default=os.environ.get("CXX", "g++"))
    parser.add_argument("--cc", default=os.environ.get("CC", "gcc"))
    parser.add_argument("--check", action="store_true",
                        help="gen-python: fail if the checked-in bindings differ")
    args = parser.parse_args(argv)
    try:
        profile = load_profile(args.profile)
        prefix = build(profile, args.out_dir, args.jobs, args.cxx, args.cc)
        if args.action == "build":
            print(publish(prefix, args.out_dir))
        else:
            print(gen_python(prefix, args.check))
        return 0
    except (DepsError, product_profile.ProfileError, OSError, KeyError) as error:
        print("build_deps: error: %s" % error, file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
