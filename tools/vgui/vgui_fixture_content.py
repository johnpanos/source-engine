#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""VGUI fixture materials (RFC 0010 V0 fixed-screen corpus).

Repository-owned materials and textures to draw VGUI with, so the surface,
its counters, the draw-list recorder and its consumers can be exercised
without licensed game content. Every texture is drawn here from closed-form
rules (no image library, font or third-party art), written as uncompressed
VTF 7.2 by tools/quality/vtf_write.py, and paired with a VMT. Output is
deterministic: the same script writes byte-identical files.

    python3 tools/vgui/vgui_fixture_content.py write OUT_DIR
    python3 tools/vgui/vgui_fixture_content.py check
    python3 tools/vgui/vgui_fixture_content.py record [--update]

`write` lays the files out as a game directory's `materials/` tree, ready to
mount as a search path. `check` regenerates them in memory and requires:

- each file's SHA-256 and size equal the recorded manifest
  (quality/fixtures/vgui-surface/materials.json), so a generator change is a
  reviewed fixture change, never silent;
- each texture, decoded by the independent reader tools/quality/vtf_decode.py,
  has its probe texels: literal expected values written in SPECS below, apart
  from the code that draws the pixels;
- each VMT, parsed by tools/render/material_inventory.py, names its texture,
  and the blend it declares is the blend the surface derives from its flags
  (vguimatsurface/MatSystemSurface.cpp RecordQuads: $additive is additive;
  else $translucent or $vertexalpha is alpha; else opaque).

- the engine's own VTF container reader (texturecontainer::vtf, which
  vtf/vtf.cpp loads every game texture through) reads each probe texel too:
  `check` builds unittests/vguitest/vgui_fixture_reader.cpp with the
  linux-headless-core profile's flags (compiler $CXX, default g++) and runs it
  on the written files.

`check` prints one checks-v1 record. `record` writes the manifest; it refuses
to replace one without --update.

The set:

- quadrants: red, green, blue and white quarters: orientation, UVs, vertex color;
- alpha_ramp: white with alpha equal to the column: alpha blending;
- additive: a uniform color drawn with $additive: additive blending;
- opaque: a color with zero alpha drawn without $translucent: the texture's
  alpha must be ignored (a consumer that reads it draws nothing);
