#!/usr/bin/env python3
"""Independent BSP2 map container reader and corpus checker (RFC 0008, F1).

This reader deliberately shares no code with the C++ implementation
(mapcontainer/). It decodes the format from the specification in
public/mapcontainer/map_container_format.h. It uses Python's hashlib BLAKE2b,
so it also checks the C++ hash independently.

Commands:
  validate <map>...          validate legacy VBSP or BSP2 files
  export   <bsp2> <out>      BSP2 -> legacy VBSP
  convert  <legacy> <out>    legacy VBSP -> BSP2 (second, independent writer)
  corpus   --tool BSP2TOOL [--out EVIDENCE] <map-or-dir>...
                             for every legacy map, checks that:
                               - C++ conversion validates here;
                               - the Python and C++ writers agree byte-for-byte;
                               - both exporters return the original file;
                               - every legacy lump and game lump matches.

Exit status: 0 on success, 1 on any validation or corpus failure, 2 on usage
errors or missing inputs (a required corpus that finds no maps fails).
"""

import argparse
import datetime
import hashlib
import json
import os
import re
import struct
import subprocess
import sys
import tempfile

MAGIC = b"SRCBSP2\x1a"
CONTAINER_VERSION = 1
HEADER_SIZE = 64
ENTRY_SIZE = 64
HASH_BLAKE2B_128 = 1
MIN_ALIGN = 16
MAX_LUMPS = 65536
COMPRESSION_MASK = 0xF
FLAG_REQUIRED = 0x100
KNOWN_FLAGS = COMPRESSION_MASK | FLAG_REQUIRED
LEGACY_IDENT = b"VBSP"
LEGACY_LUMPS = 64
LEGACY_HEADER_SIZE = 8 + 16 * LEGACY_LUMPS + 4
LUMP_GAME = 35

# Fixed element sizes of legacy lumps for versions 19-21 (public/bspfile.h).
# Carried lumps must stay whole multiples of these sizes.
LEGACY_ELEMENT_SIZES = {
    1: ("planes", 20), 2: ("texdata", 32), 3: ("vertexes", 12), 5: ("nodes", 32),
    6: ("texinfo", 72), 7: ("faces", 56), 12: ("edges", 4), 13: ("surfedges", 4),
    14: ("models", 48), 16: ("leaffaces", 2), 17: ("leafbrushes", 2), 18: ("brushes", 12),
    19: ("brushsides", 8), 27: ("originalfaces", 56), 30: ("vertnormals", 12),
    31: ("vertnormalindices", 2), 44: ("texdata_string_table", 4),
}


class FormatError(Exception):
    def __init__(self, code, detail=""):
        super().__init__(f"{code}: {detail}" if detail else code)
        self.code = code


def blake128(data):
    return hashlib.blake2b(data, digest_size=16).digest()


def fourcc(text):
    return struct.unpack("<I", text.encode("ascii"))[0]


def fourcc_text(value):
    raw = struct.pack("<I", value)
    return "".join(chr(b) if 32 <= b < 127 else "?" for b in raw)


def legacy_fourcc(index):
    return fourcc("L%03d" % index)


def legacy_index(value):
    text = struct.pack("<I", value)
    if text[:1] != b"L" or not text[1:].isdigit():
        return None
    index = int(text[1:])
    return index if index < LEGACY_LUMPS else None


LHDR = fourcc("LHDR")
LGAP = fourcc("LGAP")


# ---------------------------------------------------------------------------
# Legacy VBSP
# ---------------------------------------------------------------------------

def parse_legacy_header(raw, file_size):
    if len(raw) < LEGACY_HEADER_SIZE:
        raise FormatError("truncated", "legacy header")
    if raw[:4] != LEGACY_IDENT:
        raise FormatError("unsupported-legacy-ident")
    version = struct.unpack_from("<i", raw, 4)[0]
    lumps = [struct.unpack_from("<iiii", raw, 8 + 16 * i) for i in range(LEGACY_LUMPS)]
    revision = struct.unpack_from("<i", raw, 8 + 16 * LEGACY_LUMPS)[0]
    for i, (ofs, length, _ver, uncompressed) in enumerate(lumps):
        if length < 0 or uncompressed < 0:
            raise FormatError("legacy-header-invalid", "lump %d" % i)
        if length and (ofs < LEGACY_HEADER_SIZE or ofs + length > file_size):
            raise FormatError("lump-out-of-bounds", "lump %d" % i)
    return {"version": version, "lumps": lumps, "revision": revision}


