#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
#
# Box3D restitution scenes, IVP-referenced (RFC 0005 Q0 gap corpus; RFC 0004
# Q-PHYSICS; RFC 0013 progress "restitution patch").
#
# With the pinned Box3D restitution patch (fork johnpanos/box3d 78c90a0: "only
# touching points bounce; stop once separated and not approaching") three
# gameplay checks of the parity suite fail, but only inside the whole run:
#
#   corner       vcollide.model-simulates        (test_vphysics_conformance.cpp:1106;
#                fails for models/player/chell.phy, 23 limbs each landing on
#                a corner; the single metal_box drop rides along)
#   tumble       dynamics.tumble-travel-bounded  (test_vphysics_dynamics.cpp:397)
#   held-floor   dynamics.held-floor-quiet       (test_vphysics_dynamics.cpp:907)
#
# The host unittests/physicstest/corpus/restitution_scenes.cpp runs one scene
# for one provider per process. This script is the oracle:
#
#   1. IVP is the reference. `record` runs every scene variant (the canonical
#      parameters plus seeded variations) under IVP and writes per-observation
#      ranges to quality/fixtures/corpus/physics/restitution-ivp.json using
#      the tolerance policy below. Nothing a Box3D build measures goes in.
#   2. `check` runs one scene's variants for one provider and fails on:
#        - gameplay bounds, the parity suite's own clauses and thresholds
#          (travel < 48; travel < 30 and spin < 360 deg/s; every solid
#          finite with its origin in (-1, 150)), on the canonical variant and
#          on each perturbed variant where IVP holds the clause too;
#        - analytic invariants: finite; a resting body's geometry is on the
#          floor, not in it; a passive box's mechanical energy never exceeds
#          its start and never rises by more than 1% of it after the first
#          contact (IVP's largest rise is 0.17%); a box released at rest does
#          not rise, move or keep speed;
#        - the recorded IVP ranges of the gated observations.
#      It ends with one checks-v1 record (conformance_result.Checks).
#   3. `report` prints IVP, Box3D upstream (9e5a4cd) and Box3D patched
#      (78c90a0) side by side.
#
# Providers: `ivp` is <build>/vphysics/libvphysics.so. The Box3D builds are
# staged copies, because one Waf tree compiles one box3d source at a time:
# `stage-box3d` copies <build>/vphysics_box3d/libvphysics_box3d.so into
# <build>/corpus-restitution/providers/box3d-<commit>/ with a build.json that
# names the compiled box3d commit and the SHA-256 of its contact_solver.c.
#
# Setup (from the repository root; a private Waf tree):
#   WAFLOCK=.lock-waf-corpus-physics ./waf configure --platform-provider=sdl3 \
#     --render-backend=native-vulkan --build-games=portal --physics-backend=both \
#     --disable-warns -T release -o build-corpus-physics
#   git -C box3d checkout 9e5a4cd
#   WAFLOCK=.lock-waf-corpus-physics ./waf build -j8 \
#     --targets=vphysics,vphysics_box3d,vphysics_conformance
#   python3 tools/quality/corpus/physics_restitution.py stage-box3d
#   git -C box3d checkout 78c90a0   # fetched from the fork
#   WAFLOCK=.lock-waf-corpus-physics ./waf build -j8 --targets=vphysics_box3d
#   python3 tools/quality/corpus/physics_restitution.py stage-box3d
#   git -C box3d checkout 9e5a4cd
#   python3 tools/quality/corpus/physics_restitution.py build-host
# Game content comes from run/runtime (or $CORPUS_RUNTIME); nothing from the
# game is copied into the repository.
#
# Python 3 standard library only.
#
# ============================================================================

import argparse
import ast
import concurrent.futures
import datetime
import hashlib
import json
import os
import re
import shutil
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
QUALITY = os.path.dirname(HERE)
sys.path.insert(0, QUALITY)
sys.path.insert(0, HERE)

import conformance  # noqa: E402
from conformance_result import Checks  # noqa: E402
import provenance  # noqa: E402
from source_content import ContentResolver  # noqa: E402

ROOT = conformance.repo_root()
FIXTURE = os.path.join(ROOT, "quality", "fixtures", "corpus", "physics", "restitution-ivp.json")
FIXTURE_SCHEMA = "corpus-physics-restitution/v1"
FIXTURE_VERSION = 1
HOST_SOURCE = "unittests/physicstest/corpus/restitution_scenes.cpp"
# The translation units the host includes (compiled into it) and links.
HOST_INCLUDED = ["unittests/physicstest/test_vphysics_conformance.cpp",
                 "unittests/physicstest/test_vphysics_dynamics.cpp"]
HOST_LINKED = ["unittests/physicstest/test_vphysics_constraints.cpp",
               "unittests/physicstest/test_vphysics_objects.cpp",
               "unittests/physicstest/test_vphysics_fluids.cpp",
               "unittests/physicstest/test_vphysics_saverestore.cpp",
               "unittests/physicstest/test_vphysics_collide.cpp",
               "unittests/physicstest/test_vphysics_player.cpp",
               "unittests/physicstest/test_vphysics_vehicle.cpp",
               "unittests/physicstest/test_vphysics_bench.cpp"]

BOX3D_UPSTREAM = "9e5a4cde862fba95ff19f096b79567f3ea6c01fd"
BOX3D_PATCHED = "78c90a09ec420aad9432db8ad63e8ed2c3871372"
PROVIDERS = {
    "ivp": None,
    "box3d-upstream": BOX3D_UPSTREAM,
    "box3d-patched": BOX3D_PATCHED,
}

