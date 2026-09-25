#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
#
# Waf toolchain-recorder fixtures (RFC 0005 Q0 gap corpus, RFC 0006 M0 /
# roadmap R03).
#
# scripts/waifulib/toolchain_dialect.py writes <out>/toolchain-invocations.json,
# the evidence tools/quality/toolchain_boundary.py certifies: the final argv of
# every C/C++ compile with its target and dialect, and the policy.json digest
# it was recorded with. This script builds tiny Waf projects
# (quality/fixtures/corpus/toolchain/<project>/) with the repository's own
# ./waf, the REAL recorder and the real toolchain_policy.py, in a private copy
# under the scratch directory, and checks the recorded evidence after each
# step against an oracle that does not use the recorder:
#
#   * The compilers are wrapper scripts that log every argv they run, its
#     working directory and exit status. The object files on disk are modelled
#     from that log: an output's evidence is the argv of its last compile, and
#     it has none if that compile failed or never ran.
#   * A record is truthful when it lists every modelled object exactly once
#     with that argv, claims no object that was not compiled, attributes each
#     entry to the language of the compiler that ran it, and its configure-time
#     probe list belongs to the policy it claims.
#   * The record is then fed to the real checker (toolchain_boundary's
#     verify_invocations, the function `check` runs). Evidence must be
#     truthful, or the checker must reject it. A record that is untruthful AND
#     accepted is the defect the gate cannot see.
#
# For project c_in_cxx the expected language, dialect and standard flag of
# each source are also declared independently below (EXPECTED_C_IN_CXX).
#
# Scenarios (one manifest row each; --seed doctors the first truthful record
# in memory for the negative rows):
#
#   c-in-cxx            (a) .c sources in cxx-only task generators (engine forms)
#   c-extension-map     (a) a '.C' source in a c-only target (Waf compiles C++)
#   build-lifecycle     (b) install before any build, build after install, no-op
#   install-after-build (b) `waf install` recompiles; the record is not rewritten
#   partial-targets     (b) `waf build --targets` records targets it did not build
#   failed-build        (b) a failed build leaves a record claiming its compiles
#   policy-change       (c) stale digest flagged, rebuild refreshes, a cosmetic
#                           policy edit, verify-mode drift fails loudly
#   policy-clangdb-only (c) `waf clangdb` refreshes the digest without compiling
#   policy-probe-stale  (c) the probe list stays from configure under a new policy
#
# Usage:
#   toolchain_recorder.py --scenario NAME [--seed FAULT] [--out DIR]
#   toolchain_recorder.py --list
#
# Python 3 standard library only. No assert statements (checks-v1).
#
# ============================================================================

import argparse
import copy
import hashlib
import json
import os
import shutil
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
QUALITY = os.path.dirname(HERE)
sys.path.insert(0, QUALITY)
sys.path.insert(0, HERE)

import provenance  # noqa: E402
import toolchain_boundary  # noqa: E402
import toolchain_policy as tp  # noqa: E402
from conformance_result import Checks  # noqa: E402

ROOT = provenance.ROOT
FIXTURES = os.path.join(ROOT, "quality", "fixtures", "corpus", "toolchain")
WAF = os.path.join(ROOT, "waf")
POLICY_REL = tp.POLICY_PATH
RECORD = os.path.join("build", "toolchain-invocations.json")
SUITE_PREFIX = "corpus.toolchain.recorder."