def check_legacy_structure(version, index, data, origin):
    """Structure checks on a carried legacy lump, relative to its legacy origin."""
    if 19 <= version <= 21 and index in LEGACY_ELEMENT_SIZES:
        name, size = LEGACY_ELEMENT_SIZES[index]
        if len(data) % size:
            raise FormatError("legacy-structure", "%s length %d not a multiple of %d" % (name, len(data), size))
    if index == LUMP_GAME and data:
        if len(data) < 4:
            raise FormatError("legacy-structure", "game lump header")
        count = struct.unpack_from("<i", data, 0)[0]
        if count < 0 or 4 + 16 * count > len(data):
            raise FormatError("legacy-structure", "game lump count %d" % count)
        for k in range(count):
            _gid, _flags, _ver, gofs, glen = struct.unpack_from("<iHHii", data, 4 + 16 * k)
            if glen == 0:
                continue
            # Absolute legacy offsets must land inside this lump; BSP2 readers
            # rebase them with the legacy origin.
            if not (origin <= gofs and gofs + glen <= origin + len(data)):
                raise FormatError("legacy-structure", "game lump %d outside its lump" % k)


# ---------------------------------------------------------------------------
# BSP2
# ---------------------------------------------------------------------------

class Bsp2File:
    def __init__(self, data, known=(), verify=True, allow_unknown_required=False):
        self.data = data
        if len(data) < HEADER_SIZE:
            raise FormatError("truncated", "header")
        if data[:8] != MAGIC:
            raise FormatError("bad-magic")
        (version, flags, header_size, entry_size, directory_offset, count, hash_alg, revision,
         reserved) = struct.unpack_from("<IIIIQIIiI", data, 8)
        directory_hash = data[48:64]
        if version != CONTAINER_VERSION:
            raise FormatError("unsupported-container-version")
        if flags or header_size != HEADER_SIZE or entry_size != ENTRY_SIZE or reserved:
            raise FormatError("unsupported-layout")
        if hash_alg != HASH_BLAKE2B_128:
            raise FormatError("unsupported-hash")
        if count > MAX_LUMPS:
            raise FormatError("too-many-lumps")
        directory_end = directory_offset + count * ENTRY_SIZE
        if directory_offset < HEADER_SIZE or directory_end > len(data):
            raise FormatError("directory-out-of-bounds")
        directory = data[directory_offset:directory_end]
        if blake128(directory) != directory_hash:
            raise FormatError("directory-hash-mismatch")
        self.revision = revision
        self.entries = []
        known = set(known) | {LHDR, LGAP}
        for i in range(count):
            (cc, lver, lflags, align, offset, stored, uncompressed, lreserved) = struct.unpack_from(
                "<IIIIQQQQ", directory, i * ENTRY_SIZE)
            digest = directory[i * ENTRY_SIZE + 48:(i + 1) * ENTRY_SIZE]
            name = fourcc_text(cc)
            if lreserved:
                raise FormatError("unsupported-layout", name)
            if lflags & ~KNOWN_FLAGS:
                raise FormatError("unknown-flags", name)
            if align < MIN_ALIGN or align & (align - 1):
                raise FormatError("lump-misaligned", name)
            if offset % align:
                raise FormatError("lump-misaligned", name)
            if offset < HEADER_SIZE or offset + stored > directory_offset:
                raise FormatError("lump-out-of-bounds", name)
            is_known = cc in known or legacy_index(cc) is not None
            if not is_known and lflags & FLAG_REQUIRED and not allow_unknown_required:
                raise FormatError("unknown-required-lump", name)
            if lflags & COMPRESSION_MASK:
                if is_known or lflags & FLAG_REQUIRED:
                    raise FormatError("unsupported-compression", name)
            elif stored != uncompressed:
                raise FormatError("size-mismatch", name)
            self.entries.append({"fourcc": cc, "name": name, "version": lver, "flags": lflags,
                                 "alignment": align, "offset": offset, "size": stored, "hash": digest})
        previous_end = HEADER_SIZE
        for entry in sorted(self.entries, key=lambda e: (e["offset"], e["size"])):
            if not entry["size"]:
                continue
            if entry["offset"] < previous_end:
                raise FormatError("lump-overlap", entry["name"])
            previous_end = entry["offset"] + entry["size"]
        ids = [e["fourcc"] for e in self.entries]
        if len(ids) != len(set(ids)):
            raise FormatError("duplicate-lump")
        self.by_id = {e["fourcc"]: e for e in self.entries}
        if verify:
            for entry in self.entries:
                if entry["flags"] & COMPRESSION_MASK:
                    continue
                if blake128(self.lump(entry)) != entry["hash"]:
                    raise FormatError("content-hash-mismatch", entry["name"])
        self.legacy = None
        self.legacy_size = None
        has_legacy = any(legacy_index(e["fourcc"]) is not None or e["fourcc"] == LGAP
                         for e in self.entries)
        if LHDR in self.by_id:
            payload = self.lump(self.by_id[LHDR])
            if blake128(payload) != self.by_id[LHDR]["hash"]:
                raise FormatError("content-hash-mismatch", "LHDR")
            if len(payload) != 8 + LEGACY_HEADER_SIZE:
                raise FormatError("legacy-header-invalid", "LHDR size")
            self.legacy_size = struct.unpack_from("<Q", payload, 0)[0]
            try:
                self.legacy = parse_legacy_header(payload[8:], self.legacy_size)
            except FormatError as error:
                raise FormatError("legacy-header-invalid", str(error))
            if self.legacy_size < LEGACY_HEADER_SIZE or self.legacy["revision"] != revision:
                raise FormatError("legacy-header-invalid", "size or revision")
            for i, (_ofs, length, lver, _u) in enumerate(self.legacy["lumps"]):
                entry = self.by_id.get(legacy_fourcc(i))
                if (entry["size"] if entry else 0) != length or (entry and entry["version"] != lver):
                    raise FormatError("legacy-lump-mismatch", "L%03d" % i)
        elif has_legacy:
            raise FormatError("missing-legacy-header")
        self.gap_records = self._validate_gaps() if self.legacy is not None else []

    def lump(self, entry):
        return self.data[entry["offset"]:entry["offset"] + entry["size"]]

    def legacy_lump(self, index):
        entry = self.by_id.get(legacy_fourcc(index))
        return self.lump(entry) if entry else b""

    def _validate_gaps(self):
        if LGAP not in self.by_id:
            return []
        covered = [(0, LEGACY_HEADER_SIZE)]
        for ofs, length, _v, _u in self.legacy["lumps"]:
            if length:
                covered.append((ofs, ofs + length))
        gap_records = []
        gaps = self.lump(self.by_id[LGAP])
        cursor = 0
        previous_end = 0
        while cursor < len(gaps):
            if len(gaps) - cursor < 16:
                raise FormatError("legacy-gaps-invalid")
            offset, length = struct.unpack_from("<QQ", gaps, cursor)
            cursor += 16
            end = offset + length
            if (not length or length > len(gaps) - cursor or end > self.legacy_size
                    or offset < previous_end
                    or any(offset < c_end and c_begin < end for c_begin, c_end in covered)):
                raise FormatError("legacy-gaps-invalid")
            next_cursor = (cursor + length + 7) & ~7
            if next_cursor > len(gaps) or any(gaps[cursor + length:next_cursor]):
                raise FormatError("legacy-gaps-invalid")
            gap_records.append((offset, end, cursor))
            previous_end = end
            cursor = next_cursor
        return gap_records

    def export_legacy(self):
        if self.legacy is None:
            raise FormatError("missing-legacy-header")
        out = bytearray(self.legacy_size)
        out[:LEGACY_HEADER_SIZE] = self.lump(self.by_id[LHDR])[8:]
        for i, (ofs, length, _v, _u) in enumerate(self.legacy["lumps"]):
            if length:
                out[ofs:ofs + length] = self.legacy_lump(i)
        if self.gap_records:
            gaps = self.lump(self.by_id[LGAP])
            for offset, end, data_cursor in self.gap_records:
                out[offset:end] = gaps[data_cursor:data_cursor + end - offset]
        return bytes(out)

    def check_legacy_structures(self):
        for i, (ofs, _length, _v, _u) in enumerate(self.legacy["lumps"]):
            check_legacy_structure(self.legacy["version"], i, self.legacy_lump(i), ofs)


