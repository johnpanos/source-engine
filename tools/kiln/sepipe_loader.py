"""Import sepipe (RFC 0027) for a harness, from the tools tree.

    import sepipe_loader
    sepipe = sepipe_loader.load()
    session = sepipe.Session(str(ROOT))

sepipe is built by the tools profile (`kiln build tools-linux`) for the pinned
host Python. When the module is not built yet, load() builds it once through
./kiln. Harnesses make play and run requests through it rather than calling
launchers or staging scripts (RFC 0027 L1).
"""

import importlib
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
TOOLS_PROFILE = "tools-linux"
MODULE_DIR = ROOT / "out" / TOOLS_PROFILE / "dev" / "build" / "product" / "kiln" / "python"


class LoadError(Exception):
    pass


def load():
    if str(MODULE_DIR) not in sys.path:
        sys.path.insert(0, str(MODULE_DIR))
    if not any(MODULE_DIR.glob("sepipe*.so")):
        result = subprocess.run([str(ROOT / "kiln"), "build", TOOLS_PROFILE], cwd=ROOT,
                                capture_output=True, text=True)
        if result.returncode != 0:
            raise LoadError("kiln build %s failed: %s" % (TOOLS_PROFILE, result.stderr.strip()[-600:]))
    try:
        return importlib.import_module("sepipe")
    except ImportError as error:
        raise LoadError("cannot import sepipe from %s (built for another Python?): %s"
                        % (MODULE_DIR, error)) from error


class Run:
    """A play or run request on a thread, for a harness that watches the game
    while it runs: poll() is None until the run returns, stop() cancels it
    (the run provider stops its programs) and waits."""

    def __init__(self, sepipe, session, method, profile, **options):
        import threading
        self.returncode = None
        self.error = None
        self._cancel = sepipe.Cancellation()
        self._error_type = sepipe.KilnError

        def target():
            try:
                self.returncode = getattr(session, method)(profile, cancel=self._cancel, **options)
            except self._error_type as error:
                self.error = str(error)
                self.returncode = 1
        self._thread = threading.Thread(target=target, name="kiln-" + method, daemon=True)
        self._thread.start()

    def poll(self):
        return None if self._thread.is_alive() else self.returncode

    def stop(self, timeout=60):
        self._cancel.cancel()
        self._thread.join(timeout)
        if self._thread.is_alive():
            raise LoadError("the run did not stop within %d s of cancellation" % timeout)


_SESSIONS = {}


def session():
    """One sepipe session over this checkout, shared within the process."""
    if "session" not in _SESSIONS:
        _SESSIONS["session"] = load().Session(str(ROOT))
    return _SESSIONS["session"]


def packaged_runtime(profile, flavor="dev"):
    """The profile's packaged runtime (`kiln package <profile>`): built and
    packaged as needed, then its directory. Tools that read game content
    from a staged runtime use this instead of run/runtime*."""
    try:
        session().build(profile, flavor=flavor, up_to="package")
        return Path(session().plan(profile, flavor=flavor)["runtime"])
    except LoadError:
        raise
    except Exception as error:  # sepipe.KilnError
        raise LoadError("kiln package %s --flavor %s: %s" % (profile, flavor, error)) from error


def add_arguments(parser, profile):
    """The --profile/--flavor pair a portal_boot caller takes in place of
    the legacy --runtime/--build."""
    parser.add_argument("--profile", default=profile, help="kiln profile (default %s)" % profile)
    parser.add_argument("--flavor", default="dev", help="the profile's build flavor")


def boot_arguments(args):
    """portal_boot.py's arguments for a caller's --profile/--flavor."""
    return ["--profile", args.profile, "--flavor", args.flavor]
