#!/usr/bin/env python3
"""Self-tests for tools/quality/lighting_fixtures.py (RFC 0016 K11 fixtures).

Covers the entity conversions against vrad's own arithmetic, the projected
light's closed form and the reference check built on it, the error metric,
and `check` on a small fixture root with seeded defects: a missing
reference, a reference file that changed, a tolerance edited in place, a
tolerance fixed after the comparison that uses it, a new tolerance version
fixed after a comparison with no reason, a term with no negative control, an
unknown term, a manifest camera the fixture lacks, a fixture changed since
its references, a mislabelled sample count and a fixture with no tolerance.
Each must be rejected, and the clean root and the checked-in set must pass.

    python3 -m unittest tools/quality/tests/test_lighting_fixtures.py
"""

import copy
import datetime
import json
import math
import os
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

import numpy as np
from PIL import Image

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE.parent))
import lighting_fixtures as lf  # noqa: E402

ROOT = HERE.parents[2]


def vrad_world_light(keys, kind):
    """vrad's arithmetic (lightmap.cpp LightForString, SetLightFalloffParams,
    the export's 1/255), written independently of the converter."""
    values = [float(v) for v in keys["_light"].split()]
    r, g, b, scaler = values
    intensity = [((c / 255.0) ** 2.2) * 255.0 * scaler / 255.0 for c in (r, g, b)]
    if kind in ("point", "spot"):
        c, l, q = (float(keys[k]) for k in ("_constant_attn", "_linear_attn", "_quadratic_attn"))
        ratio = c + 100 * l + 100 * 100 * q
        intensity = [v * ratio for v in intensity]
    return [v / 255.0 for v in intensity]


class EntityConversions(unittest.TestCase):
    def test_point_light_compiles_to_the_spheres_intensity(self):
        light = {"kind": "point", "name": "Bulb", "center_m": [1.0, 2.0, 3.0],
                 "radiance": [110.0, 55.0, 11.0]}
        keys = lf.point_entity(light)
        self.assertEqual(keys["classname"], "light")
        want = [c * lf.LIGHT_RADIUS_UNITS ** 2 for c in light["radiance"]]
        for got, expect in zip(vrad_world_light(keys, "point"), want):
            self.assertAlmostEqual(got / expect, 1.0, places=5)
        self.assertEqual(keys["origin"], lf.vec_text(lf.units([1.0, 2.0, 3.0])))

    def test_spot_direction_and_cone(self):
        light = {"kind": "spot", "name": "Spot", "center_m": [0.0, 0.0, 5.0],
                 "direction": lf.normalize([1.0, 0.0, -1.0]), "radiance": [1000.0] * 3,
                 "inner_degrees": 14.0, "outer_degrees": 24.0, "exponent": 1.0}
        keys = lf.point_entity(light)
        self.assertEqual(keys["classname"], "light_spot")
        pitch = float(keys["pitch"])
        yaw = float(keys["angles"].split()[1])
        # map_utils.cpp SetupLightNormalFromProps
        normal = [math.cos(math.radians(yaw)) * math.cos(math.radians(pitch)),
                  math.sin(math.radians(yaw)) * math.cos(math.radians(pitch)),
                  math.sin(math.radians(pitch))]
        for a, b in zip(normal, light["direction"]):
            self.assertAlmostEqual(a, b, places=4)
        self.assertEqual(keys["_inner_cone"], "14.0000")
        self.assertEqual(keys["_cone"], "24.0000")

    def test_environment_sun_and_sky(self):
        sun = {"kind": "sun", "name": "Sun", "travel": lf.normalize([0.3, 0.2, -0.8]),
               "irradiance": [5.0, 4.4, 3.6], "angle_degrees": 0.53}
        sky = {"kind": "sky", "name": "Sky", "radiance": [0.25, 0.33, 0.5]}
        keys = lf.environment_entity(sun, sky)
        for got, want in zip(vrad_world_light(keys, "sky"), sun["irradiance"]):
            self.assertAlmostEqual(got / (want / math.pi), 1.0, places=5)
        ambient = vrad_world_light({"_light": keys["_ambient"]}, "sky_ambient")
        for got, want in zip(ambient, sky["radiance"]):
            self.assertAlmostEqual(got / want, 1.0, places=5)

    def test_rect_front_is_its_emission_direction(self):
        # As LightingScene.rect_light builds it from look_rotation's rows.
        direction = lf.normalize([0.3, 0.8, -0.5])
        back = [-c for c in direction]
        helper = [0.0, 0.0, 1.0]
        x = lf.normalize(np.cross(helper, back))
        y = list(np.cross(back, x))
        size = (1.6, 1.0)
        light = {"name": "Key", "center_m": [3.0, 0.4, 3.4],
                 "half_u_m": [c * size[1] / 2 for c in y],
                 "half_v_m": [c * size[0] / 2 for c in x], "radiance": [14.0, 13.5, 12.5]}
        keys = lf.rect_entity(light)
        u = np.array([float(v) for v in keys["halfu"].split()])
        v = np.array([float(v) for v in keys["halfv"].split()])
        front = np.cross(u, v)
        self.assertGreater(np.dot(front / np.linalg.norm(front), direction), 0.9999)
        self.assertAlmostEqual(4 * np.linalg.norm(u) * np.linalg.norm(v),
                               size[0] * size[1] * lf.SOURCE_UNITS_PER_METER ** 2, places=1)

    def test_world_light_comparison_catches_a_wrong_light(self):
        lights = [{"kind": "point", "name": "A", "center_m": [0.0, 0.0, 1.0],
                   "radiance": [10.0, 10.0, 10.0]}]
        expected = lf.expected_world_lights(lights)
        good = [{"type": "point", "origin": lf.units([0.0, 0.0, 1.0]),
                 "intensity": [40.0, 40.0, 40.0]}]
        self.assertEqual(lf.compare_world_lights(expected, good), [])
        doubled = [dict(good[0], intensity=[80.0, 80.0, 80.0])]
        self.assertTrue(lf.compare_world_lights(expected, doubled))
        moved = [dict(good[0], origin=[5.0, 0.0, 39.37])]
        self.assertTrue(lf.compare_world_lights(expected, moved))
        self.assertTrue(lf.compare_world_lights(expected, []))


