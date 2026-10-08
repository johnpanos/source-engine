# RFC 0001 rank 12 / R26: foundation contracts and native providers

Status: `done` (2026-10-08). Every required profile has native evidence (Linux
x86_64 and i386, Windows PE under Wine, and Android on a Galaxy Tab S8 Ultra),
and the hard prerequisite R11 is done (see below).
Source revision: `6eabeb23a` plus a dirty tree (the change described here).

## What is installed

### Contracts (`public/platform/contracts/`, standard library only)

| Contract | Header | Shared suite | Bad providers |
| --- | --- | --- | --- |
| `platform.clock.v1` (existing) | `clock.h` | `clock/clock_conformance.h` | 3 |
| `platform.wall-clock.v1` | `wall_clock.h` | `wall_clock/wall_clock_conformance.h` | 6 |
| `platform.thread.v1` | `thread.h` | `thread/thread_conformance.h` | 9 |
| `platform.virtual-memory.v1` | `virtual_memory.h` | `virtual_memory/virtual_memory_conformance.h` | 9 |
| `platform.process-environment.v1` | `process_environment.h` | `process_environment/process_environment_conformance.h` | 8 |
| `platform.paths.v1` (existing, amended) | `paths.h` | `paths/paths_conformance.h` | 8 (4 new) |
| `platform.diagnostics.v1` | `diagnostics.h` | `diagnostics/diagnostics_conformance.h` | 11 |

Suites are under `unittests/platformtest/`; contract docs are in
`unittests/platformtest/contracts/`.

Decisions recorded in the contracts:

- Wall time is signed 64-bit nanoseconds since the Unix epoch, so the
  representable range is 1677–2262. UTC breakdown always succeeds. Local
  breakdown may be refused, and a refusal leaves `out` unchanged.
- Threads sit beneath the task runners. Feature code posts work to runners and
  does not start threads. Names keep 15 bytes. Priority is `kOk` or
  `kUnsupported`, never a privilege failure.
- Virtual memory has no execute access, because store builds generate no code at
  runtime. Freshly committed pages read zero; decommitted pages read zero after a
  recommit.
- The process environment is a read-only snapshot. Names are case-sensitive on
  every provider, Windows included.
- Paths (amended): absolute, no `.` or `..` segments, valid UTF-8, and the
  executable directory is the parent of the executable file.
- Diagnostics (amended): a text sink frames each message with a severity tag and
  a line terminator. The crash reporter holds at most 32 annotations in fixed
  storage, so its signal handler never allocates.

### Native providers

- `platform/posix/foundation_providers.h` (library `platform_posix`, module
  `platform.posix`): clocks, pthreads, mmap virtual memory, process environment,
  Linux and root-supplied paths, a framed fd sink, an Android logcat sink, and a
  crash reporter. The crash reporter writes text reports on demand and from a
  fatal-signal handler on an alternate stack, restores the previous handlers,
  and re-raises the signal. Only one reporter owns the handlers at a time.
- `platform/win32/foundation_providers.h` (module `platform.win32`): the same
  set, over QueryPerformanceCounter, GetSystemTimePreciseAsFileTime,
  `_beginthreadex`, VirtualAlloc, CommandLineToArgvW and the environment block,
  GetModuleFileNameW and known folders, a framed handle sink,
  OutputDebugStringW, and an unhandled-exception filter.

### Consumer

`kiln.core`'s `DefaultSessionConfig` read `$HOME` with `getenv`. It now takes a
`const platform::IProcessEnvironment &`. The `kiln` composition root
(`kiln::ComposeDefault(argc, argv)`) creates the POSIX provider and passes it in;
sepipe and the `play_embedded` sample pass theirs.

## Evidence (2026-10-08)

