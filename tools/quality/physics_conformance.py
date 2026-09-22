#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
#
# VPhysics provider conformance runner (RFC 0004 / RFC 0005 Q-PHYSICS).
#
# Runs the shared provider suite (unittests/physicstest, built by Waf as
# `vphysics_conformance`) once per provider module -- never two providers in
# one process, because both export the same entry points -- then:
#
#   1. requires the reference provider (IVP) to pass every check, so a check
#      that fails on the legacy oracle is exposed as a wrong check;
#   2. compares each candidate's observations (surface table, parsed keyvalues,
#      decoded geometry, traces) against the reference within each
#      observation's declared tolerance;
#   3. proves sensitivity: each deliberately-broken provider fault must make the
#      checks it targets fail on the reference provider.
#
# Content comes from the staged runtime's real game files (Portal first, then
# HL2, as gameinfo.txt searches them). Nothing from the game is copied into the
# repository; fixtures are extracted into the (git-ignored) output directory.
#
# Dependency-free: Python 3 standard library only.
#
# ============================================================================

import argparse
import datetime
import json
import os
import re
import struct
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import conformance  # noqa: E402  (shared source identity; one owner)

EVIDENCE_SCHEMA = "physics-conformance-evidence/v1"
REFERENCE = "vphysics"

# Real collision models exercised by the suite: the Portal weighted cube, the
# floor button base (multi-convex ring), the turret, and Chell's 23-solid
# ragdoll (ragdoll constraints, per-bone solids).
PHY_FIXTURES = [
    "models/props/metal_box.phy",
    "models/props/button_base_reference.phy",
    "models/props/turret_01.phy",
    "models/player/chell.phy",
]

SURFACE_MANIFEST = "scripts/surfaceproperties_manifest.txt"

# Every collision model shipped in these packs is swept (corpus mode): each
# must decode, and its per-solid shape must agree with the oracle.
CORPUS_VPKS = ["portal/portal_pak_dir.vpk", "hl2/hl2_misc_dir.vpk"]

# Search order mirrors portal/gameinfo.txt: Portal VPK, Portal loose files,
# then the shared HL2 VPK and loose files.
SEARCH_PATHS = [
    ("vpk", "portal/portal_pak_dir.vpk"),
    ("dir", "portal"),
    ("vpk", "hl2/hl2_misc_dir.vpk"),
    ("dir", "hl2"),
]

# Sensitivity faults: each must make at least one check with one of these
# prefixes fail when injected into the reference provider.
FAULTS = {
    "surfaceprops-null": ["surfaceprops.data-never-null"],
    "surfaceprops-index": ["surfaceprops.unknown-name", "surfaceprops.name-roundtrip"],
    "pairhash-ordered": ["pairhash.symmetric-lookup"],
    "collision-unit-aabb": ["collide.bbox-aabb", "vcollide.solid-aabb"],
    "collision-trace-miss": ["trace.", "vcollide.solid-trace-down"],
    "vcollide-keyvalues": ["vcollide.load-keyvalues"],
    "sim-noop": ["sim.", "env.simulation-clock"],
}

LINE_RE = re.compile(r"^(PASS|FAIL) (\S+) (\S+?)(?:: (.*))?$")


# ---------------------------------------------------------------------------
# Content
# ---------------------------------------------------------------------------

class VpkDirectory:
    """Minimal reader for Valve VPK v1/v2 directory files."""

    def __init__(self, path):
        self.path = path
        self.entries = {}
        with open(path, "rb") as stream:
            data = stream.read()
        signature, version, tree_size = struct.unpack_from("<III", data, 0)
        if signature != 0x55AA1234 or version not in (1, 2):
            raise ValueError("%s: not a VPK directory (version %r)" % (path, version))
        header_size = 12 if version == 1 else 28
        self.data_base = header_size + tree_size
        self.dir_data = data
        pos = header_size

        def read_string():
            nonlocal pos
            end = data.index(b"\0", pos)
            text = data[pos:end].decode("utf-8", "replace")
            pos = end + 1
            return text

        while True:
            ext = read_string()
            if not ext:
                break
            while True:
                folder = read_string()
                if not folder:
                    break
                while True:
                    name = read_string()
                    if not name:
                        break
                    crc, preload, archive, offset, length, terminator = struct.unpack_from("<IHHIIH", data, pos)
                    pos += 18
                    preload_bytes = data[pos:pos + preload]
                    pos += preload
                    folder_part = "" if folder == " " else folder + "/"
                    self.entries[(folder_part + name + "." + ext).lower()] = (
                        archive, offset, length, preload_bytes)

    def read(self, relative):
        entry = self.entries.get(relative.lower())
        if entry is None:
            return None
        archive, offset, length, preload = entry
        if archive == 0x7FFF:
            start = self.data_base + offset
            return preload + self.dir_data[start:start + length]
        archive_path = self.path.replace("_dir.vpk", "_%03d.vpk" % archive)
        with open(archive_path, "rb") as stream:
            stream.seek(offset)
            return preload + stream.read(length)


