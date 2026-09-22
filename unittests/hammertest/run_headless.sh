#!/bin/sh
# Headless strict-module test target for RFC 0002 (HAM-BUILD-001).
#
# Compiles and runs the strict Hammer editor modules and their conformance tests
# on the Linux headless core profile: C++20, no MFC, no tier0/platform.h, no PCH,
# no display, no GPU. This is the installed command referenced by
# RFC/0002-progress.md; it is intentionally independent of the legacy MFC build,
# which is not reproducible on this host.
#
# Usage: unittests/hammertest/run_headless.sh
# Exit:  0 = all suites pass, non-zero = compile or test failure.

set -eu

ROOT="$( cd "$( dirname "$0" )/../.." && pwd )"
CXX="${CXX:-g++}"
CXXSTD="${CXXSTD:-c++20}"
OUT="${OUT:-$ROOT/build/hammertest}"
mkdir -p "$OUT"

# Strict include roots only. No tier0/mathlib/MFC roots are permitted here; that
# is what keeps hammer.geometry dependency-free and portable.
INCLUDES="-I $ROOT/public"

WARN="-Wall -Wextra -Werror"

run_suite()
{
	name="$1"
	shift
	bin="$OUT/$name"
	# shellcheck disable=SC2086
	"$CXX" -std="$CXXSTD" $WARN $INCLUDES "$@" -o "$bin"
	"$bin"
}

echo "hammertest: building strict modules with $CXX -std=$CXXSTD"

run_suite test_aabb \
	"$ROOT/hammer/core/geometry/aabb.cpp" \
	"$ROOT/unittests/hammertest/geometry/test_aabb.cpp"

run_suite test_aabb_negative \
	"$ROOT/hammer/core/geometry/aabb.cpp" \
	"$ROOT/unittests/hammertest/geometry/test_aabb_negative.cpp"

echo "hammertest: all headless suites passed"
