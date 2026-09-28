"""Fixtures for the portal view-continuity judge (tools/quality/portal_view_trace.py).

A synthetic walk goes north into a portal on the north wall and comes out of
its partner on the east wall heading west, as qa_portal_walk does. The judge
must pass it and fail each seeded defect: a jump, a view the teleport did not
turn, a walk with no crossing, a crossing through an inactive portal and a
snap back through the portal.
"""

import math
from pathlib import Path
import sys
import unittest

QUALITY = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(QUALITY))
import portal_view_trace as trace

NORTH = ((0.0, 640.0, 56.0), (0.0, -90.0, 0.0))    # faces south
EAST = ((640.0, 0.0, 56.0), (0.0, 180.0, 0.0))     # faces west
LIMITS = {"max_speed": 400, "max_turn_rate": 30, "min_crossings": 1, "max_crossings": 1}


def portal_lines(frame=1, active=(1, 1)):
    return ["PVIEW_PORTAL f=%d portal 10 linked 11 active %d origin (%.3f %.3f %.3f) angles (%.3f %.3f %.3f)"
            % ((frame, active[0]) + NORTH[0] + NORTH[1]),
            "PVIEW_PORTAL f=%d portal 11 linked 10 active %d origin (%.3f %.3f %.3f) angles (%.3f %.3f %.3f)"
            % ((frame, active[1]) + EAST[0] + EAST[1])]


def view_line(frame, eye, angles):
    t = frame * 0.005
    return ("PVIEW f=%d t=%.4f rt=%.4f eye (%.3f %.3f %.3f) ang (%.3f %.3f %.3f) body (%.3f %.3f %.3f) through -1"
            % ((frame, t, t) + tuple(eye) + tuple(angles) + (eye[0], eye[1], eye[2] - 64.0)))


def walk(steps=80, step=1.0, start=600.0):
    """Eyes walking north at `step` units a frame; past the north portal's
    plane they continue out of the east portal through its transform."""
    matrix = trace.portal_transform(NORTH, EAST)
    lines = []
    for i in range(steps):
        y = start + i * step
        eye, angles = (0.0, y, 64.0), (0.0, 90.0, 0.0)
        if y > 640.0:
            eye = trace.apply_point(matrix, eye)
            angles = (0.0, 180.0, 0.0)
        lines.append(view_line(100 + i, eye, angles))
    return lines


class PortalTransformTests(unittest.TestCase):
    def test_north_to_east(self):
        matrix = trace.portal_transform(NORTH, EAST)
        # One unit into the north portal is one unit out of the east one.
        out = trace.apply_point(matrix, (0.0, 641.0, 64.0))
        for a, b in zip(out, (639.0, 0.0, 64.0)):
            self.assertAlmostEqual(a, b, places=4)
        forward = trace.apply_vector(matrix, (0.0, 1.0, 0.0))
        for a, b in zip(forward, (-1.0, 0.0, 0.0)):
            self.assertAlmostEqual(a, b, places=4)

    def test_round_trip_is_identity(self):
        there = trace.portal_transform(NORTH, EAST)
        back = trace.portal_transform(EAST, NORTH)
        point = (12.0, 600.0, 70.0)
        for a, b in zip(trace.apply_point(back, trace.apply_point(there, point)), point):
            self.assertAlmostEqual(a, b, places=4)


class JudgeTests(unittest.TestCase):
    def judge(self, window, before=None, limits=LIMITS):
        return trace.judge(portal_lines() if before is None else before, window, limits)

    def test_continuous_crossing_passes(self):
        ok, detail, bad = self.judge(walk())
        self.assertTrue(ok, detail)
        self.assertIn("1 crossings, 0 bad steps", detail)

    def test_jump_fails(self):
        lines = walk()
        lines[20] = view_line(120, (0.0, 670.0, 64.0), (0.0, 90.0, 0.0))
        ok, detail, bad = self.judge(lines)
        self.assertFalse(ok)
        self.assertTrue(bad)

    def test_view_not_turned_fails(self):
        lines = [line.replace("ang (0.000 180.000 0.000)", "ang (0.000 90.000 0.000)") for line in walk()]
        ok, detail, bad = self.judge(lines)
        self.assertFalse(ok)
        self.assertGreater(bad[0]["turn"], bad[0]["turn_limit"])

    def test_no_crossing_fails(self):
        ok, detail, _ = self.judge(walk(steps=30))
        self.assertFalse(ok)
        self.assertIn("0 crossings", detail)

    def test_inactive_portal_is_not_a_crossing(self):
        ok, detail, bad = self.judge(walk(), before=portal_lines(active=(1, 0)))
        self.assertFalse(ok)
        self.assertTrue(bad)

    def test_snap_back_counts_as_a_second_crossing(self):
        lines = walk()
        # One frame back in front of the north portal, then out again.
        lines.insert(46, view_line(145, (0.0, 639.8, 64.0), (0.0, 90.0, 0.0)))
        ok, detail, _ = self.judge(lines)
        self.assertFalse(ok, detail)

    def test_portal_changes_inside_the_window_apply(self):
        # The pair appears only after the first frames, as when portals are
        # fired inside a window.
        lines = walk()
        window = lines[:5] + portal_lines(frame=104) + lines[5:]
        ok, detail, _ = self.judge(window, before=[])
        self.assertTrue(ok, detail)

    def test_window_lines(self):
        lines = ["a", "QA_WINDOW w BEGIN", "b", "QA_WINDOW w END", "c"]
        self.assertEqual(trace.window_lines(lines, "w"), (["a"], ["b"]))
        with self.assertRaises(ValueError):
            trace.window_lines(lines, "other")


if __name__ == "__main__":
    unittest.main()
