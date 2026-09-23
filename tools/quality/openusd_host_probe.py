#!/usr/bin/env python3
"""Validate the pinned OpenUSD host build and the World Stage fixture."""

import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile


ROOT = Path(__file__).resolve().parents[2]
PROFILE = ROOT / "quality/product_profiles/openusd-linux-tools.json"
FIXTURE = ROOT / "quality/fixtures/worldstage/minimal.usda"
SCHEMA = ROOT / "utils/worldstage/schema.usda"


def digest(data):
    return hashlib.sha256(data).hexdigest()


def run(*args, env=None):
    return subprocess.run(args, capture_output=True, text=True, timeout=120, check=False,
                          env=env)


def checked_revision(path, revision):
    result = run("git", "-C", str(path), "rev-parse", "HEAD")
    if result.returncode or result.stdout.strip() != revision:
        raise ValueError(f"{path} is not the pinned revision")
    result = run("git", "-C", str(path), "status", "--porcelain", "--untracked-files=no")
    if result.returncode or result.stdout.strip():
        raise ValueError(f"{path} has tracked source changes")


def cache_values(path):
    values = {}
    for line in path.read_text(encoding="utf-8").splitlines():
        if line.startswith("//") or line.startswith("#") or "=" not in line:
            continue
        key, value = line.split("=", 1)
        values[key.split(":", 1)[0]] = value
    return values


def require_success(label, result):
    if result.returncode:
        raise ValueError(f"{label} failed: {(result.stdout + result.stderr)[-2000:]}")


