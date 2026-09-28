#!/bin/sh
# Builds the GTK4 + libadwaita Hammer shell (RFC 0002, linux-gtk-desktop) and
# copies it to [output-binary]. The shell is the Waf target hammer_gtk of the
# tools product, so it links the Waf-built editor libraries and the RFC 0016
# render core (with its pinned Vulkan Memory Allocator and shaders) instead of
# compiling its own copies. It builds in its own tools tree (build-hammer-gtk
# unless HAMMER_GTK_TREE names another), configured with
# --render-core-vulkan=on when new or older than a wscript; later runs rebuild
# incrementally.
#
# Requires: gtk4, libadwaita-1 and the Vulkan loader (pkg-config), and what the
# tools product needs.
# Usage: hammer/gtk/build.sh [output-binary]   (run from anywhere)
set -eu

ROOT="$( cd "$( dirname "$0" )/../.." && pwd )"
OUT="${1:-$ROOT/hammer/gtk/hammer_gtk}"
# KTX2 material previews (RFC 0008 F3) when both roots of the pinned
# KTX-Software build are given; they build in a tree of their own.
KTX_ARGS=""
if [ -n "${KTX_SOURCE_ROOT:-}" ] || [ -n "${KTX_BUILD_ROOT:-}" ]; then
	if [ -z "${KTX_SOURCE_ROOT:-}" ] || [ -z "${KTX_BUILD_ROOT:-}" ]; then
		echo "set both KTX_SOURCE_ROOT and KTX_BUILD_ROOT for KTX2 previews" >&2
		exit 2
	fi
	KTX_ARGS="--ktx-source-root=$KTX_SOURCE_ROOT --ktx-build-root=$KTX_BUILD_ROOT"
	TREE="${HAMMER_GTK_TREE:-build-hammer-gtk-ktx}"
else
	TREE="${HAMMER_GTK_TREE:-build-hammer-gtk}"
fi
cd "$ROOT"
export WAFLOCK=".lock-waf-$( basename "$TREE" )"
LOG="$ROOT/$TREE.log"

# Configure when the tree is new, left the shell out, or is older than a build
# script (a changed wscript can add configure-time steps, such as the shader
# artifacts of RFC 0016 K4).
stale() {
	[ ! -f "$TREE/c4che/_cache.py" ] && return 0
	grep -q "^HAMMER_GTK = True" "$TREE/c4che/_cache.py" || return 0
	[ -n "$( find . -path ./build -prune -o -path "./$TREE" -prune -o -path './build-*' -prune \
		-o -path ./.git -prune -o -name wscript -newer "$TREE/c4che/_cache.py" -print -quit )" ]
}
if stale; then
	# shellcheck disable=SC2086
	if ! ./waf configure --tools --disable-warns -T release -o "$TREE" --render-core-vulkan=on \
		--prefix="$ROOT/$TREE/install" $KTX_ARGS >"$LOG" 2>&1; then
		tail -30 "$LOG" >&2
		echo "hammer/gtk/build.sh: configure failed (log: $LOG)" >&2
		exit 1
	fi
	if ! grep -q "^HAMMER_GTK = True" "$TREE/c4che/_cache.py"; then
		grep -i "hammer gtk shell\|vulkan loader" "$LOG" >&2 || true
		echo "hammer/gtk/build.sh: the tools product left the GTK shell out (log: $LOG)" >&2
		exit 1
	fi
fi
if ! ./waf build --targets=hammer_gtk >>"$LOG" 2>&1; then
	grep -E "error|Error" "$LOG" | tail -30 >&2
	echo "hammer/gtk/build.sh: build failed (log: $LOG)" >&2
	exit 1
fi
if [ "$( realpath -m "$OUT" )" != "$( realpath -m "$TREE/hammer/gtk/hammer_gtk" )" ]; then
	cp "$TREE/hammer/gtk/hammer_gtk" "$OUT"
fi
echo "built $OUT"
