#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
#
# Self-tests for the Clang/64-bit legacy sweep (tools/quality/corpus/
# clang64_sweep.py, suite corpus.clang64.sweep). They prove the oracle's parts
# before the ratchet is trusted: diagnostic parsing, the hazard classification
# (including the LP64 printf width filter and the pointer-sized narrowing
# split), the recorded-argv translation (the products' -w must not survive),
# and the exact ratchet (new, stale, multiplicity, line moves). When clang++ is
# installed, the seeded TU is swept for real. Run:
#
#   python3 -m unittest tools/quality/tests/test_corpus_clang64.py -v
#
# ============================================================================

import os
import shutil
import sys
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
QUALITY_DIR = os.path.dirname(HERE)
sys.path.insert(0, os.path.join(QUALITY_DIR, "corpus"))
sys.path.insert(0, QUALITY_DIR)

import clang64_sweep as sweep  # noqa: E402


def site(path="a.cpp", line=1, diag="-Wformat", message="m", text="x;"):
    return {"file": path, "line": line, "column": 1, "diagnostic": diag,
            "message": message, "line_text": text, "targets": ["cfg:t"]}


class FormatWidthTest(unittest.TestCase):
    def test_size_t_with_percent_d_is_a_hazard(self):
        hazard, detail = sweep.format_width(
            "format specifies type 'int' but the argument has type 'size_t' "
            "(aka 'unsigned long')")
        self.assertTrue(hazard)
        self.assertEqual(detail, "4 vs 8 bytes")

    def test_pointer_with_percent_x_is_a_hazard(self):
        self.assertTrue(sweep.format_width(
            "format specifies type 'unsigned int' but the argument has type 'void *'")[0])

    def test_scanf_pointee_width(self):
        self.assertTrue(sweep.format_width(
            "format specifies type 'int *' but the argument has type 'size_t *' "
            "(aka 'unsigned long *')")[0])
        self.assertFalse(sweep.format_width(
            "format specifies type 'char *' but the argument has type 'unsigned char *'")[0])

    def test_same_width_and_float_are_not_hazards(self):
        self.assertFalse(sweep.format_width(
            "format specifies type 'unsigned int' but the argument has type 'int'")[0])
        self.assertFalse(sweep.format_width(
            "format specifies type 'int' but the argument has type 'float'")[0])
        self.assertFalse(sweep.format_width("more '%' conversions than data arguments")[0])

    def test_aka_resolves_typedefs(self):
        self.assertTrue(sweep.format_width(
            "format specifies type 'int' but the argument has type 'intp' (aka 'long long')")[0])
        self.assertFalse(sweep.format_width(
            "format specifies type 'unsigned long' but the argument has type 'uint64' "
            "(aka 'unsigned long long')")[0])


class ClassifyTest(unittest.TestCase):
    def test_small_int_cast_error_is_a_site(self):
        self.assertEqual(sweep.classify(
            "error", "cast from pointer to smaller type 'int' loses information", None)[:2],
            ("site", sweep.SMALL_INT_ID))

    def test_other_errors_are_blocking(self):
        self.assertEqual(sweep.classify("error", "no matching function for call to 'f'", None)[0],
                         "blocking")
        self.assertEqual(sweep.classify("fatal error", "'x.h' file not found", None)[0],
                         "blocking")

    def test_promoted_warning_keeps_its_class(self):
        self.assertEqual(sweep.classify("error", "cast to 'void *' from smaller integer type 'int'",
                                        "-Werror,-Wint-to-void-pointer-cast")[:2],
                         ("site", "-Wint-to-void-pointer-cast"))

    def test_shorten_split_by_source_type(self):
        kind, diag, _ = sweep.classify(
            "warning", "implicit conversion loses integer precision: 'const uintp' "
            "(aka 'const unsigned long long') to 'unsigned int'", "-Wshorten-64-to-32")
        self.assertEqual((kind, diag), ("site", sweep.POINTER_SIZED_ID))
        kind, _, _ = sweep.classify(
            "warning", "implicit conversion loses integer precision: 'size_t' "
            "(aka 'unsigned long') to 'int'", "-Wshorten-64-to-32")
        self.assertEqual(kind, "advisory")

    def test_unselected_warning_is_ignored(self):
        self.assertIsNone(sweep.classify("warning", "unused variable 'x'", "-Wunused-variable")[0])

    def test_diagnostic_line_parses(self):
        match = sweep.DIAGNOSTIC.match(
            "../utils/vrad/vraddisps.cpp:565:72: error: cast from pointer to smaller type "
            "'int' loses information")
        self.assertEqual((match.group("line"), match.group("col"), match.group("severity")),
                         ("565", "72", "error"))
        match = sweep.DIAGNOSTIC.match(
            "a.cpp:1:2: warning: format specifies type 'int' but the argument has type "
            "'long' [-Wformat]")
        self.assertEqual(match.group("options"), "-Wformat")


class TranslateTest(unittest.TestCase):
    def test_compile_only_and_warning_flags_dropped(self):
        argv = sweep.translate_argv(["g++", "-MMD", "-O2", "-std=c++20", "-fpermissive", "-w",
                                     "-Wall", "-Werror=format", "-Ipublic", "-DX=1",
                                     "../a.cpp", "-c", "-o/tmp/a.o", "-MF", "a.d"])
        self.assertEqual(argv[0], sweep.CLANG)
        head = argv[:-len(sweep.SWEEP_FLAGS)]
        self.assertEqual(head[1:], ["-O2", "-std=c++20", "-fpermissive", "-Ipublic", "-DX=1",
                                    "../a.cpp"])
        self.assertEqual(argv[-len(sweep.SWEEP_FLAGS):], sweep.SWEEP_FLAGS)
        self.assertNotIn("-w", argv)

    def test_compiler_is_not_a_cache_shim(self):
        self.assertNotIn(os.path.basename(os.path.realpath(sweep.CLANG)), ("ccache", "sccache"))


class RatchetTest(unittest.TestCase):
    def test_identical_is_clean(self):
        self.assertEqual(sweep.compare([site()], [site()]), ([], []))

    def test_line_move_is_not_a_change(self):
        self.assertEqual(sweep.compare([site(line=40)], [site(line=12)]), ([], []))

    def test_new_and_stale(self):
        new, stale = sweep.compare([site(text="a;")], [site(text="b;")])
        self.assertEqual([s["line_text"] for s in new], ["a;"])
        self.assertEqual([s["line_text"] for s in stale], ["b;"])

    def test_multiplicity(self):
        new, stale = sweep.compare([site(line=1), site(line=2)], [site(line=1)])
        self.assertEqual(len(new), 1)
        self.assertEqual(stale, [])
        new, stale = sweep.compare([site()], [site(line=1), site(line=2)])
        self.assertEqual((len(new), len(stale)), (0, 1))

    def test_scoped_compare_reports_no_stale(self):
        self.assertEqual(sweep.compare([], [site()], scope_new_only=True), ([], []))


@unittest.skipUnless(shutil.which("clang++"), "clang++ not installed")
class SeededSweepTest(unittest.TestCase):
    def test_seeded_sites(self):
        sites, advisory, blocking, incomplete, _ = sweep.sweep(sweep.seeded_tus(), 1)
        self.assertEqual(incomplete, [])
        self.assertEqual(blocking, [])
        self.assertEqual([(s["line"], s["diagnostic"]) for s in sites],
                         [(41, sweep.SMALL_INT_ID), (47, "-Wint-to-void-pointer-cast"),
                          (53, "-Wformat")])
        self.assertIn("( int )&ctx", sites[0]["line_text"])


if __name__ == "__main__":
    unittest.main()
