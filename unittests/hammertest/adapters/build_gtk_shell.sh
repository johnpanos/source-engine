#!/usr/bin/env bash
#
# Builds the thin GTK4/libadwaita Hammer UI sibling (hammer.adapters.gtk) against
# the reusable, MFC-free editor libraries. A successful compile+link proves the
# sibling shell binds only to the reusable libraries + the toolkit; it holds no
# editor logic of its own. Running the GUI needs a display; the shell's delegated
# behavior is covered headlessly by unittests/hammertest/adapters/test_shell_workflow.cpp.
#
# Requires: gtk4 and libadwaita-1 development packages (pkg-config). RFC 0002 D4
# pins GTK >= 4.12, libadwaita >= 1.4; this repo has been verified with 4.22 / 1.9.
set -euo pipefail

REPO="${REPO:-$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)}"
OUT="${1:-$REPO/build/hammer-gtk}"
CXX="${CXX:-g++}"

if ! pkg-config --exists gtk4 libadwaita-1; then
  echo "gtk4 / libadwaita-1 development packages not found (pkg-config)." >&2
  exit 2
fi
echo "gtk4 $(pkg-config --modversion gtk4), libadwaita-1 $(pkg-config --modversion libadwaita-1)"

mkdir -p "$(dirname "$OUT")"
cd "$REPO"
$CXX -std=c++20 -Wall -Wextra $(pkg-config --cflags gtk4 libadwaita-1) -I public -I . \
  hammer/adapters/gtk/hammer_gtk_shell.cpp \
  hammer/adapters/platform/disk_file_store.cpp \
  hammer/core/app/editor_document.cpp \
  hammer/core/app/entity_selection.cpp \
  hammer/core/app/property_value.cpp \
  hammer/core/app/document_history.cpp \
  hammer/core/app/save_orchestrator.cpp \
  hammer/core/formats/keyvalues.cpp \
  $(pkg-config --libs gtk4 libadwaita-1) -o "$OUT"

echo "built thin GTK sibling: $OUT"
file "$OUT"
