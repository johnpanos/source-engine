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

# The editor core the shell links: the interaction authority (EditorController)
# and the shared services it composes, plus the DiskFileStore adapter for file I/O.
CORE="\
$ROOT/hammer/core/app/editor_controller.cpp \
$ROOT/hammer/core/geometry/brush.cpp \
$ROOT/hammer/core/geometry/rounding.cpp \
$ROOT/hammer/core/formats/keyvalues.cpp \
$ROOT/hammer/core/formats/vpk_archive.cpp \
$ROOT/hammer/core/formats/vtf_image.cpp \
$ROOT/hammer/core/formats/material.cpp \
$ROOT/hammer/core/formats/material_catalog.cpp \
$ROOT/hammer/core/formats/search_path_assets.cpp \
$ROOT/hammer/core/app/document_history.cpp \
$ROOT/hammer/core/app/save_orchestrator.cpp \
$ROOT/hammer/adapters/platform/disk_file_store.cpp \
$ROOT/hammer/adapters/platform/disk_byte_store.cpp"

HOST="\
$ROOT/hammer/gtk/app.cpp \
$ROOT/hammer/gtk/offscreen.cpp \
$ROOT/hammer/gtk/renderer.cpp"

# shellcheck disable=SC2046
"$CXX" -std=c++20 -Wall -Wextra -Werror -O2 \
	-I"$ROOT/public" -I"$ROOT" -I"$ROOT/hammer/gtk" \
	$(pkg-config --cflags $PKGS) \
	$HOST $CORE \
	$(pkg-config --libs $PKGS) \
	-o "$OUT"

echo "built $OUT"
