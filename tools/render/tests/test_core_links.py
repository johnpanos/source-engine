"""Seeded fixtures for tools/render/core_links.py: each defect must be reported."""
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import core_links


class CoreLinksTest(unittest.TestCase):
    def test_strong_symbols_skip_weak_and_undefined(self):
        out = ('0000 T render::graph::CompileGraph(render::graph::GraphBuilder&&)\n'
               '0000 W foundation::Expected<render::frame::FrameStats, x>::~Expected()\n'
               '0000 t RenderCore_Create\n')
        self.assertEqual(core_links.strong_symbols(out),
                         ['render::graph::CompileGraph(render::graph::GraphBuilder&&)', 'RenderCore_Create'])

    def test_core_code_matches_modules_and_entries_only(self):
        self.assertEqual(core_links.core_code([
            'render::scene::CreateRenderScene()', 'RenderCore_Destroy', 'render::SelectLegacyRenderProfile()',
            'std::vector<render::device::BufferId>::push_back()', 'Engine_BindRenderCore']),
            ['render::scene::CreateRenderScene()', 'RenderCore_Destroy'])

    def test_clean_dedicated_passes(self):
        self.assertEqual(core_links.check_dedicated({'engine', 'dedicated'}, {'tier0'},
                                                    {'libengine.so': ['Host_Frame()']}), [])

    def test_dedicated_defects_are_reported(self):
        errors = core_links.check_dedicated({'engine', 'render_scene'}, {'render_device'},
                                            {'libengine.so': ['render::frame::StageName()', 'Engine_BindRenderCore']})
        self.assertEqual(len(errors), 4)

    def test_client_wiring_passes(self):
        symbols = {'a/liblauncher.so': ['RenderCore_Create', 'render::renderer::CreateRenderer()'],
                   'a/libengine.so': ['Engine_BindRenderCore', 'RenderCoreHost_BeginFrame()'],
                   'a/libclient.so': ['CHLClient::Init()']}
        self.assertEqual(core_links.check_client(symbols), [])

    def test_engine_linking_a_render_module_fails(self):
        symbols = {'a/liblauncher.so': ['RenderCore_Create'],
                   'a/libengine.so': ['Engine_BindRenderCore', 'render::scene::CreateRenderScene()']}
        self.assertTrue(any('engine defines render-core code' in e for e in core_links.check_client(symbols)))

    def test_missing_binding_or_root_fails(self):
        symbols = {'a/liblauncher.so': [], 'a/libengine.so': []}
        self.assertEqual(len(core_links.check_client(symbols)), 2)

    def test_second_copy_of_the_core_fails(self):
        symbols = {'a/liblauncher.so': ['RenderCore_Create'], 'a/libengine.so': ['Engine_BindRenderCore'],
                   'a/libclient.so': ['render::graph::CompileGraph()']}
        self.assertTrue(any('second copy' in e for e in core_links.check_client(symbols)))


if __name__ == '__main__':
    unittest.main()
