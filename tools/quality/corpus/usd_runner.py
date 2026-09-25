#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
#
# USD test modules under the pinned OpenUSD Python, with no silent skips
# (RFC 0005 Q0 gap corpus, suites corpus.usd.*).
#
# The USD front end's tests (tools/quality/tests/test_usd_*.py) need `pxr`,
# which only the pinned toolchain Python has; under any other Python they raise
# unittest.SkipTest, so a plain `unittest` run reports OK without testing
# anything. This runner starts the toolchain's `usd_python` with its
# `usd_pythonpath`, runs the named modules there, and reports one checks-v1
# check per test. A skip (module-level or per test), an import error, a
# setUpClass error, an expected failure or unexpected success, a module with
# zero tests, and an interpreter that does not report the pinned OpenUSD
# version are all failures.
#
#   python3 tools/quality/corpus/usd_runner.py tools.quality.tests.test_usd_scene
#
# Toolchain (the pbrt map toolchain file, schema pbrt-map-toolchain/v1, whose
# `usd_python` and `usd_pythonpath` are used; nothing else in it is needed):
#   --toolchain PATH, else $SOURCE_USD_TOOLCHAIN (used exactly; no fallback),
#   else build/toolchains/pbrt-map-toolchain.json of this checkout, else that of
#   the main worktree when this is a linked `git worktree` (the ignored build/
#   directory exists only in the main tree). Relative paths inside it resolve
#   against the checkout that holds it. `--usd-python P --usd-pythonpath PP`
#   bypasses the file (the no-pxr negative control uses it).
#
# The pin comes from quality/product_profiles/pbrt-map-linux-tools.json and the
# OpenUSD host profile it links (tag -> "0.<major>.<minor>" as
# pxr.Usd.GetVersion() reports it, as pbrt_map_toolchain.py check compares) and
# the profile's Python major.minor (pxr's extension modules are built for it).
#
# `--override name=path` replaces a gap-corpus fixture (seeded negative
# controls); any ambient $SOURCE_USD_CORPUS_OVERRIDES is removed otherwise.
#
# ============================================================================

import argparse
import json
import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
QUALITY = os.path.dirname(HERE)
ROOT = os.path.dirname(os.path.dirname(QUALITY))
RESULT_MARKER = "USD_RUNNER_RESULT "
TOOLCHAIN_ENV = "SOURCE_USD_TOOLCHAIN"
OVERRIDE_ENV = "SOURCE_USD_CORPUS_OVERRIDES"
TOOLCHAIN_SCHEMA = "pbrt-map-toolchain/v1"
TOOLCHAIN_RELATIVE = os.path.join("build", "toolchains", "pbrt-map-toolchain.json")
PIN_PROFILE = os.path.join(ROOT, "quality", "product_profiles", "pbrt-map-linux-tools.json")
# Environment that would change what the child Python imports.
SCRUBBED_ENV = ("PYTHONPATH", "PYTHONHOME", "PYTHONSTARTUP", "PYTHONUSERBASE",
                "PYTHONSAFEPATH", "PYTHONINSPECT", "PYTHONOPTIMIZE")
DETAIL_LIMIT = 400


# ------------------------------------------------------------------ child
# Runs under the USD Python: standard library and unittest only.

def one_line(text):
    text = " ".join(str(text).split())
    return text if len(text) <= DETAIL_LIMIT else text[:DETAIL_LIMIT - 3] + "..."


def exception_line(err):
    import traceback
    return one_line(traceback.format_exception_only(err[0], err[1])[-1])