# Independent oracle for project c_in_cxx: source -> (target, language of the
# compiler Waf runs, policy dialect, exact standard flags in the argv, flags
# that must be absent). Derived from Waf's extension map (c.py: .c; cxx.py:
# .cpp .cc .cxx .C .c++) and the fixture policy by hand, not by the recorder.
EXPECTED_C_IN_CXX = {
    "tool_main.cpp": ("cxxtool", "c++", "cxx20", ["-std=c++20"], ["-fpermissive"]),
    "lz.c": ("cxxtool", "c", "c-toolchain-default", [], []),
    "nodes/node_c.c": ("cxxnodes", "c", "c-toolchain-default", [], []),
    "nodes/node_cpp.cpp": ("cxxnodes", "c++", "cxx20", ["-std=c++20"], ["-fpermissive"]),
    "shl.cpp": ("cxxshlib", "c++", "cxx20-permissive", ["-std=c++20"], []),
    "shl_c.c": ("cxxshlib", "c", "c-toolchain-default", [], []),
    "kern.c": ("ckernel", "c", "c-fixture-gnu11", ["-std=gnu11"], ["-ffast-math"]),
    "kern_host.cpp": ("ckernel", "c++", "cxx20-permissive", ["-std=c++20"], []),
}
EXPECTED_EXTENSION_MAP = {
    "plain.c": ("cupper", "c", "c-fixture-gnu11", ["-std=gnu11"], []),
    "upper.C": ("cupper", "c++", "cxx20-permissive", ["-std=c++20"], []),
    "upper2.C": ("cxxupper", "c++", "cxx20", ["-std=c++20"], ["-fpermissive"]),
}

WRAPPER = """#!{python}
# Toolchain-recorder fixture compiler wrapper: runs {real} and logs the argv.
import json, os, subprocess, sys
rc = subprocess.call([{real!r}] + sys.argv[1:])
with open({log!r}, "a") as stream:
    stream.write(json.dumps({{"tool": {tool!r}, "argv": sys.argv, "cwd": os.getcwd(),
                              "rc": rc}}) + "\\n")
sys.exit(rc)
"""


def sha256_file(path):
    with open(path, "rb") as stream:
        return hashlib.sha256(stream.read()).hexdigest()


