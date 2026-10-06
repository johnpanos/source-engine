#!/usr/bin/env python3
"""Block-compressed (BC) encoding and decoding for baked HDR data.

Encoding follows RFC 0008 "Textures: KTX2": the pinned KTX-Software `ktx`
encodes a master once to UASTC (UASTC HDR 4x4 for HDR data) and transcodes it
to the device format. Here that is

  encode_bc6h(rgb)   float RGB (>= 0)            -> BC6H unsigned, 16 B / 4x4
  encode_bc7(rgba8)  uint8 RGBA                   -> BC7 unorm,     16 B / 4x4
  encode_bc4(r8)     uint8 single channel         -> BC4 unorm,      8 B / 4x4

each returning the raw blocks, row-major by block. The decoders run the
vendored external/bcdec (MIT) through a small helper compiled on first use,
so a bake can measure what it shipped (`error_report`).

Run its fixtures with:  python3 -m unittest tools/texture/tests/test_bc_codec.py
"""

import ctypes
import hashlib
import struct
import subprocess
import tempfile
from pathlib import Path

import numpy as np

ROOT = Path(__file__).resolve().parents[2]
BCDEC = ROOT / "external" / "bcdec" / "bcdec.h"
KTX2_MAGIC = b"\xabKTX 20\xbb\r\n\x1a\n"
VK_FORMAT = {"bc4": 139, "bc6hu": 143, "bc7": 145}
BLOCK_BYTES = {"bc4": 8, "bc6hu": 16, "bc7": 16}

_HELPER_SOURCE = r"""
#define BCDEC_IMPLEMENTATION
#include "bcdec.h"
static void place( int width, int height, int bx, int by, int channels, int size,
    const unsigned char *tile, unsigned char *out )
{
    for ( int y = 0; y < 4; ++y )
        for ( int x = 0; x < 4; ++x )
        {
            const int px = bx * 4 + x, py = by * 4 + y;
            if ( px < width && py < height )
                for ( int c = 0; c < channels * size; ++c )
                    out[( py * width + px ) * channels * size + c] =
                        tile[( y * 4 + x ) * channels * size + c];
        }
}
void decode_bc6h( const unsigned char *blocks, int width, int height, float *out )
{
    const int bw = ( width + 3 ) / 4, bh = ( height + 3 ) / 4;
    for ( int by = 0; by < bh; ++by )
        for ( int bx = 0; bx < bw; ++bx )
        {
            float tile[48];
            bcdec_bc6h_float( blocks + 16 * ( by * bw + bx ), tile, 12, 0 );
            place( width, height, bx, by, 3, 4, (const unsigned char *)tile, (unsigned char *)out );
        }
}
void decode_bc7( const unsigned char *blocks, int width, int height, unsigned char *out )
{
    const int bw = ( width + 3 ) / 4, bh = ( height + 3 ) / 4;
    for ( int by = 0; by < bh; ++by )
        for ( int bx = 0; bx < bw; ++bx )
        {
            unsigned char tile[64];
            bcdec_bc7( blocks + 16 * ( by * bw + bx ), tile, 16 );
            place( width, height, bx, by, 4, 1, tile, out );
        }
}
void decode_bc4( const unsigned char *blocks, int width, int height, unsigned char *out )
{
    const int bw = ( width + 3 ) / 4, bh = ( height + 3 ) / 4;
    for ( int by = 0; by < bh; ++by )
        for ( int bx = 0; bx < bw; ++bx )
        {
            unsigned char tile[16];
            bcdec_bc4( blocks + 8 * ( by * bw + bx ), tile, 4 );
            place( width, height, bx, by, 1, 1, tile, out );
        }
}
"""


class CodecError(Exception):
    pass


def default_tool():
    """The pinned ktx from build/toolchains (the map toolchain records it), or
    None when no toolchain is installed."""
    import json
    for name in ("pbrt-map-toolchain.json", "pbrt-map-toolchain-bazzite.json"):
        path = ROOT / "build" / "toolchains" / name
        if path.is_file():
            tool = json.loads(path.read_text()).get("ktx")
            if tool and Path(tool).is_file():
                return Path(tool)
    return None


