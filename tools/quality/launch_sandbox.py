"""Where a harness launch may write outside its own output directory.

One owner for launch isolation (RFC 0005: required runs neither depend on nor
mutate ambient state). A harness that starts the engine, a retail game or
another product process builds the child's environment here:

    sandbox = launch_sandbox.Sandbox(output / "sandbox", write_paths=[stage])
    environment = sandbox.environment(os.environ)
    ... launch with env=environment, cwd=stage ...
    evidence["sandbox"] = sandbox.finish()

What it does:

- The child gets a throwaway HOME with private XDG config, data and state
  directories under `root`, recreated for every Sandbox. SDL preferences,
  GTK/fontconfig settings, Steam API client state (steam_api finds the client
  through ~/.steam) and anything else a library saves under $HOME land there.
  XDG_RUNTIME_DIR is kept: it holds sockets (Wayland, PipeWire, D-Bus), not
  saved state; kiln's private display session owns the private D-Bus session.
- XDG_CACHE_HOME points at a shared harness cache
  (<real cache>/source-engine-harness), not the player's: driver shader caches
  (Mesa, RADV) are content-addressed and change timing only, and frame-time
  budgets were recorded with a warm driver cache. `cache="private"` gives the
  launch a cold cache of its own instead.
- Every declared write path (a staged runtime, a retail mirror's write
  directories) must resolve outside the player's real locations: the real
  XDG config/data/state directories (which hold the Steam install), ~/.steam
  and the player runtimes `kiln play` uses (out/<profile>/<flavor>/runtime),
  plus the run/ runtimes the retired launchers used. For a
  directory tree, the engine's standard write subdirectories (cfg, save,
  screenshots, platform config) of each real top-level directory are checked
  too, so a cfg/ linked into a Steam install is refused before the launch.
- The player's saved configs (config.cfg in those runtimes and in every Steam
  game) are hashed before the launch and again by finish(); a change is
  logged loudly and recorded in the evidence. Another process (the player)
  can change them during the run, so the audit is evidence, not a verdict.

Opt-outs are explicit and loud: `grants={"<grant>": "<reason>"}` with a
non-empty reason, printed to stderr as "launch-sandbox: OPT-OUT" and recorded
in the evidence. Grants:

- "real-home": the child keeps the caller's HOME and XDG directories.
- "steam": a private HOME that links the real Steam client (~/.steam and the
  Steam root), for a retail game that needs the running client.
- "player-write-paths": a declared write path may be a player location.

SOURCE_HARNESS_PROTECTED (os.pathsep-separated) adds protected locations.
"""

import hashlib
import os
from pathlib import Path
import shutil
import sys

SCHEMA = "launch-sandbox/v1"
ROOT = Path(__file__).resolve().parents[2]
PROTECTED_VARIABLE = "SOURCE_HARNESS_PROTECTED"
GRANTS = ("real-home", "steam", "player-write-paths")
# The runtimes the retired launchers (./play, run.sh, ./play_p2, ./play_fstop)
# played from; a player's saved state may still live there. Harness runtimes
# under run/ (runtime-p2-audio, retail-p2-*) belong to their harnesses and are
# not listed. kiln's player runtimes are out/<profile>/<flavor>/runtime.
PLAYER_RUNTIMES = ("runtime", "runtime-native", "runtime-dxvk", "runtime-p2", "runtime-fstop")
# Engine write directories inside a game or platform directory: cfg/config.cfg
# and the other archived settings, saves (retail Portal 2 uses SAVE),
# screenshots, and the platform's config/*.vdf.
ENGINE_WRITE_SUBPATHS = ("cfg", "save", "SAVE", "screenshots", "config")
XDG_PRIVATE = (("XDG_CONFIG_HOME", ".config"), ("XDG_DATA_HOME", ".local/share"),
               ("XDG_STATE_HOME", ".local/state"))
CACHE_NAME = "source-engine-harness"


class SandboxError(ValueError):
    """A launch would write a player location."""


def _xdg(environ, variable, home, default):
    value = environ.get(variable)
    return Path(value) if value and os.path.isabs(value) else Path(home) / default


