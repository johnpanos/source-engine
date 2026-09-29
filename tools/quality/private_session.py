"""Private D-Bus sessions for the isolated headless compositors.

`dbus-run-session` starts a bus that still shares the login session's
XDG_RUNTIME_DIR. If anything on that bus (GTK, mutter, xdg-desktop-portal)
activates the Flatpak document portal, a second xdg-document-portal claims
$XDG_RUNTIME_DIR/doc, and when the private session ends it unmounts the login
session's FUSE mount while the login session's portal keeps running. After that
every Flatpak app fails with "bwrap: Can't find source path
/run/user/<uid>/doc/by-app/<app>" until the portal is restarted.

`dbus_run_session(directory)` returns the argv prefix for a private bus whose
configuration shadows the portal's activation file, so it cannot start. Every
harness that runs `dbus-run-session` must use it.
"""

from pathlib import Path

# Services a private session must never start: each one owns state in the
# shared runtime directory.
BLOCKED_SERVICES = ("org.freedesktop.portal.Documents",)

SESSION_CONFIGS = (Path("/usr/share/dbus-1/session.conf"), Path("/etc/dbus-1/session.conf"))


def session_config():
    for path in SESSION_CONFIGS:
        if path.is_file():
            return path
    raise FileNotFoundError("no D-Bus session.conf in %s" % ", ".join(map(str, SESSION_CONFIGS)))


def write_config(directory):
    """Writes the private bus configuration under `directory`; returns its path."""
    directory = Path(directory).resolve()
    services = directory / "services"
    services.mkdir(parents=True, exist_ok=True)
    for name in BLOCKED_SERVICES:
        # The first <servicedir> wins, so this shadows the system file; the
        # activation fails and nothing claims the shared mount.
        (services / (name + ".service")).write_text(
            "[D-BUS Service]\nName=%s\nExec=/bin/false\n" % name)
    config = directory / "session.conf"
    config.write_text(
        '<!DOCTYPE busconfig PUBLIC "-//freedesktop//DTD D-Bus Bus Configuration 1.0//EN"\n'
        ' "http://www.freedesktop.org/standards/dbus/1.0/busconfig.dtd">\n'
        "<busconfig>\n"
        "  <servicedir>%s</servicedir>\n"
        "  <include>%s</include>\n"
        "</busconfig>\n" % (services, session_config()))
    return config


def dbus_run_session(directory):
    """The argv prefix for a private bus; `directory` receives its config."""
    return ["dbus-run-session", "--config-file=%s" % write_config(directory), "--"]
