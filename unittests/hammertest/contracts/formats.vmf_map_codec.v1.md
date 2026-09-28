# Contract: `formats.vmf_map_codec.v1`

Module: `hammer.formats`
Header: `public/hammer/formats/vmf_map_codec.h` · Impl: `hammer/core/formats/vmf_map_codec.cpp`
Implements: `ports.map_codec.v1` (`hammer::ports::IMapCodec`)
Conformance: `unittests/hammertest/formats/test_vmf_map_codec.cpp` (+ `_negative`),
shared oracle `unittests/hammertest/formats/map_codec_oracle.h`
Migration: `HAM-VMFCODEC-001` (proposed id; R08-DOMAIN)
Depends on: `content.keyvalues-text` (parser/writer), `hammer.scene`, `hammer.ports`

The full-fidelity Valve Map Format codec. It replaces the lossy
`EditorController::ToVmf`/`LoadVmf` pair (brush entities flattened, texture axes
regenerated, connections, groups and visgroups dropped) as the persistence of the
typed `MapDocument`. Strict, MFC-free, GPU-free.

## 1. Purpose, consumers, scope

- Consumers: whatever composition root wires `IMapCodec` for `hammer.app`
  (edit session open/save), `hammer_cli`, MCP, and tools.
- Scope: every VMF block legacy Hammer and Portal 2-era Hammer write:
  `versioninfo`, `visgroups` (nested), `viewsettings`, `world` (solids, groups,
  `hidden` wrappers), `entity` (keys, `connections`, brush solids, `hidden`
  solids, `editor`), top-level `hidden` entities, `cameras`, `cordon` (single
  form) and `cordons` (list form). Any other block is kept verbatim.

## 2. Accepted inputs

- Decode: any text. The keyvalues grammar is `content.keyvalues-text`'s (quoted
  strings, bare block names, `//` comments, CRLF or LF). Key/value pairs outside
  a block are rejected (`not VMF text`).
- Encode: any `MapDocument`.

## 3. Results and guarantees

- **Mapping.** `versioninfo`/`viewsettings` pairs → settings pairs; `visgroup`
  blocks → the visgroup tree; `world` `id` → `worldVmfId`, `classname
  worldspawn` implied, other pairs → `worldKeys` in order; solids, groups and
  hidden wrappers → objects; unknown world children → `worldExtraChildren`;
  `entity` → `Entity` (`id`, `classname`, other pairs in order with duplicates,
  connections through `scene::ParseConnection` keeping the separator, brush
  solids with `owner`); `editor` → `EditorInfo` (`color`, repeated
  `visgroupid`, `groupid` → the runtime group id, `visgroupshown`,
  `visgroupautoshown`, everything else in `extra`); `side` → `Side` (authored
  plane points exact, `uaxis`/`vaxis` `[x y z shift] scale`, rotation, lightmap
  scale, smoothing groups, `dispinfo` verbatim, other pairs/children kept);
  `cameras`, `cordon`, `cordons` → settings; other top-level blocks →
  `unknownBlocks` in order.
- **Runtime id order.** World solids (file order, hidden ones where they
  appear), then world groups (file order), then each entity followed by its
  solids (file order). Encode writes world solids in id order, then groups, then
  world extras, then entities in id order each with its solids in id order, so
  `Decode(Encode(Decode(t)))` issues identical ids. A world whose groups precede
  its solids, or extras between them, is reordered to this layout (content kept).
- **Warnings (recoverable, content kept):**
  - a solid with a malformed side (plane, axis, rotation, lightmap scale or
    smoothing groups) or with fewer than four sides → kept verbatim (in
    `worldExtraChildren` or the entity's `extraChildren`, inside its `hidden`
    wrapper when it had one), line of the side or solid;
  - an unparsable connection → the raw pair in a preserved `connections` block
    in the entity's `extraChildren`, line of the connections block;
  - a duplicate side id → a fresh id (the first side in id order keeps it);
  - missing ids → fresh ids (one summary warning); a non-numeric id → a fresh
    id; duplicate object ids are kept with a warning;
  - `groupid` naming no group → the membership is dropped; naming a group kept
    verbatim (one with children other than one `editor`) → the pair stays in
    `EditorInfo::extra`; a group cycle is broken at the group that closes it;
  - child blocks of `editor`, `versioninfo`, `viewsettings` or `connections`
    → kept in a separate verbatim block of the same name;
  - a second `world`/`versioninfo`/`visgroups`/`viewsettings`/`cameras`/cordon
    block, or a `visgroups`/`cameras`/`cordon(s)` block not in the expected form
    → kept verbatim in `unknownBlocks`.
  Fresh persistent ids are allocated after every id in the file (including ids
  inside verbatim content) was noted, so they never collide.
