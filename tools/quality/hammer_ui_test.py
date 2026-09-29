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

The viewport case (RFC 0002 R17) opens the sample room with a VPK of two
generated VTF textures mounted and judges the viewports as a user sees them:
the camera view is textured; each frame's size is the view's logical size
times the window's scale; F12 writes view captures equal to the frames shown;
Ctrl+Shift+R (reload assets, three times while frames are in flight) and a
dragged view divider (a resize) each bring the views back, textured and at the
new size; Ctrl+Q closes the editor, which exits cleanly and reports no device
resource left by the viewports.

The properties case (RFC 0002, the Object Properties window) opens two lights
made by the command layer, with an FGD of typed keys (--fgd): Ctrl+A, Alt+Enter,
then in the window it records that the lights' differing _distance shows as
"(different values)", types a new value into that field and presses Enter
(apply), and toggles the mixed "Initially dark" flag (an inconsistent check
box). Back in the editor it saves, undoes, saves, undoes and saves. The oracle
judges the saved maps: both lights edited; one undo takes back only the flag on
both, a second the value on both (each edit was one undo step). Its control,
properties-cancel, presses Cancel instead of Enter; the distance check must
fail. Both place the pointer in the properties window from its X11 origin, so
the Wayland backend runs them only when named.

The visgroups case (RFC 0018 F6, the Visgroups panel) opens two blocks made by
the command layer, a red one on the left and a green one on the right already
in the file's visgroup "Other", with the viewport case's VPK mounted. It clicks
the red block's centre in the top view, presses New Visgroup (a visgroup
holding the selection, named "1 object"), clicks the new visgroup's check box
to hide it and again to show it, and presses Ctrl+Z three times, saving after
each stage. The oracle judges the frames and the saved maps: hidden, the red
block's pixels are gone from the camera and top frames while the green block's
stay; shown, they return; the hidden map lists the visgroup with the red solid
in it and "visgroupshown" "0", which survives the command layer opening and
saving it again; three undos leave only "Other" in the map and in the panel.
Its control, visgroups-other, hides "Other" instead (hidden.camera must fail).

--backend runs GTK as an X11 client (under Xwayland) or a Wayland client of
the private compositor, and --scale sets the display scale: GDK_SCALE for X11
(integers), the compositor's monitor scale for Wayland (fractional, through
the private session's keyfile settings, never the user's).

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
import launch_sandbox  # noqa: E402
import private_session  # noqa: E402

ROOT = HERE.parents[1]
SCREEN = (1280, 800)
CASES = ("room", "no-hollow", "no-light", "viewport", "properties", "properties-cancel",
         "visgroups", "visgroups-other")
# Cases that place the pointer in a second toplevel from its X11 origin; the
# Wayland backend runs them only when named.
X11_ONLY_CASES = ("properties", "properties-cancel")
BACKENDS = ("x11", "wayland")
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


def judge_viewport(case_dir, facts, args):
    """Check names -> (ok, detail) for the viewport case (R17)."""
    results = {}
    first = facts.get("first", {})
    results["textured"] = (first.get("textured", 0) >= 0.02,
                           "%.1f%% of the camera frame shows a texture colour"
                           % (100 * first.get("textured", 0)))
    # Frames are the view's logical size times the window's scale.
    scale = facts.get("monitor_scale", args.scale)

    def sized(entry):
        (fw, fh), (lw, lh) = entry.get("frame", [0, 0]), entry.get("logical", [0, 0])
        return (lw > 0 and abs(fw - lw * scale) <= 2 and abs(fh - lh * scale) <= 2,
                "%dx%d frame for a %dx%d view at scale %s" % (fw, fh, lw, lh, scale))
    results["pixel-size"] = sized(first)
    w, h, capture = read_png(case_dir / "captures" / "ui_viewport-camera.png")
    shown = [read_ppm(case_dir / name) for name in ("camera-before-capture.ppm",
                                                    "camera-after-capture.ppm")]
    results["capture"] = (bool(capture) and any((w, h, capture) == frame for frame in shown),
                          "%dx%d capture against the frames shown %s"
                          % (w, h, [(fw, fh) for fw, fh, _ in shown]))
    reload = facts.get("reload", {})
    results["reload-restored"] = (reload.get("new_frame", False) and reload.get("textured", 0) >= 0.02,
                                  "after three reloads: %s" % reload)
    resize = facts.get("resize", {})
    ok, detail = sized(resize)
    results["resize-restored"] = (ok and resize.get("textured", 0) >= 0.02, "after a resize: " + detail)
    log = (case_dir / "hammer_gtk.log").read_text(errors="replace") \
        if (case_dir / "hammer_gtk.log").is_file() else ""
    left = re.search(r"render teardown: (\d+) device resource", log)
    results["teardown"] = (facts.get("exit") == 0 and left is not None and left.group(1) == "0",
                           "exit %s, %s left" % (facts.get("exit"), left.group(1) if left else "none reported"))
    return results


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


# ---- Viewport case: assets and image files ----------------------------------

# The sample room's two materials, each a flat VTF colour the frames can find.
VIEW_MATERIALS = {"dev/dev_measurewall01a": (200, 40, 40), "dev/dev_measuregeneric01b": (40, 170, 60)}


def vtf_rgba8888(width, height, rgb):
    """A VTF 7.2 file of one RGBA8888 mip, no low-res image."""
    import struct
    header = bytearray(0x50)
    header[0:4] = b"VTF\0"
    struct.pack_into("<IIIHHIHH", header, 4, 7, 2, 0x50, width, height, 0, 1, 0)
    struct.pack_into("<f", header, 0x30, 1.0)
    struct.pack_into("<iBiBBH", header, 0x34, 0, 1, -1, 0, 0, 1)
    return bytes(header) + bytes(rgb + (255,)) * (width * height)


