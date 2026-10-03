#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Write uncompressed VTF 7.2 textures from RGBA8888 bytes.

The one VTF writer for generated, repository-owned art: the Android touch
icons (tools/android/touch_icons.py) and the VGUI fixture materials
(tools/vgui/vgui_fixture_content.py). Layout follows public/vtf/vtf.h: an
80-byte 7.2 header, no low-resolution image, one mip level, and the frames
one after another. Output is a pure function of the arguments.

tools/quality/vtf_decode.py is the independent reader that checks it.
"""

import struct

# public/vtf/vtf.h CompiledVtfFlags
POINTSAMPLE = 0x0001
TRILINEAR = 0x0002
CLAMPS = 0x0004
CLAMPT = 0x0008
SRGB = 0x0040
NOMIP = 0x0100
NOLOD = 0x0200
ONEBITALPHA = 0x1000
EIGHTBITALPHA = 0x2000

IMAGE_FORMAT_RGBA8888 = 0
HEADER_BYTES = 80


def vtf(rgba, width, height, flags, frames=1):
    """Uncompressed single-mip RGBA8888 VTF 7.2. `rgba` holds `frames`
    images of width x height texels, rows top to bottom, one after another."""
    if width <= 0 or height <= 0 or frames <= 0:
        raise ValueError("a texture has a positive size and frame count")
    if width > 0xFFFF or height > 0xFFFF or frames > 0xFFFF:
        raise ValueError("VTF sizes and frame counts are 16-bit")
    if len(rgba) != width * height * 4 * frames:
        raise ValueError("expected %d bytes of RGBA8888, got %d"
                         % (width * height * 4 * frames, len(rgba)))
    header = struct.pack(
        "<4s2iiHHIHH4s3f4sfiBiBBH",
        b"VTF\0", 7, 2, HEADER_BYTES, width, height, flags, frames, 0, b"\0" * 4,
        0.5, 0.5, 0.5, b"\0" * 4, 1.0,
        IMAGE_FORMAT_RGBA8888,
        1,           # mip levels
        -1, 0, 0,    # no low-resolution image
        1)           # depth
    return header.ljust(HEADER_BYTES, b"\0") + bytes(rgba)
