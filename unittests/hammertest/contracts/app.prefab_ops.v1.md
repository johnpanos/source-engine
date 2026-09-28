# Contract: `app.prefab_ops.v1`

Module: `hammer.app`
Header: `public/hammer/app/ops/prefab_ops.h`, `public/hammer/app/fragment_io.h` ·
Impl: `hammer/core/app/ops/prefab_ops.cpp`, `hammer/core/app/fragment_io.cpp`
Conformance: `unittests/hammertest/app/test_prefab_ops.cpp`
Migration: `R08-DOMAIN`
Depends on: `app.clipboard` (`Copy`, `Paste`), `app.transform_ops`
(`TransformedSolid`, `TransformedEntity`), `app.decal_ops`
(`TransformedOverlay`), `ports.map_codec`, `ports.file_store`

## 1. Purpose, consumers, scope

Prefab insertion (legacy `CPrefab3D`, the Entity tool's prefab click and
`CMapDoc::OnInsertprefabOriginal`): a prefab file read as a detached
`MapFragment` and placed into the document. Consumers: the Entity tool, the
Insert Prefab command, the asset browser, the command layer and MCP. Prefab
libraries (`CPrefabLibrary`, RMF prefabs) and scale-to-box insertion are out
of scope. Required.

## 2. Accepted inputs

`LoadFragment`: an injected codec and file store and a path.
`FragmentFromDocument`: any document. `InsertPrefab`: a fragment, a finite
point, finite (pitch, yaw, roll) degrees, `group`, and an anchor
(`BoundsCenter`, the default, or `Origin`). `TransformedFragment`: any affine
map. `ExpandNameKeyword`: a name and a reader.

## 3. Results and guarantees

- `LoadFragment` fails with `ReadFailed` for an unreadable path and
  `DecodeFailed` for a codec error (with its line) or a file without objects;
  it never touches a session.
- Anchor: `BoundsCenter` puts the prefab's bounding-box center on the point
  (legacy Insert Prefab / `CreateAtPoint`); `Origin` puts the prefab's
  0 0 0 there (legacy Entity-tool click, `CreateAtPointAroundOrigin`). No
  legacy path rests the prefab's bottom on the clicked surface. The rotation
  turns the prefab about the anchor; solids keep texture alignment; overlays
  keep their basis on their faces.
- Insertion is `app::Paste` of the placed fragment: new runtime and VMF ids,
  overlay `sides` remapped, visgroups matched or created. With `group`, more
  than one top-level object is wrapped in one new group; one object never is
  (`CPrefab3D::Create`).
- `&i` keyword: the first `&i` in a targetname becomes one more than the
  highest number used as `<before>N<after>` in the target (case-insensitive,
  all digits); prefab key values and connection targets equal to the old
  name follow (`ExpandObjectKeywords`).
- Refusals: an empty prefab (`Nothing`); non-finite point or angles, a
  `BoundsCenter` prefab without extent, a degenerate rotation (`Rejected`),
  all before anything is staged.

## 4. Ownership, threading

Fragments are values; the operations are pure functions over the edit and
the ports. Single sequence; the codec and store follow their own contracts.

## 5. Invariants

Every insertion passes `ValidateEdit` and commits; VMF ids stay unique; the
prefab fragment is never modified.

## 6. Side effects and performance

Linear in the prefab size plus one scan of the target's entity names per
keyword name. `LoadFragment` reads one file.

## 7. Conformance suite and providers

`test_prefab_ops.cpp` (37 checks, gcc and clang, `-Wall -Wextra -Werror`),
with `FakeMapCodec` and `InMemoryFileStore`: fragment extraction, load
errors, both anchors, rotation about the center, grouping rule, keyword
expansion and references, overlay remap and basis, and negative checks that
stage nothing. Seeded faults detected: anchor offset dropped, overlay basis
not transformed.
