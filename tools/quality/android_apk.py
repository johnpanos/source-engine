#!/usr/bin/env python3
"""Independent verifier for the Android APK that build-android-apk.sh produces.

The product profile (quality/product_profiles/portal-android-native-vulkan.json)
declares what a correct package contains; this reads the finished APK and checks
it against those facts without reusing the build script's logic:

  * native code: exactly the declared ABIs and, per ABI, exactly the declared
    modules (the engine loads most of them by name at run time, so a module a
    wscript change silently drops only fails on a device); ELF class/machine for
    the ABI; PT_LOAD alignment for 16 KB page devices; every DT_NEEDED packaged
    or a platform library; no text relocations; the native entry point exported
  * uncompressed native libraries start on a page boundary
  * classes.dex carries the activity class
  * the touch-control assets named by their manifest exist and are valid VTFs
  * the binary manifest (via aapt2): package, version, SDK levels, exact
    permissions, required features, launchable activity, native-code ABIs,
    extractNativeLibs and the configuration changes that keep the activity
    alive across rotation and foldable display swaps
  * zipalign -c and apksigner verify
  * the build variant: a debug package is debuggable; a release package is
    not debuggable and is not signed with the Android debug certificate

    python3 tools/quality/android_apk.py check APK --abi arm64-v8a [--abi x86_64]
        [--variant debug|release] [--build-tools DIR] [--report FILE]

Every failure is reported; the exit status is nonzero when any check fails.
"""

import argparse
import json
import struct
import subprocess
import sys
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
DEFAULT_PROFILE = ROOT / "quality/product_profiles/portal-android-native-vulkan.json"

# ELF e_machine per Android ABI (all declared ABIs are 64-bit little-endian).
ABI_MACHINE = {"arm64-v8a": 183, "x86_64": 62}

PT_LOAD = 1
SHT_STRTAB, SHT_DYNAMIC, SHT_DYNSYM = 3, 6, 11
DT_NEEDED, DT_SONAME, DT_TEXTREL, DT_FLAGS = 1, 14, 22, 30
DF_TEXTREL = 0x4

# android.R.attr.configChanges flags (android.content.pm.ActivityInfo).
CONFIG_CHANGES = {
    "mcc": 0x1, "mnc": 0x2, "locale": 0x4, "touchscreen": 0x8, "keyboard": 0x10,
    "keyboardHidden": 0x20, "navigation": 0x40, "orientation": 0x80,
    "screenLayout": 0x100, "uiMode": 0x200, "screenSize": 0x400,
    "smallestScreenSize": 0x800, "density": 0x1000, "layoutDirection": 0x2000,
    "colorMode": 0x4000, "grammaticalGender": 0x8000,
    "fontWeightAdjustment": 0x10000000, "fontScale": 0x40000000,
}

# The subject of the SDK's generated debug keystore (~/.android/debug.keystore).
DEBUG_CERTIFICATE_DN = "CN=Android Debug, O=Android, C=US"

VARIANTS = ("debug", "release")

# public/vtf/vtf.h: IMAGE_FORMAT_RGBA8888 and its bytes per pixel.
VTF_RGBA8888 = 0


class Failures:
    def __init__(self):
        self.items = []

    def add(self, message):
        self.items.append(message)

    def __bool__(self):
        return bool(self.items)


# ---------------------------------------------------------------------------
# ELF
# ---------------------------------------------------------------------------

class ElfError(ValueError):
    pass


