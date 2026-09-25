# RFC 0011: Runtime Indirect Lighting with Substitutable Probe Producers

- Status: Proposed (2026-09-24); no implementation gate complete
- Date: 2026-09-24
- Scope: The runtime indirect-light (global illumination) data contract, a
  grid-addressable probe volume with visibility, the lighting policy that
  divides work between lightmaps and runtime producers, a published runtime
  light set, and four substitutable producers (baked, precomputed radiosity,
  SDF-traced, hardware ray query) that a graphics setting can switch at runtime
- Formats: [RFC 0008](0008-canonical-world-data-and-runtime-formats.md) owns
  the `PRBV` and `LMAP` lumps and the new `RTRN` transfer lump; this RFC
  defines the semantics they carry and amends their structure (see
  [Amendments](#amendments-to-rfc-0007-and-rfc-0008))
- Bake: [RFC 0007](0007-physically-based-lighting-pipeline.md) owns the light
  baker; this RFC adds separated direct/indirect outputs, probe visibility and
  the radiosity transfer precompute to its outputs
- Render seams and platforms: [RFC 0001](0001-capability-based-platform-architecture.md)
  owns render providers, `RenderFeature`, the native Vulkan backend (R28/R32)
  and platform profiles (R29/R36)
- Scheduling: [RFC 0003](0003-dependency-aware-job-system.md) owns the job
  graph and executors the CPU producer runs on
- Verification: [RFC 0005](0005-quality-and-correctness-harnesses.md)
- Language and synchronization: [RFC 0006](0006-modern-cpp-ownership-and-synchronization.md)

## Decision and boundary

RFC 0008 F10 requires "a real-time indirect-light response to moving lights
or objects on a declared desktop profile ... and explicit fallbacks on
profiles that lack those capabilities". It leaves the technique open. This RFC
fixes the architecture, not a single technique:

1. **One consumer contract.** Shaders read indirect light from a
   grid-addressable **probe volume** with per-probe irradiance and visibility,
   plus an optional surface indirect atlas. They never know which producer
   wrote it.
2. **Several producers.** Each producer implements one contract, declares what
   it responds to and what it needs, and passes one shared suite:
   - **Baked:** the Cycles bake, loaded from `PRBV` (every profile);
   - **Precomputed radiosity:** a CPU job gather over a baked transfer
     matrix, the Enlighten technique (every profile, including mobile);
   - **SDF-traced:** compute-shader probe tracing against a signed distance
     field (desktop and capable mobile);
   - **Ray query:** probe tracing with `VK_KHR_ray_query` (devices that
     expose it).
3. **One policy owner** decides what the lightmap carries and what the
   producer adds, so no light is counted twice.
4. **Runtime switching** is a normal operation: a graphics setting selects a
   producer, the new one is seeded from the current published volume, and the
   old one is retired behind GPU completion.

The world-space cache matters for this engine: portal views, recursive views
and water reflections sample the same volume for free, whereas screen-space
GI would break at every portal boundary.

Not in scope: glossy reflections (RFC 0007 F / R50 owns reflection probes),
screen-space effects, direct-light shadow techniques beyond what the producers
consume, legacy shader families (they keep ambient cubes), and light transport
*through* open portals (a later extension; see open decisions).

## Observed starting point

Facts observed in the tree at `73e7ec64` plus the shared dirty tree on
2026-09-24. They are not implementation claims.

**World lighting on the native path is fully baked and static.**

- The WMSH world draw (`engine/gl_rsurf.cpp:275`, `r_worldmesh_draw` default 2
  at :67) reads only the `LMAP` page. `LMAP` v1 is one linear `RGBA16F` KTX2
  page (`public/render/world_mesh_upload.h:38`, checked in
  `materialsystem/shaderapivulkan/vulkan_world_lightmap.cpp:27-62`).
- The bake sums direct and indirect: Cycles `DIFFUSE` with
  `pass_filter={"DIRECT","INDIRECT"}` (`tools/quality/pbrt_lightmap_bake.py:394`).
  No separated layer exists.
- A 2:1 page carries an L1 luminance gradient for normal maps
  (`tools/quality/lightmap_directional.py`, `world_pbr.frag:53`). The page's
  top rows hold one equirect reflection probe with box-filtered mips
  (`tools/quality/reflection_probe.py:8`).
- `world_pbr` has a direct-light term, but nothing sets it: `PbrWorldScene`
  defaults `lightRadiance` to zero (`vulkan_device.h:557-571`) and
  `SelectPbrWorldMaterial` (`vulkan_world_pbr.cpp:273-313`) never fills it.
  Only `test_world_pbr_pixels.cpp:180-204` does.
- No light styles, dlights or elights reach the WMSH path. `LSTY`, `PRBV`,
  `RPRB` and `MTBL` exist only in RFC text; only `LHDR`, `LGAP`, `Lnnn`, `WMSH`
  and `LMAP` are implemented (`public/mapcontainer/map_container_format.h:60-90`,
  `world_mesh_format.h:56-59`).

**Dynamic models use legacy leaf ambient data that does not match the bake.**

- `model_pbr.frag` and `skin.frag` read an ambient cube (`c4..c9`) and four
  local lights (`c20..c25`). The engine fills them in
  `CModelRender::StudioSetupLighting` (`engine/l_studio.cpp:1388`) from
  `R_StudioGetAmbientLightForPoint` → `ComputeAmbientFromLeaf`
  (`engine/lightcache.cpp:679,728`) → `Mod_LeafAmbientColorAtPos`
  (`engine/modelloader.cpp:6553`).
- On pipeline-built BSP2 maps those leaf samples come from `vrad -bounce 0` on
  the collision VMF (`tools/quality/pbrt_map_build.py:437`), whose only light
  is a fallback ceiling `light` (`pbrt_collision_vmf.py:412,436-437`). Model
  lighting is therefore unrelated to the Cycles bake the world uses.
- No probe-volume or SH L2 runtime code exists. `public/render/pbr_sh_l1.h` is
  included only by its unit test.

**The native Vulkan device is graphics-only.**

- `CVulkanContext` targets Vulkan 1.1 (`vulkan_device.cpp:208`) with one
  graphics queue and optional present queue (:372-387, :438). Enabled features
  are `occlusionQueryPrecise`, `samplerAnisotropy`, `shaderClipDistance` and
  `textureCompressionBC` (:486-513).
- There are no compute pipelines, storage images or buffers, 3D images,
  timeline semaphores, descriptor indexing or ray-query code anywhere in
  `materialsystem`, `public`, `engine` or `unittests`. Descriptors are combined
  image samplers and uniform buffers only.
- Resource retirement uses submit serials and fences:
  `RetiredTexture{…, afterSerial}` and `RetireCompletedTextures()`
  (`vulkan_device.h:1498-1510`, `vulkan_device.cpp:3982,4009`).
- `RenderFeature` (`public/render/render_backend.h:83-92`) has
  `kComputeShaders`, but the render-seam provider advertises it
  unconditionally (`vulkan_render_backend.cpp:28-44`) and the legacy native
  backend reports an empty `RenderAdapterInfo`
  (`shaderapivulkan.cpp:2371`). Neither report is evidence of a compute path.

**No shadows or dynamic direct lighting exist on native Vulkan.**

- `SetFlashlightState[Ex]` are unimplemented (`shaderapivulkan.cpp:2018-2027`);
  `SupportsShadowDepthTextures()` returns false (:2198) and `CreateDepthTexture`
  returns 0 (:8398). Video-options P7 (flashlight shadow depth) is not started.
- No clustered or tiled light lists exist.

**Scheduling, settings and tests.**

- The RFC 0003 graph API (`public/jobsystem/job_graph.h`,
  `pooled_executor.h`) has no production consumer; engine cohorts use
  `JobGraphParallelProcess` (`public/vstdlib/jobgraph_parallel.h`).
- Video settings flow from archived convars through
  `CMaterialSystem::OverrideConfig` (`cmaterialsystem.cpp:2397`) to
  `ChangeVideoMode` and mode-change callbacks (`shaderapivulkan.cpp:1489,2714`).
  `render.present-policy.v1` (`public/render/render_present_policy.h`) is the
  model for a small native-type-free policy header with a Vulkan binding.
- GPU pixel suites (`world_pbr_native_pixel_conformance`,
  `model_pbr_native_pixel_conformance`, …) are standalone wscript programs;
  `quality/conformance.manifest.json` has only `linux-headless-core` rows.
- The closest GI fixture is `quality/fixtures/usd-maps/room/room.usda`: a room
  with a window opening, a `DistantLight` sun, a `RectLight` ceiling panel, a
  `DomeLight` sky and an emissive screen. There is no furnace, leak or
  moving-occluder fixture.

## Goals

- Dynamic objects on BSP2 maps are lit by the same bake as the world.
- A light that changes (switched, dimmed, moved, recolored) changes the
  indirect light it contributes, on every declared profile, within a declared
  latency.
- On desktop profiles with the capability, moving geometry also changes
  indirect light (a door closing darkens the room behind it).
- The producer is a graphics setting. Switching it at runtime neither drops a
  frame to black nor frees in-use GPU memory.
- Every producer is checked against the same independent oracle: Cycles
  renders of each interaction state, plus analytic cases.
- Mobile power and memory are budgeted before optimization, and the CPU
  producer runs within them.

## Non-goals

- Replacing the Cycles lightmap bake. Lightmaps remain the highest-quality
  answer for static light on static surfaces on every profile.
- Screen-space GI, voxel cone tracing, light propagation volumes and Lumen-style
  surface caches. They are rejected alternatives (below), not deferred phases.
- Glossy indirect light. Probes store irradiance; specular stays with RFC 0007
  reflection probes. Relightable reflection probes (open decision 5) were
  decided as an RFC 0007 amendment that consumes this RFC's change.
- Claiming identical pixels across producers. Each producer declares its own
  tolerance against the shared reference.

## Layers and owners

| Concern | Owner | Consumers |
| --- | --- | --- |
| Probe-volume structure and encoding (`PRBV`) | RFC 0008 writer and reader, semantics in this RFC | Every producer, shaders, legacy exporter |
| Baked values (lightmap layers, probes, transfer) | RFC 0007 baker | Baked and radiosity producers |
| Runtime light set (which lights exist this frame, their state) | New `render.light-set.v1`, published by the engine | Radiosity/SDF/RT producers; RFC 0008 F5 clustered lighting |
| Indirect-light policy per producer | New `render.indirect-policy.v1` | Shader variant selection, producers, tests |
| Producer lifecycle, capabilities and switching | New `render.indirect-light.v1` | Native Vulkan provider, settings, tests |
| Selected producer (user setting) | `r_indirect_producer` archived convar, validated at composition | Video options UI |
| GPU resources and completion | Native Vulkan provider (serial/fence tokens today) | Producers, switching |

The header names below are proposed. None of them exist yet.

## Probe volume contract (`render.probe-volume.v1`)

A map carries zero or more **probe grids**. Each grid is an axis-aligned
box with an origin, spacing and dimensions, and is directly addressable by
the GPU and by CPU producers. Per probe:

- **Irradiance:** octahedral map, 6×6 interior texels plus a 1-texel border
  (8×8), linear HDR (`RGBA16F` master; `B10G11R11_UFLOAT` is a per-profile
  encoding decided by measurement: open decision 1 found it lossless enough
  but unsigned, so it is for baked layers only, never a change volume).
- **Visibility:** octahedral mean and mean-squared distance to the first hit,
  14×14 interior plus border (16×16), `RG16F`. The shader applies a Chebyshev
  test, which is what stops light leaking through walls and thin panels.
- **State:** active/inactive (inside solid) and a bounded relocation offset
  that moves a probe out of geometry without changing its grid index.

Each grid also carries a **style layer** count for light styles, a content
hash and a producer ID/epoch when published at runtime.

Consumers sample the eight surrounding probes with trilinear weights, a
backface term, and the visibility test. The sampling function lives in one
shared GLSL include and one C++ reference implementation. The C++ version is
the oracle for the GPU version.

The storage is a 2D texture atlas, not a 3D image. That means the baked tier
needs no new device feature, which today's graphics-only device lacks. (See
G1.)

This replaces RFC 0008's SH L2 tetrahedral plan (its open decision 3). The
reasons:

1. runtime producers must write probes by index, which a tetrahedral mesh
   makes awkward;
2. visibility data is required for leak control and SH L2 carries none;
3. keeping one encoding for baked and runtime probes makes load a copy.

SH L2 remains a bake-side intermediate and a measured alternative encoding
(open decision 1).

## Indirect-light policy (`render.indirect-policy.v1`)

Exactly one policy is active per map and producer. The policy says what the
lightmap contains and what the producer publishes:

| Policy | Lightmap on world | Producer publishes | Dynamic objects read | World reads |
| --- | --- | --- | --- | --- |
| `Baked` | Total (direct + indirect), per style | Nothing (baked volume is static) | Baked volume | Lightmap |
| `BakedPlusDelta` | Total, per style | Indirect from **changed** lights only (a signed delta volume, plus an optional delta surface atlas) | Baked volume + delta | Lightmap + delta |
| `RuntimeIndirect` | Direct only, per style | All indirect | Runtime volume | Lightmap direct + runtime indirect (atlas if published, else volume) |

- "Changed" lights are those the light set marks as differing from their
  baked state: moved, toggled beyond their style, recolored, or not baked at
  all (`light_dynamic`, dlights, elights, the flashlight).
- `RuntimeIndirect` requires the bake to write the direct and indirect layers
  separately (amendment to RFC 0007). Only producers that also respond to
  geometry motion benefit from it, because geometry motion changes static
  lights' bounce too.
- The policy is part of the shader variant key. A seeded double count
  (`BakedPlusDelta` data read under the `RuntimeIndirect` variant, or the
  reverse) must fail the furnace oracle.
- Light styles on the world stay in the lightmap in every policy (RFC 0008
  `LSTY`). A producer that responds to intensity treats a style scalar change
  as a changed light only under `RuntimeIndirect`.

## Runtime light set (`render.light-set.v1`)

Producers need the lights of the current frame, which the native world path
does not have today. The engine publishes an immutable per-frame snapshot:

- world lights (from `LUMP_WORLDLIGHTS`, or the World Stage lights for native
  USD maps) with their baked identity, style and current scalar;
- dynamic lights (`dlight_t`, entity lights, `light_dynamic`, the flashlight)
  with position, direction, cone, color and radius;
- per light: `baked` flag, whether it currently matches its baked state, and a
  stable ID across frames;
- the frame's epoch.

The snapshot is value data built on the main thread before render work is
scheduled, and borrowed read-only by producers for that frame. It has no
native types. RFC 0008 F5's clustered direct lighting consumes the same
snapshot. That gives one owner for "which lights exist" and removes the need
for the engine light cache and a renderer light list to be kept in sync.

## Producer contract (`render.indirect-light.v1`)

Illustrative shape (final names follow the header review):

```cpp
namespace render::indirect
{
enum class ProducerKind : uint8_t { Baked, PrecomputedRadiosity, SdfTraced, RayQuery };

enum class Response : uint32_t
{
	LightIntensity = 1u << 0, // style scalars, toggles, dimming, color
	LightMotion    = 1u << 1, // moved or unbaked lights
	GeometryMotion = 1u << 2, // doors, panels, props occlude and bounce
	Emission       = 1u << 3, // emissive material changes
};

struct ProducerCaps
{
	ProducerKind kind;
	ResponseSet responds;
	PolicySet policies;               // which render.indirect-policy.v1 modes
	RenderFeatureSet requiredFeatures;
	uint32_t convergenceFrames;       // declared; checked by the suite
	bool publishesSurfaceAtlas;
};

class IIndirectLightProducer
{
public:
	virtual ~IIndirectLightProducer() = default;
	virtual ProducerCaps Caps() const = 0;
	// Validates scene data and allocates. Fails without side effects.
	[[nodiscard]] virtual Expected<void, IndirectError> Begin(
	    const IndirectScene &scene, const ProbeVolumeView &seed ) = 0;
	// Adds this frame's CPU jobs and GPU work; never blocks on the GPU.
	virtual void Schedule( FrameWork &work, const LightSetSnapshot &lights ) = 0;
	// The newest complete volume, or none before the first publication.
	virtual std::optional<PublishedVolume> Published() const = 0;
	// Stops scheduling. Resources are released once the ticket completes.
	[[nodiscard]] virtual RetireTicket End() = 0;
};
}
```

Every producer, including fakes, passes one shared suite. The obligations:

- **Validation:**
  - `Begin` either succeeds or leaves no resources, jobs or GPU work behind.
  - `Begin` rejects scene data it cannot use (missing `RTRN`, a missing SDF
    build input, an unsupported policy) with a structured `IndirectError`.
- **Publication:**
  - A published volume is complete and immutable, and carries a strictly
    increasing epoch.
  - It is visible to consumers only after the producer's CPU publication and
    GPU completion, following RFC 0006 happens-before rules.
- **Seeding:** the first publication after `Begin` must not be darker than the
  seed by more than the declared tolerance. A producer never publishes an
  unconverged zero volume.
- **Response:**
  - Within `convergenceFrames` of a change the producer declares it responds
    to, its output is within tolerance of the steady state for the new scene.
  - It does not claim a `Response` bit that the scenario suite shows it lacks.
- **Lifetime:**
  - `End` returns a ticket.
  - No resource the producer freed is still referenced by a submitted frame.
  - Destroying the producer before its ticket completes is a contract
    violation that the suite detects.
- **Scheduling:**
  - Producers use the frame's executor (RFC 0003) and never create threads.
  - CPU producers have a serial mode that is the oracle for pooled execution.
- **Budget:** each producer declares its per-frame CPU/GPU cost on each
  profile. The runner reports measured cost against it.

Deliberately bad producers must each fail the suite:

- publishes before seeding (black flash);
- ignores visibility (leaks);
- frees a volume a pending frame reads;
- reuses an epoch;
- claims `GeometryMotion` without responding to it;
- double-counts under the wrong policy;
- blocks on the GPU inside `Schedule`.

## Runtime switching

The selected producer is `r_indirect_producer`, an archived convar with the
values `baked`, `radiosity`, `sdf` and `rayquery`. Video options exposes only
the values the composed device and map support.

- **Startup validation:** at composition, a saved value the profile cannot
  satisfy is rejected. The rejection is logged as a structured diagnostic, the
  profile's declared default is used, and the evidence records the
  substitution.
- **No silent fallback** at any other time.

Switching from producer A to producer B:

1. Build the seed from A's newest published volume (or the baked volume).
2. `B.Begin(scene, seed)`. On failure, A stays active, the setting reverts,
   and the error is reported. Nothing has been mutated.
3. B schedules updates while A's last volume remains the published one.
   A stops scheduling.
4. When B publishes its first epoch, consumers cross-fade from A's last volume
   to B's over a declared number of frames. Both volumes are resident only
   during the fade, and the memory budget includes that peak.
5. `A.End()`. A's resources retire when the provider's completion serial
   passes the last frame that sampled them. This uses the same mechanism as
   `RetireCompletedTextures` today, extended to buffers and compute resources
   in G5.

Special cases:

- **Device loss:** all GPU producers are dropped without waiting. The baked
  volume is republished from CPU data. The previous selection is re-established
  through steps 1–5 after recovery, and the event is reported.
- **Backgrounding (mobile):** CPU producers stop scheduling. GPU producers
  release transient resources and re-seed from the baked volume on resume.
- **Map change:** producers end with the map and begin again on the next map.

Because switching is only these operations, it doubles as the lifecycle test.
The same sequence also covers map load, device loss and backgrounding.

## Producers

### Baked

Loads `PRBV` and publishes it once. It needs no new device feature. It is the
seed and fallback for every other producer, and it is the policy-`Baked`
answer on every profile. Its values come from the RFC 0007 Cycles bake,
amended to write probe visibility.

### Precomputed radiosity (CPU)

Offline, the baker adds a transfer precompute to the `RTRN` lump:

- the static world is split into **surface patches**: clusters of lightmap
  chart texels at a declared density, thousands to tens of thousands per map;
- per patch, a sparse list of source patches and form factors with visibility
  baked in, computed by the same ray tracer as the lightmap bake;
- per probe, a sparse list of patches and weights (probe gather);
- per patch, per baked light: static visibility for direct-light injection.

At runtime, each frame (or every Nth frame on low-power profiles):

1. **Inject** direct light into patches from the light set: baked-position
   lights through their precomputed visibility, and moved or dynamic lights
   through a shadow term. The shadow term needs shadow maps, so this response
   is only declared once shadow maps exist on the profile.
2. **Gather** one bounce: each patch sums its sources' previous exitance.
   Feeding results forward gives multiple bounces over successive frames.
3. **Publish** a low-resolution surface indirect atlas, mapped through the
   chart atlas, and the probe volume via the probe gather.

The gather is data-parallel with one writer per patch. The reduction order is
fixed within a patch, so serial and pooled execution produce identical output.
It becomes the first production consumer of the RFC 0003 graph executor, and
must match the serial oracle exactly.

GPU cost is two small uploads per publication. No compute is needed.

This producer declares `LightIntensity`, `Emission` and, once shadow maps
exist, `LightMotion`. It does not declare `GeometryMotion`: moving objects
receive light from the probe volume but neither occlude nor bounce it.

### SDF-traced (compute)

- The static world is voxelized from `WMSH` into a clipmapped signed distance
  field with albedo and normal at build or load time.
- Dynamic objects add conservative proxy shapes (boxes or capsules from their
  bounds or physics hulls) every frame.
- A compute pass traces a fixed ray budget per probe for a rotating subset of
  probes. Each ray samples the light set at its hit point (using shadow maps,
  or SDF shadows where shadow maps are missing). The pass blends results into
  irradiance and visibility with declared hysteresis.
- The producer declares `LightIntensity`, `LightMotion`, `GeometryMotion` and
  `Emission`, and supports `BakedPlusDelta` and `RuntimeIndirect`.
- It requires the G5 compute foundation. Godot 4's SDFGI and AMD FidelityFX
  Brixelizer GI are the reference designs to evaluate. Their licenses are to
  be recorded before any code is used.
- *As built (open decision 7, decided):* the field is a uniform SDFV baked at
  map build, not a clipmap voxelized from `WMSH` at load. The producer uses
  neither reference's code.

### Ray query (compute)

- Same probe update as the SDF producer, but rays hit real triangles:
  - bottom-level acceleration structures from `WMSH` and model meshes;
  - a top-level structure rebuilt each frame for dynamic instances.
- Requires the Vulkan 1.2+ device path, `VK_KHR_acceleration_structure`,
  `VK_KHR_ray_query` and buffer device address, each queried rather than
  assumed.
- MoltenVK is not assumed to expose these. Apple profiles do not offer the
  option until a device query shows support.

## Validation and acceptance

Harness families: Q-PRESENTATION (pixels, lifecycle, switching, loss),
Q-CONTENT (`PRBV`/`RTRN` readers, fuzzing, legacy derivation), Q-JOBS
(radiosity serial/pooled equivalence), Q-FOUNDATION (producer and light-set
contracts) and Q-PRODUCT (installed settings, budgets, portal views).

### Fixtures (new, versioned under `quality/fixtures/gi/`)

| Fixture | Purpose |
| --- | --- |
| `furnace` | Closed box, uniform albedo ρ and emission: analytic infinite-bounce radiosity; detects missing or double-counted bounce |
| `thin-wall` | Two rooms split by a 4-unit panel, one lit: indirect light in the dark room must stay below the declared leak bound; a visibility-disabled build must exceed it |
| `room-states` | `quality/fixtures/usd-maps/room/room.usda` with states: ceiling panel on/off, sun at two angles, screen emission on/off |
| `door` | A room lit through a doorway, door open and closed: the geometry-motion oracle |
| `probe-grid` | Analytic single distant light and uniform sky sampled at known probes: irradiance and visibility reference |
| `portal-view` | A lit chamber viewed directly and through a portal pair: identical indirect contribution in both views |
| Portal corpus maps | Product budgets and switching soak on real content |

**Reference oracle.** The pinned Cycles renders each fixture state with the
same materials:

- a total-light frame;
- an indirect-only frame (the Cycles `INDIRECT` pass filter) with a matching
  camera.

The runtime provides a debug view that isolates indirect light, and that view
is compared with the Cycles indirect-only frame per region. The runtime is
never compared against its own screenshots.

### Suites (manifest rows)

- `render.probe-volume` and seeded sensitivities:
  - trilinear weights;
  - octahedral mapping and borders;
  - visibility test;
  - relocation;
  - C++ reference matches GLSL (headless, then GPU).
- `render.light-set`: snapshot immutability, stable IDs, baked-state
  comparison.
- `render.indirect-policy`: variant selection; seeded double count fails the
  furnace.
- `render.indirect-light.<producer>`: the shared producer suite against each
  producer and each bad producer.
- `render.indirect-switching`: every ordered producer pair, a switch during a
  pending publication, a failed `Begin`, device loss mid-fade, backgrounding.
  All use completion-serial checks.
- `world.prbv` and `world.rtrn`: independent Python readers, malformed-input
  fuzzing, hash and bounds rejection.
- `jobsystem.radiosity`: serial versus pooled byte-identical at 1, 2 and N
  workers.

The GPU suites need a GPU runner profile in `quality/conformance.manifest.json`.
None exists today, so it is a G0 deliverable. Standalone wscript programs do not
certify a gate.

## Delivery plan and gates

Each gate lists what "done" means. Numbers marked *provisional* are proposed
targets that G0 replaces with measured budgets before optimization, as
AGENTS.md requires. A gate is unverified on any profile whose runner or device
was unavailable.

### G0: Baseline, fixtures and runner

Done when:

1. The fixture set above exists with Cycles total and indirect-only references
   for every state. The Cycles revision, stage hashes and seeds are recorded.
2. The conformance manifest has a GPU profile, such as
   `linux-native-vulkan-gpu`. The existing world and model PBR pixel programs
   run through it with `checks-v1` records, and a zero-check run fails.
3. The indirect-light debug view exists on native Vulkan, which lets the
   Cycles indirect-only comparison run today. A seeded "indirect doubled"
   build fails it.
4. Frame-time and memory budgets for indirect light are recorded per declared
   profile before any producer optimization: Linux desktop (Radeon 8060S),
   Android (Fold7), and the Apple profiles once their runners exist. Each
   budget has an owner.
5. The dynamic-model inconsistency is captured: `room-states` with a model
   under the ceiling panel shows today's leaf-ambient error against Cycles.
   This is the "before" measurement.

### G1: Probe volume, baked producer and consumer

Depends on G0 and RFC 0008 F5 carriage.

Done when:

1. The RFC 0007 bake writes probe irradiance and visibility from panoramic
   Cycles radiance and depth renders. `bsp2tool` packs `PRBV`, and the
   independent Python reader validates structure, hashes, active masks and
   relocation bounds.
2. The engine loads `PRBV` for BSP2 maps. Malformed and truncated lumps are
   rejected with structured errors, the map stays playable, and it falls back
   to leaf ambient with a reported diagnostic.
3. `model_pbr.frag` samples the volume per pixel on native Vulkan. For legacy
   families (`skin`, `VertexLitGeneric`), the ambient cube for a model is
   evaluated from the volume at its lighting origin. That replaces
   `ComputeAmbientFromLeaf` for BSP2 maps with `PRBV` and leaves legacy maps
   unchanged.
4. **`probe-grid`:** irradiance within the declared analytic tolerance. The
   provisional bound is 3% relative error for a uniform sky and 5% for a
   single distant light at probe normals.
5. **`thin-wall`:** dark-room indirect stays below the leak bound (provisional:
   2% of the lit room's mean). The visibility-disabled sensitivity build
   exceeds it.
6. **`room-states`, baked state:** a dynamic model's indirect-only pixels are
   within tolerance of the Cycles indirect-only frame, and measurably better
   than the G0 leaf-ambient capture.
7. RFC 0008's legacy exporter derives `LUMP_LEAF_AMBIENT_*` from `PRBV`, so
   D3D9/DXVK models match the same bake, and a DXVK capture confirms it. The
   VRAD fallback-light leaf ambient is no longer produced for pipeline maps.
8. No new device feature is required: the volume is a 2D atlas. Load cost and
   memory per corpus map are recorded.

Implementation notes (2026-09-24, [progress](0011-progress.md#g1-probe-volume-baked-producer-and-consumer)):

- The bake measures irradiance with a Cycles diffuse bake on receiver quads
  (one per tile texel, invisible to light transport). It measures distance
  moments with BVH rays rather than panoramic renders. This is exact per texel
  for every light type, including suns and lamps a panorama cannot see.
- Item 7's DXVK capture is deferred by the user's native-Vulkan scope
  decision. The leaf ambient is still derived from the volume.
- Per-pixel sampling needs nine bound descriptor sets. Devices that bind fewer
  keep the ambient cube evaluated from the same volume.

### G2: Light set, separated bake, policy

Depends on G1.

Done when:

1. `render.light-set.v1` is published each frame from world lights, styles,
   dlights and elights. Its suite covers stable IDs, style scalars and the
   baked-state comparison. A seeded ID reuse fails.
2. The bake writes direct and indirect lightmap layers separately (RFC 0007
   amendment). Per texel, direct plus indirect matches the total within the
   bake's noise band. The furnace indirect matches the analytic value. The
   seeded swapped-layers build fails.
3. `LMAP` carries the separated layers (RFC 0008 amendment) and the reader
   validates them.
4. `render.indirect-policy` selects variants by policy, and the seeded double
   count fails the furnace.
5. `world_pbr` consumes the light set for direct light from unbaked lights
   (replacing the unused `lightRadiance` push constant). This is the minimal
   direct path the producers' `LightMotion` response needs to be visible;
   RFC 0008 F5 clustered lighting later replaces it through the same snapshot.

### G3: Producer contract and runtime switching

Depends on G1 and G2.

Done when:

1. `render.indirect-light.v1` exists with the shared suite. The baked producer
   and a scripted fake producer pass it, and each listed bad producer fails
   it.
2. `r_indirect_producer` is archived and validated at composition. Video
   options lists only supported values. An unsupported saved value produces
   the recorded diagnostic and the default.
3. `render.indirect-switching` passes on native Vulkan for baked↔fake,
   including a failed `Begin`, device loss mid-fade, and backgrounding on
   Android. It must show:
   - no black frame, checked as frame mean luminance never below
     (1 − tolerance) × the seed's;
   - no retired resource referenced after its completion serial;
   - peak memory during a fade within budget.
4. **`portal-view`:** indirect light through the portal matches direct view
   within tolerance for every producer present.

### G4: Precomputed radiosity producer

Depends on G3 and RFC 0003 R20. This is the default dynamic tier on every
profile.

Done when:

1. The baker writes `RTRN`: patches, sparse transfer, probe gather and
   per-light visibility. The independent reader validates index bounds,
   normalization, and that per-patch transfer sums are ≤ 1. Fuzzing rejects
   malformed data.
2. **`furnace`:** converges to the analytic infinite-bounce value within
   tolerance (provisional: 2%) in the declared number of frames. A
   one-bounce-only seeded build fails.
3. **`room-states`:** toggling the ceiling panel or the screen's emission
   converges within `convergenceFrames` (provisional: ≤ 0.5 s at 60 Hz). After
   convergence, indirect-only pixels match the Cycles indirect-only reference
   for that state within the producer's declared tolerance.
4. **`jobsystem.radiosity`:** serial and pooled output are byte-identical at 1,
   2 and N workers, under TSan on the stress configuration.
5. The CPU cost fits the G0 budget on the Linux desktop and on the Fold7,
   including a low-core configuration. *Provisional:* ≤ 1 ms per update on one
   desktop worker; ≤ 2 ms on the Fold7 at a declared update rate. Memory for
   `RTRN` is within the per-profile budget.
6. The producer runs on Android on the installed APK. Backgrounding and resume
   re-seed without a black frame.

### G5: GPU compute foundation

Depends on R28 and R32. Independent of G1–G4, so it can proceed in parallel.

Done when:

1. The native Vulkan device creates compute pipelines, storage images and
   buffers, and dispatches on the graphics queue. It enables features through a
   `VkPhysicalDeviceFeatures2` chain queried per device.
2. `RenderFeature` gains truthful bits (compute, storage images, ray query).
   The native backend reports them from device queries, not unconditionally.
   A provider that claims a bit it did not enable fails `render.profile`.
3. Retirement covers buffers and compute resources by completion serial. A
   seeded early-free build fails.
4. A compute conformance suite passes on the Linux GPU profile and on the
   Fold7.

### G6: SDF-traced producer

Depends on G3 and G5.

Done when:

1. It passes the shared producer suite, including `thin-wall`.
2. **`door`:** closing the door darkens the far room's indirect light, and it
   converges within the declared frames (provisional: ≤ 1 s) to within the
   producer's tolerance of the Cycles indirect-only reference for each state.
   The radiosity producer, which does not claim `GeometryMotion`, is shown
   *not* to pass this scenario. That demonstrates the scenario can tell the
   two apart.
3. **`room-states`:** sun angle changes converge.
4. GPU cost fits the desktop budget at 1920×1080 (*provisional:* ≤ 2 ms on the
   Radeon 8060S). On Android it is either measured within budget or declared
   unsupported for the profile.

### G7: Ray-query producer

Depends on G3 and G5, plus device support.

Done when:

1. The device path queries and enables acceleration-structure and ray-query
   features. Devices without them do not offer the option.
2. It passes the same producer suite and scenarios as G6, with tolerances at
   least as tight as SDF's.
3. GPU cost fits the desktop budget (*provisional:* ≤ 1.5 ms at 1920×1080 on
   the declared GPU). On the Fold7 it is either measured or declared
   unsupported.

### G8: Product

Depends on G4 plus whichever of G6 and G7 are complete.

Done when:

1. The per-profile default producer is chosen from measurements and recorded
   in the product profiles (`quality/product_profiles/`). Rollback to `baked`
   is a setting change only.
2. Installed Linux and Android products pass a switching soak over the Portal
   corpus: every producer pair, map changes, and 30 minutes of play, with no
   leaks and no validation errors.
3. Apple profiles run baked and radiosity on their runners when R29 provides
   them. Otherwise that claim is recorded as unverified.

### G9: Moved lights (amendment, 2026-09-25)

Added after G8 at the user's direction. It closes open decision 4 and backs
the traced producers' `LightMotion` claim, which G6 and G7 showed only for a
moved sun's indirect light. The pass scenario, chosen by the user, is a
light swinging on a rope.

**Decisions.**

- **Shadow technique (decision 4): SDF shadows.** The frame's unbaked lights
  are shadowed per pixel by sphere tracing the map's `SDFV` distance field,
  uploaded as a 3D texture. The traced producers already shadow with the
  same field, so direct and bounced light agree about what is occluded.
  Shadows are softened by the light's source radius. A profile or map
  without an `SDFV` draws unbaked lights unshadowed and the log says so.
  Shadow maps remain RFC 0008 F5's option for lights that need sharp
  shadows or have no field.
- **Inverse-square dynamic lights.** Source's dlight falloff is not
  physical and cannot be compared with Cycles. A `light_dynamic` with
  spawnflag 16 (`DLIGHT_INVERSE_SQUARE`) is an inverse-square bulb: its
  diffuse light at distance d is its color times (100 / d)², following
  vrad's convention of stating brightness at 100 units. Its source radius
  is 2 units, and its radius is a smooth cutoff. The light set carries the
  falloff kind; the native path honors it, and legacy paths keep the dlight
  falloff.
- **Producers take unbaked lights.** The traced producers add the frame's
  unbaked lights to their light buffer, visible to every probe, and a light
  moving changes their configuration. Radiosity does not; it declares
  neither `LightMotion` nor the scenario.

**Fixture `swing`.** A 5 × 5 × 3 m room with a red wall, a pillar and a
probe sphere. A bulb (a 5 cm emissive sphere) hangs on a 1.5 m rope from the
ceiling. The bake excludes the bulb, which is dynamic. Cycles references are
rendered for three states: `rest` (hanging straight down) and `left` and
`right` (40 degrees out). In game the bulb is a physics prop on a rope
constraint, with a parented inverse-square `light_dynamic`, released from
40 degrees so it swings.

Done when:

1. The light set carries the falloff kind and source radius, and a GPU check
   matches the native world path's inverse-square light to the CPU model,
   with a seeded legacy-falloff control that misses.
2. A GPU check of the SDF shadow gives lit, occluded and penumbra texels
   against an analytic occluder, and a seeded no-shadow control fails.
3. **`swing`, frozen:** for each state the lamp is held at the state's
   position (`DisableMotion`, then a `point_teleport`).
   - After the producer's declared convergence, the diffuse view
     (`mat_indirect_view 3`: all diffuse light, no albedo) matches Cycles
     DiffDir + DiffInd per world region within the producer's tolerance,
     and the indirect view matches DiffInd.
   - The shadowed pixels (the east-wall and floor pixels where Cycles'
     direct light is under a tenth of the region's) match the reference's
     light within the same tolerance.
   - Radiosity's indirect view fails every state (the bulb is unbaked), and
     shadows off (`r_indirect_shadows 0`) fails the shadowed pixels. These
     show the scenario tells them apart.
   - Models are measured, not gated: their light from the bulb is the
     engine's dlight path, unshadowed.
4. **`swing`, swinging:** while the lamp swings, consecutive frames
   (`startmovie` at a fixed 1/60 s step; screenshots cannot be consecutive)
   show no flicker beyond the motion. The high-frequency term, the RMS of
   each pixel's second temporal difference relative to mean luminance, may
   exceed the baked producer's by at most 0.002. Baked shows the same
   moving direct light and shadows, with no bounce.
5. The SDF shadow's cost at 1920 × 1080 is measured against a desktop budget
   (provisional: ≤ 1 ms for one light on the Radeon 8060S), and the
   producer's cost under continuous motion stays within its G6/G7 budget.
   Android, where the traced producers are unsupported, draws the bulb
   unshadowed with its direct light, and records that.

### G10: Light through open portals (amendment, 2026-09-25)

Added after G9 at the user's direction. It closes open decision 6. It
matters most for Portal: an open portal pair joins two places, and light
must flow through it the way it flows through a doorway.

**Decisions.**

- **The portal set.** The client publishes the frame's open portals, both
  portals of every activated, linked pair, to the engine
  (`render/indirect_portals.h`, `VIndirectLightPortals001`). Each carries
  its frame, its half size and the transform to its partner. The host hands
  them to producers in `FrameWork::portals`; `r_indirect_portals 0` (cheat)
  withholds them.
- **Traced producers (ray transforms).** A ray that enters an open portal
  from its front continues from the linked portal (at most two hops). A
  point in front of a portal sees the analytic lights around the linked
  portal through it: virtual lights at their images, each leg
  shadow-tested in its own space.
- **Radiosity (dynamic links).** When the set changes, receivers in front of
  a portal (patches and probes) gain links to the patches in front of its
  linked portal, at their images. The form factor is cos · cos · A /
  (π d² + A), and the probe SH follows the gather's convention. The map's
  SDFV lights beyond are sampled at their images too. The links carry the
  whole reflected light (bake plus change), and the absolute bake is solved
  once. They are unoccluded, since the transfer holds no geometry to test.
- **Layering.** None of this light is in the bake, so probes carry it in
  their indirect layer as well as their total. The world adds that layer's
  change, and models read the total.

**Fixture `portal-light`.** Room A, lit by a ceiling panel, and room B, with
no light of its own, are sealed and 6 m apart in the map. A linked
`prop_portal` pair on their facing walls is the only connection, so the
pair's transform is the 6 m translation. The Cycles reference joins the
rooms at one wall plane with a portal-sized opening (`open`), or plugs it
(`closed`).

Done when:

1. A GPU check shows a traced producer carrying light through a pair of
   portals on the contract's thin wall, like the wall erased, in the
   indirect layer, with the closed wall dark.
2. In game, with the pair open, room B's diffuse view matches Cycles `open`
   per world region within the producer's tolerance, for the SDF,
   ray-query and radiosity producers. Deactivating a portal makes room B
   match `closed`, which is dark.
3. The bake, and a traced producer denied the portals, fail `open`: the
   scenario tells them apart.

## Roadmap

Not yet ranked. AGENTS.md owns ranks and states. This RFC proposes the
following rows for the user to rank:

| Proposed ID | Scope | Prerequisites |
| --- | --- | --- |
| R70 | GI baseline, fixtures, GPU runner profile, indirect debug view (G0) | R02, R47 |
| R71 | Probe volume, baked producer, model consumer and legacy leaf-ambient derivation (G1) | R70, R53, R54 |
| R72 | Light set, separated bake and indirect policy (G2) | R71, R49 |
| R73 | Producer contract and runtime switching (G3) | R71, R72 |
| R74 | Precomputed radiosity producer (G4) | R73, R20 |
| R75 | Native Vulkan compute foundation (G5) | R28, R32 |
| R76 | SDF-traced producer (G6) | R73, R75 |
| R77 | Ray-query producer (G7) | R73, R75 |
| R78 | GI product defaults, soak and platform evidence (G8) | R74, R29 |
| R79 | Moved lights: SDF shadows, inverse-square dynamic lights, traced producers' unbaked lights, `swing` fixture (G9) | R76 or R77 |
| R80 | Light through open portals: client portal set, traced ray transforms and virtual lights, radiosity portal links, `portal-light` fixture (G10) | R74, R76 or R77 |

R71 supplies the `PRBV` portion of R56, and R74 together with R76 or R77
supplies the real-time indirect-light portion of R63. Neither row closes those
parents alone.

## Risks and mitigations

| Risk | Mitigation |
| --- | --- |
| Probe leaking through Portal's thin panels | Visibility is mandatory in the format; `thin-wall` gates every producer; relocation offsets |
| Radiosity patches too coarse for small rooms, blotchy indirect | Patch density is a declared bake setting measured on `room-states`; the surface atlas carries patch results at chart resolution |
| `RTRN` memory too large on mobile | Budget set in G0; sparse transfer with a pruning threshold measured against the furnace; per-profile patch density |
| Cross-fade memory peak during switching | Budgeted explicitly; fade length declared; switch refused with a diagnostic if the peak does not fit |
| Moving-light response needs shadow maps that do not exist | Declared per producer and profile; `LightMotion` claimed only after the shadow term exists; the suite enforces it |
| Hysteresis makes SDF/RT response visibly laggy | `convergenceFrames` is part of the declared contract and measured per scenario |
| MoltenVK lacks ray query | Apple offers baked, radiosity, and SDF if its compute path passes G5; no RT claim without a device query |
| A producer's look differs noticeably from another | Every producer is compared with the same Cycles reference; tolerances are per producer and published in its contract |
| Direct-only lightmaps under `RuntimeIndirect` double bake memory | Separated layers are a profile option; `Baked` and `BakedPlusDelta` keep the total layer only |

## Alternatives considered

- **Screen-space GI as the dynamic tier.** Cheap, but wrong off-screen and at
  portal boundaries, where every recursive view would need its own pass.
  Rejected as a producer; a screen-space accent could be evaluated later on top
  of a producer.
- **Voxel cone tracing.** Also gives rough glossy light, but leaks through thin
  geometry and is bandwidth-heavy on mobile. Rejected.
- **Light propagation volumes.** Too blurry and leaky; superseded. Rejected.
- **A Lumen-style surface cache with screen probes.** Best quality, but a very
  large engineering effort and not viable on mobile. Out of scope.
- **SH L2 tetrahedral probes (RFC 0008's original plan).** Compact and exact
  for irradiance, but not writable by index and without visibility. Kept as a
  measured encoding alternative only.
- **Separate GI systems per technique, each with its own data.** That would
  mean several consumer paths and several sets of tests. The single volume
  contract keeps one consumer and one oracle.

## Open decisions and required evidence

1. Probe irradiance encoding: octahedral 6×6 versus SH L2, and
   `B10G11R11_UFLOAT` versus `RGBA16F`. Decided in G1 by analytic error and
   memory.
   *Decided 2026-09-25: octahedral 6×6 in `RGBA16F`, as implemented since G1.*
   - SH L2 reconstructs smooth irradiance more accurately
     (`tools/quality/probe_encoding.py`). Mean error relative to the
     environment's level: sky 0.02% against 3.9%, colour bleed 0.1% against
     3.3%, a window 5.3% against 5.9%, a sun 9.9% against 11.9%.
   - SH L2 still loses:
     - it rings negative under a sun (103 of 2048 normals);
     - it carries no visibility, so the 1 KB octahedral visibility tile
       stays either way and dominates the memory (1,536 against 1,078 bytes
       per probe);
     - every producer writes and blends the octahedral tile per texel, and
       the G1 oracles gate that tile.
   - Strong direct light already goes through the light set, not the tile
     (G2).
   - `B10G11R11_UFLOAT` adds no measurable error (within 0.2% of `RGBA16F`)
     and halves the irradiance bytes. It is unsigned, so it can hold only
     the baked total and indirect layers, never a `BakedPlusDelta` change
     volume. It stays a per-profile option until a mobile memory budget
     needs it.
   - SH L2 irradiance with the octahedral visibility is the recorded
     alternative, if the between-texel error is ever what fails a gate.
2. Grid layout: uniform grids per region versus clipmapped cascades around the
   camera for large maps. Decided by corpus memory in G1, and required before
   G6.
   *Decided 2026-09-25: uniform grids, one per map at the pipeline's spacing
   (PRBV allows 16).*
   - The G1 corpus fits the budget (16 MB, 20 ms load) with an order of
     magnitude to spare, except `staircase2`. Its 1 m spacing over a large
     exterior gives 5,280 probes (2.6× the memory). The remedy is that map's
     `probe_volume.spacing_m` or `bounds_m`, not a runtime structure.
   - The camera-centred priority a clipmap would give comes from the traced
     producers' probe focus: the camera's PVS clusters first, a rotating
     budget for the rest. That keeps the cost bounded on a whole Portal map
     (4,862 probes: 0.61 ms against 3.48 ms without focus), with no
     cascade storage.
3. Whether the radiosity producer publishes a surface atlas or only probes,
   decided by `room-states` quality versus memory in G4.
4. Shadow technique for moved lights (shadow atlas versus SDF shadows), shared
   with RFC 0008 F5. This decides when `LightMotion` can be claimed.
   *Decided 2026-09-25 (G9): SDF shadows from the `SDFV` for unbaked lights;
   shadow maps stay F5's option for sharp shadows.*
5. Whether relightable reflection probes are added so dynamic light reaches
   specular. This would be an RFC 0007 F amendment, not part of this RFC's
   producers.
   *Decided 2026-09-25: yes, as an RFC 0007 image-based lighting amendment
   (R50-RELIGHT). Probes carry relight bands (RPRB v2: albedo, distance,
   normal), and `world_pbr` adds albedo times this RFC's change (the change
   volume and the unbaked lights' shadowed direct light) at the point each
   probe saw. The `mirror-lamp` gate passes against Cycles, and the probes
   as baked fail it (RFC/0007-progress.md, R50-RELIGHT).*
6. Light transport through open portals: dynamic patch links for radiosity, or
   ray transforms for SDF/RT. A later extension with its own fixture.
   *Decided 2026-09-25 (G10): both, plus virtual lights for the analytic
   lights beyond a portal; the fixture is `portal-light`.*
7. Which reference implementation the SDF producer follows (Godot SDFGI or
   FidelityFX Brixelizer GI), after license and fit review.
   *Decided 2026-09-25: neither.*
   - The producer is this RFC's own design, and no code, shaders or data from
     either project are used, so neither license applies.
   - The design, which fits the probe contract and Source maps better than
     either reference:
     - a uniform SDFV baked at map build (0.1 m voxels, with reflectance,
       emission, analytic light records and per-cell light lists), not a
       clipmapped field voxelized at load;
     - the probe volume's own texels traced per probe, with focus and a
       rotating budget;
     - brush-entity proxy boxes for moving geometry, and the same field's
       SDF shadows for unbaked lights (decision 4).
   - A future clipmapped or runtime-voxelized field would be a new producer
     behind the same contract, reviewed on its own.

## Amendments to RFC 0007 and RFC 0008

This RFC amends, with the same date:

- **RFC 0007 bake outputs:**
  - per lightmap sample, direct and indirect written separately in addition to
    the total;
  - probe outputs in the `render.probe-volume.v1` encoding with visibility
    instead of bare SH L2;
  - the optional `RTRN` transfer precompute.

  Real-time GI moves from RFC 0007's non-goals to this RFC.
- **RFC 0008:**
  - `PRBV` becomes the grid-addressable volume with visibility, which resolves
    its open decision 3;
  - `LMAP` may carry separated direct and indirect layers;
  - a new `RTRN` lump;
  - the legacy exporter derives leaf ambient samples from `PRBV`;
  - the native world path's "dynamic objects" and F10 real-time GI entries
    point here.

The amended sections in those RFCs say so inline.

## Source references

- `engine/gl_rsurf.cpp` (`Shader_DrawWorldMeshBatches`, `r_worldmesh_draw`),
  `engine/modelloader.cpp` (`Map_LoadWorldMesh`, `Mod_LeafAmbientColorAtPos`),
  `engine/lightcache.cpp` (`ComputeAmbientFromLeaf`,
  `R_StudioGetAmbientLightForPoint`), `engine/l_studio.cpp`
  (`StudioSetupLighting`)
- `materialsystem/shaderapivulkan/vulkan_device.{h,cpp}` (`PbrWorldScene`,
  `CreateLogicalDevice`, `RetireCompletedTextures`),
  `vulkan_world_pbr.cpp`, `vulkan_world_lightmap.cpp`,
  `shaders/world_pbr.frag`, `shaders/world_pbr_probe.glsl`,
  `shaders/model_pbr.frag`, `shaderapivulkan.cpp` (flashlight and shadow
  stubs, `ChangeVideoMode`)
- `public/render/render_backend.h` (`RenderFeature`), `render_profile.h`,
  `render_present_policy.h`, `world_mesh_upload.h`, `pbr_sh_l1.h`
- `public/mapcontainer/map_container_format.h`, `world_mesh_format.h`,
  `utils/bsp2tool/bsp2tool.cpp`, `tools/quality/bsp2_reader.py`
- `tools/quality/pbrt_map_build.py`, `pbrt_lightmap_bake.py`,
  `lightmap_directional.py`, `lightmap_ktx2.py`, `reflection_probe.py`,
  `pbrt_collision_vmf.py`
- `public/jobsystem/job_graph.h`, `pooled_executor.h`,
  `public/vstdlib/jobgraph_parallel.h`
- `quality/conformance.manifest.json`, `quality/fixtures/usd-maps/room/room.usda`
- External designs (not dependencies): Majercik et al., "Dynamic Diffuse
  Global Illumination with Ray-Traced Irradiance Fields" (JCGT 2019);
  Martin and Einarsson, "A Real-Time Radiosity Architecture for Video Games"
  (SIGGRAPH 2010); Godot 4 SDFGI; AMD FidelityFX Brixelizer GI

## Proposed decision

Accept for planning.

- **First:** G0 (fixtures, GPU runner profile, debug view, budgets) and G1
  (probe volume with the baked producer). G1 fixes today's mismatch between
  model lighting and the world bake on every profile and needs no new device
  feature.
- **In parallel:** G5, the compute foundation.
- **Before any GPU producer:** G4, precomputed radiosity, because it is the
  only dynamic tier that fits every declared platform.
- **Later:** the SDF and ray-query producers.
- **Defaults:** chosen per profile from G8 measurements, not from this
  document.