def real_locations(environ=None):
    """The caller's real home and XDG directories, read from `environ`."""
    environ = os.environ if environ is None else environ
    home = Path(environ.get("HOME") or os.path.expanduser("~"))
    locations = {"home": home, "cache": _xdg(environ, "XDG_CACHE_HOME", home, ".cache")}
    for variable, default in XDG_PRIVATE:
        locations[variable] = _xdg(environ, variable, home, default)
    locations["steam_dot"] = home / ".steam"
    locations["steam_root"] = locations["XDG_DATA_HOME"] / "Steam"
    return locations


def protected_locations(environ=None, repo=ROOT):
    """Directories no harness launch may write: the player's saved state."""
    environ = os.environ if environ is None else environ
    real = real_locations(environ)
    paths = [real[variable] for variable, _ in XDG_PRIVATE] + [real["steam_dot"]]
    paths += [Path(repo) / "run" / name for name in PLAYER_RUNTIMES]
    paths += sorted((Path(repo) / "out").glob("*/*/runtime"))
    paths += [Path(entry) for entry in environ.get(PROTECTED_VARIABLE, "").split(os.pathsep)
              if entry]
    unique = []
    for path in paths:
        path = Path(os.path.abspath(path))
        if path not in unique:
            unique.append(path)
    return unique


def _within(path, parent):
    return path == parent or parent in path.parents


def _resolved_forms(path):
    """The absolute path and its symlink-resolved form (a protected location
    may itself be a link, as ~/.steam's entries are)."""
    return {Path(os.path.abspath(path)), Path(path).resolve()}


def sentinel_files(protected):
    """The player's saved configs under the protected locations."""
    found, seen = [], set()
    # A game directory's cfg (a player runtime's portal/cfg, a Steam game's
    # update/cfg and portal2/cfg), directly or in the Steam library.
    patterns = ("*/cfg/config.cfg", "Steam/steamapps/common/*/*/cfg/config.cfg",
                "steam/steamapps/common/*/*/cfg/config.cfg")
    for location in protected:
        if not location.is_dir():
            continue
        for pattern in patterns:
            for path in sorted(location.glob(pattern)):
                if path.is_file() and path.resolve() not in seen:
                    seen.add(path.resolve())
                    found.append(path)
    return found


def _protected_hit(path, protected):
    """The protected location `path` lies in, directly or through links."""
    for form in _resolved_forms(path):
        for location in protected:
            for resolved in _resolved_forms(location):
                if _within(form, resolved):
                    return location
    return None


def _digest(path):
    try:
        return hashlib.sha256(Path(path).read_bytes()).hexdigest()
    except OSError:
        return None


def check_write_paths(paths, grants=None, environ=None, repo=ROOT, log=None):
    """Refuses write paths in a player location, before anything is staged
    into them; Sandbox() applies the same check. Returns the paths checked."""
    grants = dict(grants or {})
    return Sandbox._check(paths, protected_locations(environ, repo), grants,
                          log if log is not None else sys.stderr)


