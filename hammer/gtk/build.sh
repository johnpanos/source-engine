#!/bin/sh
# Developer build for the GTK4 + libadwaita Hammer desktop shell (RFC 0002,
# linux-gtk-desktop). This is a separate product from the engine's waf build; it
# links the strict headless editor core (hammer/core) with a native GTK UI.
#
# Requires: gtk4, libadwaita-1, epoxy (pkg-config), a C++20 compiler.
# Usage: hammer/gtk/build.sh [output-binary]   (run from the repo root or anywhere)
set -eu

ROOT="$( cd "$( dirname "$0" )/../.." && pwd )"
OUT="${1:-$ROOT/hammer/gtk/hammer_gtk}"
CXX="${CXX:-g++}"

PKGS="gtk4 libadwaita-1 epoxy"

CORE="\
$ROOT/hammer/core/geometry/brush.cpp \
$ROOT/hammer/core/formats/keyvalues.cpp \
$ROOT/hammer/core/app/editor_document.cpp \
$ROOT/hammer/core/app/document_history.cpp \
$ROOT/hammer/core/app/save_orchestrator.cpp"

HOST="\
$ROOT/hammer/gtk/app.cpp \
$ROOT/hammer/gtk/offscreen.cpp \
$ROOT/hammer/gtk/renderer.cpp"

# shellcheck disable=SC2046
"$CXX" -std=c++20 -Wall -Wextra -Werror -O2 \
	-I"$ROOT/public" -I"$ROOT/hammer/gtk" \
	$(pkg-config --cflags $PKGS) \
	$HOST $CORE \
	$(pkg-config --libs $PKGS) \
	-o "$OUT"

echo "built $OUT"
