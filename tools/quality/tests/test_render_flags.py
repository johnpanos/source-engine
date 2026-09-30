"""tools/quality/render_flags.sh: the render switches ./play and ./play_p2 parse."""

from pathlib import Path
import subprocess
import unittest

ROOT = Path(__file__).resolve().parents[3]
SCRIPT = ROOT / "tools/quality/render_flags.sh"


def parse(*args, default=None):
    """(backend, game arguments, rest) after render_flags_parse args."""
    prelude = "RENDER_CORE_WORLD_DEFAULT=%s; " % default if default is not None else ""
    quoted = " ".join("'%s'" % arg for arg in args)
    script = ('. "%s"; %srender_flags_parse %s; echo "$RENDER_BACKEND"; '
              'echo "${RENDER_GAME_ARGS[*]}"; echo "${RENDER_REST[*]}"' % (SCRIPT, prelude, quoted))
    out = subprocess.run(["bash", "-c", script], capture_output=True, text=True, check=True)
    backend, game, rest = out.stdout.split("\n")[:3]
    return backend, game, rest


CORE_WORLD = "+sv_cheats 1 +r_core_world 1"


class RenderFlagsTest(unittest.TestCase):
    def test_play_default_draws_the_core_world(self):
        self.assertEqual(parse("testchmb_a_01", default=1),
                         ("native", CORE_WORLD, "testchmb_a_01"))

    def test_no_core_world_opts_out(self):
        self.assertEqual(parse("--no-core-world", "x", default=1), ("native", "", "x"))

    def test_without_a_default_the_legacy_world(self):
        # ./play_p2 sets no default: three Portal 2 maps still fail fatally
        # under the core (quality/workloads/core-world-smoke-v1.json).
        self.assertEqual(parse(), ("native", "", ""))
        self.assertEqual(parse("--core-world"), ("native", CORE_WORLD, ""))

    def test_core_world_only_on_the_native_backend(self):
        self.assertEqual(parse("--dxvk", default=1), ("dxvk", "", ""))
        self.assertEqual(parse("--null", default=1), ("null", "", ""))
        self.assertEqual(parse("--no-core", default=1), ("native", "-norendercore", ""))

    def test_other_switches_keep_their_arguments(self):
        backend, game, rest = parse("--validate", "--core-world", "map1", "-console")
        self.assertEqual((backend, rest), ("native", "map1 -console"))
        self.assertEqual(game, "-vkvalidate " + CORE_WORLD)


if __name__ == "__main__":
    unittest.main()
