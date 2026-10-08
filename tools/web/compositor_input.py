#!/usr/bin/env python3
"""Plays input into the web product through its compositor (RFC 0029 W5).

    compositor_input.py <script>

where <script> is `<at s>:<action>[:<args>]`, comma-separated, times from
now. Actions: `click` (left button at the screen's centre: the page's canvas
takes it, and with it pointer lock, as a user's click), `look:<s>:<dx>`
(relative motion, dx per 1/60 s for s seconds), `key:<evdev code>:<s>` (a key
held for s seconds; W is 17, S 31, Escape 1).

The input goes through mutter's RemoteDesktop API on the display session's
own bus (found in the browser's environment: kiln's private session runs
the browser under its own dbus-run-session), so the browser sees trusted
events, which pointer lock and the page's focus need; a page's synthetic
events cannot grant either.
"""

import os
import sys
import time
from pathlib import Path


def session_bus_address():
    """The private display session's D-Bus: the browser's own address."""
    for proc in Path("/proc").iterdir():
        if not proc.name.isdigit():
            continue
        try:
            environ = (proc / "environ").read_bytes().split(b"\0")
            command = (proc / "cmdline").read_bytes()
        except OSError:
            continue
        values = dict(item.split(b"=", 1) for item in environ if b"=" in item)
        display = values.get(b"WAYLAND_DISPLAY", b"")
        if (b"firefox" in command or b"chrome" in command) and display.startswith(
                os.environ.get("COMPOSITOR_INPUT_DISPLAY", "kiln-").encode()):
            address = values.get(b"DBUS_SESSION_BUS_ADDRESS")
            if address:
                return address.decode()
    return None


class Remote:
    BTN_LEFT = 0x110

    def __init__(self, address):
        from gi.repository import Gio
        self.Gio = Gio
        flags = (Gio.DBusConnectionFlags.AUTHENTICATION_CLIENT |
                 Gio.DBusConnectionFlags.MESSAGE_BUS_CONNECTION)
        self.bus = Gio.DBusConnection.new_for_address_sync(address, flags, None, None)
        self.session = None
        self.session = self.call("CreateSession").unpack()[0]
        self.call("Start")

    def call(self, method, signature=None, *values):
        from gi.repository import GLib
        return self.bus.call_sync(
            "org.gnome.Mutter.RemoteDesktop", self.session or "/org/gnome/Mutter/RemoteDesktop",
            "org.gnome.Mutter.RemoteDesktop" + (".Session" if self.session else ""), method,
            GLib.Variant(signature, values) if signature else None, None,
            self.Gio.DBusCallFlags.NONE, -1, None)

    def motion(self, dx, dy):
        self.call("NotifyPointerMotionRelative", "(dd)", float(dx), float(dy))

    def button(self, down):
        self.call("NotifyPointerButton", "(ib)", self.BTN_LEFT, down)

    def key(self, code, down):
        self.call("NotifyKeyboardKeycode", "(ub)", int(code), down)


def main(argv):
    if len(argv) != 2:
        print(__doc__, file=sys.stderr)
        return 2
    address = None
    for _ in range(100):
        address = session_bus_address()
        if address:
            break
        time.sleep(0.2)
    if not address:
        print("compositor_input: no browser in a kiln display session", file=sys.stderr)
        return 1
    remote = Remote(address)
    start = time.monotonic()
    steps = sorted((float(s.split(":")[0]), s.split(":")[1:]) for s in argv[1].split(",") if s)
    for at, (action, *args) in steps:
        time.sleep(max(0.0, start + at - time.monotonic()))
        if action == "click":
            # Home against the top-left corner, then to the centre of the
            # session's 1920x1080 virtual monitor.
            remote.motion(-10000, -10000)
            remote.motion(960, 540)
            remote.button(True)
            time.sleep(0.05)
            remote.button(False)
        elif action == "look":
            seconds, dx = float(args[0]), float(args[1])
            for _ in range(int(seconds * 60)):
                remote.motion(dx, 0)
                time.sleep(1 / 60)
        elif action == "key":
            code, seconds = int(args[0]), float(args[1])
            remote.key(code, True)
            time.sleep(seconds)
            remote.key(code, False)
        print("compositor_input: %.1f s %s %s" % (time.monotonic() - start, action,
              ":".join(args)), file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