class Project:
    """A private copy of one fixture project with logging compilers."""

    def __init__(self, fixture, out, cc, cxx):
        self.fixture = fixture
        self.dir = os.path.join(out, fixture)
        shutil.rmtree(self.dir, ignore_errors=True)
        shutil.copytree(os.path.join(FIXTURES, fixture), self.dir)
        self.bin = os.path.join(out, "bin-" + fixture)
        os.makedirs(self.bin, exist_ok=True)
        self.log = os.path.join(out, "compiles-%s.jsonl" % fixture)
        open(self.log, "w").close()
        self.transcript = []
        wrappers = {}
        for tool, real in (("cc", cc), ("cxx", cxx)):
            path = os.path.join(self.bin, tool)
            with open(path, "w") as stream:
                stream.write(WRAPPER.format(python=sys.executable, real=shutil.which(real) or real,
                                            log=self.log, tool=tool))
            os.chmod(path, 0o755)
            wrappers[tool] = path
        self.env = dict(os.environ)
        for name in ("WAFLOCK", "CFLAGS", "CXXFLAGS", "CPPFLAGS", "LDFLAGS", "LINKFLAGS"):
            self.env.pop(name, None)
        self.env.update({"TOOLCHAIN_FIXTURE_REPO": ROOT, "CC": wrappers["cc"],
                         "CXX": wrappers["cxx"], "PYTHONDONTWRITEBYTECODE": "1"})
        self.objects = {}  # output (relative to build/) -> last compile
        self.offset = 0
        self.build_dir = os.path.join(self.dir, "build")

    # -- Waf ---------------------------------------------------------------

    def waf(self, *args, targets_env=None):
        env = dict(self.env)
        if targets_env:
            env["TOOLCHAIN_FIXTURE_TARGETS"] = targets_env
        argv = [sys.executable, WAF] + list(args)
        done = subprocess.run(argv, cwd=self.dir, env=env, capture_output=True, text=True,
                              timeout=300)
        output = done.stdout + done.stderr
        self.transcript.append("$ (cd %s && %s)\n%s\n[exit %d]\n"
                               % (self.dir, " ".join(argv[1:]), output, done.returncode))
        return done.returncode, output

    def configure(self):
        rc, output = self.waf("configure", "--prefix=" + os.path.join(self.dir, "prefix"))
        self.consume()  # configuration checks and probes are not build evidence
        self.objects.clear()
        return rc, output

    # -- compile log -------------------------------------------------------

    def consume(self):
        """Compiles logged since the last call, applied to the object model."""
        with open(self.log, "r") as stream:
            stream.seek(self.offset)
            lines = stream.read()
            self.offset = stream.tell()
        ran = []
        for line in lines.splitlines():
            record = json.loads(line)
            argv = record["argv"]
            if "-c" not in argv:
                continue
            output = None
            for i, arg in enumerate(argv):
                if arg == "-o" and i + 1 < len(argv):
                    output = argv[i + 1]
                elif arg.startswith("-o") and len(arg) > 2:
                    output = arg[2:]
            if output is None:
                continue
            output = os.path.normpath(os.path.join(record["cwd"], output))
            relative = os.path.relpath(output, self.build_dir)
            if relative.startswith("..") or ".conf_check" in relative:
                continue
            compile_ = {"argv": argv, "tool": record["tool"], "ok": record["rc"] == 0,
                        "rc": record["rc"], "output": relative}
            self.objects[relative] = compile_
            ran.append(compile_)
        return ran

    # -- evidence ----------------------------------------------------------

    def record(self):
        path = os.path.join(self.dir, RECORD)
        if not os.path.isfile(path):
            return None
        with open(path, "r", encoding="utf-8") as stream:
            return json.load(stream)

    def record_digest(self):
        path = os.path.join(self.dir, RECORD)
        return sha256_file(path) if os.path.isfile(path) else None

    def policy_path(self):
        return os.path.join(self.dir, POLICY_REL)

    def policy(self):
        return tp.load_policy(self.dir)

    def write_policy(self, policy):
        with open(self.policy_path(), "w", encoding="utf-8") as stream:
            json.dump(policy, stream, indent=1)
            stream.write("\n")

    def checker(self, record):
        """The real checker's verdict on `record` (errors; empty = accepted)."""
        if record is None:
            # `toolchain_boundary.py check` exits FATAL on a missing file.
            return ["no %s (toolchain_boundary check: FATAL, file not found)" % RECORD]
        errors, _ = toolchain_boundary.verify_invocations(self.dir, self.policy(), record,
                                                          RECORD)
        return errors


def probed_by(policy):
    """Dialects toolchain_dialect.configure probes under `policy` (gnu family)."""
    return sorted(name for name, entry in policy["dialects"].items()
                  if entry.get("probe") and "gnu" in entry)


def truth_problems(project, record):
    """Where `record` differs from the compiles that produced the objects.
    Returns [(key, text)] in a stable order (target, source)."""
    problems = []
    if record is None:
        return [("record", "no toolchain-invocations.json was written")]
    claimed = {}
    for entry in record.get("entries", []):
        claimed.setdefault(entry.get("output"), []).append(entry)
    keys = sorted(set(claimed) | {o for o, c in project.objects.items()},
                  key=lambda o: (claimed[o][0]["target"], claimed[o][0]["source"])
                  if o in claimed else ("~", o))
    for output in keys:
        entries = claimed.get(output, [])
        compiled = project.objects.get(output)
        name = "%s/%s" % (entries[0]["target"], entries[0]["source"]) if entries else output
        if not entries:
            if compiled and compiled["ok"]:
                problems.append((name, "compiled (%s) but missing from the record"
                                 % compiled["tool"]))
            continue
        if len(entries) > 1:
            problems.append((name, "recorded %d times" % len(entries)))
        entry = entries[0]
        if compiled is None:
            problems.append((name, "claimed, but no compile of it ever ran"))
            continue
        if not compiled["ok"]:
            problems.append((name, "claimed, but its last compile failed (exit %d)"
                             % compiled["rc"]))
            continue
        language = "c++" if compiled["tool"] == "cxx" else "c"
        if entry.get("language") != language:
            problems.append((name, "recorded as %s but the %s compiler ran it"
                             % (entry.get("language"), language)))
        if entry.get("arguments") != compiled["argv"]:
            recorded, ran = entry.get("arguments") or [], compiled["argv"]
            only_rec = [a for a in recorded if a not in ran]
            only_ran = [a for a in ran if a not in recorded]
            problems.append((name, "recorded argv differs from the compile that built it "
                             "(recorded only %s; executed only %s)" % (only_rec, only_ran)))
    policy = project.policy()
    current = sha256_file(project.policy_path())
    if record.get("policy_sha256") == current:
        recorded = sorted(record.get("toolchain", {}).get("probed_dialects") or [])
        if recorded != probed_by(policy):
            problems.append(("toolchain.probed_dialects",
                             "record claims policy %s but carries configure-time probes %s; "
                             "that policy probes %s" % (current[:12], recorded, probed_by(policy))))
    return problems


