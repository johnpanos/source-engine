#!/bin/sh
# 3D camera navigation math test for the GTK Hammer shell (RFC 0002).
#
# Compiles hammer/gtk/tests/test_camera_nav.cpp against the real
# hammergtk::Renderer and runs it. The test exercises the camera math the
# interactive presenter drives for the classic Hammer navigation -- FlyMove
# (WASD/QE flight), FlyLook (Z mouse-look, eye stays put) and PixelToRay
# (3D click-to-select) -- with no window or GL context, so it runs anywhere.
#
# Usage: hammer/gtk/tests/camera_nav_test.sh
# Exit:  0 = pass, non-zero = build or check failure.
set -eu

ROOT="$( cd "$( dirname "$0" )/../../.." && pwd )"
CXX="${CXX:-g++}"
WORK="$( mktemp -d )"
trap 'rm -rf "$WORK"' EXIT

BIN="$WORK/test_camera_nav"
echo "camera_nav_test: building with $CXX..."
# renderer.cpp calls GL only in its draw/upload paths (not exercised here), but
# still links libepoxy for those symbols; no context is created.
"$CXX" -std=c++20 -Wall -Wextra -O2 \
	-I"$ROOT/public" -I"$ROOT" -I"$ROOT/hammer/gtk" \
	$(pkg-config --cflags epoxy) \
	"$ROOT/hammer/gtk/tests/test_camera_nav.cpp" \
	"$ROOT/hammer/gtk/renderer.cpp" \
	"$ROOT/hammer/core/formats/material_catalog.cpp" \
	"$ROOT/hammer/core/formats/material.cpp" \
	"$ROOT/hammer/core/formats/vtf_image.cpp" \
	"$ROOT/hammer/core/formats/vpk_archive.cpp" \
	"$ROOT/hammer/core/formats/search_path_assets.cpp" \
	"$ROOT/hammer/core/geometry/brush.cpp" \
	"$ROOT/hammer/core/geometry/displacement.cpp" \
	"$ROOT/hammer/core/formats/keyvalues.cpp" \
	$(pkg-config --libs epoxy) \
	-o "$BIN"

echo "camera_nav_test: running..."
"$BIN"
