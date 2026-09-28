"""Fixtures for tools/render/render_abi.py: the layout-dump parser, the table
writer, the table comparison and the seeded-swap mutator. No compiler runs
here; legacy.render-abi.sensitivity exercises the compiled path."""
import os
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import render_abi

DUMP = """\
Original map
 RenderAbiRecordProbe_IMesh::~RenderAbiRecordProbe_IMesh() -> IMesh::~IMesh()
Vtable for 'RenderAbiRecordProbe_IMesh' (12 entries).
   0 | offset_to_top (0)
   1 | RenderAbiRecordProbe_IMesh RTTI
       -- (IMesh, 0) vtable address --
       -- (IVertexBuffer, 0) vtable address --
   2 | RenderAbiRecordProbe_IMesh::~RenderAbiRecordProbe_IMesh() [complete]
   3 | RenderAbiRecordProbe_IMesh::~RenderAbiRecordProbe_IMesh() [deleting]
   4 | int IVertexBuffer::VertexCount() const [pure]
   5 | void IMesh::Draw(CPrimList *, int) [pure]
   6 | void RenderAbiRecordProbe_IMesh::RenderAbiRecordProbeTail()
   7 | offset_to_top (-8)
   8 | RenderAbiRecordProbe_IMesh RTTI
       -- (IIndexBuffer, 8) vtable address --
   9 | RenderAbiRecordProbe_IMesh::~RenderAbiRecordProbe_IMesh() [complete]
       [this adjustment: -8 non-virtual] method: IIndexBuffer::~IIndexBuffer()
  10 | RenderAbiRecordProbe_IMesh::~RenderAbiRecordProbe_IMesh() [deleting]
       [this adjustment: -8 non-virtual] method: IIndexBuffer::~IIndexBuffer()
  11 | bool IIndexBuffer::Lock(int, bool, IndexDesc_t &) [pure]

VTable indices for 'RenderAbiRecordProbe_IMesh' (1 entries).
   4 | void RenderAbiRecordProbe_IMesh::RenderAbiRecordProbeTail()
"""

HEADER = """\
#define abstract_class class
abstract_class IThing : public IBase
{
public:
	// Overrides of IBase.
	virtual bool Connect( CreateInterfaceFn factory ) = 0;
	virtual ~IThing() {}
	virtual int First( int a, /* ; ( */ int b ) = 0;
	virtual const char *Second() const = 0;
#if defined( SOMETHING )
	virtual void Hidden() = 0;
#endif
protected:
	virtual void Third() = 0;

	virtual void Fourth() = 0;
	virtual void Inline() { int x; ( void )x; }
};
"""


def only_imesh():
    return mock.patch.object(render_abi, "INTERFACES", [("IMesh", "materialsystem/imesh.h", None)])


