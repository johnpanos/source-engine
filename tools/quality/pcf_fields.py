#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
#
# particles.pcf-fields: the particle modulation and visibility fields retail
# Portal 2 authors are read by this tree's particle library.
#
# Reads every particle system file (.pcf, binary DMX v2-v5; dx80 variants
# skipped) in the Portal 2 VPKs and counts the uses of the per-operator
# modulation fields ("operator time offset/scale seed/min/max", "operator
# strength scale seed", "operator strength random scale min/max", "operator
# time strength random scale max") and the renderer visibility inputs
# ("Visibility input distance/dot minimum/maximum", "Visibility Radius FOV
# Scale base"). Each field Portal 2 sets must be in the unpack list every
# operator (BEGIN_PARTICLE_OPERATOR_UNPACK) or every renderer
# (BEGIN_PARTICLE_RENDER_OPERATOR_UNPACK) reads, with the DMX attribute type
# retail stores (a mismatched type is skipped by CDmxElement::UnpackIntoStructure),
# and the renderers that set them must use the renderer unpack list.
#
# The legacy VPKs (Portal: portal/portal_pak_dir.vpk and hl2/hl2_misc_dir.vpk)
# must set none of these fields and no visibility control point, so the
# library's no-op defaults cover all of their effects.
#
# A seeded copy of the unpack lists without "operator strength scale seed"
# must fail the same check.
#
# Usage: pcf_fields.py check [--portal2 VPK]... [--legacy VPK]...
# Defaults come from PCF_PORTAL2_VPKS and PCF_LEGACY_VPKS (comma-separated).
# ============================================================================

import argparse
import os
import re
import struct
import sys
from collections import Counter
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from conformance_result import Checks  # noqa: E402
from source_content import VpkDirectory  # noqa: E402

ROOT = Path(__file__).resolve().parents[2]
PARTICLES_H = ROOT / "public" / "particles" / "particles.h"
RENDER_OPS = ROOT / "particles" / "builtin_particle_render_ops.cpp"

# DMX attribute type ids (binary DMX) and the unpack macro type names.
DMX_INT, DMX_FLOAT = 2, 3
UNPACK_TYPES = {"int": DMX_INT, "float": DMX_FLOAT}

OPERATOR_FIELDS = [
    "operator time offset seed", "operator time offset min", "operator time offset max",
    "operator time scale seed", "operator time scale min", "operator time scale max",
    "operator strength scale seed", "operator strength random scale min",
    "operator strength random scale max", "operator time strength random scale max",
]
RENDERER_FIELDS = [
    "Visibility input distance minimum", "Visibility input distance maximum",
    "Visibility input dot minimum", "Visibility input dot maximum",
    "Visibility Radius FOV Scale base",
]
VISIBILITY_CP = "Visibility Proxy Input Control Point Number"


# --- binary DMX (after the reader session source-engine-d5 used for its gap scan) ---

_SIZES = {2: 4, 3: 4, 4: 1, 7: 16, 8: 4, 9: 8, 10: 12, 11: 16, 12: 12, 13: 16, 14: 64}
_FMT = {2: "<i", 3: "<f", 4: "<?", 8: "4B", 9: "<2f", 10: "<3f", 11: "<4f", 12: "<3f", 13: "<4f"}


class _Reader:
    def __init__(self, data):
        self.data, self.pos = data, 0

    def take(self, n):
        value = self.data[self.pos:self.pos + n]
        self.pos += n
        return value

    def i32(self):
        return struct.unpack("<i", self.take(4))[0]

    def u16(self):
        return struct.unpack("<H", self.take(2))[0]

    def cstr(self):
        end = self.data.index(b"\0", self.pos)
        text = self.data[self.pos:end].decode("latin1")
        self.pos = end + 1
        return text


