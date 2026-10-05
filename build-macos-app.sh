#!/usr/bin/env bash
# Build the macOS Portal client on Linux: build-apple-app.sh with the macOS
# profile (quality/product_profiles/portal-macos-native-vulkan.json), which
# owns the macOS pins, SDK, bundle keys and the build-macos/ out directory.
#
#   ./build-macos-app.sh [build-apple-app.sh options]
#
# The MacOSX SDK comes from Xcode on the Mac (--fetch-sdk-from HOST). The app
# reads its content from ~/Library/Application Support/<bundle id>/.

set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
exec "$ROOT/build-apple-app.sh" \
	--profile "$ROOT/quality/product_profiles/portal-macos-native-vulkan.json" "$@"
