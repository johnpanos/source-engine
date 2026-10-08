#!/usr/bin/env python3
"""Run and debug the 3DS build in Azahar through its scripting harness.

The harness is a patch to Azahar's Qt frontend (dependencies/3ds/src/azahar,
src/citra_qt/harness.{h,cpp}; built by tools/n3ds/build_azahar.sh): with
`--harness <socket>` it answers line requests on a UNIX socket (`threads`,
`read`, `pause`, `resume`, `status`, `quit`) and pushes emulation errors as
events. This client drives it.

  azahar_harness.py run [--headless] [--content DIR] [--map NAME] [--args ...]
      Package and stage the build (run_azahar.py), start Azahar with the
      harness (visible unless --headless), then follow the run. It ends the
      moment the app faults (its handler writes crash.txt), Azahar reports an
      emulation error, the guest process ends, or the guest stops making
      progress (the main thread's PC and the app's logs unchanged across
      --stall seconds of samples). Then it prints every guest thread,
      symbolized: PC, LR and the return addresses on its stack.
  azahar_harness.py threads      Dump the threads of a running session.
  azahar_harness.py send REQUEST Send one raw request.

The emulator window stays open after the report unless --quit is given.
"""

import argparse
import json
import os
import re
import socket
import subprocess
import sys
import threading
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
AZAHAR_BUILD = ROOT / "dependencies/3ds/src/azahar/build"
AZAHAR_BINARY = AZAHAR_BUILD / "bin/Release/azahar"
sys.path.insert(0, str(Path(__file__).resolve().parent))
import azahar_ns  # noqa: E402

SOCKET = azahar_ns.SOCKET
import n3ds_tree  # noqa: E402
ELF = n3ds_tree.ELF
ADDR2LINE = ROOT / "dependencies/3ds/devkitpro/devkitARM/bin/arm-none-eabi-addr2line"
GAME = azahar_ns.GAME
TEXT_START, TEXT_END = 0x00100000, 0x04000000

sys.path.insert(0, str(Path(__file__).resolve().parent))
import run_azahar  # noqa: E402


class Session:
    def __init__(self, path=None, connect_within=60.0):
        path = path or SOCKET
        self.sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        deadline = time.time() + connect_within
        while True:
            try:
                self.sock.connect(str(path))
                break
            except (FileNotFoundError, ConnectionRefusedError):
                if time.time() > deadline:
                    raise SystemExit("no harness at %s (is the patched Azahar running?)" % path)
                time.sleep(0.05)
        self.buffer = b""
        self.events = []

    def _line(self, timeout=None):
        self.sock.settimeout(timeout)
        while b"\n" not in self.buffer:
            chunk = self.sock.recv(65536)
            if not chunk:
                raise EOFError("harness closed")
            self.buffer += chunk
        line, self.buffer = self.buffer.split(b"\n", 1)
        return json.loads(line)

    def request(self, text):
        self.sock.sendall(text.encode() + b"\n")
        while True:
            reply = self._line(timeout=10)
            if "event" in reply:
                self.events.append(reply)
                continue
            return reply

    def poll_events(self):
        """Returns pushed events without blocking."""
        try:
            while True:
                self.events.append(self._line(timeout=0.0))
        except (BlockingIOError, socket.timeout, TimeoutError):
            pass
        events, self.events = self.events, []
        return events


def symbolize(addresses):
    addresses = sorted({a for a in addresses if TEXT_START <= a < TEXT_END})
    if not addresses:
        return {}
    out = subprocess.run([str(ADDR2LINE), "-f", "-C", "-e", str(ELF)] + ["%x" % a for a in addresses],
                         capture_output=True, text=True).stdout.splitlines()
    names = {}
    for i, address in enumerate(addresses):
        function = out[2 * i] if 2 * i < len(out) else "?"
        where = out[2 * i + 1] if 2 * i + 1 < len(out) else "?"
        names[address] = "%s (%s)" % (function, where.split("/")[-1])
    return names


