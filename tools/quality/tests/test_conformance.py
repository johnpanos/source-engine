#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
#
# Self-tests for the shared conformance runner (RFC 0005 Q1 / roadmap R02).
#
# These prove the runner's outcome handling before it is trusted as a gate: it
# must detect suites that pass, fail, crash, hang, exhaust memory, omit or
# duplicate their result record, run zero checks, rely on assert(), fail to
# compile, go missing, or need an unavailable provider; it must fail on zero
# discovery, unmatched selectors, an unavailable compiler, and an unsupported
# schema; and it must retain every repeated attempt and its logs. Run:
#
#   python3 -m unittest discover -s tools/quality/tests -v
#
# ============================================================================

import json
import os
import shutil
import sys
import tempfile
import time
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
QUALITY_DIR = os.path.dirname(HERE)
sys.path.insert(0, QUALITY_DIR)

import conformance  # noqa: E402

REPO = conformance.repo_root()
CPP = "tools/quality/tests/fixtures/cpp"
PROFILES_DIR = "tools/quality/tests/fixtures/profiles"
CXX = os.environ.get("CXX", "g++")


def make_suite(sid, sources, expect="pass", kind="self-test", extra_flags=None,
               timeout=None, protocol=True):
    s = {
        "id": sid,
        "domain": "Q-SELFTEST",
        "rfc": "0005",
        "kind": kind,
        "profile": "self-test",
        "sources": [CPP + "/" + x for x in sources],
        "expect": expect,
    }
    if protocol and expect == "pass":
        s["result_protocol"] = conformance.RESULT_PROTOCOL
    if extra_flags:
        s["extra_flags"] = extra_flags
    if timeout is not None:
        s["timeout_seconds"] = timeout
    return s


