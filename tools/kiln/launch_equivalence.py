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
    "PLAY_BUILD_DIR": "./play_release -> ./play plumbing: the tree name (build-release); kiln selects the tree "
                      "by --flavor, and no engine code reads the variable",
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
    "play_p2-workshop": (["./play_p2", "--workshop"], ["portal2", "--mounts", "p2ce-workshop"],
                         "portal2-linux-native-vulkan", "p2"),
    "play_p2-retail": (["./play_p2", "--retail", "-novid"], ["portal2-retail", "--", "-novid"], None, "retail"),
    "play_fstop": (["./play_fstop"], ["fstop"], "fstop-linux-native-vulkan", "fstop"),
    # RFC 0023 release flavors: the old wrappers against kiln's release trees.
    "play_release": (["./play_release"], ["portal", "--flavor", "release"], "portal-linux-native-vulkan",
                     "play"),
    "play_p2_release": (["./play_p2_release"], ["portal2", "--flavor", "release"],
                        "portal2-linux-native-vulkan", "p2"),
    "play_p2_fsr_release": (["./play_p2_fsr_release"], ["portal2-fsr", "--flavor", "release"], "portal2-fsr",
                            "p2"),
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


def flavor_of(kiln_args):
    return kiln_args[kiln_args.index("--flavor") + 1] if "--flavor" in kiln_args else "dev"


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


def old_launch(mode, argv, profile, kind, extra_env, flavor="dev", mounts=()):
    capture = SCRATCH / ("capture-%s.bin" % mode)
    capture.unlink(missing_ok=True)
    runtime = SCRATCH / ("runtime-" + kind)
    if kind == "retail":
        runtime = SCRATCH / "retail-has-no-runtime"
    if kind == "fstop":
        # stage_fstop_runtime.py appends its F-Stop lines again on every
        # re-staging (a recorded defect kiln does not carry over), so the
        # reference is a freshly staged runtime.
        shutil.rmtree(runtime, ignore_errors=True)
    build = tree(profile, flavor) if profile else None
    if flavor != "dev":
        runtime = Path(str(runtime) + "-" + flavor)
    env = {k: v for k, v in os.environ.items()
           if k not in ("SDL_VIDEODRIVER", "SDL_VIDEO_DRIVER", "LD_LIBRARY_PATH", "SteamAppId", "SteamGameId",
                        "WAFLOCK", "NO_LOCK_IN_TOP", "NO_LOCK_IN_RUN")}
    env.update({"LD_PRELOAD": str(build_shim()), "KILN_CAPTURE_FILE": str(capture),
                "KILN_CAPTURE_NAMES": "hl2_launcher:portal2.sh"})
    aliases = []
    if kind == "play":
        env.update({"BUILD": "0", "PLAY_BUILD_DIR": str(build), "RUNTIME": str(runtime),
                    "BASE_RUNTIME": str(MAIN_RUN / "runtime")})
        if flavor == "release":
            # ./play_release names build-release itself (PLAY_BUILD_DIR is
            # overwritten); point that name at kiln's release tree.
            link = ROOT / "build-release"
            if not link.exists() and not link.is_symlink():
                link.symlink_to(build.relative_to(ROOT), target_is_directory=True)
                aliases.append(link)
    elif kind == "p2":
        flavor_lock = ".lock-waf-kilneq-%s-%s" % (profile, flavor)
        aliases = lock_aliases(build, flavor_lock)
        # One reference runtime per mount selection: stage_portal2_runtime.py
        # never removes Workshop packs a run without --workshop no longer
        # mounts (a recorded defect kiln does not carry over: its packager
        # removes an unselected mount set's entries).
        runtime = Path(str(runtime) + "-" + profile + "".join("-" + m for m in mounts))
        if not mounts:
            shutil.rmtree(runtime / "workshop", ignore_errors=True)
        env.update({"P2_BUILD_DIR": str(build), "P2_RUNTIME": str(runtime),
                    "P2_WAFLOCK": flavor_lock, "P2_AV1_MEDIA": str(MAIN_RUN / "media-av1")})
    elif kind == "retail":
        pass  # ./play_p2 --retail execs Steam's portal2.sh before building anything
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