def expected_problems(project, record, expected):
    """The independent per-source oracle (project c_in_cxx)."""
    problems = []
    policy = project.policy()
    by_source = {e["source"]: e for e in (record or {}).get("entries", [])}
    for source, (target, language, dialect, std, absent) in sorted(expected.items()):
        name = "%s/%s" % (target, source)
        entry = by_source.get(source)
        if entry is None:
            problems.append((name, "not recorded"))
            continue
        if entry.get("target") != target:
            problems.append((name, "recorded under target %r" % entry.get("target")))
        if entry.get("language") != language:
            problems.append((name, "recorded language %r, Waf compiles it as %s"
                             % (entry.get("language"), language)))
        if entry.get("dialect") != dialect:
            problems.append((name, "recorded dialect %r, policy selects %r for its %s compile"
                             % (entry.get("dialect"), dialect, language)))
        found = [a for a in entry.get("arguments", []) if a.startswith("-std=")]
        if found != std:
            problems.append((name, "standard flags %s, expected %s" % (found, std)))
        bad = [a for a in absent if a in entry.get("arguments", [])]
        if bad:
            problems.append((name, "carries %s" % bad))
        if tp.target_dialect(policy, target, language) != dialect:
            problems.append((name, "fixture oracle disagrees with the fixture policy"))
    return problems


# ---------------------------------------------------------------------------
# Seeded faults (negative rows): doctor the first truthful record in memory.
# ---------------------------------------------------------------------------

def seed_record(record, fault):
    record = copy.deepcopy(record)
    entries = record["entries"]

    def find(source):
        return next(e for e in entries if e["source"] == source)

    if fault == "misattribute-c":
        entry = find("lz.c")
        entry["language"], entry["dialect"] = "c++", "cxx20"
    elif fault == "wrong-std":
        entry = entries[0]
        entry["arguments"] = [("-std=c++17" if a.startswith("-std=") else a)
                              for a in entry["arguments"]]
        if not any(a.startswith("-std=") for a in entry["arguments"]):
            entry["arguments"].insert(1, "-std=c++17")
    elif fault == "drop-entry":
        del entries[-1]
    elif fault == "phantom-entry":
        ghost = copy.deepcopy(entries[0])
        ghost["source"] = "ghost.cpp"
        ghost["output"] = "ghost.cpp.9.o"
        entries.append(ghost)
    elif fault == "policy-digest":
        record["policy_sha256"] = "0" * 64
    else:
        raise ValueError("unknown seed %r" % fault)
    return record


# ---------------------------------------------------------------------------
# Scenario engine
# ---------------------------------------------------------------------------

