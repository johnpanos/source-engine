#!/usr/bin/env python3
"""RFC 0011 G1 probe-volume oracles on baked PRBV files.

    python3 tools/quality/gi_probes.py bake --fixture probe-grid --state sun --out DIR
    python3 tools/quality/gi_probes.py analytic --fixture probe-grid --state sky \\
        --prbv FILE --out FILE.json
    python3 tools/quality/gi_probes.py leak --fixture thin-wall --prbv FILE --out FILE.json
    python3 tools/quality/gi_probes.py malformed --map-build DIR --out DIR

`bake` extracts one fixture state's stage (as `gi_reference.py render` does)
and bakes its probe volume with `probe_volume_bake.py`, the map pipeline's
baker, under the fixture's map-export profile. The map build bakes only the
baked state; `bake` covers the others.

`analytic` samples the volume with the reference sampler
(`probe_volume.Volume.sample`) at the fixture's analytic probe positions. It
compares the samples with irradiance integrated exactly over the finite floor
square (see the fixture's analytic note), for both layers:

  - at probe normals, the 36 directions of an irradiance tile's texels, where
    the tile stores the value and only the probe blend is judged. This is the
    gate: total within ANALYTIC_TOLERANCE of the reference, and indirect
    within the same fraction of the state's floor radiance;
  - at 64 other normals, where the 6 x 6 octahedral tile's bilinear filtering
    adds its own error. Measured, not gated.

`malformed` boots a built map with its PRBV lump replaced by a malformed,
a truncated and an unverifiable payload (G1.2). Each must boot and play, and
the engine must report the structured rejection and fall back to the leaf
ambient instead of loading the volume.

`leak` samples each room of the thin-wall fixture on a lattice of interior
points, along the six axis normals (a model's ambient cube). The measure is
the fixture's `leak_bound`: dark-room mean indirect light over lit-room mean
indirect light. It must stay under the bound with visibility, and exceed it
with visibility disabled (the sensitivity control).
"""

import argparse
import json
import math
import shutil
import struct
import subprocess
import sys
from pathlib import Path

import numpy as np

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
sys.path.insert(0, str(HERE))
import bsp2_reader  # noqa: E402
import gi_reference  # noqa: E402
import map_scene  # noqa: E402
import pbrt_map_build  # noqa: E402
import probe_volume  # noqa: E402

SCHEMA = "gi-probe-oracle/v1"
SOURCE_UNITS_PER_METER = 39.37007874015748
WORK = ROOT / "quality-results" / "rfc0011-probes"
# RFC 0011 G1.4 provisional bounds: relative error at probe normals.
ANALYTIC_TOLERANCE = {"sky": 0.03, "sun": 0.05}
REFERENCE_DIRECTIONS = 400000
MEASURED_NORMALS = 64
LEAK_LATTICE = 5
AXES = np.array([(1, 0, 0), (-1, 0, 0), (0, 1, 0), (0, -1, 0), (0, 0, 1), (0, 0, -1)], float)


def luminance(rgb):
    return float(gi_reference.luminance(rgb))


def write(path, record):
    Path(path).parent.mkdir(parents=True, exist_ok=True)
    Path(path).write_text(json.dumps(record, indent=2, sort_keys=True) + "\n")


# ------------------------------------------------------------------ bake

def probe_settings(fixture):
    manifest = json.loads((fixture["directory"] / fixture["map_manifest"]).read_text())
    profile = pbrt_map_build.load_profile(manifest["quality"])
    settings = pbrt_map_build.with_defaults(manifest, profile, "probe_volume")
    lightmap = pbrt_map_build.with_defaults(manifest, profile, "lightmap") or {}
    if not settings:
        raise SystemExit("%s: its export profile declares no probe volume" % fixture["name"])
    return settings, lightmap.get("light_paths", "blender-default")