# The stub play_p2 the coop check puts beside a copy of play_p2_coop: it
# records its arguments; a host answers the engine's server challenge until
# the client has written its join line into the host's log.
COOP_STUB = r"""#!/usr/bin/env python3
import json, os, socket, sys, time
from pathlib import Path
root = Path(__file__).resolve().parent
args = sys.argv[1:]
with open(root / "calls.jsonl", "a") as out:
    out.write(json.dumps({"args": args, "no_build": os.environ.get("P2_NO_BUILD")}) + "\n")
log = root / "run/runtime-p2/engine.log"
if "+maxplayers" in args:
    port = int(args[args.index("-port") + 1])
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(("0.0.0.0", port))
    sock.settimeout(0.5)
    deadline = time.time() + 120
    # Lingers past the join: play_p2_coop checks the host still runs before
    # it reads the join line (every 5 s).
    stopped = None
    while time.time() < deadline and (stopped is None or time.time() < stopped + 12):
        if stopped is None and (root / "stop").exists():
            stopped = time.time()
        try:
            data, peer = sock.recvfrom(64)
        except OSError:
            continue
        if data[:5] == b"\xff\xff\xff\xffW":
            sock.sendto(b"\xff\xff\xff\xffA0000", peer)
else:
    log.parent.mkdir(parents=True, exist_ok=True)
    with open(log, "a") as out:
        out.write('Client "stub" connected (127.0.0.2:27005)\n')
    (root / "stop").touch()
    time.sleep(12)  # still running when play_p2_coop checks
"""


