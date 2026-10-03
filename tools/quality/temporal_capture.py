#!/usr/bin/env python3
"""Inspect r_temporal_capture output (requires NumPy); a diagnostic, not full qualification.

Camera mode compares world-depth pixels with rigid camera reprojection. It is
only an oracle when those surfaces are stationary. Static mode requires a
frozen scene/camera and includes the viewmodel. Missing motion is reported as
coverage, never silently counted as stationary. Pixel centers include Source's
half-pixel raster offset, while velocity excludes projection jitter.
Reset mode checks the actual provider-dispatch reset flag and complete finite
input/output images. Invalid prior motion is expected after a discontinuity;
this mode does not certify image quality or motion coverage in later frames.
Object mode uses captured pose bounds and previous displacement for an isolated
rigidly translating object; it rejects deformation and zero displacement.
Pose mode projects sampled triangles, checks their visibility against actual
depth, and interpolates previous vertex positions independently of the shader.
It is a sampled GPU diagnostic, not complete surface or animation qualification.
"""
import argparse
import json
from pathlib import Path

import numpy as np


def inspect_pose(meta, motion, depth, object_identity, view_identity, tolerance):
    objects = [obj for obj in meta.get('objects', []) if obj['identity'] == object_identity
               and (view_identity is None or obj['view'] == view_identity)]
    if len(objects) != 1:
        raise ValueError('select one captured object/view for pose probes')
    obj = objects[0]
    if obj.get('recorded_viewport') is not True:
        raise ValueError('native recorded viewport unavailable')
    if not np.isfinite(obj['translation_error']) or obj['translation_error'] <= .001:
        raise ValueError('pose probes require a nonuniform change in vertex positions')
    current = np.asarray(obj['current_to_clip']).reshape(4, 4)
    previous = np.asarray(obj['previous_to_clip']).reshape(4, 4)
    vx, vy, vw, vh, near, far = obj['viewport']
    if not (vw > 0 and vh > 0 and far > near):
        raise ValueError('invalid pose viewport')
    jx, jy = meta['jitter']
    height, width = depth.shape
    probes = {}
    visible_triangles = 0
    for triangle in obj['triangles']:
        positions = np.asarray(triangle).reshape(2, 3, 3)
        clip = np.c_[positions[0], np.ones(3)] @ current.T
        if not np.isfinite(clip).all() or (clip[:, 3] <= 0).any():
            continue
        ndc = clip[:, :3] / clip[:, 3:4]
        screen = ndc[:, :2] * [vw / 2, -vh / 2] + [vx + vw / 2 + jx, vy + vh / 2 + jy]
        basis = np.vstack([screen.T, np.ones(3)])
        if abs(np.linalg.det(basis)) < .1:
            continue
        visible = False
        for seed in ([1/3]*3, [.6, .2, .2], [.2, .6, .2], [.2, .2, .6]):
            x, y = np.rint(np.asarray(seed) @ screen).astype(int)
            if not (0 <= x < width and 0 <= y < height):
                continue
            bary = np.linalg.solve(basis, [x, y, 1])
            if bary.min() < .02:
                continue
            predicted_depth = (bary @ ndc[:, 2]) * (far - near) + near
            if not np.isfinite(depth[y, x]) or abs(predicted_depth - depth[y, x]) > 1e-6:
                continue
            # Perspective-correct interpolation of world positions; depth is
            # instead linear in screen barycentrics, as in the native rasterizer.
            weights = bary / clip[:, 3]
            weights /= weights.sum()
            old = np.r_[weights @ positions[1], 1] @ previous.T
            if not np.isfinite(old).all() or old[3] <= 0:
                continue
            now = [(x - vx - jx) * 2 / vw - 1, 1 - (y - vy - jy) * 2 / vh]
            expected = (old[:2] / old[3] - now) * [vw / 2, -vh / 2]
            probes[(x, y)] = expected
            visible = True
        visible_triangles += visible
    errors = []
    valid = 0
    for (x, y), expected in probes.items():
        actual = motion[y, x]
        if np.isfinite(actual).all() and (np.abs(actual) < 60000).all():
            valid += 1
            errors.append(np.linalg.norm(actual - expected))
    errors = np.asarray(errors)
    return {'mode': 'pose', 'object_identity': object_identity, 'view_identity': obj['view'],
            'sampled_triangles': len(obj['triangles']), 'visible_triangles': visible_triangles,
            'selected_pixels': len(probes), 'valid_pixels': valid,
            'translation_error_world': obj['translation_error'],
            'error_pixels_max': float(errors.max()) if errors.size else None,
            'failed_pixels': int((errors > tolerance).sum()) + len(probes) - valid,
            'tolerance_pixels': tolerance,
            'pass': bool(probes and valid == len(probes) and np.isfinite(errors).all()
                         and (errors <= tolerance).all())}


