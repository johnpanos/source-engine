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

## First device run, touch, motion and orientation (2026-09-25)

- **First device run (user report):** `7444d5c7` ran on the iPhone 16 Pro
  (iOS 27.0), signed and installed by the user's Mac tooling, and exited 0.
  The single-instance lock was dropped for iOS in that commit: an app sandbox
  cannot lock files in `/tmp`, and the system runs one instance anyway.
- **User direction:** port the Android touch UI ("i have no controls"), the
  motion controls, portrait, and a way to dismiss the keyboard.
- **Touch controls:** `touch_enable` and `touch_gyro` default on for
  `PLATFORM_IOS`, as on Android, and the Options dialog has the Touch page.
  `build-ios-app.sh` packages the icons from `tools/android/touch_icons.py`
  into `Portal.app/touch/`. At startup the app root installs them into
  `<game>/custom/android_touch/`, the path Android uses, through the shared
  `launcher_main/mobile_app_root.cpp`. That file now holds the installer and
  the arguments-file reader that were duplicated in `android_main.cpp`. The
  Android root compiles against it with the NDK r30 clang, and the 56 Android
  profile and APK tests pass.
- **Motion controls:** `inputsystem/gyro_sensor_ios.mm` implements the
  `CGyroSensor` contract with Core Motion's device-motion stream, which
  provides the bias-corrected rotation rate and gravity with sample
  timestamps. A private serial operation queue owns the shared integrator and
  up filter (`gyro_math`). Core Motion's device axes match Android's natural
  axes. SDL3 reports iOS display orientation with the same semantics (natural
  portrait on the main screen), so `gyro_sdl.cpp`'s screen mapping is
  unchanged. Reading the gyroscope needs no usage-description key. The
  binary links CoreMotion.
- **Orientation:** all four orientations, as on Android (SDL hint and
  Info.plist); the back buffer follows the drawable
  (`mat_windowed_fullscreen`). The video options use Android's mobile policy
  (`MOBILE_VIDEO_OPTIONS`: no windowed mode, gamma or aspect filter).
  Info.plist also declares `UIApplicationSupportsIndirectInputEvents`, which
  SDL asked for in the first device log.
- **Keyboard dismissal:** `IInputSystem::StopTextInput` was added at the end
  of the interface; the only implementer is in-tree. It acts only where SDL
  reports a screen keyboard, the same condition under which `sdl3mgr` stops
  keeping text input on for the whole window. So desktop input is unchanged,
  and Wayland and X11 report one only without a physical keyboard or in Steam
  gamepad mode. The keyboard hides:
  - when a text field loses focus;
  - when a VGUI tap lands outside the focused field (`InputWin32.cpp`);
  - on Return on iOS (`SDL_HINT_RETURN_KEY_HIDES_IME`).
- **Evidence:** `./build-ios-app.sh` builds; the static-composition check
  passes (22 modules); the bundle holds the 18 icon files and the manifest.
  The Linux static tree (`build-static`) builds the shared input, VGUI and
  GameUI changes.
- **Unverified:** the touch layout, gyro aiming, rotation and keyboard
  behavior on the device. They need a user run.
- **Device config (2026-09-25, user direction):** the first launch had
  archived `touch_enable "0"` and `touch_gyro "0"` into the phone's
  `config.cfg` (the defaults then), and the synced content held an older
  generic HL2 `touch.cfg`. Archived values override the new defaults.
  - The phone's `config.cfg` now has both set to 1, and its `touch.cfg` is a
    `touch_loaddefaults` stub, so the game writes the Portal layout for this
    screen on the next launch. Written with `xcrun devicectl` from the Mac
    and read back.
  - The Mac's content copy no longer ships a `touch.cfg`; it was renamed
    `touch.cfg.hl2-layout-2026-09-21`.
  - Backups: `~/deploy/phone-cfg-backup-2026-09-25` on the Mac.

## LightmappedGeneric within MoltenVK's descriptor-set limit (2026-09-25)

- **Finding (first device log):** "LightmappedGeneric pipeline unavailable:
  needs the skin constants, nine descriptor sets and 13 attributes". MoltenVK
  reports `maxBoundDescriptorSets` 8. The stage gave each of its eight
  sampler registers its own one-texture set, plus the constants: nine. So
  every world surface fell back to the textured pipeline's flat lightmap.
- **Fix (user direction):** `lightmapped.frag` and `lightmappedpaint.frag`
  now read one grouped texture set (`CGroupedDescriptors::kLightmappedGroup`:
  s0, s1, s2 cube, s4, s5, s7, s8, s12 as bindings 0–7) and then the
  constants: two sets. The group reuses the PBR stages' per-frame-slot pools,
  reuse table and invalidation (`vulkan_descriptor_groups.h`). Each image
  resolves through `GroupedImage` to the same view, sRGB view and sampler as
  the per-texture sets did. The grouped descriptors are now created before
  the lightmapped pipeline. The stage also fits Vulkan's minimum of four
  sets, so the `.four-sets` suites no longer decline it.
