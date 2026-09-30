#!/usr/bin/env python3
"""Run the map pipeline's Cycles steps on another machine's GPU over SSH.

Every GPU-heavy step of the lighting back end (`pbrt_map_build.py`: the
baker seam's operations in `light_baker.py`, the lightmap bake, reflection
probes, probe volume, radiosity transfer and SDF volume) is one Blender
process reading and writing files by absolute path. With a `remote_blender`
block in the toolchain file, those steps run on the remote host instead, and
nothing else changes: the other steps, the step cache and the outputs stay
local. For each remote step:

  1. push: rsync mirrors the scripts (tools/quality, quality/), the build
     directory (with --delete, so the outputs the step set aside are gone
     there too) and every other input path to the SAME absolute paths on the
     host: scenes, receipts and USD stages name each other by absolute path;
  2. run: ssh runs the pinned Blender there with the step's arguments and
     environment; its output streams back into the local step log, with the
     usual progress lines and stall timeout;
  3. pull: rsync brings the build directory back (without deleting).

The host needs Linux, an NVIDIA driver (OptiX or CUDA), Blender at the
profile's pinned version (the official tarball is fine), rsync, SSH key
access, and write access to this checkout's absolute path
(`sudo mkdir -p <root> && sudo chown $USER <root>` once if the user differs).
It needs no OpenUSD, KTX, compile tools, game content or GPU drivers beyond
Blender's own.

    # add the block to a copy of the provisioned toolchain
    python3 tools/quality/remote_blender.py configure --host user@gpu-box \\
        --blender /opt/blender-5.2.2-linux-x64/blender \\
        --out build/toolchains/pbrt-map-toolchain-gpu.json
    # check the host (SSH, rsync, Blender version, root, Cycles GPU devices)
    python3 tools/quality/remote_blender.py check --toolchain build/toolchains/pbrt-map-toolchain-gpu.json --smoke
    # use it: any pipeline command that takes --toolchain, with the GPU device
    python3 tools/quality/portal2_gi_chamber.py --toolchain build/toolchains/pbrt-map-toolchain-gpu.json --device gpu

The block:

    "remote_blender": {
      "host": "user@gpu-box",               # ssh destination
      "blender": "/opt/blender/blender",    # the host's Blender
      "ssh": ["ssh", "-o", "BatchMode=yes"],   # optional transport
      "steps": ["bake", "probe", "probe-volume", "radiosity", "sdf", "render"],   # optional
      # optional: the post-bake Python steps on the host too (TOOL_STEPS), so
      # their large EXR outputs are made there instead of uploaded from here
      "python": "/opt/blender/5.2/python/bin/python3.13",
      "env": {"PYTHONPATH": "...", "LD_LIBRARY_PATH": "..."},
      "tools": {"openimagedenoise": {"version": "2.4.0", "sha256": "..."}}
    }

`render` is the reference renders run through `gi_reference.Tools.blender`
(`gi_reference.py`, `lighting_fixtures.py`, `gi_probes.py`). `vast_blender.py`
rents a vast.ai host and writes a toolchain with this block.

A step's cache key includes the remote Blender's version and binary digest,
not the host: the same pinned Blender on another (rented) host reuses the
cache, and a different Blender rebakes, as switching local Blender does.
"""

import argparse
import hashlib
import json
import re
import os
import shlex
import signal
import subprocess
import sys
import threading
import time
import uuid
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
sys.path.insert(0, str(HERE))
import light_baker  # noqa: E402

# The baker seam's operations: every light-transport product a map carries,
# and the reference renders (`gi_reference.Tools.blender`).
REFERENCE_RENDER = "render"
REMOTE_STEPS = tuple(light_baker.OPERATIONS) + (REFERENCE_RENDER,)
# Post-bake steps that are one Python script (numpy, scipy, imageio,
# OpenImageIO, OIDN): opt-in, with the block's `python`. ktx2 and the USD
# steps stay local (their tools are this host's builds, and ktx2 comes after
# the last remote step, so nothing it writes would be uploaded).
TOOL_STEPS = ("noise", "denoise", "directional", "rprb")
DEFAULT_SSH = ("ssh", "-o", "BatchMode=yes", "-o", "ServerAliveInterval=30")
# Repository trees the Blender scripts read besides their inputs: the scripts
# and their imports, and the export/product profiles some of them load.
SUPPORT_TREES = ("tools/quality", "quality")
EXCLUDES = ("__pycache__", ".previous", "logs")


