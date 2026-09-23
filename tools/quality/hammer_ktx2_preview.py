#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Exercise the GTK Hammer product with a packaged KTX2 material preview."""

import argparse
from collections import defaultdict
import hashlib
import json
from pathlib import Path
import re
import struct
import subprocess
import tempfile

import conformance


ROOT = Path(__file__).resolve().parents[2]
MAGIC = bytes.fromhex("ab4b5458203230bb0d0a1a0a")
MATERIALS = ("dev_measurewall01a", "dev_measuregeneric01b")


class GateFailure(ValueError):
    pass


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def build_vpk(package, corrupt):
    files = {}
    for name in MATERIALS:
        files[f"materials/dev/{name}.vmt"] = (
            f'"LightmappedGeneric" {{ "$basetexture" "dev/{name}" }}'.encode()
        )
        files[f"materials/dev/{name}.ktx2"] = (
            package.replace(MAGIC, b"\0" * len(MAGIC), 1) if corrupt else package
        )
    branches = defaultdict(lambda: defaultdict(list))
    for path, data in files.items():
        stem, extension = path.rsplit(".", 1)
        directory, name = stem.rsplit("/", 1)
        branches[extension][directory].append((name, data))
    tree = bytearray()
    payload = bytearray()
    for extension, directories in sorted(branches.items()):
        tree += extension.encode() + b"\0"
        for directory, entries in sorted(directories.items()):
            tree += directory.encode() + b"\0"
            for name, data in sorted(entries):
                tree += name.encode() + b"\0"
                tree += struct.pack("<IHHIIH", 0, 0, 0x7FFF, len(payload), len(data), 0xFFFF)
                payload += data
            tree += b"\0"
        tree += b"\0"
    tree += b"\0"
    header = struct.pack("<IIIIIII", 0x55AA1234, 2, len(tree), len(payload), 0, 0, 0)
    return header + tree + payload


def read_ppm(path):
    data = Path(path).read_bytes()
    match = re.match(rb"P6\n(\d+) (\d+)\n255\n", data)
    if not match:
        raise GateFailure("Hammer output is not an unambiguous P6 image")
    width, height = (int(group) for group in match.groups())
    pixels = data[match.end():]
    if (width, height) != (640, 480) or len(pixels) != width * height * 3:
        raise GateFailure("Hammer output has incomplete or wrong-sized pixels")
    red = sum(1 for i in range(0, len(pixels), 3)
              if pixels[i] > 40 and pixels[i] > pixels[i + 1] * 2
              and pixels[i] > pixels[i + 2] * 2)
    return red, pixels


def run_product(binary, vmf, vpk, ppm):
    process = subprocess.run(
        [str(binary), "--textured", str(ppm), str(vmf), str(vpk),
         "--width", "640", "--height", "480"],
        cwd=ROOT, capture_output=True, text=True, timeout=60)
    if process.returncode != 0 or not ppm.is_file() or \
            "textured: wrote" not in process.stdout:
        raise GateFailure("Hammer product failed to render: " + process.stderr[-1000:])
    return process


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--binary", type=Path, required=True)
    parser.add_argument("--fixture", type=Path,
                        default=ROOT / "quality/fixtures/ktx2/red-8x8-rgba8-mips.ktx2")
    parser.add_argument("--vmf", type=Path, default=ROOT / "hammer/gtk/samples/room.vmf")
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args(argv)
    evidence = {"outcome": "fail"}
    try:
        binary = args.binary.resolve(strict=True)
        vmf = args.vmf.resolve(strict=True)
        package = args.fixture.read_bytes()
        if package.count(MAGIC) != 1:
            raise GateFailure("the pinned fixture is missing one KTX2 signature")
        with tempfile.TemporaryDirectory(prefix="hammer-ktx2-") as work:
            work = Path(work)
            runs = []
            images = []
            for label, corrupt in (("valid", False), ("corrupt", True)):
                vpk = work / f"{label}_dir.vpk"
                ppm = work / f"{label}.ppm"
                vpk.write_bytes(build_vpk(package, corrupt))
                process = run_product(binary, vmf, vpk, ppm)
                red, pixels = read_ppm(ppm)
                runs.append({"label": label, "red_pixels": red,
                             "image_sha256": sha256(ppm), "vpk_sha256": sha256(vpk),
                             "stdout": process.stdout, "stderr": process.stderr})
                images.append(pixels)
            if runs[0]["red_pixels"] < 1000 or runs[1]["red_pixels"] != 0 or \
                    images[0] == images[1]:
                raise GateFailure("KTX2 positive and malformed controls did not diverge")
            evidence.update({"outcome": "pass", "runs": runs,
                             "binary_sha256": sha256(binary), "fixture_sha256": sha256(args.fixture),
                             "vmf_sha256": sha256(vmf),
                             "source_identity": conformance.source_identity(str(ROOT))})
    except (OSError, ValueError, subprocess.SubprocessError) as error:
        evidence["error"] = str(error)
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(evidence, indent=2, sort_keys=True) + "\n")
    print(f"Hammer KTX2 preview: {evidence['outcome']} ({args.out})")
    return 0 if evidence["outcome"] == "pass" else 1


if __name__ == "__main__":
    raise SystemExit(main())
