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

- Returned paths are normalized engine paths: UTF-8, `/`-separated, no `//`, and
  no trailing `/` except a lone root `/`. No native separators (`\`) and no
  embedded NUL.
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
- `test_paths_negative.cpp` (`sensitivity`) feeds the same predicate four broken
  providers (backslash separator, trailing slash, double slash,
  fabricated-path-when-unavailable) and asserts each is caught while the
  conforming backend passes.

### Native providers (added as they land)

A real POSIX (`/proc/self/exe`, `$XDG_DATA_HOME`, `$TMPDIR`) or Win32
(`GetModuleFileName`, known folders) provider adds one manifest row running the
same shared suite. Documented per-platform normalization/encoding is certified by
the native run, not by the fake.