class RemoteBlender:
    def __init__(self, config, root=ROOT):
        if not config.get("host") or not config.get("blender"):
            raise ValueError("remote_blender needs a host and a blender path")
        self.host = config["host"]
        self.blender = config["blender"]
        self.ssh = list(config.get("ssh") or DEFAULT_SSH)
        self.steps = tuple(config.get("steps") or REMOTE_STEPS)
        unknown = set(self.steps) - set(REMOTE_STEPS) - set(TOOL_STEPS) - {"stage"}
        if unknown:
            raise ValueError("remote_blender steps must be Blender or tool steps, not %s"
                             % sorted(unknown))
        self.python = config.get("python")
        self.env = dict(config.get("env") or {})
        self.tools = dict(config.get("tools") or {})
        if set(self.steps) & set(TOOL_STEPS) and not self.python:
            raise ValueError("remote_blender tool steps %s need the host's `python`"
                             % sorted(set(self.steps) & set(TOOL_STEPS)))
        self.root = Path(root)
        self._identity = None

    def applies(self, step):
        return step in self.steps

    # ------------------------------------------------------------ transport
    def remote(self, command, capture=True, check=True):
        """Run a shell command on the host."""
        return subprocess.run(self.ssh + [self.host, command], capture_output=capture, text=True,
                              check=check)

    def rsync(self, sources, destination, delete=False, relative=True, update=False):
        command = ["rsync", "-a", "-e", " ".join(shlex.quote(p) for p in self.ssh)]
        command += ["--exclude=" + e for e in EXCLUDES]
        if relative:
            command.append("--relative")
        if delete:
            command.append("--delete")
        if update:
            command.append("--update")
        subprocess.run(command + [str(s) for s in sources] + [destination], check=True,
                       capture_output=True, text=True)

    def push(self, paths, mirror):
        """Mirror the support trees, `mirror` (with deletion) and `paths` to the host."""
        support = [self.root / t for t in SUPPORT_TREES]
        self.rsync([str(p) + "/" if p.is_dir() else p for p in support], self.host + ":/")
        self.rsync([str(mirror) + "/"], self.host + ":/", delete=True)
        extra = sorted({str(Path(p)) for p in paths
                        if Path(p).exists() and not _inside(p, [mirror] + support)})
        if extra:
            self.rsync(extra, self.host + ":/")

    def pull(self, mirror, update=False):
        """Bring `mirror` back; `update` keeps local files newer than the host's
        (a log appended here while the host ran)."""
        self.rsync(["%s:%s/" % (self.host, mirror)], str(mirror) + "/", relative=False,
                   update=update)

    def command(self, arguments, env):
        """The local command that runs Blender with `arguments` on the host."""
        exports = " ".join("%s=%s" % (k, shlex.quote(str(v))) for k, v in sorted(env.items()))
        line = "cd %s && env %s %s %s" % (
            shlex.quote(str(self.root)), exports, shlex.quote(self.blender),
            " ".join(shlex.quote(str(a)) for a in arguments))
        return self.detached(line)

    def detached(self, line):
        """The local command that runs shell `line` on the host as a detached
        job (`execute`): it survives a dropped connection, and its output and
        exit status come back here."""
        return [sys.executable, str(Path(__file__).resolve()), "exec",
                "--ssh", json.dumps(self.ssh), "--host", self.host, "--", line]

    def tool_command(self, arguments, env):
        """The local command that runs a Python tool step (`arguments` after
        the interpreter) on the host with its `python` and `env`."""
        merged = dict(self.env, **env)
        exports = " ".join("%s=%s" % (k, shlex.quote(str(v))) for k, v in sorted(merged.items()))
        line = "cd %s && env %s %s %s" % (
            shlex.quote(str(self.root)), exports, shlex.quote(self.python),
            " ".join(shlex.quote(str(a)) for a in arguments))
        return self.detached(line)

    def tool_identity(self, name):
        """A pinned tool's identity on the host (the block's `tools`), or None."""
        identity = self.tools.get(name)
        return dict(identity, remote=True) if identity else None

    # ------------------------------------------------------------- identity
    def identity(self):
        """Host, Blender version and binary digest, for step cache keys."""
        if self._identity is None:
            blender = shlex.quote(self.blender)
            result = self.remote("%s --version | head -1; sha256sum \"$(readlink -f %s)\""
                                 % (blender, blender))
            lines = result.stdout.splitlines()
            version = re.search(r"^Blender (\S+)", lines[0] if lines else "")
            self._identity = {"host": self.host,
                              "version": version.group(1) if version else "unknown",
                              "sha256": lines[-1].split()[0] if lines else "unknown"}
        return self._identity

    def cache_identity(self):
        """The part of `identity` a step's cache key names: the Blender, not
        the host running it (each rented host has a new address)."""
        identity = self.identity()
        return {"version": identity["version"], "sha256": identity["sha256"]}


