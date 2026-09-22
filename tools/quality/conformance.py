#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
#
# Shared conformance runner for RFC 0005 (Q1 / roadmap R02).
#
# One host-side orchestrator that builds, runs, and evidences every headless
# conformance suite declared in the authoritative manifest
# (quality/conformance.manifest.json). It is the connective tissue that lets
# "achieve parity as we go through all our RFCs" scale: a domain adds one
# manifest row when it lands a suite, and this runner guarantees the suite is
# built with the correct sources, run under a bounded timeout, classified by
# outcome, and recorded in immutable evidence -- so a suite can never silently
# drift out of the gate (as the hammer geometry suites did when aabb.cpp grew a
# dependency that run_headless.sh did not track).
#
# Dependency-free: Python 3 standard library only, per RFC 0005 "the
# orchestrator may use dependency-free Python".
#
# The runner is itself tested with negative fixtures (tools/quality/tests/) that
# discover no suites, deliberately fail, crash, hang, fail to compile, and go
# missing -- proving outcome handling before it is trusted as a gate.
#
# ============================================================================

import argparse
import datetime
import hashlib
import json
import os
import subprocess
import sys

MANIFEST_SCHEMA = "conformance-manifest/v1"
PROFILE_SCHEMA = "conformance-profile/v1"
EVIDENCE_SCHEMA = "conformance-evidence/v1"

# Outcomes the runner can observe for a suite. `expect` in the manifest names
# the outcome a suite SHOULD produce; a suite passes the gate only when its
# observed outcome equals its expected outcome.
OUTCOME_PASS = "pass"            # built, ran, exit 0
OUTCOME_FAIL = "fail"            # built, ran, exit != 0 (a check failed)
OUTCOME_CRASH = "crash"          # terminated by a signal
OUTCOME_TIMEOUT = "timeout"      # exceeded the profile/suite timeout
OUTCOME_COMPILE_ERROR = "compile-error"  # did not build
OUTCOME_MISSING_SOURCE = "missing-source"  # a declared source file is absent

VALID_EXPECT = {
    OUTCOME_PASS,
    OUTCOME_FAIL,
    OUTCOME_CRASH,
    OUTCOME_TIMEOUT,
    OUTCOME_COMPILE_ERROR,
    OUTCOME_MISSING_SOURCE,
}
VALID_KIND = {"positive", "sensitivity", "self-test"}


class ManifestError(Exception):
    """A structural problem with the manifest or a profile -- always fatal."""


def repo_root():
    # tools/quality/conformance.py -> repo root is two levels up.
    return os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))


# ---------------------------------------------------------------------------
# Loading and validation
# ---------------------------------------------------------------------------

def load_json(path):
    try:
        with open(path, "r", encoding="utf-8") as f:
            return json.load(f)
    except FileNotFoundError:
        raise ManifestError("file not found: %s" % path)
    except json.JSONDecodeError as e:
        raise ManifestError("invalid JSON in %s: %s" % (path, e))


def load_manifest(path):
    data = load_json(path)
    schema = data.get("schema")
    if schema != MANIFEST_SCHEMA:
        raise ManifestError(
            "unsupported manifest schema %r (expected %r)" % (schema, MANIFEST_SCHEMA))
    suites = data.get("suites")
    if not isinstance(suites, list):
        raise ManifestError("manifest 'suites' must be a list")
    seen = set()
    for i, s in enumerate(suites):
        sid = s.get("id")
        if not sid:
            raise ManifestError("suite #%d has no id" % i)
        if sid in seen:
            raise ManifestError("duplicate suite id: %s" % sid)
        seen.add(sid)
        if not s.get("sources"):
            raise ManifestError("suite %s declares no sources" % sid)
        expect = s.get("expect", OUTCOME_PASS)
        if expect not in VALID_EXPECT:
            raise ManifestError(
                "suite %s has invalid expect %r (valid: %s)"
                % (sid, expect, ", ".join(sorted(VALID_EXPECT))))
        kind = s.get("kind", "positive")
        if kind not in VALID_KIND:
            raise ManifestError(
                "suite %s has invalid kind %r (valid: %s)"
                % (sid, kind, ", ".join(sorted(VALID_KIND))))
        if not s.get("profile"):
            raise ManifestError("suite %s declares no profile" % sid)
    return data


