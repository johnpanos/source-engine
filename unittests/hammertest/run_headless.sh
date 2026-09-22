#!/bin/sh
# Headless strict-module test target for RFC 0002 (HAM-BUILD-001).
#
# Runs the Hammer editor's headless conformance suites (RFC 0002, Q-EDITOR) on
# the linux-headless-core profile: C++20, no MFC, no tier0/platform.h, no PCH,
# no display, no GPU. It delegates to the shared conformance runner so the suite
# source lists, include roots, and flags live in ONE authority
# (quality/conformance.manifest.json + quality/profiles/), and cannot drift out
# of sync with this script -- which is exactly what happened before, when
# aabb.cpp grew a rounding.cpp dependency this script did not track.
#
# Usage: unittests/hammertest/run_headless.sh
# Exit:  0 = all Q-EDITOR RFC 0002 suites pass under every compiler, non-zero otherwise.

set -eu

ROOT="$( cd "$( dirname "$0" )/../.." && pwd )"
RUNNER="$ROOT/tools/quality/conformance.py"

# Build and run the RFC 0002 editor suites under each available compiler, matching
# the historical intent of this script (gcc and clang, -Wall -Wextra -Werror).
COMPILERS="${HAMMERTEST_CXX:-g++ clang++}"

status=0
for cxx in $COMPILERS; do
	if ! command -v "$cxx" >/dev/null 2>&1; then
		echo "hammertest: skipping unavailable compiler $cxx"
		continue
	fi
	echo "hammertest: running RFC 0002 Q-EDITOR suites with $cxx"
	if ! python3 "$RUNNER" check --cxx "$cxx" --rfc 0002 --domain Q-EDITOR --out -; then
		status=1
	fi
done

if [ "$status" -eq 0 ]; then
	echo "hammertest: all headless suites passed"
fi
exit "$status"
