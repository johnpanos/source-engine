#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
#
# Physics benchmark acceptance gate (RFC 0013; RFC 0004 performance and
# determinism; RFC 0005 Q-PHYSICS).
#
# Runs the benchmark scenes of the shared VPhysics host (`vphysics_conformance
# --bench`, unittests/physicstest/test_vphysics_bench.cpp) once per process
# for every declared provider, workload and worker count, in interleaved
# rounds, then evaluates the gates declared in quality/budgets/physics-v1.json:
#
#   * every declared run completes: its checks pass, it reports one sample per
#     tick, and it neither crashes nor times out;
#   * determinism: a run repeats bitwise across rounds, and a provider that
#     claims worker-count invariance produces one digest for every count;
#   * scene quality: stack collapse, lost bodies, joint error and tunneling;
#   * timing: per-profile p95/p50 limits, the RFC 0004 review ratio against
#     IVP, and minimum worker speedups;
#   * the parallel-step capability contract (`--bench contract`).
#
# A gate is `required` (its failure fails the run) or `planned` (evaluated and
# reported, never certified). A rule may be a declared known gap with an
# owner: it keeps its gate open, and a known gap that starts passing is
# reported for review. `--sensitivity` injects host faults and requires the
# matching rule to fail, proving the gates can fail.
#
# Content (the Portal cube and surface properties) comes from the staged
# runtime, extracted into the git-ignored output directory as the conformance
# runner does. Dependency-free: Python 3 standard library only.
#
# ============================================================================

import argparse
import datetime
import json
import os
import platform
import statistics
import subprocess
import sys
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import conformance  # noqa: E402  (shared source identity; one owner)
import physics_conformance  # noqa: E402  (fixture extraction and output parsing; one owner)

EVIDENCE_SCHEMA = "physics-bench-evidence/v1"
BUDGET_SCHEMA = "physics-budget/v1"
DEFAULT_BUDGET = os.path.join("quality", "budgets", "physics-v1.json")
UNSUPPORTED_EXIT = 3


# ---------------------------------------------------------------------------
# Budget declaration
# ---------------------------------------------------------------------------

class BudgetError(Exception):
    pass


RULE_FIELDS = {
    "complete": [],
    "contract": ["provider"],
    "metric_max": ["workload", "provider", "metric", "max"],
    "digest_repeatable": ["workload", "provider"],
    "digest_invariant": ["workload", "provider"],
    "relative": ["workload", "stat", "candidate", "reference", "max_ratio"],
    "speedup": ["workload", "provider", "stat", "base_workers", "workers", "min_ratio"],
    "timing": ["workload", "provider", "workers", "stat", "limit_us"],
    "memory": ["workload", "provider", "workers", "limit_kb"],
}
STATS = ("p50", "p95", "p99", "max", "total")


def load_budget(path):
    with open(path) as stream:
        budget = json.load(stream)
    if budget.get("schema") != BUDGET_SCHEMA:
        raise BudgetError("%s: schema %r is not %s" % (path, budget.get("schema"), BUDGET_SCHEMA))
    workloads = budget.get("workloads") or {}
    if not workloads:
        raise BudgetError("no workloads declared")
    for name, workload in workloads.items():
        for key in ("scene", "count", "ticks", "seed", "workers"):
            if key not in workload:
                raise BudgetError("workload %s: missing %s" % (name, key))
        if not workload["workers"] or any(w < 0 for w in workload["workers"]):
            raise BudgetError("workload %s: workers must be a non-empty list of counts >= 0" % name)
    for gate_name, gate in (budget.get("gates") or {}).items():
        if gate.get("state") not in ("required", "planned"):
            raise BudgetError("gate %s: state must be required or planned" % gate_name)
        if not gate.get("owner"):
            raise BudgetError("gate %s: missing owner" % gate_name)
        for rule in gate_rules(budget, gate_name, None):
            check_rule(budget, gate_name, rule)
    for profile_name, profile in (budget.get("profiles") or {}).items():
        for rule in profile.get("rules", []):
            if rule.get("gate") not in budget.get("gates", {}):
                raise BudgetError("profile %s: rule for unknown gate %r" % (profile_name, rule.get("gate")))
            check_rule(budget, rule["gate"], rule)
    return budget


