"""Sensitivity of the camera diagnostic; no GPU or golden image required."""
import json
from pathlib import Path
import sys
import tempfile
import unittest

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from temporal_capture import inspect


class TemporalCaptureTests(unittest.TestCase):
    def test_reset_requires_dispatch_reset_and_complete_finite_images(self):
        with tempfile.TemporaryDirectory() as directory:
            prefix = Path(directory) / 'reset'
            meta = {'render': [4, 2], 'output': [6, 3],
                    'native_frame': 12, 'history_reset': True}
            def write_meta():
                Path(str(prefix) + '.json').write_text(json.dumps(meta))
            write_meta()
            np.full((2, 4), .5, dtype='<f4').tofile(str(prefix) + '.depth.r32f')
            # Missing previous correspondence is legal on the reset frame.
            np.full((2, 4, 2), 65504, dtype='<f2').tofile(str(prefix) + '.motion.rg16f')
            np.ones((2, 4, 4), dtype='<f2').tofile(str(prefix) + '.color.rgba16f')
            output = np.ones((3, 6, 4), dtype='<f2')
            output.tofile(str(prefix) + '.output.rgba16f')
            self.assertTrue(inspect(prefix, 'reset')['pass'])
            meta['history_reset'] = False
            write_meta()
            self.assertFalse(inspect(prefix, 'reset')['pass'])
            meta['history_reset'] = True
            write_meta()
            output[0, 0, 0] = np.nan
            output.tofile(str(prefix) + '.output.rgba16f')
            self.assertFalse(inspect(prefix, 'reset')['pass'])
            Path(str(prefix) + '.output.rgba16f').write_bytes(b'')
            with self.assertRaises(ValueError):
                inspect(prefix, 'reset')

    def test_signed_pixel_displacement_and_missing_coverage(self):
        with tempfile.TemporaryDirectory() as directory:
            prefix = Path(directory) / 'frame'
            width, height = 40, 20
            previous = np.eye(4)
            previous[0, 3] = -0.1
            meta = {'render': [width, height], 'jitter': [.25, -.25], 'camera_valid': True,
                    'current_to_clip': np.eye(4).flatten().tolist(),
                    'previous_to_clip': previous.flatten().tolist()}
            Path(str(prefix) + '.json').write_text(json.dumps(meta))
            np.full((height, width), .5, dtype='<f4').tofile(str(prefix) + '.depth.r32f')
            motion = np.zeros((height, width, 2), dtype='<f2')
            motion[..., 0] = -2
            motion.tofile(str(prefix) + '.motion.rg16f')
            self.assertTrue(inspect(prefix, 'camera')['pass'])
            # Wrong sign, normalized rather than pixel units, and stale zero motion fail.
            for wrong in (2, -.1, 0):
                motion[..., 0] = wrong
                motion.tofile(str(prefix) + '.motion.rg16f')
                self.assertFalse(inspect(prefix, 'camera')['pass'])
            motion.fill(65504)
            motion.tofile(str(prefix) + '.motion.rg16f')
            result = inspect(prefix, 'static')
            self.assertFalse(result['pass'])
            self.assertEqual(result['valid_motion_fraction'], 0)
            # Truncation must fail rather than report a smaller passing image.
            Path(str(prefix) + '.motion.rg16f').write_bytes(b'')
            with self.assertRaises(ValueError):
                inspect(prefix, 'camera')


if __name__ == '__main__':
    unittest.main()
