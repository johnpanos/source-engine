#!/bin/sh

git submodule init && git submodule update
# RFC 0016 K4: the render core embeds SPIR-V the build generates with the
# pinned shader tools (built once from sha256-pinned sources).
python3 tools/render/shader_toolchain.py build &&
./waf configure -T release --sanitize=address,undefined --disable-warns --tests --prefix=out/ $* &&
./waf install &&
cd out &&
./unittest &&
DYLD_LIBRARY_PATH=bin/ ./unittest_legacy || exit 1
