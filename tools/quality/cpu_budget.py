"""How many CPUs this process may actually use (no Blender import).

`os.cpu_count()` is the machine's thread count. In a container it can be far
more than the process may use: a rented vast.ai host reported 96 CPUs with a
cgroup quota of 46, and the bakes that sized their forked ray-cast workers by
the count ran twice as many workers as the quota allowed, throttled. The
budget is the smallest of the CPU affinity mask and the cgroup CPU quota
(cgroup v2 `cpu.max`, or v1 `cpu.cfs_quota_us` / `cpu.cfs_period_us`).
"""

import math
import os
from pathlib import Path

CGROUP = Path("/sys/fs/cgroup")


def quota(root=CGROUP):
    """The cgroup CPU quota in CPUs, or None when there is none."""
    v2 = root / "cpu.max"
    try:
        limit, period = v2.read_text().split()[:2]
        return None if limit == "max" else int(limit) / int(period)
    except (OSError, ValueError):
        pass
    try:
        limit = int((root / "cpu" / "cpu.cfs_quota_us").read_text())
        period = int((root / "cpu" / "cpu.cfs_period_us").read_text())
        return None if limit <= 0 else limit / period
    except (OSError, ValueError):
        return None


def available_cpus(root=CGROUP):
    """CPUs this process may use: affinity, capped by the cgroup quota."""
    try:
        count = len(os.sched_getaffinity(0))
    except (AttributeError, OSError):
        count = os.cpu_count() or 1
    limit = quota(root)
    if limit is not None:
        count = min(count, max(1, math.floor(limit)))
    return max(1, count)
