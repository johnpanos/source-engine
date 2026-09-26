#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Self-tests for the oracle of tools/quality/hammer_ui_test.py (RFC 0002).

The UI suite trusts its verdict only after the judge is shown to accept the
intended room and reject each way the files can be wrong: a solid block, an
entity outside the room, a missing or duplicated light, a leaked or failed
build, and missing files. No compositor is needed.
"""

import json
import os
import sys
import tempfile
import unittest
from pathlib import Path

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(HERE))
import hammer_ui_test  # noqa: E402

SOLID = 'solid { "id" "%d" side { "plane" "(0 0 0) (1 0 0) (0 1 0)" } }'


def vmf(walls=6, entities=(("info_player_start", "-64 0 32"), ("light", "64 0 96"))):
    world = "\n".join(SOLID % i for i in range(walls))
    ents = "\n".join('entity { "id" "%d" "classname" "%s" "origin" "%s" }' % (100 + i, c, o)
                     for i, (c, o) in enumerate(entities))
    return 'versioninfo { "editorversion" "400" }\nworld { "classname" "worldspawn"\n%s\n}\n%s\n' % (
        world, ents)


class JudgeTest(unittest.TestCase):
    def setUp(self):
        self.tmp = Path(tempfile.mkdtemp(prefix="hammer-ui-"))
        self.vmf = self.tmp / "ui_room.vmf"
        self.build = self.tmp / "build.json"

    def judge(self, text=None, record=None):
        if text is not None:
            self.vmf.write_text(text)
        if record is not None:
            self.build.write_text(json.dumps(record))
        return {k: ok for k, (ok, _) in hammer_ui_test.judge(self.vmf, self.build).items()}

    def failing(self, **kwargs):
        return sorted(k for k, ok in self.judge(**kwargs).items() if not ok)

    def test_the_room_passes(self):
        self.assertEqual([], self.failing(text=vmf(), record={"status": "pass", "leaked": False}))

    def test_a_solid_block_fails_walls(self):
        self.assertEqual(["walls"], self.failing(text=vmf(walls=1),
                                                 record={"status": "pass", "leaked": False}))

    def test_entities_must_be_inside_the_room(self):
        for origin in ("-64 0 0", "-64 0 112", "176 0 32", "0 -176 32", "not a vector"):
            bad = vmf(entities=(("info_player_start", origin), ("light", "64 0 96")))
            self.assertEqual(["player"], self.failing(text=bad, record={"status": "pass", "leaked": False}),
                             origin)

    def test_exactly_one_light(self):
        record = {"status": "pass", "leaked": False}
        self.assertEqual(["light"], self.failing(
            text=vmf(entities=(("info_player_start", "-64 0 32"),)), record=record))
        self.assertEqual(["light"], self.failing(
            text=vmf(entities=(("info_player_start", "-64 0 32"), ("light", "64 0 96"),
                               ("light", "0 0 96"))), record=record))

    def test_brush_entity_solids_are_not_walls(self):
        text = vmf(walls=5) + 'entity { "classname" "func_detail" %s }\n' % (SOLID % 9)
        self.assertEqual(["walls"], self.failing(text=text, record={"status": "pass", "leaked": False}))

    def test_leaked_or_failed_builds_fail(self):
        self.assertEqual(["compiled", "sealed"],
                         self.failing(text=vmf(), record={"status": "leak", "leaked": True}))
        self.assertEqual(["compiled"],
                         self.failing(text=vmf(), record={"status": "fail", "leaked": False}))

    def test_missing_files_fail_everything(self):
        self.assertEqual(["compiled", "light", "player", "saved", "sealed", "walls"], self.failing())


if __name__ == "__main__":
    unittest.main()
