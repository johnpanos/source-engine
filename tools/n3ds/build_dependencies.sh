#!/bin/sh
# Cross-build the 3DS profile's pinned SDL3 into dependencies/3ds/prefix
# (run inside the devkitarm container by build-3ds.sh).
set -e
cd "$(dirname "$0")/../../dependencies/3ds"
archive=archives/SDL3-3.4.16.tar.gz
echo "7322236cd12090c3eb40b9728be4d49c76f66ad17d04369584d4ecad5cf77c68  $archive" | sha256sum -c -
mkdir -p src && tar xzf $archive -C src
cmake -S src/SDL3-3.4.16 -B build/sdl3 -DCMAKE_TOOLCHAIN_FILE="$DEVKITPRO/cmake/3DS.cmake" \
	-DCMAKE_BUILD_TYPE=Release -DSDL_SHARED=OFF -DSDL_STATIC=ON -DSDL_TESTS=OFF -DSDL_EXAMPLES=OFF \
	-DCMAKE_INSTALL_PREFIX="$PWD/prefix" >/dev/null
cmake --build build/sdl3 -j16
cmake --install build/sdl3 >/dev/null
