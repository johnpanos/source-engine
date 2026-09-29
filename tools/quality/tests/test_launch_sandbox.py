"""Harness launches must not write the player's saved state.

tools/quality/launch_sandbox.py gives a launch a throwaway HOME/XDG and refuses
write paths in the player's locations (Steam installs, ./play* runtimes). These
tests run a fake engine against a fake home, Steam install and repository:

- through the sandbox, a launch that saves cfg values leaves every sentinel
  real-location file byte-identical;
- the negative control launches the same fake engine unsandboxed and must be
  caught (the sentinel changes and finish() reports it);
- a runtime whose cfg/ links into the Steam install, a retail mirror whose
  update/ is one link, and a player runtime are refused before any launch;
- no script in tools/quality starts a product binary without the helper,
  except the reviewed allowlist, which is an exact ratchet.
"""

import io
import os
from pathlib import Path
import re
import subprocess
import sys
import tempfile
import unittest

HERE = Path(__file__).resolve().parent
QUALITY = HERE.parent
sys.path.insert(0, str(QUALITY))
import launch_sandbox  # noqa: E402

# A product that saves settings the way the engine and its libraries do: an
# archived cfg in its game directory, a library's file under $HOME/.config,
# and a retail config reached through the Steam root under XDG_DATA_HOME.
FAKE_ENGINE = r"""
import os, pathlib
home = pathlib.Path(os.environ["HOME"])
data = pathlib.Path(os.environ.get("XDG_DATA_HOME") or home / ".local/share")
config = pathlib.Path(os.environ.get("XDG_CONFIG_HOME") or home / ".config")
for path in (config / "fakegame/settings.ini",
             data / "Steam/steamapps/common/Portal 2/update/cfg/config.cfg",
             pathlib.Path("portal2/cfg/config.cfg")):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text('hud_quickinfo "0"\n')
"""
RETAIL_CONFIG = ".local/share/Steam/steamapps/common/Portal 2/update/cfg/config.cfg"
PLAYER_CONFIG = "run/runtime-p2/portal2/cfg/config.cfg"


class Fixture(unittest.TestCase):
    def setUp(self):
        self.tmp = Path(tempfile.mkdtemp(prefix="launch-sandbox-"))
        self.addCleanup(lambda: subprocess.run(["rm", "-rf", str(self.tmp)], check=False))
        self.real = self.tmp / "real-home"
        self.repo = self.tmp / "repo"
        self.sentinels = {}
        for path, text in ((self.real / RETAIL_CONFIG, 'hud_quickinfo "1"\n'),
                           (self.repo / PLAYER_CONFIG, 'closecaption "1"\n')):
            path.parent.mkdir(parents=True)
            path.write_text(text)
            self.sentinels[path] = path.read_bytes()
        (self.real / ".steam").mkdir()
        # The caller's environment: only HOME, as a login shell may have it.
        self.environ = {"HOME": str(self.real), "PATH": os.environ.get("PATH", "")}
        self.stage = self.tmp / "out/runtime"
        (self.stage / "portal2/cfg").mkdir(parents=True)
        self.log = io.StringIO()

    def sandbox(self, **kwargs):
        kwargs.setdefault("write_paths", [self.stage])
        return launch_sandbox.Sandbox(self.tmp / "out/sandbox", environ=self.environ,
                                      repo=self.repo, log=self.log, **kwargs)

    def launch(self, environment):
        subprocess.run([sys.executable, "-c", FAKE_ENGINE], cwd=self.stage, env=environment,
                       check=True)

    def assert_sentinels_unchanged(self):
        for path, before in self.sentinels.items():
            self.assertEqual(path.read_bytes(), before, str(path))


