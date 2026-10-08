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


def installed(profile, flavor="dev"):
    """The profile's engine install (`kiln build`'s engine-install artifact)."""
    try:
        built = session().build(profile, flavor=flavor)
    except LoadError:
        raise
    except Exception as error:  # sepipe.KilnError
        raise LoadError("kiln build %s --flavor %s: %s" % (profile, flavor, error)) from error
    for artifact in built.get("artifacts", []):
        if artifact.get("name") == "engine-install":
            return Path(artifact["path"])
    raise LoadError("kiln build %s produced no engine-install artifact" % profile)


def environment_overrides(environment, base=None):
    """The changes from `base` (this process's environment) to a harness's
    `environment`, as kiln run options (a removed variable maps to None)."""
    import os
    base = os.environ if base is None else base
    overrides = {key: value for key, value in environment.items() if base.get(key) != value}
    overrides.update({key: None for key in base if key not in environment})
    return overrides


def run_test(profile, flavor, runtime, arguments, log, timeout, environment=None, wrapper=(),
             display="none", stop_when=None, watch=None, poll_seconds=0.1):
    """A harness's test command through kiln.api: the profile's program from
    the private `runtime`, `arguments` in place of its launch template, the
    harness's `environment` (a full environment; its changes are applied),
    a diagnostic `wrapper` and a display session. Polls until the run ends,
    `timeout` seconds pass (the run is cancelled), or `stop_when()` holds
    (the run gets two more seconds). `watch(pid)` is called while it runs for
    each started process. Returns (returncode, timed_out, seconds, error)."""
    import time
    pids = []
    run = Run(load(), session(), "run", profile, flavor=flavor, runtime=str(runtime),
              exact_arguments=list(arguments), wrapper=list(wrapper), display=display,
              environment=environment_overrides(environment) if environment is not None else {},
              log=str(log), started=lambda name, pid: pids.append(pid))
    started = time.monotonic()
    deadline = started + timeout
    timed_out = False
    while run.poll() is None:
        if watch:
            for pid in list(pids):
                watch(pid)
        if stop_when is not None and stop_when():
            deadline = min(deadline, time.monotonic() + 2.0)
        if time.monotonic() > deadline:
            timed_out = True
            run.stop()
            break
        time.sleep(poll_seconds)
    return run.returncode, timed_out, time.monotonic() - started, run.error


def game_of(profile):
    """The game directory a profile launches (its launch.game)."""
    try:
        return session().resolve(profile)["launch"]["game"]
    except LoadError:
        raise
    except Exception as error:  # sepipe.KilnError, or a profile without a game
        raise LoadError("%s names no launch.game: %s" % (profile, error)) from error
