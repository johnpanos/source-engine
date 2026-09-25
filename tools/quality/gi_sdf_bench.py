#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
#
# SDF GI performance fixtures (RFC 0011 G6/G7; RFC 0005 evidence).
#
# Runs the declared workloads of quality/budgets/indirect-sdf-perf-v1.json
# in interleaved rounds (one process per workload per round, medians across
# rounds) and judges every declared row against its budget and its reviewed
# baseline:
#
#   * CPU workloads (`indirect_sdf_bench`, unittests/rendertest): the SDF
#     producer's Schedule and the host's work per published volume, and the
#     baked direct light's occlusion build and compose;
#   * GPU workloads (`--gpu`; `indirect_sdf_native_conformance --bench`,
#     unittests/shaderapivulkantest): the trace dispatch's timestamps and
#     Schedule's CPU time on the device. Budgets named by `budget_ref` come
#     from quality/budgets/indirect-light-v1.json, their one owner.
#
# A row is `regressed` (fails) when it measures above max(budget, baseline)
# by more than the fixture's tolerance; `debt` when its reviewed baseline is
# over budget (the burn-down list, ranked, with owners); `improved` when it
# measures well under its baseline (`--update-baselines` then lowers the
# baseline; baselines are never raised automatically); otherwise `within`.
# A missing scene leaves its rows `unverified`, a crash, a failed oracle, a
# stage that measured nothing or a result that does not repeat fails.
# `--sensitivity` makes named stages slower and requires their rows to
# regress, proving the gate can fail.
#
# Synthetic scenes (probe volume and signed distance field sized like the
# real maps) are generated deterministically with the formats' own writers
# (probe_volume.build, sdf_volume.build) and cached by specification. Map
# scenes are the pipeline's outputs under quality-results/, optional.
#
# ============================================================================

import argparse
import datetime
import fnmatch
import hashlib
import json
import os
import re
import shutil
import statistics
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import conformance  # noqa: E402  (build commands, identities, bounded execution; one owner)

SCHEMA = "gi-sdf-perf/v1"
EVIDENCE_SCHEMA = "gi-sdf-perf-evidence/v1"
DEFAULT_FIXTURES = os.path.join("quality", "budgets", "indirect-sdf-perf-v1.json")
BENCHES = ("producer", "occlusion", "gpu")
VERDICTS = ("regressed", "failed", "missing", "unverified", "debt", "improved", "within")
FAILING = ("regressed", "failed", "missing")
BENCH_LINE = re.compile(r"^BENCH (\{.*\})\s*$", re.MULTILINE)
CONFORMANCE_LINE = re.compile(r"^CONFORMANCE (\d+) (\d+)\s*$", re.MULTILINE)


class FixtureError(Exception):
    pass


# ---------------------------------------------------------------------------
# Fixture declaration
# ---------------------------------------------------------------------------

def load_fixtures(path):
    with open(path) as stream:
        data = json.load(stream)
    validate_fixtures(data, path)
    return data


