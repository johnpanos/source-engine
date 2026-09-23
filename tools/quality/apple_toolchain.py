#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""R03 native Apple C++20 compiler/library and static-link probes.

Reads quality/toolchain/apple_profiles.json. macOS runs its binary; iOS device
and simulator targets are compiled, archived and linked, with device runtime
left to native product gates. No engine product support is inferred here.
"""

import argparse
import datetime
import json
import os
from pathlib import Path
import platform
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]
PROFILE = ROOT / "quality/toolchain/apple_profiles.json"
PROBE = ROOT / "quality/toolchain/probes/cxx20.cpp"
EVIDENCE_SCHEMA = "apple-toolchain-evidence/v1"

sys.path.insert(0, str(ROOT / "tools/quality"))
from conformance import source_identity  # noqa: E402
import toolchain_policy  # noqa: E402


class ProbeError(Exception):
    pass


def command(argv, env, timeout=120):
    result = subprocess.run(argv, env=env, capture_output=True, text=True, timeout=timeout)
    if result.returncode != 0:
        raise ProbeError("%s exited %d: %s" % (
            " ".join(argv), result.returncode, (result.stdout + result.stderr)[-800:]))
    return (result.stdout + result.stderr).strip()


def check_profile(data):
    if data.get("schema") != "source-apple-toolchain-profiles/v1":
        raise ProbeError("unsupported Apple toolchain profile schema")
    if data.get("dialect_policy") != toolchain_policy.POLICY_PATH.replace(os.sep, "/"):
        raise ProbeError("Apple profile does not name the authoritative dialect policy")
    if data.get("standard_library") != "libc++":
        raise ProbeError("Apple probe requires the declared libc++ library")
    profiles = data.get("profiles", [])
    if len(profiles) != 3 or len({p.get("id") for p in profiles}) != 3:
        raise ProbeError("Apple profile must declare three distinct targets")
    if {p.get("sdk") for p in profiles} != {"macosx", "iphoneos", "iphonesimulator"}:
        raise ProbeError("Apple profile is missing a required SDK target")


def check(data, out_dir, env):
    if platform.system() != "Darwin" or platform.machine() != "arm64":
        raise ProbeError("Apple native probe requires an arm64 macOS runner")
    xcode = data["xcode"]
    developer_dir = Path(xcode["developer_dir"])
    if not developer_dir.is_dir():
        raise ProbeError("pinned Xcode is absent: %s" % developer_dir)
    env = dict(env, DEVELOPER_DIR=str(developer_dir))
    observed = command(["xcodebuild", "-version"], env)
    if observed.splitlines() != ["Xcode " + xcode["version"], "Build version " + xcode["build"]]:
        raise ProbeError("Xcode pin mismatch: %s" % observed)

    results = []
    for profile in data["profiles"]:
        sdk = profile["sdk"]
        sdk_version = command(["xcrun", "--sdk", sdk, "--show-sdk-version"], env)
        if sdk_version != profile["sdk_version"]:
            raise ProbeError("%s SDK version %s differs from %s" % (
                sdk, sdk_version, profile["sdk_version"]))
        sdk_path = command(["xcrun", "--sdk", sdk, "--show-sdk-path"], env)
        compiler = command(["xcrun", "--sdk", sdk, "--find", "clang++"], env)
        libtool = command(["xcrun", "--sdk", sdk, "--find", "libtool"], env)
        stem = profile["id"]
        obj = out_dir / (stem + ".o")
        archive = out_dir / (stem + ".a")
        exe = out_dir / (stem + ".probe")
        flags = ["-std=c++20", "-stdlib=libc++", "-target", profile["target"],
                 "-isysroot", sdk_path]
        commands = [
            [compiler, *flags, "-c", str(PROBE), "-o", str(obj)],
            [libtool, "-static", "-o", str(archive), str(obj)],
            [compiler, *flags, str(archive), "-o", str(exe)],
        ]
        for argv in commands:
            command(argv, env)
        archs = command(["lipo", "-archs", str(exe)], env).split()
        if archs != [profile["arch"]]:
            raise ProbeError("%s linked architecture %s" % (stem, " ".join(archs)))
        runtime = None
        if profile["execute"]:
            runtime = command([str(exe)], env)
            if "cxx20-probe ok __cplusplus=202002" not in runtime:
                raise ProbeError("%s ran without the expected C++20 result" % stem)
        results.append({"profile": stem, "sdk": sdk, "sdk_version": sdk_version,
                        "compiler": compiler, "flags": flags, "commands": commands,
                        "linked_arch": archs, "runtime": runtime,
                        "decision": "pass"})
    return observed, results


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--out", default="quality-results/apple-cxx20.json")
    args = parser.parse_args(argv)
    out = Path(args.out)
    if not out.is_absolute():
        out = ROOT / out
    evidence = {"schema": EVIDENCE_SCHEMA,
                "generated_at": datetime.datetime.now(datetime.timezone.utc).isoformat(),
                "identity": source_identity(str(ROOT)), "profile": str(PROFILE.relative_to(ROOT)),
                "xcode": None, "results": [], "errors": [], "decision": "incomplete"}
    try:
        data = json.loads(PROFILE.read_text(encoding="utf-8"))
        check_profile(data)
        with tempfile.TemporaryDirectory(prefix="apple-toolchain-") as scratch:
            evidence["xcode"], evidence["results"] = check(data, Path(scratch), os.environ)
        evidence["decision"] = "pass"
    except (OSError, json.JSONDecodeError, ProbeError, subprocess.TimeoutExpired) as error:
        evidence["errors"].append(str(error))
        evidence["decision"] = "fail"
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(json.dumps(evidence, indent=1) + "\n", encoding="utf-8")
    print("Apple toolchain: %d/%d profiles -> %s (%s)" % (
        len(evidence["results"]), 3, evidence["decision"], out))
    for error in evidence["errors"]:
        print("FAIL " + error, file=sys.stderr)
    return 0 if evidence["decision"] == "pass" else 1


if __name__ == "__main__":
    raise SystemExit(main())
