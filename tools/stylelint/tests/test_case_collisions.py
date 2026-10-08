"""Tracked paths that differ only in case (tools/stylelint/case_collisions.py)."""

import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import case_collisions  # noqa: E402


class CaseCollisionTests(unittest.TestCase):
    def test_the_repository_has_none(self):
        self.assertEqual([], case_collisions.collisions(
            case_collisions.tracked(case_collisions.ROOT)))

    def test_seeded_forwarding_header_is_caught(self):
        paths = ["public/appframework/IAppSystem.h", "public/appframework/iappsystem.h",
                 "public/tier1/KeyValues.h"]
        self.assertEqual([["public/appframework/IAppSystem.h",
                           "public/appframework/iappsystem.h"]],
                         case_collisions.collisions(paths))

    def test_directories_differing_in_case_collide_too(self):
        self.assertEqual(1, len(case_collisions.collisions(["Game/a.h", "game/a.h"])))

    def test_distinct_names_and_imported_trees_pass(self):
        self.assertEqual([], case_collisions.collisions(
            ["kvtext/keyvalues.h", "public/tier1/KeyValues.h",
             "external/vpc/x.h", "external/vpc/X.h"]))


if __name__ == "__main__":
    unittest.main()
