# RFC 0009 progress: USD-native map authoring

Updated: 2026-09-25
Portfolio row: R59 (U0–U2), active. R60 (U3–U4) is planned.

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


## U2: prop role cohorts and geometric entities (active, 2026-09-25)

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

Implemented in an isolated worktree; results are recorded here when merged.
