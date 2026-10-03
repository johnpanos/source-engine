#!/usr/bin/env python3
"""Rent a vast.ai GPU host for the Blender renders, run them, and give it back.

`remote_blender.py` runs Blender steps on another Linux host over SSH when
the toolchain carries a `remote_blender` block. This tool supplies that host
from vast.ai, under a hard spending cap:

    # rent a host, run N render commands against it, destroy it (always)
    python3 tools/quality/vast_blender.py run \\
        --render "python3 tools/quality/lighting_fixtures.py render --fixture cornell-floors --device gpu" \\
        --render "python3 tools/quality/gi_reference.py render --fixture box --device gpu"

    # or by hand
    python3 tools/quality/vast_blender.py up          # prints the toolchain it wrote
    python3 tools/quality/portal2_gi_chamber.py --toolchain build/toolchains/pbrt-map-toolchain-vast.json --device gpu
    python3 tools/quality/vast_blender.py down
    python3 tools/quality/vast_blender.py status      # instances, credit, spend against the cap

Each `--render` command runs from the repository root with
`--toolchain <the vast toolchain>` appended, or substituted for `{toolchain}`
when the command names it. Pass `--device gpu` to the render tools: their
default device is the CPU (`cycles_device.py`).

Up: search offers (one verified NVIDIA RTX-class GPU, cheapest first, direct
SSH, cheap transfer), rent one with this tool's SSH key, install the profile's
pinned Blender from its pinned tarball digest, check the host with
`remote_blender.check --smoke`, and write the toolchain file.

Spending cap: the ledger (`$XDG_STATE_HOME/source-engine/vast-ledger.json`)
holds the cap (default $10) and every instance this tool rented. Spend is the
larger of the ledger's estimate (each instance's hourly rate, storage
included, over its lifetime, plus a transfer allowance) and the account
credit billed since the ledger began. A rental is refused unless spend plus
its worst case (rate x `--hours` plus transfer) fits under the cap. Every
rental gets a detached reaper that destroys it at its deadline (the earlier
of `--hours` and the cap) even if this process dies; `run` and `up` failures
destroy it at once. `budget --cap X` is the only way to change the cap.

The API key comes from `~/.vast.env` (`API_KEY=...`), or `VAST_API_KEY`; it
is sent only to console.vast.ai and never printed.
"""

import argparse
import contextlib
import fcntl
import json
import os
import re
import shlex
import signal
import subprocess
import sys
import time
import urllib.error
import urllib.parse
import urllib.request
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
sys.path.insert(0, str(HERE))
import remote_blender  # noqa: E402

API = "https://console.vast.ai"
KEY_FILE = Path.home() / ".vast.env"
STATE = Path(os.environ.get("XDG_STATE_HOME") or Path.home() / ".local/state") / "source-engine"
LEDGER = STATE / "vast-ledger.json"
SSH_KEY = STATE / "vast_ed25519"
DEFAULT_CAP_USD = 10.0
BASE_TOOLCHAIN = ROOT / "build/toolchains/pbrt-map-toolchain.json"
VAST_TOOLCHAIN = ROOT / "build/toolchains/pbrt-map-toolchain-vast.json"
LABEL = "source-engine-blender"

# What the rented host installs, pinned by digest: the profile's Blender, the
# OIDN library and the Python packages of the post-bake tool steps.
HOST_PROFILE = ROOT / "quality/remote_hosts/pbrt-map-remote-host.json"
HOST = json.loads(HOST_PROFILE.read_text())
BLENDER_VERSION = HOST["blender"]["version"]
BLENDER_SHA256 = HOST["blender"]["sha256"]
BLENDER_REMOTE = HOST["blender"]["directory"] + "/" + HOST["blender"]["executable"]
REMOTE_PYTHON = HOST["blender"]["directory"] + "/" + HOST["blender"]["python"]
OIDN_LIB_DIR = str(Path(HOST["oidn"]["directory"]) / Path(HOST["oidn"]["library"]).parent)
PYTHON_TARGET = HOST["python_packages"]["target"]
# Blender needs only the driver's libcuda/libnvoptix, which the NVIDIA
# container runtime mounts; `all` capabilities include OptiX.
IMAGE = "nvidia/cuda:12.4.1-base-ubuntu22.04"
GPUS = ("RTX 3090", "RTX 3090 Ti", "RTX 4080", "RTX 4080S", "RTX 4090", "RTX 5080",
        "RTX 5090", "RTX A5000", "RTX A6000", "L40", "L40S")