class RunSuiteClassificationTest(unittest.TestCase):
    """run_suite() must classify every outcome correctly."""

    @classmethod
    def setUpClass(cls):
        cls.profile = conformance.load_profile(
            os.path.join(REPO, PROFILES_DIR), "self-test")
        cls.build_dir = tempfile.mkdtemp(prefix="conf-selftest-")

    def _run(self, suite, **kwargs):
        return conformance.run_suite(REPO, CXX, self.profile, suite, self.build_dir, **kwargs)

    def test_pass(self):
        r = self._run(make_suite("pass", ["pass.cpp"]))
        self.assertEqual(r["outcome"], conformance.OUTCOME_PASS)
        self.assertTrue(r["matched"])
        self.assertTrue(r["certified"])
        self.assertTrue(r["build_ok"])
        self.assertEqual(r["checks"], 1)

    def test_required_check_results(self):
        for flags, expected in [([], True), (["-DCHECKS=0"], False),
                                (["-DFAILURES=1"], False), (["-DDUPLICATE"], False)]:
            with self.subTest(flags=flags):
                suite = make_suite("checks", ["check_results.cpp"], extra_flags=flags)
                result = self._run(suite)
                self.assertEqual(result["matched"], expected)
                if not expected:
                    self.assertIn("result protocol", result["first_divergence"])

    def test_missing_record_is_an_incomplete_run(self):
        result = self._run(make_suite("incomplete", ["no_record.cpp"]))
        self.assertFalse(result["matched"])
        self.assertEqual(result["outcome"], conformance.OUTCOME_FAIL)
        self.assertIn("incomplete", result["first_divergence"])

    def test_min_checks_detects_dropped_cases(self):
        suite = make_suite("shrunk", ["check_results.cpp"])
        suite["min_checks"] = 5  # the fixture reports 4
        result = self._run(suite)
        self.assertFalse(result["matched"])
        self.assertIn("at least 5", result["first_divergence"])

    def test_negative_fixture_requires_its_specific_defect(self):
        suite = make_suite("wrong-diagnostic", ["compile_error.cpp"], expect="compile-error")
        suite["expected_diagnostic"] = "not-a-real-diagnostic"
        self.assertFalse(self._run(suite)["matched"])
        suite = make_suite("wrong-signal", ["crash.cpp"], expect="crash")
        suite["expected_signal"] = 999
        self.assertFalse(self._run(suite)["matched"])

    def test_failing_is_detected(self):
        # A failing suite declared as expect=pass must be reported as a mismatch.
        r = self._run(make_suite("failing", ["failing.cpp"]))
        self.assertEqual(r["outcome"], conformance.OUTCOME_FAIL)
        self.assertFalse(r["matched"])
        self.assertIn("FAIL", r["first_divergence"] or "")

    def test_crash_is_detected(self):
        r = self._run(make_suite("crash", ["crash.cpp"]))
        self.assertEqual(r["outcome"], conformance.OUTCOME_CRASH)
        self.assertFalse(r["matched"])
        self.assertIsNotNone(r["signal"])

    def test_timeout_is_detected(self):
        r = self._run(make_suite("hang", ["hang.cpp"], timeout=2))
        self.assertEqual(r["outcome"], conformance.OUTCOME_TIMEOUT)
        self.assertFalse(r["matched"])

    def test_timeout_kills_the_process_group_and_keeps_output(self):
        log_dir = tempfile.mkdtemp(prefix="conf-logs-")
        pid_file = os.path.join(log_dir, "child.pid")
        os.environ["CHILD_PID_FILE"] = pid_file
        try:
            r = self._run(make_suite("hang-child", ["hang_with_child.cpp"], timeout=2),
                          log_dir=log_dir)
        finally:
            del os.environ["CHILD_PID_FILE"]
        self.assertEqual(r["outcome"], conformance.OUTCOME_TIMEOUT)
        # Output written before the hang is retained as a diagnostic.
        self.assertIn("parent waiting forever", r["detail"])
        with open(r["attempts"][0]["log"], encoding="utf-8") as log:
            self.assertIn("parent waiting forever", log.read())
        with open(pid_file, encoding="utf-8") as stream:
            child = int(stream.read())
        deadline = time.monotonic() + 5
        alive = True
        while alive and time.monotonic() < deadline:
            try:
                os.kill(child, 0)
                time.sleep(0.05)
            except ProcessLookupError:
                alive = False
        self.assertFalse(alive, "suite child process outlived the timeout")

    def test_memory_is_bounded(self):
        r = self._run(make_suite("hog", ["memory_hog.cpp"]))
        self.assertIn(r["outcome"], (conformance.OUTCOME_CRASH, conformance.OUTCOME_FAIL))
        self.assertFalse(r["matched"])
        self.assertEqual(r["memory_limit_mb"], 64)

    def test_compile_error_is_detected(self):
        r = self._run(make_suite("broken", ["compile_error.cpp"]))
        self.assertEqual(r["outcome"], conformance.OUTCOME_COMPILE_ERROR)
        self.assertFalse(r["matched"])
        self.assertFalse(r["build_ok"])

    def test_missing_source_is_detected(self):
        r = self._run(make_suite("gone", ["does_not_exist.cpp"]))
        self.assertEqual(r["outcome"], conformance.OUTCOME_MISSING_SOURCE)
        self.assertFalse(r["matched"])

    def test_expect_fail_matches_a_failing_suite(self):
        # A suite declared expect=fail that fails is a MATCH (self-test semantics).
        r = self._run(make_suite("neg", ["failing.cpp"], expect="fail"))
        self.assertEqual(r["outcome"], conformance.OUTCOME_FAIL)
        self.assertTrue(r["matched"])

    def test_expect_fail_flags_a_passing_suite(self):
        # A suite declared expect=fail that PASSES is a mismatch.
        r = self._run(make_suite("neg2", ["pass.cpp"], expect="fail"))
        self.assertEqual(r["outcome"], conformance.OUTCOME_PASS)
        self.assertFalse(r["matched"])

    def test_sensitivity_ok(self):
        r = self._run(make_suite("sens", ["sensitivity_ok.cpp"], kind="sensitivity"))
        self.assertEqual(r["outcome"], conformance.OUTCOME_PASS)
        self.assertTrue(r["matched"])

    def test_vacuous_sensitivity_is_flagged(self):
        r = self._run(make_suite("sensv", ["sensitivity_vacuous.cpp"], kind="sensitivity"))
        self.assertEqual(r["outcome"], conformance.OUTCOME_FAIL)
        self.assertFalse(r["matched"])


