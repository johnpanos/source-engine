# Contract: `platform.module-resolver.v1` (and `platform.file-probe.v1`)

Module: `platform.contracts` · Types: `platform::IModuleResolver`,
`platform::IFileProbe`, `platform::IModuleVerifier`, `platform::ModuleSearchPolicy`,
`platform::VirtualPath`, `platform::NativePath`
Headers: `public/platform/contracts/module_resolver.h`, `public/platform/contracts/path_types.h`
Shared suites: `unittests/platformtest/module_resolver/module_resolver_conformance.h`,
`unittests/platformtest/file_probe/file_probe_conformance.h`
Implementations: `platform/resolver/` (the portable resolver), `platform/posix` and
`platform/win32` (file probes)
RFC: 0001 rank 5, "Path representation and encoding" · Roadmap: R11 · Domain: Q-FOUNDATION

## 1. Purpose, consumers, required vs optional

Resolving a module name to one native path is separate from opening it.
`platform.dynamic-library.v1` opens exactly the path it is given. The resolver
searches roots and patterns; an `IFileProbe` says whether a candidate exists; an
optional `IModuleVerifier` applies validation or signature policy to the chosen
candidate before anything opens it. The first consumer is Tier 1's
`Sys_LoadModule` POSIX search (`tier1/module_search_bridge.cpp`).

`VirtualPath` is a validated relative UTF-8 engine name. `NativePath` is opaque
and keeps the native representation (bytes on POSIX, UTF-16 on Windows); only
platform backends and the Tier 1 bridge read its storage, through
`platform/native_path/native_path_access.h` (a backend module, so archlint
refuses any other include). Display text is a separate, lossy conversion.

## 2. Accepted inputs

- Module names: any text; names that are not a valid `VirtualPath` are refused
  with `kInvalidName` before any probe.
- Policies: at least one non-empty root and one pattern; pattern directories
  are `VirtualPath`s; prefixes contain no separator. Anything else is
  `kInvalidPolicy`, before any probe.

## 3. Results and guarantees

- Candidates are `root / directory / prefix + file name`, root-major then
  pattern order. The file name is the module name with `extension` appended;
  with `replaceExtension`, an existing extension (the last `.` after the last
  `/`, unless it is the first character) is removed first.
- The first candidate that qualifies wins: a regular file by default, or
  anything that exists with `acceptAnyExisting`. Nothing after it is probed.
- With a verifier, only that first qualifying candidate is verified. A
  rejection ends the search with `kRejected`; a later copy is never
  substituted for a refused module.
- `kNotFound` and `kRejected` carry every probed candidate in order.
- A probe follows links, never creates or changes anything, reads only its own
  platform's `NativePath` flavor (other flavors are `kMissing`) and reports
  file kinds: missing, regular file, directory, other.

## 4. Ownership, threading, ordering

The resolver borrows its probe and verifier, which outlive it. Resolve is
`const`; a verifier with state is the caller's to synchronize.

## 5. Invariants and legal sequences

Resolution has no side effects besides probe and verifier calls. The resolver
never opens a library.

## 6. Side effects and performance

One probe per candidate up to the hit; no allocation beyond the result.

## 7. Conformance suites and providers

- `module_resolver_conformance.h` runs any resolver, built by a factory over the
  suite's fake probe and verifier. Row `platform.module_resolver` runs it
  against the portable resolver and tests the path types.
  `platform.module_resolver.sensitivity` catches eight broken resolvers:
  pattern-major order, an extension never replaced, directories accepted, a
  dropped attempted entry, invalid input reported as not found, the verifier
  skipped, a fall-through after a rejection, and probing after the hit.
- `file_probe_conformance.h` runs a probe over a native fixture built by the
  test. On POSIX (`platform.foundation.posix`, also TSan, ASan, i386 and the
  Android device lane) the fixture covers regular files, directories, links to
  each, dangling links, a FIFO, UTF-8 and non-UTF-8 names and a path below a
  file; three broken probes (`lstat`, every kind a file, names decoded as
  UTF-8) are caught on the same fixture. On Win32 (`platform.foundation.win32`)
  it covers files, both separators, directories, a path below a file, a
  non-BMP name and the `NUL` device. An unpaired-surrogate name runs where the
  file system can store one; Wine cannot, and the test says so.
- `platform.module_search_bridge` is the equivalence oracle for the first
  consumer: the bridge against the legacy search frozen verbatim from
  `tier1/interface.cpp`, on 400 seeded directory trees. Two seeded mutations
  of the bridge (strict file kinds, swapped patterns) fail it.

Recorded deviations of the bridge from the legacy search: a found path is the
same file but carries no `//` when the root ends in `/`; a module name with a
`..` segment is refused rather than searched (no first-party caller passes one).
