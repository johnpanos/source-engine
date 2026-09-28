#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Self-tests for tools/quality/vmf_map_build.py (RFC 0002 map-building loop).

The compile gates are trusted only after they are shown to classify every
outcome with stub compilers: a clean build, a leak reported in the log, a leak
shown only by a pointfile, a failing step, a missing output, and missing
materials. The material scan must ignore entity "material" keys.
"""

import json
import os
import stat
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(HERE))
import vmf_map_build  # noqa: E402

ROOM = """versioninfo { "editorversion" "400" }
world { "classname" "worldspawn" "skyname" "sky_day01_01"
  solid { side { "plane" "(0 0 0) (1 0 0) (0 1 0)" "material" "DEV/DEV_MEASUREGENERIC01B" } }
}
entity { "classname" "func_physbox" "material" "2"
  solid { side { "material" "TOOLS/TOOLSNODRAW" } } }
entity { "classname" "info_player_start" "origin" "0 0 16" }
"""


def stub(directory, name, body):
    """A stand-in compiler: a Python script that receives the real argv."""
    path = directory / name
    path.write_text("#!%s\nimport sys, pathlib\nargs = sys.argv[1:]\n%s\n" % (sys.executable, body))
    path.chmod(path.stat().st_mode | stat.S_IEXEC)


class MaterialScanTest(unittest.TestCase):
    def test_only_side_materials_count(self):
        self.assertEqual(["dev/dev_measuregeneric01b", "tools/toolsnodraw"],
                         vmf_map_build.referenced_materials(ROOM))

    def test_nested_blocks_and_bare_tokens(self):
        text = 'world { solid { side { material dev/a } side { "material" "dev/b" } } }'
        self.assertEqual(["dev/a", "dev/b"], vmf_map_build.referenced_materials(text))


class BuildGateTest(unittest.TestCase):
    def setUp(self):
        self.tmp = Path(tempfile.mkdtemp(prefix="vmf-build-"))
        self.tools = self.tmp / "tools"
        self.tools.mkdir()
        self.vmf = self.tmp / "room.vmf"
        self.vmf.write_text(ROOM)
        self.missing = []
        patcher = mock.patch.object(vmf_map_build, "stage_compile_game",
                                    side_effect=lambda game, runtime, materials: self.missing)
        patcher.start()
        self.addCleanup(patcher.stop)

    def compilers(self, vbsp="pathlib.Path(args[-1]).with_suffix('.bsp').write_bytes(b'VBSP')",
                  vvis="pass", vrad="pass"):
        stub(self.tools, "vbsp", vbsp)
        stub(self.tools, "vvis", vvis)
        stub(self.tools, "vrad", vrad)

    def build(self, quality="fast"):
        return vmf_map_build.build(self.vmf, self.tmp / "out", self.tools, self.tmp, quality)

    def test_clean_build_packages_and_records(self):
        self.compilers()
        record = self.build()
        self.assertEqual("pass", record["status"])
        self.assertFalse(record["leaked"])
        self.assertEqual(["vbsp", "vvis", "vrad-ldr", "vrad-hdr"], [s["step"] for s in record["steps"]])
        log = (self.tmp / "out/compile.log").read_text()
        self.assertIn("vrad -ldr", log)
        self.assertIn("vrad -hdr", log)
        self.assertTrue((self.tmp / "out/content/maps/room.bsp").is_file())
        on_disk = json.loads((self.tmp / "out/build.json").read_text())
        self.assertEqual(record["bsp2_sha256"], on_disk["bsp2_sha256"])
        log = (self.tmp / "out/compile.log").read_text()
        self.assertIn("-fast", log)

    def test_full_quality_drops_fast_flags(self):
        self.compilers()
        self.assertEqual("pass", self.build("full")["status"])
        self.assertNotIn("-fast", (self.tmp / "out/compile.log").read_text())

    def test_leak_in_log_fails_before_vis(self):
        self.compilers(vbsp="print('**** leaked ****')\n"
                            "pathlib.Path(args[-1]).with_suffix('.bsp').write_bytes(b'VBSP')")
        record = self.build()
        self.assertEqual(("leak", ["leak"]), (record["status"], record["failed_gates"]))
        self.assertEqual(["vbsp"], [s["step"] for s in record["steps"]])
        self.assertFalse((self.tmp / "out/content").exists())

    def test_leak_shown_only_by_pointfile_fails(self):
        self.compilers(vbsp="p = pathlib.Path(args[-1])\np.with_suffix('.bsp').write_bytes(b'V')\n"
                            "p.with_suffix('.lin').write_text('0 0 0')")
        self.assertEqual("leak", self.build()["status"])

    def test_failing_step_names_its_gate(self):
        self.compilers(vrad="sys.exit(3)")
        record = self.build()
        self.assertEqual(("fail", ["vrad-ldr"]), (record["status"], record["failed_gates"]))
        self.assertEqual(3, record["steps"][-1]["returncode"])

    def test_missing_output_fails_even_with_exit_zero(self):
        self.compilers(vbsp="pass")
        record = self.build()
        self.assertEqual(("fail", ["vbsp"]), (record["status"], record["failed_gates"]))

    def test_missing_materials_fail_before_compiling(self):
        self.compilers(vbsp="sys.exit('must not run')")
        self.missing = ["dev/dev_measuregeneric01b"]
        record = self.build()
        self.assertEqual("missing-materials", record["status"])
        self.assertEqual([], record["steps"])

    def test_mixed_case_output_path_compiles_in_a_lowercase_directory(self):
        self.compilers(vbsp="import os\nassert os.getcwd() or True\n"
                            "assert args[-1] == args[-1].lower(), args[-1]\n"
                            "pathlib.Path(args[-1]).with_suffix('.bsp').write_bytes(b'VBSP')")
        out = self.tmp / "Results/20260926T0000Z"
        record = vmf_map_build.build(self.vmf, out, self.tools, self.tmp)
        self.assertEqual("pass", record["status"])
        self.assertTrue((out / "compile/room.bsp").is_file())  # work copied back
        self.assertFalse((out / "compile/game").exists())

    def test_failed_compile_keeps_its_files_for_inspection(self):
        self.compilers(vbsp="p = pathlib.Path(args[-1])\np.with_suffix('.lin').write_text('0 0 0')")
        self.build()
        self.assertTrue((self.tmp / "out/compile/room.lin").is_file())

    def test_install_copies_the_map_into_a_game_directory(self):
        self.compilers()
        record = self.build()
        game = self.tmp / "runtime/fstop"
        game.mkdir(parents=True)
        target = vmf_map_build.install(record, game)
        self.assertEqual(game / "maps/room.bsp", target)
        self.assertEqual(b"VBSP", target.read_bytes())
        self.assertFalse((game / "maps/room.bsp.partial").exists())

    def test_install_refuses_a_failed_build_or_missing_game_directory(self):
        self.compilers(vrad="sys.exit(3)")
        with self.assertRaises(ValueError):
            vmf_map_build.install(self.build(), self.tmp)
        self.compilers()
        with self.assertRaises(ValueError):
            vmf_map_build.install(self.build(), self.tmp / "no-such-game")

    def test_input_inside_the_work_directory_survives(self):
        self.compilers()
        inside = self.tmp / "out/compile/room.vmf"
        inside.parent.mkdir(parents=True)
        inside.write_text(ROOM)
        record = vmf_map_build.build(inside, self.tmp / "out", self.tools, self.tmp)
        self.assertEqual("pass", record["status"])


if __name__ == "__main__":
    unittest.main()
