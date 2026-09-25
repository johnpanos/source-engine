"""Cycles device policy for the map, bake and GI tools (no Blender import).

Bakes, previews and correctness checks run on the CPU (user decision,
2026-09-25). On 2026-09-25 this host's HIP compute wedged (amdgpu MES failed to
respond to REMOVE_QUEUE): every Cycles GPU job then hung in hipStreamCreate
until a reboot, while CPU bakes kept working. The CPU is also the only device
with bit-identical results, and the one where Cycles offers path guiding.
`gpu` (HIP, failing when Cycles finds none) remains an explicit opt-in per
manifest or profile. On this host a CPU bake is bit-identical
across runs at the same seed, while two HIP bakes of the same scene differ, so
only the CPU satisfies the RFC 0007 `Exact` determinism class. CPU+GPU hybrid
rendering was measured about 24% slower than the GPU alone on the shared
CPU/GPU power budget of this host, so it is not offered.

`auto` (GPU when Cycles finds one, else CPU) is an explicit opt-in for
GPU-less hosts; its results are not reproducible across hosts.
"""

DEVICES = ("gpu", "auto", "cpu")
BAKE_DEVICE = "cpu"
CHECK_DEVICE = "cpu"


def determinism(device, denoised):
    """RFC 0007 determinism class of a Cycles result from `device` (as
    `configure_cycles` reports it): `exact` for an undenoised CPU result,
    otherwise `statistical`. OIDN output is statistical until a measurement
    (bake_determinism.py) shows it bit-stable on the profile."""
    return "exact" if device == "CPU" and not denoised else "statistical"
