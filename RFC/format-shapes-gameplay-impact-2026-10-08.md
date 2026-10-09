# Format shapes in the engine and games: gameplay impact and migration, 2026-10-08

User question (2026-10-08): "for this, how would portal gameplay be affected:
Format structs used (mstudio*_t, BSP lumps, VTF/PHY headers) │ 271 │ 246 │
1,601", then "write all of this down".

The rule is [RFC 0027's](0027-product-pipeline-lowering-streaming-kiln.md#formats-stay-in-their-libraries-user-direction-2026-10-08)
"formats stay in their libraries". It is checked by the archlint CAP012
ratchet `format-shapes-outside-owners` in
[`architecture/structure.json`](../architecture/structure.json), with
`engine/` 271, `game/` 246 and 1,601 in all product code at 2026-10-08. The
[direction audit](direction-audit-2026-10-08.md#formats-outside-their-owners)
records how the rule came about. This document records what those uses do,
what moving them could break in Portal, and how to move them without changing
gameplay.

## Short answer

Done correctly, Portal gameplay does not change. The migration changes
*how* the engine and games read model and map data: through runtime models
instead of the on-disk records (`mstudio*_t`, `d*_t` lumps). It does not
change *what* they read. Nothing is affected yet: the ratchet only stops new
uses. Each move is a behavior-preserving change, gated by an equivalence
oracle and the gameplay suites listed below.

## What the uses are

Counts are from the ratchet's patterns over code with comments and strings
removed, at `8e7094447`.

### `game/` (246)

All are studio model records. No BSP lump, VTF header or PHY header is named
in `game/`.

| Record | Uses | What it carries |
| --- | --- | --- |
| `studiohdr_t` | 43 | the model header (contents, surface property, hull, counts) |
| `mstudioseqdesc_t` | 38 | sequences: bounds, activity, events, flags |
| `mstudiohitboxset_t`, `mstudiobbox_t` | 35, 33 | hitbox sets and boxes (bone, bounds, group) |
| `mstudiobone_t` | 18 | bones: parent, surface property, flags |
| `mstudioflexcontroller_t` | 15 | facial flex controllers |
| `mstudioevent_t` | 12 | animation events |
| `mstudiobodyparts_t` | 9 | body groups |
| attachments, pose parameters, anim descriptions, vertices, others | about 40 | |

By owner:

| Group | Uses | Files |
| --- | --- | --- |
| Shared Source game code | 150 | `baseanimating.cpp` (server), `c_baseanimating.cpp` (client), `animation.cpp`, `studio_shared.cpp`, `baseflex.cpp`/`c_baseflex.cpp`, `bone_merge_cache.cpp`, `ragdoll.cpp`, `sceneentity.cpp`, ... |
| Visual only | 72 | `emissive_area_lights.cpp` (27), `particlesystemquery.cpp` (12), `c_entitydissolve.*` (10), `c_weapon_physcannon.cpp` (HL2 and Portal, 8), `view_beams.cpp`, `c_entityparticletrail.cpp`, `fx.cpp`, `dynamic_occluders.cpp`, `basemodel_panel.cpp` |
| Other games' code (HL2, Counter-Strike) | 13 | `hl2_player.cpp`, `c_cs_player.cpp`, ... |
| Portal's own code | 11 | five files, below |

The shared group is code Portal runs: Portal is built on these classes.

### `engine/` (271)

About 200 are BSP structures and about 70 are studio records.

| Record | Uses | Main files |
| --- | --- | --- |
| `dworldlight_t` | 89 | `lightcache.cpp`: the on-disk light record is also the runtime light |
| `studiohdr_t`, `mstudiomodel_t`, `mstudiobodyparts_t`, `vertexFileHeader_t` | 79 | `l_studio.cpp`, `ModelInfo.cpp` |
| `ddispinfo_t`, `dareaportal_t`, `dface_t`, `texinfo_t`, `dbrushside_t`, `dleaf_t`, `dnode_t`, `dvertex_t`, `dedge_t`, `dmodel_t`, `darea_t`, `doccluderdata_t`, `dplane_t` | about 100 | `cmodel_bsp.cpp` (collision model), `modelloader.cpp` (world), `disp_mapload.cpp`, `r_areaportal.cpp`, `staticpropmgr.cpp` |

`ModelInfo.cpp` is the conduit to the games: `modelinfo->GetStudiomodel()`
returns the raw `studiohdr_t`, and `CStudioHdr` (declared in `studio.h`
itself) hands back raw `mstudio*` pointers. Nearly every game-side use starts
from one of the two.

## Where Portal gameplay depends on it

### Portal's own code (11 uses)

| Site | What it decides |
| --- | --- |
| `game/shared/portal/PortalSimulation.cpp:553`, `game/shared/portal2/portalsimulation.cpp:936` | An animating entity's extent from its hitboxes at their current bone positions. This decides what the portal simulation clones and collides through a portal's hole |
| `PortalSimulation.cpp:2030`, `portalsimulation.cpp:3506` | A static prop's trace `contents` and surface property, read from `studiohdr_t`. This decides what a trace through the portal environment hits and which material it reports |
| `game/shared/portal2/portal_player_shared.cpp:452` | For a trace that hits the player, the surface property of the hit hitbox's bone (`SURF_HITBOX`, `"**studio**"`). This drives impact sound and decal response |
| `game/shared/portal/weapon_portalbase.cpp:324`, `game/shared/portal2/weapon_portalbase.cpp:290` | The portal gun's bounding box, widened by its current sequence's bounds |

### Shared code Portal runs

- **Animation** (`baseanimating.cpp`, `animation.cpp`, `c_baseanimating.cpp`):
  sequence lookup by activity, sequence duration, cycle and looping, events
  fired at authored cycles, pose parameters, bone setup for traces,
  attachments. Turret aim (pose parameters), turret firing (sequences and
  events), the portal gun's muzzle attachment and animation-event sounds go
  through here.
- **Hitbox traces**: bone-space hitboxes for every trace against a model.
- **Ragdolls**: bone and constraint mapping (the collision data itself is
  PHY, read by the physics provider, a declared consumer).
- **Choreography** (`sceneentity.cpp`, `c_sceneentity.cpp`): GLaDOS and
  Wheatley scenes drive flex controllers and gestures.

### Engine code Portal runs

- **Collision** (`cmodel_bsp.cpp`, `disp_mapload.cpp`): the collision BSP
  built from lumps. Every trace, player movement, portal placement and every
  physics query depends on it. This is the highest-risk cohort.
- **Area portals** (`r_areaportal.cpp`, `dareaportal_t`): visibility through
  doors and chamber transitions.
- **Static props** (`staticpropmgr.cpp`): prop collision and the props the
  portal simulation reads.
- **World lights** (`lightcache.cpp`): lighting only, not gameplay.

## What could break, and why

1. **Index identity.** Sequence numbers, hitbox sets, bones and attachments
   are referred to by index across process and time boundaries:
   - networking: `SendPropInt( SENDINFO(m_nSequence), ...)` and
     `SendPropInt( SENDINFO(m_nHitboxSet), ...)` in `baseanimating.cpp`
     (lines 248 and 254), received in `c_baseanimating.cpp` (lines 173 and
     178), with `m_nSequence` predicted;
   - save games and map keys: `DEFINE_KEYFIELD( m_nHitboxSet, ...)` and
     `DEFINE_KEYFIELD( m_nSequence, ...)` (`baseanimating.cpp` lines 171
     and 172);
   - demos, which record the network stream.

   A runtime model must keep the same indices and order as the compiled
   model, or saves, demos and multiplayer (Portal 2 co-op) desynchronize.
2. **Exact values.** Hitbox order and float bounds feed portal cloning and
   traces. A model API that re-derives bounds, merges hitboxes or reorders
   them could change which objects are cloned or what a trace hits at the
   edge of a portal. The runtime model returns the authored values bit for
   bit.
3. **Event timing.** Events fire at authored cycle points. Turret behavior,
   sounds and scene timing depend on them, so the runtime model keeps the
   event list and cycles unchanged.
4. **Collision.** Rebuilding the collision model from a translator must give
   the same planes, brushes, leaves and displacement collision. A
   difference changes traces everywhere.
5. **Mods.** Not a gameplay risk. Mod game DLLs built against `studio.h` and
   `bspfile.h` keep working, because the headers stay as the frozen ABI. The
   ratchet constrains first-party code only.

## How to migrate without changing gameplay

Cohort by cohort, behavior-preserving, under the ratchet:

1. **Runtime model interfaces.** A model interface (skeleton, sequences with
   events and bounds, hitbox sets, attachments, pose parameters, body groups,
   contents and surface property, flex controllers) in its own header,
   implemented over today's compiled model, with indices identical to the
   compiled model's. A world interface for collision and area portals
   implemented over the map container (RFC 0008's `world.map-container`
   reader is the seam, R53). These are the runtime representations RFC 0027
   asks for, owned by the modern model asset path (R62) and the resolver
   (R83).
2. **Equivalence oracle before any caller moves.** For every model in the
   Portal and Portal 2 content, the interface must return values equal to
   the raw records for every hitbox, bone, sequence, event, attachment, pose
   parameter, body group, contents and surface property, with indices
   identical. For collision: the collision model built through the world
   interface is byte-equal to today's on the Portal and Portal 2 map corpus,
   and a trace corpus gives identical results. A seeded wrong index, a
   reordered hitbox and a perturbed bound must each fail the oracle.
3. **Move callers in cohorts.** Portal's 11 sites first (small and
   gameplay-critical), then the shared animation code, then the engine's
   `ModelInfo`/`CStudioHdr` conduit (retired last, when nothing first-party
   asks for raw records), then collision. Each cohort records its decrease in
   the ratchet in the same change.
4. **Gameplay suites on every cohort.** Installed:
   - `tools/quality/portal_view_trace.py`: the `qa_portal_walk` map and the
     PVIEW view-continuity check through portals;
   - `tools/quality/portal2_storybeats.py`: the input-driven walkthroughs
     `corpus.portal2.storybeats-wakeup` and `corpus.portal2.storybeats-laser-intro`,
     with their seeded comparator controls;
   - `tools/quality/portal2_retail.py`: comparisons against the retail game
     (including the laser fixtures);
   - `tools/quality/portal2_demo_suite.py`: demo playback
     (`corpus.portal2.demos.*`), which also checks the network stream's
     indices;
   - `tools/quality/physics_conformance.py` (`physics.conformance`) on IVP
     and Box3D, and `tools/quality/portal2_physics.py`;
   - `tools/quality/portal_boot.py` boots, including level transitions.

   To add before the first cohort moves, because nothing installed checks
   them: a save/load round trip that compares entity animation state
   (sequence, cycle, hitbox set) before and after, and a turret scenario
   (aim and fire through pose parameters and events). A Portal 2 co-op
   session check follows R93; until then co-op index identity is covered
   only by the demo suite's network stream.
5. **Visual-only uses go to their owners**, not to the game API. The 72
   effect and lighting uses move to their owners:
   - `emissive_area_lights.cpp` reads raw VVD vertex data on the client;
     that is render work for a render translator under RFC 0016's
     [anti-corruption boundary](0016-render-core.md#the-anti-corruption-boundary-user-decision-2026-10-08);
   - the dissolve, particle and beam effects take attachments and hitboxes
     from the runtime model.
6. **World lights.** `dworldlight_t` as the runtime light is replaced by a
   runtime light record (RFC 0011's light set already exists); lighting
   only, judged by images.

## Not covered here

- The other 1,084 uses outside `engine/` and `game/` (material system,
  studiorender, datacache, and others), which move with their own rows.
- Format *headers* (`format-includes-outside-owners`), which shrink as the
  uses above move.
- No code has moved. This document records the plan and its risks; no gate
  is claimed.

## Corrections to the first answer in chat

- Portal's own uses are 11 in five files, not about 10 in three: both
  `weapon_portalbase.cpp` files use sequence bounds.
- `portal_player_shared.cpp:452` sets a hit's surface property (impact
  response), not damage or hit groups.
- The shared group is 150 of 246 (61 %), not "about two thirds"; 72 are
  visual only.
