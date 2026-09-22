# Source Engine
[![GitHub Actions Status](https://github.com/nillerusr/source-engine/actions/workflows/build.yml/badge.svg)](https://github.com/nillerusr/source-engine/actions/workflows/build.yml) [![GitHub Actions Status](https://github.com/nillerusr/source-engine/actions/workflows/tests.yml/badge.svg)](https://github.com/nillerusr/source-engine/actions/workflows/tests.yml)
 Discord: [![Discord Server](https://img.shields.io/discord/672055862608658432.svg)](https://discord.gg/hZRB7WMgGw)
 

Information from [wikipedia](https://wikipedia.org/wiki/Source_(game_engine)):

Source is a 3D game engine developed by Valve.
It debuted as the successor to GoldSrc with Half-Life: Source in June 2004,
followed by Counter-Strike: Source and Half-Life 2 later that year.
Source does not have a concise version numbering scheme; instead, it was released in incremental versions

Source code is based on TF2 2018 leak. Don't use it for commercial purposes.

This project is using waf buildsystem. If you have waf-related questions look https://waf.io/book

# Features:
- Android, OSX, FreeBSD, Windows, Linux( glibc, musl ) support
- Arm support( except windows )
- 64bit support
- Modern toolchains support
- Fixed many undefined behaviours
- Touch support( even on windows/linux/osx )
- VTF 7.5 support
- PBR support
- bsp v19-v21 support( bsp v21 support is partial, portal 2 and csgo maps works fine )
- mdl v46-49 support
- Removed useless/unnecessary dependencies
- Achivement system working without steam
- Fixed many bugs
- Serverbrowser works without steam

# Current tasks
- Rewrite materialsystem for OpenGL render
- dxvk-native support
- Elbrus port
- Bink audio support( for video_bink )

# How to Build

## Fedora Linux 44

These instructions are for a traditional x86-64 Fedora installation using DNF. Install the build tools and 64-bit development libraries with:

```sh
sudo dnf install \
    git python3 gcc gcc-c++ make pkgconf-pkg-config ccache \
    sdl2-compat-devel freetype-devel fontconfig-devel \
    openal-soft-devel libjpeg-turbo-devel libpng-devel libcurl-devel \
    bzip2-devel libedit-devel zlib-ng-compat-devel
```

Clone the repository with its submodules, then configure and build a release version:

```sh
git clone --recursive --depth 1 https://github.com/nillerusr/source-engine.git
cd source-engine
python3 ./waf configure -T release --use-ccache
python3 ./waf build
```

If the repository was cloned without `--recursive`, initialize its submodules before configuring:

```sh
git submodule update --init --recursive
```

### 32-bit build on x86-64

Install the 32-bit development libraries in addition to the packages above:

```sh
sudo dnf install \
    glibc-devel.i686 libstdc++-devel.i686 \
    sdl2-compat-devel.i686 freetype-devel.i686 fontconfig-devel.i686 \
    openal-soft-devel.i686 libjpeg-turbo-devel.i686 libpng-devel.i686 \
    libcurl-devel.i686 bzip2-devel.i686 libedit-devel.i686 \
    zlib-ng-compat-devel.i686
```

Point `pkg-config` at Fedora's 32-bit metadata and enable the WAF 32-bit target:

```sh
PKG_CONFIG_LIBDIR=/usr/lib/pkgconfig:/usr/share/pkgconfig \
    python3 ./waf configure -T release --32bits --use-ccache
python3 ./waf build
```

### Optional Opus voice support

The engine requires Opus custom modes, which Fedora's standard `opus-devel` package does not enable. Build Opus from source with `--enable-custom-modes` as described in the [English building instructions](https://github.com/nillerusr/source-engine/wiki/Source-Engine-%28EN%29), then add `--enable-opus` to the WAF configure command. For a 32-bit engine, build Opus with 32-bit compiler flags as well.

## Other platforms

- [Building instructions (English)](https://github.com/nillerusr/source-engine/wiki/Source-Engine-%28EN%29)
- [Building instructions (Russian)](https://github.com/nillerusr/source-engine/wiki/Source-Engine-%28RU%29)

# Support me
BTC: `bc1qnjq92jj9uqjtafcx2zvnwd48q89hgtd6w8a6na`

ETH: `0x5d0D561146Ed758D266E59B56e85Af0b03ABAF46`

XMR: `48iXvX61MU24m5VGc77rXQYKmoww3dZh6hn7mEwDaLVTfGhyBKq2teoPpeBq6xvqj4itsGh6EzNTzBty6ZDDevApCFNpsJ`
