#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Pinned KTX-Software encode/transcode feasibility for RFC 0008 F3.

This is a host-tool and CPU decode check. It does not certify device format
support, engine/Hammer readers, or GPU pixels.
"""

import argparse
import hashlib
import json
import math
from pathlib import Path
import platform
import re
import struct
import subprocess
import tempfile


ROOT = Path(__file__).resolve().parents[2]
DEFAULT_PROFILE = ROOT / "quality/product_profiles/ktx2-linux-tools.json"


class ProbeFailure(ValueError):
    pass


def run(command, *, expect_success=True):
    result = subprocess.run([str(part) for part in command], capture_output=True,
                            text=True, timeout=120)
    if (expect_success and result.returncode != 0) or \
            (not expect_success and result.returncode <= 0):
        raise ProbeFailure("command returned %d: %s\n%s" %
                           (result.returncode, " ".join(map(str, command)), result.stderr[:1000]))
    return result.stdout


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def compare_bytes(expected, actual, tolerance):
    if len(expected) != len(actual):
        raise ProbeFailure("decoded LDR byte count differs")
    error = max((abs(a - b) for a, b in zip(expected, actual)), default=0)
    if error > tolerance:
        raise ProbeFailure("decoded LDR channel error %d exceeds %d" % (error, tolerance))
    return error


def compare_half(expected, actual, tolerance):
    if len(expected) != len(actual) or len(actual) % 2:
        raise ProbeFailure("decoded HDR byte count differs")
    source = (value[0] for value in struct.iter_unpack("<e", expected))
    decoded = (value[0] for value in struct.iter_unpack("<e", actual))
    error = 0.0
    for authored, result in zip(source, decoded):
        if not math.isfinite(authored) or not math.isfinite(result):
            raise ProbeFailure("decoded HDR channel is nonfinite")
        error = max(error, abs(authored - result))
    if error > tolerance:
        raise ProbeFailure("decoded HDR channel error %g exceeds %g" % (error, tolerance))
    return error


def profile_at(path):
    profile = json.loads(Path(path).read_text())
    if profile.get("schema") != "source-host-tool-profile/v1":
        raise ProbeFailure("unsupported host-tool profile")
    revision = profile["dependencies"]["ktx_software"]["revision"]
    if not re.fullmatch(r"[0-9a-f]{40}", revision):
        raise ProbeFailure("KTX-Software revision must be a full Git SHA")
    fixture = profile["fixture"]
    if (fixture["width"], fixture["height"]) != (8, 8):
        raise ProbeFailure("this fixture requires an 8x8 image")
    if set(profile["texture_classes"]) != {"base-color", "normal", "mask", "hdr"}:
        raise ProbeFailure("profile must declare all four texture classes")
    return profile


def check_build(profile, source, build):
    source = Path(source).resolve()
    build = Path(build).resolve()
    revision = run(["git", "-C", source, "rev-parse", "HEAD"]).strip()
    if revision != profile["dependencies"]["ktx_software"]["revision"]:
        raise ProbeFailure("KTX-Software source revision differs from profile")
    if run(["git", "-C", source, "status", "--porcelain", "--untracked-files=no"]).strip():
        raise ProbeFailure("KTX-Software tracked source has local changes")
    target = profile["target"]
    if platform.system().lower() != target["os"] or platform.machine() != target["architecture"]:
        raise ProbeFailure("host OS/architecture differs from profile")
    if run([profile["toolchain"]["cxx"], "-dumpfullversion"]).strip() != profile["toolchain"]["version"]:
        raise ProbeFailure("C++ compiler version differs from profile")
    cmake_version = run(["cmake", "--version"]).splitlines()[0].removeprefix("cmake version ")
    if cmake_version != profile["toolchain"]["cmake"]:
        raise ProbeFailure("CMake version differs from profile")
    cache = (build / "CMakeCache.txt").read_text()
    facts = {}
    for line in cache.splitlines():
        if ":" in line and "=" in line and not line.startswith(("#", "//")):
            key, value = line.split("=", 1)
            facts[key.split(":", 1)[0]] = value
    if Path(facts.get("CMAKE_HOME_DIRECTORY", "")).resolve() != source:
        raise ProbeFailure("CMake build uses a different KTX-Software source")
    if facts.get("CMAKE_BUILD_TYPE") != profile["toolchain"]["build_type"]:
        raise ProbeFailure("CMake build type differs from profile")
    if facts.get("CMAKE_GENERATOR") != profile["build"]["generator"]:
        raise ProbeFailure("CMake generator differs from profile")
    build_compiler = facts.get("CMAKE_CXX_COMPILER")
    if not build_compiler or run([build_compiler, "-dumpfullversion"]).strip() != profile["toolchain"]["version"]:
        raise ProbeFailure("CMake build compiler differs from profile")
    for key, value in profile["build"]["cmake_options"].items():
        if facts.get(key) != value:
            raise ProbeFailure("CMake option %s differs from profile" % key)
    tool = build / profile["build"]["binary"]
    if not tool.is_file():
        raise ProbeFailure("KTX CLI binary is missing")
    tool_version = run([tool, "--version"]).strip()
    if revision[:7] not in tool_version:
        raise ProbeFailure("KTX CLI version does not contain pinned revision")
    return tool, revision, tool_version


def fixture_bytes(kind):
    colors = ((255, 0, 0, 255), (0, 255, 0, 255),
              (0, 0, 255, 255), (255, 255, 255, 255))
    normals = ((128, 128), (255, 0), (0, 255), (64, 192))
    masks = (0, 85, 170, 255)
    hdr = (0.5, 1.0, 2.0, 4.0)
    out = bytearray()
    for y in range(8):
        for x in range(8):
            index = (y // 4) * 2 + x // 4
            if kind == "base-color":
                out.extend(colors[index])
            elif kind == "normal":
                out.extend(normals[index])
            elif kind == "mask":
                out.append(masks[index])
            else:
                out.extend(struct.pack("<eeee", hdr[index], hdr[index], hdr[index], 1.0))
    return bytes(out)


def inspect(tool, path, expected_format):
    run([tool, "validate", path])
    info = json.loads(run([tool, "info", "--format", "mini-json", path]))
    header = info["header"]
    if not info["valid"] or header["vkFormat"] != expected_format:
        raise ProbeFailure("KTX validation or VkFormat mismatch: %s" % path.name)
    if (header["pixelWidth"], header["pixelHeight"]) != (8, 8):
        raise ProbeFailure("KTX image dimensions differ from fixture")
    return {"vk_format": header["vkFormat"], "sha256": sha256(path)}


def probe_cohort(tool, root, kind, fixture, settings):
    source_format = settings["create_format"]
    decoded_target = settings["oracle_target"]
    expected = fixture_bytes(kind)
    raw = root / (kind + ".raw")
    original = root / (kind + "-plain.ktx2")
    master = root / (kind + "-uastc.ktx2")
    decoded = root / (kind + "-decoded.ktx2")
    extracted = root / (kind + "-decoded.raw")
    raw.write_bytes(expected)
    run([tool, "create", "--format", source_format, "--raw", "--width", 8,
         "--height", 8, raw, original])
    run([tool, "encode", "--codec", "uastc-hdr-4x4" if kind == "hdr" else "uastc-ldr-4x4",
         original, master])
    run([tool, "transcode", "--target", decoded_target, master, decoded])
    run([tool, "extract", "--raw", decoded, extracted])
    tolerance = fixture["hdr_max_channel_error"] if kind == "hdr" else fixture["ldr_max_channel_error"]
    error = compare_half(expected, extracted.read_bytes(), tolerance) if kind == "hdr" else \
        compare_bytes(expected, extracted.read_bytes(), tolerance)
    formats = {}
    for target, expected_format in settings["formats"].items():
        output = root / (kind + "-" + target + ".ktx2")
        run([tool, "transcode", "--target", target, master, output])
        formats[target] = inspect(tool, output, expected_format)
    return {"master_sha256": sha256(master), "decoded_max_channel_error": error,
            "formats": formats}


def probe(profile, source, build):
    tool, revision, tool_version = check_build(profile, source, build)
    with tempfile.TemporaryDirectory(prefix="rfc0008-ktx-") as directory:
        root = Path(directory)
        cohorts = {kind: probe_cohort(tool, root, kind, profile["fixture"], settings)
                   for kind, settings in profile["texture_classes"].items()}
        invalid = bytearray((root / "base-color-uastc.ktx2").read_bytes())
        invalid[0] ^= 1
        bad_path = root / "bad-magic.ktx2"
        bad_path.write_bytes(invalid)
        diagnostic = run([tool, "validate", bad_path], expect_success=False)
        if "Not a KTX2 file" not in diagnostic:
            raise ProbeFailure("bad magic did not produce the KTX2 identifier diagnostic")
    return {"outcome": "pass", "profile": profile["id"], "source_revision": revision,
            "tool_version": tool_version, "cohorts": cohorts,
            "target_count": sum(len(value["formats"]) for value in cohorts.values()),
            "negative_control": "bad KTX2 magic rejected"}


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--profile", type=Path, default=DEFAULT_PROFILE)
    parser.add_argument("--source", type=Path, required=True)
    parser.add_argument("--build", type=Path, required=True)
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args(argv)
    evidence = {"outcome": "fail"}
    try:
        profile = profile_at(args.profile)
        evidence = probe(profile, args.source, args.build)
        evidence["profile_sha256"] = sha256(args.profile)
        code = 0
    except (OSError, ProbeFailure, KeyError, TypeError, ValueError,
            subprocess.SubprocessError) as error:
        evidence["error"] = str(error)
        code = 1
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(evidence, indent=2, sort_keys=True) + "\n")
    print("KTX2 probe: %s (%s)" % (evidence["outcome"], args.out))
    return code


if __name__ == "__main__":
    raise SystemExit(main())
