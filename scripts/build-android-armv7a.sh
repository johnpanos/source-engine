#!/bin/sh

git submodule init && git submodule update
wget https://dl.google.com/android/repository/android-ndk-r10e-linux-x86_64.zip -o /dev/null
unzip android-ndk-r10e-linux-x86_64.zip
export ANDROID_NDK_HOME=$PWD/android-ndk-r10e/
export NDK_HOME=$PWD/android-ndk-r10e/
# RFC 0016 K4: the render core embeds SPIR-V the build generates with the
# pinned shader tools (built once from sha256-pinned sources).
python3 tools/render/shader_toolchain.py build &&
./waf configure -T debug --android=armeabi-v7a-hard,4.9,21 --togles --disable-warns &&
./waf build
