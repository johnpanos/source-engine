#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
#
# Reproducible baseline and profile inventory (RFC 0005 Q0 / roadmap R01).
#
# quality/baseline.json declares, once:
#
#   tools      host tools, SDKs, libraries and sanitizer runtimes a profile needs
#   content    licensed or external corpora (never copied into the repository)
#   profiles   the support matrix: every declared product/OS/architecture, the
#              profile file that owns its facts, and what it needs here
#   checks     installed check/build/run commands with the outcome recorded as
#              this revision's baseline (pass, or a known failure with an owner)
#   baselines  identified baseline captures and budgets per harness domain
#
# Every check declares `budget_seconds`, how long it should take on the
# declared host, so the expected cost of an audit is known before it runs. Its
# timeout defaults to 3x the budget (at least 10 s); a longer timeout needs a
# `timeout_reason`, such as a from-scratch build. `after` names checks whose
# outputs it reads; `schedule: serial` runs a timing- or device-sensitive
# check alone, after every concurrent check has finished.
#
# `audit` probes availability, runs the selected checks and writes
# baseline-evidence/v1. A check whose outcome differs from its recorded
# baseline -- a new failure or a known failure that now passes -- fails the
# audit, so the recorded baseline cannot silently go stale. A check whose
# requirements are missing is `unavailable`: it is reported, never counted as
# passing, and fails the audit only with --strict.
#
# Profile facts (pins, flags, evidence status) stay in the profile files; this
# tool reads them and does not copy them. Dependency-free: Python 3 stdlib only.
#
# ============================================================================

import argparse
import concurrent.futures
import datetime
import json
import math
import os
import platform
import re
import shutil
import signal
import subprocess
import sys
import tempfile
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import conformance  # noqa: E402  (source identity, JSON loading, output tails)


SCHEMA = "source-baseline/v1"
EVIDENCE_SCHEMA = "baseline-evidence/v1"

OUTCOMES = ("pass", "fail", "crash", "timeout")
UNAVAILABLE = "unavailable"
TOOL_KINDS = ("command", "pkg-config", "path", "env", "compile")
TIERS = ("north-star", "preserved", "product-scope")
# Whether a profile's native runner must exist before a gate that names the
# profile can close. "optional" profiles (Apple and MSVC, user decision
# 2026-09-25) stay declared and are reported when unavailable, but never block.
RUNNER_REQUIREMENTS = ("required", "optional")
BASELINE_KINDS = ("capture", "budget")
BASELINE_STATES = ("recorded", "partial", "missing")
ROW_PATTERN = re.compile(r"^R\d+$")
SCHEDULES = ("parallel", "serial")
TIMEOUT_FACTOR = 3
MIN_TIMEOUT_SECONDS = 10


def budget_timeout(check):
    """The longest a check may run without a stated reason."""
    return max(MIN_TIMEOUT_SECONDS, int(math.ceil(TIMEOUT_FACTOR * check["budget_seconds"])))


def check_timeout(check):
    return check.get("timeout", budget_timeout(check))


class BaselineError(Exception):
    """A structural problem with the declaration -- always fatal."""


# ---------------------------------------------------------------------------
# Loading and validation
# ---------------------------------------------------------------------------

def _require(entry, keys, where):
    for key in keys:
        if key not in entry:
            raise BaselineError("%s: missing required field '%s'" % (where, key))


def _unique(entries, where):
    ids = [entry["id"] for entry in entries]
    duplicates = sorted({i for i in ids if ids.count(i) > 1})
    if duplicates:
        raise BaselineError("%s: duplicate id(s) %s" % (where, ", ".join(duplicates)))
    return set(ids)


def _reject_cycles(by_id):
    state = {}

    def visit(ident, path):
        if state.get(ident) == "done":
            return
        if state.get(ident) == "active":
            raise BaselineError("checks form an after-cycle: %s" % " -> ".join(path + [ident]))
        state[ident] = "active"
        for dep in by_id[ident].get("after", []):
            visit(dep, path + [ident])
        state[ident] = "done"

    for ident in by_id:
        visit(ident, [])