- checker: a one-texel checkerboard with point sampling: texel alignment;
- atlas: 4 x 4 distinct cells: DrawTexturedSubRect texture coordinates;
- frames: three solid frames, red, green and blue: DrawSetTextureFrame ($frame);
- wide: a 64 x 16 red ramp: a non-square texture;
- tinted: the quadrants texture under a material $color: state only the
  material holds (the draw list's legacy-material case);
- vgui/cursors/*: the thirteen software cursors the surface loads at start
  (vguimatsurface/Cursor.cpp), each a distinct outline glyph.
"""

import argparse
import hashlib
import json
import os
import re
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools" / "quality"))
import vtf_write  # noqa: E402

MANIFEST = "quality/fixtures/vgui-surface/materials.json"
MANIFEST_SCHEMA = "vgui-fixture-materials/v1"

# Flags of every generated texture: clamped, one mip, no LOD reduction.
BASE_FLAGS = vtf_write.CLAMPS | vtf_write.CLAMPT | vtf_write.NOMIP | vtf_write.NOLOD

# VMT keys by blend, as the game's VGUI image materials write them.
VMT_KEYS = {
    "alpha": [("$vertexcolor", "1"), ("$vertexalpha", "1"), ("$translucent", "1"),
              ("$ignorez", "1"), ("$no_fullbright", "1")],
    "additive": [("$vertexcolor", "1"), ("$vertexalpha", "1"), ("$additive", "1"),
                 ("$ignorez", "1"), ("$no_fullbright", "1")],
    "opaque": [("$vertexcolor", "1"), ("$ignorez", "1"), ("$no_fullbright", "1")],
}

CURSORS = ["arrow", "ibeam", "hourglass", "crosshair", "waitarrow", "up", "sizenwse",
           "sizenesw", "sizewe", "sizens", "sizeall", "no", "hand"]


# ---------------------------------------------------------------------------
# Texel rules: (x, y, frame) -> (r, g, b, a)
# ---------------------------------------------------------------------------

def quadrants(x, y, frame):
    left, top = x < 32, y < 32
    if top:
        return (255, 0, 0, 255) if left else (0, 255, 0, 255)
    return (0, 0, 255, 255) if left else (255, 255, 255, 255)


def alpha_ramp(x, y, frame):
    return (255, 255, 255, x)


def additive(x, y, frame):
    return (128, 64, 32, 255)


def opaque(x, y, frame):
    return (40, 120, 200, 0)


def checker(x, y, frame):
    return (255, 255, 255, 255) if (x + y) % 2 == 0 else (0, 0, 0, 255)


def atlas(x, y, frame):
    column, row = x // 32, y // 32
    return (column * 64 + 32, row * 64 + 32, 128, 255)


def frames(x, y, frame):
    return ((255, 0, 0, 255), (0, 255, 0, 255), (0, 0, 255, 255))[frame]


def wide(x, y, frame):
    return (x * 4 + 2, 0, 0, 255)


def cursor(index):
    """A 32 x 32 outline square, and a bar of index + 1 two-texel cells on
    row 16 so each cursor is distinct; transparent elsewhere."""
    def rule(x, y, frame):
        border = x in (0, 31) or y in (0, 31)
        bar = y in (16, 17) and 2 <= x < 2 + 2 * (index + 1)
        return (255, 255, 255, 255) if border or bar else (0, 0, 0, 0)
    return rule


# ---------------------------------------------------------------------------
# The set. Probes are literal: (frame, x, y, (r, g, b, a)).
# ---------------------------------------------------------------------------

def _spec(name, rule, width, height, blend, probes, frame_count=1, flags=0, texture=None,
          extra_keys=(), purpose=""):
    return {"name": name, "rule": rule, "width": width, "height": height, "frames": frame_count,
            "flags": BASE_FLAGS | flags | (vtf_write.EIGHTBITALPHA if blend != "opaque" else 0),
            "blend": blend, "probes": probes, "texture": texture or name,
            "extra_keys": list(extra_keys), "purpose": purpose}


SPECS = [
    _spec("vgui/fixture/quadrants", quadrants, 64, 64, "alpha",
          [(0, 0, 0, (255, 0, 0, 255)), (0, 63, 0, (0, 255, 0, 255)),
           (0, 0, 63, (0, 0, 255, 255)), (0, 63, 63, (255, 255, 255, 255)),
           (0, 31, 31, (255, 0, 0, 255)), (0, 32, 32, (255, 255, 255, 255))],
          purpose="orientation, texture coordinates and vertex color modulation"),
    _spec("vgui/fixture/alpha_ramp", alpha_ramp, 256, 4, "alpha",
          [(0, 0, 0, (255, 255, 255, 0)), (0, 128, 2, (255, 255, 255, 128)),
           (0, 255, 3, (255, 255, 255, 255))],
          purpose="alpha blending: alpha equals the column"),
    _spec("vgui/fixture/additive", additive, 64, 64, "additive",
          [(0, 0, 0, (128, 64, 32, 255)), (0, 63, 63, (128, 64, 32, 255))],
          purpose="additive blending"),
    _spec("vgui/fixture/opaque", opaque, 64, 64, "opaque",
          [(0, 10, 10, (40, 120, 200, 0))],
          purpose="opaque blend ignores the texture's zero alpha"),
    _spec("vgui/fixture/checker", checker, 64, 64, "alpha",
          [(0, 0, 0, (255, 255, 255, 255)), (0, 1, 0, (0, 0, 0, 255)),
           (0, 0, 1, (0, 0, 0, 255)), (0, 63, 63, (255, 255, 255, 255))],
          flags=vtf_write.POINTSAMPLE, purpose="texel alignment under point sampling"),
    _spec("vgui/fixture/atlas", atlas, 128, 128, "alpha",
          [(0, 0, 0, (32, 32, 128, 255)), (0, 127, 0, (224, 32, 128, 255)),
           (0, 0, 127, (32, 224, 128, 255)), (0, 70, 40, (160, 96, 128, 255))],
          purpose="DrawTexturedSubRect: cell (column, row) is (64 c + 32, 64 r + 32, 128)"),
    _spec("vgui/fixture/frames", frames, 16, 16, "alpha",
          [(0, 8, 8, (255, 0, 0, 255)), (1, 8, 8, (0, 255, 0, 255)), (2, 8, 8, (0, 0, 255, 255))],
          frame_count=3, purpose="DrawSetTextureFrame selects $frame 0, 1 and 2"),
    _spec("vgui/fixture/wide", wide, 64, 16, "alpha",
          [(0, 0, 0, (2, 0, 0, 255)), (0, 63, 15, (254, 0, 0, 255)), (0, 32, 8, (130, 0, 0, 255))],
          purpose="a non-square texture: red is 4 x + 2"),
    _spec("vgui/fixture/tinted", None, 64, 64, "alpha", [], texture="vgui/fixture/quadrants",
          extra_keys=[("$color", "[0.5 0.5 1]")],
          purpose="material-only state: the quadrants under $color (legacy-material draws)"),
] + [
    _spec("vgui/cursors/" + name, cursor(index), 32, 32, "alpha",
          [(0, 0, 0, (255, 255, 255, 255)), (0, 16, 8, (0, 0, 0, 0)),
           (0, 2 + 2 * index, 16, (255, 255, 255, 255)),
           (0, 3 + 2 * (index + 1), 16, (0, 0, 0, 0))],
          purpose="software cursor %d (vguimatsurface/Cursor.cpp)" % index)
    for index, name in enumerate(CURSORS)
]


# ---------------------------------------------------------------------------
# Generation
# ---------------------------------------------------------------------------

def texels(spec):
    out = bytearray()
    for frame in range(spec["frames"]):
        for y in range(spec["height"]):
            for x in range(spec["width"]):
                out += bytes(spec["rule"](x, y, frame))
    return bytes(out)


def vmt_text(spec):
    lines = ['"UnlitGeneric"', "{", '\t"$basetexture" "%s"' % spec["texture"]]
    for key, value in VMT_KEYS[spec["blend"]] + spec["extra_keys"]:
        lines.append('\t"%s" "%s"' % (key, value))
    lines.append("}")
    return "\n".join(lines) + "\n"


def files():
    """{relative path under the game directory: bytes} for the whole set."""
    out = {}
    for spec in SPECS:
        if spec["rule"] is not None:
            out["materials/%s.vtf" % spec["name"]] = vtf_write.vtf(
                texels(spec), spec["width"], spec["height"], spec["flags"], spec["frames"])
        out["materials/%s.vmt" % spec["name"]] = vmt_text(spec).encode()
    return out


def sha256(data):
    return hashlib.sha256(data).hexdigest()


def manifest_of(generated):
    return {
        "schema": MANIFEST_SCHEMA,
        "generator": "tools/vgui/vgui_fixture_content.py",
        "note": "Generated, never hand-edited: `record --update` after a reviewed generator change.",
        "materials": [
            {"name": spec["name"], "texture": spec["texture"], "blend": spec["blend"],
             "width": spec["width"], "height": spec["height"], "frames": spec["frames"],
             "flags": spec["flags"], "purpose": spec["purpose"],
             "probes": [{"frame": f, "x": x, "y": y, "rgba": list(rgba)}
                        for f, x, y, rgba in spec["probes"]]}
            for spec in SPECS],
        "files": {path: {"sha256": sha256(data), "bytes": len(data)}
                  for path, data in sorted(generated.items())},
    }


# ---------------------------------------------------------------------------
# Checks
# ---------------------------------------------------------------------------

def derived_blend(pairs):
    """The surface's blend for a material's flags (MatSystemSurface.cpp RecordQuads)."""
    flags = {key.lower(): value.strip() for key, value in pairs}
    if flags.get("$additive") == "1":
        return "additive"
    if flags.get("$translucent") == "1" or flags.get("$vertexalpha") == "1":
        return "alpha"
    return "opaque"


READER_SOURCES = ["texturecontainer/vtf/container.cpp",
                  "unittests/vguitest/vgui_fixture_reader.cpp"]
READER_PROFILE = "linux-headless-core"


def native_reader(generated, work, cxx):
    """Build the C++ reader and run it on the written set. Returns
    (checks, failures, output lines without its own record), or raises
    RuntimeError when it cannot be built or reports no record."""
    import conformance  # noqa: E402  (the shared runner's profiles and build commands)

    game = Path(work) / "game"
    for path, data in generated.items():
        target = game / path
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(data)
    probes = []
    for spec in SPECS:
        if spec["rule"] is None:
            continue
        for frame, x, y, rgba in spec["probes"]:
            probes.append("%s %d %d %d %d %d %d %d %d %d %d" % (
                game / ("materials/%s.vtf" % spec["name"]), spec["width"], spec["height"],
                spec["frames"], frame, x, y, *rgba))
    probe_list = Path(work) / "probes.txt"
    probe_list.write_text("\n".join(probes) + "\n")
    manifest = conformance.load_json(str(ROOT / "quality" / "conformance.manifest.json"))
    profile = conformance.load_profile(
        str(ROOT / manifest.get("profiles_dir", "quality/profiles")), READER_PROFILE)
    suite = {"id": "vgui.fixture-materials.reader", "units": [
        {"id": "reader", "dialect": profile.get("dialect", "cxx20"), "sources": READER_SOURCES}]}
    binary = str(Path(work) / "vgui_fixture_reader")
    for command in conformance.unit_build_commands(str(ROOT), cxx, profile, suite, binary):
        built = subprocess.run(command, cwd=ROOT, capture_output=True, text=True)
        if built.returncode != 0:
            raise RuntimeError("building the reader failed:\n" + (built.stdout + built.stderr)[-3000:])
    run = subprocess.run([binary, str(probe_list)], cwd=ROOT, capture_output=True, text=True,
                         timeout=120)
    record = re.search(r"^CONFORMANCE (\d+) (\d+)$", run.stdout, re.M)
    if record is None:
        raise RuntimeError("the reader reported no record (exit %d):\n%s"
                           % (run.returncode, (run.stdout + run.stderr)[-3000:]))
    lines = [line for line in run.stdout.splitlines() if not line.startswith("CONFORMANCE ")]
    return int(record.group(1)), int(record.group(2)), lines


def check(generated, recorded, work=None, cxx=None):
    sys.path.insert(0, str(ROOT / "tools" / "render"))
    import material_inventory  # noqa: E402  (an independent VMT reader)
    import vtf_decode  # noqa: E402  (an independent VTF reader)

    checks = failures = 0

    def verdict(ok, label):
        nonlocal checks, failures
        checks += 1
        failures += 0 if ok else 1
        print("%s %s" % ("PASS" if ok else "FAIL", label))

    verdict(recorded.get("schema") == MANIFEST_SCHEMA, "manifest schema " + MANIFEST_SCHEMA)
    expected = recorded.get("files", {})
    verdict(sorted(expected) == sorted(generated), "the generated file set equals the manifest's")
    for path in sorted(generated):
        data = generated[path]
        entry = expected.get(path, {})
        verdict(entry.get("sha256") == sha256(data) and entry.get("bytes") == len(data),
                "%s matches its recorded SHA-256 and size" % path)

    for spec in SPECS:
        name = spec["name"]
        vmt = generated.get("materials/%s.vmt" % name)
        verdict(vmt is not None, "%s.vmt is in the set" % name)
        if vmt is None:
            continue
        root = material_inventory.parse_kv(vmt.decode())
        shader = root.children[0] if root.children else None
        pairs = shader.pairs if shader is not None else []
        keys = {key.lower(): value for key, value in pairs}
        verdict(shader is not None and shader.name == "UnlitGeneric" and
                keys.get("$basetexture") == spec["texture"],
                "%s.vmt is UnlitGeneric on %s" % (name, spec["texture"]))
        verdict(derived_blend(pairs) == spec["blend"],
                "%s: the surface derives blend %s from its flags (declared %s)"
                % (name, derived_blend(pairs), spec["blend"]))
        if spec["rule"] is None:
            verdict("materials/%s.vtf" % spec["texture"] in generated,
                    "%s: its texture %s is in the set" % (name, spec["texture"]))
            continue
        data = generated.get("materials/%s.vtf" % name)
        verdict(data is not None, "%s.vtf is in the set" % name)
        if data is None:
            continue
        head = vtf_decode.header(data)
        verdict(head["width"] == spec["width"] and head["height"] == spec["height"] and
                head["frames"] == spec["frames"] and head["flags"] == spec["flags"] and
                head["format"] == vtf_write.IMAGE_FORMAT_RGBA8888 and head["mips"] == 1,
                "%s.vtf header: %dx%d, %d frame(s), flags 0x%x, RGBA8888, one mip"
                % (name, spec["width"], spec["height"], spec["frames"], spec["flags"]))
        for frame, x, y, rgba in spec["probes"]:
            image = decode_frame(vtf_decode, data, head, frame)
            actual = tuple(int(c) for c in image[y][x])
            verdict(actual == tuple(rgba), "%s frame %d texel (%d, %d) is %s (read %s)"
                    % (name, frame, x, y, tuple(rgba), actual))
    if work is not None:
        try:
            native_checks, native_failures, lines = native_reader(generated, work, cxx)
            for line in lines:
                print("native " + line)
            checks += native_checks
            failures += native_failures
            verdict(native_checks > 0, "the engine's VTF reader checked %d probes" % native_checks)
        except (RuntimeError, OSError, subprocess.SubprocessError) as error:
            verdict(False, "the engine's VTF reader ran: %s" % error)
    print("CONFORMANCE %d %d" % (checks, failures))
    sys.stdout.flush()
    return 0 if checks > 0 and failures == 0 else 1


def decode_frame(vtf_decode, data, head, frame):
    """Frame `frame` through the independent reader. It decodes frame 0, so a
    later frame is decoded from a copy whose image data starts at that frame."""
    if frame == 0:
        return vtf_decode.decode(data)[0]
    size = head["width"] * head["height"] * 4
    start = head["header_size"] + frame * size
    shifted = data[:head["header_size"]] + data[start:start + size]
    return vtf_decode.decode(shifted)[0]


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    sub = parser.add_subparsers(dest="command", required=True)
    write_parser = sub.add_parser("write", help="write the set as a game directory's materials/")
    write_parser.add_argument("out", type=Path)
    check_parser = sub.add_parser("check", help="regenerate and check against the manifest and the readers")
    check_parser.add_argument("--out", type=Path, help="work directory (default: a temporary one)")
    check_parser.add_argument("--cxx", default=os.environ.get("CXX", "g++"))
    check_parser.add_argument("--no-native", action="store_true",
                              help="skip the engine's VTF reader (Python readers only)")
    record_parser = sub.add_parser("record", help="write the manifest")
    record_parser.add_argument("--update", action="store_true")
    args = parser.parse_args(argv)

    generated = files()
    manifest_path = ROOT / MANIFEST
    if args.command == "write":
        for path, data in generated.items():
            target = args.out / path
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(data)
        print("%d VGUI fixture files -> %s" % (len(generated), args.out))
        return 0
    if args.command == "record":
        if manifest_path.exists() and not args.update:
            print("%s exists; use `check`, or --update after a reviewed generator change"
                  % MANIFEST, file=sys.stderr)
            return 2
        manifest_path.parent.mkdir(parents=True, exist_ok=True)
        manifest_path.write_text(json.dumps(manifest_of(generated), indent=2) + "\n")
        print("recorded %s" % MANIFEST)
        return 0
    try:
        recorded = json.loads(manifest_path.read_text())
    except (OSError, ValueError) as error:
        print("error: cannot read %s: %s" % (MANIFEST, error), file=sys.stderr)
        return 2
    if args.no_native:
        return check(generated, recorded)
    if args.out:
        args.out.mkdir(parents=True, exist_ok=True)
        return check(generated, recorded, str(args.out), args.cxx)
    with tempfile.TemporaryDirectory(prefix="vgui_fixture_") as work:
        return check(generated, recorded, work, args.cxx)


if __name__ == "__main__":
    sys.exit(main())