def parse_elf(data):
    """Facts the package checks need from a 64-bit little-endian ELF shared object."""
    if len(data) < 64 or data[:4] != b"\x7fELF":
        raise ElfError("not an ELF file")
    if data[4] != 2 or data[5] != 1:
        raise ElfError("not ELF64 little-endian (class %d, data %d)" % (data[4], data[5]))
    (e_type, e_machine, _, _, e_phoff, e_shoff, _, _, e_phentsize, e_phnum,
     e_shentsize, e_shnum, _) = struct.unpack_from("<HHIQQQIHHHHHH", data, 16)

    def within(offset, size):
        if offset < 0 or size < 0 or offset + size > len(data):
            raise ElfError("truncated ELF (range %d+%d)" % (offset, size))

    load_aligns = []
    within(e_phoff, e_phentsize * e_phnum)
    for i in range(e_phnum):
        p_type, _, _, _, _, _, _, p_align = struct.unpack_from(
            "<IIQQQQQQ", data, e_phoff + i * e_phentsize)
        if p_type == PT_LOAD:
            load_aligns.append(p_align)

    within(e_shoff, e_shentsize * e_shnum)
    sections = []
    for i in range(e_shnum):
        (_, sh_type, _, _, sh_offset, sh_size, sh_link, _, _, sh_entsize) = struct.unpack_from(
            "<IIQQQQIIQQ", data, e_shoff + i * e_shentsize)
        sections.append((sh_type, sh_offset, sh_size, sh_link, sh_entsize))

    def string(table, offset):
        _, t_offset, t_size, _, _ = sections[table]
        within(t_offset, t_size)
        if offset >= t_size:
            raise ElfError("string offset outside its table")
        end = data.index(b"\0", t_offset + offset, t_offset + t_size)
        return data[t_offset + offset:end].decode("utf-8", "replace")

    needed, soname, textrel, exports = [], None, False, set()
    for sh_type, sh_offset, sh_size, sh_link, sh_entsize in sections:
        if sh_type == SHT_DYNAMIC:
            within(sh_offset, sh_size)
            for off in range(sh_offset, sh_offset + sh_size - 15, 16):
                tag, value = struct.unpack_from("<qQ", data, off)
                if tag == 0:
                    break
                if tag == DT_NEEDED:
                    needed.append(string(sh_link, value))
                elif tag == DT_SONAME:
                    soname = string(sh_link, value)
                elif tag == DT_TEXTREL or (tag == DT_FLAGS and value & DF_TEXTREL):
                    textrel = True
        elif sh_type == SHT_DYNSYM:
            within(sh_offset, sh_size)
            for off in range(sh_offset, sh_offset + sh_size - 23, sh_entsize or 24):
                st_name, _, _, st_shndx = struct.unpack_from("<IBBH", data, off)
                if st_name and st_shndx:  # defined here, not an import
                    exports.add(string(sh_link, st_name))
    return {
        "type": e_type, "machine": e_machine, "load_aligns": load_aligns,
        "needed": needed, "soname": soname, "textrel": textrel, "exports": exports,
    }


# ---------------------------------------------------------------------------
# VTF (the touch icons)
# ---------------------------------------------------------------------------

def check_vtf(data):
    """Returns a problem description, or None for an uncompressed VTF 7.2 the client can atlas."""
    if len(data) < 80 or data[:4] != b"VTF\0":
        return "not a VTF file"
    major, minor, header_size, width, height = struct.unpack_from("<iiIHH", data, 4)
    if (major, minor) != (7, 2):
        return "VTF version %d.%d, expected 7.2" % (major, minor)
    if width != height or width & (width - 1) or not width:
        return "icon is %dx%d; touch.cpp atlases square power-of-two images" % (width, height)
    image_format, mips = struct.unpack_from("<iB", data, 52)
    if image_format != VTF_RGBA8888 or mips != 1:
        return "format %d with %d mips, expected single-mip RGBA8888" % (image_format, mips)
    if len(data) != header_size + width * height * 4:
        return "payload is %d bytes, expected %d" % (len(data) - header_size, width * height * 4)
    return None


# ---------------------------------------------------------------------------
# Package contents
# ---------------------------------------------------------------------------

def local_data_offset(apk_file, info):
    apk_file.seek(info.header_offset)
    header = apk_file.read(30)
    name_len, extra_len = struct.unpack_from("<HH", header, 26)
    return info.header_offset + 30 + name_len + extra_len


