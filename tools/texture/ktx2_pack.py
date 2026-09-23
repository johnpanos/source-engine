#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Transcode a pinned UASTC KTX2 master to an explicit package format.

The caller chooses the target format from the host-tool profile. Device format
selection belongs to the product profile and is not inferred here.
"""

import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import sys
import tempfile


ROOT = Path(__file__).resolve().parents[2]
DEFAULT_PROFILE = ROOT / "quality/product_profiles/ktx2-linux-tools.json"
DEFAULT_PRODUCT_PROFILE = ROOT / "quality/product_profiles/portal-linux-wayland-native-vulkan.json"


class PackError(ValueError):
    pass


def command(*args):
    result = subprocess.run([str(arg) for arg in args], capture_output=True,
                            text=True, timeout=600)
    if result.returncode != 0:
        raise PackError("command failed (%d): %s" %
                        (result.returncode, result.stderr[:1000]))
    return result.stdout


def digest(path):
    value = hashlib.sha256()
    with Path(path).open("rb") as stream:
        for chunk in iter(lambda: stream.read(65536), b""):
            value.update(chunk)
    return value.hexdigest()


def inspect(tool, path):
    command(tool, "validate", path)
    report = json.loads(command(tool, "info", "--format", "mini-json", path))
    if not report.get("valid") or "header" not in report:
        raise PackError("KTX2 validation did not produce a valid header")
    return report


def selected_target(plan_path, product_path, host_path, profile, texture_class):
    plan = json.loads(Path(plan_path).read_text())
    if plan.get("outcome") != "pass" or plan.get("missing_required_classes"):
        raise PackError("texture selection plan did not pass")
    if plan.get("host_tool_profile_sha256") != digest(host_path) or \
            plan.get("host_tool_profile") != profile["id"]:
        raise PackError("texture selection plan uses a different host-tool profile")
    if plan.get("product_profile_sha256") != digest(product_path):
        raise PackError("texture selection plan uses a different product profile")
    chosen = plan["selections"][texture_class]["selected"]
    if chosen is None:
        raise PackError("texture class has no selected device format")
    target = chosen["target"]
    expected = profile["texture_classes"][texture_class]["formats"].get(target)
    if expected != chosen["vk_format"]:
        raise PackError("selection plan format differs from host-tool profile")
    return target


def package(tool, profile, texture_class, target, master, destination):
    if profile.get("schema") != "source-host-tool-profile/v1":
        raise PackError("unsupported host-tool profile")
    profile_id = profile["id"]
    revision = profile["dependencies"]["ktx_software"]["revision"]
    if not re.fullmatch(r"[0-9a-f]{40}", revision):
        raise PackError("KTX-Software revision must be a full Git SHA")
    if revision[:7] not in command(tool, "--version"):
        raise PackError("KTX CLI does not match the pinned source revision")
    settings = profile["texture_classes"].get(texture_class)
    if settings is None:
        raise PackError("unknown texture class: %s" % texture_class)
    expected_format = settings["formats"].get(target)
    if expected_format is None:
        raise PackError("target %s is not declared for %s" % (target, texture_class))
    master = Path(master).resolve()
    destination = Path(destination).resolve()
    if master == destination:
        raise PackError("source and destination must differ")
    source = inspect(tool, master)
    header = source["header"]
    blocks = source.get("dataFormatDescriptor", {}).get("blocks", [])
    model = blocks[0].get("colorModel") if blocks else None
    expected_model = "KHR_DF_MODEL_UASTC_HDR_4x4" if texture_class == "hdr" else \
        "KHR_DF_MODEL_UASTC"
    if model != expected_model:
        raise PackError("source is not the required UASTC master for %s" % texture_class)
    transfer = blocks[0].get("transferFunction")
    expected_transfer = "KHR_DF_TRANSFER_SRGB" if texture_class == "base-color" else \
        "KHR_DF_TRANSFER_LINEAR"
    if transfer != expected_transfer:
        raise PackError("source transfer function differs from texture class")
    if header["pixelWidth"] < 1 or header["pixelHeight"] < 1:
        raise PackError("source has invalid image dimensions")

    destination.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix=".ktx2-pack-", dir=destination.parent) as scratch:
        temporary = Path(scratch) / "package.ktx2"
        command(tool, "transcode", "--target", target, master, temporary)
        packed = inspect(tool, temporary)
        packed_header = packed["header"]
        if packed_header["vkFormat"] != expected_format:
            raise PackError("transcoded Vulkan format differs from profile")
        for field in ("pixelWidth", "pixelHeight", "pixelDepth", "layerCount",
                      "faceCount", "levelCount"):
            if packed_header[field] != header[field]:
                raise PackError("transcode changed %s" % field)
        output_hash = digest(temporary)
        input_hash = digest(master)
        receipt = {"profile": profile_id, "ktx_revision": revision,
                   "texture_class": texture_class, "target": target,
                   "vk_format": expected_format, "source_sha256": input_hash,
                   "package_sha256": output_hash,
                   "dimensions": [header["pixelWidth"], header["pixelHeight"]],
                   "levels": header["levelCount"]}
        os.replace(temporary, destination)
    return receipt


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--profile", type=Path, default=DEFAULT_PROFILE)
    parser.add_argument("--product-profile", type=Path, default=DEFAULT_PRODUCT_PROFILE)
    parser.add_argument("--ktx", type=Path, required=True)
    parser.add_argument("--texture-class", required=True)
    target_choice = parser.add_mutually_exclusive_group(required=True)
    target_choice.add_argument("--target")
    target_choice.add_argument("--selection-plan", type=Path)
    parser.add_argument("--master", type=Path, required=True)
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args(argv)
    try:
        profile = json.loads(args.profile.read_text())
        target = args.target if args.target else selected_target(
            args.selection_plan, args.product_profile, args.profile, profile,
            args.texture_class)
        result = package(args.ktx, profile, args.texture_class, target,
                         args.master, args.out)
    except (OSError, KeyError, TypeError, ValueError, subprocess.SubprocessError) as error:
        print("KTX2 pack: %s" % error, file=sys.stderr)
        return 1
    print(json.dumps(result, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
