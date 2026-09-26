#!/bin/sh

git submodule init && git submodule update
sudo apt-get update
sudo apt-get install -f -y libopenal-dev g++-multilib gcc-multilib libpng-dev libjpeg-dev libfreetype6-dev libfontconfig1-dev libcurl4-gnutls-dev libsdl2-dev zlib1g-dev libbz2-dev libedit-dev

# The legacy SDL2/OpenGL client profile; Waf's default client is native Vulkan.
./waf configure -T debug --disable-warns --render-backend=legacy $* &&
./waf build
