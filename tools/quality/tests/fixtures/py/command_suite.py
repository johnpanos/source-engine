#!/usr/bin/env python3
# Command-suite fixture for the conformance runner self-tests. The mode argument
# selects the behavior under test; each non-pass mode is a defect the runner
# must reject.
import os
import sys
import time

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "../../.."))
from conformance_result import Checks, report_conformance  # noqa: E402

mode = sys.argv[1]
out = sys.argv[2] if len(sys.argv) > 2 else ""
checks = Checks()
if mode == "pass":
    checks.equal(1 + 1, 2, "arithmetic")
    # The scratch directory exists, is empty and is shared with the environment.
    checks.check(os.path.isdir(out) and not os.listdir(out), "scratch-empty")
    checks.equal(os.environ.get("CONFORMANCE_OUT"), out, "scratch-env")
    checks.equal(os.getcwd(), os.environ.get("EXPECT_CWD", os.getcwd()), "cwd-is-root")
    with open(os.path.join(out, "leftover"), "w") as stream:
        stream.write("x")
    sys.exit(checks.report())
if mode == "failing":
    checks.equal(1 + 1, 3, "arithmetic")
    sys.exit(checks.report())
if mode == "zero":
    sys.exit(checks.report())
if mode == "exit0-with-failure":
    checks.equal(1, 2, "counted")
    checks.report()
    sys.exit(0)
if mode == "missing-record":
    sys.exit(0)
if mode == "duplicate-record":
    checks.equal(1, 1, "one")
    report_conformance(1, 0)
    sys.exit(checks.report())
if mode == "raise":
    checks.equal(1, 1, "one")
    raise RuntimeError("raised after a passing check")
if mode == "hang":
    print("progress before hanging", flush=True)
    time.sleep(60)
sys.exit("unknown mode %r" % mode)
