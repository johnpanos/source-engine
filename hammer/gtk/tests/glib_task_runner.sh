#!/bin/sh
# platform.task-runner.v1 conformance for the GTK shell's GlibTaskRunner
# (RFC 0002, R08-ASYNC-BUILD): compiles hammer/gtk/tests/test_glib_task_runner.cpp
# with the shared runner suite against GLib and runs it. Needs no display.
# Prints the checks-v1 record (corpus suite corpus.hammer.glib-runner).
#
# Usage: hammer/gtk/tests/glib_task_runner.sh [OUT_DIR]
# Exit:  0 = pass, non-zero = build or check failure.
set -eu

ROOT="$( cd "$( dirname "$0" )/../../.." && pwd )"
CXX="${CXX:-g++}"
if [ $# -ge 1 ]; then
	WORK="$1"
	mkdir -p "$WORK"
else
	WORK="$( mktemp -d )"
	trap 'rm -rf "$WORK"' EXIT
fi

BIN="$WORK/test_glib_task_runner"
"$CXX" -std=c++20 -Wall -Wextra -O2 -pthread \
	-I"$ROOT/public" -I"$ROOT" -I"$ROOT/unittests/platformtest/task_runner" \
	$(pkg-config --cflags glib-2.0) \
	"$ROOT/hammer/gtk/tests/test_glib_task_runner.cpp" \
	"$ROOT/hammer/gtk/glib_task_runner.cpp" \
	$(pkg-config --libs glib-2.0) \
	-o "$BIN"
"$BIN"
