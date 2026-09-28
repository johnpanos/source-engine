"""Hermetic fixtures for tools/render/shader_artifacts.py (RFC 0016 K4,
render.shader-artifacts): binding flattening for the GLSL 4.50 target,
permutation numbering, the layout check against seeded mismatches and the
bind-group ceiling. No compiler is needed; the conformance rows
render.shader-artifacts and .sensitivity run the real tools.
"""
import io
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import shader_artifacts as sa  # noqa: E402
from conformance_result import Checks  # noqa: E402

MAGIC_HEADER = [0x07230203, 0x00010000, 0, 100, 0]


def decorate(target, decoration, value):
    return [(4 << 16) | sa.OP_DECORATE, target, decoration, value]


def module(*instructions):
    words = list(MAGIC_HEADER)
    for instruction in instructions:
        words += instruction
    return sa.bytes_of(words)


def decorations(spirv):
    """{(target, decoration): value} of a module's OpDecorate words."""
    words = sa.words_of(spirv)
    found, index = {}, 5
    while index < len(words):
        count, opcode = words[index] >> 16, words[index] & 0xFFFF
        if opcode == sa.OP_DECORATE:
            found[(words[index + 1], words[index + 2])] = words[index + 3]
        index += count
    return found


def unit(name, source, extra=(), stage=None):
    item = sa.Unit(name, source, ["-O"], list(extra), "material")
    if stage:
        item.stage = stage
    return item


def binding(set_, number, kind="combined-image-sampler", count=1):
    return {"set": set_, "binding": number, "kind": kind, "count": count}


def reflection(*bindings, push=False):
    return {"bindings": list(bindings), "push_constants": push,
            "specialization_constants": [], "modes": []}


LAYOUTS = {
    "schema": sa.LAYOUT_SCHEMA,
    "passes": {
        "lit": {
            "sources": ["shaders/lit.*"],
            "push_constants": ["vertex"],
            "bindings": [
                {"set": 0, "binding": 0, "kind": "combined-image-sampler",
                 "stages": ["fragment"]},
                {"set": 1, "binding": 0, "kind": "uniform-buffer",
                 "stages": ["vertex", "fragment"]},
            ],
        },
    },
    "families": {},
}


def failures(run):
    checks = Checks(io.StringIO())
    run(checks)
    return checks.stream.getvalue(), checks.failures


class FlattenTest(unittest.TestCase):
    def test_sets_fold_into_group_slot_ranges(self):
        spirv = module(decorate(10, sa.DECORATION_DESCRIPTOR_SET, 2),
                       decorate(10, sa.DECORATION_BINDING, 3),
                       decorate(11, sa.DECORATION_BINDING, 1))
        flat = decorations(sa.flatten_bindings(spirv))
        self.assertEqual(flat[(10, sa.DECORATION_DESCRIPTOR_SET)], 0)
        self.assertEqual(flat[(10, sa.DECORATION_BINDING)], 2 * sa.GL_SLOTS_PER_GROUP + 3)
        # No set decoration means set 0.
        self.assertEqual(flat[(11, sa.DECORATION_BINDING)], 1)

    def test_binding_beyond_a_group_range_fails(self):
        spirv = module(decorate(10, sa.DECORATION_DESCRIPTOR_SET, 1),
                       decorate(10, sa.DECORATION_BINDING, sa.GL_SLOTS_PER_GROUP))
        with self.assertRaises(sa.ArtifactError):
            sa.flatten_bindings(spirv)

    def test_malformed_module_fails(self):
        with self.assertRaises(sa.ArtifactError):
            sa.flatten_bindings(sa.bytes_of(MAGIC_HEADER + [0]))


class InventoryTest(unittest.TestCase):
    def test_every_backend_unit_has_a_unique_key(self):
        units = sa.inventory()
        keys = [(u.source, u.permutation) for u in units]
        self.assertEqual(len(keys), len(set(keys)))
        self.assertGreater(len([u for u in units if u.header == "legacy"]), 100)

    def test_permutation_bits_follow_sorted_axes(self):
        units = {u.name: u for u in sa.inventory()}
        plain, clip = units["g_worldPbrFragSpv"], units["g_worldPbrClipFragSpv"]
        self.assertEqual(plain.permutation, 0)
        self.assertEqual(plain.axes, sorted(plain.axes))
        self.assertEqual(clip.permutation, 1 << clip.axes.index("-DCLIP_PLANES"))
        both = units["g_worldPbrLightClipFragSpv"]
        self.assertEqual(both.permutation, (1 << both.axes.index("-DCLIP_PLANES")) |
                         (1 << both.axes.index("-DDIRECT_LIGHTS")))


