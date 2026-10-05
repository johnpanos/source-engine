"""Step command runner of pbrt_map_build: output capture and the silence stop.

A GPU fault can leave a Cycles bake waiting forever on a lost HIP queue; the
pipeline must stop such a step instead of waiting. These run real child
processes (sh and sleep); no Blender or GPU.
"""

import io
import sys
import time
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import pbrt_map_build  # noqa: E402


class RunLoggedTest(unittest.TestCase):

    def test_output_and_exit_status_pass_through(self):
        handle, seen = io.StringIO(), []
        status, quiet = pbrt_map_build.run_logged(
            ["sh", "-c", "echo one; echo two; exit 3"], handle, on_line=seen.append, silence=10)
        self.assertEqual((status, quiet), (3, None))
        self.assertEqual(seen, ["one\n", "two\n"])
        self.assertEqual(handle.getvalue(), "one\ntwo\n")

    def test_steady_output_is_not_stopped(self):
        """Each line comes sooner than the limit, the whole run takes longer."""
        started = time.monotonic()
        status, quiet = pbrt_map_build.run_logged(
            ["sh", "-c", "for i in 1 2 3 4 5 6; do echo $i; sleep 0.3; done"], io.StringIO(),
            silence=1)
        self.assertEqual((status, quiet), (0, None))
        self.assertGreater(time.monotonic() - started, 1.5)

    def test_silent_command_and_its_children_are_stopped(self):
        handle, seen = io.StringIO(), []
        started = time.monotonic()
        status, quiet = pbrt_map_build.run_logged(
            ["sh", "-c", "sleep 60 & echo $!; wait"], handle, on_line=seen.append, silence=1)
        self.assertEqual((status, quiet), (None, 1))
        self.assertLess(time.monotonic() - started, 10)
        child = int(seen[0])
        # The background child is in the killed process group too.
        deadline = time.monotonic() + 5
        while Path("/proc/%d" % child).exists() and time.monotonic() < deadline:
            time.sleep(0.05)
        alive = Path("/proc/%d" % child).exists() and \
            Path("/proc/%d/stat" % child).read_text().split()[2] != "Z"
        self.assertFalse(alive)
        self.assertIn("no output for 1 s", handle.getvalue())

    def test_no_limit_waits(self):
        status, quiet = pbrt_map_build.run_logged(["sh", "-c", "sleep 1.2; echo done"],
                                                  io.StringIO())
        self.assertEqual((status, quiet), (0, None))


if __name__ == "__main__":
    unittest.main()


class StepSilenceTest(unittest.TestCase):
    """No step may wait forever: a command that stops printing is stopped."""

    def test_a_silent_command_is_stopped_by_default(self):
        handle, seen = io.StringIO(), []
        real, pbrt_map_build.DEFAULT_STEP_SILENCE = pbrt_map_build.DEFAULT_STEP_SILENCE, 1
        try:
            status, quiet = pbrt_map_build.run_logged(["sh", "-c", "sleep 30"], handle,
                                                      on_line=seen.append)
        finally:
            pbrt_map_build.DEFAULT_STEP_SILENCE = real
        self.assertEqual((status, quiet), (None, 1))
        self.assertIn("stopped: no output for 1 s", handle.getvalue())

    def test_the_default_applies_when_a_step_asks_for_no_bound(self):
        """A step that passes silence=None is bounded, not left waiting."""
        self.assertEqual(pbrt_map_build.DEFAULT_STEP_SILENCE, 10 * 60)
        handle = io.StringIO()
        real, pbrt_map_build.DEFAULT_STEP_SILENCE = pbrt_map_build.DEFAULT_STEP_SILENCE, 1
        try:
            status, quiet = pbrt_map_build.run_logged(["sh", "-c", "sleep 30"], handle,
                                                      silence=None)
        finally:
            pbrt_map_build.DEFAULT_STEP_SILENCE = real
        self.assertEqual((status, quiet), (None, 1))