- **Encode layout** (legacy writer order): `versioninfo`, `visgroups`,
  `viewsettings` (always written), `world` (`id`, `mapversion` when it is the
  first world key, `classname`, other keys), entities (hidden ones wrapped
  inline), `cameras` (always), then `cordon` when the form is Single and it can
  hold the content (one unnamed cordon, one box, its flag equal to
  `cordonsActive`), else `cordons` when the form is not None or cordons exist,
  then `unknownBlocks`. Side keys: `id plane material uaxis vaxis rotation
  lightmapscale smoothing_groups` (signed, as legacy writes it), extras,
  `dispinfo`, extra children. Editor keys: `color groupid visgroupid…
  visgroupshown visgroupautoshown` then extras; the editor block is omitted when
  it is all defaults, the object has no group and no preserved `editor` block.
- **Numbers.** `scene::FormatNumber` (`%.10g`) when that text reads back to the
  same double, else the shortest exact decimal (`std::to_chars`); every
  coordinate, axis, shift, scale and connection delay round-trips bit for bit.
  Numbers are parsed locale-independently (`std::from_chars`).
- **Normalizations** (content preserved, text changes): missing standard side
  keys are written with their defaults; `visgroupshown`/`visgroupautoshown` are
  written whenever an editor block is; number spellings are canonical (e.g.
  MSVC's `1.52588e-005` → `1.52588e-05`); pair/child interleaving becomes pairs
  first (as the keyvalues writer always does).
- **Encode fails** for: a `"` in any key, value or block name; a block name that
  is empty, starts with `//` or holds blanks or braces; a non-finite number; a
  connection that does not read back to itself (a field holding its separator,
  or a separator other than `,`/0x1B); a solid owned by a missing entity; a
  reference to a missing group.

## 4. Ownership, threading

- Stateless and pure; `const` methods; safe on any thread for distinct inputs.

## 5. Invariants

- Decoded documents pass `MapDocument::Validate()` (checked before returning; a
  violation is reported as an internal `CodecError`, never returned).
- Nothing in the input is dropped silently: every datum is modeled, kept
  verbatim, or named in a warning (only unresolvable `groupid` references and
  malformed ids are dropped, each with a warning).

## 6. Side effects and performance

- No I/O, no logging. One parse, one pass to map blocks to lines (for
  warnings), one pass to build objects. The Portal 2 maps
  `sp_a2_trust_fling.vmf` (1.5 MB, 764 solids, 455 entities, 25 groups) and
  `zoo_mechanics.vmf` decode with zero warnings.

## 7. Conformance suite and providers

- `test_vmf_map_codec.cpp` (53 checks): the shared oracle's clauses (canonical
  fixture field by field and byte for byte, both cordon forms, the oddity
  fixture, an edited document with runtime ids out of write order, an empty
  document, cordon-form normalization), warning text and lines per oddity,
  exact-number output, verbatim groups/editor children, parse errors with lines,
  foreign text, each Encode failure, and every `hammer/gtk/samples/*.vmf`
  (round trip; `room.vmf` also semantically equal under
  `kvtext::CompareKeyValues`). With `HAMMER_VMF_CORPUS=<dir>` it also round-trips
  every `.vmf` there (numbers compared by value); not part of the required run.
- `test_vmf_map_codec_negative.cpp` (37 checks): 14 seeded bad codecs, each
  detected by its named oracle clause; the real codec is the control.
- Sources: `hammer/core/formats/vmf_map_codec.cpp`,
  `hammer/core/scene/{map_objects,map_document,map_queries,solid_geometry}.cpp`,
  `mapgeometry/{brush,polytope,texture_axes}.cpp`, `kvtext/keyvalues.cpp`, and
  the suite source. g++ and clang++, `-std=c++20 -Wall -Wextra -Werror`,
  default and `-O2 -DNDEBUG`.