class Sandbox:
    def __init__(self, root, write_paths=(), grants=None, cache="harness", environ=None,
                 repo=ROOT, log=None):
        self.root = Path(root).resolve()
        self.grants = dict(grants or {})
        self.log = log if log is not None else sys.stderr
        base = dict(os.environ if environ is None else environ)
        self.real = real_locations(base)
        self.protected = protected_locations(base, repo)
        for grant, reason in self.grants.items():
            if grant not in GRANTS:
                raise ValueError("unknown launch-sandbox grant %r (known: %s)"
                                 % (grant, ", ".join(GRANTS)))
            if not isinstance(reason, str) or not reason.strip():
                raise ValueError("launch-sandbox grant %r needs a reason" % grant)
            self._say("OPT-OUT %s: %s" % (grant, reason))
        if cache not in ("harness", "private"):
            raise ValueError("cache must be 'harness' or 'private'")
        self.cache = cache
        self.write_paths = [Path(path) for path in write_paths]
        self.checked = self.check_write_paths(self.write_paths)
        self.home = self.root / "home"
        self._prepare_home()
        self.sentinels = {str(path): _digest(path) for path in sentinel_files(self.protected)}
        self.changed = None

    def _say(self, message):
        print("launch-sandbox: " + message, file=self.log, flush=True)

    def _protected_hit(self, path):
        return _protected_hit(path, self.protected)

    def check_write_paths(self, paths):
        """Refuses write paths that resolve into a player location; returns
        every path checked."""
        return Sandbox._check(paths, self.protected, self.grants, self.log)

    @staticmethod
    def _check(paths, protected, grants, log):
        checked, failures = [], []
        for path in (Path(entry) for entry in paths):
            candidates = [path]
            if path.is_dir() and not path.is_symlink():
                for child in sorted(path.iterdir()):
                    if not child.is_dir() or child.is_symlink():
                        continue  # a linked overlay is read-only content
                    candidates += [child / name for name in ENGINE_WRITE_SUBPATHS
                                   if (child / name).exists() or (child / name).is_symlink()]
            for candidate in candidates:
                checked.append(str(candidate))
                hit = _protected_hit(candidate, protected)
                if hit is not None:
                    failures.append("%s resolves into the player location %s"
                                    % (candidate, hit))
                elif candidate != path and candidate.is_symlink() and \
                        not _within(candidate.resolve(), path.resolve()):
                    failures.append("%s is linked out of the private tree to %s"
                                    % (candidate, candidate.resolve()))
        if failures:
            if "player-write-paths" in grants:
                for failure in failures:
                    print("launch-sandbox: OPT-OUT player-write-paths allows: " + failure,
                          file=log, flush=True)
            else:
                raise SandboxError("launch would write outside its sandbox: " +
                                   "; ".join(failures))
        return checked

    def _prepare_home(self):
        if "real-home" in self.grants:
            return
        if self._protected_hit(self.root) is not None:
            raise SandboxError("sandbox root %s is inside a player location" % self.root)
        if self.home.is_symlink() or self.home.is_file():
            self.home.unlink()
        elif self.home.exists():
            shutil.rmtree(self.home)
        for _, relative in XDG_PRIVATE:
            (self.home / relative).mkdir(parents=True, exist_ok=True)
        if "steam" in self.grants:
            links = ((self.home / ".steam", self.real["steam_dot"]),
                     (self.home / ".local/share/Steam", self.real["steam_root"]))
            for link, target in links:
                if target.exists():
                    link.symlink_to(target, target_is_directory=True)
        self.cache_home = (self.root / "cache" if self.cache == "private"
                           else self.real["cache"] / CACHE_NAME)
        self.cache_home.mkdir(parents=True, exist_ok=True)

    def environment(self, base=None):
        """`base` (default os.environ) with the private home applied."""
        environment = dict(os.environ if base is None else base)
        if "real-home" in self.grants:
            return environment
        environment["HOME"] = str(self.home)
        for variable, relative in XDG_PRIVATE:
            environment[variable] = str(self.home / relative)
        environment["XDG_CACHE_HOME"] = str(self.cache_home)
        return environment

    def audit(self):
        """Protected configs whose bytes changed since the Sandbox was made."""
        changed = []
        for path, before in self.sentinels.items():
            if _digest(path) != before:
                changed.append(path)
        return changed

    def finish(self):
        """Audits the player's configs; returns the evidence record."""
        self.changed = self.audit()
        for path in self.changed:
            self._say("WARNING: a player config changed during the launch: %s" % path)
        record = {
            "schema": SCHEMA,
            "root": str(self.root),
            "home": None if "real-home" in self.grants else str(self.home),
            "cache": {"policy": self.cache,
                      "path": None if "real-home" in self.grants else str(self.cache_home)},
            "opt_outs": [{"grant": grant, "reason": reason}
                         for grant, reason in sorted(self.grants.items())],
            "write_paths_checked": self.checked,
            "protected": [str(path) for path in self.protected],
            "sentinels": len(self.sentinels),
            "protected_changed": self.changed,
        }
        return record
