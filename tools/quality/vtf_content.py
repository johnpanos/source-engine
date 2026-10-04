"""VTF texture helpers shared by map content bridges.

`compile_texture` runs the in-repo `vtex` on an in-memory image and returns
the VTF hash; `verify_solid_vtf` checks that a 4x4 solid preview survived
compilation with exact channel bytes (VTEX stores it as BGR888).
"""

import argparse
import hashlib
import json
import struct
import subprocess
from pathlib import Path


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def power_of_two(value):
    return 1 << (value - 1).bit_length()


def compile_texture(image, destination, vtex, clamp=False, kind=None):
    destination = Path(destination)
    destination.parent.mkdir(parents=True, exist_ok=True)
    # VTEX's PNG path ignores sidecar options. Finite surface tiles need its
    # TGA/config path so the native sampler receives CLAMPS and CLAMPT.
    source = destination.with_suffix(".tga" if clamp else ".png")
    image.save(source)
    config = destination.with_suffix(".txt")
    if clamp:
        config.write_text("clamps 1\nclampt 1\n", encoding="utf-8")
    subprocess.run([str(vtex), str(source)],
                   check=True, capture_output=True, text=True, timeout=120)
    encoded = destination.with_suffix(".vtf")
    if not encoded.is_file():
        raise RuntimeError("VTEX did not produce " + str(encoded))
    source.unlink()
    if clamp:
        config.unlink()
    if kind:
        return compress_vtf(encoded, kind)
    return sha256(encoded)


def verify_solid_vtf(path, rgb):
    """Check the compiled 4x4 preview texels against authored RGB channels."""
    payload = Path(path).read_bytes()
    if (len(payload) < 48 or payload[:4] != b"VTF\0" or
            int.from_bytes(payload[16:18], "little") != 4 or
            int.from_bytes(payload[18:20], "little") != 4 or
            int.from_bytes(payload[52:56], "little") != 3 or
            payload[-48:] != bytes((rgb[2], rgb[1], rgb[0])) * 16):
        raise ValueError("VTEX changed solid material channels: " + str(path))


# VTEX has no DXT compressor on POSIX: it writes BGR888/BGRA8888 with a full mip
# chain, and its PNG path sets NOMIP, so the engine sampled mip 0 of an
# uncompressed image. `compress_vtf` rewrites that file's image data as DXT1 (DXT5
# with alpha) and clears NOMIP. VTF 7.5 BC7 (format 70) is not a legacy
# `ImageFormat`, so the legacy loader and the native backend cannot take it.
IMAGE_FORMAT_BGR888 = 3
IMAGE_FORMAT_BGRA8888 = 12
IMAGE_FORMAT_DXT1 = 13
IMAGE_FORMAT_DXT5 = 15
TEXTUREFLAGS_NOMIP = 0x100
TEXTUREFLAGS_ENVMAP = 0x4000
RESOURCE_LOWRES = 0x01
RESOURCE_HIGHRES = 0x30
# Solid-colour previews and small tiles stay exact; compression pays for large images.
MIN_COMPRESSED_EDGE = 64
KINDS = ("color", "data", "normal")


def _mip_extent(size, level):
    return max(1, size >> level)


def _normal_chain(base, count):
    """Mip chain of a tangent-space normal map: vectors averaged, then renormalised."""
    import numpy as np
    vectors = base.astype(np.float32) / 127.5 - 1.0
    chain = []
    for level in range(count):
        if level:
            height, width = vectors.shape[:2]
            if width > 1:
                vectors = vectors[:, 0::2] + vectors[:, 1::2]
            if height > 1:
                vectors = vectors[0::2] + vectors[1::2]
        length = np.sqrt((vectors * vectors).sum(axis=2, keepdims=True))
        unit = vectors / np.maximum(length, 1e-6)
        chain.append(np.clip(np.rint((unit + 1.0) * 127.5), 0, 255).astype(np.uint8))
    return chain


