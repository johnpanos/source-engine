#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Real-tool positive and negative checks for the RFC 0008 KTX2 packer."""

import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import tempfile

import ktx2_probe as probe


ROOT = Path(__file__).resolve().parents[2]
PACKER = ROOT / "tools/texture/ktx2_pack.py"
sys.path.insert(0, str(ROOT / "tools" / "texture"))
import ktx2_select as selector  # noqa: E402


def run_packer(tool, kind, target, master, destination, plan=None):
    args = [sys.executable, str(PACKER), "--ktx", str(tool),
            "--texture-class", kind, "--master", str(master), "--out", str(destination)]
    args.extend(["--selection-plan", str(plan)] if plan else ["--target", target])
    return subprocess.run(args,
                          capture_output=True, text=True, timeout=120)


def check(condition, message):
    if not condition:
        raise probe.ProbeFailure(message)


def fixture(tool, directory, kind, settings):
    raw = directory / (kind + ".raw")
    plain = directory / (kind + "-plain.ktx2")
    master = directory / (kind + "-master.ktx2")
    raw.write_bytes(probe.fixture_bytes(kind))
    probe.run([tool, "create", "--format", settings["create_format"], "--raw",
               "--width", 8, "--height", 8, raw, plain])
    codec = "uastc-hdr-4x4" if kind == "hdr" else "uastc-ldr-4x4"
    probe.run([tool, "encode", "--codec", codec, plain, master])
    return master


def run(profile, source, build):
    tool, revision, version = probe.check_build(profile, source, build)
    results = {}
    with tempfile.TemporaryDirectory(prefix="rfc0008-ktx-pack-") as scratch:
        root = Path(scratch)
        for kind, settings in profile["texture_classes"].items():
            master = fixture(tool, root, kind, settings)
            for target, expected_format in settings["formats"].items():
                destination = root / (kind + "-" + target + "-package.ktx2")
                command = run_packer(tool, kind, target, master, destination)
                check(command.returncode == 0, "packer failed for %s/%s: %s" %
                      (kind, target, command.stderr[:1000]))
                receipt = json.loads(command.stdout)
                info = probe.inspect(tool, destination, expected_format)
                check(receipt["package_sha256"] == info["sha256"],
                      "packer receipt hash differs from package")
                check(receipt["source_sha256"] == probe.sha256(master),
                      "packer receipt hash differs from master")
                results[kind + ":" + target] = info["sha256"]

        product, _, product_path, host_path = selector.load_profiles(
            selector.DEFAULT_PRODUCT_PROFILE)
        device_formats = {}
        for kind, choices in product["intent"]["texture_pipeline"]["format_preferences"].items():
            vk_format = profile["texture_classes"][kind]["formats"][choices[0]]
            device_formats[vk_format] = {"VkFormatProperties": {
                "optimalTilingFeatures": sorted(selector.REQUIRED_FEATURES)}}
        device_report = {"$schema": "https://schema.khronos.org/vulkan/profiles-0.8-latest.json",
                         "capabilities": {"device": {"formats": device_formats,
                             "properties": {"VkPhysicalDeviceProperties": {
                                 "deviceName": "packer conformance GPU"}}}}}
        plan_data = selector.select(product, profile, device_report,
                                    selector.sha256(product_path), selector.sha256(host_path),
                                    "synthetic-device")
        check(plan_data["outcome"] == "pass", "synthetic device selection failed")
        plan = root / "selection.json"
        plan.write_text(json.dumps(plan_data))
        selected = root / "selected-package.ktx2"
        attempt = run_packer(tool, "base-color", None, root / "base-color-master.ktx2",
                             selected, plan)
        check(attempt.returncode == 0 and json.loads(attempt.stdout)["target"] == "bc7",
              "packer did not consume the product selection plan")

        sentinel = root / "existing.ktx2"
        sentinel.write_bytes(b"existing output")
        base_master = root / "base-color-master.ktx2"
        hdr_master = root / "hdr-master.ktx2"
        negatives = (
            ("wrong-transfer", "normal", "bc5", base_master, "transfer function"),
            ("wrong-codec", "base-color", "bc7", hdr_master, "UASTC master"),
            ("undeclared-target", "base-color", "bc6hu", base_master, "not declared"),
        )
        for label, kind, target, master, diagnostic in negatives:
            attempt = run_packer(tool, kind, target, master, sentinel)
            check(attempt.returncode > 0 and diagnostic in attempt.stderr and
                  sentinel.read_bytes() == b"existing output",
                  "%s failed to reject without mutating destination" % label)
        corrupt = root / "corrupt-master.ktx2"
        content = bytearray(base_master.read_bytes())
        content[0] ^= 1
        corrupt.write_bytes(content)
        attempt = run_packer(tool, "base-color", "bc7", corrupt, sentinel)
        check(attempt.returncode > 0 and "command failed" in attempt.stderr and
              sentinel.read_bytes() == b"existing output",
              "corrupt input failed to reject without mutating destination")
        bad_tool = root / "bad-ktx"
        bad_tool.write_text("#!/usr/bin/env python3\n"
                            "import subprocess, sys\n"
                            "args = sys.argv[1:]\n"
                            "if args[:1] == ['transcode']:\n"
                            "    args[args.index('bc7')] = 'bc3'\n"
                            "raise SystemExit(subprocess.call([%s, *args]))\n" %
                            json.dumps(str(tool)))
        bad_tool.chmod(0o755)
        attempt = run_packer(bad_tool, "base-color", "bc7", base_master, sentinel)
        check(attempt.returncode > 0 and "format differs" in attempt.stderr and
              sentinel.read_bytes() == b"existing output",
              "bad transcode provider failed to reject without mutating destination")
        plan_data["product_profile_sha256"] = "stale"
        plan.write_text(json.dumps(plan_data))
        attempt = run_packer(tool, "base-color", None, base_master, sentinel, plan)
        check(attempt.returncode > 0 and "different product profile" in attempt.stderr and
              sentinel.read_bytes() == b"existing output",
              "stale selection plan failed to reject without mutating destination")
        check(not list(root.glob(".ktx2-pack-*")), "packer left temporary output")
    return {"outcome": "pass", "profile": profile["id"],
            "ktx_revision": revision, "ktx_version": version,
            "profile_sha256": probe.sha256(probe.DEFAULT_PROFILE),
            "packer_sha256": hashlib.sha256(PACKER.read_bytes()).hexdigest(),
            "positive_cases": results, "selection_plan_cases": 1,
            "negative_cases": 6}


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", type=Path, required=True)
    parser.add_argument("--build", type=Path, required=True)
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args(argv)
    try:
        evidence = run(probe.profile_at(probe.DEFAULT_PROFILE), args.source, args.build)
        code = 0
    except (OSError, KeyError, TypeError, ValueError, subprocess.SubprocessError) as error:
        evidence = {"outcome": "fail", "error": str(error)}
        code = 1
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(evidence, indent=2, sort_keys=True) + "\n")
    print("KTX2 pack conformance: %s (%s)" % (evidence["outcome"], args.out))
    return code


if __name__ == "__main__":
    raise SystemExit(main())
