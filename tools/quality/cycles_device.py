"""Cycles device policy for the map, bake and GI tools (no Blender import).

Bakes and previews run on the GPU: HIP on the AMD Linux host, failing when
Cycles finds none rather than silently taking hours on the CPU. Correctness
checks run on the CPU: reference renders, analytic and held-out oracles, and
fixture builds that oracles judge. On this host a CPU bake is bit-identical
across runs at the same seed, while two HIP bakes of the same scene differ, so
only the CPU satisfies the RFC 0007 `Exact` determinism class. CPU+GPU hybrid
rendering was measured about 24% slower than the GPU alone on the shared
CPU/GPU power budget of this host, so it is not offered.

`auto` (GPU when Cycles finds one, else CPU) is an explicit opt-in for
GPU-less hosts; its results are not reproducible across hosts.
"""

DEVICES = ("gpu", "auto", "cpu")
BAKE_DEVICE = "gpu"
CHECK_DEVICE = "cpu"