class Run:
    def __init__(self, scenario, checks, seed):
        self.scenario = scenario
        self.checks = checks
        self.seed = seed
        self.seeded = False
        self.steps = []

    def name(self, step, what):
        return "%s%s.%s.%s" % (SUITE_PREFIX, self.scenario, step, what)

    def waf_ok(self, step, rc, output, want_ok=True):
        ok = (rc == 0) == want_ok
        tail = " | ".join(output.strip().splitlines()[-3:])
        self.checks.check(ok, self.name(step, "waf"), "expected %s, exit %d: %s"
                          % ("success" if want_ok else "failure", rc, tail))

    def evidence(self, project, step, expect, reject_text=None, expected=None):
        """Judge the record after `step`.

        expect: 'truthful'  record matches the compiles and the checker accepts it;
                'rejected'  the checker must reject it (containing reject_text);
                'truthful-or-rejected'  untruthful evidence must not be accepted.
        """
        record = project.record()
        if self.seed and not self.seeded and expect == "truthful" and record is not None:
            record = seed_record(record, self.seed)
            self.seeded = True
            print("SEEDED %s into the record judged at step %s" % (self.seed, step))
        problems = truth_problems(project, record)
        if expected:
            problems += expected_problems(project, record, expected)
        errors = project.checker(record)
        self.steps.append({"step": step, "expect": expect, "problems": problems,
                           "checker_errors": errors[:20],
                           "entries": len((record or {}).get("entries", [])),
                           "record_policy_sha256": (record or {}).get("policy_sha256"),
                           "policy_sha256": sha256_file(project.policy_path())})
        print("step %s: %d entries, %d truth problem(s), checker %s"
              % (step, len((record or {}).get("entries", [])), len(problems),
                 "rejects (%d)" % len(errors) if errors else "accepts"))
        if expect == "truthful":
            if not problems:
                self.checks.check(True, self.name(step, "evidence"))
            for key, text in problems:
                self.checks.check(False, self.name(step, "evidence"), "%s: %s" % (key, text))
            # The checker must agree with the oracle: accept truthful evidence
            # and reject untruthful evidence.
            if problems and not errors:
                self.checks.check(False, self.name(step, "checker"),
                                  "toolchain_boundary accepted untruthful evidence")
            else:
                self.checks.check(not errors or problems, self.name(step, "checker"),
                                  "toolchain_boundary rejected evidence the oracle found "
                                  "truthful: %s" % (errors[0] if errors else ""))
        elif expect == "rejected":
            hit = [e for e in errors if reject_text is None or reject_text in e]
            self.checks.check(bool(hit), self.name(step, "rejected"),
                              "toolchain_boundary accepted the record; expected a rejection "
                              "containing %r (truth problems: %s)"
                              % (reject_text, "; ".join("%s: %s" % p for p in problems[:3])))
        elif expect == "truthful-or-rejected":
            if errors or not problems:
                self.checks.check(True, self.name(step, "accepted"))
            for key, text in (problems if not errors else []):
                self.checks.check(False, self.name(step, "accepted"),
                                  "toolchain_boundary accepted untruthful evidence: %s: %s"
                                  % (key, text))
        return record, problems, errors

    def compiled(self, step, ran, want, label):
        got = sorted(c["output"] for c in ran)
        self.checks.check(got == sorted(want), self.name(step, "compiled"),
                          "%s: expected compiles of %s, observed %s" % (label, sorted(want), got))


# -- (a) --------------------------------------------------------------------

def scenario_c_in_cxx(run, out, args):
    project = Project("c_in_cxx", out, args.cc, args.cxx)
    rc, output = project.configure()
    run.waf_ok("configure", rc, output)
    rc, output = project.waf("build", "-j1", targets_env="common")
    run.waf_ok("build", rc, output)
    ran = project.consume()
    run.checks.equal(len(ran), len(EXPECTED_C_IN_CXX), run.name("build", "compile-count"))
    run.evidence(project, "build", "truthful", expected=EXPECTED_C_IN_CXX)
    return project


