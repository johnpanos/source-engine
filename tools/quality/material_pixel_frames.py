# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Whole-frame captures of material pixel conformance families.

Families that judge full frames (portal, modellight) have the harness write each
case's frame to a raw RGB file next to the report. The runner moves the frames
into the capture, where each one travels as base64 of zlib-compressed RGB rows
(top row first).
"""

import base64
from pathlib import Path
import zlib


class CaptureError(ValueError):
    pass


def encode_frame(rgb):
    return base64.b64encode(zlib.compress(bytes(rgb), 9)).decode("ascii")


def decode_frame(case, frame):
    data = zlib.decompress(base64.b64decode(case["frame"]))
    if len(data) != frame[0] * frame[1] * 3:
        raise CaptureError("case %s holds %d bytes, not a %dx%d RGB frame"
                           % (case["name"], len(data), frame[0], frame[1]))
    return data


def embed_frames(report, directory):
    """Moves each case's raw frame file into the capture."""
    width, height = report["frame"]
    for case in report["cases"]:
        path = Path(directory) / case.pop("frame_file")
        rgb = path.read_bytes()
        if len(rgb) != width * height * 3:
            raise CaptureError("%s holds %d bytes, not a %dx%d RGB frame"
                               % (path, len(rgb), width, height))
        case["frame"] = encode_frame(rgb)
        path.unlink()
    return report


def validate(path, report, cases):
    """The capture holds exactly `cases`, in order, each with a whole frame."""
    names = [case.get("name") for case in report.get("cases", [])]
    if names != list(cases):
        raise CaptureError("%s has %s cases %s, expected %s"
                           % (path, report.get("family"), names, list(cases)))
    if len(report.get("frame", [])) != 2:
        raise CaptureError("%s has no frame size" % path)
    for case in report["cases"]:
        if "frame" not in case:
            raise CaptureError("%s: case %s has no frame" % (path, case["name"]))
        decode_frame(case, report["frame"])
    return report


def write_png(path, width, height, rgb):
    """A plain PNG of a frame, for looking at captures."""
    raw = b"".join(b"\x00" + bytes(rgb[y * width * 3:(y + 1) * width * 3])
                   for y in range(height))

    def chunk(kind, data):
        body = kind + data
        return (len(data).to_bytes(4, "big") + body +
                (zlib.crc32(body) & 0xffffffff).to_bytes(4, "big"))

    header = width.to_bytes(4, "big") + height.to_bytes(4, "big") + b"\x08\x02\x00\x00\x00"
    Path(path).write_bytes(b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", header) +
                           chunk(b"IDAT", zlib.compress(raw, 9)) + chunk(b"IEND", b""))