def child(modules):
    import importlib
    import traceback
    import unittest

    class Recorder(unittest.TestResult):
        """Every test's outcome, in run order; the worst outcome wins."""
        RANK = {"pass": 0, "skip": 1, "expected-failure": 2, "unexpected-success": 2,
                "fail": 3, "error": 4}

        def __init__(self):
            super().__init__()
            self.records = []
            self.index = {}

        def record(self, test, outcome, detail=""):
            key = test.id()
            if key not in self.index:
                self.index[key] = len(self.records)
                self.records.append({"id": key, "outcome": outcome, "detail": detail})
                return
            entry = self.records[self.index[key]]
            if self.RANK[outcome] > self.RANK[entry["outcome"]]:
                entry.update(outcome=outcome, detail=detail)

        def addSuccess(self, test):
            super().addSuccess(test)
            self.record(test, "pass")

        def addFailure(self, test, err):
            super().addFailure(test, err)
            self.record(test, "fail", exception_line(err))

        def addError(self, test, err):
            super().addError(test, err)
            self.record(test, "error", exception_line(err))

        def addSkip(self, test, reason):
            super().addSkip(test, reason)
            self.record(test, "skip", one_line(reason))

        def addExpectedFailure(self, test, err):
            super().addExpectedFailure(test, err)
            self.record(test, "expected-failure", exception_line(err))

        def addUnexpectedSuccess(self, test):
            super().addUnexpectedSuccess(test)
            self.record(test, "unexpected-success", "marked expectedFailure but passed")

        def addSubTest(self, test, subtest, err):
            super().addSubTest(test, subtest, err)
            if err is not None:
                failed = issubclass(err[0], test.failureException)
                self.record(test, "fail" if failed else "error",
                            "%s: %s" % (subtest.id()[len(test.id()):].strip(),
                                        exception_line(err)))

    report = {"executable": sys.executable, "version": sys.version.split()[0],
              "version_info": list(sys.version_info[:3]), "openusd": None,
              "openusd_error": None, "numpy": None, "modules": []}
    try:
        from pxr import Usd
        report["openusd"] = "%d.%d.%d" % tuple(Usd.GetVersion())
    except Exception as error:  # reported, and counted by the parent
        report["openusd_error"] = one_line("%s: %s" % (type(error).__name__, error))
    try:
        import numpy
        report["numpy"] = "%s (%s)" % (numpy.__version__, os.path.dirname(numpy.__file__))
    except Exception:
        pass
    sys.path.insert(0, ROOT)
    for spec in modules:
        name, _, case = spec.partition(":")
        entry = {"module": name, "case": case or None, "load": "ok", "detail": "", "tests": []}
        try:
            module = importlib.import_module(name)
        except unittest.SkipTest as skip:
            entry.update(load="skipped", detail=one_line(skip))
        except BaseException:
            entry.update(load="error", detail=one_line(traceback.format_exc().splitlines()[-1]))
        else:
            loader = unittest.TestLoader()
            if case and not isinstance(getattr(module, case, None), type):
                entry.update(load="error", detail="%s has no test case class %s" % (name, case))
                report["modules"].append(entry)
                continue
            suite = loader.loadTestsFromTestCase(getattr(module, case)) if case \
                else loader.loadTestsFromModule(module)
            if loader.errors:
                entry.update(load="error", detail=one_line(loader.errors[0].splitlines()[-1]))
            recorder = Recorder()
            suite.run(recorder)
            entry["tests"] = recorder.records
        report["modules"].append(entry)
    sys.stdout.write("\n" + RESULT_MARKER + json.dumps(report) + "\n")
    sys.stdout.flush()
    return 0


# ----------------------------------------------------------------- parent

def main_worktree_root():
    """The main checkout of a linked git worktree, or None."""
    try:
        out = subprocess.run(["git", "-C", ROOT, "rev-parse", "--path-format=absolute",
                              "--git-common-dir"], capture_output=True, text=True, timeout=30)
    except (OSError, subprocess.SubprocessError):
        return None
    common = out.stdout.strip() if out.returncode == 0 else ""
    if not common or os.path.basename(common) != ".git":
        return None
    main = os.path.dirname(common)
    return None if os.path.realpath(main) == os.path.realpath(ROOT) else main


def toolchain_candidates(args):
    if args.toolchain:
        return [(args.toolchain, "--toolchain")]
    if os.environ.get(TOOLCHAIN_ENV):
        return [(os.environ[TOOLCHAIN_ENV], "$" + TOOLCHAIN_ENV)]
    candidates = [(os.path.join(ROOT, TOOLCHAIN_RELATIVE), "this checkout's build/toolchains")]
    main = main_worktree_root()
    if main:
        candidates.append((os.path.join(main, TOOLCHAIN_RELATIVE),
                           "main worktree's build/toolchains"))
    return candidates