def validate_fixtures(data, where="fixtures"):
    if data.get("schema") != SCHEMA:
        raise FixtureError("%s: schema %r is not %s" % (where, data.get("schema"), SCHEMA))
    for key in ("suites", "scenes", "workloads", "profiles", "tolerance"):
        if not isinstance(data.get(key), dict):
            raise FixtureError("%s: %r must be an object" % (where, key))
    tolerance = data["tolerance"]
    if not (tolerance.get("ratio", 0) > 1.0 and tolerance.get("absolute_ms", -1) >= 0
            and 0 < tolerance.get("improved_ratio", 0) < 1.0):
        raise FixtureError("%s: tolerance needs ratio > 1, absolute_ms >= 0 and "
                           "0 < improved_ratio < 1" % where)
    for name in ("cpu", "gpu"):
        if not data["suites"].get(name):
            raise FixtureError("%s: suites.%s names no manifest suite" % (where, name))
    for name, scene in data["scenes"].items():
        kinds = [k for k in ("files", "synthetic") if k in scene]
        if len(kinds) != 1:
            raise FixtureError("scene %s: exactly one of files or synthetic" % name)
        if "files" in scene and not scene["files"].get("prbv") and not scene["files"].get("wmsh"):
            raise FixtureError("scene %s: files name no prbv or wmsh" % name)
    for name, workload in data["workloads"].items():
        if workload.get("bench") not in BENCHES:
            raise FixtureError("workload %s: bench must be one of %s" % (name, ", ".join(BENCHES)))
        if workload.get("scene") and workload["scene"] not in data["scenes"]:
            raise FixtureError("workload %s: unknown scene %s" % (name, workload["scene"]))
        if not workload.get("scene") and not workload.get("synthetic_texels"):
            raise FixtureError("workload %s: names no scene" % name)
    for profile_id, profile in data["profiles"].items():
        rows = profile.get("rows")
        if not isinstance(rows, list) or not rows:
            raise FixtureError("profile %s: rows must be a nonempty list" % profile_id)
        seen = set()
        for row in rows:
            key = (row.get("workload"), row.get("metric"))
            if row.get("workload") not in data["workloads"]:
                raise FixtureError("profile %s: row names unknown workload %r"
                                   % (profile_id, row.get("workload")))
            if not row.get("metric") or not row.get("owner"):
                raise FixtureError("profile %s: row %s needs metric and owner" % (profile_id, key))
            if ("budget_ms" in row) == ("budget_ref" in row):
                raise FixtureError("profile %s: row %s needs exactly one of budget_ms or "
                                   "budget_ref" % (profile_id, key))
            baseline = row.get("baseline_ms")
            if baseline is not None and not (isinstance(baseline, (int, float)) and baseline >= 0):
                raise FixtureError("profile %s: row %s baseline_ms must be >= 0 or null"
                                   % (profile_id, key))
            if key in seen:
                raise FixtureError("profile %s: duplicate row %s" % (profile_id, key))
            seen.add(key)
    for item in data.get("sensitivity", []):
        if item.get("workload") not in data["workloads"] or not item.get("seed_slow") \
                or not item.get("metric"):
            raise FixtureError("sensitivity entry %r needs a known workload, seed_slow and "
                               "metric" % item)


def resolve_budget(row, budgets, light_profile):
    """A row's budget in ms: its own, or `budget_ref` (producer.field) in the
    indirect-light budget file's profile."""
    if "budget_ms" in row:
        return row["budget_ms"]
    producer, _, field = row["budget_ref"].partition(".")
    try:
        return budgets["profiles"][light_profile]["budgets"][producer][field]
    except KeyError:
        raise FixtureError("budget_ref %s is not in the indirect-light budgets of %s"
                           % (row["budget_ref"], light_profile))


# ---------------------------------------------------------------------------
# Scenes
# ---------------------------------------------------------------------------

def generator_digest():
    """The synthetic scenes' generator: build_synthetic and the formats'
    writers."""
    import inspect
    here = os.path.dirname(os.path.abspath(__file__))
    digest = hashlib.sha256(inspect.getsource(build_synthetic).encode())
    for name in ("probe_volume.py", "sdf_volume.py"):
        with open(os.path.join(here, name), "rb") as stream:
            digest.update(stream.read())
    return digest.hexdigest()


