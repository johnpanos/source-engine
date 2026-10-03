#!/usr/bin/env python3
"""Inspect r_temporal_capture output (requires NumPy); a diagnostic, not full qualification.

Camera mode compares world-depth pixels with rigid camera reprojection. It is
only an oracle when those surfaces are stationary. Static mode requires a
frozen scene/camera and includes the viewmodel. Missing motion is reported as
coverage, never silently counted as stationary. Pixel centers include Source's
half-pixel raster offset, while velocity excludes projection jitter.
"""
import argparse
import json
from pathlib import Path

import numpy as np


def inspect(prefix, mode, tolerance=0.25):
    meta = json.loads(Path(str(prefix) + '.json').read_text())
    width, height = meta['render']
    if width <= 0 or height <= 0:
        raise ValueError('invalid capture extent')
    motion = np.fromfile(str(prefix) + '.motion.rg16f', dtype='<f2').reshape(height, width, 2).astype('f8')
    depth = np.fromfile(str(prefix) + '.depth.r32f', dtype='<f4').reshape(height, width)
    valid = np.isfinite(motion).all(axis=2) & (np.abs(motion) < 60000).all(axis=2)
    valid &= np.isfinite(depth) & (depth >= 0) & (depth <= 1)
    selected = valid.copy()
    if mode == 'camera':
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
        old = clip @ np.linalg.inv(current).T @ previous.T
        with np.errstate(divide='ignore', invalid='ignore'):
            expected = (old[..., :2] / old[..., 3:4] - clip[..., :2]) * [width / 2, -height / 2]
        # Portal 2 renders its weapon in a separate compressed depth range.
        selected &= (depth > 0.2) & (depth < 1) & (old[..., 3] > 0)
    else:
        expected = np.zeros_like(motion)
    errors = np.linalg.norm(motion - expected, axis=2)[selected]
    return {'mode': mode, 'render': [width, height], 'valid_motion_fraction': float(valid.mean()),
            'selected_pixels': int(selected.sum()), 'selected_fraction': float(selected.mean()),
            'error_pixels_median': float(np.median(errors)) if errors.size else None,
            'error_pixels_max': float(errors.max()) if errors.size else None,
            'within_tolerance_fraction': float((errors <= tolerance).mean()) if errors.size else 0,
            'tolerance_pixels': tolerance,
            'pass': bool(errors.size and np.isfinite(errors).all() and (errors <= tolerance).all())}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('prefix', type=Path)
    parser.add_argument('--mode', choices=['camera', 'static'], required=True)
    args = parser.parse_args()
    result = inspect(args.prefix, args.mode)
    print(json.dumps(result, indent=2))
    return 0 if result['pass'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