def resolve_interpreter(args):
    """(usd_python, usd_pythonpath, origin, problems)."""
    if args.usd_python is not None:
        return args.usd_python, args.usd_pythonpath or "", {"source": "--usd-python"}, []
    candidates = toolchain_candidates(args)
    path, source = next(((p, s) for p, s in candidates if os.path.isfile(p)), candidates[0])
    origin = {"source": source, "toolchain": os.path.abspath(path),
              "searched": [p for p, _ in candidates]}
    if not os.path.isfile(path):
        return None, None, origin, ["no toolchain file (searched %s); provision it with "
                                    "tools/quality/pbrt_map_toolchain.py provision or set $%s"
                                    % (", ".join(p for p, _ in candidates), TOOLCHAIN_ENV)]
    try:
        with open(path, "r", encoding="utf-8") as stream:
            toolchain = json.load(stream)
    except (OSError, ValueError) as error:
        return None, None, origin, ["%s is unreadable: %s" % (path, error)]
    problems = []
    if toolchain.get("schema") != TOOLCHAIN_SCHEMA:
        problems.append("%s schema is %r, expected %r" % (path, toolchain.get("schema"),
                                                          TOOLCHAIN_SCHEMA))
    checkout = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(path))))
    values = {}
    for key in ("usd_python", "usd_pythonpath"):
        value = toolchain.get(key)
        if not value:
            problems.append("%s: %s missing" % (path, key))
            continue
        values[key] = value if os.path.isabs(value) else os.path.join(checkout, value)
        if not os.path.exists(values[key]):
            problems.append("%s: %s %s does not exist" % (path, key, values[key]))
    return values.get("usd_python"), values.get("usd_pythonpath"), origin, problems


def load_pin(profile_path):
    sys.path.insert(0, QUALITY)
    import pbrt_map_toolchain
    profile, linked = pbrt_map_toolchain.load_profiles(profile_path)
    openusd = linked["openusd"]["dependencies"]["openusd"]
    python = profile["toolchain"]["python"]
    return {"profile": profile["id"], "openusd_tag": openusd["tag"],
            "openusd_revision": openusd["revision"],
            "openusd_version": "0." + openusd["tag"].lstrip("v"),
            "python": python, "python_abi": ".".join(python.split(".")[:2]),
            "usd_python": profile["toolchain"].get("usd_python")}


def child_environment(pythonpath, overrides):
    env = dict(os.environ)
    for name in SCRUBBED_ENV + (OVERRIDE_ENV,):
        env.pop(name, None)
    if pythonpath:
        env["PYTHONPATH"] = pythonpath
    # The user site stays enabled: on the reference host the USD Python's
    # numpy (which usd_scene needs) is installed only there and is not pinned
    # by any profile; its version and path are recorded in the provenance.
    env.pop("PYTHONNOUSERSITE", None)
    env["PYTHONDONTWRITEBYTECODE"] = "1"  # no __pycache__ in the checkout
    if overrides:
        env[OVERRIDE_ENV] = ";".join(overrides)
    return env


def module_file(spec):
    return os.path.join(ROOT, *spec.partition(":")[0].split(".")) + ".py"