def cmd_bake(args):
    fixture = gi_reference.load_fixture(args.fixture)
    settings, light_paths = probe_settings(fixture)
    tools = gi_reference.Tools(args.toolchain)
    directory = Path(args.out)
    if directory.exists():
        shutil.rmtree(directory)
    directory.mkdir(parents=True)
    log = directory / "log.txt"
    scene_path, stage_path = directory / "scene.json", directory / "stage.usdc"
    tools.usd_python("usd_scene.py", ["extract", "--scene",
                                      gi_reference.state_stage(fixture, args.state),
                                      "--out-scene", scene_path, "--out-stage", stage_path], log)
    arguments = ["--scene", scene_path, "--stage", stage_path,
                 "--spacing", str(settings["spacing_m"]),
                 "--samples", str(settings.get("samples", 4096)),
                 "--light-paths", light_paths, "--out", directory / "probes.prbv",
                 "--work", directory / "bake"]
    if settings.get("bounds_m"):
        arguments += ["--bounds"] + [str(v) for v in settings["bounds_m"]]
    scene = map_scene.parse(scene_path)
    if scene["environment"]:
        import imageio.v3 as iio
        environment = directory / "environment.exr"
        iio.imwrite(environment, map_scene.environment_equirect(scene, 2048).astype(np.float32))
        arguments += ["--environment", environment]
    tools.blender("probe_volume_bake.py", arguments, log)
    print("[%s/%s] baked %s" % (args.fixture, args.state, directory / "probes.prbv"))
    return 0


# ------------------------------------------------------------------ analytic

def sphere_directions(count):
    i = np.arange(count) + 0.5
    z = 1.0 - 2.0 * i / count
    r = np.sqrt(np.maximum(0.0, 1.0 - z * z))
    phi = math.pi * (3.0 - math.sqrt(5.0)) * i
    return np.stack([r * np.cos(phi), r * np.sin(phi), z], axis=-1)


def floor_hits(position, directions, half):
    """True where a ray from `position` (meters, above z = 0) hits the square."""
    down = directions[:, 2] < -1e-12
    t = np.where(down, -position[2] / np.where(down, directions[:, 2], -1.0), np.inf)
    x = position[0] + t * directions[:, 0]
    y = position[1] + t * directions[:, 1]
    return down & (np.abs(x) <= half) & (np.abs(y) <= half)


def analytic_irradiance(analytic, state, position, normals, directions):
    """(total, indirect) irradiance / pi for each normal (rows)."""
    rho = analytic["albedo"]
    half = analytic["floor_half_extent_m"]
    hits = floor_hits(position, directions, half)
    if state == "sky":
        sky = analytic["sky_radiance"]
        radiance_direct, floor = np.where(hits, 0.0, sky), rho * sky
    else:
        sun = analytic["sun"]
        radiance_direct = np.zeros(len(directions))
        floor = rho * sun["irradiance"] * math.sin(math.radians(sun["elevation_degrees"])) / \
            math.pi
    radiance_indirect = np.where(hits, floor, 0.0)
    cosines = np.maximum(normals @ directions.T, 0.0)
    # E / pi = (1 / pi) * sum(L cos) * (4 pi / N).
    weight = 4.0 / len(directions)
    indirect = cosines @ radiance_indirect * weight
    direct = cosines @ radiance_direct * weight
    if state == "sun":
        sun = analytic["sun"]
        direct = direct + sun["irradiance"] * np.maximum(normals @ np.asarray(sun["to_sun"]),
                                                         0.0) / math.pi
    return direct + indirect, indirect, floor


