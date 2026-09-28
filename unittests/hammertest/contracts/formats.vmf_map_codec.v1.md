# Contract: `formats.vmf_map_codec.v1`

Module: `hammer.formats`
Header: `public/hammer/formats/vmf_map_codec.h` · Impl: `hammer/core/formats/vmf_map_codec.cpp`
Implements: `ports.map_codec.v1` (`hammer::ports::IMapCodec`)
Conformance: `unittests/hammertest/formats/test_vmf_map_codec.cpp` (+ `_negative`),
`unittests/hammertest/formats/test_vmf_portal2_roundtrip.cpp`; shared oracle
`unittests/hammertest/formats/map_codec_oracle.h`
Migration: `HAM-VMFCODEC-001` (proposed id; R08-DOMAIN)
Depends on: `content.keyvalues-text` (parser/writer), `hammer.scene`, `hammer.ports`

The strict Valve Map Format codec. It replaces the lossy `EditorController`
`ToVmf`/`LoadVmf` pair (brush entities flattened, texture axes regenerated,
connections, groups and visgroups dropped) as the persistence of the typed
`MapDocument`. There are no escape hatches: every block and key loads into a
typed field and round-trips; content the model cannot hold is a `CodecError`.
Strict, MFC-free, GPU-free.

## 1. Purpose, consumers, scope

- Consumers: the composition root that wires `IMapCodec` for `hammer.app` (edit
  session open/save), `hammer_cli`, MCP, and tools.
- Scope: every block legacy Hammer and the Portal 2 Hammer write for map content:
  `versioninfo`, `visgroups` (nested), `viewsettings`, `world` (solids, groups,
  `hidden` wrappers), `entity` (keys, `connections`, brush solids, `hidden`
  solids, `editor`), top-level `hidden` entities, `cameras`, `cordon` (single
  form) and `cordons` (list form), and `dispinfo` with every block
  `CMapDisp::SaveVMF` writes plus the pre-release keys it still reads.
- Not modeled, so rejected: legacy's optional `quickhide` and `autosave`
  blocks, the `cordonsolid` editor key, `dispinfo` mapping axes (`uaxis`/`vaxis`),
  world-level `connections`, and any other block or key.

## 2. Accepted inputs

- Decode: any text. The keyvalues grammar is `content.keyvalues-text`'s (quoted
  strings, bare block names, `//` comments, CRLF or LF).
- Encode: any `MapDocument`.

## 3. Results and guarantees

- **Mapping.** `versioninfo` → `VersionInfo` (`editorversion`, `editorbuild`,
  `mapversion`, `formatversion`, `prefab`); `viewsettings` → `ViewSettings`
  (`bSnapToGrid`, `bShowGrid`, `bShowLogicalGrid`, `nGridSpacing`,
  `bShow3DGrid`); `visgroup` blocks (`name`, `visgroupid`, optional `color`) →
  the visgroup tree; `world` `id` → `worldVmfId`, `classname` must be
  `worldspawn`, every other pair → `worldKeys` in order (the worldspawn data
  model, not an escape hatch); `entity` → `Entity` (`id`, required `classname`,
  every other pair in order with duplicates, connections through
  `scene::ParseConnection` keeping the separator, brush solids with `owner`);
  `editor` → `EditorInfo` (`color`, repeated `visgroupid`, `groupid` → the
  runtime group id, `visgroupshown`, `visgroupautoshown`, `comments`,
  `logicalpos` as integer `[x y]`); `side` → `Side` (authored plane points,
  `uaxis`/`vaxis` `[x y z shift] scale`, rotation, lightmap scale, smoothing
  groups signed or unsigned); `dispinfo` → `Displacement` (`power` 1 to 4,
  `startposition`, `flags`, `elevation`, `subdiv`, `mintess`, `smooth`, `alpha`;
  `normals`, `distances`, `offsets`, `offset_normals`, `alphas` as rows
  `row0`..`rowN` of `2^power + 1` values or vectors, `triangle_tags` as
  `2^power` rows of two tags per quad, `allowed_verts` as one pair keyed by its
  word count; each array absent when its block is); `cameras`, `cordon`,
  `cordons` → settings.
- **Absent keys** take the model's defaults (legacy's) and are written
  explicitly on save. Required: a side's `plane`, an entity's `classname`, a
  displacement's `power` and `startposition`, a visgroup's `name` and
  `visgroupid`, a camera's `position` and `look`, a cordon's `mins` and `maxs`.
- **Errors** (`CodecError { message, line }`, the message prefixed by the block
  path, e.g. `world/solid[12]/side[3]: unknown key 'foo'`; indices count
  same-named siblings from 0; singleton blocks have none): text that is not
  keyvalues (`not VMF text: …`); a pair outside any block; an unknown or second
  top-level singleton block; an unknown key or child block anywhere; a duplicate
  key (except `visgroupid`, entity and world keys); a malformed number, id, flag,
  color, vector, plane, axis or logical position; an unparsable connection; a
  solid with fewer than four sides; displacement rows whose count, keys or
  lengths do not match the power, or an `allowed_verts` key that is not its word
  count; a duplicate side id (overlays name side ids, so reassigning one is not
  lossless) or a duplicate object id (legacy keeps one id space for solids,
  entities and groups); a `groupid` naming no group; a group that contains
  itself; a world classname other than `worldspawn`.
