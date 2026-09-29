# Contract: `content.studio-model.v1`

Module: `content.studio-model` · Header: `public/mdl/studio_model.h`
Impl: `mdl/studio_model.cpp` · Waf target: `mdl` (strict C++20 static library)
Conformance: `unittests/mdltest/test_studio_model.cpp` (`content.studio-model`),
`unittests/mdltest/test_studio_model_pose.cpp` (`content.studio-model.pose`),
`unittests/mdltest/test_studio_model_corpus.cpp` (`content.studio-model.corpus`)
Fixture writer: `unittests/mdltest/synthetic_model.h`
Depends on: `foundation` (`Expected`) only.

## Purpose

Read the drawable level of detail 0 of a Source studio model (MDL + VVD + VTX)
as values, so the editor's viewports, the compile tools and the engine can share
one reader that needs no tier0, tier1, mathlib or `studio.h`. First consumer: the
GTK Hammer viewports (RFC 0002 R17 follow-up, props and instances).

## Obligations

- Accepts MDL `IDST` versions 44 to 49, VVD `IDSV` version 4, VTX version 7,
  with equal checksums in the three files. The MDL version selects the VTX
  strip-group and strip layouts (33/35 bytes for version 49, 25/27 otherwise),
  as `datacache/mdlcache.cpp` does.
- Returns, for the chosen body (part `p` draws model `(body / base_p) % count_p`),
  every LOD 0 mesh with triangles: its skin reference, its vertices (the mesh's
  range of the VVD's LOD 0 list after fixups; bind pose, model space) and a
  triangle list over them, wound counter-clockwise seen from outside (the files'
  clockwise order with the second and third index swapped). Tristrip strips
  become lists. Bounds cover the vertices the triangles use.
- `TextureIndex` maps a mesh and skin through the skin table; a skin out of
  range uses family 0 (engine behavior).
- `ResolveMaterials` names each texture as a VMF names a material: the first
  `$cdmaterials` directory with `materials/<dir><texture>.vmt`, else the first
  directory's candidate marked not found. Lower case, `/` separators.
- `LoadModel` reads `.mdl`, `.vvd` and the first of `.dx90.vtx`, `.vtx`,
  `.dx80.vtx`, `.sw.vtx` through `IModelFiles`, after `CanonicalModelPath`;
  also the `.ani` file the header names (when it names animation blocks) and
  the `$includemodel` models, depth first with their `.ani` files (an absent
  include is skipped, as the engine skips it).
- Skeleton (added 2026-09-28, R17 follow-up "posed models"): bones (name,
  parent before child, reference position/quaternion, euler base, posscale,
  rotscale, poseToBone, flags); per-vertex VVD bone weights (1 to 3 bones, each
  naming a bone); per sequence its label, flags, stored box, blend count, the
  animation at blend (0, 0), its bone weights and the local pose of every bone
  at frame 0. Frame 0 is decoded as `public/bone_setup.cpp` does: RLE records
  (raw Quaternion48/64, raw Vector48, run-length streams times rotscale/posscale
  added to the euler/position base, delta records), frame x bone data
  (`STUDIO_FRAMEANIM`: Quaternion48, Quaternion48S, Vector48, float vectors,
  frame or constant), section 0, `.ani` blocks, zero-frame data when the block
  is not loaded. Sequence weights blend with the reference (0: the reference;
  delta sequences add). `$includemodel` sequences follow the engine's virtual
  model (own first, depth first, first label wins unless `STUDIO_OVERRIDE`,
  bones mapped by name).
- `PoseModel` skins vertices and normals to a pose's model space
  (bone-to-model x poseToBone, weighted) and recomputes bounds; the reference
  pose gives the bind vertices; by sequence, a `$staticprop` model is returned
  unchanged (the engine's static-prop path).

## Inputs and outputs

Input: three byte strings (`ModelBytes`) or an `IModelFiles` and a model path.
Output: `foundation::Expected<Model, ModelError>`. `Model` is a value (equality
comparable); `ModelError` is `{ status, file, offset }`.

## Failure behavior

Malformed input returns an error, never a partial model: `BadMagic`,
`UnsupportedVersion`, `Truncated` (a read past the end), `BadCount` (negative or
disagreeing counts, a list strip not a multiple of three), `BadOffset` (outside
the file or misaligned), `BadIndex` (a vertex, index, texture or skin reference
naming nothing), `ChecksumMismatch`; `MissingFile` from `LoadModel`, and
`MissingFile` in `ani` when a first frame lives in an `.ani` block that was not
given and the model has no zero-frame data. Errors in an included model are
reported in `included mdl` / `included ani`. Every count
and offset is bounded by the file size before use; `Describe` renders
"<status> in <file> at byte <offset>".

## Lifetime and threading

Pure functions over borrowed bytes; the result owns its data. `IModelFiles` is
borrowed for the call. No globals; callable from any thread.

## Evidence and oracles

- `content.studio-model` (linux-headless-core): synthetic models written by an
  independent writer parse to what they describe (versions 44, 45, 48, 49;
  fixups; tristrips; body groups; skin families; material resolution order;
  VTX file preference); 23 malformed cases fail with their exact status and
  file; every strict prefix of each file fails; 3000 random byte-damage trials
  never yield an out-of-range index. Also passes under clang++ ASan/UBSan
  (`-fsanitize=address,undefined -fno-sanitize-recover=all`, run by hand).
- `content.studio-model.corpus` (optional; `STUDIO_MODEL_CORPUS_VPKS`): every
  model with a `.vvd` in Portal's and Portal 2's VPKs parses; at least 97% of
  triangles face their vertex normals (the winding swap: without it about 1%
  would); each game's `metal_box.mdl` matches figures from an independent
  Python walk of the records (Portal: version 44, 4664 triangles, 2 textures;
  Portal 2: version 49, 5352 triangles, 12 textures).
- `content.studio-model.pose` (linux-headless-core): synthetic models with
  hand-worked expected poses for every first-frame encoding, hierarchy,
  blended weights, sequence weights, deltas, blends, sections, `.ani` blocks,
  zero-frame data, `$includemodel` merging and the static-prop rule; seeded
  defects (swapped quaternion components, dropped parent concatenation,
  ignored posscale) rejected; malformed skeletons, weights, sequences,
  records, blocks and frame data fail with their status and file.
- `content.studio-model.corpus` also judges posing against the compiler's own
  data: each full-weight sequence's first frame lies inside its stored box
  (vertices; bone origins for sequences of animation-only included models;
  8681 of 8682 in Portal 2, 285 of 285 in Portal), the bind pose misses the box
  for 6586 of them (the defect the viewport had), the reference pose gives the
  bind vertices for every model, every `$staticprop` (1444 + 119) is unchanged
  at its first frame, and the three seeded defects are caught for most
  sequences they change. Named models of `sp_a2_trust_fling` (cube dropper,
  panel arms from their included animation model, turbine elevator), the faith
  plate and the turret match root rotations from an independent Python walk.
- Not covered: LOD above 0, flexes, eyeballs, bone controllers, procedural
  bones, IK, pose parameters (blend grids draw their first animation),
  animations merged across included models by name, the zero-frame
  Quaternion32 rotation (`BONE_HAS_SAVEFRAME_ROT32`), material replacement
  lists, big-endian (console) files.
