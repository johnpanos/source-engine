#!/usr/bin/env python3
"""UI-driven Hammer conformance: make a map the way a person does (RFC 0002).

Each case starts the real GTK editor (hammer_gtk) in its own headless mutter
session and drives it as a user would: widgets are found by their accessible
names on the AT-SPI bus, and pointer and key input enters through the
compositor's own input path (a org.gnome.Mutter.RemoteDesktop session on the
private session bus; XTest is not delivered by Xwayland here). Positions in a
view come from the status-bar coordinates the editor shows while hovering:

  1. halve the grid twice with "[" (64 -> 16), pick the Block tool;
  2. drag a 384x384 block in the top view, drag its 128-unit height in the
     front view (a new box there keeps the pending box's depth, as in
     legacy Hammer) and press Return;
  3. press F to hollow it into a room (walls one grid unit thick);
  4. pick the Entity tool, click a player start into the front view, choose
     "light" in the class dropdown (keyboard: Down, Return) and click a light;
  5. press F9, which saves and compiles the map.

The oracle judges the files the editor wrote, never the UI's own claims: the
saved VMF must hold a six-wall room with both entities inside it, the build
record must show a leak-free compile, and the frames the live editor showed
(written with HAMMER_GTK_FRAME_DIR by its render-core viewports) must draw the
room's walls in the top view and shaded geometry in the camera view. Two negative controls drive the same UI
with one step left out; the oracle must reject each:

  no-hollow  skips F: the saved map is one solid block (room.walls fails).
  no-light   places no light (room.light fails).

  hammer_ui_test.py --cli build-r03-tools/hammer/cli/hammer_cli --out quality-results/hammer-ui

The shell is built from the current source into the output directory
(hammer/gtk/build.sh) unless --gtk names a binary. Needs mutter, the AT-SPI
bus and registry daemons, python3-gobject with Atspi, and the compile
toolchain vmf_map_build.py uses.

Windows open only inside the private compositor; nothing reaches the desktop.
Results are reported as checks-v1 (tools/quality/conformance_result.py).
"""

import argparse
import json
import os
import re
import shutil
import subprocess
import sys
import time
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

from conformance_result import Checks  # noqa: E402

ROOT = HERE.parents[1]
SCREEN = (1280, 800)
CASES = ("room", "no-hollow", "no-light")
ENTITY_CLASSES = ("info_player_start", "light")  # the dropdown's order (hammer/gtk/app.cpp)

# The authored room: a block from (-192,-192,0) to (192,192,128), hollowed with
# 16-unit walls. Entities go in the front view (x/z), so they land at y = 0.
ROOM_HALF = 192
ROOM_HEIGHT = 128
PLAYER = (-64, 32)   # front-view (x, z)
LIGHT = (64, 96)


# ---- Oracle ------------------------------------------------------------------

def vmf_blocks(text):
    """Top-level (name, body) blocks of a VMF, with bodies as raw text."""
    blocks, i = [], 0
    pattern = re.compile(r'\s*("?)([A-Za-z_]+)\1\s*\{')
    while True:
        m = pattern.search(text, i)
        if not m:
            return blocks
        depth, j = 1, m.end()
        while depth and j < len(text):
            depth += {"{": 1, "}": -1}.get(text[j], 0)
            j += 1
        blocks.append((m.group(2), text[m.end():j - 1]))
        i = j


def keyvalue(body, key):
    m = re.search(r'"%s"\s+"([^"]*)"' % re.escape(key), body)
    return m.group(1) if m else None


# What the live editor last showed, from the frames it writes with
# HAMMER_GTK_FRAME_DIR (its render-core viewports, RFC 0016 "Editor
# viewports"): the top view must show the room's walls as edge lines, the
# camera view shaded geometry over its clear color.
EDGE_COLORS = ((128, 133, 148), (255, 148, 38))  # plain and selected edges
CAMERA_CLEAR = (33, 36, 43)


