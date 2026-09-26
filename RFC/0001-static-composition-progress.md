# RFC 0001 static composition (iOS prerequisite)

Updated: 2026-09-25

## Scope

iOS links every first-party module into the app (AGENTS.md "Platform-compliant
composition"). This slice makes that composition build and run on Linux, where
it can be tested, before any Apple toolchain exists here. It serves R29 (the
iOS static-composition proof) and R39 (typed linked factories instead of
filename discovery). It closes neither row.

Owner: the iOS port session (source-engine-38), at the user's direction:
"build the iOS version, but before we can do that you need to get static
composition compiling."

## Mechanism

`./waf configure --static-composition` (`scripts/waifulib/static_composition.py`)
builds every first-party shared library of the product as a **module object**:

1. The module's own objects and the first-party static libraries it links
   privately (tier1, mathlib, ...) are partially linked with `-r`.
   `--force-group-allocation` dissolves COMDAT groups, so the final link cannot
   fold one module's inline functions into another's.
2. `objcopy --localize-hidden` makes every symbol the module did not export
   local. Each module keeps its own tier1 copy, globals, interface registry and
   inline-function statics, as it did as a shared library. GCC builds use
   `-fno-gnu-unique`, since `STB_GNU_UNIQUE` statics would stay global.
3. Only exported (default visibility) symbols stay global. `CreateInterface` is
   hidden in this configuration (`public/tier1/interface.h`); each module with
   an interface registry exports one generated adapter,
   `StaticModule_<target>_CreateInterface`, instead.
4. Programs link the closure of module objects they use, foundation first, plus
   the system libraries those modules use. A module object never links another
   module object.

The global symbols of the 22 linked module objects equal the shared build's
dynamic exports, apart from the renamed `CreateInterface` and the physics
entries below. The static composition therefore keeps the shared-library
symbol boundary.

Mach-O is not implemented yet: `ld64 -r` localizes hidden symbols itself,
and the tool fails configure for Apple targets until that step exists.

## Composition changes

Typed linked factories replace the runtime loads that a static product
hit, so the product needs no module search path. Desktop products share the
first three.

| Load | Before | Now |
| --- | --- | --- |
| Physics (launcher) | `LoadModule( "<-physics>.so" )` | `PhysicsIVP_Describe()` / `PhysicsBox3D_Describe()` catalog (`public/vphysics/provider_catalog.h`); `-physics` selects by name; `Physics_Create` (no callers, duplicated by both providers) deleted |
| File system and queued loader (launcher) | `LoadModule( filesystem_stdio )` twice | `FileSystemStdio_Create()` passed to `CSteamApplication`; `FileSystemStdio_CreateQueuedLoader()` |
| `sourcevr` (launcher) | `LoadModule`, always failing | removed: no product builds it |
| Engine tool framework | the engine loading `engine.so` to reach itself | `AddSystem( toolframework, ... )` |
| Client, server, GameUI (engine) | loaded by name from `GAMEBIN` / `EXECUTABLE_PATH` | static products bind them with `Engine_BindLinkedGameModules` (`public/engine/linked_game_modules.h`); desktop products bind nothing and keep the game-extension loading |
| Game-declared app systems (soundemittersystem, scenefilecache) | loaded by module name | static products bind them by interface version; a declared system that was not linked fails composition |
| Server factory users (`sv_plugin`, `enginetool`, material proxies) | `Sys_GetFactory( module )` | the engine's stored factories, valid for both kinds of product |
| Steam platform menu (server browser) | loaded by the DLL name a content file gives | not in static products |

The static root is `launcher_main/static_composition.cpp`. It binds the linked
client, server and GameUI entries and the two app systems before
`LauncherMain`.

A lifetime bug surfaced: the client's static `CRopeManager` released material
references in its destructor. In a static image that runs at process exit,
after the material system has shut down (on desktop, `dlclose` of the client
ran it earlier). `IRopeManager::Shutdown()` now releases them from
`CHLClient::Shutdown`, which also releases the depth-write material that was
never released before.

## Decisions (agent decisions under the user's standing instruction, 2026-09-25)

- Module objects with symbol localization, not a single merged link or
  per-symbol renaming. Why: it keeps each module's tier1 state, registry and
  ConVar list separate, exactly as shared libraries do; renaming needs the
  whole symbol table and misses inline statics.
- The launcher links both physics providers and the file system on desktop
  too. Why: it removes two filename loads from the client root (R39), and
  Android already packages both physics modules.
- `public/engine/linked_game_modules.h` joins the legacy ABI package
  (`architecture/modules.json` `legacyAbi`), like the dedicated composition
  bridge. Why: the game module contract is the versioned `CreateInterface` ABI;
  the table passes those factories and nothing else.
- Static products leave out the server browser and the platform-menu loader.
  Why: those modules are Steam desktop features, found by the DLL name a
  content file gives, which the iOS policy excludes.
- `utils/vtex` and the loader test fixtures are not built in static trees: a
  desktop tool module and shared libraries by definition.

## Evidence

Revision `12310bba` plus this change, applied alone in a detached worktree,
because the shared tree's engine did not compile (another session's
unfinished audio work).

