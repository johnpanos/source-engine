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
        self.assertEqual(['CAP008 mixed_lib: arch_module nope is not a capability or Hammer module'], errors)
        errors, _, _ = capabilities.link_graph_errors(self.block, declared('backend', 'feature'))
        self.assertEqual(['CAP008 mixed_lib: conflicting arch_module declarations backend, feature'], errors)
        block = self.owned([{'id': 'old', 'owner': 'R46', 'reason': 'r', 'targets': ['mixed_lib']}])
        errors, _, _ = capabilities.link_graph_errors(block, declared('backend'))
        self.assertEqual(['CAP008 mixed_lib: declares arch_module; remove it from its legacy group'], errors)

    def test_hammer_module_owners_are_judged_by_the_combined_graph(self):
        hammer = {'strictIncludeRoots': ['public/hammer/', 'hammer/core/'],
                  'modules': [{'id': 'hammer.app', 'allowedEdges': ['base']},
                              {'id': 'hammer.composition', 'allowedEdges': ['hammer.app', 'feature']}],
                  'includeExceptions': [{'path': 'hammer/core/app/controller.cpp',
                                         'dependency': 'other', 'owner': 'R22'}]}
        record = self.record(('app_lib', 'hammer/core/app/controller.cpp', ['base_lib', 'other_lib'],
                              'hammer.app'),
                             ('base_lib', 'public/base/b.cpp', [], 'base'),
                             ('other_lib', 'other/o.cpp', [], 'other'),
                             ('cli', 'hammer/cli/main.cpp', ['app_lib', 'feature_lib'], 'hammer.composition'),
                             ('feature_lib', 'feature/a.cpp', [], 'feature'))
        errors, judged, _ = capabilities.link_graph_errors(self.block, record, None, hammer)
        self.assertEqual(([], 5), (errors, judged))
        # Without the recorded exception, the app may not link other_lib.
        hammer['includeExceptions'] = []
        errors, _, _ = capabilities.link_graph_errors(self.block, record, None, hammer)
        self.assertEqual(1, len(errors), errors)
        self.assertIn('app_lib (hammer.app): links other_lib whose other', errors[0])
        # A Hammer owner compiling a module outside its closure fails too.
        record['entries'] += self.record(('app_lib', 'backend/n.cpp', [], 'hammer.app'))['entries']
        errors, _, _ = capabilities.link_graph_errors(self.block, record, None, hammer)
        self.assertTrue(any("compiles backend, which is outside its owner's closure" in e for e in errors))
        # Without the Hammer block, a Hammer owner is unknown.
        errors, _, _ = capabilities.link_graph_errors(self.block, record)
        self.assertTrue(any('arch_module hammer.app is not a capability or Hammer module' in e
                            for e in errors))

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


class AbiVocabularyTest(unittest.TestCase):
    """CAP010: preserved ABI headers never include the C++20 vocabulary."""

    def test_preserved_abi_headers_exclude_vocabulary(self):
        root = Path(tempfile.mkdtemp())
        self.addCleanup(lambda: __import__('shutil').rmtree(root))
        (root / 'abi').mkdir()
        (root / 'abi/clean.h').write_text('#include "tier0/platform.h"\n// #include "foundation/expected.h"\n')
        (root / 'abi/leak.h').write_text('#include <cstdint>\n#include "foundation/expected.h"\n')
        (root / 'abi/checks.h').write_text('#  include "testing/checks.h"\n')
        (root / 'abi/adapter.cpp').write_text('#include "foundation/expected.h"\n')
        paths = ['abi/clean.h', 'abi/leak.h', 'abi/checks.h', 'abi/adapter.cpp', 'abi/missing.h']
        errors = capabilities.abi_vocabulary_errors(root, paths, archlint.strip_comments_and_literals)
        self.assertEqual(2, len(errors), errors)
        self.assertTrue(errors[0].startswith('CAP010 abi/checks.h') and 'testing/checks.h' in errors[0])
        self.assertTrue(errors[1].startswith('CAP010 abi/leak.h') and 'foundation/expected.h' in errors[1])


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