SURFACE_MANIFEST = "scripts/surfaceproperties_manifest.txt"
CUBE_MODEL = "models/props/metal_box.phy"

CHELL_MODEL = "models/player/chell.phy"
MODELS = [CUBE_MODEL, CHELL_MODEL]

# ---------------------------------------------------------------------------
# Suites, cases and variants. The canonical variant carries the parity suite's
# parameters exactly; the others are fixed perturbations around it that IVP
# uses to measure how far each observation moves. Order is stable: it is the
# order of the checks, and a manifest row names its first failing check.
#
# corner: vcollide.model-simulates fails under the patched Box3D only for the
# multi-solid fixture (models/player/chell.phy, 23 limbs dropped separately,
# each landing on a corner or edge). The single-body box drop of the same
# clause ("a box landing on a corner") rides along: it passes the clause for
# every provider but shows how each lands.
# ---------------------------------------------------------------------------
CORNER_VARIANTS = [
    {"id": "canonical", "args": []},
    {"id": "yaw-57", "args": ["--angles", "10,57,5"]},
    {"id": "pitch-15-roll-10", "args": ["--angles", "15,20,10"]},
    {"id": "pitch-5-roll-0", "args": ["--angles", "5,20,0"]},
    {"id": "drop-160", "args": ["--height", "160"]},
    {"id": "drop-240", "args": ["--height", "240"]},
]

SUITES = {
    "corner": {
        "cases": [
            {"id": "ragdoll", "scene": "corner", "model": CHELL_MODEL,
             "source": "unittests/physicstest/test_vphysics_conformance.cpp:1033-1109",
             "variants": CORNER_VARIANTS,
             "gameplay": ["vcollide.model-simulates"],
             "analytic": ["finite", "rests-on-floor"],
             # 23 independent limbs: the recorded trajectory is whichever limb
             # breaks the rule (or is fastest), so its metrics are reported
             # but not compared.
             "gated": []},
            {"id": "box", "scene": "corner", "model": CUBE_MODEL,
             "source": "unittests/physicstest/test_vphysics_conformance.cpp:1033-1109",
             "variants": CORNER_VARIANTS,
             "gameplay": ["vcollide.model-simulates"],
             "analytic": ["finite", "energy", "rests-on-floor"],
             # Spin first: how a box lands on its corner (tumbles, or bounces
             # straight up) is the signature this case exists to compare.
             "gated": ["max_angular", "apex_max", "travel", "settle_s"]},
        ],
    },
    "tumble-travel": {
        "cases": [
            {"id": "box", "scene": "tumble", "model": CUBE_MODEL,
             "source": "unittests/physicstest/test_vphysics_dynamics.cpp:364-398",
             "variants": [
                 {"id": "canonical", "args": []},
                 {"id": "yaw-30", "args": ["--angles", "25,30,15"]},
                 {"id": "pitch-20-roll-20", "args": ["--angles", "20,0,20"]},
                 {"id": "pitch-30-roll-10", "args": ["--angles", "30,0,10"]},
                 {"id": "drop-48", "args": ["--height", "48"]},
             ],
             "gameplay": ["dynamics.tumble-rests-flat", "dynamics.tumble-settles",
                          "dynamics.tumble-travel-bounded"],
             "analytic": ["finite", "energy", "rests-on-floor"],
             "gated": ["travel", "max_angular", "apex_max", "settle_s", "audible_impacts"]},
        ],
    },
    "held-floor": {
        "cases": [
            {"id": "box", "scene": "held-floor", "model": CUBE_MODEL,
             "source": "unittests/physicstest/test_vphysics_dynamics.cpp:887-913",
             "variants": [
                 {"id": "canonical", "args": []},
                 {"id": "yaw-30", "args": ["--angles", "0,30,0"]},
                 {"id": "depth-4", "args": ["--depth", "4"]},
                 {"id": "depth-12", "args": ["--depth", "12"]},
                 {"id": "start-80", "args": ["--height", "80"]},
             ],
             "gameplay": ["dynamics.held-floor-contact", "dynamics.held-floor-quiet"],
             # A held body is driven by the controller: no energy invariant.
             "analytic": ["finite", "rests-on-floor"],
             "gated": ["travel", "quiet_angular", "gap"]},
            # Analytic control: released at rest on the floor. Released at
            # the face (height 0) IVP rises about 2 units out of its 0.5-unit
            # collision distance, so the release is at and above that shell.
            {"id": "rest", "scene": "rest", "model": CUBE_MODEL,
             "source": "unittests/physicstest/corpus/restitution_scenes.cpp RunRest",
             "variants": [
                 {"id": "shell-0.5", "args": ["--height", "0.5"]},
                 {"id": "shell-0.5-yaw-45", "args": ["--height", "0.5", "--angles", "0,45,0"]},
                 {"id": "above-1", "args": ["--height", "1"]},
             ],
             "gameplay": [],
             "analytic": ["finite", "energy", "rests-on-floor", "rest-no-gain", "rest-still"],
             "gated": ["z_gain", "travel"]},
        ],
    },
}

# The parity suite's own thresholds (the gameplay bounds), with its lines.
TUMBLE_TRAVEL_MAX = 48.0        # test_vphysics_dynamics.cpp:397
HELD_TRAVEL_MAX = 30.0          # test_vphysics_dynamics.cpp:907
HELD_ANGULAR_MAX = 360.0        # test_vphysics_dynamics.cpp:907


