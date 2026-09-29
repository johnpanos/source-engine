import json
import sys
import tempfile
import types
import unittest
from pathlib import Path
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import remote_blender  # noqa: E402
import vast_blender  # noqa: E402

OFFER = {"id": 7, "gpu_name": "RTX 3090", "dph_total": 0.25, "storage_total_cost": 0.05,
         "inet_up_cost": 0.01, "inet_down_cost": 0.02}


class FakeVast:
    def __init__(self):
        self.destroyed = []

    def destroy(self, instance_id):
        self.destroyed.append(instance_id)

    def instance(self, instance_id):
        return None if instance_id in self.destroyed else {"actual_status": "loading"}


class VastBlenderTest(unittest.TestCase):
    def setUp(self):
        self.tmp = Path(tempfile.mkdtemp())
        self.ledger = vast_blender.Ledger(self.tmp / "ledger.json")

    def test_key_from_env_file_and_never_in_errors(self):
        key_file = self.tmp / "vast.env"
        key_file.write_text("# comment\nexport API_KEY=\"sekrit123\"\n")
        with mock.patch.object(vast_blender, "KEY_FILE", key_file), \
                mock.patch.dict("os.environ", {}, clear=False) as env:
            env.pop("VAST_API_KEY", None)
            self.assertEqual(vast_blender.api_key(), "sekrit123")
        error = vast_blender.VastError("GET", "/users/current/", 401, "denied")
        self.assertNotIn("sekrit123", str(error))

    def test_budget_refuses_a_rental_that_could_cross_the_cap(self):
        with self.ledger.open() as data:
            data["baseline_credit"] = 10.0
        data = self.ledger.read()
        hours, worst, problem = vast_blender.plan_budget(data, 10.0, OFFER, 1.0, 5.0)
        self.assertIsNone(problem)
        self.assertAlmostEqual(worst, 0.30 + 5 * 0.02)
        # $9.80 billed already: a $0.40 worst case does not fit under $10.
        hours, worst, problem = vast_blender.plan_budget(data, 0.20, OFFER, 1.0, 5.0)
        self.assertIsNone(hours)
        self.assertIn("exceeds the $10.00 cap", problem)
        # Nor does a long rental on an untouched account.
        self.assertIsNotNone(vast_blender.plan_budget(data, 10.0, OFFER, 40.0, 5.0)[2])

    def test_spend_is_the_larger_of_estimate_and_billed(self):
        data = {"cap_usd": 10.0, "baseline_credit": 10.0, "instances": {
            "1": {"rate_usd_h": 0.5, "created": 0.0, "destroyed": 7200.0, "transfer_usd": 0.1}}}
        total, estimate, billed = vast_blender.spend(data, credit=9.5, now=9999.0)
        self.assertAlmostEqual(estimate, 1.1)
        self.assertAlmostEqual(billed, 0.5)
        self.assertAlmostEqual(total, 1.1)
        self.assertAlmostEqual(vast_blender.spend(data, credit=7.0)[0], 3.0)
        # Credit added later never makes spend negative.
        self.assertAlmostEqual(vast_blender.spend(data, credit=50.0, now=1.0)[0], 1.1)
        # A live instance accrues until now.
        data["instances"]["1"].pop("destroyed")
        self.assertAlmostEqual(vast_blender.estimated_spend(data, now=3600.0), 0.6)

    def test_ssh_endpoint_prefers_direct(self):
        row = {"public_ipaddr": "1.2.3.4\n", "ports": {"22/tcp": [{"HostPort": "40022"}]},
               "ssh_host": "ssh5.vast.ai", "ssh_port": 12345}
        self.assertEqual(vast_blender.ssh_endpoint(row), ("1.2.3.4", 40022))
        row["ports"] = {}
        self.assertEqual(vast_blender.ssh_endpoint(row), ("ssh5.vast.ai", 12345))
        self.assertIsNone(vast_blender.ssh_endpoint({}))

    def test_toolchain_block_drives_remote_blender(self):
        base = self.tmp / "base.json"
        base.write_text(json.dumps({"schema": "pbrt-map-toolchain/v1", "blender": "blender"}))
        ssh = vast_blender.ssh_transport(9, 40022)
        out = vast_blender.write_toolchain(ssh, "root@1.2.3.4", 9, base, self.tmp / "t.json")
        remote = remote_blender.from_toolchain(json.loads(out.read_text()))
        self.assertEqual(remote.host, "root@1.2.3.4")
        self.assertEqual(remote.blender, vast_blender.BLENDER_REMOTE)
        self.assertEqual(remote.command([], {})[:len(ssh)], ssh)
        self.assertTrue(remote.applies("render") and remote.applies("bake"))
        # The tool steps run there too, with the host's pinned OIDN as their identity.
        for step in ("noise", "denoise", "directional", "rprb"):
            self.assertTrue(remote.applies(step), step)
        self.assertFalse(remote.applies("ktx2"))
        self.assertEqual(remote.tool_identity("openimagedenoise")["sha256"],
                         vast_blender.HOST["oidn"]["sha256"])
        blender_only = vast_blender.write_toolchain(ssh, "root@1.2.3.4", 9, base,
                                                    self.tmp / "b.json", tools=False)
        self.assertFalse(remote_blender.from_toolchain(
            json.loads(blender_only.read_text())).applies("denoise"))

    def test_provisioning_verifies_the_pinned_tarball(self):
        script = vast_blender.provision_script(Path("/src/engine"))
        self.assertIn(vast_blender.BLENDER_SHA256 + "  blender.tar.xz' | sha256sum -c", script)
        self.assertIn("mkdir -p /src/engine", script)
        self.assertIn(vast_blender.HOST["oidn"]["sha256"] + "  oidn.tar.gz' | sha256sum -c", script)
        self.assertIn("--require-hashes", script)
        for wheel in vast_blender.HOST["python_packages"]["wheels"]:
            self.assertIn("%s==%s --hash=sha256:%s" % (wheel["name"], wheel["version"],
                                                        wheel["sha256"]), script)
        self.assertNotIn("oidn.tar.gz", vast_blender.provision_script(Path("/x"), tools=False))
        self.assertTrue(vast_blender.BLENDER_REMOTE.startswith("/opt/blender-%s-"
                                                               % vast_blender.BLENDER_VERSION))

    def test_failed_start_destroys_the_instance(self):
        fake = FakeVast()
        entry = {"id": 42, "rate_usd_h": 0.3, "created": 0.0, "deadline": 1.0}
        with self.ledger.open() as data:
            data["instances"]["42"] = dict(entry)
        args = types.SimpleNamespace(start_timeout=0)
        with mock.patch.object(vast_blender, "Vast", lambda: fake), \
                mock.patch.object(vast_blender, "Ledger", lambda: self.ledger), \
                mock.patch.object(vast_blender, "rent", lambda *a: entry), \
                mock.patch.object(vast_blender, "log", lambda message: None):
            with self.assertRaises(RuntimeError):
                vast_blender.up(args)
        self.assertEqual(fake.destroyed, [42])
        self.assertEqual(self.ledger.read()["instances"]["42"]["destroy_reason"],
                         "start failed")
        self.assertEqual(vast_blender.live_instances(self.ledger), [])