Build and check:

```sh
WAFLOCK=.lock-waf-static ./waf configure -o build-static --build-games portal \
  --platform-provider sdl3 --render-backend native-vulkan --disable-warns -T release \
  --static-composition --ktx-source-root dependencies/pbrt-map/ktx-software \
  --ktx-build-root dependencies/pbrt-map/ktx-reader-build
WAFLOCK=.lock-waf-static ./waf build --targets=hl2_launcher
python3 tools/quality/static_composition.py check --tree build-static \
  --program launcher_main/hl2_launcher --require client --require server \
  --require GameUI --require engine --require launcher --require vphysics \
  --require vphysics_box3d --require filesystem_stdio --require scenefilecache \
  --require soundemittersystem
python3 -m unittest tools/quality/tests/test_static_composition.py
```

- `hl2_launcher` is one 56.6 MB program. Its `DT_NEEDED` entries are system
  libraries only: SDL3, Vulkan, OpenAL, curl, freetype, fontconfig, png, jpeg,
  z, zstd, bz2, libstdc++ and libc. The tree builds no `.so`.
- Checker: PASS, 24 module objects, 22 linked entries, 0 errors. Before the
  physics catalog it failed on `Physics_Create` defined by both providers and
  an unlinked `vtex_dll`.
- Checker self-tests: 8 pass. Seven seeded defects are each reported: a hidden
  symbol left global, a GNU-unique symbol, a strong symbol in two modules, a
  first-party `DT_NEEDED`, a shared library in the tree, an unlinked module and
  a missing required module.
- Runtime, from a runtime copy with every first-party `.so` removed, with
  `SDL_VIDEO_DRIVER=offscreen`: `+map testchmb_a_00 +wait 300 +quit` exits 0
  with `-renderer null` (IVP and Box3D) and with `-renderer native-vulkan`
  (RADV device up, 4x MSAA back buffer).
  - `-moduleloadtelemetry` reports 0 loader events.
  - `LD_DEBUG=files` shows only system libraries: SDL video drivers, the
    Vulkan loader and ICDs, and their dependencies.
  - Control: the desktop product under the same flag reports 219 events for
    14 module names.
- Desktop shared build of the same source: `testchmb_a_00` exits 0 with
  `-physics vphysics` and `-physics vphysics_box3d`.
  - Breakpoints confirm `CreateIVPPhysics` and `CreateBox3DPhysics`
    respectively.
  - `-physics bogus` is refused with "Required physics provider 'bogus' is not
    available in this product."
  - The filesystem, physics, `engine` and `sourcevr` loads are gone. The client
    still loads launcher, client, server, GameUI, ServerBrowser,
    soundemittersystem and scenefilecache by name.
- Dedicated build of the same source: `testchmb_a_00` runs (`status` shows the
  map) and exits 0.
- `archlint check --all`, `baseline --verify` and `inventory --verify` pass.
  - Reviewed ratchet changes: 6 loader occurrences removed (2 `sv_plugin`, 1
    `enginetool`, 1 material-proxy `Sys_GetFactory`, and the pair of
    `CreateInterfaceFn` + `Sys_GetFactory` in each `sv_plugin` line).
  - Relocated excerpts: 2 `sv_plugin` `CreateInterfaceFn gameServerFactory`
    lines, and the client and server `Sys_LoadModuleFromFileSystem` lines,
    which were re-indented under the desktop branch.
  - Inventory: 318 → 314 sites.
- Style check on the changed lines: 0 failures. Archlint tests: 131 pass.

## iOS host toolchain and Mach-O module objects (2026-09-25)

The user asked to do as much as possible without a Mac. There is no iOS SDK on
this host yet, so nothing has been compiled for iOS.

- **Toolchain.** `python3 tools/ios/build_toolchain.py` builds a Linux-hosted
  iOS cross toolchain into `dependencies/ios/toolchain`, from pins in
  `quality/product_profiles/portal-ios-native-vulkan.json`:
  - LLVM 22.1.8: clang and ld64.lld. The OpenPGP signature was verified once
    against the LLVM release keys; the sha256 is the pin.
  - swift-corelibs-libdispatch swift-6.4.0-RELEASE.
  - cctools-port `904de2a7`, which provides Apple's ld64-956.6.
  - Stages rebuild only when their inputs change: a rerun reports all three
    up to date.
  - The script ends with an SDK-free smoke test. `ld64 -r` must localize the
    hidden symbols of an arm64-apple-ios module object, and ld64.lld must link
    a program from it. It passes.