def write_bsp2(revision, lumps):
    """lumps: list of (fourcc, version, flags, alignment, bytes)."""
    body = bytearray(b"\0" * HEADER_SIZE)
    directory = bytearray()
    for cc, version, flags, align, payload in lumps:
        body.extend(b"\0" * (-len(body) % align))
        offset = len(body)
        body.extend(payload)
        directory.extend(struct.pack("<IIIIQQQQ", cc, version, flags, align, offset, len(payload),
                                     len(payload), 0))
        directory.extend(blake128(payload))
    body.extend(b"\0" * (-len(body) % MIN_ALIGN))
    directory_offset = len(body)
    body.extend(directory)
    body[:HEADER_SIZE] = MAGIC + struct.pack("<IIIIQIIiI", CONTAINER_VERSION, 0, HEADER_SIZE, ENTRY_SIZE,
                                             directory_offset, len(lumps), HASH_BLAKE2B_128, revision,
                                             0) + blake128(bytes(directory))
    return bytes(body)


def convert_legacy(data):
    header = parse_legacy_header(data, len(data))
    lumps_hdr = header["lumps"]
    covered = sorted([(0, LEGACY_HEADER_SIZE)] + [(o, o + n) for o, n, _v, _u in lumps_hdr if n])
    gaps = bytearray()

    def record(begin, end):
        i = begin
        while i < end:
            while i < end and data[i] == 0:
                i += 1
            run = i
            while run < end and data[run] != 0:
                run += 1
            if run > i:
                gaps.extend(struct.pack("<QQ", i, run - i))
                gaps.extend(data[i:run])
                gaps.extend(b"\0" * (-len(gaps) % 8))
            i = run

    covered_end = 0
    for begin, end in covered:
        if begin > covered_end:
            record(covered_end, begin)
        covered_end = max(covered_end, end)
    if covered_end < len(data):
        record(covered_end, len(data))

    lumps = [(LHDR, 1, FLAG_REQUIRED, MIN_ALIGN, struct.pack("<Q", len(data)) + data[:LEGACY_HEADER_SIZE])]
    if gaps:
        lumps.append((LGAP, 1, FLAG_REQUIRED, MIN_ALIGN, bytes(gaps)))
    for i, (ofs, length, lver, _u) in enumerate(lumps_hdr):
        if length:
            lumps.append((legacy_fourcc(i), lver & 0xFFFFFFFF, FLAG_REQUIRED, MIN_ALIGN, data[ofs:ofs + length]))
    return write_bsp2(header["revision"], lumps)


