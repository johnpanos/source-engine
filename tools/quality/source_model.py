#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Copy an installed studio model under a new name with its own material directory.

A map's dynamic models (map_scene `props`) must draw the map's materials
without changing the shipped model for every other map. `retarget()` reads a
model's files (.mdl plus its .vvd/.vtx/.phy/.ani companions) through
`source_content.ContentResolver`, rewrites the MDL's internal name and its
material search directories (`$cdmaterials`) to one new directory, and
writes the set under the new path. Companion files are copied byte for byte:
they are tied to the MDL by its checksum, which is unchanged.

Only studiohdr_t fields are edited; the new strings are appended after the
original data and `length` grows to cover them, so every existing offset in
the file stays valid.

`read_model()` reads the header facts a map compiler checks a prop role
against (RFC 0009 U2): the $staticprop flag, the movement hull and the
sequence names.
"""

import struct
from pathlib import Path

MDL_ID = b"IDST"
SUPPORTED_VERSIONS = (44, 45, 46, 47, 48, 49)
COMPANIONS = (".vvd", ".dx90.vtx", ".dx80.vtx", ".sw.vtx", ".vtx", ".phy", ".ani")
NAME_OFFSET, NAME_BYTES = 12, 64
LENGTH_OFFSET = 76
TEXTURES_OFFSET = 204      # numtextures, textureindex
CD_TEXTURES_OFFSET = 212   # numcdtextures, cdtextureindex
TEXTURE_RECORD_BYTES = 64  # mstudiotexture_t
HULL_OFFSET = 104          # hull_min, hull_max
FLAGS_OFFSET = 152
SEQUENCES_OFFSET = 188     # numlocalseq, localseqindex
SEQDESC_BYTES = 212        # mstudioseqdesc_t
STUDIOHDR_FLAGS_STATIC_PROP = 0x10


class ModelError(ValueError):
    pass


def c_string(data, offset):
    end = data.index(b"\0", offset)
    return data[offset:end].decode("ascii")


def header(data):
    if len(data) < 408 or data[:4] != MDL_ID:
        raise ModelError("not a studio model")
    version = struct.unpack_from("<i", data, 4)[0]
    if version not in SUPPORTED_VERSIONS:
        raise ModelError("unsupported MDL version %d" % version)
    length = struct.unpack_from("<i", data, LENGTH_OFFSET)[0]
    if length != len(data):
        raise ModelError("MDL length %d differs from its %d bytes" % (length, len(data)))
    return version


def textures(data):
    """Material names (relative to each $cdmaterials directory)."""
    count, index = struct.unpack_from("<ii", data, TEXTURES_OFFSET)
    names = []
    for i in range(count):
        record = index + i * TEXTURE_RECORD_BYTES
        name_offset = struct.unpack_from("<i", data, record)[0]
        names.append(c_string(data, record + name_offset))
    return names


def material_directories(data):
    count, index = struct.unpack_from("<ii", data, CD_TEXTURES_OFFSET)
    return [c_string(data, struct.unpack_from("<i", data, index + 4 * i)[0])
            for i in range(count)]


def rewrite(data, model_name, material_directory):
    """MDL bytes with a new internal name and a single material directory."""
    header(data)
    encoded_name = model_name.encode("ascii")
    if len(encoded_name) >= NAME_BYTES:
        raise ModelError("model name is longer than studiohdr_t allows")
    directory = material_directory.replace("/", "\\").rstrip("\\") + "\\"
    out = bytearray(data)
    out[NAME_OFFSET:NAME_OFFSET + NAME_BYTES] = encoded_name.ljust(NAME_BYTES, b"\0")
    while len(out) % 4:
        out.append(0)
    string_offset = len(out)
    out += directory.encode("ascii") + b"\0"
    while len(out) % 4:
        out.append(0)
    table_offset = len(out)
    out += struct.pack("<i", string_offset)
    struct.pack_into("<ii", out, CD_TEXTURES_OFFSET, 1, table_offset)
    struct.pack_into("<i", out, LENGTH_OFFSET, len(out))
    return bytes(out)


def retarget(resolver, source_model, out_root, model_path, material_directory):
    """Write `source_model` as `model_path` under `out_root`; returns a receipt.

    `model_path` and `source_model` are game paths (`models/...mdl`);
    `material_directory` is relative to `materials/`.
    """
    for path in (source_model, model_path):
        if not path.startswith("models/") or not path.endswith(".mdl"):
            raise ModelError("model paths are models/...mdl: " + path)
    data, origin = resolver.read(source_model)
    if data is None:
        raise ModelError("installed content has no " + source_model)
    version = header(data)
    checksum = struct.unpack_from("<i", data, 8)[0]
    rewritten = rewrite(data, model_path[len("models/"):], material_directory)
    target = Path(out_root) / model_path
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_bytes(rewritten)
    copied = {".mdl": origin}
    stem = source_model[:-len(".mdl")]
    for suffix in COMPANIONS:
        companion, companion_origin = resolver.read(stem + suffix)
        if companion is None:
            continue
        if suffix != ".ani" and len(companion) >= 12:
            # vvd/vtx/phy each record the checksum of the MDL they belong to.
            offsets = {".vvd": 8, ".phy": 12}
            offset = offsets.get(suffix, 16 if suffix.endswith(".vtx") else None)
            if offset is not None:
                found = struct.unpack_from("<i", companion, offset)[0]
                if found != checksum:
                    raise ModelError("%s%s checksum %d differs from the MDL's %d" %
                                     (stem, suffix, found, checksum))
        (target.parent / (target.name[:-len(".mdl")] + suffix)).write_bytes(companion)
        copied[suffix] = companion_origin
    if ".vvd" not in copied or not any(s.endswith(".vtx") for s in copied):
        raise ModelError(source_model + " lacks its vertex (.vvd) or strip (.vtx) data")
    return {"source_model": source_model, "model": model_path, "mdl_version": version,
            "checksum": checksum, "files": copied,
            "source_material_directories": material_directories(data),
            "material_directory": material_directory,
            "textures": textures(data)}


def read_model(data):
    """{version, static_prop, hull: ((min), (max)), sequences: [label]} of a
    studio model, or ModelError."""
    version = header(data)
    flags = struct.unpack_from("<i", data, FLAGS_OFFSET)[0]
    count, index = struct.unpack_from("<ii", data, SEQUENCES_OFFSET)
    if count < 0 or index < 0 or index + count * SEQDESC_BYTES > len(data):
        raise ModelError("sequence table is outside the file")
    sequences = []
    for i in range(count):
        record = index + i * SEQDESC_BYTES
        label = struct.unpack_from("<i", data, record + 4)[0]
        try:
            sequences.append(c_string(data, record + label))
        except (ValueError, UnicodeDecodeError) as error:
            raise ModelError("sequence %d has no readable label: %s" % (i, error))
    return {"version": version, "static_prop": bool(flags & STUDIOHDR_FLAGS_STATIC_PROP),
            "hull": (struct.unpack_from("<3f", data, HULL_OFFSET),
                     struct.unpack_from("<3f", data, HULL_OFFSET + 12)),
            "sequences": sequences}


def find_sequence(model, name):
    """The index of sequence `name`: Source looks labels up without case."""
    return next((i for i, label in enumerate(model["sequences"])
                 if label.lower() == name.lower()), None)
