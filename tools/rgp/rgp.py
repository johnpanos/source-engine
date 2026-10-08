#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Radeon GPU Profiler (RGP) captures, read headlessly through the real RGP.

    rgp.py capture --out FILE.rgp --after SECONDS -- COMMAND...
    rgp.py isa TRACE.rgp --event N [--event M ...] --out DIR
    rgp.py shot TRACE.rgp --page overview|barriers|expensive|event [--event N] --out DIR

capture   runs COMMAND with RADV's trace layer (MESA_VK_TRACE=rgp) and triggers
          one frame capture SECONDS after start; the .rgp is moved to FILE.
isa       the instruction-timing table of each event's shader, as RGP measured
          it, written as DIR/event-N.json: every ISA instruction (opcode,
          operands, hit count, cost %, latency) with its basic block, and a
          summary (traced waves, cost and executed instructions per wave by
          class, basic blocks with iterations per wave, the costliest
          instructions). DIR/event-N.png holds RGP's view of the event,
          including its shader statistics (registers, occupancy).
shot      a screenshot of one RGP page (DIR/PAGE.png) to read.

RGP has no command line and its Qt build exposes no accessibility tree. This
tool runs it in a private headless mutter (no window on the desktop), drives
it with the compositor's RemoteDesktop input, reads the instruction table
through RGP's own table copy (Ctrl+A, Ctrl+C) and grabs its X windows. It
selects an event through the Event timing filter, so any event ID works.
The layout is RGP 2.7's at 2560x1440; a check fails the run at once when a
step does not produce what it should.

