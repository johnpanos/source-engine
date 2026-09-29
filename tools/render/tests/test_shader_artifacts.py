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


def op(opcode, *operands):
    return [((len(operands) + 1) << 16) | opcode] + list(operands)


def names(spirv):
    """{target: name} of a module's OpName instructions."""
    found = {}
    for index, count, opcode in sa.instructions(sa.words_of(spirv)):
        if opcode == sa.OP_NAME:
            raw = sa.bytes_of(sa.words_of(spirv)[index + 2:index + count])
            found[sa.words_of(spirv)[index + 1]] = raw.split(b"\0")[0].decode()
    return found


# A module with a sampled texture (id 20, set 2 binding 1), a sampler (21,
# set 2 binding 2), a storage image (22) and a push-constant block (type 30).
GL_MODULE = module(
    op(17, 1),                                   # OpCapability Shader
    op(15, 4, 1, *sa.string_words("main")),      # OpEntryPoint Fragment %1 "main"
    op(sa.OP_NAME, 20, *sa.string_words("image")),
    decorate(20, sa.DECORATION_DESCRIPTOR_SET, 2), decorate(20, sa.DECORATION_BINDING, 1),
    decorate(21, sa.DECORATION_DESCRIPTOR_SET, 2), decorate(21, sa.DECORATION_BINDING, 2),
    decorate(22, sa.DECORATION_DESCRIPTOR_SET, 3), decorate(22, sa.DECORATION_BINDING, 0),
    op(22, 2, 32),                               # OpTypeFloat %2 32
    op(sa.OP_TYPE_IMAGE, 10, 2, 1, 0, 0, 0, 1, 0),  # sampled image
    op(sa.OP_TYPE_SAMPLER, 11),
    op(sa.OP_TYPE_IMAGE, 12, 2, 1, 0, 0, 0, 2, 1),  # storage image
    op(30, 30, 2),                               # OpTypeStruct %30 { float }
    op(sa.OP_TYPE_POINTER, 40, sa.STORAGE_UNIFORM_CONSTANT, 10),
    op(sa.OP_TYPE_POINTER, 41, sa.STORAGE_UNIFORM_CONSTANT, 11),
    op(sa.OP_TYPE_POINTER, 42, sa.STORAGE_UNIFORM_CONSTANT, 12),
    op(sa.OP_TYPE_POINTER, 43, sa.STORAGE_PUSH_CONSTANT, 30),
    op(sa.OP_VARIABLE, 40, 20, sa.STORAGE_UNIFORM_CONSTANT),
    op(sa.OP_VARIABLE, 41, 21, sa.STORAGE_UNIFORM_CONSTANT),
    op(sa.OP_VARIABLE, 42, 22, sa.STORAGE_UNIFORM_CONSTANT),
    op(sa.OP_VARIABLE, 43, 23, sa.STORAGE_PUSH_CONSTANT))


class GlArtifactTest(unittest.TestCase):
    def test_textures_samplers_and_draw_constants_are_named_for_the_adapter(self):
        found = sa.gl_names(GL_MODULE)
        self.assertEqual(found, {20: sa.GL_TEXTURE_PREFIX + "2_1", 21: sa.GL_SAMPLER_PREFIX + "2_2",
                                 30: sa.GL_DRAW_CONSTANTS_BLOCK})

    def test_rename_replaces_names_before_the_annotations(self):
        renamed = sa.rename(GL_MODULE, sa.gl_names(GL_MODULE))
        self.assertEqual(names(renamed)[20], "rg_t2_1")
        self.assertEqual(names(renamed)[30], sa.GL_DRAW_CONSTANTS_BLOCK)
        self.assertEqual(sum(1 for n in names(renamed) if n == 20), 1)
        # Every OpName precedes the first decoration (the module's layout).
        opcodes = [opcode for _, _, opcode in sa.instructions(sa.words_of(renamed))]
        self.assertLess(max(i for i, o in enumerate(opcodes) if o == sa.OP_NAME),
                        opcodes.index(sa.OP_DECORATE))

    def test_specialization_constants_are_listed_after_the_version(self):
        text = ("#version 450\n"
                "#ifndef SPIRV_CROSS_CONSTANT_ID_9\n#define SPIRV_CROSS_CONSTANT_ID_9 1.5\n#endif\n"
                "#ifndef SPIRV_CROSS_CONSTANT_ID_7\n#define SPIRV_CROSS_CONSTANT_ID_7 0\n#endif\n")
        out = sa.mark_specialization(text, [{"id": 9, "type": "float"}, {"id": 7, "type": "int"}])
        self.assertEqual(out.splitlines()[1], sa.GL_SPECIALIZATION_LINE + " 7:int 9:float")
        self.assertEqual(out.splitlines()[2:], text.splitlines()[1:])
        self.assertEqual(sa.mark_specialization(text, []), text)

    def test_a_constant_without_a_macro_or_of_another_type_fails(self):
        with self.assertRaises(sa.ArtifactError):
            sa.mark_specialization("#version 450\n", [{"id": 3, "type": "int"}])
        with self.assertRaises(sa.ArtifactError):
            sa.mark_specialization("#version 450\n#define SPIRV_CROSS_CONSTANT_ID_3 0.0lf\n",
                                   [{"id": 3, "type": "double"}])


class StoreReflectionTest(unittest.TestCase):
    def test_draw_constant_bytes_span_the_last_member(self):
        types = {"_11": {"members": [
            {"type": "mat4", "offset": 0, "matrix_stride": 16, "row_major": True},
            {"type": "vec4", "offset": 64}]},
            "_12": {"members": [
                {"type": "float", "offset": 0, "array": [2], "array_stride": 16},
                {"type": "_11", "offset": 32}]}}
        self.assertEqual(sa.block_bytes(types["_11"]["members"], types), 80)
        self.assertEqual(sa.block_bytes(types["_12"]["members"], types), 112)

    def test_an_unknown_type_fails(self):
        with self.assertRaises(sa.ArtifactError):
            sa.block_bytes([{"type": "dvec2", "offset": 0}], {})


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