def check_contents(apk_path, profile, abis, failures):
    android = profile["android"]
    page = android["page_size_alignment"]
    platform = set(android["platform_libraries"])
    expected = set(android["packaged_libraries"])
    entry = android["native_entry"]
    facts = {"abis": {}}

    declared = set(profile["target"]["abis"])
    for abi in abis:
        if abi not in declared:
            failures.add("ABI %s is not declared by the profile" % abi)

    with zipfile.ZipFile(apk_path) as apk, open(apk_path, "rb") as raw:
        names = set(apk.namelist())
        for required in ("AndroidManifest.xml", "classes.dex", "resources.arsc"):
            if required not in names:
                failures.add("APK lacks %s" % required)

        packaged_abis = {n.split("/")[1] for n in names if n.startswith("lib/") and n.count("/") == 2}
        if packaged_abis != set(abis):
            failures.add("APK native ABIs %s, expected %s" % (sorted(packaged_abis), sorted(abis)))

        for abi in abis:
            infos = {i.filename.split("/")[2]: i for i in apk.infolist()
                     if i.filename.startswith("lib/%s/" % abi) and i.filename.count("/") == 2}
            present = set(infos)
            for name in sorted(expected - present):
                failures.add("%s: %s is not packaged" % (abi, name))
            for name in sorted(present - expected):
                failures.add("%s: %s is packaged but not declared by the profile" % (abi, name))
            facts["abis"][abi] = {"libraries": len(present)}

            for name, info in sorted(infos.items()):
                where = "%s/%s" % (abi, name)
                if info.compress_type == zipfile.ZIP_STORED:
                    offset = local_data_offset(raw, info)
                    if offset % page:
                        failures.add("%s: stored uncompressed at offset %d, not %d-aligned"
                                     % (where, offset, page))
                try:
                    elf = parse_elf(apk.read(info))
                except (ValueError, IndexError, struct.error) as error:  # ElfError included
                    failures.add("%s: %s" % (where, error))
                    continue
                if elf["machine"] != ABI_MACHINE.get(abi):
                    failures.add("%s: ELF machine %d, expected %s"
                                 % (where, elf["machine"], ABI_MACHINE.get(abi)))
                if not elf["load_aligns"]:
                    failures.add("%s: no PT_LOAD segments" % where)
                for align in elf["load_aligns"]:
                    if align < page:
                        failures.add("%s: PT_LOAD aligned to %d, below the %d-byte page size"
                                     % (where, align, page))
                        break
                if elf["textrel"]:
                    failures.add("%s: has text relocations (refused by Android 6+)" % where)
                for needed in elf["needed"]:
                    if needed not in present and needed not in platform:
                        failures.add("%s: needs %s, which is neither packaged nor a platform library"
                                     % (where, needed))
                if name == entry["library"] and entry["symbol"] not in elf["exports"]:
                    failures.add("%s: does not export %s" % (where, entry["symbol"]))

        if "classes.dex" in names:
            descriptor = ("L%s;" % android["activity"].replace(".", "/")).encode()
            if descriptor not in apk.read("classes.dex"):
                failures.add("classes.dex does not define %s" % android["activity"])

        manifest = android["touch_assets"]
        if manifest not in names:
            failures.add("APK lacks %s" % manifest)
        else:
            base = manifest.rsplit("/", 1)[0]
            icons = [line.strip() for line in apk.read(manifest).decode().splitlines() if line.strip()]
            if not icons:
                failures.add("%s lists no icons" % manifest)
            for icon in icons:
                path = "%s/%s" % (base, icon)
                if ".." in icon.split("/") or icon.startswith("/"):
                    failures.add("%s: unsafe entry %s" % (manifest, icon))
                elif path not in names:
                    failures.add("%s lists %s, which is not packaged" % (manifest, path))
                else:
                    problem = check_vtf(apk.read(path))
                    if problem:
                        failures.add("%s: %s" % (path, problem))
            facts["touch_icons"] = len(icons)
    return facts


# ---------------------------------------------------------------------------
# Binary manifest (aapt2 dump output)
# ---------------------------------------------------------------------------

