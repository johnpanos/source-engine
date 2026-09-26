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


class CompileDependencyTest(unittest.TestCase):
    """CAP005: the compiler-resolved transitive include graph (-MMD .d files)."""

    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.root = Path(self.directory.name).resolve()
        self.tree = self.root / 'build-x'
        self.tree.mkdir()
        self.block = {
            'modules': [
                {'id': 'base', 'paths': ['public/base/'], 'allowedEdges': []},
                {'id': 'feature', 'paths': ['public/feature/'], 'allowedEdges': ['base']},
                {'id': 'app', 'paths': ['app/'], 'allowedEdges': ['feature']},
                {'id': 'other', 'paths': ['public/other/'], 'allowedEdges': []},
                {'id': 'backend', 'kind': 'backend', 'paths': ['backend/'],
                 'allowedEdges': ['feature'], 'externalHeaders': ['SDL.h']}],
            'standardHeaders': [], 'targets': {}}
        for name, text in {'public/base/value.h': '', 'public/feature/api.h': '#include "base/value.h"\n',
                           'public/other/x.h': '', 'legacy/old.h': '',
                           'thirdparty/lib/lib.h': ''}.items():
            self.write(name, text)

    def write(self, name, text):
        path = self.root / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text)

    def unit(self, source, headers, text='', name='unit.d'):
        self.write(source, text)
        prerequisites = ' \\\n '.join('../' + p if not p.startswith('/') else p for p in [source] + headers)
        (self.tree / name).write_text('%s.o: %s\n' % (self.tree / source, prerequisites))

    def errors(self):
        depfiles = [(self.tree, path) for path in sorted(self.tree.glob('*.d'))]
        return capabilities.compile_dep_errors(
            self.root, self.block, depfiles, archlint.strip_comments_and_literals)

    def test_transitive_closure_is_allowed(self):
        # app may reach base only through feature's allowlist: app -> feature -> base.
        self.unit('app/main.cpp', ['public/feature/api.h', 'public/base/value.h'])
        self.assertEqual(([], 1), self.errors())

    def test_header_outside_the_closure_is_rejected(self):
        self.unit('app/main.cpp', ['public/feature/api.h', 'public/other/x.h'])
        errors, _ = self.errors()
        self.assertEqual(1, len(errors))
        self.assertIn('public/other/x.h (other) is outside the allowed closure', errors[0])

    def test_portable_unit_rejects_unowned_vendored_and_external_headers(self):
        self.unit('app/main.cpp', ['legacy/old.h', 'thirdparty/lib/lib.h', '/usr/include/freetype2/ft.h'])
        errors, _ = self.errors()
        self.assertEqual(3, len(errors))
        self.assertTrue(any('unowned header legacy/old.h' in e for e in errors))
        self.assertEqual(2, sum('external header' in e for e in errors))

    def test_system_native_sdk_reached_through_a_first_party_header_is_rejected(self):
        # -MMD never lists /usr/include/vulkan/vulkan.h; the reached header's include does.
        self.write('public/feature/api.h', '#include "base/value.h"\n#include <vulkan/vulkan.h>\n')
        self.unit('app/main.cpp', ['public/feature/api.h', 'public/base/value.h'])
        errors, _ = self.errors()
        self.assertEqual(1, len(errors))
        self.assertIn('native SDK header <vulkan/vulkan.h> through public/feature/api.h', errors[0])

    def test_native_include_in_the_unit_itself_is_rejected_unless_commented(self):
        self.unit('app/main.cpp', [], text='// #include <SDL3/SDL_video.h>\n/*\n#include <X11/Xlib.h>\n*/\n')
        self.assertEqual(([], 1), self.errors())
        self.unit('app/main.cpp', [], text='#include <SDL3/SDL_video.h>\n')
        self.assertEqual(1, len(self.errors()[0]))

    def test_native_module_keeps_its_grants_but_not_foreign_modules(self):
        self.unit('backend/impl.cpp', ['public/feature/api.h', 'legacy/old.h', '/usr/include/SDL2/SDL.h'],
                  text='#include <SDL.h>\n', name='a.d')
        self.assertEqual(([], 1), self.errors())
        self.unit('backend/impl2.cpp', ['public/other/x.h'], name='b.d')
        errors, checked = self.errors()
        self.assertEqual(2, checked)
        self.assertEqual(1, len(errors))

    def test_units_outside_strict_modules_are_not_counted(self):
        self.unit('engine/legacy.cpp', ['public/other/x.h'])
        self.assertEqual(([], 0), self.errors())

    def test_depfile_parser_handles_continuations_and_escaped_spaces(self):
        target, prerequisites = capabilities.parse_depfile(
            'a.o: ../src/a.cpp \\\n ../inc/with\\ space.h ../inc/b.h\n')
        self.assertEqual('a.o', target)
        self.assertEqual(['../src/a.cpp', '../inc/with space.h', '../inc/b.h'], prerequisites)


