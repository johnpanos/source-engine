#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
#
# Per-target toolchain boundary verifier (RFC 0006 M0 / roadmap R03).
#
# Checks, against quality/toolchain/policy.json:
#
#   invocations  final compile argv recorded by Waf (toolchain-invocations.json):
#                one standard flag per task matching the target's dialect,
#                required/forbidden flags, recorded dialect == policy dialect,
#                one libstdc++ dual-ABI value per link closure, fresh policy.
#   facades      each declared legacy facade compiles alone as strict C++11 (the
#                frozen external-consumer dialect) and as C++20.
#   probes       each dialect's probe compiles, links and runs per compiler.
#
# `check` runs all three and writes toolchain-evidence/v1. Mixed-dialect ABI
# fixtures are ordinary conformance suites (quality/conformance.manifest.json,
# domain Q-FOUNDATION, migration TOOLCHAIN-M0); the runner executes them.
# Python 3 standard library only.
#
# ============================================================================

import argparse
import datetime
import hashlib
import json
import os
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

import toolchain_policy as tp  # noqa: E402
from conformance import compiler_identity, source_identity, repo_root  # noqa: E402

INVOCATIONS_SCHEMA = "toolchain-invocations/v1"
EVIDENCE_SCHEMA = "toolchain-evidence/v1"
LINKED_FEATURES = {"cprogram", "cxxprogram", "cshlib", "cxxshlib"}
FACADE_STRICT_FLAGS = ["-pedantic-errors", "-Wall", "-Wextra", "-Werror", "-fsyntax-only"]


def policy_digest(root):
    with open(os.path.join(root, tp.POLICY_PATH), "rb") as stream:
        return hashlib.sha256(stream.read()).hexdigest()


def load_invocations(path):
    with open(path, "r", encoding="utf-8") as stream:
        data = json.load(stream)
    if data.get("schema") != INVOCATIONS_SCHEMA:
        raise tp.PolicyError("%s: unsupported schema %r" % (path, data.get("schema")))
    return data


# ---------------------------------------------------------------------------
# invocations
# ---------------------------------------------------------------------------

def abi_value(policy, arguments):
    define = "-D" + policy["abi"]["libstdcxx_dual_abi_define"]
    value = None
    for arg in arguments:
        if arg == define:
            value = "1"
        elif arg.startswith(define + "="):
            value = arg.split("=", 1)[1]
    return value if value is not None else policy["abi"]["libstdcxx_dual_abi_default"]


def verify_invocations(root, policy, data, label):
    """Returns (errors, summary) for one toolchain-invocations record."""
    errors = []
    entries = data.get("entries", [])
    if not entries:
        return ["%s: zero compile invocations recorded" % label], {}
    if data.get("policy_sha256") != policy_digest(root):
        errors.append("%s: recorded with a different policy.json (stale evidence); rebuild" % label)
    toolchain = data.get("toolchain", {})
    families = {}
    for language, key in (("c++", "compiler_cxx"), ("c", "compiler_cc")):
        try:
            families[language] = tp.compiler_family(policy, toolchain.get(key))
        except tp.PolicyError as error:
            errors.append("%s: %s" % (label, error))
    if errors and len(families) < 2:
        return errors, {}

    by_target = {}
    dialect_counts = {}
    for entry in entries:
        target, language = entry["target"], entry["language"]
        where = "%s: %s [%s]" % (label, entry["source"], target)
        expected = tp.target_dialect(policy, target, language)
        if entry.get("dialect") != expected:
            errors.append("%s: TOOLCHAIN006 recorded dialect %r, policy selects %r"
                          % (where, entry.get("dialect"), expected))
        try:
            problems = tp.check_flags(policy, expected, families[language], entry["arguments"])
        except tp.PolicyError as error:
            problems = [str(error)]
        errors += ["%s: %s" % (where, problem) for problem in problems]
        info = by_target.setdefault(target, {"use": entry.get("use", []),
                                             "features": entry.get("features", []),
                                             "abi": set(), "dialects": set()})
        info["dialects"].add(expected)
        if language == "c++":
            info["abi"].add(abi_value(policy, entry["arguments"]))
        dialect_counts[expected] = dialect_counts.get(expected, 0) + 1

    # One libstdc++ dual-ABI value per linked binary's C++ closure.
    for target, info in sorted(by_target.items()):
        if len(info["abi"]) > 1:
            errors.append("%s: TOOLCHAIN007 target %s compiles with mixed %s values %s"
                          % (label, target, policy["abi"]["libstdcxx_dual_abi_define"],
                             sorted(info["abi"])))
        if not LINKED_FEATURES & set(info["features"]):
            continue
        closure, pending = set(), [target]
        while pending:
            name = pending.pop()
            if name in closure or name not in by_target:
                continue
            closure.add(name)
            pending += by_target[name]["use"]
        values = {}
        for name in closure:
            for value in by_target[name]["abi"]:
                values.setdefault(value, []).append(name)
        if len(values) > 1:
            detail = "; ".join("%s=%s: %s" % (policy["abi"]["libstdcxx_dual_abi_define"],
                                               value, ", ".join(sorted(names)))
                               for value, names in sorted(values.items()))
            errors.append("%s: TOOLCHAIN007 linked target %s mixes dual-ABI values (%s)"
                          % (label, target, detail))

    summary = {
        "label": label,
        "toolchain": toolchain,
        "entries": len(entries),
        "targets": len(by_target),
        "dialect_counts": dialect_counts,
        "policy_targets_covered": sorted(t for t in by_target if t in policy["targets"]),
    }
    return errors, summary


