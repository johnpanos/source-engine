# RFC 0001 Portal SDL3 / Vulkan compatibility slice

Updated: 2026-09-22

Portal now boots, loads an active player into `testchmb_a_00` and
`testchmb_a_01`, renders textured geometry and lighting, and shuts down cleanly
on the selected Linux x86_64 SDL3 / native Wayland / Vulkan profile. This is the
RFC's compatibility waypoint: DXVK Native translates the retained D3D9 material
implementation to Vulkan. It is not the future native Vulkan material renderer,
a default-provider change, or four-platform acceptance.

## Composition and migration boundary

The launcher composes mandatory systems through named linked factories. Its
renderer catalog selects the linked compatibility or null descriptor and binds
`render::LegacyShaderServices` before material-system connection. Material code
no longer opens a shader backend filename or discovers its service interfaces.
The old material vtable is preserved; its legacy filename entry point is a
bounded adapter to the linked descriptor. Native SDK types remain in the
provider. The narrow capability contracts in `public/render` remain the target
for subsequent resource, material and presentation migrations; the temporary
legacy service bundle is explicitly named and is not a new public capability bag.

The material binding suite exercises the actual material implementation and
actual full/null descriptors: missing required services, bad descriptors,
optional debug support, duplicate binding, foreign receivers, early connection
failure, cancellation, retry, legacy interface views and no window initialization.
The separately named shader-extension compatibility host owns mod-provided shader
library loading and lifetime. The built-in `stdshader_dx9` loader remains a
first-party migration debt; extraction of the mod host does not retire it.

## Outcome-driven diagnosis

The first native boots reached an active map but rendered black or white. The
boot gate was strengthened to reject missing, uniform and nearly black/white
captures; a successful process exit alone cannot certify rendering. The native
GPU suite independently renders uploaded textured geometry and checks every
pixel before and after resize. Its deliberately wrong expected pixel fails at
the first mismatch.

The opt-in provider trace records actual draw HRESULTs, viewport and target
sizes, texture binding masks, render state, requested shader names/static/dynamic
indices, and native bound shader identities. It distinguishes failed requests
with stale bindings, missing shaders, failed state queries, skipped meshes and
intentional depth-only draws. Its analyzer rejects missing or malformed records,
contradictory counts, zero detailed coverage and incomplete shutdown. Successful
draw submission does not claim visibility; screenshots are retained separately.
See [render diagnostics](../tools/quality/render_trace.md).

The trace isolated two concrete problems: POSIX incorrectly reported OpenGL for
the native Vulkan compatibility build, and supplied Portal shader archives used
different permutation layouts from the compiled Source selector headers. The
shader tool now compiles source-matched bytecode using a pinned existing compiler,
checks selector layouts, packs independently verified VCS files, and records
source/compiler/artifact hashes. Private runtime staging gives that pack priority
over incompatible VPK shader files. It never rewrites the supplied Portal runtime.
The tested workload is explicitly bounded; later scenes can require additional
static permutations. See [shader artifacts](../tools/quality/shader_artifacts.md).

## Reproduction and evidence

The [product profile](../quality/product_profiles/portal-linux-wayland.json)
owns dependency and compiler settings, selected providers, shader compiler profile
and required checks. [Profile instructions](../quality/product_profiles/README.md)
cover isolated Waf configuration/build/install, artifact generation and the
installed-product runner. The runtime at `/home/john/source-engine-portal-runtime`
supplies separately held Portal content; source provenance/distribution warnings
remain unchanged.

Observed native results on this machine:

- SDL3 lifecycle/input suite: 170 checks, zero failures on native Wayland.
- DXVK native textured presentation: 83 checks pass, including exact pixel
  readback, reset/resize and cleanup. The seeded pixel error fails as expected.
- Native math suite: 88 analytic checks pass.
- Shader-extension ownership/rollback suite: 36 checks pass.
- Shared RFC 0001 runner: 13 selected suites matched their declared outcomes,
  including negative providers and compile/crash fixtures.
- Opening-chamber run `/tmp/source-engine-portal-vulkan-07/evidence.json`:
  visible textured scene, active map/player and clean exit; its trace records
  22,721 submitted draws with zero shader-request, draw or presentation errors.
  The screenshot and `render-report.json` sit beside the evidence.
- Second-chamber run `/tmp/source-engine-portal-vulkan-08/evidence.json`:
  visible textured scene, but one shader-load failure before the capture window.
  That older harness result is **not** clean shader acceptance. The permutation
  was added to the workload and the new gate rejects this trace.
- `/tmp/source-engine-portal-vulkan-09/evidence.json` correctly fails startup:
  the user's interactive Portal process held the existing single-instance lock.
  It does not certify a 1920x1080 test run.

Generated evidence, captures and logs are local artifacts, not committed golden
images. Source/build identities and dependency hashes are recorded by each run.
A changed source or profile requires new evidence. Vulkan validation-layer
coverage and ASan were unavailable on this host; native rendering success does
not substitute for those lanes or for performance/power budgets.

## Earlier phases and remaining scope

Phase A's loader inventory, telemetry and frozen-header ABI work and Phase B's
explicit app-system instances are documented separately in their progress records.
This slice re-runs architecture enforcement and shared contracts; it does not
reinterpret fake-provider results as native loader evidence. The audit also found
mandatory filename composition still present in the dedicated-server roots and
legacy tools still selecting the null backend by filename; those cohorts are
being converted and require their own build/runtime evidence.

Global retirement Phase D is still open: remaining first-party shader, audio,
input and video selection must move into composition-owned typed catalogs, and
all declared products/profiles need their acceptance evidence. Phases E–G retain
their own tool/extension/public-loader retirement work. This record must not be
used to mark those broader portfolio rows complete.