def parse_badging(text):
    facts = {"permissions": [], "features": [], "native_code": [], "debuggable": False}
    for line in text.splitlines():
        key, _, rest = line.strip().partition(":")
        if key == "application-debuggable":
            facts["debuggable"] = True
        elif key == "package":
            for field in rest.split():
                name, _, value = field.partition("=")
                facts[name] = value.strip("'")
        elif key in ("minSdkVersion", "targetSdkVersion"):
            facts[key] = rest.strip().strip("'")
        elif key == "uses-permission":
            facts["permissions"].append(rest.split("name='", 1)[1].split("'", 1)[0])
        elif key == "uses-feature":
            facts["features"].append(rest.split("name='", 1)[1].split("'", 1)[0])
        elif key == "launchable-activity":
            facts["activity"] = rest.split("name='", 1)[1].split("'", 1)[0]
        elif key == "native-code":
            facts["native_code"] = [abi.strip("'") for abi in rest.split()]
    return facts


def parse_manifest_tree(text):
    """Application and activity attributes from `aapt2 dump xmltree`."""
    facts, element = {}, None
    for line in text.splitlines():
        stripped = line.strip()
        if stripped.startswith("E: "):
            element = stripped[3:].split()[0]
        elif stripped.startswith("A: ") and element in ("application", "activity"):
            attribute, _, value = stripped[3:].partition("=")
            name = attribute.rsplit(":", 1)[-1].split("(", 1)[0]
            facts["%s.%s" % (element, name)] = value.split(" (Raw", 1)[0].strip('"')
    return facts


def parse_signer_dns(text):
    """Signer certificate subjects from `apksigner verify --print-certs`."""
    return [line.split("certificate DN:", 1)[1].strip()
            for line in text.splitlines() if "certificate DN:" in line]


def dn_components(dn):
    """A certificate subject's attributes, independent of the tool's RDN order."""
    return frozenset(part.strip() for part in dn.split(","))


def is_debug_certificate(dn):
    return dn_components(dn) == dn_components(DEBUG_CERTIFICATE_DN)


def check_variant(badging, tree, signer_dns, variant, failures):
    """A debug package allows run-as and native debugging; a release package
    must allow neither and must carry a real signing key."""
    debuggable = badging["debuggable"] or tree.get("application.debuggable") == "true"
    if variant == "release":
        if debuggable:
            failures.add("release package is debuggable")
        if not signer_dns:
            failures.add("release package reports no signer certificate")
        if any(is_debug_certificate(dn) for dn in signer_dns):
            failures.add("release package is signed with the Android debug certificate")
    elif not debuggable:
        failures.add("debug package is not debuggable (no run-as or native debugging)")


def check_manifest(badging, tree, profile, abis, failures):
    android = profile["android"]
    expect = {
        "name": android["application_id"],
        "versionCode": str(android["version_code"]),
        "versionName": android["version_name"],
        "minSdkVersion": str(android["min_sdk"]),
        "targetSdkVersion": str(android["target_sdk"]),
        "activity": android["activity"],
    }
    for key, value in expect.items():
        if badging.get(key) != value:
            failures.add("manifest %s is %r, expected %r" % (key, badging.get(key), value))
    if sorted(badging["permissions"]) != sorted(android["permissions"]):
        failures.add("manifest permissions %s, expected exactly %s"
                     % (sorted(badging["permissions"]), sorted(android["permissions"])))
    for feature in android["required_features"]:
        if feature not in badging["features"]:
            failures.add("manifest does not require %s" % feature)
    if sorted(badging["native_code"]) != sorted(abis):
        failures.add("manifest native-code %s, expected %s" % (badging["native_code"], sorted(abis)))

    if tree.get("application.extractNativeLibs") != "true":
        failures.add("application extractNativeLibs is %r; the engine loads modules by path"
                     % tree.get("application.extractNativeLibs"))
    if tree.get("activity.name") != android["activity"]:
        failures.add("manifest activity is %r" % tree.get("activity.name"))
    try:
        changes = int(tree.get("activity.configChanges", "0"), 0)
    except ValueError:
        changes = 0
    for name in android["required_config_changes"]:
        if not changes & CONFIG_CHANGES[name]:
            failures.add("activity configChanges lacks %s: the activity would be recreated "
                         "on rotation or a foldable display swap" % name)


