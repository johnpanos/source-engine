"""Host-side tests of rdc.py: the private qrenderdoc config, the headless
environment, and the request/response protocol against fake qrenderdoc
executables (answering, failing, silent and hung). The replay worker itself
needs RenderDoc and a capture; see tools/renderdoc/README.md."""

import json
import os
import stat
import sys
import tempfile
import textwrap
import unittest
from pathlib import Path
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import rdc  # noqa: E402

FAKE = textwrap.dedent("""\
    #!{python}
    import json, os, sys, time
    mode = os.environ["FAKE_MODE"]
    request = json.load(open(os.environ["RDC_REQUEST"]))
    config = os.path.join(os.environ["XDG_DATA_HOME"], "qrenderdoc", "UI.config")
    seen = {{"request": request, "argv": sys.argv[1:],
             "display": os.environ.get("DISPLAY"), "wayland": os.environ.get("WAYLAND_DISPLAY"),
             "qpa": os.environ.get("QT_QPA_PLATFORM"), "config": json.load(open(config))}}
    if mode == "answer":
        json.dump({{"result": seen}}, open(os.environ["RDC_RESPONSE"], "w"))
    elif mode == "error":
        json.dump({{"error": "Traceback: boom"}}, open(os.environ["RDC_RESPONSE"], "w"))
    elif mode == "silent":
        print("no response written")
        sys.exit(3)
    elif mode == "hang":
        time.sleep(60)
    """)


class RdcHostTest(unittest.TestCase):
    def setUp(self):
        self.scratch = tempfile.TemporaryDirectory()
        root = Path(self.scratch.name)
        self.fake = root / "qrenderdoc"
        self.fake.write_text(FAKE.format(python=sys.executable))
        self.fake.chmod(self.fake.stat().st_mode | stat.S_IEXEC)
        self.capture = root / "frame.rdc"
        self.capture.write_bytes(b"RDOC")
        patcher = mock.patch.dict(os.environ, {"XDG_CACHE_HOME": str(root / "cache"),
                                               "DISPLAY": ":0", "WAYLAND_DISPLAY": "wayland-0"})
        patcher.start()
        self.addCleanup(patcher.stop)
        self.addCleanup(self.scratch.cleanup)

    def run_fake(self, mode, timeout=30):
        with mock.patch.dict(os.environ, {"FAKE_MODE": mode}):
            return rdc.replay("draws", self.capture, {"bindings": True}, timeout=timeout,
                              qrenderdoc=str(self.fake))

    def test_answer_is_returned_with_request_and_headless_environment(self):
        seen = self.run_fake("answer")
        self.assertEqual(seen["request"], {"command": "draws", "capture": str(self.capture),
                                           "args": {"bindings": True}})
        self.assertEqual(seen["argv"], ["--python", str(rdc.WORKER)])
        self.assertIsNone(seen["display"])
        self.assertIsNone(seen["wayland"])
        self.assertEqual(seen["qpa"], "offscreen")
        # The config qrenderdoc reads suppresses the first-run analytics dialog.
        self.assertEqual(seen["config"]["rdocConfigData"], 1)
        self.assertTrue(seen["config"]["Analytics_TotalOptOut"])

    def test_worker_error_raises(self):
        with self.assertRaisesRegex(rdc.RdcError, "boom"):
            self.run_fake("error")

    def test_missing_response_raises(self):
        with self.assertRaisesRegex(rdc.RdcError, "without a response"):
            self.run_fake("silent")

    def test_hung_replay_times_out(self):
        with self.assertRaisesRegex(rdc.RdcError, "did not answer within 1s"):
            self.run_fake("hang", timeout=1)

    def test_missing_capture_and_missing_qrenderdoc(self):
        with self.assertRaisesRegex(rdc.RdcError, "no capture"):
            rdc.replay("info", self.capture.with_name("absent.rdc"), qrenderdoc=str(self.fake))
        with mock.patch("shutil.which", return_value=None):
            with self.assertRaisesRegex(rdc.RdcError, "qrenderdoc is not installed"):
                rdc.replay("info", self.capture)

    def test_private_config_keeps_other_settings_and_repairs_magic(self):
        home = rdc.private_home()
        config = home / "qrenderdoc" / "UI.config"
        config.write_text(json.dumps({"Font_PreferMonospaced": True,
                                      "Analytics_TotalOptOut": False}))
        rdc.private_home()
        settled = json.loads(config.read_text())
        self.assertTrue(settled["Font_PreferMonospaced"])
        self.assertTrue(settled["Analytics_TotalOptOut"])
        self.assertEqual(settled["rdocConfigData"], 1)
        self.assertTrue(str(home).startswith(os.environ["XDG_CACHE_HOME"]))

    def test_worker_is_valid_python(self):
        compile(rdc.WORKER.read_text(), str(rdc.WORKER), "exec")

    def test_capture_requires_out(self):
        with self.assertRaises(SystemExit):
            with mock.patch("sys.stderr"):
                rdc.main(["capture", "--map", "gi_door"])


if __name__ == "__main__":
    unittest.main()