def open_any(data):
    if data[:8] == MAGIC:
        return "bsp2", Bsp2File(data)
    header = parse_legacy_header(data, len(data))
    return "legacy", header


# ---------------------------------------------------------------------------
# Commands
# ---------------------------------------------------------------------------

def read(path):
    with open(path, "rb") as handle:
        return handle.read()


def cmd_validate(args):
    status = 0
    for path in args.maps:
        try:
            kind, parsed = open_any(read(path))
            if kind == "bsp2" and parsed.legacy is not None:
                parsed.check_legacy_structures()
            print("ok %s %s" % (kind, path))
        except FormatError as error:
            print("FAIL %s: %s" % (path, error))
            status = 1
    return status


def cmd_export(args):
    try:
        out = Bsp2File(read(args.input)).export_legacy()
    except FormatError as error:
        print("FAIL %s" % error)
        return 1
    with open(args.output, "wb") as handle:
        handle.write(out)
    return 0


def cmd_convert(args):
    try:
        out = convert_legacy(read(args.input))
    except FormatError as error:
        print("FAIL %s" % error)
        return 1
    with open(args.output, "wb") as handle:
        handle.write(out)
    return 0


def collect_maps(paths):
    maps = []
    for path in paths:
        if os.path.isdir(path):
            for base, _dirs, files in os.walk(path):
                maps += [os.path.join(base, f) for f in files if f.endswith(".bsp")]
        elif os.path.isfile(path):
            maps.append(path)
        else:
            raise FileNotFoundError(path)
    return sorted(maps)


