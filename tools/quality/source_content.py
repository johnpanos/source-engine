#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Read installed Source game content: VPK v1/v2 directories and loose files.

`ContentResolver(runtime)` searches a staged runtime in portal/gameinfo.txt
order (Portal VPK, Portal loose files, the shared HL2 VPK and loose files).
Shared by the physics corpus harness and the map pipeline's dynamic-model
step.
"""

import os
import struct

# Search order mirrors portal/gameinfo.txt: Portal VPK, Portal loose files,
# then the shared HL2 VPK and loose files.
SEARCH_PATHS = [
    ("vpk", "portal/portal_pak_dir.vpk"),
    ("dir", "portal"),
    ("vpk", "hl2/hl2_misc_dir.vpk"),
    ("dir", "hl2"),
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


class ContentResolver:
    def __init__(self, runtime):
        self.layers = []
        for kind, relative in SEARCH_PATHS:
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
                candidate = os.path.join(path, relative)
                data = open(candidate, "rb").read() if os.path.isfile(candidate) else None
            if data is not None:
                return data, "%s:%s" % (os.path.basename(path), relative)
        return None, None
