# One map quality profile for Source 2 quality and performance

Status: Proposal, 2026-10-02. The user selected one production profile for
now. The settings and implementation sequence below are recommendations;
this document does not change executable profiles or certify a build.

Use one `source2` quality profile for new maps, legacy relights, Hammer
builds and content tools. It should produce consistent materials, directional
baked indirect light, reflections and runtime direct lighting, with the
complete game image meeting the existing desktop High performance gate.
Faster iteration should come from reusing unchanged work and rebuilding
dependencies, rather than selecting a lower quality preset.

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

## Proposed production settings

These are one set of initial values to validate, not measured optimal values.
They replace the production preset choices together. Changes prompted by
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

1. Implement the color directional lightmap representation owned by RFC 0008,
   proving normal-map and colored-bounce behavior in `render_lab`. Replace the
   existing approximation for the migrated format while preserving old readers.
2. Package compressed, GPU-ready lightmaps and reflection data, with profile
   capability checks and measured image error. Separate compatibility/tool
   outputs from the layers the active runtime uploads. Preserve sun visibility
   and other semantic channels when changing formats; HDR compression alone
   does not carry every current RGBA channel.
3. Add conservative spatial candidates for reflection selection and improve
   light/caster bounds. Preserve selection, blend order, valid light falloff
   and the complete shadow filter. Build on the existing clustered lights,
   cached static shadows and moving-caster composition.
4. Preserve PVS-aware meshlet bounds, model instances and authored LOD semantics
   in compiled data. Remove repeated conversion and unchanged resource work
   where measurements show a cost. Keep GPU completion and invalidation owned
   by the existing device, resource and graph contracts.
5. Complete RFC 0012's normal-variance roughness mips, alpha-coverage mips and
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

Use the existing harness families and owning comparators. The single profile
must require the following evidence rather than allow a map to set a gate null:

- Bake and runtime agreement on materials, visible emitters, directional indirect
  light, direct-light ownership, probe visibility, reflections and shadows.
  Include moving doors and objects, grazing metal, thin walls and glass.
- Chart seams, coverage, dark gradients and denoise detail, with seeded defects
  detected. The existing p99 noise statistic remains recorded under the user's
  prior decision; it is not repurposed into a misleading universal sample gate.
- Reflection coverage and fit, with the current audit limits as the starting
  authority. A count limit is not a quality pass. Map-specific threshold
  overrides disappear; any comparator change is reviewed centrally with fixtures.
- Matching lab and game cameras, linear-light term comparisons and final images.
  Supply a reviewed camera set per map; absent required images mean unverified
  acceptance. Keep reference-render settings in the test harness, outside the
  production profile selector.
- Complete gameplay/render cohorts under the existing High frame budget,
  including turns, nested views, moving casters, reload and memory pressure.
- Legacy gameplay identity for relights, and authored role, collision, visibility,
  entity and save/reopen checks for native USD maps.

Separate build completion from qualification. A successfully built candidate
may be inspected explicitly, with failed or unavailable gates attached to its
receipt. It cannot replace the qualified package or acquire a passing label
because it rendered a frame. No hidden second preset is created for candidates.

## Migration and bounded delivery

1. **Consolidate selection.** Inventory callers and per-map overrides. Update
   `source2.json`, point every production frontend at it, reject quality overrides,
   and make runtime qualification reference the existing High owner. Old preset
   names fail with migration guidance; do not silently reinterpret them.
2. **Retire competing presets.** Migrate every named caller, then delete production
   `legacy-relight`, `portal2-chamber`, `preview`, `source2-max` and their fast/preview
   variants in the same change. Move fixture-only profiles out of the production
   selector, preserving their test semantics. Keep historical receipts immutable.
3. **Prove one complete chamber.** Use the same profile in relight, authoring,
   lab and game. Fix material, directional-light and probe errors before calling
   it qualified. Run the pre-existing negative controls as well as positive cases.
4. **Make that image cheaper.** Implement compact lighting storage and measured
   shadow/light/probe execution improvements. Preserve the full image and compare
   complete-frame timings on the declared device.
5. **Scale to representative maps.** Add multiple lightmap pages and local probe
   grids where the existing fixed layouts fail; complete incremental authoring
   through the shared graph. Requalify the same profile on large maps and all
   required frame cohorts before promotion.

This sequence is bounded by the existing roadmap prerequisites and render
priority. It does not require waiting for every planned format before removing
preset ambiguity, and it does not declare the consolidated preset finished
before its quality and performance evidence passes.

## Owners and tracking

This proposal selects policy and migration scope. Domain semantics stay with
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
