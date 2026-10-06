#!/usr/bin/env python3
"""Run the map pipeline's Cycles steps on another machine's GPU over SSH.

Every GPU-heavy step of the lighting back end (`pbrt_map_build.py`: the
baker seam's operations in `light_baker.py`, the lightmap bake, reflection
probes, probe volume, radiosity transfer and SDF volume) is one Blender
process reading and writing files by absolute path. With a `remote_blender`
block in the toolchain file, those steps run on the remote host instead, and
nothing else changes: the other steps, the step cache and the outputs stay
local. For each remote step:

  1. push: rsync mirrors the scripts (tools/quality, tools/texture, quality/), the build
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

`provision` installs that host instead of asking for it: it reads the pinned
host profile (`quality/remote_hosts/pbrt-map-remote-host.json`), works out
which prefix the host can use (`/opt`, or `~/.local/opt` when the root
filesystem is read-only, as on an immutable or atomic image such as Bazzite,
Silverblue or SteamOS), fetches every pinned archive by digest, installs the
post-bake Python packages with the host's own Blender Python, writes the
toolchain and then reports what it found:

    python3 tools/quality/remote_blender.py provision --host bazzite@192.168.0.13 \\
        --out build/toolchains/pbrt-map-toolchain-bazzite.json
    python3 tools/quality/remote_blender.py check --toolchain build/toolchains/pbrt-map-toolchain-bazzite.json --smoke

The pinned archives are ordinary files: a prefix you own installs them
unchanged, with no package manager and no root. Packages are installed only
when the host is actually missing `rsync`, `curl`, `tar`, `xz` or `sha256sum`,
and an unknown distribution is refused by name rather than guessed at.

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
SUPPORT_TREES = ("tools/quality", "tools/texture", "quality")
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

    def rsync(self, sources, destination, delete=False, update=False, cwd=None, relative=True):
        # --relative keeps each source at the path it is named with, so a
        # relative source run from the checkout lands at the same absolute path
        # on the host; a plain rsync would drop the leading directories. It is
        # off when pulling, whose destination already IS that path.
        command = ["rsync", "-a", "-e", " ".join(shlex.quote(p) for p in self.ssh)]
        if relative:
            command.append("--relative")
        command += ["--exclude=" + e for e in EXCLUDES]
        if delete:
            command.append("--delete")
        if update:
            command.append("--update")
        # the output is the remote side's stderr: rsync says which path it could
        # not read, and a read-only host prefix otherwise fails as exit 23 alone
        subprocess.run(command + [str(s) for s in sources] + [destination], check=True,
                       text=True, cwd=cwd)

    def push(self, paths, mirror):
        """Mirror the support trees, `mirror` (with deletion) and `paths` to the
        host, each at the SAME absolute path it has here.

        The paths are relative to this checkout and the destination is the host's
        copy of it, rather than absolute paths into the host's `/`: an atomic or
        immutable host has a read-only root, and the checkout is the one place
        on it that a remote step can write."""
        support = [self.root / t for t in SUPPORT_TREES]
        destination = "%s:%s/" % (self.host, self.root)
        # no trailing slash on a source: rsync reads that as "the contents of",
        # which would land each tree in the checkout root instead of at its own
        # path (and let the mirror's --delete erase it again)
        self.rsync([self._relative(p) for p in support], destination, cwd=self.root)
        self.rsync([self._relative(mirror)], destination, delete=True, cwd=self.root)
        extra = sorted({str(Path(p)) for p in paths
                        if Path(p).exists() and not _inside(p, [mirror] + support)})
        if extra:
            outside = [p for p in extra if not _inside(p, [self.root])]
            if outside:
                raise ValueError("remote steps read and write paths inside %s, but %s are "
                                 "outside it; there is no place on the host to mirror them to"
                                 % (self.root, outside))
            self.rsync(extra, destination, cwd=self.root)

    def _relative(self, path):
        return str(Path(path).resolve().relative_to(self.root.resolve()))

    def pull(self, mirror, update=False):
        """Bring `mirror` back; `update` keeps local files newer than the host's
        (a log appended here while the host ran)."""
        self.rsync(["%s:%s/" % (self.host, mirror)], str(mirror) + "/", update=update,
                   relative=False)

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
    if remote.python:
        # the tool steps' own interpreter and environment, which the block names
        facts["tool_steps"] = {}
        for name, command in (("python", "test -x %s" % shlex.quote(remote.python)),
                              ("python_packages", "test -d %s" % shlex.quote(
                                  remote.env.get("PYTHONPATH", ""))),
                              ("denoiser", "test -e %s" % shlex.quote(
                                  str(Path(remote.env.get("LD_LIBRARY_PATH", "")) /
                                      "libOpenImageDenoise.so.2"))),
                              ("exr_codec", "test -e %s" % shlex.quote(
                                  remote.env.get("IMAGEIO_FREEIMAGE_LIB", "")))):
            present = remote.remote(command, check=False)
            facts["tool_steps"][name] = not present.returncode
            if present.returncode:
                problems.append("the host's %s is missing; run `remote_blender.py provision` "
                                "against it, or drop the tool steps from the block" % name)
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


# ============================================================== provisioning
# What a remote host installs, pinned by digest: `quality/remote_hosts/
# pbrt-map-remote-host.json` owns the versions, URLs and digests; this module
# owns installing them and recording where they landed. `vast_blender.py` rents
# a Debian host and calls the same code.
HOST_PROFILE = ROOT / "quality/remote_hosts/pbrt-map-remote-host.json"
HOST = json.loads(HOST_PROFILE.read_text())
# The profile's canonical install prefix, and the fallback for a host whose
# root filesystem is not writable without root (an immutable or atomic image:
# Bazzite, Silverblue, SteamOS). The pinned archives are ordinary files, so a
# user-owned prefix installs them unchanged and needs no package manager and no
# sudo.
CANONICAL_BASE = "/opt"
USER_BASE = ".local/opt"
# What a Debian host needs from apt for Blender's X11/GL libraries; a host that
# already has these (or cannot install anything without a layer) is reported
# rather than assumed.
BLENDER_LIBS = ("libx11-6 libxi6 libxxf86vm1 libxfixes3 libxrender1 libxext6 libxkbcommon0 "
                "libsm6 libice6 libgl1 libegl1 libglu1-mesa")
BASE_TOOLS = ("rsync", "curl", "tar", "xz", "sha256sum")


def relayout(package, base):
    """`package` with its install paths moved from the canonical prefix to `base`."""
    moved = dict(package)
    for key in ("directory", "path"):
        if key in moved:
            moved[key] = base + moved[key][len(CANONICAL_BASE):]
    return moved


def paths(base):
    """Every path the remote steps need on a host installed under `base`."""
    blender = relayout(HOST["blender"], base)
    oidn = relayout(HOST["oidn"], base)
    freeimage = relayout(HOST["freeimage"], base)
    directory = base + HOST["blender"]["directory"][len(CANONICAL_BASE):]
    return {"base": base,
            "blender": directory + "/" + HOST["blender"]["executable"],
            "python": directory + "/" + HOST["blender"]["python"],
            "python_target": base + HOST["python_packages"]["target"][len(CANONICAL_BASE):],
            "oidn_lib_dir": oidn["directory"] + "/" + str(Path(oidn["library"]).parent),
            "freeimage": freeimage["path"]}


def tool_env(base):
    """The environment of the Python tool steps on a host installed under `base`."""
    installed = paths(base)
    return {"PYTHONPATH": installed["python_target"],
            "LD_LIBRARY_PATH": installed["oidn_lib_dir"],
            "IMAGEIO_FREEIMAGE_LIB": installed["freeimage"]}


def fetch_pinned(package, archive):
    """Shell lines that download `package` into the canonical prefix, verify its
    digest and unpack it, unless its directory exists. A dropped transfer
    resumes (-C -); a bad or stuck source moves to the next URL."""
    return [
        "if [ ! -d %s ]; then" % shlex.quote(package["directory"]),
        "  cd %s" % shlex.quote(str(Path(package["directory"]).parent)),
        "  for url in %s; do" % " ".join(shlex.quote(u) for u in package["urls"]),
        "    for try in 1 2 3; do",
        "      curl -fsSL --retry 3 --retry-all-errors --speed-limit 1000000 --speed-time 30 "
        "-C - -o %s \"$url\" && break" % archive,
        "    done",
        "    echo '%s  %s' | sha256sum -c --quiet - && break" % (package["sha256"], archive),
        "    rm -f %s" % archive,
        "  done",
        "  test -f %s" % archive,
        "  tar xf %s && rm %s" % (archive, archive),
        "fi",
    ]


def fetch_pinned_file(package):
    """Shell lines that download one pinned file to its `path` and verify it."""
    path = shlex.quote(package["path"])
    return [
        "if ! echo '%s  %s' | sha256sum -c --quiet - >/dev/null 2>&1; then"
        % (package["sha256"], package["path"]),
        "  mkdir -p %s" % shlex.quote(str(Path(package["path"]).parent)),
        "  for url in %s; do" % " ".join(shlex.quote(u) for u in package["urls"]),
        "    curl -fsSL --retry 3 --retry-all-errors -o %s \"$url\" && "
        "echo '%s  %s' | sha256sum -c --quiet - && break" % (path, package["sha256"],
                                                              package["path"]),
        "    rm -f %s" % path,
        "  done",
        "  test -f %s" % path,
        "fi",
    ]


def package_lines(facts, distro):
    """Shell lines that install what the host is missing, or nothing when it is
    an immutable image that already carries the tools (the pinned archives
    themselves never need a package manager)."""
    missing = [tool for tool in BASE_TOOLS if tool not in facts.get("tools", {})]
    if not missing:
        return []
    if distro in ("debian", "ubuntu"):
        return ["export DEBIAN_FRONTEND=noninteractive",
                "apt-get update -qq",
                "apt-get install -y -qq --no-install-recommends %s >/dev/null"
                % " ".join(missing + ["ca-certificates"] + list(BLENDER_LIBS))]
    if distro in ("fedora", "rhel", "centos"):
        return ["dnf install -y -q %s >/dev/null" % " ".join(missing)]
    raise ValueError("no package manager is known for %r; install %s by hand, or point the "
                     "toolchain at paths that already exist" % (distro, " ".join(missing)))


def requirements(base):
    return "\n".join("%s==%s --hash=sha256:%s" % (wheel["name"], wheel["version"], wheel["sha256"])
                     for wheel in HOST["python_packages"]["wheels"])


TOOLS_IMPORT = ("import ctypes, numpy, scipy, imageio.v3 as iio, OpenImageIO, PIL; "
                "ctypes.CDLL('libOpenImageDenoise.so.2'); "
                "iio.imwrite('/tmp/check.exr', numpy.ones((4, 4, 4), numpy.float32)); "
                "assert iio.imread('/tmp/check.exr').shape == (4, 4, 4); "
                "print('TOOLS', numpy.__version__)")


def provision_script(base, distro, facts, tools=True):
    """The shell a remote host runs to install the pinned Blender, OIDN, imageio
    EXR codec and (with `tools`) the post-bake Python packages under `base`."""
    installed = paths(base)
    lines = ["set -eu", "mkdir -p %s" % shlex.quote(base)]
    lines += package_lines(facts, distro)
    lines += fetch_pinned(relayout(HOST["blender"], base), "blender.tar.xz")
    python = shlex.quote(installed["python"])
    if tools:
        lines += fetch_pinned(relayout(HOST["oidn"], base), "oidn.tar.gz")
        lines += fetch_pinned_file(relayout(HOST["freeimage"], base))
        wanted = shlex.quote(base + "/source-python.txt.new")
        lines += ["cat > %s <<'REQUIREMENTS'\n%s\nREQUIREMENTS" % (wanted, requirements(base)),
                  "%s -m pip --version >/dev/null 2>&1 || %s -m ensurepip --default-pip >/dev/null"
                  % (python, python),
                  "if ! cmp -s %s %s; then" % (wanted, shlex.quote(base + "/source-python.txt")),
                  "  %s -m pip install --quiet --disable-pip-version-check "
                  "--root-user-action=ignore --no-deps --only-binary :all: --require-hashes "
                  "--upgrade --target %s -r %s" % (python, shlex.quote(installed["python_target"]),
                                                   wanted),
                  "  mv %s %s" % (wanted, shlex.quote(base + "/source-python.txt")),
                  "fi",
                  "env %s %s -c %s" % (" ".join("%s=%s" % item for item in sorted(tool_env(base).items())),
                                       python, shlex.quote(TOOLS_IMPORT))]
    lines += [shlex.quote(installed["blender"]) + " --version | head -1", "echo PROVISIONED"]
    return "\n".join(lines) + "\n"



def survey(remote):
    """What a candidate host is: its distro, the tools it has, and which install
    prefix it can use. Nothing here needs root."""
    facts = {}
    release = remote.remote("cat /etc/os-release 2>/dev/null || true", check=False)
    facts["os"] = dict(re.findall(r"^([A-Z_]+)=(.*)$", release.stdout, re.M))
    facts["distro"] = facts["os"].get("ID", "unknown")
    facts["tools"] = {}
    for tool in BASE_TOOLS + ("nvidia-smi",):
        facts["tools"][tool] = bool(remote.remote("command -v %s" % tool, check=False).returncode == 0)
    canonical = remote.remote("test -w %s" % CANONICAL_BASE, check=False)
    facts["base"] = CANONICAL_BASE if canonical.returncode == 0 else "$HOME/" + USER_BASE
    facts["base_writable"] = canonical.returncode == 0
    facts["home"] = remote.remote("printf %s \"$HOME\"", check=False).stdout.strip()
    # a derivative image (Bazzite, Silverblue, SteamOS) carries its parent's ID
    # only in ID_LIKE; an unknown name is refused by name rather than guessed at
    facts["distro"] = facts["os"].get("ID", "unknown")
    if facts["distro"] not in ("debian", "ubuntu", "fedora", "rhel", "centos"):
        for parent in re.findall(r'"([^"]+)"', facts["os"].get("ID_LIKE", "")):
            if parent in ("debian", "ubuntu", "fedora", "rhel", "centos"):
                facts["distro"] = parent
                break
    return facts


def provision(remote, base=None, tools=True):
    """Install the pinned host packages on `remote` and return (facts, problems)."""
    facts = survey(remote)
    problems = []
    install = base or facts["base"]
    if install.startswith("$HOME"):
        install = facts["home"] + install[len("$HOME"):]
    if not facts["base_writable"] and not install.startswith(facts["home"]):
        problems.append("%s is not writable and %s is not under your home; pass --base with a "
                        "directory you own" % (CANONICAL_BASE, install))
    if problems:
        return facts, problems
    facts["installed_at"] = install
    if not facts["tools"].get("nvidia-smi"):
        facts["notes"] = ["nvidia-smi not found on the host; Cycles will run on the CPU"]
    script = provision_script(install, facts["distro"], facts, tools)
    result = subprocess.run(remote.ssh + [remote.host, "bash -s"], input=script,
                            capture_output=True, text=True)
    facts["provision"] = (result.stdout or "").strip().splitlines()
    if result.returncode:
        problems.append("provisioning failed: %s"
                        % (result.stderr or result.stdout).strip()[-400:])
        return facts, problems
    # the steps mirror this checkout to the SAME absolute path on the host, so
    # that path has to exist and be writable there, by this user or by root
    root = shlex.quote(str(remote.root))
    mirror = remote.remote("mkdir -p %s && test -w %s" % (root, root), check=False)
    facts["mirror_root"] = str(remote.root)
    if mirror.returncode:
        problems.append("%s is not writable on the host; run there once: sudo mkdir -p %s && "
                        "sudo chown $USER %s" % (remote.root, root, root))
    return facts, problems


def block_for(host, base, ssh=None, blender=None, tools=True):
    """The `remote_blender` block for a host provisioned under `base`: the
    pinned Blender there, and (with `tools`) the host's Python, its environment
    and the pinned tools the post-bake steps name in their cache keys."""
    installed = paths(base)
    block = {"host": host, "blender": blender or installed["blender"]}
    if ssh:
        block["ssh"] = shlex.split(ssh)
    block["steps"] = list(REMOTE_STEPS) + (list(TOOL_STEPS) if tools else [])
    if tools:
        block["python"] = installed["python"]
        block["env"] = tool_env(base)
        block["tools"] = {"openimagedenoise": {"version": HOST["oidn"]["version"],
                                              "sha256": HOST["oidn"]["sha256"]}}
    return block


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    commands = parser.add_subparsers(dest="command", required=True)
    configure = commands.add_parser("configure", help="write a toolchain with a remote_blender")
    configure.add_argument("--host", required=True)
    configure.add_argument("--blender", help="Blender's path on the host (default: the pinned one)")
    configure.add_argument("--ssh", help="ssh command, e.g. 'ssh -p 2222 -o BatchMode=yes'")
    configure.add_argument("--base", help="where the host installed the pinned packages "
                                         "(default: /opt, or ~/.local/opt when /opt is read-only)")
    configure.add_argument("--toolchain", type=Path,
                           default=ROOT / "build/toolchains/pbrt-map-toolchain.json")
    configure.add_argument("--out", type=Path,
                           default=ROOT / "build/toolchains/pbrt-map-toolchain-gpu.json")
    installer = commands.add_parser("provision",
                                    help="install the pinned Blender and tools on a remote host")
    installer.add_argument("--host", required=True, help="ssh destination, e.g. bazzite@192.168.0.13")
    installer.add_argument("--ssh", help="ssh command")
    installer.add_argument("--base", help="install prefix (default: the first writable of /opt, "
                                          "~/.local/opt)")
    installer.add_argument("--no-tools", action="store_true",
                           help="install Blender only, leaving the Python steps local")
    installer.add_argument("--toolchain", type=Path,
                           default=ROOT / "build/toolchains/pbrt-map-toolchain.json")
    installer.add_argument("--out", type=Path,
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
        block = block_for(args.host, args.base or CANONICAL_BASE, args.ssh, args.blender)
        toolchain = json.loads(args.toolchain.read_text())
        toolchain["remote_blender"] = block
        args.out.write_text(json.dumps(toolchain, indent=2) + "\n")
        print("wrote " + str(args.out))
        return 0
    if args.command == "provision":
        remote = RemoteBlender(block_for(args.host, CANONICAL_BASE, args.ssh))
        facts, problems = provision(remote, args.base, not args.no_tools)
        if "installed_at" in facts:
            block = block_for(args.host, facts["installed_at"], args.ssh, tools=not args.no_tools)
            toolchain = json.loads(args.toolchain.read_text())
            toolchain["remote_blender"] = block
            args.out.write_text(json.dumps(toolchain, indent=2) + "\n")
            facts["wrote"] = str(args.out)
        print(json.dumps({"host": args.host, "facts": facts, "problems": problems}, indent=2))
        return 1 if problems else 0
    remote = from_toolchain(json.loads(args.toolchain.read_text()))
    if remote is None:
        parser.error("%s has no remote_blender block" % args.toolchain)
    facts, problems = check(remote, pinned_blender(), args.smoke)
    print(json.dumps({"host": remote.host, "facts": facts, "problems": problems}, indent=2))
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main())