def read_ppm(path):
    data = path.read_bytes() if path.is_file() else b""
    match = re.match(rb"P6\s+(\d+)\s+(\d+)\s+255\s", data)
    if not match:
        return 0, 0, b""
    width, height = int(match.group(1)), int(match.group(2))
    return width, height, data[match.end():match.end() + width * height * 3]


def judge_frames(frames):
    results = {}
    width, height, top = read_ppm(frames / "top.ppm")
    edges = sum(1 for i in range(0, len(top) - 2, 3) if tuple(top[i:i + 3]) in EDGE_COLORS)
    results["shown.top"] = (width > 0 and edges >= 200,
                            "%dx%d top frame, %d edge pixels" % (width, height, edges))
    width, height, camera = read_ppm(frames / "camera.ppm")
    drawn = sum(1 for i in range(0, len(camera) - 2, 3)
                if sum(abs(a - b) for a, b in zip(camera[i:i + 3], CAMERA_CLEAR)) > 18)
    share = drawn / float(width * height) if width and height else 0.0
    results["shown.camera"] = (share >= 0.02, "%dx%d camera frame, %.1f%% drawn"
                               % (width, height, 100 * share))
    return results


def judge_frame_path(log_path):
    """How the live editor's frames reached GTK (its log names the path): as
    dmabufs when the device exports images, else read back; a refused dmabuf
    import fails."""
    text = log_path.read_text(errors="replace") if log_path.is_file() else ""
    match = re.search(r"hammer_gtk: viewport frames as ([a-z -]+)", text)
    refused = "dmabuf frames refused" in text
    path = match.group(1) if match else "not logged"
    return {"frames.path": (bool(match) and not refused,
                            path + ("; a dmabuf import was refused" if refused else ""))}


def judge(vmf_path, build_path):
    """Check names -> (ok, detail) for one case's outputs."""
    results = {}
    text = vmf_path.read_text() if vmf_path.is_file() else ""
    results["saved"] = (bool(text), str(vmf_path))
    blocks = vmf_blocks(text)
    world = next((body for name, body in blocks if name == "world"), "")
    walls = len(re.findall(r"\bsolid\s*\{", world))
    results["walls"] = (walls == 6, "%d world solids, want 6" % walls)
    entities = {}
    for name, body in blocks:
        if name == "entity":
            entities.setdefault(keyvalue(body, "classname"), []).append(keyvalue(body, "origin"))

    def inside(origin):
        try:
            x, y, z = (float(v) for v in origin.split())
        except (AttributeError, ValueError):
            return False
        inner = ROOM_HALF - 16
        return abs(x) < inner and abs(y) < inner and 16 <= z < ROOM_HEIGHT - 16

    for classname, check in (("info_player_start", "player"), ("light", "light")):
        origins = entities.get(classname, [])
        results[check] = (len(origins) == 1 and inside(origins[0]),
                          "%s origins %s" % (classname, origins))
    record = json.loads(build_path.read_text()) if build_path.is_file() else {}
    results["compiled"] = (record.get("status") == "pass",
                           "build status %s %s" % (record.get("status"), record.get("failed_gates")))
    results["sealed"] = (record.get("leaked") is False, "leaked %s" % record.get("leaked"))
    return results


# ---- Driver (runs inside the private compositor session) ---------------------