def projector(fov=60.0, near=4.0, far=600.0):
    forward, right, up = lf.angle_vectors(0.0, 0.0)
    return {"origin_units": [0.0, 0.0, 0.0], "forward": forward, "right": right, "up": up,
            "horizontal_fov_degrees": fov, "vertical_fov_degrees": fov, "near_z": near,
            "far_z": far, "attenuation": [0.0, 100.0, 0.0], "color": [2.0, 1.0, 0.5]}


class ProjectedLight(unittest.TestCase):
    def irradiance(self, point, normal, cookie=None, light=None):
        cookie = np.ones((4, 4, 3)) if cookie is None else cookie
        return lf.projector_irradiance(light or projector(), np.array([[point]], float),
                                       np.array([[normal]], float), cookie)[0, 0]

    def test_on_axis_attenuation(self):
        # Portal's linear 100 / d, clamped to 1, facing the light.
        e = self.irradiance([200.0, 0.0, 0.0], [-1.0, 0.0, 0.0])
        np.testing.assert_allclose(e, [1.0, 0.5, 0.25], rtol=1e-9)
        e = self.irradiance([50.0, 0.0, 0.0], [-1.0, 0.0, 0.0])
        np.testing.assert_allclose(e, [2.0, 1.0, 0.5], rtol=1e-9)

    def test_frustum_near_far_and_end_falloff(self):
        self.assertEqual(self.irradiance([2.0, 0.0, 0.0], [-1, 0, 0]).max(), 0.0)
        self.assertEqual(self.irradiance([700.0, 0.0, 0.0], [-1, 0, 0]).max(), 0.0)
        self.assertEqual(self.irradiance([100.0, 0.0, 90.0], [-1, 0, 0]).max(), 0.0)
        # endFalloff: halfway between 0.6 far and far.
        e = self.irradiance([480.0, 0.0, 0.0], [-1.0, 0.0, 0.0])
        np.testing.assert_allclose(e[0], 2.0 * 100.0 / 480.0 * 0.5, rtol=1e-9)
        self.assertEqual(self.irradiance([200.0, 0.0, 0.0], [1.0, 0.0, 0.0]).max(), 0.0)

    def test_cookie_orientation(self):
        # u runs along right (-y for yaw 0), v down from up (+z).
        cookie = np.zeros((2, 2, 3))
        cookie[0, 0] = [1.0, 0.0, 0.0]  # top-left: up and left (+y)
        top_left = self.irradiance([100.0, 30.0, 30.0], [-1.0, 0.0, 0.0], cookie)
        top_right = self.irradiance([100.0, -30.0, 30.0], [-1.0, 0.0, 0.0], cookie)
        self.assertGreater(top_left[0], 0.0)
        self.assertEqual(top_right[0], 0.0)

    def synthetic_passes(self, light, cookie, scale=1.0, flip=False):
        """A Lambertian wall at x = 150 units facing the light: Position and
        Normal passes and a DiffDir of the formula (optionally seeded wrong)."""
        ys, zs = np.meshgrid(np.linspace(-80, 80, 64), np.linspace(80, -80, 48))
        positions = np.stack([np.full_like(ys, 150.0), ys, zs], axis=-1)
        normals = np.broadcast_to([-1.0, 0.0, 0.0], positions.shape)
        used = cookie[::-1] if flip else cookie
        diffuse = lf.projector_irradiance(light, positions, normals, used) * scale
        return {"Position": positions / lf.SOURCE_UNITS_PER_METER,
                "Normal": np.array(normals), "DiffDir": diffuse,
                "IndexOB": np.full(ys.shape + (1,), 1.0)}

    def test_reference_check_rejects_seeded_renders(self):
        cookie = (np.indices((16, 16)).sum(axis=0) % 2)[..., None] * np.ones(3) * 0.8 + 0.2
        with tempfile.TemporaryDirectory() as temporary:
            directory = Path(temporary)
            Image.fromarray((cookie * 255).round().astype(np.uint8)).save(directory / "c.png")
            cookie = np.asarray(Image.open(directory / "c.png"), float)[..., :3] / 255.0
            light = dict(projector(), cookie_image="c.png")
            fixture = {"directory": directory}
            index = {"Floor": 1}
            good = lf.projector_check(fixture, self.synthetic_passes(light, cookie), index,
                                      light)
            self.assertTrue(good["ok"], good)
            bright = lf.projector_check(fixture, self.synthetic_passes(light, cookie, 1.2),
                                        index, light)
            self.assertFalse(bright["ok"], bright)
            flipped = lf.projector_check(
                fixture, self.synthetic_passes(light, cookie, flip=True), index, light)
            self.assertFalse(flipped["ok"], flipped)