def report_threads(session, stack_words=96, reply=None):
    if reply is None:
        reply = session.request("threads %d" % stack_words)
    if not reply.get("ok"):
        print("threads: %s" % reply)
        return
    threads = [t for t in reply["threads"] if t["process"] not in ("", None) and t["status"] != "dead"]
    app = [t for t in threads if int(t["regs"][15], 16) < TEXT_END] or threads
    addresses = []
    for t in app:
        addresses += [int(t["regs"][15], 16), int(t["regs"][14], 16)] + [int(w, 16) for w in t["stack"]]
    names = symbolize(addresses)
    for t in app:
        pc, lr = int(t["regs"][15], 16), int(t["regs"][14], 16)
        print("thread %d %s [%s] core %d%s" % (t["id"], t["name"], t["status"], t["core"],
                                               " (current)" if t["current"] else ""))
        print("  pc %08x %s" % (pc, names.get(pc, "")))
        print("  lr %08x %s" % (lr, names.get(lr, "")))
        frames = [int(w, 16) for w in t["stack"] if int(w, 16) in names]
        for address in frames[:80]:
            print("     %08x %s" % (address, names[address]))


def main_thread_pc(session):
    """The guest's progress signature: every live application thread's
    status and registers (a thread blocked in a service call that keeps
    issuing new calls changes r0-r3, LR or SP)."""
    reply = session.request("threads 0")
    state = []
    for t in reply.get("threads", []):
        if int(t["regs"][15], 16) < TEXT_END and t["status"] != "dead":
            state.append((t["id"], t["status"], tuple(t["regs"])))
    return tuple(state)


def log_sizes():
    return tuple((GAME / n).stat().st_size if (GAME / n).exists() else -1
                 for n in ("early.txt", "trace.txt", "console.log"))


class ConsoleTail:
    """Prints the guest's console.log (stdout and engine spew) as it grows."""

    def __init__(self, path=None):
        path = path or GAME / "console.log"
        self.path, self.offset, self.partial = path, 0, b""

    def pump(self, echo=True):
        """Returns the complete lines added since the last call."""
        try:
            with self.path.open("rb") as handle:
                if handle.seek(0, 2) < self.offset:
                    self.offset = 0
                handle.seek(self.offset)
                data = handle.read()
                self.offset = handle.tell()
        except FileNotFoundError:
            return []
        lines = (self.partial + data).split(b"\n")
        self.partial = lines.pop()
        text = [line.decode("utf-8", "replace").rstrip("\r") for line in lines]
        if echo:
            for line in text:
                print("| " + line, flush=True)
        return text


def follow(session, stall, limit):
    """Returns why the run ended: crash, error, exit, stall or limit."""
    tail = ConsoleTail()
    start = time.time()
    last_progress = time.time()
    last_state = None
    while time.time() - start < limit:
        tail.pump()
        for event in session.poll_events():
            if event.get("event") == "guest_stop":
                (ROOT / "build-3ds/guest_stop.json").write_text(json.dumps(event, indent=1))
                print("GUEST STOP (%s), dump in build-3ds/guest_stop.json" % event.get("reason"))
                if event.get("report"):
                    print(event["report"])
                report_threads(session, reply=event.get("dump", {}))
                return "guest_stop"
            if event.get("event") == "error" and event.get("result") == 16:
                return "exit"
            print("EVENT %s" % json.dumps(event))
            return "error"
        crash = GAME / "crash.txt"
        if crash.exists() and crash.stat().st_size:
            print("CRASH (app exception handler):\n" + crash.read_text(errors="replace")[:2000])
            return "crash"
        try:
            state = (main_thread_pc(session), log_sizes())
        except EOFError:
            return "exit"
        if state != last_state:
            last_state, last_progress = state, time.time()
        elif time.time() - last_progress > stall:
            print("STALL: main thread and logs unchanged for %.0f s (at %.0f s)" % (stall, time.time() - start))
            return "stall"
        time.sleep(0.25)
    return "limit"