class Driver:
    def __init__(self, log):
        import gi
        gi.require_version("Atspi", "2.0")
        from gi.repository import Atspi
        self.Atspi = Atspi
        self.log = log
        self.app = None

    def note(self, message):
        self.log.append("%.2f %s" % (time.monotonic(), message))

    def find_app(self, timeout=20.0):
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            desk = self.Atspi.get_desktop(0)
            for i in range(desk.get_child_count()):
                child = desk.get_child_at_index(i)
                if child is not None and child.get_name() == "hammer_gtk":
                    self.app = child
                    if self.find(lambda n, r: r == "frame" and n == "top (x/y)"):
                        return True
            time.sleep(0.25)
        return False

    def nodes(self):
        stack = [self.app]
        while stack:
            node = stack.pop()
            try:
                name, role = node.get_name(), node.get_role_name()
                count = node.get_child_count()
            except Exception:
                continue
            yield node, name, role
            stack.extend(node.get_child_at_index(i) for i in range(count))

    def find(self, predicate):
        for node, name, role in self.nodes():
            if predicate(name, role):
                return node
        return None

    def named(self, name, role=None):
        node = self.find(lambda n, r: n == name and (role is None or r == role))
        if node is None:
            raise RuntimeError("no %s named %r" % (role or "widget", name))
        return node

    def label_matching(self, pattern):
        regex = re.compile(pattern)
        node = self.find(lambda n, r: r == "label" and bool(regex.match(n or "")))
        return node.get_name() if node else None

    def wait_label(self, pattern, timeout=5.0):
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            text = self.label_matching(pattern)
            if text:
                return text
            time.sleep(0.1)
        status = [n for _, n, r in self.nodes() if r == "label" and n and
                  re.match(r"^(Grid|\d+ brush|For Help|Built|build_map)", n)]
        raise RuntimeError("no label matching %r (status bar: %s)" % (pattern, status))

    def wait_active(self, timeout=5.0):
        """Waits until the editor's window is the active (keyboard-focused) one."""
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            frame = self.find(lambda n, r: r == "frame" and (n or "").startswith("Hammer - "))
            if frame is not None and frame.get_state_set().contains(self.Atspi.StateType.ACTIVE):
                self.note("window active")
                return
            time.sleep(0.1)
        raise RuntimeError("the editor window never became active")

    def extents(self, node):
        e = node.get_extents(self.Atspi.CoordType.WINDOW)
        return e.x, e.y, e.width, e.height

    def press(self, node):
        node.do_action(0)
        time.sleep(0.2)

    # Linux input-event codes (the compositor takes evdev keycodes).
    KEYS = {"Shift": 42, "[": 26, "f": 33, "Return": 28, "Down": 108, "F9": 67}
    BTN_LEFT = 0x110

    def remote(self, method, signature=None, *values):
        from gi.repository import Gio, GLib
        return self.bus.call_sync(
            "org.gnome.Mutter.RemoteDesktop", self.session or "/org/gnome/Mutter/RemoteDesktop",
            "org.gnome.Mutter.RemoteDesktop" + (".Session" if self.session else ""), method,
            GLib.Variant(signature, values) if signature else None, None,
            Gio.DBusCallFlags.NONE, -1, None)

    def start_input(self):
        from gi.repository import Gio
        self.bus = Gio.bus_get_sync(Gio.BusType.SESSION)
        self.session = None
        self.session = self.remote("CreateSession").unpack()[0]
        self.remote("Start")

    def key(self, name):
        for down in (True, False):
            self.remote("NotifyKeyboardKeycode", "(ub)", self.KEYS[name], down)
            time.sleep(0.05)
        time.sleep(0.2)

    def move(self, x, y):
        """Pointer to screen (x, y): home against the top-left corner, then move
        by the offset (the session has no absolute stream without a screencast)."""
        self.remote("NotifyPointerMotionRelative", "(dd)", -10000.0, -10000.0)
        self.remote("NotifyPointerMotionRelative", "(dd)", float(round(x)), float(round(y)))
        self.pointer = (round(x), round(y))
        time.sleep(0.1)

    def button(self, down):
        self.remote("NotifyPointerButton", "(ib)", self.BTN_LEFT, down)
        time.sleep(0.1)

    def click(self, x, y):
        self.move(x, y)
        self.button(True)
        self.button(False)
        time.sleep(0.2)

    def drag(self, x0, y0, x1, y1, steps=8):
        self.move(x0, y0)
        self.button(True)
        px, py = round(x0), round(y0)
        for i in range(1, steps + 1):
            nx, ny = round(x0 + (x1 - x0) * i / steps), round(y0 + (y1 - y0) * i / steps)
            self.remote("NotifyPointerMotionRelative", "(dd)", float(nx - px), float(ny - py))
            px, py = nx, ny
            time.sleep(0.05)
        self.button(False)
        time.sleep(0.2)

    COORDS = r"^[xyz] -?\d+  [xyz] -?\d+$"

    def hover_world(self, x, y):
        """Hovers screen (x, y) and returns the world (u, v) the status bar shows."""
        before = self.label_matching(self.COORDS)
        self.move(x - 2, y - 2)
        self.move(x, y)
        deadline = time.monotonic() + 3
        text = self.label_matching(self.COORDS)
        while text == before and time.monotonic() < deadline:
            time.sleep(0.1)
            text = self.label_matching(self.COORDS)
        _, u, _, v = text.split()
        return float(u), float(v)

    def calibrate(self, view):
        """Screen <-> world mapping of a 2D view, read from the status-bar
        coordinates while hovering (as a user reads them) and checked at a
        third point. Returns a function world (u, v) -> screen (x, y)."""
        x, y, w, h = self.extents(self.named(view, "frame"))
        (ax, ay), (bx, by) = (x + w * 0.3, y + h * 0.3), (x + w * 0.7, y + h * 0.7)
        (au, av), (bu, bv) = self.hover_world(ax, ay), self.hover_world(bx, by)
        if bu == au or bv == av:
            raise RuntimeError("%s calibration did not move" % view)
        su, sv = (bx - ax) / (bu - au), (by - ay) / (bv - av)
        self.note("%s: %.3f px/unit u, %.3f px/unit v" % (view, su, sv))

        def to_screen(u, v):
            px, py = ax + (u - au) * su, ay + (v - av) * sv
            if not (x < px < x + w and y < py < y + h):
                raise RuntimeError("world (%s, %s) is outside the %s view" % (u, v, view))
            return px, py
        cu, cv = self.hover_world(x + w * 0.5, y + h * 0.4)
        pu, pv = (x + w * 0.5 - ax) / su + au, (y + h * 0.4 - ay) / sv + av
        # Judged in pixels: the label shows whole units, so at a zoom below one
        # pixel per unit a correct view is off by up to a pixel's worth.
        if abs(cu - pu) * abs(su) > 1.5 or abs(cv - pv) * abs(sv) > 1.5:
            raise RuntimeError("%s is not linear: (%s, %s) predicted (%.1f, %.1f)"
                               % (view, cu, cv, pu, pv))
        return to_screen

    def entity_class(self):
        combo = self.find(lambda n, r: r == "combo box" and n in ENTITY_CLASSES)
        if combo is None:
            raise RuntimeError("no entity class dropdown")
        return combo

    def choose_class(self, classname):
        """Opens the entity-class dropdown and steps down its list to
        `classname` with the keyboard (Down, Return)."""
        combo = self.entity_class()
        if combo.get_name() == classname:
            return
        cx, cy, cw, ch = self.extents(combo)
        self.click(cx + cw / 2, cy + ch / 2)
        time.sleep(0.3)
        for _ in range(ENTITY_CLASSES.index(classname) - ENTITY_CLASSES.index(combo.get_name())):
            self.key("Down")
        self.key("Return")
        deadline = time.monotonic() + 3
        while time.monotonic() < deadline:
            if self.entity_class().get_name() == classname:
                return
            time.sleep(0.1)
        raise RuntimeError("entity class did not change to %s" % classname)

    @staticmethod
    def inside(node, ancestor):
        parent = node.get_parent()
        while parent is not None:
            if parent == ancestor:
                return True
            parent = parent.get_parent()
        return False