# ============================================================ detached jobs
# A remote step runs as a job on the host, not as the ssh session's child: a
# dropped connection (seen on a rented host mid-radiosity, leaving the Blender
# running as an orphan) no longer fails the step, nor does this machine
# sleeping: the client streams the job's output from a byte offset and
# reconnects after a drop or a resume (RECONNECT_S counts only time awake).
# A job whose command line matches a live one (the same step, the same
# inputs) replaces it, so a rerun never overlaps its own orphan. The client
# touches a heartbeat, and the host's watchdog kills a job whose client has
# been gone for LEASE_S: long enough for a laptop to sleep through a bake
# (a 15-minute lease killed testchmb_a_15's radiosity while this machine
# slept), short of a rental's reaper deadline.
JOBS = "/tmp/remote-blender"
HEARTBEAT_S = 60
LEASE_S = 6 * 3600
RECONNECT_S = 600


def start_script(directory, line):
    """The host script that starts `line` as a detached job with its watchdog,
    after stopping any live job with the same command line."""
    key = hashlib.sha256(line.encode()).hexdigest()[:16]
    job = "(%s); echo $? > %s/rc" % (line, directory)
    watchdog = ("job=$(cat {d}/pid); while kill -0 $job 2>/dev/null; do "
                "if [ $(( $(date +%s) - $(stat -c %Y {d}/hb) )) -gt {lease} ]; then "
                "kill -TERM -$job; sleep 10; kill -KILL -$job 2>/dev/null; fi; sleep 30; done"
                ).format(d=directory, lease=LEASE_S)
    replace = ("for d in {jobs}/*/; do if [ \"$(cat $d/key 2>/dev/null)\" = {key} ] && "
               "[ ! -f $d/rc ] && kill -0 $(cat $d/pid) 2>/dev/null; then "
               "echo replacing $d; kill -TERM -$(cat $d/pid); fi; done").format(jobs=JOBS, key=key)
    return "\n".join([
        replace,
        "set -e",
        "mkdir -p %s && cd %s && touch hb && echo %s > key" % (directory, directory, key),
        "nohup setsid bash -c %s > out 2>&1 < /dev/null &" % shlex.quote(job),
        "echo $! > pid",
        "nohup setsid bash -c %s > /dev/null 2>&1 < /dev/null &" % shlex.quote(watchdog),
        "echo STARTED",
    ])


