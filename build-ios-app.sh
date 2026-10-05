#!/usr/bin/env bash
# Build the iOS Portal client on Linux: build-apple-app.sh with the iOS
# profile (quality/product_profiles/portal-ios-native-vulkan.json), which
# owns the iOS pins, SDK, bundle keys and the build-ios/ out directory.
#
#   ./build-ios-app.sh [build-apple-app.sh options]
#
# The iPhoneOS SDK comes from Xcode on the Mac (--fetch-sdk-from HOST).
# Content goes into the app's Documents folder on the device.

set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
exec "$ROOT/build-apple-app.sh" \
	--profile "$ROOT/quality/product_profiles/portal-ios-native-vulkan.json" "$@"