def drive(case, log):
    """The user steps for one case. Raises on a UI that does not respond."""
    d = Driver(log)
    d.start_input()
    if not d.find_app():
        raise RuntimeError("hammer_gtk did not appear on the accessibility bus")
    d.note("editor up")
    top_view = d.named("top (x/y)", "frame")
    tx, ty, tw, th = d.extents(top_view)
    d.click(tx + tw / 2, ty + th / 2)  # focus the window (the Select tool ignores an empty click)
    d.wait_active()
    # The first key from a new virtual keyboard carries its keymap to Xwayland and
    # is not delivered as a key; a lone Shift absorbs that.
    d.key("Shift")

    d.key("[")
    d.wait_label(r"^Grid: 32\b")
    d.key("[")
    d.wait_label(r"^Grid: 16\b")
    d.press(d.named("Block Tool", "toggle button"))
    d.wait_label(r"^Grid: 16  ·  Block$")
    d.note("grid 16, block tool")

    top = d.calibrate("top (x/y)")
    x0, y0 = top(-ROOM_HALF, -ROOM_HALF)
    x1, y1 = top(ROOM_HALF, ROOM_HALF)
    d.drag(x0, y0, x1, y1)
    # The height: redraw the box in the front view from its top-left corner
    # (outside the pending box, so not a handle); the depth (y) carries over.
    front = d.calibrate("front (x/z)")
    x0, y0 = front(-ROOM_HALF, ROOM_HEIGHT)
    x1, y1 = front(ROOM_HALF, 0)
    d.drag(x0, y0, x1, y1)
    d.key("Return")  # commits the block
    d.wait_label(r"^1 brush\(es\)")
    d.note("block committed")

    if case != "no-hollow":
        d.key("f")
        d.wait_label(r"^6 brush\(es\)")
        d.note("hollowed")

    d.press(d.named("Entity Tool", "toggle button"))
    d.choose_class("info_player_start")
    d.click(*front(*PLAYER))
    d.note("player start placed")
    if case != "no-light":
        d.choose_class("light")
        d.click(*front(*LIGHT))
        d.note("light placed")

    d.key("F9")  # save and compile
    d.note("F9")
    # The build runs on the UI thread; the help line reports its outcome.
    text = d.wait_label(r"^(Built |build_map)", timeout=300)
    d.note("build reported: " + text)


