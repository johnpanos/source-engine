#!/usr/bin/env python3
"""The traced producers' probe focus for a camera position, from a map's BSP
and PRBV: an independent implementation of the engine host's ProbeFocus
(engine/indirect_light_host.cpp), for benches and for checking the host.

    python3 tools/quality/gi_focus.py --bsp map.bsp --prbv probe_volume.prbv \\
        --eye X Y Z [--radius 1500] [--out focus.txt]

The focus is the active probes with a cluster (that of an open leaf at the
probe or half a spacing along an axis) among those the camera's cluster can see
(its PVS, and itself) and those clusters' neighbours (clusters whose open
leaves' bounds, one unit apart, touch); by distance (the probes within
--radius) when that does not narrow the map: no visibility, the camera in no
cluster, no probe found, or more than three quarters of the active probes.
Prints a JSON summary; --out writes the probe indices, one per line (the
bench's --focus input).
"""

import argparse
import json
import sys
from pathlib import Path

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parent))
import legacy_bsp  # noqa: E402
import probe_volume  # noqa: E402


def probe_positions(volume):
    """(N, 3) positions and (N,) active flags of the first grid's probes."""
    grid = volume.grids[0]
    positions, active = [], []
    for i in range(grid.count):
        index = (i % grid.dims[0], (i // grid.dims[0]) % grid.dims[1],
                 i // (grid.dims[0] * grid.dims[1]))
        state = grid.state(i)
        positions.append(grid.origin + np.asarray(index) * grid.spacing + state[:3])
        active.append(state[3] >= 0.5)
    return np.asarray(positions), np.asarray(active)


def neighbours(contents, clusters, mins, maxs):
    """Per cluster, the clusters whose open leaves touch its own."""
    open_leaf = ((contents & legacy_bsp.CONTENTS_SOLID) == 0) & (clusters >= 0)
    lo, hi, cl = mins[open_leaf] - 1.0, maxs[open_leaf] + 1.0, clusters[open_leaf]
    count = int(clusters.max()) + 1 if len(clusters) else 0
    result = [set() for _ in range(count)]
    for i in range(len(cl)):
        touch = (lo <= hi[i]).all(axis=1) & (hi >= lo[i]).all(axis=1) & (cl != cl[i])
        for other in np.unique(cl[touch]):
            result[cl[i]].add(int(other))
    return result


def probe_clusters(bsp, clusters, positions, active, spacing):
    """(N, 7) clusters of each probe's centre and its six half-spacing
    offsets (-1: solid, none, or an inactive probe)."""
    offsets = [np.zeros(3)]
    for axis in range(3):
        for sign in (0.5, -0.5):
            step = np.zeros(3)
            step[axis] = sign * spacing[axis]
            offsets.append(step)
    return np.array([[clusters[bsp.point_leaf(p + o)] if a else -1 for o in offsets]
                     for p, a in zip(positions, active)])


def focus(bsp, volume, eye, radius):
    positions, active = probe_positions(volume)
    pvs = bsp.visibility()
    contents, clusters, mins, maxs = bsp.leaves()
    spacing = volume.grids[0].spacing if volume is not None else np.ones(3)
    probe_cluster = probe_clusters(bsp, clusters, positions, active, spacing)
    camera = int(clusters[bsp.point_leaf(np.asarray(eye, np.float64))]) if pvs is not None else -1
    chosen, mode = [], "distance"
    if pvs is not None and 0 <= camera < len(pvs):
        seen = pvs[camera].copy()
        seen[camera] = True
        near = seen.copy()
        for c, others in enumerate(neighbours(contents, clusters, mins, maxs)):
            if seen[c]:
                for other in others:
                    near[other] = True
        hit = (probe_cluster >= 0) & near[np.maximum(probe_cluster, 0)]
        chosen = [int(p) for p in np.flatnonzero(hit.any(axis=1))]
        if chosen and len(chosen) * 4 <= int(active.sum()) * 3:
            mode = "visibility"
    if mode == "distance":
        distance = np.linalg.norm(positions - np.asarray(eye, np.float64), axis=1)
        chosen = [int(p) for p in np.flatnonzero(active & (distance <= radius))]
    return chosen, {"mode": mode, "camera_cluster": camera, "focus": len(chosen),
                    "active": int(active.sum()), "probes": int(len(active))}


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--bsp", type=Path, required=True)
    parser.add_argument("--prbv", type=Path, required=True)
    parser.add_argument("--eye", type=float, nargs=3, required=True)
    parser.add_argument("--radius", type=float, default=1500.0)
    parser.add_argument("--out", type=Path)
    args = parser.parse_args()
    chosen, summary = focus(legacy_bsp.LegacyBsp.read(args.bsp),
                            probe_volume.read(args.prbv), args.eye, args.radius)
    if args.out:
        args.out.write_text("".join("%d\n" % p for p in chosen))
    print(json.dumps(summary))


if __name__ == "__main__":
    main()