class LinkGraphTest(unittest.TestCase):
    """CAP006/CAP008: target ownership and the recorded Waf use graph."""

    block = {'modules': [
        {'id': 'base', 'paths': ['public/base/'], 'allowedEdges': []},
        {'id': 'feature', 'paths': ['feature/'], 'allowedEdges': ['base']},
        {'id': 'other', 'paths': ['other/'], 'allowedEdges': []},
        {'id': 'backend', 'kind': 'backend', 'paths': ['backend/'], 'allowedEdges': ['feature']}],
        'standardHeaders': [], 'targets': {}}

    def record(self, *targets):
        """Entries from (target, source, use[, arch_module]) tuples."""
        entries = []
        for target, source, use, *module in targets:
            entry = {'target': target, 'source': source, 'use': list(use)}
            if module:
                entry['arch_module'] = module[0]
            entries.append(entry)
        return {'entries': entries}

    def test_allowed_edges_and_legacy_libraries_pass(self):
        errors, judged, skipped = capabilities.link_graph_errors(self.block, self.record(
            ('feature_lib', 'feature/a.cpp', ['base_lib', 'tier0'], 'feature'),
            ('base_lib', 'public/base/b.cpp', [], 'base')))
        self.assertEqual(([], 2, 0), (errors, judged, skipped))

    def test_portable_target_linking_native_library_fails(self):
        for lib in ('SDL3', 'VULKAN', 'X11', 'WAYLAND_CLIENT', 'GTK4'):
            with self.subTest(lib=lib):
                errors, _, _ = capabilities.link_graph_errors(self.block, self.record(
                    ('feature_lib', 'feature/a.cpp', [lib], 'feature')))
                self.assertEqual(1, len(errors))
                self.assertIn('native library ' + lib, errors[0])

    def test_portable_target_linking_outside_its_closure_fails(self):
        errors, _, _ = capabilities.link_graph_errors(self.block, self.record(
            ('feature_lib', 'feature/a.cpp', ['other_lib', 'backend_lib'], 'feature'),
            ('other_lib', 'other/o.cpp', [], 'other'),
            ('backend_lib', 'backend/n.cpp', ['VULKAN'], 'backend')))
        closure_errors = [e for e in errors if 'outside the allowed closure' in e]
        self.assertEqual(2, len(closure_errors))
        self.assertTrue(any('backend_lib (backend): uses native library VULKAN' in e for e in errors))
        self.assertTrue(any('other_lib whose other' in e for e in errors))
        self.assertTrue(any('backend_lib whose backend' in e for e in errors))

    def test_native_target_needs_a_uselib_grant(self):
        block = copy.deepcopy(self.block)
        errors, judged, _ = capabilities.link_graph_errors(block, self.record(
            ('backend_lib', 'backend/n.cpp', ['VULKAN', 'SDL3'], 'backend')))
        self.assertEqual(1, judged)
        self.assertEqual(2, len(errors))
        block['modules'][3]['uselib'] = ['VULKAN']
        errors, _, _ = capabilities.link_graph_errors(block, self.record(
            ('backend_lib', 'backend/n.cpp', ['VULKAN', 'SDL3'], 'backend')))
        self.assertEqual(1, len(errors))
        self.assertIn('native library SDL3', errors[0])

    def test_every_target_needs_a_declared_or_legacy_owner(self):
        errors, judged, legacy = capabilities.link_graph_errors(self.block, self.record(
            ('strict_lib', 'feature/a.cpp', []),
            ('mixed_lib', 'backend/n.cpp', ['SDL3']),
            ('mixed_lib', 'engine/legacy.cpp', ['SDL3']),
            ('legacy', 'engine/x.cpp', ['SDL3'])))
        self.assertEqual((0, 0), (judged, legacy))
        self.assertEqual(3, len(errors))
        self.assertTrue(all(e.startswith('CAP008') and 'no architectural owner' in e for e in errors))

    def owned(self, legacy):
        block = copy.deepcopy(self.block)
        block['targetOwners'] = {'legacy': legacy}
        return block

    def test_legacy_group_owns_targets_without_judging_them(self):
        block = self.owned([{'id': 'old', 'owner': 'R46', 'reason': 'legacy',
                             'targets': ['legacy', 'mixed_lib']}])
        self.assertEqual([], capabilities.target_owners(block)[1])
        errors, judged, legacy = capabilities.link_graph_errors(block, self.record(
            ('mixed_lib', 'backend/n.cpp', ['SDL3']),
            ('mixed_lib', 'engine/legacy.cpp', ['SDL3']),
            ('legacy', 'engine/x.cpp', ['SDL3'])))
        self.assertEqual(([], 0, 2), (errors, judged, legacy))

    def test_wholly_strict_target_must_leave_its_legacy_group(self):
        block = self.owned([{'id': 'old', 'owner': 'R46', 'reason': 'legacy', 'targets': ['feature_lib']}])
        errors, _, _ = capabilities.link_graph_errors(block, self.record(('feature_lib', 'feature/a.cpp', [])))
        self.assertEqual(1, len(errors))
        self.assertIn('CAP008 feature_lib: every source is strict; declare arch_module', errors[0])

    def test_declared_owner_bounds_a_mixed_target(self):
        block = copy.deepcopy(self.block)
        block['modules'][3]['uselib'] = ['VULKAN']
        record = self.record(('mixed_lib', 'backend/n.cpp', ['VULKAN', 'base_lib'], 'backend'),
                             ('mixed_lib', 'engine/legacy.cpp', ['tier0'], 'backend'),
                             ('base_lib', 'public/base/b.cpp', [], 'base'))
        errors, judged, legacy = capabilities.link_graph_errors(block, record)
        self.assertEqual(([], 2, 0), (errors, judged, legacy))
        # The owner's grants and closure, not the legacy sources, decide.
        record['entries'] += self.record(('mixed_lib', 'engine/more.cpp', ['SDL3', 'other_lib'], 'backend'),
                                         ('other_lib', 'other/o.cpp', [], 'other'))['entries']
        errors, _, _ = capabilities.link_graph_errors(block, record)
        self.assertEqual(2, len(errors))
        self.assertTrue(any('native library SDL3' in e for e in errors))
        self.assertTrue(any('links other_lib whose other' in e for e in errors))

    def test_declared_owner_must_cover_every_strict_module_it_compiles(self):
        errors, _, _ = capabilities.link_graph_errors(self.block, self.record(
            ('mixed_lib', 'feature/a.cpp', [], 'feature'),
            ('mixed_lib', 'backend/n.cpp', [], 'feature'),
            ('mixed_lib', 'engine/legacy.cpp', [], 'feature')))
        self.assertEqual(1, len(errors))
        self.assertIn("compiles backend, which is outside its owner's closure", errors[0])

    def test_invalid_declarations_fail(self):
        def declared(*modules):
            return self.record(*[('mixed_lib', f'engine/{i}.cpp', [], m) for i, m in enumerate(modules)])
        errors, _, _ = capabilities.link_graph_errors(self.block, declared('nope'))
        self.assertEqual(['CAP008 mixed_lib: arch_module nope is not a capability module'], errors)
        errors, _, _ = capabilities.link_graph_errors(self.block, declared('backend', 'feature'))
        self.assertEqual(['CAP008 mixed_lib: conflicting arch_module declarations backend, feature'], errors)
        block = self.owned([{'id': 'old', 'owner': 'R46', 'reason': 'r', 'targets': ['mixed_lib']}])
        errors, _, _ = capabilities.link_graph_errors(block, declared('backend'))
        self.assertEqual(['CAP008 mixed_lib: declares arch_module; remove it from its legacy group'], errors)

    def test_sidecar_declarations_are_validated(self):
        group = {'id': 'g', 'owner': 'R46', 'reason': 'r', 'targets': ['a']}
        cases = [
            (dict(self.owned([group]), targetOwners={'legacy': [group], 'modules': {'a': 'base'}}),
             'unknown section'),
            (self.owned([group, dict(group, id='h')]), 'more than one legacy target group'),
            (self.owned([dict(group, owner='later')]), 'roadmap row'),
            (self.owned([dict(group, reason='')]), 'needs a reason'),
            (self.owned([dict(group, targets=[])]), 'lists no targets'),
            (self.owned([dict(group, targets=['b', 'a'])]), 'sorted'),
        ]
        for block, message in cases:
            with self.subTest(message=message):
                errors = capabilities.target_owners(block)[1]
                self.assertEqual(1, len(errors), errors)
                self.assertIn(message, errors[0])
        root = Path(tempfile.mkdtemp())
        self.addCleanup(lambda: __import__('shutil').rmtree(root))
        errors = capabilities.check(root, cases[0][0], archlint.strip_comments_and_literals)
        self.assertTrue(any('CAP008' in e for e in errors))

    def test_owner_recorded_in_no_declared_tree_is_stale(self):
        block = self.owned([{'id': 'g', 'owner': 'R46', 'reason': 'r', 'targets': ['gone', 'legacy']}])
        records = [self.record(('legacy', 'engine/x.cpp', []))]
        self.assertEqual(['CAP008 stale target owner: gone is recorded in no declared build tree'],
                         capabilities.stale_target_owners(block, records))

    def test_every_first_party_shared_library_is_reviewed(self):
        def shared(*targets):
            return [{'entries': [{'target': t, 'source': f'engine/{t}.cpp', 'use': [],
                                  'features': ['cxx', 'cxxshlib']} for t in targets]
                     + [{'target': 'tool', 'source': 'utils/t.cpp', 'use': [], 'features': ['cxx', 'cxxprogram']}]}]
        block = copy.deepcopy(self.block)
        block['sharedLibraries'] = {'groups': [
            {'id': 'debt', 'owner': 'R39', 'reason': 'static on iOS', 'targets': ['client', 'engine']}]}
        self.assertEqual([], capabilities.shared_library_groups(block)[1])
        self.assertEqual(([], 2), capabilities.shared_library_errors(block, shared('client', 'engine')))
        errors, built = capabilities.shared_library_errors(block, shared('client', 'engine', 'newmod'))
        self.assertEqual(3, built)
        self.assertEqual(1, len(errors))
        self.assertIn('CAP009 newmod: new first-party shared library', errors[0])
        errors, _ = capabilities.shared_library_errors(block, shared('engine'))
        self.assertEqual(['CAP009 stale shared library: client is built as a shared library by no declared tree'],
                         errors)
        block['sharedLibraries']['groups'].append(
            {'id': 'twice', 'owner': 'R07', 'reason': 'r', 'targets': ['engine']})
        errors = capabilities.shared_library_groups(block)[1]
        self.assertEqual(['CAP009 target engine is in more than one shared library group'], errors)
        root = Path(tempfile.mkdtemp())
        self.addCleanup(lambda: __import__('shutil').rmtree(root))
        self.assertIn(errors[0], capabilities.check(root, block, archlint.strip_comments_and_literals))

    def test_targets_command_needs_every_declared_tree(self):
        root = Path(tempfile.mkdtemp())
        self.addCleanup(lambda: __import__('shutil').rmtree(root))
        block = self.owned([{'id': 'g', 'owner': 'R46', 'reason': 'r', 'targets': ['legacy']}])
        (root / 'tree').mkdir()
        (root / 'tree/toolchain-invocations.json').write_text(
            __import__('json').dumps(self.record(('legacy', 'engine/x.cpp', []))))
        manifest = {'capabilityModules': block}
        self.assertEqual(0, archlint.targets_command(root, manifest, ['tree']))
        self.assertEqual(1, archlint.targets_command(root, manifest, ['tree', 'unbuilt']))
        stale = self.owned([{'id': 'g', 'owner': 'R46', 'reason': 'r', 'targets': ['elsewhere', 'legacy']}])
        self.assertEqual(1, archlint.targets_command(root, {'capabilityModules': stale}, ['tree']))
        self.assertEqual(0, archlint.targets_command(root, {'capabilityModules': stale}, ['tree'], partial=True))
        (root / 'tree/toolchain-invocations.json').write_text(
            __import__('json').dumps(self.record(('legacy', 'engine/x.cpp', []), ('new_lib', 'engine/y.cpp', []))))
        self.assertEqual(1, archlint.targets_command(root, manifest, ['tree']))

    def compile(self, target, source, arguments, use=(), module='feature'):
        return {'target': target, 'source': source, 'use': list(use), 'arguments': arguments,
                'directory': '/repo/build-x', 'arch_module': module}

    def test_include_roots_parse_every_flag_form(self):
        entry = self.compile('t', 'a.cpp', ['g++', '-I../public', '-I', 'gen', '-isystem', '/usr/include/SDL3',
                                            '-isystem/opt/x', '-iquote', 'q', '-idirafter', '../late', '-c'])
        self.assertEqual({'/repo/public', '/repo/build-x/gen', '/usr/include/SDL3', '/opt/x',
                          '/repo/build-x/q', '/repo/late'}, capabilities.include_roots(entry))

    def test_portable_target_may_not_attach_foreign_include_directories(self):
        clean = ['-I../public', '-Ipublic', '-I../common']
        errors, judged, _ = capabilities.link_graph_errors(self.block, {'entries': [
            self.compile('feature_lib', 'feature/a.cpp', clean)]}, '/repo')
        self.assertEqual(([], 1), (errors, judged))
        for flag, reason in (('-I/usr/include/SDL3', 'native SDK'), ('-I../thirdparty/x', 'vendored'),
                             ('-Ithirdparty/x', 'vendored'), ('-I/opt/sdk', 'external'),
                             ('-I../box3d/include', 'vendored')):
            with self.subTest(flag=flag):
                errors, _, _ = capabilities.link_graph_errors(self.block, {'entries': [
                    self.compile('feature_lib', 'feature/a.cpp', clean + [flag])]}, '/repo')
                self.assertEqual(1, len(errors), errors)
                self.assertIn(f'portable target attaches {reason} include directory', errors[0])

    def test_native_owner_may_attach_its_sdk_include_directories(self):
        block = copy.deepcopy(self.block)
        block['modules'][3]['uselib'] = ['VULKAN']
        errors, _, _ = capabilities.link_graph_errors(block, {'entries': [
            self.compile('backend_lib', 'backend/n.cpp', ['-I/usr/include/vulkan', '-I../thirdparty/x'],
                         ['VULKAN'], 'backend')]}, '/repo')
        self.assertEqual([], errors)

    def test_target_use_cycles_fail(self):
        block = self.owned([{'id': 'g', 'owner': 'R46', 'reason': 'r', 'targets': ['a', 'b', 'c', 'd']}])
        errors, _, _ = capabilities.link_graph_errors(block, self.record(
            ('a', 'engine/a.cpp', ['b', 'tier0']), ('b', 'engine/b.cpp', ['c']),
            ('c', 'engine/c.cpp', ['a']), ('d', 'engine/d.cpp', ['d'])))
        self.assertEqual(['CAP006 target use cycle: a -> b -> c', 'CAP006 target use cycle: d'], errors)
        self.assertEqual([], capabilities.use_cycles({'a': {'b'}, 'b': {'c', 'SDL3'}, 'c': set()}))

    def test_portable_module_cannot_grant_uselib(self):
        root = Path(tempfile.mkdtemp())
        self.addCleanup(lambda: __import__('shutil').rmtree(root))
        (root / 'feature').mkdir()
        (root / 'feature/a.h').write_text('')
        block = copy.deepcopy(self.block)
        block['modules'][1]['uselib'] = ['VULKAN']
        errors = capabilities.check(root, block, archlint.strip_comments_and_literals)
        self.assertTrue(any('CAP004 feature' in e for e in errors))


