"""VTF texture helpers shared by map content bridges.

`compile_texture` runs the in-repo `vtex` on an in-memory image and returns
the VTF hash; `verify_solid_vtf` checks that a 4x4 solid preview survived
compilation with exact channel bytes (VTEX stores it as BGR888).
"""

import hashlib
import subprocess
from pathlib import Path


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def power_of_two(value):
    return 1 << (value - 1).bit_length()


def compile_texture(image, destination, vtex, clamp=False):
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
