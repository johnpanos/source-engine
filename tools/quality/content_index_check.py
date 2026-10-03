#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Independent RFC 0015 index reader for conformance, never a build authority.

Python production entry points call the canonical C++ libraries or content_build
CLI. This deliberately separate reader checks the serialized index from a
different implementation, including block and identity hashes.
"""

import argparse
import hashlib
import json
from pathlib import Path
import struct

KINDS = ("model", "material", "texture", "particle-file", "soundscript",
         "sound", "scene", "caption", "nav", "map")
HEADER = struct.Struct("<8sIIIIQIIiI16s")
DIRECTORY = struct.Struct("<IIIIQQQQ16s")
ENTRY = struct.Struct("<BBHHHHHHQ16sQ")
EDGE = struct.Struct("<IBBHQ")


def hash16(data):
    return hashlib.blake2b(data, digest_size=16).digest()


def inspect(data):
    if len(data) < HEADER.size + DIRECTORY.size:
        raise ValueError("truncated header")
    magic, version, flags, header_size, directory_size, directory_offset, count, \
        algorithm, revision, reserved, directory_hash = HEADER.unpack_from(data)
    if ((magic, version, flags, header_size, directory_size, count, algorithm,
         revision, reserved) != (b"SRCASIX\x1a", 1, 0, 64, 64, 1, 1, 0, 0) or
            directory_offset % 16 or directory_offset + 64 != len(data)):
        raise ValueError("invalid header")
    directory = data[directory_offset:]
    if hash16(directory) != directory_hash:
        raise ValueError("directory hash mismatch")
    fourcc, block_version, block_flags, alignment, offset, size, raw_size, zero, \
        block_hash = DIRECTORY.unpack(directory)
    if ((fourcc, block_version, block_flags, alignment, offset, zero) !=
            (int.from_bytes(b"INDX", "little"), 1, 0, 16, 64, 0) or
            size != raw_size or offset + size > directory_offset):
        raise ValueError("invalid block directory")
    block = data[offset:offset + size]
    if hash16(block) != block_hash:
        raise ValueError("block hash mismatch")
    if len(block) < 16:
        raise ValueError("truncated payload")
    tag, entry_count, edge_count, zero = struct.unpack_from("<4sIII", block)
    if tag != b"AIX1" or zero or entry_count > 1000000 or edge_count > 4000000:
        raise ValueError("invalid payload")
    cursor = 16

    def take(size):
        nonlocal cursor
        if cursor + size > len(block):
            raise ValueError("truncated record")
        value = block[cursor:cursor + size]
        cursor += size
        return value

    entries = []
    hashes = {}
    variants = set()
    for _ in range(entry_count):
        kind, zero, name_len, variant_len, path_len, compiler_len, key_len, flags, \
            name_hash, content_hash, content_size = ENTRY.unpack(take(ENTRY.size))
        if kind < 1 or kind > len(KINDS) or zero or flags:
            raise ValueError("invalid entry")
        name, variant, path, compiler, key = (
            take(length).decode("utf-8") for length in
            (name_len, variant_len, path_len, compiler_len, key_len))
        kind_name = KINDS[kind - 1]
        expected_hash = hashlib.blake2b((kind_name + "\0" + name).encode(),
                                        digest_size=8).digest()
        if (name != name.lower() or name.startswith("/") or
                any(part in ("", ".", "..") for part in name.split("/")) or
                int.from_bytes(expected_hash, "little") != name_hash or
                (kind_name, name, variant) in variants):
            raise ValueError("invalid entry identity")
        variants.add((kind_name, name, variant))
        if name_hash in hashes and hashes[name_hash] != (kind_name, name):
            raise ValueError("name hash collision")
        hashes[name_hash] = (kind_name, name)
        entries.append({"kind": kind_name, "name": name, "variant": variant,
                        "path": path, "compiler": compiler, "key": key,
                        "hash": content_hash.hex(), "size": content_size})
    edges = []
    for _ in range(edge_count):
        source, kind, optional, length, name_hash = EDGE.unpack(take(EDGE.size))
        name = take(length).decode("utf-8")
        if (source >= len(entries) or not 1 <= kind <= len(KINDS) or optional > 1 or
                int.from_bytes(hashlib.blake2b((KINDS[kind - 1] + "\0" + name).encode(),
                                               digest_size=8).digest(), "little") != name_hash):
            raise ValueError("invalid edge")
        edges.append({"source": source, "kind": KINDS[kind - 1],
                      "name": name, "optional": bool(optional)})
    if cursor != len(block):
        raise ValueError("trailing payload")
    return {"entries": entries, "edges": edges}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("index", type=Path)
    args = parser.parse_args()
    print(json.dumps(inspect(args.index.read_bytes()), indent=2))


if __name__ == "__main__":
    main()
