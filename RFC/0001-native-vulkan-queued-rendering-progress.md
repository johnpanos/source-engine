# Native Vulkan queued rendering (mat_queue_mode 2) — progress

Scope: let the native Vulkan shader API (`materialsystem/shaderapivulkan`)
run under the queued, multithreaded material system: the main thread
builds frame N+1 while the material system's render thread (`MatQueue`)
replays frame N into the device. Roadmap context: R32 (native Vulkan MVP)
and R20 (bounded parallel execution), tracked as **R32-QUEUED**. This record
closes no roadmap gate.

Before this slice the provider declared `supportsQueuedRendering = false`,
so `CMaterialSystem::EndFrame` forced native Vulkan single-threaded whatever
`mat_queue_mode` asked for (the SDL3 default is 2, which DXVK uses).

## Design

Three things stood between the backend and the queued material system.

### 1. The queued-mesh contract (`vulkan_mesh_layout.{h,cpp}`)

The queued material system builds a dynamic mesh on the main thread into
memory that `IShaderAPI::ComputeVertexDescription` describes. On the render
thread it locks the backend's dynamic mesh and copies those bytes verbatim
from the lowest component pointer (`CMatQueuedMesh::ExecuteDefferredBuild`).
The native `ComputeVertexDescription` was unimplemented, and the native mesh
used a fixed record whose defaults (white color, weight 1 on bone 0) were
seeded at lock, so a verbatim copy would overwrite them with garbage.

- `vulkan_mesh_layout` is the one owner of the vertex record: offsets, the
  stride (104 bytes; 232 for wide texture-coordinate formats such as
  SpriteCard, whose eight float4 sets now live inline instead of in a
  separate array; 4 for a static-prop color stream), and
  `DescribeMeshRecords`, which both `CEmptyMesh::Lock` and
  `ComputeVertexDescription` use.
- A format's absent components point at scratch with size 0, as D3D9's do,
  and receive their identity at unlock (`FillAbsentMeshComponents`), after
  the builder or the queued copy wrote the rest. A mesh with no format
  (format 0) keeps the old seed-at-lock behavior.