def main():
    parser = argparse.ArgumentParser(
        description="Run USD test modules under the pinned OpenUSD Python; skips fail.")
    parser.add_argument("modules", nargs="*", metavar="MODULE[:Class]",
                        help="dotted test modules, e.g. tools.quality.tests.test_usd_scene, "
                             "optionally limited to one TestCase class")
    parser.add_argument("--child", action="store_true", help=argparse.SUPPRESS)
    parser.add_argument("--toolchain", help="pbrt map toolchain file (see the header)")
    parser.add_argument("--usd-python", help="interpreter to use instead of the toolchain's")
    parser.add_argument("--usd-pythonpath", help="PYTHONPATH for --usd-python (may be empty)")
    parser.add_argument("--pin-profile", default=PIN_PROFILE,
                        help="host tool profile that owns the pins (default: %(default)s)")
    parser.add_argument("--override", action="append", default=[], metavar="NAME=PATH",
                        help="replace a gap-corpus fixture (seeded negative controls)")
    parser.add_argument("--timeout", type=float, default=300.0)
    args = parser.parse_args()
    if args.child:
        return child(args.modules)
    if not args.modules:
        parser.error("name at least one test module")
    if args.usd_pythonpath is not None and args.usd_python is None:
        parser.error("--usd-pythonpath needs --usd-python")

    sys.path.insert(0, QUALITY)
    from conformance_result import Checks
    from corpus import provenance

    checks = Checks()
    pin = load_pin(args.pin_profile)
    usd_python, pythonpath, origin, problems = resolve_interpreter(args)
    checks.check(not problems, "toolchain.resolved", "; ".join(problems))
    report, run_detail = None, ""
    if not problems:
        command = [usd_python, os.path.abspath(__file__), "--child"] + args.modules
        try:
            out = subprocess.run(command, cwd=ROOT, capture_output=True, text=True,
                                 env=child_environment(pythonpath, args.override),
                                 timeout=args.timeout)
            lines = out.stdout.splitlines()
            payload = [ln[len(RESULT_MARKER):] for ln in lines if ln.startswith(RESULT_MARKER)]
            # The child's own output is diagnostics, never result lines.
            sys.stderr.write("".join(ln + "\n" for ln in lines
                                     if not ln.startswith(RESULT_MARKER)) + out.stderr)
            if len(payload) == 1:
                report = json.loads(payload[0])
            else:
                run_detail = "exit %d, %d result records; stderr: %s" % (
                    out.returncode, len(payload), one_line(out.stderr[-DETAIL_LIMIT:]))
        except (OSError, subprocess.SubprocessError, ValueError) as error:
            run_detail = one_line(error)
        checks.check(report is not None, "usd-python.completed", run_detail)

    counts = {}
    for entry in (report or {}).get("modules", []):
        name = entry["module"]
        short = name.rsplit(".", 1)[-1] + (":" + entry["case"] if entry.get("case") else "")
        load_detail = {"skipped": "skipped at import (a skip is a failure here): ",
                       "error": "import/load error: "}.get(entry["load"], "") + entry["detail"]
        checks.check(entry["load"] == "ok", short + ".load", load_detail)
        for test in entry["tests"]:
            label = test["id"][len(name) + 1:] if test["id"].startswith(name + ".") \
                else test["id"]
            detail = test["outcome"] + (": " + test["detail"] if test["detail"] else "")
            checks.check(test["outcome"] == "pass", "%s.%s" % (name.rsplit(".", 1)[-1], label),
                         detail)
        counts[short] = {outcome: sum(t["outcome"] == outcome for t in entry["tests"])
                        for outcome in ("pass", "fail", "error", "skip", "expected-failure",
                                        "unexpected-success")}
        checks.check(len(entry["tests"]) > 0, short + ".tests-ran",
                     "zero tests ran" if entry["load"] == "ok" else "module did not load")

    if report is not None:
        checks.check(report["openusd"] is not None, "openusd.import",
                     report["openusd_error"] or "")
        checks.equal(report["openusd"], pin["openusd_version"], "openusd.version")
        checks.equal(".".join(map(str, report["version_info"][:2])), pin["python_abi"],
                     "python.abi")
        checks.equal(os.path.realpath(report["executable"]), os.path.realpath(usd_python),
                     "python.executable")

    inputs = {"pin_profile": args.pin_profile, "usd_scene": os.path.join(QUALITY, "usd_scene.py")}
    for name in args.modules:
        inputs["module:" + name] = module_file(name)
    for directory in (os.path.join(ROOT, "quality/fixtures/usd-maps/corpus"),):
        for base, _, files in os.walk(directory):
            for filename in sorted(files):
                path = os.path.join(base, filename)
                inputs["fixture:" + os.path.relpath(path, ROOT)] = path
    for entry in args.override:
        inputs["override:" + entry.partition("=")[0]] = os.path.join(
            ROOT, entry.partition("=")[2])
    if origin.get("toolchain") and os.path.isfile(origin["toolchain"]):
        inputs["toolchain"] = origin["toolchain"]
    provenance.emit(provenance.collect(
        os.environ.get("CONFORMANCE_SUITE", "usd-runner"),
        tools={"openusd": (report or {}).get("openusd"),
               "openusd_pin": "%s (%s)" % (pin["openusd_tag"], pin["openusd_revision"]),
               "usd_python": usd_python, "usd_python_version": (report or {}).get("version"),
               "usd_pythonpath": pythonpath, "numpy": (report or {}).get("numpy")},
        inputs=inputs,
        extra={"usd_interpreter": origin, "pin": pin, "overrides": args.override,
               "test_counts": counts}))
    return checks.report()


if __name__ == "__main__":
    sys.exit(main())
