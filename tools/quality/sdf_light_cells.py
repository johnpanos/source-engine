#!/usr/bin/env python3
"""The SDFV light cells: which analytic lights may reach each region.

The SDF-traced producer shades a probe and every ray hit with the lights its
cell lists (sdf_volume.py documents the encoding), so its cost follows the
lights near a point, not the map's total. `build` decides the lists; the
bake (sdf_volume_bake.py) owns the call. Everything is in Source units and
the lightmap's diffuse unit (irradiance / pi), the SDFV's.

A light reaches a cell unless one of these proves it cannot light any point
of it:

  range      its brightest possible light at the cell's nearest point is
             below `cutoff`: a rect L A / (pi d^2), a sphere or spot L r^2 / d^2
             (d from the light's surface); a styled light counts at
             STYLE_HEADROOM x, the brightest the stock styles drive a light;
  side       the cell lies wholly behind a one-sided rect or spot;
  visibility (a compiled map's PVS) no cluster of the cell's open leaves is
             potentially visible from a cluster of the light's open leaves.

Each test only removes a light that contributes nothing (or less than the
cutoff), so the lists are conservative. Distant lights and the dome reach
every cell. A light that touches no open leaf (buried in a brush) and a
leaf without a cluster are treated as seeing everything.
"""

import math

import numpy as np

STYLE_HEADROOM = 2.1   # the stock light styles peak at 'z' = 25/12 of 'm'
LEAF_MARGIN = 1.0      # units: leaf bounds are rounded shorts
CONTENTS_SOLID = 0x1


def light_reach(light):
    """(centre (3,), extent, reach distance or inf, one-sided normal or None)
    for an SDFV light record at the cutoff scale 1 (see `build`)."""
    kind = light["kind"]
    rgb = max(light["rgb"])
    peak = rgb * (STYLE_HEADROOM if light.get("style", -1) >= 0 else 1.0)
    a = np.asarray(light.get("a", (0, 0, 0)), np.float64)
    b = np.asarray(light.get("b", (0, 0, 0)), np.float64)
    c = np.asarray(light.get("c", (0, 0, 0)), np.float64)
    if kind in ("distant", "dome"):
        return a, 0.0, math.inf, None
    if kind == "rect":
        axes = np.cross(b, c)
        area = 4.0 * np.linalg.norm(axes)
        normal = axes / np.linalg.norm(axes) if area > 0 else None
        return a, float(np.linalg.norm(b) + np.linalg.norm(c)), peak * area / math.pi, normal
    if kind == "sphere":
        return a, float(b[0]), peak * b[0] ** 2, None
    if kind == "spot":
        return a, float(c[0]), peak * c[0] ** 2, b / np.linalg.norm(b)
    raise ValueError("unknown light kind " + str(kind))


def grid_for(lo, hi, size):
    """Cells of `size` covering [lo, hi]: (origin, dims)."""
    lo = np.asarray(lo, np.float64)
    extent = np.asarray(hi, np.float64) - lo
    dims = np.maximum(np.ceil(extent / size).astype(int), 1)
    return lo, dims


def cell_boxes(origin, size, dims):
    """(lo, hi) of every cell, x fastest: arrays (N, 3)."""
    zz, yy, xx = np.meshgrid(np.arange(dims[2]), np.arange(dims[1]), np.arange(dims[0]),
                             indexing="ij")
    index = np.stack([xx, yy, zz], axis=-1).reshape(-1, 3)
    lo = origin + index * size
    return lo, lo + size


