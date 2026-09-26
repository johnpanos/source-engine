#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Read Source engine compiled shader files (`.vcs`).

The retail game ships the fxc-compiled D3D9 bytecode of every stdshader
combination in `shaders/fxc/*.vcs` inside `hl2/hl2_misc_dir.vpk`. This module
extracts the token stream for one (static, dynamic) combination so that
`d3d9_shader_vm.py` can disassemble or interpret it.

Usage:

    vcs = VcsFile.from_vpk(vpk_path, "unlitgeneric_ps20b")
    code = vcs.bytecode(static_index, dynamic_index)   # bytes or None

    python3 tools/quality/source_vcs.py unlitgeneric_ps20b
    python3 tools/quality/source_vcs.py unlitgeneric_ps20b --static 1 --disasm

Combo indices. The generated `fxctmp9/<shader>.inc` headers give the shader
DLL two numbers: `<shader>_Static_Index::GetIndex()` and
`<shader>_Dynamic_Index::GetIndex()`. The static index already includes the
dynamic-combo scale: its first static variable is multiplied by the number of
dynamic combos, so it is always a multiple of `header.dynamic_combos`. The
loader (materialsystem/shaderapidx9/vertexshaderdx8.cpp) then does:

* `LoadAndCreateShaders`: `FindCombo(m_nStaticIndex / m_nDynamicCombos)` --
  the *static combo ID* stored in `StaticComboRecord_t` is the static index
  divided by the dynamic combo count. `FindCombo` first maps the ID through the
  sorted v6 `StaticComboAliasRecord_t` table (duplicate static combos), then
  binary-searches the sorted `StaticComboRecord_t` table. The combo's data runs
  from its record's `m_nFileOffset` to the next record's offset (the table
  ends with a sentinel record, ID 0xffffffff).