def scenario_c_extension_map(run, out, args):
    project = Project("c_in_cxx", out, args.cc, args.cxx)
    rc, output = project.configure()
    run.waf_ok("configure", rc, output)
    rc, output = project.waf("build", "-j1", targets_env="extension-map")
    run.waf_ok("build", rc, output)
    ran = project.consume()
    languages = sorted((os.path.basename(c["output"]).split(".")[0], c["tool"]) for c in ran)
    run.checks.equal(languages, [("plain", "cc"), ("upper", "cxx"), ("upper2", "cxx")],
                     run.name("build", "waf-extension-map"))
    run.evidence(project, "build", "truthful", expected=EXPECTED_EXTENSION_MAP)
    return project


# -- (b) --------------------------------------------------------------------

def built_fresh(run, project, step="build"):
    rc, output = project.configure()
    run.waf_ok("configure", rc, output)
    rc, output = project.waf("build", "-j1")
    run.waf_ok(step, rc, output)
    run.compiled(step, project.consume(), ["alpha.cpp.1.o", "beta.cpp.2.o"], "first build")
    run.evidence(project, step, "truthful")


def scenario_build_lifecycle(run, out, args):
    project = Project("install_record", out, args.cc, args.cxx)
    rc, output = project.configure()
    run.waf_ok("configure", rc, output)
    # `waf install` with no earlier build compiles everything and writes nothing.
    rc, output = project.waf("install", "-j1")
    run.waf_ok("install-first", rc, output)
    run.compiled("install-first", project.consume(), ["alpha.cpp.1.o", "beta.cpp.2.o"],
                 "install compiles")
    run.evidence(project, "install-first", "rejected", reject_text="no build/toolchain")
    # `waf build` afterwards compiles nothing; the dry-run record must equal
    # what install compiled.
    rc, output = project.waf("build", "-j1")
    run.waf_ok("build-after-install", rc, output)
    run.compiled("build-after-install", project.consume(), [], "up-to-date build")
    run.evidence(project, "build-after-install", "truthful")
    before = project.record_digest()
    rc, output = project.waf("build", "-j1")
    run.waf_ok("noop-build", rc, output)
    run.compiled("noop-build", project.consume(), [], "no-op build")
    run.checks.equal(project.record_digest(), before, run.name("noop-build", "record-stable"))
    run.evidence(project, "noop-build", "truthful")
    return project


def scenario_install_after_build(run, out, args):
    project = Project("install_record", out, args.cc, args.cxx)
    built_fresh(run, project)
    before = project.record_digest()
    rc, output = project.waf("install", "-j1", "--fixture-define=FIXTURE_V2")
    run.waf_ok("install-v2", rc, output)
    run.compiled("install-v2", project.consume(), ["alpha.cpp.1.o", "beta.cpp.2.o"],
                 "install recompiles both with the new define")
    print("install-v2: record %s" % ("unchanged" if project.record_digest() == before
                                     else "rewritten"))
    run.evidence(project, "install-v2", "truthful-or-rejected")
    return project


def scenario_partial_targets(run, out, args):
    project = Project("install_record", out, args.cc, args.cxx)
    built_fresh(run, project)
    rc, output = project.waf("build", "-j1", "--targets=alpha", "--fixture-define=FIXTURE_V2")
    run.waf_ok("targets-alpha-v2", rc, output)
    run.compiled("targets-alpha-v2", project.consume(), ["alpha.cpp.1.o"],
                 "--targets=alpha compiles alpha only")
    run.evidence(project, "targets-alpha-v2", "truthful-or-rejected")
    return project


def scenario_failed_build(run, out, args):
    project = Project("install_record", out, args.cc, args.cxx)
    built_fresh(run, project)
    rc, output = project.waf("build", "-j1", "--fixture-define=FIXTURE_BREAK")
    run.waf_ok("break", rc, output, want_ok=False)
    ran = project.consume()
    run.checks.check(any(c["output"] == "beta.cpp.2.o" and not c["ok"] for c in ran),
                     run.name("break", "beta-failed"), "beta.cpp did not fail to compile")
    run.evidence(project, "break", "truthful-or-rejected")
    return project