class ContentResolver:
    def __init__(self, runtime):
        self.layers = []
        for kind, relative in SEARCH_PATHS:
            path = os.path.join(runtime, relative)
            if kind == "vpk" and os.path.isfile(path):
                self.layers.append(("vpk", path, VpkDirectory(path)))
            elif kind == "dir" and os.path.isdir(path):
                self.layers.append(("dir", path, None))
        if not self.layers:
            raise FileNotFoundError("no game content under %s" % runtime)

    def read(self, relative):
        for kind, path, vpk in self.layers:
            if kind == "vpk":
                data = vpk.read(relative)
            else:
                candidate = os.path.join(path, relative)
                data = open(candidate, "rb").read() if os.path.isfile(candidate) else None
            if data is not None:
                return data, "%s:%s" % (os.path.basename(path), relative)
        return None, None


def extract_fixtures(runtime, out_dir):
    resolver = ContentResolver(runtime)
    fixture_dir = os.path.join(out_dir, "fixtures")
    os.makedirs(fixture_dir, exist_ok=True)
    provenance = []

    def extract(relative):
        data, origin = resolver.read(relative)
        if data is None:
            raise FileNotFoundError("required fixture missing from runtime: %s" % relative)
        target = os.path.join(fixture_dir, relative.replace("/", "__"))
        with open(target, "wb") as stream:
            stream.write(data)
        provenance.append({"path": relative, "origin": origin, "bytes": len(data)})
        return target

    manifest_path = extract(SURFACE_MANIFEST)
    with open(manifest_path, "r", encoding="utf-8", errors="replace") as stream:
        surface_files = re.findall(r'"file"\s+"([^"]+)"', stream.read())
    if not surface_files:
        raise ValueError("surface property manifest lists no files")
    surfaces = [extract(name) for name in surface_files]
    phys = [extract(name) for name in PHY_FIXTURES]
    return surfaces, phys, provenance


def extract_corpus(runtime, out_dir):
    """Extracts every .phy in the corpus packs; returns (list file, count)."""
    corpus_dir = os.path.join(out_dir, "corpus")
    os.makedirs(corpus_dir, exist_ok=True)
    lines = []
    for relative in CORPUS_VPKS:
        path = os.path.join(runtime, relative)
        if not os.path.isfile(path):
            raise FileNotFoundError("corpus pack missing from runtime: %s" % relative)
        vpk = VpkDirectory(path)
        pack = os.path.basename(relative).replace("_dir.vpk", "")
        for name in sorted(k for k in vpk.entries if k.endswith(".phy")):
            target = os.path.join(corpus_dir, pack + "__" + name.replace("/", "__"))
            with open(target, "wb") as stream:
                stream.write(vpk.read(name))
            lines.append("%s:%s\t%s" % (pack, name, target))
    list_file = os.path.join(corpus_dir, "corpus.txt")
    with open(list_file, "w") as stream:
        stream.write("\n".join(lines) + "\n")
    return list_file, len(lines)


# ---------------------------------------------------------------------------
# Running and parsing
# ---------------------------------------------------------------------------

def parse_output(text):
    result = {"checks": [], "observations": {}, "tiers": {}, "summary": None}
    for line in text.splitlines():
        match = LINE_RE.match(line)
        if match:
            result["checks"].append({
                "status": match.group(1).lower(), "tier": match.group(2),
                "name": match.group(3), "detail": match.group(4) or ""})
            continue
        if line.startswith("OBS "):
            parts = line.split()
            if len(parts) >= 4:
                result["observations"][parts[1]] = {"tolerance": parts[2], "values": parts[3:]}
            continue
        if line.startswith("TIER "):
            parts = line.split()
            result["tiers"][parts[1]] = {"checks": int(parts[2]), "failed": int(parts[3])}
            continue
        match = re.match(r"^CONFORMANCE ([0-9]+) ([0-9]+)$", line)
        if match:
            if result["summary"] is not None:
                result["summary"] = "duplicate"
            else:
                result["summary"] = {"checks": int(match.group(1)), "failed": int(match.group(2))}
    return result