def validate(root, declaration):
    """Reject unknown schemas, dangling references and missing linked records."""
    if declaration.get("schema") != SCHEMA:
        raise BaselineError("unsupported schema %r (expected %s)" % (declaration.get("schema"), SCHEMA))
    for section in ("tools", "content", "profiles", "checks", "baselines"):
        if not isinstance(declaration.get(section), list):
            raise BaselineError("section '%s' must be a list" % section)

    for tool in declaration["tools"]:
        _require(tool, ("id", "kind"), "tool")
        if tool["kind"] not in TOOL_KINDS:
            raise BaselineError("tool %s: unknown kind %r" % (tool["id"], tool["kind"]))
    tool_ids = _unique(declaration["tools"], "tools")

    for corpus in declaration["content"]:
        _require(corpus, ("id", "marker", "candidates", "redistribution"), "content")
    content_ids = _unique(declaration["content"], "content")

    def check_requires(owner, requires):
        for name in requires:
            kind, _, ident = name.partition(":")
            if kind == "tool" and ident not in tool_ids:
                raise BaselineError("%s: requires unknown tool %r" % (owner, ident))
            if kind == "content" and ident not in content_ids:
                raise BaselineError("%s: requires unknown content %r" % (owner, ident))
            if kind not in ("tool", "content", "path"):
                raise BaselineError("%s: requirement %r must be tool:, content: or path:" % (owner, name))

    for check in declaration["checks"]:
        _require(check, ("id", "group", "domain", "rows", "argv", "baseline"), "check")
        expected = check["baseline"].get("outcome")
        if expected not in OUTCOMES:
            raise BaselineError("check %s: baseline outcome %r is not one of %s"
                % (check["id"], expected, ", ".join(OUTCOMES)))
        if expected != "pass":
            _require(check["baseline"], ("reason", "owner"), "check %s baseline" % check["id"])
        for row in check["rows"]:
            if not ROW_PATTERN.match(row):
                raise BaselineError("check %s: row %r is not an R<number> id" % (check["id"], row))
        check_requires("check %s" % check["id"], check.get("requires", []))
        budget = check.get("budget_seconds")
        if isinstance(budget, bool) or not isinstance(budget, (int, float)) or budget <= 0:
            raise BaselineError("check %s: budget_seconds must be a positive number of seconds"
                % check["id"])
        if check.get("timeout", 0) > budget_timeout(check) and not check.get("timeout_reason"):
            raise BaselineError("check %s: timeout %ss exceeds %sx its %ss budget; "
                "state a timeout_reason or tighten it"
                % (check["id"], check["timeout"], TIMEOUT_FACTOR, budget))
        if check.get("schedule", "parallel") not in SCHEDULES:
            raise BaselineError("check %s: schedule %r is not one of %s"
                % (check["id"], check.get("schedule"), ", ".join(SCHEDULES)))
    check_ids = _unique(declaration["checks"], "checks")
    by_id = {check["id"]: check for check in declaration["checks"]}
    for check in declaration["checks"]:
        for dep in check.get("after", []):
            if dep not in check_ids or dep == check["id"]:
                raise BaselineError("check %s: after names unknown check %r" % (check["id"], dep))
            if check.get("schedule", "parallel") == "parallel" and \
                    by_id[dep].get("schedule", "parallel") == "serial":
                raise BaselineError("check %s: a concurrent check cannot run after serial %s"
                    % (check["id"], dep))
    _reject_cycles(by_id)

    for profile in declaration["profiles"]:
        _require(profile, ("id", "tier", "product", "os", "arch", "profile"), "profile")
        if profile["tier"] not in TIERS:
            raise BaselineError("profile %s: tier %r is not one of %s"
                % (profile["id"], profile["tier"], ", ".join(TIERS)))
        if profile["profile"] is None:
            _require(profile, ("gap",), "profile %s (no profile file)" % profile["id"])
        elif not os.path.isfile(os.path.join(root, profile["profile"])):
            raise BaselineError("profile %s: profile file %s does not exist" % (profile["id"], profile["profile"]))
        check_requires("profile %s" % profile["id"], profile.get("requires", []))
        if profile.get("runner_requirement", "required") not in RUNNER_REQUIREMENTS:
            raise BaselineError("profile %s: runner_requirement %r is not one of %s"
                % (profile["id"], profile["runner_requirement"], ", ".join(RUNNER_REQUIREMENTS)))
        for ident in profile.get("checks", []):
            if ident not in check_ids:
                raise BaselineError("profile %s: unknown check %r" % (profile["id"], ident))
    _unique(declaration["profiles"], "profiles")

    for entry in declaration["baselines"]:
        _require(entry, ("id", "domain", "kind", "state", "owner", "note"), "baseline")
        if entry["kind"] not in BASELINE_KINDS:
            raise BaselineError("baseline %s: kind %r is not one of %s" % (entry["id"], entry["kind"], ", ".join(BASELINE_KINDS)))
        if entry["state"] not in BASELINE_STATES:
            raise BaselineError("baseline %s: state %r is not one of %s" % (entry["id"], entry["state"], ", ".join(BASELINE_STATES)))
        for record in entry.get("records", []):
            if not os.path.exists(os.path.join(root, record.split("#", 1)[0])):
                raise BaselineError("baseline %s: linked record %s does not exist" % (entry["id"], record))
        if entry["state"] != "missing" and not entry.get("records"):
            raise BaselineError("baseline %s: a %s baseline must link its records" % (entry["id"], entry["state"]))
    _unique(declaration["baselines"], "baselines")
    return declaration


