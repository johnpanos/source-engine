# One map quality profile for Source 2 quality and performance

Status: Partially implemented, 2026-10-02. The user selected one production
profile. The [installed bake/compiler slice](0007-progress.md#single-production-map-profile-and-compiled-reflection-candidates-2026-10-02)
enforces its settings and adds runtime-consumed reflection candidates. Larger
representation and authoring changes below remain proposals; 120 FPS is unverified.
The user's later direction requires invariant checks without oracle comparisons;
any new bake uses the remote render machine.

Use one `source2` quality profile for new maps, legacy relights, Hammer
builds and content tools. It should produce consistent materials, directional
baked indirect light, reflections and runtime direct lighting, with the
complete game image meeting the existing desktop High performance gate.
Faster iteration should come from reusing unchanged work and rebuilding
dependencies, rather than selecting a lower quality preset.

The bake and compiler should also produce static visibility, light/probe
candidate data and reusable caster inputs that reduce runtime preparation and
PBR shading. Reaching the existing 120 FPS gate is the performance objective.

Source 2 is the quality and performance goal. The preset name is not a claim
of parity or a claim that these are Valve's settings.

## One profile and one authority

Evolve the existing
[`quality/map_export_profiles/source2.json`](../quality/map_export_profiles/source2.json)
into the sole production map quality profile. Do not add another profile
registry, a preset inheritance tree, or separate authoring and release tiers.
The existing Linux High product profile references this map quality profile;
it continues to own runtime settings. The existing render budget continues
to own performance limits. These are complementary owners, not selectable
quality alternatives.

All production entry points resolve the same profile and record its revision
and content hash. Map manifests contain authored content and source references,
not overrides for sample counts, output resolution, denoising, probe budgets,
light transport, or acceptance thresholds. The present `with_defaults` merge
in `pbrt_map_build.py` must stop accepting those quality overrides.

Authored light intensities, materials, collision roles, probe influence boxes,
coverage regions and validation cameras remain map data. They must not become
an indirect way to reduce required lighting coverage or bypass quality checks.

The profile can evolve through reviewed revisions. All new builds use its one
current revision; immutable older receipts remain reproducible in Git history.
An older package is not relabelled as having passed the new revision.

Exact CPU, malformed-input and negative-control fixtures remain test inputs,
outside the production preset selector. They cannot publish production maps.
OS, ABI, packaging and device capability profiles also retain their existing
owners. One content quality policy does not erase platform requirements or
certify mobile and Apple support.

## Observed starting point

The current production presets vary materially. `legacy-relight` uses a 4096
atlas and 4096 samples but has no directional page or reflection probes.
`portal2-chamber-fast` uses 256 lightmap samples and 64 directional samples.
The current `source2` preset uses a 2048 atlas and 2048 samples; retained Intro4
and laser-intro manifests override it to 4096 and change other settings.
Some descriptions still say CPU although their executable device field is GPU.

The retained
[Intro4 source2 atlas receipt](../quality-results/relight/sp_a1_intro4_source2/lighting/atlas.ktx2.json)
records three 8192 by 4096 RGBA16F layers: 768 MiB of texel payload. Its
[probe receipt](../quality-results/relight/sp_a1_intro4_source2/lighting/probe_volume/prbv-bake.json)
records 38,080 probes and 156,875,424 bytes. These observations concern that
package, not every map or the current GPU residency.

That build also
[failed its reflection audit](../quality-results/relight/sp_a1_intro4_source2/audit.json),
including 49.25 percent uncovered walkable samples with its 16-probe limit.
Later probe64 work exists; this older result demonstrates that a high sample
count and a preset name do not establish acceptance.

The current
[directional encoding](../tools/quality/lightmap_directional.py)
fits a luminance gradient beside RGB irradiance. It cannot preserve the full
color variation of light arriving from different directions. The current
[layout owner](../tools/quality/lightmap_layout.py) fits a uniform texel density
into one atlas, so a larger map can receive less detail under the same preset.

## Frame cost evidence and immediate priorities

The user's supplied cost-overlay screenshot on 2026-10-02 shows sample 1156,
two frames old. Its largest visible scopes are:

| Scope | CPU recording wall time in ms | GPU time in ms |
| --- | ---: | ---: |
| Core world view | 48.030 | 75.098 |
| Core world | 0.296 | 67.141 |
| Prepare lights and shadows | 35.354 | 0.741 |
| World surfaces | 0.074 | 28.459 |
| Posed model PBR | 0.020 | 21.320 |
| Static model PBR | 0.074 | 17.201 |
| Prepare lit view bindings | 8.684 | 0.384 |
| GTAO | 0.009 | 4.802 |
| World prepass | 0.057 | 1.286 |
| Shadow tile restore | 0.013 | 1.069 |
| Shadow depth | 0.838 | 0.537 |
| Cluster BVH assignment | 0.001 | 0.688 |

These are inclusive scopes, not additive rows or a complete present-to-present
frame measurement. The CPU timer measures wall time inside recording labels,
including any waits or driver work. The GPU rows localize expensive draw scopes;
they do not establish whether shadow sampling, light loops, probe selection,
material fetches, overdraw or another operation dominates those scopes.
Posed-model draw time is not a measurement of animation or skinning alone.

The same recording interval reports 270 resource creations (208 buffers,
62 groups, zero textures), 269 destructions and 90,449.8 KiB of requested buffer
sizes. Light/shadow preparation accounts for 71,040.1 KiB of that size activity;
lit view bindings report 222 creations. These counters make transient buffer
and binding work a specific investigation target. They are logical resource
activity, not driver heap allocations, copied-byte measurements or live VRAM.

The first performance work therefore targets **CPU light/shadow preparation,
CPU view binding preparation, and GPU PBR shading across world and models**.
The visible shadow-depth and cluster-assignment GPU costs are much smaller;
neither should be blamed for the large CPU preparation interval without deeper
measurement. Lightmap compression remains valuable, but this capture does not
establish it as the first frame-time fix.

Use the same map, camera, full-image settings and warmed workload to collect a
repeatable trace. Break preparation down into planning, caster work, buffer
creation/fill, binding creation and waits. Within the shared surface shader,
measure light-list lengths, shadow receiver work, reflection candidates, texture
traffic and coverage. Term-off captures can isolate costs as diagnostics only;
they cannot become the accepted product image. Record actual resolution,
hardware clocks, profiler overhead and complete-frame timing alongside scopes.

## Proposed production settings

These are one set of initial values to validate, not measured optimal values.
They define the production profile together. Changes prompted by
validation update the one profile for all maps, rather than creating exceptions.

| Setting | Proposed value or rule |
| --- | --- |
| Profile name | `source2` |
| Baker | GPU Cycles through the existing `map_lighting.py` and `light_baker.py` path; record the actual backend and device |
| GPU unavailable | Fail the production bake with a clear diagnostic; no silent CPU or reduced-quality fallback |
| Lightmap samples | 2048 for every production bake |
| Directional samples | 2048; no separate low-sample directional shortcut |
| Light transport | Explicit `gi-reference`: 64 maximum/diffuse bounces, 16 glossy and transmission bounces, 16 transparent bounces, zero volume bounces, no direct or indirect clamping, existing caustics policy |
| Denoising | Chart-local OpenImageDenoise, with raw outputs retained as build evidence |
| Atlas | 4096 base atlas initially; directional storage dimensions and memory reported separately |
| Lightmap products | Separated direct and indirect, with the total derived from them; directional indirect required |
| Diffuse probes | 2 m requested spacing, 4096 samples, visibility and relocation required, maximum 50,000 probes |
| Probe overflow | Disable global spacing inflation; fail preflight if the required coverage cannot fit |
| Reflection captures | 256 cube-face resolution, 512 stored equirectangular width, 1024 samples, `gi-reference`, GGX roughness mips |
| Reflection placement | Up to the existing 64-probe capacity; room and glossy-surface coverage must pass, not merely reach the count |
| Moving-light GI | Off; no RTRN, SDFV or reflection relight bands in this profile |
| Runtime lighting | Baked indirect plus clustered runtime direct lighting and moving-caster shadows |
| Material inputs | Preserve authored color, normal, roughness, metalness, AO, emission and supported transmission semantics through the shared asset path |

The 2 m probe spacing is a starting policy, not proof against leaking at doors
or thin walls. Maps that fail the geometry or image checks remain unqualified.
Local grids covering relevant space are the next placement improvement; their
spacing and refinement rule must be validated and recorded in a subsequent
revision of this same profile. The existing broad grid may exceed the cap on
large retail maps. That is an explicit implementation limitation, not permission
to make their lighting progressively coarser.

Similarly, 4096 is an immediate consolidation setting, not the final density
contract. Multiple pages at a measured minimum texel density are required to
make quality independent of map size. Determine that density from the chamber
and large-map corpus before claiming the layout gate. Do not invent an
unvalidated density number or a virtual-texturing system for this first slice.

The current converter's 2048 texture-size cap is a separate material-quality
limitation. Remove that unconditional cap through RFC 0008's texture producer
and RFC 0015's asset resolver. Preserve available source resolution and use
ordinary mips and residency management; do not upscale assets. External P2:CE
and Workshop content remains read-only and opt-in under the existing mount
policy. Missing packs report the external-content coverage unavailable.

## Bake outputs that reduce runtime frame time

The first bake extension to investigate is **precomputed static direct
visibility**, accompanied by compiled spatial candidates and caster data.
Indirect light is already baked. Raising its sample count will not remove
the per-light work visible in the capture.

The current soft-shadow receiver performs a 16-sample blocker search and,
when needed, 16 filtered comparison samples per evaluated shadow. This is a
specific candidate for the large PBR draw cost, not a measured attribution of
all 67 ms. Static shadow-depth caching already exists; baking another depth
image alone does not eliminate receiver-side search and filtering.

The [October 2 engine comparison](0016-rendercore-engine-comparison-2026-10-02.md#shadows-distinguish-producing-depth-from-sampling-it)
adds stronger evidence than the screenshot alone: a retained diagnostic reduced
arrival GPU median from about 61 ms to 23.8 ms by disabling receiver visibility
while leaving shadow production, falloff and BRDF evaluation enabled. This
implicates receiver execution and shader resource behavior; the difference is
not an additive shadow pass or a promised bake speedup. The diagnostic changes
the image, and even its remaining cost misses the 120 FPS target.

Use that comparison to focus the first bake experiment on skipping certified
constant visibility before the costly filter. Hardware comparison sampling,
direct world-cube face selection and cached static depth already exist. The
experiment must add value beyond them. Compare offline classification with
runtime conservative bounds under the same shadow owner; retain the simpler
measured winner, rather than require two acceleration mechanisms.

The comparison also identifies runtime work the bake cannot replace: coherent
light traversal, shader live-range/register control, persistent descriptor
bindings and depth rejection across cohorts. Ordered candidate data from the
compiler must preserve per-fragment membership and accumulation order, but it
does not itself make shader execution coherent. Light/probe candidate refinement
and compact list storage follow measured list waste; GPU assignment is already
present and relatively cheap in its retained benchmark. Do not replace it or
start a GPU-driven geometry/async-compute rewrite to solve an unproven bottleneck.

| Additional compiled output | Runtime work it can remove | Required behavior |
| --- | --- | --- |
| Static visibility classification for fixed lights and fixed receivers | Blocker/filter work in provably fully lit or occluded regions | Conservative classification over positions, bias and the complete filter footprint; uncertain regions retain the full filter |
| Light candidate lists for world clusters and model-receiver cells | Evaluation of lights that cannot contribute, including their shadows and BRDF | Retain every potentially contributing light and account for runtime lights and movable geometry |
| Reflection candidates per spatial cell or surface cluster | Full probe-rank scans during world and model shading | Preserve influence, priority, nearest selection, blending and boundary transitions |
| GPU-ready static caster geometry and per-light candidate ranges | Repeated static caster gathering, conversion and payload creation during CPU preparation | Invalidate through existing owners when geometry, lights, coverage materials or transforms change; keep moving casters current |

These are proposed extensions to existing map/render contracts, not new format
names or implemented switches. Keep the data compact rather than adding one
full-resolution lightmap per light. Start with one fixed light and world/static
receivers, then expand after an image-preserving complete-frame improvement.
Spatial receiver cells also help moving models through conservative candidate
reduction, although their final shading remains dynamic.

A statically lit region still needs moving-shadow evaluation when a mover can
affect it. A region certified fully blocked by immutable geometry can skip that
light only when the proof covers the actual shadow semantics. Uncertain regions
and penumbrae retain the full filter. Separately averaged static and dynamic
soft-shadow masks cannot simply be multiplied: joint visibility depends on
which emitter samples each occluder blocks. A cached filtered result must
preserve that composition or be invalidated when it cannot.

A few sampled bake rays are not proof that a light never reaches a cell. Doors,
alpha-tested casters, transmission, destruction and switchable lights must be
represented as mutable inputs or excluded from static proofs. Do not derive
candidate lists solely from one camera or a closed-door state. False positives
cost time; false negatives remove valid lighting and fail acceptance.

Keep direct specular and normal-map response in the shared runtime material
model. A total-light color atlas cannot replace view-dependent highlights and
moving-shadow behavior. This proposal accelerates the current runtime-direct
contract. Changes to visibility representation must be specified and proven
under RFC 0016's existing shadow owner before product integration.

Measure the reduction in CPU preparation and GPU draws plus complete-frame
timing at High. Compiled caster inputs may help the 35 ms CPU scope, but waits,
allocation and binding overhead still require runtime fixes. Static-receiver
visibility cannot eliminate posed-model lighting costs. Bake work contributes
to 120 FPS; the screenshot cannot establish that baking alone will reach it.

## Runtime settings and performance

Use the existing
[Linux High settings](../quality/product_profiles/portal2-linux-native-vulkan-high.json):
4x MSAA, anisotropy 16, highest texture mip, LOD 0, High GTAO and shadows, depth
prepass, moving shadows, runtime direct light and baked indirect. This is the
qualification composition; the experimental core handoff does not become a
shipped default merely because this proposal selects its quality target.

The authoritative acceptance target is
[`linux-desktop-high-120`](../quality/budgets/render-v1.json), including its
native resolution, every-frame limit and complete image requirements. No
sample count, effect, resolution or contributing light is reduced to meet it.
More bake samples at a fixed output layout do not increase runtime shading
work; better runtime performance must come from representation and execution.

Prioritize these changes within their existing owners:

1. Remove measured repeated CPU preparation and resource churn. Reuse immutable
   light/caster data and compatible view bindings through their existing owners;
   update only revision-dependent payloads, using completion-safe buffer storage.
   Preserve per-view transforms, moving casters and nested-view isolation. Do not
   retain stale resources or add a second cache authority to suppress counters.
2. Reduce the measured GPU PBR cost shared by world and models. Add conservative
   reflection candidates and improve light/caster bounds; optimize shadow receiver
   work only with the complete filter preserved. Keep selection, blend order,
   valid light falloff and material response. Build on the existing clustered
   lights, cached static shadows and moving-caster composition. Measure complete
   draws and frames, since a faster isolated shader need not improve the frame.
3. Implement the color directional lightmap representation owned by RFC 0008,
   proving normal-map and colored-bounce behavior in `render_lab`. Replace the
   existing approximation for the migrated format while preserving old readers.
4. Package compressed, GPU-ready lightmaps and reflection data, with profile
   capability checks and measured image error. Separate compatibility/tool
   outputs from the layers the active runtime uploads. Preserve sun visibility
   and other semantic channels when changing formats; HDR compression alone
   does not carry every current RGBA channel.
5. Preserve PVS-aware meshlet bounds, model instances and authored LOD semantics
   in compiled data. Remove repeated conversion and unchanged resource work
   where measurements show a cost. Keep GPU completion and invalidation owned
   by the existing device, resource and graph contracts.
6. Complete RFC 0012's normal-variance roughness mips, alpha-coverage mips and
   alpha-to-coverage, judged through camera movement as well as still images.

The 768 MiB example motivates storage work; it does not prove that memory
bandwidth is the present frame bottleneck. Record GPU pass times, CPU recording,
uploads, residency and complete-frame timing before attributing a speedup.

## Authoring and iteration

New maps retain editable USD as their source. Native USD compiler slices
already exist on Linux; the full editor workflow and shared build graph remain
open. VMF and compiled-BSP relights remain supported inputs to the same lighting
pipeline, preserving their declared gameplay and content semantics.

Hammer and the game use the same core lighting and material definitions.
The editor shows the last completed bake and clearly marks invalidated regions
while a build runs. Any unbaked direct-light feedback is labelled incomplete;
it is not another quality profile and cannot be published as a completed bake.

Use RFC 0015's one content build graph. A light or material change rebuilds
all outputs it can affect, including indirect transport and reflection captures.
Local edits must not be assumed to have only local GI consequences. An entity
metadata edit with no rendering, collision or visibility dependency reuses the
corresponding unchanged outputs. Stable chart identities and dependency hashes
enable reuse; cancellation and failure retain the previous published package.

Measure no-op builds, entity-only edits, material edits, light edits and geometry
edits. Optimize actual expensive stages before adding persistent baker processes
or another execution framework.

## Acceptance

For this user-directed implementation, validate invariants at production
boundaries and on the native runtime, without oracle or reference-image
comparisons. The production profile disables the reference and camera-image
gates. Existing independent suites remain available for unrelated work.

- Reject malformed compiled data, nonfinite values, invalid indices, missing
  candidate coverage and partial publication after failure.
- Require valid directional pages, declared sample counts, denoising, authored
  material channels, reflection placement coverage and compiled candidate data.
- Check finite nonnegative radiance, valid selected ranks and normalized
  nonnegative blend weights in the actual runtime shader, including boundaries
  and fallback regions. Do not truncate a contributing light or probe.
- Preserve relight gameplay identity, authored roles, collision, visibility,
  entities and save/reopen behavior.
- Measure complete gameplay/render cohorts under the existing High frame
  budget, including turns, nested views, moving casters, reload and memory pressure.

Separate build completion from qualification. A successfully built candidate
may be inspected explicitly, with failed or unavailable gates attached to its
receipt. It cannot replace the qualified package or acquire a passing label
because it rendered a frame. No hidden second preset is created for candidates.

## Implementation priorities

1. **Precompute static work for the captured bottlenecks.** Trace the screenshot's
   scene, including shadow receiver cost. Prove static visibility classification
   on one fixed light and receiver cohort; compile conservative light/probe
   candidates and reusable caster inputs as their measured costs justify. Pair
   these with the runtime consumers and preparation/binding fixes. Use runtime
   invariants and complete-frame measurements, keeping one quality profile.
   Follow the engine comparison's receiver and coherent-execution priorities;
   compiled-data improvements and runtime shader improvements are complementary
   parts of reaching the frame floor, not independent claims of 120 FPS.
2. **Prove the complete lighting result.** Use that profile in relight, authoring,
   lab and game. Fix material, directional-light and probe errors, and implement
   compact lighting storage. Run negative controls and complete-frame timings
   before calling the result qualified. Performance work does not wait for every
   quality feature, and performance success does not waive a quality failure.
3. **Scale to representative maps.** Add multiple lightmap pages and local probe
   grids where the existing fixed layouts fail; complete incremental authoring
   through the shared graph. Requalify the same profile on large maps and all
   required frame cohorts before promotion.

This sequence is bounded by the existing roadmap prerequisites and render
priority. Profile selection is a single fixed policy throughout. Quality and
performance evidence determine when its output is qualified.

## Owners and tracking

This proposal selects quality policy and implementation priorities. Domain semantics stay with
[RFC 0007](0007-physically-based-lighting-pipeline.md) for the baker,
[RFC 0008](0008-canonical-world-data-and-runtime-formats.md) for compiled formats,
[RFC 0009](0009-usd-native-map-authoring.md) for authoring,
[RFC 0012](0012-antialiasing-msaa-specular-alpha-coverage.md) for filtering,
[RFC 0015](0015-asset-identity-content-build-graph.md) for builds and assets,
and [RFC 0016](0016-render-core.md) for rendering and its binding rules.
Upon implementation, executable settings live only in their existing owners;
this dated proposal is retained as rationale, not another configuration source.

Affected existing work: R48/R49 baker and lighting, R50 reflections, R54/R55/R56
compiled resources, R57 incremental builds, R59/R60 authoring, R65/R66 filtering,
and R95/R96/R91 complete rendering. No implementation row advances on the
strength of this document. Verification for this change is documentation review
and local link/whitespace checks; no new runtime or platform evidence is claimed.
