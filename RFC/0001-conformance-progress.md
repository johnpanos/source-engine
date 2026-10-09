# RFC 0001 capability conformance progress (Q-FOUNDATION)

Updated: 2026-09-25

Bounded scope: shared conformance suites for the RFC 0001 capability contracts,
wired into the shared runner (`tools/quality/conformance.py`,
`quality/conformance.manifest.json`) so every capability provider is gated the
same way as the other RFC domains. This record tracks the conformance harness for
RFC 0001; contract *implementation* (native providers, loader retirement) is
tracked in `0001-phase-a-progress.md` / `0001-phase-b-progress.md`.

This is deliberately built in parallel with the providers: the suite defines the
behavior a provider must satisfy, so a native Win32/POSIX backend has a green
target to implement against and adds one manifest row when it lands.

## Delivered

| Capability | Contract | Suites (Q-FOUNDATION, rfc 0001) | State |
| --- | --- | --- | --- |
| Dynamic library (RFC 0001 reference migration) | `public/platform/contracts/dynamic_library.h` | `platform.dynamic_library` (+ `.sensitivity`) | test backend green |
| Monotonic clock | `public/platform/contracts/clock.h` | `platform.clock` (+ `.sensitivity`) | test backend green |
| Platform paths | `public/platform/contracts/paths.h` | `platform.paths` (+ `.sensitivity`) | test backend green |
| Composition kernel (rank 3 / R06) | `public/platform/composition.h`, `platform.composition.v1.md` | `platform.composition`, `platform.test-runner` | test providers green |
| Tool process (Phase E) | `public/platform/contracts/tool_process.h` | `platform.tool_process` (+ `.sensitivity`) | test backend green |
| Gyro aim policy (Android input) | `inputsystem/gyro_math.cpp` | `input.gyro` (+ `.sensitivity`) | pure policy green |
| Single-vibrator rumble policy (Android input) | `inputsystem/vibrator_policy.cpp` | `input.vibrator` (+ `.sensitivity`) | pure policy green |

**R06 clause map (reviewed 2026-09-25).** Each implementation clause of the
R06 row has evidence in `platform.composition` (`test_composition.cpp`) or
`platform.test-runner`:

| R06 clause | Evidence |
| --- | --- |
| Unit runner composes typed providers without ambient factories | `utils/unittest/capability_runner.cpp` builds an `ApplicationComposition` from typed `ProviderDescriptor::Define<…>` entries, with no `CreateInterface` or factory lookup (`platform.test-runner`) |
| Required/optional validation | `ValidateBeforeEffects`, and the optional-dependency cases: absent is explicit, present creates an edge, a mixed required+optional declaration is rejected |
| Failure-at-each-stage rollback | `LifecycleOracle` at all six stages (construct, connect and init of source and consumer), with ordered rollback |
| Repeat-instance tests | `RetryAndIsolation`: retry after a failed start, a second independent composition with separate authority, a teardown that leaves the first running, and idempotent stop |
| Negative providers | The same oracle rejects `Borrow`, `Task`, `Subscription` and `Bridge` faults |

**R06 closure (done, 2026-09-25).** R05 closed the same day, so R06 is
`done` on the clause map above plus a fresh run. It was closed as an agent
decision under the user's standing instruction. RFC 0001 rank 3's other
named items are also covered:

- *Deterministic start/stop ordering:* the ordered lifecycle oracle, and the
  stable registration-order tie break between independent roots.
- *Domain-scoped legacy bridges:* `legacy_binding.h`, bound per composition
  and cleared on stop; the `Bridge` fault is caught.
- *Rollback with subscription and task drain, and destruction after the last
  borrower:* the `Borrow`, `Task` and `Subscription` faults.
- *Startup with genuinely absent optional services:* the optional-absent
  case.

Fresh evidence (2026-09-25): `--domain Q-FOUNDATION` on g++ and clang++.
Each run matched 57 of 60 suites, including
`platform.composition` (252) and `platform.test-runner` (273), with 0
mismatched, 2,272 checks on each compiler. The 3 skips are optional `corpus.clang64.*` rows whose build
trees do not exist here.

The AGENTS.md note on R04/R06 asks for early "explicit factories" for iOS.
They are the kernel's typed `ProviderDescriptor::Define<…>` factories, with
no string or `CreateInterface` lookup. R04's CAP009 ratchet stops new shared
modules.

Closing R06 clears `roadmap.check`'s "R15 done while R06 partial" error.

Not claimed:

- Native providers, and the dedicated product's composition (R12).
- Hosted CI runs.

The last four rows were added after the first three (2026-09-22 to
2026-09-24). The composition and tool-process rows follow the contract pattern
below. The two input rows test pure policy code; they have no contract header
and no provider. Their record is the
[Android profile README](../quality/product_profiles/README.md).

