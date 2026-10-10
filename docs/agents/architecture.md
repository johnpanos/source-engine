# Architecture, code and harness rules

Part of [AGENTS.md](../../AGENTS.md). Read before changing a boundary, a render module, C++ code or a harness.

## Architecture rules

- Features depend on narrow contracts, providers implement them, application
  roots compose them. No service locator, capability bag or tier alias.
- No platform or adapter code outside providers, and no content formats or
  their on-disk shapes outside format libraries and translators (user
  direction, 2026-10-08; CAP012 in `architecture/structure.json`). A real
  need is a `declared` consumer with its reason and row; convenience is not.
- One owner per authoritative state and policy; derived caches carry
  revisions and invalidate through the owner's commit.
- Required behavior fails composition by name; nothing silently falls back
  after partial mutation. Providers outlive borrowers; drain with
  acknowledgment before teardown; test partial startup and shutdown.
- Preserve advertised vtables/layouts, formats, callback timing, random
  order and gameplay unless a versioned decision says otherwise.
- Migrations are bounded, with named callers and a deletion condition. The
  old copy is deleted in the change that replaces it.
- DRY is one authoritative representation of knowledge, not merged syntax.
  LSP is behavioral: each replaceable contract has one shared suite run
  against every implementation, fakes included, plus deliberately bad
  providers.

### Render binding rules (user decisions; only the user can change them)

[RFC 0016's binding rules](../../RFC/0016-render-core.md#binding-rules-for-all-render-work-user-decision-2026-09-28)
are mandatory. In short:

- Legacy render paths are frozen against features; they take defect fixes,
  core plumbing, explicit user requests, and changes that serve the core
  (each commit carries a `Frozen-path:` line). Legacy backends only shrink
  (`tools/render/retirement_scans.py legacy-backends`: 3 backends, 23,698
  lines on 2026-10-10). shaderapivulkan, shaderapidx9, ToGL, ToGLES and DXVK
  Native are deleted; clients render only through the core.
- New render work lands on the core in its owning module and is proven in
  `render_lab` before integration. Legacy types never enter the core
  ([anti-corruption boundary](../../RFC/0016-render-core.md#the-anti-corruption-boundary-user-decision-2026-10-08),
  CAP011 rules 8–9); never add a `pending` entry or raise a ratchet to land.
- **The adapters are Vulkan, GL/GLES and null, and R89's scene path comes
  first** ([user direction, 2026-10-10](../../RFC/0016-render-core.md#adapter-freeze-and-scene-first-user-direction-2026-10-10);
  CAP011 rule 10). Direct3D 12, WebGPU, Metal and PICA are being deleted with
  the 3DS and WebAssembly products. No new adapter and no adapter features
  until R89 is done.
- Preserve the complete image and meet the
  [hard render budgets](../../RFC/0016-render-core.md#hard-render-budgets-user-decision-2026-10-01);
  a miss blocks performance acceptance, never cuts an effect. FSR's timing
  miss is advisory (2026-10-03 exception).
- If it is faster on the GPU, it runs on the GPU
  ([RFC 0003 placement policy](../../RFC/0003-dependency-aware-job-system.md#cpugpu-execution-placement-user-decision-2026-10-01)).
- Judge optimization on complete gameplay frames with the
  [resolution sweep](../../RFC/0016-render-core.md#optimization-resolution-sweep-user-decision-2026-10-03)
  (720p, 1080p, 1440p, 4K).

### Job-based execution

Explicit dependencies, scoped inputs/outputs, nonblocking compute and
ordered observable effects; not "everything parallel". The serial graph is a
required oracle and low-capacity mode. Process-wide budgets respect mobile
power and backgrounding.

## C++ and code style

- Every in-tree target is C++20 (`quality/toolchain/policy.json`):
  `cxx20-permissive` for unmigrated code, strict `cxx20` for new and migrated
  targets, `legacy-cxx11` only for frozen fixtures, Box3D `box3d-c17`. Never
  set `-std` per target; never change floating-point flags tree-wide.
- Prefer values, RAII, strong IDs, scoped enums, `[[nodiscard]]`,
  `std::optional`, project `Expected<T, E>`. `shared_ptr` only for real
  shared ownership. None of these cross preserved ABIs.
- Match surrounding Source conventions; `.clang-format` owns formatting of
  new/edited code (tabs, Allman, 100 columns); keep `memdbgon.h` last;
  preserve license headers; no unrelated reformatting.
- Ring buffers need a real bounded-transport use case. CPU publication, task
  completion and GPU completion are different contracts; recycle GPU
  resources only behind completion tokens.

## Harnesses and acceptance

RFC 0005 owns the eight harness families (Q-ARCH, Q-FOUNDATION, Q-EDITOR,
Q-JOBS, Q-PHYSICS, Q-CONTENT, Q-PRESENTATION, Q-PRODUCT). Required runs fail
on zero or missing tests, missing providers, crashes, timeouts and incomplete
results; skipped coverage certifies nothing. Test runners and comparators
with negative fixtures. Record revision, pins, toolchain, settings, seeds,
counts and reproduction commands. Never update goldens or baselines just to
get green. Use separate ASan/UBSan and TSan configurations.
