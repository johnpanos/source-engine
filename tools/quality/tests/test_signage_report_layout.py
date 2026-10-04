"""Source reports may join a console marker or follow it on the next line."""
from pathlib import Path
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from intro4_material_check import signage_reports, signage_report_sensitivity

# Known Intro4 geometry, independently fixed by the gameplay fixture.
SOURCES = (
    ("256 175.9 144.2", "-1.00 0.00 0.00"),
    ("256 207.9 144.1", "-1.00 0.00 0.00"),
    ("-80.1 176.1 0.1", "0.00 0.00 1.00"),
    ("-48.1 176 0.1", "0.00 0.00 1.00"),
    ("657 8.5 104.2", "-1.00 0.00 0.00"),
    ("-1560.1 -63.3 -32", "0.00 1.00 0.00"),
)


def report(separator):
    result = []
    for index, (center, front) in enumerate(SOURCES):
        for frame in range(2):
            result.append(f"RC_SIGNAGE_{index}_1{separator}"
                          "area lights generation 17: 0 lit (1 without a slot), 0 dropped\n"
                          f"slotless area 0 key 16 at {center} facing {front} "
                          "area 128 one-sided radiance 4 4 4 reach 100\n")
    result.append('"cl_surface_core_emission" = "1"\n')
    return "".join(result)


class SignageReportLayoutTests(unittest.TestCase):
    def test_both_layouts_and_seeded_bad_sources(self):
        for separator in (" ", "\n", " \n"):
            with self.subTest(separator=separator):
                text = report(separator)
                checks = signage_reports(text)
                self.assertEqual(len(checks), 6)
                self.assertTrue(all(check['passed'] for check in checks))
                self.assertTrue(all(check['passed'] for check in signage_report_sensitivity(text)))
                missing = text.replace("RC_SIGNAGE_3_1", "RC_SIGNAGE_3_0", 1)
                checks = signage_reports(missing)
                self.assertFalse(next(check['passed'] for check in checks
                                      if check['name'] == 'signage.boxdispenser.core-only-front'))


if __name__ == "__main__":
    unittest.main()