def check_rule(budget, gate_name, rule):
    kind = rule.get("kind")
    if kind not in RULE_FIELDS:
        raise BudgetError("gate %s: unknown rule kind %r" % (gate_name, kind))
    for field in RULE_FIELDS[kind]:
        if field not in rule:
            raise BudgetError("gate %s: %s rule missing %s" % (gate_name, kind, field))
    workload = rule.get("workload")
    if workload is not None and workload not in budget["workloads"]:
        raise BudgetError("gate %s: rule names unknown workload %s" % (gate_name, workload))
    if rule.get("stat") is not None and rule["stat"] not in STATS:
        raise BudgetError("gate %s: unknown stat %s" % (gate_name, rule["stat"]))
    gap = rule.get("known_gap")
    if gap is not None and not (isinstance(gap, dict) and gap.get("owner") and gap.get("note")):
        raise BudgetError("gate %s: a known gap needs an owner and a note" % gate_name)


def gate_rules(budget, gate_name, profile):
    """Profile-independent rules plus the selected profile's rules for the gate."""
    rules = list(budget["gates"][gate_name].get("rules", []))
    if profile is not None:
        rules += [r for r in profile.get("rules", []) if r.get("gate") == gate_name]
    return rules


def planned_runs(budget, providers):
    """Every (provider, workload, workers) the declaration asks for."""
    runs = []
    for name in sorted(budget["workloads"]):
        workload = budget["workloads"][name]
        for provider in providers:
            if provider == budget["reference"]:
                if workload.get("reference", True):
                    runs.append((provider, name, 0))
                continue
            for workers in workload["workers"]:
                runs.append((provider, name, workers))
    return runs


# ---------------------------------------------------------------------------
# Running and parsing
# ---------------------------------------------------------------------------

def parse_bench(text):
    parsed = physics_conformance.parse_output(text)
    parsed.update({"samples": None, "metrics": {}, "digest": None, "memory": None, "unsupported": None,
                   "bench": None, "build_us": None})
    for line in text.splitlines():
        parts = line.split()
        if not parts:
            continue
        if parts[0] == "SAMPLES":
            if parsed["samples"] is not None:
                parsed["samples"] = "duplicate"
            else:
                try:
                    parsed["samples"] = [float(v) for v in parts[1:]]
                except ValueError:
                    parsed["samples"] = "malformed"
        elif parts[0] == "METRIC" and len(parts) == 3:
            parsed["metrics"][parts[1]] = float(parts[2])
        elif parts[0] == "DIGEST" and len(parts) == 2:
            parsed["digest"] = parts[1]
        elif parts[0] == "MEMORY" and len(parts) == 5:
            start, built, end, peak = (int(v) for v in parts[1:])
            parsed["memory"] = {"rss_start_kb": start, "rss_built_kb": built, "rss_end_kb": end, "peak_kb": peak,
                                "growth_kb": peak - start}
        elif parts[0] == "UNSUPPORTED":
            parsed["unsupported"] = " ".join(parts[1:])
        elif parts[0] == "BENCH" and len(parts) == 5:
            parsed["bench"] = {"scene": parts[1], "workers": int(parts[2]), "bodies": int(parts[3]),
                               "constraints": int(parts[4])}
        elif parts[0] == "BUILD_US" and len(parts) == 2:
            parsed["build_us"] = float(parts[1])
    return parsed


def percentile(sorted_values, fraction):
    if not sorted_values:
        return None
    index = min(len(sorted_values) - 1, max(0, int(round(fraction * (len(sorted_values) - 1)))))
    return sorted_values[index]