class HermeticHeaderTest(unittest.TestCase):
    """CAP007: portable public headers compile alone with a clean full closure."""

    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.root = Path(self.directory.name).resolve()
        for name in ('public/base/value.h', 'public/base/api.h', 'backend/native.h', 'public/backend/b.h'):
            (self.root / name).parent.mkdir(parents=True, exist_ok=True)
            (self.root / name).write_text('')
        self.block = {'modules': [
            {'id': 'base', 'paths': ['public/base/'], 'allowedEdges': []},
            {'id': 'backend', 'kind': 'backend', 'paths': ['public/backend/', 'backend/'], 'allowedEdges': []}],
            'standardHeaders': [], 'targets': {}}

    def fake(self, closure, fail=()):
        def run(include_line):
            name = include_line.split('"')[1]
            if name in fail:
                return False, 'error: boom', []
            return True, '', [str(self.root / 'public' / name)] + closure.get(name, [])
        return run

    def test_clean_closure_passes_and_native_modules_are_skipped(self):
        errors, count = capabilities.hermetic_errors(self.root, self.block, self.fake(
            {'base/api.h': ['/usr/include/c++/16/vector', str(self.root / 'public/base/value.h')]}))
        self.assertEqual(([], 2), (errors, count))

    def test_system_native_sdk_in_the_closure_fails(self):
        errors, _ = capabilities.hermetic_errors(self.root, self.block, self.fake(
            {'base/api.h': ['/usr/include/vulkan/vulkan_core.h', '/usr/include/SDL3/SDL_video.h']}))
        self.assertEqual(2, len(errors))
        self.assertTrue(all('native SDK header' in e for e in errors))

    def test_non_public_repository_header_fails(self):
        errors, _ = capabilities.hermetic_errors(self.root, self.block, self.fake(
            {'base/api.h': [str(self.root / 'backend/native.h')]}))
        self.assertEqual(1, len(errors))
        self.assertIn('non-public repository header backend/native.h', errors[0])

    def test_header_that_does_not_compile_alone_fails(self):
        errors, _ = capabilities.hermetic_errors(self.root, self.block, self.fake({}, fail=('base/api.h',)))
        self.assertEqual(1, len(errors))
        self.assertIn('does not compile alone', errors[0])

    @unittest.skipUnless(__import__('shutil').which('g++'), 'g++ not installed')
    def test_real_compiler_rejects_a_missing_include(self):
        (self.root / 'public/base/api.h').write_text('#include "base/absent.h"\n')
        errors, _ = capabilities.hermetic_errors(
            self.root, self.block, capabilities.compiler_deps(self.root, 'g++'))
        self.assertEqual(1, len(errors))
        self.assertIn('does not compile alone', errors[0])