def load(root, path):
    try:
        declaration = conformance.load_json(path)
    except conformance.ManifestError as error:
        raise BaselineError(str(error))
    return validate(root, declaration)


# ---------------------------------------------------------------------------
# Probing
# ---------------------------------------------------------------------------

def _expand(path):
    return os.path.expandvars(os.path.expanduser(path))


def _run(argv, timeout, cwd=None, env=None):
    try:
        completed = subprocess.run(argv, cwd=cwd, env=env, capture_output=True, timeout=timeout, check=False)
    except (OSError, subprocess.SubprocessError) as error:
        return None, str(error)
    text = conformance._as_text(completed.stdout) + conformance._as_text(completed.stderr)
    return completed.returncode, text


def probe_tool(root, tool, timeout=60):
    """Return {available, detail}. Never raises for a missing tool."""
    kind = tool["kind"]
    if kind == "command":
        if shutil.which(tool["argv"][0]) is None:
            return {"available": False, "detail": "%s not on PATH" % tool["argv"][0]}
        code, text = _run(tool["argv"], timeout, cwd=root)
        if code != 0:
            return {"available": False, "detail": "exit %s: %s" % (code, conformance.tail(text, 2))}
        lines = [line.strip() for line in text.splitlines() if line.strip()]
        if tool.get("expect"):
            matched = [line for line in text.splitlines() if re.search(tool["expect"], line)]
            if not matched:
                return {"available": False, "detail": "output does not match %r" % tool["expect"]}
            lines = [line.strip() for line in matched]
        return {"available": True, "detail": lines[0] if lines else ""}
    if kind == "pkg-config":
        code, text = _run(["pkg-config", "--modversion", tool["module"]], timeout)
        if code != 0:
            return {"available": False, "detail": "pkg-config module %s not found" % tool["module"]}
        return {"available": True, "detail": "%s %s" % (tool["module"], text.strip())}
    if kind == "path":
        candidates = list(tool.get("candidates", []))
        if tool.get("env") and os.environ.get(tool["env"]):
            candidates.insert(0, os.environ[tool["env"]])
        for candidate in candidates:
            path = _expand(candidate)
            path = path if os.path.isabs(path) else os.path.join(root, path)
            if os.path.exists(path):
                return {"available": True, "detail": path}
        return {"available": False, "detail": "none of %s exist" % ", ".join(candidates) if candidates else "no candidate"}
    if kind == "env":
        value = os.environ.get(tool["variable"])
        if not value:
            return {"available": False, "detail": "%s is unset" % tool["variable"]}
        return {"available": True, "detail": "%s=%s" % (tool["variable"], value)}
    if kind == "compile":
        if shutil.which(tool["cxx"]) is None:
            return {"available": False, "detail": "%s not on PATH" % tool["cxx"]}
        with tempfile.TemporaryDirectory(prefix="baseline-probe-") as work:
            source = os.path.join(work, "probe.cpp")
            binary = os.path.join(work, "probe")
            with open(source, "w", encoding="utf-8") as stream:
                stream.write(tool["source"])
            code, text = _run([tool["cxx"], *tool.get("flags", []), source, "-o", binary], timeout)
            if code != 0:
                lines = [line.strip() for line in text.splitlines() if line.strip()]
                return {"available": False, "detail": "compile/link failed: %s" % (lines[0] if lines else "")}
            code, text = _run([binary], timeout)
            if code != 0:
                return {"available": False, "detail": "probe exited %s" % code}
        return {"available": True, "detail": "compiled, linked and ran"}
    raise BaselineError("tool %s: unknown kind %r" % (tool["id"], kind))