- The dynamic mesh takes the format the queued material system computes
  (`CShaderAPIVulkan::DynamicMeshFormat`: the material's format with the
  current skinning's bone weights, as `CMatQueuedMesh::OnGetDynamicMesh`
  and D3D9's mesh manager do). The flex stream has its own mesh, so a flex
  build cannot overwrite the dynamic mesh.
- Single-threaded behavior change: writes to components a format lacks are
  now discarded (D3D9 semantics) instead of stored.

### 2. Device ownership

The material system hands the device between threads with
`AcquireThreadOwnership`/`ReleaseThreadOwnership` (the render thread owns it
while queued; `CMaterialSystem::Lock` moves it to the main thread after
waiting for the render thread). Backend state is used by the owner alone,
as ToGL requires of its GL context; no per-call lock was added.

- The backend records the owner and counts, by site, entries reached from
  another thread while a thread owns the device (`NoteDeviceUse`: material
  bind, passes, draws, constants, textures, targets, clears, present,
  read-back, world batches, light set). The screenshot census prints
  `device owner changes=N cross-thread calls=M` and each offending site.
- Shared state the main thread legitimately touches is synchronized: the
  live-mesh registry (static meshes are created on the main thread) has a
  mutex; the mesh revision counter is atomic.
- The GPU compute service (RFC 0011 G6) is called by engine producers on
  the main thread while frames are recorded on the render thread.
  `ComputeResources` and `GpuComputeService`'s dispatch queue are now
  thread-safe; a dispatch's serial is never a submission already recorded;
  the serials producers read are published by the per-submission flush
  (the owner still reads the device's live completed serial, so
  single-threaded latency is unchanged). `gpu_compute.h` documents it.

### 3. Frame-ordered capabilities (`materialsystem/render_capability_queue.{h,cpp}`)

The engine calls the provider's world-mesh (`IWorldMeshUpload`) and light-set
(`ILightSetConsumer`) capabilities on the main thread, in frame order among
render-context calls that are queued. The material system now hands out
adapters that, when the calling context has a render call queue, queue each
call with a copy of every byte it borrows; otherwise they call the provider
directly. A queued upload returns true (the provider reports rejections
itself); a queued `DrawBatch` returns the provider's last verdict for the
bound material, so the engine's per-batch bookkeeping learns a rejection
one frame late. Map load and screenshots already force single-threaded mode,
so load-time uploads keep their synchronous results.

### Defects found by running it

- `GetCurrentDynamicVBSize()` returned 0. The queued context sizes a dynamic
  build as that over the record stride, so queued particle renderers got a
  0-vertex budget and queued empty meshes until the frame's 8 MB call-queue
  allocator returned null (segfault in `CMatCallQueue::QueueFunctorInternal`).
  It now reports `kDynamicMeshVertices` (32768, the direct path's limit)
  base records.
- `CMaterialSystem::UpdateLightmap` ignores a main-thread update while a
  render call queue exists. The RFC 0011 probe relight of brush entities
  (`R_RelightBrushEntitiesFromProbes`, `engine/gl_lightmap.cpp`) called
  `R_BuildLightMapGuts` directly, so under the queue the `gi_door` door kept
  its lit lightmap after closing. It also shared `g_ProbeLitSurfaces` and the
  `blocklights` scratch with the render thread's queued rebuilds. It now
  queues each surface's store-and-rebuild, as `R_BuildLightMap` does.
- Portal 2 (reported by a peer session) aborted with "Getting a dynamic mesh
  without resolving the previous one" after the first portal:
  `CPortalRender::DrawPortalsUsingStencils` draws index ranges of its dynamic
  portal-quad mesh, then recurses into a portal view whose own rebuild asks
  for the dynamic mesh while the parent's is unresolved. Single-threaded
  hardware meshes discard the pending build silently; the queued mesh errors.
  `ResolvePortalQuadMesh` marks it drawn and nulls it after each block of
  range draws, and where the code already gave it up; the existing null
  checks rebuild the same geometry from its replay data.
- The native mesh skipped D3D9's queue hooks (`IShaderUtil::OnDrawMesh`,
  `OnSetColorMesh`, `OnSetPrimitiveType`). The engine draws static meshes on
  the main thread itself (`CShadowMgr::RenderFlashlights` in Portal 2's
  `sp_a1_intro7`), so under the queue the material pass ran on the main
  thread and crashed. `CEmptyMesh` now calls the hooks as `CMeshDX8` does:
  the queued context re-queues the call and the render thread's hardware
  context lets it through.

### Defaults

The SDL3 default for `mat_queue_mode` is 2, so any native-Vulkan launch that
does not pin it now runs queued. `run.conf`/`run.sh`, `portal_boot.py` and
`frame_pacing.py` (new `--mat-queue-mode`, default 0) keep pinning 0, so
baselines are unchanged. The Android main pins `+mat_queue_mode 0`
(overridable in `commandline.txt`): the render thread is unmeasured there.
Making queued the desktop launcher default is left to the user.

## Evidence

Host: AMD Radeon 8060S (RADV STRIX_HALO), Fedora 44, shared with other
agents (load average 90–115 on 32 threads during every timing run). Private
tree `build-mtq` (`WAFLOCK=.lock-waf-mtq`, native Vulkan, Portal). Artifacts
are under `quality-results/mtq/`. All runs headless (`SDL_VIDEODRIVER=offscreen`).

| Check | Result |
| --- | --- |
| `queued_mesh_contract_vulkan_conformance` (new, CPU only) | 173 checks, 0 failures: 7 formats (lightmapped world, skinned, one-bone, UI, position-only, SpriteCard wide, flex) as static and dynamic meshes, the color stream and the flex mesh; queued replay equals the direct build byte for byte; absent components carry identities although the replayed bytes were garbage and the builder wrote stray values; the D3D9-packed negative control is detected |
| `material_pixel_conformance`, all 14 families, hdr none | pass on baseline and new; all 101 cases' `pixels.json` byte-identical. Baseline = HEAD `shaderapivulkan.cpp` (plus another session's unrelated hunk) linked with this tree's other objects |
| `portal_boot` testchmb_a_00, mode 2 / mode 0 | pass / pass. Mode 2: owner changes 4, cross-thread calls 0, 338 frames presented from the render thread; mode 0: 0, 0, 0 |
| `portal_boot` testchmb_a_09, testchmb_a_11, escape_00, mode 2 | pass; 0 cross-thread calls; 338–341 render-thread frames each |
| `gi_door` (WMSH, PRBV, SDF shadow field, SDF producer, door closed mid-run), mode 2 vs mode 0 | screenshots byte-identical after the relight fix (both fail portal_boot's scene-detail gate because the closed room is dark by design); mode 2: 0 cross-thread calls, 479 render-thread frames |
| stock-map screenshots across modes | differ, but no more than repeats of the same mode (mode 0 vs mode 0: 9.5% of pixels > 2, max 71): animation, not the mode |
| `frame_pacing.py` portal scenario, 3 interleaved rounds per mode | warm-pass interval median mode 2: 15.3–16.7 ms, mode 0: 15.8–30.0 ms; p99 mode 2: 28–34 ms, mode 0: 34–76 ms. 0 cross-thread calls, 1933 render-thread frames per mode-2 run. Every run fails the hitch budget on this loaded host |
| same, mode 2 with `+host_thread_mode 1` and the pooled job graphs | medians 16.2–16.9 ms, p99 34.6–36.3 ms; 0 cross-thread calls, 1934 render-thread frames |
| Portal 2 scenarios (`portal2_scenarios.py`, private `build-mtq-p2`), mode 2 | `sp_a2_triple_laser_traverse` and all 4 default scenarios (intro5, intro7, wakeup, a2_core) pass; 0 cross-thread calls. Before the two Portal 2 fixes: triple-laser aborted, intro7 segfaulted, wakeup failed; all pass in mode 0 |
| `test_frame_pacing.py` | 21 tests OK |
| stylelint (new files whole, edited files changed lines) | clean |
| archlint `check --all` | 64 new / 1 stale, none in files this slice touched (pre-existing Portal 2/GameUI/Box3D drift) |

After the mesh queue hooks (the last change to the backend), the contract
test (173/0), all 14 pixel families (101 cases byte-identical to the
baseline again), the Portal boots in both modes and the `gi_door` pair
(byte-identical across modes) were rerun on the final backend
(`quality-results/mtq/reverify.txt`, `pix2/`, `rb*`, `rdoor*`). The
frame-pacing runs predate the hooks.

The frame-interval comparison is not a controlled benchmark: the host was
overloaded and the `cpu` column is the presenting thread's time, which is the
render thread in mode 2, so it does not compare across modes.

## Not done

- No TSan run of the product in queued mode; the census covers the hooked
  entry points only (bind, passes, draws, constants, textures, targets,
  clears, present, read-back, snapshots, world batches, light set).
- Android, macOS and iOS are unmeasured; Android stays pinned to mode 0.
- Resize, alt-tab/device-loss and window-mode changes under queued native
  were not exercised (`portal_boot --resize-stress --resize-mode queued`).
- `IWorldMeshUpload::IsResident` is read on the main thread; it changes only
  at map load and unload, which run single-threaded.
- Portal 2's intro scenes take `materials->Lock()` about once per frame in
  mode 2 (ownership changes ≈ frames): correct, but it serializes the
  threads there. The caller is not yet identified.
- The GPU compute service's queued dispatch-serial rule has no concurrent
  unit test; it is exercised only by the in-game SDF producer.