def gameplay_rule(name, v):
    """(holds, detail) of one parity-suite clause over a run's observations."""
    if name == "vcollide.model-simulates":
        # test_vphysics_conformance.cpp:1098-1106: every solid created, finite,
        # origin z in (-1, 150), and a single-solid model slower than 20.
        ok = v["created"] == v["solids"] and v["all_finite"] == 1 and v["rested"] == 1
        return ok, "created %d/%d finite %d rested %d; %d solid(s) break the rule, lowest origin z %.3f, " \
            "lowest geometry z %.3f, fastest %.3f" % (
                v["created"], v["solids"], v["all_finite"], v["rested"], v["unrested"], v["min_final_z"],
                v["min_geom_final_z"], v["worst_speed"])
    if name == "dynamics.tumble-rests-flat":
        # test_vphysics_dynamics.cpp:393: Near( rest.z, restZ, 0.5 ) and tilt < 1.
        ok = abs(v["rest_offset_z"]) <= 0.5 and v["final_tilt"] < 1.0
        return ok, "z offset %.3f tilt %.3f" % (v["rest_offset_z"], v["final_tilt"])
    if name == "dynamics.tumble-settles":
        # test_vphysics_dynamics.cpp:395: settled within 3 s.
        ok = 0.0 <= v["settle_s"] < 3.0
        return ok, "settled %.3f s" % v["settle_s"]
    if name == "dynamics.tumble-travel-bounded":
        return v["travel"] < TUMBLE_TRAVEL_MAX, "travel %.3f (bound %g)" % (v["travel"], TUMBLE_TRAVEL_MAX)
    if name == "dynamics.held-floor-contact":
        # test_vphysics_dynamics.cpp:906: -1 < gap < 3.
        return -1.0 < v["gap"] < 3.0, "gap %.3f" % v["gap"]
    if name == "dynamics.held-floor-quiet":
        ok = v["travel"] < HELD_TRAVEL_MAX and v["quiet_angular"] < HELD_ANGULAR_MAX
        return ok, "travel %.3f angular %.3f (bounds %g, %g)" % (
            v["travel"], v["quiet_angular"], HELD_TRAVEL_MAX, HELD_ANGULAR_MAX)
    raise KeyError(name)


# Analytic invariants: provider-independent physics, not IVP measurements.
ENERGY_RATIO_MAX = 1.01         # E(t) never above the release energy (+1%)
ENERGY_RISE_FRACTION = 0.01     # no rise above the running minimum after first contact > 1% of E(0)
GEOMETRY_FLOOR_Z = -1.0         # a resting body's geometry at most 1 unit into the floor
REST_GAIN_MAX = 0.1             # released at rest: the origin rises at most 0.1 unit
REST_SPEED_MAX = 1.0            # and ends slower than the settle rule's 1 in/s
REST_TRAVEL_MAX = 0.5           # and within half a unit of where it was released


def analytic_rule(name, v):
    if name == "finite":
        ok = v["finite"] == 1 and v.get("all_finite", 1) == 1
        return ok, "finite %d all %d" % (v["finite"], v.get("all_finite", 1))
    if name == "energy":
        # Passive rigid box on a static floor with restitution <= 1: the
        # mechanical energy (kinetic + rotational about the principal axes +
        # m g z) cannot rise. Box only: the cube's principal axes are its
        # local axes, which the rotational term assumes.
        rise = v["energy_rise"] / v["energy0"] if v["energy0"] > 0 else float("inf")
        ok = v["energy_ratio_max"] <= ENERGY_RATIO_MAX and rise <= ENERGY_RISE_FRACTION
        return ok, "max E/E0 %.4f (bound %g), largest rise %.4f of E0 (bound %g)" % (
            v["energy_ratio_max"], ENERGY_RATIO_MAX, rise, ENERGY_RISE_FRACTION)
    if name == "rests-on-floor":
        # A body at rest is held up by its contact: its collision geometry
        # ends on the floor, not in it (the final pose; a solid that slid off
        # the slab is excluded and counted in off_floor). The transient depth
        # during an impact is reported (geom_min_z_seen), not bounded here.
        finals = [v["min_geom_final_z"]] if "min_geom_final_z" in v else []
        if v.get("over_floor", 1) == 1:
            finals.append(v["geom_min_z"])
        lowest = min(finals) if finals else 0.0
        return lowest > GEOMETRY_FLOOR_Z, "final geometry z %.3f (bound %g); %d off the floor" % (
            lowest, GEOMETRY_FLOOR_Z, v.get("off_floor", 0))
    if name == "rest-no-gain":
        return v["z_gain"] <= REST_GAIN_MAX, "rose %.4f (bound %g)" % (v["z_gain"], REST_GAIN_MAX)
    if name == "rest-still":
        ok = v["final_speed"] < REST_SPEED_MAX and v["travel"] < REST_TRAVEL_MAX
        return ok, "final speed %.4f travel %.4f (bounds %g, %g)" % (
            v["final_speed"], v["travel"], REST_SPEED_MAX, REST_TRAVEL_MAX)
    raise KeyError(name)


