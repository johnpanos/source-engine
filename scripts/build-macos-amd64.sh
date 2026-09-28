#!/bin/sh

git submodule init && git submodule update

brew install sdl2

# RFC 0016 K4: the render core embeds SPIR-V the build generates with the
# pinned shader tools (built once from sha256-pinned sources).
python3 tools/render/shader_toolchain.py build &&
./waf configure -T debug --disable-warns $* &&
./waf build