def coop_check(record, evidence):
    """./play_p2_coop's two play_p2 calls, each through play_p2's proven launch,
    against kiln's coop-pair peers."""
    scratch = SCRATCH / "coop"
    shutil.rmtree(scratch, ignore_errors=True)
    scratch.mkdir(parents=True)
    shutil.copy2(ROOT / "play_p2_coop", scratch / "play_p2_coop")
    for name in ("tools", "quality"):
        (scratch / name).symlink_to(ROOT / name, target_is_directory=True)
    (scratch / "play_p2").write_text(COOP_STUB)
    (scratch / "play_p2").chmod(0o755)
    port = 27315  # away from a real game's 27015
    result = subprocess.run([str(scratch / "play_p2_coop"), "--port", str(port)],
                            cwd=scratch, capture_output=True, text=True, timeout=180)
    calls = [json.loads(line) for line in (scratch / "calls.jsonl").read_text().splitlines()] \
        if (scratch / "calls.jsonl").is_file() else []
    if not record.check(result.returncode == 0 and len(calls) == 2,
                        "coop: ./play_p2_coop starts a host, then a client, and sees the join",
                        (result.stdout + result.stderr)[-600:]):
        return
    plan = json.loads(kiln("play", "portal2-coop", "--dry-run"))
    record.check([p["name"] for p in plan["peers"]] == ["host", "client"], "coop: kiln starts the host, then the client")
    record.check(calls[1]["no_build"] == "1" and calls[0]["no_build"] is None,
                 "coop: the client reuses the host's staged runtime (P2_NO_BUILD)")
    address = next((a.split(":")[0] for a in calls[1]["args"] if a.count(".") == 3 and ":" in a), "")

    def normalized(argv, host_port):
        out = []
        for argument in argv:
            argument = argument.replace(address, "{lan_address}")
            argument = argument.replace(str(host_port + 10), "{client_port}").replace(str(host_port), "{port}")
            out.append(argument)
        return out
    for index, peer in enumerate(plan["peers"]):
        # play_p2 with the arguments play_p2_coop gave it: Portal 2's launch,
        # already proven equal to ./play_p2's exec.
        expected = json.loads(kiln("play", "portal2", "--dry-run", "--", *calls[index]["args"]))["argv"]
        old_argv, kiln_argv = normalized(expected, port), normalized(peer["argv"], 27015)
        evidence["coop"][peer["name"]] = {"play_p2_args": calls[index]["args"], "old_argv": old_argv,
                                         "kiln_argv": kiln_argv}
        record.check(kiln_argv == old_argv, "coop: the %s's game argv equals ./play_p2_coop's" % peer["name"],
                     "first divergence: %r" % (next(((a, b) for a, b in zip(old_argv, kiln_argv) if a != b),
                                                    (len(old_argv), len(kiln_argv))),))
    # Seeded: a client without the LAN flag on the host must be caught.
    seeded = [a for a in plan["peers"][0]["argv"] if a not in ("+sv_lan",)]
    record.check(seeded != plan["peers"][0]["argv"], "coop: seeded difference caught: the host without +sv_lan")

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
    for mode in [m for m in selected if m in MODES]:
        argv, kiln_args, profile, kind, *rest = MODES[mode]
        extra_env = rest[0] if rest else {}
        flavor = flavor_of(kiln_args)
        if profile and (profile, flavor) not in built:
            kiln("build", kiln_args[0], "--flavor", flavor)
            built.add((profile, flavor))
        try:
            mounts = [kiln_args[i + 1] for i, arg in enumerate(kiln_args[:-1]) if arg == "--mounts"]
            capture, runtime, base = old_launch(mode, argv, profile, kind, extra_env, flavor, mounts)
        except RuntimeError as error:
            record.check(False, mode + ": the old launcher reaches the game exec", str(error)[-600:])
            continue
        captures[mode] = (capture, runtime, base)
        same_argv, same_env, same_cwd, detail = compare(mode, capture, runtime, base, kiln_args, evidence["modes"])
        record.check(same_argv, mode + ": kiln play --dry-run gives the old launcher's argv", detail)
        record.check(same_env, mode + ": and its environment", detail)
        record.check(same_cwd, mode + ": and its working directory")
        mounts = [kiln_args[i + 1] for i, arg in enumerate(kiln_args[:-1]) if arg == "--mounts"]
        key = "+".join([profile or "", flavor] + mounts)
        if profile and key not in packaged:
            kiln("package", kiln_args[0], "--flavor", flavor, *[x for m in mounts for x in ("--mounts", m)])
            packaged[key] = Path(json.loads(kiln("play", kiln_args[0], "--flavor", flavor,
                                                 "--dry-run"))["runtime"])
            old_count, new_count, problems = compare_manifests(runtime, packaged[key])
            evidence["manifests"][key] = {"old_entries": old_count, "kiln_entries": new_count,
                                          "differences": problems[:50]}
            record.check(not problems, mode + ": the kiln runtime has the old runtime's manifest (%d entries)"
                         % old_count, "; ".join(problems[:5]))
            if not problems:
                # Seeded: one changed byte in a copied file must be caught.
                victim = next((p for p in sorted(Path(packaged[key]).rglob("*.txt"))
                               if p.is_file() and not p.is_symlink()), None)
                if victim:
                    saved = victim.read_bytes()
                    victim.write_bytes(saved + b" ")
                    _, _, seeded = compare_manifests(runtime, packaged[key])
                    victim.write_bytes(saved)
                    record.check(bool(seeded), mode + ": seeded difference caught: a changed runtime file")
    if "coop" in selected:
        evidence["coop"] = {}
        coop_check(record, evidence)
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
    run.add_argument("--mode", action="append", choices=sorted(MODES) + ["coop"])
    run.add_argument("--keep", action="store_true")
    sub.add_parser("list")
    args = parser.parse_args(argv)
    if args.command == "list":
        for name, (argv_, kiln_args, *_rest) in MODES.items():
            print("%-22s %-60s kiln play %s" % (name, " ".join(argv_), " ".join(kiln_args)))
        return 0
    return check(args.mode or list(MODES) + ["coop"], args.keep)


if __name__ == "__main__":
    sys.exit(main())
