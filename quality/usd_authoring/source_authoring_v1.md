# Source authoring profile (USD-native maps, RFC 0009 U0-U2)

Status: profile versions 1 (U0) and 2 (U2) for roadmap R59, in one file. It
defines the editable USD map source that the native compiler (U1) reads;
[Profile version 2](#profile-version-2-u2) lists what U2 added. It is **not** RFC 0008's compiled
World Stage schema (`utils/worldstage/schema.usda`, `SourceWorldAPI` and the
`source:` properties). Those carry derived compiler output, and an authoring
stage that contains them fails validation (`schema.compiled-data`).

| File | Owns |
| --- | --- |
| [`source_authoring_v1.json`](source_authoring_v1.json) | Machine-readable facts: roles, per-role properties and allowed values, the id, model and material patterns, tolerances, name hints and the error-code catalog |
| This document | Meaning and reasons: the decisions behind those facts |
| [`tools/quality/usd_authoring_validate.py`](../../tools/quality/usd_authoring_validate.py) | The validator; it reads the JSON and emits only codes declared there |
| [`quality/fixtures/usd-authoring/room/`](../fixtures/usd-authoring/room/) | The hand-authored positive fixture (declares v1) |
| [`quality/fixtures/usd-authoring/roles/`](../fixtures/usd-authoring/roles/) | The U2 role fixture (declares v2; reuses the room's world and prefab layers) |
| [`tools/quality/tests/test_usd_authoring.py`](../../tools/quality/tests/test_usd_authoring.py) | The oracles, which generate every bad variant from the fixture |

The scope is deliberately small. It covers what the first playable USD room
needs, and every other role, class, property value or USD feature fails
validation by name. No feature is half-supported.

## Encoding

- **Namespaced attributes, not `customData` or a compiled schema plugin.**
  Source meaning is carried by typed attributes in the `sourcemap:` namespace,
  such as `uniform token sourcemap:role`. Attributes are typed and composable.
  usdview, usdcat and DCC importers show and keep them, and the Hammer document
  layer can edit them through ordinary `UsdAttribute` calls.
  - `customData` is untyped metadata, and tools often drop it.
  - A codeless `usdGenSchema` plugin would need `plugInfo.json` registration
    in every tool and runtime that opens a map. U0 does not need that.
  - The names follow USD's API-schema property convention, so a later codeless
    `SourceMap…API` can declare the same properties without renaming authored
    data.
- `sourcemap:` is kept separate from the compiled stage's `source:` on purpose,
  so authored and derived data can never be confused.
- **Stage record.** The root layer carries
  `customLayerData = { dictionary sourcemap = { string profile = "source-authoring"; int version = 1 } }`.
  A different profile or an unknown version fails (`stage.profile-*`). A
  version bump is required for any change that alters the meaning of existing
  data, and for a new feature, so that a tool that knows only the older
  version rejects a newer stage by its version instead of misreading it.
  Versions 1 and 2 are accepted; see [Profile version 2](#profile-version-2-u2).
- The root layer must declare a `defaultPrim`, so a map can be referenced
  whole.

## Units and up axis

The root layer must author both `metersPerUnit` and `upAxis`. USD's fallbacks
(0.01 and Y) are not accepted silently.

- **Fixture convention:** `upAxis = "Z"` and `metersPerUnit = 0.0254`, so one
  USD unit is one Source unit (one inch) and the compiler's conversion is the
  identity.
- **Accepted:** `upAxis` Y or Z and any positive `metersPerUnit`.
- **Conversion to Source space:** first scale by `metersPerUnit / 0.0254`, then
  for Y up map `(x, y, z)` to `(x, -z, y)`. The validator applies this
  conversion to every point, origin and angle in its report, and U1 must apply
  the same one.
- **One unit system per stage.** USD does not convert between layers. Any
  sublayer or referenced layer that authors a different `metersPerUnit` or
  `upAxis` fails (`composition.units-mismatch`).
- **Rigid placements are judged in Source space.** Model assets are in Source
  units, so a centimetre stage places a model under a 2.54 scale. An unscaled
  placement there would shrink it and fails `transform.not-rigid`.
- **Tolerances** are in Source units: 0.01 units for planarity, convexity and
  coplanarity.

## Roles

A prim's role is its authored `uniform token sourcemap:role`. The validator
never infers a role from a prim name, a path, a material or the geometry alone:

- A Gprim, a light, or a prim with `sourcemap:` data but no role fails
  `role.missing`.
- A prim named like a role or class (see the JSON's `name_hints`) that declares
  no role and contains no role prims fails `role.name-only`. A grouping scope
  may use such a name when it holds role prims.
- Prims without a role may be only structure: untyped prims, `Xform`, `Scope`,
  `Material`, `Shader`, `NodeGraph` or `Camera`. The compiler ignores cameras;
  they are editor state. Any other prim type fails (`feature.unsupported`).
- A role prim may contain only `materialBind` face subsets
  (`role.children-unsupported`). One prim is one authored object.
- Each role declares its properties. An undeclared `sourcemap:` property fails
  (`property.unknown`), as does a token outside the supported values
  (`property.value-unsupported`, whose message says "reserved" for values kept
  for later profiles).

| Role | v1 | Prim | Required | Optional | Compile contract (U1) |
| --- | --- | --- | --- | --- | --- |
| `world_solid` | supported | `Mesh` | `id`, `contents` = `solid`, `surfaceIds`, `primvars:st`, a bound Material | — | World render faces, collision, sealing and visibility, one brush per prim |
| `prop_static` | supported | `Xform` | `id`, `model`, `collision` = `vphysics` or `none` | `skin` | Static-prop placement; never sealing world structure |
| `prop_physics` | supported | `Xform` | `id`, `model` | `skin`, `massScale` | `prop_physics` entity; its collision and surface properties come from the model |
| `entity_point` | supported | `Xform` | `id`, `classname` = `info_player_start` | — | Point entity at the prim's origin and angles |
| `entity_brush` | supported | `Mesh` | `id`, `classname` = `trigger_multiple` or `trigger_once`, one convex solid | `surfaceIds` | Brush entity; the compiler assigns `tools/toolstrigger`, and any bound Material is only an editor preview |
| `light` | supported | `SphereLight`, no `ShapingAPI` | `id`, authored `inputs:intensity` > 0 and `inputs:color` | UsdLux inputs | Baked point light; the intensity-to-`_light` conversion is a U1 policy |
| `prop_dynamic` | supported since v2 | `Xform` | `id`, `model`, `collision` = `vphysics` or `none` | `skin`, `defaultAnimation`; `parent` is reserved | `prop_dynamic` entity: movable and animated, never a physics body; in a v1 stage it fails `role.unsupported` |

The property names in the table are shortened; each is `sourcemap:<name>`.

Placement roles (props, point entities, lights) need a rigid transform:
rotation and translation only, with a positive determinant, in Source space.
Solid roles accept any finite, non-singular affine transform. Geometry is
checked after the transform is applied.

## World-solid geometry profile

**Choice.** A world solid is one **closed convex polyhedron**, given as a
`UsdGeomMesh` with `subdivisionScheme = "none"`:

- Each polygon face is planar and convex.
- Each face is exactly one brush side: no two faces share a plane.
- Every corner is one shared point.
- Every edge is used exactly once in each direction.
- The faces wind outward according to `orientation`, which flips when the
  local-to-world transform mirrors.

**Why this and not planes or free meshes.**

- It is exactly a `vbsp` brush. U1 can derive one plane per face and reuse the
  existing convex CSG, BSP, portal, visibility and collision-hull algorithms
  through an in-memory adapter, with no VMF and no temporary files.
- It is still standard USD geometry, so any USD tool renders, inspects and
  edits it. A planes-only encoding would be invisible outside the editor.
- A general triangle mesh would need new closed-world, portal and collision
  construction (RFC 0009 open question 2). That remains a later capability, and
  a non-convex world part is authored today as several convex solids.
- The editor's block and clip tools map one-to-one: cutting a solid makes two
  solids, and a new face gets a new surface id.

**Checks, in order.** The first failure ends the chain for that prim, because
each check relies on the ones before it:

1. subdivision is `none`;
2. every point is finite;
3. every face has at least three distinct, in-range vertices and non-zero area;
4. every face is planar;
5. no directed edge is repeated (manifold, consistent winding);
6. every edge has an opposite edge (closed);
7. the enclosed volume is positive (outward);
8. no vertex lies in front of any face plane (convex; on a closed solid this
   also rejects every concave face);
9. no two faces are coplanar.

**Surfaces.** `uniform int[] sourcemap:surfaceIds` has one non-negative value
per face, unique within the solid.

- An edit keeps a surviving face's value.
- A new face (from a clip or split) takes an unused value.
- The global surface identity is `(object id, surface id)`. The validator
  reports it with the face's resolved material.

**Texture coordinates.** `primvars:st` is required, with `faceVarying` or
`vertex` interpolation, and indexed primvars are accepted. On each face, st
must be one non-degenerate affine function of position, in texture repeats.
That is exactly what a BSP texinfo can represent, so U1 converts it losslessly.
t points up, as in UsdPreviewSurface: the texel column is s × width and the
texel row is (1 − t) × height, so the texinfo U vector is d(s)/d(position) ×
width and the V vector is −d(t)/d(position) × height (U1 decision; see
`source_compile_v1.json` "texture"). A non-affine or collapsed mapping fails
`uv.not-affine`.

**Contents.** `sourcemap:contents` must be `solid`: the solid renders, collides,
seals and blocks visibility. `detail`, `playerclip`, `monsterclip`, `window`,
`grate`, `water` and `nodraw` are reserved. They fail until a later profile
defines their collision and visibility semantics.

## Materials

- A world-solid face resolves its Material with standard `UsdShade` binding
  resolution, all-purpose.
- Per-face materials use `UsdGeomSubset`s of family `materialBind` (element
  `face`), which must pass `UsdGeomSubset::ValidateFamily`. Faces outside every
  subset use the prim's binding. A face with no Material fails
  `material.missing`.
- The prim or subset that holds the binding must apply `MaterialBindingAPI`.
  USD 25.11 still honours bindings without it, but with a warning, and USD is
  phasing that out, so a binding without the API fails now instead of
  vanishing in a later release.
- The Material carries `uniform string sourcemap:material`: a lowercase VMT
  path relative to `materials/`, without the extension, for example
  `metal/metal_modular_floor001`. It is the compile identity.
- A `UsdPreviewSurface` network under the Material is an optional preview for
  generic USD tools. It is not compiled.

## Model references

`uniform string sourcemap:model` is a game-filesystem path, not a USD asset
path. It is lowercase, relative, uses forward slashes, has no `.` or `..`
segments, and matches `models/…/<name>.mdl`. That makes it one canonical
spelling in Source's case-insensitive filesystem. A string, not an `asset`,
keeps USD's resolver from anchoring it to the layer's directory.

U0 checks the syntax only. The fixture's models,
`models/props/lab_desk01/lab_desk01.mdl` and `models/props/metal_box.mdl`,
exist in Portal's `portal_pak_dir.vpk` in the local runtime, but the validator
does not open game content. The U1 compiler checks existence and collision
files per role (`source_compile_v1.json` "model_files"): a `prop_physics`
model needs its `.phy`, and so does a `prop_static` or `prop_dynamic` with
`collision = vphysics`. U2 adds the per-role model capabilities (RFC 0009 open
question 3, policy `model_roles`): a `prop_static` model must be compiled with
`$staticprop`, as vbsp requires, and a `prop_dynamic` must have its default
animation and every sequence a `SetAnimation` connection names.

## Authored-id policy

- **Creation.** Every role prim has `uniform string sourcemap:id` matching
  `^[a-z0-9][a-z0-9._-]{0,63}$`. The creating tool assigns it once, whether that
  is the editor, an importer or a hand author. The editor's generator is a
  U3 decision; hand-authored fixtures use readable ids.
- **Uniqueness** is per composed stage, across all layers, references and
  prefab placements (`id.duplicate`). A referenced prefab carries no id. Each
  placement authors its own over the prefab's role and model, as the fixture's
  `MetalBox` does.
- **Persistence.** The id belongs to the object, not to its path. Renaming,
  reparenting, moving between layers, transforming, rematerialling and
  reshaping keep it, and a save/reopen or a USDA/USDC round trip preserves it.
  The tests cover rename, reparenting into a group, flattening and the USDC
  round trip. The other edits belong to the editor's U3 gate.
- **Deletion.** The object's id is retired. A retired id is not reused in the
  same map lineage. The validator detects reuse only when the role differs;
  the editor must enforce the rest.
- **Role changes.** An object keeps its id only while its role is unchanged.
  With `--previous`, an id that now names a different role fails
  `id.role-changed`, because its compiled outputs would change meaning.
  Converting, for example, `prop_static` to `prop_physics` creates a new object
  with a new id.
- **Mapping to compiled outputs.** The validator's `objects` table is the
  compiler's id input: id, role, prim path, class, model, Source-space origin,
  angles and bounds, and for solids each surface id with its Source material.
  Since U1 it also carries, for each solid, every face's outward unit plane,
  its corners (counter-clockwise seen from outside) and its st rows (`s` and
  `t` as `[Gx, Gy, Gz, g0]`, so st = G · p + g0 in repeats at the Source point
  p); for each light, its UsdLux inputs (exposure, diffuse, normalize,
  enableColorTemperature and the radius in Source units); and prop collision,
  skin and mass scale. These fields extend the report; the authoring schema is
  unchanged.
  U1 must record the authored id on every compiled record it derives: entity
  records, static-prop entries, the world triangles of each
  `(object id, surface id)`, and brush sides and leaves where it keeps
  provenance. RFC 0008's compiled stage owns that encoding. BSP face ids are
  never an identity for these maps.
- **Diff.** With `--previous`, the report's `diff` lists added, removed and
  moved ids. That is the input for incremental builds and stale-id diagnostics.

## Composition

The validator checks the **composed** stage, so sublayers, references,
payloads (all loaded) and selected variants apply before any role check.
Flattening does not change a valid stage's `objects` table.

- Any USD composition error fails `composition.error`: an unresolved sublayer,
  reference or prim path. Prim checks are then skipped, because a partially
  composed stage is not the author's intent.
- Instancing (`instanceable`, instance proxies) and `PointInstancer` fail
  `feature.unsupported`. Each authored object is its own prim with its own id.
- Time samples on a role prim, or on any transform above it, fail
  `feature.unsupported`, because v1 maps are static.

## Validator

```sh
PYTHONPATH=build/toolchains/openusd-25.11/lib/python /usr/bin/python3.12 \
    tools/quality/usd_authoring_validate.py validate \
    quality/fixtures/usd-authoring/room/room.usda \
    [--json report.json] [--previous earlier.usda|earlier.json]
```

- The command prints `FAIL <code> <prim>: <message>` for each error and ends
  with the checks-v1 record `CONFORMANCE <checks> <failures>`. It exits 0 only
  when there are no errors.
- `--json` writes `source-authoring-validation/v1`: `errors` (code, path,
  message), `objects` (above), `layers`, `checks`, and `diff` when
  `--previous` is given.
- Error codes and their meanings are listed only in the JSON profile's
  `errors`. Emitting an undeclared code raises.
- Each problem is reported once:
  - Checks that depend on a failed check are skipped.
  - Missing units or up axis assume this profile's convention for the
    remaining checks.
- `usdchecker` is necessary but not sufficient. The fixture passes it, and so
  does a role-less stage that this validator rejects.

Tests: `tools/quality/tests/test_usd_authoring.py`, run through the
`corpus.usd-authoring.validator` conformance row under the pinned OpenUSD
Python. Every declared code has at least one generated bad variant, and each
variant must produce exactly that one code at its intended prim.

## Fixture

[`room/room.usda`](../fixtures/usd-authoring/room/room.usda) is the root layer.
It holds the profile record, the metrics and the `/Map` default prim, and it
sublayers two layers:

- `world.usda`: six world solids forming a sealed 512 × 384 × 192-unit room,
  16 units thick, and three Materials that bind Portal VMTs;
- `entities.usda`: `info_player_start`, a `trigger_multiple` solid, a
  `prop_static` lab desk, a `prop_physics` metal box referenced from
  `prefabs/metal_box.usda`, and a `SphereLight`. The desk stands at z 37.7:
  `lab_desk01`'s model origin is its top, and U1 found the U0 placement (z 0)
  under the floor.

## Profile version 2 (U2)

Version 2 adds the prop and geometric-entity cohorts. The JSON marks each new
role, class, property and class value with `"since": 2`; in a stage that
declares version 1 they fail as unsupported (`role.unsupported`,
`entity.class-unsupported`, `property.unsupported`), so v1 stages validate
exactly as before.

- **`prop_dynamic`.** A placement of an existing model with a rigid transform.
  - `collision` is required and becomes the entity's `solid`
    (`vphysics` 6, `none` 0; `bbox` is reserved).
  - `defaultAnimation` is optional and must be a sequence name; the compiler
    checks that the model has it.
  - `parent` is declared but reserved (`property.unsupported`): parenting is
    not simple enough for this slice, so it fails explicitly rather than being
    ignored.
- **Keys that belong to another role or class** fail `property.misplaced`,
  which names their owners (for example `massScale` on a `prop_dynamic`
  belongs to `prop_physics`). Keys no role declares still fail
  `property.unknown`.
- **Per-class properties of `entity_brush`** (`classes` in the JSON) are
  merged over the role's properties:
  - `trigger_multiple` and `trigger_once`: optional `touchFilter`
    (`token[]` of `clients` and `physics`; unauthored means clients, as in v1)
    and optional `connections`.
  - `func_movelinear` (since v2): a moving brush with authored surfaces. Its
    faces need surface ids, a bound Material and affine st like a world solid
    (the compiler keeps its materials instead of the trigger tool material).
    `moveDelta` (`float3`, non-zero) is the displacement in the prim's space,
    transformed with it and converted to Source units; `moveSeconds` (> 0) is
    the travel time. A duration keeps the speed free of unit conversion.
- **Connections.** `uniform string[] sourcemap:connections` holds one entry per
  entity output, `"Output target-id Input"` or
  `"Output target-id Input parameter"`, separated by single spaces. The target
  is a `sourcemap:id`, so a rename or regroup never breaks it; the compiler
  gives every connection target its id as its targetname. The JSON's `io`
  table declares the outputs of each trigger class and the inputs each target
  accepts: `prop_dynamic` `SetAnimation` and `SetDefaultAnimation` (a sequence
  parameter), `func_movelinear` `Open` and `Close` (no parameter). Failures:
  `io.invalid` (syntax or an output the class lacks), `io.target-missing` and
  `io.input-unsupported` (including a parameter that does not fit).
- **Roles in reports.** The validator's objects carry `default_animation`,
  `touch_filter`, parsed `connections`, `move` and `move_seconds` beside the
  v1 fields.

## Open items for U1

Resolved by the first U1 slice (policy in
[`source_compile_v1.json`](source_compile_v1.json), reasons in the
[RFC 0009 progress record](../../RFC/0009-progress.md)): the compiler inputs,
the light conversion, the trigger's class defaults, the leak test, model and
collision existence, the lightmap scale and the trigger's tool material, and
the authored-id encoding in compiled outputs. Still open: entity keys and I/O
(U2), non-`solid` contents and `nodraw`, per-role model capabilities beyond
file existence (U2), and id generation (U3). The original list follows.

- **Compiler inputs.** The composed stage after it passes this validator; the
  `objects` table (ids, Source-space placements, surfaces and materials); the
  world solids' Source-space faces and planes; and texture sizes from the
  VMTs' base textures, to scale texinfo.
- **Light conversion.** The UsdLux intensity/colour to Source `_light`
  brightness policy, and whether `inputs:radius` has any compile meaning.
- **Entity keys.** v1 has none beyond classname: no spawnflags, filters or
  I/O. The trigger compiles with the class defaults. U2 adds entity keys and
  connections as declared properties with their own checks.
- **Leak and seal.** The validator does not detect leaks: a closed room is a
  compiler and visibility result. U1's bad-seal fixture fails there, without
  publishing a partial package.
- **Model content.** Model existence and model-capability checks per role.
- **Materials and lighting.** The lightmap scale, which uses the compiler
  default; `nodraw` and tool materials; and non-`solid` contents.
- **Id generation.** The editor's id generator and the persistence of retired
  ids (U3).