def cmd_analytic(args):
    fixture = gi_reference.load_fixture(args.fixture)
    analytic = fixture.get("analytic") or {}
    if "floor_half_extent_m" not in analytic or args.state not in ANALYTIC_TOLERANCE:
        raise SystemExit("%s/%s has no probe analytic declaration" % (args.fixture, args.state))
    volume = probe_volume.read(args.prbv)
    directions = sphere_directions(REFERENCE_DIRECTIONS)
    tile = probe_volume.interior_directions(probe_volume.IRRADIANCE_TILE - 2).reshape(-1, 3)
    rng = np.random.default_rng(20260924)
    other = rng.normal(size=(MEASURED_NORMALS, 3))
    other /= np.linalg.norm(other, axis=1, keepdims=True)
    tolerance = ANALYTIC_TOLERANCE[args.state]
    probes, failures = [], []
    worst = {"gated_total": 0.0, "gated_indirect": 0.0, "measured_total": 0.0,
             "measured_indirect": 0.0}
    for position in analytic["probes_m"]:
        position = np.asarray(position, float)
        record = {"position_m": position.tolist()}
        for name, normals in (("gated", tile), ("measured", other)):
            total, indirect, floor = analytic_irradiance(analytic, args.state, position,
                                                         normals, directions)
            got_total, got_indirect = [], []
            for n in normals:
                for layer, out in ((0, got_total), (1, got_indirect)):
                    value = volume.sample(position * SOURCE_UNITS_PER_METER, n, layer)
                    if value is None:
                        raise SystemExit("probe %s lies outside the volume" % position.tolist())
                    out.append(luminance(value))
            total_error = np.abs(np.asarray(got_total) - total) / np.maximum(total, 1e-9)
            # Near the sun's terminator the total is tiny; the error against
            # the brightest normal's irradiance shows its absolute size.
            peak_error = np.abs(np.asarray(got_total) - total) / float(total.max())
            indirect_error = np.abs(np.asarray(got_indirect) - indirect) / floor
            worst[name + "_total"] = max(worst[name + "_total"], float(total_error.max()))
            worst[name + "_indirect"] = max(worst[name + "_indirect"],
                                            float(indirect_error.max()))
            record[name] = {"normals": len(normals),
                            "total_max_relative_error": float(total_error.max()),
                            "total_mean_relative_error": float(total_error.mean()),
                            "total_max_error_over_peak": float(peak_error.max()),
                            "indirect_max_error_over_floor": float(indirect_error.max()),
                            "indirect_mean_error_over_floor": float(indirect_error.mean())}
            if name == "gated":
                if total_error.max() > tolerance:
                    failures.append("probe %s: total error %.4f > %.2f at probe normals" %
                                    (position.tolist(), total_error.max(), tolerance))
                if indirect_error.max() > tolerance:
                    failures.append("probe %s: indirect error %.4f of the floor radiance > "
                                    "%.2f at probe normals" %
                                    (position.tolist(), indirect_error.max(), tolerance))
        probes.append(record)
        print("  probe %-18s probe normals: total %.4f indirect %.4f | other normals: total "
              "%.4f (%.4f of peak) indirect %.4f" % (
                  position.tolist(), record["gated"]["total_max_relative_error"],
                  record["gated"]["indirect_max_error_over_floor"],
                  record["measured"]["total_max_relative_error"],
                  record["measured"]["total_max_error_over_peak"],
                  record["measured"]["indirect_max_error_over_floor"]))
    result = {"schema": SCHEMA, "oracle": "analytic", "fixture": args.fixture,
              "state": args.state, "prbv": str(Path(args.prbv).resolve()),
              "prbv_sha256": gi_reference.sha256(args.prbv), "tolerance": tolerance,
              "reference": "exact integral over the finite floor square, %d directions" %
                           REFERENCE_DIRECTIONS,
              "probes": probes, "worst": worst, "failures": failures,
              "status": "fail" if failures else "pass"}
    write(args.out, result)
    print("GI probe analytic %s/%s: %s" % (args.fixture, args.state, result["status"]))
    return 0 if not failures else 1


# ------------------------------------------------------------------ leak

def room_points(volume, x_range):
    """A lattice of points inside one room: x within `x_range` (units), y and
    z across the grid, each inset by a quarter of the spacing."""
    grid = volume.grids[0]
    low = grid.origin + grid.spacing * 0.25
    high = grid.origin + (np.asarray(grid.dims) - 1) * grid.spacing - grid.spacing * 0.25
    xs = np.linspace(max(x_range[0], low[0]), min(x_range[1], high[0]), LEAK_LATTICE)
    ys = np.linspace(low[1], high[1], LEAK_LATTICE)
    zs = np.linspace(low[2], high[2], LEAK_LATTICE)
    return [np.array([x, y, z]) for x in xs for y in ys for z in zs]


def room_mean(volume, points, layer, visibility):
    values = []
    for point in points:
        for axis in AXES:
            value = volume.sample(point, axis, layer, visibility)
            if value is not None:
                values.append(luminance(value))
    return float(np.mean(values)), len(values)