- **Why Apple's ld64.** LLVM's ld64.lld rejects `-r` ("not yet implemented").
  Apple's `ld64 -r` localizes hidden symbols itself, including inline-function
  statics, and pulls members from private archives.
  `scripts/waifulib/static_composition.py` uses it for Mach-O targets
  (`DEST_BINFMT` `mac-o`) in place of `-r` plus objcopy.
- **Checker.** `tools/quality/static_composition.py` now reads Mach-O files:
  `llvm-nm -m` visibility and the load-command dylibs.
  - Its tests run the same seeded defects for ELF and for Mach-O (built with
    the pinned toolchain): 16 tests, with the GNU-unique case skipped on
    Mach-O.
  - The Mach-O tests skip when the toolchain is absent (hosted CI).
- **MoltenVK.** The pinned Khronos release v1.4.2 `MoltenVK-ios.tar` matches
  the published digest. Its static `libMoltenVK.a` targets iOS (minos 15.0,
  SDK 26.5); building MoltenVK itself would need Xcode.
- **No loading in static products.** In a static product, `Sys_LoadLibraryWithError`
  refuses every first-party module load, and the refusal is recorded like any
  other failed load.
  - Evidence: with a loadable `vstdlib.so` placed where `-tools` resolves it,
    telemetry reports `success=0 error=static composition: modules are
    linked, not loaded`.
  - System-library probes are unaffected.
- **Portability.** The native Vulkan backend now opts into portability
  enumeration when the instance offers it (`VK_KHR_portability_enumeration`
  and `VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR`). It enables
  `VK_KHR_portability_subset` when the device exposes it, as MoltenVK does.
  - Linux (RADV, whose loader offers the instance extension): the static
    product still boots `testchmb_a_00` on native Vulkan with 0 loader events.
  - The device-side path has not run: no Linux driver exposes the subset.
- **Waf iOS target (up to the SDK).**
  - `./waf configure --ios-sdk=<iPhoneOS.sdk> --static-composition
    --platform-provider sdl3 --render-backend native-vulkan` selects the
    pinned toolchain (`--ios-toolchain`, default `dependencies/ios/toolchain`)
    and `arm64-apple-ios<--ios-deployment-target, 17.0>`.
  - The target is part of the compiler command, and xcompile maps
    `__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__` to `DEST_OS` `ios`. Waf's
    own table maps it to darwin, and the old merge overwrote the new entry.
  - iOS requires `--static-composition`, SDL3 and native Vulkan.
  - Defines: `OSX` for Apple POSIX semantics, plus `PLATFORM_IOS` to exclude
    the macOS-only APIs (agent decision under the user's standing
    instruction). Why: the tree's 366 `OSX` conditionals select mach timing,
    sysctl, `malloc/malloc.h` and BSD `qsort_r` argument order, which iOS
    shares.
  - Checked with a directory that only claims to be an iPhoneOS SDK (a
    settings file, no headers):
    - configure reports ios / aarch64 / mac-o, passes the product gate, and
      stops at the first compile-and-link probe;
    - a missing or macOS SDK is rejected with a message;
    - the Linux configure is unchanged.
- **Apple audit.** A read-only audit of the product modules for iOS is the
  punch list for the SDK phase.
  - No `ios` target exists: Waf would call an iOS compiler `darwin` and apply
    the macOS defines, frameworks and audio/voice sources.
  - macOS-only APIs are compiled into the product: Carbon (sys_dll, the voice
    and OpenAL code, MatSystemSurface, videoservices), the Pasteboard
    (vgui2 system_posix), FSEvents (tier1 fileio), SCDynamicStore
    (downloadthread) and the CoreAudio HAL.
  - Process spawning: `fork`/`system("open …")` in vgui2, the launcher and
    FileOpenDialog; `/tmp` lock and relaunch files.
  - Dependencies: fontconfig is linked unconditionally by vguimatsurface, and
    Vulkan and KTX are configured only for linux/android.
  - Model: the Android SDL3 prefix wiring (`ANDROID_SDL3`: pkg-config into a
    cross-built prefix; in-tree bzip2; no fontconfig or OpenAL).

## First iOS build (2026-09-25)

