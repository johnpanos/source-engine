#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================

import array
import importlib.util
import json
from pathlib import Path
import re
import subprocess
import tempfile
import threading
import unittest
from unittest.mock import patch

from PIL import Image


ROOT = Path(__file__).resolve().parents[3]
SPEC = importlib.util.spec_from_file_location("map_swipe", ROOT / "tools/render/map_swipe_compare.py")
swipe = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(swipe)


class MapSwipeTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.build = self.root / "build"
        (self.build / "c4che").mkdir(parents=True)
        (self.build / "c4che/_cache.py").write_text("RENDER_FSR411 = True\nGAMES = 'portal2'\n")
        self.out = self.root / "captures"
        self.calls = []
        self.barrier = threading.Barrier(2)
        self.defect = None

    def boot(self, command, **kwargs):
        self.calls.append(command)
        self.barrier.wait(timeout=5)  # A sequential host launch must fail this fixture.
        out = Path(command[command.index("--out") + 1])
        out.mkdir()
        lines = [command[i + 1] for i, token in enumerate(command[:-1])
                 if token == "--console-command"]
        frames = []
        for line in lines:
            if not line.startswith("alias vo_shot"):
                continue
            position = [float(n) for n in re.search(r"cmd setpos ([^;]+)", line)[1].split()]
            position[2] += 64
            angles = [float(n) for n in re.search(r"cmd setang ([^;]+)", line)[1].split()]
            name = re.search(r"screenshot (swipe_[\w-]+)", line)[1]
            frames.append((name, position, angles))
        # portal_boot appends its closing screenshot after the scripted sequence.
        frames.append(("closing", frames[-1][1], frames[-1][2]))
        screenshots, oracles = [], []
        for index, (name, position, angles) in enumerate(frames):
            image = out / (name + ".tga")
            extent = (32, 64) if self.defect == "extent" and index == 0 else (64, 64)
            Image.new("RGB", extent, (index + 1, 10, 20)).save(image)
            if self.defect == "name" and index == 0:
                image = image.rename(out / "wrong.tga")
            screenshots.append({"path": str(image)})
            if self.defect == "camera" and index == 0:
                position = [position[0] + 1, *position[1:]]
            view = {"type": "3d", "stack": 1, "target": "backbuffer", "origin": position,
                    "angles": angles, "fov": 90, "viewport": [0, 0, 64, 64]}
            hdr = out.name == "b-boot" and name != "closing"
            if hdr:
                directory = out / "runtime/portal2/screenshots"
                directory.mkdir(parents=True, exist_ok=True)
                path = directory / (name + "_hdr.pfm")
                with path.open("wb") as file:
                    file.write(b"PF\n64 64\n-1.0\n")
                    array.array("f", [4.0] * (64 * 64 * 3)).tofile(file)
                metadata = {"schema": "source-hdr-capture/v1", "encoding": "linear",
                            "primaries": "Rec.709", "stage": "scene-before-output",
                            "width": 64, "height": 64, "rgb_max": 4.0,
                            "exposure": 3, "display_peak_nits": 10000,
                            "reference_white_nits": 203,
                            "hdr_output_active": self.defect != "hdr-inactive",
                            "output_pass_recorded": self.defect != "output-pass"}
                Path(str(path) + ".json").write_text(json.dumps(metadata))
                if self.defect == "hdr":
                    path.write_bytes(b"PF\n64 64\n-1.0\n")
            for _ in range(2 if hdr else 1):
                oracle = out / ("vulkan-native-%d.jsonl" % len(oracles))
                oracle.write_text(json.dumps({"schema": "source-view-oracle/v1"}) + "\n" +
                                  json.dumps({"event": "label_begin",
                                              "name": "vieworacle " + json.dumps(view)}) + "\n")
                oracles.append({"path": str(oracle)})
        if self.defect == "count":
            oracles.pop()
        evidence = {"status": "fail" if self.defect == "boot" else "pass",
                    "screenshots": list(reversed(screenshots)),
                    "view_oracle_captures": list(reversed(oracles)), "failures": []}
        (out / "evidence.json").write_text(json.dumps(evidence))
        log = "[NativeVulkan] HDR output: Rec. 2020 / PQ (10-bit)\n"
        log += swipe.FIZZLER_BEGIN + "\n  StartDisabled: %d\n" % (
            0 if self.defect == "fizzler" else 1) + swipe.FIZZLER_END + "\n"
        if self.defect != "fsr":
            log += "FSR game: 64x64 -> 64x64, before post/HUD\n"
        (out / "stdout.log").write_text(log)
        return subprocess.CompletedProcess(command, 0, "", "")

    def run_gallery(self):
        with patch.object(swipe.subprocess, "run", self.boot):
            return swipe.main(["--all-captures", "--in-compositor", "--width", "64",
                               "--height", "64", "--build", str(self.build),
                               "--content-root-a", str(self.root),
                               "--content-root-b", str(self.root), "--out", str(self.out)])

    def test_two_parallel_hosts_capture_every_selected_pose(self):
        self.assertEqual(self.run_gallery(), 0)
        self.assertEqual(len(self.calls), 2)
        for command in self.calls:
            self.assertEqual(command[command.index("--capture-wait") + 1], "10")
            lines = [command[i + 1] for i, token in enumerate(command[:-1])
                     if token == "--console-command"]
            self.assertLess(lines.index("wait 134"), next(
                i for i, line in enumerate(lines) if line.startswith("alias vo_shot")))
        self.assertFalse((self.out / "panel").exists())
        self.assertEqual(len(list(self.out.glob("*/comparison.json"))), 19)
        self.assertEqual((self.out / "index.html").read_text().count('<a href='), 19)
        for name, _, _ in swipe.INTRO4_CAPTURES:
            receipt = json.loads((self.out / name / "comparison.json").read_text())
            self.assertIn("-norendercore", receipt["left"]["engine_args"])
            self.assertIn("r_core_world 0", receipt["left"]["startup_commands"])
            self.assertNotIn("-fsr", receipt["right"]["engine_args"])
            self.assertIn("r_temporal_scale 0", receipt["right"]["startup_commands"])
            self.assertIn("mat_hdr_exposure 3", receipt["right"]["startup_commands"])
            self.assertEqual(receipt["metrics"]["size"], [64, 64])
            self.assertEqual(receipt["right"]["hdr_export"]["raw_rgb_peak"], 4.0)
            self.assertIn("mat_hdr_peak_nits 10000", receipt["right"]["startup_commands"])
            self.assertTrue(receipt["right"]["fizzler_confirmation"]["disabled"])

    def test_bad_capture_never_publishes_gallery(self):
        for defect in ("count", "name", "camera", "extent", "boot", "hdr", "fizzler",
                       "hdr-inactive", "output-pass"):
            with self.subTest(defect=defect):
                self.defect = defect
                self.out = self.root / defect
                self.assertEqual(self.run_gallery(), 1)
                self.assertFalse((self.out / "index.html").exists())


if __name__ == "__main__":
    unittest.main()
