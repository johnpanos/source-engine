"""Oracle tests for the spark_lab fixtures (tools/quality/spark_light_scene.py).

The logs are synthetic fx_spark_lights_debug 2 logs of the map's layout: a
fall burst whose sparks land on the floor and slide, and the budget row. A log
written by the current policy passes; logs written by the first policy (the
light counting only sparks near the source, a first-come budget) and logs with
uncarried or unlit sparks must fail.
"""

import math
import sys
import unittest
from pathlib import Path

QUALITY = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(QUALITY))

import spark_light_scene as scene  # noqa: E402

VIEW = (1000.0, 380.0, 72.0)
LAYOUT = {
    "sources": {"fall": (-768, 0, 512), "wall": (-256, -496, 192), "pcf": (256, 0, 64),
                **{"budget%d" % i: (960 - 112 * i, 300, 96) for i in range(6)}},
}
LAYOUT["budget_sources"] = [n for n in LAYOUT["sources"] if n.startswith("budget")]


def fall_sparks(frames=120):
    """Per frame, the fall burst's sparks: thrown up and out from the
    source, landing on the floor and sliding."""
    source = LAYOUT["sources"]["fall"]
    sparks = []
    for i in range(24):
        angle = 0.2618 * i
        sparks.append({"pos": list(source), "vel": [250 * math.cos(angle), 250 * math.sin(angle), 150.0],
                       "life": 0.0, "die": 1.0 + 0.05 * i})
    out = []
    dt = 1.0 / 60.0
    for _ in range(frames):
        live = []
        for spark in sparks:
            spark["life"] += dt
            if spark["life"] >= spark["die"]:
                continue
            spark["vel"][2] -= 800 * dt
            for k in range(3):
                spark["pos"][k] += spark["vel"][k] * dt
            if spark["pos"][2] < 0:
                spark["pos"][2] = 0.0
                spark["vel"][2] = 0.0
            live.append((*spark["pos"], 1.0 - spark["life"] / spark["die"]))
        out.append(live)
    return out


def current_light(sparks, base):
    total = sum(s[3] for s in sparks)
    center = tuple(sum(s[k] * s[3] for s in sparks) / total for k in range(3))
    spread = math.sqrt(sum(s[3] * scene.distance(s[:3], center) ** 2 for s in sparks) / total)
    return center, min(base + 1.5 * spread, 2.5 * base)


def first_light(sparks, base):
    return scene.first_policy_light(sparks, LAYOUT["sources"]["fall"], base), base


def write_log(light_policy=current_light, first_come=False, uncarried=False, unlit_sparks=False):
    lines = []
    frame = 100

    def emit_frame(lights, sparks):
        lines.append("sparkdbg view %d %.2f %.2f %.2f 4" % (frame, *VIEW))
        for key, burst in sparks.items():
            for s in burst:
                lines.append("sparkdbg spark %d %d %.2f %.2f %.2f %.4f" % (frame, key, *s))
        for key, (lit, origin, radius, strength) in lights.items():
            lines.append("sparkdbg light %d %d %d %.2f %.2f %.2f %.2f %.5f %.2f"
                         % (frame, key, int(lit), *origin, radius, strength, 0.0))

    # The other scenes' first frames: a trail burst lit where it starts.
    for key, name in ((201, "wall"), (202, "pcf")):
        emit_frame({key: (True, LAYOUT["sources"][name], 192.0, 0.3)}, {})
        frame += 1
    # The fall burst: lit at its source, then following its sparks.
    emit_frame({300: (True, LAYOUT["sources"]["fall"], 192.0, 0.3)}, {})
    frame += 1
    for sparks in fall_sparks():
        if not sparks:
            break
        origin, radius = light_policy(sparks, 192.0)
        lights = {} if origin is None or unlit_sparks else {300: (True, origin, radius, 0.3)}
        burst = {300: sparks}
        if uncarried:
            burst[0] = sparks[:3]
        emit_frame(lights, burst)
        frame += 1
    # The budget row, the farthest asking first (lower keys).
    row = sorted(range(6), key=lambda i: -i)
    lights = {}
    for order, i in enumerate(row):
        near_rank = 5 - i  # budget0 is the nearest the view
        lit = order < 4 if first_come else near_rank >= 2
        lights[400 + order] = (lit, LAYOUT["sources"]["budget%d" % i], 128.0, 0.1)
    emit_frame(lights, {})
    return "\n".join(lines) + "\n"


class SparkLightSceneTests(unittest.TestCase):
    def judge(self, **kwargs):
        return scene.evaluate(scene.parse_log(write_log(**kwargs)), LAYOUT)

    def test_current_policy_passes(self):
        report = self.judge()
        self.assertEqual(report["status"], "pass", report["failures"])
        self.assertTrue(report["fall_reached_floor"])
        self.assertEqual(report["budget_row_checks"], 1)
        self.assertGreater(report["first_policy"]["failed"], 0)

    def test_first_policy_light_is_rejected(self):
        report = self.judge(light_policy=first_light)
        self.assertEqual(report["status"], "fail")
        self.assertTrue(any("reaches" in f or "asks for no light" in f for f in report["failures"]))

    def test_first_come_budget_is_rejected(self):
        report = self.judge(first_come=True)
        self.assertEqual(report["status"], "fail")
        self.assertTrue(any("outranks" in f or "nearest" in f for f in report["failures"]))

    def test_uncarried_sparks_are_rejected(self):
        report = self.judge(uncarried=True)
        self.assertTrue(any("belong to no burst light" in f for f in report["failures"]))

    def test_sparks_without_light_are_rejected(self):
        report = self.judge(unlit_sparks=True)
        self.assertTrue(any("asks for no light" in f for f in report["failures"]))

    def test_missing_scene_is_rejected(self):
        layout = dict(LAYOUT, sources=dict(LAYOUT["sources"], extra=(0, 400, 64)))
        report = scene.evaluate(scene.parse_log(write_log()), layout)
        self.assertTrue(any("scene extra produced no burst" in f for f in report["failures"]))

    def test_empty_log_fails(self):
        report = scene.evaluate(scene.parse_log(""), LAYOUT)
        self.assertEqual(report["status"], "fail")


if __name__ == "__main__":
    unittest.main()