def parse_dmx(data):
    """Elements of a binary DMX file as dicts: name -> (type id, value)."""
    match = re.search(rb"binary (\d+)", data[:64])
    if not match:
        raise ValueError("not a binary DMX file")
    version = int(match.group(1))
    header_end = data.index(b"\n") + 1
    if data[header_end:header_end + 1] == b"\0":
        header_end += 1
    r = _Reader(data)
    r.pos = header_end

    def string_index():
        return r.i32() if version >= 5 else r.u16()

    strings = []
    if version >= 2:
        strings = [r.cstr() for _ in range(r.i32() if version >= 4 else r.u16())]

    def value(type_id, in_array):
        if type_id == 1:
            index = r.i32()
            if index == -2:
                r.cstr()
            return index
        if type_id == 5:
            if version >= 4 and not in_array:
                return strings[string_index()]
            return r.cstr()
        if type_id == 6:
            return r.take(r.i32())
        if type_id in _FMT:
            unpacked = struct.unpack(_FMT[type_id], r.take(_SIZES[type_id]))
            return unpacked[0] if len(unpacked) == 1 else unpacked
        return r.take(_SIZES[type_id])

    elements = []
    for _ in range(r.i32()):
        type_name = strings[string_index()] if version >= 2 else r.cstr()
        name = strings[string_index()] if version >= 4 else r.cstr()
        r.take(16)
        elements.append({"_type": type_name, "_name": name})
    for element in elements:
        for _ in range(r.i32()):
            name = strings[string_index()] if version >= 2 else r.cstr()
            type_id = r.take(1)[0]
            if type_id >= 15:
                element[name] = (type_id, [value(type_id - 14, True) for _ in range(r.i32())])
            else:
                element[name] = (type_id, value(type_id, False))
    return elements


def pcf_elements(vpk_paths):
    """(pcf path, elements) for every non-dx80 .pcf in the VPKs."""
    for vpk_path in vpk_paths:
        vpk = VpkDirectory(vpk_path)
        for relative in sorted(vpk.entries):
            if relative.endswith(".pcf") and "dx80" not in relative:
                yield relative, parse_dmx(vpk.read(relative))


# --- unpack lists ---

def unpack_macro(text, macro):
    """{attribute name: dmx type} of one #define'd unpack macro in particles.h."""
    match = re.search(r"#define %s\(.*?\n((?:.*\\\n)*.*\n)" % re.escape(macro), text)
    if not match:
        return {}
    body = match.group(1).replace("\\\n", " ")
    fields = {}
    for name, type_name in re.findall(
            r'DMXELEMENT_UNPACK_FIELD\(\s*"([^"]+)"\s*,\s*"[^"]*"\s*,\s*(\w+)\s*,', body):
        fields[name] = UNPACK_TYPES.get(type_name)
    return fields


def renderer_unpack_kind(text, factory_name):
    """Which unpack macro the operator registered as factory_name uses."""
    match = re.search(r'DEFINE_PARTICLE_OPERATOR\(\s*(\w+)\s*,\s*"%s"' % re.escape(factory_name), text)
    if not match:
        return None
    kind = re.search(r"(BEGIN_PARTICLE_(?:RENDER_)?OPERATOR_UNPACK)\(\s*%s\s*\)" % match.group(1), text)
    return kind.group(1) if kind else None


# --- the check ---

def survey(vpk_paths):
    uses = Counter()
    types = {}
    renderers = Counter()
    visibility_cp = 0
    files = 0
    for _, elements in pcf_elements(vpk_paths):
        files += 1
        for element in elements:
            for field in OPERATOR_FIELDS + RENDERER_FIELDS:
                if field in element:
                    uses[field] += 1
                    types.setdefault(field, set()).add(element[field][0])
                    if field in RENDERER_FIELDS and "functionName" in element:
                        renderers[element["functionName"][1]] += 1
            cp = element.get(VISIBILITY_CP)
            if cp is not None and cp[1] >= 0:
                visibility_cp += 1
    return files, uses, types, renderers, visibility_cp


def fields_read(checks, prefix, portal2, operator_unpack, renderer_unpack, render_ops_text):
    """Every in-scope field Portal 2 sets is read with its retail type."""
    files, uses, types, renderers, _ = portal2
    ok = True
    for field in OPERATOR_FIELDS:
        if not uses[field]:
            continue
        retail = types[field]
        read = operator_unpack.get(field)
        ok &= checks.check(read is not None and retail == {read},
                           "%soperator field read: %r (%d uses)" % (prefix, field, uses[field]),
                           "unpack type %r, retail types %r" % (read, sorted(retail)))
    for field in RENDERER_FIELDS:
        if not uses[field]:
            continue
        retail = types[field]
        read = renderer_unpack.get(field)
        ok &= checks.check(read is not None and retail == {read},
                           "%srenderer field read: %r (%d uses)" % (prefix, field, uses[field]),
                           "unpack type %r, retail types %r" % (read, sorted(retail)))
    for renderer in sorted(renderers):
        kind = renderer_unpack_kind(render_ops_text, renderer)
        ok &= checks.check(kind == "BEGIN_PARTICLE_RENDER_OPERATOR_UNPACK",
                           "%s%s reads the renderer unpack list" % (prefix, renderer), "uses %r" % kind)
    return ok


