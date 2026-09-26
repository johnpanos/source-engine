"""Sensitivity tests for the static-composition checker (tools/quality/static_composition.py).

Each test builds a small tree the way scripts/waifulib/static_composition.py
does, for ELF (g++ -r, objcopy --localize-hidden) and for Mach-O (clang for
arm64-apple-ios, Apple's ld64 -r, ld64.lld): module objects and a program
linking them. The well-formed tree must pass, and each seeded defect must be
reported: a hidden symbol left global, an STB_GNU_UNIQUE symbol (ELF), a
strong symbol defined by two modules, a first-party shared library the program
loads, a shared library in the tree, an unlinked module and a missing required
module.

ELF needs g++, readelf and objcopy. Mach-O needs the iOS host toolchain
(tools/ios/build_toolchain.py) or IOS_LD64 / IOS_LD64_LLD pointing at ld64 and
ld64.lld, plus clang, llvm-nm and llvm-objdump; it is skipped without them.
"""

import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import static_composition

ROOT = Path(__file__).resolve().parents[3]
IOS_TOOLCHAIN = ROOT / 'dependencies' / 'ios' / 'toolchain'

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

SHARED = 'extern "C" int Engine_Run() { return 0; }\n'


def sh(*argv, cwd):
    subprocess.run([str(a) for a in argv], cwd=cwd, check=True, capture_output=True)


def tool(env, *candidates):
    for candidate in [os.environ.get(env)] + list(candidates):
        if candidate and (Path(candidate).is_file() or shutil.which(str(candidate))):
            return str(candidate)
    return None


class ElfFormat:
    shared_name = 'libengine.so'
    has_unique = True

    @staticmethod
    def available():
        return all(shutil.which(t) for t in ('g++', 'readelf', 'objcopy'))

    def module(self, directory, source, target, unique, localize):
        flags = ['-fPIC', '-fvisibility=hidden', '-O1'] + ([] if unique else ['-fno-gnu-unique'])
        sh('g++', *flags, '-c', source, '-o', 'm.cpp.1.o', cwd=directory)
        sh('g++', '-r', '-nostdlib', '-Wl,--force-group-allocation', 'm.cpp.1.o', '-o',
           'partial.o', cwd=directory)
        if localize:
            sh('objcopy', '--localize-hidden', 'partial.o', target, cwd=directory)
        else:
            shutil.copy(directory / 'partial.o', directory / target)
        (directory / 'partial.o').unlink()

    def shared(self, directory, source):
        sh('g++', '-fPIC', '-shared', source, '-o', self.shared_name, cwd=directory)

    def program(self, directory, objects, shared_dir=None, allow_undefined=False):
        extra = []
        if shared_dir:
            extra += ['-L' + str(shared_dir), '-Wl,--no-as-needed', '-lengine']
        if allow_undefined:
            extra += ['-Wl,--unresolved-symbols=ignore-all']
        sh('g++', '-fPIC', 'main.cpp', *objects, *extra, '-o', 'hl2_launcher', cwd=directory)


class MachOFormat:
    shared_name = 'libengine.dylib'
    has_unique = False
    target = ['-target', 'arm64-apple-ios17.0']
    platform = ['-arch', 'arm64', '-platform_version', 'ios', '17.0', '17.0']

    def __init__(self):
        self.clang = tool('IOS_CLANG', IOS_TOOLCHAIN / 'bin' / 'clang++', 'clang++')
        self.ld64 = tool('IOS_LD64', IOS_TOOLCHAIN / 'bin' / 'ld64')
        self.lld = tool('IOS_LD64_LLD', IOS_TOOLCHAIN / 'bin' / 'ld64.lld')

    def available(self):
        return all((self.clang, self.ld64, self.lld, shutil.which('llvm-nm'),
                    shutil.which('llvm-objdump')))

    def module(self, directory, source, target, unique, localize):
        sh(self.clang, *self.target, '-fvisibility=hidden', '-O1', '-c', source, '-o', 'm.cpp.1.o',
           cwd=directory)
        # -keep_private_externs is the seeded defect: hidden symbols stay global.
        keep = [] if localize else ['-keep_private_externs']
        sh(self.ld64, '-r', '-arch', 'arm64', *keep, 'm.cpp.1.o', '-o', target, cwd=directory)

    def shared(self, directory, source):
        sh(self.clang, *self.target, '-c', source, '-o', 'e.o', cwd=directory)
        sh(self.lld, *self.platform, '-dylib', '-install_name', '@rpath/' + self.shared_name,
           '-undefined', 'dynamic_lookup', 'e.o', '-o', self.shared_name, cwd=directory)

    def program(self, directory, objects, shared_dir=None, allow_undefined=False):
        sh(self.clang, *self.target, '-c', 'main.cpp', '-o', 'main.o', cwd=directory)
        extra = [str(Path(shared_dir) / self.shared_name)] if shared_dir else []
        sh(self.lld, *self.platform, '-e', '_main', '-undefined', 'dynamic_lookup', 'main.o',
           *objects, *extra, '-o', 'hl2_launcher', cwd=directory)


