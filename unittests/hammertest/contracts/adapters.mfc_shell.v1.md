# Contract: `adapters.mfc_shell.v1`

Module: `hammer.adapters.mfc` · Shell: `hammer/adapters/mfc/hammer_mfc_shell.cpp`
Workflow oracle: `unittests/hammertest/adapters/test_mfc_workflow.cpp`
Build/verify: `unittests/hammertest/adapters/build_mfc_shell.sh`
Evidence: `architecture/hammer_mfc_parity.json`
Migration: `HAM-MFCSHELL-001`

The sibling of `adapters.gtk_shell.v1`: a **thin MFC UI shell** that holds only
toolkit glue (a `CWinApp` / `CFrameWnd`, a menu, and command handlers) and
delegates every editor decision to the same reusable, engine-free libraries. It is
a NEW shell, not the legacy Hammer MFC shell, so it depends only on MFC + the
reusable libraries — not on tier0/vgui2/materialsystem — and therefore builds and
links against the extracted libraries alone.

## 1. Thinness obligation

- The shell **may** contain: MFC window/menu/command-handler code and code that
  renders document state into MFC controls.
- The shell **may not** contain editor policy — content mutation, history/undo,
  save atomicity, or serialization. Each `ON_COMMAND` handler
  (`OnNew`/`OnApplyEdit`/`OnSave`/`OnOpen`/`OnUndo`/`OnRedo`) is a one- or two-line
  delegation to `EditorDocument` / `IFileStore`; the `CEdit` view is refreshed from
  the document, which is the single authority. No document state is duplicated in
  frame fields.

## 2. Behavioral obligation (delegated workflow)

Driving the composition the shell uses, verified by `test_mfc_workflow.cpp` linked
against MFC and run under Wine:

- **New**: seeds a fresh document; reports **unmodified**.
- **Set Key**: commits one edit; reports **modified** and **can undo**.
- **Save**: `SaveStatus::kOk`, persists a real file via `DiskFileStore`, clears modified.
- **Open** (fresh document): reads the saved file back; content matches.
- **Undo / Redo**: restore the prior / next snapshot exactly.

## 3. Build obligation (the sibling binds only to MFC + the reusable libraries)

`build_mfc_shell.sh` compiles and links `hammer_mfc_shell.cpp` with the real
MSVC+MFC toolchain (cl 19.29, atlmfc; shared MFC via `mfc140.dll`) against MFC and
the reusable-library sources (`disk_file_store`, `editor_document`,
`document_history`, `save_orchestrator`, `property_value`, `keyvalues`) — and
nothing else. A successful link to a PE32+ GUI executable with no engine objects is
the evidence that the shell carries no hidden editor logic and no Source-engine
dependency. Running the GUI needs a display and is not part of this gate; the
console workflow oracle covers the delegated behavior at runtime.

## 4. Relationship to the legacy MFC shell and to the GTK sibling

- This is **not** the legacy Hammer shell (`CMapDoc`/`CMapView` bound to the whole
  Source Windows engine, tracked as `windows-mfc-legacy` = unverified). It is a new
  thin sibling that proves the factored architecture on the MFC side.
- It is the mirror of `adapters.gtk_shell.v1`: both render the same
  `EditorDocument`, persist through the same `DiskFileStore`, and serialize via the
  same `hammer::formats` codec. The only per-sibling code is toolkit glue (DRY).
- **Local gate:** the MSVC+MFC toolchain is not in CI. The MFC-free core's Windows
  behavior is covered reproducibly by the MinGW+Wine lane
  (`hammer_windows_parity.json`); this contract adds the MFC-linked shell proof
  on top, run locally under Wine.