Each capability ships: a pure-abstract contract header (no tier0, no native SDK),
a single shared conformance predicate, a deterministic test backend (positive
subject), a sensitivity suite that feeds the same predicate deliberately-broken
providers and asserts each is caught (proving the suite is not vacuous), a
contract doc, and two manifest rows. See `unittests/platformtest/README.md`.

RFC 0001 clauses currently exercised:
- dynamic library: valid load + single ownership, known-symbol resolution
  (exact address when known), missing-library and missing-symbol **structured**
  failures (operation + status + echoed request), null-argument rejection,
  explicit optional-capability (no-load) reporting, unload releases the handle,
  clean second load/unload in one process, and end-state emptiness (loader never
  destroyed with live handles).
- monotonic clock: positive resolution, never-backward samples, zero self-elapsed,
  monotonic-in-end elapsed, and additive conversion across an intermediate point.
- platform paths: per-location availability reported explicitly, normalized
  UTF-8 `/`-separated paths (no `\`, no `//`, no trailing `/`), stable results,
  and safe buffer handling (null/zero-size/one-byte -> -1, no overflow).

## Verification

```sh
python3 tools/quality/conformance.py check --domain Q-FOUNDATION --out -
python3 tools/archlint/archlint.py check --all
python3 tools/archlint/archlint.py baseline --verify
```

At this revision: 6 Q-FOUNDATION suites pass under g++ and clang++ on
`linux-headless-core`; the architecture baseline/inventory remain current (the new
`public/platform/` and `unittests/platformtest/` trees are archlint-strict from
first commit and carry no native includes).

Update (2026-09-25): the manifest now declares 21 Q-FOUNDATION suites. The
latest local full-domain evidence,
`quality-results/conformance.20260923T152310Z.json` (g++, dirty tree over
`c289e206`), matched all 17 suites then declared, with 1,303 checks. That
includes `platform.composition` (252 checks), `platform.test-runner` (273) and
`platform.tool_process` (49). The four `input.*` suites were added later. The
Android profile README records `input.gyro` at 68 checks and `input.vibrator`
at 43. The architecture check and loader inventory are no longer current
tree-wide; see the [Phase A record](0001-phase-a-progress.md). No finding is in
`public/platform/` or `unittests/platformtest/`.

## Not claimed here

- **No native evidence.** The test backends certify contract *semantics* only.
  Fake-provider success is not evidence of OS loader/timer behavior (RFC 0001,
  "Backend conformance tests"). Native Win32/POSIX providers must run the same
  shared suites (with a real fixture shared library for dynamic-library loading)
  before any platform support claim.
- The real native loader (`Sys_LoadModule`/`Sys_GetFactory` in
  `tier1/interface.{h,cpp}`) is not yet migrated behind `IDynamicLibraryLoader`;
  that migration is the RFC 0001 reference migration and is separate work.

## Next capabilities (same pattern)

Threading, virtual memory, process environment, and diagnostics — each is a
self-contained contract + shared suite + test backend + sensitivity suite.
Render-backend conformance is delivered separately in Q-PRESENTATION:
`render.backend.null` and `render.presentation.headless`, each with a
sensitivity row, plus `render.profile` (R15 and R16; see the
[render seam](0001-render-seam-progress.md) and
[presentation bridge](0001-presentation-bridge-progress.md) records).

Window and input contracts (`public/platform/window/`, `platform/window/`) have
a fake provider and the SDL3 provider (`platform/sdl3/window_system/`), both run
by the manifest. R14 is `done` (2026-10-08); see the
[R14 closure](#r14-closure-the-sdl3-windowinput-provider-done-2026-10-08).

### R14 closure: the SDL3 window/input provider (done 2026-10-08)

User goal "complete R14". The done condition (AGENTS.md, after the 2026-09-26
SDL3 retarget): the shared window/input suite passes against an SDL3 provider
and the fake backend; current product behavior captured and preserved;
normalized events, optional behavior, surface ownership and input lifecycle
conformance pass.

What changed:

- `platform_sdl3::Sdl3WindowSystem` (`platform/sdl3/window_system/`, module
  `platform.sdl3.window`) exports all six capabilities over SDL3. Native events
  are decoded there and normalized by the one `InputNormalizer`, the same owner
  the fake uses. One connected instance per process (SDL is process-global).
- Its surfaces are `platform/sdl3/render_surface` objects, so the SDL3
  presentation bridges (sdl3-vulkan, sdl3-d3d12) present to its windows through
  the private endpoint `RenderSurfaces()`. `Sdl3RenderSurfaces` gained the
  owner-reported visibility (`SetVisible`: zero extent while hidden or
  minimized, as the events said) and deferred reclamation (`Collect`: an
  invalidated surface is freed only after its presentation detaches), which the
  contract requires and the old immediate `Destroy` did not give.
- Product behavior kept from the SDL3 launcher (`appframework/sdl3mgr.cpp`),
  checked by `sdl3.product.*` clauses in the native suite: a flipped wheel
  reads unflipped; positions truncate like the launcher's while fractional
  relative motion carries instead of being lost; text input starts per window
  except where it raises an on-screen keyboard; gamepad input is withheld
  without focus (`gamepadsWithoutFocus`, default off, is the root's explicit
  opt-out).
- The contract document
  [`platform.window.v1`](../unittests/platformtest/contracts/platform.window.v1.md),
  cited by the headers but never written, records every obligation by its
  check name.
- Shared suite fix: `gamepad.axes` drove a trigger already at rest to "-100
  clamps to 0", which is no transition on a real device (SDL drops unchanged
  axis values). It now checks the high clamp on the left trigger and the low
  clamp on the right trigger, each a transition; the fake and sensitivity rows
  still pass.
- Deleted: the SDL2 window provider (`platform/sdl2/window_system/`) and its
  test (`unittests/platformtest/window_sdl2/`), which no build or manifest row
  used; SDL2 gets no further work (2026-09-26 decision). Module entries
  `platform.sdl2.window*` are replaced by `platform.sdl3.window*`.

Evidence (rows in `quality/conformance.manifest.json`, at `94d243c5` plus this
change, SDL 3.4.18, the profile's pinned version):

| Row | Driver | Result |
| --- | --- | --- |
| `platform.window` | fake, composition | 421 checks, 0 failures |
| `platform.window.sensitivity` | broken fakes | 23 checks, every seed caught |
| `platform.window.sdl3` | SDL3 offscreen, g++ and clang++ | 212 checks, 0 failures |
| `platform.window.sdl3.native-drivers` | SDL3 Wayland in kiln `private`, X11 in `private-x11` | 424 checks (212 each), 0 failures |

Each SDL3 run composes the provider through the R06 kernel twice (repeat
instance), checks the one-connected-instance rule and reconnects after
teardown. One recorded skip per run: `message.shown`, a modal box needs a user
to dismiss it. Two SDL device-path facts are handled in the test driver, not
the provider: SDL holds a released Guide button for 250 ms, and a virtual
trigger rests at half travel.

Reproduce:

```sh
python3 tools/quality/conformance.py check --suite platform.window \
    --suite platform.window.sensitivity --suite platform.window.sdl3 \
    --suite platform.window.sdl3.native-drivers --out /tmp/claude-1000/r14/ev.json
python3 tools/quality/conformance.py check --cxx clang++ --suite platform.window.sdl3 \
    --out /tmp/claude-1000/r14/ev-clang.json
```

Not done here (other rows): no product consumer composes the provider yet. The
launcher still drives SDL3 through `appframework/sdl3mgr.cpp` and the
`platform/sdl3/legacy_include` adapter; migrating those callers is R18. No
Android, iOS, macOS or hosted CI run of this provider.

### R18: SDL3 private to its owners (in progress, 2026-10-08)

User goal "Finish R18". Done condition (AGENTS.md): the SDL3 window/input suites
and representative behavior pass on every declared SDL3 profile; the SDK
dependency is private and SDL is off generic include paths (the
`platform/sdl3/legacy_include` adapter's callers migrated); supported interop
pairs tested; the SDL2 legacy profiles still build.

What changed:

- The adapter is deleted. Its callers fall into two groups:
  - SDL backends now include `<SDL3/SDL.h>` and use SDL3 names: the launcher
    manager (`appframework/sdl3mgr.cpp`, `sdl_window_provider.cpp`), the input
    system (`inputsystem/*_sdl.cpp`, `inputsystem.cpp`, `vibrator_device.cpp`)
    and the engine's audio backends (`engine/audio/*_sdl.cpp`). The rename used
    SDL3's own `SDL_oldnames.h` table, the same one `SDL_ENABLE_OLD_NAMES` used,
    and the SDL2-only branches were resolved with `unifdef -DUSE_SDL3`. All nine
    files compile to the same instructions and relocations as before (objdump
    comparison against HEAD built with the adapter; only `__FILE__` strings
    differ).
  - Everything else no longer touches SDL. Engine (`host`, `sys_dll`,
    `sys_getmodes`, `sys_mainwind`, `matsys_interface`), gameui, vgui2,
    vguimatsurface and video ask the launcher through a new
    `ILauncherPlatformServices` (`public/appframework/ilauncherplatformservices.h`,
    `LauncherPlatformServices001`, from `ILauncherMgr::QueryInterface`): displays
    by index, clipboard, URLs, show/redraw, pointer, window size and system
    cursors. tier0's assert dialog and the engine's message boxes go through one
    new tier0 hook, `SetPlatformMessageBoxFunc`/`ShowPlatformMessageBox`, which the
    launcher installs; tier0 no longer links SDL.
- Build: `SDL3` is a uselib of exactly the launcher manager (appframework), the
  input system, the engine and the launcher. The `INCLUDES_SDL2` path is gone from
  every target, and SDL3 configurations no longer alias `SDL2` to SDL3. The SDL2
  launcher manager (`appframework/sdlmgr.cpp`) and the non-SDL3 Android launcher
  (`launcher/android/`) were unreachable (every SDL product is an SDL3 client) and
  are deleted.
- Enforcement: `tools/quality/sdl_boundary.py check` (baseline `sdl.boundary`)
  allows SDL headers and SDL calls only in the files and prefixes listed in
  `architecture/sdl_boundary.json`. The list is exact and shrink-only. Six
  self-tests (`tools/quality/tests/test_sdl_boundary.py`) cover a new include, a
  new call, a stale entry, comments, strings and opaque types, and third-party
  exclusion.
- Product suites repaired, since they were failing before this change:
  - `sdl3_launcher_conformance` checked for a Vulkan window flag the core render
    backend no longer sets. SDL3 makes a Vulkan surface without it, as a probe
    showed. The test also crashed in a ConVar write without a cvar system,
    accepted only Wayland, and saw stray X11 focus events.
  - `input_gamepad_slots_test` lacked the client's `joystick` ConVar, so rumble
    was always off. It also showed two product bugs, both fixed:
    `CInputSystem::StopRumble` never stopped player 2's pad, and a reopened pad
    reported nothing until its next transition (`SampleGamepadState`).
- The lane (`tools/quality/window_sdl3_lane.py --tree`) runs these on real
  drivers (manifest `platform.sdl3.product-suites`).
  `tools/quality/android_window.py` cross-builds the window suite for Android
  and runs it on an attached device (baseline `android.window-build`).

Evidence on Linux x86_64, SDL 3.4.18 (manifest rows):

| Row | Result |
| --- | --- |
| `platform.window` / `.sensitivity` | 421 / 23 checks, pass |
| `platform.window.sdl3` (offscreen, g++ and clang++) | 212, 0 failures |
| `platform.window.sdl3.native-drivers` (Wayland and X11, private sessions) | 424, 0 failures |
| `platform.sdl3.product-suites` (launcher 365, gamepad slots 102, voice capture 33, per driver; the seeded slot-collapse provider is caught on both) | 1,002, 0 failures |
| X11 launcher run repeated 5 times | 5 of 5 pass |
| `portal_boot --resize-stress`, headless | pass |

Builds: portal (Linux), the WebAssembly profile and the Android arm64 APK
(`android_apk` verifier passes) build. The 3DS build compiles every SDL-related
target; its only failure, `platform_foundation_legacyabi` (POSIX providers using
`sys/syscall.h`, `tm_gmtoff`, `siginfo_t`), is outside SDL. The iOS, macOS and
tvOS apps stop before any SDL code at `render_device_metal` (TOOLCHAIN003,
RFC 0025); Apple runners are optional.

Known failures that are not this change's:

- `portal_boot --resize-stress` on real Wayland fails "missing or blank resize
  image". HEAD without this change fails it 3 of 3 runs (up to 10 sizes). The
  core shader API's present after a resize; shared with source-engine r91.
- `stylelint --changed` flags `engine/host.cpp:132`. clang-format's suggestion
  re-indents the file-scope declarations after the removed include, because it
  misreads the file's earlier structure. Not applied.

Open for done:

- An Android arm64 hardware run (`android_window.py check`). No device was
  attached; the Android profile is unverified.

Since then (2026-10-08):

- The SDL3–Vulkan pair's native suite, deleted with `shaderapivulkan`
  (154155845), is restored as `unittests/rendertest/test_sdl3_vulkan_presentation.cpp`.
  It is built beside the core presenter (`render/bridge/sdl3-vulkan/wscript`)
  and run by the product lane: render.presentation.v1's shared suite passes on
  Wayland (49 checks, with two recorded skips: Wayland cannot restore a window)
  and on X11, and 5 pixel checks pass on each. The SDL3–D3D12 pair keeps its
  own lane (`render.presentation.sdl3-d3d12`).
- The launcher window is shown at the mode's size (de3e3dca8). On the core
  backend the window was created visible at 1280x720, and the engine's mode
  reached it as an asynchronous resize. The UI then laid out at the old size:
  r91 saw the menu background drawn at 1280x683 of a 1920x1080 frame. The
  window is now hidden until the mode switch, and its first show syncs and
  reapplies the size. `sdl3_launcher_conformance` checks the drawable right
  after the first show; seeding the visible window back fails it on X11.
- `platform.sdl3.product-suites`: 1,114 checks, 0 failures, on Wayland and X11.
