import argparse
import io
import json
import subprocess
import sys
import tempfile
import unittest
from contextlib import redirect_stdout
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import archlint
import structure


class StructureRatchetTest(unittest.TestCase):
    """CAP012/CAP013: each seeded violation is caught, and recording only
    ever lowers what is allowed."""

    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.root = Path(self.directory.name)
        subprocess.run(['git', 'init', '-q', str(self.root)], check=True)
        self.manifest = {'capabilityModules': {'modules': [
            {'id': 'core.composition', 'paths': ['core/composition/'], 'allowedEdges': []},
            {'id': 'core.pass.world', 'paths': ['core/pass/world/'], 'allowedEdges': []},
            {'id': 'core.legacy', 'paths': ['core/legacy/'], 'allowedEdges': []}]}}
        self.write('core/composition/root.cpp', 'void Wire() { AddPass(); }\n')
        self.write('core/legacy/frontend.cpp', 'ITexture *t;\n')
        self.write('tier/thread.cpp', '#ifdef _WIN32\nint a;\n#elif POSIX\nint b;\n#endif\n')
        self.write('tier/guarded.h', '#ifdef _WIN32\n#pragma once\n#endif\nint c;\n')
        self.document = {
            'schema': structure.SCHEMA,
            'rules': [
                {'id': 'legacy-types', 'kind': 'identifiers',
                 'scope': {'modules': ['core.*'], 'exceptModules': ['core.legacy']},
                 'identifiers': ['ITexture'], 'reason': 'no legacy types', 'owner': 'R0',
                 'target': 'zero', 'counts': {}},
                {'id': 'thin', 'kind': 'identifiers', 'scope': {'modules': ['core.composition']},
                 'identifiers': ['AddPass'], 'reason': 'composition wires', 'owner': 'R0',
                 'target': 'zero', 'counts': {}},
                {'id': 'branches', 'kind': 'platform-branches', 'scope': {'paths': ['tier/']},
                 'reason': 'providers own platforms', 'owner': 'R0', 'target': 'zero', 'counts': {}}],
            'lines': {'ceilings': {}}}
        self.save()
        self.run_command(adopt='legacy-types')
        self.run_command(adopt='thin')
        self.run_command(adopt='branches')
        document = self.load()
        counts, lines = self.measure()
        document['lines']['ceilings'] = {area: {'ceiling': n} for area, n in lines.items()}
        self.save(document)

    def write(self, name, text):
        path = self.root / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text)

    def save(self, document=None):
        (self.root / 'architecture').mkdir(exist_ok=True)
        structure.write(self.root, document or self.document)

    def load(self):
        return structure.read_structure(self.root)

    def measure(self):
        return structure.measure(self.root, self.load(), self.manifest['capabilityModules'],
                                 archlint.strip_comments_and_literals)

    def errors(self):
        return structure.check_errors(self.root, self.manifest, archlint.strip_comments_and_literals)

    def run_command(self, **options):
        args = argparse.Namespace(report=False, write=False, verify=False, adopt=None, raise_area=None,
                                  reason=None, top=10)
        for key, value in options.items():
            setattr(args, key, value)
        with redirect_stdout(io.StringIO()):
            return structure.command(self.root, self.manifest, args, archlint.strip_comments_and_literals)

    def test_baseline_is_clean_and_counts_what_it_should(self):
        self.assertEqual(self.errors(), [])
        rules = {rule['id']: rule['counts'] for rule in self.load()['rules']}
        self.assertEqual(rules['legacy-types'], {})  # the legacy frontend is excepted
        self.assertEqual(rules['thin'], {'core/composition/root.cpp': 1})
        # Two real branches; the pragma-once guard is not one.
        self.assertEqual(rules['branches'], {'tier/thread.cpp': 2})

    def test_comments_and_strings_do_not_count(self):
        self.write('core/pass/world/pass.cpp', '// ITexture\nconst char *s = "ITexture";\n')
        self.assertEqual([e for e in self.errors() if 'CAP012' in e], [])

    def test_legacy_type_in_a_new_file_fails(self):
        self.write('core/pass/world/pass.cpp', 'class ITexture;\nvoid Draw( ITexture * );\n')
        errors = self.errors()
        self.assertTrue(any('CAP012 legacy-types core/pass/world/pass.cpp: 2 (new file)' in e for e in errors))

    def test_higher_count_fails(self):
        self.write('core/composition/root.cpp', 'void Wire() { AddPass(); AddPass(); }\n')
        self.assertTrue(any('CAP012 thin core/composition/root.cpp: 2 (was 1)' in e for e in self.errors()))

    def test_new_platform_branch_fails(self):
        self.write('tier/thread.cpp', '#ifdef _WIN32\n#elif POSIX\n#elif defined( PLATFORM_BSD )\n#endif\n')
        self.assertTrue(any('CAP012 branches tier/thread.cpp: 3 (was 2)' in e for e in self.errors()))

    def test_decrease_fails_until_recorded_and_write_never_raises(self):
        self.write('core/composition/root.cpp', 'void Wire() {}\n')
        self.write('core/pass/world/pass.cpp', 'ITexture *leak;\n')
        self.assertTrue(any('fell to 0 from 1' in e for e in self.errors()))
        self.assertEqual(self.run_command(write=True), 1)  # the new leak still fails
        rules = {rule['id']: rule['counts'] for rule in self.load()['rules']}
        self.assertEqual(rules['thin'], {})
        self.assertEqual(rules['legacy-types'], {})
        self.assertTrue(any('legacy-types core/pass/world/pass.cpp' in e for e in self.errors()))

    def test_line_ceiling(self):
        self.write('core/composition/more.cpp', 'int x;\n\n// comment only\nint y;\n')
        errors = self.errors()
        self.assertTrue(any('CAP013 core.composition: 3 code lines is above its ceiling 1 (+2)' in e
                            for e in errors), errors)
        with self.assertRaises(SystemExit):
            self.run_command(raise_area='core.composition')  # no reason
        self.assertEqual(self.run_command(raise_area='core.composition', reason='split'), 0)
        entry = self.load()['lines']['ceilings']['core.composition']
        self.assertEqual(entry['ceiling'], 3)
        self.assertEqual(entry['raised'][-1]['reason'], 'split')
        self.assertEqual(self.errors(), [])

    def test_write_lowers_ceiling_and_new_area_needs_a_raise(self):
        self.write('core/composition/root.cpp', '\n')
        self.write('newdir/file.cpp', 'int z;\n')
        self.run_command(write=True)
        ceilings = self.load()['lines']['ceilings']
        self.assertEqual(ceilings['core.composition'], {'ceiling': 0})  # lowered, never raised
        self.assertNotIn('newdir/', ceilings)
        self.assertTrue(any('CAP013 newdir/: new area' in e for e in self.errors()))

    def test_adopt_refuses_a_recorded_rule(self):
        with self.assertRaises(SystemExit):
            self.run_command(adopt='thin')

    def test_shape(self):
        document = self.load()
        document['rules'][0]['kind'] = 'vibes'
        document['rules'][1]['extra'] = 1
        self.save(document)
        errors = self.errors()
        self.assertTrue(any('kind must be' in e for e in errors))
        self.assertTrue(any('unknown key extra' in e for e in errors))


class RepositoryStructureTest(unittest.TestCase):
    """The recorded structure covers the real tree's rules."""

    def test_recorded_rules(self):
        root = Path(__file__).resolve().parents[3]
        document = structure.read_structure(root)
        self.assertEqual(structure.shape_errors(document), [])
        ids = {rule['id'] for rule in document['rules']}
        for required in ('render-core-legacy-types', 'render-composition-thin', 'render-scene-bypass',
                         'render-pass-content-import', 'tier0-platform-branches'):
            self.assertIn(required, ids)


if __name__ == '__main__':
    unittest.main()
