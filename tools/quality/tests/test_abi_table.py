"""Fixtures for tools/quality/abi_table.py beyond the render table's own
(tools/render/tests/test_render_abi.py): namespaced interfaces, typed
INTERFACE entries, global-scope qualification and derived interfaces, as the
VGUI table (tools/vgui/vgui_abi.py) uses them. No compiler runs here;
legacy.vgui-abi.sensitivity exercises the compiled path."""
import os
import sys
import unittest

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))
sys.path.insert(0, os.path.join(ROOT, "tools", "quality"))
sys.path.insert(0, os.path.join(ROOT, "tools", "vgui"))
import abi_table  # noqa: E402
import vgui_abi  # noqa: E402

# A namespaced base, a namespaced interface derived from it, and a global
# interface whose base is outside the table.
DUMP = """\
Vtable for 'TestAbiRecordProbe_IBase' (5 entries).
   0 | offset_to_top (0)
   1 | TestAbiRecordProbe_IBase RTTI
       -- (TestAbiRecordProbe_IBase, 0) vtable address --
       -- (ns::IBase, 0) vtable address --
   2 | ns::HThing ns::IBase::Make(const char *) [pure]
   3 | ns::IBorder *ns::IBase::Border() const [pure]
   4 | void TestAbiRecordProbe_IBase::TestAbiRecordProbeTail()

Vtable for 'TestAbiRecordProbe_IDerived' (6 entries).
   0 | offset_to_top (0)
   1 | TestAbiRecordProbe_IDerived RTTI
       -- (TestAbiRecordProbe_IDerived, 0) vtable address --
       -- (ns::IBase, 0) vtable address --
       -- (ns::IDerived, 0) vtable address --
   2 | ns::HThing ns::IBase::Make(const char *) [pure]
   3 | ns::IBorder *ns::IBase::Border() const [pure]
   4 | void ns::IDerived::Extra(HThing) [pure]
   5 | void TestAbiRecordProbe_IDerived::TestAbiRecordProbeTail()

Vtable for 'TestAbiRecordProbe_IGlobal' (5 entries).
   0 | offset_to_top (0)
   1 | TestAbiRecordProbe_IGlobal RTTI
       -- (IAppSystem, 0) vtable address --
       -- (IGlobal, 0) vtable address --
       -- (TestAbiRecordProbe_IGlobal, 0) vtable address --
   2 | bool IAppSystem::Connect(CreateInterfaceFn) [pure]
   3 | wchar_t *IGlobal::Find(const char *) [pure]
   4 | void TestAbiRecordProbe_IGlobal::TestAbiRecordProbeTail()
"""

INTERFACES = [
    ("IBase", "ns::IBase", "ns/IBase.h", None),
    ("IDerived", "ns::IDerived", "ns/IDerived.h", None),
    ("IGlobal", "IGlobal", "IGlobal.h", None),
]


def spec(**options):
    return abi_table.Spec("test-abi-v1", "quality/fixtures/test.h", "test.abi", "TEST_ABI", "TestAbi",
                          "TestAbiPmf", "TestAbiSeededExtra", INTERFACES, ["// preamble"], **options)


def table(**options):
    s = spec(**options)
    return s, abi_table.table_from_layouts(s, abi_table.parse_layouts(DUMP), {})


