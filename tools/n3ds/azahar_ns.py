#!/usr/bin/env python3
"""Namespaced Azahar runs: several sessions can each run the 3DS build at once.

A namespace (N3DS_NS=<name>, or `n3ds.py --ns <name>`) is a private Azahar
portable user directory, build-3ds/azahar/<name>/user/. Azahar uses
`<cwd>/user/` as its whole user directory when it exists
(common/file_util.cpp, SetUserPath), so the emulator started with that cwd
gets its own config, SD card (the game directory and its logs), harness
socket, GDB stub port and log. Its NAND points at the shared one (read for
system data; this homebrew title saves to the SD card).

Without a namespace, the shared paths are used as before
(~/.var/app/org.azahar_emu.Azahar/..., build-3ds/azahar-harness.sock). In
either case, starting or stopping a run stops only that namespace's own
emulator instance (the flatpak instance it started), never every Azahar.
"""

import hashlib
import os
import re
import signal
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
APP = "org.azahar_emu.Azahar"
SHARED_DATA = Path.home() / ".var/app" / APP / "data/azahar-emu"
SHARED_CONFIG = Path.home() / ".var/app" / APP / "config/azahar-emu/qt-config.ini"
BASE_GDB_PORT = 24689

NAME = os.environ.get("N3DS_NS", "").strip() or None
if NAME and not re.fullmatch(r"[A-Za-z0-9_.-]{1,24}", NAME):
    raise SystemExit("N3DS_NS must be 1-24 characters of [A-Za-z0-9_.-]: %r" % NAME)

if NAME:
    HOME = ROOT / "build-3ds/azahar" / NAME
    USER = HOME / "user"
    SDMC = USER / "sdmc"
    NAND = SHARED_DATA / "nand"
    SOCKET = HOME / "harness.sock"
    HARNESS_LOG = HOME / "harness.log"
    EMULATOR_LOG = USER / "log/azahar_log.txt"
    PROBE_PNG = HOME / "probe.png"
    # A stable port per name, away from the shared default.
    GDB_PORT = BASE_GDB_PORT + 1 + int(hashlib.sha1(NAME.encode()).hexdigest(), 16) % 1000
else:
    HOME = ROOT / "build-3ds"
    USER = None
    SDMC = SHARED_DATA / "sdmc"
    SOCKET = ROOT / "build-3ds/azahar-harness.sock"
    HARNESS_LOG = ROOT / "build-3ds/azahar-harness.log"
    EMULATOR_LOG = SHARED_DATA / "log/azahar_log.txt"
    PROBE_PNG = ROOT / "build-3ds/probe.png"
    GDB_PORT = BASE_GDB_PORT
GAME = SDMC / "source-engine"
PIDFILE = HOME / ("azahar.pid" if NAME else "azahar-shared.pid")


def describe():
    return "namespace %s: game %s, socket %s, gdb port %d" % (NAME or "(shared)", GAME, SOCKET, GDB_PORT)


def _set(lines, key, value):
    """Sets key=value (and key\\default=false) in the [section] lines of a QSettings file."""
    out = [l for l in lines if not (l.startswith(key + "=") or l.startswith(key + "\\default="))]
    return out + ["%s=%s" % (key, value), "%s\\default=false" % key]


def prepare(headless=False):
    """Creates the namespace's user directory and config (first use: from the shared config).

    A headless run (Qt offscreen: no window system, so no OpenGL or Vulkan
    surface) renders with Azahar's software renderer; a visible run keeps the
    graphics API of the shared config."""
    if not NAME:
        return
    config = USER / "config/qt-config.ini"
    for d in (config.parent, SDMC, EMULATOR_LOG.parent):
        d.mkdir(parents=True, exist_ok=True)
    # System data (shared fonts, the MAC address file) is the user's, read-mostly.
    sysdata = USER / "sysdata"
    if not sysdata.exists() and (SHARED_DATA / "sysdata").is_dir():
        sysdata.symlink_to(SHARED_DATA / "sysdata")
    text = config.read_text() if config.exists() else (
        SHARED_CONFIG.read_text() if SHARED_CONFIG.exists() else "")
    sections, current = {}, None
    order = []
    for line in text.splitlines():
        m = re.fullmatch(r"\[(.+)\]", line.strip())
        if m:
            current = m.group(1)
            if current not in sections:
                sections[current] = []
                order.append(current)
            continue
        if current is not None:
            sections[current].append(line)
    for name in ("Data%20Storage", "Debugging", "Renderer"):
        if name not in sections:
            sections[name] = []
            order.append(name)
    data = sections["Data%20Storage"]
    data = _set(data, "use_custom_storage", "true")
    data = _set(data, "sdmc_directory", str(SDMC) + "/")
    data = _set(data, "nand_directory", str(NAND) + "/")
    sections["Data%20Storage"] = data
    debug = _set(sections["Debugging"], "gdbstub_port", str(GDB_PORT))
    sections["Debugging"] = debug
    if headless:
        sections["Renderer"] = _set(sections["Renderer"], "graphics_api", "0")
    elif config.exists() and SHARED_CONFIG.exists():
        shared = [l for l in SHARED_CONFIG.read_text().splitlines() if l.startswith("graphics_api")]
        sections["Renderer"] = [l for l in sections["Renderer"] if not l.startswith("graphics_api")] + shared
    body = []
    for name in order:
        body.append("[%s]" % name)
        body.extend(l for l in sections[name] if l.strip())
        body.append("")
    config.write_text("\n".join(body))


def popen(command, headless=False, **kwargs):
    """Starts `flatpak run ...` for this namespace (cwd = its home, so Azahar finds user/)."""
    stop()
    prepare(headless)
    HOME.mkdir(parents=True, exist_ok=True)
    process = subprocess.Popen(command, cwd=str(HOME), start_new_session=True, **kwargs)
    PIDFILE.write_text(str(process.pid))
    return process


def _instances():
    out = subprocess.run(["flatpak", "ps", "--columns=instance,pid,child-pid,application"],
                         capture_output=True, text=True).stdout
    rows = []
    for line in out.splitlines():
        parts = line.split()
        if len(parts) >= 4 and parts[3] == APP:
            rows.append(parts[:3])
    return rows


def _descendants(pid):
    found, frontier = {pid}, [pid]
    while frontier:
        parent = frontier.pop()
        try:
            children = Path("/proc/%d/task/%d/children" % (parent, parent)).read_text().split()
        except OSError:
            continue
        for child in map(int, children):
            if child not in found:
                found.add(child)
                frontier.append(child)
    return found


def stop():
    """Stops the emulator this namespace started (if any); other instances keep running."""
    if not PIDFILE.exists():
        return
    try:
        pid = int(PIDFILE.read_text().strip())
    except ValueError:
        PIDFILE.unlink()
        return
    try:
        cmdline = Path("/proc/%d/cmdline" % pid).read_bytes()
    except OSError:
        cmdline = b""
    if APP.encode() not in cmdline:
        # Exited (the guest quit), or the pid now belongs to something else.
        PIDFILE.unlink(missing_ok=True)
        if SOCKET.exists():
            SOCKET.unlink()
        return
    family = _descendants(pid)
    for instance, ipid, child in _instances():
        if int(ipid) in family or int(child) in family or int(ipid) == pid:
            subprocess.run(["flatpak", "kill", instance], stderr=subprocess.DEVNULL)
    try:
        os.killpg(pid, signal.SIGTERM)
    except (ProcessLookupError, PermissionError):
        pass
    PIDFILE.unlink(missing_ok=True)
    if SOCKET.exists():
        SOCKET.unlink()