def probe_content(root, corpus):
    candidates = list(corpus["candidates"])
    if corpus.get("env") and os.environ.get(corpus["env"]):
        candidates.insert(0, os.environ[corpus["env"]])
    for candidate in candidates:
        base = _expand(candidate)
        base = base if os.path.isabs(base) else os.path.join(root, base)
        if os.path.exists(os.path.join(base, corpus["marker"])):
            return {"available": True, "detail": base}
    return {"available": False, "detail": "no candidate contains %s" % corpus["marker"]}


def requirement_status(root, requires, tools, content):
    """Names of unmet requirements (tool:, content:, path:)."""
    missing = []
    for name in requires:
        kind, _, ident = name.partition(":")
        if kind == "tool" and not tools[ident]["available"]:
            missing.append(name)
        elif kind == "content" and not content[ident]["available"]:
            missing.append(name)
        elif kind == "path" and not os.path.exists(os.path.join(root, ident)):
            missing.append(name)
    return missing


# ---------------------------------------------------------------------------
# Checks
# ---------------------------------------------------------------------------

def _substitute(value, variables):
    for key, replacement in variables.items():
        value = value.replace("{%s}" % key, replacement)
    return value


def _run_group(argv, cwd, env, timeout):
    """Run argv in its own process group with output in a file.

    A pipe would keep the check open for as long as any descendant (a
    compiler, wineserver) holds it, past the check's own exit or timeout;
    a file does not. On timeout the whole group is killed, so the timeout
    bounds the check's cost. Returns (returncode or None on timeout, text)."""
    with tempfile.TemporaryFile() as output:
        process = subprocess.Popen(argv, cwd=cwd, env=env, stdin=subprocess.DEVNULL,
            stdout=output, stderr=subprocess.STDOUT, start_new_session=True)
        try:
            code = process.wait(timeout=timeout)
        except subprocess.TimeoutExpired:
            code = None
        finally:
            try:
                os.killpg(process.pid, signal.SIGKILL)
            except ProcessLookupError:
                pass
            if code is None:
                process.wait()
        output.seek(0)
        return code, output.read().decode("utf-8", "replace")


def run_check(root, check, variables, log_dir):
    """Run one check; classify pass/fail/crash/timeout from the process result."""
    argv = [_substitute(arg, variables) for arg in check["argv"]]
    env = dict(os.environ)
    for key, value in check.get("env", {}).items():
        env[key] = _substitute(value, variables)
    cwd = os.path.join(root, check.get("cwd", "."))
    timeout = check_timeout(check)
    started = time.monotonic()
    try:
        code, text = _run_group(argv, cwd, env, timeout)
        if code is None:
            outcome = "timeout"
        elif code == 0:
            outcome = "pass"
        elif code < 0:
            outcome = "crash"
        else:
            outcome = "fail"
    except OSError as error:
        code = None
        text = str(error)
        outcome = "fail"
    seconds = round(time.monotonic() - started, 2)
    log = None
    if log_dir:
        log = os.path.join(log_dir, "%s.log" % check["id"])
        with open(log, "w", encoding="utf-8") as stream:
            stream.write("$ %s\n" % " ".join(argv))
            stream.write(text)
    return {
        "outcome": outcome,
        "exit_code": code,
        "signal": -code if code is not None and code < 0 else None,
        "seconds": seconds,
        "budget_seconds": check["budget_seconds"],
        "timeout": timeout,
        "over_budget": seconds > check["budget_seconds"],
        "command": argv,
        "cwd": check.get("cwd", "."),
        "log": log,
        "tail": conformance.tail(text, 12),
    }