BLENDER_LIBS = ("libx11-6 libxi6 libxxf86vm1 libxfixes3 libxrender1 libxext6 libxkbcommon0 "
                "libsm6 libice6 libgl1 libegl1 libglu1-mesa")


# ===================================================================== API
def api_key():
    if os.environ.get("VAST_API_KEY"):
        return os.environ["VAST_API_KEY"]
    try:
        text = KEY_FILE.read_text()
    except OSError:
        raise SystemExit("no vast API key: set VAST_API_KEY or API_KEY=... in %s" % KEY_FILE)
    match = re.search(r"^\s*(?:export\s+)?API_KEY\s*=\s*[\"']?([^\"'\s]+)", text, re.M)
    if not match:
        raise SystemExit("%s has no API_KEY=... line" % KEY_FILE)
    return match.group(1)


class Vast:
    """The few console.vast.ai endpoints this tool uses (see the vastai CLI)."""

    def __init__(self, key=None):
        self._key = key or api_key()

    def call(self, method, path, body=None, query=None):
        url = API + (path if path.startswith("/api/") else "/api/v0" + path)
        if query:
            url += "?" + "&".join("%s=%s" % (k, urllib.parse.quote_plus(json.dumps(v)))
                                  for k, v in query.items())
        data = json.dumps(body).encode() if body is not None else None
        request = urllib.request.Request(url, data=data, method=method, headers={
            "Authorization": "Bearer " + self._key, "Content-Type": "application/json"})
        for attempt in range(4):
            try:
                with urllib.request.urlopen(request, timeout=60) as response:
                    text = response.read().decode()
                    return json.loads(text) if text.strip() else {}
            except urllib.error.HTTPError as error:
                detail = error.read().decode(errors="replace")[:300]
                if error.code in (429, 502, 503, 504) and attempt < 3:
                    time.sleep(2 + 4 * attempt)
                    continue
                raise VastError(method, path, error.code, detail)
            except (urllib.error.URLError, TimeoutError) as error:
                if attempt < 3:
                    time.sleep(2 + 4 * attempt)
                    continue
                raise VastError(method, path, None, str(error))

    def credit(self):
        return float(self.call("GET", "/users/current/").get("credit") or 0.0)

    def offers(self, query):
        return self.call("POST", "/bundles/", query)["offers"]

    def create(self, offer_id, body):
        return self.call("PUT", "/asks/%d/" % offer_id, body)

    def instance(self, instance_id):
        """The instance row, or None once it is gone."""
        try:
            row = self.call("GET", "/instances/%d/" % instance_id, query={"owner": "me"})
        except VastError as error:
            if error.code == 404:
                return None
            raise
        return row.get("instances") or None

    def instances(self):
        rows, params = [], {"select_filters": {}, "order_by": [{"col": "id", "dir": "asc"}],
                            "limit": 25}
        while True:
            page = self.call("GET", "/api/v1/instances/", query=params)
            rows += page.get("instances") or []
            if not page.get("next_token"):
                return rows
            params["after_token"] = page["next_token"]

    def destroy(self, instance_id):
        try:
            return self.call("DELETE", "/instances/%d/" % instance_id, {})
        except VastError as error:
            if error.code == 404:
                return {}
            raise

    def ssh_keys(self):
        keys = self.call("GET", "/ssh/")
        return keys if isinstance(keys, list) else keys.get("ssh_keys", [])

    def add_ssh_key(self, public):
        return self.call("POST", "/ssh/", {"ssh_key": public})

    def attach_ssh_key(self, instance_id, public):
        return self.call("POST", "/instances/%d/ssh/" % instance_id, {"ssh_key": public})


class VastError(Exception):
    def __init__(self, method, path, code, detail):
        super().__init__("vast %s %s failed (%s): %s" % (method, path, code, detail))
        self.code = code