HEAP_BASE = 0x08000000
HEAP_DUMP_BYTES = 112 * 1024 * 1024  # the heap is about 100 MB; unmapped pages read as zeros


def arm_heap_dump(session, path):
    if path:
        path = path.resolve()
        print(session.request("dump_on_stop %x %d %s" % (HEAP_BASE, HEAP_DUMP_BYTES, path)))


def package():
    """The CXI (exheader: New 3DS 178 MB mode); see tools/n3ds/package_cia.sh."""
    subprocess.run([str(ROOT / "tools/n3ds/package_cia.sh"), str(ELF)], check=True, stdout=subprocess.DEVNULL)
    return ROOT / "build-3ds/Portal2.cxi"


def start_emulator(headless, app, hold=False):
    azahar_ns.stop()
    command = ["flatpak", "run", "--filesystem=%s" % ROOT, "--command=%s" % AZAHAR_BINARY]
    if headless and azahar_ns.PRIVATE_DISPLAY:
        command += ["--socket=wayland", "--env=QT_QPA_PLATFORM=wayland"]
    elif headless:
        command.append("--env=QT_QPA_PLATFORM=offscreen")
    command += ["org.azahar_emu.Azahar", "--harness", str(SOCKET)]
    if hold:
        command.append("--harness-paused")
    command.append(str(app))
    azahar_ns.HARNESS_LOG.parent.mkdir(parents=True, exist_ok=True)
    log = open(azahar_ns.HARNESS_LOG, "w")
    azahar_ns.popen(command, headless=headless, stdout=log, stderr=subprocess.STDOUT)
    print(azahar_ns.describe(), flush=True)


GDB = ROOT / "dependencies/3ds/devkitpro/devkitARM/bin/arm-none-eabi-gdb"
# A breakpoint or script line gdb could not take: the run cannot be trusted.
GDB_SCRIPT_ERROR = re.compile(r'(Function|No symbol) ".*" (not defined|in current context)|^No symbol table|'
                              r'Error in sourced command file|^Junk at end')
GDB_PORT = azahar_ns.GDB_PORT


def gdb_connected():
    out = subprocess.run(["ss", "-tnH", "state", "established", "( sport = :%d )" % GDB_PORT],
                         capture_output=True, text=True).stdout
    return bool(out.strip())