* `CreateDynamicCombos_Ver5`: the data is a sequence of blocks, each a uint32
  size word (0xffffffff ends the list; high bits 0x80000000 = stored,
  0x40000000 = Valve LZMA, 0 = bzip2) followed by the payload. An unpacked
  block holds repeated (combo ID uint32, byte size uint32, bytecode) records.
  The dynamic combo number is `ID - m_nStaticIndex` when `ID >= m_nStaticIndex`
  and `ID` otherwise ("ver5 stores combos as full combo, ver6 as dynamic
  combo # only"). Dynamic combos that are absent were skipped by the
  shader's SKIP rules.
* Versions 2 and 4 (diff-compressed; assembly shaders and old fxc files) use a
  per-combo dictionary of (offset, size) entries indexed by the full combo
  number `static_index + dynamic_index`, optionally delta-coded against a
  reference combo (`tier1/undiff.cpp`). Version 2 has no CRC field, so its
  header is 24 bytes. The engine's loader itself only accepts 4, 5 and 6.

`bytecode()` returns None for a skipped combination and raises ValueError for
indices outside the declared combo space.
"""

import argparse
import bz2
import lzma
import os
import struct
import sys
from collections import namedtuple
from pathlib import Path

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from source_content import VpkDirectory  # noqa: E402

ROOT = Path(__file__).resolve().parents[2]

VcsHeader = namedtuple("VcsHeader", [
    "version", "total_combos", "dynamic_combos", "flags", "centroid_mask",
    # v5/v6: number of StaticComboRecord_t including the sentinel.
    "num_static_records",
    # v2/v4: size of the diff reference combo.
    "diff_reference_size",
    "source_crc32",
])

SENTINEL_STATIC_ID = 0xFFFFFFFF
BLOCK_END = 0xFFFFFFFF
LZMA_ID = b"LZMA"
SHADER_DIRECTORIES = ("fxc", "vsh", "psh")


class VcsError(ValueError):
    """A malformed or unsupported .vcs file."""


def default_vpk_paths():
    """Candidate hl2_misc_dir.vpk locations: this tree, then the main checkout
    of a git worktree (a worktree's `.git` is a file naming the common dir)."""
    roots = [ROOT]
    git_file = ROOT / ".git"
    if git_file.is_file():
        text = git_file.read_text().strip()
        if text.startswith("gitdir:"):
            gitdir = Path(text.split(":", 1)[1].strip())
            # <main>/.git/worktrees/<name>
            if gitdir.parent.name == "worktrees":
                roots.append(gitdir.parent.parent.parent)
    env = os.environ.get("SOURCE_VCS_VPK")
    paths = [Path(env)] if env else []
    for root in roots:
        paths.append(root / "run" / "runtime" / "hl2" / "hl2_misc_dir.vpk")
    return paths


def find_default_vpk():
    for path in default_vpk_paths():
        if path.is_file():
            return path
    return None


def list_vpk_shaders(vpk, directory="fxc"):
    """Shader names (without extension) in shaders/<directory>/ of a VPK."""
    if not isinstance(vpk, VpkDirectory):
        vpk = VpkDirectory(str(vpk))
    prefix = "shaders/%s/" % directory
    names = [key[len(prefix):-4] for key in vpk.entries
             if key.startswith(prefix) and key.endswith(".vcs")]
    return sorted(names)


def decompress_valve_lzma(data):
    """Decode a Valve `lzma_header_t` stream (tier1/lzmaDecoder.cpp)."""
    if len(data) < 17 or data[:4] != LZMA_ID:
        raise VcsError("LZMA block lacks the Valve 'LZMA' header")
    actual_size, lzma_size = struct.unpack_from("<II", data, 4)
    props = data[12:17]
    d = props[0]
    if d >= 9 * 5 * 5:
        raise VcsError("bad LZMA properties byte %d" % d)
    lc = d % 9
    d //= 9
    lp = d % 5
    pb = d // 5
    dict_size = struct.unpack_from("<I", props, 1)[0]
    filters = [{"id": lzma.FILTER_LZMA1, "dict_size": max(dict_size, 4096),
                "lc": lc, "lp": lp, "pb": pb}]
    decoder = lzma.LZMADecompressor(format=lzma.FORMAT_RAW, filters=filters)
    payload = data[17:17 + lzma_size]
    out = decoder.decompress(payload, max_length=actual_size)
    if len(out) != actual_size:
        raise VcsError("LZMA block decoded to %d bytes, header says %d"
                       % (len(out), actual_size))
    return out


def apply_diffs(reference, diff):
    """Port of tier1/undiff.cpp ApplyDiffs for version 2/4 files."""
    out = bytearray()
    copy_src = 0
    pos = 0
    end = len(diff)

    def signed16(value):
        return value - 0x10000 if value > 32767 else value

    while pos < end:
        op = diff[pos]
        pos += 1
        if op == 0:
            size = diff[pos] + 256 * diff[pos + 1]
            ofs = signed16(diff[pos + 2] + 256 * diff[pos + 3])
            start = copy_src + ofs
            out += reference[start:start + size]
            copy_src = start + size
            pos += 4
        elif op & 0x80:
            size = op & 0x7F
            if size == 0:
                size = diff[pos]
                if size == 0:
                    size = diff[pos + 1] + 256 * diff[pos + 2] + 65536 * diff[pos + 3]
                    out += diff[pos + 4:pos + 4 + size]
                    pos += size + 4
                else:
                    ofs = signed16(diff[pos + 1] + 256 * diff[pos + 2])
                    start = copy_src + ofs
                    out += reference[start:start + size]
                    copy_src = start + size
                    pos += 3
            else:
                ofs = diff[pos]
                if ofs > 127:
                    ofs -= 256
                start = copy_src + ofs
                out += reference[start:start + size]
                copy_src = start + size
                pos += 1
        else:
            size = op & 0x7F
            out += diff[pos:pos + size]
            pos += size
    return bytes(out)


class VcsFile:
    """One parsed .vcs file. Bytecode is decoded lazily per static combo."""

    def __init__(self, data, name=None):
        self.data = bytes(data)
        self.name = name
        if len(self.data) < 24:
            raise VcsError("%s: too short for a shader header" % (name or "vcs"))
        version = struct.unpack_from("<i", self.data, 0)[0]
        self.static_records = []   # [(static combo id, file offset)] incl. sentinel
        self.alias_records = []    # [(static combo id, source static combo id)]
        self.dictionary = None     # v2/v4: [(offset, size)] per full combo
        self.reference_combo = b""
        self._cache = {}
        if version in (5, 6):
            fields = struct.unpack_from("<iIiIIII", self.data, 0)
            self.header = VcsHeader(fields[0], fields[1], fields[2], fields[3],
                                    fields[4], fields[5], None, fields[6])
            pos = 28
            count = self.header.num_static_records
            if pos + 8 * count > len(self.data):
                raise VcsError("%s: static combo table overruns the file" % name)
            for index in range(count):
                self.static_records.append(struct.unpack_from("<II", self.data, pos))
                pos += 8
            if version == 6:
                num_dups = struct.unpack_from("<i", self.data, pos)[0]
                pos += 4
                if num_dups < 0 or pos + 8 * num_dups > len(self.data):
                    raise VcsError("%s: bad alias record count %d" % (name, num_dups))
                for index in range(num_dups):
                    self.alias_records.append(struct.unpack_from("<II", self.data, pos))
                    pos += 8
            if not self.static_records or self.static_records[-1][0] != SENTINEL_STATIC_ID:
                raise VcsError("%s: static combo table lacks its sentinel" % name)
        elif version in (2, 4):
            if version == 4:
                fields = struct.unpack_from("<iIiIIII", self.data, 0)
                pos = 28
                crc = fields[6]
            else:
                fields = struct.unpack_from("<iIiIII", self.data, 0)
                pos = 24
                crc = None
            self.header = VcsHeader(fields[0], fields[1], fields[2], fields[3],
                                    fields[4], None, fields[5], crc)
            ref_size = self.header.diff_reference_size
            self.reference_combo = self.data[pos:pos + ref_size]
            pos += ref_size
            total = self.header.total_combos
            if pos + 8 * total > len(self.data):
                raise VcsError("%s: combo dictionary overruns the file" % name)
            self.dictionary = [struct.unpack_from("<ii", self.data, pos + 8 * i)
                               for i in range(total)]
        else:
            raise VcsError("%s: unsupported .vcs version %d" % (name or "vcs", version))
        if self.header.dynamic_combos <= 0 or self.header.total_combos <= 0:
            raise VcsError("%s: empty combo space" % name)
        # m_nTotalCombos is an int32 in the engine and overflows for the
        # largest shaders (lightmappedgeneric_ps20b stores 2717908992 as a
        # negative number). It is read unsigned here; when the signed value
        # is negative the upper bound is not trusted.
        self.total_combos_reliable = self.header.total_combos < 0x80000000

    @classmethod
    def from_vpk(cls, vpk_path, shader_name):
        """Load `shaders/fxc/<name>.vcs` (then vsh/, psh/) from a VPK
        directory. `shader_name` may also be `vsh/<name>` or a full path."""
        vpk = vpk_path if isinstance(vpk_path, VpkDirectory) else VpkDirectory(str(vpk_path))
        name = shader_name[:-4] if shader_name.lower().endswith(".vcs") else shader_name
        if name.startswith("shaders/"):
            candidates = [name + ".vcs"]
        elif "/" in name:
            candidates = ["shaders/%s.vcs" % name]
        else:
            candidates = ["shaders/%s/%s.vcs" % (d, name) for d in SHADER_DIRECTORIES]
        for relative in candidates:
            data = vpk.read(relative)
            if data is not None:
                return cls(data, name=relative)
        raise FileNotFoundError("%s: no %s" % (vpk.path, " or ".join(candidates)))

    @classmethod
    def from_path(cls, path):
        with open(path, "rb") as stream:
            return cls(stream.read(), name=str(path))

    # -- combo space ---------------------------------------------------------

    @property
    def num_static_combos(self):
        """Declared static combo count (including skipped ones)."""
        return self.header.total_combos // self.header.dynamic_combos

    def static_combo_ids(self):
        """Static combo IDs with stored data (static_index // dynamic_combos),
        including v6 aliases of stored combos, sorted."""
        if self.dictionary is not None:
            # Dictionary formats are small, pre-overflow files.
            ids = []
            dyn = self.header.dynamic_combos
            for static_id in range(self.num_static_combos):
                entries = self.dictionary[static_id * dyn:(static_id + 1) * dyn]
                if any(offset != -1 and size > 0 for offset, size in entries):
                    ids.append(static_id)
            return ids
        ids = {record[0] for record in self.static_records[:-1]}
        ids.update(alias for alias, source in self.alias_records if source in ids)
        return sorted(ids)

    def static_indices(self):
        """Stored static indices as the shader DLL passes them."""
        return [static_id * self.header.dynamic_combos for static_id in self.static_combo_ids()]

    def _binary_search(self, static_id, records):
        # Mirrors BinarySearchCombos: records sorted by ID.
        lo, hi = 0, len(records) - 1
        while lo <= hi:
            mid = (lo + hi) // 2
            probe = records[mid][0]
            if static_id < probe:
                hi = mid - 1
            elif static_id > probe:
                lo = mid + 1
            else:
                return mid
        return -1

    def find_static_record(self, static_id):
        """Index into static_records for a static combo ID (FindCombo), or -1."""
        alias = self._binary_search(static_id, self.alias_records)
        if alias != -1:
            static_id = self.alias_records[alias][1]
        # The sentinel is part of the searched table in the engine too, but a
        # real ID never equals 0xffffffff.
        return self._binary_search(static_id, self.static_records)

    def _check_indices(self, static_index, dynamic_index):
        dyn = self.header.dynamic_combos
        if static_index < 0 or static_index % dyn:
            raise ValueError("%s: static index %d is not a multiple of the %d dynamic combos"
                             % (self.name, static_index, dyn))
        if self.total_combos_reliable and static_index >= self.header.total_combos:
            raise ValueError("%s: static index %d outside %d total combos"
                             % (self.name, static_index, self.header.total_combos))
        if dynamic_index is not None and not 0 <= dynamic_index < dyn:
            raise ValueError("%s: dynamic index %d outside %d dynamic combos"
                             % (self.name, dynamic_index, dyn))

    def dynamic_combos(self, static_index):
        """{dynamic index: bytecode} for one static index; {} if skipped."""
        self._check_indices(static_index, None)
        if static_index in self._cache:
            return self._cache[static_index]
        if self.dictionary is not None:
            combos = self._dictionary_combos(static_index)
        else:
            combos = self._block_combos(static_index)
        self._cache[static_index] = combos
        return combos

    def _dictionary_combos(self, static_index):
        combos = {}
        for dyn in range(self.header.dynamic_combos):
            offset, size = self.dictionary[static_index + dyn]
            if offset == -1 or size <= 0:
                continue
            code = self.data[offset:offset + size]
            if len(code) != size:
                raise VcsError("%s: combo %d overruns the file" % (self.name, static_index + dyn))
            if self.reference_combo:
                code = apply_diffs(self.reference_combo, code)
            combos[dyn] = code
        return combos

    def _block_combos(self, static_index):
        record = self.find_static_record(static_index // self.header.dynamic_combos)
        if record == -1 or record + 1 >= len(self.static_records):
            return {}
        start = self.static_records[record][1]
        end = self.static_records[record + 1][1]
        if not 0 < start <= end <= len(self.data):
            raise VcsError("%s: static combo data range %d..%d is invalid"
                           % (self.name, start, end))
        combos = {}
        pos = start
        while True:
            if pos + 4 > len(self.data):
                raise VcsError("%s: block list runs off the end of the file" % self.name)
            size_word = struct.unpack_from("<I", self.data, pos)[0]
            pos += 4
            if size_word == BLOCK_END:
                break
            kind = size_word & 0xC0000000
            if kind == 0x80000000:
                size = size_word & 0x3FFFFFFF
                block = self.data[pos:pos + size]
            elif kind == 0x40000000:
                size = size_word & 0x3FFFFFFF
                block = decompress_valve_lzma(self.data[pos:pos + size])
            elif kind == 0:
                size = size_word
                block = bz2.decompress(self.data[pos:pos + size])
            else:
                raise VcsError("%s: unrecognized block compression 0x%08x"
                               % (self.name, size_word))
            pos += size
            inner = 0
            while inner < len(block):
                combo_id, code_size = struct.unpack_from("<II", block, inner)
                inner += 8
                code = block[inner:inner + code_size]
                if len(code) != code_size:
                    raise VcsError("%s: combo %d overruns its block" % (self.name, combo_id))
                inner += code_size
                dyn = combo_id - static_index if combo_id >= static_index else combo_id
                if not 0 <= dyn < self.header.dynamic_combos:
                    raise VcsError("%s: combo id %d does not belong to static index %d"
                                   % (self.name, combo_id, static_index))
                combos[dyn] = bytes(code)
        return combos

    def bytecode(self, static_index, dynamic_index=0):
        """D3D9 token bytes for a combination, or None if it was skipped."""
        self._check_indices(static_index, dynamic_index)
        return self.dynamic_combos(static_index).get(dynamic_index)

    def first_combo(self):
        """(static_index, dynamic_index, bytecode) of the first stored combo."""
        for static_index in self.static_indices():
            combos = self.dynamic_combos(static_index)
            if combos:
                dyn = min(combos)
                return static_index, dyn, combos[dyn]
        return None


def _load(args):
    if args.shader.endswith(".vcs") and os.path.isfile(args.shader):
        return VcsFile.from_path(args.shader)
    vpk = args.vpk or find_default_vpk()
    if vpk is None:
        raise SystemExit("no hl2_misc_dir.vpk found; pass --vpk")
    return VcsFile.from_vpk(vpk, args.shader)


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    parser.add_argument("shader", help="shader name (e.g. unlitgeneric_ps20b) or a .vcs path")
    parser.add_argument("--vpk", help="VPK directory file (default: hl2_misc_dir.vpk)")
    parser.add_argument("--static", type=int, help="static index as the shader DLL passes it")
    parser.add_argument("--dynamic", type=int, default=0, help="dynamic index")
    parser.add_argument("--disasm", action="store_true", help="print the disassembly")
    parser.add_argument("--list", action="store_true", help="list every stored combo")
    args = parser.parse_args(argv)
    vcs = _load(args)
    h = vcs.header
    if args.static is None:
        print("%s: version %d, %d total combos, %d dynamic, %d static, flags 0x%x, "
              "centroid mask 0x%x, crc 0x%08x"
              % (vcs.name, h.version, h.total_combos, h.dynamic_combos,
                 vcs.num_static_combos, h.flags, h.centroid_mask, (h.source_crc32 or 0)))
        ids = vcs.static_combo_ids()
        print("stored static combos: %d (%d aliases)" % (len(ids), len(vcs.alias_records)))
        if args.list:
            for static_id in ids:
                combos = vcs.dynamic_combos(static_id * h.dynamic_combos)
                print("  static index %d (id %d): dynamic %s"
                      % (static_id * h.dynamic_combos, static_id, sorted(combos)))
        if not args.disasm:
            return 0
        first = vcs.first_combo()
        if first is None:
            print("no stored combos")
            return 1
        static_index, dynamic_index, code = first
        print("first combo: --static %d --dynamic %d" % (static_index, dynamic_index))
    else:
        static_index, dynamic_index = args.static, args.dynamic
        code = vcs.bytecode(static_index, dynamic_index)
        if code is None:
            print("combo static %d dynamic %d is skipped" % (static_index, dynamic_index))
            return 1
    print("static %d dynamic %d: %d bytes" % (static_index, dynamic_index, len(code)))
    if args.disasm:
        import d3d9_shader_vm
        print(d3d9_shader_vm.disassemble(code))
    return 0


if __name__ == "__main__":
    sys.exit(main())