def sample_stats(samples):
    ordered = sorted(samples)
    return {"p50": percentile(ordered, 0.50), "p95": percentile(ordered, 0.95), "p99": percentile(ordered, 0.99),
            "max": ordered[-1], "total": sum(ordered)}


def classify(parsed, returncode, timed_out, expect_samples):
    """Outcome of one bench process: pass, fail, unsupported, crash, timeout or incomplete."""
    if timed_out:
        return "timeout"
    if returncode is not None and returncode < 0:
        return "crash"
    if returncode == UNSUPPORTED_EXIT and parsed["unsupported"]:
        return "unsupported"
    summary = parsed["summary"]
    if not isinstance(summary, dict) or summary["checks"] == 0:
        return "incomplete"
    failed = sum(1 for c in parsed["checks"] if c["status"] == "fail")
    if failed != summary["failed"] or len(parsed["checks"]) != summary["checks"]:
        return "incomplete"
    if expect_samples is not None:
        samples = parsed["samples"]
        if not isinstance(samples, list) or len(samples) != expect_samples or parsed["digest"] is None:
            return "incomplete"
    if summary["failed"] or returncode != 0:
        return "fail"
    return "pass"


def run_bench(binary, library, surfaces, cube, workload, workers, env, timeout, fault=None):
    command = [binary, "--provider", library]
    for path in surfaces:
        command += ["--surfaceprops", path]
    if cube and workload.get("authored_cube"):
        command += ["--phy", cube]
    command += ["--bench", workload["scene"], "--count", str(workload["count"]), "--ticks", str(workload["ticks"]),
                "--workers", str(workers), "--seed", str(workload["seed"])]
    if fault:
        command += ["--fault", fault]
    timed_out = False
    try:
        run = subprocess.run(command, capture_output=True, text=True, env=env, timeout=timeout)
        stdout, stderr, returncode = run.stdout, run.stderr, run.returncode
    except subprocess.TimeoutExpired as exc:
        stdout, stderr = conformance._as_text(exc.stdout), conformance._as_text(exc.stderr)
        returncode, timed_out = None, True
    parsed = parse_bench(stdout)
    expect = None if workload["scene"] == "contract" else workload["ticks"]
    outcome = classify(parsed, returncode, timed_out, expect)
    result = {
        "command": command, "returncode": returncode, "outcome": outcome,
        "failed_checks": ["%s %s: %s" % (c["tier"], c["name"], c["detail"]) for c in parsed["checks"] if c["status"] == "fail"],
        "metrics": parsed["metrics"], "digest": parsed["digest"], "memory": parsed["memory"],
        "unsupported": parsed["unsupported"], "bench": parsed["bench"], "build_us": parsed["build_us"],
        "stats": sample_stats(parsed["samples"]) if isinstance(parsed["samples"], list) and parsed["samples"] else None,
    }
    if outcome in ("crash", "timeout", "incomplete"):
        result["stdout_tail"] = conformance.tail(stdout, 20)
        result["stderr_tail"] = conformance.tail(stderr, 20)
    return result


def key_of(provider, workload, workers):
    return "%s/%s/w%d" % (provider, workload, workers)


def aggregate(rounds):
    """Median of each statistic across rounds; metrics from the last round."""
    stats = [r["stats"] for r in rounds if r.get("stats")]
    merged = None
    if stats:
        merged = {name: statistics.median(s[name] for s in stats) for name in STATS}
    memory = [r["memory"]["growth_kb"] for r in rounds if r.get("memory")]
    outcomes = sorted(set(r["outcome"] for r in rounds))
    return {
        "outcome": outcomes[0] if len(outcomes) == 1 else "mixed:" + ",".join(outcomes),
        "rounds": len(rounds),
        "stats": merged,
        "memory_growth_kb": statistics.median(memory) if memory else None,
        "digests": [r.get("digest") for r in rounds],
        "metrics": rounds[-1].get("metrics", {}) if rounds else {},
        "failed_checks": sorted(set(f for r in rounds for f in r.get("failed_checks", []))),
        "bench": rounds[-1].get("bench") if rounds else None,
    }


