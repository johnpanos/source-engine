#!/usr/bin/env python3
"""Spatial leaf visibility for imported WMSH meshlets.

The engine draws the meshlets referenced by the leaves it walks for a view
(PVS and frustum, `R_RecursiveWorldNode`), and never walks a leaf whose
contents are exactly CONTENTS_SOLID. A meshlet must therefore be referenced
by every non-solid leaf through which some eye ray can reach it:

* the non-solid leaves its bounds touch; and
* when its bounds reach into solid leaves (collision solids around furniture,
  the sealed shell and the void beyond it, which render geometry may occupy
  or be seen through), every non-solid leaf bordering the connected solid
  region those leaves belong to. A ray that reaches a point inside solid
  last left open space through such a leaf.

Leaf and meshlet boxes are compared with padding because vbsp stores leaf
bounds as truncated shorts (up to one unit inside the true leaf). A meshlet no leaf references would
never draw, so it falls back to every non-solid leaf. Everything here is
conservative: it may keep a meshlet another view could skip, never the
reverse.

`morton_order` sorts triangles along a Z-order curve so the writer's fixed
size meshlets are spatially compact, which is what makes leaf and frustum
culling effective.
"""

import numpy as np

CONTENTS_SOLID = 1
EPSILON = 2.0
SOLID_PAD = 1.0
# Solid boxes that touch (within truncation) form one connected region.
TOUCH = 1.0


def _boxes(pairs):
    array = np.asarray([tuple(low) + tuple(high) for low, high in pairs], dtype=np.float64)
    return array.reshape(-1, 6)


def _overlap(a, b, pad):
    """Boolean matrix: box a[i] and box b[j] overlap or lie within `pad`."""
    return np.all((a[:, None, :3] - pad <= b[None, :, 3:]) &
                  (b[None, :, :3] - pad <= a[:, None, 3:]), axis=2)


def _solid_components(solid_boxes):
    """Label connected solid regions (boxes that touch or overlap)."""
    count = len(solid_boxes)
    parent = list(range(count))

    def find(i):
        while parent[i] != i:
            parent[i] = parent[parent[i]]
            i = parent[i]
        return i

    touching = _overlap(solid_boxes, solid_boxes, TOUCH)
    for i, j in zip(*np.nonzero(np.triu(touching, 1))):
        parent[find(i)] = find(j)
    return [find(i) for i in range(count)]


def assign(meshlet_boxes, leaves, chunk=4096):
    """Per leaf, the meshlet indices it references; plus a summary.

    `meshlet_boxes` is [(mins, maxs)] in Source units; `leaves` is
    [(contents, mins, maxs)] from `worldstage_mesh_pack.leaf_volumes`.
    """
    open_leaves = [index for index, leaf in enumerate(leaves) if leaf[0] != CONTENTS_SOLID]
    # Leaf 0 is the BSP's shared solid placeholder with empty bounds.
    solid_leaves = [index for index, leaf in enumerate(leaves)
                    if leaf[0] == CONTENTS_SOLID and tuple(leaf[1]) != tuple(leaf[2])]
    if not open_leaves:
        raise ValueError("BSP has no non-solid leaves to reference meshlets")
    open_boxes = _boxes([(leaves[i][1], leaves[i][2]) for i in open_leaves])
    solid_boxes = _boxes([(leaves[i][1], leaves[i][2]) for i in solid_leaves])
    labels = _solid_components(solid_boxes) if len(solid_boxes) else []
    # Non-solid leaves bordering each connected solid region.
    borders = {}
    if len(solid_boxes):
        adjacent = _overlap(solid_boxes, open_boxes, EPSILON)
        for row, label in enumerate(labels):
            borders.setdefault(label, np.zeros(len(open_leaves), dtype=bool))
            borders[label] |= adjacent[row]
    meshlets = _boxes(meshlet_boxes)
    references = [[] for _ in leaves]
    fallback = 0
    through_solid = 0
    for start in range(0, len(meshlets), chunk):
        block = meshlets[start:start + chunk]
        direct = _overlap(block, open_boxes, EPSILON)
        # Solid leaves the meshlet may reach into. Truncated short bounds can
        # sit up to one unit inside the true leaf, so a meshlet within one
        # unit of a solid box counts as entering it.
        inside = (_overlap(block, solid_boxes, SOLID_PAD) if len(solid_boxes)
                  else np.zeros((len(block), 0), dtype=bool))
        for row in range(len(block)):
            selected = direct[row].copy()
            solid_rows = np.nonzero(inside[row])[0]
            if len(solid_rows):
                through_solid += 1
                for label in {labels[i] for i in solid_rows}:
                    selected |= borders[label]
            if not selected.any():
                fallback += 1
                selected[:] = True
            for column in np.nonzero(selected)[0]:
                references[open_leaves[column]].append(start + row)
    total = sum(len(item) for item in references)
    summary = {"policy": "open leaves touched, plus open leaves bordering solid regions entered",
               "open_leaves": len(open_leaves), "solid_leaves": len(solid_leaves),
               "solid_regions": len(set(labels)), "meshlets": len(meshlets),
               "meshlets_through_solid": through_solid, "meshlets_in_every_leaf_fallback": fallback,
               "leaf_references": total,
               "mean_leaves_per_meshlet": round(total / max(1, len(meshlets)), 2)}
    return references, summary


def morton_order(centroids):
    """Indices of `centroids` along a 3D Z-order curve (10 bits per axis)."""
    points = np.asarray(centroids, dtype=np.float64).reshape(-1, 3)
    if not len(points):
        return []
    low, high = points.min(axis=0), points.max(axis=0)
    cells = np.clip(((points - low) / np.maximum(high - low, 1e-9) * 1023).astype(np.int64),
                    0, 1023)

    def spread(values):
        values = values & 0x3FF
        values = (values | (values << 16)) & 0x030000FF
        values = (values | (values << 8)) & 0x0300F00F
        values = (values | (values << 4)) & 0x030C30C3
        return (values | (values << 2)) & 0x09249249

    codes = spread(cells[:, 0]) | (spread(cells[:, 1]) << 1) | (spread(cells[:, 2]) << 2)
    return list(np.argsort(codes, kind="stable"))
