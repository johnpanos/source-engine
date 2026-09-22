import copy
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import archlint
import capabilities


class CapabilityBoundaryTest(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.root = Path(self.directory.name)
        self.block = {
            'modules': [
                {'id': 'base', 'paths': ['public/base/'], 'allowedEdges': []},
                {'id': 'feature', 'paths': ['public/feature/'], 'allowedEdges': ['base']}],
            'standardHeaders': ['memory'],
            'targets': {'strict': {'allowedUse': []}}}

    def write(self, name, text):
        path = self.root / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text)

    def check(self):
        return capabilities.check(self.root, self.block, archlint.strip_comments_and_literals)

    def test_allowed_dependency_and_standard_header(self):
        self.write('public/base/value.h', '#include <memory>\n')
        self.write('public/feature/api.h', '#include "base/value.h"\n')
        self.assertEqual(self.check(), [])

    def test_native_and_transitive_leaks(self):
        self.write('public/base/value.h', '#include <windows.h>\n')
        self.write('public/feature/api.h', '#include "base/value.h"\n')
        self.assertTrue(any('CAP002' in e for e in self.check()))

    def test_reverse_and_legacy_edges(self):
        self.write('public/base/value.h', '#include "feature/api.h"\n')
        self.write('public/feature/api.h', '#include "tier0/platform.h"\n')
        self.write('public/tier0/platform.h', '')
        self.assertEqual(len(self.check()), 2)

    def test_macro_and_external_root(self):
        self.write('public/base/value.h', '#ifdef _WIN32\n#endif\n#include <SDL3/SDL.h>\n')
        self.assertEqual(len(self.check()), 2)

    def test_comments_not_dependencies(self):
        self.write('public/base/value.h', '// #include <windows.h>\n/*\n#include <windows.h>\n*/\n')
        self.assertEqual(self.check(), [])

    def test_cycles_rejected(self):
        self.block['modules'][0]['allowedEdges'] = ['feature']
        self.assertTrue(any('CAP004' in e for e in self.check()))

    def test_waf_links_and_include_roots(self):
        self.assertEqual(capabilities.target_errors('strict', [], ['public'], self.block), [])
        self.assertEqual(len(capabilities.target_errors('strict', ['tier0'], ['thirdparty'], self.block)), 2)

    def native_module(self):
        self.block['modules'].append({
            'id': 'backend', 'kind': 'backend', 'paths': ['public/base/private/'],
            'allowedEdges': [], 'externalHeaders': ['SDL3/SDL.h'],
            'legacyIncludes': ['tier0/platform.h']})

    def test_specific_backend_owns_only_its_private_subtree(self):
        self.native_module()
        self.write('public/base/private/backend.h', '#include <SDL3/SDL.h>\n')
        self.assertEqual(capabilities.owner('public/base/private/backend.h', self.block), 'backend')
        self.assertEqual(self.check(), [])
        self.write('public/base/api.h', '#include <SDL3/SDL.h>\n')
        self.assertTrue(any('CAP002 public/base/api.h' in e for e in self.check()))

    def test_private_backend_cannot_leak_through_portable_header(self):
        self.native_module()
        self.write('public/base/private/backend.h', '#include <SDL3/SDL.h>\n')
        self.write('public/base/api.h', '#include "private/backend.h"\n')
        self.assertTrue(any('CAP002 public/base/api.h' in e for e in self.check()))

    def test_backend_sdk_permission_is_exact(self):
        self.native_module()
        self.write('public/base/private/backend.h', '#include <windows.h>\n')
        self.assertTrue(any('CAP002' in e for e in self.check()))

    def test_portable_module_cannot_grant_itself_native_access(self):
        self.block['modules'][0]['externalHeaders'] = ['SDL3/SDL.h']
        self.write('public/base/api.h', '#include <SDL3/SDL.h>\n')
        self.assertTrue(any('CAP004' in e for e in self.check()))
        self.assertTrue(any('CAP002' in e for e in self.check()))

    def test_equal_specificity_remains_ambiguous(self):
        self.native_module()
        duplicate = copy.deepcopy(self.block['modules'][-1])
        duplicate['id'] = 'other-backend'
        self.block['modules'].append(duplicate)
        self.write('public/base/private/backend.h', '')
        self.assertTrue(any('ambiguous ownership' in e for e in self.check()))

    def test_native_legacy_include_is_not_inherited_by_portable_package(self):
        self.native_module()
        self.write('public/tier0/platform.h', '')
        self.write('public/base/private/backend.h', '#include "tier0/platform.h"\n')
        self.assertEqual(self.check(), [])
        self.write('public/base/api.h', '#include "tier0/platform.h"\n')
        self.assertTrue(any('CAP002 public/base/api.h' in e for e in self.check()))