# ---------------------------------------------------------------------------
# Gate evaluation (pure: tested by tools/quality/tests/test_physics_bench.py)
# ---------------------------------------------------------------------------

def matching(results, provider, workload, workers="all"):
    out = {}
    for key, value in results.items():
        p, w, n = key.split("/")
        if p == provider and w == workload and (workers == "all" or int(n[1:]) == workers):
            out[key] = value
    return out


TIMING_KINDS = ("relative", "speedup", "timing")


def evaluate_rule(rule, results, contract, contended=None):
    """Returns (ok, detail); ok is None when the rule's inputs were not run.

    Timing rules compare wall-clock statistics, which a contended host
    distorts; `contended` (a description, or None) makes them not-run.
    """
    kind = rule["kind"]
    if contended and kind in TIMING_KINDS:
        return None, "host contended: %s" % contended
    if kind == "complete":
        bad = sorted("%s %s" % (k, v["outcome"]) for k, v in results.items() if v["outcome"] not in ("pass", "unsupported"))
        return (not bad and bool(results)), ("; ".join(bad) if bad else "%d runs complete" % len(results))
    if kind == "contract":
        run = contract.get(rule["provider"])
        if run is None:
            return None, "contract not run"
        return run["outcome"] == "pass", "%s%s" % (run["outcome"], (": " + "; ".join(run["failed_checks"])) if run["failed_checks"] else "")
    if kind == "metric_max":
        runs = matching(results, rule["provider"], rule["workload"], rule.get("workers", "all"))
        if not runs:
            return None, "not run"
        worst, detail = None, []
        for key, run in sorted(runs.items()):
            value = run["metrics"].get(rule["metric"])
            if value is None:
                return False, "%s reported no %s" % (key, rule["metric"])
            worst = value if worst is None else max(worst, value)
            detail.append("%s=%g" % (key, value))
        return worst <= rule["max"], "%s <= %g: %s" % (rule["metric"], rule["max"], ", ".join(detail))
    if kind == "digest_repeatable":
        runs = matching(results, rule["provider"], rule["workload"], rule.get("workers", "all"))
        if not runs:
            return None, "not run"
        bad = [k for k, r in runs.items() if len(r["digests"]) < 2 or None in r["digests"] or len(set(r["digests"])) != 1]
        return not bad, ("differs across rounds (or fewer than 2 rounds): " + ", ".join(sorted(bad))) if bad else "%d configurations repeat bitwise" % len(runs)
    if kind == "digest_invariant":
        runs = matching(results, rule["provider"], rule["workload"])
        digests = set(d for r in runs.values() for d in r["digests"])
        if len(runs) < 2:
            return None, "fewer than two worker counts run"
        return (len(digests) == 1 and None not in digests), "%d worker counts, %d distinct digests" % (len(runs), len(digests))
    if kind in ("relative", "speedup", "timing"):
        stat = rule["stat"]
        if kind == "timing":
            run = results.get(key_of(rule["provider"], rule["workload"], rule["workers"]))
            if not run or not run["stats"]:
                return None, "not run"
            value = run["stats"][stat]
            return value <= rule["limit_us"], "%s %.1f us <= %.1f us" % (stat, value, rule["limit_us"])
        if kind == "relative":
            cand = results.get(key_of(rule["candidate"]["provider"], rule["workload"], rule["candidate"]["workers"]))
            ref = results.get(key_of(rule["reference"]["provider"], rule["workload"], rule["reference"]["workers"]))
            if not cand or not ref or not cand["stats"] or not ref["stats"]:
                return None, "not run"
            ratio = cand["stats"][stat] / max(ref["stats"][stat], 1e-9)
            return ratio <= rule["max_ratio"], "%s ratio %.3f <= %.3f" % (stat, ratio, rule["max_ratio"])
        base = results.get(key_of(rule["provider"], rule["workload"], rule["base_workers"]))
        wide = results.get(key_of(rule["provider"], rule["workload"], rule["workers"]))
        if not base or not wide or not base["stats"] or not wide["stats"]:
            return None, "not run"
        speedup = base["stats"][stat] / max(wide["stats"][stat], 1e-9)
        return speedup >= rule["min_ratio"], "%s speedup %d->%d workers %.2fx >= %.2fx" % (
            stat, rule["base_workers"], rule["workers"], speedup, rule["min_ratio"])
    if kind == "memory":
        run = results.get(key_of(rule["provider"], rule["workload"], rule["workers"]))
        if not run or run["memory_growth_kb"] is None:
            return None, "not run"
        return run["memory_growth_kb"] <= rule["limit_kb"], "peak growth %d KiB <= %d KiB" % (run["memory_growth_kb"], rule["limit_kb"])
    raise BudgetError("unknown rule kind %s" % kind)


