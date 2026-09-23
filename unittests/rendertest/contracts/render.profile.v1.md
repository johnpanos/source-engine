# Contract: `render.profile.v1`

Module: `render.contracts` (public, backend-agnostic)
Contract header: `public/render/render_profile.h`
Legacy owner of the quirk table and selection: `materialsystem/legacy_render_backend_provider.{h,cpp}`
Positive/negative main: `unittests/rendertest/test_render_profile.cpp` (`render.profile`)
Sensitivity: `tools/quality/tests/test_render_profile_sensitivity.py`
Legacy integration: `unittests/shaderextensiontest/test_legacy_render_provider.cpp`,
`unittests/shaderextensiontest/test_material_binding.cpp`
Migration: `REND-PROFILE-001` · RFC 0001 "Capabilities, profiles, and quirks", rank 8

RFC 0001 separates three things the legacy hardware config mixes. Device
capabilities are facts a provider reports. The feature profile is application
policy: the features the engine chooses to use. Quirks are documented
workarounds selected from backend, driver, adapter and version data. This
contract represents the last two and how they are chosen from the first.

## 1. Purpose, consumers, required vs optional

- A composition root states a `RenderProfileRequest`: required features (their
  absence fails composition) and preferred features (used when available).
- `SelectRenderFeatureProfile` combines the request with one adapter's
  `RenderAdapterInfo` and a quirk table. It returns a `RenderFeatureProfile` or a
  structured `RenderProfileError`.
- Consumers ask the profile semantic questions (`enabled.Has(...)`,
  `workarounds.Has(...)`). They never ask which backend or driver is running.

The material system selects the profile in `Init`, after the adapter is chosen and
the device manager is initialized, from the root's request (set before
`Connect`), the bound legacy provider's adapter facts and `LegacyRenderQuirks()`.
Roots that state no request get `PreferAvailableRenderFeatures()`. The first
migrated consumer is the texture manager's normalization-cubemap format, which
used to test `IsOpenGL()`.

## 2. Accepted inputs

- `backendId`: non-empty provider id.
- Quirk entries must have a non-empty `id` and `reason`. They must name at least
  one of `backendId`, `driverApi` or `vendorId`, have ordered device and driver
  ranges, and have at least one known effect (removed features or workarounds).
  A null selector or zero vendor matches anything; ranges are inclusive.

## 3. Results and guarantees

- The whole quirk table is validated before any match. An invalid entry fails
  every selection with `kInvalidQuirk` and `error.quirk` pointing at it.
- A required feature the adapter lacks fails with `kMissingRequiredFeature`,
  naming the lowest missing feature.
- A matching quirk that removes a required feature fails with `kRemovedByQuirk`,
  naming the feature and the quirk.
- More than `kRenderMaxAppliedQuirks` matches fail with `kTooManyQuirks`.
- Malformed calls fail with `kInvalidProvider`.
- On success: `enabled = (required | preferred) & supported & ~removed`,
  `workarounds` is the union of the matching quirks' workarounds, and the applied
  quirks are recorded in table order. The error, if supplied, is reset to `kOk`.
- On failure the output profile is unchanged.
- Selection is a pure function of its inputs.

The legacy layer adds two statuses. `kInvalidProvider` means the backend's
adapter facts contradict its manager (for example the describe hook rejects an
enumerated adapter or claims the reserved feature). `kInvalidAdapter` means the
requested adapter does not exist.

## 4. Ownership

Quirk tables are static data owned by one policy module; profiles point into
them. The composition root owns the request. The material system owns the
selected profile from `Init` to `Shutdown` and injects it into its consumers;
there is no process-global profile.

## 5. Conformance

- `render.profile` (headless, 134 checks): positive selections, each selector
  and inclusive range boundary, table order, every failure status, unchanged
  output on failure, adapter value equality over the driver facts.
- Sensitivity: the Python test compiles the suite against eight seeded defects
  in the header (exclusive range, skipped validation, removed features kept,
  required not checked, quirk may remove required, driverApi ignored, output
  written on failure, capacity off by one). The suite must fail on each.
- Legacy integration: the real null and native Vulkan adapters select a profile
  with no quirks. A fake OpenGL-translating adapter selects
  `gl.float-normalization-cubemaps`, and d3d9/vulkan/none/empty APIs do not. A
  required feature on the null adapter fails structurally. The material
  system accepts a request only before `Connect` and reports no profile before
  `Init`.

## 6. Not yet covered

- Adapter limits (`RenderDeviceCaps` maximums) in selection. Legacy providers
  report adapter features only.
- DX-level policy. `m_nDXLevel` still drives shader and material compatibility
  through the legacy hardware config (RFC 0001 migration step 8).
- Remaining identity checks outside the texture manager (for example
  `ShouldAlwaysUseShaderModel2bShaders`), which migrate by cohort.