class ResponseFileTest(unittest.TestCase):
    """Relative @response files resolve against --root, not the caller's cwd."""

    def test_rooted_flags(self):
        self.assertEqual(conformance.rooted_flags("/r", ["@a/b.rsp", "@/abs.rsp", "-Dx", "-I@y"]),
                         ["@/r/a/b.rsp", "@/abs.rsp", "-Dx", "-I@y"])

    def test_response_file_applies_from_another_cwd(self):
        profile = conformance.load_profile(os.path.join(REPO, PROFILES_DIR), "self-test")
        build_dir = tempfile.mkdtemp(prefix="conf-rsp-")
        suite = make_suite("rsp", ["response_file.cpp"],
                           extra_flags=["@" + CPP + "/response_file.rsp"])
        previous = os.getcwd()
        os.chdir(build_dir)
        try:
            r = conformance.run_suite(REPO, CXX, profile, suite, build_dir)
        finally:
            os.chdir(previous)
            shutil.rmtree(build_dir, ignore_errors=True)
        self.assertEqual(r["outcome"], conformance.OUTCOME_PASS, r)


class ProviderTest(unittest.TestCase):
    """Unavailable providers never pass; optional skips never certify."""

    @classmethod
    def setUpClass(cls):
        cls.profile = conformance.load_profile(
            os.path.join(REPO, PROFILES_DIR), "self-test")
        cls.build_dir = tempfile.mkdtemp(prefix="conf-provider-")

    def _run(self, suite):
        return conformance.run_suite(REPO, CXX, self.profile, suite, self.build_dir)

    def test_required_unavailable_provider_fails(self):
        for req in ("executable:conformance-selftest-no-such-tool",
                    "env:CONFORMANCE_SELFTEST_UNSET_VARIABLE",
                    "path:tools/quality/tests/fixtures/no-such-content"):
            with self.subTest(req=req):
                suite = make_suite("needs", ["pass.cpp"])
                suite["requires"] = [req]
                r = self._run(suite)
                self.assertEqual(r["outcome"], conformance.OUTCOME_UNAVAILABLE)
                self.assertFalse(r["matched"])
                self.assertFalse(r["certified"])
                self.assertIn(req, r["detail"])

    def test_optional_unavailable_provider_skips_with_reason(self):
        suite = make_suite("maybe", ["pass.cpp"])
        suite["requires"] = ["executable:conformance-selftest-no-such-tool"]
        suite["optional"] = True
        r = self._run(suite)
        self.assertEqual(r["outcome"], conformance.OUTCOME_SKIPPED)
        self.assertTrue(r["matched"])
        self.assertFalse(r["certified"])
        self.assertIn("conformance-selftest-no-such-tool", r["skip_reason"])

    def test_available_provider_runs(self):
        suite = make_suite("has", ["pass.cpp"])
        suite["requires"] = ["executable:" + CXX, "path:" + CPP + "/pass.cpp"]
        r = self._run(suite)
        self.assertEqual(r["outcome"], conformance.OUTCOME_PASS)
        self.assertTrue(r["certified"])


class OptimizedBuildTest(unittest.TestCase):
    """Test assertions stay effective in optimized NDEBUG builds."""

    @classmethod
    def setUpClass(cls):
        cls.profile = conformance.load_profile(
            os.path.join(REPO, PROFILES_DIR), "self-test")
        cls.build_dir = tempfile.mkdtemp(prefix="conf-release-")

    def test_counted_failure_fails_in_every_config(self):
        for config in sorted(conformance.BUILD_CONFIGS):
            with self.subTest(config=config):
                r = conformance.run_suite(REPO, CXX, self.profile,
                                          make_suite("counted", ["counted_failure.cpp"]),
                                          self.build_dir, config=config)
                self.assertEqual(r["outcome"], conformance.OUTCOME_FAIL)
                self.assertEqual(r["failed_checks"], 1)
                if config == "release":
                    self.assertIn("-DNDEBUG", r["repro"])

    def test_assert_based_suite_is_rejected(self):
        # Staged under a unittests/ path, where the rule applies to test sources.
        root = tempfile.mkdtemp(prefix="conf-assert-")
        os.makedirs(os.path.join(root, "unittests"))
        shutil.copy(os.path.join(REPO, CPP, "assert_only.cpp"), os.path.join(root, "unittests"))
        suite = make_suite("asserts", ["x"])
        suite["sources"] = ["unittests/assert_only.cpp"]
        r = conformance.run_suite(root, CXX, self.profile, suite, self.build_dir)
        self.assertEqual(r["outcome"], conformance.OUTCOME_INVALID_ORACLE)
        self.assertFalse(r["matched"])
        self.assertIn("unittests/assert_only.cpp", r["first_divergence"])

    def test_static_assert_and_comments_are_not_flagged(self):
        root = tempfile.mkdtemp(prefix="conf-assert-ok-")
        os.makedirs(os.path.join(root, "unittests"))
        with open(os.path.join(root, "unittests", "ok.cpp"), "w", encoding="utf-8") as f:
            f.write("// never use assert( x ) here\nstatic_assert( sizeof( int ) >= 2 );\n")
        suite = {"sources": ["unittests/ok.cpp"]}
        self.assertEqual(conformance.assert_users(root, suite), [])