# ================================================================== ledger
class Ledger:
    """The spending cap and every instance this tool rented, shared by all
    sessions on this machine (locked while read-modify-written)."""

    def __init__(self, path=LEDGER):
        self.path = Path(path)

    @contextlib.contextmanager
    def open(self):
        self.path.parent.mkdir(parents=True, exist_ok=True)
        with open(self.path.with_suffix(".lock"), "w") as lock:
            fcntl.flock(lock, fcntl.LOCK_EX)
            data = json.loads(self.path.read_text()) if self.path.is_file() else {}
            data.setdefault("cap_usd", DEFAULT_CAP_USD)
            data.setdefault("instances", {})
            yield data
            temporary = self.path.with_suffix(".tmp")
            temporary.write_text(json.dumps(data, indent=2, sort_keys=True) + "\n")
            temporary.replace(self.path)

    def read(self):
        with self.open() as data:
            return json.loads(json.dumps(data))


def estimated_spend(data, now=None):
    """The ledger's estimate: rate over each instance's life, plus transfer."""
    now = now or time.time()
    total = 0.0
    for entry in data["instances"].values():
        end = entry.get("destroyed") or now
        total += entry["rate_usd_h"] * max(0.0, end - entry["created"]) / 3600.0
        total += entry.get("transfer_usd", 0.0)
    return total


def spend(data, credit=None, now=None):
    """Spend against the cap: the larger of the estimate and billed credit."""
    estimate = estimated_spend(data, now)
    billed = 0.0
    if credit is not None and data.get("baseline_credit") is not None:
        billed = max(0.0, data["baseline_credit"] - credit)
    return max(estimate, billed), estimate, billed


def offer_rate(offer):
    """Hourly worst case of an offer: its total (GPU and allocated disk) plus
    the storage figure again, in case the total leaves it out."""
    return float(offer["dph_total"]) + float(offer.get("storage_total_cost") or 0.0)


def transfer_allowance(offer, gigabytes):
    return gigabytes * max(float(offer.get("inet_up_cost") or 0.0),
                           float(offer.get("inet_down_cost") or 0.0))


def plan_budget(data, credit, offer, hours, transfer_gb, now=None):
    """(allowed hours, worst-case cost, problem): the rental's lifetime under
    the cap, or a problem when even `hours` would not fit."""
    spent = spend(data, credit, now)[0]
    rate, transfer = offer_rate(offer), transfer_allowance(offer, transfer_gb)
    worst = rate * hours + transfer
    if spent + worst > data["cap_usd"] + 1e-9:
        return None, worst, ("spend $%.2f + this rental's worst case $%.2f ($%.3f/h x %.2f h + "
                             "$%.2f transfer) exceeds the $%.2f cap"
                             % (spent, worst, rate, hours, transfer, data["cap_usd"]))
    return hours, worst, None


# ================================================================== offers
def offer_query(args):
    query = {"verified": {"eq": True}, "external": {"eq": False}, "rentable": {"eq": True},
             "rented": {"eq": False}, "num_gpus": {"eq": 1},
             "gpu_name": {"in": list(args.gpu or GPUS)},
             "reliability2": {"gte": args.min_reliability},
             "cuda_max_good": {"gte": 12.4}, "disk_space": {"gte": args.disk},
             "inet_down": {"gte": 200}, "inet_up": {"gte": 100},
             "inet_up_cost": {"lte": args.max_transfer_cost},
             "inet_down_cost": {"lte": args.max_transfer_cost},
             "direct_port_count": {"gte": 1}, "dph_total": {"lte": args.max_dph},
             "cpu_cores_effective": {"gte": args.min_cpus},
             "cpu_ram": {"gte": args.min_ram_gb * 1024},
             "order": [["dph_total", "asc"]], "type": "on-demand", "limit": 20,
             "allocated_storage": args.disk}
    return query


def describe(offer):
    return ("offer %d: %s, %.0f CPUs, %.0f GB RAM, $%.3f/h (+$%.4f/h storage), %s, "
            "reliability %.3f, driver %s" % (
                offer["id"], offer["gpu_name"], offer.get("cpu_cores_effective") or 0,
                (offer.get("cpu_ram") or 0) / 1024.0, offer["dph_total"],
                offer.get("storage_total_cost") or 0, offer.get("geolocation"),
                offer.get("reliability2", 0), offer.get("driver_version")))