# The pinned encoder refuses a source wider or taller than 16384 texels
# (basis_universal's BASISU_MAX_SUPPORTED_TEXTURE_DIMENSION). Taller images go
# through in bands of whole block rows: blocks are independent and stored
# row-major, so the bands' blocks concatenate to the whole image's.
MAX_ENCODE_DIMENSION = 16384
MAX_ENCODE_ROWS = MAX_ENCODE_DIMENSION  # a multiple of four


def block_bytes(target, width, height):
    """Bytes of `target` blocks covering width x height (partial blocks padded)."""
    return ((width + 3) // 4) * ((height + 3) // 4) * BLOCK_BYTES[target]


def _ktx2_level0(path, expected_format):
    data = Path(path).read_bytes()
    if data[:12] != KTX2_MAGIC:
        raise CodecError("%s is not KTX2" % path)
    vk_format, = struct.unpack_from("<I", data, 12)
    if vk_format != expected_format:
        raise CodecError("ktx produced VkFormat %d, expected %d" % (vk_format, expected_format))
    offset, length, _ = struct.unpack_from("<QQQ", data, 80)
    return data[offset:offset + length]


def _encode_bands(tool, texels, vk_source, codec, target, extra=()):
    """Blocks of `texels` (height, width, channels; the source format's
    dtype), encoded in bands of MAX_ENCODE_ROWS rows."""
    height, width = texels.shape[:2]
    if width > MAX_ENCODE_DIMENSION:
        raise CodecError("%d texels wide: the encoder takes at most %d"
                         % (width, MAX_ENCODE_DIMENSION))
    if MAX_ENCODE_ROWS % 4:
        raise CodecError("bands must be whole block rows")
    blocks = []
    for top in range(0, height, MAX_ENCODE_ROWS):
        band = np.ascontiguousarray(texels[top:top + MAX_ENCODE_ROWS])
        blocks.append(_encode(tool, band.tobytes(), vk_source, width, band.shape[0], codec, target,
                              extra))
    return b"".join(blocks)


def _encode(tool, raw, vk_source, width, height, codec, target, extra=()):
    with tempfile.TemporaryDirectory(prefix="bc-codec-") as work:
        work = Path(work)
        source = work / "source.raw"
        source.write_bytes(raw)
        master = work / "master.ktx2"
        output = work / "output.ktx2"
        for command in ([str(tool), "create", "--raw", "--format", vk_source, "--width", str(width),
                         "--height", str(height), "--encode", codec, *extra, str(source),
                         str(master)],
                        [str(tool), "transcode", "--target", target, str(master), str(output)]):
            result = subprocess.run(command, capture_output=True, text=True)
            if result.returncode:
                raise CodecError("%s failed: %s" % (command[1], result.stderr.strip()[-400:]))
        blocks = _ktx2_level0(output, VK_FORMAT[target])
    if len(blocks) != block_bytes(target, width, height):
        raise CodecError("%s blocks have %d bytes, expected %d" %
                         (target, len(blocks), block_bytes(target, width, height)))
    return blocks


def encode_bc6h(tool, rgb):
    """BC6H unsigned blocks of float RGB (height, width, 3), every value >= 0."""
    rgb = np.asarray(rgb, dtype=np.float32)
    if rgb.ndim != 3 or rgb.shape[2] != 3 or not np.isfinite(rgb).all() or rgb.min() < 0:
        raise CodecError("BC6H takes finite non-negative RGB")
    height, width, _ = rgb.shape
    rgba = np.concatenate([rgb, np.ones((height, width, 1), np.float32)], axis=2)
    return _encode_bands(tool, rgba.astype("<f2"), "R16G16B16A16_SFLOAT", "uastc-hdr-4x4", "bc6hu")


def encode_bc7(tool, rgba8):
    """BC7 unorm blocks of uint8 RGBA (height, width, 4)."""
    rgba8 = np.ascontiguousarray(rgba8, dtype=np.uint8)
    if rgba8.ndim != 3 or rgba8.shape[2] != 4:
        raise CodecError("BC7 takes uint8 RGBA")
    return _encode_bands(tool, rgba8, "R8G8B8A8_UNORM", "uastc", "bc7", ("--uastc-quality", "4"))


def encode_bc4(tool, r8):
    """BC4 unorm blocks of one uint8 channel (height, width)."""
    r8 = np.ascontiguousarray(r8, dtype=np.uint8)
    if r8.ndim != 2:
        raise CodecError("BC4 takes one uint8 channel")
    rgba = np.repeat(r8[..., None], 4, axis=2)
    rgba[..., 3] = 255
    return _encode_bands(tool, rgba, "R8G8B8A8_UNORM", "uastc", "bc4", ("--uastc-quality", "4"))


_library = None


def _helper():
    """The bcdec helper, compiled into build/ once per bcdec revision."""
    global _library
    if _library is None:
        if not BCDEC.is_file():
            raise CodecError("external/bcdec is missing")
        digest = hashlib.sha256(BCDEC.read_bytes() + _HELPER_SOURCE.encode()).hexdigest()[:16]
        out = ROOT / "build" / "bc_codec" / ("libbcdecode-%s.so" % digest)
        if not out.is_file():
            out.parent.mkdir(parents=True, exist_ok=True)
            source = out.with_suffix(".c")
            source.write_text(_HELPER_SOURCE)
            temporary = out.with_name(out.name + ".tmp")
            result = subprocess.run(["cc", "-O2", "-shared", "-fPIC", "-I", str(BCDEC.parent),
                                     str(source), "-o", str(temporary)], capture_output=True,
                                    text=True)
            if result.returncode:
                raise CodecError("cannot build the bcdec helper: " + result.stderr[-400:])
            temporary.replace(out)
        _library = ctypes.CDLL(str(out))
    return _library


def _decode(name, blocks, width, height, shape, dtype, target):
    if len(blocks) != block_bytes(target, width, height):
        raise CodecError("%s: %d bytes do not cover %dx%d" % (target, len(blocks), width, height))
    out = np.zeros(shape, dtype)
    pointer = ctypes.POINTER(ctypes.c_float if dtype == np.float32 else ctypes.c_ubyte)
    getattr(_helper(), name)(bytes(blocks), width, height, out.ctypes.data_as(pointer))
    return out


def decode_bc6h(blocks, width, height):
    return _decode("decode_bc6h", blocks, width, height, (height, width, 3), np.float32, "bc6hu")


def decode_bc7(blocks, width, height):
    return _decode("decode_bc7", blocks, width, height, (height, width, 4), np.uint8, "bc7")


def decode_bc4(blocks, width, height):
    return _decode("decode_bc4", blocks, width, height, (height, width), np.uint8, "bc4")


def hdr_error(reference, decoded, floor=1e-3):
    """Relative error per channel and log2 luminance error over lit texels."""
    reference = np.asarray(reference, np.float32)
    decoded = np.asarray(decoded, np.float32)
    weights = np.array([0.2126, 0.7152, 0.0722], np.float32)
    luminance = reference @ weights
    lit = luminance > floor
    if not lit.any():
        return {"lit_texels": 0}
    relative = np.abs(decoded - reference) / np.maximum(reference, floor)
    stops = np.abs(np.log2(np.maximum(decoded @ weights, floor)) - np.log2(np.maximum(luminance,
                                                                                         floor)))
    return {"lit_texels": int(lit.sum()),
            "relative_mean": float(relative[lit].mean()),
            "relative_p99": float(np.percentile(relative[lit], 99)),
            "stops_mean": float(stops[lit].mean()),
            "stops_p99": float(np.percentile(stops[lit], 99)),
            "stops_max": float(stops[lit].max())}