class Guard(Fixture):
    def test_sandboxed_launch_leaves_the_real_locations_byte_identical(self):
        sandbox = self.sandbox()
        self.assertEqual(sorted(sandbox.sentinels),
                         sorted(str(path) for path in self.sentinels))
        self.launch(sandbox.environment(self.environ))
        self.assert_sentinels_unchanged()
        self.assertFalse((self.real / ".config").exists())
        record = sandbox.finish()
        self.assertEqual(record["protected_changed"], [])
        self.assertEqual(record["opt_outs"], [])
        self.assertEqual(record["schema"], launch_sandbox.SCHEMA)
        # The writes landed in the sandbox and the stage.
        home = Path(record["home"])
        self.assertTrue((home / ".config/fakegame/settings.ini").is_file())
        self.assertTrue((home / ".local/share/Steam/steamapps/common/Portal 2/update/cfg/"
                         "config.cfg").is_file())
        self.assertIn('"0"', (self.stage / "portal2/cfg/config.cfg").read_text())

    def test_negative_control_unsandboxed_launch_is_detected(self):
        sandbox = self.sandbox()
        self.launch(dict(self.environ))  # deliberately without the sandbox
        retail = self.real / RETAIL_CONFIG
        self.assertNotEqual(retail.read_bytes(), self.sentinels[retail])
        record = sandbox.finish()
        self.assertEqual(record["protected_changed"], [str(retail)])
        self.assertIn("WARNING: a player config changed", self.log.getvalue())

    def test_environment_is_private_and_keeps_runtime_sockets(self):
        environ = dict(self.environ, XDG_RUNTIME_DIR="/run/user/1000",
                       XDG_CONFIG_HOME=str(self.real / ".config"))
        self.environ = environ
        sandbox = self.sandbox()
        environment = sandbox.environment(environ)
        self.assertEqual(environment["XDG_RUNTIME_DIR"], "/run/user/1000")
        for variable in ("HOME", "XDG_CONFIG_HOME", "XDG_DATA_HOME", "XDG_STATE_HOME"):
            self.assertTrue(Path(environment[variable]).is_relative_to(sandbox.root), variable)
        # Driver caches: shared and harness-owned, not the player's own.
        self.assertEqual(environment["XDG_CACHE_HOME"],
                         str(self.real / ".cache" / launch_sandbox.CACHE_NAME))

    def test_private_cache_and_fresh_home(self):
        sandbox = self.sandbox(cache="private")
        self.assertTrue(sandbox.cache_home.is_relative_to(sandbox.root))
        (sandbox.home / "stale").write_text("x")
        again = self.sandbox()
        self.assertFalse((again.home / "stale").exists())


class WritePaths(Fixture):
    def test_cfg_linked_into_the_steam_install_is_refused(self):
        cfg = self.stage / "portal2/cfg"
        cfg.rmdir()
        cfg.symlink_to((self.real / RETAIL_CONFIG).parent, target_is_directory=True)
        with self.assertRaisesRegex(launch_sandbox.SandboxError, "player location"):
            self.sandbox()

    def test_retail_mirror_with_a_linked_write_dir_is_refused(self):
        # The mirror layout before source-engine-78: update/ is one link.
        mirror = self.tmp / "out/mirror"
        mirror.mkdir(parents=True)
        (mirror / "update").symlink_to((self.real / RETAIL_CONFIG).parents[1],
                                       target_is_directory=True)
        with self.assertRaisesRegex(launch_sandbox.SandboxError, "update"):
            self.sandbox(write_paths=[mirror / "update"])

    def test_a_player_runtime_is_refused_before_staging(self):
        runtime = self.repo / "run/runtime-p2"
        with self.assertRaisesRegex(launch_sandbox.SandboxError, "runtime-p2"):
            launch_sandbox.check_write_paths([runtime], environ=self.environ, repo=self.repo,
                                             log=self.log)
        with self.assertRaises(launch_sandbox.SandboxError):
            self.sandbox(write_paths=[runtime])

    def test_subpath_linked_out_of_the_tree_is_refused(self):
        elsewhere = self.tmp / "elsewhere"
        elsewhere.mkdir()
        (self.stage / "portal2/screenshots").symlink_to(elsewhere, target_is_directory=True)
        with self.assertRaisesRegex(launch_sandbox.SandboxError, "linked out"):
            self.sandbox()

    def test_linked_read_only_overlays_are_not_write_paths(self):
        # A staged Portal 2 runtime links update/ and the DLC folders.
        (self.stage / "update").symlink_to((self.real / RETAIL_CONFIG).parents[1],
                                           target_is_directory=True)
        record = self.sandbox().finish()
        self.assertIn(str(self.stage / "portal2/cfg"), record["write_paths_checked"])

    def test_protected_variable_adds_locations(self):
        self.environ[launch_sandbox.PROTECTED_VARIABLE] = str(self.tmp / "out")
        with self.assertRaises(launch_sandbox.SandboxError):
            self.sandbox()