def evaluate_gates(budget, profile, results, contract, contended=None):
    verdicts = {}
    for gate_name in sorted(budget["gates"]):
        gate = budget["gates"][gate_name]
        rules = []
        for rule in gate_rules(budget, gate_name, profile):
            ok, detail = evaluate_rule(rule, results, contract, contended)
            gap = rule.get("known_gap")
            if ok is None:
                status = "not-run"
            elif gap:
                status = "known-gap" if not ok else "known-gap-now-passes"
            else:
                status = "pass" if ok else "fail"
            rules.append({"rule": rule, "status": status, "detail": detail})
        statuses = set(r["status"] for r in rules)
        if not rules or "not-run" in statuses:
            verdict = "incomplete"
        elif "fail" in statuses:
            verdict = "fail"
        elif statuses & {"known-gap", "known-gap-now-passes"}:
            verdict = "open"
        else:
            verdict = "pass"
        verdicts[gate_name] = {"state": gate["state"], "owner": gate["owner"], "verdict": verdict, "rules": rules}
    return verdicts


def run_status(verdicts, profile_status):
    """The command fails when a required gate does not pass; planned gates only report."""
    failures = []
    for name, gate in sorted(verdicts.items()):
        if gate["state"] == "required" and gate["verdict"] != "pass":
            failures.append("required gate %s: %s" % (name, gate["verdict"]))
        for rule in gate["rules"]:
            if rule["status"] == "known-gap-now-passes":
                failures.append("gate %s: known gap now passes, review the declaration (owner %s)" % (
                    name, rule["rule"]["known_gap"]["owner"]))
    if profile_status != "measured":
        failures.append("profile is %s: timing limits are not certified" % profile_status)
    return failures


def contention(load_samples, logical_cpus, max_load_per_cpu):
    """A description when any sampled 1-minute load per CPU exceeds the limit."""
    if max_load_per_cpu is None or not load_samples:
        return None
    worst = max(load_samples)
    per_cpu = worst / max(1, logical_cpus)
    if per_cpu > max_load_per_cpu:
        return "1-minute load %.1f on %d CPUs (%.2f per CPU > %.2f)" % (worst, logical_cpus, per_cpu, max_load_per_cpu)
    return None


# ---------------------------------------------------------------------------
# Sensitivity: each host fault must make its target rule fail
# ---------------------------------------------------------------------------

def sensitivity_cases(budget):
    return budget.get("sensitivity", [])