def load_expected_inventory(manifest_path):
    try:
        raw = read(manifest_path)
        manifest = json.loads(raw)
    except (OSError, json.JSONDecodeError) as error:
        raise ValueError("invalid expected corpus inventory: %s" % error) from error
    entries = manifest.get("maps") if isinstance(manifest, dict) else None
    if (not isinstance(manifest, dict) or manifest.get("schema") != "bsp2-corpus-inventory/v1" or
            not isinstance(entries, list) or not entries):
        raise ValueError("invalid expected corpus inventory schema or empty maps")
    expected = {}
    for entry in entries:
        if not isinstance(entry, dict):
            raise ValueError("invalid expected corpus inventory entry")
        name, digest, version = entry.get("name"), entry.get("sha256"), entry.get("version")
        if (not isinstance(name, str) or not re.fullmatch(r"[A-Za-z0-9_-]+\.bsp", name) or
                name in expected or not isinstance(digest, str) or
                not re.fullmatch(r"[0-9a-f]{64}", digest) or
                isinstance(version, bool) or not isinstance(version, int) or version not in (19, 20, 21)):
            raise ValueError("invalid or duplicate expected corpus map")
        expected[name] = (digest, version)
    return expected, {"path": os.path.abspath(manifest_path),
                      "sha256": hashlib.sha256(raw).hexdigest(), "maps": len(expected)}


def check_expected_inventory(maps, paths, manifest_path):
    if len(paths) != 1 or not os.path.isdir(paths[0]):
        raise ValueError("expected corpus inventory requires one directory root")
    expected, info = load_expected_inventory(manifest_path)
    actual = {os.path.basename(path): path for path in maps}
    if len(actual) != len(maps) or set(actual) != set(expected):
        missing = sorted(set(expected) - set(actual))
        extra = sorted(set(actual) - set(expected))
        raise ValueError("corpus inventory differs: missing=%s extra=%s" % (missing, extra))
    for name, path in sorted(actual.items()):
        data = read(path)
        digest = hashlib.sha256(data).hexdigest()
        version = int.from_bytes(data[4:8], "little", signed=True) if data[:4] == LEGACY_IDENT else None
        if (digest, version) != expected[name]:
            raise ValueError("corpus map hash or version differs: " + name)
    return info


def check_one(tool, path, scratch):
    original = read(path)
    if original[:4] != LEGACY_IDENT:
        return {"map": path, "outcome": "skip-not-legacy"}
    record = {"map": path, "bytes": len(original)}
    bsp2_path = os.path.join(scratch, "map.bsp2")
    export_path = os.path.join(scratch, "map.export.bsp")
    for leftover in (bsp2_path, export_path):
        if os.path.exists(leftover):
            os.remove(leftover)
    run = subprocess.run([tool, "convert", path, bsp2_path], capture_output=True, text=True,
                         timeout=120)
    if run.returncode:
        return dict(record, outcome="fail", detail="C++ convert: " + run.stderr.strip())
    cxx_bsp2 = read(bsp2_path)
    record["bsp2_bytes"] = len(cxx_bsp2)
    try:
        parsed = Bsp2File(cxx_bsp2)
        parsed.check_legacy_structures()
        record["legacy_version"] = parsed.legacy["version"]
        record["lumps"] = len(parsed.entries)
        if convert_legacy(original) != cxx_bsp2:
            return dict(record, outcome="fail", detail="Python and C++ writers disagree")
        if parsed.export_legacy() != original:
            return dict(record, outcome="fail", detail="Python export is not byte-identical")
        legacy_header = parse_legacy_header(original, len(original))
        for i, (ofs, length, _v, _u) in enumerate(legacy_header["lumps"]):
            if parsed.legacy_lump(i) != original[ofs:ofs + length]:
                return dict(record, outcome="fail", detail="lump %d differs" % i)
    except FormatError as error:
        return dict(record, outcome="fail", detail="Python reader: %s" % error)
    run = subprocess.run([tool, "export", bsp2_path, export_path], capture_output=True, text=True,
                         timeout=120)
    if run.returncode:
        return dict(record, outcome="fail", detail="C++ export: " + run.stderr.strip())
    if read(export_path) != original:
        return dict(record, outcome="fail", detail="C++ export is not byte-identical")
    return dict(record, outcome="pass")


