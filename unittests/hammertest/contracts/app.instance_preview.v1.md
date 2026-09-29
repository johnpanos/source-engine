# Contract: `app.instance_preview.v1`

Module: `hammer.app` · Header: `public/hammer/app/instance_preview.h`
Impl: `hammer/core/app/instance_preview.cpp` · Port: `public/hammer/ports/instance_content.h`
Conformance: `unittests/hammertest/app/test_instance_preview.cpp` (`hammer.app.instance_preview`)
Migration: `R17` (props and instances in the viewports)

## Purpose

The viewports draw each `func_instance`'s content without it ever entering the
document: `InstancePreview` implements `ports::IInstanceContent` over the
injected codec and file store, and the viewport extraction turns its answer into
`InstanceDraw`s (`viewport.extraction.v1`). Consumer: `EditorWorkspace`, which
owns one when it has a codec and a store.

## Obligations

- Lookup follows vbsp `CMapFile::DeterminePath`: the directory of the file that
  holds the instance, then its enclosing `maps/` directory (case-insensitive),
  then the search roots in order. The file name is `ops::InstanceFile`'s
  (`/` separators, `.vmf` added).
- Placement is the collapse rule's (`ops::PlaceInstanceContent`, shared with
  `CollapseInstance`): parameters, name fixup, rotation then translation.
  Quick-hidden objects (and the solids of a hidden brush entity) are left out.
- Nested `func_instance`s are replaced by their placed content (their files
  looked up from the parent file), merged with fresh content-local ids whose
  owner and group references follow.
- Each file is decoded once; each distinct instance (holding file, class, keys)
  is placed once. `Refresh()` re-reads every file read and repeats every lookup;
  any difference forgets everything and bumps `Revision()`, as do a new document
  path or new roots.

## Inputs and outputs

`Content(entity)` → `shared_ptr<const InstanceContent>`: `status`, resolved
`file`, `detail`, world-space `objects`, `nestedFailures`; null for a
non-instance.

## Failure behavior

`NoFile`, `NotFound` (detail: the file), `DecodeFailed` (the codec's error),
`Rejected` (the collapse rule's refusal), `Cycle` (a file that includes itself,
or deeper than `maxDepth`). A failing nested instance is counted in
`nestedFailures` and left out; it never fails the parent. Nothing is written.

## Lifetime and threading

One sequence (the session's). Borrows the codec, store and catalog, which must
outlive it. Returned content is immutable and may outlive later refreshes.

## Evidence and oracles

`hammer.app.instance_preview`: real VMF text through `VmfMapCodec` in an
in-memory store. Placement checked against hand-computed boxes (a yaw of 90
degrees) and against `CollapseInstance` on the same content; the three lookup
rules and their order; each status; nesting with unique ids; cycles terminate;
decode and placement counters; `Refresh` after a changed and an appearing file;
path changes. The extraction side (`InstanceDraw`) is covered by
`hammer.viewport.extraction.content`.