def load_profile(profiles_dir, profile_id):
    path = os.path.join(profiles_dir, profile_id + ".json")
    data = load_json(path)
    if data.get("schema") != PROFILE_SCHEMA:
        raise ManifestError(
            "profile %s has unsupported schema %r (expected %r)"
            % (profile_id, data.get("schema"), PROFILE_SCHEMA))
    if not data.get("cxx_std"):
        raise ManifestError("profile %s declares no cxx_std" % profile_id)
    return data


# ---------------------------------------------------------------------------
# Environment / evidence identity
# ---------------------------------------------------------------------------

def git(root, *args):
    try:
        out = subprocess.run(
            ["git", "-C", root, *args],
            capture_output=True, text=True, check=False)
        if out.returncode != 0:
            return None
        return out.stdout
    except (OSError, subprocess.SubprocessError):
        return None


def source_identity(root):
    rev = git(root, "rev-parse", "HEAD")
    rev = rev.strip() if rev else None
    porcelain = git(root, "status", "--porcelain")
    dirty_files = len([ln for ln in porcelain.splitlines() if ln.strip()]) if porcelain else 0
    diff = git(root, "diff", "HEAD") or ""
    dirty_digest = None
    if dirty_files:
        dirty_digest = hashlib.sha1(diff.encode("utf-8", "replace")).hexdigest()[:12]
    return {
        "source_revision": rev,
        "dirty": dirty_files > 0,
        "dirty_files": dirty_files,
        "dirty_digest": dirty_digest,
    }


def compiler_identity(cxx):
    try:
        out = subprocess.run([cxx, "--version"], capture_output=True, text=True, check=False)
        version = out.stdout.splitlines()[0] if out.stdout else None
    except OSError:
        version = None
    return {"cxx": cxx, "cxx_version": version}


# ---------------------------------------------------------------------------
# Building and running one suite
# ---------------------------------------------------------------------------

def build_command(root, cxx, profile, suite, out_bin):
    flags = ["-std=" + profile["cxx_std"]]
    flags += list(profile.get("base_flags", []))
    flags += list(suite.get("extra_flags", []))
    includes = []
    for inc in profile.get("include_roots", []):
        includes += ["-I", os.path.join(root, inc)]
    sources = [os.path.join(root, s) for s in suite["sources"]]
    return [cxx, *flags, *includes, *sources, "-o", out_bin]


def run_suite(root, cxx, profile, suite, out_dir):
    sid = suite["id"]
    expect = suite.get("expect", OUTCOME_PASS)
    kind = suite.get("kind", "positive")
    timeout = suite.get("timeout_seconds", profile.get("timeout_seconds", 60))

    result = {
        "id": sid,
        "domain": suite.get("domain"),
        "rfc": suite.get("rfc"),
        "migration": suite.get("migration"),
        "contract": suite.get("contract"),
        "kind": kind,
        "profile": suite.get("profile"),
        "expect": expect,
        "outcome": None,
        "matched": False,
        "build_ok": None,
        "exit_code": None,
        "signal": None,
        "duration_s": None,
        "first_divergence": None,
        "detail": None,
        "repro": None,
    }

    # Missing sources are a runner-level failure: a suite that cannot be found
    # can never certify anything (RFC 0005: fail on missing fixtures).
    missing = [s for s in suite["sources"] if not os.path.exists(os.path.join(root, s))]
    if missing:
        result["outcome"] = OUTCOME_MISSING_SOURCE
        result["detail"] = "missing source(s): " + ", ".join(missing)
        result["matched"] = (expect == OUTCOME_MISSING_SOURCE)
        return result

    out_bin = os.path.join(out_dir, sid.replace("/", "_").replace(".", "_"))
    cmd = build_command(root, cxx, profile, suite, out_bin)
    result["repro"] = " ".join(cmd)

    build = subprocess.run(cmd, capture_output=True, text=True, check=False)
    if build.returncode != 0:
        result["build_ok"] = False
        result["outcome"] = OUTCOME_COMPILE_ERROR
        result["first_divergence"] = first_line(build.stderr, ("error:",))
        result["detail"] = tail(build.stderr)
        result["matched"] = (expect == OUTCOME_COMPILE_ERROR)
        return result
    result["build_ok"] = True

    start = datetime.datetime.now()
    try:
        run = subprocess.run([out_bin], capture_output=True, text=True,
                             check=False, timeout=timeout)
    except subprocess.TimeoutExpired as e:
        result["duration_s"] = round((datetime.datetime.now() - start).total_seconds(), 3)
        result["outcome"] = OUTCOME_TIMEOUT
        result["detail"] = "timed out after %ss; %s" % (timeout, tail(_as_text(e.stdout)))
        result["matched"] = (expect == OUTCOME_TIMEOUT)
        return result
    result["duration_s"] = round((datetime.datetime.now() - start).total_seconds(), 3)

    rc = run.returncode
    result["exit_code"] = rc
    result["detail"] = tail(run.stdout + run.stderr)
    if rc < 0:
        result["signal"] = -rc
        result["outcome"] = OUTCOME_CRASH
    elif rc == 0:
        result["outcome"] = OUTCOME_PASS
    else:
        result["outcome"] = OUTCOME_FAIL
        result["first_divergence"] = first_line(run.stdout, ("FAIL", "fail"))

    result["matched"] = (result["outcome"] == expect)
    return result