class Visibility:
    """A compiled map's open leaves, their clusters and the PVS."""

    def __init__(self, contents, clusters, mins, maxs, pvs):
        open_leaf = (np.asarray(contents) & CONTENTS_SOLID) == 0
        self.clusters = np.asarray(clusters)[open_leaf]
        self.mins = np.asarray(mins, np.float64)[open_leaf] - LEAF_MARGIN
        self.maxs = np.asarray(maxs, np.float64)[open_leaf] + LEAF_MARGIN
        self.pvs = np.asarray(pvs, dtype=bool)

    @classmethod
    def from_bsp(cls, bsp):
        pvs = bsp.visibility()
        if pvs is None:
            return None
        contents, clusters, mins, maxs = bsp.leaves()
        return cls(contents, clusters, mins, maxs, pvs)

    def leaves_touching(self, lo, hi):
        return np.flatnonzero((self.mins <= hi).all(axis=1) & (self.maxs >= lo).all(axis=1))

    def seen_from(self, lo, hi):
        """Clusters potentially visible from any open leaf touching [lo, hi]
        (bool per cluster), or None when that is everything."""
        clusters = self.clusters[self.leaves_touching(lo, hi)]
        if not len(clusters) or (clusters < 0).any():
            return None
        return self.pvs[clusters].any(axis=0)

    def cell_pairs(self, origin, size, dims):
        """(cell, cluster) for every open leaf touching every cell."""
        cells, clusters = [], []
        for leaf in range(len(self.clusters)):
            first = np.clip(np.floor((self.mins[leaf] - origin) / size).astype(int), 0, dims - 1)
            last = np.clip(np.floor((self.maxs[leaf] - origin) / size).astype(int), 0, dims - 1)
            if (self.maxs[leaf] < origin).any() or (self.mins[leaf] > origin + dims * size).any():
                continue
            zz, yy, xx = np.meshgrid(*(np.arange(first[k], last[k] + 1) for k in (2, 1, 0)),
                                     indexing="ij")
            index = (xx + dims[0] * (yy + dims[1] * zz)).reshape(-1)
            cells.append(index)
            clusters.append(np.full(len(index), self.clusters[leaf]))
        if not cells:
            return np.zeros(0, int), np.zeros(0, int)
        return np.concatenate(cells), np.concatenate(clusters)


def box_distance(point, lo, hi):
    """Distance from `point` to each box (0 inside)."""
    return np.linalg.norm(np.maximum(np.maximum(lo - point, point - hi), 0.0), axis=1)


def build(lights, lo, hi, size, cutoff, visibility=None):
    """The SDFV cells dict ({origin, size, dims, lists}) for `lights` over
    [lo, hi], and a report."""
    if not (size > 0 and cutoff > 0):
        raise ValueError("cell size and cutoff must be positive")
    origin, dims = grid_for(lo, hi, size)
    boxes_lo, boxes_hi = cell_boxes(origin, size, dims)
    count = len(boxes_lo)
    reach = np.zeros((count, len(lights)), dtype=bool)
    pair_cells = pair_clusters = None
    if visibility is not None:
        pair_cells, pair_clusters = visibility.cell_pairs(origin, size, dims)
        open_cell = np.zeros(count, dtype=bool)
        open_cell[pair_cells] = True
    culled = {"range": 0, "side": 0, "visibility": 0}
    for index, light in enumerate(lights):
        centre, extent, strength, normal = light_reach(light)
        ok = np.ones(count, dtype=bool)
        if math.isfinite(strength):
            distance = np.maximum(box_distance(centre, boxes_lo, boxes_hi) - extent, 0.0)
            in_range = strength / np.maximum(distance, 1e-6) ** 2 >= cutoff
            culled["range"] += int((ok & ~in_range).sum())
            ok &= in_range
        if normal is not None:
            corners = np.stack([np.where(np.array(bits, bool), boxes_hi, boxes_lo)
                                for bits in np.ndindex(2, 2, 2)], axis=1)
            front = ((corners - centre) @ normal).max(axis=1) >= -extent
            culled["side"] += int((ok & ~front).sum())
            ok &= front
        if visibility is not None and math.isfinite(strength):
            seen = visibility.seen_from(centre - extent - 1.0, centre + extent + 1.0)
            if seen is not None:
                visible_pair = (pair_clusters < 0) | seen[np.maximum(pair_clusters, 0)]
                visible = np.zeros(count, dtype=bool)
                np.logical_or.at(visible, pair_cells, visible_pair)
                # A cell touching no open leaf holds no point to light.
                visible &= open_cell
                culled["visibility"] += int((ok & ~visible).sum())
                ok &= visible
        reach[:, index] = ok
    lists = [np.flatnonzero(row).tolist() for row in reach]
    lengths = reach.sum(axis=1)
    report = {"cells": int(count), "dims": dims.tolist(), "size": float(size),
              "cutoff": float(cutoff), "visibility": visibility is not None,
              "entries": int(lengths.sum()), "max_lights": int(lengths.max()) if count else 0,
              "mean_lights": float(lengths.mean()) if count else 0.0, "culled": culled}
    return {"origin": origin.tolist(), "size": float(size), "dims": dims.tolist(),
            "lists": lists}, report