class BakeOverrides(unittest.TestCase):
    def test_final_drops_preview_samples(self):
        final = lf.final_overrides(lf.with_probes())
        self.assertNotIn("lightmap", final)
        self.assertNotIn("probe_volume", final)
        self.assertEqual(final["reflection_probe"], lf.FINAL_PROBES)
        self.assertIsNone(final["radiosity"])
        self.assertEqual(lf.final_overrides({"lightmap": {"samples": 64, "layout": "planar"}}),
                         {"lightmap": {"layout": "planar"}})


class ExrWriter(unittest.TestCase):
    def test_round_trip(self):
        image = np.random.default_rng(2).uniform(0, 50, (7, 9, 3))
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / "x.exr"
            lf.write_exr(path, image)
            np.testing.assert_array_equal(lf.read_rgb_exr(path), image.astype(np.float32))
            lf.write_exr(path, image, half=True)
            np.testing.assert_array_equal(lf.read_rgb_exr(path), image.astype(np.float16))


class ErrorMetric(unittest.TestCase):
    def setUp(self):
        rng = np.random.default_rng(3)
        self.reference = rng.uniform(0.2, 1.0, (40, 50, 3))
        self.index = np.ones((40, 50), np.int64)
        self.index[:, :5] = 0      # background
        self.index[:5, :] = 7      # an emitter

    def test_identical_is_zero_and_masks_apply(self):
        stats = lf.error_stats(self.reference, self.reference, self.index, [7])
        self.assertEqual(stats["mean"], 0.0)
        self.assertEqual(stats["pixels"], 35 * 45)
        test = self.reference.copy()
        test[self.index != 1] = 1e6  # background and emitter pixels are not judged
        self.assertEqual(lf.error_stats(test, self.reference, self.index, [7])["p99"], 0.0)

    def test_uniform_and_local_errors(self):
        stats = lf.error_stats(self.reference * 1.1, self.reference, self.index, [7])
        self.assertGreater(stats["mean"], 0.02)
        local = self.reference.copy()
        local[20:26, 20:26] += 10.0  # 36 of 1575 pixels
        stats = lf.error_stats(local, self.reference, self.index, [7])
        self.assertLess(stats["mean"], 0.5)
        self.assertGreater(stats["p99"], 5.0)

    def test_shape_mismatch_and_nan(self):
        with self.assertRaises(ValueError):
            lf.error_stats(self.reference[:10], self.reference, self.index, [7])
        bad = self.reference.copy()
        bad[10, 10, 0] = np.nan
        self.assertEqual(lf.error_stats(bad, self.reference, self.index, [7])["mean"],
                         float("inf"))


