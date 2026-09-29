# Contract: `viewport.extraction.v1`

Module: `hammer.viewport` (reads `hammer.scene`, optional `hammer.ports`
`IEntityCatalog`)
Header: `public/hammer/viewport/extraction.h`, `public/hammer/viewport/view_policy.h`
· Impl: `hammer/core/viewport/extraction.cpp`, `hammer/core/viewport/view_policy.cpp`
Conformance: `unittests/hammertest/viewport/test_extraction.cpp`
Migration: R08-DOMAIN (supersedes the GTK shell's `WorldScene`-based scene
upload once it is hooked to `hammer.viewport`)

The render snapshot a renderer or presenter consumes (RFC 0002 "Rendering and
host contracts"): stable ids and derived geometry, material names, colors and
selection flags as values, with a revision-keyed cache.

## 1. Purpose, consumers, scope

- `Extract`, `SnapshotCache`, `ProjectEdges2D`; the color policy in
  `view_policy.h` (`SolidColor`, `EntityColor`, defaults).
- Consumers: the GTK host's renderer (3D faces, 2D wireframes), presenters
  (bounds for framing), future render providers. Required.
- Out of scope: textures and material resolution (a material name is a
  request), studio models, displacement surfaces, overlays, the drawing itself.

## 2. Accepted inputs

- A `DocumentReader`, a `SelectionInput` (object ids in any order, duplicates
  and stale ids allowed; selected `FaceRef`s), `ExtractOptions` (catalog,
  visibility predicate, `keepHidden`, marker half-size).
- `SnapshotCache::Update`: the document after a committed change set (forward
  or backward), that change set, the current selection, and the base and new
  caller revisions. A custom predicate must be a pure function of document
  content.

## 3. Results and guarantees

- One `SolidDraw` per shown solid with faces, one `EntityDraw` per shown point
  entity with a marker, both in id order; brush entities have no `EntityDraw`.
- Faces: `scene::BuildGeometry` polygons, vertices counter-clockwise from
  outside, the side's VMF id, material and outward normal.
- Colors: solids use their editor color, else the owning class's catalog color
  (default entity color when the class has none), else `kDefaultWorldColor`
  (0, 178, 178; the midpoint of legacy's random world colors). Entities use the
  catalog class color, else their editor color, else `kDefaultEntityColor`
  (220, 30, 220). Catalog components are rounded and clamped to 0..255.
- Entities: origin, angles (zero when absent), world marker box, the `model`
  key else the catalog model, the catalog sprite.
- Selection: an object is selected when it or any container (owning entity,
  enclosing groups) is selected; a face when its `FaceRef` is selected, which
  does not select the solid.
- Hidden objects are omitted, or with `keepHidden` included and flagged;
  `bounds` covers only what is not hidden.
- `SnapshotCache`: after `Rebuild` and every `Update`/`SetSelection`, the
  snapshot equals `Extract` of the same inputs. `Update` with a base revision
  other than the cache's rebuilds everything (`UpdateKind::Full`); otherwise it
  re-extracts only the changed objects, the solids of changed entities, the
  leaves of changed groups, the owners before/after of changed solids and the
  leaves of ids whose selection changed.
- `ProjectEdges2D`: unique edges (shared edges once), without edges that
  project to a point and without repeated screen segments.
- Models and instances (R17 follow-up, 2026-09-28): an `EntityDraw` whose model
  is a studio model (`IsStudioModelPath`: ends in `.mdl`, any case) carries
  `ModelKeys` read by `ReadModelKeys` (`skin` a non-negative integer else 0,
  `modelscale` a positive finite number else 1, `rendercolor` three 0..255
  integers else none); other entities keep the defaults. With
  `ExtractOptions::instances`, one `InstanceDraw` per shown `func_instance`
  (id order) holds the port's status, the resolved file (or the failure's
  detail), the content's solids (color: editor color, else the content owner's
  catalog color, else world) and point entities (content-local ids, brush
  entities as their solids), its content bounds, and selection/visibility of
  the instance (a selected instance or container selects all content). The
  instance keeps its `EntityDraw`; content is not in `bounds`. The cache
  re-extracts every `func_instance` when the port's `Revision()` differs from
  the one it last extracted with. `kInstanceTint` (255, 255, 128) and
  `kInstanceEdgeColor` (128, 128, 0) are the instance presentation policy
  (view_policy.h records the legacy overlay they replace).

## 4. Ownership, threading

- A snapshot owns all its data: no widget, GL or mutable scene pointers, so it
  can outlive document changes or cross threads. The cache owns its snapshot
  and selection copy; it is not synchronized. The catalog is borrowed for the
  cache's lifetime and must be immutable.

## 5. Invariants

- Snapshot vectors are sorted by id with no duplicates.
- Cached snapshot == full extraction for the cache's recorded inputs.
- Document settings are not drawn: a settings-only change set re-extracts
  nothing.

## 6. Side effects and performance

- `Extract` is O(objects) with geometry built once per solid. An incremental
  update rebuilds geometry only for its dirty set (plus an O(solids) owner scan
  and an O(draws) bounds pass); vector insertions are moves. The suite asserts
  the dirty-set size for a selection change and that a mixed change set
  re-extracts fewer objects than a rebuild.

## 7. Conformance suite and providers

- `test_extraction.cpp` (61 checks) with a local in-memory catalog: face
  winding, side ids and materials; every color rule; entity hints (catalog box,
  model key vs catalog model, sprite, clamped color); brush entities without
  entity draws; hidden and hidden-group filtering, `keepHidden`, predicate
  override and bounds; selection through owners and groups, face selection,
  stale/duplicate ids; open solids draw nothing; cache rebuild, a
  create/modify/remove/recolor/unhide/brush-conversion change set, undo,
  selection-only updates, revision mismatch, and 40 generated edits each equal
  to a full rebuild; box and wedge edge dedupe and 2D projection.
- `test_extraction_content.cpp` (`hammer.viewport.extraction.content`, 32
  checks): model keys and their defaults, catalog studio models, sprites; the
  instance draws through a real `InstancePreview` (`VmfMapCodec`, in-memory
  store); a hand-computed placement oracle that accepts the content and
  rejects four seeded wrong transforms; group selection, hidden and
  `keepHidden`; a port revision re-extracting exactly the instances; an
  instance move; cache equal to `Extract` throughout.
- Negative controls: a cache not yet updated, and one updated with an empty
  change set after a real commit, both fail the equality oracle.
- Runs headlessly with g++ and clang++ (`-std=c++20 -Wall -Wextra -Werror`).