class OptOuts(Fixture):
    def test_real_home_is_loud_and_recorded(self):
        sandbox = self.sandbox(grants={"real-home": "debugging a HOME-dependent crash"})
        self.assertEqual(sandbox.environment(self.environ)["HOME"], str(self.real))
        self.assertIn("launch-sandbox: OPT-OUT real-home: debugging", self.log.getvalue())
        record = sandbox.finish()
        self.assertEqual(record["opt_outs"], [{"grant": "real-home",
                                               "reason": "debugging a HOME-dependent crash"}])
        self.assertIsNone(record["home"])

    def test_player_write_paths_grant_allows_and_logs(self):
        runtime = self.repo / "run/runtime-p2"
        sandbox = self.sandbox(write_paths=[runtime],
                               grants={"player-write-paths": "restoring the player's cfg"})
        self.assertIn("OPT-OUT player-write-paths allows", self.log.getvalue())
        self.assertEqual(sandbox.finish()["opt_outs"][0]["grant"], "player-write-paths")

    def test_steam_grant_links_only_the_client(self):
        sandbox = self.sandbox(grants={"steam": "retail portal2_linux needs the client"})
        environment = sandbox.environment(self.environ)
        home = Path(environment["HOME"])
        self.assertNotEqual(home, self.real)
        self.assertEqual((home / ".steam").resolve(), (self.real / ".steam").resolve())
        self.assertEqual((home / ".local/share/Steam").resolve(),
                         (self.real / ".local/share/Steam").resolve())
        self.assertFalse((home / ".config").is_symlink())

    def test_grants_need_a_known_name_and_a_reason(self):
        with self.assertRaisesRegex(ValueError, "unknown"):
            self.sandbox(grants={"everything": "why not"})
        with self.assertRaisesRegex(ValueError, "reason"):
            self.sandbox(grants={"real-home": " "})


# Static check: every tools/quality script that starts a product binary builds
# its environment through launch_sandbox.Sandbox. Reviewed exceptions only;
# an entry that no longer applies fails the test, so the list only shrinks.
LAUNCH = re.compile(r"subprocess\.(?:run|Popen|call|check_call|check_output)\(|"
                    r"os\.exec\w*\(|run_product\(")
PRODUCT = re.compile(r"""["'](?:\./)?(hl2_launcher|dedicated_launcher|portal2_linux|hl2_linux|"""
                     r"""hl2\.sh|portal2\.sh|srcds_run|srcds_linux|hammer_gtk|"""
                     r"""material_pixel_conformance)["']""")
USES_HELPER = re.compile(r"launch_sandbox\.Sandbox\(")
ALLOWED = {
    # Not a launch: the product name appears for another reason.
    "gi_soak.py": "launches through portal_boot.py; the name finds the product's pid",
    "portal2_paint.py": "launches through portal2_material_shots.py; checks the retail binary",
    "stage_portal2_runtime.py": "names the staged launcher; runs only vpk",
    # Pending migration (RFC/0005-progress.md, launch sandbox record).
    "portal2_audio.py": "pending: retail cohort (links the retail binary; busy file)",
    "portal2_material_shots.py": "pending: retail cohort (busy file)",
    "portal2_physics.py": "pending: retail cohort (busy file)",
    "legacy_shader_conformance.py": "pending: busy file",
}


def unsandboxed_launchers(directory=QUALITY):
    found = []
    for path in sorted(Path(directory).glob("*.py")):
        # Comments neither launch nor count as using the helper.
        text = "\n".join(line for line in path.read_text(errors="replace").splitlines()
                         if not line.lstrip().startswith("#"))
        if LAUNCH.search(text) and PRODUCT.search(text) and not USES_HELPER.search(text):
            found.append(path.name)
    return found


class Tree(unittest.TestCase):
    def test_scanner_detects_a_bare_launch(self):
        with tempfile.TemporaryDirectory() as directory:
            Path(directory, "bare.py").write_text(
                'subprocess.run([str(stage / "hl2_launcher"), "-game", "portal"])\n')
            Path(directory, "wrapped.py").write_text(
                'sandbox = launch_sandbox.Sandbox(out, write_paths=[stage])\n'
                'subprocess.Popen(["./portal2_linux"], env=sandbox.environment())\n')
            Path(directory, "tool.py").write_text('subprocess.run(["vbsp", "map.vmf"])\n')
            Path(directory, "commented.py").write_text(
                '# see launch_sandbox.Sandbox(write_paths=...)\n'
                'subprocess.run(["./portal2_linux", "-game", "portal2"])\n')
            self.assertEqual(unsandboxed_launchers(directory), ["bare.py", "commented.py"])

    def test_no_harness_launches_a_product_without_the_sandbox(self):
        found = unsandboxed_launchers()
        self.assertEqual(sorted(set(found) - set(ALLOWED)), [],
                         "build the launch environment with launch_sandbox.Sandbox")
        self.assertEqual(sorted(set(ALLOWED) - set(found)), [],
                         "migrated or gone: remove these from ALLOWED")


if __name__ == "__main__":
    unittest.main()