def _as_text(v):
    if v is None:
        return ""
    if isinstance(v, bytes):
        return v.decode("utf-8", "replace")
    return v


def first_line(text, prefixes):
    for ln in _as_text(text).splitlines():
        stripped = ln.strip()
        for p in prefixes:
            if stripped.startswith(p):
                return stripped
    return None


def tail(text, n=20):
    lines = _as_text(text).splitlines()
    return "\n".join(lines[-n:]) if lines else ""


# ---------------------------------------------------------------------------
# Selection
# ---------------------------------------------------------------------------

def select_suites(manifest, args):
    suites = manifest["suites"]
    selectors = []
    if args.suite:
        selectors.append(("id", set(args.suite)))
    if args.domain:
        selectors.append(("domain", set(args.domain)))
    if args.rfc:
        selectors.append(("rfc", set(args.rfc)))
    if args.profile:
        selectors.append(("profile", set(args.profile)))

    if not selectors:
        return list(suites), None

    selected = []
    for s in suites:
        if all(s.get(field) in wanted for field, wanted in selectors):
            selected.append(s)
    if not selected:
        wanted = "; ".join("%s in {%s}" % (f, ", ".join(sorted(w))) for f, w in selectors)
        return [], "no suite matches required selector(s): " + wanted
    return selected, None


# ---------------------------------------------------------------------------
# Commands
# ---------------------------------------------------------------------------

def cmd_list(args):
    root = args.root
    manifest = load_manifest(os.path.join(root, args.manifest))
    for s in manifest["suites"]:
        print("%-34s %-12s rfc:%-5s expect:%-14s %s"
              % (s["id"], s.get("domain", "-"), s.get("rfc", "-"),
                 s.get("expect", OUTCOME_PASS), s.get("kind", "positive")))
    print("\n%d suite(s)" % len(manifest["suites"]))
    return 0


def cmd_check(args):
    root = args.root
    manifest_path = os.path.join(root, args.manifest)
    manifest = load_manifest(manifest_path)
    profiles_dir = os.path.join(root, manifest.get("profiles_dir", "quality/profiles"))

    suites = manifest["suites"]
    if not suites:
        # Zero discovery is a hard failure: an empty required run cannot certify
        # anything (RFC 0005 runner contract).
        print("FATAL: manifest declares zero suites (zero discovery)", file=sys.stderr)
        return 2

    selected, sel_err = select_suites(manifest, args)
    if sel_err:
        print("FATAL: " + sel_err, file=sys.stderr)
        return 2

    cxx = args.cxx
    out_dir = args.build_dir
    os.makedirs(out_dir, exist_ok=True)

    # Load every referenced profile up front so a bad profile fails fast.
    profiles = {}
    for s in selected:
        pid = s["profile"]
        if pid not in profiles:
            profiles[pid] = load_profile(profiles_dir, pid)

    expected_ids = [s["id"] for s in selected]
    print("conformance: %d suite(s) selected, cxx=%s" % (len(selected), cxx))

    results = []
    for s in selected:
        profile = profiles[s["profile"]]
        r = run_suite(root, cxx, profile, s, out_dir)
        results.append(r)
        status = "ok  " if r["matched"] else "FAIL"
        line = "  [%s] %-34s expect=%-14s got=%-14s" % (
            status, r["id"], r["expect"], r["outcome"])
        if r["duration_s"] is not None:
            line += " (%.2fs)" % r["duration_s"]
        print(line)
        if not r["matched"]:
            if r["first_divergence"]:
                print("        first divergence: " + r["first_divergence"])
            elif r["detail"]:
                print("        " + r["detail"].replace("\n", "\n        "))

    # Reconcile: every selected suite must have produced a result.
    executed_ids = [r["id"] for r in results]
    reconciled = sorted(expected_ids) == sorted(executed_ids)

    passed = sum(1 for r in results if r["matched"])
    failed = len(results) - passed
    decision = "pass" if (failed == 0 and reconciled) else "fail"

    evidence = build_evidence(root, manifest_path, manifest, cxx, profiles, results,
                              expected_ids, executed_ids, reconciled, decision)
    evidence_path = write_evidence(root, args.out, evidence)

    print("\n%d suite(s): %d matched, %d mismatched -> %s"
          % (len(results), passed, failed, decision.upper()))
    if not reconciled:
        print("FATAL: reconciliation mismatch (expected != executed)", file=sys.stderr)
    if evidence_path:
        print("evidence: " + os.path.relpath(evidence_path, root))
    return 0 if decision == "pass" else 1


