#!/usr/bin/env bash
# Build the iOS Portal 2 client on Linux and stage its content.
#
#   ./build-ios-portal2-app.sh [--profile FILE] [--no-content] [--restage] [build-apple-app.sh options]
#
# --profile selects another Portal 2 Apple profile (build-tvos-portal2-app.sh
# passes quality/product_profiles/portal2-tvos-native-vulkan.json).
#
# The Portal 2 profile (quality/product_profiles/portal2-ios-native-vulkan.json)
# extends the Portal iOS profile, so the pins, SDK and app shell stay in one
# place; build-apple-app.sh builds build-ios-p2/Portal2.app. The retail content
# is packaged from the Steam installation by `kiln package portal2-content`
# (VPKs, loose maps and materials, the retail search paths and the menu
# background) into build-ios-p2-content/, as symlinks into the installation.
# kiln finds the installation through its steam-portal2 location; another
# installation goes in .kiln/local.json's content_locations.
# Sign, install and copy the content from the Mac with:
#
#   ./ios-deploy.sh --profile quality/product_profiles/portal2-ios-native-vulkan.json --with-content
#
# The first content copy moves about 12 GB; later ones skip unchanged files.

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROFILE="$ROOT/quality/product_profiles/portal2-ios-native-vulkan.json"

args=()
CONTENT=1
RESTAGE=0
while [ $# -gt 0 ]; do
	case "$1" in
	--no-content) CONTENT=0 ;;
	--restage) RESTAGE=1 ;;
	--profile) PROFILE="$(realpath "$2")"; shift ;;
	-h|--help)
		sed -n '2,19p' "$0"
		echo
		"$ROOT/build-apple-app.sh" --help
		exit 0 ;;
	*) args+=("$1") ;;
	esac
	shift
done

STAGE="$ROOT/$(jq -er .content.stage_directory "$PROFILE")"
"$ROOT/build-apple-app.sh" --profile "$PROFILE" "${args[@]}"

if [ "$CONTENT" = 1 ]; then
	[ "$RESTAGE" = 1 ] && rm -rf "$STAGE"
	printf '\033[1;36m==> %s\033[0m\n' "Staging Portal 2 content in $STAGE" >&2
	"$ROOT/kiln" package portal2-content --runtime "$STAGE"
fi