# ---------------------------------------------------------------------------
# facades and probes
# ---------------------------------------------------------------------------

def run(cmd, timeout=120, cwd=None):
    try:
        done = subprocess.run(cmd, capture_output=True, text=True, timeout=timeout, cwd=cwd)
        return done.returncode, (done.stdout + done.stderr)
    except FileNotFoundError as error:
        return 127, str(error)
    except subprocess.TimeoutExpired:
        return 124, "timed out"


def verify_facades(root, policy, compilers):
    errors, results = [], []
    legacy = "legacy-cxx11"  # fixture-only frozen external-consumer dialect
    variants = [
        (legacy, [tp.std_flag(policy, legacy, "gnu")] + FACADE_STRICT_FLAGS),
        ("cxx20", tp.dialect_flags(policy, "cxx20") + FACADE_STRICT_FLAGS),
    ]
    facades = [f for s in policy.get("cxx20_header_sets", []) for f in s.get("legacy_facades", [])]
    if not facades:
        errors.append("TOOLCHAIN009 policy declares no legacy facades")
    for cxx in compilers:
        for facade in facades:
            for dialect, flags in variants:
                with tempfile.NamedTemporaryFile("w", suffix=".cpp", delete=False) as unit:
                    unit.write('#include "%s"\n' % os.path.relpath(facade, "public"))
                cmd = [cxx] + flags + ["-I", os.path.join(root, "public"), unit.name]
                code, output = run(cmd)
                os.unlink(unit.name)
                ok = code == 0
                results.append({"compiler": cxx, "facade": facade, "dialect": dialect,
                                "ok": ok, "repro": " ".join(cmd[:-1] + ["<unit including facade>"])})
                if not ok:
                    first = next((ln for ln in output.splitlines() if "error" in ln), output[-300:])
                    errors.append("TOOLCHAIN009 %s does not compile as %s with %s: %s"
                                  % (facade, dialect, cxx, first.strip()))
    return errors, results


def verify_probes(root, policy, compilers, c_compilers, abi_values):
    errors, results = [], []
    workdir = tempfile.mkdtemp(prefix="toolchain-probe-")
    for dialect, entry in sorted(policy["dialects"].items()):
        probe = entry.get("probe")
        if not probe or "gnu" not in entry:
            continue
        tools = compilers if entry["language"] == "c++" else c_compilers
        for tool in tools:
            for abi in (abi_values if entry["language"] == "c++" else [None]):
                exe = os.path.join(workdir, "%s-%s-%s" % (dialect, os.path.basename(tool), abi))
                cmd = [tool] + tp.dialect_flags(policy, dialect) + ["-pthread"]
                if abi is not None:
                    cmd.append("-D%s=%s" % (policy["abi"]["libstdcxx_dual_abi_define"], abi))
                cmd += [os.path.join(root, probe), "-o", exe]
                code, output = run(cmd)
                record = {"dialect": dialect, "compiler": tool, "abi": abi,
                          "compiled": code == 0, "ran": False, "output": None,
                          "repro": " ".join(cmd)}
                if code == 0:
                    code, output = run([exe], timeout=30)
                    record["ran"] = code == 0
                    record["output"] = output.strip()
                results.append(record)
                if not record["ran"]:
                    errors.append("probe %s with %s (abi=%s) failed: %s"
                                  % (dialect, tool, abi, output.strip()[-300:]))
    return errors, results


