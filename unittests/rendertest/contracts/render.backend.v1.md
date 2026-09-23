# Contract: `render.backend.v1`

Module: `render.contracts` (public, backend-agnostic)
Contract header: `public/render/render_backend.h` (`render::IRenderBackendProvider` and family)
Conformance suite: `unittests/rendertest/conformance/render_backend_conformance.{h,cpp}`
Reference provider / oracle: `unittests/rendertest/fakes/null_render_backend.{h,cpp}`
Positive main: `unittests/rendertest/test_render_backend.cpp`
Sensitivity main: `unittests/rendertest/test_render_backend_negative.cpp`
Migration: `REND-BACKEND-001` · RFC 0001 (verified under RFC 0005 Q-PRESENTATION/Q-PRODUCT)

Pins the backend-agnostic obligations of RFC 0001's render-device capability
family so that a later Vulkan, D3D9, or GL provider is validated against the same
suite the null/recording provider passes today. The material system consumes
these render services; the contract is what lets it be tested against a null
provider "without installing process-global shader interfaces" (RFC 0001).

This is a **contract-and-fixture-first** deliverable: RFC 0001's render migration
sequence makes "add contract tests and a fixture" step 1, before any provider is
ported. A green run here means the suite is **ready** for the port, not that any
GPU backend is certified. A null/recording provider certifies only command and
lifetime behavior; **image fidelity and performance need real render-provider
evidence** and are deliberately out of scope for v1.

## 1. Purpose, consumers, required vs optional

Own the portable boundary between the engine (material system, resource owners)
and a GPU backend. Consumers request behavior — adapter enumeration, device
creation, submission — never backend identity and never a native
window handle. `IRenderBackendProvider` is a genuine polymorphic contract (null,
Vulkan, D3D9, GL are behaviorally different providers), so a virtual interface is
appropriate. Rendering is optional at the product level: a dedicated server omits
the capability entirely.

Providers advertise which optional behavior they support via `RenderProviderCaps`
("claims"). The suite runs an optional section only "where advertised," so a
provider is never failed for a capability it never claimed.

## 2. Accepted inputs

- `GetAdapterInfo(index, out)`: a valid index in `[0, GetAdapterCount())`.
- `CreateDevice(request, error)`: an adapter index, required/optional feature
  sets, and a validation flag.
- `Submit(context)`, `DestroyResourceWhenComplete(handle, token)`,
  `PollCompletion()`, `SimulateDeviceLoss()`, `RecoverDevice()`.

No field in a returned value (notably `RenderAdapterInfo`) may point into
transient provider memory; descriptions are pure values that outlive the
enumeration. `RenderAdapterInfo` also carries driver facts (`vendorId`,
`deviceId`, `driverVersion`, `driverApi`). They take part in value equality and
exist only so documented quirks can name the adapters they affect
([`render.profile.v1`](render.profile.v1.md)); portable code must not branch on them.

## 3. Results and guarantees

- **Adapter descriptions are stable values** — a re-query is equal; an
  out-of-range index returns `false` and does not write `out`.
- **Structured creation failure** — an invalid adapter yields
  `kInvalidAdapter`; an unsatisfiable required feature yields
  `kUnsupportedRequiredFeature` naming the missing feature. No device is returned
  on failure.
- **Immutable capabilities** — `GetCapabilities()` never changes after
  construction.
- **Child-resource lifetime** — resources are bound to the device; a resource
  referenced by outstanding work is recycled **only after its completion token
  reports complete** — never on a CPU-side event, a flipped fence, or a frame
  index (RFC 0006).
- **Ordered completion** — submissions complete in submission order;
  `LastCompletedSubmission()` advances monotonically.
- **No presentation on the device** — devices never interpret a window. A
  pair-specific bridge creates presentations from a device and a surface; see
  [`render.presentation.v1`](render.presentation.v1.md) (roadmap R16).
- **Ownership query** — `OwnsDevice` recognizes every live device the provider
  created, so a bridge can reject a foreign device structurally.
- **Device loss** — `SimulateDeviceLoss()` enters `kDeviceLost`; a
  recovery-capable provider returns to `kAvailable`, and one without recovery
  reports `kFatal`.
- **Complete destruction** — after every device is destroyed the provider owns
  zero live devices (teardown precedes any module unload).

## 4. Ownership, threading

- The provider owns devices; a device owns its resources, command contexts and
  completion tokens. Presentations belong to the bridge that created them.
  Child objects cannot outlive their device; the device cannot outlive the
  provider.
- Completion tokens are provider-owned and valid until the device is destroyed;
  callers observe `IsComplete()` but never delete them.
- v1 is single-threaded by construction; the threading contract for concurrent
  submission is deferred to the command-model work (RFC 0001 §"Command model and
  threading") and a later suite version.

## 5. Invariants and legal sequences

- Capabilities queried twice are equal.
- `DestroyResourceWhenComplete` then `CollectCompletedDestructions` keeps the
  resource live while the token is incomplete; after `PollCompletion` completes
  the token, the next collection recycles it and the live count returns to base.

## 6. Side effects and performance

- No globals, no logging, no GPU. The null provider is fully in-memory and
  deterministic (`PollCompletion` completes the oldest outstanding submission), so
  the suite is reproducible under the headless profile.

## 7. Conformance suite and providers

The suite is C++11-compatible, so legacy-dialect targets run the same code as
the headless C++20 profile and the C++17 Vulkan target.

- `unittests/shaderextensiontest/test_legacy_render_provider.cpp`
  (`legacy_render_provider_conformance`, Waf): runs this suite against
  `LegacyRenderBackendProvider` over the real `shaderapiempty` module (the
  product null backend), the native Vulkan module where it is linked, and five
  deliberately bad legacy backends that the suite must reject. Legacy providers
  claim no offscreen devices (presentation stays on the legacy `SetMode`
  path), so they certify identity, adapter enumeration, structured
  creation failure and zero live devices. The D3D9 module enumerates adapters
  only inside a composed material system; product boots cover it.

- `test_render_backend.cpp` (`render.backend.null`, positive): the null provider
  must pass every obligation above (24 checks today) and the suite must have run a
  meaningful number of checks.
- `test_render_backend_negative.cpp` (`render.backend.sensitivity`): seven
  deliberately broken providers, each violating exactly one obligation via a
  `NullBackendDefect`, must each be **detected** — both by an overall failure and
  by the specific named check failing. This proves the suite's teeth
  (RFC 0005: a suite that never fails certifies nothing).
- Both run headlessly (g++ and clang++, `-std=c++20 -Wall -Wextra -Werror`,
  `-I public` only) via the shared conformance runner on `linux-headless-core`.
  The `-I public`-only build also mechanically proves obligation
  "absence of native window and graphics API types from public portable headers":
  the contract compiles with no SDL/Vulkan/Win32/tier0 include roots available.

## 8. Not yet covered (future suite versions / real providers)

- Image correctness and cross-backend pixel tolerances (need a real provider and
  Q-PRESENTATION image evidence).
- Concurrent/multi-threaded submission and the recorded-command-buffer threading
  model.
- Shader-compiler (`IShaderCompiler`) artifact identity and caching.
- Native window/render interop — owned by `render.presentation.v1` and its
  pair-specific bridges.
- The material system's own restoration policy across device loss (owned by the
  material system, layered on this contract).