class AbiTableTest(unittest.TestCase):
    def test_split_signature_keeps_the_declarers_namespace(self):
        self.assertEqual(
            abi_table.split_signature("vgui::HCursor vgui::ISurface::CreateCursorFromFile(const char *, const char *)"),
            ("vgui::HCursor", "vgui::ISurface", "CreateCursorFromFile", "const char *, const char *", ""))
        self.assertEqual(
            abi_table.split_signature("vgui::IBorder *vgui::IScheme::GetBorder(const char *) [pure]"),
            ("vgui::IBorder *", "vgui::IScheme", "GetBorder", "const char *", ""))
        self.assertEqual(
            abi_table.split_signature("ITexture *IMaterialSystem::Find(const char *, bool) const [pure]"),
            ("ITexture *", "IMaterialSystem", "Find", "const char *, bool", "const"))

    def test_three_tuples_name_their_own_type(self):
        self.assertEqual(abi_table.normalize(("IMesh", "materialsystem/imesh.h", None)),
                         ("IMesh", "IMesh", "materialsystem/imesh.h", None))

    def test_typed_entries_and_global_qualification(self):
        s, records = table(typed=True, qualify_global=True)
        text = abi_table.render_table(s, records)
        self.assertIn('TEST_ABI_INTERFACE( IBase, ns::IBase, "public/ns/IBase.h", 2 )', text)
        self.assertIn('TEST_ABI_INTERFACE( IGlobal, IGlobal, "public/IGlobal.h", 2 )', text)
        self.assertIn("\tns::IBase, Make, ns::HThing ( ns::IBase::*TestAbiPmf )( const char * ) )", text)
        self.assertIn("\tns::IBase, Border, ns::IBorder *( ns::IBase::*TestAbiPmf )() const )", text)
        # Global declarers gain '::'; namespaced ones are already qualified.
        self.assertIn("\t::IAppSystem, Connect, bool ( ::IAppSystem::*TestAbiPmf )( CreateInterfaceFn ) )", text)
        self.assertIn("\t::IGlobal, Find, wchar_t *( ::IGlobal::*TestAbiPmf )( const char * ) )", text)
        self.assertIn("#define TEST_ABI_TABLE_VERSION \"test-abi-v1\"", text)

    def test_untyped_unqualified_entries_are_unchanged(self):
        s, records = table()
        text = abi_table.render_table(s, records)
        self.assertIn('TEST_ABI_INTERFACE( IGlobal, "public/IGlobal.h", 2 )', text)
        self.assertIn("\tIAppSystem, Connect, bool ( IAppSystem::*TestAbiPmf )( CreateInterfaceFn ) )", text)

    def test_inheritors_follow_declarations_down_only(self):
        s, records = table(qualify_global=True)
        self.assertEqual(abi_table.inheritors(s, records),
                         {"IBase": {"IBase", "IDerived"}, "IDerived": {"IDerived"}, "IGlobal": {"IGlobal"}})

    def test_table_blocks_use_the_interface_id(self):
        s, records = table(typed=True)
        blocks = abi_table.table_blocks(abi_table.render_table(s, records))
        self.assertEqual(sorted(blocks), ["(preamble)", "IBase", "IDerived", "IGlobal"])

    def test_seeded_append_names_the_tables_extra(self):
        header = "namespace ns {\nclass IBase\n{\npublic:\n\tvirtual void A() = 0;\n};\n}\n"
        mutated, added = abi_table.seed_append(header, abi_table.bare("ns::IBase"), "TestAbiSeededExtra")
        self.assertEqual(added, "virtual void TestAbiSeededExtra() = 0;")
        self.assertLess(mutated.index("A()"), mutated.index(added))


class VguiSpecTest(unittest.TestCase):
    def test_interfaces_are_unique_and_their_headers_exist(self):
        ids = [i for i, _, _, _ in vgui_abi.SPEC.interfaces]
        self.assertEqual(len(ids), len(set(ids)))
        self.assertEqual(len(ids), 17)
        for header in [h for _, _, h, _ in vgui_abi.SPEC.interfaces] + vgui_abi.SPEC.extra_includes:
            self.assertTrue(os.path.isfile(os.path.join(ROOT, "public", header)), header)

    def test_suite_expects_every_interface(self):
        with open(os.path.join(ROOT, "unittests", "vguiabitest", "vgui_abi_conformance.cpp")) as stream:
            source = stream.read()
        self.assertIn("kExpectedInterfaces = %d;" % len(vgui_abi.SPEC.interfaces), source)
        self.assertIn('kExpectedTableVersion[] = "%s";' % vgui_abi.TABLE_VERSION, source)


if __name__ == "__main__":
    unittest.main()
