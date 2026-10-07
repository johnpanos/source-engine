#!/usr/bin/env python3
"""Where the 3DS build's heap goes: a census of a guest heap dump.

  heap_census.py <dump> [--base 0x08000000] [--elf build-3ds/launcher_main/hl2_launcher]

The dump is the newlib heap written by the Azahar harness (`dump` or
`dump_on_stop`, see tools/n3ds/azahar_harness.py --heap-dump). newlib's
malloc (dlmalloc 2.6) lays the heap out as chunks: a 4-byte previous size, a
4-byte size whose low bit says the *previous* chunk is in use, then the
payload. The census walks the chunks from the base, decides each one's state
from its successor, and attributes in-use chunks:
  * C++ objects by the vtable pointer at the start of the payload (the ELF's
    "vtable for X" symbols; the pointer is the vtable plus 8);
  * everything else by size class, with the largest blocks listed with a
    printable preview of their first bytes.
"""

import argparse
import bisect
import collections
import struct
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
NM = ROOT / "dependencies/3ds/devkitpro/devkitARM/bin/arm-none-eabi-nm"


def vtables(elf):
    out = subprocess.run([str(NM), "-C", str(elf)], capture_output=True, text=True).stdout
    table = {}
    for line in out.splitlines():
        parts = line.split(" ", 2)
        if len(parts) == 3 and parts[2].startswith("vtable for "):
            table[int(parts[0], 16) + 8] = parts[2][len("vtable for "):]
    return table


def walk(data, base, start=0):
    """Returns ([address, size, in_use] per chunk, end offset)."""
    offset = start
    chunks = []
    while offset + 8 <= len(data):
        size_field = struct.unpack_from("<I", data, offset + 4)[0]
        size = size_field & ~7
        if size < 16 or offset + size > len(data):
            break
        chunks.append([base + offset, size, False])
        offset += size
    # A chunk is in use when its successor's PREV_INUSE bit is set.
    for i in range(len(chunks) - 1):
        next_offset = chunks[i + 1][0] - base
        chunks[i][2] = bool(struct.unpack_from("<I", data, next_offset + 4)[0] & 1)
    return chunks, offset


def find_start(data, chain=500):
    """The heap may begin with the main thread's stack (libctru carves it
    from the heap): the first offset from which `chain` chunks link up."""
    # The main thread's 2 MB stack (n3ds_main.cpp __stacksize__) comes first.
    for start in range(0x200000, min(len(data), 0x1000000), 8):
        offset, n = start, 0
        while n < chain and offset + 8 <= len(data):
            size = struct.unpack_from("<I", data, offset + 4)[0] & ~7
            if size < 16 or offset + size > len(data):
                break
            offset += size
            n += 1
        if n >= chain:
            return start
    return 0


def preview(data, offset, count=48):
    raw = data[offset:offset + count]
    return "".join(chr(b) if 32 <= b < 127 else "." for b in raw)


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("dump", type=Path)
    parser.add_argument("--base", type=lambda v: int(v, 0), default=0x08000000)
    parser.add_argument("--elf", type=Path, default=ROOT / "build-3ds/launcher_main/hl2_launcher")
    parser.add_argument("--top", type=int, default=25)
    parser.add_argument("--start", type=lambda v: int(v, 0), default=None,
                        help="offset of the first chunk (default: newlib's __malloc_sbrk_base, "
                             "else the first offset where a long chunk chain begins)")
    options = parser.parse_args()

    data = options.dump.read_bytes()
    start = options.start if options.start is not None else find_start(data)
    chunks, walked = walk(data, options.base, start)
    walked -= start
    print("first chunk at %08x" % (options.base + start))
    names = vtables(options.elf)
    used = [c for c in chunks if c[2]]
    free = [c for c in chunks if not c[2]]
    print("walked %.1f MB in %d chunks: %.1f MB in use (%d), %.1f MB free (%d)" % (
        walked / 1e6, len(chunks), sum(c[1] for c in used) / 1e6, len(used),
        sum(c[1] for c in free) / 1e6, len(free)))

    by_class = collections.Counter()
    count_class = collections.Counter()
    unknown = []
    for address, size, _ in used:
        offset = address - options.base + 8
        first = struct.unpack_from("<I", data, offset)[0] if offset + 4 <= len(data) else 0
        name = names.get(first)
        if name:
            by_class[name] += size
            count_class[name] += 1
        else:
            unknown.append((size, address))
    print("\nC++ objects by class (bytes, count):")
    for name, total in by_class.most_common(options.top):
        print("  %8.2f MB %6d  %s" % (total / 1e6, count_class[name], name[:110]))
    print("\nother allocations by size class:")
    classes = collections.Counter()
    for size, _ in unknown:
        bucket = 1 << max(4, (size - 1).bit_length())
        classes[bucket] += size
    for bucket, total in sorted(classes.items(), key=lambda kv: -kv[1])[:12]:
        print("  <= %9d B: %8.2f MB" % (bucket, total / 1e6))
    print("\nlargest other allocations:")
    for size, address in sorted(unknown, reverse=True)[:options.top]:
        print("  %8.1f KB at %08x  %s" % (size / 1024, address, preview(data, address - options.base + 8)))


if __name__ == "__main__":
    main()