# -- (c) --------------------------------------------------------------------

def policy_variant(policy, change):
    policy = copy.deepcopy(policy)
    if change == "mod-default":
        # mod leaves the strict dialect: its argv regains -fpermissive.
        del policy["targets"]["mod"]
    elif change == "cosmetic":
        policy["description"] += " (edited; no dialect change)"
    elif change == "verify-drift":
        # The verify-mode default now requires a flag the configure-time global
        # CXXFLAGS lack; verify mode never rewrites, so the build must stop.
        policy["dialects"]["cxx20-permissive"]["gnu"]["required"].append("-fno-fast-math")
    elif change == "new-probed-dialect":
        entry = copy.deepcopy(policy["dialects"]["cxx20"])
        entry["description"] = "Fixture: strict C++20 under a new name, with its own probe."
        policy["dialects"]["cxx20-fixture-probed"] = entry
        policy["targets"]["mod"] = {"dialect": "cxx20-fixture-probed"}
    else:
        raise ValueError(change)
    return policy


def policy_built(run, out, args):
    project = Project("policy_drift", out, args.cc, args.cxx)
    rc, output = project.configure()
    run.waf_ok("configure", rc, output)
    rc, output = project.waf("build", "-j1")
    run.waf_ok("build", rc, output)
    run.compiled("build", project.consume(), ["mod.cpp.1.o", "other.cpp.2.o"], "first build")
    run.evidence(project, "build", "truthful")
    return project


def scenario_policy_change(run, out, args):
    project = policy_built(run, out, args)
    base = project.policy()
    stale = "recorded with a different policy.json"
    project.write_policy(policy_variant(base, "mod-default"))
    run.evidence(project, "changed-unbuilt", "rejected", reject_text=stale)
    rc, output = project.waf("build", "-j1")
    run.waf_ok("rebuild", rc, output)
    run.compiled("rebuild", project.consume(), ["mod.cpp.1.o"],
                 "the policy change recompiles mod only")
    run.evidence(project, "rebuild", "truthful")
    cosmetic = policy_variant(project.policy(), "cosmetic")
    project.write_policy(cosmetic)
    # Byte digest: a description edit is reported stale until a rebuild.
    run.evidence(project, "cosmetic-unbuilt", "rejected", reject_text=stale)
    rc, output = project.waf("build", "-j1")
    run.waf_ok("cosmetic-rebuild", rc, output)
    run.compiled("cosmetic-rebuild", project.consume(), [], "argv unchanged: nothing recompiles")
    run.evidence(project, "cosmetic-rebuild", "truthful")
    before = project.record_digest()
    project.write_policy(policy_variant(cosmetic, "verify-drift"))
    rc, output = project.waf("build", "-j1")
    run.waf_ok("verify-drift", rc, output, want_ok=False)
    run.checks.check("TOOLCHAIN003" in output, run.name("verify-drift", "fatal-reason"),
                     "the build did not stop on TOOLCHAIN003")
    run.compiled("verify-drift", project.consume(), [], "a fatal policy check compiles nothing")
    run.checks.equal(project.record_digest(), before, run.name("verify-drift", "record-kept"))
    run.evidence(project, "verify-drift", "rejected", reject_text=stale)
    return project


def scenario_policy_clangdb_only(run, out, args):
    project = policy_built(run, out, args)
    project.write_policy(policy_variant(project.policy(), "mod-default"))
    rc, output = project.waf("clangdb")
    run.waf_ok("clangdb", rc, output)
    run.compiled("clangdb", project.consume(), [], "clangdb compiles nothing")
    run.evidence(project, "clangdb", "truthful-or-rejected")
    return project