def inspect(prefix, mode, tolerance=0.25, object_identity=None, view_identity=None):
    meta = json.loads(Path(str(prefix) + '.json').read_text())
    width, height = meta['render']
    if width <= 0 or height <= 0:
        raise ValueError('invalid capture extent')
    motion = np.fromfile(str(prefix) + '.motion.rg16f', dtype='<f2').reshape(height, width, 2).astype('f8')
    depth = np.fromfile(str(prefix) + '.depth.r32f', dtype='<f4').reshape(height, width)
    if mode == 'pose':
        return inspect_pose(meta, motion, depth, object_identity, view_identity, tolerance)
    valid = np.isfinite(motion).all(axis=2) & (np.abs(motion) < 60000).all(axis=2)
    valid &= np.isfinite(depth) & (depth >= 0) & (depth <= 1)
    selected = valid.copy()
    if mode == 'reset':
        output_width, output_height = meta['output']
        if output_width <= 0 or output_height <= 0:
            raise ValueError('invalid output extent')
        color = np.fromfile(str(prefix) + '.color.rgba16f', dtype='<f2').reshape(height, width, 4)
        output = np.fromfile(str(prefix) + '.output.rgba16f', dtype='<f2').reshape(
            output_height, output_width, 4)
        finite = all(np.isfinite(image).all() for image in (color, depth, motion, output))
        depth_valid = bool(((depth >= 0) & (depth <= 1)).all())
        reset = meta.get('history_reset') is True
        return {'mode': mode, 'render': [width, height],
                'output': [output_width, output_height], 'history_reset': reset,
                'native_frame': meta.get('native_frame'), 'finite_images': bool(finite),
                'valid_depth': depth_valid, 'valid_motion_fraction': float(valid.mean()),
                'pass': bool(reset and meta.get('native_frame', 0) > 0 and finite and depth_valid)}
    object_coverage = None
    if mode in ('camera', 'object'):
        if not meta['camera_valid']:
            raise ValueError('camera history unavailable')
        current = np.array(meta['current_to_clip']).reshape(4, 4)
        previous = np.array(meta['previous_to_clip']).reshape(4, 4)
        if not np.isfinite(current).all() or not np.isfinite(previous).all():
            raise ValueError('nonfinite camera history')
        y, x = np.mgrid[:height, :width]
        jx, jy = meta['jitter']
        clip = np.stack([2 * (x - jx) / width - 1, 1 - 2 * (y - jy) / height,
                         depth, np.ones_like(depth)], axis=-1)
        world = clip @ np.linalg.inv(current).T
        if mode == 'object':
            objects = [obj for obj in meta.get('objects', []) if obj['identity'] == object_identity]
            if len(objects) != 1:
                raise ValueError('capture does not contain the selected object')
            obj = objects[0]
            offset = np.asarray(obj['previous_offset'], dtype=float)
            bounds = np.asarray(obj['bounds'], dtype=float)
            if (not np.isfinite(offset).all() or not np.isfinite(bounds).all() or
                    not 0 <= obj['translation_error'] <= .001 or np.linalg.norm(offset) < .01):
                raise ValueError('object must have a finite, nonzero rigid translation')
            with np.errstate(divide='ignore', invalid='ignore'):
                world /= world[..., 3:4]
            # This fixture must isolate its object: depth positions inside the
            # current AABB select its visible pixels, not a screen-space box.
            candidates = ((world[..., :3] >= bounds[:3] - .05).all(axis=2) &
                          (world[..., :3] <= bounds[3:] + .05).all(axis=2) &
                          np.isfinite(world).all(axis=2) & (depth > .2) & (depth < 1))
            object_coverage = float(valid[candidates].mean()) if candidates.any() else 0
            selected &= candidates
            world[..., :3] += offset
        old = world @ previous.T
        with np.errstate(divide='ignore', invalid='ignore'):
            expected = (old[..., :2] / old[..., 3:4] - clip[..., :2]) * [width / 2, -height / 2]
        # Portal 2 renders its weapon in a separate compressed depth range.
        selected &= (depth > 0.2) & (depth < 1) & (old[..., 3] > 0)
    else:
        expected = np.zeros_like(motion)
    errors = np.linalg.norm(motion - expected, axis=2)[selected]
    return {'mode': mode, 'render': [width, height], 'valid_motion_fraction': float(valid.mean()),
            'object_identity': object_identity, 'object_valid_motion_fraction': object_coverage,
            'selected_pixels': int(selected.sum()), 'selected_fraction': float(selected.mean()),
            'error_pixels_median': float(np.median(errors)) if errors.size else None,
            'error_pixels_max': float(errors.max()) if errors.size else None,
            'within_tolerance_fraction': float((errors <= tolerance).mean()) if errors.size else 0,
            'tolerance_pixels': tolerance,
            'pass': bool(errors.size and np.isfinite(errors).all() and (errors <= tolerance).all()
                         and (object_coverage is None or object_coverage == 1))}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('prefix', type=Path)
    parser.add_argument('--mode', choices=['camera', 'static', 'reset', 'object', 'pose'], required=True)
    parser.add_argument('--object', type=int, dest='object_identity',
                        help='motion identity of an isolated, rigidly translating captured object')
    parser.add_argument('--view', type=int, dest='view_identity', help='captured semantic view identity')
    args = parser.parse_args()
    result = inspect(args.prefix, args.mode, object_identity=args.object_identity,
                     view_identity=args.view_identity)
    print(json.dumps(result, indent=2))
    return 0 if result['pass'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
