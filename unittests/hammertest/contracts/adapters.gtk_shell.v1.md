# Contract: `adapters.gtk_shell.v1`

Module: `hammer.adapters.gtk` · Shell: `hammer/adapters/gtk/hammer_gtk_shell.cpp`
Composition (headless, no GTK): `hammer::app::EditorDocument` +
`hammer::adapters::platform::DiskFileStore` + `hammer::formats`
Build recipe: `unittests/hammertest/adapters/build_gtk_shell.sh`
Conformance suite (workflow): `unittests/hammertest/adapters/test_shell_workflow.cpp`
Migration: `HAM-GTKSHELL-001`

This records what makes the GTK UI a **thin sibling** rather than a second editor:
the shell contains only toolkit glue, and its every action delegates to the
reusable, MFC-free libraries. The MFC shell (`hammer.adapters.mfc`) is its sibling
and drives the same libraries. The obligation is verified two ways because a GUI
cannot run on the headless CI profile.

## 1. Thinness obligation (what the shell may and may not contain)

- The shell **may** contain: GTK/libadwaita widget construction, signal handlers,
  and code that renders document state into widgets (`Refresh`).
- The shell **may not** contain: editor policy — content mutation, history/undo,
  save atomicity, serialization, or file I/O. Each of these must be a call into a
  reusable library. Concretely, every button handler is a one- or two-line
  delegation to `EditorDocument` / `IFileStore`, and no document state is duplicated
  in shell fields (the widgets are refreshed from the document, which is the one
  authority).

## 2. Behavioral obligation (what the delegated workflow must do)

Driving the composition the shell uses, the following must hold (verified by
`test_shell_workflow.cpp` with the real `DiskFileStore`, no GTK):

- **New**: seeds a fresh document; it reports **unmodified**.
- **Set Key**: commits exactly one edit; the document reports **modified** and
  **can undo**.
- **Save**: returns `SaveStatus::kOk`, persists a real file, and clears modified.
- **Open** (into a fresh document): reads the saved file back; content matches
  what was saved.
- **Undo / Redo**: restore the prior / next content snapshot exactly.

## 3. Build obligation (the sibling binds only to the reusable libraries)

`build_gtk_shell.sh` compiles and links the shell against
`gtk4` + `libadwaita-1` and the reusable-library sources (`disk_file_store`,
`editor_document`, `document_history`, `save_orchestrator`, `keyvalues`) — and
nothing else. A successful link with no other first-party objects is the evidence
that the shell holds no hidden editor logic and no MFC dependency. Verified with
gtk4 4.22 / libadwaita 1.9 (D4 pins: GTK >= 4.12, libadwaita >= 1.4). Running the
GUI needs a display and is not part of the headless gate.

## 4. Relationship to siblings and DRY

Both UI siblings render the same `EditorDocument` and persist through the same
`IFileStore`; the only per-sibling code is toolkit glue. No editor state, undo
stack, or provider catalog is duplicated between them (DRY). The Source-value
marshalling both need is the separate `hammer.adapters.source` seam
(`adapters.source.vector_interop.v1`).