def build_evidence(root, manifest_path, manifest, cxx, profiles, results,
                   expected_ids, executed_ids, reconciled, decision):
    ident = source_identity(root)
    ident.update(compiler_identity(cxx))
    return {
        "schema": EVIDENCE_SCHEMA,
        "generated_at": datetime.datetime.now(datetime.timezone.utc).isoformat(),
        "manifest": os.path.relpath(manifest_path, root),
        "manifest_schema": manifest.get("schema"),
        "identity": ident,
        "profiles": {
            pid: {
                "cxx_std": p.get("cxx_std"),
                "base_flags": p.get("base_flags", []),
                "include_roots": p.get("include_roots", []),
                "timeout_seconds": p.get("timeout_seconds"),
            } for pid, p in profiles.items()
        },
        "expected_ids": sorted(expected_ids),
        "executed_ids": sorted(executed_ids),
        "reconciled": reconciled,
        "counts": {
            "total": len(results),
            "matched": sum(1 for r in results if r["matched"]),
            "mismatched": sum(1 for r in results if not r["matched"]),
        },
        "suites": results,
        "decision": decision,
    }


def write_evidence(root, out_arg, evidence):
    if out_arg == "-":
        return None
    if out_arg:
        path = out_arg if os.path.isabs(out_arg) else os.path.join(root, out_arg)
    else:
        stamp = datetime.datetime.now(datetime.timezone.utc).strftime("%Y%m%dT%H%M%SZ")
        out_dir = os.path.join(root, "quality-results")
        os.makedirs(out_dir, exist_ok=True)
        path = os.path.join(out_dir, "conformance.%s.json" % stamp)
    with open(path, "w", encoding="utf-8") as f:
        json.dump(evidence, f, indent=2, sort_keys=False)
        f.write("\n")
    return path


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------

def build_parser():
    p = argparse.ArgumentParser(
        prog="conformance.py",
        description="Shared conformance runner (RFC 0005 Q1 / roadmap R02).")
    p.add_argument("--root", default=repo_root(),
                   help="Repository root (default: inferred from script location).")
    p.add_argument("--manifest", default="quality/conformance.manifest.json",
                   help="Manifest path, relative to --root.")
    sub = p.add_subparsers(dest="command", required=True)

    lst = sub.add_parser("list", help="List declared conformance suites.")
    lst.set_defaults(func=cmd_list)

    chk = sub.add_parser("check", help="Build, run, and evidence conformance suites.")
    chk.add_argument("--cxx", default=os.environ.get("CXX", "g++"),
                     help="C++ compiler to use (default: $CXX or g++).")
    chk.add_argument("--suite", action="append", help="Select by suite id (repeatable).")
    chk.add_argument("--domain", action="append", help="Select by domain, e.g. Q-EDITOR.")
    chk.add_argument("--rfc", action="append", help="Select by RFC number, e.g. 0002.")
    chk.add_argument("--profile", action="append", help="Select by profile id.")
    chk.add_argument("--build-dir", default=os.path.join(repo_root(), "build", "quality"),
                     help="Directory for compiled suite binaries.")
    chk.add_argument("--out", default=None,
                     help="Evidence JSON path (default: quality-results/<timestamp>.json; "
                          "'-' to skip writing).")
    chk.set_defaults(func=cmd_check)
    return p


def main(argv=None):
    args = build_parser().parse_args(argv)
    try:
        return args.func(args)
    except ManifestError as e:
        print("FATAL: " + str(e), file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
