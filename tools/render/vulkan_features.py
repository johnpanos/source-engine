#!/usr/bin/env python3
"""render.device.vulkan-features (RFC 0016 K1, "Feature support recorded").

The Vulkan adapter (render/device/vulkan/) requires timeline semaphores,
synchronization2 and dynamic rendering. This tool records, per declared
profile, whether the profile's device supports each of them, and checks the
record.

  probe --platform linux     builds tools/render/vulkan_features_probe.c with
                             the host cc and runs it here;
  probe --platform android   builds it with the pinned NDK clang
                             (quality/product_profiles/portal-android-native-vulkan.json)
                             and runs it from /data/local/tmp over adb. It
                             never installs, uninstalls or launches an app and
                             never touches /sdcard/Android/data;
  check                      reads quality/fixtures/render-device/vulkan-features-v1.json
                             and reports checks-v1. Every required profile must
                             be measured with all three features supported and
                             complete evidence; an optional profile is either
                             measured or `unavailable` with a reason. An
                             optional profile may be `host-only`: only host
                             mode serves it (Vulkan 1.1 with the timeline and
                             synchronization2 extensions, without dynamic
                             rendering; user decision 2026-09-29). The core
                             must serve those devices before cutover (RFC 0016
                             K9 "Devices without dynamic rendering").

The probe prints one JSON document; `probe --out` stores it with its sha256,
and `probe --entry` prints the profile entry to paste into the record.
"""

import argparse
import datetime
import hashlib
import json
import os
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools" / "quality"))
import conformance_result  # noqa: E402

SCHEMA = "render-device-vulkan-features/v1"
RECORD = ROOT / "quality" / "fixtures" / "render-device" / "vulkan-features-v1.json"
PROBE_SOURCE = ROOT / "tools" / "render" / "vulkan_features_probe.c"
ANDROID_PROFILE = ROOT / "quality" / "product_profiles" / "portal-android-native-vulkan.json"
FEATURES = ("timeline_semaphore", "synchronization2", "dynamic_rendering")
HOST_FEATURES = ("timeline_semaphore", "synchronization2")
REQUIRED_PROFILES = ("linux-desktop", "android-fold7")
EVIDENCE_FIELDS = ("date", "command", "host", "probe_sha256", "loader_api")
DEVICE_FIELDS = ("name", "vendor_id", "device_id", "api_version", "driver_name", "driver_info")
DEVICE_TMP = "/data/local/tmp/render_vulkan_features_probe"


class FeatureError(Exception):
    pass


def ndk_clang():
    profile = json.loads(ANDROID_PROFILE.read_text())
    ndk = profile["dependencies"]["ndk"]
    triple = profile["android"]["ndk_triple"]["arm64-v8a"]
    api = profile["android"]["min_sdk"]
    root = ROOT / "dependencies" / "android" / ndk["extracted_directory"]
    clang = root / "toolchains/llvm/prebuilt/linux-x86_64/bin" / ("%s%d-clang" % (triple, api))
    if not clang.is_file():
        raise FeatureError("pinned NDK clang not found: %s" % clang)
    return clang


def build_probe(platform, out_dir):
    binary = Path(out_dir) / ("probe-" + platform)
    compiler = shutil.which("cc") if platform == "linux" else str(ndk_clang())
    if not compiler:
        raise FeatureError("no host C compiler")
    subprocess.run([compiler, "-std=c11", "-O1", "-Wall", "-o", str(binary), str(PROBE_SOURCE),
                    "-lvulkan"], check=True)
    return binary


def adb(args, device=None, **kwargs):
    command = ["adb"] + (["-s", device] if device else []) + args
    return subprocess.run(command, check=True, capture_output=True, text=True, **kwargs)


def run_probe(platform, device=None):
    with tempfile.TemporaryDirectory() as scratch:
        binary = build_probe(platform, scratch)
        if platform == "linux":
            output = subprocess.run([str(binary)], check=True, capture_output=True, text=True).stdout
            host = {"platform": "linux", "uname": os.uname().release, "node": os.uname().nodename}
        else:
            adb(["push", str(binary), DEVICE_TMP], device)
            adb(["shell", "chmod", "755", DEVICE_TMP], device)
            output = adb(["shell", DEVICE_TMP], device).stdout
            adb(["shell", "rm", "-f", DEVICE_TMP], device)
            host = {"platform": "android"}
            for key, prop in (("model", "ro.product.model"), ("device", "ro.product.device"),
                              ("fingerprint", "ro.build.fingerprint"),
                              ("soc", "ro.soc.model")):
                host[key] = adb(["shell", "getprop", prop], device).stdout.strip()
            if device:
                host["serial"] = device
    return output, host


