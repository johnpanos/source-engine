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

KTX_ENABLED=0
if [ -n "${KTX_SOURCE_ROOT:-}" ] || [ -n "${KTX_BUILD_ROOT:-}" ]; then
	if [ -z "${KTX_SOURCE_ROOT:-}" ] || [ -z "${KTX_BUILD_ROOT:-}" ]; then
		echo "set both KTX_SOURCE_ROOT and KTX_BUILD_ROOT for KTX2 previews" >&2
		exit 2
	fi
	KTX_SOURCE_ROOT=$( cd "$KTX_SOURCE_ROOT" && pwd -P )
	KTX_BUILD_ROOT=$( cd "$KTX_BUILD_ROOT" && pwd -P )
	EXPECTED_KTX_REVISION=$( python3 -c 'import json, sys; print(json.load(open(sys.argv[1]))["dependencies"]["ktx_software"]["revision"])' \
		"$ROOT/quality/product_profiles/ktx2-linux-tools.json" )
	ACTUAL_KTX_REVISION=$( git -C "$KTX_SOURCE_ROOT" rev-parse HEAD )
	if [ "$ACTUAL_KTX_REVISION" != "$EXPECTED_KTX_REVISION" ]; then
		echo "KTX source revision differs from the pinned profile" >&2
		exit 2
	fi
	if [ -n "$( git -C "$KTX_SOURCE_ROOT" status --porcelain --untracked-files=no )" ]; then
		echo "KTX source checkout has tracked changes" >&2
		exit 2
	fi
	KTX_READER_ARCHIVE="$KTX_BUILD_ROOT/lib/libktx_read.a"
	if [ ! -f "$KTX_READER_ARCHIVE" ] || \
		! grep -Fqx "CMAKE_HOME_DIRECTORY:INTERNAL=$KTX_SOURCE_ROOT" "$KTX_BUILD_ROOT/CMakeCache.txt" || \
		! grep -Fqx "CMAKE_BUILD_TYPE:STRING=Release" "$KTX_BUILD_ROOT/CMakeCache.txt"; then
		echo "build the pinned ktx_read target before building Hammer" >&2
		exit 2
	fi
	KTX_ENABLED=1
fi

# The editor core the shell links: the interaction authority (EditorController)
# and the shared services it composes, plus the DiskFileStore adapter for file I/O.
CORE="\
$ROOT/hammer/core/app/editor_controller.cpp \
$ROOT/hammer/core/geometry/brush.cpp \
$ROOT/hammer/core/geometry/displacement.cpp \
$ROOT/hammer/core/geometry/rounding.cpp \
$ROOT/hammer/core/formats/keyvalues.cpp \
$ROOT/hammer/core/formats/vmf_geometry.cpp \
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

# shellcheck disable=SC2046,SC2086
set -- -std=c++20 -Wall -Wextra -Werror -O2 \
	"-I$ROOT/public" "-I$ROOT" "-I$ROOT/hammer/gtk" \
	$(pkg-config --cflags $PKGS) $HOST $CORE
if [ "$KTX_ENABLED" -eq 1 ]; then
	set -- "$@" -DHAMMER_KTX_PREVIEW \
		"-I$KTX_SOURCE_ROOT/lib/include" "-I$KTX_SOURCE_ROOT/external/dfdutils" \
		"$ROOT/hammer/adapters/source/ktx2_preview.cpp" \
		"$ROOT/texturecontainer/ktx2_reader.cpp"
fi
# shellcheck disable=SC2046
set -- "$@" $(pkg-config --libs $PKGS)
if [ "$KTX_ENABLED" -eq 1 ]; then
	set -- "$@" "$KTX_READER_ARCHIVE" -lz -lzstd
fi
"$CXX" "$@" -o "$OUT"

echo "built $OUT"
