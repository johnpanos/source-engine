#!/usr/bin/env python3
"""Independently compare Cycles flat/RNM preview bakes with Source face bases."""

import argparse
import hashlib
import json
import struct
from pathlib import Path

import imageio.v3 as iio
import numpy as np

from bsp2_reader import open_any


SOURCE_BASIS = (
    (0.81649661064147949, 0.0, 0.57735025882720947),
    (-0.40824821591377258, 0.70710676908493042, 0.57735025882720947),
    (-0.40824821591377258, -0.70710676908493042, 0.57735025882720947),
)


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def unit(vector):
    norm = np.linalg.norm(vector)
    if not np.isfinite(norm) or norm < 1e-8:
        raise ValueError("BSP face has a degenerate texture axis")
    return vector / norm


def expected_directions(bsp):
    data = bsp.read_bytes()
    kind, header = open_any(data)
    if kind != "legacy" or header["version"] != 21:
        raise ValueError("expected a compiled legacy v21 BSP")

    def lump(index):
        offset, size, *_ = header["lumps"][index]
        return data[offset:offset + size]

    faces, planes, texinfos, models = (lump(i) for i in (7, 1, 6, 14))
    if len(models) != 48 or len(faces) % 56 or len(planes) % 20 or len(texinfos) % 72:
        raise ValueError("BSP world or face structures are invalid")
    first_face, count = struct.unpack_from("<ii", models, 40)
    if first_face < 0 or count < 1 or first_face + count > len(faces) // 56:
        raise ValueError("BSP world face range is invalid")
    result = {}
    for face_id in range(first_face, first_face + count):
        face = faces[56 * face_id:56 * (face_id + 1)]
        plane_id = struct.unpack_from("<H", face, 0)[0]
        texinfo_id = struct.unpack_from("<h", face, 10)[0]
        if plane_id >= len(planes) // 20 or not 0 <= texinfo_id < len(texinfos) // 72:
            raise ValueError("BSP face references an invalid plane or texinfo")
        normal = unit(np.array(struct.unpack_from("<fff", planes, 20 * plane_id)))
        texture = texinfos[72 * texinfo_id:72 * (texinfo_id + 1)]
        s_axis = np.array(struct.unpack_from("<fff", texture, 0))
        t_axis = np.array(struct.unpack_from("<fff", texture, 16))
        axis_y = unit(np.cross(normal, s_axis))
        axis_x = unit(np.cross(axis_y, normal))
        if np.dot(normal, np.cross(s_axis, t_axis)) < 0:
            axis_y = -axis_y
        result["Mesh_" + str(face_id)] = {
            "flat": normal,
            **{"rnm" + str(index): unit(local[0] * axis_x + local[1] * axis_y +
                                         local[2] * normal)
               for index, local in enumerate(SOURCE_BASIS)},
        }
    return result


def compare(bsp, stage, manifest_path, bake_paths, negative_self_test=False):
    expected = expected_directions(bsp)
    manifest = json.loads(manifest_path.read_text())
    charts = manifest["lightmap_atlas"]["charts"]
    if set(charts) != {name.removeprefix("Mesh_") for name in expected}:
        raise ValueError("Portal manifest omits a compiled world chart")
    stage_hash = sha256(stage)
    bakes = {}
    for mode, path in bake_paths.items():
        evidence = json.loads(path.with_suffix(".json").read_text())
        if (evidence["status"] != "pass" or evidence["source_basis"] != mode or
                evidence["stage_sha256"] != stage_hash or
                evidence["exr_sha256"] != sha256(path) or
                set(evidence["source_face_directions"]) != set(expected) or
                set(evidence["chart_max_linear"]) != set(charts)):
            raise ValueError("Cycles " + mode + " evidence does not match the World Stage")
        if not all(value > 1e-5 for value in evidence["chart_max_linear"].values()):
            raise ValueError("Cycles " + mode + " left a required chart dark")
        image = iio.imread(path)
        if (image.ndim != 3 or image.shape[2] < 3 or image.dtype != np.float32 or
                image.shape[:2] != (evidence["height"], evidence["width"]) or
                not np.isfinite(image).all()):
            raise ValueError("Cycles " + mode + " EXR has invalid dimensions or samples")
        bakes[mode] = (evidence, image[:, :, :3])
    if len({tuple(item[1].shape) for item in bakes.values()}) != 1:
        raise ValueError("directional bakes have different atlas dimensions")
    if len({item[0]["samples"] for item in bakes.values()}) != 1:
        raise ValueError("directional bakes have different sample budgets")

    def check_directions(records):
        worst = 0.0
        for mode, directions in records.items():
            for name, authored in directions.items():
                error = float(np.linalg.norm(np.asarray(authored) - expected[name][mode]))
                worst = max(worst, error)
                if error > 1e-4:
                    raise ValueError("Cycles " + mode + " direction diverges from BSP " + name)
        return worst

    directions = {mode: item[0]["source_face_directions"] for mode, item in bakes.items()}
    worst = check_directions(directions)
    negative_rejected = False
    if negative_self_test:
        altered = {mode: dict(values) for mode, values in directions.items()}
        altered["rnm0"][next(iter(expected))] = [0.0, 0.0, 0.0]
        try:
            check_directions(altered)
        except ValueError:
            negative_rejected = True
        if not negative_rejected:
            raise ValueError("seeded wrong RNM direction was accepted")

    difference = {}
    modes = tuple(bake_paths)
    for index, left in enumerate(modes):
        for right in modes[index + 1:]:
            delta = float(np.mean(np.abs(bakes[left][1] - bakes[right][1])))
            if delta <= 1e-3:
                raise ValueError("Cycles directional passes are indistinguishable: " +
                                 left + "/" + right)
            difference[left + "/" + right] = delta
    return {"status": "pass", "scope": "cycles-rnm-preview-directions",
            "bsp_sha256": sha256(bsp), "stage_sha256": stage_hash,
            "manifest_sha256": sha256(manifest_path),
            "exr_sha256": {mode: sha256(path) for mode, path in bake_paths.items()},
            "faces": len(expected), "samples": bakes["flat"][0]["samples"],
            "width": bakes["flat"][0]["width"], "height": bakes["flat"][0]["height"],
            "worst_direction_error": worst, "mean_absolute_pass_difference": difference,
            "negative_wrong_direction_rejected": negative_rejected}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--bsp", type=Path, required=True)
    parser.add_argument("--stage", type=Path, required=True)
    parser.add_argument("--manifest", type=Path, required=True)
    for mode in ("flat", "rnm0", "rnm1", "rnm2"):
        parser.add_argument("--" + mode, type=Path, required=True)
    parser.add_argument("--negative-self-test", action="store_true")
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()
    paths = {mode: getattr(args, mode) for mode in ("flat", "rnm0", "rnm1", "rnm2")}
    evidence = compare(args.bsp, args.stage, args.manifest, paths, args.negative_self_test)
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(evidence, indent=2, sort_keys=True) + "\n")
    print(json.dumps(evidence, sort_keys=True))


if __name__ == "__main__":
    main()
