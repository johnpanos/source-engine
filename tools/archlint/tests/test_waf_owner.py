"""Positive and negative Waf projects for the build-time `arch_module` check.

Each case builds a small project with the repository's own ./waf and
scripts/waifulib/arch_owner.py, so the RFC 0001 rule is proven where it runs:
a target that declares a wrong owner fails its build.
"""
import json
import os
import shutil
import subprocess
import tempfile
import unittest
from pathlib import Path

REPO = Path(__file__).resolve().parents[3]

WSCRIPT = """
def options(opt):
	opt.load('compiler_cxx')

def configure(conf):
	conf.load('compiler_cxx')

def build(bld):
	bld.load('arch_owner', tooldir=%r)
	bld(features='cxx', source=['backend/n.cpp'], name='backend_lib', target='backend_lib',
		arch_module=%r, use=%r)
	bld(features='cxx', source=['other/o.cpp'], name='other_lib', target='other_lib')
	bld(features='cxx', source=['engine/legacy.cpp'], name='legacy_lib', target='legacy_lib', use=['SDL3'])
"""

BLOCK = {
    'modules': [
        {'id': 'base', 'paths': ['public/base/'], 'allowedEdges': []},
        {'id': 'feature', 'paths': ['feature/'], 'allowedEdges': ['base']},
        {'id': 'other', 'paths': ['other/'], 'allowedEdges': []},
        {'id': 'backend', 'kind': 'backend', 'paths': ['backend/'], 'allowedEdges': ['feature'],
         'uselib': ['VULKAN']}],
    'standardHeaders': [], 'targets': {}}


@unittest.skipUnless(shutil.which('g++') or shutil.which('c++'), 'needs a host C++ compiler')
class WafOwnerProjectTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.directory = tempfile.TemporaryDirectory()
        cls.root = Path(cls.directory.name)
        (cls.root / 'tools').mkdir()
        (cls.root / 'tools/archlint').symlink_to(REPO / 'tools/archlint')
        (cls.root / 'architecture').mkdir()
        for relative in ('backend/n.cpp', 'other/o.cpp', 'engine/legacy.cpp'):
            path = cls.root / relative
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text('int %s() { return 0; }\n' % path.stem)
        cls.write(owner='backend', use=['VULKAN'])
        result = cls.waf('configure')
        if result.returncode:
            raise unittest.SkipTest('fixture project does not configure: ' + result.stdout[-400:])

    @classmethod
    def tearDownClass(cls):
        cls.directory.cleanup()

    @classmethod
    def write(cls, owner, use, block=None):
        (cls.root / 'wscript').write_text(WSCRIPT % (str(REPO / 'scripts/waifulib'), owner, use))
        (cls.root / 'architecture/modules.json').write_text(
            json.dumps({'capabilityModules': block or BLOCK}))

    @classmethod
    def waf(cls, command):
        env = {key: value for key, value in os.environ.items() if key != 'WAFLOCK'}
        return subprocess.run(['python3', str(REPO / 'waf'), command], cwd=cls.root, env=env,
                              capture_output=True, text=True, timeout=120)

    def build(self, owner, use, block=None):
        self.write(owner, use, block)
        return self.waf('build')

    def test_correct_owner_builds(self):
        result = self.build('backend', ['VULKAN', 'legacy_lib'])
        self.assertEqual(0, result.returncode, result.stdout + result.stderr)

    def test_wrong_owner_fails_the_build(self):
        cases = [
            ('feature', ['VULKAN'], None, "compiles backend, which is outside its owner's closure"),
            ('backend', ['SDL3'], None, 'uses native library SDL3'),
            ('backend', ['other_lib'], None, 'links other_lib whose other is outside the allowed closure'),
            ('missing', [], None, 'arch_module missing is not a capability module'),
            ('backend', [], dict(BLOCK, targetOwners={'legacy': [
                {'id': 'old', 'owner': 'R46', 'reason': 'legacy', 'targets': ['backend_lib']}]}),
             'declares arch_module; remove it from its legacy group'),
        ]
        for owner, use, block, message in cases:
            with self.subTest(message=message):
                result = self.build(owner, use, block)
                self.assertNotEqual(0, result.returncode)
                self.assertIn(message, result.stdout + result.stderr)


if __name__ == '__main__':
    unittest.main()