def _encode_blocks(rgba, alpha):
    import numpy as np
    try:
        import etcpak
    except ImportError as error:
        raise RuntimeError("etcpak is required to block-compress VTF images "
                           "(pip install etcpak)") from error
    height, width = rgba.shape[:2]
    padded_w, padded_h = -(-width // 4) * 4, -(-height // 4) * 4
    if (padded_w, padded_h) != (width, height):
        rgba = np.pad(rgba, ((0, padded_h - height), (0, padded_w - width), (0, 0)), mode="edge")
    data = np.ascontiguousarray(rgba).tobytes()
    return (etcpak.compress_bc3 if alpha else etcpak.compress_bc1)(data, padded_w, padded_h)


def compress_vtf(path, kind):
    """Rewrite a VTEX BGR(A)888 VTF as DXT1/DXT5 with mips enabled; returns its sha256.

    kind: "color" and "data" keep VTEX's own mip chain; "normal" rebuilds the
    chain with renormalised vectors. Images under MIN_COMPRESSED_EDGE, files
    already compressed, and layouts this does not model are left untouched.
    """
    import numpy as np
    if kind not in KINDS:
        raise ValueError("unknown VTF kind: " + str(kind))
    path = Path(path)
    data = path.read_bytes()
    if len(data) < 80 or data[:4] != b"VTF\0":
        raise ValueError("not a VTF file: " + str(path))
    major, minor, header_size = struct.unpack_from("<3I", data, 4)
    width, height, flags, frames = struct.unpack_from("<HHIH", data, 16)
    fmt, = struct.unpack_from("<i", data, 52)
    mips = data[56]
    if fmt in (IMAGE_FORMAT_DXT1, IMAGE_FORMAT_DXT5):
        return sha256(path)
    if (major, minor) != (7, 5) or fmt not in (IMAGE_FORMAT_BGR888, IMAGE_FORMAT_BGRA8888) or \
            frames != 1 or flags & TEXTUREFLAGS_ENVMAP or struct.unpack_from("<H", data, 63)[0] != 1:
        raise ValueError("unsupported VTF layout for compression: " + str(path))
    if min(width, height) < MIN_COMPRESSED_EDGE:
        return sha256(path)
    resources = {}
    for index in range(struct.unpack_from("<I", data, 68)[0]):
        tag, offset = struct.unpack_from("<4sI", data, 80 + 8 * index)
        resources[tag[0]] = offset
    if RESOURCE_HIGHRES not in resources or resources[RESOURCE_HIGHRES] > len(data):
        raise ValueError("VTF has no high-resolution image resource: " + str(path))
    start = resources[RESOURCE_HIGHRES]
    channels = 4 if fmt == IMAGE_FORMAT_BGRA8888 else 3
    extents = [(_mip_extent(width, i), _mip_extent(height, i)) for i in range(mips)]
    if len(data) - start != sum(w * h * channels for w, h in extents):
        raise ValueError("VTF image data does not match its header: " + str(path))
    # Stored smallest mip first.
    levels = [None] * mips
    cursor = start
    for level in range(mips - 1, -1, -1):
        w, h = extents[level]
        levels[level] = np.frombuffer(data, np.uint8, w * h * channels, cursor).reshape(h, w, channels)
        cursor += w * h * channels
    alpha = channels == 4
    if kind == "normal":
        if alpha:
            raise ValueError("normal map with alpha is not modelled: " + str(path))
        rgb_levels = _normal_chain(levels[0][..., ::-1], mips)
    else:
        rgb_levels = [level[..., 2::-1] for level in levels]
    blocks = []
    for level in range(mips - 1, -1, -1):
        rgb = rgb_levels[level]
        if alpha:
            rgba = np.dstack((rgb, levels[level][..., 3]))
        else:
            rgba = np.dstack((rgb, np.full(rgb.shape[:2], 255, np.uint8)))
        blocks.append(_encode_blocks(rgba, alpha))
    output = bytearray(data[:start])
    struct.pack_into("<I", output, 20, flags & ~TEXTUREFLAGS_NOMIP)
    struct.pack_into("<i", output, 52, IMAGE_FORMAT_DXT5 if alpha else IMAGE_FORMAT_DXT1)
    output += b"".join(blocks)
    temporary = path.with_name(path.name + ".tmp")
    temporary.write_bytes(bytes(output))
    temporary.replace(path)
    return sha256(path)


def recompress_content(root, receipt=None):
    """Block-compress an existing playable-content root's PBR textures in place.

    Uses each file's own exported mip-0 BGR(A)888 data, so no bake or source
    texture is needed. Updates the per-material VTF hashes in `receipt` (the
    build's content.json) when given. Returns (files changed, bytes before, after).
    """
    root = Path(root)
    kinds = {"basecolor": "color", "mrao": "data", "normal": "normal"}
    record = json.loads(Path(receipt).read_text()) if receipt else None
    changed = before = after = 0
    for path in sorted(root.glob("materials/*/*/*.vtf")):
        kind = kinds.get(path.stem)
        if not kind:
            continue
        size = path.stat().st_size
        digest = compress_vtf(path, kind)
        new_size = path.stat().st_size
        before, after = before + size, after + new_size
        if new_size != size:
            changed += 1
        if record is not None:
            entry = record["materials"].get(path.parent.name)
            key = path.stem + "_vtf_sha256"
            if entry is not None and entry.get(key) not in (None, digest):
                entry[key] = digest
    if record is not None:
        Path(receipt).write_text(json.dumps(record, indent=2, sort_keys=True) + "\n")
    return changed, before, after


def main():
    parser = argparse.ArgumentParser(description=recompress_content.__doc__)
    parser.add_argument("root", type=Path, help="content root containing materials/")
    parser.add_argument("--receipt", type=Path, help="content.json whose VTF hashes to update")
    args = parser.parse_args()
    changed, before, after = recompress_content(args.root, args.receipt)
    print("recompressed %d VTFs: %.1f MiB -> %.1f MiB" % (changed, before / 2**20, after / 2**20))


if __name__ == "__main__":
    main()
