#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Which reflection probes a map's real surfaces blend, and from how far.

Samples the visible world faces of a BSP2 map (area-weighted, sky, nodraw
and tool faces skipped, nudged off the surface along their normal) and runs
the runtime's probe selection at each sample with the face's own normal:
reflection_probes.glsl's rank-ordered shares, facing term, tie rules and the
two-probe blend (ReflectionProbesRadianceDebug). It reports, by surface
area, how much is reflected by the global probe and by "open" probes (a fit
face that saw sky or void and so reaches OPEN_EXTENT), and how far those
pixels are from the capture they reflect.

A reflection taken far from its capture shows another place: the metric
`open_far_area_share` is the share of surface area whose blend gives more
than half its weight to an open probe whose capture is farther than --far
units away.

  python3 tools/quality/reflection_probe_coverage.py --map MAP.bsp [--far 500] [--json OUT]
"""

import argparse
import json
from pathlib import Path
import sys

import numpy as np

import bsp2_reader
import legacy_bsp
import reflection_probe
import reflection_probe_set as rps

FACING_EDGE = 0.1  # kReflectionProbeFacingEdge
SKIP_FLAGS = (legacy_bsp.SURF_SKY2D | legacy_bsp.SURF_SKY | legacy_bsp.SURF_NODRAW |
              legacy_bsp.SURF_TRIGGER | legacy_bsp.SURF_HINT | legacy_bsp.SURF_SKIP)
OPEN_REACH = 0.5 * reflection_probe.OPEN_EXTENT * rps.SOURCE_UNITS_PER_METER


def load(path):
    data = Path(path).read_bytes()
    kind, bsp2 = bsp2_reader.open_any(data)
    if kind != "bsp2":
        raise ValueError("expects a BSP2 map")
    entry = bsp2.by_id.get(bsp2_reader.fourcc("RPRB"))
    if entry is None:
        raise ValueError("the map has no RPRB lump")
    return legacy_bsp.LegacyBsp(bsp2.export_legacy()), rps.read(bsp2.lump(entry))


def surface_samples(bsp, count, rng, nudge=1.0):
    texinfo = bsp.texinfo()
    faces = [f for f in bsp.world_faces()
             if f["dispinfo"] < 0 and not texinfo[f["texinfo"]]["flags"] & SKIP_FLAGS]
    triangles, normals = [], []
    for face in faces:
        p = face["points"]
        for i in range(1, len(p) - 1):
            triangles.append((p[0], p[i], p[i + 1]))
            normals.append(face["plane_normal"])
    tri = np.array(triangles)
    normal = np.array(normals, dtype=np.float64)
    area = 0.5 * np.linalg.norm(np.cross(tri[:, 1] - tri[:, 0], tri[:, 2] - tri[:, 0]), axis=1)
    pick = rng.choice(len(tri), size=count, p=area / area.sum())
    u, v = rng.random(count), rng.random(count)
    flip = u + v > 1
    u[flip], v[flip] = 1 - u[flip], 1 - v[flip]
    a, b, c = tri[pick, 0], tri[pick, 1], tri[pick, 2]
    points = a + (b - a) * u[:, None] + (c - a) * v[:, None] + normal[pick] * nudge
    return points, normal[pick], float(area.sum())


def smoothstep(e0, e1, x):
    t = np.clip((x - e0) / (e1 - e0), 0.0, 1.0)
    return t * t * (3 - 2 * t)


def select(probes, points, normals):
    """ranks (N, 2) and weights (N, 2) as the runtime blends them."""
    order = sorted(probes, key=lambda p: p["rank"])
    n = len(points)
    remaining = np.ones(n)
    top0, top1, third = -np.ones(n), -np.ones(n), np.zeros(n)
    rank0, rank1 = np.zeros(n, int), np.zeros(n, int)
    for probe in order:
        if probe["global"]:
            weight = np.ones(n)
        else:
            outside = np.maximum(np.maximum(probe["influence_min"] - points,
                                            points - probe["influence_max"]), 0.0)
            toward = probe["capture"] - points
            facing = np.sum(normals * toward, axis=1) / np.maximum(np.linalg.norm(toward, axis=1), 1e-9)
            weight = ((1 - smoothstep(0.0, probe["fade"], np.linalg.norm(outside, axis=1))) *
                      smoothstep(-FACING_EDGE, FACING_EDGE, facing))
        share = weight * remaining
        remaining = remaining - share
        first = share > top0
        second = ~first & (share > top1)
        rest = ~first & ~second
        third = np.where(first | second, np.maximum(third, top1), np.maximum(third, share))
        top1 = np.where(first, top0, np.where(second, share, top1))
        rank1 = np.where(first, rank0, np.where(second, probe["rank"], rank1))
        top0 = np.where(first, share, top0)
        rank0 = np.where(first, probe["rank"], rank0)
        third = np.where(rest, np.maximum(third, share), third)
    w0, w1 = top0 - third, np.where(len(probes) > 1, top1 - third, 0.0)
    total = w0 + w1
    tiny = total <= 1e-12
    w0, w1 = np.where(tiny, 1.0, w0), np.where(tiny, 1.0 if len(probes) > 1 else 0.0, w1)
    total = w0 + w1
    return np.stack([rank0, rank1], 1), np.stack([w0 / total, w1 / total], 1)


def is_open(probe):
    if probe["global"]:
        return False
    reach = np.maximum(probe["capture"] - probe["influence_min"], probe["influence_max"] - probe["capture"])
    return bool(np.any(reach >= OPEN_REACH))


def analyze(bsp, layout, samples=40000, far=500.0, seed=11):
    rng = np.random.default_rng(seed)
    points, normals, area = surface_samples(bsp, samples, rng)
    probes = layout["probes"]
    by_rank = {p["rank"]: p for p in probes}
    ranks, weights = select(probes, points, normals)
    open_ranks = {p["rank"] for p in probes if is_open(p)}
    global_rank = next(p["rank"] for p in probes if p["global"])
    lead = np.argmax(weights, axis=1)
    lead_rank = ranks[np.arange(len(points)), lead]
    lead_weight = weights[np.arange(len(points)), lead]
    capture = np.array([by_rank[r]["capture"] for r in lead_rank])
    distance = np.linalg.norm(capture - points, axis=1)
    open_lead = np.isin(lead_rank, list(open_ranks)) & (lead_weight > 0.5)
    local = [p for p in probes if not p["global"] and p["rank"] not in open_ranks]
    nearest = np.min(np.linalg.norm(np.array([p["capture"] for p in local])[None] - points[:, None], axis=2),
                     axis=1) if local else np.full(len(points), np.inf)
    share = lambda mask: round(float(np.mean(mask)), 4)
    return {
        "samples": len(points), "surface_area": round(area, 1), "probes": len(probes),
        "open_probes": sorted(int(r) for r in open_ranks),
        "global_lead_area_share": share((lead_rank == global_rank) & (lead_weight > 0.5)),
        "open_lead_area_share": share(open_lead),
        "open_far_area_share": share(open_lead & (distance > far)),
        "open_far_with_nearer_room_probe_share": share(open_lead & (distance > far) & (nearest < distance)),
        "open_lead_distance_percentiles": [round(float(v), 1) for v in
                                           (np.percentile(distance[open_lead], [25, 50, 75, 95])
                                            if open_lead.any() else [])],
        "nearest_room_capture_percentiles_where_open_leads": [
            round(float(v), 1) for v in (np.percentile(nearest[open_lead], [25, 50, 75, 95])
                                         if open_lead.any() else [])],
        "far_units": far, "seed": seed,
    }


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--map", required=True, type=Path)
    parser.add_argument("--samples", type=int, default=40000)
    parser.add_argument("--far", type=float, default=500.0)
    parser.add_argument("--seed", type=int, default=11)
    parser.add_argument("--json", type=Path)
    args = parser.parse_args(argv)
    bsp, layout = load(args.map)
    result = dict(analyze(bsp, layout, args.samples, args.far, args.seed), map=str(args.map))
    text = json.dumps(result, indent=1)
    if args.json:
        args.json.write_text(text + "\n")
    print(text)
    return 0


if __name__ == "__main__":
    sys.exit(main())