def debug_run(options):
    """Holds the guest at its entry point, attaches gdb to Azahar's GDB stub
    (Emulation > Configure > Debug: GDB stub), sets the breakpoints, resumes,
    and prints the backtrace at the first stop."""
    app = package()
    run_azahar.stage(options.content, ("+map %s " % options.map if options.map else "") + options.args)
    start_emulator(options.headless, app, hold=True)
    session = Session()
    if options.speed is not None:
        session.request("speed %g" % options.speed)
    arm_heap_dump(session, options.heap_dump)
    breaks = options.breaks or ["exit", "abort", "_exit"]
    command = [str(GDB), "-q", "-batch", "-ex", "set pagination off", "-ex", "set confirm off",
               "-ex", "set tcp auto-retry on", "-ex", "set tcp connect-timeout 60",
               "-ex", "target remote 127.0.0.1:%d" % GDB_PORT, "-ex", "printf \"GDB_ATTACHED %x\\n\", $pc"]
    for location in breaks:
        command += ["-ex", "break %s" % location]
    command += ["-ex", "source %s" % (ROOT / "tools/n3ds/gdb/errors.gdb")]
    if options.gdb_script:
        command += ["-ex", "source %s" % options.gdb_script.resolve()]
    command += ["-ex", "continue", "-ex", "bt 60", "-ex", "info registers"]
    for extra in options.gdb:
        command += ["-ex", extra]
    command.append(str(ELF))
    gdb = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True,
                           errors="replace", bufsize=1)
    # gdb's output streams as it happens (breakpoint commands print while the
    # guest runs); an unread pipe would fill and block gdb with the guest
    # halted at a breakpoint.
    gdb_lines = []
    gdb_attached = threading.Event()
    gdb_failed = threading.Event()

    def read_gdb():
        for line in gdb.stdout:
            line = line.rstrip("\n")
            if line.startswith("GDB_ATTACHED "):
                gdb_attached.set()
            if "no version information" in line or line.startswith("Breakpoint "):
                continue
            gdb_lines.append(line)
            print("gdb| " + line, flush=True)
            if GDB_SCRIPT_ERROR.search(line):
                gdb_failed.set()

    reader = threading.Thread(target=read_gdb, daemon=True)
    reader.start()
    # The stub answers packets only from the emulation loop: release the guest
    # once gdb's connection is established; the stub halts it on the handshake.
    while not gdb_connected():
        if gdb.poll() is not None:
            break
        time.sleep(0.05)
    session.request("resume")
    # Self-test: gdb must report its remote connection, or nothing it prints
    # later can be trusted.
    if not gdb_attached.wait(30):
        print("DEBUGGER NOT ATTACHED: gdb never reported the remote connection")
    else:
        print("debugger attached", flush=True)
    # Whichever comes first: a gdb stop (exit, abort, a breakpoint), a guest
    # fault or stop reported by the harness, or a stall.
    last_state, last_progress = None, time.time()
    started = last_report = time.time()
    reason = "gdb"
    memory, memory_at = None, 0.0
    while gdb.poll() is None:
        if gdb_failed.is_set():
            print("GDB SCRIPT ERROR: a breakpoint or command was not accepted (see gdb| lines); stopping")
            reason = "gdb script error"
            break
        if time.time() - memory_at > 2.0:
            try:
                memory, memory_at = session.request("memory"), time.time()
            except EOFError:
                pass
        events = session.poll_events()
        stop = next((e for e in events if e.get("event") == "guest_stop"), None)
        if stop and stop.get("reason") != "exception":
            try:
                print("memory at stop: %s" % json.dumps(session.request("memory")))
            except (EOFError, TimeoutError):
                pass
        if stop and stop.get("reason") == "exception":
            (ROOT / "build-3ds/guest_stop.json").write_text(json.dumps(stop, indent=1))
            print("GUEST FAULT\n" + stop.get("report", ""))
            report_threads(session, reply=stop.get("dump", {}))
            reason = "fault"
            break
        try:
            state = (main_thread_pc(session), log_sizes())
        except EOFError:
            reason = "emulator closed"
            break
        if state != last_state:
            last_state, last_progress = state, time.time()
        if time.time() - last_report > 5.0 and state[0]:
            # Where the main thread is, every few seconds (a slow phase looks
            # different from a hang).
            regs = state[0][0][2]
            names = symbolize([int(regs[15], 16), int(regs[14], 16)])
            print("[%4.0f s] main %s pc %s lr %s" % (time.time() - started, state[0][0][1],
                  names.get(int(regs[15], 16), regs[15]), names.get(int(regs[14], 16), regs[14])),
                  flush=True)
            last_report = time.time()
        if time.time() - last_progress > options.stall:
            print("STALL: no guest progress for %.0f s" % options.stall)
            session.request("pause")
            report_threads(session)
            reason = "stall"
            break
        time.sleep(0.25)
    if gdb.poll() is None:
        gdb.kill()
    gdb.wait()
    reader.join(5)
    print("run ended: %s" % reason)
    if memory:
        print("last memory sample: application %.1f/%.1f MB used, process %.1f MB" % (
            memory["application_used"] / 1e6, memory["application_size"] / 1e6,
            memory["process_used"] / 1e6))
    for name in ("console.log",):
        path = GAME / name
        if path.exists():
            print("-- %s (tail)" % name)
            print("\n".join(l for l in path.read_text(errors="replace").splitlines()[-12:]
                            if "Can't find module" not in l))
    return 0


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = parser.add_subparsers(dest="command", required=True)
    run = sub.add_parser("run")
    run.add_argument("--headless", action="store_true")
    run.add_argument("--content", type=Path)
    run.add_argument("--map", default="n3ds_chamber")
    run.add_argument("--args", default="")
    run.add_argument("--heap-dump", type=Path, help="write the guest heap here when the guest stops "
                     "(tools/n3ds/heap_census.py reads it)")
    run.add_argument("--speed", type=float, default=None,
                     help="emulation speed in percent for this session (0 = unlimited); default keeps the user setting")
    run.add_argument("--stall", type=float, default=5.0, help="seconds without guest progress that count as a hang")
    run.add_argument("--limit", type=float, default=600.0)
    run.add_argument("--quit", action="store_true", help="close the emulator after the report")
    debug = sub.add_parser("debug", help="hold at entry, attach gdb, set breakpoints, run to the first stop")
    debug.add_argument("--headless", action="store_true")
    debug.add_argument("--content", type=Path)
    debug.add_argument("--map", default="n3ds_chamber")
    debug.add_argument("--args", default="")
    debug.add_argument("--heap-dump", type=Path, help="write the guest heap here when the guest stops "
                     "(tools/n3ds/heap_census.py reads it)")
    debug.add_argument("--break", dest="breaks", action="append", default=[],
                       help="gdb breakpoint location (repeatable; default: exit, abort, _exit)")
    debug.add_argument("--gdb", action="append", default=[], help="extra gdb command run at the stop")
    debug.add_argument("--speed", type=float, default=None,
                     help="emulation speed in percent for this session (0 = unlimited); default keeps the user setting")
    debug.add_argument("--stall", type=float, default=20.0, help="seconds without guest progress that count as a hang")
    debug.add_argument("--gdb-script", type=Path, help="gdb script sourced before the guest runs "
                       "(breakpoints with `commands` that print and continue)")
    shot = sub.add_parser("screenshot", help="save the next frame of a running session as a PNG")
    shot.add_argument("path", type=Path)
    sub.add_parser("threads")
    send = sub.add_parser("send")
    send.add_argument("request", nargs="+")
    options = parser.parse_args()

    if options.command == "debug":
        return debug_run(options)
    if options.command == "screenshot":
        path = options.path.resolve()
        if path.exists():
            path.unlink()
        reply = Session().request("screenshot %s" % path)
        if not reply.get("ok"):
            raise SystemExit("screenshot: %s" % reply)
        deadline = time.time() + 30
        while not path.exists() and time.time() < deadline:
            time.sleep(0.1)
        print(path if path.exists() else "screenshot: no frame within 30 s")
        return 0 if path.exists() else 1
    if options.command == "threads":
        report_threads(Session())
        return 0
    if options.command == "send":
        print(json.dumps(Session().request(" ".join(options.request))))
        return 0

    if not AZAHAR_BINARY.exists():
        raise SystemExit("build the harness first: tools/n3ds/build_azahar.sh")
    app = package()
    run_azahar.stage(options.content, ("+map %s " % options.map if options.map else "") + options.args)
    start_emulator(options.headless, app)
    session = Session()
    if options.speed is not None:
        session.request("speed %g" % options.speed)
    arm_heap_dump(session, options.heap_dump)
    print("harness connected; following the run")
    reason = follow(session, options.stall, options.limit)
    print("run ended: %s" % reason)
    if reason in ("stall", "crash", "limit"):
        session.request("pause")
        report_threads(session)
    for name in ("console.log", "trace.txt", "early.txt"):
        path = GAME / name
        if path.exists():
            print("-- %s (tail)" % name)
            print("\n".join(path.read_text(errors="replace").splitlines()[-20:]))
    if options.quit:
        session.request("quit")
    return 0 if reason == "limit" else 1


if __name__ == "__main__":
    sys.exit(main())
