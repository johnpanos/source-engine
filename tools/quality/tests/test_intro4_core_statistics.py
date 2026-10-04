"""A declined live material must fail game evidence even with no failed claims."""
import sys
from pathlib import Path
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from intro4_material_check import validate_core_statistics

CLAIMS = "r_core_world_stats: views queued 12 drawn 12 failed 0 skipped 0\n"
DYNAMIC = "r_core_world_stats: dynamic draws 24 refused 0 last refusal ''\n"


class CoreStatisticsTests(unittest.TestCase):
    def test_complete_zero_failure_statistics(self):
        validate_core_statistics(CLAIMS + DYNAMIC)

    def test_missing_statistics_cannot_certify(self):
        for log in ("", CLAIMS, DYNAMIC):
            with self.subTest(log=log), self.assertRaises(ValueError):
                validate_core_statistics(log)

    def test_refused_material_cannot_hide_behind_successful_claims(self):
        with self.assertRaisesRegex(ValueError, "unsupported live material"):
            validate_core_statistics(CLAIMS + DYNAMIC.replace("refused 0", "refused 1"))

    def test_later_clean_report_does_not_erase_failures(self):
        for bad in (CLAIMS.replace("failed 0", "failed 1"),
                    DYNAMIC.replace("refused 0", "refused 7")):
            with self.subTest(log=bad), self.assertRaises(ValueError):
                validate_core_statistics(bad + CLAIMS + DYNAMIC)


if __name__ == "__main__":
    unittest.main()
