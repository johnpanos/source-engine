#!/usr/bin/env bash
#
# Builds the thin MFC UI sibling (hammer.adapters.mfc) with the real MSVC+MFC
# toolchain and verifies it under Wine. This is the sibling of the GTK shell:
# both are thin UI layers over the SAME reusable, engine-free editor libraries.
#
# Crucially, this NEW thin MFC sibling depends only on MFC + the reusable
# libraries -- NOT on tier0/vgui2/materialsystem -- so unlike the legacy Hammer
# MFC shell it builds and links against the extracted libraries alone. It is a
# LOCAL gate: the MSVC+MFC toolchain (mstorsjo/msvc-wine, atlmfc) is a large
# licensed install and is not in CI. See architecture/hammer_mfc_parity.json.
#
# Two artifacts:
#   1. hammer_mfc_shell.cpp  -> a real PE32+ GUI app (compile+link proof; a GUI
#      needs a display to run, like the GTK shell).
#   2. test_mfc_workflow.cpp -> a console MFC-linked binary that RUNS under Wine
#      and exercises New/SetKey/Save/Open/Undo/Redo through EditorDocument +
#      DiskFileStore, proving MFC and the reusable libraries coexist and the
#      delegated workflow executes.
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO="${REPO:-$(cd "$HERE/../../.." && pwd)}"
MSVC_BIN="${MSVC_BIN:-$REPO/../scratchpad/msvc-dest/bin/x64}"
OUTDIR="${OUTDIR:-$(mktemp -d)}"

if [ ! -x "$MSVC_BIN/cl" ]; then
  echo "MSVC toolchain not found at $MSVC_BIN (set MSVC_BIN). Provisioned via mstorsjo/msvc-wine." >&2
  exit 2
fi
export PATH="$MSVC_BIN:$PATH"
export WINEDEBUG="${WINEDEBUG:--all}"

mkdir -p "$OUTDIR"
DEFS="$OUTDIR/mfc_defines.h"
printf '#pragma once\n#define _AFXDLL 1\n#define _CRT_SECURE_NO_WARNINGS 1\n#define _CRT_NONSTDC_NO_DEPRECATE 1\n' > "$DEFS"

LIBS=(
  "$REPO/hammer/adapters/platform/disk_file_store.cpp"
  "$REPO/hammer/core/app/editor_document.cpp"
  "$REPO/hammer/core/app/document_history.cpp"
  "$REPO/hammer/core/app/save_orchestrator.cpp"
  "$REPO/hammer/core/app/property_value.cpp"
  "$REPO/hammer/core/formats/keyvalues.cpp"
)
CL=(cl /nologo /EHsc /std:c++20 /MD "/FI$DEFS" -I "$REPO/public" -I "$REPO")

cd "$OUTDIR"

echo "== 1. thin MFC GUI sibling: compile + link =="
"${CL[@]}" "$REPO/hammer/adapters/mfc/hammer_mfc_shell.cpp" "${LIBS[@]}" \
  /Fe:mfc_shell.exe /link /SUBSYSTEM:WINDOWS >/dev/null
file mfc_shell.exe

echo "== 2. MFC-linked workflow oracle: build + run under Wine =="
"${CL[@]}" "$REPO/unittests/hammertest/adapters/test_mfc_workflow.cpp" "${LIBS[@]}" \
  /Fe:mfc_wf.exe >/dev/null
wine mfc_wf.exe
RC=$?

echo "mfc sibling: shell links, workflow exit $RC (expect 0)"
[ "$RC" -eq 0 ] && echo "MFC SIBLING OK" || { echo "MFC SIBLING FAILED"; exit 1; }