def write_pfm(path, image):
    image = np.asarray(image, "<f4")
    with open(path, "wb") as stream:
        stream.write(b"PF\n%d %d\n-1.0\n" % (image.shape[1], image.shape[0]))
        stream.write(image[::-1].tobytes())


class FixtureRoot:
    """A minimal fixture root: one fixture exercising every term, with
    rendered-looking references, a tolerance and a manifest."""

    FIXED = "2026-09-29T08:00:00+00:00"

    def __init__(self, directory):
        self.root = Path(directory)
        fixture_dir = self.root / "tiny"
        (fixture_dir / "references").mkdir(parents=True)
        (fixture_dir / "tiny.usda").write_text("#usda 1.0\n")
        judged = [t for t in lf.TERMS if "judged_by" not in lf.TERMS[t]]
        self.fixture = {"schema": "gi-fixture/v1", "name": "tiny", "stage": "tiny.usda",
                        "states": {"default": {"layer": None, "note": "on"}},
                        "cameras": {"front": {"eye": [0, 0, 1], "forward": [0, 1, 0],
                                              "up": [0, 0, 1]}},
                        "film": {"width": 64, "height": 48}, "horizontal_fov_degrees": 90.0,
                        "dynamic_models": [], "lighting": {"terms": judged, "projectors": [],
                                                           "media": {}, "map": {}}}
        lf.write_json(fixture_dir / "fixture.json", self.fixture)
        self.fixture["directory"] = fixture_dir
        rng = np.random.default_rng(5)
        self.image = rng.uniform(0.1, 1.0, (48, 64, 3)).astype(np.float32)
        lf.write_exr(fixture_dir / "references" / "default.front.total.exr", self.image)
        index = np.ones((48, 64), np.uint16)
        Image.fromarray(index).save(fixture_dir / "references" / "default.front.index.png")
        refs = fixture_dir / "references"
        views = {"default.front": {
            "state": "default", "camera": "front",
            "files": {"total": {"file": "default.front.total.exr",
                                "sha256": lf.sha256(refs / "default.front.total.exr")},
                      "index": {"file": "default.front.index.png",
                                "sha256": lf.sha256(refs / "default.front.index.png")}},
            "object_index": {"Wall": 1}, "emitter_indices": [], "judged_pixels": 3072}}
        record = {"schema": lf.REFERENCES_SCHEMA, "fixture": "tiny",
                  "fixture_reference_digest": lf.reference_digest(self.fixture),
                  "samples": 16, "seed": 1, "status": "preview",
                  "renders": {"default": {"stage_layers": {}}}, "views": views,
                  "analytic": []}
        lf.write_json(refs / "references.json", record)
        entry = {"fixture": "tiny", "version": 1, "mean": 0.05, "p99": 0.5,
                 "fixed": self.FIXED}
        entry["digest"] = lf.tolerance_digest(entry)
        lf.write_json(self.root / "tolerances.json", {"schema": lf.TOLERANCES_SCHEMA,
                                                      "entries": [entry]})
        terms = {t: dict(v, fixtures=["tiny"] if t in judged else [])
                 for t, v in lf.TERMS.items()}
        lf.write_json(self.root / "manifest.json", {
            "schema": lf.MANIFEST_SCHEMA, "terms": terms,
            "fixtures": [{"name": "tiny", "terms": judged, "cameras": ["front"],
                          "states": ["default"], "reused_map": False}]})

    def edit(self, name, change):
        path = self.root / name
        value = json.loads(path.read_text())
        change(value)
        lf.write_json(path, value)


