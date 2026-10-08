#!/usr/bin/env python3
"""RFC 0027 L1 gate: launch equivalence between the old launchers and kiln.

For every launcher mode, the old launcher runs for real (staging and all) up to
the moment it exec()s the game; tools/kiln/exec_capture.c records that exec
(working directory, argv, environment) and stops it. `kiln play --dry-run`
gives kiln's plan for the same request. Paths are normalized ({root},
{runtime}); argv must be identical and the launcher-set environment equal,
apart from the recorded differences below. Seeded differences (a dropped
switch, a swapped switch order, an extra variable) must be caught.

    launch_equivalence.py check [--mode NAME]... [--keep]
    launch_equivalence.py list

Old launchers build into kiln's own trees (out/<profile>/<flavor>) through Waf
lock aliases and stage into scratch runtimes under out/launch-equivalence, so
neither run/ nor any build-* tree is touched. Prints one checks-v1 record.
"""

import argparse
import json
import os
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SCRATCH = ROOT / "out" / "launch-equivalence"


def legacy_run_directory():
    """The run/ that holds the seed runtime and AV1 transcodes: this checkout's,
    or for a linked worktree, the main checkout's (KILN_LEGACY_RUN overrides)."""
    if os.environ.get("KILN_LEGACY_RUN"):
        return Path(os.environ["KILN_LEGACY_RUN"])
    if (ROOT / "run/runtime/portal/gameinfo.txt").is_file():
        return ROOT / "run"
    common = subprocess.run(["git", "rev-parse", "--git-common-dir"], cwd=ROOT, capture_output=True,
                            text=True).stdout.strip()
    return (ROOT / common).resolve().parent / "run"


MAIN_RUN = legacy_run_directory()
STEAM_P2 = Path.home() / ".local/share/Steam/steamapps/common/Portal 2"

# Variables the old launchers export for their own plumbing (./play hands its
# choices to ./run.sh through the environment). They are not part of the
# game's launch; kiln passes these facts as data, not environment.
RECORDED_ENV_DIFFERENCES = {
    "BUILD_DIR": "./play -> run.sh plumbing: the tree to stage",
    "RENDERER": "./play -> run.sh plumbing: the renderer id, an argv value",
    "RUNTIME": "./play -> run.sh plumbing: the runtime directory",
    "BASE_RUNTIME": "./play -> run.sh plumbing: the seed runtime",
    "MAP": "./play -> run.sh plumbing: the map, an argv value",
    "CCACHE_DIR": "launcher_ccache.sh exports the build's compiler cache, which the game inherits; "
                  "in kiln the compiler cache belongs to the engine stage (RFC 0027 shared concerns), "
                  "not the launch",
}
NOISE = {"PWD", "OLDPWD", "SHLVL", "_", "LD_PRELOAD", "KILN_CAPTURE_FILE", "KILN_CAPTURE_NAMES"}

# name: (old argv, kiln play arguments, kiln profile for its tree, legacy kind)
MODES = {
    "play": (["./play"], ["portal"], "portal-linux-native-vulkan", "play"),
    "play-null": (["./play", "--null"], ["portal", "--set", "null"], "portal-linux-native-vulkan", "play"),
    "play-map-switches": (["./play", "--validate", "--no-core-world", "testchmb_a_02", "-console"],
                          ["portal", "testchmb_a_02", "--set", "validate", "--set", "no-core-world",
                           "--", "-console"], "portal-linux-native-vulkan", "play"),
    "play-render-state": (["./play", "--moving-light-gi", "--baked-direct", "--area-lights", "--no-core",
                           "--core-probe=empty", "--soft-shadows", "--probe-bounce", "--no-legacy-ports"],
                          ["portal", "--set", "moving-light-gi", "--set", "baked-direct", "--set", "area-lights",
                           "--set", "no-core", "--set", "core-probe-empty", "--set", "soft-shadows",
                           "--set", "probe-bounce", "--set", "no-legacy-ports"],
                          "portal-linux-native-vulkan", "play"),
    "play-env": (["./play"], ["portal", "--set", "ivp", "--set", "serial-jobs"], "portal-linux-native-vulkan",
                 "play", {"PHYSICS": "vphysics", "JOB_ARGS": ""}),
    "play_p2": (["./play_p2"], ["portal2"], "portal2-linux-native-vulkan", "p2"),
    "play_p2-map": (["./play_p2", "+map", "sp_a1_intro4"], ["portal2", "sp_a1_intro4"],
                    "portal2-linux-native-vulkan", "p2"),
    "play_p2-switches": (["./play_p2", "--validate", "--moving-light-gi", "--null", "--hard-shadows"],
                         ["portal2", "--set", "validate", "--set", "moving-light-gi", "--set", "null",
                          "--set", "hard-shadows"], "portal2-linux-native-vulkan", "p2"),
    "play_p2-env": (["./play_p2"], ["portal2", "--set", "bink", "--set", "sync-queue", "--set", "ivp"],
                    "portal2-linux-native-vulkan", "p2",
                    {"VIDEO_ARGS": "-video-provider bink", "QUEUE_ARGS": "", "PHYSICS": "vphysics"}),
    "play_p2_fsr": (["./play_p2_fsr"], ["portal2-fsr"], "portal2-fsr", "p2"),
    "play_fstop": (["./play_fstop"], ["fstop"], "fstop-linux-native-vulkan", "fstop"),
    "play_fstop-map": (["./play_fstop", "+map", "lab_01"], ["fstop", "--", "+map", "lab_01"],
                       "fstop-linux-native-vulkan", "fstop"),
}
# Seeded differences the comparison must catch: (mode, kiln arguments).
SEEDED = {
    "dropped-switch": ("play-map-switches", ["portal", "testchmb_a_02", "--set", "validate", "--", "-console"]),
    "swapped-switch-order": ("play_p2-switches", ["portal2", "--set", "hard-shadows", "--set", "moving-light-gi",
                                                  "--set", "null", "--set", "validate"]),
    "extra-map": ("play_p2", ["portal2", "sp_a1_intro1"]),
}