class RepeatAndSeedTest(unittest.TestCase):
    """Repeats retain every attempt; the seed reaches the suite."""

    @classmethod
    def setUpClass(cls):
        cls.profile = conformance.load_profile(
            os.path.join(REPO, PROFILES_DIR), "self-test")
        cls.build_dir = tempfile.mkdtemp(prefix="conf-repeat-")

    def _run(self, **kwargs):
        return conformance.run_suite(REPO, CXX, self.profile,
                                     make_suite("env", ["environment.cpp"]),
                                     self.build_dir, log_dir=tempfile.mkdtemp(), **kwargs)

    def test_seed_and_attempt_are_exported(self):
        r = self._run(seed=4242, repeat=2)
        self.assertTrue(r["matched"])
        self.assertEqual(len(r["attempts"]), 2)
        with open(r["attempts"][1]["log"], encoding="utf-8") as log:
            text = log.read()
        self.assertIn("seed=4242 attempt=1", text)

    def test_intermittent_failure_is_retained(self):
        os.environ["FAIL_ON_ATTEMPT"] = "1"
        try:
            r = self._run(repeat=3)
        finally:
            del os.environ["FAIL_ON_ATTEMPT"]
        self.assertFalse(r["matched"])
        self.assertEqual(len(r["attempts"]), 3)  # no stop-at-green, no retry
        self.assertEqual([a["matched"] for a in r["attempts"]], [True, False, True])
        self.assertEqual(r["outcome"], conformance.OUTCOME_FAIL)
        self.assertIn("attempt 1", r["first_divergence"])


class ManifestValidationTest(unittest.TestCase):
    """Structural manifest problems are fatal."""

    def _write(self, obj):
        f = tempfile.NamedTemporaryFile(
            "w", suffix=".json", delete=False, dir=self.tmp)
        json.dump(obj, f)
        f.close()
        return f.name

    def _load(self, suites):
        return conformance.load_manifest(
            self._write({"schema": conformance.MANIFEST_SCHEMA, "suites": suites}))

    def setUp(self):
        self.tmp = tempfile.mkdtemp(prefix="conf-manifest-")

    def test_unknown_schema_rejected(self):
        path = self._write({"schema": "bogus/v9", "suites": []})
        with self.assertRaises(conformance.ManifestError):
            conformance.load_manifest(path)

    def test_duplicate_id_rejected(self):
        with self.assertRaises(conformance.ManifestError):
            self._load([make_suite("dup", ["pass.cpp"]), make_suite("dup", ["pass.cpp"])])

    def test_invalid_expect_rejected(self):
        bad = make_suite("x", ["pass.cpp"])
        bad["expect"] = "sometimes"
        with self.assertRaises(conformance.ManifestError):
            self._load([bad])

    def test_passing_suite_without_result_protocol_rejected(self):
        with self.assertRaises(conformance.ManifestError):
            self._load([make_suite("x", ["pass.cpp"], protocol=False)])
        # Negative fixtures expecting another outcome need no record.
        self._load([make_suite("y", ["failing.cpp"], expect="fail", protocol=False)])

    def test_runner_outcomes_cannot_be_expected(self):
        for outcome in (conformance.OUTCOME_SKIPPED, conformance.OUTCOME_UNAVAILABLE,
                        conformance.OUTCOME_INVALID_ORACLE):
            with self.subTest(outcome=outcome), self.assertRaises(conformance.ManifestError):
                self._load([make_suite("x", ["pass.cpp"], expect=outcome)])

    def test_bad_requirements_rejected(self):
        for requires in (["gpu"], ["socket:x"], ["executable:"], "executable:x"):
            with self.subTest(requires=requires), self.assertRaises(conformance.ManifestError):
                suite = make_suite("x", ["pass.cpp"])
                suite["requires"] = requires
                self._load([suite])

    def test_optional_without_provider_rejected(self):
        suite = make_suite("x", ["pass.cpp"])
        suite["optional"] = True
        with self.assertRaises(conformance.ManifestError):
            self._load([suite])

    def test_invalid_min_checks_rejected(self):
        suite = make_suite("x", ["pass.cpp"])
        suite["min_checks"] = 0
        with self.assertRaises(conformance.ManifestError):
            self._load([suite])