ARTIFACT_SUITES = {
    # manifest suite -> Waf static archives that replace its engine unit's
    # module sources (paths relative to the Waf build directory)
    "toolchain.abi.jobsystem-batch": ["jobsystem/libjobsystem.a"],
    "toolchain.abi.mapcontainer": ["mapcontainer/libmapcontainer.a"],
}


def verify_artifacts(root, policy, build_dir, compilers, dual_abi):
    """Links each frozen-consumer fixture against the Waf-built C++20 archives
    of one configuration, so the shipped objects (not a fixture rebuild) are
    what the C++11 consumer calls. Fixture-only sources keep their dialects."""
    errors, results = [], []
    manifest = json.load(open(os.path.join(root, "quality", "conformance.manifest.json")))
    suites = {s["id"]: s for s in manifest["suites"]}
    workdir = tempfile.mkdtemp(prefix="toolchain-artifacts-")
    for sid, archives in sorted(ARTIFACT_SUITES.items()):
        suite = suites.get(sid)
        if suite is None:
            errors.append("artifact fixture %s is missing from the manifest" % sid)
            continue
        paths = [os.path.join(build_dir, a) for a in archives]
        missing = [p for p in paths if not os.path.isfile(p)]
        if missing:
            errors.append("artifact fixture %s: %s not built in %s" % (sid, ", ".join(missing), build_dir))
            continue
        for cxx in compilers:
            objects, cmds, ok, output = [], [], True, ""
            for unit in suite["units"]:
                flags = tp.dialect_flags(policy, unit["dialect"]) + unit.get("flags", [])
                flags += ["-D%s=%s" % (policy["abi"]["libstdcxx_dual_abi_define"], dual_abi),
                          "-pthread", "-I", os.path.join(root, "public"),
                          "-I", os.path.join(root, "unittests", "toolchaintest")]
                for source in unit["sources"]:
                    # Engine-unit module sources come from the archive instead.
                    if unit["dialect"] != "legacy-cxx11" and not source.startswith("unittests/"):
                        continue
                    obj = os.path.join(workdir, "%s-%s-%s-%d.o" % (
                        sid, os.path.basename(cxx), unit["id"], len(objects)))
                    cmds.append([cxx] + flags + ["-c", os.path.join(root, source), "-o", obj])
                    objects.append(obj)
            exe = os.path.join(workdir, "%s-%s" % (sid, os.path.basename(cxx)))
            cmds.append([cxx] + objects + paths + ["-pthread", "-o", exe])
            for cmd in cmds:
                code, output = run(cmd)
                if code != 0:
                    ok = False
                    break
            record = {"suite": sid, "compiler": cxx, "archives": archives, "linked": ok,
                      "ran": False, "result": None, "repro": " && ".join(" ".join(c) for c in cmds)}
            if ok:
                code, output = run([exe], timeout=60)
                record["ran"] = code == 0
                record["result"] = next((ln for ln in output.splitlines()
                                         if ln.startswith("CONFORMANCE ")), None)
            results.append(record)
            if not record["ran"]:
                errors.append("artifact fixture %s with %s against %s failed: %s"
                              % (sid, cxx, build_dir, output.strip()[-400:]))
    return errors, results


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------

def invocation_inputs(paths):
    loaded = []
    for path in paths:
        loaded.append((path, load_invocations(path)))
    return loaded


def coverage_errors(policy, summaries):
    covered = set()
    for summary in summaries:
        covered.update(summary.get("policy_targets_covered", []))
    absent = sorted(set(policy["targets"]) - covered)
    return ["TOOLCHAIN010 policy target %s appears in no supplied configuration" % t
            for t in absent], sorted(covered)


