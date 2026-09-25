#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
#
# Parallel, tiered runner for the quality tool self-tests (tools/quality/tests).
#
#   selftest.py                 fast tier: every test not marked @slow
#   selftest.py --tier slow     only the @slow tests
#   selftest.py --tier all      everything
#
# Each TestCase class runs in its own worker process, --jobs at a time, so the
# tier takes about as long as its slowest class rather than the sum of all of
# them. Every test's duration is recorded; a fast-tier test that takes longer
# than --test-budget seconds is reported so it can be sped up or marked @slow.
# Plain `python3 -m unittest discover -s tools/quality/tests` still runs
# every test in one process.
#
# Zero selected tests, a worker that crashes or exceeds --timeout, and any
# failure or error fail the run (RFC 0005 runner rules). Python 3 stdlib only.
#
# ============================================================================
import argparse
import concurrent.futures
import json
import os
import subprocess
import sys
import tempfile
import time
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
DEFAULT_START = os.path.join(HERE, "tests")
TIERS = ("fast", "slow", "all")
SLOW_ATTRIBUTE = "quality_tier_slow"


def slow(test):
    """Mark a test method or class for the slow tier."""
    setattr(test, SLOW_ATTRIBUTE, True)
    return test


def is_slow(test):
    method = getattr(test, test._testMethodName, None)
    return bool(getattr(method, SLOW_ATTRIBUTE, False) or getattr(type(test), SLOW_ATTRIBUTE, False))


def iter_tests(suite):
    for item in suite:
        if isinstance(item, unittest.TestSuite):
            yield from iter_tests(item)
        else:
            yield item


def discover(start, pattern):
    loader = unittest.TestLoader()
    suite = loader.discover(start, pattern=pattern, top_level_dir=start)
    tests, generated = [], []
    for test in iter_tests(suite):
        # The loader stands in its own tests for a module that failed to
        # import (_FailedTest) or skipped itself (ModuleSkipped). Workers cannot
        # load those by name, so the parent runs them: an import error is
        # reported, never dropped.
        if type(test).__module__ == "unittest.loader":
            generated.append(test)
        else:
            tests.append(test)
    return tests, generated


def run_discovery(start, pattern, out_path):
    """Discovery worker: import the test modules in this process only, and
    describe them. The parent never imports a test module, so it keeps no
    module cache between runs and pays no import cost."""
    sys.path.insert(0, start)
    tests, generated = discover(start, pattern)
    loader_result = unittest.TestResult()
    for test in generated:
        test.run(loader_result)
    report = {
        "tests": [{"id": t.id(), "class": "%s.%s" % (type(t).__module__, type(t).__qualname__),
                   "slow": is_slow(t)} for t in tests],
        "generated_ran": loader_result.testsRun,
        "generated_errors": [[t.id(), text] for t, text in
                             loader_result.errors + loader_result.failures],
        "generated_skips": [[t.id(), reason] for t, reason in loader_result.skipped],
    }
    with open(out_path, "w", encoding="utf-8") as handle:
        json.dump(report, handle)
    return 0


def discover_in_subprocess(start, pattern, timeout):
    with tempfile.TemporaryDirectory(prefix="selftest-discover-") as work:
        out_path = os.path.join(work, "discovery.json")
        completed = subprocess.run(
            [sys.executable, os.path.abspath(__file__), "--start", start, "--pattern", pattern,
             "--discover", out_path],
            stdin=subprocess.DEVNULL, capture_output=True, text=True, timeout=timeout)
        if not os.path.exists(out_path):
            raise RuntimeError("test discovery failed (exit %s):\n%s"
                               % (completed.returncode, completed.stdout + completed.stderr))
        with open(out_path, encoding="utf-8") as handle:
            return json.load(handle)


def select(tests, tier):
    if tier == "all":
        return list(tests)
    want_slow = tier == "slow"
    return [t for t in tests if t["slow"] == want_slow]