def rule_by_id(budget, profile, rule_id):
    for gate_name in budget["gates"]:
        for rule in gate_rules(budget, gate_name, profile):
            if rule.get("id") == rule_id:
                return rule
    raise BudgetError("sensitivity names unknown rule %s" % rule_id)


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def host_description():
    cpu = platform.processor() or platform.machine()
    try:
        with open("/proc/cpuinfo") as stream:
            for line in stream:
                if line.startswith("model name"):
                    cpu = line.split(":", 1)[1].strip()
                    break
    except OSError:
        pass
    load = os.getloadavg() if hasattr(os, "getloadavg") else None
    return {"cpu": cpu, "logical_cpus": os.cpu_count(), "system": platform.platform(), "load_average": load}


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--build", default="build", help="Waf build directory")
    parser.add_argument("--runtime", default="run/runtime", help="staged game runtime with content")
    parser.add_argument("--budget", default=DEFAULT_BUDGET)
    parser.add_argument("--profile", default="linux-x86_64-desktop", help="budget profile whose limits apply")
    parser.add_argument("--rounds", type=int, default=3, help="interleaved rounds (determinism needs at least 2)")
    parser.add_argument("--workload", action="append", help="limit to these workloads (diagnostic; gates report not-run)")
    parser.add_argument("--sensitivity", action="store_true", help="also prove each declared fault fails its rule")
    parser.add_argument("--out", default="quality-results/physics-bench")
    parser.add_argument("--timeout", type=int, default=900)
    parser.add_argument("--wait-quiet", type=int, default=0, metavar="SECONDS",
                        help="before each round, wait up to SECONDS for the host load to fall under the profile's limit")
    args = parser.parse_args(argv)

    root = conformance.repo_root()
    budget = load_budget(os.path.join(root, args.budget) if not os.path.isabs(args.budget) else args.budget)
    profile = (budget.get("profiles") or {}).get(args.profile)
    if profile is None:
        print("physics bench: unknown profile %s" % args.profile)
        return 2
    build = os.path.abspath(args.build)
    out_dir = os.path.abspath(args.out)
    os.makedirs(out_dir, exist_ok=True)
    binary = os.path.join(build, "unittests", "physicstest", "vphysics_conformance")
    providers = budget["providers"]
    libraries = {p: physics_conformance.provider_library(build, p) for p in providers}
    missing = [p for p in [binary] + list(libraries.values()) if not os.path.isfile(p)]
    if missing:
        print("physics bench: missing build outputs: %s" % ", ".join(missing))
        return 2
    if args.rounds < 1:
        print("physics bench: --rounds must be at least 1")
        return 2

    surfaces, phys, provenance = physics_conformance.extract_fixtures(os.path.abspath(args.runtime), out_dir)
    cube = next(p for p in phys if isinstance(p, str) and p.endswith("metal_box.phy"))
    env = dict(os.environ)
    env["LD_LIBRARY_PATH"] = os.pathsep.join([os.path.join(build, d) for d in ("tier0", "vstdlib")] + [env.get("LD_LIBRARY_PATH", "")])

    runs = planned_runs(budget, providers)
    if args.workload:
        runs = [r for r in runs if r[1] in args.workload]
    rounds = {key_of(*r): [] for r in runs}
    load_samples = []

    def sample_load():
        if hasattr(os, "getloadavg"):
            load_samples.append(os.getloadavg()[0])

    limit = profile.get("max_load_per_cpu")

    def wait_quiet():
        deadline = time.monotonic() + args.wait_quiet
        while limit is not None and hasattr(os, "getloadavg") and time.monotonic() < deadline:
            if os.getloadavg()[0] / max(1, os.cpu_count() or 1) <= limit:
                return
            time.sleep(15)

    for round_index in range(args.rounds):
        wait_quiet()
        sample_load()
        # Interleave: every configuration once per round, rotated so no
        # configuration always runs first on a warming or loaded host.
        order = runs[round_index % len(runs):] + runs[:round_index % len(runs)] if runs else []
        for provider, workload, workers in order:
            result = run_bench(binary, libraries[provider], surfaces, cube, budget["workloads"][workload], workers,
                               env, args.timeout)
            rounds[key_of(provider, workload, workers)].append(result)
            stats = result["stats"]
            print("round %d %-38s %-11s %s" % (round_index + 1, key_of(provider, workload, workers), result["outcome"],
                  ("p50 %.0f p95 %.0f us" % (stats["p50"], stats["p95"])) if stats else ""), flush=True)
    sample_load()
    results = {key: aggregate(value) for key, value in rounds.items()}
    contended = contention(load_samples, os.cpu_count() or 1, profile.get("max_load_per_cpu"))

    contract = {}
    contract_workload = {"scene": "contract", "count": 1, "ticks": 1, "seed": 1, "authored_cube": True}
    for provider in providers:
        contract[provider] = run_bench(binary, libraries[provider], surfaces, cube, contract_workload, 0, env, args.timeout)

    verdicts = evaluate_gates(budget, profile, results, contract, contended)
    failures = run_status(verdicts, profile.get("status", "unverified"))
    if contended:
        failures.append("timing rules not evaluated, host contended: %s" % contended)
    if args.workload:
        failures.append("diagnostic run limited to %s" % ", ".join(args.workload))

    sensitivity = {}
    if args.sensitivity:
        for case in sensitivity_cases(budget):
            rule = rule_by_id(budget, profile, case["rule"])
            provider = case["provider"]
            if case["workload"] == "contract":
                faulted = run_bench(binary, libraries[provider], surfaces, cube, contract_workload, 0, env,
                                    args.timeout, fault=case["fault"])
                ok, detail = evaluate_rule(rule, results, {provider: faulted})
            else:
                workload = budget["workloads"][case["workload"]]
                faulted_results = dict(results)
                for workers in case.get("workers", [0]):
                    faulted = run_bench(binary, libraries[provider], surfaces, cube, workload, workers, env,
                                        args.timeout, fault=case["fault"])
                    # Two identical faulted rounds so repeat checks stay meaningful.
                    faulted_results[key_of(provider, case["workload"], workers)] = aggregate([faulted, faulted])
                # Proves the rule can fail, so host contention does not exempt it.
                ok, detail = evaluate_rule(rule, faulted_results, contract)
            detected = ok is False
            sensitivity[case["fault"]] = {"rule": case["rule"], "detected": detected, "detail": detail}
            if not detected:
                failures.append("fault %s not detected by rule %s (%s)" % (case["fault"], case["rule"], detail))
    else:
        failures.append("sensitivity skipped")

    evidence = {
        "schema": EVIDENCE_SCHEMA,
        "generated": datetime.datetime.now(datetime.timezone.utc).isoformat(),
        "source": conformance.source_identity(root),
        "host": host_description(),
        "build": build,
        "budget": args.budget,
        "profile": args.profile,
        "rounds": args.rounds,
        "load_samples": load_samples,
        "contended": contended,
        "fixtures": provenance,
        "runs": rounds,
        "results": results,
        "contract": contract,
        "gates": verdicts,
        "sensitivity": sensitivity,
        "failures": failures,
        "status": "pass" if not failures else "fail",
        "reproduce": "python3 tools/quality/physics_bench.py " + " ".join(argv if argv is not None else sys.argv[1:]),
    }
    evidence_path = os.path.join(out_dir, "evidence.json")
    with open(evidence_path, "w") as stream:
        json.dump(evidence, stream, indent=2)

    for name, gate in sorted(verdicts.items()):
        print("gate %-14s %-9s %s (owner %s)" % (name, gate["state"], gate["verdict"], gate["owner"]))
        for rule in gate["rules"]:
            if rule["status"] != "pass":
                print("    %-20s %s %s" % (rule["status"], rule["rule"].get("id", rule["rule"]["kind"]), rule["detail"]))
    for fault, info in sorted(sensitivity.items()):
        print("fault %-28s %s" % (fault, "detected" if info["detected"] else "NOT DETECTED"))
    print("Physics bench: %s (%s)" % (evidence["status"], evidence_path))
    for failure in failures:
        print("  " + failure)
    return 0 if not failures else 1


if __name__ == "__main__":
    sys.exit(main())
