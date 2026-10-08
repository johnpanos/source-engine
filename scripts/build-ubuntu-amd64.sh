#!/bin/sh

git submodule init && git submodule update
sudo apt-get update
sudo apt-get install -f -y libopenal-dev g++-multilib gcc-multilib libpng-dev libjpeg-dev libfreetype6-dev libfontconfig1-dev libcurl4-gnutls-dev libsdl2-dev zlib1g-dev libbz2-dev libedit-dev

# The dedicated (-d) lane; clients render only through native Vulkan.
# RFC 0016 K4: the render core embeds SPIR-V the build generates with the
# pinned shader tools (built once from sha256-pinned sources).
python3 tools/render/shader_toolchain.py build &&
./waf configure -T debug --disable-warns $* &&
./waf build