# ---------------------------------------------------------------------------
# Tolerance policy for IVP ranges (versioned with the fixture). For each
# observation, over the IVP runs of every variant of a case:
#
#     margin = max(floor, RANGE_RELATIVE * max |value|)
#     range  = [min - margin, max + margin], the low end clamped at 0
#
# The floor is the observation's resolution (units, deg/s, seconds, counts);
# the relative half-width admits that a single drop is chaotic (the parity
# suite compares one tumble's travel only as a bound, TestTumbleStatistics
# only means). An observation with a "never" sentinel (-1: never settled) in
# any IVP variant is recorded but has no range.
# ---------------------------------------------------------------------------
RANGE_RELATIVE = 0.5
RANGE_FLOORS = {
    "travel": 4.0, "apex_max": 2.0, "gap": 1.0, "z_gain": 0.5, "final_tilt": 2.0,
    "geom_min_z_seen": 1.0, "min_geom_final_z": 1.0,
    "max_angular": 90.0, "quiet_angular": 90.0,
    "settle_s": 0.5, "sleep_s": 0.5,
    "bounces": 1.0, "audible_impacts": 1.0, "impacts": 2.0, "unrested": 1.0,
}
SIGNED = {"geom_min_z_seen", "min_geom_final_z", "gap"}
SENTINEL = {"settle_s", "sleep_s"}


def ivp_range(values, key):
    if key in SENTINEL and any(x < 0 for x in values):
        return None
    top = max(abs(x) for x in values)
    margin = max(RANGE_FLOORS[key], RANGE_RELATIVE * top)
    low = min(values) - margin
    if key not in SIGNED:
        low = max(0.0, low)
    return [round(low, 4), round(max(values) + margin, 4)]


def label(suite, *parts):
    return "restitution-%s.%s" % (suite, ".".join(parts))
# ---------------------------------------------------------------------------
# Build: the host and the staged Box3D providers
# ---------------------------------------------------------------------------

def host_dir(build):
    return os.path.join(build, "corpus-restitution")


def host_binary(build):
    return os.path.join(host_dir(build), "restitution_scenes")


def waf_env(build):
    """The physicstest target's Waf ConfigSet (literal assignments only)."""
    for name in ("unittests/physicstest_cache.py", "_cache.py"):
        path = os.path.join(build, "c4che", name)
        if os.path.isfile(path):
            env = {}
            with open(path, "r", encoding="utf-8") as stream:
                for line in stream:
                    match = re.match(r"^(\w+) = (.*)$", line.rstrip("\n"))
                    if match:
                        try:
                            env[match.group(1)] = ast.literal_eval(match.group(2))
                        except (ValueError, SyntaxError):
                            pass
            return env, path
    raise FileNotFoundError("no Waf configuration under %s/c4che (configure the tree first)" % build)


def build_host(build, jobs=8):
    """Compiles the host with the flags Waf uses for vphysics_conformance and
    links it against the tree's tier0/tier1/vstdlib/mathlib."""
    env, cache = waf_env(build)
    out = host_dir(build)
    objects_dir = os.path.join(out, "obj")
    os.makedirs(objects_dir, exist_ok=True)
    cxx = env["CXX"]
    flags = [f for f in env.get("CXXFLAGS", []) if f != "-MMD"]
    includes = ["-I" + os.path.join(ROOT, p) for p in ("public", "public/tier0", "public/tier1")]
    includes += ["-I" + p for p in env.get("INCLUDES", [])]
    defines = ["-D" + d for d in env.get("DEFINES", [])]
    sources = [HOST_SOURCE] + HOST_LINKED
    commands = []
    for source in sources:
        obj = os.path.join(objects_dir, os.path.basename(source) + ".o")
        commands.append(cxx + flags + includes + defines + [os.path.join(ROOT, source), "-c", "-o", obj])

    def compile_one(argv):
        run = subprocess.run(argv, capture_output=True, text=True)
        return argv, run.returncode, run.stderr

    with concurrent.futures.ThreadPoolExecutor(max_workers=max(1, jobs)) as pool:
        for argv, rc, err in pool.map(compile_one, commands):
            if rc != 0:
                print("compile failed: %s\n%s" % (" ".join(argv), err[-4000:]))
                return 1
    link = cxx + env.get("LINKFLAGS", []) + [
        os.path.join(objects_dir, os.path.basename(s) + ".o") for s in sources] + [
        "-o", host_binary(build),
        "-Wl,-Bstatic", "-L" + os.path.join(build, "tier1"), "-L" + os.path.join(build, "mathlib"),
        "-ltier1", "-lmathlib", "-Wl,-Bdynamic",
        "-L" + os.path.join(build, "vstdlib"), "-L" + os.path.join(build, "tier0"),
        "-lvstdlib", "-ltier0", "-ldl", "-lm", "-lSDL3"]
    run = subprocess.run(link, capture_output=True, text=True)
    if run.returncode != 0:
        print("link failed: %s\n%s" % (" ".join(link), run.stderr[-4000:]))
        return 1
    info = {
        "binary": os.path.relpath(host_binary(build), ROOT),
        "sha256": provenance.file_sha256(host_binary(build)),
        "waf_env": os.path.relpath(cache, ROOT),
        "compiler": provenance.tool_version(cxx + ["--version"]),
        "sources": {s: provenance.file_sha256(os.path.join(ROOT, s)) for s in sources + HOST_INCLUDED},
        "built": datetime.datetime.now(datetime.timezone.utc).isoformat(),
    }
    with open(os.path.join(out, "host.json"), "w", encoding="utf-8") as stream:
        json.dump(info, stream, indent=2, sort_keys=True)
    print("built %s" % host_binary(build))
    return 0


