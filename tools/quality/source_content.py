#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Read installed Source game content: VPK v1/v2 directories and loose files.

`ContentResolver(runtime)` searches a staged runtime in portal/gameinfo.txt
order (Portal VPK, Portal loose files, the shared HL2 texture and misc VPKs
and loose files). A staged Portal 2 runtime (kiln package portal2)
is searched in portal2/gameinfo.txt order instead. Shared by the physics corpus
harness, the map pipeline's dynamic-model step, the legacy relight scene and
the VMF compile (tools/quality/vmf_map_build.py).
"""

import os
import json
from pathlib import Path
import struct

# Search order mirrors portal/gameinfo.txt: Portal VPK, Portal loose files,
# then the shared HL2 texture and misc VPKs and loose files.
SEARCH_PATHS = [
    ("vpk", "portal/portal_pak_dir.vpk"),
    ("dir", "portal"),
    ("vpk", "hl2/hl2_textures_dir.vpk"),
    ("vpk", "hl2/hl2_misc_dir.vpk"),
    ("dir", "hl2"),
]

# portal2/gameinfo.txt order: the update, DLC2 and DLC1 packs and loose files,
# then Portal 2's own pack and loose files.
PORTAL2_SEARCH_PATHS = [
    ("vpk", "update/pak01_dir.vpk"),
    ("dir", "update"),
    ("vpk", "portal2_dlc2/pak01_dir.vpk"),
    ("dir", "portal2_dlc2"),
    ("vpk", "portal2_dlc1/pak01_dir.vpk"),
    ("dir", "portal2_dlc1"),
    ("vpk", "portal2/pak01_dir.vpk"),
    ("dir", "portal2"),
]


class VpkDirectory:
    """Minimal reader for Valve VPK v1/v2 directory files."""

    def __init__(self, path):
        self.path = path
        self.entries = {}
        with open(path, "rb") as stream:
            data = stream.read()
        signature, version, tree_size = struct.unpack_from("<III", data, 0)
        if signature != 0x55AA1234 or version not in (1, 2):
            raise ValueError("%s: not a VPK directory (version %r)" % (path, version))
        header_size = 12 if version == 1 else 28
        self.data_base = header_size + tree_size
        self.dir_data = data
        pos = header_size

        def read_string():
            nonlocal pos
            end = data.index(b"\0", pos)
            text = data[pos:end].decode("utf-8", "replace")
            pos = end + 1
            return text

        while True:
            ext = read_string()
            if not ext:
                break
            while True:
                folder = read_string()
                if not folder:
                    break
                while True:
                    name = read_string()
                    if not name:
                        break
                    crc, preload, archive, offset, length, terminator = struct.unpack_from("<IHHIIH", data, pos)
                    pos += 18
                    preload_bytes = data[pos:pos + preload]
                    pos += preload
                    folder_part = "" if folder == " " else folder + "/"
                    self.entries[(folder_part + name + "." + ext).lower()] = (
                        archive, offset, length, preload_bytes)

    def read(self, relative):
        entry = self.entries.get(relative.lower())
        if entry is None:
            return None
        archive, offset, length, preload = entry
        if archive == 0x7FFF:
            start = self.data_base + offset
            return preload + self.dir_data[start:start + length]
        archive_path = self.path.replace("_dir.vpk", "_%03d.vpk" % archive)
        with open(archive_path, "rb") as stream:
            stream.seek(offset)
            return preload + stream.read(length)


# A staged F-Stop runtime (kiln package fstop) is searched in
# fstop/gameinfo.txt order: the fstop game directory, the Portal order, then
# Valve's F-Stop-era depot content (lower-case mirrors).
FSTOP_SEARCH_PATHS = [("dir", "fstop")] + SEARCH_PATHS + [
    ("dir", "fstop_valve"),
    ("dir", "fstop_valve_tempcontent"),
]


class ContentResolver:
    def __init__(self, runtime, archives=()):
        self.layers = [("vpk", str(path), VpkDirectory(str(path))) for path in archives]
        portal2 = os.path.isfile(os.path.join(runtime, "portal2/pak01_dir.vpk"))
        fstop = os.path.isfile(os.path.join(runtime, "fstop/gameinfo.txt"))
        order = PORTAL2_SEARCH_PATHS if portal2 else FSTOP_SEARCH_PATHS if fstop else SEARCH_PATHS
        for kind, relative in order:
            path = os.path.join(runtime, relative)
            if kind == "vpk" and os.path.isfile(path):
                self.layers.append(("vpk", path, VpkDirectory(path)))
            elif kind == "dir" and os.path.isdir(path):
                self.layers.append(("dir", path, None))
        if not self.layers:
            raise FileNotFoundError("no game content under %s" % runtime)

    def read(self, relative):
        for kind, path, vpk in self.layers:
            if kind == "vpk":
                data = vpk.read(relative)
            else:
                # The engine folds path case; the F-Stop mirrors are lower case.
                candidate = os.path.join(path, relative)
                if not os.path.isfile(candidate):
                    candidate = os.path.join(path, relative.lower())
                data = open(candidate, "rb").read() if os.path.isfile(candidate) else None
            if data is not None:
                return data, "%s:%s" % (os.path.basename(path), relative)
        return None, None


def material_overrides(path):
    """Explicit authored replacements; archives stay read-only external inputs.

    This is map content, not a second global mount or quality policy. The
    compiled scene supplies both the baker and the generated runtime materials.
    """
    path = Path(path).resolve()
    data = json.loads(path.read_text())
    if data.get("schema") != "source-material-overrides/v1":
        raise ValueError("invalid material override schema")
    archives = []
    for source in data.get("archives", []):
        archive = (path.parent / Path(source["path"]).expanduser()).resolve()
        if not archive.is_file() or not archive.name.endswith("_dir.vpk"):
            raise ValueError("missing VPK directory: " + str(archive))
        archives.append(dict(source, path=str(archive)))
    if not archives:
        raise ValueError("material overrides require external archives")
    mapping = data.get("materials", {})
    def valid(name):
        return (isinstance(name, str) and name == name.lower() and
                not name.startswith("/") and "\\" not in name and
                all(p not in ("", ".", "..") for p in name.split("/")) and
                not name.endswith((".vmt", ".vtf")))
    if not mapping or not all(valid(k) and valid(v) for k, v in mapping.items()):
        raise ValueError("material overrides require normalized explicit logical names")
    return {"schema": data["schema"], "archives": archives, "materials": mapping}
