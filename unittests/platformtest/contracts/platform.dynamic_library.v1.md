# Contract: `platform.dynamic_library.v1`

Module: `platform.contracts` · Types: `platform::IDynamicLibraryLoader`,
`platform::IDynamicLibrary`, `platform::DynamicLibraryError`
Header: `public/platform/contracts/dynamic_library.h`
Shared suite: `unittests/platformtest/dynamic_library/dynamic_library_conformance.h`
Conformance: `unittests/platformtest/dynamic_library/test_dynamic_library.cpp` (+ `_negative`)
Test backend: `unittests/platformtest/dynamic_library/fake_dynamic_library.h`
RFC: 0001 ("Reference migration: dynamic-library loading") · Migration: `PLAT-DYNLIB-001`
Domain: Q-FOUNDATION

## 1. Purpose, consumers, required vs optional

Dynamic-library loading is the reference platform capability in RFC 0001.
Portable code requests it through this contract; a Win32, POSIX, console, or
deterministic test provider satisfies it. The contract is behavioral: it names
what a loader does, not which OS implements it. Consumers are composition roots
and extension hosts that must open an already-resolved binary, resolve a symbol,
and release the binary with correct lifetime.

This contract deliberately does **not** own path resolution, depot/local-copy,
signature, or allowlist policy — those belong to a separate module resolver
(RFC 0001, "Module resolver"). It opens one already-resolved path. It is also
distinct from the `CreateInterface` in-process interface registry: module
loading and interface registration are separate responsibilities and this
contract has no dependency on the registry, tier0, or tier1.

All clauses are **required** except the no-load capability (§3), which is
**optional** but must be reported explicitly.

## 2. Accepted inputs

- `Load(path, error)`: `path` is a non-null, non-empty already-resolved native
  path. A null/empty path yields `kInvalidArgument`, not undefined behavior.
- `FindSymbol(name, error)`: `name` is a non-null, non-empty symbol name. A
  null/empty name yields `kInvalidArgument`.
- `Unload(library)`: accepts any pointer, including null or a pointer this loader
  did not produce; both are no-ops.
- `TryResolveNoLoad(path, error)`: same `path` domain as `Load`.
  Unsupported providers report `kUnsupportedNoLoad` before inspecting the path;
  supported providers reject null and empty paths with `kInvalidArgument`.
- Error-output pointers are optional on every operation. Omitting an output
  cannot change success, failure, or ownership.

## 3. Results and guarantees

- `Load` returns a non-null `IDynamicLibrary*` owned by the loader on success,
  or nullptr with a structured `error` on failure (`kInvalidArgument`,
  `kNotFound` when the path cannot be opened, `kProviderError` otherwise). On
  success the error, when provided, is left `kOk`.
- `FindSymbol` returns the symbol address, or nullptr with `error` set to
  `kSymbolNotFound` (absent) or `kInvalidArgument` (bad name). Its `operation`
  is `kFindSymbol` and `requested` echoes the passed `name` pointer.
- `Unload` releases the library; afterward the pointer and every symbol obtained
  from it are invalid.
- Duplicate successful loads have distinct library objects and independent
  releases, even when a native provider internally obtains the same OS handle.
  Releasing one cannot invalidate the surviving library's symbols.
- `LiveLibraryCount()` returns the number of libraries the loader currently owns
  (loaded, not yet unloaded). It is 0 for a fresh loader, increments on each
  successful `Load`, and decrements on each `Unload` of a live library.
- `SupportsNoLoad()` reports whether resolve-without-load is available.
  `TryResolveNoLoad` returns true for a resolvable path when supported; when
  **not** supported it returns false and sets `error` to `kUnsupportedNoLoad`
  with operation `kResolveNoLoad` — an explicit report, never a silent success.

Errors are structured values (operation + status enum + native `providerCode` +
borrowed `requested` pointer). A caller identifies the failing path/symbol,
operation, and provider error without parsing log text.

## 4. Ownership, threading, ordering

- A loaded library has exactly one owner: the loader that produced it. Symbol
  addresses are borrowed values bounded by the library's lifetime and cannot
  outlive it.
- A loader MUST NOT be destroyed while it still owns live libraries. The test
  backend asserts `LiveLibraryCount() == 0` in its destructor; native backends
  enforce the same rule.
- The contract states no internal synchronization. Distinct loaders are
  independent. (A provider that is thread-safe documents that separately.)

## 5. Invariants and legal sequences

- `Load` of a valid path then `Unload` returns `LiveLibraryCount()` to its prior
  value (release completeness).
- A missing path never yields a live library and never increments the count.
- After `Unload`, a fresh `Load` of the same path succeeds — a clean second
  instance in the same process (RFC 0001 "clean second application instance").
- Optional no-load never mutates library ownership.
- Success overwrites a previous failure in the caller's error output. A failed
  load or symbol lookup leaves other live libraries usable; a later valid
  operation still succeeds.
- Foreign-library unload requests cannot consume the receiving loader's own
  references or change the producing loader's ownership. Distinct loader
  instances can be composed and shut down independently.

## 6. Side effects and performance

- The contract mandates no I/O beyond the native load/unload itself, no logging,
  and no global mutation. `LiveLibraryCount` is O(1) or O(n) over live handles.
  The test backend performs no I/O at all (in-memory fixtures).

## 7. Conformance suite and providers

- `dynamic_library_conformance.h` contains the shared operation-sequence and
  independent-loader predicates every provider runs: fresh-loader emptiness,
  valid and duplicate load ownership, known-symbol resolution (with exact
  address when known), missing/invalid-symbol structured failure, null and
  foreign unload, surviving duplicate symbols, missing/invalid-library failure,
  overwritten stale errors, optional error outputs, explicit no-load behavior
  before/during/after library ownership, repeat load/unload, and final emptiness.
- `test_dynamic_library.cpp` runs it against the deterministic test backend
  (`CFakeDynamicLibraryLoader`) in both no-load-unsupported and no-load-supported
  configurations and with two independent loaders. **Positive; certifies
  contract semantics, not OS behavior.**
- `test_dynamic_library_negative.cpp` (`sensitivity`) feeds the same predicate
  ten deliberately broken loaders: retained ownership on unload, successful
  missing load, silent unsupported no-load, successful missing symbol, stale
  success error, duplicate ownership aliasing, null unload consuming ownership,
  no-load acquiring ownership, wrong error operation, and foreign unload
  consuming ownership. Every defect must be caught. The same fault-injection
  implementation also runs with all defects disabled, proving rejection is not
  caused by an unrelated defect in the test double.

### Native providers (added as they land)

When a real POSIX (`dlopen`/`dlsym`/`dlclose`) or Win32 (`LoadLibrary`/
`GetProcAddress`/`FreeLibrary`) provider is implemented, it adds one manifest row
whose sources are `dynamic_library_conformance.h` + the provider + a fixture
shared library exporting `CreateInterface`. Fake-provider success is **not**
evidence of OS behavior (RFC 0001 "Backend conformance tests"); the native rows
supply that evidence. The real native loader today is `Sys_LoadModule` /
`Sys_GetFactory` in `tier1/interface.{h,cpp}`; migrating it behind this contract
is the RFC 0001 reference migration.