def box3d_identity():
    """The box3d submodule's checked-out commit and whether its source is clean."""
    head = subprocess.run(["git", "-C", os.path.join(ROOT, "box3d"), "rev-parse", "HEAD"],
                          capture_output=True, text=True).stdout.strip()
    dirty = subprocess.run(["git", "-C", os.path.join(ROOT, "box3d"), "status", "--porcelain", "--", "src",
                            "include"], capture_output=True, text=True).stdout.strip()
    return head, bool(dirty)


def provider_slot(build, commit):
    return os.path.join(host_dir(build), "providers", "box3d-" + commit[:7])


def provider_path(build, name):
    if name == "ivp":
        return os.path.join(build, "vphysics", "libvphysics.so")
    return os.path.join(provider_slot(build, PROVIDERS[name]), "libvphysics_box3d.so")


def provider_info(build, name):
    """What was compiled into a provider: for Box3D the staged build.json."""
    path = provider_path(build, name)
    info = {"library": os.path.relpath(path, ROOT),
            "sha256": provenance.file_sha256(path) if os.path.isfile(path) else None}
    if name != "ivp":
        record = os.path.join(os.path.dirname(path), "build.json")
        if os.path.isfile(record):
            with open(record, "r", encoding="utf-8") as stream:
                info["box3d"] = json.load(stream)
    return info


def stage_box3d(build):
    """Copies the tree's freshly built Box3D provider into the slot of the
    box3d commit it was compiled from."""
    library = os.path.join(build, "vphysics_box3d", "libvphysics_box3d.so")
    head, dirty = box3d_identity()
    if dirty:
        print("stage-box3d: box3d has local changes; stage only a committed source")
        return 1
    if not os.path.isfile(library):
        print("stage-box3d: %s missing (build vphysics_box3d first)" % library)
        return 1
    # A library older than any box3d source was not built from this checkout.
    newest = 0.0
    for directory in ("src", "include"):
        for base, _, files in os.walk(os.path.join(ROOT, "box3d", directory)):
            for name in files:
                newest = max(newest, os.path.getmtime(os.path.join(base, name)))
    if os.path.getmtime(library) < newest:
        print("stage-box3d: %s is older than the box3d %s source; rebuild it" % (library, head[:7]))
        return 1
    slot = provider_slot(build, head)
    os.makedirs(slot, exist_ok=True)
    shutil.copy2(library, os.path.join(slot, "libvphysics_box3d.so"))
    solver = os.path.join(ROOT, "box3d", "src", "contact_solver.c")
    record = {
        "box3d_commit": head,
        "contact_solver_c_sha256": provenance.file_sha256(solver),
        "library_sha256": provenance.file_sha256(library),
        "staged": datetime.datetime.now(datetime.timezone.utc).isoformat(),
        "role": {BOX3D_UPSTREAM: "upstream", BOX3D_PATCHED: "patched (fork johnpanos/box3d)"}.get(head, "other"),
    }
    with open(os.path.join(slot, "build.json"), "w", encoding="utf-8") as stream:
        json.dump(record, stream, indent=2, sort_keys=True)
    print("staged box3d %s -> %s" % (head[:7], slot))
    return 0


# ---------------------------------------------------------------------------
# Content and runs
# ---------------------------------------------------------------------------

def default_runtime():
    return os.environ.get("CORPUS_RUNTIME") or os.path.join(ROOT, "run", "runtime")


def extract_content(runtime, out_dir):
    """Surface properties (manifest order, `default` first) and the model."""
    resolver = ContentResolver(runtime)
    content = os.path.join(out_dir, "content")
    os.makedirs(content, exist_ok=True)
    origins = {}

    def extract(relative):
        data, origin = resolver.read(relative)
        if data is None:
            raise FileNotFoundError("required content missing from runtime: %s" % relative)
        target = os.path.join(content, relative.replace("/", "__"))
        with open(target, "wb") as stream:
            stream.write(data)
        origins[relative] = {"origin": origin, "sha256": hashlib.sha256(data).hexdigest()}
        return target

    manifest = extract(SURFACE_MANIFEST)
    with open(manifest, "r", encoding="utf-8", errors="replace") as stream:
        names = re.findall(r'"file"\s+"([^"]+)"', stream.read())
    if not names:
        raise ValueError("surface property manifest lists no files")
    surfaces = [extract(name) for name in names]
    models = {model: extract(model) for model in MODELS}
    return surfaces, models, origins


def run_scene(build, provider, scene, variant, surfaces, model, fault=None, trace=False, timeout=120):
    """Runs one scene variant in its own process; returns the parsed record."""
    command = [host_binary(build), "--provider", provider_path(build, provider), "--scene", scene]
    for path in surfaces:
        command += ["--surfaceprops", path]
    command += ["--phy", model] + list(variant["args"])
    if fault:
        command += ["--fault", fault]
    if trace:
        command.append("--trace")
    env = dict(os.environ)
    env["LD_LIBRARY_PATH"] = os.pathsep.join(
        [os.path.join(build, "tier0"), os.path.join(build, "vstdlib"), env.get("LD_LIBRARY_PATH", "")])
    try:
        run = subprocess.run(command, capture_output=True, text=True, env=env, timeout=timeout)
        stdout, returncode, outcome = run.stdout, run.returncode, None
    except subprocess.TimeoutExpired as exc:
        stdout, returncode, outcome = conformance._as_text(exc.stdout), None, "timeout"
    records = [line for line in stdout.splitlines() if line.startswith("RUN ")]
    values = {}
    if len(records) == 1:
        for token in records[0].split()[1:]:
            key, _, value = token.partition("=")
            try:
                values[key] = float(value)
            except ValueError:
                values[key] = value
    if outcome is None:
        if returncode is not None and returncode < 0:
            outcome = "crash"
        elif returncode != 0 or len(records) != 1:
            outcome = "incomplete"
        else:
            outcome = "ran"
    return {"outcome": outcome, "values": values, "command": command, "returncode": returncode,
            "failures": [line for line in stdout.splitlines() if line.startswith("FAIL ")],
            "trace": [line for line in stdout.splitlines() if line.startswith("TICK ")] if trace else [],
            "stdout_tail": conformance.tail(stdout, 10) if outcome != "ran" else ""}