class ReferenceRenderRoutingTest(unittest.TestCase):
    def test_tools_blender_mirrors_the_work_directory(self):
        import gi_reference
        calls = []

        class Remote:
            host = "root@h"

            def applies(self, step):
                return step == "render"

            def push(self, paths, mirror):
                calls.append(("push", mirror, sorted(str(p) for p in paths)))

            def command(self, options, env):
                calls.append(("env", env))
                return ["ssh", "root@h", "blender"]

            def pull(self, mirror, update=False):
                calls.append(("pull", mirror, update))

        work = Path(tempfile.mkdtemp()) / "state"
        tools = gi_reference.Tools.__new__(gi_reference.Tools)
        tools.values = {"blender": "blender", "ocio": "/abs/config.ocio"}
        tools.remote = Remote()
        texture = "/elsewhere/legacy-scene/textures/wall.png"
        with mock.patch.object(gi_reference, "run", lambda command, env, log:
                               calls.append(("run", command))), \
                mock.patch.object(gi_reference.map_scene, "parse", lambda path: {"path": path}), \
                mock.patch.object(gi_reference.map_scene, "input_files",
                                  lambda scene: [str(scene["path"]), texture]), \
                mock.patch.dict("os.environ", {"LIGHTING_EXTRAS": str(work / "extras.json")}):
            tools.blender("gi_reference_blender.py", ["--scene", work / "scene.json",
                                                      "--samples", "4"], work / "log.txt")
        self.assertEqual(calls[0][:2], ("push", work.resolve()))
        self.assertIn(str(work / "scene.json"), calls[0][2])
        # The scene's textures outside the work directory go too.
        self.assertIn(texture, calls[0][2])
        self.assertEqual(calls[1], ("env", {"OCIO": "/abs/config.ocio",
                                            "LIGHTING_EXTRAS": str(work / "extras.json")}))
        self.assertEqual(calls[2], ("run", ["ssh", "root@h", "blender"]))
        self.assertEqual(calls[3], ("pull", work.resolve(), True))
        # A step the block leaves out stays local.
        calls.clear()
        tools.remote.applies = lambda step: False
        with mock.patch.object(gi_reference, "run", lambda command, env, log:
                               calls.append(("run", command[0]))):
            tools.blender("x.py", [], work / "log.txt", step="probe-volume")
        self.assertEqual(calls, [("run", "blender")])


if __name__ == "__main__":
    unittest.main()