# ===================================================================== ssh
def ensure_ssh_key(vast):
    if not SSH_KEY.is_file():
        SSH_KEY.parent.mkdir(parents=True, exist_ok=True)
        subprocess.run(["ssh-keygen", "-q", "-t", "ed25519", "-N", "", "-C", LABEL,
                        "-f", str(SSH_KEY)], check=True)
    public = SSH_KEY.with_suffix(".pub").read_text().strip()
    body = public.split()[1]
    if not any(body in (k.get("public_key") or k.get("ssh_key") or "") for k in vast.ssh_keys()):
        vast.add_ssh_key(public)
    return public


def ssh_endpoint(row):
    """(host, port) for SSH: the direct mapping when there is one, else the proxy."""
    ports = (row.get("ports") or {}).get("22/tcp") or []
    if row.get("public_ipaddr") and ports:
        return row["public_ipaddr"].strip(), int(ports[0]["HostPort"])
    if row.get("ssh_host") and row.get("ssh_port"):
        return row["ssh_host"], int(row["ssh_port"])
    return None


def ssh_transport(instance_id, port):
    known = STATE / ("known_hosts-%d" % instance_id)
    return ["ssh", "-i", str(SSH_KEY), "-p", str(port), "-o", "BatchMode=yes",
            "-o", "IdentitiesOnly=yes", "-o", "StrictHostKeyChecking=accept-new",
            "-o", "UserKnownHostsFile=" + str(known), "-o", "ConnectTimeout=20",
            "-o", "ServerAliveInterval=30", "-o", "LogLevel=ERROR"]