# ---------------------------------------------------------------------------
# Running cases
# ---------------------------------------------------------------------------

def run_case(build, provider, case, surfaces, models, fault=None, jobs=4):
    """Every variant of a case, one process each; results in variant order."""
    def one(variant):
        return run_scene(build, provider, case["scene"], variant, surfaces, models[case["model"]], fault)

    with concurrent.futures.ThreadPoolExecutor(max_workers=max(1, jobs)) as pool:
        return list(pool.map(one, case["variants"]))


def observed_keys(values):
    return [k for k in RANGE_FLOORS if k in values]


def load_fixture():
    if not os.path.isfile(FIXTURE):
        return None
    with open(FIXTURE, "r", encoding="utf-8") as stream:
        return json.load(stream)


def case_fixture(fixture, suite_id, case):
    return ((fixture or {}).get("suites", {}).get(suite_id, {}).get("cases", {}).get(case["id"]))


def summary_line(case, variant, values, keys):
    return "OBS %s[%s] %s" % (case["id"], variant["id"], " ".join(
        "%s=%.4g" % (k, values[k]) for k in keys if isinstance(values.get(k), float)))


def evaluate(checks, suite_id, case, results, recorded):
    """Emits one case's checks in a fixed order: runs, gameplay, analytic,
    IVP ranges. `recorded` is the case's fixture entry (IVP)."""
    variants = case["variants"]
    ran = []
    for variant, result in zip(variants, results):
        ok = checks.check(result["outcome"] == "ran", label(suite_id, "run", "%s[%s]" % (case["id"], variant["id"])),
                          "%s rc %s %s %s" % (result["outcome"], result["returncode"], result["failures"][:2],
                                              result["stdout_tail"]))
        ran.append(ok)
        if ok:
            print(summary_line(case, variant, result["values"],
                               observed_keys(result["values"]) + ["final_z", "energy_ratio_max"]))

    # Gameplay: the parity suite's clauses. The canonical variant always; a
    # perturbed variant only where the reference (IVP) holds the clause too.
    holds = (recorded or {}).get("ivp_holds", {})
    for name in case["gameplay"]:
        for index, (variant, result) in enumerate(zip(variants, results)):
            if not ran[index]:
                continue
            applies = variant["id"] == variants[0]["id"] or holds.get(name, {}).get(variant["id"], False)
            ok, detail = gameplay_rule(name, result["values"])
            if not applies:
                print("NOTE %s: not gated (IVP does not hold it here); %s %s" % (
                    label(suite_id, "gameplay", name, "%s[%s]" % (case["id"], variant["id"])),
                    "holds" if ok else "fails", detail))
                continue
            checks.check(ok, label(suite_id, "gameplay", name, "%s[%s]" % (case["id"], variant["id"])), detail)

    for name in case["analytic"]:
        for index, (variant, result) in enumerate(zip(variants, results)):
            if ran[index]:
                ok, detail = analytic_rule(name, result["values"])
                checks.check(ok, label(suite_id, "analytic", name, "%s[%s]" % (case["id"], variant["id"])), detail)

    ranges = (recorded or {}).get("ranges", {})
    for key in case["gated"]:
        bounds = ranges.get(key)
        if bounds is None:
            checks.check(False, label(suite_id, "ivp-range", case["id"], key),
                         "no recorded IVP range (re-record the fixture)")
            continue
        for index, (variant, result) in enumerate(zip(variants, results)):
            if ran[index]:
                value = result["values"].get(key)
                checks.within(value, bounds[0], bounds[1],
                              label(suite_id, "ivp-range", case["id"], "%s[%s]" % (key, variant["id"])))


def fixture_checks(checks, suite_id, fixture):
    ok = checks.check(fixture is not None, label(suite_id, "fixture.present"), os.path.relpath(FIXTURE, ROOT))
    if not ok:
        return False
    checks.equal(fixture.get("schema"), FIXTURE_SCHEMA, label(suite_id, "fixture.schema"))
    checks.equal(fixture.get("version"), FIXTURE_VERSION, label(suite_id, "fixture.version"))
    checks.equal(fixture.get("tolerance_policy", {}).get("relative"), RANGE_RELATIVE,
                 label(suite_id, "fixture.policy-relative"))
    checks.equal(fixture.get("tolerance_policy", {}).get("floors"), RANGE_FLOORS,
                 label(suite_id, "fixture.policy-floors"))
    for case in SUITES[suite_id]["cases"]:
        recorded = case_fixture(fixture, suite_id, case)
        checks.check(recorded is not None, label(suite_id, "fixture.case", case["id"]), "not recorded")
        if recorded:
            checks.equal([[v["id"], v["args"]] for v in recorded.get("variants", [])],
                         [[v["id"], v["args"]] for v in case["variants"]],
                         label(suite_id, "fixture.variants", case["id"]))
    return True