- **Warning** (the only one; lossless): objects and sides without ids get fresh
  ids above every id in the file (early VMFs), in runtime-id order.
- **Runtime id order.** World solids (file order, hidden ones where they
  appear), then world groups (file order), then each entity followed by its
  solids (file order). Encode writes world solids in id order, then groups, then
  entities in id order each with its solids in id order, so
  `Decode(Encode(Decode(t)))` issues identical ids. A world whose groups precede
  its solids is written solids first (legacy order).
- **Encode layout** (legacy writer order): `versioninfo` (five keys),
  `visgroups`, `viewsettings` (five keys), `world` (`id`, `mapversion` when it is
  the first world key, `classname`, other keys; solids; groups), entities
  (hidden ones wrapped inline; keys, `connections`, solids, `editor`), `cameras`,
  then `cordon` when the form is Single and it can hold the content (one unnamed
  cordon, one box, its flag equal to `cordonsActive`), else `cordons` when the
  form is not None or cordons exist. Every solid, entity and group gets an
  `editor` block: `color` (when set), `groupid`, `visgroupid`…,
  `visgroupshown`, `visgroupautoshown`, `comments`, `logicalpos`. Side keys:
  `id plane material uaxis vaxis rotation lightmapscale smoothing_groups`
  (signed, as legacy), then `dispinfo`: `power startposition flags [mintess
  smooth alpha] elevation subdiv`, then the present row blocks in legacy order.
- **Numbers.** `scene::FormatNumber` (`%.10g`) when that text reads back to the
  same double, else the shortest exact decimal (`std::to_chars`); coordinates,
  axes, displacement values and connection delays round-trip bit for bit.
  Parsing is locale-independent (`std::from_chars`).
- **Encode fails** for: a `"` in any key or value; a non-finite number; a
  connection that does not read back to itself; an `id` or `classname` among
  entity or world keys; a displacement power outside 1 to 4 or an array whose
  size does not match it; a solid owned by a missing entity; a reference to a
  missing group.

## 4. Ownership, threading

- Stateless and pure; `const` methods; safe on any thread for distinct inputs.

## 5. Invariants

- A decoded document passes `MapDocument::Validate()` (checked before
  returning; a violation is an internal `CodecError`, never returned).
- Nothing in the input is dropped: every datum is modeled or the decode fails.
  Normalizations that change text but not meaning: number spellings
  (MSVC's `1.52588e-005` → `1.52588e-05`), key order within a block (legacy
  order), pairs before child blocks, one `hidden` wrapper per hidden object,
  several `connections` blocks of one entity merged, absent keys written with
  their defaults.

## 6. Side effects and performance

- No I/O, no logging. One parse, one re-tokenization to map blocks and pairs to
  source lines, one pass to build objects. The vendored Portal 2 maps
  (`sp_a2_trust_fling.vmf`, 1.5 MB, 764 solids, 455 entities, 25 groups;
  `zoo_mechanics.vmf`) decode with no warnings.

## 7. Conformance suite and providers

- `test_vmf_map_codec.cpp` (42 checks): the oracle's clauses (canonical fixture
  field by field and byte for byte, both cordon forms, missing ids, the 32
  rejection cases with their exact paths and lines, an edited document with
  runtime ids out of write order and a typed displacement, an empty document,
  cordon-form normalization), exact-number and displacement output, parse
  errors with lines, foreign text, each Encode failure, and every
  `hammer/gtk/samples/*.vmf` (round trip; `displacement.vmf`'s typed
  displacement). With `HAMMER_VMF_CORPUS=<dir>` it also round-trips every
  `.vmf` there; not part of the required run.
- `test_vmf_portal2_roundtrip.cpp` (40 checks): both vendored Portal 2 maps
  decode with no warnings, validate, reload as `SameContent` with identical
  runtime ids, save to a byte-identical fixed point, compare semantically equal
  to the original file (numbers by value), and match its structural counts; the
  comparator catches a changed key and a dropped connection and accepts a
  respelled number.
- `test_vmf_map_codec_negative.cpp` (58 checks): 20 seeded bad codecs, each
  detected by its named oracle clause (including a lenient codec that strips
  whatever the strict decoder rejects, dropped logical positions and comments,
  lost or shifted displacement data, and a real-map clause over
  `zoo_mechanics.vmf`); the real codec is the control.
- Sources: `hammer/core/formats/vmf_map_codec.cpp`,
  `hammer/core/scene/{map_objects,map_document,map_queries,solid_geometry}.cpp`,
  `mapgeometry/{brush,polytope,texture_axes}.cpp`, `kvtext/keyvalues.cpp`, and
  the suite source. g++ and clang++, `-std=c++20 -Wall -Wextra -Werror`,
  default and `-O2 -DNDEBUG`.