class Record:
    def __init__(self):
        self.checks = self.failures = 0

    def check(self, condition, name, detail=""):
        self.checks += 1
        if not condition:
            self.failures += 1
            print("FAIL: %s%s" % (name, (": " + detail) if detail else ""), file=sys.stderr)
        return condition


def build_shim():
    out = SCRATCH / "exec_capture.so"
    out.parent.mkdir(parents=True, exist_ok=True)
    source = ROOT / "tools/kiln/exec_capture.c"
    if not out.is_file() or out.stat().st_mtime < source.stat().st_mtime:
        subprocess.run(["cc", "-O1", "-shared", "-fPIC", "-o", str(out), str(source), "-ldl"], check=True)
    return out


def read_capture(path):
    parts = Path(path).read_bytes().split(b"\0")
    values = [p.decode("utf-8", "surrogateescape") for p in parts]
    at = 0

    def take():
        nonlocal at
        value = values[at]
        at += 1
        return value
    record = {}
    while at < len(values) - 1:
        key = take()
        if key in ("cwd", "path"):
            record[key] = take()
        elif key in ("argv", "env"):
            count = int(take())
            record[key] = [take() for _ in range(count)]
    record["env"] = dict(item.split("=", 1) for item in record.get("env", []) if "=" in item)
    return record


def kiln(*args):
    result = subprocess.run([str(ROOT / "kiln"), *args], cwd=ROOT, capture_output=True, text=True)
    if result.returncode:
        raise RuntimeError("kiln %s: %s" % (" ".join(args), result.stderr.strip()))
    return result.stdout


def tree(profile, flavor="dev"):
    return ROOT / "out" / profile / flavor / "build"


def lock_aliases(build, name):
    """The old launchers find a tree through a Waf lock in the repository root
    and ensure_configured.py reads one inside the tree; kiln keeps its own lock
    (.lock-waf-kiln) only inside. Alias both for the launcher's lock name."""
    lock = build / ".lock-waf-kiln"
    made = []
    for target in (ROOT / name, build / name):
        if not target.exists():
            shutil.copy2(lock, target)
            made.append(target)
    return made


def align_published_maps():
    """The old staging reads run/maps beside its own scripts; give a linked
    worktree the same store kiln's workspace names (both see one store)."""
    for name in ("maps", "runtime-p2"):
        local = ROOT / "run" / name
        if MAIN_RUN != ROOT / "run" and not local.exists() and (MAIN_RUN / name).exists():
            local.parent.mkdir(parents=True, exist_ok=True)
            local.symlink_to(MAIN_RUN / name, target_is_directory=True)