def schedule(checks, parallel, execute):
    """Run `execute(check)` for every check and return {id: result}.

    Concurrent checks start as soon as every selected check they run `after`
    has finished, at most `parallel` at a time. Serial checks then run one at
    a time in declaration order, with nothing else running."""
    selected = {check["id"] for check in checks}
    concurrent_checks = [c for c in checks if c.get("schedule", "parallel") == "parallel"]
    serial_checks = [c for c in checks if c.get("schedule", "parallel") == "serial"]
    results = {}
    pending = list(concurrent_checks)
    running = {}
    with concurrent.futures.ThreadPoolExecutor(max_workers=max(1, parallel)) as pool:
        while pending or running:
            for check in list(pending):
                if len(running) >= parallel:
                    break
                deps = [d for d in check.get("after", []) if d in selected]
                if all(d in results for d in deps):
                    pending.remove(check)
                    running[pool.submit(execute, check)] = check
            done, _ = concurrent.futures.wait(running, return_when=concurrent.futures.FIRST_COMPLETED)
            for future in done:
                check = running.pop(future)
                results[check["id"]] = future.result()
    for check in serial_checks:
        results[check["id"]] = execute(check)
    return results


def compare(check, result):
    """Return the deviation text, or None when the outcome matches the baseline."""
    expected = check["baseline"]["outcome"]
    observed = result["outcome"]
    if observed == UNAVAILABLE or observed == expected:
        return None
    if expected == "pass":
        return "regression: recorded baseline is pass, observed %s" % observed
    if observed == "pass":
        return ("known %s now passes: review and update the recorded baseline (owner %s)"
            % (expected, check["baseline"]["owner"]))
    return "changed failure mode: recorded %s, observed %s" % (expected, observed)


def select_checks(declaration, groups, ids):
    checks = declaration["checks"]
    if groups:
        checks = [check for check in checks if check["group"] in groups]
    if ids:
        unknown = sorted(set(ids) - {check["id"] for check in declaration["checks"]})
        if unknown:
            raise BaselineError("unmatched check selector(s): %s" % ", ".join(unknown))
        checks = [check for check in checks if check["id"] in ids]
    if not checks:
        raise BaselineError("selectors matched zero checks")
    return checks


# ---------------------------------------------------------------------------
# Profiles and baselines
# ---------------------------------------------------------------------------

def profile_status(root, profile, tools, content, check_results):
    """Derived support facts: the profile file owns intent and evidence."""
    owned = None
    if profile["profile"]:
        data = conformance.load_json(os.path.join(root, profile["profile"]))
        owned = {
            "schema": data.get("schema"),
            "evidence_status": (data.get("evidence") or {}).get("status", "not recorded in profile"),
        }
    missing = requirement_status(root, profile.get("requires", []), tools, content)
    checks = {ident: check_results[ident]["outcome"] for ident in profile.get("checks", []) if ident in check_results}
    return {
        "id": profile["id"],
        "tier": profile["tier"],
        "product": profile["product"],
        "os": profile["os"],
        "arch": profile["arch"],
        "profile": profile["profile"],
        "profile_facts": owned,
        "gap": profile.get("gap"),
        "host_requirements_missing": missing,
        "checks": checks,
    }