def group_by_class(tests, chunk):
    """Worker groups: each TestCase class, split into runs of at most `chunk`
    tests so one large class does not set the whole tier's wall time."""
    classes = {}
    for test in tests:
        classes.setdefault(test["class"], []).append(test["id"])
    groups = {}
    for key, ids in classes.items():
        if len(ids) <= chunk:
            groups[key] = ids
            continue
        for index in range(0, len(ids), chunk):
            groups["%s[%d]" % (key, index // chunk)] = ids[index:index + chunk]
    return groups


# ---------------------------------------------------------------------------
# Worker: run the named tests in this process and write one JSON result.
# ---------------------------------------------------------------------------

class _TimedResult(unittest.TextTestResult):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.durations = {}
        self._started = {}

    def startTest(self, test):
        self._started[test.id()] = time.monotonic()
        super().startTest(test)

    def stopTest(self, test):
        super().stopTest(test)
        started = self._started.pop(test.id(), None)
        if started is not None:
            self.durations[test.id()] = round(time.monotonic() - started, 3)


def run_worker(start, ids, out_path):
    sys.path.insert(0, start)
    loader = unittest.TestLoader()
    suite = unittest.TestSuite()
    for ident in ids:
        suite.addTests(loader.loadTestsFromName(ident))
    stream = open(os.devnull, "w")
    runner = unittest.TextTestRunner(stream=stream, resultclass=_TimedResult, verbosity=0)
    result = runner.run(suite)
    report = {
        "ran": result.testsRun,
        "failures": [[t.id(), text] for t, text in result.failures],
        "errors": [[t.id(), text] for t, text in result.errors],
        "unexpected_successes": [t.id() for t in result.unexpectedSuccesses],
        "skipped": len(result.skipped),
        "durations": result.durations,
    }
    with open(out_path, "w", encoding="utf-8") as handle:
        json.dump(report, handle)
    return 0


# ---------------------------------------------------------------------------
# Parent
# ---------------------------------------------------------------------------

def run_group(start, name, ids, timeout):
    with tempfile.TemporaryDirectory(prefix="selftest-") as work:
        out_path = os.path.join(work, "result.json")
        log_path = os.path.join(work, "output.log")
        started = time.monotonic()
        with open(log_path, "w", encoding="utf-8") as log:
            process = subprocess.Popen(
                [sys.executable, os.path.abspath(__file__), "--start", start, "--worker", out_path,
                 *ids],
                stdout=log, stderr=subprocess.STDOUT, stdin=subprocess.DEVNULL,
                start_new_session=True)
            try:
                code = process.wait(timeout=timeout)
            except subprocess.TimeoutExpired:
                code = None
            finally:
                try:
                    os.killpg(process.pid, 9)
                except ProcessLookupError:
                    pass
                if code is None:
                    process.wait()
        elapsed = round(time.monotonic() - started, 2)
        with open(log_path, encoding="utf-8", errors="replace") as log:
            output = log.read()
        report = None
        if os.path.exists(out_path):
            with open(out_path, encoding="utf-8") as handle:
                report = json.load(handle)
    if code is None:
        problem = "timed out after %ss" % timeout
    elif report is None:
        problem = "worker exited %s without a result" % code
    else:
        problem = None
    return {"group": name, "ids": ids, "seconds": elapsed, "report": report,
            "problem": problem, "output": output[-4000:]}


def main(argv=None):
    parser = argparse.ArgumentParser(description="Parallel, tiered quality self-tests.")
    parser.add_argument("--start", default=DEFAULT_START, help="test directory")
    parser.add_argument("--pattern", default="test*.py")
    parser.add_argument("--tier", choices=TIERS, default="fast")
    parser.add_argument("--jobs", type=int, default=max(1, os.cpu_count() or 1))
    parser.add_argument("--timeout", type=float, default=120,
                        help="seconds one test class may take before its worker is killed")
    parser.add_argument("--test-budget", type=float, default=5.0,
                        help="seconds one fast-tier test should take at most")
    parser.add_argument("--chunk", type=int, default=4,
                        help="most tests of one class in one worker (default 4)")
    parser.add_argument("--list", action="store_true", help="list the selected tests and exit")
    parser.add_argument("--json", help="write the run's results here")
    parser.add_argument("--worker", help=argparse.SUPPRESS)
    parser.add_argument("--discover", help=argparse.SUPPRESS)
    parser.add_argument("ids", nargs="*", help=argparse.SUPPRESS)
    args = parser.parse_args(argv)
    start = os.path.abspath(args.start)
    if args.worker:
        return run_worker(start, args.ids, args.worker)
    if args.discover:
        return run_discovery(start, args.pattern, args.discover)
    if args.jobs < 1:
        print("selftest: --jobs must be at least 1", file=sys.stderr)
        return 2

    try:
        found = discover_in_subprocess(start, args.pattern, args.timeout)
    except (RuntimeError, subprocess.TimeoutExpired) as error:
        print("selftest: %s" % error, file=sys.stderr)
        return 1
    real = select(found["tests"], args.tier)
    if args.list:
        for test in real:
            print(test["id"])
        return 0
    if not real:
        print("selftest: zero tests selected (tier %s) -- nothing is certified" % args.tier,
              file=sys.stderr)
        return 1

    groups = group_by_class(real, max(1, args.chunk))
    started = time.monotonic()
    results = []
    with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as pool:
        futures = [pool.submit(run_group, start, name, ids, args.timeout)
                   for name, ids in sorted(groups.items())]
        for future in concurrent.futures.as_completed(futures):
            results.append(future.result())
    elapsed = time.monotonic() - started
    results.sort(key=lambda r: r["group"])

    ran = failures = errors = skipped = 0
    durations = {}
    problems = []
    ran += found["generated_ran"]
    skipped += len(found["generated_skips"])
    for ident, text in found["generated_errors"]:
        errors += 1
        print("ERROR: %s\n%s" % (ident, text))
    for ident, reason in found["generated_skips"]:
        print("SKIPPED MODULE: %s (%s)" % (ident, reason))
    for r in results:
        if r["problem"]:
            problems.append(r)
            continue
        report = r["report"]
        ran += report["ran"]
        failures += len(report["failures"])
        errors += len(report["errors"]) + len(report["unexpected_successes"])
        skipped += report["skipped"]
        durations.update(report["durations"])
        for kind in ("failures", "errors"):
            for ident, text in report[kind]:
                print("%s: %s\n%s" % (kind[:-1].upper() if kind == "errors" else "FAIL", ident, text))
        for ident in report["unexpected_successes"]:
            print("UNEXPECTED SUCCESS: %s" % ident)
    for r in problems:
        print("WORKER %s: %s\n%s" % (r["group"], r["problem"], r["output"]))

    over = []
    if args.tier == "fast":
        over = sorted(((s, i) for i, s in durations.items() if s > args.test_budget), reverse=True)
    slowest = sorted(((s, i) for i, s in durations.items()), reverse=True)[:5]
    print("\nslowest tests:")
    for seconds, ident in slowest:
        print("  %6.2fs  %s" % (seconds, ident))
    for seconds, ident in over:
        print("OVER BUDGET %.2fs > %.1fs: %s (speed it up or mark it @slow)"
              % (seconds, args.test_budget, ident))

    ok = not failures and not errors and not problems and ran > 0
    print("\nselftest (%s tier): %d test(s) in %d group(s) on %d job(s), %.1fs: %d failure(s), "
          "%d error(s), %d worker problem(s), %d skipped, %d over budget -> %s"
          % (args.tier, ran, len(groups), args.jobs, elapsed, failures, errors, len(problems),
             skipped, len(over), "PASS" if ok else "FAIL"))
    if args.json:
        with open(args.json, "w", encoding="utf-8") as handle:
            json.dump({"tier": args.tier, "ran": ran, "failures": failures, "errors": errors,
                       "worker_problems": [r["group"] for r in problems], "skipped": skipped,
                       "seconds": round(elapsed, 2), "durations": durations,
                       "over_budget": [i for _, i in over]}, handle, indent=2)
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