def build_synthetic(spec):
    """(prbv bytes, sdfv bytes, focus probe indices) of a synthetic scene:
    a probe grid in a box room divided by slabs with doorways, a switchable
    rectangle light (style 32) under the ceiling and fixed sphere lights,
    listed in 2 m light cells by range."""
    import numpy as np
    import probe_volume
    import sdf_volume

    rng = np.random.default_rng(int(spec.get("seed", 1)))
    dims = [int(v) for v in spec["probe_dims"]]
    spacing = [float(v) for v in spec["probe_spacing"]]
    origin = np.asarray(spec.get("origin", [0.0, 0.0, 0.0]), np.float64)
    count = int(np.prod(dims))
    tile = probe_volume.IRRADIANCE_TILE - 2
    total = rng.uniform(0.05, 1.0, size=(count, 1, 1, 3)) * \
        rng.uniform(0.8, 1.2, size=(count, tile, tile, 1))
    irradiance = np.stack([total, 0.5 * total]).astype(np.float32)
    max_distance = 2.0 * max(spacing)
    active = (rng.uniform(size=count) < float(spec.get("active_fraction", 1.0))).astype(float)
    grid = {"origin": origin.tolist(), "spacing": spacing, "dims": dims,
            "max_relocation": 0.25 * min(spacing), "max_distance": max_distance,
            "irradiance": irradiance,
            "visibility": probe_volume.uniform_visibility(count, 0.5 * max_distance),
            "offsets": np.zeros((count, 3)), "active": active}
    prbv = probe_volume.build([grid], 2)

    # The room: the probe grid's bounds grown a spacing, with slabs across x
    # every `room_every` probes (a doorway in each).
    lo = origin - np.asarray(spacing)
    hi = origin + (np.asarray(dims) - 1) * np.asarray(spacing) + np.asarray(spacing)
    voxel = float(spec["voxel"])
    vdims = [int(v) for v in spec["voxel_dims"]] if "voxel_dims" in spec else \
        [int(np.ceil((hi[k] - lo[k]) / voxel)) + 5 for k in range(3)]
    vlo = lo - 2 * voxel
    zs, ys, xs = np.meshgrid(*(vlo[k] + np.arange(vdims[k]) * voxel for k in (2, 1, 0)),
                             indexing="ij")
    points = np.stack([xs, ys, zs], axis=-1).astype(np.float32)

    def box_distance(p, a, b):
        centre, half = (a + b) / 2, (b - a) / 2
        q = np.abs(p - centre) - half
        return np.linalg.norm(np.maximum(q, 0), axis=-1) + np.minimum(q.max(axis=-1), 0)

    distance = -box_distance(points, lo, hi)
    every = int(spec.get("room_every", 4))
    door = 0.5 * min(spacing)
    for i in range(every, dims[0] - 1, every):
        x = origin[0] + (i + 0.5) * spacing[0]
        mid_y = 0.5 * (lo[1] + hi[1])
        for y0, y1 in ((lo[1], mid_y - door), (mid_y + door, hi[1])):
            slab = box_distance(points, np.array([x - 4.0, y0, lo[2]]),
                                np.array([x + 4.0, y1, hi[2]]))
            distance = np.minimum(distance, slab)
    max_clamp = 8.0 * voxel
    distance = np.clip(distance, -max_clamp, max_clamp)
    shape = distance.shape
    reflectance = np.full(shape + (3,), 0.5, np.float32)
    emission = np.zeros(shape + (3,), np.float32)
    source = np.full(shape, sdf_volume.NO_SOURCE)

    centre = 0.5 * (lo + hi)
    lights = [{"kind": "rect", "style": 32, "rgb": (5.0, 5.0, 5.0),
               "a": (centre[0], centre[1], hi[2] - 8.0), "b": (32.0, 0.0, 0.0),
               "c": (0.0, -32.0, 0.0)}]
    for _ in range(int(spec.get("lights", 1)) - 1):
        at = lo + rng.uniform(0.1, 0.9, size=3) * (hi - lo)
        lights.append({"kind": "sphere", "style": -1, "rgb": (200.0, 180.0, 160.0),
                       "a": tuple(at.tolist()), "b": (4.0, 0.0, 0.0)})
    cell = float(spec.get("cell", 78.74))
    cdims = [max(1, int(np.ceil((hi[k] - lo[k] + 4 * voxel) / cell))) for k in range(3)]
    reach = float(spec.get("light_range", 600.0))
    lists = []
    for z in range(cdims[2]):
        for y in range(cdims[1]):
            for x in range(cdims[0]):
                c = vlo + (np.array([x, y, z]) + 0.5) * cell
                lists.append([i for i, light in enumerate(lights)
                              if np.linalg.norm(np.asarray(light["a"]) - c) <= reach
                              + 0.87 * cell])
    cells = {"origin": vlo.tolist(), "size": cell, "dims": cdims, "lists": lists}
    sdfv = sdf_volume.build(vlo.tolist(), voxel, vdims, distance, reflectance, emission, source,
                            lights, max_clamp, cells)

    # The focus: active probes within `focus_radius` of the grid's centre.
    focus = []
    radius = float(spec.get("focus_radius", 0.0))
    for i in range(count):
        index = np.array([i % dims[0], (i // dims[0]) % dims[1], i // (dims[0] * dims[1])])
        at = origin + index * np.asarray(spacing)
        if active[i] and np.linalg.norm(at - centre) <= radius:
            focus.append(i)
    return prbv, sdfv, focus


def scene_files(root, name, scene, cache_dir):
    """{prbv, sdfv, wmsh, lmap, focus} paths of a scene, or None when a
    declared file is absent (the scene's rows are then unverified)."""
    if "files" in scene:
        paths = {}
        for key, relative in scene["files"].items():
            path = os.path.join(root, relative)
            if not os.path.exists(path):
                return None
            paths[key] = path
        return paths
    spec = scene["synthetic"]
    key = hashlib.sha256((json.dumps(spec, sort_keys=True) + generator_digest())
                         .encode()).hexdigest()[:16]
    directory = os.path.join(cache_dir, "%s-%s" % (name, key))
    paths = {"prbv": os.path.join(directory, "volume.prbv"),
             "sdfv": os.path.join(directory, "field.sdfv"),
             "focus": os.path.join(directory, "focus.txt")}
    if not all(os.path.exists(p) for p in paths.values()):
        # Scenes of an older specification or generator are stale.
        if os.path.isdir(cache_dir):
            for entry in os.listdir(cache_dir):
                if entry.rsplit("-", 1)[0] == name and entry != os.path.basename(directory):
                    shutil.rmtree(os.path.join(cache_dir, entry), ignore_errors=True)
        os.makedirs(directory, exist_ok=True)
        prbv, sdfv, focus = build_synthetic(spec)
        for path, data in ((paths["prbv"], prbv), (paths["sdfv"], sdfv)):
            with open(path + ".tmp", "wb") as stream:
                stream.write(data)
            os.replace(path + ".tmp", path)
        with open(paths["focus"] + ".tmp", "w") as stream:
            stream.write("\n".join(str(p) for p in focus) + "\n")
        os.replace(paths["focus"] + ".tmp", paths["focus"])
    return paths


# ---------------------------------------------------------------------------
# Runs
# ---------------------------------------------------------------------------

def workload_argv(binary, workload, files, seed_slow=None):
    bench = workload["bench"]
    proxy = [str(v) for v in workload.get("proxy", [])]
    if bench == "producer":
        argv = [binary, "producer", files["prbv"], files["sdfv"]]
        for flag in ("frames", "change", "budget"):
            if flag in workload:
                argv += ["--" + flag, str(workload[flag])]
        if workload.get("focus") and files.get("focus"):
            argv += ["--focus", files["focus"]]
    elif bench == "occlusion":
        if workload.get("synthetic_texels"):
            argv = [binary, "occlusion", "--synthetic", str(workload["synthetic_texels"]),
                    str(workload.get("synthetic_lights", 1))]
        else:
            argv = [binary, "occlusion", files["wmsh"], files["lmap"], files["sdfv"]]
        for flag in ("repeats", "workers"):
            if flag in workload:
                argv += ["--" + flag, str(workload[flag])]
    else:
        argv = [binary, "--bench", files["prbv"], files["sdfv"]]
        if workload.get("focus") and files.get("focus"):
            argv += ["--focus", files["focus"]]
        for flag in ("frames", "budget"):
            if flag in workload:
                argv += ["--" + flag, str(workload[flag])]
    if proxy:
        argv += ["--proxy"] + proxy
    if seed_slow:
        argv += ["--seed-slow", seed_slow]
    return argv


def parse_run(stdout, bench):
    """(bench record or None, problem or None) from one process's output."""
    records = BENCH_LINE.findall(stdout)
    if len(records) != 1:
        return None, "expected one BENCH record, got %d" % len(records)
    try:
        record = json.loads(records[0])
    except ValueError as e:
        return None, "unparseable BENCH record: %s" % e
    if bench != "gpu":
        verdicts = CONFORMANCE_LINE.findall(stdout)
        if len(verdicts) != 1:
            return record, "expected one CONFORMANCE record, got %d" % len(verdicts)
        checks, failures = int(verdicts[0][0]), int(verdicts[0][1])
        if checks == 0 or failures:
            return record, "oracle: %d of %d checks failed" % (failures, checks)
    return record, None


def metric_value(record, metric):
    """A dotted metric of a record ("frame_publish.median_ms"), or None when
    absent or measured over zero samples."""
    value = record
    parts = metric.split(".")
    for part in parts:
        if not isinstance(value, dict) or part not in value:
            return None
        value = value[part]
    if len(parts) > 1 and isinstance(record.get(parts[0]), dict) \
            and record[parts[0]].get("count") == 0:
        return None
    if isinstance(value, list):  # the GPU bench's [before, after] pairs
        value = value[-1]
    return float(value) if isinstance(value, (int, float)) else None


def judge(row, budget, values, tolerance):
    """A row's verdict from its per-round values (None: not measured)."""
    result = {"workload": row["workload"], "metric": row["metric"], "owner": row["owner"],
              "budget_ms": budget, "baseline_ms": row.get("baseline_ms"),
              "rounds": values}
    if not values or any(v is None for v in values):
        result["verdict"] = "missing"
        result["detail"] = "the metric was not measured in every round"
        return result
    measured = statistics.median(values)
    result["measured_ms"] = round(measured, 4)
    baseline = row.get("baseline_ms")
    ceiling = max(budget, baseline if baseline is not None else budget)
    limit = ceiling * tolerance["ratio"] + tolerance["absolute_ms"]
    result["limit_ms"] = round(limit, 4)
    if measured > limit:
        result["verdict"] = "regressed"
        result["detail"] = "%.4f ms is over %.4f (max of budget and baseline, with tolerance)" \
            % (measured, limit)
    elif baseline is not None and measured < baseline * tolerance["improved_ratio"] \
            and baseline - measured > tolerance["absolute_ms"]:
        result["verdict"] = "improved"
        result["detail"] = "under the baseline %.4f: lower it (--update-baselines)" % baseline
    elif (baseline if baseline is not None else measured) > budget:
        result["verdict"] = "debt"
    else:
        result["verdict"] = "within"
    reference = baseline if baseline is not None else measured
    result["debt_ms"] = round(max(0.0, reference - budget), 4)
    return result


def run_workloads(root, fixtures, names, binaries, files_by_scene, rounds, timeout, env_by_bench,
                  seed_slow=None, log=print):
    """{workload: {"records": [...], "problems": [...]}} over interleaved rounds."""
    runs = {name: {"records": [], "problems": [], "argv": None} for name in names}
    for index in range(rounds):
        for name in names:
            workload = fixtures["workloads"][name]
            files = files_by_scene.get(workload.get("scene"), {})
            argv = workload_argv(binaries[workload["bench"]], workload, files,
                                 (seed_slow or {}).get(name))
            runs[name]["argv"] = argv
            code, stdout, stderr, timed_out = conformance.execute(
                argv, timeout, None, env_by_bench[workload["bench"]])
            record, problem = parse_run(stdout, workload["bench"])
            if timed_out:
                problem = "timed out after %d s" % timeout
            elif code != 0 and problem is None:
                problem = "exit status %d" % code
            if problem:
                runs[name]["problems"].append("round %d: %s; stderr: %s"
                                              % (index + 1, problem, conformance.tail(stderr, 5)))
            runs[name]["records"].append(record)
            log("  round %d/%d %-44s %s" % (index + 1, rounds, name, problem or "ok"))
    for name, run in runs.items():
        digests = {r.get("digest") for r in run["records"] if r and r.get("digest")}
        if len(digests) > 1:
            run["problems"].append("the result does not repeat across rounds (digests %s)"
                                   % ", ".join(sorted(digests)))
    return runs


def evaluate(fixtures, profile_id, runs, budgets, unavailable):
    """Row results for a profile: runs of each workload, or `unverified` for
    workloads not run (unavailable: {workload: reason})."""
    profile = fixtures["profiles"][profile_id]
    tolerance = fixtures["tolerance"]
    results = []
    for row in profile["rows"]:
        budget = resolve_budget(row, budgets, profile.get("indirect_light_profile"))
        name = row["workload"]
        if name in unavailable:
            results.append({"workload": name, "metric": row["metric"], "owner": row["owner"],
                            "budget_ms": budget, "baseline_ms": row.get("baseline_ms"),
                            "verdict": "unverified", "detail": unavailable[name]})
            continue
        if name not in runs:
            continue
        run = runs[name]
        values = [metric_value(r, row["metric"]) if r else None for r in run["records"]]
        result = judge(row, budget, values, tolerance)
        if run["problems"]:
            result["verdict"] = "failed"
            result["detail"] = "; ".join(run["problems"][:3])
        results.append(result)
    return results


def update_baselines(fixtures, profile_id, results, calibrate=False):
    """Lowers improved rows' baselines to their measurement (all measured
    rows with calibrate); returns the changes."""
    by_key = {(r["workload"], r["metric"]): r for r in results}
    changes = []
    for row in fixtures["profiles"][profile_id]["rows"]:
        result = by_key.get((row["workload"], row["metric"]))
        if not result or "measured_ms" not in result or result["verdict"] in ("failed", "missing"):
            continue
        if calibrate or result["verdict"] == "improved":
            new = round(result["measured_ms"], 3)
            changes.append((row["workload"], row["metric"], row.get("baseline_ms"), new))
            row["baseline_ms"] = new
    return changes


# ---------------------------------------------------------------------------
# Command line
# ---------------------------------------------------------------------------

def build_binaries(root, fixtures, benches, cxx, out_dir):
    """{bench: binary}, {bench: run environment}; built from the manifest's
    suites in the release configuration."""
    manifest = conformance.load_manifest(os.path.join(root, "quality", "conformance.manifest.json"))
    suites = {s["id"]: s for s in manifest["suites"]}
    profiles_dir = os.path.join(root, manifest.get("profiles_dir", "quality/profiles"))
    binaries, envs, unavailable = {}, {}, {}
    for kind in sorted({"gpu" if b == "gpu" else "cpu" for b in benches}):
        suite = suites.get(fixtures["suites"][kind])
        if suite is None:
            raise FixtureError("manifest has no suite %s" % fixtures["suites"][kind])
        profile = conformance.load_profile(profiles_dir, suite["profile"])
        missing = conformance.missing_providers(root, suite, profile)
        if missing:
            unavailable[kind] = "unavailable provider(s): " + "; ".join(missing)
            continue
        binary = os.path.join(out_dir, "bin", suite["id"].replace(".", "_") + ".release")
        os.makedirs(os.path.dirname(binary), exist_ok=True)
        command = conformance.build_command(root, cxx, profile, suite, binary, "release")
        build = conformance.subprocess.run(command, capture_output=True, text=True, check=False)
        if build.returncode != 0:
            raise FixtureError("building %s failed:\n%s" % (suite["id"],
                                                            conformance.tail(build.stderr, 30)))
        env = conformance.run_environment(profile)
        for bench in benches:
            if ("gpu" if bench == "gpu" else "cpu") == kind:
                binaries[bench] = binary
                envs[bench] = env
    return binaries, envs, unavailable


def print_results(results, log=print):
    log("")
    log("%-10s %-40s %-32s %9s %9s %9s %9s" % ("verdict", "workload", "metric", "budget",
                                                "baseline", "measured", "debt"))
    order = {v: i for i, v in enumerate(VERDICTS)}
    for r in sorted(results, key=lambda r: (order[r["verdict"]], -r.get("debt_ms", 0.0))):
        def ms(key):
            value = r.get(key)
            return "%9.3f" % value if isinstance(value, (int, float)) else "%9s" % "-"
        log("%-10s %-40s %-32s %s %s %s %s" % (r["verdict"], r["workload"], r["metric"],
                                               ms("budget_ms"), ms("baseline_ms"),
                                               ms("measured_ms"), ms("debt_ms")))
        if r.get("detail") and r["verdict"] != "debt":
            log("           %s" % r["detail"])


def debt_list(results):
    """The burn-down list: rows over budget, largest debt first."""
    rows = [r for r in results if r["verdict"] in ("debt", "regressed") and r.get("debt_ms")]
    return [{"workload": r["workload"], "metric": r["metric"], "owner": r["owner"],
             "budget_ms": r["budget_ms"], "baseline_ms": r["baseline_ms"],
             "measured_ms": r.get("measured_ms"), "debt_ms": r["debt_ms"]}
            for r in sorted(rows, key=lambda r: -r["debt_ms"])]


def cmd_run(args):
    root = conformance.repo_root()
    fixtures_path = os.path.join(root, args.fixtures)
    fixtures = load_fixtures(fixtures_path)
    budgets = conformance.load_json(os.path.join(root, fixtures["budgets"]))
    profile_id = args.profile or fixtures["default_profile"]
    if profile_id not in fixtures["profiles"]:
        raise FixtureError("no profile %s in %s" % (profile_id, args.fixtures))
    stamp = datetime.datetime.now().strftime("%Y%m%d-%H%M%S")
    out_dir = os.path.abspath(args.out or os.path.join(root, "quality-results", "gi-sdf-perf",
                                                       stamp))
    os.makedirs(out_dir, exist_ok=True)
    cache_dir = os.path.join(root, "quality-results", "gi-sdf-perf", "scenes")

    names = [n for n in fixtures["workloads"]
             if any(fnmatch.fnmatch(n, p) for p in (args.workload or ["*"]))]
    unavailable = {}
    if not args.gpu:
        for n in [n for n in names if fixtures["workloads"][n]["bench"] == "gpu"]:
            unavailable[n] = "GPU workloads run with --gpu on a GPU runner"
            names.remove(n)
    files_by_scene = {}
    for name in list(names):
        scene_name = fixtures["workloads"][name].get("scene")
        if not scene_name:
            continue
        if scene_name not in files_by_scene:
            scene = fixtures["scenes"][scene_name]
            print("scene %s ..." % scene_name, flush=True)
            files_by_scene[scene_name] = scene_files(root, scene_name, scene, cache_dir)
        if files_by_scene[scene_name] is None:
            scene = fixtures["scenes"][scene_name]
            unavailable[name] = "scene %s is absent (%s)" % (
                scene_name, scene.get("rebuild", "no rebuild command declared"))
            names.remove(name)

    benches = sorted({fixtures["workloads"][n]["bench"] for n in names})
    binaries, envs, missing = build_binaries(root, fixtures, benches, args.cxx, out_dir)
    for name in list(names):
        kind = "gpu" if fixtures["workloads"][name]["bench"] == "gpu" else "cpu"
        if kind in missing:
            if args.gpu and kind == "gpu":
                raise FixtureError("--gpu: " + missing[kind])
            unavailable[name] = missing[kind]
            names.remove(name)

    print("gi_sdf_bench: %d workload(s), %d round(s), profile %s"
          % (len(names), args.rounds, profile_id), flush=True)
    runs = run_workloads(root, fixtures, names, binaries, files_by_scene, args.rounds,
                         args.timeout, envs)
    results = evaluate(fixtures, profile_id, runs, budgets, unavailable)
    print_results(results)

    sensitivity = []
    if args.sensitivity:
        items = [i for i in fixtures.get("sensitivity", []) if i["workload"] in names]
        if not items:
            raise FixtureError("--sensitivity: no declared sensitivity workload was run")
        slow_runs = run_workloads(root, fixtures, [i["workload"] for i in items], binaries,
                                  files_by_scene, 1, args.timeout, envs,
                                  {i["workload"]: i["seed_slow"] for i in items})
        slow_results = evaluate(fixtures, profile_id, slow_runs, budgets, {})
        for item in items:
            match = [r for r in slow_results
                     if r["workload"] == item["workload"] and r["metric"] == item["metric"]]
            detected = bool(match) and match[0]["verdict"] == "regressed"
            sensitivity.append({"workload": item["workload"], "seed_slow": item["seed_slow"],
                                "metric": item["metric"], "detected": detected,
                                "verdict": match[0]["verdict"] if match else "missing"})
            print("sensitivity %-40s %-24s %s" % (item["workload"], item["seed_slow"],
                                                  "detected" if detected else "NOT DETECTED"))

    changes = []
    if args.update_baselines or args.calibrate:
        changes = update_baselines(fixtures, profile_id, results, args.calibrate)
        if changes:
            with open(fixtures_path, "w") as stream:
                json.dump(fixtures, stream, indent=2)
                stream.write("\n")
        for workload, metric, old, new in changes:
            print("baseline %s %s: %s -> %s" % (workload, metric, old, new))

    failing = [r for r in results if r["verdict"] in FAILING]
    failing_sensitivity = [s for s in sensitivity if not s["detected"]]
    evidence = {
        "schema": EVIDENCE_SCHEMA,
        "recorded": datetime.datetime.now().isoformat(timespec="seconds"),
        "source": conformance.source_identity(root),
        "compiler": conformance.compiler_identity(args.cxx),
        "host": dict(conformance.host_identity(), cpus=os.cpu_count(),
                     load=list(os.getloadavg()) if hasattr(os, "getloadavg") else None),
        "fixtures": {"path": args.fixtures,
                     "sha256": conformance.file_digest(root, args.fixtures)},
        "profile": profile_id,
        "rounds": args.rounds,
        "runs": {n: {"argv": r["argv"], "records": r["records"], "problems": r["problems"]}
                 for n, r in runs.items()},
        "results": results,
        "debt": debt_list(results),
        "sensitivity": sensitivity,
        "baseline_changes": [list(c) for c in changes],
        "decision": "fail" if failing or failing_sensitivity else "pass",
    }
    path = os.path.join(out_dir, "evidence.json")
    with open(path, "w") as stream:
        json.dump(evidence, stream, indent=2)
        stream.write("\n")
    counts = {v: sum(1 for r in results if r["verdict"] == v) for v in VERDICTS}
    print("\n%s; %d debt row(s)" % (", ".join("%s %d" % (v, c) for v, c in counts.items() if c),
                                     len(evidence["debt"])))
    print("evidence: %s -> %s" % (os.path.relpath(path, root), evidence["decision"].upper()))
    return 1 if evidence["decision"] == "fail" else 0


def cmd_validate(args):
    root = conformance.repo_root()
    fixtures = load_fixtures(os.path.join(root, args.fixtures))
    budgets = conformance.load_json(os.path.join(root, fixtures["budgets"]))
    for profile_id, profile in fixtures["profiles"].items():
        for row in profile["rows"]:
            resolve_budget(row, budgets, profile.get("indirect_light_profile"))
    print("%s: valid (%d scenes, %d workloads, %d profile(s))"
          % (args.fixtures, len(fixtures["scenes"]), len(fixtures["workloads"]),
             len(fixtures["profiles"])))
    return 0


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="command", required=True)
    run = sub.add_parser("run", help="run the workloads and judge the rows")
    run.add_argument("--fixtures", default=DEFAULT_FIXTURES)
    run.add_argument("--profile")
    run.add_argument("--workload", action="append", help="fnmatch pattern (repeatable)")
    run.add_argument("--rounds", type=int, default=3)
    run.add_argument("--timeout", type=int, default=600)
    run.add_argument("--cxx", default="g++")
    run.add_argument("--gpu", action="store_true", help="also run the GPU workloads")
    run.add_argument("--out", help="evidence directory (default quality-results/gi-sdf-perf/<stamp>)")
    run.add_argument("--sensitivity", action="store_true",
                     help="also require the declared seeded slowdowns to regress")
    run.add_argument("--update-baselines", action="store_true",
                     help="lower improved rows' baselines to their measurement")
    run.add_argument("--calibrate", action="store_true",
                     help="set every measured row's baseline (review the diff)")
    run.set_defaults(func=cmd_run)
    validate = sub.add_parser("validate", help="validate the fixture file")
    validate.add_argument("--fixtures", default=DEFAULT_FIXTURES)
    validate.set_defaults(func=cmd_validate)
    args = parser.parse_args(argv)
    try:
        return args.func(args)
    except FixtureError as e:
        print("gi_sdf_bench: %s" % e, file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