def split_paths(value):
    return [p for p in (value or "").split(",") if p]


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("command", choices=["check"])
    parser.add_argument("--portal2", action="append", default=None, help="Portal 2 _dir.vpk")
    parser.add_argument("--legacy", action="append", default=None, help="Portal 1 / HL2 _dir.vpk")
    args = parser.parse_args()
    portal2_vpks = args.portal2 or split_paths(os.environ.get("PCF_PORTAL2_VPKS"))
    legacy_vpks = args.legacy or split_paths(os.environ.get("PCF_LEGACY_VPKS"))

    checks = Checks()
    if not checks.check(portal2_vpks and legacy_vpks, "content",
                        "pass --portal2 and --legacy (or PCF_PORTAL2_VPKS and PCF_LEGACY_VPKS)"):
        return checks.report()
    for path in portal2_vpks + legacy_vpks:
        if not checks.check(os.path.isfile(path), "content: %s" % path, "missing"):
            return checks.report()

    header = PARTICLES_H.read_text()
    render_ops_text = RENDER_OPS.read_text()
    operator_unpack = unpack_macro(header, "BEGIN_PARTICLE_OPERATOR_UNPACK")
    renderer_unpack = dict(operator_unpack)
    renderer_unpack.update(unpack_macro(header, "BEGIN_PARTICLE_RENDER_OPERATOR_UNPACK"))
    checks.check(len(operator_unpack) >= 5 and len(renderer_unpack) > len(operator_unpack),
                 "unpack lists parsed", "%d operator, %d renderer fields" % (len(operator_unpack), len(renderer_unpack)))

    portal2 = survey(portal2_vpks)
    files, uses, _, renderers, _ = portal2
    print("Portal 2: %d particle files" % files)
    for field in OPERATOR_FIELDS + RENDERER_FIELDS:
        print("  %5d  %s" % (uses[field], field))
    print("  renderers setting visibility inputs: %s" % dict(renderers))
    checks.check(files >= 50, "Portal 2 particle files read", "%d files" % files)
    checks.check(all(uses[f] >= 500 for f in OPERATOR_FIELDS),
                 "Portal 2 sets every modulation field on its operators",
                 ", ".join("%s: %d" % (f, uses[f]) for f in OPERATOR_FIELDS))
    checks.check(all(uses[f] >= 1 for f in RENDERER_FIELDS),
                 "Portal 2 sets every visibility input", ", ".join("%s: %d" % (f, uses[f]) for f in RENDERER_FIELDS))
    fields_read(checks, "", portal2, operator_unpack, renderer_unpack, render_ops_text)

    # Seeded: a list that does not read the strength seed must fail the same check.
    seeded_checks = Checks(stream=open(os.devnull, "w"))
    seeded = dict(operator_unpack)
    seeded.pop("operator strength scale seed", None)
    seeded_renderer = dict(renderer_unpack)
    seeded_renderer.pop("operator strength scale seed", None)
    detected = not fields_read(seeded_checks, "seeded ", portal2, seeded, seeded_renderer, render_ops_text)
    checks.check(detected, "seeded: an unpack list without \"operator strength scale seed\" is rejected")

    legacy_files, legacy_uses, _, _, legacy_cp = survey(legacy_vpks)
    print("legacy: %d particle files, %d in-scope field uses, %d visibility control points" %
          (legacy_files, sum(legacy_uses.values()), legacy_cp))
    checks.check(legacy_files >= 10, "legacy particle files read", "%d files" % legacy_files)
    checks.check(sum(legacy_uses.values()) == 0,
                 "legacy content sets no modulation field or new visibility input", repr(dict(legacy_uses)))
    checks.check(legacy_cp == 0, "legacy content has no visibility control point, so visibility stays off",
                 "%d renderers" % legacy_cp)
    return checks.report()


if __name__ == "__main__":
    sys.exit(main())
