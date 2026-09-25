"""Positive and negative fixtures for the paint gel screenshot checks."""

import datetime
import json
import os
import sys
import tempfile
import unittest
from pathlib import Path

import numpy as np
from PIL import Image

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import portal2_paint_shots as shots  # noqa: E402

BLUE = (0, 150, 240)
ORANGE = (240, 100, 10)
SCENARIO = "sp_a3_speed_ramp_gel"


def view():
    image = np.full((240, 320, 3), 60, dtype=np.uint8)
    image[:, 150:170] = (90, 80, 50)  # a pillar
    return image


def with_blobs(image, colour, x, radius, count=6):
    image = image.copy()
    yy, xx = np.mgrid[0:image.shape[0], 0:image.shape[1]]
    for i in range(count):
        cy = 20 + i * 35
        image[(yy - cy) ** 2 + (xx - x) ** 2 <= radius ** 2] = colour
    return image


def gel(radius):
    return with_blobs(with_blobs(view(), ORANGE, 60, radius), BLUE, 260, radius)


class CheckShotsTest(unittest.TestCase):
    def outcomes(self, **overrides):
        images = {"empty": view(), "isosurface": gel(9), "spheres": gel(7)}
        images.update(overrides)
        return {name: ok for name, (ok, _) in shots.check_shots(images).items()}

    def test_both_gels_drawn_as_isosurface(self):
        self.assertTrue(all(self.outcomes().values()), self.outcomes())

    def test_no_gel_fails(self):
        outcomes = self.outcomes(isosurface=view(), spheres=view())
        self.assertFalse(outcomes["gel.visible"])
        self.assertFalse(outcomes["gel.repulsion_blue"])
        self.assertFalse(outcomes["gel.propulsion_orange"])
        self.assertFalse(outcomes["gel.spheres_visible"])

    def test_untextured_gel_fails_colour_checks(self):
        # Both streams one pale colour: the base textures were not bound.
        pale = with_blobs(with_blobs(view(), (225, 177, 120), 60, 9), (225, 177, 120), 260, 9)
        outcomes = self.outcomes(isosurface=pale)
        self.assertTrue(outcomes["gel.visible"])
        self.assertFalse(outcomes["gel.repulsion_blue"])
        self.assertFalse(outcomes["gel.propulsion_orange"])

    def test_one_stream_missing_fails_its_colour(self):
        only_orange = with_blobs(view(), ORANGE, 60, 9)
        outcomes = self.outcomes(isosurface=only_orange)
        self.assertTrue(outcomes["gel.propulsion_orange"])
        self.assertFalse(outcomes["gel.repulsion_blue"])

    def test_sphere_fallback_in_isosurface_shot_fails(self):
        outcomes = self.outcomes(isosurface=gel(7), spheres=gel(7))
        self.assertFalse(outcomes["gel.isosurface_drawn"])

    def test_missing_or_mismatched_shots_raise(self):
        with self.assertRaises(shots.ShotError):
            shots.check_shots({"empty": view(), "isosurface": gel(9)})
        with self.assertRaises(shots.ShotError):
            shots.check_shots({"empty": view(), "isosurface": gel(9),
                               "spheres": np.zeros((10, 10, 3), dtype=np.uint8)})


class CollectShotsTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)
        self.runtime = self.root / "runtime"
        self.screenshots = self.runtime / "portal2/screenshots"
        self.screenshots.mkdir(parents=True)
        self.evidence = self.root / "evidence"
        (self.evidence / SCENARIO).mkdir(parents=True)
        self.started = datetime.datetime.now(datetime.timezone.utc) - datetime.timedelta(minutes=1)

    def tearDown(self):
        self.temp.cleanup()

    def write_run(self, status="pass", names=("empty", "isosurface", "spheres"), files=None):
        evidence = {"started_utc": self.started.isoformat(), "runtime": str(self.runtime),
                    "results": [{"scenario": SCENARIO, "status": status}]}
        (self.evidence / "evidence.json").write_text(json.dumps(evidence))
        (self.evidence / SCENARIO / "console.log").write_text(
            "".join("QA_SHOT %s %s\n" % (SCENARIO, name) for name in names))
        images = {"empty": view(), "isosurface": gel(9), "spheres": gel(7)}
        for index, name in enumerate(files if files is not None else names):
            path = self.screenshots / ("sp_a3_speed_ramp%04d.tga" % index)
            Image.fromarray(images.get(name, view())).save(path)

    def test_run_passes_and_writes_pngs(self):
        self.write_run()
        self.assertEqual(shots.main(["--evidence", str(self.evidence)]), 0)
        for name in ("empty", "isosurface", "spheres"):
            self.assertTrue((self.evidence / SCENARIO / "shots" / (name + ".png")).is_file())
        report = json.loads((self.evidence / SCENARIO / "shots/shots.json").read_text())
        self.assertEqual(report["status"], "pass")

    def test_failed_scenario_fails(self):
        self.write_run(status="fail")
        self.assertEqual(shots.main(["--evidence", str(self.evidence)]), 1)

    def test_screenshot_count_mismatch_fails(self):
        self.write_run(files=("empty", "isosurface"))
        self.assertEqual(shots.main(["--evidence", str(self.evidence)]), 1)

    def test_stale_screenshots_are_ignored(self):
        stale = self.screenshots / "sp_a3_speed_ramp9999.tga"
        Image.fromarray(view()).save(stale)
        old = (self.started - datetime.timedelta(hours=1)).timestamp()
        os.utime(stale, (old, old))
        self.write_run()
        self.assertEqual(shots.main(["--evidence", str(self.evidence)]), 0)


if __name__ == "__main__":
    unittest.main()
