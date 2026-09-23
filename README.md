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

# North star

Build a **job-based Source engine with Vulkan rendering, SDL3 platform
integration, and platform-compliant Linux, macOS, iOS, and Android backends**.
This requires both the engine architecture and the infrastructure to build,
verify, package, distribute, and maintain each supported platform.

- Job graphs, explicit ownership, bounded execution, and deterministic commit
  replace implicit scheduling and feature-owned threads incrementally. Serial
  execution remains a required correctness/reference mode.
- Vulkan is the target rendering API; SDL3 supplies the selected window/input
  and platform integration behind narrow contracts. On macOS and iOS, the
  planned Vulkan implementation uses [MoltenVK over Metal](https://github.com/KhronosGroup/MoltenVK),
  with its portability limits negotiated and tested rather than hidden.
- Linux, macOS, iOS, and Android are first-class program targets, not a Linux
  implementation with untested portability claims. Native lifecycle, input,
  storage, permissions, graphics, and packaging belong to explicit backends.
- Preserve declared Source content, gameplay, tool, and ABI compatibility.
  Modern C++20, results, RAII, ring buffers, and fences serve clear contracts;
  they do not substitute for behavioral tests or security boundaries.

The actionable architecture, platform/distribution constraints, gates, and ranked
roadmap live in [AGENTS.md](AGENTS.md). Domain contracts are defined by
[RFC 0001 (platform/render)](RFC/0001-capability-based-platform-architecture.md),
[0002 (Hammer)](RFC/0002-hammer-responsibility-factorization.md),
[0003 (jobs)](RFC/0003-dependency-aware-job-system.md),
[0004 (physics)](RFC/0004-box3d-primary-physics-backend.md),
[0005 (harnesses)](RFC/0005-quality-and-correctness-harnesses.md), and
[0006 (C++20/ownership/synchronization)](RFC/0006-modern-cpp-ownership-and-synchronization.md).

## Infrastructure is part of the deliverable

1. **Build:** versioned platform profiles, pinned dependencies/toolchains,
   separate host tools and target binaries, reproducible clean builds, shader
   artifacts, and native packaging. Apple and Android SDK/build integrations
   must be explicit; no requirement for a developer's private sibling checkout.
2. **Verify:** shared provider contracts and domain oracles, serial/parallel
   equivalence, malformed-input and lifetime tests, applicable sanitizers, and
   real GPU/device lifecycle tests. Cross-compilation and simulator success
   alone cannot certify a shipping device profile.
3. **Fit the platform:** on iOS, statically link first-party engine/game/provider
   modules into the app and use typed factories, not runtime shared-module
   discovery. Use public APIs, normal app lifecycle/storage, and only needed
   permissions. Do not download native plugins or require JIT, private APIs,
   or platform-policy workarounds. Use ordinary packaging/signing required by the
   intended distribution channel, not a custom signing or update framework.

App-store compatibility is an architectural requirement, not a guarantee of
approval. Check current store rules and code/content distribution rights before
submission. Basic input validation and credential hygiene still apply; a broad
security/compliance program is not a prerequisite to engine modernization.

These are target requirements, not claims that the infrastructure is already
installed. The current Waf configuration still uses SDL2 integration and legacy
C++ settings, and existing CI does not certify the new four-platform stack.
iOS is a target, not a currently verified port. Linux is the first reference
slice; Apple/mobile build and lifecycle feasibility must begin early. The GTK
editor remains a separate desktop product, not an implied mobile editor port.

# Existing features

The list below describes the project's historical feature set, not acceptance
evidence for the new job/Vulkan/SDL3 stack. Existing Windows/FreeBSD and other
profiles are not removed by defining the new north star.

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

# Current priorities

- Establish trustworthy build/test baselines and platform-compatible build profiles.
- Deliver the serial job graph, then measured parallel migrations with rollback.
- Establish SDL3 and Vulkan contracts, including Apple portability and mobile lifecycle.
- Pass each platform's native correctness, performance, packaging, and store-compatibility checks.

The [ranked roadmap](AGENTS.md#unified-ranked-roadmap) is authoritative. Legacy
OpenGL adapters and DXVK experiments are migration tools, not the final render
architecture; auxiliary features do not displace the platform infrastructure.

# Portal 2 source target (in progress)

`./play_p2 --configure-only` creates an isolated `build-p2` Waf configuration
and stages VPKs from the installed Portal 2 game into `run/runtime-p2` as
symlinks. `./play_p2` builds and launches this repository's Portal 2 target;
it currently stops at the [missing source inventory](game/shared/portal2/MISSING.md).
`./play_p2 --retail` launches the installed game directly.

# How to Build

The commands below build the existing configuration. They do not enable or
prove the proposed SDL3/Vulkan/job stack. New platform commands must be recorded
with working build and test evidence before being advertised here.

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