def host_identity():
    memory = None
    try:
        with open("/proc/meminfo", encoding="utf-8") as stream:
            for line in stream:
                if line.startswith("MemTotal:"):
                    memory = line.split(":", 1)[1].strip()
    except OSError:
        pass
    return {
        "system": platform.system(),
        "release": platform.release(),
        "machine": platform.machine(),
        "cpu_count": os.cpu_count(),
        "memory": memory,
        "python": platform.python_version(),
    }


# ---------------------------------------------------------------------------
# Commands
# ---------------------------------------------------------------------------

def probe_all(root, declaration):
    tools = {tool["id"]: probe_tool(root, tool) for tool in declaration["tools"]}
    content = {corpus["id"]: probe_content(root, corpus) for corpus in declaration["content"]}
    return tools, content


def print_availability(declaration, tools, content):
    print("tools:")
    for tool in declaration["tools"]:
        status = tools[tool["id"]]
        print("  [%s] %-24s %s" % ("ok " if status["available"] else "---", tool["id"], status["detail"]))
    print("content:")
    for corpus in declaration["content"]:
        status = content[corpus["id"]]
        print("  [%s] %-24s %s" % ("ok " if status["available"] else "---", corpus["id"], status["detail"]))


def cmd_validate(args):
    declaration = load(args.root, args.declaration)
    print("baseline: %s is valid (%d tools, %d content, %d profiles, %d checks, %d baselines)" % (
        os.path.relpath(args.declaration, args.root), len(declaration["tools"]), len(declaration["content"]),
        len(declaration["profiles"]), len(declaration["checks"]), len(declaration["baselines"])))
    return 0


def cmd_probe(args):
    declaration = load(args.root, args.declaration)
    tools, content = probe_all(args.root, declaration)
    print_availability(declaration, tools, content)
    return 0


def cmd_audit(args):
    root = args.root
    if args.parallel < 1:
        raise BaselineError("--parallel must be at least 1")
    declaration = load(root, args.declaration)
    checks = select_checks(declaration, args.group, args.check)
    tools, content = probe_all(root, declaration)
    print_availability(declaration, tools, content)

    stamp = datetime.datetime.now(datetime.timezone.utc).strftime("%Y%m%dT%H%M%SZ")
    out_dir = None
    if args.out != "-":
        out_dir = args.out or os.path.join(root, "quality-results", "baseline", stamp)
        out_dir = out_dir if os.path.isabs(out_dir) else os.path.join(root, out_dir)
        os.makedirs(os.path.join(out_dir, "logs"), exist_ok=True)
    variables = {"root": root, "jobs": str(args.jobs), "scratch": out_dir or tempfile.gettempdir()}
    for ident, status in content.items():
        if status["available"]:
            variables["content:%s" % ident] = status["detail"]

    print("checks (%d concurrent, budget %.0fs of serial work):"
        % (args.parallel, sum(check["budget_seconds"] for check in checks)))
    started = time.monotonic()
    log_dir = os.path.join(out_dir, "logs") if out_dir else None

    def execute(check):
        missing = requirement_status(root, check.get("requires", []), tools, content)
        if missing:
            result = {"outcome": UNAVAILABLE, "missing": missing, "seconds": 0.0,
                "budget_seconds": check["budget_seconds"], "timeout": check_timeout(check),
                "over_budget": False}
        else:
            result = run_check(root, check, variables, log_dir)
        deviation = compare(check, result)
        result.update({
            "id": check["id"],
            "group": check["group"],
            "domain": check["domain"],
            "rows": check["rows"],
            "baseline": check["baseline"],
            "deviation": deviation,
        })
        marker = "ok  " if deviation is None and result["outcome"] != UNAVAILABLE else (
            "n/a " if result["outcome"] == UNAVAILABLE else "DEV ")
        detail = deviation or (", ".join(result["missing"]) if result["outcome"] == UNAVAILABLE else "")
        slow = " SLOW" if result["over_budget"] else ""
        print("  [%s] %-34s recorded=%-8s observed=%-11s %6.1fs/%-5gs%s %s" % (
            marker, check["id"], check["baseline"]["outcome"], result["outcome"],
            result["seconds"], check["budget_seconds"], slow, detail))
        sys.stdout.flush()
        return result

    by_id = schedule(checks, args.parallel, execute)
    # Evidence lists checks in declaration order, whatever order they finished in.
    results = {check["id"]: by_id[check["id"]] for check in checks}
    deviations = [ident for ident, result in results.items() if result["deviation"]]
    unavailable = [ident for ident, result in results.items() if result["outcome"] == UNAVAILABLE]
    over_budget = [ident for ident, result in results.items() if result["over_budget"]]
    elapsed = round(time.monotonic() - started, 2)

    profiles = [profile_status(root, profile, tools, content, results) for profile in declaration["profiles"]]
    counts = {outcome: sum(1 for r in results.values() if r["outcome"] == outcome) for outcome in OUTCOMES + (UNAVAILABLE,)}
    decision = "fail" if deviations or (args.strict and unavailable) or \
        (args.strict_budget and over_budget) else "pass"
    evidence = {
        "schema": EVIDENCE_SCHEMA,
        "generated_at": datetime.datetime.now(datetime.timezone.utc).isoformat(),
        "declaration": os.path.relpath(args.declaration, root),
        "declaration_schema": SCHEMA,
        "identity": conformance.source_identity(root),
        "host": host_identity(),
        "selectors": {"groups": args.group or [], "checks": args.check or []},
        "tools": tools,
        "content": content,
        "checks": results,
        "profiles": profiles,
        "baselines": declaration["baselines"],
        "counts": counts,
        "deviations": deviations,
        "unavailable": unavailable,
        "over_budget": over_budget,
        "parallel": args.parallel,
        "seconds": elapsed,
        "strict": args.strict,
        "strict_budget": args.strict_budget,
        "decision": decision,
    }
    print("%d check(s): %s; %d deviation(s), %d unavailable, %d over budget in %.1fs -> %s" % (
        len(results), ", ".join("%d %s" % (n, k) for k, n in counts.items() if n),
        len(deviations), len(unavailable), len(over_budget), elapsed, decision.upper()))
    if out_dir:
        path = os.path.join(out_dir, "evidence.json")
        with open(path, "w", encoding="utf-8") as stream:
            json.dump(evidence, stream, indent=2)
            stream.write("\n")
        print("evidence: %s" % os.path.relpath(path, root))
    return 0 if decision == "pass" else 1