def inner(args):
    """Inside the compositor: start the accessibility bus, the editor and the
    driver; write the driver log."""
    out = Path(args.case_dir)
    procs = []
    log = []
    status = "pass"
    try:
        for command in (["/usr/libexec/at-spi-bus-launcher", "--launch-immediately"],
                        ["/usr/libexec/at-spi2-registryd", "--use-gnome-session=false"]):
            procs.append(subprocess.Popen(command, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL))
            time.sleep(1)
        frames = out / "frames"
        frames.mkdir(parents=True, exist_ok=True)
        env = dict(os.environ, GDK_BACKEND="x11", GTK_CSD="1", GTK_A11Y="atspi",
                   HAMMER_GTK_FRAME_DIR=str(frames))
        app_log = open(out / "hammer_gtk.log", "w")
        procs.append(subprocess.Popen(
            [args.gtk, "--maximized", "--open", str(out / "author" / args.vmf_name),
             "--builds", str(out / "builds"), "--no-publish"],
            cwd=str(ROOT), env=env, stdout=app_log, stderr=subprocess.STDOUT))
        drive(args.case, log)
    except Exception as error:  # the log records where the UI stopped responding
        status = "driver-error: %s" % error
    finally:
        for proc in reversed(procs):
            proc.terminate()
        for proc in procs:
            try:
                proc.wait(timeout=5)
            except subprocess.TimeoutExpired:
                proc.kill()
        (out / "driver.json").write_text(json.dumps({"status": status, "log": log}, indent=2) + "\n")
    return 0


# ---- Outer harness -----------------------------------------------------------

