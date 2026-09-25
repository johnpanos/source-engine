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

## Launcher trial (2026-09-25)

`./play` now passes `+mat_queue_mode 2` (`run.conf`/`run.sh` `EXTRA_ARGS`;
it pinned 0 before). The ConVar is archived, so the launcher passes 2
explicitly: a `config.cfg` saved under the old pin holds 0. `./play_p2` never
pinned it, and the SDL3 default is 2, so Portal 2 already ran queued. The same
change withdrew `+host_thread_mode 1`
([scheduler record, section 6](0003-scheduler-nodes-progress.md)).

Host: load about 2 on 32 threads, `build/` and `build-p2/` at `341e6c0b`,
headless unless stated.

| Check | Result |
| --- | --- |
| `frame_pacing.py` portal scenario, 3 interleaved rounds, mode 0 / mode 2 | warm interval median 12.8 / 9.9, 23.1 (noisy) / 8.4, 16.5 / 12.9 ms; p99 about 24 / 19 ms |
| Portal 2 scenarios, all four workloads (wheatley 4, triple laser 3, catapult 1, paint 1) | 9/9 pass; every run presents from the render thread with 0 cross-thread calls |
| Portal 1 two-map walk with the new `./play` arguments | the player moves on both maps; 1491 render-thread presents, 0 cross-thread calls |
| `portal_boot --resize-stress`, isolated headless mutter, mode 2 / mode 0 | both fail the harness, differently. Every size completes in both. Mode 0: blank images at 640x480 and 1024x576. Mode 2: three intermediate sizes coalesced and 35 presents scaled while a size settled. Both: each resize request costs the main thread 13 to 35 ms (budget 2 ms queued, 16.7 ms sync). No run of this harness on native had passed before |
| TSan product tree (clang, `build-tsan-queued`), frame-pacing scenario, one pass per mode | mode 0: 1901 reports, 158 signatures (pooled bones and particles, traces, spatial partition, model cache: known engine-pool debt). Mode 2: 1966 reports, 191 signatures, 46 not seen in mode 0 |

The 46 mode-2-only signatures, by kind:

- Source's queued design, the same under D3D9 and DXVK: material proxies on
  the main thread against queued `CMaterialVar::Set*` on the render thread;
  `R_MarkLights` marking surfaces against the queued `R_BuildLightMapGuts`;
  `g_nDebugVarsSignature` written in `BeginFrame` and read in `DrawMesh`;
  spew from `DrawElements`. At worst a value one frame old or torn.
- Native backend, debug only: `EnableDebugTextureList` writes a flag that
  the render thread's `EndFrame` reads (texture list panel).
- Existing families in new pairings, not queue-specific: particle random
  contexts, bone setup, SDL audio, the spatial partition.
- Not queue-specific but a real bug, found in this run by chance: Box3D's
  `GetSurfaceIndex` calls `CUtlSymbolTable::Find`, which writes a shared
  search context, from pooled particle traces on two workers at once. A
  lookup can return another thread's index. Fixed the same day: the Box3D
  table is a `CUtlSymbolTableMT`, as IVP's is
  (`vphysics_box3d/physics_material.h`). The shared physics suite's new
  `surfaceprops.concurrent-lookup` (four threads, 200,000 lookups) failed
  Box3D before the fix (54,724 wrong) and passes both providers after it;
  its bad provider `surfaceprops-unsynchronized` is detected. A queued TSan
  rerun has no `GetSurfaceIndex` report (8 and 10 before).

At exit TSan hung inside its own report of a mutex destroyed while locked in
`CFileSystem_Stdio`'s destructor, in both modes, so those runs were killed
after their workload. The mutex was `console.log`'s: on POSIX an open
writable file holds its inode's write lock, and the `-condebug` log handle
was never closed (`~ConsoleLogManager` runs after the filesystem). Fixed the
same day: `Con_Shutdown` closes the log, and later spew goes only to the
tier0 log until `Con_Init` (`engine/console.cpp`). At filesystem teardown
`console.log` is no longer open, it still ends with the last shutdown
lines, and the queued TSan rerun exits by itself (88 s, TSan status 66 for
the remaining reports).

## Not done

- Android, macOS and iOS are unmeasured; Android stays pinned to mode 0.
- Alt-tab/device-loss and window-mode changes under queued native were not
  exercised. Resize was (see the trial), and the resize harness does not pass
  in either mode.
- `IWorldMeshUpload::IsResident` is read on the main thread; it changes only
  at map load and unload, which run single-threaded.
- Portal 2's intro scenes take `materials->Lock()` about once per frame in
  mode 2 (ownership changes ≈ frames): correct, but it serializes the
  threads there. The caller is not yet identified.
- The GPU compute service's queued dispatch-serial rule has no concurrent
  unit test; it is exercised only by the in-game SDF producer.