def execute(ssh, host, line, stream=None, sleep=time.sleep):
    """Run `line` on the host as a detached job; stream its output to
    `stream` (stdout's bytes); return its exit status."""
    stream = stream or sys.stdout.buffer
    if stream is sys.stdout.buffer:
        # ssh switches the descriptors it inherits to non-blocking; ours
        # share their open file with the pipeline's log pipe.
        os.set_blocking(sys.stdout.fileno(), True)
    directory = "%s/%s" % (JOBS, uuid.uuid4().hex)
    started = subprocess.run(ssh + [host, "bash -s"], input=start_script(directory, line),
                             capture_output=True, text=True)
    if "STARTED" not in started.stdout:
        stream.write(("remote job did not start: %s\n" % started.stderr.strip()).encode())
        return started.returncode or 1
    stop = threading.Event()

    def heartbeat():
        while not stop.wait(HEARTBEAT_S):
            subprocess.run(ssh + [host, "touch %s/hb" % directory], capture_output=True,
                           stdin=subprocess.DEVNULL)

    def cancel(*_):
        subprocess.run(ssh + [host, "kill -TERM -$(cat %s/pid) 2>/dev/null" % directory],
                       capture_output=True, stdin=subprocess.DEVNULL)
        raise SystemExit(143)

    threading.Thread(target=heartbeat, daemon=True).start()
    previous = {sig: signal.signal(sig, cancel) for sig in (signal.SIGTERM, signal.SIGINT)}
    offset, contact = 0, time.monotonic()
    try:
        while True:
            tail = subprocess.Popen(ssh + [host, "tail -c +%d --pid=$(cat %s/pid) -f %s/out"
                                           % (offset + 1, directory, directory)],
                                    stdin=subprocess.DEVNULL, stdout=subprocess.PIPE,
                                    stderr=subprocess.DEVNULL)
            for chunk in iter(lambda: tail.stdout.read1(65536), b""):
                stream.write(chunk)
                stream.flush()
                offset += len(chunk)
                contact = time.monotonic()
            if tail.wait() == 0:
                break
            if time.monotonic() - contact > RECONNECT_S:
                stream.write(b"lost the host for %d s; the job ends at its lease\n" % RECONNECT_S)
                return 255
            stream.write(b"[remote_blender] connection lost; reconnecting\n")
            sleep(10)
        for _ in range(10):
            status = subprocess.run(ssh + [host, "cat %s/rc" % directory],
                                    capture_output=True, text=True, stdin=subprocess.DEVNULL)
            if status.returncode == 0 and status.stdout.strip():
                return int(status.stdout.strip())
            sleep(3)
        stream.write(b"remote job left no exit status (killed?)\n")
        return 1
    finally:
        stop.set()
        for sig, handler in previous.items():
            signal.signal(sig, handler)


def _inside(path, roots):
    path = Path(path).resolve()
    return any(path == Path(r).resolve() or Path(r).resolve() in path.parents for r in roots)


def from_toolchain(toolchain, root=ROOT):
    config = toolchain.get("remote_blender")
    return RemoteBlender(config, root) if config else None


# ================================================================== check
DEVICE_PROBE = """
import bpy
prefs = bpy.context.preferences.addons["cycles"].preferences
for backend in ("OPTIX", "CUDA", "HIP", "ONEAPI"):
    try:
        prefs.compute_device_type = backend
    except TypeError:
        continue
    prefs.get_devices()
    for device in prefs.devices:
        if device.type == backend:
            print("CYCLES_DEVICE %s %s" % (backend, device.name))
"""

SMOKE = """
import bpy, time
scene = bpy.context.scene
scene.render.engine = "CYCLES"
prefs = bpy.context.preferences.addons["cycles"].preferences
for backend in ("OPTIX", "CUDA"):
    try:
        prefs.compute_device_type = backend
    except TypeError:
        continue
    prefs.get_devices()
    gpus = [d for d in prefs.devices if d.type == backend]
    if gpus:
        for d in prefs.devices:
            d.use = d.type == backend
        scene.cycles.device = "GPU"
        break
scene.cycles.samples = 64
scene.render.resolution_x = scene.render.resolution_y = 256
scene.render.filepath = "/tmp/remote_blender_smoke.png"
started = time.time()
bpy.ops.render.render(write_still=True)
print("SMOKE_RENDER %s %.1fs" % (scene.cycles.device, time.time() - started))
"""