def cmd_leak(args):
    fixture = gi_reference.load_fixture(args.fixture)
    panel, bound = fixture.get("panel"), fixture.get("leak_bound")
    if not panel or not bound:
        raise SystemExit("%s declares no panel and leak bound" % args.fixture)
    volume = probe_volume.read(args.prbv)
    inset = 0.25 * float(volume.grids[0].spacing.min())
    lit_x = (-1e9, panel["x_min_m"] * SOURCE_UNITS_PER_METER - inset)
    dark_x = (panel["x_max_m"] * SOURCE_UNITS_PER_METER + inset, 1e9)
    lit, dark = room_points(volume, lit_x), room_points(volume, dark_x)
    runs = {}
    for name, visibility in (("visibility", True), ("visibility-disabled", False)):
        run = {}
        for layer, label in ((1, "indirect"), (0, "total")):
            lit_mean, lit_count = room_mean(volume, lit, layer, visibility)
            dark_mean, dark_count = room_mean(volume, dark, layer, visibility)
            run[label] = {"lit_mean": lit_mean, "dark_mean": dark_mean,
                          "ratio": dark_mean / lit_mean if lit_mean > 0 else float("inf"),
                          "samples": [lit_count, dark_count]}
        runs[name] = run
        print("  %-20s indirect dark/lit %.5f (dark %.5f, lit %.4f) | total %.5f" % (
            name, run["indirect"]["ratio"], run["indirect"]["dark_mean"],
            run["indirect"]["lit_mean"], run["total"]["ratio"]))
    limit = bound["provisional"]
    failures = []
    if not runs["visibility"]["indirect"]["ratio"] < limit:
        failures.append("dark-room indirect %.4f of the lit room is not below the bound %.3f" %
                        (runs["visibility"]["indirect"]["ratio"], limit))
    if not runs["visibility-disabled"]["indirect"]["ratio"] > limit:
        failures.append("the visibility-disabled control (%.4f) does not exceed the bound; the "
                        "oracle cannot tell visibility from no leak" %
                        runs["visibility-disabled"]["indirect"]["ratio"])
    result = {"schema": SCHEMA, "oracle": "leak", "fixture": args.fixture,
              "prbv": str(Path(args.prbv).resolve()), "prbv_sha256": gi_reference.sha256(args.prbv),
              "measure": bound["measure"], "bound": limit, "lattice": LEAK_LATTICE,
              "runs": runs, "failures": failures, "status": "fail" if failures else "pass"}
    write(args.out, result)
    print("GI probe leak %s: %s" % (args.fixture, result["status"]))
    return 0 if not failures else 1


# ------------------------------------------------------------------ malformed