class EndToEndTest(unittest.TestCase):
    """Drive the full `check` command through main()."""

    def _check(self, suites, extra_args=None, out="-", cxx=CXX, command="check"):
        f = tempfile.NamedTemporaryFile("w", suffix=".json", delete=False)
        json.dump(
            {"schema": conformance.MANIFEST_SCHEMA,
             "profiles_dir": PROFILES_DIR, "suites": suites}, f)
        f.close()
        args = ["--root", REPO, "--manifest", f.name, command]
        if command == "check":
            args += ["--cxx", cxx, "--out", out,
                     "--build-dir", tempfile.mkdtemp(prefix="conf-e2e-")]
        if extra_args:
            args += extra_args
        try:
            return conformance.main(args)
        finally:
            os.unlink(f.name)

    def test_all_pass_returns_zero(self):
        self.assertEqual(self._check([make_suite("p", ["pass.cpp"])]), 0)

    def test_any_fail_returns_one(self):
        rc = self._check([make_suite("p", ["pass.cpp"]),
                          make_suite("f", ["failing.cpp"])])
        self.assertEqual(rc, 1)

    def test_zero_discovery_is_fatal(self):
        self.assertEqual(self._check([]), 2)
        self.assertEqual(self._check([], command="plan"), 2)

    def test_unmatched_selector_is_fatal(self):
        rc = self._check([make_suite("p", ["pass.cpp"])],
                         extra_args=["--suite", "nope"])
        self.assertEqual(rc, 2)

    def test_partially_unmatched_selector_is_fatal(self):
        # One real id plus one mistyped id must not shrink to a passing run.
        rc = self._check([make_suite("p", ["pass.cpp"])],
                         extra_args=["--suite", "p", "--suite", "p-typo"])
        self.assertEqual(rc, 2)

    def test_selector_runs_only_matching(self):
        rc = self._check([make_suite("p", ["pass.cpp"]),
                          make_suite("f", ["failing.cpp"])],
                         extra_args=["--suite", "p"])
        self.assertEqual(rc, 0)  # the failing suite is filtered out

    def test_unavailable_compiler_is_fatal(self):
        rc = self._check([make_suite("p", ["pass.cpp"])], cxx="conformance-no-such-compiler")
        self.assertEqual(rc, 2)

    def test_required_unavailable_provider_fails_the_run(self):
        suite = make_suite("p", ["pass.cpp"])
        suite["requires"] = ["executable:conformance-selftest-no-such-tool"]
        self.assertEqual(self._check([suite]), 1)

    def test_optional_skip_passes_but_is_not_certified(self):
        suite = make_suite("maybe", ["pass.cpp"])
        suite["requires"] = ["env:CONFORMANCE_SELFTEST_UNSET_VARIABLE"]
        suite["optional"] = True
        out = os.path.join(tempfile.mkdtemp(prefix="conf-ev-"), "evidence.json")
        self.assertEqual(self._check([suite, make_suite("p", ["pass.cpp"])], out=out), 0)
        with open(out, encoding="utf-8") as f:
            evidence = json.load(f)
        self.assertEqual(evidence["counts"]["skipped"], 1)
        self.assertEqual(evidence["counts"]["certified"], 1)
        skipped = [s for s in evidence["suites"] if s["id"] == "maybe"][0]
        self.assertFalse(skipped["certified"])
        self.assertIn("CONFORMANCE_SELFTEST_UNSET_VARIABLE", skipped["skip_reason"])

    def test_plan_enumerates_without_running(self):
        suite = make_suite("needs", ["does_not_exist.cpp"])
        suite["requires"] = ["executable:conformance-selftest-no-such-tool"]
        self.assertEqual(self._check([suite], command="plan"), 0)

    def test_evidence_is_complete_and_reproducible(self):
        out_dir = tempfile.mkdtemp(prefix="conf-ev-")
        out = os.path.join(out_dir, "evidence.json")
        rc = self._check([make_suite("p", ["pass.cpp"]), make_suite("f", ["failing.cpp"])],
                         out=out, extra_args=["--seed", "7", "--config", "release"])
        self.assertEqual(rc, 1)
        with open(out, encoding="utf-8") as f:
            evidence = json.load(f)
        self.assertEqual(evidence["schema"], conformance.EVIDENCE_SCHEMA)
        self.assertEqual(evidence["decision"], "fail")
        self.assertTrue(evidence["reconciled"])
        self.assertEqual(evidence["expected_ids"], ["f", "p"])
        self.assertEqual(evidence["run"]["seed"], 7)
        self.assertEqual(evidence["run"]["config"], "release")
        self.assertIn("--seed", evidence["run"]["invocation"])
        for key in ("source_revision", "cxx_version", "cxx_target", "host", "submodules"):
            self.assertIn(key, evidence["identity"])
        self.assertTrue(evidence["manifest_digest"])
        failing = [s for s in evidence["suites"] if s["id"] == "f"][0]
        self.assertTrue(failing["input_digest"])
        self.assertIn("-DNDEBUG", failing["repro"])
        # Full per-suite logs live beside the evidence and are referenced by it.
        run_log = failing["attempts"][0]["log"]
        self.assertTrue(run_log.startswith(os.path.join(out_dir, "evidence.logs")))
        with open(run_log, encoding="utf-8") as log:
            self.assertIn("deliberate self-test failure", log.read())
        self.assertTrue(os.path.isfile(failing["logs"]["build"]))

    def test_interrupted_run_leaves_incomplete_evidence(self):
        out = os.path.join(tempfile.mkdtemp(prefix="conf-ev-"), "evidence.json")
        original = conformance.run_suite
        calls = []

        def interrupted(*args, **kwargs):
            if calls:
                raise KeyboardInterrupt
            calls.append(1)
            return original(*args, **kwargs)

        conformance.run_suite = interrupted
        try:
            with self.assertRaises(KeyboardInterrupt):
                self._check([make_suite("a", ["pass.cpp"]), make_suite("b", ["pass.cpp"])],
                            out=out)
        finally:
            conformance.run_suite = original
        with open(out, encoding="utf-8") as f:
            evidence = json.load(f)
        self.assertEqual(evidence["decision"], "incomplete")
        self.assertEqual(evidence["executed_ids"], ["a"])
        self.assertFalse(evidence["reconciled"])