def old_launch(mode, argv, profile, kind, extra_env):
    capture = SCRATCH / ("capture-%s.bin" % mode)
    capture.unlink(missing_ok=True)
    runtime = SCRATCH / ("runtime-" + kind)
    if kind == "fstop":
        # stage_fstop_runtime.py appends its F-Stop lines again on every
        # re-staging (a recorded defect kiln does not carry over), so the
        # reference is a freshly staged runtime.
        shutil.rmtree(runtime, ignore_errors=True)
    build = tree(profile)
    env = {k: v for k, v in os.environ.items()
           if k not in ("SDL_VIDEODRIVER", "SDL_VIDEO_DRIVER", "LD_LIBRARY_PATH", "SteamAppId", "SteamGameId",
                        "WAFLOCK", "NO_LOCK_IN_TOP", "NO_LOCK_IN_RUN")}
    env.update({"LD_PRELOAD": str(build_shim()), "KILN_CAPTURE_FILE": str(capture),
                "KILN_CAPTURE_NAMES": "hl2_launcher:portal2.sh"})
    aliases = []
    if kind == "play":
        env.update({"BUILD": "0", "PLAY_BUILD_DIR": str(build), "RUNTIME": str(runtime),
                    "BASE_RUNTIME": str(MAIN_RUN / "runtime")})
    elif kind == "p2":
        flavor_lock = ".lock-waf-kilneq-" + profile
        aliases = lock_aliases(build, flavor_lock)
        env.update({"P2_BUILD_DIR": str(build), "P2_RUNTIME": str(runtime) + "-" + profile,
                    "P2_WAFLOCK": flavor_lock, "P2_AV1_MEDIA": str(MAIN_RUN / "media-av1")})
        runtime = Path(str(runtime) + "-" + profile)
    elif kind == "fstop":
        aliases = lock_aliases(build, ".lock-waf-fstop")
        env.update({"FSTOP_BUILD_DIR": str(build), "FSTOP_RUNTIME": str(runtime),
                    "FSTOP_BASE_RUNTIME": str(MAIN_RUN / "runtime")})
    env.update(extra_env)
    try:
        result = subprocess.run(argv, cwd=ROOT, env=env, capture_output=True, text=True, timeout=3600)
    finally:
        for alias in aliases:
            alias.unlink(missing_ok=True)
    if not capture.is_file():
        raise RuntimeError("%s did not reach the game exec (exit %d):\n%s" %
                           (" ".join(argv), result.returncode, (result.stdout + result.stderr)[-2000:]))
    record = read_capture(capture)
    base = {k: v for k, v in env.items() if k not in NOISE}
    return record, runtime.resolve(), base


def normalize(values, runtime):
    out = []
    for value in values:
        value = value.replace(str(runtime), "{runtime}").replace(str(ROOT), "{root}")
        out.append(value)
    return out


def launcher_env(record_env, base):
    """What the launcher set or changed relative to the environment it got."""
    return {k: v for k, v in record_env.items() if k not in NOISE and base.get(k) != v}


def kiln_env(plan, base):
    env = {}
    for name, value in plan["environment"].items():
        if value is None:
            continue
        env[name] = value.replace("{inherit}", base.get(name, ""))
    return {k: v for k, v in env.items() if base.get(k) != v}


def compare(mode, record, old_runtime, base, kiln_args, record_out):
    plan = json.loads(kiln("play", kiln_args[0], "--dry-run", *kiln_args[1:]))
    new_runtime = Path(plan["runtime"])
    old_argv = normalize(record["argv"], old_runtime)
    new_argv = normalize(plan["argv"], new_runtime)
    same_argv = old_argv == new_argv
    detail = ""
    if not same_argv:
        first = next((i for i, (a, b) in enumerate(zip(old_argv, new_argv)) if a != b),
                     min(len(old_argv), len(new_argv)))
        detail = "first divergence at argv[%d]: old %r, kiln %r" % (
            first, old_argv[first:first + 3], new_argv[first:first + 3])
    old_env = {k: v for k, v in launcher_env(record["env"], base).items() if k not in RECORDED_ENV_DIFFERENCES}
    old_env = dict(zip(old_env, normalize(old_env.values(), old_runtime)))
    new_env = kiln_env(plan, base)
    new_env = dict(zip(new_env, normalize(new_env.values(), new_runtime)))
    same_env = old_env == new_env
    if not same_env:
        keys = sorted(set(old_env) | set(new_env))
        detail += " environment differs: " + ", ".join(
            "%s old=%r kiln=%r" % (k, old_env.get(k), new_env.get(k)) for k in keys if old_env.get(k) != new_env.get(k))
    same_cwd = normalize([record["cwd"]], old_runtime) == normalize([plan["working_directory"]], new_runtime)
    record_out[mode] = {"old_argv": old_argv, "kiln_argv": new_argv, "old_env": old_env, "kiln_env": new_env,
                        "argv_equal": same_argv, "env_equal": same_env, "cwd_equal": same_cwd}
    return same_argv, same_env, same_cwd, detail.strip()


def runtime_manifest(root):
    """Every entry of a runtime: files by content hash, links by resolved
    target, directories by name. kiln's own record is not content."""
    import hashlib
    root = Path(root)
    out = {}
    for directory, directories, files in os.walk(root, followlinks=False):
        for name in directories + files:
            path = Path(directory) / name
            relative = path.relative_to(root).as_posix()
            if name == ".kiln-package.json":
                continue
            if path.is_symlink():
                out[relative] = ("link", os.path.realpath(path))
            elif path.is_dir():
                out[relative] = ("dir",)
            else:
                digest = hashlib.sha256()
                with path.open("rb") as stream:
                    for chunk in iter(lambda: stream.read(1 << 20), b""):
                        digest.update(chunk)
                out[relative] = ("file", digest.hexdigest())
    return out