def build_parser():
    parser = argparse.ArgumentParser(description="Reproducible baseline and profile inventory (RFC 0005 Q0 / R01).")
    parser.add_argument("--root", default=conformance.repo_root(), help="repository root")
    parser.add_argument("--declaration", default=None, help="baseline declaration (default quality/baseline.json)")
    sub = parser.add_subparsers(dest="command", required=True)
    sub.add_parser("validate", help="validate the declaration and its linked records")
    sub.add_parser("probe", help="report tool and content availability on this host")
    audit = sub.add_parser("audit", help="probe, run checks, compare with the recorded baseline")
    audit.add_argument("--group", action="append", help="select a check group (repeatable)")
    audit.add_argument("--check", action="append", help="select a check id (repeatable)")
    audit.add_argument("--jobs", type=int, default=max(1, (os.cpu_count() or 2) - 2), help="build parallelism")
    audit.add_argument("--parallel", type=int, default=4,
        help="concurrent checks (default 4); serial-scheduled checks always run alone")
    audit.add_argument("--strict", action="store_true", help="also fail when a selected check is unavailable")
    audit.add_argument("--strict-budget", action="store_true",
        help="also fail when a check takes longer than its budget_seconds")
    audit.add_argument("--out", default=None, help="evidence directory ('-' to skip writing)")
    return parser


def main(argv=None):
    args = build_parser().parse_args(argv)
    args.root = os.path.abspath(args.root)
    args.declaration = os.path.abspath(args.declaration or os.path.join(args.root, "quality", "baseline.json"))
    try:
        if args.command == "validate":
            return cmd_validate(args)
        if args.command == "probe":
            return cmd_probe(args)
        return cmd_audit(args)
    except BaselineError as error:
        print("baseline: error: %s" % error, file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