def build_view_vpk():
    """A _dir.vpk (version 2, everything in the directory file) holding the
    sample room's materials and their VTFs."""
    import struct
    from collections import defaultdict
    files = {}
    for name, rgb in VIEW_MATERIALS.items():
        files["materials/%s.vmt" % name] = ('"LightmappedGeneric" { "$basetexture" "%s" }' % name).encode()
        files["materials/%s.vtf" % name] = vtf_rgba8888(16, 16, rgb)
    branches = defaultdict(lambda: defaultdict(list))
    for path, data in files.items():
        stem, extension = path.rsplit(".", 1)
        directory, name = stem.rsplit("/", 1)
        branches[extension][directory].append((name, data))
    tree, payload = bytearray(), bytearray()
    for extension, directories in sorted(branches.items()):
        tree += extension.encode() + b"\0"
        for directory, entries in sorted(directories.items()):
            tree += directory.encode() + b"\0"
            for name, data in sorted(entries):
                tree += name.encode() + b"\0"
                tree += struct.pack("<IHHIIH", 0, 0, 0x7FFF, len(payload), len(data), 0xFFFF)
                payload += data
            tree += b"\0"
        tree += b"\0"
    tree += b"\0"
    return struct.pack("<IIIIIII", 0x55AA1234, 2, len(tree), len(payload), 0, 0, 0) + tree + payload


