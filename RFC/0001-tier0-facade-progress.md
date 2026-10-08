# RFC 0001 R103: Tier 0 platform internals over the foundation providers

Status: `done` (2026-10-08) on every required profile: Linux x86_64 and i386,
Windows PE under Wine, and Android arm64. Rules and cohorts:
[RFC 0001, "Tier 0 facade over the foundation providers"](0001-capability-based-platform-architecture.md#tier-0-facade-over-the-foundation-providers-user-direction-2026-10-08).
Work was done on branch `r103-tier0-facade` (worktree `../source-engine-r103`)
and merged into `subsystem-refactor`.

Mod compatibility is kept (user decision, 2026-10-08). Every recorded export
keeps its name, kind and frozen declaration on every profile, and a mod binary
built once against the pre-R103 headers and never rebuilt loads into the new
Tier 0 and passes.

## What changed

`tier0/foundation_facade.{h,cpp}` owns Tier 0's private provider instances,
one per mechanism. They are created on first use and never destroyed, with no
setter and no registry. Tier 0's exports answer through them.

| Slice | Cohort | Commit | Ratchet | Highlights |
| --- | --- | --- | --- | --- |
| T0 | Fixtures and ratchet | `9c276c677` | n/a | Export fixture (`tier0_abi.py`), the kept mod binary, the OS-call ratchet (`tier0_ratchet.py`), `tier0_facade.py` |
| T1 | Time | `77a782233` | 32 → 0 | `Plat_FloatTime`/`MSTime`/`USTime` on the monotonic clock; POSIX now starts at 0 (it began anywhere in [0, 1) s); new export `Plat_MonotonicNanoseconds` |
| T2 | Threads | `61557f9db` | 91 → 0 | Backend-only `IPosixThreads`/`IWin32Threads` keep native handles and ids; new exports `Plat_ThreadSleep`, `Plat_ThreadSleepMicroseconds`, `Plat_ThreadYield` |
| T3 | Process environment | `e5edbd6f2` | 22 → 0 | One command line and one debugger check; Linux rebuilds the legacy `/proc` string exactly |
| T4 | Debug output | `c140b45be` | 6 → 0 | Logcat (tag SRCENG) and the Windows debugger channel through the diagnostics provider; the debugger bytes are unchanged |
| T5 | Crash reporting | `b5aba2adb` | 8 → 0 | `IStackCapture` and `IWatchdog` join `platform.diagnostics.v1`; the exception filter is owned by the provider; about 460 lines of dead malloc-hook code deleted |
| T6 | Memory and paths | `3d2f196f8` | 19 → 0 | The small-block heap, the poison and low-memory reservations on `Win32ProcessVirtualMemory()` (bookkeeping off the CRT heap, `ReserveAt`); module names through `Win32ModuleFileNameA/W` and `Win32ModuleOfAddress`; new Windows export `Plat_GetModuleFileNameOf` for `memoverride.cpp` |

Also on the branch:

- `6e6a76c97` keeps the provider archive out of Tier 0's exports
  (`--exclude-libs`). `tier0_abi.py` refuses any new std internal.
- `4ff16d66e` adds the i386 and Android gates. `GetCallStack` no longer makes a
  sibling call, so its frames start at `GetCallStack` as documented. On bionic
  the unwinder stops at `main`, and the tail call had left one frame.
- `0a3086a8e` adds the Windows PE gate.
- `be75f1ff1` puts the 18 `public/tier0` headers that declare recorded exports
  into `legacyAbi.paths` (CAP010, rule 1). It also adds native clauses for the
  Win32 process instance, `ReserveAt` and the module-name helpers.

The ratchet is zero in every cohort. `tools/quality/tier0_ratchet.py check`
fails if a count grows.

## Evidence (2026-10-08)

| Profile | Tier 0 build | Gate | Result |
| --- | --- | --- | --- |
| linux-x86_64 | `build-r103` (worktree) | `tier0_facade.py check` | 305 / 0: exports 259, mod fixture 19, behavior oracle 11, ratchet |
| linux-x86_64 | `./kiln build portal` | `portal_boot.py --headless --map testchmb_a_00` | pass |
| linux-i386 | private `--32bits` tree, host g++ 16.2.1 | `tier0_facade.py check --platform linux-i386` | 299 / 0: exports 253, mod fixture 19, oracle 11 |
| windows-x86_64 | `./kiln build dedicated-windows` (MSVC 19.44 under Wine) | `tier0_facade.py check --platform windows-x86_64` | 393 / 0: exports 344, mod fixture 19, Win32 oracle 20 |
| windows-x86_64 | same | dedicated server `+map testchmb_a_00 +quit` under Wine | loads the map, exits 0 |
| windows-x86_64 PE (MinGW) | n/a | `platform.foundation.win32` (`parity_wine.py`) | 2,756 / 0 |
| android-arm64-v8a | private `--dedicated` NDK r30 tree, API 29 | `android_tier0.py check`, Galaxy Tab S8 Ultra (SM-X900), wireless adb | 312 / 0: exports 282, mod fixture 19, oracle 11 |

Baselines: each platform's export fixture and kept mod binary were recorded
from the pre-R103 Tier 0 of `subsystem-refactor` at `1cfb77fb5`. Each was
built with that platform's configuration in a private output directory and
compiled against that revision's headers. linux-x86_64 was recorded at T0.

The pre-R103 Tier 0 run through the same gates, as controls:

- **i386:** the mod fixture passes, and `GetCallStack` returned no frames.
- **Android:** see "Observations" for its unwinder exports.
- **Windows:** the mod DLL passes 19/0. The oracle fails only its checks of the
  exports R103 adds.

Observations:

- The pre-R103 Android Tier 0 already exported 27 symbols of the NDK's
  unwinder. Stack capture links more of `libunwind.a`, which adds 10 more.
  Excluding the archive would drop the recorded ones and fail the fixture, so
  they stay; additions are allowed by the fixture rule.
- `Plat_GetModuleFilename` and `Plat_GetModuleFileNameOf` are Windows-only
  exports; Linux and Android export neither.

Reproduction:

```sh
python3 tools/quality/tier0_facade.py check --tier0 <tree>/tier0                       # linux-x86_64
python3 tools/quality/tier0_facade.py check --tier0 <i386 tree>/tier0 --platform linux-i386
./kiln build dedicated-windows
python3 tools/quality/tier0_facade.py check --tier0 out/dedicated-windows/dev/build/tier0 \
    --platform windows-x86_64 --wineprefix out/dedicated-windows/dev/wineprefix
python3 tools/quality/android_tier0.py check --tier0 <android tree>/tier0 [--device SERIAL]
python3 tools/quality/tier0_ratchet.py check
```

The i386 tree needs the 32-bit `libbz2.so` link name. On this Fedora host only
the runtime `libbz2.so.1` is installed, so a private directory with the
symlink goes on `LDFLAGS`. The `ci-tests-linux-i386` kiln profile pins Ubuntu's
GCC 13.3 and refuses the host compiler.

## Unverified

- Apple (optional runner): the POSIX facade paths for Apple compile in the
  existing iOS build only; no macOS or iOS run of the gate.
- Hosted CI has not run the new lanes. The Android and Windows lanes are local
  commands, not manifest rows.
- 32-bit Windows: no build exists, so its code paths are reviewed, not
  compiled.