RGP comes from AMD's Radeon Developer Tool Suite (gpuopen.com); set RGP to
its RadeonGPUProfiler, or install it under ~/.local/opt. See README.md.
Exit status: 0 success, 1 a step failed, 2 usage.
"""

import argparse
import collections
import glob
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import time

ROOT = Path(__file__).resolve().parents[2]
WIDTH, HEIGHT = 2560, 1440
SCHEMA = "rgp-instruction-timing/v1"

# RGP 2.7, maximized at 2560x1440 (screen pixels).
TAB_OVERVIEW = (250, 77)
TAB_EVENTS = (351, 77)
EVENTS_EVENT_TIMING = (286, 119)
EVENTS_INSTRUCTION_TIMING = (613, 119)
OVERVIEW_PAGES = {"overview": (40, 116), "barriers": (29, 145), "expensive": (68, 203)}
EVENT_FILTER = (1892, 163)
EVENT_TREE_FIRST_ROW = (150, 238)
INSTRUCTION_TABLE = (600, 330)
READY_PIXEL, READY_COLOR = (150, 116), (143, 193, 231)  # the selected sidebar row
KEY = {"esc": 1, "enter": 28, "ctrl": 29, "shift": 42, "a": 30, "c": 46, "end": 107, "backspace": 14}


class RgpError(Exception):
    pass


def find_rgp():
    path = os.environ.get("RGP")
    if not path:
        found = sorted(glob.glob(os.path.expanduser(
            "~/.local/opt/RadeonDeveloperToolSuite-*/RadeonGPUProfiler")))
        path = found[-1] if found else None
    if not path or not os.access(path, os.X_OK):
        raise RgpError("RadeonGPUProfiler not found; set RGP or see tools/rgp/README.md")
    return path


# --- Instruction table ---------------------------------------------------------

def parse_instruction_table(text):
    """RGP's copied instruction-timing table: tab-separated line number,
    opcode, operands, hit count, cost %, latency. A row with no hit count
    opens a basic block (a label)."""
    instructions, block = [], None
    for line in text.splitlines():
        fields = [field.strip() for field in line.split("\t")]
        if len(fields) < 6 or not fields[0].isdigit():
            continue
        number, opcode, operands, hits, cost, latency = fields[:6]
        if not hits:
            block = opcode
            continue
        try:
            row = {"line": int(number), "opcode": opcode, "operands": operands, "hits": int(hits),
                   "cost_pct": float(cost) if cost else 0.0,
                   "latency_clk": int(latency.split()[0]) if latency else 0,
                   "block": block or "entry"}
        except ValueError as error:
            raise RgpError("unreadable instruction row %r: %s" % (line[:120], error)) from error
        instructions.append(row)
    if not instructions or not instructions[0]["block"]:
        raise RgpError("the copied text is not an instruction-timing table")
    return instructions


def instruction_class(opcode):
    if opcode.startswith("s_waitcnt"):
        return "wait"
    if opcode.startswith(("image_", "buffer_", "global_", "tbuffer_", "flat_", "scratch_")):
        return "vmem"
    if opcode.startswith(("s_load", "s_buffer_load")):
        return "smem"
    if opcode.startswith(("ds_", "lds_")):
        return "lds"
    if opcode.startswith("v_"):
        return "valu"
    if opcode.startswith("s_"):
        return "salu"
    return "other"


def summarize(instructions, top=25):
    waves = instructions[0]["hits"]
    if waves <= 0:
        raise RgpError("the event's shader has no traced waves")
    classes = collections.defaultdict(lambda: {"cost_pct": 0.0, "executed_per_wave": 0.0})
    blocks = collections.OrderedDict()
    for row in instructions:
        kind = instruction_class(row["opcode"])
        classes[kind]["cost_pct"] += row["cost_pct"]
        classes[kind]["executed_per_wave"] += row["hits"] / waves
        block = blocks.setdefault(row["block"], {"block": row["block"], "first_line": row["line"],
                                                 "instructions": 0, "hits": 0, "cost_pct": 0.0,
                                                 "memory": []})
        block["instructions"] += 1
        block["hits"] = max(block["hits"], row["hits"])
        block["cost_pct"] += row["cost_pct"]
        if kind in ("vmem", "smem") and len(block["memory"]) < 8:
            block["memory"].append("%s %s" % (row["opcode"], row["operands"]))
    for block in blocks.values():
        block["iterations_per_wave"] = round(block["hits"] / waves, 2)
        block["cost_pct"] = round(block["cost_pct"], 2)
    return {
        "waves": waves,
        "instructions": len(instructions),
        "executed_instructions": sum(1 for row in instructions if row["hits"] > 0),
        "total_cost_pct": round(sum(row["cost_pct"] for row in instructions), 2),
        "classes": {kind: {"cost_pct": round(v["cost_pct"], 2),
                           "executed_per_wave": round(v["executed_per_wave"], 1)}
                    for kind, v in sorted(classes.items(), key=lambda item: -item[1]["cost_pct"])},
        "blocks": sorted(blocks.values(), key=lambda b: -b["cost_pct"])[:top],
        "loops": [b["block"] for b in sorted(blocks.values(), key=lambda b: -b["cost_pct"])
                  if b["iterations_per_wave"] > 1.5][:top],
        "top_instructions": sorted(instructions, key=lambda row: -row["cost_pct"])[:top],
    }


# --- The session (inside the private compositor) ---------------------------------

class Session:
    def __init__(self, out):
        from gi.repository import Gio
        self.out = Path(out)
        self.bus = Gio.bus_get_sync(Gio.BusType.SESSION)
        self.session = None
        self.session = self._remote("CreateSession").unpack()[0]
        self._remote("Start")
        self._key(KEY["shift"])  # the first key on the new keyboard carries its keymap

    def _remote(self, method, signature=None, *values):
        from gi.repository import Gio, GLib
        return self.bus.call_sync(
            "org.gnome.Mutter.RemoteDesktop", self.session or "/org/gnome/Mutter/RemoteDesktop",
            "org.gnome.Mutter.RemoteDesktop" + (".Session" if self.session else ""), method,
            GLib.Variant(signature, values) if signature else None, None,
            Gio.DBusCallFlags.NONE, 5000, None)

    def _key(self, code, down=None):
        for state in ((True, False) if down is None else (down,)):
            self._remote("NotifyKeyboardKeycode", "(ub)", code, state)
            time.sleep(0.04)

    def key(self, name, *mods):
        for mod in mods:
            self._key(KEY[mod], True)
        self._key(KEY[name])
        for mod in reversed(mods):
            self._key(KEY[mod], False)
        time.sleep(0.2)

    def type_digits(self, text):
        codes = dict(zip("1234567890", range(2, 12)))
        for ch in text:
            if ch not in codes:
                raise RgpError("only digits can be typed: %r" % text)
            self._key(codes[ch])
        time.sleep(0.2)

    def click(self, point, double=False):
        x, y = point
        self._remote("NotifyPointerMotionRelative", "(dd)", -10000.0, -10000.0)
        self._remote("NotifyPointerMotionRelative", "(dd)", float(x), float(y))
        time.sleep(0.1)
        for _ in range(2 if double else 1):
            for state in (True, False):
                self._remote("NotifyPointerButton", "(ib)", 0x110, state)
                time.sleep(0.06)
        time.sleep(0.4)

    def shot(self, path):
        """Every mapped window composited at its screen position (rootless
        Xwayland has no readable root window), so menus show too."""
        from PIL import Image
        from Xlib import X, display
        dpy = display.Display(os.environ["DISPLAY"])
        root = dpy.screen().root
        canvas = Image.new("RGB", (WIDTH, HEIGHT), (40, 40, 40))

        def walk(window, depth):
            try:
                attrs, geometry = window.get_attributes(), window.get_geometry()
            except Exception:
                return
            if depth and attrs.map_state == X.IsViewable and attrs.win_class == X.InputOutput \
                    and geometry.width > 8 and geometry.height > 8:
                try:
                    at = root.translate_coords(window, 0, 0)
                    raw = window.get_image(0, 0, geometry.width, geometry.height, X.ZPixmap, 0xffffffff)
                    canvas.paste(Image.frombytes("RGB", (geometry.width, geometry.height), raw.data,
                                                 "raw", "BGRX"), (at.x, at.y))
                    return
                except Exception:
                    pass
            if depth < 3:
                for child in window.query_tree().children:
                    walk(child, depth + 1)

        walk(root, 0)
        canvas.save(path)
        dpy.close()
        return str(path)

    def main_window_size(self):
        from Xlib import X, display
        dpy = display.Display(os.environ["DISPLAY"])
        best = (0, 0)
        for window in dpy.screen().root.query_tree().children:
            try:
                attrs, geometry = window.get_attributes(), window.get_geometry()
            except Exception:
                continue
            if attrs.map_state == X.IsViewable and geometry.width * geometry.height > best[0] * best[1]:
                best = (geometry.width, geometry.height)
        dpy.close()
        return best

    def _clipboard(self, what, timeout=15):
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            text = subprocess.run(["xclip", "-o", "-selection", "clipboard"], capture_output=True,
                                  text=True, timeout=10).stdout
            if text.strip():
                return text
            time.sleep(0.5)
        raise RgpError("RGP copied nothing from %s" % what)

    def _clear_clipboard(self):
        subprocess.run(["xclip", "-i", "-selection", "clipboard", "/dev/null"], timeout=10, check=True)

    def copy_selection(self):
        self._clear_clipboard()
        self.key("c", "ctrl")
        return self._clipboard("the selected row", timeout=5).strip()

    def copy_table(self):
        self._clear_clipboard()
        self.click(INSTRUCTION_TABLE)
        self.key("a", "ctrl")
        self.key("c", "ctrl")
        return self._clipboard("the instruction table")


def wait_ready(rgp, session, timeout=60):
    """RGP's main window maximized and drawn, or fail."""
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        if rgp.poll() is not None:
            raise RgpError("RGP exited with status %s while loading the trace" % rgp.returncode)
        width, height = session.main_window_size()
        if width >= 1000:
            if width < WIDTH:
                session.click((WIDTH // 2, 18), double=True)  # maximize by its title bar
                time.sleep(2)
                width, height = session.main_window_size()
            if width < WIDTH:
                raise RgpError("RGP's window did not maximize (%dx%d)" % (width, height))
            break
        time.sleep(1)
    else:
        raise RgpError("RGP showed no window within %d s" % timeout)
    # The trace is loaded when the Overview tab opens on its frame summary
    # (the sidebar's first row highlighted).
    from PIL import Image
    probe = session.out / "ready-probe.png"
    while time.monotonic() < deadline:
        if rgp.poll() is not None:
            raise RgpError("RGP exited with status %s while loading the trace" % rgp.returncode)
        session.click(TAB_OVERVIEW)
        session.click(OVERVIEW_PAGES["overview"])
        session.shot(probe)
        if Image.open(probe).convert("RGB").getpixel(READY_PIXEL) == READY_COLOR:
            probe.unlink()
            return
        time.sleep(2)
    raise RgpError("RGP did not finish loading the trace within %d s" % timeout)


def select_event(session, event):
    session.click(TAB_EVENTS)
    session.click(EVENTS_EVENT_TIMING)
    session.click(EVENT_FILTER)
    session.key("a", "ctrl")
    session.key("backspace")
    session.type_digits(str(event))
    session.key("enter")
    time.sleep(1.5)
    # The filter leaves the event under its marker groups; it is the last row.
    session.click(EVENT_TREE_FIRST_ROW)
    session.key("end")
    time.sleep(0.5)
    # The tree copies its selected row's label: it must name this event.
    try:
        label = session.copy_selection()
    except RgpError:
        raise RgpError("event %d is not in the trace (the filtered event tree is empty)" % event)
    if not label.startswith("%d " % event):
        raise RgpError("event %d is not in the trace (the filter selected %r)" % (event, label[:80]))
    return label


def run_isa(session, args, out):
    previous = None
    results = []
    for event in args.event:
        label = select_event(session, event)
        session.click(EVENTS_INSTRUCTION_TIMING)
        time.sleep(args.settle)
        text = session.copy_table()
        if text == previous:
            raise RgpError("event %d gave the previous event's table: no such event, "
                           "or it has no traced shader" % event)
        previous = text
        instructions = parse_instruction_table(text)
        picture = session.shot(out / ("event-%d.png" % event))
        record = {"schema": SCHEMA, "trace": str(args.trace), "event": event, "api_call": label,
                  "screenshot": picture, "summary": summarize(instructions, args.top),
                  "instructions": instructions}
        path = out / ("event-%d.json" % event)
        path.write_text(json.dumps(record, indent=1) + "\n")
        (out / ("event-%d.txt" % event)).write_text(text)
        results.append({"event": event, "json": str(path), "waves": record["summary"]["waves"]})
    return results


def run_shot(session, args, out):
    if args.page == "event":
        if args.event is None:
            raise RgpError("--page event needs --event")
        select_event(session, args.event[0])
        session.click(EVENTS_EVENT_TIMING)
    else:
        session.click(TAB_OVERVIEW)
        session.click(OVERVIEW_PAGES[args.page])
    time.sleep(2)
    return [{"page": args.page, "png": session.shot(out / (args.page + ".png"))}]


def inner(argv):
    args = parse_args(argv)
    out = Path(args.out)
    result = {"ok": False}
    procs = []
    try:
        for command in (["/usr/libexec/at-spi-bus-launcher", "--launch-immediately"],
                        ["/usr/libexec/at-spi2-registryd", "--use-gnome-session=false"]):
            procs.append(subprocess.Popen(command, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL))
        home = out / "rgp-home"
        home.mkdir(parents=True, exist_ok=True)
        environment = dict(os.environ, QT_QPA_PLATFORM="xcb", HOME=str(home))
        rgp = subprocess.Popen([find_rgp(), str(args.trace)], env=environment,
                               stdout=open(out / "rgp.log", "w"), stderr=subprocess.STDOUT)
        procs.append(rgp)
        session = Session(out)
        wait_ready(rgp, session, args.load_timeout)
        result["results"] = run_isa(session, args, out) if args.command == "isa" else run_shot(session, args, out)
        result["ok"] = True
    except Exception as error:
        result["error"] = "%s: %s" % (type(error).__name__, error)
        try:
            result["failure_screenshot"] = Session.shot(None, out / "failure.png")
        except Exception:
            pass
    finally:
        for process in reversed(procs):
            process.terminate()
        (out / "result.json").write_text(json.dumps(result, indent=1) + "\n")


# --- Outside ----------------------------------------------------------------------

def outer(args, argv):
    out = Path(args.out).resolve()
    out.mkdir(parents=True, exist_ok=True)
    (out / "result.json").unlink(missing_ok=True)
    find_rgp()
    if not Path(args.trace).is_file():
        raise RgpError("no trace %s" % args.trace)
    if not shutil.which("xclip"):
        raise RgpError("xclip is not installed")
    sys.path.insert(0, str(ROOT / "tools/kiln"))
    import sepipe_loader
    inner_argv = [a if a != args.out else str(out) for a in argv]
    inner_argv = [str(Path(a).resolve()) if a == args.trace else a for a in inner_argv]
    command = [sys.executable, str(Path(__file__).resolve()), "--inner", *inner_argv]
    budget = args.load_timeout + 90 * len(args.event or [1])
    # kiln's private-x11 session: a private bus and headless mutter whose
    # Xwayland RGP and xclip use.
    try:
        sepipe_loader.run_under_display("private-x11", out / "display", (WIDTH, HEIGHT, 60),
                                        command, os.environ, out / "session.log", budget)
    except subprocess.TimeoutExpired:
        raise RgpError("the RGP session exceeded %d s (see %s)" % (budget, out / "session.log"))
    except sepipe_loader.LoadError as error:
        raise RgpError(str(error)) from error
    result_path = out / "result.json"
    if not result_path.is_file():
        raise RgpError("the session wrote no result (see %s)" % (out / "session.log"))
    result = json.loads(result_path.read_text())
    if not result["ok"]:
        raise RgpError("%s%s" % (result.get("error"), " (screenshot %s)" % result["failure_screenshot"]
                                 if result.get("failure_screenshot") else ""))
    return result


def capture(args):
    out = Path(args.out).resolve()
    with tempfile.TemporaryDirectory(prefix="rgp-capture-") as work:
        trigger = Path(work) / "trigger"
        before = set(glob.glob("/tmp/*.rgp"))
        environment = dict(os.environ, MESA_VK_TRACE="rgp", MESA_VK_TRACE_TRIGGER=str(trigger))
        process = subprocess.Popen(args.run, env=environment)
        deadline = time.monotonic() + args.after
        while time.monotonic() < deadline:
            if process.poll() is not None:
                raise RgpError("the command exited (status %s) before the capture time" % process.returncode)
            time.sleep(0.5)
        trigger.touch()
        deadline = time.monotonic() + 60
        written = []
        while time.monotonic() < deadline and not written:
            written = sorted(set(glob.glob("/tmp/*.rgp")) - before)
            if process.poll() is not None and not written:
                break
            time.sleep(0.5)
        if args.stop and process.poll() is None:
            process.terminate()
        process.wait()
        if not written:
            raise RgpError("RADV wrote no .rgp (is the device AMD and RADV the driver?)")
        time.sleep(1)
        shutil.move(written[-1], out)
    return [{"rgp": str(out)}]


def parse_args(argv):
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    commands = parser.add_subparsers(dest="command", required=True)
    grab = commands.add_parser("capture", help="run a command under RADV's trace layer and capture one frame")
    grab.add_argument("--out", required=True)
    grab.add_argument("--after", type=float, required=True, help="seconds after start to capture")
    grab.add_argument("--stop", action="store_true", help="stop the command once the capture is written")
    grab.add_argument("run", nargs=argparse.REMAINDER, metavar="-- COMMAND")
    isa = commands.add_parser("isa", help="instruction timing of events as JSON")
    shot = commands.add_parser("shot", help="a screenshot of one RGP page")
    shot.add_argument("--page", required=True, choices=["overview", "barriers", "expensive", "event"])
    for sub in (isa, shot):
        sub.add_argument("trace")
        sub.add_argument("--out", required=True)
        sub.add_argument("--event", type=int, action="append", required=sub is isa)
        sub.add_argument("--load-timeout", type=int, default=90, help="seconds to load the trace")
        sub.add_argument("--settle", type=float, default=8.0, help="seconds for instruction timing to build")
        sub.add_argument("--top", type=int, default=25)
    args = parser.parse_args(argv)
    if args.command == "capture":
        args.run = args.run[1:] if args.run[:1] == ["--"] else args.run
        if not args.run:
            parser.error("capture needs -- COMMAND")
    return args


def main(argv=None):
    argv = list(sys.argv[1:] if argv is None else argv)
    if argv[:1] == ["--inner"]:
        inner(argv[1:])
        return 0
    args = parse_args(argv)
    try:
        if args.command == "capture":
            results = capture(args)
        else:
            results = outer(args, argv)["results"]
    except RgpError as error:
        print("rgp: FAIL %s" % error, file=sys.stderr)
        return 1
    print(json.dumps(results, indent=1))
    return 0


if __name__ == "__main__":
    sys.exit(main())
