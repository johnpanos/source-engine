"""Sensitivity tests for the static-composition checker (tools/quality/static_composition.py).

Each test builds a small tree the way scripts/waifulib/static_composition.py
does: module objects partially linked with `-r` and localized with objcopy,
and a program linking them. The well-formed tree must pass, and each seeded
defect must be reported: a hidden symbol left global, an STB_GNU_UNIQUE
symbol, a strong symbol defined by two modules, a first-party DT_NEEDED, a
shared library in the tree, an unlinked module and a missing required module.
Needs g++, readelf and objcopy.
"""

import json
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import static_composition

TOOLS = ('g++', 'readelf', 'objcopy')

MODULE = r'''
struct Registry { int count; };
inline Registry &Local() { static Registry r; return r; }
extern "C" void *CreateInterface( const char *, int * ) { return &Local(); }
int g_ModuleState%(n)d;
extern "C" __attribute__(( visibility( "default" ) )) int %(export)s() { return Local().count; }
extern "C" __attribute__(( visibility( "default" ) )) void *StaticModule_%(name)s_CreateInterface(
	const char *p, int *r ) { return CreateInterface( p, r ); }
'''

PROGRAM = r'''
extern "C" int Alpha_Run();
int main() { return Alpha_Run(); }
'''


def sh(*argv, cwd):
    subprocess.run(argv, cwd=cwd, check=True, capture_output=True)


@unittest.skipUnless(all(shutil.which(tool) for tool in TOOLS), 'needs ' + ', '.join(TOOLS))
class StaticCompositionCheckTest(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.tree = Path(self.tmp.name)
        self.modules = self.tree / 'modules.json'
        self.modules.write_text(json.dumps({'capabilityModules': {'sharedLibraries': {
            'groups': [{'id': 'fixture', 'targets': ['alpha', 'beta', 'engine']}]}}}))

    def tearDown(self):
        self.tmp.cleanup()

    def module(self, name, export, n, unique=False, localize=True, extra=''):
        directory = self.tree / name
        directory.mkdir(exist_ok=True)
        (directory / 'm.cpp').write_text(MODULE % {'name': name, 'export': export, 'n': n} + extra)
        flags = ['-fPIC', '-fvisibility=hidden', '-O1']
        if not unique:
            flags.append('-fno-gnu-unique')
        sh('g++', *flags, '-c', 'm.cpp', '-o', 'm.cpp.1.o', cwd=directory)
        sh('g++', '-r', '-nostdlib', '-Wl,--force-group-allocation', 'm.cpp.1.o', '-o',
           'partial.o', cwd=directory)
        target = 'lib%s.o' % name
        if localize:
            sh('objcopy', '--localize-hidden', 'partial.o', target, cwd=directory)
        else:
            shutil.copy(directory / 'partial.o', directory / target)
        (directory / 'partial.o').unlink()
        return directory / target

    def program(self, objects, extra=()):
        directory = self.tree / 'launcher_main'
        directory.mkdir(exist_ok=True)
        (directory / 'main.cpp').write_text(PROGRAM)
        sh('g++', '-fPIC', 'main.cpp', *[str(o) for o in objects], *extra, '-o', 'hl2_launcher',
           cwd=directory)

    def check(self, required=()):
        errors, _, _ = static_composition.check(
            self.tree, 'launcher_main/hl2_launcher', list(required), self.modules)
        return errors

    def well_formed(self):
        return [self.module('alpha', 'Alpha_Run', 1), self.module('beta', 'Beta_Run', 2)]

    def test_well_formed_tree_passes(self):
        self.program(self.well_formed())
        self.assertEqual(self.check(required=['alpha', 'beta']), [])

    def test_hidden_symbol_left_global_is_reported(self):
        objects = [self.module('alpha', 'Alpha_Run', 1, localize=False),
                   self.module('beta', 'Beta_Run', 2)]
        # The unlocalized module's CreateInterface would collide; link without it.
        self.program(objects[1:], extra=['-Wl,--unresolved-symbols=ignore-all'])
        errors = self.check()
        self.assertTrue(any('hidden symbol' in e and 'alpha' in e for e in errors), errors)

    def test_gnu_unique_symbol_is_reported(self):
        objects = [self.module('alpha', 'Alpha_Run', 1, unique=True), self.module('beta', 'Beta_Run', 2)]
        self.program(objects[:1])
        errors = self.check()
        self.assertTrue(any('STB_GNU_UNIQUE' in e for e in errors), errors)

    def test_strong_symbol_in_two_modules_is_reported(self):
        shared = 'extern "C" __attribute__(( visibility( "default" ) )) int Shared_Create() { return 1; }\n'
        objects = [self.module('alpha', 'Alpha_Run', 1, extra=shared),
                   self.module('beta', 'Beta_Run', 2, extra=shared)]
        self.program(objects[:1])
        errors = self.check()
        self.assertTrue(any('Shared_Create is defined by module objects alpha, beta' in e
                            for e in errors), errors)

    def test_first_party_needed_library_and_shared_output_are_reported(self):
        objects = self.well_formed()
        engine = self.tree / 'engine'
        engine.mkdir()
        (engine / 'e.cpp').write_text('extern "C" int Engine_Run() { return 0; }\n')
        sh('g++', '-fPIC', '-shared', 'e.cpp', '-o', 'libengine.so', cwd=engine)
        self.program(objects, extra=['-L' + str(engine), '-Wl,--no-as-needed', '-lengine'])
        errors = self.check()
        self.assertTrue(any('needs first-party shared library libengine.so' in e for e in errors), errors)
        self.assertTrue(any('libengine.so: first-party shared library built' in e for e in errors), errors)

    def test_unlinked_module_is_reported(self):
        objects = self.well_formed()
        self.program(objects[:1])
        errors = self.check()
        self.assertIn('%s does not link module beta' % (self.tree / 'launcher_main/hl2_launcher'),
                      errors)

    def test_missing_required_module_is_reported(self):
        self.program(self.well_formed())
        errors = self.check(required=['gamma'])
        self.assertTrue(any('does not link required module gamma' in e for e in errors), errors)

    def test_tree_without_module_objects_fails(self):
        directory = self.tree / 'launcher_main'
        directory.mkdir()
        (directory / 'main.cpp').write_text('int main() { return 0; }\n')
        sh('g++', 'main.cpp', '-o', 'hl2_launcher', cwd=directory)
        errors = self.check()
        self.assertTrue(any('has no module objects' in e for e in errors), errors)


if __name__ == '__main__':
    unittest.main()