The iPhoneOS 26.5 SDK came from Xcode 26.6 on the user's macOS VM (`ssh
macvm`). It is copied into `dependencies/ios/sdk` (gitignored) and never
downloaded. The pinned toolchain links a C++20 program against it.

`./build-ios-app.sh` builds the iOS client end to end on Linux:

1. It runs the toolchain script.
2. It checks the SDK (`--sdk`, or `--fetch-sdk-from HOST` over ssh).
3. It builds `libclang_rt.ios.a` from the pinned LLVM `os_version_check.c`,
   for `@available`. Xcode ships this file; libSystem has the other builtins.
4. It cross-builds with CMake (`tools/ios/ios-toolchain.cmake`): SDL3 (static;
   Metal, Vulkan, UIKit), the static KTX reader, freetype (on the SDK's zlib),
   libpng, libjpeg and curl. MoltenVK v1.4.2 comes from its pinned static
   release.
5. It configures and builds the engine with `--ios-sdk --static-composition`.
6. It runs the static-composition check as a required step.
7. It assembles an unsigned `Portal.app` whose Info.plist is generated from the
   profile: bundle id `com.panos.sourceengine` (user choice), iOS 17.0,
   arm64 + Metal, landscape, launch screen, and Files-app access to Documents.

The result:

- `hl2_launcher` is a 54 MB arm64 Mach-O: platform iOS, minos 17.0, SDK 26.5.
- It loads only system frameworks and libraries: UIKit, Metal, QuartzCore,
  IOSurface, CFNetwork, GameController, AVFoundation, libc++, libSystem, libz,
  libbz2 and libiconv.
- Static-composition check: PASS, 24 module objects, 22 linked entries, 0
  errors.
- On the Mac VM: `plutil -lint` OK, `lipo`/`vtool` show arm64 and platform IOS
  17.0, and an ad-hoc `codesign` passes `--verify`.

Fixes needed for iOS (each is guarded, so desktop builds are unchanged):

- **Waf:** Waf's darwin link settings for `ios` (no ELF `-Bstatic` markers);
  `pkg-config --static` for the static dependencies; an iOS dependency branch
  like Android's, with no fontconfig or OpenAL and SDL3 audio.
- **KTX:** it builds a static framework on Apple, so its archive is copied to
  where the pin names it.
- **Vendored libraries:** freetype's bundled zlib and libpng 1.6.38 misread
  clang's predefined `TARGET_OS_MAC` (libpng fixed this in 1.6.40). freetype
  uses the SDK's zlib; libpng builds with `-fno-define-target-os-macros`.
- **IVP:** it reached `alloca` only through transitive macOS headers. The
  ivp submodule is upstream, so the IVP targets get `-include alloca.h` on
  iOS from the superproject.
- **macOS-only APIs guarded with `PLATFORM_IOS`:**
  - five Carbon includes;
  - the vgui2 clipboard, which now uses SDL3's, and ShellExecute, which now
    uses `SDL_OpenURL` (iOS has no `fork`);
  - `system()` in FileOpenDialog and the launcher relaunch file;
  - download proxies: `CFNetworkCopySystemProxySettings` replaces
    SCDynamicStore, and the engine links CFNetwork;
  - the video skip keys, which now come from SDL
    (`PeekAndRemoveKeyboardEvents`);
  - the voice mixer, which uses the portable stub.
- **Static-composition tool (a real gap, fixed):** the system libraries of a
  module's private static libraries (tier1's iconv) now reach the program link.
  Linux hid the gap because iconv is part of libc.
- **Entry point:** `launcher_main/ios_main.cpp`. SDL3 owns `main` (UIKit
  start). The content root is the app's Documents container and the library
  path is the bundle (the app-container paths shared with Android). It then
  binds the linked game modules and runs `LauncherMain` with the mobile
  arguments.
- **Signing and installing:** these belong to the user's Mac tooling
  (`~/src/mac/ios-deploy.sh`, set up in another session: free Personal Team,
  the connected iPhone 16 Pro on iOS 27). Linux produces the unsigned bundle.

## Not done

- Device evidence: the app has not been signed with a provisioning profile,
  installed or started. It needs Developer Mode on the phone, content in
  Documents, and the iOS obligations in AGENTS.md (lifecycle, touch, memory
  pressure).
- The static root's list of modules lives in `launcher_main/wscript`. An iOS
  product profile should own it.
- Static products still contain the loader's callers: the `-tools` path,
  server plugin discovery and optional providers such as haptics and p4lib.
  Their loads are now refused rather than compiled out. The link-map check of
  the iOS package is still to come.
- Module static destructors run at process exit in a static image. Only the
  rope manager was fixed; this run found no other exit-time faults, and no
  sanitizer run of the static product exists.
- No CI lane builds `--static-composition`, and hosted CI has not run.
