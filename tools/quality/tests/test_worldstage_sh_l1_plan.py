#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Reject unsafe or non-directional supplemental Cycles bake plans."""

import json
from pathlib import Path
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from worldstage_sh_l1_plan import load_plan  # noqa: E402


class DirectionPlanTests(unittest.TestCase):
    def check_rejected(self, entry):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "plan.json"
            path.write_text(json.dumps({"schema": 1,
                                        "id": "rfc0008-sh-l1-supplemental-v1",
                                        "additional": [entry]}))
            with self.assertRaises(ValueError):
                load_plan(path)

    def test_path_escape_id_is_rejected(self):
        self.check_rejected({"id": "sh../escape", "normal": [0, 0, 1]})

    def test_nonunit_direction_is_rejected(self):
        self.check_rejected({"id": "sh00", "normal": [0, 0, 2]})

    def test_lower_hemisphere_is_rejected(self):
        self.check_rejected({"id": "sh00", "normal": [0, 0, -1]})


if __name__ == "__main__":
    unittest.main()