class Check(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.tree = FixtureRoot(self.temporary.name)

    def tearDown(self):
        self.temporary.cleanup()

    def problems(self):
        return lf.check(self.tree.root, maps=False)

    def assertRejected(self, fragment):
        problems = self.problems()
        self.assertTrue(any(fragment in p for p in problems),
                        "expected a problem containing %r, got %s" % (fragment, problems))

    def test_clean_root_passes(self):
        self.assertEqual(self.problems(), [])

    def test_missing_reference_record(self):
        (self.tree.root / "tiny/references/references.json").unlink()
        self.assertRejected("no references")

    def test_missing_reference_image(self):
        (self.tree.root / "tiny/references/default.front.total.exr").unlink()
        self.assertRejected("missing or differs")

    def test_changed_reference_image(self):
        lf.write_exr(self.tree.root / "tiny/references/default.front.total.exr",
                     self.tree.image * 1.01)
        self.assertRejected("missing or differs")

    def test_fixture_changed_after_render(self):
        self.tree.edit("tiny/fixture.json",
                       lambda f: f["cameras"]["front"].update(eye=[0, 0, 2]))
        self.assertRejected("changed since its references")

    def test_preview_labelled_final(self):
        self.tree.edit("tiny/references/references.json", lambda r: r.update(status="final"))
        self.assertRejected("does not match 16 samples")

    def test_tolerance_edited_in_place(self):
        self.tree.edit("tolerances.json", lambda t: t["entries"][0].update(mean=0.5))
        self.assertRejected("edited after it was fixed")

    def test_no_tolerance(self):
        self.tree.edit("tolerances.json", lambda t: t.update(entries=[]))
        self.assertRejected("no tolerance fixed")

    def record(self, compared, version=1):
        result = lf.compare("tiny", "default", "front", self.pfm(), root=self.tree.root,
                            now=datetime.datetime.fromisoformat(compared))
        result["tolerance_version"] = version
        lf.write_json(self.tree.root / "results" / ("r%d.json" % version), result)
        return result

    def pfm(self):
        path = self.tree.root / "lab.pfm"
        write_pfm(path, self.tree.image)
        return path

    def test_compare_passes_the_reference_and_certifies_nothing_in_preview(self):
        result = lf.compare("tiny", "default", "front", self.pfm(), root=self.tree.root)
        self.assertTrue(result["pass"])
        self.assertFalse(result["certifies"])
        self.assertLess(result["mean"], 1e-6)
        black = self.tree.root / "black.pfm"
        write_pfm(black, np.zeros_like(self.tree.image))
        self.assertFalse(lf.compare("tiny", "default", "front", black,
                                    root=self.tree.root)["pass"])

    def test_recorded_comparison_after_tolerance_passes(self):
        self.record("2026-09-29T09:00:00+00:00")
        self.assertEqual(self.problems(), [])

    def test_tolerance_fixed_after_the_comparison(self):
        self.record("2026-09-29T07:00:00+00:00")
        self.assertRejected("after the comparison")

    def test_new_version_after_a_comparison_needs_a_reason(self):
        self.record("2026-09-29T09:00:00+00:00")
        later = {"fixture": "tiny", "version": 2, "mean": 0.2, "p99": 2.0,
                 "fixed": "2026-09-29T10:00:00+00:00"}
        later["digest"] = lf.tolerance_digest(later)
        self.tree.edit("tolerances.json", lambda t: t["entries"].append(later))
        self.assertRejected("without a stated reason")
        self.tree.edit("tolerances.json", lambda t: t["entries"][1].update(
            reason="the reference was re-rendered at 2048 samples"))
        self.assertEqual(self.problems(), [])

    def test_comparison_against_a_changed_reference_is_stale(self):
        self.record("2026-09-29T09:00:00+00:00")
        refs = self.tree.root / "tiny/references"
        lf.write_exr(refs / "default.front.total.exr", self.tree.image * 1.5)
        self.tree.edit("tiny/references/references.json", lambda r: r["views"][
            "default.front"]["files"]["total"].update(
                sha256=lf.sha256(refs / "default.front.total.exr")))
        self.assertRejected("stale")

    def test_comparison_whose_tolerance_changed(self):
        self.record("2026-09-29T09:00:00+00:00")
        self.tree.edit("results/r1.json", lambda r: r.update(tolerance_digest="0" * 64))
        self.assertRejected("changed after the comparison")

    def test_term_without_negative_control(self):
        self.tree.edit("manifest.json",
                       lambda m: m["terms"]["brdf"].update(negative_control=""))
        self.assertRejected("brdf has no negative control")

    def test_unknown_and_missing_terms(self):
        self.tree.edit("manifest.json", lambda m: m["fixtures"][0]["terms"].append("magic"))
        self.assertRejected("unknown term magic")
        self.tree.edit("manifest.json", lambda m: m["terms"].pop("sun"))
        self.assertRejected("lacks term sun")

    def test_unexercised_term(self):
        self.tree.edit("manifest.json", lambda m: m["terms"]["sun"].update(fixtures=[]))
        self.assertRejected("sun is exercised by no fixture")

    def test_unbuilt_map_is_reported(self):
        fixture = dict(self.tree.fixture, lighting=dict(self.tree.fixture["lighting"],
                                                        map={"bsp": "run/maps/nope/x.bsp"}))
        self.assertTrue(any("is not built" in p for p in lf.check_map(fixture)))

    def test_map_entities_must_match(self):
        with tempfile.TemporaryDirectory() as temporary:
            bsp = Path(temporary) / "m.bsp"
            text = ('{\n"classname" "worldspawn"\n}\n{\n"classname" "light"\n'
                    '"targetname" "A"\n"_light" "1 1 1 1"\n"hammerid" "5"\n}\n').encode()
            header = bytearray(1036)
            header[0:4] = b"VBSP"
            header[4:8] = (20).to_bytes(4, "little")
            header[8:12] = (1036).to_bytes(4, "little")
            header[12:16] = len(text).to_bytes(4, "little")
            bsp.write_bytes(bytes(header) + text)
            directory = Path(temporary) / "fx"
            directory.mkdir()
            fixture = {"name": "fx", "directory": directory,
                       "lighting": {"entities": "entities.json",
                                    "map": {"bsp": str(bsp)}}}
            entities = [{"classname": "light", "targetname": "A", "_light": "1 1 1 1"}]
            lf.write_json(directory / "entities.json", {"entities": entities})
            self.assertEqual(lf.check_map(fixture), [])
            entities[0]["_light"] = "2 2 2 2"
            lf.write_json(directory / "entities.json", {"entities": entities})
            self.assertTrue(any("differs" in p for p in lf.check_map(fixture)))
            entities.append({"classname": "light_spot", "targetname": "B"})
            lf.write_json(directory / "entities.json", {"entities": entities})
            self.assertTrue(any("lacks entity" in p for p in lf.check_map(fixture)))

    def test_manifest_camera_missing_from_fixture(self):
        self.tree.edit("manifest.json", lambda m: m["fixtures"][0]["cameras"].append("side"))
        self.assertRejected("camera side is not in the fixture")


class CheckedInSet(unittest.TestCase):
    def test_checked_in_fixtures_pass(self):
        # The published maps are local build products (run/maps, untracked);
        # `lighting_fixtures.py check` requires them, the data check here not.
        self.assertEqual(lf.check(maps=False), [])

    def test_every_term_has_a_negative_control_and_a_fixture(self):
        manifest = lf.load_manifest()
        for term, entry in manifest["terms"].items():
            self.assertTrue(entry["negative_control"], term)
            self.assertTrue(entry["fixtures"] or entry.get("judged_by"), term)
        counts = {f["name"]: f["lights"] for f in manifest["fixtures"]}
        self.assertEqual(counts["area-room"]["rect"], 64)
        self.assertEqual(counts["foggy-hall"]["point"] + counts["foggy-hall"]["spot"], 256)

    def test_generator_is_reproducible(self):
        toolchain = ROOT / "build/toolchains/pbrt-map-toolchain.json"
        if not toolchain.is_file():
            self.skipTest("no provisioned map toolchain (OpenUSD Python)")
        values = json.loads(toolchain.read_text())
        env = dict(os.environ, PYTHONPATH=values["usd_pythonpath"])
        result = subprocess.run([values["usd_python"], str(ROOT / "tools/quality/"
                                                          "lighting_fixtures.py"),
                                 "generate", "--check"], env=env, capture_output=True,
                                text=True, cwd=ROOT)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)


if __name__ == "__main__":
    unittest.main()