class PreviewProfileTest(unittest.TestCase):
    """The one production profile, and the declared preview rung beside it."""

    def test_only_the_production_profile_loads_by_default(self):
        self.assertEqual(pbrt_map_build.load_profile("source2")["name"], "source2")
        for name in ("portal2-chamber-preview", "legacy-relight-preview", "portal2-chamber"):
            with self.assertRaises(ValueError) as raised:
                pbrt_map_build.load_profile(name)
            self.assertIn("--preview", str(raised.exception))

    def test_a_fixture_profile_is_not_a_preview_rung(self):
        # fixture profiles load for their own lanes; they are not a cheap export
        self.assertEqual(pbrt_map_build.load_profile("gi-fixture")["purpose"], "fixture")
        self.assertNotIn("gi-fixture", pbrt_map_build.PREVIEW_PROFILES.values())

    def test_the_preview_rung_is_a_declared_profile_of_its_own(self):
        for production, preview in pbrt_map_build.PREVIEW_PROFILES.items():
            self.assertEqual(pbrt_map_build.load_profile(production)["name"], production)
            rung = pbrt_map_build.load_profile(preview, preview=True)
            self.assertEqual(rung["name"], preview)
            # a preview run is cheaper than the profile it stands in for
            self.assertLess(rung["lightmap"]["samples"],
                            pbrt_map_build.load_profile(production)["lightmap"]["samples"])

    def test_a_preview_rung_bounds_its_walkable_sample_grid(self):
        """Placement's coverage passes cost O(walkable samples) serial rays each;
        a whole retail map reached 23,594 samples and 259 s, so every rung that
        places probes automatically caps the grid (the release profiles' 2000)."""
        for name in pbrt_map_build.PREVIEW_PROFILES.values():
            profile = pbrt_map_build.load_profile(name, preview=True)
            placement = (profile.get("reflection_probe") or {}).get("placement") or {}
            self.assertEqual(placement.get("max_walkable"), 2000, name)

    def test_a_preview_rung_does_not_bake_at_a_release_resolution(self):
        """Valve's own rungs: the default Full Compile bakes 1k lightmaps and
        Final Compile 2k (Source 2 level-design docs, Building Lighting); their
        in-Hammer 'Preview Baked Lighting' bakes no lightmaps at all. A preview
        rung therefore stays at or below 1k, whatever its sample count."""
        for production, preview in pbrt_map_build.PREVIEW_PROFILES.items():
            rung = pbrt_map_build.load_profile(preview, preview=True)
            self.assertLessEqual(rung["lightmap"]["size"], 1024, preview)
            self.assertLess(rung["lightmap"]["size"],
                            pbrt_map_build.load_profile(production)["lightmap"]["size"], preview)

    def test_the_ladder_has_valve_three_rungs_and_rises(self):
        """Source 2 level-design docs, Building Lighting: 'Preview Baked
        Lighting', then Full Compile (1k lightmaps), then Final Compile (2k).
        Our cheapest rung is a small real bake, since Valve's own preview bakes
        no lightmaps at all; the release rung is denser than Valve's 2k."""
        self.assertEqual(list(pbrt_map_build.QUALITY_LADDER),
                         ["preview", "full-compile", "final-compile"])
        self.assertEqual(pbrt_map_build.QUALITY_LADDER["final-compile"],
                         pbrt_map_build.DEFAULT_QUALITY)
        cost = []
        for rung, name in pbrt_map_build.QUALITY_LADDER.items():
            profile = pbrt_map_build.load_profile(name, rung != "final-compile")
            lightmap = profile["lightmap"]
            cost.append((lightmap["size"], lightmap["samples"],
                         (profile.get("probe_volume") or {}).get("spacing_m", 0),
                         (profile.get("reflection_probe") or {}).get("samples", 0)))
        self.assertEqual([c[0] for c in cost], sorted(c[0] for c in cost))
        self.assertEqual([c[1] for c in cost], sorted(c[1] for c in cost))
        # the two cheaper rungs carry no reflection probes' full sample count
        self.assertEqual([c[3] for c in cost], [0, 64, 1024])
        # Valve's Full Compile is 1k; nothing below it exceeds that
        self.assertLessEqual(cost[1][0], 1024)

    def test_a_maps_seam_gate_and_device_share_the_lightmap_block(self):
        """A front end's `extra` may carry this map's seam gate while --device
        still names the Cycles device; the block is merged, not replaced."""
        import map_lighting
        manifest = map_lighting.manifest_for("/maps/room.bsp", "room", device="gpu",
                                             extra={"lightmap": {"seam_gate": {"p99": 0.004}}})
        self.assertEqual(manifest["lightmap"]["device"], "gpu")
        self.assertEqual(manifest["lightmap"]["seam_gate"], {"p99": 0.004})
        plain = map_lighting.manifest_for("/maps/room.bsp", "room", device="gpu")
        self.assertEqual(plain["lightmap"], {"device": "gpu"})
        self.assertNotIn("lightmap", map_lighting.manifest_for("/maps/room.bsp", "room"))

    def test_a_preview_manifest_names_the_rung_and_says_it_is_not_production(self):
        import map_lighting
        manifest = map_lighting.manifest_for("/maps/room.bsp", "room", preview="full-compile")
        self.assertTrue(manifest["preview"])
        self.assertEqual(manifest["quality"], pbrt_map_build.QUALITY_LADDER["full-compile"])
        for rung in ("preview", "full-compile"):
            self.assertTrue(map_lighting.manifest_for("/maps/room.bsp", "room",
                                                      preview=rung)["preview"], rung)
        # the top rung is the production profile, so it is not stamped a preview
        top = map_lighting.manifest_for("/maps/room.bsp", "room", preview="final-compile")
        self.assertNotIn("preview", top)
        self.assertEqual(top["quality"], pbrt_map_build.DEFAULT_QUALITY)
        with self.assertRaises(KeyError):
            map_lighting.manifest_for("/maps/room.bsp", "room", preview="final-quality")
        production = map_lighting.manifest_for("/maps/room.bsp", "room")
        self.assertNotIn("preview", production)
        self.assertEqual(production["quality"], "source2")
