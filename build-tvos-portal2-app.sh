#!/usr/bin/env bash
# Build the tvOS Portal 2 client on Linux and stage its content:
# build-ios-portal2-app.sh with the tvOS Portal 2 profile
# (quality/product_profiles/portal2-tvos-native-vulkan.json), which extends
# the Portal tvOS profile and builds build-tvos-p2/Portal2.app.
#
#   ./build-tvos-portal2-app.sh [build-ios-portal2-app.sh options]
#
# Sign, install and copy the content from the Mac with:
#
#   ./ios-deploy.sh --profile quality/product_profiles/portal2-tvos-native-vulkan.json --device tv --with-content
#
# Content goes into the app's Library/Caches on the Apple TV, which the
# system may purge when storage runs low.

set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
exec "$ROOT/build-ios-portal2-app.sh" \
	--profile "$ROOT/quality/product_profiles/portal2-tvos-native-vulkan.json" "$@"