def cmd_check(args):
    root = args.root
    policy = tp.load_policy(root)
    errors, report = [], {}
    inputs = invocation_inputs(args.invocations)

    summaries = []
    for path, data in inputs:
        e, summary = verify_invocations(root, policy, data, os.path.relpath(path, root))
        errors += e
        summaries.append(summary)
    report["invocations"] = summaries
    if args.require_all_targets:
        e, covered = coverage_errors(policy, summaries)
        errors += e
        report["policy_targets_covered"] = covered

    report["artifacts"] = []
    for build_dir in args.artifacts or []:
        # Artifacts must match the configuration's own compiler and dual-ABI value.
        data = next((d for p, d in inputs
                     if os.path.dirname(os.path.abspath(p)) == os.path.abspath(build_dir)), None)
        if data is None:
            errors.append("--artifacts %s has no matching invocation file" % build_dir)
            continue
        cxx = data["toolchain"]["compiler_cxx"]
        abi = sorted({abi_value(policy, e["arguments"]) for e in data["entries"]
                      if e["target"] in ("jobsystem", "mapcontainer")})
        if len(abi) != 1:
            errors.append("--artifacts %s: cannot determine one dual-ABI value" % build_dir)
            continue
        e, results = verify_artifacts(root, policy, os.path.abspath(build_dir), [cxx], abi[0])
        errors += e
        report["artifacts"] += results

    e, report["facades"] = verify_facades(root, policy, args.cxx)
    errors += e
    e, report["probes"] = verify_probes(root, policy, args.cxx, args.cc, ["0", "1"])
    errors += e

    decision = "fail" if errors else "pass"
    evidence = {
        "schema": EVIDENCE_SCHEMA,
        "generated_at": datetime.datetime.now(datetime.timezone.utc).isoformat(),
        "identity": source_identity(root),
        "policy_sha256": policy_digest(root),
        "compilers": [compiler_identity(c) for c in args.cxx + args.cc],
        "report": report,
        "errors": errors,
        "decision": decision,
    }
    for summary in summaries:
        print("invocations %s: %d compile tasks, %d targets, dialects %s"
              % (summary.get("label"), summary.get("entries", 0), summary.get("targets", 0),
                 json.dumps(summary.get("dialect_counts", {}), sort_keys=True)))
    if report["artifacts"]:
        print("artifacts: %d/%d frozen consumers linked to Waf archives and passed"
              % (sum(r["ran"] for r in report["artifacts"]), len(report["artifacts"])))
    print("facades: %d/%d compiled" % (sum(r["ok"] for r in report["facades"]),
                                       len(report["facades"])))
    print("probes: %d/%d compiled, linked and ran" % (sum(r["ran"] for r in report["probes"]),
                                                      len(report["probes"])))
    for error in errors:
        print("FAIL " + error)
    if args.out != "-":
        out = args.out or os.path.join(root, "quality-results", "toolchain.%s.json"
                                       % datetime.datetime.now(datetime.timezone.utc)
                                       .strftime("%Y%m%dT%H%M%SZ"))
        os.makedirs(os.path.dirname(out), exist_ok=True)
        with open(out, "w", encoding="utf-8") as stream:
            json.dump(evidence, stream, indent=1)
            stream.write("\n")
        print("evidence: " + os.path.relpath(out, root))
    print("toolchain boundary: %d invocation file(s), %d error(s) -> %s"
          % (len(inputs), len(errors), decision.upper()))
    return 0 if decision == "pass" else 1


def build_parser():
    parser = argparse.ArgumentParser(prog="toolchain_boundary.py", description=__doc__)
    parser.add_argument("--root", default=repo_root())
    sub = parser.add_subparsers(dest="command", required=True)
    check = sub.add_parser("check", help="verify invocations, artifacts, facades and probes")
    check.add_argument("invocations", nargs="+",
                       help="toolchain-invocations.json files written by Waf builds")
    check.add_argument("--cxx", action="append", default=None,
                       help="C++ compiler for facades/probes (repeatable; default g++ and clang++)")
    check.add_argument("--cc", action="append", default=None,
                       help="C compiler for C probes (repeatable; default gcc and clang)")
    check.add_argument("--artifacts", action="append", default=None,
                       help="Waf build directory whose C++20 archives the frozen consumers "
                            "link against (repeatable; must also be an invocations input)")
    check.add_argument("--require-all-targets", action="store_true",
                       help="fail unless every policy target appears in some configuration")
    check.add_argument("--out", default=None, help="evidence path ('-' to skip)")
    check.set_defaults(func=cmd_check)
    return parser


def main(argv=None):
    args = build_parser().parse_args(argv)
    args.cxx = args.cxx or ["g++", "clang++"]
    args.cc = args.cc or ["gcc", "clang"]
    try:
        return args.func(args)
    except (tp.PolicyError, OSError, json.JSONDecodeError) as error:
        print("FATAL: %s" % error, file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
