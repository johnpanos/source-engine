# toolchain.abi.v1

The frozen external-consumer boundary of RFC 0006 M0 (roadmap R03). First-party
modules compile as C++20, but some of their facades are declared C++11 so that
prebuilt consumers (mods and plugins) built with an older toolchain keep
working. These suites prove that the facades behave identically across that
dialect boundary. The consumer side is compiled in the fixture-only
`legacy-cxx11` dialect of `quality/toolchain/policy.json`; the engine side is
the real module source compiled as `cxx20`. Both link into one executable.

Facades covered:

- `jobsystem` (`jobsystem/parallel_batch.h`, `jobsystem/worker_backend.h`):
  `toolchain.abi.jobsystem-batch`.
- `mapcontainer` (`mapcontainer/map_container.h`):
  `toolchain.abi.mapcontainer`.

Obligations:

- **Layout.** Every boundary value type has the same size, alignment and
  member offsets in the C++11 and C++20 units: `BatchDesc`,
  `MapContainerStatus`, `MapLumpInfo` and `MapContainerOpenOptions`. On LP64
  they also match the frozen table in `abi_checks.h`, because a change there is
  an unversioned ABI change whichever dialect made it. Standard types that
  cross a facade (`std::function<void( int )>` through
  `IWorkerBackend::ParallelFor`, and `std::string` as the libstdc++ dual-ABI
  witness) have the same size and alignment.
- **Dialect identity.** Each unit's records carry its `__cplusplus`: exactly
  `201103L` for the consumer and at least `202002L` for the engine. A fixture
  that silently compiled both sides in one dialect would prove nothing, so it
  fails.
- **Calls both ways.** C++20 code calls through a vtable the C++11 consumer
  implements (`IWorkerBackend`, `IMapByteSource`). The C++11 consumer calls
  C++20 entry points and vtables (`ExecuteParallelBatch`, `IMapContainer`).
  Values cross in both directions with their documented results. In serial
  and parallel mode, every batch item is processed with the expected result
  and the processed count equals the item count. Begin and end calls pair up,
  participants stay within `maxParticipants`, and legacy and BSP2 containers
  report the same lumps, bytes and content-hash failures.
- **Failure paths.** A batch without a process callback is rejected across the
  boundary. The fixture checks the rejection, not the absence of callbacks.
  A malformed container reports a structured status with a named error and
  no container.
- **One C++ library ABI.** Both sides are built with
  `-D_GLIBCXX_USE_CXX11_ABI=0`, the engine's libstdc++ ABI. Mixing ABIs across
  a facade that passes standard types is not supported.

Sensitivity suites (each must fail):

- `toolchain.abi.sensitivity-dialect-layout` adds a type whose layout depends
  on the dialect (`TOOLCHAIN_SEED_DIALECT_LAYOUT`). The layout comparison must
  reject it.
- `toolchain.abi.sensitivity-dual-abi` builds the consumer with
  `_GLIBCXX_USE_CXX11_ABI=1`. The `std::string` witness must differ, and the
  suite must fail.

Scope: Linux x86_64 with libstdc++ (gcc and clang), the `linux-headless-core`
profile. The Waf-built static archives are checked separately by
`tools/quality/toolchain_boundary.py check --artifacts`. MSVC, Apple and
Android consumers are not covered.
