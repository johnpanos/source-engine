"""Cycles device policy for the map, bake and GI tools (no Blender import).

Map bakes default to `gpu` (user decision, 2026-09-30); this requires a usable
Cycles GPU and fails if one is unavailable. CPU remains an explicit choice for
the bake-determinism profile: at the same seed it is bit-identical across runs
and is the only device that satisfies RFC 0007's `Exact` determinism class.
`auto` (GPU when Cycles finds one, else CPU) is an explicit opt-in for GPU-less
hosts; its results are not reproducible across hosts. Correctness/reference
checks remain on the CPU by default.
"""

DEVICES = ("gpu", "auto", "cpu")
BAKE_DEVICE = "gpu"
CHECK_DEVICE = "cpu"


def determinism(device, denoised):
    """RFC 0007 determinism class of a Cycles result from `device` (as
    `configure_cycles` reports it): `exact` for an undenoised CPU result,
    otherwise `statistical`. OIDN output is statistical until a measurement
    (bake_determinism.py) shows it bit-stable on the profile."""
    return "exact" if device == "CPU" and not denoised else "statistical"