def replace_lump(data, fourcc, payload, rehash=True):
    """BSP2 bytes with lump `fourcc`'s payload replaced; with rehash False the
    old content hash is kept, so only the lump's verification fails."""
    container = bsp2_reader.Bsp2File(data, allow_unknown_required=True)
    lumps = []
    for entry in container.entries:
        body = data[entry["offset"]:entry["offset"] + entry["size"]]
        if entry["fourcc"] == fourcc:
            body = payload
        lumps.append((entry["fourcc"], entry["version"], entry["flags"], entry["alignment"],
                      body))
    out = bytearray(bsp2_reader.write_bsp2(container.revision, lumps))
    if not rehash:
        rebuilt = bsp2_reader.Bsp2File(bytes(out), verify=False, allow_unknown_required=True)
        for index, entry in enumerate(rebuilt.entries):
            if entry["fourcc"] == fourcc:
                # Flip one payload byte after hashing: the directory still
                # verifies, the lump's content hash does not.
                out[entry["offset"] + len(payload) // 2] ^= 0x5A
    return bytes(out)


def cmd_malformed(args):
    build = Path(args.map_build)
    manifest = json.loads((build / "build.json").read_text()) \
        if (build / "build.json").is_file() else {}
    maps = sorted((build / "content" / "maps").glob("*.bsp"))
    if len(maps) != 1:
        raise SystemExit("%s: expected one built map" % build)
    source = maps[0]
    data = source.read_bytes()
    container = bsp2_reader.Bsp2File(data, allow_unknown_required=True)
    prbv = next((e for e in container.entries if e["fourcc"] == probe_volume.MAGIC), None)
    if not prbv:
        raise SystemExit("%s carries no PRBV lump" % source)
    good = data[prbv["offset"]:prbv["offset"] + prbv["size"]]
    bad_counts = bytearray(good)
    struct.pack_into("<I", bad_counts, 12, 0)  # grid count 0
    variants = {
        # The control: the unmodified lump loads.
        "intact": (good, True, None),
        "invalid-counts": (bytes(bad_counts), True, r"PRBV rejected \(invalid-counts\)"),
        "truncated": (good[:len(good) // 2], True, r"PRBV rejected \((truncated|invalid-atlas)\)"),
        "hash": (good, False, r"PRBV read or hash failed"),
    }
    profile, _ = pbrt_map_toolchain_profiles()
    out = Path(args.out)
    results, failures = {}, []
    for name, (payload, rehash, expected) in variants.items():
        directory = out / name
        if directory.exists():
            shutil.rmtree(directory)
        shutil.copytree(build / "content", directory / "content")
        (directory / "content" / "maps" / source.name).write_bytes(
            replace_lump(data, probe_volume.MAGIC, payload, rehash))
        boot = directory / "boot"
        result = subprocess.run(
            [sys.executable, HERE / "portal_boot.py", "--runtime", profile["runtime"],
             "--build", profile["client_build"], "--content-root", directory / "content",
             "--renderer", "native-vulkan", "--headless", "--map", source.stem,
             "--console-command", "r_drawvgui 0; r_worldmesh_draw 2", "--out", boot],
            cwd=ROOT, capture_output=True, text=True)
        evidence = json.loads((boot / "evidence.json").read_text()) \
            if (boot / "evidence.json").is_file() else {}
        log = "\n".join(p.read_text(errors="replace") for p in boot.rglob("*.log"))
        import re
        loaded = bool(re.search(r"PRBV v\d+, \d+ grid", log))
        reported = bool(re.search(expected, log)) if expected else False
        ok = evidence.get("status") == "pass" and (
            loaded and not re.search(r"PRBV (rejected|read or hash|version)", log)
            if expected is None else reported and not loaded)
        results[name] = {"boot_status": evidence.get("status"), "reported": reported,
                         "volume_loaded": loaded, "expected_diagnostic": expected,
                         "status": "pass" if ok else "fail", "boot": str(boot)}
        if not ok:
            failures.append("%s: boot %s, diagnostic %s, volume loaded %s" % (
                name, evidence.get("status"), reported, loaded))
        print("  %-15s boot %-5s diagnostic %-5s loaded %s" % (
            name, evidence.get("status"), reported, loaded))
    record = {"schema": SCHEMA, "oracle": "malformed", "map": source.stem,
              "variants": results, "failures": failures,
              "status": "fail" if failures else "pass"}
    write(out / "malformed.json", record)
    print("GI probe malformed %s: %s" % (source.stem, record["status"]))
    return 0 if not failures else 1


def pbrt_map_toolchain_profiles():
    import pbrt_map_toolchain
    profile, _ = pbrt_map_toolchain.load_profiles()
    toolchain = pbrt_map_toolchain.load(ROOT / profile["layout"]["toolchain_file"])
    return toolchain, profile


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    commands = parser.add_subparsers(dest="command", required=True)
    bake = commands.add_parser("bake")
    bake.add_argument("--fixture", required=True)
    bake.add_argument("--state", required=True)
    bake.add_argument("--out", required=True)
    bake.add_argument("--toolchain", type=Path)
    analytic = commands.add_parser("analytic")
    analytic.add_argument("--fixture", required=True)
    analytic.add_argument("--state", required=True)
    analytic.add_argument("--prbv", required=True)
    analytic.add_argument("--out", required=True)
    leak = commands.add_parser("leak")
    leak.add_argument("--fixture", required=True)
    leak.add_argument("--prbv", required=True)
    leak.add_argument("--out", required=True)
    malformed = commands.add_parser("malformed")
    malformed.add_argument("--map-build", required=True)
    malformed.add_argument("--out", required=True)
    args = parser.parse_args()
    return {"bake": cmd_bake, "analytic": cmd_analytic, "leak": cmd_leak,
            "malformed": cmd_malformed}[args.command](args)


if __name__ == "__main__":
    sys.exit(main())