def run_provider(binary, library, surfaces, phys, env, timeout, fault=None, corpus=None):
    command = [binary, "--provider", library]
    for path in surfaces:
        command += ["--surfaceprops", path]
    for path in phys:
        command += ["--phy", path]
    if corpus:
        command += ["--corpus", corpus]
    if fault:
        command += ["--fault", fault]
    try:
        run = subprocess.run(command, capture_output=True, text=True, env=env, timeout=timeout)
        stdout, stderr, returncode = run.stdout, run.stderr, run.returncode
        outcome = None
    except subprocess.TimeoutExpired as exc:
        stdout = conformance._as_text(exc.stdout)
        stderr = conformance._as_text(exc.stderr)
        returncode, outcome = None, "timeout"
    parsed = parse_output(stdout)
    if outcome is None:
        if returncode is not None and returncode < 0:
            outcome = "crash"
        elif not isinstance(parsed["summary"], dict) or parsed["summary"]["checks"] == 0:
            # Missing, duplicate, or zero results are never a pass.
            outcome = "incomplete"
        elif parsed["summary"]["failed"] or returncode != 0:
            outcome = "fail"
        else:
            outcome = "pass"
    # The protocol summary must agree with the per-check records.
    if outcome in ("pass", "fail"):
        failed = sum(1 for c in parsed["checks"] if c["status"] == "fail")
        if failed != parsed["summary"]["failed"] or len(parsed["checks"]) != parsed["summary"]["checks"]:
            outcome = "incomplete"
    parsed.update({
        "command": command, "returncode": returncode, "outcome": outcome,
        "stderr_tail": conformance.tail(stderr, 20)})
    if outcome in ("crash", "timeout", "incomplete"):
        parsed["stdout_tail"] = conformance.tail(stdout, 20)
    return parsed


# ---------------------------------------------------------------------------
# Comparison
# ---------------------------------------------------------------------------

def values_match(reference, candidate, tolerance):
    if len(reference) != len(candidate):
        return False
    if tolerance == "x":
        return reference == candidate
    kind, amount = tolerance[0], float(tolerance[1:])
    for ref, cand in zip(reference, candidate):
        try:
            a, b = float(ref), float(cand)
        except ValueError:
            return False
        limit = amount if kind == "a" else amount * max(abs(a), 1e-6)
        if abs(a - b) > limit:
            return False
    return True


def compare_observations(reference, candidate):
    divergences = []
    for key in sorted(reference):
        ref = reference[key]
        cand = candidate.get(key)
        if cand is None:
            divergences.append({"key": key, "reference": ref["values"], "candidate": None})
        elif not values_match(ref["values"], cand["values"], ref["tolerance"]):
            divergences.append({"key": key, "tolerance": ref["tolerance"],
                                "reference": ref["values"], "candidate": cand["values"]})
    extra = sorted(set(candidate) - set(reference))
    return {"compared": len(reference), "divergent": len(divergences) + len(extra),
            "divergences": divergences, "unexpected_keys": extra}


