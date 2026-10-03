# Contract: `app.replace_textures.v1`

Modules: `hammer.app` (operation, commands) and `hammer.presenters` (dialog model)
Headers: `public/hammer/app/ops/texture_ops.h` (Replace Textures section),
`public/hammer/presenters/replace_textures.h` · Impl:
`hammer/core/app/ops/texture_ops.cpp`, `hammer/core/app/session_commands.cpp`
(`replace_material`, `mark_material`), `hammer/core/presenters/replace_textures.cpp`
Host: `hammer/gtk/replace_textures_dialog.cpp` (Tools > Replace Textures..., and
Replace... in the Texture Application window)
Conformance: `unittests/hammertest/app/test_texture_ops.cpp`,
`unittests/hammertest/app/test_session_commands.cpp`,
`unittests/hammertest/presenters/test_replace_textures.cpp`,
`unittests/hammertest/legacy/test_replace_textures_parity.cpp`,
`tools/quality/hammer_ui_test.py` cases `replace-textures` and `replace-textures-cancel`
Migration: `HAM-REPLACETEX-001` (R08-REPLACE-TEXTURES)

## 1. Purpose, consumers, scope

Legacy Hammer's Replace Textures workflow (`CReplaceTexDlg`,
`CMapDoc::ReplaceTextures`, `ReplaceTexFunc`), retired from the MFC shell:
replace or mark the faces that use a material, everywhere or within the
selected objects. Consumers: the GTK host's dialog, scripts and agents through
the command layer. Required wherever the editor edits textures.

## 2. Accepted inputs

A query: Find, Replace, a match mode (Exact, Partial, Substitute: legacy actions
0, 1, 2), a scope (the ids a selection stands for, or every solid), hidden
objects, rescale; for mark only, a target (solids, or faces when the face tool
is active). Rescale needs the `IMaterialInfo` port.

## 3. Results and guarantees

- Names follow RFC 0015's identity (`content.asset-identity`, the one owner):
  ASCII case-insensitive, `\` equal to `/`. Stored names are the normalized
  identity. A result that is not a valid name refuses the operation.
- Exact compares whole names; Partial and Substitute find the first occurrence.
  Partial sets the whole name; Substitute replaces the occurrence. An empty
  Find matches nothing.
- Hidden solids (`scene::IsVisible`) are skipped unless asked for; marking never
  includes them.
- Rescale keeps texel density: per axis, scale times old/new size and shift
  divided by it; a missing size refuses the whole operation.
- One `Replace Textures` history step per replacement; the count of faces
  changed is reported; no match is Nothing and records nothing. Marking is a
  selection change (no history).
- The dialog model reports legacy's messages: "N textures replaced.",
  "N solids marked.", "N faces marked.".

Recorded deviations from legacy (asserted by the parity suite):

- D1: the menu and Face Edit entry points cleared the selection before
  marking within it and marked nothing; here marking within the selection
  marks within it (legacy's texture bar path did).
- D2: rescale with an unknown size divided by a placeholder's zero size;
  here it refuses.
- D3: a substitution giving an invalid name was stored; here it refuses.
- D4: a replacement that changes nothing set the modified flag and kept an
  undo step; here the session records nothing unless a stored spelling
  changed.

## 4. Ownership, threading

The operation is a pure function over a borrowed `scene::DocumentEdit`; the
session commits it. The dialog model borrows the session and commands and
holds only its draft. Single sequence.

## 5. Invariants

A refusal stages nothing. Every successful edit passes `scene::ValidateEdit`.
The GTK host holds widgets only; every rule lives in the model or the
operation.

## 6. Side effects and performance

The staged edit, or the selection. Cost is proportional to the faces in scope.

## 7. Conformance suite and providers

- `hammer.app.texture_ops`, `hammer.app.session_commands`: the operation and
  commands, with refusals.
- `hammer.presenters.replace_textures`: the model's defaults, validation,
  preview, messages, history, marking and save/reopen through the real VMF
  codec.
- `hammer.app.replace_textures.legacy_parity`: the frozen legacy menu path
  against the model on 1,500 seeded cases, D1–D4 asserted, seven seeded defects
  detected; `hammer.legacy.replace_textures.freeze` pins the frozen copy to its
  git revision.
- `corpus.hammer.ui` cases `replace-textures` and its Cancel control: the real
  GTK editor driven through the menu, judged on saved maps and frames.