def run_case(case, args, out):
    case_dir = out / case
    shutil.rmtree(case_dir, ignore_errors=True)  # judge only this run's files
    author = case_dir / "author"
    author.mkdir(parents=True, exist_ok=True)
    vmf_name = "ui_" + case.replace("-", "_") + ".vmf"
    # The editor opens a saved, empty map so F9 has a path: the same command
    # layer, from the headless host.
    script = author / "new.hcmd"
    script.write_text("new_map\nsave path=%s\n" % vmf_name)
    created = subprocess.run([str(args.cli), "--script", str(script), "--root", str(author)],
                             capture_output=True, text=True)
    if created.returncode:
        return {"driver": "hammer_cli failed: " + created.stderr.strip()}, {}
    started = time.monotonic()
    display = "hammer-ui-%d-%s" % (os.getpid(), case)
    session = subprocess.run(
        ["dbus-run-session", "--", "mutter", "--headless", "--virtual-monitor",
         "%dx%d" % SCREEN, "--wayland-display", display, "--",
         sys.executable, str(Path(__file__).resolve()), "--inner", "--case", case,
         "--case-dir", str(case_dir), "--vmf-name", vmf_name, "--gtk", str(args.gtk.resolve())],
        capture_output=True, text=True, timeout=args.timeout)
    (case_dir / "session.log").write_text(session.stdout + session.stderr)
    driver = json.loads((case_dir / "driver.json").read_text()) if (case_dir / "driver.json").is_file() \
        else {"status": "no driver record (session exit %d)" % session.returncode}
    stem = vmf_name[:-4]
    verdict = judge(author / vmf_name, case_dir / "builds" / stem / "build.json")
    verdict.update(judge_frames(case_dir / "frames"))
    verdict.update(judge_frame_path(case_dir / "hammer_gtk.log"))
    driver["elapsed_seconds"] = round(time.monotonic() - started, 2)
    return driver, verdict


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--gtk", type=Path, help="a built hammer_gtk (default: build one)")
    parser.add_argument("--cli", type=Path)
    parser.add_argument("--out", type=Path, default=ROOT / "quality-results/hammer-ui")
    parser.add_argument("--case", choices=CASES, action="append",
                        help="run only these cases (default: all)")
    parser.add_argument("--timeout", type=float, default=600)
    parser.add_argument("--inner", action="store_true", help=argparse.SUPPRESS)
    parser.add_argument("--case-dir", help=argparse.SUPPRESS)
    parser.add_argument("--vmf-name", help=argparse.SUPPRESS)
    args = parser.parse_args()
    if args.inner:
        args.case = args.case[0]
        return inner(args)
    if args.cli is None:
        parser.error("--cli is required")

    checks = Checks()
    out = args.out.resolve()
    if args.gtk is None:
        out.mkdir(parents=True, exist_ok=True)
        args.gtk = out / "hammer_gtk"
        built = subprocess.run([str(ROOT / "hammer/gtk/build.sh"), str(args.gtk)],
                               capture_output=True, text=True)
        (out / "build-gtk.log").write_text(built.stdout + built.stderr)
        checks.equal(built.returncode, 0, "shell.built")
        if built.returncode:
            return checks.report()
    summary = {"schema": "hammer-ui/v1", "cases": {}}
    for case in args.case or CASES:
        driver, verdict = run_case(case, args, out)
        summary["cases"][case] = {"driver": driver,
                                  "verdict": {k: {"ok": ok, "detail": detail}
                                              for k, (ok, detail) in verdict.items()}}
        if case == "room":
            checks.equal(driver.get("status"), "pass", "room.driven")
            for name, (ok, detail) in verdict.items():
                checks.check(ok, "room." + name, detail)
        else:
            # The control must still be driven to the end (so its outputs exist),
            # and the oracle must reject exactly the step it leaves out.
            target = {"no-hollow": "walls", "no-light": "light"}[case]
            checks.equal(driver.get("status"), "pass", case + ".driven")
            ok, detail = verdict.get(target, (True, "not judged"))
            checks.check(not ok, case + ".rejected", "%s: %s" % (target, detail))
    out.mkdir(parents=True, exist_ok=True)
    (out / "hammer-ui.json").write_text(json.dumps(summary, indent=2) + "\n")
    return checks.report()


if __name__ == "__main__":
    sys.exit(main())