def check(remote, pinned, smoke=False):
    """Facts about the host and the problems that stop it baking."""
    facts, problems = {}, []
    try:
        remote.remote("true")
    except subprocess.CalledProcessError as error:
        return facts, ["ssh %s failed: %s" % (remote.host, (error.stderr or "").strip()[-300:])]
    for tool in ("rsync", "nvidia-smi"):
        found = remote.remote("command -v %s" % tool, check=False)
        if found.returncode:
            (problems if tool == "rsync" else facts.setdefault("notes", [])).append(
                "%s not found on the host" % tool)
    gpu = remote.remote("nvidia-smi --query-gpu=name,driver_version,memory.total "
                        "--format=csv,noheader", check=False)
    facts["nvidia_smi"] = gpu.stdout.strip()
    identity = remote.identity()
    facts["blender"] = identity
    if identity["version"] != pinned:
        problems.append("blender on the host is %s; the profile pins %s"
                        % (identity["version"], pinned))
    root = shlex.quote(str(remote.root))
    writable = remote.remote("mkdir -p %s && test -w %s" % (root, root), check=False)
    if writable.returncode:
        problems.append("%s is not writable on the host; run there once: sudo mkdir -p %s && "
                        "sudo chown $USER %s" % (remote.root, root, root))
    devices = remote.remote("%s -b --factory-startup --python-expr %s 2>&1"
                            % (shlex.quote(remote.blender), shlex.quote(DEVICE_PROBE)),
                            check=False)
    facts["cycles_devices"] = re.findall(r"^CYCLES_DEVICE (\S+) (.+)$", devices.stdout, re.M)
    if not facts["cycles_devices"]:
        problems.append("Blender on the host sees no Cycles GPU device")
    if smoke and not problems:
        render = remote.remote("%s -b --factory-startup --python-expr %s 2>&1"
                               % (shlex.quote(remote.blender), shlex.quote(SMOKE)), check=False)
        match = re.search(r"^SMOKE_RENDER (\S+) (\S+)$", render.stdout, re.M)
        facts["smoke"] = match.groups() if match else render.stdout.strip()[-400:]
        if not match or match.group(1) != "GPU":
            problems.append("the GPU smoke render did not run on the GPU")
    return facts, problems


def pinned_blender():
    profile = json.loads((ROOT / "quality/product_profiles/pbrt-map-linux-tools.json").read_text())
    return profile["host_packages"]["blender"]["version"]


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    commands = parser.add_subparsers(dest="command", required=True)
    configure = commands.add_parser("configure", help="write a toolchain with a remote_blender")
    configure.add_argument("--host", required=True)
    configure.add_argument("--blender", required=True, help="Blender's path on the host")
    configure.add_argument("--ssh", help="ssh command, e.g. 'ssh -p 2222 -o BatchMode=yes'")
    configure.add_argument("--toolchain", type=Path,
                           default=ROOT / "build/toolchains/pbrt-map-toolchain.json")
    configure.add_argument("--out", type=Path,
                           default=ROOT / "build/toolchains/pbrt-map-toolchain-gpu.json")
    checker = commands.add_parser("check", help="check the host a toolchain names")
    checker.add_argument("--toolchain", type=Path,
                         default=ROOT / "build/toolchains/pbrt-map-toolchain-gpu.json")
    checker.add_argument("--smoke", action="store_true", help="also render on the host's GPU")
    runner = commands.add_parser("exec", help=argparse.SUPPRESS)
    runner.add_argument("--ssh", required=True, help="the ssh command, as JSON")
    runner.add_argument("--host", required=True)
    runner.add_argument("line")
    args = parser.parse_args()
    if args.command == "exec":
        return execute(json.loads(args.ssh), args.host, args.line)
    if args.command == "configure":
        toolchain = json.loads(args.toolchain.read_text())
        block = {"host": args.host, "blender": args.blender}
        if args.ssh:
            block["ssh"] = shlex.split(args.ssh)
        toolchain["remote_blender"] = block
        args.out.write_text(json.dumps(toolchain, indent=2) + "\n")
        print("wrote " + str(args.out))
        return 0
    remote = from_toolchain(json.loads(args.toolchain.read_text()))
    if remote is None:
        parser.error("%s has no remote_blender block" % args.toolchain)
    facts, problems = check(remote, pinned_blender(), args.smoke)
    print(json.dumps({"host": remote.host, "facts": facts, "problems": problems}, indent=2))
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main())