# ---------------------------------------------------------------------------
# SDK tools
# ---------------------------------------------------------------------------

def default_build_tools(profile):
    """The pinned build-tools that build-android-apk.sh extracts."""
    tools = profile["dependencies"]["sdk_build_tools"]
    return ROOT / "dependencies" / "android" / tools["extracted_directory"]


def run_tool(args):
    result = subprocess.run(args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    return result.returncode, result.stdout


def check_with_sdk(apk_path, build_tools, profile, abis, failures, variant="debug"):
    for tool in ("aapt2", "zipalign", "apksigner"):
        if not (build_tools / tool).is_file():
            failures.add("missing %s (build-tools %s)" % (
                build_tools / tool, profile["dependencies"]["sdk_build_tools"]["version"]))
            return
    code, badging = run_tool([str(build_tools / "aapt2"), "dump", "badging", str(apk_path)])
    code_tree, tree = run_tool([str(build_tools / "aapt2"), "dump", "xmltree",
                                "--file", "AndroidManifest.xml", str(apk_path)])
    manifest_ok = not (code or code_tree)
    if manifest_ok:
        badging, tree = parse_badging(badging), parse_manifest_tree(tree)
        check_manifest(badging, tree, profile, abis, failures)
    else:
        failures.add("aapt2 cannot read the manifest:\n%s%s" % (badging, tree))

    page_kb = str(profile["android"]["page_size_alignment"] // 1024)
    code, output = run_tool([str(build_tools / "zipalign"), "-c", "-P", page_kb, "4", str(apk_path)])
    if code:
        failures.add("zipalign -c -P %s failed:\n%s" % (page_kb, output))
    code, output = run_tool([str(build_tools / "apksigner"), "verify", "--print-certs",
                             str(apk_path)])
    if code:
        failures.add("apksigner verify failed:\n%s" % output)
    elif manifest_ok:
        check_variant(badging, tree, parse_signer_dns(output), variant, failures)


def check(apk_path, profile, abis, build_tools, variant="debug"):
    failures = Failures()
    facts = check_contents(apk_path, profile, abis, failures)
    facts["variant"] = variant
    check_with_sdk(apk_path, build_tools, profile, abis, failures, variant)
    return failures, facts


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    sub = parser.add_subparsers(dest="command", required=True)
    cmd = sub.add_parser("check", help="verify a built APK against the product profile")
    cmd.add_argument("apk", type=Path)
    cmd.add_argument("--abi", action="append", required=True, help="an ABI the APK must carry")
    cmd.add_argument("--variant", choices=VARIANTS, default="debug",
                     help="the build variant the package must be (default: debug)")
    cmd.add_argument("--profile", type=Path, default=DEFAULT_PROFILE)
    cmd.add_argument("--build-tools", type=Path, help="SDK build-tools directory "
                     "(default: the profile's pinned build-tools under dependencies/android)")
    cmd.add_argument("--report", type=Path, help="write a JSON report here")
    args = parser.parse_args(argv)

    profile = json.loads(args.profile.read_text())
    build_tools = args.build_tools or default_build_tools(profile)
    failures, facts = check(args.apk, profile, args.abi, build_tools, args.variant)
    report = {
        "schema": "android-apk-check/v1",
        "profile": profile["id"],
        "apk": str(args.apk),
        "abis": args.abi,
        "facts": facts,
        "failures": failures.items,
        "result": "fail" if failures else "pass",
    }
    if args.report:
        args.report.write_text(json.dumps(report, indent=2) + "\n")
    for item in failures.items:
        print("FAIL: %s" % item, file=sys.stderr)
    counts = ", ".join("%s %d libraries" % (abi, f["libraries"]) for abi, f in facts["abis"].items())
    print("android_apk: %s: %s (%s; %s; %s touch icons)" % (
        report["result"], args.apk.name, args.variant, counts, facts.get("touch_icons", 0)))
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