def provider_checks(checks, suite_id, build, provider):
    path = provider_path(build, provider)
    ok = checks.check(os.path.isfile(path), label(suite_id, "provider.present", provider), path)
    if ok and PROVIDERS[provider]:
        info = provider_info(build, provider).get("box3d", {})
        # The staged library must have been compiled from the named box3d.
        checks.equal(info.get("box3d_commit"), PROVIDERS[provider], label(suite_id, "provider.box3d-commit", provider))
        checks.equal(info.get("library_sha256"), provenance.file_sha256(path),
                     label(suite_id, "provider.library-digest", provider))
    return ok


def common_provenance(suite, build, providers, fault=None, extra=None):
    host = os.path.join(host_dir(build), "host.json")
    host_info = None
    if os.path.isfile(host):
        with open(host, "r", encoding="utf-8") as stream:
            host_info = json.load(stream)
    block = provenance.collect(
        suite,
        tools={"host": host_info and host_info.get("compiler")},
        inputs={"fixture": FIXTURE, "host": host_binary(build)},
        extra=dict({"providers": {p: provider_info(build, p) for p in providers},
                    "host_build": host_info, "fault": fault or "none"}, **(extra or {})))
    return block


# ---------------------------------------------------------------------------
# Commands
# ---------------------------------------------------------------------------

def scratch_dir(args):
    out = args.out or os.environ.get("CONFORMANCE_OUT") or os.path.join(host_dir(args.build), "scratch")
    os.makedirs(out, exist_ok=True)
    return out


def cmd_check(args):
    checks = Checks()
    suite_id = args.suite
    fixture = load_fixture()
    block = common_provenance("corpus.physics.restitution-%s" % suite_id, args.build, [args.provider], args.fault)
    provenance.emit(block, args.out or os.environ.get("CONFORMANCE_OUT"))
    have_fixture = fixture_checks(checks, suite_id, fixture)
    have_provider = provider_checks(checks, suite_id, args.build, args.provider)
    have_host = checks.check(os.path.isfile(host_binary(args.build)), label(suite_id, "host.present"),
                             host_binary(args.build))
    if not (have_fixture and have_provider and have_host):
        return checks.report()
    surfaces, models, _ = extract_content(args.runtime, scratch_dir(args))
    print("PROVIDER %s %s FAULT %s" % (args.provider, provider_path(args.build, args.provider), args.fault or "none"))
    for case in SUITES[suite_id]["cases"]:
        results = run_case(args.build, args.provider, case, surfaces, models, args.fault, args.jobs)
        evaluate(checks, suite_id, case, results, case_fixture(fixture, suite_id, case))
    return checks.report()


def cmd_record(args):
    """Runs every case under IVP and writes the fixture."""
    out = scratch_dir(args)
    surfaces, models, origins = extract_content(args.runtime, out)
    suites = {}
    refuse = []
    for suite_id, suite in SUITES.items():
        cases = {}
        for case in suite["cases"]:
            results = run_case(args.build, "ivp", case, surfaces, models, None, args.jobs)
            bad = [v["id"] for v, r in zip(case["variants"], results) if r["outcome"] != "ran"]
            if bad:
                refuse.append("%s/%s did not run: %s" % (suite_id, case["id"], bad))
                continue
            values = {v["id"]: r["values"] for v, r in zip(case["variants"], results)}
            keys = observed_keys(results[0]["values"])
            ranges = {k: ivp_range([values[v["id"]][k] for v in case["variants"]], k) for k in keys}
            holds = {}
            for name in case["gameplay"]:
                holds[name] = {v["id"]: gameplay_rule(name, values[v["id"]])[0] for v in case["variants"]}
                if not holds[name][case["variants"][0]["id"]]:
                    refuse.append("IVP fails %s on %s/%s canonical" % (name, suite_id, case["id"]))
            for name in case["analytic"]:
                for v in case["variants"]:
                    ok, detail = analytic_rule(name, values[v["id"]])
                    if not ok:
                        refuse.append("IVP breaks analytic %s on %s/%s[%s]: %s" % (
                            name, suite_id, case["id"], v["id"], detail))
            cases[case["id"]] = {
                "scene": case["scene"], "model": case["model"], "source": case["source"],
                "gameplay": case["gameplay"], "analytic": case["analytic"], "gated": case["gated"],
                "variants": [{"id": v["id"], "args": v["args"]} for v in case["variants"]],
                "ivp": {vid: {k: values[vid][k] for k in sorted(values[vid]) if isinstance(values[vid][k], float)}
                        for vid in values},
                "ranges": ranges,
                "ivp_holds": holds,
            }
        suites[suite_id] = {"cases": cases}
    if refuse:
        print("record: refusing to write the fixture:\n  " + "\n  ".join(refuse))
        return 1
    block = common_provenance("corpus.physics.restitution (record)", args.build, ["ivp"],
                              extra={"content": origins})
    fixture = {
        "schema": FIXTURE_SCHEMA,
        "version": FIXTURE_VERSION,
        "reference": "ivp",
        "description": "IVP reference ranges for the Box3D restitution scenes "
                       "(tools/quality/corpus/physics_restitution.py). Each case lists its variants, the IVP "
                       "observations per variant, the ranges derived from them by the tolerance policy, and "
                       "where IVP itself holds each gameplay clause. No Box3D measurement is recorded here.",
        "tolerance_policy": {
            "relative": RANGE_RELATIVE, "floors": RANGE_FLOORS, "signed": sorted(SIGNED),
            "sentinel": sorted(SENTINEL),
            "rule": "margin = max(floor, relative * max|IVP value over variants)); range = [min - margin, "
                    "max + margin], low end clamped at 0 unless signed; no range when a sentinel "
                    "observation is -1 (never) in any IVP variant",
        },
        "gameplay_bounds": {"dynamics.tumble-travel-bounded": {"travel_lt": TUMBLE_TRAVEL_MAX},
                            "dynamics.held-floor-quiet": {"travel_lt": HELD_TRAVEL_MAX,
                                                          "quiet_angular_lt": HELD_ANGULAR_MAX}},
        "analytic_bounds": {"energy_ratio_max": ENERGY_RATIO_MAX, "energy_rise_fraction": ENERGY_RISE_FRACTION,
                            "geometry_floor_z": GEOMETRY_FLOOR_Z, "rest_gain_max": REST_GAIN_MAX,
                            "rest_speed_max": REST_SPEED_MAX, "rest_travel_max": REST_TRAVEL_MAX},
        "provenance": block,
        "suites": suites,
    }
    os.makedirs(os.path.dirname(FIXTURE), exist_ok=True)
    with open(FIXTURE, "w", encoding="utf-8") as stream:
        json.dump(fixture, stream, indent=2, sort_keys=True)
        stream.write("\n")
    print("wrote %s" % os.path.relpath(FIXTURE, ROOT))
    return 0


