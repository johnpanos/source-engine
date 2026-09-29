"""Private D-Bus sessions must not start the Flatpak document portal.

A second xdg-document-portal on a harness's private bus unmounts the login
session's $XDG_RUNTIME_DIR/doc when the harness exits, which breaks every
Flatpak app ("bwrap: Can't find source path .../doc/by-app/..."). The session
config from tools/quality/private_session.py blocks that activation, and no
first-party script may start a private bus without it.
"""

from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile
import unittest

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
sys.path.insert(0, str(HERE.parent))
import private_session  # noqa: E402

# A private bus started without the helper: `dbus-run-session --` in a shell
# script, or ["dbus-run-session", "--", ...] in Python.
BARE_SESSION = re.compile(r"""dbus-run-session["']?\s*,?\s*["']?--(?:["'\s]|$)""")
SCANNED = ("*.py", "*.sh", "*.bash")


def bare_sessions(text):
    return [number for number, line in enumerate(text.splitlines(), 1)
            if BARE_SESSION.search(line)]


class Detector(unittest.TestCase):
    def test_bare_forms_are_found(self):
        for line in ('command = ["dbus-run-session", "--", "mutter"]',
                     "['dbus-run-session', '--', 'mutter']",
                     "dbus-run-session -- mutter --headless &",
                     "exec dbus-run-session --"):
            self.assertEqual(bare_sessions(line), [1], line)

    def test_helper_and_tool_checks_pass(self):
        for line in ('for tool in ("mutter", "dbus-run-session"):',
                     'dbus-run-session --config-file=/x/session.conf -- mutter',
                     'return ["dbus-run-session", "--config-file=%s" % config, "--"]'):
            self.assertEqual(bare_sessions(line), [], line)


class Tree(unittest.TestCase):
    def test_no_script_starts_a_bare_private_bus(self):
        listed = subprocess.run(["git", "ls-files", "-z", "--", *SCANNED], cwd=ROOT,
                                capture_output=True, check=True).stdout.split(b"\0")
        offenders = []
        for name in filter(None, listed):
            path = ROOT / name.decode()
            if not path.is_file():
                continue
            for number in bare_sessions(path.read_text(errors="replace")):
                offenders.append("%s:%d" % (path.relative_to(ROOT), number))
        self.assertEqual(offenders, [], "use private_session.dbus_run_session()")


class Config(unittest.TestCase):
    def test_config_shadows_the_document_portal(self):
        with tempfile.TemporaryDirectory() as directory:
            config = private_session.write_config(directory)
            text = config.read_text()
            services = Path(directory).resolve() / "services"
            self.assertLess(text.index("<servicedir>%s</servicedir>" % services),
                            text.index("<include>"))
            stub = (services / "org.freedesktop.portal.Documents.service").read_text()
            self.assertIn("Name=org.freedesktop.portal.Documents", stub)
            self.assertIn("Exec=/bin/false", stub)

    @unittest.skipUnless(shutil.which("dbus-run-session") and shutil.which("gdbus"),
                         "needs dbus-run-session and gdbus")
    def test_private_bus_refuses_the_document_portal(self):
        with tempfile.TemporaryDirectory() as directory:
            call = ["gdbus", "call", "--session", "--dest", "org.freedesktop.portal.Documents",
                    "--object-path", "/org/freedesktop/portal/documents",
                    "--method", "org.freedesktop.portal.Documents.GetMountPoint"]
            result = subprocess.run(private_session.dbus_run_session(directory) + call,
                                    capture_output=True, text=True, timeout=30)
            self.assertNotEqual(result.returncode, 0, result.stdout)
            self.assertIn("org.freedesktop.DBus.Error.Spawn", result.stderr)


if __name__ == "__main__":
    unittest.main()