| Profile | Row or check | Result |
| --- | --- | --- |
| linux-x86_64, g++ 16.2.1 | `platform.foundation.posix`, default and `--config release` | 2,720 checks, 0 failures |
| linux-x86_64, clang++ 22.1.8 | `platform.foundation.posix` | 2,720 / 0 |
| linux-x86_64, TSan | `platform.foundation.posix.tsan` | 2,720 / 0 |
| linux-x86_64, ASan+UBSan | `platform.foundation.posix.asan` | 2,720 / 0 |
| linux-i386 (`-m32`) | `platform.foundation.posix.i386` (baseline `foundation.posix-i386`) | 2,720 / 0, also with 64-bit `time_t` |
| windows-x86_64 PE, MinGW g++ 16.2.1, Wine 11.0 Staging | `platform.foundation.win32` (baseline `parity.wine.foundation`) | 2,640 / 0 |
| android arm64-v8a, x86_64 (NDK r30, API 29) | baseline `android.foundation-build` | both ABIs build |
| android arm64-v8a device: Galaxy Tab S8 Ultra (SM-X900, Android 16 / API 36, Qualcomm), wireless adb | `tools/quality/android_foundation.py check` | 2,720 / 0, and the logcat records at I, W and E |
| iOS arm64 (pinned toolchain, iPhoneOS 26.5 SDK) | compile of every `platform/posix` provider | compiles; optional profile, no run |
| shared suites on the fakes, g++ and clang++ | `platform.{clock,wall_clock,thread,virtual_memory,process_environment,diagnostics,paths}` and `.sensitivity` | every bad provider caught |
| consumer | `kiln.l0` (private `--tests` tree) and `kiln.sepipe` | 142 / 0 and 15 / 0 |

Besides the shared suites, the native rows check:

- real concurrency (a two-thread rendezvous) and a requested stack size being
  honoured;
- faults on read-only and reserved pages;
- the crash handler: the report contains the annotation, and the process still
  dies by `SIGSEGV`, or with exit code `0xC0000005` on Windows;
- abort when the provider is destroyed with an unjoined thread;
- handler restore and single ownership;
- refused address space (2^62 bytes, or all but 64 pages on 32-bit);
- a report directory that has disappeared (`kFailed`, id buffer unchanged);
- a real time zone (`TZ=Asia/Kolkata`);
- the environment snapshot not following a later `setenv`;
- record boundaries in the framed debug output, read through a
  `SOCK_SEQPACKET` socket on POSIX and a message-mode pipe on Windows.

Sanitizer runtimes are told to leave fatal signals to the handler under test; the
test sets these options itself when it re-executes.

Reproduction:

```sh
python3 tools/quality/conformance.py check --suite platform.foundation.posix
CONFORMANCE_TSAN=1 CONFORMANCE_ASAN=1 CONFORMANCE_M32=1 python3 tools/quality/conformance.py \
    check --cxx clang++ --suite platform.foundation.posix.tsan --suite platform.foundation.posix.asan
CONFORMANCE_M32=1 python3 tools/quality/conformance.py check --suite platform.foundation.posix.i386
WINEDEBUG=-all python3 tools/quality/parity_wine.py check --suite platform.foundation.win32
python3 tools/quality/android_foundation.py check     # needs an attached device; exit 3 = unavailable
python3 tools/quality/baseline.py audit --check parity.wine.foundation \
    --check android.foundation-build --check foundation.posix-i386
```

Tooling changes made for this:

- `conformance.py` gains the `windows-pe` runner class. The native runner skips
  that class, as it does `apple-device`.
- `parity_wine.py` gains `--suite` and honours a suite's `link_flags`.
- `tools/quality/android_foundation.py` is the Android lane.

## Unverified

- Android ran on the Galaxy Tab S8 Ultra. The Fold7 and the x86_64 ABI have
  not run the suite on a device.
- Apple (optional runner): compile only; no macOS or iOS run. No macOS SDK is
  present.
- MSVC (optional): the Win32 providers are built with MinGW only.
- Hosted CI has not run the new rows.
- The engine's tier0 functions (`Plat_FloatTime`, `ThreadSleep`, `CommandLine()`,
  spew, minidumps) used their own implementations when R26 closed. R103 moved
  them onto these providers with mod-facing exports kept (2026-10-08,
  [record](0001-tier0-facade-progress.md)).

## R11: paths and module resolution (2026-10-08)

R26's hard prerequisite. What R11 asks for, and where it now is:

- **Native and virtual paths distinct.** `public/platform/contracts/path_types.h`
  defines `VirtualPath`, a validated relative UTF-8 name, and `NativePath`, an
  opaque value: POSIX bytes or Windows UTF-16. Neither converts to the other
  implicitly. Joining a `VirtualPath` onto a `NativePath` is the only portable
  operation, and display text is a separate lossy conversion. Native storage is
  read only through `platform/native_path/native_path_access.h`. That is a
  backend module (`platform.native-path`), so archlint refuses any other
  include; the Tier 1 bridge is its one legacy user.
- **Resolution and verification separate from opening.**
  `public/platform/contracts/module_resolver.h` (`platform.module-resolver.v1`):
  - one portable resolver (`platform/resolver/`);
  - injected file probes (`CreatePosixFileProbe` and `CreateWin32FileProbe`);
  - an optional verifier, where signature and validation policy goes. A
    rejection ends the search.

  The resolver never opens a library. `platform.dynamic-library.v1` opens the
  resolved path.
- **The loader bridge converted.** `Sys_LoadModule`'s POSIX search
  (`foundLibraryWithPrefix` and the absolute-path `stat`) now runs through
  `tier1/module_search_bridge.cpp`, over the resolver and the POSIX probe. The
  bridge's control flow, telemetry and messages are unchanged. Tier 1 links the
  `_legacyabi` builds of the resolver and probe. RFC 0001 rank 5 also names "the
  extension resolver": no extension host exists yet, and R41 builds its hosts on
  this resolver.
- **Encoding, search and failure corpus.**
  - The shared resolver suite: 51 checks, with 8 bad resolvers caught. It
    covers root-major order, the exact attempted list, the legacy extension
    rule, file kinds, verifier policy, invalid names and policies refused before
    any probe, and UTF-16 roots with a non-BMP name.
  - The path-type tests: 31 checks, covering refusals including overlong
    UTF-8 and surrogates, joins, and lossy display of bytes that aren't UTF-8
    and of unpaired surrogates.
  - The file-probe suite on native fixtures. On POSIX it covers links, dangling
    links, a FIFO, and names that are and aren't UTF-8, and 3 bad probes are
    caught. On Win32 it covers both separators, a non-BMP name and the `NUL`
    device. An unpaired-surrogate name is created where the file system allows
    it; Wine refuses, and the test prints that.
  - `platform.module_search_bridge`: the bridge against the legacy search,
    frozen verbatim, on 400 seeded trees (1,483 checks, 0 differences). Seeded
    mutations of the bridge fail it: strict file kinds with 167 failures,
    swapped patterns with 180.

Recorded deviations of the bridge from the legacy search:

- A found path is the same file but no longer contains `//` when the root ends
  in `/`.
- A module name with a `..` segment is refused instead of searched. No caller in
  this repository passes one literally; names built at run time could.
- A null Android `APP_LIB_PATH` resolves nothing; it used to probe `(null)/...`.

Evidence:

- Linux x86_64 with g++, clang++, TSan and ASan/UBSan, and i386: the resolver,
  sensitivity and bridge rows pass, and `platform.foundation.posix` passes with
  the probe and resolver at 2,752 checks.
- Windows PE under Wine: `platform.foundation.win32` passes with 2,663 checks.
- Portal on native Vulkan, built through `./kiln build portal`, boots headless
  to `testchmb_a_00` with every module resolved through the bridge
  (`portal_boot.py`).
- The shared `build-p2` tree builds in full.
- Android arm64 on the Galaxy Tab S8 Ultra: 2,752 checks, 0 failures, with
  the logcat records. Android's SELinux policy refuses `mkfifo` to the shell
  user, so the FIFO case is created only where allowed (the test prints a note
  there), and `/dev/null` covers "another kind of file" on every POSIX
  platform.

Lesson from this slice: a Tier 1 build-graph change has to land in one step.
For about ten minutes the shared tree could not configure: the `tier1/wscript`
edit had a syntax error, and the new targets weren't registered in
`architecture/modules.json` and `quality/toolchain/policy.json`. That blocked
another session until it was fixed.