def scenario_policy_probe_stale(run, out, args):
    project = policy_built(run, out, args)
    project.write_policy(policy_variant(project.policy(), "new-probed-dialect"))
    rc, output = project.waf("build", "-j1")
    run.waf_ok("rebuild", rc, output)
    run.compiled("rebuild", project.consume(), [], "same argv under the new dialect name")
    run.evidence(project, "rebuild", "truthful-or-rejected")
    return project


SCENARIOS = {
    "c-in-cxx": scenario_c_in_cxx,
    "c-extension-map": scenario_c_extension_map,
    "build-lifecycle": scenario_build_lifecycle,
    "install-after-build": scenario_install_after_build,
    "partial-targets": scenario_partial_targets,
    "failed-build": scenario_failed_build,
    "policy-change": scenario_policy_change,
    "policy-clangdb-only": scenario_policy_clangdb_only,
    "policy-probe-stale": scenario_policy_probe_stale,
}
SEEDS = ("misattribute-c", "wrong-std", "drop-entry", "phantom-entry", "policy-digest")


def main(argv=None):
    parser = argparse.ArgumentParser(description="Waf toolchain-recorder fixtures (gap corpus)")
    parser.add_argument("--scenario", choices=sorted(SCENARIOS))
    parser.add_argument("--seed", choices=SEEDS, default=None,
                        help="doctor the first truthful record (negative rows)")
    parser.add_argument("--out", default=os.environ.get("CONFORMANCE_OUT"),
                        help="private scratch directory (default $CONFORMANCE_OUT or a temp dir)")
    parser.add_argument("--cc", default="gcc")
    parser.add_argument("--cxx", default="g++")
    parser.add_argument("--list", action="store_true")
    args = parser.parse_args(argv)
    if args.list:
        for name in sorted(SCENARIOS):
            print(name)
        return 0
    if not args.scenario:
        parser.error("--scenario is required")
    out = args.out or tempfile.mkdtemp(prefix="toolchain-recorder-")
    out = os.path.abspath(out)
    os.makedirs(out, exist_ok=True)

    command = [sys.executable, os.path.relpath(os.path.abspath(__file__), ROOT),
               "--scenario", args.scenario] + (["--seed", args.seed] if args.seed else []) + \
        ["--out", "<scratch dir>"]
    block = provenance.collect(
        SUITE_PREFIX + args.scenario + ("" if not args.seed else ".seed-" + args.seed),
        tools={"waf": provenance.tool_version([sys.executable, WAF, "--version"]),
               args.cc: provenance.tool_version([args.cc, "--version"]),
               args.cxx: provenance.tool_version([args.cxx, "--version"])},
        inputs={"recorder": os.path.join(ROOT, "scripts", "waifulib", "toolchain_dialect.py"),
                "checker": os.path.join(ROOT, "tools", "quality", "toolchain_boundary.py"),
                "policy_module": os.path.join(ROOT, "tools", "quality", "toolchain_policy.py"),
                "waf_clangdb": os.path.join(ROOT, ".waf3-2.0.26-84a26fddc7f40e4e4f00569f36be455a",
                                            "waflib", "extras", "clang_compilation_database.py")},
        command=command)
    provenance.emit(block, out)

    checks = Checks()
    run = Run(args.scenario, checks, args.seed)
    project = None
    try:
        project = SCENARIOS[args.scenario](run, out, args)
    finally:
        if args.seed:
            checks.check(run.seeded, SUITE_PREFIX + args.scenario + ".seed-applied",
                         "seed %s found no truthful record to doctor" % args.seed)
        evidence = {"schema": "corpus-toolchain-recorder/v1", "scenario": args.scenario,
                    "seed": args.seed, "steps": run.steps}
        with open(os.path.join(out, "evidence.json"), "w", encoding="utf-8") as stream:
            json.dump(evidence, stream, indent=1)
            stream.write("\n")
        if project is not None:
            with open(os.path.join(out, "waf-transcript.txt"), "w", encoding="utf-8") as stream:
                stream.write("\n".join(project.transcript))
    return checks.report()


if __name__ == "__main__":
    sys.exit(main())