def check(args):
    profile_bytes = PROFILE.read_bytes()
    profile = json.loads(profile_bytes)
    if profile.get("schema") != "source-host-tool-profile/v1":
        raise ValueError("the OpenUSD host profile schema is invalid")
    cxx = profile["toolchain"]["cxx"]
    compiler_version = run(cxx, "-dumpfullversion").stdout.strip()
    if compiler_version != profile["toolchain"]["version"]:
        raise ValueError("host compiler version differs from the OpenUSD profile")
    python_version = run(str(args.python), "--version").stdout.strip().removeprefix("Python ")
    if python_version != profile["toolchain"]["python"]:
        raise ValueError("Python version differs from the OpenUSD profile")
    jinja_version = run(str(args.python), "-c", "import jinja2; print(jinja2.__version__)")
    if jinja_version.returncode or jinja_version.stdout.strip() != profile["dependencies"]["jinja2"]["version"]:
        raise ValueError("Jinja2 version differs from the OpenUSD profile")
    checked_revision(args.openusd_source, profile["dependencies"]["openusd"]["revision"])
    checked_revision(args.onetbb_source, profile["dependencies"]["onetbb"]["revision"])
    tbb_cache = cache_values(args.onetbb_build / "CMakeCache.txt")
    if Path(tbb_cache.get("CMAKE_HOME_DIRECTORY", "")) != args.onetbb_source.resolve():
        raise ValueError("the oneTBB build uses another source checkout")
    for option, value in profile["build"]["onetbb_cmake_options"].items():
        if tbb_cache.get(option) != value:
            raise ValueError(f"oneTBB CMake option {option} differs from the profile")
    cache = cache_values(args.openusd_build / "CMakeCache.txt")
    if Path(cache.get("CMAKE_HOME_DIRECTORY", "")) != args.openusd_source.resolve():
        raise ValueError("the OpenUSD build uses another source checkout")
    for option, value in profile["build"]["cmake_options"].items():
        if cache.get(option) != value:
            raise ValueError(f"OpenUSD CMake option {option} differs from the profile")
    if not cache.get("TBB_DIR", "").startswith(str(args.onetbb_install.resolve())):
        raise ValueError("OpenUSD did not use the pinned oneTBB installation")
    if cache.get("Python3_EXECUTABLE") != str(args.python.resolve()):
        raise ValueError("OpenUSD did not use the declared Python interpreter")

    checker = args.openusd_install / "bin/usdchecker"
    cat = args.openusd_install / "bin/usdcat"
    generator = args.openusd_source / "pxr/usd/usd/usdGenSchema.py"
    for binary in (checker, cat, generator):
        if not binary.is_file():
            raise ValueError(f"required OpenUSD tool is missing: {binary}")

    positive = run(str(checker), str(FIXTURE))
    require_success("source fixture usdchecker", positive)
    if "Success!" not in positive.stdout:
        raise ValueError("usdchecker produced incomplete positive output")
    with tempfile.TemporaryDirectory(prefix="rfc0008-usd-probe-") as temporary:
        root = Path(temporary)
        binary_stage = root / "minimal.usdc"
        require_success("USD binary serialization", run(str(cat), str(FIXTURE), "-o", str(binary_stage)))
        require_success("USD binary round-trip checker", run(str(checker), str(binary_stage)))
        malformed = root / "missing-up-axis.usda"
        source = FIXTURE.read_text(encoding="utf-8")
        if source.count('    upAxis = "Z"\n') != 1:
            raise ValueError("fixture no longer has one Z-up declaration")
        malformed.write_text(source.replace('    upAxis = "Z"\n', ""), encoding="utf-8")
        negative = run(str(checker), str(malformed))
        if negative.returncode == 0 or "MissingUpAxisMetadata" not in negative.stdout + negative.stderr:
            raise ValueError("usdchecker missed the absent up-axis negative control")
        env = os.environ.copy()
        env["PYTHONPATH"] = str(args.openusd_install / "lib/python")
        env["LD_LIBRARY_PATH"] = os.pathsep.join(
            (str(args.openusd_install / "lib"), str(args.onetbb_install / "lib64"))
        )
        generated = run(str(args.python), str(generator), str(SCHEMA),
                        str(root / "generated"), env=env)
        require_success("Source schema generation", generated)
        generated_files = sorted(
            {path.name: digest(path.read_bytes()) for path in (root / "generated").iterdir()
             if path.is_file()}.items()
        )
        if not any(name == "generatedSchema.usda" for name, _ in generated_files):
            raise ValueError("usdGenSchema omitted generatedSchema.usda")
        include_root = root / "include/utils"
        include_root.mkdir(parents=True)
        (include_root / "worldstage").symlink_to(root / "generated", target_is_directory=True)
        generated_sources = sorted((root / "generated").glob("*.cpp"))
        generated_sources = [path for path in generated_sources
                             if not path.name.startswith("wrap")]
        if len(generated_sources) != 5:
            raise ValueError("generated Source schema C++ sources are incomplete")
        python_include = cache.get("_Python3_INCLUDE_DIR", "")
        if not python_include:
            raise ValueError("OpenUSD did not record the Python C++ include path")
        compiled = run(
            cxx, "-std=c++20", "-fsyntax-only", "-w",
            "-I" + str(root / "include"),
            "-I" + str(args.openusd_install / "include"),
            "-I" + str(args.onetbb_install / "include"),
            "-I" + python_include,
            *(str(path) for path in generated_sources),
        )
        require_success("generated Source schema C++20 syntax", compiled)
        stage_hash = digest(binary_stage.read_bytes())

    return {
        "status": "pass",
        "profile_sha256": digest(profile_bytes),
        "fixture_sha256": digest(FIXTURE.read_bytes()),
        "schema_sha256": digest(SCHEMA.read_bytes()),
        "usdchecker_sha256": digest(checker.read_bytes()),
        "usdGenSchema_sha256": digest(generator.read_bytes()),
        "usdc_sha256": stage_hash,
        "generated_files": generated_files,
        "generated_cpp20_sources": [path.name for path in generated_sources],
        "negative_cases": ["missing-up-axis"],
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--openusd-source", type=Path, required=True)
    parser.add_argument("--openusd-build", type=Path, required=True)
    parser.add_argument("--openusd-install", type=Path, required=True)
    parser.add_argument("--onetbb-source", type=Path, required=True)
    parser.add_argument("--onetbb-build", type=Path, required=True)
    parser.add_argument("--onetbb-install", type=Path, required=True)
    parser.add_argument("--python", type=Path, default=Path("/usr/bin/python3.12"))
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()
    try:
        result = check(args)
    except (OSError, ValueError, json.JSONDecodeError, subprocess.TimeoutExpired) as error:
        result = {"status": "fail", "reason": str(error)}
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    if result["status"] != "pass":
        print("OpenUSD host probe failed: " + result["reason"], file=sys.stderr)
        return 1
    print("OpenUSD host probe passed: schema generation and stage validation")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