class RenderAbiTest(unittest.TestCase):
    def test_split_signature(self):
        self.assertEqual(render_abi.split_signature("ITexture *IMaterialSystem::Find(const char *, bool) const [pure]"),
                         ("ITexture *", "IMaterialSystem", "Find", "const char *, bool", "const"))
        self.assertEqual(render_abi.split_signature("void IMatRenderContext::PrintfVA(char *, __va_list_tag *) [pure]"),
                         ("void", "IMatRenderContext", "PrintfVA", "char *, va_list", ""))
        with self.assertRaises(render_abi.AbiError):
            render_abi.split_signature("not a signature")

    def test_layout_sections_and_table(self):
        layouts = render_abi.parse_layouts(DUMP)
        self.assertEqual([s["base"] for s in layouts["RenderAbiRecordProbe_IMesh"]], ["IMesh", "IIndexBuffer"])
        with only_imesh():
            (record,) = render_abi.table_from_layouts(layouts, {})
        self.assertEqual(record["slots"], 4)
        self.assertEqual(record["secondary"], [{"base": "IIndexBuffer", "offset": 8, "slots": 3}])
        self.assertEqual([(m["adj"], m["slot"], m["name"]) for m in record["methods"]],
                         [(0, 2, "VertexCount"), (0, 3, "Draw"), (8, 2, "Lock")])
        self.assertEqual([(d["adj"], d["slot"], d["kind"]) for d in record["destructors"]],
                         [(0, 0, "complete"), (0, 1, "deleting"), (8, 0, "complete"), (8, 1, "deleting")])

    def test_missing_probe_tail_is_an_error(self):
        dump = "\n".join(l for l in DUMP.splitlines() if "RecordProbeTail" not in l)
        with only_imesh(), self.assertRaises(render_abi.AbiError):
            render_abi.table_from_layouts(render_abi.parse_layouts(dump), {})

    def test_virtual_inheritance_is_refused(self):
        dump = DUMP.replace("   0 | offset_to_top (0)", "   0 | vbase_offset (8)\n   0 | offset_to_top (0)")
        with self.assertRaises(render_abi.AbiError):
            render_abi.parse_layouts(dump)

    def test_rendered_entries_name_the_declarer(self):
        with only_imesh():
            text = render_abi.render_table(render_abi.table_from_layouts(render_abi.parse_layouts(DUMP), {}))
        self.assertIn("RENDER_ABI_INTERFACE( IMesh, \"public/materialsystem/imesh.h\", 4 )", text)
        self.assertIn("RENDER_ABI_SECONDARY( IMesh, IIndexBuffer, 8, 3 )", text)
        self.assertIn("\tIIndexBuffer, Lock, bool ( IIndexBuffer::*RenderAbiPmf )( int, bool, IndexDesc_t & ) )",
                      text)
        self.assertIn("\tIVertexBuffer, VertexCount, int ( IVertexBuffer::*RenderAbiPmf )() const )", text)
        self.assertIn("\tIMesh, Draw, void ( IMesh::*RenderAbiPmf )( CPrimList *, int ) )", text)
        self.assertIn("RENDER_ABI_DESTRUCTOR( IMesh, 8, 1, \"deleting\" )", text)

    def test_compare_reports_the_changed_interface(self):
        with only_imesh():
            text = render_abi.render_table(render_abi.table_from_layouts(render_abi.parse_layouts(DUMP), {}))
            changed = text.replace("RENDER_ABI_METHOD( IMesh, 0, 3,", "RENDER_ABI_METHOD( IMesh, 0, 4,")
            with mock.patch("sys.stdout"):
                self.assertEqual(render_abi.compare_tables(text, text), 0)
                self.assertEqual(render_abi.compare_tables(text, changed), 1)
        self.assertEqual([n for n, lines in render_abi.table_blocks(changed).items()
                          if lines != render_abi.table_blocks(text)[n]], ["IMesh"])

    def test_members_skip_comments_access_and_bodies(self):
        names = [" ".join(m.split()) for _, _, m in render_abi.member_declarations(HEADER, "IThing")]
        self.assertEqual(names[:2], ["public:", "virtual bool Connect( CreateInterfaceFn factory ) = 0;"])
        self.assertIn("virtual ~IThing() {}", names)
        self.assertIn("#endif", names)
        self.assertIn("protected:", names)
        self.assertIn("virtual void Third() = 0;", names)
        self.assertEqual(names[-1], "virtual void Inline() { int x; ( void )x; }")
        self.assertTrue(any(n.startswith("virtual int First( int a,") for n in names))

    def test_seeded_swaps_are_adjacent_plain_virtuals(self):
        swaps = [(a, b) for _, a, b in render_abi.seed_swaps(HEADER, "IThing")]
        # The destructor, the #if block and the access specifier separate
        # pairs; the inline body is never a candidate.
        self.assertEqual(swaps, [
            ("virtual int First( int a, /* ; ( */ int b ) = 0;", "virtual const char *Second() const = 0;"),
            ("virtual void Third() = 0;", "virtual void Fourth() = 0;")])
        mutated, _, _ = next(render_abi.seed_swaps(HEADER, "IThing"))
        self.assertLess(mutated.index("Second()"), mutated.index("First("))
        self.assertEqual(sorted(mutated), sorted(HEADER))

    def test_seeded_append_follows_the_last_plain_virtual(self):
        mutated, added = render_abi.seed_append(HEADER, "IThing")
        self.assertEqual(mutated.count(added), 1)
        self.assertLess(mutated.index("Fourth()"), mutated.index(added))
        self.assertLess(mutated.index(added), mutated.index("Inline()"))
        self.assertEqual(mutated.replace("\n\t" + added, ""), HEADER)

    def test_unknown_class_is_an_error(self):
        with self.assertRaises(render_abi.AbiError):
            list(render_abi.seed_swaps(HEADER, "IOther"))

    def test_record_refuses_to_replace_the_table(self):
        with tempfile.TemporaryDirectory() as root:
            table = os.path.join(root, render_abi.TABLE)
            os.makedirs(os.path.dirname(table))
            with open(table, "w") as stream:
                stream.write("recorded\n")
            with mock.patch.object(render_abi, "ROOT", root), \
                    mock.patch.object(render_abi, "derive_table", return_value=[]), \
                    mock.patch("sys.stderr"):
                self.assertEqual(render_abi.main(["record"]), 2)
            with open(table) as stream:
                self.assertEqual(stream.read(), "recorded\n")


if __name__ == "__main__":
    unittest.main()
