"""Runtime oracles for USD map roles (usd_map_runtime.py --roles, RFC 0009 U2).

    SOURCE_USD_MAP_TOOLS=<installed tools with vbsp -authored> \\
    SOURCE_USD_MAP_CLIENT_BUILD=<portal client Waf output> \\
    SOURCE_USD_MAP_DEDICATED_BUILD=<portal dedicated Waf output> \\
    PYTHONPATH=build/toolchains/openusd-25.11/lib/python \\
        /usr/bin/python3.12 -m unittest tools.quality.tests.test_usd_map_runtime -v

Compiles the U2 role fixture and runs it headless: in the windowless client
(null renderer, SDL offscreen; the only product that simulates without a
network player) and in the dedicated server (spawn state and traces). Both
builds must contain map_entity_probe, map_entity_probe_schedule and
map_trace_probe (game/server/entitylist.cpp). Seeded runtime mutants compile a
changed map past the output checks and must fail the runtime checks: the
runtime oracle does not lean on the compile-time one.
"""

import io
import json
import os
import sys
import tempfile
import unittest
from contextlib import redirect_stdout
from pathlib import Path

try:
    import pxr  # noqa: F401
except ImportError as error:  # system Python has no OpenUSD; see the docstring
    raise unittest.SkipTest("test_usd_map_runtime needs OpenUSD's pxr: %s" % error)

ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(ROOT / "tools/quality"))
sys.path.insert(0, str(ROOT / "tools/quality/tests"))
import usd_map_compile as compiler  # noqa: E402
import usd_map_runtime  # noqa: E402
from test_usd_authoring import ROLES  # noqa: E402

TOOLS = os.environ.get("SOURCE_USD_MAP_TOOLS")
CLIENT = os.environ.get("SOURCE_USD_MAP_CLIENT_BUILD")
DEDICATED = os.environ.get("SOURCE_USD_MAP_DEDICATED_BUILD")
RUNTIME = Path(os.environ.get("SOURCE_USD_MAP_RUNTIME", ROOT / "run/runtime"))
MAP = "usd_roles"


def unchecked(bsp2, report, provenance, profile, runtime):
    """Stands in for the output checks, so a mutant map reaches the runtime."""
    return {"schema": "source-usd-map-check/v1", "checks": 1, "failures": [],
            "observations": {}}


@unittest.skipUnless(TOOLS and CLIENT and DEDICATED,
                     "set SOURCE_USD_MAP_TOOLS, SOURCE_USD_MAP_CLIENT_BUILD and "
                     "SOURCE_USD_MAP_DEDICATED_BUILD")
class RoleRuntimeTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        if not (RUNTIME / "portal/portal_pak_dir.vpk").is_file():
            raise unittest.SkipTest("Portal content missing at %s" % RUNTIME)
        cls.temporary = tempfile.TemporaryDirectory(prefix="usd-map-runtime-test-")
        cls.scratch = Path(cls.temporary.name)
        cls.good = cls.compile("good")
        cls.runs = {}

    @classmethod
    def tearDownClass(cls):
        cls.temporary.cleanup()

    @classmethod
    def compile(cls, label, change=None):
        """The role fixture's build; `change` edits its brush set, and then
        the output checks are bypassed."""
        original = compiler.build_brushset
        if change:
            def build(report, profile, name):
                brushset, objects = original(report, profile, name)
                change(brushset)
                return brushset, objects
            compiler.build_brushset = build
        try:
            result = compiler.compile_stage(ROLES / "roles.usda", MAP, TOOLS, RUNTIME,
                                            cls.scratch / label,
                                            checker=unchecked if change else None)
        finally:
            compiler.build_brushset = original
        stages = cls.scratch / label / "stages"
        return {"bsp2": result["bsp2"], "report": stages / "validate/authoring-report.json",
                "provenance": stages / "assemble/provenance.json"}

    def run_product(self, build, product, label):
        out = self.scratch / ("run-%s-%s" % (label, product))
        argv = ["--map-file", str(build["bsp2"]), "--map", MAP, "--report",
                str(build["report"]), "--provenance", str(build["provenance"]), "--roles",
                "--runtime", str(RUNTIME), "--build", CLIENT if product == "client" else DEDICATED,
                "--product", product, "--out", str(out)]
        with redirect_stdout(io.StringIO()):
            status = usd_map_runtime.main(argv)
        evidence = json.loads((out / "evidence.json").read_text())
        evidence["status"] = status
        return evidence

    def failed(self, evidence):
        return {failure.split(":")[0] for failure in evidence["failed"]}

    # ---------------------------------------------------------- positive

    def test_client_role_behaviour(self):
        evidence = self.run_product(self.good, "client", "good")
        self.assertEqual(evidence["failed"], [])
        self.assertEqual(evidence["status"], 0)
        self.assertTrue(evidence["behaviour_checked"])
        self.assertGreaterEqual(evidence["simulated_seconds"], usd_map_runtime.MIN_SECONDS)
        box = [s["origin"][2] for s in evidence["role_samples"]["prop.drop-box"]]
        self.assertGreater(box[0], 150.0)
        self.assertLess(abs(box[-1] - 20.25), usd_map_runtime.REST_TOLERANCE)
        fired = {(o[2], o[3], o[4]) for o in evidence["outputs_fired"]}
        self.assertEqual(fired, {("entity.drop-trigger", "prop.lever-switch", "SetAnimation"),
                                 ("entity.drop-trigger", "entity.lift", "Open")})

    def test_dedicated_spawn_state_and_traces(self):
        evidence = self.run_product(self.good, "dedicated", "good")
        self.assertEqual(evidence["failed"], [])
        self.assertFalse(evidence["behaviour_checked"])
        hits = {(t["id"], t["hull"]): t["hit"] for t in evidence["role_traces"]}
        self.assertEqual(hits, {("prop.lab-desk", False): "static_prop",
                                ("prop.lab-desk", True): "static_prop",
                                ("prop.ghost-desk", False): "world",
                                ("prop.ghost-desk", True): "world"})

    # ----------------------------------------------------------- mutants

    def test_runtime_mutant_dynamic_prop_as_physics(self):
        def change(brushset):
            switch = next(e for e in brushset["entities"] if e.get("id") == "prop.lever-switch")
            switch["classname"] = "prop_physics"
            switch["keys"].pop("DefaultAnim")
        evidence = self.run_product(self.compile("dynamic-as-physics", change), "client",
                                    "dynamic-as-physics")
        # The game removes the switch as a physics prop: no prop_dynamic
        # spawns, so nothing with its name can be sampled.
        self.assertTrue({"runtime.entity-spawned", "roles.sampled"} <= self.failed(evidence),
                        evidence["failed"])

    def test_runtime_mutant_trigger_ignores_physics(self):
        def change(brushset):
            trigger = next(e for e in brushset["entities"]
                           if e.get("id") == "entity.drop-trigger")
            trigger["keys"]["spawnflags"] = "1"
        evidence = self.run_product(self.compile("clients-only", change), "client",
                                    "clients-only")
        self.assertTrue({"roles.connection-fired", "roles.movelinear-opens",
                         "roles.dynamic-animates"} <= self.failed(evidence),
                        evidence["failed"])
        self.assertNotIn("roles.physics-falls", self.failed(evidence))

    def test_runtime_mutant_static_prop_not_solid(self):
        def change(brushset):
            desk = next(e for e in brushset["entities"] if e.get("id") == "prop.lab-desk")
            desk["keys"]["solid"] = "0"
        evidence = self.run_product(self.compile("desk-not-solid", change), "dedicated",
                                    "desk-not-solid")
        self.assertTrue({"roles.static-prop-blocks", "roles.static-prop-top"} <=
                        self.failed(evidence), evidence["failed"])


if __name__ == "__main__":
    unittest.main()
