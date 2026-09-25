"""vrad_cycles: the vrad drop-in that hooks a regular compile into the Cycles
relight. Argument handling, the vrad/relight sequence and its failure paths
run against a fake vrad and a recording relight (no Blender, no game content);
the game-directory material layer and the pipeline's exclusion policy are
checked on their own."""

import os
import stat
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(ROOT / "tools" / "quality"))

import legacy_bsp_relight  # noqa: E402
import legacy_bsp_scene  # noqa: E402
import pbrt_map_build  # noqa: E402
import vrad_cycles  # noqa: E402

# A vrad that logs its arguments and writes <map>.bsp, or fails when asked.
FAKE_VRAD = """#!/bin/sh
printf '%s\\n' "$@" > "$(dirname "$0")/vrad-args.txt"
[ -n "$FAKE_VRAD_FAIL" ] && exit 7
for last; do :; done
case "$last" in -*) exit 0;; esac
[ -n "$FAKE_VRAD_NO_OUTPUT" ] && exit 0
printf 'VBSP' > "${last%.bsp}.bsp"
"""


class ArgumentTests(unittest.TestCase):
    def test_cycles_options_are_removed_and_vrad_order_kept(self):
        options, vrad = vrad_cycles.split_arguments(
            ["-both", "--cycles-quality", "legacy-relight-preview", "-game", "g",
             "--cycles-boot", "--cycles-map-name=room_lit", "-final", "maps/room"])
        self.assertEqual(vrad, ["-both", "-game", "g", "-final", "maps/room"])
        self.assertEqual(options, {"quality": "legacy-relight-preview", "boot": True,
                                   "map_name": "room_lit"})

    def test_bad_options_are_usage_errors(self):
        with self.assertRaises(vrad_cycles.UsageError):
            vrad_cycles.split_arguments(["maps/room", "--cycles-out"])
        with self.assertRaises(vrad_cycles.UsageError):
            vrad_cycles.split_arguments(["--cycles-bake-harder", "maps/room"])

    def test_map_bsp_follows_vrad(self):
        cwd = Path("/work")
        self.assertEqual(vrad_cycles.map_bsp(["-game", "g", "maps/room"], cwd),
                         Path("/work/maps/room.bsp"))
        self.assertEqual(vrad_cycles.map_bsp(["maps/room.bsp"], cwd), Path("/work/maps/room.bsp"))
        # vrad strips any extension (Q_StripExtension) and loads <map>.bsp.
        self.assertEqual(vrad_cycles.map_bsp(["/abs/room.vmf"], cwd), Path("/abs/room.bsp"))
        self.assertEqual(vrad_cycles.map_bsp(["/abs.d/room"], cwd), Path("/abs.d/room.bsp"))
        self.assertIsNone(vrad_cycles.map_bsp([], cwd))
        self.assertIsNone(vrad_cycles.map_bsp(["-help"], cwd))
        self.assertIsNone(vrad_cycles.map_bsp(["-game", "g"], cwd))

    def test_game_directory_like_vrad(self):
        cwd = Path("/work")
        self.assertEqual(vrad_cycles.game_directory(["-game", "g", "m"], {}, cwd),
                         Path("/work/g"))
        self.assertEqual(vrad_cycles.game_directory(["-VPROJECT", "/a", "-game", "/b", "m"],
                                                    {}, cwd), Path("/b"))
        self.assertEqual(vrad_cycles.game_directory(["m"], {"VPROJECT": "/v"}, cwd), Path("/v"))
        self.assertIsNone(vrad_cycles.game_directory(["m"], {}, cwd))

    def test_map_name_must_be_a_pipeline_name(self):
        self.assertEqual(vrad_cycles.default_map_name(Path("x/Room_2.bsp")), "room_2_relit")
        with self.assertRaises(vrad_cycles.UsageError):
            vrad_cycles.default_map_name(Path("x/my-room.bsp"))


class SequenceTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.cwd = Path(self.temp.name)
        (self.cwd / "maps").mkdir()
        self.vrad = self.cwd / "vrad"
        self.vrad.write_text(FAKE_VRAD)
        self.vrad.chmod(self.vrad.stat().st_mode | stat.S_IXUSR)
        self.calls = []
        patches = [
            mock.patch.object(legacy_bsp_relight, "load_toolchain",
                              lambda path=None: {"compile_tools": str(self.cwd)}),
            mock.patch.object(legacy_bsp_relight, "relight", self.record),
            mock.patch.dict(os.environ, {}, clear=False),
        ]
        for patch in patches:
            patch.start()
            self.addCleanup(patch.stop)
        os.environ.pop("FAKE_VRAD_FAIL", None)
        os.environ.pop("FAKE_VRAD_NO_OUTPUT", None)
        self.relight_error = None

    def tearDown(self):
        self.temp.cleanup()

    def record(self, *args):
        self.calls.append(args)
        if self.relight_error:
            raise SystemExit(self.relight_error)
        return {"status": "pass"}

    def main(self, *argv):
        return vrad_cycles.main(list(argv), environ={}, cwd=self.cwd)

    def test_vrad_then_relight_with_the_compile_inputs(self):
        status = self.main("-game", "game", "-final", "--cycles-quality",
                           "legacy-relight-preview", "--cycles-no-publish",
                           "--cycles-out", "build", "--cycles-device", "cpu", "maps/room")
        self.assertEqual(status, 0)
        self.assertEqual((self.cwd / "vrad-args.txt").read_text().split(),
                         ["-game", "game", "-final", "maps/room"])
        (bsp, name, out, _toolchain, quality, game, force_from, boot, keep_going,
         publish, device), = self.calls
        self.assertEqual((bsp, name, out), (self.cwd / "maps/room.bsp", "room_relit",
                                            self.cwd / "build"))
        self.assertEqual((quality, game, force_from, boot, keep_going, publish, device),
                         ("legacy-relight-preview", self.cwd / "game", None, False, False,
                          False, "cpu"))

    def test_defaults_publish_the_full_relight(self):
        self.assertEqual(self.main("maps/room"), 0)
        (_bsp, name, out, _t, quality, game, _f, _b, _k, publish, device), = self.calls
        self.assertEqual((name, quality, game, publish, device),
                         ("room_relit", pbrt_map_build.LEGACY_QUALITY, None, True, None))
        self.assertEqual(out, legacy_bsp_relight.default_out("room_relit"))

    def test_vrad_failure_stops_before_the_relight(self):
        os.environ["FAKE_VRAD_FAIL"] = "1"
        self.assertEqual(self.main("maps/room"), 7)
        self.assertEqual(self.calls, [])

    def test_no_map_runs_vrad_only(self):
        self.assertEqual(self.main("-help"), 0)
        self.assertEqual(self.calls, [])

    def test_missing_bsp_after_vrad_fails(self):
        os.environ["FAKE_VRAD_NO_OUTPUT"] = "1"
        self.assertEqual(self.main("maps/room"), 1)
        self.assertEqual(self.calls, [])

    def test_relight_failure_is_nonzero_and_keeps_vrads_bsp(self):
        self.relight_error = "step bake failed"
        self.assertEqual(self.main("maps/room"), 1)
        self.assertEqual((self.cwd / "maps/room.bsp").read_bytes(), b"VBSP")

    def test_usage_errors_run_nothing(self):
        self.assertEqual(self.main("--cycles-from", "nonsense", "maps/room"), 2)
        self.assertEqual(self.main("--cycles-device", "optix", "maps/room"), 2)
        self.assertEqual(self.main("maps/my-room"), 2)
        self.assertFalse((self.cwd / "vrad-args.txt").exists())
        self.assertEqual(self.calls, [])


class GameDirectoryTests(unittest.TestCase):
    def test_game_materials_first_ignoring_case_then_content(self):
        class Content:
            def read(self, relative):
                return (b"runtime", "vpk:" + relative) if relative.endswith(".vtf") else \
                    (None, None)

        with tempfile.TemporaryDirectory() as temp:
            folder = Path(temp) / "materials" / "Dev"
            folder.mkdir(parents=True)
            (folder / "Wall.VMT").write_bytes(b"LightmappedGeneric {}")
            (Path(temp) / "notes.vmt").write_bytes(b"not a material path")
            game = legacy_bsp_scene.GameDirectory(temp, Content())
            self.assertEqual(game.read("materials/dev/wall.vmt"),
                             (b"LightmappedGeneric {}", "game:materials/dev/wall.vmt"))
            self.assertEqual(game.read("materials\\DEV\\wall.vmt")[0], b"LightmappedGeneric {}")
            self.assertEqual(game.read("materials/dev/wall.vtf"),
                             (b"runtime", "vpk:materials/dev/wall.vtf"))
            self.assertEqual(game.read("notes.vmt"), (None, None))


class ExclusionPolicyTests(unittest.TestCase):
    def test_profile_exclusions_apply_only_to_scenes_with_them(self):
        scene = {"wall": {}, "relight_occluder": {}}
        self.assertEqual(pbrt_map_build.applicable_exclusions(["relight_occluder"], set(), scene),
                         ["relight_occluder"])
        self.assertEqual(pbrt_map_build.applicable_exclusions(
            ["relight_occluder"], set(), {"wall": {}}), [])

    def test_manifest_exclusions_stay_for_the_bake_to_reject(self):
        self.assertEqual(pbrt_map_build.applicable_exclusions(
            ["glas"], {"glas"}, {"glass": {}}), ["glas"])


class RelightManifestTests(unittest.TestCase):
    def test_relight_writes_game_and_device_into_the_manifest(self):
        with tempfile.TemporaryDirectory() as temp:
            bsp = Path(temp) / "room.bsp"
            bsp.write_bytes(b"VBSP")

            class Stop(Exception):
                pass

            def stop(manifest, *args, **kwargs):
                self.manifest = manifest
                raise Stop()

            with mock.patch.object(pbrt_map_build, "Pipeline", stop):
                with self.assertRaises(Stop):
                    legacy_bsp_relight.relight(bsp, "room_relit", Path(temp) / "out", {},
                                               game=temp, device="cpu")
            self.assertEqual(self.manifest["legacy_game"], str(Path(temp).resolve()))
            self.assertEqual(self.manifest["lightmap"], {"device": "cpu"})
            self.assertEqual(self.manifest["quality"], pbrt_map_build.LEGACY_QUALITY)


class ManifestTests(unittest.TestCase):
    def test_legacy_game_must_be_a_directory(self):
        with tempfile.TemporaryDirectory() as temp:
            bsp = Path(temp) / "room.bsp"
            bsp.write_bytes(b"VBSP")
            manifest = Path(temp) / "manifest.json"
            manifest.write_text('{"schema": "pbrt-map-manifest/v1", "map": "room_relit", '
                                '"legacy_bsp": "%s", "legacy_game": "%s"}' % (bsp, temp))
            self.assertEqual(pbrt_map_build.load_manifest(manifest)["legacy_game"],
                             str(Path(temp).resolve()))
            manifest.write_text('{"schema": "pbrt-map-manifest/v1", "map": "room_relit", '
                                '"legacy_bsp": "%s", "legacy_game": "%s"}' % (bsp, bsp))
            with self.assertRaises(ValueError):
                pbrt_map_build.load_manifest(manifest)


if __name__ == "__main__":
    unittest.main()