def fault_detected(result, prefixes):
    return any(c["status"] == "fail" and any(c["name"].startswith(p) for p in prefixes)
               for c in result["checks"])


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def provider_library(build, name):
    return os.path.join(build, name, "lib%s.so" % name)


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build", default="build", help="Waf build directory")
    parser.add_argument("--runtime", default="run/runtime", help="staged game runtime with content")
    parser.add_argument("--providers", default="vphysics,vphysics_box3d",
                        help="comma-separated provider modules; %s is the reference" % REFERENCE)
    parser.add_argument("--require", choices=["all", "boot"], default="all",
                        help="tiers a candidate must pass (the reference must always pass all)")
    parser.add_argument("--out", default="quality-results/physics-conformance")
    parser.add_argument("--timeout", type=int, default=600)
    parser.add_argument("--skip-corpus", action="store_true",
                        help="diagnostic runs only; evidence records the skip and the gate fails")
    parser.add_argument("--skip-sensitivity", action="store_true",
                        help="diagnostic runs only; evidence records the skip and the gate fails")
    args = parser.parse_args(argv)

    root = conformance.repo_root()
    build = os.path.abspath(args.build)
    out_dir = os.path.abspath(args.out)
    os.makedirs(out_dir, exist_ok=True)
    binary = os.path.join(build, "unittests", "physicstest", "vphysics_conformance")
    providers = [p for p in args.providers.split(",") if p]
    if REFERENCE not in providers:
        providers.insert(0, REFERENCE)

    missing = [p for p in [binary] + [provider_library(build, n) for n in providers] if not os.path.isfile(p)]
    if missing:
        print("physics conformance: missing build outputs: %s" % ", ".join(missing))
        return 2

    surfaces, phys, provenance = extract_fixtures(os.path.abspath(args.runtime), out_dir)
    corpus, corpus_count = (None, 0) if args.skip_corpus else extract_corpus(os.path.abspath(args.runtime), out_dir)

    env = dict(os.environ)
    library_dirs = [os.path.join(build, d) for d in ("tier0", "vstdlib")]
    env["LD_LIBRARY_PATH"] = os.pathsep.join(library_dirs + [env.get("LD_LIBRARY_PATH", "")])

    results = {}
    for name in providers:
        results[name] = run_provider(binary, provider_library(build, name), surfaces, phys, env, args.timeout,
                                     corpus=corpus)

    reference = results[REFERENCE]
    gate_failures = []
    if reference["outcome"] != "pass":
        gate_failures.append("reference %s did not pass (%s)" % (REFERENCE, reference["outcome"]))

    comparisons = {}
    for name in providers:
        if name == REFERENCE:
            continue
        result = results[name]
        comparisons[name] = compare_observations(reference["observations"], result["observations"])
        if result["outcome"] in ("crash", "timeout", "incomplete"):
            gate_failures.append("%s %s" % (name, result["outcome"]))
            continue
        # A check the reference ran but the candidate skipped (for example a
        # construction returned null and its dependents never ran) is missing
        # coverage, which can never certify the candidate.
        ran = set((c["tier"], c["name"]) for c in result["checks"])
        missing = sorted(set((c["tier"], c["name"]) for c in reference["checks"]) - ran)
        comparisons[name]["missing_checks"] = ["%s %s" % m for m in missing]
        required = ["boot"] if args.require == "boot" else list(result["tiers"])
        missing_required = [m for m in missing if m[0] in required]
        if missing_required:
            gate_failures.append("%s: %d reference checks not run" % (name, len(missing_required)))
        for tier in required:
            counts = result["tiers"].get(tier)
            if not counts or counts["checks"] == 0 or counts["failed"]:
                gate_failures.append("%s tier %s: %s" % (name, tier, counts))
        if args.require == "all" and comparisons[name]["divergent"]:
            gate_failures.append("%s: %d observations diverge from %s" % (
                name, comparisons[name]["divergent"], REFERENCE))

    if args.skip_corpus:
        gate_failures.append("corpus skipped")

    sensitivity = {}
    if args.skip_sensitivity:
        gate_failures.append("sensitivity skipped")
    else:
        for fault, prefixes in sorted(FAULTS.items()):
            faulted = run_provider(binary, provider_library(build, REFERENCE), surfaces, phys, env,
                                   args.timeout, fault=fault)
            detected = faulted["outcome"] in ("fail", "crash") and fault_detected(faulted, prefixes)
            sensitivity[fault] = {"detected": detected, "outcome": faulted["outcome"],
                                  "failed_checks": [c["name"] for c in faulted["checks"] if c["status"] == "fail"]}
            if not detected:
                gate_failures.append("fault %s not detected" % fault)

    evidence = {
        "schema": EVIDENCE_SCHEMA,
        "generated": datetime.datetime.now(datetime.timezone.utc).isoformat(),
        "source": conformance.source_identity(root),
        "build": build,
        "reference": REFERENCE,
        "require": args.require,
        "fixtures": provenance,
        "corpus_models": corpus_count,
        "corpus_packs": CORPUS_VPKS,
        "providers": results,
        "comparisons": comparisons,
        "sensitivity": sensitivity,
        "gate_failures": gate_failures,
        "status": "pass" if not gate_failures else "fail",
        "reproduce": "python3 tools/quality/physics_conformance.py " + " ".join(argv if argv is not None else sys.argv[1:]),
    }
    evidence_path = os.path.join(out_dir, "evidence.json")
    with open(evidence_path, "w") as stream:
        json.dump(evidence, stream, indent=2)

    for name in providers:
        result = results[name]
        tiers = ", ".join("%s %d/%d" % (t, c["checks"] - c["failed"], c["checks"]) for t, c in sorted(result["tiers"].items()))
        print("%-16s %-10s %s" % (name, result["outcome"], tiers))
        for check in result["checks"]:
            if check["status"] == "fail":
                print("    FAIL %s %s: %s" % (check["tier"], check["name"], check["detail"]))
        if name in comparisons:
            comp = comparisons[name]
            print("    observations: %d compared, %d divergent; %d reference checks not run" % (
                comp["compared"], comp["divergent"], len(comp.get("missing_checks", []))))
            for div in comp["divergences"][:15]:
                print("      %s ref=%s got=%s" % (div["key"], " ".join(div["reference"]),
                                                  " ".join(div["candidate"]) if div["candidate"] else "(missing)"))
    for fault, info in sorted(sensitivity.items()):
        print("fault %-22s %s" % (fault, "detected" if info["detected"] else "NOT DETECTED"))
    print("Physics conformance: %s (%s)" % (evidence["status"], evidence_path))
    for failure in gate_failures:
        print("  " + failure)
    return 0 if not gate_failures else 1


if __name__ == "__main__":
    sys.exit(main())