VULKANINFO_SUMMARY = """==========
VULKANINFO
==========

Vulkan Instance Version: 1.4.341

Devices:
========
GPU0:
	apiVersion         = 1.4.354
	deviceType         = PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU
	deviceName         = AMD Radeon 8060S Graphics (RADV STRIX_HALO)
	driverName         = radv
	driverInfo         = Mesa 26.2.2
GPU1:
	apiVersion         = 1.4.354
	deviceType         = PHYSICAL_DEVICE_TYPE_CPU
	deviceName         = llvmpipe (LLVM 22.1.8, 256 bits)
	driverName         = llvmpipe
"""


class RunnerClassTest(unittest.TestCase):
    """GPU profiles: class selection, profile env/link/providers, device probing."""

    @classmethod
    def setUpClass(cls):
        cls.profile = conformance.load_profile(
            os.path.join(REPO, PROFILES_DIR), "self-test")
        cls.build_dir = tempfile.mkdtemp(prefix="conf-runner-")

    def _gpu(self, sid, sources, **kwargs):
        suite = make_suite(sid, sources, **kwargs)
        suite["profile"] = "self-test-gpu"
        return suite

    def _profile(self, **fields):
        return dict(self.profile, **fields)

    def test_unselected_check_runs_only_the_headless_class(self):
        suites = [make_suite("p", ["pass.cpp"]), self._gpu("g", ["failing.cpp"])]
        self.assertEqual(EndToEndTest()._check(suites), 0)
        self.assertEqual(EndToEndTest()._check(suites, extra_args=["--runner", "gpu"]), 1)
        self.assertEqual(EndToEndTest()._check(suites, extra_args=["--runner", "all"]), 1)
        # Naming a GPU suite selects it without --runner.
        self.assertEqual(EndToEndTest()._check(suites, extra_args=["--suite", "g"]), 1)

    def test_a_class_with_no_suites_is_fatal(self):
        self.assertEqual(EndToEndTest()._check([self._gpu("g", ["pass.cpp"])]), 2)
        self.assertEqual(EndToEndTest()._check([make_suite("p", ["pass.cpp"])],
                                               extra_args=["--runner", "gpu"]), 2)

    def test_unknown_runner_class_is_rejected(self):
        directory = tempfile.mkdtemp(prefix="conf-profile-")
        with open(os.path.join(directory, "bad.json"), "w", encoding="utf-8") as f:
            json.dump({"schema": conformance.PROFILE_SCHEMA, "id": "bad",
                       "cxx_std": "c++20", "runner": "quantum"}, f)
        with self.assertRaises(conformance.ManifestError):
            conformance.load_profile(directory, "bad")

    def test_profile_environment_reaches_the_suite(self):
        profile = self._profile(run_env={"FAIL_ON_ATTEMPT": "0"})
        r = conformance.run_suite(REPO, CXX, profile, make_suite("env", ["environment.cpp"]),
                                  self.build_dir)
        self.assertEqual(r["outcome"], conformance.OUTCOME_FAIL)
        os.environ["FAIL_ON_ATTEMPT"] = "0"
        try:
            profile = self._profile(run_env_unset=["FAIL_ON_ATTEMPT"])
            r = conformance.run_suite(REPO, CXX, profile,
                                      make_suite("env", ["environment.cpp"]), self.build_dir)
        finally:
            del os.environ["FAIL_ON_ATTEMPT"]
        self.assertEqual(r["outcome"], conformance.OUTCOME_PASS)

    def test_profile_link_flags_are_applied(self):
        profile = self._profile(link_flags=["-lconformance-selftest-no-such-library"])
        r = conformance.run_suite(REPO, CXX, profile, make_suite("p", ["pass.cpp"]),
                                  self.build_dir)
        self.assertEqual(r["outcome"], conformance.OUTCOME_COMPILE_ERROR)
        self.assertIn("-lconformance-selftest-no-such-library", r["repro"])

    def test_profile_providers_apply_to_every_suite(self):
        profile = self._profile(requires=["executable:conformance-selftest-no-such-tool"])
        r = conformance.run_suite(REPO, CXX, profile, make_suite("p", ["pass.cpp"]),
                                  self.build_dir)
        self.assertEqual(r["outcome"], conformance.OUTCOME_UNAVAILABLE)
        self.assertIn("executable:conformance-selftest-no-such-tool", r["requires"])

    def test_vulkaninfo_summary_is_parsed(self):
        devices = conformance.parse_vulkaninfo_summary(VULKANINFO_SUMMARY)
        self.assertEqual([d["deviceType"] for d in devices],
                         ["PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU", "PHYSICAL_DEVICE_TYPE_CPU"])
        self.assertEqual(devices[0]["driverName"], "radv")

    def test_vulkan_device_provider_requires_a_real_gpu(self):
        original = conformance.vulkan_devices
        try:
            cpu_only = [d for d in conformance.parse_vulkaninfo_summary(VULKANINFO_SUMMARY)
                        if d["deviceType"].endswith("_CPU")]
            conformance.vulkan_devices = lambda env=None: (cpu_only, None)
            ok, detail = conformance.provider_available(REPO, "vulkan-device:gpu")
            self.assertFalse(ok)
            self.assertIn("PHYSICAL_DEVICE_TYPE_CPU", detail)
            self.assertTrue(conformance.provider_available(REPO, "vulkan-device:any")[0])
            self.assertFalse(conformance.provider_available(REPO, "vulkan-device:tpu")[0])
            conformance.vulkan_devices = lambda env=None: ([], "vulkaninfo not found on PATH")
            ok, detail = conformance.provider_available(REPO, "vulkan-device:gpu")
            self.assertFalse(ok)
            self.assertIn("vulkaninfo", detail)
        finally:
            conformance.vulkan_devices = original

    def test_expected_divergence_names_the_defect(self):
        # A deliberate failure of another kind must not satisfy the row.
        failing = make_suite("other", ["failing.cpp"], expect="fail")
        failing["expected_divergence"] = "zero checks executed"
        r = conformance.run_suite(REPO, CXX, self.profile, failing, self.build_dir)
        self.assertEqual(r["outcome"], conformance.OUTCOME_FAIL)
        self.assertFalse(r["matched"])


if __name__ == "__main__":
    unittest.main()