class LayoutTest(unittest.TestCase):
    def check(self, units, reflections, layouts=LAYOUTS):
        return failures(lambda checks: sa.check_layouts(checks, units, reflections, layouts))

    def test_matching_layout_passes(self):
        units = [unit("f", "shaders/lit.frag"), unit("v", "shaders/lit.vert")]
        text, failed = self.check(units, {
            "f": reflection(binding(0, 0), binding(1, 0, "uniform-buffer")),
            "v": reflection(binding(1, 0, "uniform-buffer"), push=True)})
        self.assertEqual(failed, 0, text)

    def test_wrong_kind_fails(self):
        units = [unit("f", "shaders/lit.frag"), unit("v", "shaders/lit.vert")]
        text, failed = self.check(units, {
            "f": reflection(binding(0, 0, "sampled-texture"), binding(1, 0, "uniform-buffer")),
            "v": reflection()})
        self.assertIn("layout.lit.f", text)
        self.assertIn("declared combined-image-sampler", text)

    def test_undeclared_binding_fails(self):
        units = [unit("f", "shaders/lit.frag")]
        text, _ = self.check(units, {"f": reflection(binding(0, 0), binding(1, 0, "uniform-buffer"),
                                                     binding(2, 4))})
        self.assertIn("set 2 binding 4 (combined-image-sampler) is not declared", text)

    def test_wrong_stage_fails(self):
        units = [unit("v", "shaders/lit.vert"), unit("f", "shaders/lit.frag")]
        text, _ = self.check(units, {"v": reflection(binding(0, 0)),
                                     "f": reflection(binding(1, 0, "uniform-buffer"))})
        self.assertIn("used by the vertex stage, declared for fragment", text)

    def test_push_constants_on_an_undeclared_stage_fail(self):
        units = [unit("f", "shaders/lit.frag")]
        text, _ = self.check(units, {"f": reflection(binding(0, 0), binding(1, 0, "uniform-buffer"),
                                                     push=True)})
        self.assertIn("uses push constants", text)

    def test_unused_declaration_fails(self):
        units = [unit("f", "shaders/lit.frag")]
        text, _ = self.check(units, {"f": reflection(binding(0, 0))})
        self.assertIn("layout.lit.declarations-used", text)

    def test_unit_without_a_layout_fails(self):
        units = [unit("x", "shaders/other.frag")]
        text, _ = self.check(units, {"x": reflection()})
        self.assertIn("layout.owner.x", text)


class FamilyTest(unittest.TestCase):
    def shape(self, groups):
        return failures(lambda checks: sa.check_family_shape(
            checks, "fixture", {"sources": [], "groups": groups}))

    def test_four_groups_pass(self):
        text, failed = self.shape({"frame": [], "view": [], "material": [
            {"binding": 0, "kind": "sampled-texture", "stages": ["fragment"]}], "draw": []})
        self.assertEqual(failed, 0, text)

    def test_a_fifth_group_fails_the_ceiling(self):
        text, _ = self.shape(sa.FIFTH_GROUP_FAMILY["groups"])
        self.assertIn("family.fixture.bind-group-ceiling", text)

    def test_combined_samplers_are_not_port_kinds(self):
        text, _ = self.shape({"material": [
            {"binding": 0, "kind": "combined-image-sampler", "stages": ["fragment"]}]})
        self.assertIn("port-kind", text)

    def test_family_sets_are_role_indices(self):
        layouts = {"schema": sa.LAYOUT_SCHEMA, "passes": {}, "families": {"fam": {
            "sources": ["shaders/fam.frag"],
            "groups": {"material": [{"binding": 1, "kind": "sampled-texture",
                                     "stages": ["fragment"]}]}}}}
        good = failures(lambda c: sa.check_layouts(
            c, [unit("f", "shaders/fam.frag")],
            {"f": reflection(binding(2, 1, "sampled-texture"))}, layouts))
        self.assertEqual(good[1], 0, good[0])
        bad = failures(lambda c: sa.check_layouts(
            c, [unit("f", "shaders/fam.frag")],
            {"f": reflection(binding(0, 1, "sampled-texture"))}, layouts))
        self.assertIn("set 0 binding 1", bad[0])


class DeclarationTest(unittest.TestCase):
    def test_the_committed_declaration_loads_and_names_its_exclusions(self):
        layouts = sa.load_layouts()
        for kind in ("passes", "families"):
            for name, entry in layouts[kind].items():
                self.assertTrue(entry.get("sources") or entry.get("units"), name)
                for rule in entry.get("excluded_targets", []):
                    self.assertIn(rule["format"], sa.FORMATS)
                    self.assertTrue(rule["reason"])


if __name__ == "__main__":
    unittest.main()
