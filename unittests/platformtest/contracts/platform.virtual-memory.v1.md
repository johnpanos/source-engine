# Contract: `platform.virtual-memory.v1`

Module: `platform.contracts` · Types: `platform::IVirtualMemory`,
`MemoryRegion`, `PageAccess`, `MemoryResult`
Header: `public/platform/contracts/virtual_memory.h`
Shared suite: `unittests/platformtest/virtual_memory/virtual_memory_conformance.h`
Conformance: `unittests/platformtest/virtual_memory/test_virtual_memory.cpp` (+ `_negative`)
Test backend: `unittests/platformtest/virtual_memory/fake_virtual_memory.h`
RFC: 0001 (foundation capability "Virtual memory") · Migration: `PLAT-VMEM-001`
Domain: Q-FOUNDATION

## 1. Purpose, consumers, required vs optional

Reserve address space up front and commit it as an allocator grows (memory
pools, streaming buffers, guard pages). All clauses are **required**. There is
deliberately **no execute access**: store-targeted builds generate no code at
runtime, and nothing in the engine needs it. GPU-visible and unified (UMA)
memory belong to `render.device.v2`, not here.

## 2. Accepted inputs

- `Reserve(size)`: `size > 0`.
- `Commit`/`Protect`/`Decommit(address, size)`: a non-empty, page-aligned range
  inside one live reservation; `Protect` additionally needs every page committed.
- `Release(region)`: exactly a region `Reserve` returned and not yet released.

## 3. Results and guarantees

- `PageSize()` is a power of two; `AllocationGranularity()` is a power-of-two
  multiple of it.
- A reservation's base is granularity-aligned and its size is the request
  rounded up to whole pages; reservations never overlap. Refusal leaves `out`
  unchanged.
- Freshly committed pages read zero. Recommitting committed pages keeps their
  contents. `Protect` keeps contents. `Decommit` discards them, so a later
  commit reads zero; decommitting reserved pages is allowed.
- Malformed ranges, double release and wrong-size release → `kInvalidArgument`.

## 4. Ownership, threading, ordering

The caller owns each reservation until `Release`. Calls on different
reservations may run concurrently; calls on the same reservation are serialized
by the caller.

## 5. Invariants and legal sequences

Reserve → (Commit | Protect | Decommit)* → Release. Page states: reserved ↔
committed(access).

## 6. Side effects and performance

Native providers map and unmap address space; no logging. Access outside a
committed read-write page is undefined behavior (natively, a fault); the fake
does not trap it.

## 7. Conformance suite and providers

- The shared predicate never faults natively: it only reads committed pages and
  only writes read-write pages. It covers sizes, alignment and rounding,
  malformed ranges, zero-on-commit, contents through protect and recommit,
  partial-range protect, decommit isolation, non-overlap and release rules.
- `test_virtual_memory.cpp`: 4 KiB/4 KiB (POSIX), 4 KiB/64 KiB (Windows) and
  16 KiB/16 KiB (Apple silicon, Android 16 KB). **Positive; contract semantics
  only.**
- `test_virtual_memory_negative.cpp` (`sensitivity`): a non-power-of-two page,
  stale recommit, an unzeroed commit, a base not aligned to the granularity, an
  unrounded size, accepted unaligned commits, protecting uncommitted pages,
  double release and release ignoring size are each caught.

### Native providers (installed 2026-10-08, R26)

- POSIX (`platform/posix/foundation_providers.h`, library `platform_posix`):
  `CreatePosixVirtualMemory` (mmap/mprotect; Decommit maps fresh pages over the range). Row `platform.foundation.posix` runs this suite and the native
  clauses on Linux with g++ and clang++, in release, under TSan and ASan/UBSan
  (`.tsan`, `.asan`) and as i386 (`.i386`). The same source cross-builds for
  Android arm64-v8a and x86_64 at API 29 (`tools/quality/android_foundation.py`)
  and compiles for iOS arm64.
- Win32 (`platform/win32/foundation_providers.h`): `CreateWin32VirtualMemory` (VirtualAlloc/VirtualProtect/VirtualFree, 64 KiB granularity). Row
  `platform.foundation.win32` runs as a static PE under Wine
  (`tools/quality/parity_wine.py check --suite platform.foundation.win32`).
- Evidence and what is still unverified: `RFC/0001-foundation-providers-progress.md`.