# Entries that may differ between the runtimes, with the reason.
RECORDED_MANIFEST_DIFFERENCES = {}


def compare_manifests(old_root, new_root):
    old, new = runtime_manifest(old_root), runtime_manifest(new_root)
    problems = []
    for key in sorted(set(old) | set(new)):
        if key in RECORDED_MANIFEST_DIFFERENCES:
            continue
        if old.get(key) != new.get(key):
            problems.append("%s: old %s, kiln %s" % (key, old.get(key, ("absent",))[0],
                                                     new.get(key, ("absent",))[0]))
    return len(old), len(new), problems


def check(selected, keep):
    record = Record()
    SCRATCH.mkdir(parents=True, exist_ok=True)
    align_published_maps()
    evidence = {"schema": "kiln-launch-equivalence/v1", "recorded_env_differences":
                {k: v for k, v in RECORDED_ENV_DIFFERENCES.items() if v}, "modes": {}, "seeded": {}}
    built = set()
    packaged = {}
    captures = {}
    evidence["manifests"] = {}
    for mode in selected:
        argv, kiln_args, profile, kind, *rest = MODES[mode]
        extra_env = rest[0] if rest else {}
        if profile not in built:
            kiln("build", kiln_args[0])
            built.add(profile)
        try:
            capture, runtime, base = old_launch(mode, argv, profile, kind, extra_env)
        except RuntimeError as error:
            record.check(False, mode + ": the old launcher reaches the game exec", str(error)[-600:])
            continue
        captures[mode] = (capture, runtime, base)
        same_argv, same_env, same_cwd, detail = compare(mode, capture, runtime, base, kiln_args, evidence["modes"])
        record.check(same_argv, mode + ": kiln play --dry-run gives the old launcher's argv", detail)
        record.check(same_env, mode + ": and its environment", detail)
        record.check(same_cwd, mode + ": and its working directory")
        if profile not in packaged:
            kiln("package", kiln_args[0])
            packaged[profile] = Path(json.loads(kiln("play", kiln_args[0], "--dry-run"))["runtime"])
            old_count, new_count, problems = compare_manifests(runtime, packaged[profile])
            evidence["manifests"][profile] = {"old_entries": old_count, "kiln_entries": new_count,
                                              "differences": problems[:50]}
            record.check(not problems, mode + ": the kiln runtime has the old runtime's manifest (%d entries)"
                         % old_count, "; ".join(problems[:5]))
            if not problems:
                # Seeded: one changed byte in a copied file must be caught.
                victim = next((p for p in sorted(Path(packaged[profile]).rglob("*.txt"))
                               if p.is_file() and not p.is_symlink()), None)
                if victim:
                    saved = victim.read_bytes()
                    victim.write_bytes(saved + b" ")
                    _, _, seeded = compare_manifests(runtime, packaged[profile])
                    victim.write_bytes(saved)
                    record.check(bool(seeded), mode + ": seeded difference caught: a changed runtime file")
    for name, (mode, kiln_args) in SEEDED.items():
        if mode not in captures:
            continue
        capture, runtime, base = captures[mode]
        same_argv, same_env, _, detail = compare("seeded:" + name, capture, runtime, base, kiln_args,
                                                 evidence["seeded"])
        record.check(not (same_argv and same_env), "seeded difference caught: " + name, detail)
    out = SCRATCH / "evidence.json"
    out.write_text(json.dumps(evidence, indent=1) + "\n")
    print("evidence: %s" % out, file=sys.stderr)
    if not keep:
        for path in SCRATCH.glob("capture-*.bin"):
            path.unlink()
    print("CONFORMANCE %d %d" % (record.checks, record.failures), flush=True)
    return 0 if record.checks and not record.failures else 1


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    sub = parser.add_subparsers(dest="command", required=True)
    run = sub.add_parser("check")
    run.add_argument("--mode", action="append", choices=sorted(MODES))
    run.add_argument("--keep", action="store_true")
    sub.add_parser("list")
    args = parser.parse_args(argv)
    if args.command == "list":
        for name, (argv_, kiln_args, *_rest) in MODES.items():
            print("%-22s %-60s kiln play %s" % (name, " ".join(argv_), " ".join(kiln_args)))
        return 0
    return check(args.mode or list(MODES), args.keep)


if __name__ == "__main__":
    sys.exit(main())
