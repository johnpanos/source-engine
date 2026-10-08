# Contract: `platform.paths.v1`

Module: `platform.contracts` · Types: `platform::IPlatformPaths`,
`platform::PlatformPathId`
Header: `public/platform/contracts/paths.h`
Shared suite: `unittests/platformtest/paths/paths_conformance.h`
Conformance: `unittests/platformtest/paths/test_paths.cpp` (+ `_negative`)
Test backend: `unittests/platformtest/paths/fake_paths.h`
RFC: 0001 (foundation capability "Platform paths") · Migration: `PLAT-PATHS-001`
Domain: Q-FOUNDATION

## 1. Purpose, consumers, required vs optional

Reports well-known base locations by role — executable file/dir, per-user data,
temp, and native-library dir — so portable code asks for a location instead of
branching on the OS. A native provider resolves the real location; the test
provider returns fixed normalized values. Asset search-path resolution is a
separate concern; this contract reports base locations only.

Each location is individually **optional** (a platform may not provide one) but
its availability MUST be reported explicitly (§3). The normalization/encoding and
buffer-handling clauses are **required** for every available location.

## 2. Accepted inputs

- `IsAvailable(id)` / `GetPath(id, buffer, bufferSize)` take any `PlatformPathId`.
- `GetPath` accepts any `buffer`/`bufferSize`, including null buffer or a size too
  small; those are rejected, not written past.

## 3. Results and guarantees

- Returned paths are normalized engine paths: well-formed UTF-8, `/`-separated,
  no `//`, and no trailing `/` except a lone root `/`. No native separators
  (`\`) and no embedded NUL.
- Paths are absolute (`/...`, or a drive root `C:/...` on Windows) and contain
  no `.` or `..` segment (amended 2026-10-08, R26).
- When both are available, `kExecutableDir` is the parent directory of
  `kExecutableFile` (amended 2026-10-08, R26).
- `GetPath` returns the length written (excluding NUL) on success, or **-1**
  without a partial write when the location is unavailable, `buffer` is null,
  `bufferSize <= 0`, or the path plus NUL does not fit.
- Results are **stable**: repeated `GetPath` calls for the same `id` return the
  identical string.
- Absence is explicit: an unavailable location reports `IsAvailable == false` and
  `GetPath == -1`, never a fabricated or empty path.

## 4. Ownership, threading, ordering

- `IPlatformPaths` owns no external resource and returns copies into caller
  buffers; nothing is borrowed across the call. Methods are `const`. The contract
  states no thread-safety guarantee beyond that of `const` reads.

## 5. Invariants and legal sequences

- `IsAvailable(id) == true` iff a subsequent `GetPath(id, ...)` with an adequate
  buffer returns a value `>= 0`; the two never disagree.
- A one-byte buffer never receives a non-empty path (returns -1, no overflow).

## 6. Side effects and performance

- No mutation of global state and no logging in the contract. The test backend
  performs no OS access. `GetPath` is O(path length).

## 7. Conformance suite and providers

- `paths_conformance.h` enforces, for each `PlatformPathId`: available → non-empty
  normalized path + stability; unavailable → `-1`; plus null-buffer, zero-size,
  and one-byte-buffer (no-overflow) handling. `IsNormalizedEnginePath` is the
  shared normalization predicate.
- `test_paths.cpp` runs it against the deterministic backend (`CFakePlatformPaths`,
  which reports `kNativeLibraryDir` unavailable to exercise the absence path).
  **Positive; certifies contract semantics, not native path resolution.**
- The predicate also checks absoluteness, dot segments, UTF-8 validity and the
  executable directory's parent relation (2026-10-08).
- `test_paths_negative.cpp` (`sensitivity`) feeds the same predicate eight broken
  providers (backslash separator, trailing slash, double slash,
  fabricated-path-when-unavailable, relative path, `..` segment, invalid UTF-8,
  executable directory not the file's parent) and asserts each is caught while
  the conforming backend passes.

### Native providers (installed 2026-10-08, R26)

- POSIX (`platform/posix/foundation_providers.h`, library `platform_posix`):
  `CreateLinuxPlatformPaths` (/proc/self/exe, XDG data, TMPDIR) and `CreateSuppliedPlatformPaths` for app containers. Row `platform.foundation.posix` runs this suite and the native
  clauses on Linux with g++ and clang++, in release, under TSan and ASan/UBSan
  (`.tsan`, `.asan`) and as i386 (`.i386`). The same source cross-builds for
  Android arm64-v8a and x86_64 at API 29 (`tools/quality/android_foundation.py`)
  and compiles for iOS arm64.
- Win32 (`platform/win32/foundation_providers.h`): `CreateWin32PlatformPaths` (GetModuleFileNameW, Local AppData, GetTempPathW). Row
  `platform.foundation.win32` runs as a static PE under Wine
  (`tools/quality/parity_wine.py check --suite platform.foundation.win32`).
- Evidence and what is still unverified: `RFC/0001-foundation-providers-progress.md`.
