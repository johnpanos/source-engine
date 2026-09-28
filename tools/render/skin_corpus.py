#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Skinning corpus for RFC 0016 K6 (render.skinning.corpus).

    python3 tools/render/skin_corpus.py capture --build INSTALL --runtime RUNTIME \\
        --out DIR [--map MAP ...] [--game portal|portal2] [--meshes N]
    python3 tools/render/skin_corpus.py compact CAPTURE... --out FILE [--poses N]
    python3 tools/render/skin_corpus.py stats FILE

capture  boots each map headless on native Vulkan with software skinning
         (+mat_softwareskin 1, so studiorender's R_StudioSoftwareProcessMesh
         skins every model) and SOURCE_SKIN_CAPTURE set, writing
         DIR/<map>.skcap (format in studiorender/skin_capture.h). The build
         must contain studiorender's capture hook. A failed boot fails.
compact  merges captures, keeping each distinct mesh (same vertex inputs)
         in at most N distinct poses (palettes), default 3, so repeated
         frames do not dominate. Prints the vertex and bone-count histogram.
stats    prints meshes, vertices, bones-per-vertex and flexed counts.

The GPU suite reads the compacted file from $RENDER_SKIN_CORPUS. Python 3
standard library only.
"""

import argparse
import collections
import hashlib
import os
import struct
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
MAGIC = b"SKCAP001"
VERTEX_BYTES = 100  # in 40, bones 4, weights 12, flags 4, out 40
DEFAULT_MAPS = {"portal": ["testchmb_a_00", "testchmb_a_08", "testchmb_a_01", "escape_00"],
                "portal2": ["sp_a1_wakeup"]}


def meshes(path):
    """(vertex count, bone count, palette bytes, vertex bytes) per mesh."""
    data = Path(path).read_bytes()
    if data[:8] != MAGIC:
        raise ValueError("%s: not a skin capture" % path)
    at = 8
    while at < len(data):
        if data[at:at + 4] != b"MESH" or at + 12 > len(data):
            raise ValueError("%s: bad mesh header at byte %d" % (path, at))
        vertices, bones = struct.unpack_from("<II", data, at + 4)
        at += 12
        palette = data[at:at + bones * 48]
        at += bones * 48
        block = data[at:at + vertices * VERTEX_BYTES]
        at += vertices * VERTEX_BYTES
        if len(palette) != bones * 48 or len(block) != vertices * VERTEX_BYTES:
            raise ValueError("%s: truncated mesh" % path)
        yield vertices, bones, palette, block


def vertex_inputs(block, vertices):
    """The per-vertex input bytes (everything but the legacy output)."""
    return b"".join(block[v * VERTEX_BYTES:v * VERTEX_BYTES + 60] for v in range(vertices))


def stats(path):
    count = vertices = flexed = 0
    bones = collections.Counter()
    for n, _, _, block in meshes(path):
        count += 1
        vertices += n
        for v in range(n):
            bones[block[v * VERTEX_BYTES + 40]] += 1
            flexed += struct.unpack_from("<I", block, v * VERTEX_BYTES + 56)[0] & 1
    return {"meshes": count, "vertices": vertices, "bones_per_vertex": dict(sorted(bones.items())),
            "flexed": flexed}


def compact(captures, out, poses):
    kept = collections.defaultdict(set)
    written = 0
    with open(out, "wb") as sink:
        sink.write(MAGIC)
        for path in captures:
            for n, b, palette, block in meshes(path):
                shape = hashlib.sha256(vertex_inputs(block, n)).hexdigest()
                pose = hashlib.sha256(palette).hexdigest()
                if pose in kept[shape] or len(kept[shape]) >= poses:
                    continue
                kept[shape].add(pose)
                sink.write(b"MESH" + struct.pack("<II", n, b) + palette + block)
                written += 1
    print("skin_corpus: %d meshes (%d distinct) written to %s" % (written, len(kept), out))
    return written


def capture(args):
    out = Path(args.out)
    out.mkdir(parents=True, exist_ok=True)
    maps = args.map or DEFAULT_MAPS[args.game]
    failures = 0
    for name in maps:
        env = dict(os.environ, SOURCE_SKIN_CAPTURE=str(out / (name + ".skcap")),
                   SOURCE_SKIN_CAPTURE_MESHES=str(args.meshes))
        command = [sys.executable, str(ROOT / "tools/quality/portal_boot.py"), "--runtime", args.runtime,
                   "--build", args.build, "--renderer", "native-vulkan", "--headless", "--game", args.game,
                   "--map", name, "--startup-command", "mat_softwareskin 1", "--out", str(out / ("boot-" + name))]
        result = subprocess.run(command, env=env, capture_output=True, text=True)
        ok = result.returncode == 0 and (out / (name + ".skcap")).exists()
        failures += not ok
        print("skin_corpus: %s %s" % (name, "captured" if ok else "FAILED (%s)" % result.stdout.strip()[-300:]))
    return 1 if failures else 0


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = parser.add_subparsers(dest="command", required=True)
    cap = sub.add_parser("capture")
    cap.add_argument("--build", required=True)
    cap.add_argument("--runtime", required=True)
    cap.add_argument("--out", required=True)
    cap.add_argument("--map", action="append")
    cap.add_argument("--game", choices=sorted(DEFAULT_MAPS), default="portal")
    cap.add_argument("--meshes", type=int, default=1500)
    com = sub.add_parser("compact")
    com.add_argument("captures", nargs="+")
    com.add_argument("--out", required=True)
    com.add_argument("--poses", type=int, default=3)
    st = sub.add_parser("stats")
    st.add_argument("file")
    args = parser.parse_args(argv)
    if args.command == "capture":
        return capture(args)
    if args.command == "compact":
        written = compact(args.captures, args.out, args.poses)
        print("skin_corpus:", stats(args.out))
        return 0 if written else 1
    print(stats(args.file))
    return 0


if __name__ == "__main__":
    sys.exit(main())
