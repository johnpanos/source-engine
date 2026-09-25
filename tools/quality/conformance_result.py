#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
#
# The `checks-v1` terminal result record for command suites (RFC 0005 Q1).
#
# Python twin of public/testing/conformance_result.h. A command suite in
# quality/conformance.manifest.json ends by calling Checks.report(), which
# prints exactly one `CONFORMANCE <checks> <failures>` line and returns the
# matching exit status. The runner accepts a run only when the process exits 0
# AND stdout carries exactly one record with a nonzero check count and zero
# failures, so a script that returns early, raises after printing progress or
# evaluates nothing fails even with exit status 0.
#
# Checks are counted calls, never `assert` statements: `python -O` removes
# those, and the runner rejects command suites whose Python sources use them.
# Python 3 standard library only.
#
# ============================================================================

import sys


class Checks:
    """Counted checks with a one-line reason per failure."""

    def __init__(self, stream=None):
        self.checks = 0
        self.failures = 0
        self.stream = stream or sys.stdout

    def check(self, condition, name, detail=""):
        """Count one check; print `FAIL <name>: <detail>` when it does not hold."""
        self.checks += 1
        if not condition:
            self.failures += 1
            print("FAIL %s%s" % (name, (": " + detail) if detail else ""), file=self.stream)
        return bool(condition)

    def equal(self, actual, expected, name):
        return self.check(actual == expected, name,
                          "expected %r, got %r" % (expected, actual))

    def within(self, value, low, high, name):
        """low <= value <= high; either bound may be None."""
        ok = value is not None and (low is None or value >= low) and \
            (high is None or value <= high)
        return self.check(ok, name, "%r outside [%r, %r]" % (value, low, high))

    def report(self):
        return report_conformance(self.checks, self.failures, self.stream)


def report_conformance(checks, failures, stream=None):
    """Print the record and return the matching process exit status."""
    stream = stream or sys.stdout
    print("CONFORMANCE %d %d" % (checks, failures), file=stream)
    stream.flush()
    return 0 if checks > 0 and failures == 0 else 1