class LayerContractTest(unittest.TestCase):
    """CAP011: the RFC 0016 render layer contract, one seeded fixture per rule."""

    EDGES = {
        'foundation': [],
        'jobs.graph': ['foundation'],
        'render.math': ['foundation'],
        'render.contracts': [],
        'render.device': ['foundation', 'render.math', 'render.contracts'],
        'render.graph': ['foundation', 'render.math', 'render.device', 'jobs.graph'],
        'render.resources': ['render.device', 'content.texture-contract'],
        'render.material': ['render.graph', 'render.resources', 'content.keyvalues-text'],
        'render.scene': ['render.material'],
        'render.frame': ['render.scene', 'render.graph'],
        'render.renderer': ['render.frame'],
        'render.pass.shadows': ['render.frame', 'render.scene'],
        'render.pass.present': ['render.frame'],
        'render.legacy-frontend': ['render.frame', 'render.contracts'],
        'render.composition': ['render.renderer', 'render.pass.shadows', 'render.pass.present',
                               'render.legacy-frontend', 'render.device.vulkan', 'render.device.null',
                               'render.bridge.sdl3-vulkan', 'render.indirect-light'],
        'render.device.vulkan': ['foundation', 'render.math', 'render.contracts', 'render.device'],
        'render.device.null': ['render.device'],
        'render.bridge.sdl3-vulkan': ['render.device', 'platform.sdl3.render-surface'],
        'render.indirect-light': ['render.contracts', 'world.map-container'],
        'content.texture-contract': [],
        'content.keyvalues-text': [],
        'platform.sdl3.render-surface': [],
        'world.map-container': [],
    }

    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.root = Path(self.directory.name)
        modules = []
        for mid, edges in self.EDGES.items():
            directory = mid.replace('.', '/').replace('-', '_')
            module = {'id': mid, 'paths': [f'{directory}/'], 'allowedEdges': list(edges)}
            if mid.startswith(('render.device.', 'render.bridge.')):
                module['kind'] = 'backend'
            modules.append(module)
        self.contract = {
            'id': 'render', 'rfc': '0016', 'prefix': 'render.',
            'layers': [
                ['foundation', 'render.math', 'render.contracts', 'jobs.graph'],
                ['render.device'],
                ['render.graph', 'render.shader-library', 'render.resources'],
                ['render.material'],
                ['render.scene'],
                ['render.frame'],
                ['render.renderer', 'render.pass.*', 'render.legacy-frontend'],
                ['render.composition']],
            'externalBases': ['content.keyvalues-text', 'content.texture-contract'],
            'independent': [['render.renderer', 'render.pass.*', 'render.legacy-frontend'],
                            ['render.device.vulkan', 'render.device.gl', 'render.device.null']],
            'adapters': {'render.device': ['render.device.vulkan', 'render.device.gl',
                                           'render.device.null', 'render.bridge.*']},
            'adapterConsumers': ['render.composition'],
            'outside': [{'modules': ['render.indirect-light'], 'owner': 'R91',
                         'reason': 'Predates the core; moves onto it with the remaining cohorts.'}],
            'backendIdentity': {'identifier': 'diagnosticBackend',
                                'allowedModules': ['render.device', 'render.composition']},
            'planned': ['render.device.gl', 'render.shader-library']}
        self.block = {'modules': modules, 'standardHeaders': [], 'targets': {},
                      'layerContracts': [self.contract]}

    def module(self, mid):
        return next(m for m in self.block['modules'] if m['id'] == mid)

    def add(self, mid, edges, kind=None):
        module = {'id': mid, 'paths': [mid.replace('.', '/') + '/'], 'allowedEdges': edges}
        if kind:
            module['kind'] = kind
        self.block['modules'].append(module)

    def write(self, name, text):
        path = self.root / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text)

    def errors(self):
        return capabilities.layer_contract_errors(self.root, self.block, archlint.strip_comments_and_literals)

    def assertRule(self, rule, subject):
        errors = self.errors()
        self.assertTrue(any(e.startswith(f'CAP011 rule {rule} ') and subject in e for e in errors), errors)
        return errors

    def test_valid_contract_passes(self):
        self.assertEqual(self.errors(), [])
        # check() runs CAP011 alongside the other capability checks.
        self.assertEqual(capabilities.check(self.root, self.block, archlint.strip_comments_and_literals), [])

    def test_upward_edge_fails_rule_1(self):
        self.module('render.scene')['allowedEdges'].append('render.frame')
        errors = self.assertRule(1, 'render.scene: edge to render.frame (layer 5) from layer 4')
        self.assertEqual(len(errors), 1)

    def test_same_layer_edge_fails_rule_1(self):
        self.module('render.graph')['allowedEdges'].append('render.resources')
        self.assertRule(1, 'render.graph: edge to render.resources (layer 2)')

    def test_layer_zero_family_edge_fails_but_external_vocabulary_passes(self):
        self.module('render.contracts')['allowedEdges'].append('render.math')
        self.assertRule(1, 'render.contracts: edge to render.math (layer 0)')
        self.module('render.contracts')['allowedEdges'] = ['foundation', 'jobs.graph']
        self.assertEqual(self.errors(), [])
        self.module('jobs.graph')['allowedEdges'].append('render.math')
        self.assertRule(1, 'jobs.graph: edge to render.math')

    def test_external_base_passes_and_undeclared_base_fails(self):
        self.module('render.scene')['allowedEdges'].append('content.keyvalues-text')
        self.assertEqual(self.errors(), [])
        self.module('render.scene')['allowedEdges'].append('world.map-container')
        self.assertRule(1, 'render.scene: edge to undeclared base world.map-container')

    def test_edge_to_outside_module_fails_rule_1(self):
        self.module('render.frame')['allowedEdges'].append('render.indirect-light')
        self.assertRule(1, 'render.frame: edge to render.indirect-light, which is outside')

    def test_sibling_edge_fails_rule_2(self):
        self.module('render.pass.shadows')['allowedEdges'].append('render.renderer')
        errors = self.assertRule(2, 'render.pass.shadows: edge to independent sibling render.renderer')
        self.assertFalse(any('rule 1' in e for e in errors), errors)

    def test_pass_to_pass_edge_fails_rule_2(self):
        self.module('render.pass.present')['allowedEdges'].append('render.pass.shadows')
        self.assertRule(2, 'render.pass.present: edge to independent sibling render.pass.shadows')

    def test_portable_edge_to_adapter_fails_rule_3(self):
        self.module('render.graph')['allowedEdges'].append('render.device.vulkan')
        errors = self.assertRule(3, 'render.graph: edge to adapter render.device.vulkan')
        self.assertEqual(len(errors), 1)

    def test_pattern_adapter_is_an_adapter(self):
        self.module('render.frame')['allowedEdges'].append('render.bridge.sdl3-vulkan')
        self.assertRule(3, 'render.frame: edge to adapter render.bridge.sdl3-vulkan')

    def test_adapter_consumers_and_test_fixtures_may_name_adapters(self):
        self.add('render.device.vulkan.tests', ['render.device.vulkan'], kind='native-test')
        self.contract['layers'][7].append('render.device.vulkan.tests')
        self.assertEqual(self.errors(), [])

    def test_adapter_to_adapter_edge_fails(self):
        self.contract['planned'].remove('render.device.gl')
        self.add('render.device.gl', ['render.device', 'render.device.vulkan'], kind='backend')
        errors = self.errors()
        self.assertTrue(any(e.startswith('CAP011 rule 2 render.device.gl: edge to independent sibling '
                                         'render.device.vulkan') for e in errors), errors)
        self.assertTrue(any(e.startswith('CAP011 rule 3 render.device.gl: adapter of render.device '
                                         'depends on render.device.vulkan (adapter)') for e in errors), errors)

    def test_adapter_edge_above_layer_zero_fails_rule_3(self):
        self.module('render.device.null')['allowedEdges'].append('render.graph')
        self.assertRule(3, 'render.device.null: adapter of render.device depends on render.graph (layer 2)')

    def test_adapter_native_grant_passes(self):
        self.module('render.device.null')['allowedEdges'].append('platform.sdl3.render-surface')
        self.assertEqual(self.errors(), [])

    def test_unlayered_render_module_fails_rule_4(self):
        self.add('render.stray', ['render.contracts'])
        self.assertRule(4, 'render.stray: render.* module has no layer')

    def test_pass_pattern_places_new_passes(self):
        self.add('render.pass.bloom', ['render.frame'])
        self.assertEqual(self.errors(), [])

    def test_declared_module_that_does_not_exist_fails_rule_4(self):
        self.contract['planned'].remove('render.shader-library')
        self.assertRule(4, 'render.shader-library: declared in the layer contract but not a capability module')

    def test_planned_module_that_exists_fails_rule_4(self):
        self.add('render.shader-library', ['render.device'])
        self.assertRule(4, 'render.shader-library: planned but already a capability module')

    def test_module_in_two_places_fails_rule_4(self):
        self.contract['layers'][2].append('render.device.null')
        self.assertRule(4, 'render.device.null: declared in more than one place')
        self.contract['layers'][2].remove('render.device.null')
        self.contract['outside'].append({'modules': ['render.indirect-light'], 'owner': 'R90', 'reason': 'x'})
        self.assertRule(4, 'render.indirect-light: listed in more than one outside group')

    def test_outside_exempts_a_pattern_matched_legacy_module(self):
        self.add('render.bridge.legacy-mesh', ['render.contracts', 'render.indirect-light'], kind='backend')
        self.assertRule(3, 'render.bridge.legacy-mesh: adapter of render.device depends on render.indirect-light')
        self.contract['outside'][0]['modules'].append('render.bridge.legacy-mesh')
        self.assertEqual(self.errors(), [])
        self.contract['outside'][0]['modules'].append('render.device.null')
        self.assertRule(4, 'render.device.null: both in the layer contract and outside it')

    def test_outside_group_needs_owner_and_reason(self):
        self.contract['outside'][0].update(owner='later', reason='')
        errors = self.errors()
        self.assertIn('CAP011 layerContracts render outside group 1: owner must be a roadmap row', errors)
        self.assertIn('CAP011 layerContracts render outside group 1: needs a reason', errors)

    def test_contract_structure(self):
        del self.contract['prefix']
        self.contract['layer'] = []
        self.contract['adapters'] = {'render.nowhere': ['render.device.null']}
        errors = self.errors()
        for expected in ('missing key prefix', 'unknown key layer', 'adapter port render.nowhere is not in a layer'):
            self.assertTrue(any(expected in e for e in errors), errors)
        self.contract.update(prefix='render.', layers=['render.device'])
        del self.contract['layer']
        self.assertTrue(any('layers must be a list of non-empty module lists' in e for e in self.errors()))

    def test_backend_identity_comparison_fails_rule_5(self):
        self.write('render/pass/shadows/feature.cpp',
                   'void Draw( const DeviceFacts &facts )\n{\n\tif ( facts.diagnosticBackend == "vulkan" )\n'
                   '\t\treturn;\n}\n')
        errors = self.assertRule(5, 'render/pass/shadows/feature.cpp:3 (render.pass.shadows): compares')
        self.assertEqual(len(errors), 1)

    def test_backend_identity_comparison_forms(self):
        for line in ('if ( "gl" != facts.diagnosticBackend )', 'if ( facts.diagnosticBackend.starts_with( "v" ) )',
                     'if ( strcmp( name, facts.diagnosticBackend ) == 0 )',
                     'auto order = diagnosticBackend <=> other;'):
            self.write('render/graph/compile.cpp', line + '\n')
            self.assertRule(5, 'render/graph/compile.cpp:1 (render.graph)')

    def test_backend_identity_mentions_pass(self):
        self.write('render/pass/shadows/feature.cpp',
                   '// diagnosticBackend == "x"\n/* diagnosticBackend != y */\n'
                   'void Log( const DeviceFacts &facts )\n{\n\tLog( facts.diagnosticBackend );\n'
                   '\tLog( "diagnosticBackend == vulkan" );\n}\n')
        self.write('render/device/vulkan/facts.cpp', 'void Fill( DeviceFacts &facts )\n{\n'
                   '\tfacts.diagnosticBackend = "vulkan";\n}\n')
        self.write('render/composition/select.cpp', 'bool b = config.diagnosticBackend == "gl";\n')
        self.assertEqual(self.errors(), [])
        # An adapter is portable for rule 5 unless allowed.
        self.write('render/device/vulkan/facts.cpp', 'bool b = facts.diagnosticBackend == "gl";\n')
        self.assertRule(5, 'render/device/vulkan/facts.cpp:1 (render.device.vulkan)')