def cmd_corpus(args):
    input_failures = []
    if not os.access(args.tool, os.X_OK):
        input_failures.append("bsp2tool not executable: %s" % args.tool)
    try:
        maps = collect_maps(args.paths)
    except FileNotFoundError as missing:
        maps = []
        input_failures.append("missing corpus path: %s" % missing)
    if not maps and not input_failures:
        input_failures.append("corpus contains zero BSP maps")
    inventory = None
    if args.expect_manifest and not input_failures:
        try:
            inventory = check_expected_inventory(maps, args.paths, args.expect_manifest)
        except (OSError, ValueError) as error:
            input_failures.append(str(error))
    for failure in input_failures:
        print("FAIL " + failure)
    results = []
    if not input_failures:
        with tempfile.TemporaryDirectory(prefix="bsp2-corpus-") as scratch:
            for path in maps:
                try:
                    result = check_one(args.tool, path, scratch)
                except (OSError, FormatError, subprocess.TimeoutExpired) as error:
                    result = {"map": path, "outcome": "fail", "detail": str(error)}
                results.append(result)
                print("%-5s %s%s" % (result["outcome"].split("-")[0], path,
                                     (" -- " + result["detail"]) if "detail" in result else ""))
    passed = sum(r["outcome"] == "pass" for r in results)
    failed = sum(r["outcome"] != "pass" for r in results)
    ok = passed > 0 and failed == 0 and not input_failures
    tool_sha256 = None
    try:
        if os.path.isfile(args.tool):
            tool_sha256 = hashlib.sha256(read(args.tool)).hexdigest()
    except OSError:
        pass
    evidence = {
        "schema": "bsp2-corpus-evidence/v1",
        "rfc": "0008",
        "phase": "F1",
        "time": datetime.datetime.now(datetime.timezone.utc).isoformat(),
        "tool": os.path.abspath(args.tool),
        "tool_sha256": tool_sha256,
        "python": sys.version.split()[0],
        "paths": args.paths,
        "maps": len(results),
        "passed": passed,
        "failed": failed,
        "outcome": "pass" if ok else "fail",
        "results": results,
        "expected_inventory": inventory,
        "failures": input_failures,
    }
    if args.out:
        os.makedirs(os.path.dirname(os.path.abspath(args.out)), exist_ok=True)
        with open(args.out, "w") as handle:
            json.dump(evidence, handle, indent=2)
            handle.write("\n")
    print("%d map(s): %d passed, %d failed -> %s" % (len(results), passed, failed, "PASS" if ok else "FAIL"))
    return 2 if input_failures else 0 if ok else 1


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = parser.add_subparsers(dest="command", required=True)
    p = sub.add_parser("validate")
    p.add_argument("maps", nargs="+")
    p.set_defaults(func=cmd_validate)
    p = sub.add_parser("export")
    p.add_argument("input")
    p.add_argument("output")
    p.set_defaults(func=cmd_export)
    p = sub.add_parser("convert")
    p.add_argument("input")
    p.add_argument("output")
    p.set_defaults(func=cmd_convert)
    p = sub.add_parser("corpus")
    p.add_argument("--tool", required=True, help="bsp2tool executable")
    p.add_argument("--out", help="evidence JSON path")
    p.add_argument("--expect-manifest", help="required exact map names, versions and SHA-256 digests")
    p.add_argument("paths", nargs="+")
    p.set_defaults(func=cmd_corpus)
    args = parser.parse_args(argv)
    return args.func(args)


if __name__ == "__main__":
    sys.exit(main())