def fetch_pinned(package, archive):
    """Shell lines that download `package` (a HOST entry) into /opt/<archive>,
    verify its digest and unpack it, unless its directory exists. A dropped
    transfer resumes (-C -); a bad or stuck source moves to the next URL."""
    return [
        "if [ ! -d %s ]; then" % shlex.quote(package["directory"]),
        "  cd /opt",
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


def tool_env():
    """The environment of the Python tool steps on the host: the pinned
    packages, the OIDN library and imageio's EXR codec."""
    return {"PYTHONPATH": PYTHON_TARGET, "LD_LIBRARY_PATH": OIDN_LIB_DIR,
            "IMAGEIO_FREEIMAGE_LIB": HOST["freeimage"]["path"]}


def provision_script(root, tools=True):
    lines = [
        "set -eu",
        "export DEBIAN_FRONTEND=noninteractive",
        "apt-get update -qq",
        "apt-get install -y -qq --no-install-recommends rsync xz-utils curl ca-certificates "
        + BLENDER_LIBS + " >/dev/null",
    ] + fetch_pinned(HOST["blender"], "blender.tar.xz")
    if tools:
        requirements = "\n".join("%s==%s --hash=sha256:%s" % (w["name"], w["version"], w["sha256"])
                                  for w in HOST["python_packages"]["wheels"])
        python = shlex.quote(REMOTE_PYTHON)
        lines += fetch_pinned(HOST["oidn"], "oidn.tar.gz")
        lines += fetch_pinned_file(HOST["freeimage"]) + [
            "%s -m pip --version >/dev/null 2>&1 || %s -m ensurepip --default-pip >/dev/null"
            % (python, python),
            "printf '%s\\n' > /opt/source-python.new" % requirements,
            "if ! cmp -s /opt/source-python.new /opt/source-python.txt; then",
            "  %s -m pip install --quiet --disable-pip-version-check --root-user-action=ignore "
            "--no-deps --only-binary :all: --require-hashes --upgrade --target %s "
            "-r /opt/source-python.new" % (python, shlex.quote(PYTHON_TARGET)),
            "  mv /opt/source-python.new /opt/source-python.txt",
            "fi",
            "env %s %s -c %s" % (" ".join("%s=%s" % i for i in sorted(tool_env().items())), python,
                                 shlex.quote("import ctypes, numpy, scipy, imageio.v3 as iio, "
                                             "OpenImageIO, PIL; "
                                             "ctypes.CDLL('libOpenImageDenoise.so.2'); "
                                             "iio.imwrite('/tmp/check.exr', numpy.ones((4, 4, 4), "
                                             "numpy.float32)); "
                                             "assert iio.imread('/tmp/check.exr').shape "
                                             "== (4, 4, 4); "
                                             "print('TOOLS', numpy.__version__)")),
        ]
    return "\n".join(lines + ["mkdir -p %s" % shlex.quote(str(root)), "echo PROVISIONED"])


# ================================================================ lifecycle
def log(message):
    print("[vast %s] %s" % (time.strftime("%H:%M:%S"), message), file=sys.stderr, flush=True)


def destroy(vast, ledger, instance_id, reason):
    """Destroy an instance, wait until vast no longer lists it running, record it."""
    vast.destroy(instance_id)
    for _ in range(30):
        row = vast.instance(instance_id)
        if row is None or row.get("actual_status") in (None, "exited", "offline") and \
                row.get("intended_status") != "running":
            break
        time.sleep(5)
    else:
        log("instance %d still listed after destroy; retrying" % instance_id)
        vast.destroy(instance_id)
    with ledger.open() as data:
        entry = data["instances"].get(str(instance_id))
        if entry and not entry.get("destroyed"):
            entry["destroyed"] = time.time()
            entry["destroy_reason"] = reason
    log("destroyed instance %d (%s)" % (instance_id, reason))


def start_reaper(instance_id, deadline):
    """A detached process that destroys the instance at its deadline even if
    this one dies."""
    STATE.mkdir(parents=True, exist_ok=True)
    with open(STATE / ("reaper-%d.log" % instance_id), "a") as stream:
        subprocess.Popen([sys.executable, str(Path(__file__).resolve()), "reap",
                          "--instance", str(instance_id), "--deadline", str(deadline)],
                         stdin=subprocess.DEVNULL, stdout=stream, stderr=stream,
                         start_new_session=True, cwd=str(ROOT))


def reap(args):
    vast, ledger = Vast(), Ledger()
    while True:
        entry = ledger.read()["instances"].get(str(args.instance))
        if entry is None or entry.get("destroyed"):
            return 0
        if time.time() >= args.deadline:
            destroy(vast, ledger, args.instance, "reaper deadline")
            return 0
        time.sleep(min(30.0, max(1.0, args.deadline - time.time())))


def rent(vast, ledger, args):
    """Rent one host under the cap; returns the ledger entry (with its id)."""
    public = ensure_ssh_key(vast)
    offers = vast.offers(offer_query(args))
    if args.location:
        needle = args.location.casefold()
        offers = [offer for offer in offers
                  if needle in str(offer.get("geolocation") or "").casefold()]
    if not offers:
        raise SystemExit("no vast offer matches (max $%.2f/h, GPUs %s)"
                         % (args.max_dph, ", ".join(args.gpu or GPUS)))
    credit = vast.credit()
    for offer in offers[:args.attempts]:
        with ledger.open() as data:
            if data.get("baseline_credit") is None:
                data["baseline_credit"] = credit
            hours, worst, problem = plan_budget(data, credit, offer, args.hours,
                                                args.transfer_gb)
            if problem:
                raise SystemExit("refusing to rent: " + problem)
            log(describe(offer))
            log("worst case $%.2f; spent so far $%.2f of the $%.2f cap"
                % (worst, spend(data, credit)[0], data["cap_usd"]))
            if args.dry_run:
                return None
            body = {"client_id": "me", "image": IMAGE, "disk": args.disk, "label": LABEL,
                    "env": {"NVIDIA_DRIVER_CAPABILITIES": "all"},
                    "runtype": "ssh_direc ssh_proxy", "onstart": None}
            try:
                created = vast.create(offer["id"], body)
            except VastError as error:
                log("offer %d not rented: %s" % (offer["id"], error))
                continue
            instance_id = int(created["new_contract"])
            now = time.time()
            entry = {"id": instance_id, "offer": offer["id"], "gpu": offer["gpu_name"],
                     "rate_usd_h": offer_rate(offer), "created": now,
                     "transfer_usd": transfer_allowance(offer, args.transfer_gb),
                     "deadline": now + hours * 3600.0}
            data["instances"][str(instance_id)] = entry
        start_reaper(instance_id, entry["deadline"])
        log("rented instance %d; reaper destroys it by %s"
            % (instance_id, time.strftime("%H:%M:%S", time.localtime(entry["deadline"]))))
        try:
            vast.attach_ssh_key(instance_id, public)
        except VastError as error:
            log("attaching the key to %d failed (%s); relying on the account key"
                % (instance_id, error))
        return entry
    raise SystemExit("no offer could be rented")


def wait_ready(vast, instance_id, timeout):
    """Wait until the instance runs and answers SSH; returns the transport and host."""
    deadline, status = time.time() + timeout, None
    while time.time() < deadline:
        row = vast.instance(instance_id)
        if row is None:
            raise RuntimeError("instance %d disappeared while starting" % instance_id)
        if row.get("actual_status") != status:
            status = row.get("actual_status")
            log("instance %d: %s %s" % (instance_id, status, (row.get("status_msg") or "")[:120]))
        endpoint = ssh_endpoint(row) if status == "running" else None
        if endpoint:
            ssh = ssh_transport(instance_id, endpoint[1])
            host = "root@" + endpoint[0]
            probe = subprocess.run(ssh + [host, "true"], capture_output=True, text=True)
            if probe.returncode == 0:
                return ssh, host
        time.sleep(10)
    raise RuntimeError("instance %d not reachable over SSH within %d s" % (instance_id, timeout))


def write_toolchain(ssh, host, instance_id, base, out, tools=True):
    toolchain = json.loads(Path(base).read_text())
    block = {"host": host, "blender": BLENDER_REMOTE, "ssh": ssh}
    if tools:
        block.update({
            "steps": list(remote_blender.REMOTE_STEPS) + list(HOST["remote_tool_steps"]),
            "python": REMOTE_PYTHON, "env": tool_env(),
            "tools": {"openimagedenoise": {"version": HOST["oidn"]["version"],
                                           "sha256": HOST["oidn"]["sha256"]}}})
    toolchain["remote_blender"] = block
    toolchain["vast"] = {"instance": instance_id}
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(json.dumps(toolchain, indent=2) + "\n")
    return out


def up(args):
    vast, ledger = Vast(), Ledger()
    entry = rent(vast, ledger, args)
    if entry is None:
        return None
    instance_id = entry["id"]
    try:
        ssh, host = wait_ready(vast, instance_id, args.start_timeout)
        log("provisioning %s (Blender %s)" % (host, BLENDER_VERSION))
        tools = args.tool_steps
        result = subprocess.run(ssh + [host, "bash -s"], input=provision_script(ROOT, tools),
                                capture_output=True, text=True)
        if result.returncode or "PROVISIONED" not in result.stdout:
            raise RuntimeError("provisioning failed:\n" + (result.stdout + result.stderr)[-2000:])
        out = write_toolchain(ssh, host, instance_id, args.toolchain, args.out, tools)
        remote = remote_blender.RemoteBlender(json.loads(out.read_text())["remote_blender"])
        facts, problems = remote_blender.check(remote, BLENDER_VERSION, smoke=True)
        log("host check: " + json.dumps(facts))
        if problems:
            raise RuntimeError("host check failed: " + "; ".join(problems))
    except BaseException:
        destroy(vast, ledger, instance_id, "start failed")
        raise
    log("ready: instance %d, toolchain %s" % (instance_id, out))
    return instance_id, out


def live_instances(ledger):
    return [int(k) for k, v in ledger.read()["instances"].items() if not v.get("destroyed")]


def down(args):
    vast, ledger = Vast(), Ledger()
    targets = [args.instance] if args.instance else live_instances(ledger)
    for instance_id in targets:
        destroy(vast, ledger, instance_id, "down")
    if not targets:
        log("no live instance in the ledger")
    return 0


def status(args):
    vast, ledger = Vast(), Ledger()
    data, credit = ledger.read(), vast.credit()
    total, estimate, billed = spend(data, credit)
    rows = vast.instances()
    print(json.dumps({
        "cap_usd": data["cap_usd"], "spent_usd": round(total, 4),
        "estimated_usd": round(estimate, 4), "billed_usd": round(billed, 4),
        "credit_usd": credit, "baseline_credit_usd": data.get("baseline_credit"),
        "live_in_ledger": live_instances(ledger),
        "account_instances": [{"id": r["id"], "status": r.get("actual_status"),
                               "label": r.get("label"), "dph": r.get("dph_total")}
                              for r in rows]}, indent=2))
    return 0


def run(args):
    """Rent, run every render command, destroy (always)."""
    if not args.render:
        raise SystemExit("run needs at least one --render command")
    started = up(args)
    if started is None:
        return 0
    instance_id, toolchain = started
    vast, ledger = Vast(), Ledger()
    failures = []
    try:
        for index, command in enumerate(args.render, 1):
            words = shlex.split(command)
            if "{toolchain}" in command:
                words = [w.replace("{toolchain}", str(toolchain)) for w in words]
            else:
                words += ["--toolchain", str(toolchain)]
            log("render %d/%d: %s" % (index, len(args.render), " ".join(words)))
            began = time.time()
            code = subprocess.call(words, cwd=str(ROOT))
            log("render %d/%d exited %d after %.0f s" % (index, len(args.render), code,
                                                         time.time() - began))
            if code:
                failures.append((index, code))
                if not args.keep_going:
                    break
    finally:
        destroy(vast, ledger, instance_id, "run finished")
        data = ledger.read()
        total, estimate, billed = spend(data, vast.credit())
        entry = data["instances"][str(instance_id)]
        log("instance %d cost about $%.3f; spend $%.2f of the $%.2f cap (billed so far $%.2f)"
            % (instance_id, estimated_spend({"instances": {"x": entry}}), total,
               data["cap_usd"], billed))
    for index, code in failures:
        log("render %d failed (exit %d)" % (index, code))
    return 1 if failures else 0


def budget(args):
    with Ledger().open() as data:
        if args.cap is not None:
            data["cap_usd"] = args.cap
        print(json.dumps({"cap_usd": data["cap_usd"]}))
    return 0


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    commands = parser.add_subparsers(dest="command", required=True)
    renting = argparse.ArgumentParser(add_help=False)
    renting.add_argument("--hours", type=float, default=1.0,
                         help="lifetime limit; the reaper destroys the host after it")
    renting.add_argument("--max-dph", type=float, default=0.60, help="max $/hour")
    renting.add_argument("--gpu", action="append", help="allowed GPU names (default: RTX class)")
    renting.add_argument("--location",
                         help="case-insensitive substring required in the provider geolocation")
    renting.add_argument("--disk", type=int, default=40, help="GB")
    renting.add_argument("--min-cpus", type=float, default=8,
                         help="effective CPU cores (the radiosity and probe steps' Python "
                              "workers run on the host's CPU)")
    renting.add_argument("--min-ram-gb", type=float, default=32)
    renting.add_argument("--min-reliability", type=float, default=0.98)
    renting.add_argument("--max-transfer-cost", type=float, default=0.02, help="$/GB")
    renting.add_argument("--transfer-gb", type=float, default=5.0,
                         help="transfer allowance counted against the cap")
    renting.add_argument("--attempts", type=int, default=3, help="offers to try")
    renting.add_argument("--start-timeout", type=int, default=900, help="seconds")
    renting.add_argument("--toolchain", type=Path, default=BASE_TOOLCHAIN)
    renting.add_argument("--out", type=Path, default=VAST_TOOLCHAIN)
    renting.add_argument("--tool-steps", action="store_true",
                         help="also run the post-bake Python steps (remote_blender.TOOL_STEPS) "
                              "on the host; measured 2026-09-29 as about even with running "
                              "them here over a 3-4 MB/s link")
    renting.add_argument("--dry-run", action="store_true",
                         help="choose an offer and check the budget; rent nothing")
    commands.add_parser("up", parents=[renting], help="rent and provision a host")
    runner = commands.add_parser("run", parents=[renting], help="up, run --render commands, down")
    runner.add_argument("--render", action="append", help="a command; repeat for N renders")
    runner.add_argument("--keep-going", action="store_true")
    downer = commands.add_parser("down", help="destroy this tool's live instances")
    downer.add_argument("--instance", type=int)
    commands.add_parser("status", help="instances, credit and spend against the cap")
    budgeter = commands.add_parser("budget", help="show or set the spending cap")
    budgeter.add_argument("--cap", type=float)
    reaper = commands.add_parser("reap", help=argparse.SUPPRESS)
    reaper.add_argument("--instance", type=int, required=True)
    reaper.add_argument("--deadline", type=float, required=True)
    args = parser.parse_args()
    if args.command != "reap":
        # A TERM unwinds like Ctrl-C, so a failed start or a run destroys its host.
        signal.signal(signal.SIGTERM, lambda *_: sys.exit(143))
    if args.command == "up":
        return 0 if up(args) or args.dry_run else 1
    return {"run": run, "down": down, "status": status, "budget": budget,
            "reap": reap}[args.command](args)


if __name__ == "__main__":
    sys.exit(main())
