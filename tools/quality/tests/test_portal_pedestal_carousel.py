"""Oracle tests for the testchmb_a_01 pedestal portal gun check.

The logs are the `portal_placed` lines of real runs (vphysics_box3d, native
Vulkan, 2026-09-26): one with CBaseCombatWeapon::SetParent keeping the gun
where the pedestal turned it, and one without it, where the constraint pulled
the gun back to its spawn pose and every shot hit the same wall.
"""

import io
import sys
import unittest
from pathlib import Path

QUALITY = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(QUALITY))

import portal_pedestal_carousel as carousel  # noqa: E402
from conformance_result import Checks  # noqa: E402

TURNING = """\
[7.1] portal_placed t=2.36 name=portal_moveable_rm2 linkage=0 portal2=0 origin=96.5 576.0 -74.0 angles=-0.0 -90.0 -0.0
[12.4] portal_placed t=5.00 name=portal_fixed_a01_0 linkage=0 portal2=1 origin=-384.0 288.0 52.0 angles=0.0 0.0 0.0
[14.0] portal_placed t=9.16 name=portal_moveable_rm2 linkage=0 portal2=0 origin=-128.0 160.5 -75.5 angles=-0.0 0.0 0.0
[21.0] portal_placed t=16.18 name=portal_moveable_rm2 linkage=0 portal2=0 origin=95.6 -64.0 -76.0 angles=-0.0 90.0 0.0
[28.0] portal_placed t=23.19 name=portal_moveable_rm2 linkage=0 portal2=0 origin=320.0 159.5 -74.0 angles=-0.0 180.0 0.0
"""

STUCK = """\
portal_placed t=2.36 name=portal_moveable_rm2 linkage=0 portal2=0 origin=96.5 576.0 -74.0 angles=-0.0 -90.0 -0.0
portal_placed t=9.36 name=portal_moveable_rm2 linkage=0 portal2=0 origin=96.5 576.0 -74.0 angles=-0.0 -90.0 -0.0
portal_placed t=16.38 name=portal_moveable_rm2 linkage=0 portal2=0 origin=96.5 576.0 -74.0 angles=-0.0 -90.0 -0.0
portal_placed t=23.39 name=portal_moveable_rm2 linkage=0 portal2=0 origin=96.5 576.0 -74.0 angles=-0.0 -90.0 -0.0
"""


def judge(text):
    stream = io.StringIO()
    checks = Checks(stream)
    carousel.judge(checks, carousel.parse_placements(text), "run")
    return checks, stream.getvalue()


class PedestalCarouselOracle(unittest.TestCase):
    def test_turning_pedestal_passes(self):
        checks, output = judge(TURNING)
        self.assertEqual(checks.failures, 0, output)
        self.assertEqual(checks.checks, 6)

    def test_stuck_pedestal_fails(self):
        checks, output = judge(STUCK)
        self.assertIn("FAIL run.quarter-turn.1", output)
        self.assertIn("FAIL run.four-walls", output)
        self.assertIn("FAIL run.different-places", output)
        self.assertEqual(checks.failures, 5)

    def test_too_few_shots_fail(self):
        three = "".join(TURNING.splitlines(keepends=True)[:4])
        checks, output = judge(three)
        self.assertIn("FAIL run.shots", output)
        self.assertIn("FAIL run.four-walls", output)

    def test_half_turn_steps_fail(self):
        # Alternating between two opposite walls is a turn, but not a quarter turn.
        lines = TURNING.splitlines(keepends=True)
        flip = lines[0] + lines[3].replace("t=16.18", "t=9.16") + \
            lines[0].replace("t=2.36", "t=16.18") + lines[3].replace("t=16.18", "t=23.19")
        checks, output = judge(flip)
        self.assertIn("FAIL run.quarter-turn.1", output)
        self.assertIn("FAIL run.four-walls", output)

    def test_orange_and_duplicate_lines_are_ignored(self):
        # engine.log and console.log carry the same lines.
        placements = carousel.parse_placements(TURNING + TURNING + "portal_placed t=bad\n")
        self.assertEqual(len(placements), 5)
        blue = [p for p in placements if not p["portal2"]]
        self.assertEqual([p["t"] for p in blue], [2.36, 9.16, 16.18, 23.19])

    def test_no_log_fails(self):
        checks, output = judge("")
        self.assertIn("FAIL run.shots", output)
        self.assertGreater(checks.failures, 0)


if __name__ == "__main__":
    unittest.main()
