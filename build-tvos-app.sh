#!/usr/bin/env bash
# Build the tvOS Portal client on Linux: build-apple-app.sh with the tvOS
# profile (quality/product_profiles/portal-tvos-native-vulkan.json), which
# owns the tvOS pins, SDK, bundle keys and the build-tvos/ out directory.
#
#   ./build-tvos-app.sh [build-apple-app.sh options]
#
# The AppleTVOS SDK comes from Xcode on the Mac (--fetch-sdk-from HOST).
# Content goes into the app's Library/Caches on the Apple TV, which the
# system may purge when storage runs low.

set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
exec "$ROOT/build-apple-app.sh" \
	--profile "$ROOT/quality/product_profiles/portal-tvos-native-vulkan.json" "$@"
