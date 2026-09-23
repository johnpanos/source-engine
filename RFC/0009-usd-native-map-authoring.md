# RFC 0009: USD-Native Map Authoring and Semantic Object Roles

- Status: Proposed (2026-09-23); no implementation gate complete
- Date: 2026-09-23
- Scope: The editable map source, world-geometry and placed-model roles, native
  USD compilation, editor persistence, and legacy VMF migration
- Compiled data and runtime: [RFC 0008](0008-canonical-world-data-and-runtime-formats.md)
  owns the compiled World Stage, BSP2, lightmap charts, render lumps, and KTX2
- Editor: [RFC 0002](0002-hammer-responsibility-factorization.md) owns document
  operations, history, persistence orchestration, and GTK presentation
- Tools and baker: [RFC 0007](0007-physically-based-lighting-pipeline.md)
- Verification: [RFC 0005](0005-quality-and-correctness-harnesses.md)

## Decision and boundary

New maps are **authored in OpenUSD, without a VMF source document or a VMF
round trip**. The authoring stage is the saved, editable authority. A map compiler
consumes a validated snapshot of that stage and produces RFC 0008's compiled
World Stage and BSP2 gameplay/render data. OpenUSD starts in desktop editor and
host-tool profiles. RFC 0008 F11 adds an opt-in development-runtime USD loader
and evaluates mobile separately; installed clients and servers continue to
read compiled packages.

This is a new input and compiler boundary, not a request to change the current
VMF-derived F2 prototype into an editor file. Existing VMF maps and their
compile path remain supported as compatibility inputs. RFC 0008's compiled
stage, baker, packer, and BSP2 reader must accept a declared source provenance
without assuming that every stage originated from BSP faces or VMF keys.