def cmd_report(args):
    """IVP, Box3D upstream and Box3D patched side by side (canonical value and
    the min..max over variants), with each provider's failing checks."""
    fixture = load_fixture()
    surfaces, models, _ = extract_content(args.runtime, scratch_dir(args))
    providers = [p for p in PROVIDERS if os.path.isfile(provider_path(args.build, p))]
    evidence = {"providers": {p: provider_info(args.build, p) for p in providers}, "suites": {}}
    for suite_id, suite in SUITES.items():
        print("\n## %s" % suite_id)
        for case in suite["cases"]:
            recorded = case_fixture(fixture, suite_id, case) or {}
            per = {}
            for provider in providers:
                per[provider] = run_case(args.build, provider, case, surfaces, models, None, args.jobs)
            keys = case["gated"] + [k for k in ("bounces", "unrested", "off_floor", "min_final_z", "min_geom_final_z", "geom_min_z_seen",
                                                "energy_ratio_max", "rest_offset_z", "final_tilt")
                                    if k not in case["gated"] and k in per[providers[0]][0]["values"]]
            print("\n%s (%s, %s)\n" % (case["id"], case["scene"], case["model"]))
            print("| observation | IVP range | " + " | ".join(providers) + " |")
            print("| --- | --- | " + " | ".join("---" for _ in providers) + " |")
            for key in keys:
                row = ["%s%s" % (key, "" if key in case["gated"] else " (reported)"),
                       str(recorded.get("ranges", {}).get(key, "-"))]
                for provider in providers:
                    vals = [r["values"].get(key) for r in per[provider] if isinstance(r["values"].get(key), float)]
                    if not vals:
                        row.append("-")
                        continue
                    row.append("%.3g (%.3g..%.3g)" % (vals[0], min(vals), max(vals)))
                print("| " + " | ".join(row) + " |")
            for name in case["gameplay"]:
                row = []
                for provider in providers:
                    outcome = [gameplay_rule(name, r["values"])[0] for r in per[provider] if r["outcome"] == "ran"]
                    row.append("%s: canonical %s, %d/%d variants" % (
                        provider, "holds" if outcome and outcome[0] else "FAILS", sum(outcome), len(outcome)))
                print("- %s: %s" % (name, "; ".join(row)))
            evidence["suites"].setdefault(suite_id, {})[case["id"]] = {
                p: [{"variant": v["id"], "outcome": r["outcome"], "values": r["values"]}
                    for v, r in zip(case["variants"], per[p])] for p in providers}
    if args.json:
        with open(args.json, "w", encoding="utf-8") as stream:
            json.dump(evidence, stream, indent=2, sort_keys=True)
    return 0


def main(argv=None):
    parser = argparse.ArgumentParser(description="Box3D restitution scenes with an IVP reference")
    parser.add_argument("command", choices=["check", "record", "report", "build-host", "stage-box3d"])
    parser.add_argument("--build", default=os.path.join(ROOT, "build-corpus-physics"),
                        help="the private Waf tree (default build-corpus-physics)")
    parser.add_argument("--runtime", default=default_runtime(), help="staged game runtime with content")
    parser.add_argument("--suite", choices=sorted(SUITES), help="check: the scene suite")
    parser.add_argument("--provider", choices=sorted(PROVIDERS), default="ivp")
    parser.add_argument("--fault", choices=["elasticity-max"],
                        help="check: seeded provider fault (negative controls)")
    parser.add_argument("--out", help="scratch directory (default $CONFORMANCE_OUT)")
    parser.add_argument("--json", help="report: also write the observations here")
    parser.add_argument("--jobs", type=int, default=4, help="scene processes at once")
    args = parser.parse_args(argv)
    args.build = os.path.abspath(args.build)
    if args.command == "build-host":
        return build_host(args.build, args.jobs)
    if args.command == "stage-box3d":
        return stage_box3d(args.build)
    if args.command == "record":
        return cmd_record(args)
    if args.command == "report":
        return cmd_report(args)
    if not args.suite:
        parser.error("check needs --suite")
    return cmd_check(args)


if __name__ == "__main__":
    sys.exit(main())
