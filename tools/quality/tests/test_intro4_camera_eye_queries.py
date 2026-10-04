"""Visible/occluded camera samples and complete state-marker parsing."""
import sys
from pathlib import Path
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from intro4_material_check import camera_eye_queries, commands


def receipt():
    parts = ['RC_CAMERA_EYE_PARENT true', '"r_pixelvisibility_partial" = "1"',
             '"mat_force_tonemap_scale" = "0"']
    for state in ('visible', 'light-off', 'hidden', 'shown', 'occluded',
                  'occluded-hidden', 'returned'):
        parts.append('RC_CAMERA_EYE_' + state)
        if state not in ('hidden', 'occluded-hidden'):
            count = 0 if state == 'occluded' else 19
            for _ in range(5):
                parts.extend([f'Pixels visible: {count} (qh:42) Pixels possible: 19 (qh:43)',
                              'Draw Proxy: qh:42 org:<143,21,151> (frame:500)'])
        parts.append('RC_CAMERA_EYE_QUERY_END')
    return '\n'.join(parts) + '\n'


class CameraEyeQueryTests(unittest.TestCase):
    def test_combined_comparisons_hold_exposure_and_restore_policy(self):
        sequence = commands('all')
        first = sequence.index('screenshot')
        last = len(sequence) - 1 - sequence[::-1].index('screenshot')
        self.assertIn('mat_force_tonemap_scale 1', sequence[:first])
        self.assertNotIn('mat_force_tonemap_scale 0', sequence[first:last])
        self.assertIn('mat_force_tonemap_scale 0', sequence[last:])
        self.assertIn('cmd script rc_camera.SetAngles(0,90,0)', sequence)

    def test_complete_camera_state_receipt(self):
        self.assertTrue(all(row['passed'] for row in camera_eye_queries(receipt())))

    def test_occluded_label_does_not_consume_occluded_hidden(self):
        rows = {row['name']: row for row in camera_eye_queries(receipt())}
        self.assertEqual(len(rows['camera-eye.occluded.visibility-query']['samples']), 5)
        self.assertTrue(rows['camera-eye.occluded-hidden.visibility-query']['passed'])

    def test_empty_query_or_wrong_parent_cannot_certify(self):
        for bad in ('', receipt().replace('Pixels possible: 19', 'Pixels possible: 0'),
                    receipt().replace('PARENT true', 'PARENT false'),
                    receipt().replace('org:<143,21,151>', 'org:<1,2,3>')):
            with self.subTest(log=bad[:60]):
                self.assertTrue(any(not row['passed'] for row in camera_eye_queries(bad)))

    def test_exposure_is_checked_at_final_restoration(self):
        held = '"mat_force_tonemap_scale" = "1"\n' + receipt()
        rows = {row['name']: row for row in camera_eye_queries(held)}
        self.assertTrue(rows['camera-eye.default-exposure-restored']['passed'])
        bad = receipt() + '"mat_force_tonemap_scale" = "1"\n'
        rows = {row['name']: row for row in camera_eye_queries(bad)}
        self.assertFalse(rows['camera-eye.default-exposure-restored']['passed'])

    def test_occluded_positive_samples_fail(self):
        bad = receipt().replace('Pixels visible: 0', 'Pixels visible: 19')
        rows = {row['name']: row for row in camera_eye_queries(bad)}
        self.assertFalse(rows['camera-eye.occluded.visibility-query']['passed'])


if __name__ == '__main__':
    unittest.main()
