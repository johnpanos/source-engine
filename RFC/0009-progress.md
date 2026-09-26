# RFC 0009 progress: USD-native map authoring

Updated: 2026-09-25
Portfolio row: R59 (U0–U2), active: the U0, U1 and U2 slices are done on
Linux desktop, and its prerequisites are open. R60 (U3–U4) is planned.

Direction (user, 2026-09-25): Hammer and the game evolve together. The target
is a working author → compile → play loop, not legacy-editor parity. Each
slice lands with a map or gameplay use.

## U0: authoring schema, profile, ID policy and validator (slice done, 2026-09-25)

Scope: a versioned Source authoring schema profile for editable USD maps, with
these parts:

- authored roles: world structure, `prop_static`, `prop_dynamic`,
  `prop_physics` and geometric entities;
- the initial world-solid geometry profile;
- the source-to-compiled ID policy;
- a hand-authored USD room fixture;
- an independent validator, with positive and deliberately bad fixtures
  proving the role, topology, reference and composition checks.

It is not RFC 0008's compiled `SourceWorldAPI` schema, and no implementation
status is inferred from that schema. U1 (the native compiler) is not part of
this slice.

Result (implemented by a subagent and verified independently here):

- **Profile.**
  - Files: `quality/usd_authoring/source_authoring_v1.json` (roles,
    properties, tolerances, 51 error codes) and `source_authoring_v1.md`
    (rationale).
  - Encoding: typed `sourcemap:*` attributes, with no compiled schema
    plugin. Anything under RFC 0008's compiled `source:` namespace fails on
    an authoring stage.
  - Stage record: `customLayerData.sourcemap = {profile, version}` and a
    required `defaultPrim`.
- **Roles.** `world_solid`, `prop_static`, `prop_physics`, `entity_point`
  (`info_player_start`), `entity_brush` (`trigger_once`/`trigger_multiple`)
  and `light` (a `SphereLight`).
  - `prop_dynamic` is reserved for U2 and fails `role.unsupported`.
  - A missing role, a role given only by name, undeclared properties,
    instancing and time samples each fail by name.
- **Geometry profile.** A `world_solid` is one closed convex mesh, and each
  face is one brush side. U1 can therefore reuse vbsp's convex CSG,
  visibility and collision.
  - Solids are checked for planarity, manifold, closure, outward winding
    (mirroring-aware), convexity and no coplanar faces.
  - Per-face `sourcemap:surfaceIds`, per-face affine `st` (a texinfo), and
    `contents = solid` are required.
- **ID policy.** `sourcemap:id` is required and unique across the composed
  stage, and it survives rename, regrouping and save/reopen. `--previous`
  reports added, removed and moved IDs and fails an ID whose role changed.
  The validator's `objects` table is the compiler's ID input.
- **Units.** Z-up with `metersPerUnit = 0.0254` (one USD unit is one Source
  unit). Other units and Y-up are converted; mixed layer units fail.
- **Fixture.** `quality/fixtures/usd-authoring/room/`, which `usdchecker`
  passes:
  - a sealed 512×384×192 room of six solids with three Portal VMTs;
  - a player start and a trigger;
  - a `prop_static` desk and a `prop_physics` box, whose models are present
    in the Portal VPK;
  - a light.
- **Evidence.**
  - `usd_authoring_validate.py validate` on the fixture: 633 checks,
    0 failures.
  - 87 tests under the pinned OpenUSD 25.11 Python cover 66 generated bad
    variants, each producing exactly its intended code; together they cover
    every declared code.
  - The subagent reports all 74 mutants caught: each of the 51 codes
    suppressed, plus 23 logic mutants.
  - `corpus.usd-authoring.validator` matches with 95 checks. It is optional
    and skips without `SOURCE_USD_TOOLCHAIN`.
  - The quality self-tests pass, and the module skips without `pxr`.

R59 stays `active`. U1 (the native compiler: authored room → BSP2 without VMF
or a prior BSP, running in client and dedicated server) is next. The profile
leaves these decisions open:

- UsdLux to `_light` conversion;
- entity keys and I/O beyond classname;
- leak detection (U1's bad-seal fixture must fail in the compiler);
- model-file and collision requirements per prop role;
- lightmap scale and tool materials;
- how compiled outputs record authored IDs (RFC 0008);
- the editor's ID generator (U3).

## U1: native USD map compiler, first slice (slice done, 2026-09-25)

Scope: the U0 room compiles from its authored USD to a playable BSP2 with no
VMF anywhere in the path and no prior BSP, reusing vbsp's algorithms through a
typed in-memory adapter. The slice includes independent checks of the output,
negative fixtures that publish nothing, and dedicated-server, client and
native Vulkan smoke runs. It closes no R59 criterion by itself: the U2 role
cohorts, `prop_dynamic` and the editor loop remain.

### Design

```text
room.usda → validate → content → game → brushset → vbsp -authored → vvis → vrad
          → assemble (BSP2 + provenance) → check (usd_map_check.py) → publish (one rename)
```

- **USD stays in Python.** The U0 validator is the only USD reader. Its
  `objects` table now also carries, per solid, each face's outward unit
  plane, its corners (counter-clockwise seen from outside) and its affine st
  rows in Source space; each light's UsdLux inputs; and prop collision, skin
  and mass scale. The additions extend `source-authoring-validation/v1`; the
  authoring schema is unchanged.
- **Typed intermediate.** `source-authored-brushset/v1` (JSON) lists the
  entities in output order, with typed origin and angles and string keys.
  Each solid is a list of sides: plane, polygon, material, lightmap scale,
  st rows and side key. It is not VMF text, and no VMF parser reads it.
- **vbsp adapter.** `vbsp -authored <map>.json`
  (`utils/vbsp/authoredmap.cpp`) fills the same `CMapFile` entities,
  `mapbrush_t` and `side_t` records as the VMF loader, reading the JSON
  through tier1's parser. vbsp's normal CSG, BSP, portals, leak test, faces,
  static props and writer follow. The adapter checks, all fatal:
  - the plane vbsp snaps from a face's corners matches the authored plane;
  - the brush vbsp rebuilds from the planes reproduces every authored corner;
  - every material exists;
  - classes the VMF loader rewrites while loading are refused.

  In authored mode a leak or a dropped `prop_static` is an error, not a
  warning. The VMF loader and the adapter share the extracted side-contents
  rule, luxel scaling and load tail (`map.cpp`); the VMF path is otherwise
  unchanged.
- **Stages** (user direction: the job-based engine goal).
  `usd_map_compile.py` runs nine stages. Each reads only its declared inputs,
  writes only its own directory, and records its input and output SHA-256s
  and a content key in `build.json`. One ordered commit
  (`playable_maps.publish`, one rename) publishes the content root with
  `provenance.json` and `build.json`.
  - vvis and vrad run with `-threads 1`. The tools' worker threads are
    private pools, so the stages stay serial until the engine job system
    schedules them (RFC 0003). RFC 0008 F6 / R57 own the graph executor and
    the cache.
  - Two builds of the fixture are byte-identical, stage for stage.

### Decisions

The policy is in `quality/usd_authoring/source_compile_v1.json`.

- **Light** (`usdlux-sphere-to-vrad-point/v1`).
  - With `normalize` off, a SphereLight's luminous intensity is
    I = intensity × 2^exposure × diffuse × π (radius × 0.0254 m)² cd.
  - vrad's brightness B is the 0–255 lightmap value at 100 units under
    inverse-square falloff (`_quadratic_attn 1`). v1 maps 10 lux to full
    white, so B = 255 I / (2.54² × 10), times the largest colour channel.
  - The colour, normalised to that channel, is written sRGB-encoded
    (c^(1/2.2), inverting vrad's decode). The fixture lamp becomes
    `255 247.2 233.4 256.5`.
  - `normalize` and `enableColorTemperature` fail
    `compile.light-unsupported`. The radius sets the intensity only.
  - Calibration against Cycles belongs to R49/R51 and needs a new policy
    version.
- **Entity keys.** The class defaults Hammer writes from base.fgd
  (`trigger_multiple`: spawnflags 1, wait 1, StartDisabled 0). No I/O (U2).
- **Leak.** Judged by vbsp's tree. The driver also requires no `.lin` and a
  portal file. The bad-seal fixture fails `compile.leak`.
- **Model files.** `prop_physics` needs `.mdl` and `.phy`. `prop_static`
  needs both with `collision = vphysics`, and only the `.mdl` with `none`.
- **Materials and lighting.** Lightmap scale 16. Brush entities use
  `tools/toolstrigger` with a world-aligned mapping. Worldspawn's
  `skyname sky_escape_01_` builds the default cubemap; no sky is visible in
  a sealed map.
- **Texture convention.** st is in repeats with t up (UsdPreviewSurface).
  The texel row is (1 − t) × height, with texture sizes from vbsp's material
  lookup.
- **Authored ids in outputs.**
  - Entity records carry `sourcemap_id`.
  - `provenance.json` (`source-usd-map-provenance/v1`) maps static props,
    brushes and brush sides to authored ids.
  - Each output face carries its side key in the legacy `LUMP_FACEIDS`. The
    key is a compiled key, not an identity; the sidecar maps it to
    `(object id, surface id)`.
- **Previous revision.** By default the published package's provenance is
  `--previous`, so an id whose role changed fails before anything is built.

### Defects found and fixed on the way

- **Fixture.** The U0 fixture's desk stood under the floor: `lab_desk01`'s
  model origin is its top (its vertices span z −37.7 to 0). vbsp, now
  strict, rejected it as outside the map. The fixture now places it at
  z = 37.7.
- **Model versions.** vbsp and vrad rejected every pre-v49 model (Portal
  ships v44), because the header conversion no longer rewrites the version.
  `Studio_IsConvertibleVersion` in `public/studio.h` now owns the
  convertible range (44–49).
- **Static-prop records.** The tools wrote 84-byte `StaticPropLump_t`
  records under static-prop lump version 10. The engine reads a BSP v21 with
  lump v10 as 76-byte `StaticPropLumpV10_21_t` records, so every compiled map
  with static props was misread past the first prop, and its flags byte came
  from an uninitialised pad. vbsp now writes, and vrad reads, the engine's
  record, zero-initialised, which also makes the lump deterministic.
- **Recorded, not changed.** vbsp's `CompactTexinfos` points never-visible
  brush sides at another used texinfo, so hidden sides do not keep their
  authored materials (8 in the fixture). The checker compares visible sides
  only.

### Evidence

Worktree `agent-a4abfc363969fbb6b`, 2026-09-25; local evidence in its ignored
`quality-results/rfc0009-u1-20260925/`.

- **Build.** `./waf configure --tools --disable-warns -T release -o
  build-u1-tools`; the tools need no OpenUSD. The map is published as
  `run/maps/usd_room` (BSP2 sha256 `a3f066fe…`), and `./play usd_room`
  mounts it.
- **Output checks** (`usd_map_check.py`, independent of the compiler): 6,592
  pass on the fixture:
  - the container, 4 entity records and the static prop in the engine's
    layout;
  - 7 brushes, and 2,880 open-surface samples with material, st and
    side-key provenance;
  - 36 rays and a player-hull landing, and vphysics models;
  - 4 clusters with 300 line-of-sight pairs visible;
  - lightmaps and the compiled point light.
- **Variants.** A divided room (a full-height wall) has 6 clusters and 2
  sealed regions: 113 sampled pairs across the seal are hidden, and 187
  line-of-sight pairs are visible. A chamfered variant (a 7-sided floor)
  passes, with 34 void points outside the chamfer filled solid.
- **Tests.** `test_usd_map_compile` has 31 tests: positives, 9 negative
  fixtures, 7 compiler mutants, and policy and runner unit tests.
  `test_usd_authoring` has 90 (3 new: faces, mirrored faces, light inputs).
- **Conformance.** `corpus.usd-authoring.validator` (98 checks) and
  `corpus.usd-map.compiler` (39 checks) match. Both are optional and skip
  without `SOURCE_USD_TOOLCHAIN`; the compiler row also needs
  `SOURCE_USD_MAP_TOOLS` and Portal content.
- **Negative fixtures.** Each fails with its code while the published store
  stays byte-identical:
  - a bad seal (`compile.leak`; the validator passes it);
  - a missing model, a physics model without `.phy` (the cake) and a
    missing material;
  - a reserved role (`role.unsupported`) and a role changed against the
    published package (`id.role-changed`);
  - an unresolved prefab (`composition.error`);
  - a stale surface id after a clip (`surface.ids-duplicate`);
  - an unsupported light input.
- **Mutants.** All caught, each by the intended layer:
  - 7 compiler mutants in the suite: swapped st axes, a dropped face,
    shuffled provenance, a doubled light, a moved entity, skipped vvis and
    a skipped driver leak test;
  - 8 C++ mutants of vbsp, built from patched sources: no leak error, no
    winding check, V axis not flipped, no side key, no entity id, reversed
    plane winding, a static prop dropped silently, and the old static-prop
    layout.
- **Dedicated server** (`build-r03-dedicated`, staged privately, 62 checks).
  It loads the file as BSP2 and spawns `info_player_start`,
  `trigger_multiple` and `prop_physics`. All 24 engine `CM_BoxTrace` probes
  agree with the Python traces within 1.5e-5 of the ray.
- **Client** (`build-r03-portal-native`, null renderer, SDL offscreen, no
  window, 63 checks). The player becomes active, with the same probes and
  spawns.
- **Native Vulkan frame** (headless, SDL offscreen, no window). It shows the
  textured, lit room with the cube and the desk. `usd_map_pixels.py` labels
  8-pixel blocks by the authored material each view ray meets:
  - the colour change across predicted material boundaries is 5.32 times
    the change within a material;
  - five wrong cameras (yaw ±10°, pitch +8°, eye +48, FOV −15) score at most
    2.54 (10 checks);
  - on a frame rendered at the default FOV 75 the oracle fails, and its
    FOV −15 control scores 5.20.
- **Repository checks.** The quality self-tests' fast tier (1,021) passes;
  archlint check and baseline pass; stylelint is clean on the changed C/C++
  lines. The loader inventory still reports its 13 known uninstrumented
  sites (R07); this slice adds none.

### Not done or unverified

- The output checks read the legacy lumps inside the BSP2. USD maps have no
  compiled World Stage (RFC 0008 schema) or WMSH/LMAP render payload yet;
  the frame uses the legacy BSP face path.
- vrad bakes LDR only; neither HDR lighting nor the Cycles baker is used.
- Runtime collision with the static prop and the trigger's touch behaviour
  are not probed (`map_container_probe` traces the world only).
- Linux desktop only: no Android or Apple run and no hosted CI lane. The
  compile tools are this worktree's `--tools` build; the pinned
  `pbrt-map-tools` install predates `-authored`.
- The pixel oracle is geometric (material boundaries). It has no reference
  image and no lighting tolerance.

Reproduce, with the tools and content paths above:

```sh
WAFLOCK=.lock-waf-u1-tools ./waf configure --tools --disable-warns -T release \
  -o build-u1-tools --prefix=$PWD/build-u1-tools/install
WAFLOCK=.lock-waf-u1-tools ./waf install -j16
PYTHONPATH=build/toolchains/openusd-25.11/lib/python /usr/bin/python3.12 \
  tools/quality/usd_map_compile.py compile quality/fixtures/usd-authoring/room/room.usda \
  --map usd_room --tools build-u1-tools/install --runtime run/runtime
SOURCE_USD_TOOLCHAIN=build/toolchains/pbrt-map-toolchain.json \
SOURCE_USD_MAP_TOOLS=$PWD/build-u1-tools/install \
  python3 tools/quality/conformance.py check --rfc 0009 --out <dir>
python3 tools/quality/usd_map_runtime.py --map-file run/maps/usd_room/maps/usd_room.bsp \
  --report <build>/stages/validate/authoring-report.json --runtime run/runtime \
  --build build-r03-dedicated --product dedicated --out <dir>
python3 tools/quality/portal_boot.py --runtime run/runtime --build build-r03-portal-native \
  --content-root <build>/stages/assemble/content --map usd_room --renderer native-vulkan \
  --headless --console-command "fov_desired 90" --console-command "cmd setpos -192 -128 0" \
  --console-command "cmd setang 10 20 0" --capture-wait 300 --out <dir>
python3 tools/quality/usd_map_pixels.py check <dir>/runtime/portal/screenshots/usd_room0000.tga \
  --report <report> --origin -192 -128 0 --angles 10 20 0 --fov 90
```

**Merged into the shared tree (2026-09-25).** The U1 diff (`e6544eb3..145a9748`
on branch `worktree-agent-a4abfc363969fbb6b`) was applied to the main working
tree without a commit. AGENTS.md and the manifest were merged by hand, keeping
exactly one `corpus.usd-authoring.validator` row. Verified here:

- `corpus.usd-map.compiler`: 39 checks. The U0 room compiles through
  `vbsp -authored`, vvis and vrad to a published BSP2, checked
  independently. It needs `SOURCE_USD_MAP_TOOLS=build-r03-tools/install`.
- `corpus.usd-authoring.validator`: 98 checks.
- Build group: `tools`, `dedicated` (gcc and clang), `portal-features`,
  `portal-dxvk` and `portal-native-clang` pass.
- The `portal-native` (gcc) and `hl2` trees fail only to link
  `vphysics_conformance`, on `RunShapeInertiaContract()`. That comes from a
  concurrent session's in-progress RFC 0013 shape-inertia work, not from U1.
- Static audit, `conformance.gcc`/`.clang` and `arch.compile-deps`: 0
  deviations.


## U2: prop role cohorts and geometric entities (slice done, 2026-09-25)

Scope, bounded:

- `prop_dynamic` becomes supported, with a declared key set;
  unsupported keys and parenting fail explicitly.
- Each role's contract gets its own oracle:
  - `prop_static`: the static-prop lump, collision per its declared property,
    and a declared bake policy;
  - `prop_dynamic`: a movable, non-physics entity;
  - `prop_physics`: validated collision, and it falls and rests at runtime.
- The trigger fires at runtime; one moving brush entity is supported if it
  fits the profile.
- Negative fixtures: swapped role metadata, `prop_physics` without collision,
  physics-only keys on a dynamic prop, and unsupported keys. None publishes.
- Runtime evidence headless only (dedicated server or windowless client).
- Stages keep U1's job-ready shape.

It closes no R59 criterion by itself: R59's prerequisites (R05, R48, R53,
R54) are open, and the evidence is Linux desktop only.

### Decisions

- **Profile version 2** (`source_authoring_v1.json`, one file). New roles,
  classes, properties and class values carry `"since": 2`. A stage that
  declares version 1 validates exactly as before, and v2 features in it fail
  as unsupported. The room fixture stays v1; the new role fixture declares v2.
- **Supported keys.**
  - `prop_dynamic`: `id`, `model`, `collision` (`vphysics` or `none`, required;
    `bbox` reserved), optional `skin` and `defaultAnimation`. `parent` is
    declared reserved, so parenting fails `property.unsupported`.
  - Triggers: optional `touchFilter` (`clients`, `physics`) and
    `connections`.
  - `func_movelinear` (the moving brush; it fits the brush-entity profile):
    authored surfaces like a world solid, `moveDelta` (a displacement in the
    prim's space) and `moveSeconds` (travel time, so the speed needs no unit
    conversion). `func_door` and other classes fail
    `entity.class-unsupported`.
  - A key that another role or class declares fails `property.misplaced` and
    names its owners (for example `massScale` on a `prop_dynamic`). Keys no
    one declares still fail `property.unknown`.
- **Connections.** `string[] sourcemap:connections`, one
  `"Output target-id Input [parameter]"` per entry. The target is an id, never
  a name, so renames and regroups keep it. The profile's `io` table declares
  each trigger class's outputs and each target's inputs (`prop_dynamic`
  `SetAnimation`/`SetDefaultAnimation` with a sequence, `func_movelinear`
  `Open`/`Close`). New codes: `io.invalid`, `io.target-missing`,
  `io.input-unsupported`.
- **Compile policy version 2** (`source_compile_v1.json`):
  - `targetname` = the id for `prop_dynamic`, `prop_physics` and brush
    entities; lights and the player start stay unnamed.
  - `prop_dynamic` `solid` from its collision (6 or 0), `DefaultAnim` from its
    default animation.
  - Trigger `spawnflags` from the touch filter (clients 1, physics 8;
    unauthored = clients, as in v1).
  - Connections travel as typed brush-set records (never key strings); `vbsp
    -authored` appends them with `CMapFile::AddConnection`, which the VMF
    loader's connections chunk now shares. Delay 0, fire every time.
  - `func_movelinear`: `movedir` is the QAngle of the move, `movedistance` its
    length, `speed` = distance / seconds; its faces keep their materials.
  - Bake policy `prop-static-casts-baked-shadows/v1`: every `prop_static`
    casts lightmap shadows (`disableshadows 0`; vrad traces its `.phy`, or a
    hull without one). `collision = none` changes game collision only.
    Dynamic and physics props move, so the bake ignores them.
  - Model roles (content stage): a `prop_static` model must be `$staticprop`
    (vbsp's rule) → `compile.model-role-mismatch`; a `prop_dynamic` must have
    its default animation and every sequence a `SetAnimation` names →
    `compile.model-sequence-missing`. `.phy` is required for `prop_physics`
    and for any prop with `collision = vphysics`. The MDL header reader is
    `source_model.read_model` (flags, hull, sequence labels).
  - This changes the U1 room's output: its trigger and cube now carry
    targetnames. The `run/maps/usd_room` package predates it.
- **Stages.** Still U1's nine stages, each with declared inputs, outputs and
  hashes, one ordered publish, serial (`vvis`/`vrad` `-threads 1`), no private
  pools. The role checks live in the validate, content, brushset and check
  stages.
- **Runtime probes** (`game/server/entitylist.cpp`, read-only, server admin
  only): `map_entity_probe <name>` (origin, movetype, solid, vphysics motion,
  gravity and shadow, asleep, sequence, cycle, game time),
  `map_entity_probe_schedule <seconds> <count> <then> <names…>` (samples at
  game-time intervals from a per-frame game system, then hands `<then>` to the
  server console), and `map_trace_probe` (a `MASK_SOLID` ray or hull through
  world, static props and entities; reports a static-prop hit and its index).

### Findings

- A dedicated server never simulates without players (engine `SV_Frame` only
  runs `SV_Think(true)` with a player), so falling, animation, triggers and
  movers are observed in the windowless client. The dedicated server checks
  the spawn state and the static-prop traces.
- Console `wait`s do not measure game time: a config's 1,200 frames of waits
  ran within one game tick. The client therefore holds the physics clock
  (`+phys_timescale 0`) until its config runs after U1's wait, releases it,
  and samples with the game-time schedule.
- A one-shot `SetAnimation` returns to the default animation when it ends
  (`CDynamicProp`), so the oracle checks that the set sequence plays (its
  cycle advances), not that it is still playing at the end.
- A `prop_physics` whose model has no physics prop data is removed at spawn.
- `archlint inventory --verify` reports the loader inventory stale at the
  base commit (line shifts in `unittests/tier1test/moduleloadtelemetrytest.cpp`);
  this slice adds no loader site. The shared tree has an uncommitted fix.

### Evidence

Worktree `agent-aa92ffab396e8aea9`, Linux desktop, 2026-09-25; local
evidence in its ignored `quality-results/rfc0009-u2-20260925/`. The evidence
below was produced on base `002d968e`. After rebasing onto `2b98a129`, the
tools, `test_usd_authoring` and `test_usd_map_compile` (175 tests) and the
quality self-tests were rerun and pass; the client and dedicated builds were
not rebuilt (that commit changes the native Vulkan backend, physics
tests and build scripts, not the server game code or the probes).
Builds from this tree: `build-u2-tools` (`--tools`), `build-u2-dedicated`
(`--dedicated … --build-games=portal`) and `build-u2-client`
(`--platform-provider=sdl3 --render-backend=native-vulkan
--build-games=portal`), gcc release.

- **Fixture** `quality/fixtures/usd-authoring/roles/`: reuses the room's
  world and prefab layers. It has a `prop_static` desk (`vphysics`), a
  `prop_static` desk with `collision = none`, the `prop_physics` cube 160 units
  up over a trigger (clients and physics), a floating `prop_dynamic` switch
  (`models/props/switch001.mdl`, default `idle`), a `func_movelinear` lift
  (64 units up in 1 s) and the light. The trigger's connections set the
  switch's animation to `down` and open the lift.
- **Validator:** 814 checks, 0 failures on the role fixture (the room: 661;
  the per-property version gate adds one check per authored property).
  `test_usd_authoring`: 119 tests (was 90): 21 role-fixture bad variants plus
  a `func_door` room variant, each with exactly its code; the role records;
  the move under a transform; a v1-only copy that is clean; a flattened USDC
  save and reopen with identical objects and no id diff; and a swap of two
  props' roles, which fails twice (`property.misplaced` on the switch, and
  `io.input-unsupported` for the trigger's `SetAnimation` on what is now a
  `prop_static`).
- **Compiler:** the role fixture compiles and publishes with 6,684
  independent output checks. New checks: targetnames; dynamic `solid` and
  `DefaultAnim`; physics props never animated; trigger `spawnflags`; every
  connection against its target's name and the policy; the lift's move
  recomputed from `movedir`/`movedistance`/`speed` against the authored
  displacement and duration; its faces' materials and st; `STATIC_PROP_NO_SHADOW`
  clear; and a baked-shadow oracle: the floor luxel where the light's line
  through each desk's top lands is 0.24 (collision desk) and 0.16 (none) of
  its mirror point's, limit 0.6. `test_usd_map_compile`: 56 tests (was 31).
- **Negative fixtures** (the published store stays byte-identical):
  - roles swapped against the published package: `id.role-changed`, twice;
  - roles and their keys swapped in a fresh document:
    `compile.model-role-mismatch` (`switch001.mdl` is not `$staticprop`);
  - a `prop_physics` without `.phy` (the cake): `compile.model-collision-missing`;
  - `massScale` on the dynamic prop: `property.misplaced`;
  - parenting: `property.unsupported`; an unknown key: `property.unknown`;
  - `SetAnimation dance` and a default animation `wave`:
    `compile.model-sequence-missing`; a missing target: `io.target-missing`.
- **Runtime** (`usd_map_runtime.py --roles`, published map
  `run/maps/usd_roles`, BSP2 sha256 `d14fddaa…`):
  - Client (null renderer, SDL offscreen, no window, IVP), 120 checks, 0
    failures. 40 samples 0.1 s apart from t = 2.355 s to 6.24 s:
    - the cube is a gravity body (movetype 6, no shadow); z 159.6 → 20.48
      (its hull rests at 20.25), after one bounce, then asleep;
    - the trigger fires both connections at t = 2.52 s (the engine's
      `developer 2` I/O trace), with no unhandled input;
    - the lift is a pusher (movetype 7) and rises 0 → 64 with 12 samples in
      between;
    - the switch keeps its placement (96, 0, 96) in every sample, is never a
      simulated body (movetype 7, no gravity), starts in `idle`, plays
      `down` (cycle 0 → 0.56 → 1) and returns to `idle`;
    - a ray and a player hull dropped on the collision desk stop at z 37.707
      (model top 37.93) as static prop 1; on the `none` desk both reach the
      floor (z 0.031).
  - Dedicated server, 111 checks, 0 failures: the spawn state (physics body,
    non-physics switch at its placement in `idle`, closed pusher lift) and the
    same traces. U1's room still passes its U1 runtime smoke on these builds
    (dedicated 62, client 63).
  - `test_usd_map_runtime`: 5 tests.
- **Mutants**, all caught by the intended check:
  - 13 in `test_usd_map_compile`: dynamic prop emitted as `prop_physics`;
    cube and switch classes swapped in the brush set; `.phy` requirement
    dropped (cake); touch filter ignored; a connection dropped; lift speed and
    direction; lift faces given another material; desk without baked shadow
    (both the lump flag and the luxel oracle fail); desk not solid; default
    animation dropped; switch unnamed.
  - 3 runtime mutants compiled past the output checks: the switch as a
    `prop_physics` (removed at spawn, never sampled); a clients-only trigger
    (no connection fires, the lift stays shut, the switch does not animate,
    the cube still falls); the collision desk made non-solid (traces reach
    the floor).
  - 12/12 validator mutants (version gates, misplaced keys, class merge,
    class-value gate, io output/target/input, brush-entity surfaces, move
    transform, non-zero move, sequence names) and 8/8 compiler/checker
    mutants (model-role and sequence checks, targetnames, touch filter,
    connection comparison, shadow oracle, move recomputation, default
    animation), each against a clean baseline run.
- **Conformance:** `corpus.usd-authoring.validator` 127 checks,
  `corpus.usd-map.compiler` 64, new `product.usd-map.roles-runtime` 13; all
  optional, all pass.
- **Repository checks:** quality self-tests (fast tier, 1,030) pass; archlint
  check and baseline pass; stylelint clean on the changed C/C++ lines.

### Not done or unverified

- Parenting, `bbox` collision, other moving brushes (`func_door`, …), NPC and
  pushable touch filters, outputs of props and movers, delays and fire counts
  other than the policy's.
- Behaviour on the dedicated server (no player, no simulation) and under
  Box3D: the client runs `-physics vphysics` (IVP).
- No native Vulkan frame of the role map; no Android or Apple run; no hosted
  CI lane.
- The shadow oracle assumes a scene symmetric about the light's y plane, as
  the fixtures are; props it cannot measure are listed, not failed. There is
  no per-vertex static-prop lighting or Cycles bake of props.
- "Save and reopen" is a USD flatten and export, not an editor session (U3).
- The model-role check reads the MDL header only (flag, hull, sequences); a
  `.phy` is checked for existence, not for a match to its model.

Reproduce, with the tools, builds and content above:

```sh
PYTHONPATH=build/toolchains/openusd-25.11/lib/python /usr/bin/python3.12 \
  tools/quality/usd_map_compile.py compile quality/fixtures/usd-authoring/roles/roles.usda \
  --map usd_roles --tools build-u2-tools/install --runtime run/runtime
python3 tools/quality/usd_map_runtime.py --map-file run/maps/usd_roles/maps/usd_roles.bsp \
  --map usd_roles --report <build>/stages/validate/authoring-report.json \
  --provenance run/maps/usd_roles/provenance.json --roles --runtime run/runtime \
  --build build-u2-client --product client --out <dir>
SOURCE_USD_TOOLCHAIN=build/toolchains/pbrt-map-toolchain.json \
SOURCE_USD_MAP_TOOLS=$PWD/build-u2-tools/install \
SOURCE_USD_MAP_CLIENT_BUILD=$PWD/build-u2-client \
SOURCE_USD_MAP_DEDICATED_BUILD=$PWD/build-u2-dedicated \
  python3 tools/quality/conformance.py check --rfc 0009 --out <dir>
```

**U2 merged into the shared tree (2026-09-25).** Commit `08e9d4d2` was
applied without a commit and verified here:

- `build.tools`, `build.portal-native`, `build.dedicated` and `build.hl2`
  pass (`libserver.so` carries the new probe commands).
- `corpus.usd-authoring.validator`: 127 checks.
- `corpus.usd-map.compiler`: 64 checks.
- `product.usd-map.roles-runtime`: 13 checks, run against the rebuilt
  `build-r03-portal-native` client and `build-r03-dedicated` server.
- `usd_room` was republished with the version 2 compiler (6,598 output
  checks), so `./play usd_room` matches the current tools.