class CheckerCases:
    """The same seeded defects for each binary format."""
    fmt = None

    def setUp(self):
        if not self.fmt.available():
            self.skipTest('toolchain for %s is not available' % type(self.fmt).__name__)
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
        target = 'lib%s.o' % name
        self.fmt.module(directory, 'm.cpp', target, unique, localize)
        (directory / 'm.cpp.1.o').unlink()
        return directory / target

    def program(self, objects, shared_dir=None, allow_undefined=False):
        directory = self.tree / 'launcher_main'
        directory.mkdir(exist_ok=True)
        (directory / 'main.cpp').write_text(PROGRAM)
        self.fmt.program(directory, [str(o) for o in objects], shared_dir, allow_undefined)
        for leftover in directory.glob('*.o'):
            leftover.unlink()

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
        self.program(objects[1:], allow_undefined=True)
        errors = self.check()
        self.assertTrue(any('hidden symbol' in e and 'alpha' in e for e in errors), errors)

    def test_gnu_unique_symbol_is_reported(self):
        if not self.fmt.has_unique:
            self.skipTest('Mach-O has no STB_GNU_UNIQUE binding')
        objects = [self.module('alpha', 'Alpha_Run', 1, unique=True),
                   self.module('beta', 'Beta_Run', 2)]
        self.program(objects[:1])
        errors = self.check()
        self.assertTrue(any('STB_GNU_UNIQUE' in e for e in errors), errors)

    def test_strong_symbol_in_two_modules_is_reported(self):
        shared = ('extern "C" __attribute__(( visibility( "default" ) )) int Shared_Create() '
                  '{ return 1; }\n')
        objects = [self.module('alpha', 'Alpha_Run', 1, extra=shared),
                   self.module('beta', 'Beta_Run', 2, extra=shared)]
        self.program(objects[:1])
        errors = self.check()
        self.assertTrue(any('Shared_Create is defined by module objects alpha, beta' in e
                            for e in errors), errors)

    def test_first_party_shared_library_is_reported(self):
        objects = self.well_formed()
        engine = self.tree / 'engine'
        engine.mkdir()
        (engine / 'e.cpp').write_text(SHARED)
        self.fmt.shared(engine, 'e.cpp')
        self.program(objects, shared_dir=engine)
        errors = self.check()
        name = self.fmt.shared_name
        self.assertTrue(any('needs first-party shared library' in e and name in e
                            for e in errors), errors)
        self.assertTrue(any('%s: first-party shared library built' % name in e for e in errors),
                        errors)

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
        (directory / 'main.cpp').write_text('extern "C" int Alpha_Run() { return 0; }\n' + PROGRAM)
        self.fmt.program(directory, [])
        errors = self.check()
        self.assertTrue(any('has no module objects' in e for e in errors), errors)


class ElfCheckTest(CheckerCases, unittest.TestCase):
    fmt = ElfFormat()


class MachOCheckTest(CheckerCases, unittest.TestCase):
    fmt = MachOFormat()


if __name__ == '__main__':
    unittest.main()