def entry_from_probe(output, host, command, required):
    document = json.loads(output)
    if "error" in document:
        raise FeatureError("probe failed: %s" % document["error"])
    selected = document["adapter_selects"]
    path = None
    if selected < 0:
        # Host mode alone may serve it (a Vulkan 1.1 device, 2026-09-29).
        selected = document.get("host_selects", -1)
        if selected < 0:
            raise FeatureError("the adapter would select no device, in host mode either")
        path = "host-only"
    chosen = document["devices"][selected]
    return {
        "required": required,
        "status": "measured",
        "device": {key: chosen[key] for key in DEVICE_FIELDS},
        "adapter_path": path or chosen["adapter"]["path"],
        "portability_subset": chosen["portability_subset"],
        "features": chosen["features"],
        "all_devices": [{"index": d["index"], "name": d["name"], "type": d["type"],
                         "eligible": d["adapter"]["eligible"],
                         "selected": d["index"] == selected} for d in document["devices"]],
        "evidence": {
            "date": datetime.date.today().isoformat(),
            "command": command,
            "host": host,
            "probe": document["probe"],
            "probe_sha256": hashlib.sha256(output.encode()).hexdigest(),
            "loader_api": document["loader_api"],
        },
    }


def validate(record):
    """Returns [(check name, ok, detail)] for a record."""
    results = []

    def check(ok, name, detail=""):
        results.append((name, bool(ok), detail))
        return ok

    if not check(record.get("schema") == SCHEMA, "schema", "expected %s" % SCHEMA):
        return results
    profiles = record.get("profiles", {})
    for name in REQUIRED_PROFILES:
        check(name in profiles, "%s.present" % name, "required profile missing")
    for name, profile in sorted(profiles.items()):
        required = name in REQUIRED_PROFILES
        check(profile.get("required", False) == required, "%s.required" % name,
              "required flag must be %s" % required)
        status = profile.get("status")
        if status == "unavailable":
            check(not required, "%s.measured" % name, "a required profile is unavailable: %s"
                  % profile.get("reason", ""))
            check(bool(profile.get("reason")) and bool(profile.get("date")), "%s.reason" % name,
                  "an unavailable profile needs a reason and a date")
            continue
        if not check(status == "measured", "%s.status" % name, "status must be measured or unavailable"):
            continue
        device = profile.get("device", {})
        check(all(device.get(field) for field in DEVICE_FIELDS), "%s.device" % name,
              "device fields %s" % ", ".join(DEVICE_FIELDS))
        evidence = profile.get("evidence", {})
        check(all(evidence.get(field) for field in EVIDENCE_FIELDS), "%s.evidence" % name,
              "evidence fields %s" % ", ".join(EVIDENCE_FIELDS))
        features = profile.get("features", {})
        path = profile.get("adapter_path")
        host_only = path == "host-only"
        if host_only:
            check(not required, "%s.host-only" % name,
                  "a required profile must serve the port, not host mode alone")
        for feature in HOST_FEATURES if host_only else FEATURES:
            value = features.get(feature, {})
            available = value.get("core") is True or value.get("extension") is True
            check(available and value.get("supported") is True, "%s.%s" % (name, feature),
                  "not supported: %s" % json.dumps(value))
        check(path in ("core13", "vulkan12+extensions", "host-only"),
              "%s.adapter-path" % name, "the adapter's creation path")
    return results


def check_command(args):
    checks = conformance_result.Checks()
    try:
        record = json.loads(Path(args.record).read_text())
    except (OSError, ValueError) as error:
        checks.check(False, "record.read", str(error))
        return checks.report()
    for name, ok, detail in validate(record):
        checks.check(ok, name, detail)
    return checks.report()


def probe_command(args):
    command = "python3 tools/render/vulkan_features.py probe --platform %s" % args.platform
    output, host = run_probe(args.platform, args.device)
    if args.out:
        Path(args.out).write_text(output)
    required = args.profile in REQUIRED_PROFILES
    entry = entry_from_probe(output, host, command, required)
    print(json.dumps({args.profile: entry} if args.entry else json.loads(output), indent=2))
    return 0


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = parser.add_subparsers(dest="command", required=True)
    probe = sub.add_parser("probe")
    probe.add_argument("--platform", choices=("linux", "android"), required=True)
    probe.add_argument("--device", help="adb serial (android)")
    probe.add_argument("--profile", default="linux-desktop")
    probe.add_argument("--entry", action="store_true", help="print the record entry")
    probe.add_argument("--out", help="store the raw probe output")
    check = sub.add_parser("check")
    check.add_argument("--record", default=str(RECORD))
    args = parser.parse_args(argv)
    try:
        return probe_command(args) if args.command == "probe" else check_command(args)
    except (FeatureError, subprocess.CalledProcessError) as error:
        print("vulkan_features: %s" % error, file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
