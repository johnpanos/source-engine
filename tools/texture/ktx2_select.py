#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Select KTX2 package formats from a product profile and Vulkan device query."""

import argparse
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import tempfile


ROOT = Path(__file__).resolve().parents[2]
DEFAULT_PRODUCT_PROFILE = ROOT / "quality/product_profiles/portal-linux-wayland-native-vulkan.json"
REQUIRED_FEATURES = {"VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT",
                     "VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT",
                     "VK_FORMAT_FEATURE_TRANSFER_DST_BIT"}


class SelectionError(ValueError):
    pass


def sha256(path):
    value = hashlib.sha256()
    with Path(path).open("rb") as stream:
        for chunk in iter(lambda: stream.read(65536), b""):
            value.update(chunk)
    return value.hexdigest()


def load_profiles(product_path):
    product_path = Path(product_path).resolve()
    product = json.loads(product_path.read_text())
    if product.get("schema") != "source-product-profile/v1" or \
            product.get("intent", {}).get("render_provider") != "native-vulkan":
        raise SelectionError("texture selection requires a native Vulkan product profile")
    pipeline = product["intent"]["texture_pipeline"]
    host_path = (ROOT / pipeline["host_tool_profile"]).resolve()
    if not host_path.is_relative_to(ROOT):
        raise SelectionError("host-tool profile must be inside the repository")
    host = json.loads(host_path.read_text())
    if host.get("schema") != "source-host-tool-profile/v1":
        raise SelectionError("unsupported host-tool profile")
    preferences = pipeline["format_preferences"]
    if set(preferences) != set(host["texture_classes"]):
        raise SelectionError("product profile must select every texture class")
    for kind, targets in preferences.items():
        if not targets or len(targets) != len(set(targets)) or \
                any(target not in host["texture_classes"][kind]["formats"] for target in targets):
            raise SelectionError("invalid format preferences for %s" % kind)
    return product, host, product_path, host_path


def select(product, host, report, product_hash, host_hash, report_hash):
    if not str(report.get("$schema", "")).startswith(
            "https://schema.khronos.org/vulkan/profiles-"):
        raise SelectionError("device report is not a Vulkan Profiles export")
    device = report["capabilities"]["device"]
    formats = device["formats"]
    properties = device["properties"]["VkPhysicalDeviceProperties"]
    if not properties.get("deviceName") or not isinstance(formats, dict):
        raise SelectionError("device report lacks identity or format properties")
    selections = {}
    missing = []
    for kind, targets in product["intent"]["texture_pipeline"]["format_preferences"].items():
        checked = []
        chosen = None
        for target in targets:
            vk_format = host["texture_classes"][kind]["formats"][target]
            details = formats.get(vk_format, {})
            features = set(details.get("VkFormatProperties", {}).get(
                "optimalTilingFeatures", []))
            absent = sorted(REQUIRED_FEATURES - features)
            checked.append({"target": target, "vk_format": vk_format,
                            "missing_features": absent})
            if not absent:
                chosen = {"target": target, "vk_format": vk_format}
                break
        selections[kind] = {"selected": chosen, "checked": checked}
        if chosen is None:
            missing.append(kind)
    return {"outcome": "fail" if missing else "pass",
            "product_profile": product["id"], "product_profile_sha256": product_hash,
            "host_tool_profile": host["id"], "host_tool_profile_sha256": host_hash,
            "device_report_sha256": report_hash,
            "device": {key: properties.get(key) for key in
                       ("deviceName", "vendorID", "deviceID", "driverVersion", "apiVersion")},
            "selections": selections, "missing_required_classes": missing}


def query_device(vulkaninfo):
    binary = shutil.which(str(vulkaninfo))
    if binary is None:
        raise SelectionError("vulkaninfo is unavailable")
    with tempfile.TemporaryDirectory(prefix="rfc0008-vulkan-query-") as scratch:
        report_path = Path(scratch) / "device.json"
        result = subprocess.run([binary, "--json", "--output", str(report_path)],
                                capture_output=True, text=True, timeout=60)
        if result.returncode != 0 or not report_path.is_file():
            raise SelectionError("vulkaninfo device query failed: %s" % result.stderr[:1000])
        contents = report_path.read_bytes()
    return json.loads(contents), hashlib.sha256(contents).hexdigest(), sha256(binary)


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--product-profile", type=Path, default=DEFAULT_PRODUCT_PROFILE)
    source = parser.add_mutually_exclusive_group(required=True)
    source.add_argument("--device-report", type=Path)
    source.add_argument("--query-device", action="store_true")
    parser.add_argument("--vulkaninfo", default="vulkaninfo")
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args(argv)
    try:
        product, host, product_path, host_path = load_profiles(args.product_profile)
        if args.query_device:
            report, report_hash, binary_hash = query_device(args.vulkaninfo)
            origin = {"kind": "live-vulkaninfo", "binary_sha256": binary_hash}
        else:
            report = json.loads(args.device_report.read_text())
            report_hash = sha256(args.device_report)
            origin = {"kind": "supplied-report", "path": str(args.device_report)}
        evidence = select(product, host, report, sha256(product_path),
                          sha256(host_path), report_hash)
        evidence["device_query"] = origin
    except (OSError, KeyError, TypeError, ValueError, subprocess.SubprocessError) as error:
        evidence = {"outcome": "fail", "error": str(error)}
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(evidence, indent=2, sort_keys=True) + "\n")
    print("KTX2 format selection: %s (%s)" % (evidence["outcome"], args.out))
    return 0 if evidence["outcome"] == "pass" else 1


if __name__ == "__main__":
    raise SystemExit(main())
