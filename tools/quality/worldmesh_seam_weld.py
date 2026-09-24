"""Bounded position weld for a declared imported USD material.

The source stage remains untouched. Only positions in the selected WMSH
material are changed; material and lightmap UVs stay attached to their corners.
This repairs sub-pixel cracks between near-coincident triangle endpoints.
"""

import numpy as np
from scipy.spatial import cKDTree


def weld_material(faces, material, distance):
    if not np.isfinite(distance) or distance <= 0:
        raise ValueError("seam weld distance must be finite and positive")
    corners = []
    positions = []
    for face_id in sorted(faces):
        face = faces[face_id]
        if face["material"] != material:
            continue
        for triangle_index, triangle in enumerate(face["triangles"]):
            for corner_index, point in enumerate(triangle[0]):
                corners.append((face_id, triangle_index, corner_index))
                positions.append(point)
    if not positions:
        raise ValueError("seam weld material has no triangles: " + material)
    # WMSH stores float32 positions. Cluster those exact values so the weld
    # decision and recorded displacement match what the GPU would receive.
    source = np.asarray(positions, dtype=np.float32)
    parent = np.arange(len(source))

    def root(index):
        while parent[index] != index:
            parent[index] = parent[parent[index]]
            index = parent[index]
        return index

    for left, right in cKDTree(source).query_pairs(distance, output_type="ndarray"):
        first, second = root(left), root(right)
        if first != second:
            parent[second] = first
    roots = np.fromiter((root(index) for index in range(len(source))),
                        dtype=np.int64, count=len(source))
    _, cluster = np.unique(roots, return_inverse=True)
    counts = np.bincount(cluster)
    centers = np.zeros((len(counts), 3), dtype=np.float64)
    np.add.at(centers, cluster, source)
    centers = (centers / counts[:, None]).astype(np.float32)
    displacement = np.linalg.norm(centers[cluster] - source, axis=1)
    maximum = float(displacement.max())
    if maximum > distance + 1e-5:
        raise ValueError("seam weld chained beyond its displacement limit")
    for index, (face_id, triangle_index, corner_index) in enumerate(corners):
        face = faces[face_id]
        triangle = face["triangles"][triangle_index]
        points = list(triangle[0])
        points[corner_index] = tuple(float(value) for value in centers[cluster[index]])
        face["triangles"][triangle_index] = (points, *triangle[1:])
    return {"material": material, "distance_source_units": distance,
            "corners": len(corners), "clusters": len(counts),
            "moved_corners": int(np.count_nonzero(displacement > 1e-6)),
            "maximum_displacement_source_units": maximum}
