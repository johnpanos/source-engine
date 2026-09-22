"""Regression tests use disposable repositories, never the user's worktree."""

import contextlib
import importlib.util
import io
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest
from unittest.mock import patch


ROOT = Path(__file__).resolve().parents[3]
SPEC = importlib.util.spec_from_file_location("stylelint", ROOT / "tools/stylelint/stylelint.py")
lint = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(lint)
FORMATTER = os.environ.get("CLANG_FORMAT", "clang-format")


class IncludeTests(unittest.TestCase):
    def test_last_include_and_comments(self):
        self.assertEqual(lint.memdbgon_violations('#include "a.h"\n#include "tier0/memdbgon.h"\n// #include "b.h"\n'), [])

    def test_late_include_and_duplicate(self):
        self.assertEqual(lint.memdbgon_violations('#include <memdbgon.h>\n#include "b.h"\n#include "memdbgon.h"\n'), [(2, "b.h"), (3, "memdbgon.h")])

    def test_raw_strings_and_comments_are_not_directives(self):
        source = '/*\n#include "bad.h"\n*/\nauto s = R"tag(\n#include "fake.h"\n)tag";\n#include "memdbgon.h"\n'
        self.assertEqual(lint.includes(source), [(7, "memdbgon.h")])

    def test_line_splicing_precedes_comments(self):
        source = '// comment \\\n#include "fake.h"\n#inc\\\nlude "memdbgon.h"\n#include \\\n"real.h"\n'
        self.assertEqual(lint.memdbgon_violations(source), [(5, "real.h")])

    def test_multiline_comment_before_header(self):
        self.assertEqual(lint.memdbgon_violations('#include /* why\n */ "memdbgon.h"\n#include "bad.h"\n'), [(3, "bad.h")])

    def test_macro_and_conditional_are_conservative(self):
        source = '#include "memdbgon.h"\n#if 0\n#include HEADER\n#endif\n'
        self.assertEqual(lint.memdbgon_violations(source), [(3, "HEADER")])

    def test_digit_separators_and_escaped_quotes(self):
        source = 'auto n = 1\'000;\nauto s = "\\\"";\n#include "memdbgon.h"\n#include "bad.h"\n'
        self.assertEqual(lint.memdbgon_violations(source), [(4, "bad.h")])

    def test_unchanged_debt_is_allowed(self):
        source = '#include "memdbgon.h"\n#include "bad.h"\n'
        self.assertEqual(lint.newly_broken_includes(source, '// note\n' + source), [])

    def test_added_duplicate_is_not_allowed(self):
        source = '#include "memdbgon.h"\n#include "bad.h"\n'
        self.assertEqual(lint.newly_broken_includes(source, source + '#include "bad.h"\n'), [(3, "bad.h")])

    def test_moving_memdbgon_up_breaks_old_include(self):
        old = '#include "a.h"\n#include "memdbgon.h"\n'
        new = '#include "memdbgon.h"\n#include "a.h"\n'
        self.assertEqual(lint.newly_broken_includes(old, new), [(2, "a.h")])


class FormatterTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        # A missing formatter is a failure, never a skipped acceptance test.
        lint.check_formatter(ROOT, FORMATTER)

    def format(self, text, ranges=None):
        return lint.format_bytes(ROOT, FORMATTER, "sample.cpp", text.encode(), ranges)[0].decode()

    def test_source_conventions_and_idempotence(self):
        source = 'void Work(int *p) { if(p) { Use(*p); } }\n'
        expected = 'void Work( int *p )\n{\n\tif ( p )\n\t{\n\t\tUse( *p );\n\t}\n}\n'
        self.assertEqual(self.format(source), expected)
        self.assertEqual(self.format(expected), expected)

    def test_includes_preserved(self):
        source = '#include "z.h"\n#include "a.h"\n#include "tier0/memdbgon.h"\n'
        self.assertEqual(self.format(source), source)

    def test_unicode_and_crlf(self):
        source = '// café\r\nint * p;\r\n'
        self.assertEqual(self.format(source), '// café\r\nint *p;\r\n')

    def test_cxx20_and_empty_file(self):
        source = 'template<class T> concept Value = requires(T t) { t.value(); };\nvoid F(std::span<int> values) { for(auto &v : values) { v++; } }\n'
        formatted = self.format(source)
        self.assertEqual(self.format(formatted), formatted)
        self.assertEqual(self.format(''), '')

    def test_only_edited_region(self):
        source = 'int * legacy;\n\nint * changed;\n'
        self.assertEqual(self.format(source, [(3, 3)]), 'int * legacy;\n\nint *changed;\n')
        self.assertEqual(self.format(source, []), source)

    def test_bad_xml_and_incomplete_parse_fail(self):
        for output in [b'invalid', b'<replacements incomplete_format="true"/>', b'<replacements incomplete_format="false"><replacement offset="99" length="1"/></replacements>']:
            with self.subTest(output=output), patch.object(lint, 'run', return_value=output):
                with self.assertRaises(lint.LintError):
                    lint.format_bytes(ROOT, FORMATTER, "x.cpp", b'int x;', None)

    def test_wrong_version_and_missing_formatter_fail(self):
        with patch.object(lint, 'run', return_value=b'clang-format version 19.0.0'):
            with self.assertRaises(lint.LintError):
                lint.check_formatter(ROOT, FORMATTER)
        with self.assertRaises(lint.LintError):
            lint.check_formatter(ROOT, '/nonexistent/source-style-formatter')


class RepositoryTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name).resolve()
        self.git('init', '-q')
        self.git('config', 'user.email', 'fixture@example.invalid')
        self.git('config', 'user.name', 'Style fixture')
        self.git('config', 'core.autocrlf', 'false')
        self.write('tools/stylelint/requirements.txt', (ROOT / 'tools/stylelint/requirements.txt').read_text())
        shutil.copyfile(ROOT / '.clang-format', self.root / '.clang-format')
        self.write('legacy.cpp', 'int * legacy;\n\nint * old;\n')
        self.commit()

    def git(self, *args):
        return subprocess.check_output(['git', *args], cwd=self.root, stderr=subprocess.PIPE).decode().strip()

    def write(self, name, text):
        path = self.root / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text)

    def commit(self):
        self.git('add', '.')
        self.git('commit', '-qm', 'fixture')

    def selected(self, base=None):
        return lint.select_files(self.root, changed=True, all_files=False, base=base, paths=[])[0]

    def main(self, *args):
        output = io.StringIO()
        with contextlib.redirect_stdout(output), contextlib.redirect_stderr(output):
            status = lint.main(['--clang-format', FORMATTER, *args], root=self.root)
        return status, output.getvalue()

    def test_staged_unstaged_untracked_and_vendor(self):
        self.write('staged.cpp', 'int a;\n')
        self.git('add', 'staged.cpp')
        self.write('legacy.cpp', 'int * legacy;\n\nint * changed;\n')
        self.write('new café file.cpp', 'int b;\n')
        self.write('thirdparty/vendor.cpp', 'int * b;\n')
        self.assertEqual(self.selected(), {'legacy.cpp': [(3, 3)], 'new café file.cpp': None, 'staged.cpp': None})

    def test_merge_base_includes_branch_and_worktree(self):
        base = self.git('rev-parse', 'HEAD')
        self.write('committed.cpp', 'int a;\n')
        self.commit()
        self.write('working.cpp', 'int b;\n')
        self.assertEqual(set(self.selected(base)), {'committed.cpp', 'working.cpp'})
        self.assertEqual(set(self.selected()), {'working.cpp'})

    def test_rename_checks_whole_file_and_deletion_is_ignored(self):
        self.git('mv', 'legacy.cpp', 'renamed.cpp')
        self.assertEqual(self.selected(), {'renamed.cpp': None})

    def test_deletion_boundary(self):
        self.write('legacy.cpp', 'int * legacy;\n\n')
        self.assertEqual(self.selected(), {'legacy.cpp': [(2, 2)]})

    def test_mode_only_does_not_reformat(self):
        self.git('config', 'core.filemode', 'true')
        (self.root / 'legacy.cpp').chmod(0o755)
        self.assertEqual(self.selected(), {'legacy.cpp': []})
        self.assertEqual(self.main('--changed')[0], 0)

    def test_invalid_base_fails_even_with_no_source_changes(self):
        status, output = self.main('--changed', '--base', 'no-such-ref')
        self.assertEqual(status, 2)
        self.assertIn('error:', output)

    def test_docs_only_is_explicitly_not_applicable(self):
        self.write('README.md', 'documentation\n')
        status, output = self.main('--changed')
        self.assertEqual(status, 0)
        self.assertIn('not applicable', output)

    def test_diff_is_read_only_and_then_passes_after_fix(self):
        self.write('new.cpp', 'int * wrong;\n')
        status, output = self.main('--changed', '--diff')
        self.assertEqual(status, 1)
        self.assertIn('STYLE001', output)
        self.assertIn('+++ b/new.cpp', output)
        self.assertEqual((self.root / 'new.cpp').read_text(), 'int * wrong;\n')
        self.write('new.cpp', 'int *right;\n')
        self.assertEqual(self.main('--changed')[0], 0)

    def historical_source(self, source):
        path = self.root / 'historical.cpp'
        path.write_bytes(source)
        self.commit()
        return path

    def test_unchanged_legacy_comment_bytes_survive_current_and_baseline(self):
        path = self.historical_source(b'// Copyright \xa9\r\nint old;\r\n')
        current = b'// Copyright \xa9\r\nint changed;\r\n'
        path.write_bytes(current)
        self.assertEqual(self.main('--changed')[0], 0)
        self.assertEqual(path.read_bytes(), current)

    def test_legacy_bytes_are_preserved_in_suggested_diff(self):
        path = self.historical_source(b'// Copyright \xa9\nint old;\n')
        current = b'// Copyright \xa9\nint * wrong;\n'
        path.write_bytes(current)
        status, output = self.main('--changed', '--diff')
        self.assertEqual(status, 1)
        self.assertIn(b' // Copyright \xa9\n', output.encode('utf-8', errors='surrogateescape'))
        self.assertIn('+int *wrong;', output)
        self.assertEqual(path.read_bytes(), current)

    def test_new_non_utf8_text_is_not_silently_accepted(self):
        (self.root / 'new.cpp').write_bytes(b'// Copyright \xa9\nint x;\n')
        status, output = self.main('--changed')
        self.assertEqual(status, 2)
        self.assertIn('invalid UTF-8', output)

    def test_modified_non_utf8_comment_requires_explicit_encoding_fix(self):
        path = self.historical_source(b'// Copyright \xa9\nint x;\n')
        path.write_bytes(b'// Copyright \xa9 changed\nint x;\n')
        self.assertEqual(self.main('--changed')[0], 2)

    def test_legacy_invalid_code_token_still_fails(self):
        path = self.historical_source(b'int \xffname;\nint old;\n')
        path.write_bytes(b'int \xffname;\nint changed;\n')
        status, output = self.main('--changed')
        self.assertEqual(status, 2)
        self.assertIn('code token', output)

    def test_utf8_current_with_legacy_baseline_is_readable(self):
        path = self.historical_source(b'// Copyright \xa9\nint old;\n')
        path.write_bytes('// Copyright ©\nint changed;\n'.encode('utf-8'))
        self.assertEqual(self.main('--changed')[0], 0)

    def test_legacy_bytes_do_not_hide_new_include_violation(self):
        path = self.historical_source(b'// Copyright \xa9\n#include "memdbgon.h"\n')
        path.write_bytes(b'// Copyright \xa9\n#include "memdbgon.h"\n#include "late.h"\n')
        status, output = self.main('--changed')
        self.assertEqual(status, 1)
        self.assertIn('STYLE002', output)

    def test_full_file_audit_does_not_invent_encoding_baseline(self):
        self.historical_source(b'// Copyright \xa9\nint x;\n')
        self.assertEqual(self.main('historical.cpp')[0], 2)

    def test_formatter_receives_legacy_source_bytes_without_reencoding(self):
        source = b'// Copyright \xa9\nint * wrong;\n'
        formatted, _ = lint.format_bytes(ROOT, FORMATTER, 'legacy.cpp', source, [(2, 2)])
        self.assertEqual(formatted, b'// Copyright \xa9\nint *wrong;\n')

    def test_include_violation_is_not_hidden_by_formatting(self):
        self.write('new.cpp', '#include "memdbgon.h"\n#include "a.h"\n')
        status, output = self.main('--changed')
        self.assertEqual(status, 1)
        self.assertIn('STYLE002', output)

    def test_explicit_and_all_are_full_file_audits(self):
        self.assertEqual(self.main('legacy.cpp')[0], 1)
        self.assertEqual(self.main('--all')[0], 1)

    def test_symlink_and_invalid_paths_fail(self):
        (self.root / 'link.cpp').symlink_to(self.root / 'legacy.cpp')
        for name in ['link.cpp', '../escape.cpp', 'missing.cpp', '/outside.cpp']:
            with self.subTest(name=name):
                self.assertEqual(self.main(name)[0], 2)

    def test_invalid_argument_combinations(self):
        for args in [[], ['--base', 'HEAD'], ['--changed', 'legacy.cpp']]:
            with self.subTest(args=args), self.assertRaises(SystemExit) as error:
                self.main(*args)
            self.assertEqual(error.exception.code, 2)


if __name__ == '__main__':
    unittest.main()