- **Evidence (Linux, RADV):**
  - `material_pixel_conformance.py` `lightmap` and `bump` families on native
    Vulkan, integer HDR: pass, and `pixels.json` is byte-identical before and
    after (`quality-results/lmgroup-{A,B}-*`);
  - the `linux-native-vulkan-gpu` conformance profile: 15 of 15 suites match
    (`render.grouped-descriptors` included);
  - `portal_boot.py` on `testchmb_a_01` with `-vkvalidate`: pass, with no
    validation messages and LightmappedGeneric available;
  - `./build-ios-app.sh` builds.
- **Unverified:** the device run (MoltenVK argument buffers). `$phong` (seven
  sets) and the post passes are within eight and unchanged.

## First tvOS build (2026-09-25)

The user asked for a tvOS product profile, a background build, and content in
the caches folder. tvOS is extra product scope, not a north-star target.
[`portal-tvos-native-vulkan.json`](../quality/product_profiles/portal-tvos-native-vulkan.json)
uses the iOS static composition and points at the iOS pins through
`pin_source`.

- **Waf:** tvOS is part of the `ios` (UIKit) family. The SDK selects it:
  - `--ios-sdk` accepts an AppleTVOS SDK, which selects `arm64-apple-tvos`
    and sets `APPLE_PLATFORM` to `tvos`;
  - clang's `__ENVIRONMENT_TV_OS_VERSION_MIN_REQUIRED__` maps to DEST_OS
    `ios`;
  - `PLATFORM_TVOS` is defined next to `PLATFORM_IOS`.

  Agent decision: nearly every `PLATFORM_IOS` and DEST_OS `ios` use means
  "UIKit, not macOS", which is also true of tvOS. Only phone and tablet
  assumptions check `PLATFORM_TVOS`:
  - the Core Motion gyro (not in the tvOS SDK);
  - the touch controls' default (tvOS has no touch screen);
  - the content directory.
- **Content:** `$HOME/Library/Caches` (user decision). tvOS gives apps no
  persistent storage outside the bundle, and the system may purge the
  caches, so the content is recopied after a purge.
- **Script:** `build-ios-app.sh --profile` reads `target.os` and uses the
  profile's SDK, compiler runtime (`libclang_rt.tvos.a`), CMake system
  (`APPLE_TARGET_OS`) and Info.plist keys. `build-tvos-app.sh` wraps it.
- **tvOS-only fixes:**
  - `fork` and `execlp` are unavailable on tvOS, so vgui2's `ShellExecute`
    no longer compiles its process branch on the UIKit family.
  - libpng 1.6.38 uses its prebuilt `pnglibconf.h` only when CMake's `IOS`
    is set, so the tvOS build passes `IOS=ON` for libpng.
  - freetype rejects a variable named `IOS_PLATFORM`, hence the name
    `APPLE_TARGET_OS`.
- **Evidence:**
  - `./build-tvos-app.sh` builds an unsigned `build-tvos/Portal.app`.
    `hl2_launcher` is a tvOS Mach-O (platform 3, minos 17.0, sdk 26.5).
  - The static-composition check passes (24 module objects, 22 linked
    entries).
  - Only system frameworks and libraries are loaded, with no Core Motion.
  - On the macOS VM: `plutil -lint` passes, an ad-hoc signature verifies,
    and `vtool` reports `TVOS`.
  - `./build-ios-app.sh` still builds, and its static check still passes.
- **Not done at first build:** signing, installing, any Apple TV run,
  input and purge detection (see below).

### On an Apple TV 4K (2026-09-26)

- **Deploy:** `ios-deploy.sh --device tv --with-content` signs, installs and
  copies content to `Library/Caches` on tvOS. `ios-deploy.sh` is the Mac
  session's script and remains uncommitted.
- **Crash at 4K:** the first runs used a 3840×2160 drawable, and the copied
  desktop `config.cfg` turned on 4x MSAA. The app was killed (signal 9)
  about 5 s after start, with no crash or jetsam report.
- **Render defaults** (user direction), set by `ios_main.cpp` on tvOS:
  - `-nohighdpi`, a new `sdl3mgr` switch: the window has no high pixel
    density, so the drawable is 1920×1080 and tvOS scales it to 4K;
  - no MSAA;
  - the Video Advanced dialog's Low values.

  They are `+` settings, so they run after `config.cfg`. With them the app
  ran for 4+ minutes and quit normally.
- **Missing content:** at startup the app checks for the game's
  `gameinfo.txt`. If it is missing, an alert says how to recopy the content.
- **Controller:** SDL3 made the Siri Remote a gamepad, added first, so it
  held the engine's single gamepad slot. `SDL_HINT_TV_REMOTE_AS_JOYSTICK=0`
  frees the slot, and an Xbox Wireless Controller then drives the game (the
  user confirmed; 108 button presses logged). Remote-driven menus are out of
  scope (user direction).
- **Open:**
  - the engine's joystick connect lines don't reach the `devicectl` console;
  - no frame-time measurement against the 60 fps target.

## Not done

- Device evidence beyond the first boot: the AGENTS.md iOS obligations
  (lifecycle, surface recreation, memory pressure) and the touch, gyro and
  rotation behavior above.
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