def read_png(path):
    """(width, height, RGB bytes) of an 8-bit RGB or RGBA, non-interlaced PNG."""
    import struct
    import zlib
    data = path.read_bytes() if path.is_file() else b""
    if data[:8] != b"\x89PNG\r\n\x1a\n":
        return 0, 0, b""
    i, idat, width, height, colour = 8, bytearray(), 0, 0, 0
    while i < len(data):
        length, kind = struct.unpack(">I4s", data[i:i + 8])
        body = data[i + 8:i + 8 + length]
        if kind == b"IHDR":
            width, height, depth, colour, _, _, interlace = struct.unpack(">IIBBBBB", body)
            if depth != 8 or colour not in (2, 6) or interlace:
                return 0, 0, b""
        elif kind == b"IDAT":
            idat += body
        i += 12 + length
    raw = zlib.decompress(bytes(idat))
    channels = 4 if colour == 6 else 3
    stride = width * channels
    rows, previous = [], bytearray(stride)
    for y in range(height):
        kind, line = raw[y * (stride + 1)], bytearray(raw[y * (stride + 1) + 1:(y + 1) * (stride + 1)])
        for x in range(stride):
            a = line[x - channels] if x >= channels else 0
            b = previous[x]
            c = previous[x - channels] if x >= channels else 0
            if kind == 1:
                line[x] = (line[x] + a) & 0xFF
            elif kind == 2:
                line[x] = (line[x] + b) & 0xFF
            elif kind == 3:
                line[x] = (line[x] + (a + b) // 2) & 0xFF
            elif kind == 4:
                p = a + b - c
                pa, pb, pc = abs(p - a), abs(p - b), abs(p - c)
                line[x] = (line[x] + (a if pa <= pb and pa <= pc else b if pb <= pc else c)) & 0xFF
        rows.append(bytes(line))
        previous = line
    rgb = bytearray()
    for line in rows:
        for x in range(width):
            rgb += line[x * channels:x * channels + 3]
    return width, height, bytes(rgb)


def colour_share(rgb, colour, tolerance=60):
    """The share of pixels near a texture colour (any shade of it: the
    camera view modulates textures by its shading)."""
    count = len(rgb) // 3
    if not count:
        return 0.0
    r0, g0, b0 = colour
    hits = 0
    for i in range(0, len(rgb) - 2, 3):
        r, g, b = rgb[i], rgb[i + 1], rgb[i + 2]
        peak = max(r, g, b)
        if peak < 40:
            continue
        scale = max(r0, g0, b0) / float(peak)
        if abs(r * scale - r0) + abs(g * scale - g0) + abs(b * scale - b0) < tolerance:
            hits += 1
    return hits / float(count)


# ---- Driver (runs inside the private compositor session) ---------------------

class Driver:
    def __init__(self, log):
        import gi
        gi.require_version("Atspi", "2.0")
        from gi.repository import Atspi
        self.Atspi = Atspi
        self.log = log
        self.app = None
        # Screen pixels per AT-SPI (logical) unit: an X11 client under
        # GDK_SCALE takes pointer input in device pixels.
        self.pointer_scale = 1.0

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

    def wait_active(self, timeout=5.0, required=True):
        """Waits until the editor's window is the active (keyboard-focused) one.
        A Wayland client's toplevel never reports ACTIVE over AT-SPI here
        (GTK 4.22 under mutter 50), although it has keyboard focus; with
        required=False the wait ends there and each key is judged by its own
        effect instead."""
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            frame = self.find(lambda n, r: r == "frame" and (n or "").startswith("Hammer - "))
            if frame is not None and frame.get_state_set().contains(self.Atspi.StateType.ACTIVE):
                self.note("window active")
                return
            time.sleep(0.1)
        frames = [(n, [s.value_nick for s in node.get_state_set().get_states()])
                  for node, n, r in self.nodes() if r == "frame" and (n or "").startswith("Hammer")]
        if not required:
            self.note("no ACTIVE state reported: %s" % frames)
            return
        raise RuntimeError("the editor window never became active: %s" % frames)

    def extents(self, node):
        e = node.get_extents(self.Atspi.CoordType.WINDOW)
        return e.x, e.y, e.width, e.height

    def press(self, node):
        node.do_action(0)
        time.sleep(0.2)

    # Linux input-event codes (the compositor takes evdev keycodes).
    KEYS = {"Shift": 42, "[": 26, "f": 33, "Return": 28, "Down": 108, "F9": 67, "F12": 88,
            "Control": 29, "r": 19, "q": 16}
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

    def chord(self, *names):
        """Presses the keys in order and releases them in reverse (Ctrl+Shift+R)."""
        for name in names:
            self.remote("NotifyKeyboardKeycode", "(ub)", self.KEYS[name], True)
            time.sleep(0.05)
        for name in reversed(names):
            self.remote("NotifyKeyboardKeycode", "(ub)", self.KEYS[name], False)
            time.sleep(0.05)
        time.sleep(0.2)

    def move(self, x, y):
        """Pointer to screen (x, y): home against the top-left corner, then move
        by the offset (the session has no absolute stream without a screencast)."""
        self.remote("NotifyPointerMotionRelative", "(dd)", -10000.0, -10000.0)
        self.remote("NotifyPointerMotionRelative", "(dd)", float(round(x * self.pointer_scale)),
                    float(round(y * self.pointer_scale)))
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
            self.remote("NotifyPointerMotionRelative", "(dd)", float((nx - px) * self.pointer_scale),
                        float((ny - py) * self.pointer_scale))
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


def drive(case, backend, log):
    """The user steps for one case. Raises on a UI that does not respond."""
    d = Driver(log)
    d.start_input()
    if not d.find_app():
        raise RuntimeError("hammer_gtk did not appear on the accessibility bus")
    d.note("editor up")
    top_view = d.named("top (x/y)", "frame")
    tx, ty, tw, th = d.extents(top_view)
    d.click(tx + tw / 2, ty + th / 2)  # focus the window (the Select tool ignores an empty click)
    d.wait_active(required=backend == "x11")
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


def frame_state(frames):
    """(mtime_ns, width, height, rgb) of the camera frame the editor last showed."""
    path = frames / "camera.ppm"
    try:
        stamp = path.stat().st_mtime_ns
    except OSError:
        return 0, 0, 0, b""
    width, height, rgb = read_ppm(path)
    return stamp, width, height, rgb


def textured(rgb):
    return max(colour_share(rgb, c) for c in VIEW_MATERIALS.values())


def wait_frame(frames, after=0, predicate=lambda state: True, timeout=20.0):
    """The first camera frame newer than 'after' for which predicate holds."""
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        state = frame_state(frames)
        if state[0] > after and state[1] and predicate(state):
            return state
        time.sleep(0.2)
    return None


def drive_viewport(case_dir, backend, scale, log, facts):
    """The viewport steps (R17): records what the judge needs in 'facts'."""
    frames = case_dir / "frames"
    d = Driver(log)
    d.pointer_scale = scale if backend == "x11" else 1.0
    d.start_input()
    if not d.find_app():
        raise RuntimeError("hammer_gtk did not appear on the accessibility bus")
    camera = d.named("camera", "frame")
    tx, ty, tw, th = d.extents(d.named("top (x/y)", "frame"))
    d.click(tx + tw / 2, ty + th / 2)
    d.wait_active(required=backend == "x11")
    d.key("Shift")

    first = wait_frame(frames, predicate=lambda s: textured(s[3]) >= 0.02)
    if first is None:
        raise RuntimeError("no textured camera frame")
    _, _, cw, ch = d.extents(camera)
    facts["first"] = {"frame": [first[1], first[2]], "logical": [cw, ch],
                      "textured": round(textured(first[3]), 4)}
    d.note("textured frame %dx%d for a %dx%d view" % (first[1], first[2], cw, ch))

    # Capture: F12 writes each view's current frame.
    (case_dir / "captures").mkdir(exist_ok=True)
    # The capture must equal the frame shown when F12 was pressed: the one
    # before, or one that landed while the capture ran.
    (case_dir / "camera-before-capture.ppm").write_bytes((frames / "camera.ppm").read_bytes())
    d.key("F12")
    deadline = time.monotonic() + 5
    capture = case_dir / "captures" / "ui_viewport-camera.png"
    while not capture.is_file() and time.monotonic() < deadline:
        time.sleep(0.1)
    time.sleep(0.3)
    (case_dir / "camera-after-capture.ppm").write_bytes((frames / "camera.ppm").read_bytes())
    d.note("capture %s" % capture.is_file())

    # Reload the mounted assets three times while frames are in flight.
    before = frame_state(frames)[0]
    for _ in range(3):
        d.chord("Control", "Shift", "r")
    reloaded = wait_frame(frames, after=before, predicate=lambda s: textured(s[3]) >= 0.02)
    facts["reload"] = {"new_frame": reloaded is not None,
                       "textured": round(textured(reloaded[3]), 4) if reloaded else 0.0}
    d.note("reload -> %s" % facts["reload"])

    # Resize: drag the divider between the camera and top views to the left.
    cx, cy, cw, ch = d.extents(camera)
    tx, ty, tw, th = d.extents(d.named("top (x/y)", "frame"))
    divider = ((cx + cw + tx) / 2.0, cy + ch / 2.0)
    before = frame_state(frames)[0]
    d.drag(divider[0], divider[1], divider[0] - 120, divider[1])
    time.sleep(0.5)
    resized = wait_frame(frames, after=before,
                         predicate=lambda s: abs(s[1] - first[1]) > 20 and textured(s[3]) >= 0.02)
    _, _, nw, nh = d.extents(camera)
    facts["resize"] = {"frame": [resized[1], resized[2]] if resized else [0, 0],
                       "logical": [nw, nh],
                       "textured": round(textured(resized[3]), 4) if resized else 0.0}
    d.note("resize -> %s" % facts["resize"])

    d.chord("Control", "q")
    d.note("quit")


def apply_monitor_scale(scale):
    """Sets the private compositor's monitor to the supported scale nearest
    'scale' (org.gnome.Mutter.DisplayConfig); returns the scale applied."""
    from gi.repository import Gio, GLib
    bus = Gio.bus_get_sync(Gio.BusType.SESSION)

    def call(method, args=None):
        return bus.call_sync("org.gnome.Mutter.DisplayConfig", "/org/gnome/Mutter/DisplayConfig",
                             "org.gnome.Mutter.DisplayConfig", method, args, None,
                             Gio.DBusCallFlags.NONE, -1, None).unpack()
    serial, monitors, _, _ = call("GetCurrentState")
    (connector, _, _, _), modes, _ = monitors[0]
    mode = next(m for m in modes if m[6].get("is-current", False))
    chosen = min(mode[5], key=lambda s: abs(s - scale))
    logical = [(0, 0, chosen, 0, True, [(connector, mode[0], {})])]
    call("ApplyMonitorsConfig", GLib.Variant("(uua(iiduba(ssa{sv}))a{sv})",
                                             (serial, 1, logical, {})))
    time.sleep(1.0)
    return chosen


def inner(args):
    """Inside the compositor: start the accessibility bus, the editor and the
    driver; write the driver log."""
    out = Path(args.case_dir)
    procs = []
    log = []
    facts = {}
    status = "pass"
    try:
        for command in (["/usr/libexec/at-spi-bus-launcher", "--launch-immediately"],
                        ["/usr/libexec/at-spi2-registryd", "--use-gnome-session=false"]):
            procs.append(subprocess.Popen(command, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL))
            time.sleep(1)
        frames = out / "frames"
        frames.mkdir(parents=True, exist_ok=True)
        env = dict(os.environ, GDK_BACKEND=args.backend, GTK_CSD="1", GTK_A11Y="atspi",
                   HAMMER_GTK_FRAME_DIR=str(frames),
                   HAMMER_GTK_CAPTURE_DIR=str(out / "captures"))
        if args.backend == "x11" and args.scale != 1:
            env["GDK_SCALE"] = str(int(round(args.scale)))
        if args.backend == "wayland" and args.scale != 1:
            facts["monitor_scale"] = apply_monitor_scale(args.scale)
        command = [args.gtk, "--maximized", "--open", str(out / "author" / args.vmf_name),
                   "--builds", str(out / "builds"), "--no-publish"]
        if args.case == "viewport" or args.case.startswith("visgroups"):
            command += ["--mount", str(out / "author" / "viewport_dir.vpk")]
        if args.case.startswith("properties"):
            command += ["--fgd", str(out / "author" / "properties.fgd")]
        app_log = open(out / "hammer_gtk.log", "w")
        app = subprocess.Popen(command, cwd=str(ROOT), env=env, stdout=app_log,
                               stderr=subprocess.STDOUT)
        procs.append(app)
        if args.case == "viewport":
            drive_viewport(out, args.backend, args.scale, log, facts)
            try:
                facts["exit"] = app.wait(timeout=20)
            except subprocess.TimeoutExpired:
                facts["exit"] = "still running"
        elif args.case.startswith("properties"):
            drive_properties(args.case, args.backend, out, out / "author" / args.vmf_name, log,
                             facts)
        elif args.case.startswith("visgroups"):
            drive_visgroups(args.case, args.backend, out, out / "author" / args.vmf_name, log,
                            facts)
        else:
            drive(args.case, args.backend, log)
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
        (out / "driver.json").write_text(
            json.dumps({"status": status, "log": log, "facts": facts}, indent=2) + "\n")
    return 0


# ---- Properties case: the Object Properties window (RFC 0002) -------------------

# The case's entity schema, given to the editor with --fgd: a light with a
# typed key of each kind the window edits.
PROPERTIES_FGD = """@SolidClass = worldspawn : "World" [ skyname(string) : "Sky" : "sky_day01_01" ]
@PointClass = light : "A light"
[
    targetname(target_source) : "Name"
    _light(color255) : "Brightness" : "255 255 255 200"
    _distance(integer) : "Maximum distance" : 0
    style(choices) : "Appearance" : 0 = [ 0 : "Normal" 10 : "Fluorescent flicker" ]
    spawnflags(flags) = [ 1 : "Initially dark" : 0 ]
]
@PointClass = info_player_start : "Player start" []
"""
# Two lights that differ in _distance and in the "Initially dark" flag, made
# by the command layer (the headless host), and nothing selected.
PROPERTIES_LIGHTS = (("-64 0 32", "100", "1"), ("64 0 32", "200", "0"))
PROPERTIES_DISTANCE = "256"
# evdev codes of the keys this case types (Driver.KEYS is the room case's).
PROPERTIES_KEYS = {"Shift": 42, "Control": 29, "Alt": 56, "Return": 28, "a": 30, "s": 31,
                   "z": 44, "2": 3, "5": 6, "6": 7}


def properties_script(vmf_name):
    lines = ["new_map"]
    for origin, distance, flags in PROPERTIES_LIGHTS:
        lines += ['place_entity classname=light origin="%s"' % origin,
                  "set_key key=_distance value=%s" % distance,
                  "set_key key=spawnflags value=%s" % flags]
    return "\n".join(lines + ["select_none", "save path=%s" % vmf_name]) + "\n"


def light_keys(path):
    """(origin, _distance, spawnflags) of each light in a saved VMF, by origin."""
    text = path.read_text() if path.is_file() else ""
    lights = [(keyvalue(body, "origin"), keyvalue(body, "_distance"), keyvalue(body, "spawnflags"))
              for name, body in vmf_blocks(text)
              if name == "entity" and keyvalue(body, "classname") == "light"]
    return sorted(lights)


def judge_properties(case_dir, facts):
    """The saved maps after each step: the edit (both lights), then one undo
    per step. Check names -> (ok, detail)."""
    saves = case_dir / "saves"
    edited, undo1, undo2 = (light_keys(saves / n) for n in ("edited.vmf", "undo1.vmf", "undo2.vmf"))
    origins = sorted(o for o, _, _ in PROPERTIES_LIGHTS)
    original = sorted(PROPERTIES_LIGHTS)
    results = {"saved": (len(edited) == 2 and [o for o, _, _ in edited] == origins,
                         "edited lights %s" % edited)}
    results["mixed.shown"] = (facts.get("distance_description") == "(different values)" and
                              facts.get("flag_indeterminate") is True,
                              "_distance %r, flag indeterminate %s" % (
                                  facts.get("distance_description"),
                                  facts.get("flag_indeterminate")))
    results["distance"] = (len(edited) == 2 and all(d == PROPERTIES_DISTANCE for _, d, _ in edited),
                           "edited _distance %s" % [d for _, d, _ in edited])
    results["flag"] = (len(edited) == 2 and all(f is not None and int(f) & 1 for _, _, f in edited),
                       "edited spawnflags %s" % [f for _, _, f in edited])
    # One undo takes back the flag toggle on both lights and nothing else; a
    # second takes back the value edit on both: each was one step.
    results["undo.flag"] = (
        [f for _, _, f in undo1] == [f for _, _, f in original] and
        [d for _, d, _ in undo1] == [d for _, d, _ in edited],
        "after one undo %s" % undo1)
    results["undo.distance"] = (undo2 == original, "after two undos %s, want %s" % (undo2, original))
    return results


def tap(d, *names):
    """Presses the keys in order and releases them in reverse."""
    for name in names:
        d.remote("NotifyKeyboardKeycode", "(ub)", PROPERTIES_KEYS[name], True)
        time.sleep(0.05)
    for name in reversed(names):
        d.remote("NotifyKeyboardKeycode", "(ub)", PROPERTIES_KEYS[name], False)
        time.sleep(0.05)
    time.sleep(0.25)


def x11_window_origin(title):
    """The screen origin of the client area of the X11 toplevel named 'title'
    (its GTK client-side frame extents excluded), or None."""
    from Xlib import X, display as xdisplay
    dpy = xdisplay.Display()
    root = dpy.screen().root
    name_atom, extents_atom = dpy.intern_atom("_NET_WM_NAME"), dpy.intern_atom("_GTK_FRAME_EXTENTS")
    stack = [root]
    while stack:
        window = stack.pop()
        try:
            name = window.get_full_property(name_atom, 0)
            if name is not None and name.value.decode("utf-8", "replace") == title:
                origin = window.translate_coords(root, 0, 0)
                frame = window.get_full_property(extents_atom, X.AnyPropertyType)
                left, _, top, _ = frame.value if frame is not None else (0, 0, 0, 0)
                return -origin.x + left, -origin.y + top
            stack.extend(window.query_tree().children)
        except Exception:
            continue
    return None


def focus_widget(d, node, title):
    """Clicks 'node' (a widget of the toplevel 'title') until it reports
    keyboard focus; its WINDOW extents are relative to that toplevel."""
    focused = d.Atspi.StateType.FOCUSED
    for _ in range(20):
        origin = x11_window_origin(title)
        if origin is not None:
            x, y, w, h = d.extents(node)
            d.click(origin[0] + x + min(w / 2, 40), origin[1] + y + h / 2)
            if node.get_state_set().contains(focused):
                return
        time.sleep(0.25)
    raise RuntimeError("could not focus %r in %r" % (node.get_name(), title))


def click_widget(d, node, title):
    """Clicks the middle of 'node', a widget of the toplevel 'title'."""
    origin = x11_window_origin(title)
    if origin is None:
        raise RuntimeError("no X11 window %r" % title)
    x, y, w, h = d.extents(node)
    d.click(origin[0] + x + w / 2, origin[1] + y + h / 2)


def wait_node(d, predicate, what, timeout=10.0):
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        node = d.find(predicate)
        if node is not None:
            return node
        time.sleep(0.2)
    raise RuntimeError("no " + what)


def save_copy(d, map_path, saves, name):
    """Ctrl+S in the editor, then a copy of the saved map once written."""
    before = map_path.stat().st_mtime_ns
    tap(d, "Control", "s")
    deadline = time.monotonic() + 10
    while map_path.stat().st_mtime_ns == before and time.monotonic() < deadline:
        time.sleep(0.1)
    if map_path.stat().st_mtime_ns == before:
        raise RuntimeError("Ctrl+S did not save for " + name)
    time.sleep(0.2)
    shutil.copyfile(map_path, saves / name)
    d.note("saved " + name)


def drive_properties(case, backend, case_dir, map_path, log, facts):
    """The Object Properties steps: select both lights (Ctrl+A), open the window
    (Alt+Enter), type a new _distance into the mixed field and apply it (the
    control cancels instead), toggle the mixed "Initially dark" flag, then save,
    undo, save, undo, save from the editor window."""
    if backend != "x11":
        raise RuntimeError("the properties case needs --backend x11: it places the pointer in "
                           "the properties window from its X11 origin")
    saves = case_dir / "saves"
    saves.mkdir(exist_ok=True)
    d = Driver(log)
    d.start_input()
    if not d.find_app():
        raise RuntimeError("hammer_gtk did not appear on the accessibility bus")
    tx, ty, tw, th = d.extents(d.named("top (x/y)", "frame"))
    corner = (tx + tw - 24, ty + 24)  # clear of the lights and of the properties window
    d.click(*corner)
    d.wait_active()
    tap(d, "Shift")
    tap(d, "Control", "a")
    d.wait_label(r".*·\s+2 selected$")
    tap(d, "Alt", "Return")
    wait_node(d, lambda n, r: r == "dialog" and n == "Object Properties", "Object Properties dialog")
    d.note("properties open")
    wait_node(d, lambda n, r: r == "label" and n == "2 entities", "2-entity summary")
    distance = wait_node(d, lambda n, r: n == "_distance" and r in ("entry", "text"),
                         "_distance field")
    facts["distance_description"] = distance.get_description()

    focus_widget(d, distance, "Object Properties")
    for digit in PROPERTIES_DISTANCE:
        tap(d, digit)
    wait_node(d, lambda n, r: r == "label" and n == "Maximum distance *", "drafted row")
    d.note("typed %s" % PROPERTIES_DISTANCE)
    if case == "properties-cancel":
        click_widget(d, wait_node(d, lambda n, r: n == "Cancel" and "button" in r, "Cancel button"),
                     "Object Properties")
    else:
        tap(d, "Return")  # Enter in a field applies, as the Apply button does
    wait_node(d, lambda n, r: r == "label" and n == "Maximum distance", "applied row")
    click_widget(d, d.named("Flags", "page tab"), "Object Properties")
    flag = wait_node(d, lambda n, r: r == "check box" and n == "Initially dark", "flag check box")
    facts["flag_indeterminate"] = flag.get_state_set().contains(d.Atspi.StateType.INDETERMINATE)
    click_widget(d, flag, "Object Properties")
    deadline = time.monotonic() + 5
    while flag.get_state_set().contains(d.Atspi.StateType.INDETERMINATE) and \
            time.monotonic() < deadline:
        time.sleep(0.1)
    d.note("flag pressed")

    d.click(*corner)  # back to the editor window
    d.wait_active()
    save_copy(d, map_path, saves, "edited.vmf")
    tap(d, "Control", "z")
    save_copy(d, map_path, saves, "undo1.vmf")
    tap(d, "Control", "z")
    save_copy(d, map_path, saves, "undo2.vmf")


# ---- Visgroups case: the Visgroups panel (RFC 0018 F6) --------------------------

# Two blocks made by the command layer: a red one on the left (x < 0) and a
# green one on the right, the green one already in the visgroup "Other". The
# case selects the red block, makes a visgroup from it with the panel's New
# button (legacy's default name "1 object"), hides and shows it with its check
# box, and undoes; its control hides "Other" instead.
VISGROUP_RED = "dev/dev_measurewall01a"
VISGROUP_GREEN = "dev/dev_measuregeneric01b"
VISGROUP_NEW = "1 object"
VISGROUP_OTHER = "Other"


def visgroups_script(vmf_name):
    return "\n".join([
        "new_map",
        'create_block mins="-192 -64 0" maxs="-64 64 128" material=%s' % VISGROUP_RED,
        'create_block mins="64 -64 0" maxs="192 64 128" material=%s' % VISGROUP_GREEN,
        "visgroup_create name=%s" % VISGROUP_OTHER,
        "visgroup_add visgroup=1",  # the green block, selected by its creation
        "select_none",
        "save path=%s" % vmf_name]) + "\n"


def vmf_visgroups(text):
    """{name: id} of every visgroup in a VMF, nested ones included."""
    found = {}

    def walk(body):
        for name, inner_body in vmf_blocks(body):
            if name == "visgroup":
                found[keyvalue(inner_body, "name")] = keyvalue(inner_body, "visgroupid")
                walk(inner_body)
    for name, body in vmf_blocks(text):
        if name == "visgroups":
            walk(body)
    return found


def vmf_solid_editors(text):
    """{material of its first side: its editor block's key values} per world solid."""
    world = next((body for name, body in vmf_blocks(text) if name == "world"), "")
    solids = {}
    for name, body in vmf_blocks(world):
        if name != "solid":
            continue
        blocks = vmf_blocks(body)
        side = next((b for n, b in blocks if n == "side"), "")
        editor = next((b for n, b in blocks if n == "editor"), "")
        solids[keyvalue(side, "material")] = dict(re.findall(r'"([^"]+)"\s+"([^"]*)"', editor))
    return solids


def visgroup_measure(frames):
    """What the live editor shows of each block: the camera frame's share of
    each block's texture colour, and the top frame's edge pixels in its left
    (red block) and right (green block) halves."""
    width, height, camera = read_ppm(frames / "camera.ppm")
    tw, th, top = read_ppm(frames / "top.ppm")
    left = right = 0
    for i in range(0, len(top) - 2, 3):
        if tuple(top[i:i + 3]) in EDGE_COLORS:
            if (i // 3) % tw < tw // 2:
                left += 1
            else:
                right += 1
    return {"red": round(colour_share(camera, VIEW_MATERIALS[VISGROUP_RED]), 4),
            "green": round(colour_share(camera, VIEW_MATERIALS[VISGROUP_GREEN]), 4),
            "left": left, "right": right, "camera": [width, height], "top": [tw, th]}


def visgroup_frames(case_dir, frames, stamp, name, predicate, timeout=10.0):
    """Waits for camera and top frames newer than 'stamp' that satisfy
    'predicate' (or the timeout), copies them to case_dir as name.*.ppm and
    returns their measurement."""
    deadline = time.monotonic() + timeout
    measure = {}
    while time.monotonic() < deadline:
        try:
            fresh = min((frames / v).stat().st_mtime_ns for v in ("camera.ppm", "top.ppm")) > stamp
        except OSError:
            fresh = False
        if fresh:
            time.sleep(0.3)  # a frame in flight at 'stamp' may land just after it
            measure = visgroup_measure(frames)
            if predicate(measure):
                break
        time.sleep(0.2)
    for view in ("camera", "top"):
        if (frames / (view + ".ppm")).is_file():
            shutil.copyfile(frames / (view + ".ppm"), case_dir / ("%s.%s.ppm" % (name, view)))
    return measure


def visgroup_check(d, name, timeout=10.0):
    return wait_node(d, lambda n, r: r == "check box" and n == name,
                     "visgroup check box %r" % name, timeout)


def toggle_visgroup(d, name, want):
    """Clicks a visgroup's check box (GTK check boxes offer no AT-SPI action)
    and waits for its state ('hidden' or 'shown') in its description."""
    x, y, w, h = d.extents(visgroup_check(d, name))
    d.click(x + w / 2, y + h / 2)
    deadline = time.monotonic() + 10
    while time.monotonic() < deadline:
        description = visgroup_check(d, name).get_description() or ""
        if description.endswith(want):
            return description
        time.sleep(0.2)
    raise RuntimeError("visgroup %r did not become %s" % (name, want))


def drive_visgroups(case, backend, case_dir, map_path, log, facts):
    """The Visgroups panel steps (RFC 0018 F6): select the red block in the top
    view, New Visgroup, hide it with its check box, show it again, then undo
    the three steps; saving a copy of the map after each stage."""
    frames = case_dir / "frames"
    saves = case_dir / "saves"
    saves.mkdir(exist_ok=True)
    d = Driver(log)
    d.start_input()
    if not d.find_app():
        raise RuntimeError("hammer_gtk did not appear on the accessibility bus")
    tx, ty, tw, th = d.extents(d.named("top (x/y)", "frame"))
    d.click(tx + tw / 2, ty + th / 2)  # empty: between the blocks
    d.wait_active(required=backend == "x11")
    tap(d, "Shift")
    facts["listed"] = sorted(n for _, n, r in d.nodes() if r == "check box" and n in
                             (VISGROUP_OTHER, VISGROUP_NEW))
    before = visgroup_frames(case_dir, frames, 0, "shown",
                             lambda m: m["red"] >= 0.01 and m["green"] >= 0.01, timeout=30)
    facts["shown"] = before
    d.note("shown: %s" % before)

    top = d.calibrate("top (x/y)")
    d.click(*top(-128, 0))  # the red block's centre handle
    d.wait_label(r".*·\s+1 selected$")
    d.press(d.named("New Visgroup", "button"))
    visgroup_check(d, VISGROUP_NEW)
    d.note("created %r" % VISGROUP_NEW)
    d.click(*top(0, 0))  # empty: clear the selection
    d.wait_label(r"^\d+ brush\(es\)$")

    target = VISGROUP_OTHER if case == "visgroups-other" else VISGROUP_NEW
    stamp = time.time_ns()
    facts["hidden_description"] = toggle_visgroup(d, target, "hidden")
    facts["hidden"] = visgroup_frames(case_dir, frames, stamp, "hidden",
                                      lambda m: m["red"] < 0.002 and m["left"] < 20)
    d.note("hidden %r: %s" % (target, facts["hidden"]))
    d.click(*top(0, 0))  # keys go to the editor window
    save_copy(d, map_path, saves, "hidden.vmf")

    stamp = time.time_ns()
    facts["shown_description"] = toggle_visgroup(d, target, "shown")
    facts["reshown"] = visgroup_frames(
        case_dir, frames, stamp, "reshown",
        lambda m: m["red"] >= before["red"] * 0.5 and m["left"] >= before["left"] * 0.5)
    d.note("shown again: %s" % facts["reshown"])
    d.click(*top(0, 0))
    save_copy(d, map_path, saves, "shown.vmf")

    for _ in range(3):  # show, hide, create
        tap(d, "Control", "z")
    deadline = time.monotonic() + 10
    while visgroup_check_present(d, VISGROUP_NEW) and time.monotonic() < deadline:
        time.sleep(0.2)
    facts["undo_removed_row"] = not visgroup_check_present(d, VISGROUP_NEW)
    facts["undo_kept_other"] = visgroup_check_present(d, VISGROUP_OTHER)
    save_copy(d, map_path, saves, "undone.vmf")


def visgroup_check_present(d, name):
    return d.find(lambda n, r: r == "check box" and n == name) is not None


def judge_visgroups(case_dir, facts, cli):
    """Check names -> (ok, detail) for the visgroups case and its control."""
    saves = case_dir / "saves"
    results = {}
    results["listed"] = (facts.get("listed") == [VISGROUP_OTHER],
                         "check boxes at open %s, want only the file's %r"
                         % (facts.get("listed"), VISGROUP_OTHER))
    shown, hidden, reshown = (facts.get(k, {}) for k in ("shown", "hidden", "reshown"))
    results["frames.before"] = (shown.get("red", 0) >= 0.01 and shown.get("green", 0) >= 0.01 and
                                shown.get("left", 0) >= 100 and shown.get("right", 0) >= 100,
                                "before hiding: %s" % shown)
    # Hidden: the red block's pixels are gone from both views; the green
    # block's stay.
    results["hidden.camera"] = (
        hidden.get("red", 1) < 0.002 and hidden.get("green", 0) >= 0.5 * shown.get("green", 0),
        "camera red %s (was %s), green %s (was %s)" % (hidden.get("red"), shown.get("red"),
                                                       hidden.get("green"), shown.get("green")))
    results["hidden.top"] = (
        hidden.get("left", 1 << 30) < 0.1 * max(shown.get("left", 0), 1) and
        hidden.get("right", 0) >= 0.5 * shown.get("right", 0),
        "top edge pixels left %s (was %s), right %s (was %s)"
        % (hidden.get("left"), shown.get("left"), hidden.get("right"), shown.get("right")))
    results["shown.again"] = (
        reshown.get("red", 0) >= 0.5 * shown.get("red", 1) and
        reshown.get("left", 0) >= 0.5 * shown.get("left", 1),
        "after showing: %s" % reshown)

    hidden_text = (saves / "hidden.vmf").read_text() if (saves / "hidden.vmf").is_file() else ""
    groups = vmf_visgroups(hidden_text)
    solids = vmf_solid_editors(hidden_text)
    red, green = solids.get(VISGROUP_RED, {}), solids.get(VISGROUP_GREEN, {})
    new_id = groups.get(VISGROUP_NEW)
    results["saved.visgroup"] = (
        new_id is not None and red.get("visgroupid") == new_id and
        green.get("visgroupid") == groups.get(VISGROUP_OTHER),
        "visgroups %s; red solid in %s, green in %s"
        % (groups, red.get("visgroupid"), green.get("visgroupid")))
    results["saved.hidden"] = (red.get("visgroupshown") == "0" and green.get("visgroupshown") == "1",
                               "visgroupshown red %s, green %s"
                               % (red.get("visgroupshown"), green.get("visgroupshown")))
    # The hidden state survives a reload: the command layer opens the saved map
    # and saves it again.
    reload_dir = case_dir / "reload"
    reload_dir.mkdir(exist_ok=True)
    if (saves / "hidden.vmf").is_file():
        shutil.copyfile(saves / "hidden.vmf", reload_dir / "hidden.vmf")
    (reload_dir / "reload.hcmd").write_text("open path=hidden.vmf\nsave path=reloaded.vmf\n")
    subprocess.run([str(cli), "--script", str(reload_dir / "reload.hcmd"), "--root", str(reload_dir)],
                   capture_output=True, text=True)
    reloaded = reload_dir / "reloaded.vmf"
    again = vmf_solid_editors(reloaded.read_text()) if reloaded.is_file() else {}
    results["saved.reloaded"] = (
        bool(again) and again.get(VISGROUP_RED, {}).get("visgroupshown") == "0" and
        again.get(VISGROUP_RED, {}).get("visgroupid") == new_id and
        vmf_visgroups(reloaded.read_text()) == groups,
        "after open and save: red %s" % again.get(VISGROUP_RED))

    shown_text = (saves / "shown.vmf").read_text() if (saves / "shown.vmf").is_file() else ""
    shown_red = vmf_solid_editors(shown_text).get(VISGROUP_RED, {})
    results["saved.shown"] = (shown_red.get("visgroupshown") == "1", "red after showing %s" % shown_red)

    undone_text = (saves / "undone.vmf").read_text() if (saves / "undone.vmf").is_file() else ""
    undone_groups = vmf_visgroups(undone_text)
    undone_red = vmf_solid_editors(undone_text).get(VISGROUP_RED, {})
    results["undo"] = (
        undone_groups == {VISGROUP_OTHER: "1"} and "visgroupid" not in undone_red and
        undone_red.get("visgroupshown") == "1" and facts.get("undo_removed_row") is True and
        facts.get("undo_kept_other") is True,
        "after three undos: visgroups %s, red %s, row removed %s, Other kept %s"
        % (undone_groups, undone_red, facts.get("undo_removed_row"), facts.get("undo_kept_other")))
    return results


# ---- Outer harness -----------------------------------------------------------

def run_case(case, args, out):
    case_dir = out / case
    shutil.rmtree(case_dir, ignore_errors=True)  # judge only this run's files
    author = case_dir / "author"
    author.mkdir(parents=True, exist_ok=True)
    vmf_name = "ui_" + case.replace("-", "_") + ".vmf"
    if case == "viewport":
        # The sample room with its two materials in a generated VPK.
        shutil.copyfile(ROOT / "hammer/gtk/samples/room.vmf", author / vmf_name)
        (author / "viewport_dir.vpk").write_bytes(build_view_vpk())
    elif case.startswith("visgroups"):
        # Two textured blocks and a visgroup from the command layer.
        (author / "viewport_dir.vpk").write_bytes(build_view_vpk())
        script = author / "visgroups.hcmd"
        script.write_text(visgroups_script(vmf_name))
        created = subprocess.run([str(args.cli), "--script", str(script), "--root", str(author)],
                                 capture_output=True, text=True)
        if created.returncode:
            return {"driver": "hammer_cli failed: " + created.stderr.strip()}, {}
    elif case.startswith("properties"):
        # Two lights from the command layer, and the schema the editor loads.
        (author / "properties.fgd").write_text(PROPERTIES_FGD)
        script = author / "lights.hcmd"
        script.write_text(properties_script(vmf_name))
        created = subprocess.run([str(args.cli), "--script", str(script), "--root", str(author)],
                                 capture_output=True, text=True)
        if created.returncode:
            return {"driver": "hammer_cli failed: " + created.stderr.strip()}, {}
    else:
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
    screen = SCREEN if args.scale < 1.5 else (SCREEN[0] * 2, SCREEN[1] * 2)
    # The compositor, the driver and the editor run with a throwaway HOME and
    # XDG directories (RFC 0005, launch sandbox). The session's settings live
    # in a keyfile under the sandbox's config directory, never the user's: it
    # enables fractional monitor scales for the Wayland scale runs.
    sandbox = launch_sandbox.Sandbox(case_dir / "sandbox")
    env = sandbox.environment(os.environ)
    keyfile = Path(env["XDG_CONFIG_HOME"]) / "glib-2.0" / "settings" / "keyfile"
    keyfile.parent.mkdir(parents=True, exist_ok=True)
    keyfile.write_text("[org/gnome/mutter]\nexperimental-features=['scale-monitor-framebuffer']\n")
    env["GSETTINGS_BACKEND"] = "keyfile"
    session = subprocess.run(
        private_session.dbus_run_session(case_dir / "dbus") +
        ["mutter", "--headless", "--virtual-monitor",
         "%dx%d" % screen, "--wayland-display", display, "--",
         sys.executable, str(Path(__file__).resolve()), "--inner", "--case", case,
         "--case-dir", str(case_dir), "--vmf-name", vmf_name, "--gtk", str(args.gtk.resolve()),
         "--backend", args.backend, "--scale", str(args.scale)],
        capture_output=True, text=True, timeout=args.timeout, env=env)
    (case_dir / "session.log").write_text(session.stdout + session.stderr)
    driver = json.loads((case_dir / "driver.json").read_text()) if (case_dir / "driver.json").is_file() \
        else {"status": "no driver record (session exit %d)" % session.returncode}
    driver["sandbox"] = sandbox.finish()
    stem = vmf_name[:-4]
    if case == "viewport":
        verdict = judge_viewport(case_dir, driver.get("facts", {}), args)
    elif case.startswith("properties"):
        verdict = judge_properties(case_dir, driver.get("facts", {}))
    elif case.startswith("visgroups"):
        verdict = judge_visgroups(case_dir, driver.get("facts", {}), args.cli)
    else:
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
    parser.add_argument("--backend", choices=BACKENDS, default="x11",
                        help="GTK's display backend inside the private compositor")
    parser.add_argument("--scale", type=float, default=1.0,
                        help="display scale (X11: integer GDK_SCALE; Wayland: monitor scale)")
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
    default = [c for c in CASES if args.backend == "x11" or c not in X11_ONLY_CASES]
    for case in args.case or default:
        driver, verdict = run_case(case, args, out)
        summary["cases"][case] = {"driver": driver,
                                  "verdict": {k: {"ok": ok, "detail": detail}
                                              for k, (ok, detail) in verdict.items()}}
        if case in ("room", "viewport", "properties", "visgroups"):
            checks.equal(driver.get("status"), "pass", case + ".driven")
            for name, (ok, detail) in verdict.items():
                checks.check(ok, case + "." + name, detail)
        else:
            # The control must still be driven to the end (so its outputs exist),
            # and the oracle must reject exactly the step it leaves out.
            target = {"no-hollow": "walls", "no-light": "light",
                      "properties-cancel": "distance", "visgroups-other": "hidden.camera"}[case]
            checks.equal(driver.get("status"), "pass", case + ".driven")
            ok, detail = verdict.get(target, (True, "not judged"))
            checks.check(not ok, case + ".rejected", "%s: %s" % (target, detail))
    out.mkdir(parents=True, exist_ok=True)
    (out / "hammer-ui.json").write_text(json.dumps(summary, indent=2) + "\n")
    return checks.report()


if __name__ == "__main__":
    sys.exit(main())
