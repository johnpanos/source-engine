#!/bin/sh

git submodule init && git submodule update
sudo dpkg --add-architecture i386
sudo apt-get update
sudo apt-get install -y aptitude
sudo aptitude install -y libopenal-dev:i386 g++-multilib gcc-multilib libpng-dev:i386 libjpeg-dev:i386 libfreetype6-dev:i386 libfontconfig1-dev:i386 libcurl4-gnutls-dev:i386 libsdl2-dev:i386 zlib1g-dev:i386 libbz2-dev:i386 libedit-dev:i386

# RFC 0016 K4: the render core embeds SPIR-V the build generates with the
# pinned shader tools (built once from sha256-pinned sources).
python3 tools/render/shader_toolchain.py build &&
PKG_CONFIG_PATH=/usr/lib/i386-linux-gnu/pkgconfig ./waf configure -T debug --disable-warns --32bits $* &&
./waf build
