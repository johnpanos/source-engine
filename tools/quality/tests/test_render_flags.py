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
# Archived lighting settings, passed on every launch: baked indirect light,
# the core's runtime direct light, area lights off.
SETTINGS = "+r_indirect_producer baked +r_core_runtime_direct 1 +r_core_area_lights 0"


def game(*parts):
    return " ".join(list(parts) + [SETTINGS])


class RenderFlagsTest(unittest.TestCase):
    def test_play_default_draws_the_core_world(self):
        self.assertEqual(parse("testchmb_a_01", default=1),
                         ("native", game(CORE_WORLD), "testchmb_a_01"))

    def test_no_core_world_opts_out(self):
        self.assertEqual(parse("--no-core-world", "x", default=1), ("native", game(), "x"))

    def test_without_a_default_the_legacy_world(self):
        # A caller that sets no default keeps the legacy world.
        self.assertEqual(parse(), ("native", game(), ""))
        self.assertEqual(parse("--core-world"), ("native", game(CORE_WORLD), ""))

    def test_core_world_only_on_the_native_backend(self):
        self.assertEqual(parse("--null", default=1), ("null", game(), ""))
        self.assertEqual(parse("--no-core", default=1), ("native", game("-norendercore"), ""))

    def test_other_switches_keep_their_arguments(self):
        backend, args, rest = parse("--validate", "--core-world", "map1", "-console")
        self.assertEqual((backend, rest), ("native", "map1 -console"))
        self.assertEqual(args, game("-vkvalidate", CORE_WORLD))


    def test_lighting_switches_change_the_archived_settings(self):
        _, args, _ = parse("--moving-light-gi", "--baked-direct", "--area-lights")
        self.assertEqual(args, "+r_indirect_producer sdf +r_core_runtime_direct 0 "
                               "+r_core_area_lights 1")

    def test_shadow_and_bounce_switches(self):
        _, args, _ = parse("--soft-shadows", "--probe-bounce")
        self.assertEqual(args, game("+r_core_shadow_pcss 1", "+r_core_probe_bounce 1"))


if __name__ == "__main__":
    unittest.main()
