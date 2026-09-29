# Contract: `presenters.editor_workspace.v1`

Module: `hammer.presenters`
Header: `public/hammer/presenters/editor_workspace.h` · Impl: `hammer/core/presenters/editor_workspace.cpp`
Conformance: `unittests/hammertest/presenters/test_editor_workspace.cpp`
Migration: `R08-DOMAIN`

## 1. Purpose, consumers, scope

The one object a UI shell binds: one document's session, settings,
clipboard, commands, tools, four cameras, render snapshot and every
presenter, plus the input-routing policy. Consumers: the GTK shell, UI-driven
tests.

## 2. Accepted inputs

`WorkspaceServices` (codec, file store, map builder, entity catalog, material
info; each nullable and borrowed; `instanceRoots`, the game's instance paths). Normalized pointer, key and wheel events
per view; focus loss; frame time; viewport sizes; map paths.

## 3. Results and guarantees

- Composition only: providers come from the host; the class palette and
  material browser exist only with their ports. Every tool is registered;
  the Selection tool starts active; the face tool's lift writes
  `EditorSettings::faceTexture`.
- The render snapshot follows session events (incremental `Update` for
  edits/undo/redo, `Rebuild` on replacement, `SetSelection` on selection).
- Pointer: camera controller first (its capture cancels the tool gesture),
  then the tool manager with that view's camera. Keys: the camera first while
  it is driven, then action shortcuts (tool activation, host requests,
  commands; a tool gesture is cancelled before a command action), then the
  camera's keys, then the active tool; a disabled shortcut falls through and
  is reported when nothing handles it. Wheel: camera, then tool. Focus loss
  cancels camera drags and the tool gesture with no edit.
- Every input returns `InputOutcome` {handled, redraw, hostRequest, status,
  actionId}; host requests are "open_dialog", "save_dialog", "run_map" and
  host-target action names.
- `New`/`Open`/`Save`/`Build` run the commands and report through the status
  bar; replacement cancels the tool gesture (`DocumentReplaced`) and frames
  the cameras on the new map (the origin for an empty one).
- Instance content (R17 follow-up): with a codec and a store the workspace owns
  an `app::InstancePreview` (roots from `instanceRoots`) that the snapshot
  borrows; its document path is the map's (set before an Open's replacement, so
  the first extraction looks up from the new map; restored when the Open or New
  fails; updated after Save and Build). `RefreshInstances()` re-reads the
  instance files and, when any changed, re-extracts the instances. Without a
  codec or store there is no preview and no `InstanceDraw`.
- `SceneSerial()` changes on every snapshot change (edit, undo/redo,
  replacement, selection, instance refresh or path change), so hosts key their
  scene on it rather than on the document revision.

## 4. Ownership, threading

Owns everything it composes; members are destroyed in reverse order
(presenters and subscriptions before the session). Single sequence. Not
copyable.

## 5. Invariants

One session, one settings owner, one command authority and one snapshot per
workspace; the snapshot's revision tracks the session's.

## 6. Side effects and performance

Per event: the snapshot update plus each presenter's refresh (problems:
one `CheckMap` per document change).

## 7. Conformance suite and providers

`test_editor_workspace.cpp` (49 checks, gcc and clang) with the fake
catalog, materials, codec, file store and a fake builder: block by shortcut
and drag, undo/redo keys, 3D entity placement, inspector draft committed by
a selection change, save/reopen, the problems panel, host requests, builds,
camera pan/zoom/fly, focus loss and replacement mid-drag; negative checks for
disabled shortcuts, failed opens, unknown actions and missing services.
