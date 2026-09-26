#!/usr/bin/env bash
# Build the iOS Portal 2 client on Linux and stage its content.
#
#   ./build-ios-portal2-app.sh [--steam-root DIR] [--no-content] [--restage] [build-ios-app.sh options]
#
# The Portal 2 profile (quality/product_profiles/portal2-ios-native-vulkan.json)
# extends the Portal iOS profile, so the pins, SDK and app shell stay in one
# place; build-ios-app.sh builds build-ios-p2/Portal2.app. The retail content
# is staged from the Steam installation by tools/quality/stage_portal2_runtime.py
# (VPKs, loose maps and materials, the retail search paths and the menu
# background) into build-ios-p2-content/, as symlinks into the installation.
# Sign, install and copy the content from the Mac with:
#
#   ./ios-deploy.sh --profile quality/product_profiles/portal2-ios-native-vulkan.json --with-content
#
# The first content copy moves about 12 GB; later ones skip unchanged files.

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROFILE="$ROOT/quality/product_profiles/portal2-ios-native-vulkan.json"
P2_STEAM_ROOT="${P2_STEAM_ROOT:-$HOME/.local/share/Steam/steamapps/common/Portal 2}"
STAGE="$ROOT/$(jq -er .content.stage_directory "$PROFILE")"

args=()
CONTENT=1
RESTAGE=0
while [ $# -gt 0 ]; do
	case "$1" in
	--steam-root) P2_STEAM_ROOT="$2"; shift ;;
	--no-content) CONTENT=0 ;;
	--restage) RESTAGE=1 ;;
	--profile) echo "error: $0 always uses $PROFILE" >&2; exit 2 ;;
	-h|--help)
		sed -n '2,16p' "$0"
		echo
		"$ROOT/build-ios-app.sh" --help
		exit 0 ;;
	*) args+=("$1") ;;
	esac
	shift
done

"$ROOT/build-ios-app.sh" --profile "$PROFILE" "${args[@]}"

if [ "$CONTENT" = 1 ]; then
	[ -f "$P2_STEAM_ROOT/portal2/pak01_dir.vpk" ] ||
		{ echo "error: no Portal 2 installation at $P2_STEAM_ROOT (--steam-root)" >&2; exit 1; }
	command -v vpk >/dev/null ||
		{ echo "error: missing host tool: vpk (pip install vpk); it extracts the menu image" >&2; exit 1; }
	[ "$RESTAGE" = 1 ] && rm -rf "$STAGE"
	printf '\033[1;36m==> %s\033[0m\n' "Staging Portal 2 content in $STAGE" >&2
	python3 "$ROOT/tools/quality/stage_portal2_runtime.py" --mount-custom \
		--steam-root "$P2_STEAM_ROOT" --runtime "$STAGE"
fi