Source 2 is a **workflow analogy**, not a format dependency: Hammer edits map
meshes separately from placed model entities, but Source 2 uses `.vmap` and
`.vmdl`, not USD. We adopt the semantic distinction in an OpenUSD map profile.
See Valve's [mesh editing guide](https://www.dota2.com.cn/wiki/Dota_2_Workshop_Tools/Level_Design/Basic_Construction/Mesh_Editing_1.htm)
and [tools announcement](https://store.steampowered.com/news/posts/?appgroupname=Half-Life:+Alyx&appids=546560&enddate=1592349047&feed=steam_community_announcements).

## Observed starting point

- `utils/vbsp/map.cpp` loads VMF/MAP; `utils/vbsp/writebsp.cpp` writes BSP
  before the opt-in `vbsp2` adapter emits `<map>.geometry.usda`.
- The current `SourceWorldAPI` schema (`utils/worldstage/schema.usda`) describes
  **compiled** BSP face IDs, atlas charts, serialized entities, and preview
  lights. It does not describe an editable map or native model placement.
- `tools/quality/usd_worldmesh_pack.py` can pack imported USD triangles into a
  WMSH preview, but takes a precompiled BSP for collision/leaves and places
  every imported meshlet in every leaf. It is not a map compiler.
- `vvis` and `vrad` read compiled BSP data. `IMapContainer` and the client/server
  already read legacy BSP or BSP2, so a complete BSP2 producer can reuse the
  runtime seam without loading USD in the game.
- RFC 0002's first editor slices intentionally exercise VMF. They establish
  document and tool behavior; they do not decide the final source format.

These are implementation observations, not claims that native USD authoring is
installed.

## Authored object roles

An authored USD prim's **role** determines its editing and compile contract.
`UsdGeomMesh` alone is only geometry; a path name or material name cannot imply
gameplay behavior. The versioned Source authoring schema owns the role.

| Role | Editing contract | Compile contract |
| --- | --- | --- |
| World structure | Brush-like block/CSG and face/edge/vertex operations on an owned solid or mesh; stable surface identities, material binding, and explicit collision/visibility policy | Produces world render surfaces, world collision and spatial/portal inputs; may affect PVS and baked lighting |
| `prop_static` placement | Transform and instance properties of a referenced model asset; editing the model opens its asset, not a private copy in the map | Fixed placement, declared collision/shadow/bake behavior and static-prop payload; does not silently become sealing world structure |
| `prop_dynamic` placement | Model reference, transform, entity keys/I/O, animation and parenting where supported | A runtime entity with movable/animated model behavior; not a physics body merely because it is dynamic |
| `prop_physics` placement | Model reference plus physical material, collision and mass/constraint properties | A runtime physics entity with validated collision data |
| Geometric entity | Map-owned geometry plus explicit entity class and transform | Trigger, moving brush-like object, areaportal, or other declared entity-specific output; never inferred from a mesh alone |

`prop_static`, `prop_dynamic`, and `prop_physics` are distinct. The first native
vertical slice need not implement every role, but unsupported required roles
must fail validation before publication. Model references initially may point
to existing MDL assets; RFC 0008 F9 adds a modern model asset path without
blocking the first playable USD map.

The editor can offer a block tool for a world solid while persisting it in USD.
Whether a given solid stores parametric planes/CSG operations or editable mesh
topology is an explicit schema capability. Flattening a parametric solid to a
triangle mesh must not discard its promised editing operations on save/reopen.

The authoring goal includes responsive, in-context work: create and reshape
world solids or meshes, place and replace model references, edit materials and
lights, inspect collision/visibility roles, and see the resulting scene in a
material-and-lighting viewport. Selection, snapping, pivots, surface UVs,
undo/redo, and asset browsing must retain their meaning across those roles.
Compile progress and errors point back to stable authored objects; a changed
light, material, or prop can be previewed without rebuilding unrelated world
geometry. RFC 0002 owns the interaction rules; RFC 0008 F10/F11 own runtime
visual parity and direct USD development preview.

## Authoring and compiled stages

```text
authored map.usda + referenced models/materials/textures
          │  validate composition, roles, assets and geometry
          ▼
native USD map compiler ──► compiled geometry.usdc + gameplay/spatial data
                                      │
                           visibility / light baker / packer
                                      ▼
                              installed map.bsp2

legacy VMF ──► existing vbsp2 path ──► same compiled-stage contract
```

The authoring stage owns editable objects, stable object/surface identity,
entity properties and connections, asset references, layer composition, and
editor metadata that must survive save/reopen. Standard USD geometry, transform,
material binding, lights, references, and layers are used where their semantics
fit. Source-specific meaning is defined once in generated, versioned schema
APIs. Exact prim paths and API class names are selected with the first schema
fixture; this RFC fixes their behavior, not untested spellings.

The compiled stage owns triangulation, generated world-mesh sections, chart
layout, compiled entity records, and provenance back to authored object/surface
IDs. Those are derived data, never editor state. BSP face IDs are valid
provenance for the VMF path only; native USD content needs stable authored IDs
and a compiled triangle-to-surface mapping even when no legacy BSP face exists.
The baker and packer consume the compiled-stage contract from RFC 0008, not the
authoring schema or VMF parser. Generated lighting lives in separate layers and
must not overwrite authored geometry, lights, or model placement.

Only one authored stage is authoritative for a live map. Editor snapshots and
compile intermediates are immutable derivatives keyed by its committed revision;
they are not a second synchronized document or undo stack. A USD scene edited
externally is accepted through the same validator and detached-import policy
as an editor save. External tools may author supported geometry and materials,
but generic valid USD is not automatically a valid playable map.

## Compiler seam and compatibility

The native compiler must produce all required data for a playable map: world
render mesh, collision and traces, spatial leaves/portals and visibility,
entity records, prop placements, surface/material identity, and bake/chart
inputs. It may reuse downstream `vbsp` algorithms through a typed, in-memory
adapter when the authored geometry supports their invariants. It must not
serialize USD to a temporary VMF, run the VMF loader, or require a separately
compiled BSP as an input. A mesh-first path may need new spatial/collision
construction; reusing the BSP2 container alone does not provide those data.

The first bounded compiler profile may limit geometry to closed, validated
world solids and a small set of entities and lights. It must state that profile
and fail unsupported features. An all-visible, single-cell diagnostic mode may
help inspect rendering, but does not pass the native visibility gate. Reusing
legacy-compatible gameplay lump encodings is allowed and desirable when their
contracts hold. Legacy VMF-to-BSP2 byte-identity remains its own oracle;
USD-native maps have semantic collision, visibility, gameplay, and rendering
oracles because no VMF byte baseline exists.

VMF import creates a **new USD authoring document** only after a declared
feature/loss report and an explicit commit. Imported brushes, entities,
overlays, displacements, instances, visgroups, and unknown content must be
classified; unsupported required data fails import. Legacy VMF export is a
separate compatibility operation with a stated loss policy. Neither format is
kept as a mutable shadow of the other.

## Validation and acceptance

The schema validator runs before any compiler output is published. It checks
the resolved USD composition, schema versions, units/up axis, finite transforms,
stable unique IDs, referenced assets, model-role compatibility, material
bindings, geometry topology, entity targets/I/O, collision and visibility
inputs, and unsupported required roles. Failure leaves the previous installed
package intact. USD's `usdchecker` is necessary, but it cannot validate these
Source-specific obligations.

Shared Q-EDITOR/Q-CONTENT/Q-PRODUCT evidence includes:

1. A hand-authored USD fixture with a sealed room, material, point light,
   player spawn, and collision compiles **without a VMF or precompiled BSP**;
   the installed BSP2 runs in client and dedicated-server profiles. Collision,
   traces, spatial membership, entities, and rendered pixels have independent
   checks. A bad seal, missing model, wrong role, invalid reference, and stale
   surface ID each fail with no partial package publication.
2. Separate static, dynamic, and physics model fixtures retain their roles
   through save/reopen and compile. Static collision and bake behavior,
   dynamic movement/I/O, and physics response use their respective oracles;
   swapping role metadata in negative fixtures fails.
3. Editor sequences cover two documents, create/edit/undo/redo/save/reopen,
   external USD changes, build cancellation, and compile/run. Authored topology,
   role, asset references, and unknown/loss diagnostics survive the sequence.
4. A versioned VMF corpus imports with per-feature results and validates its
   chosen compatibility output against the legacy path. No successful import
   silently discards required gameplay or editable content.
5. Build evidence records source-layer and asset hashes, schema/tool versions,
   profile, first divergence, and reproduction commands. Light-only and
   model-placement-only changes avoid rebuilding unaffected world geometry;
   geometry changes invalidate the dependent spatial and bake products.
6. A representative authoring task covers block/mesh editing, separate prop
   placement, material and light changes, object-linked diagnostics, viewport
   preview, and playtesting. Capture interaction traces and edit-to-preview
   latency on declared editor profiles; the viewport's material/lighting
   differences from the installed runtime have named fixtures and tolerances.

The first room is a vertical slice, not the full gate. Full native authoring
requires the declared feature corpus and installed product checks on supported
profiles. Missing platform/native evidence remains unverified under AGENTS.md.

## Delivery and dependencies

| Phase | Deliverable | Exit gate |
| --- | --- | --- |
| U0 | Authoring schema, role and geometry profile, source-to-compiled ID policy, USD fixture, independent validator | Positive and deliberately bad fixtures prove role, topology, reference and composition checks; no implementation status is inferred from RFC 0008's compiled schema |
| U1 | Native USD compiler for one room, no VMF/BSP input | Client and dedicated-server installed BSP2 smoke, independent collision/visibility/entity/render checks, clean failure recovery |
| U2 | Static, dynamic, physics and geometric-entity cohorts | Each role's distinct lifecycle, model, collision, bake and gameplay contract passes; unsupported classes fail explicitly |
| U3 | Headless editor persistence and GTK workflow | USD is the saved authority; two-document edit/history/save/reopen/build, role-aware tools, object-linked diagnostics, viewport parity and edit-to-preview budgets pass without a VMF shadow |
| U4 | VMF import, feature families and incremental build | Versioned semantic corpus, loss reports, cache traces, cancellation/recovery and supported product gates pass |

U0–U2 are RFC 0009's compiler/content work (portfolio R59); U3–U4 are the
editor workflow (R60). They depend on RFC 0008's compiled-stage/container
contracts and RFC 0002's document seams, but R54's VMF-derived stage can close
without native USD authoring. RFC 0007's baker must accept either compiled-stage
producer. No existing gate is marked done by this planning decision.

## Open implementation choices

- Which world-solid capabilities are represented as parametric planes/CSG,
  editable mesh topology, or both? The initial profile must choose and test its
  save/reopen and compiler behavior.
- Which closed-world and portal construction algorithm handles arbitrary
  authored meshes while preserving collision and PVS semantics? A bounded
  brush-compatible slice can precede it without claiming mesh-first parity.
- Which model asset profiles are valid for each prop role before a separate
  model-format modernization? Missing collision or animation data must fail
  the role's required contract.
- Which authored features can be imported losslessly from VMF? The importer
  records a decision per feature and does not silently flatten editability.
