#!/bin/sh
# Build Azahar with the scripting harness (src/citra_qt/harness.{h,cpp}) in
# the KDE 6.11 Flatpak SDK, the runtime the Azahar Flatpak runs on, so
# tools/n3ds/azahar_harness.py can run the binary inside that sandbox.
# SDL2 comes from the SDK: the bundled static SDL2 defines the Wayland
# protocol tables (wl_output_interface, ...) in the executable, and Qt's
# Wayland plugin then binds to those. LTO is off: with the SDK's GCC it
# produced a build that crashes in QGuiApplication::screenAdded (upstream CI
# builds Linux with clang, no LTO).
#   flatpak install --user flathub org.kde.Sdk//6.11   (once)
set -e
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
SRC=$ROOT/dependencies/3ds/src/azahar
flatpak run --user --command=bash --filesystem="$ROOT" org.kde.Sdk//6.11 -c "
	set -e
	export CCACHE_DIR='$ROOT/dependencies/3ds/ccache-azahar'
	cd '$SRC'
	cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
		-DENABLE_QT_TRANSLATION=OFF -DENABLE_WEB_SERVICE=OFF -DCITRA_WARNINGS_AS_ERRORS=OFF \
		-DENABLE_LIBRETRO=OFF -DENABLE_ROOM=OFF -DUSE_SYSTEM_SDL2=ON -DENABLE_CCACHE=ON -DENABLE_LTO=OFF
	ninja -C build azahar"
