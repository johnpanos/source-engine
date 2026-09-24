#!/usr/bin/env bash
# On-device test of the Android gyroscope thread (inputsystem/gyro_sensor.cpp).
#
#   tools/quality/android_gyro_device_test.sh [--serial SERIAL]
#
# Builds unittests/inputtest/android/test_gyro_sensor_device.cpp with the
# pinned NDK, pushes it to /data/local/tmp and runs it from adb shell: no APK,
# game or screen is involved, so a locked device works. Lay the device still.
# The test prints a checks-v1 record (CONFORMANCE <checks> <failures>) and
# this script exits with its status.

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
PROFILE="$ROOT/quality/product_profiles/portal-android-native-vulkan.json"
SERIAL="${ANDROID_SERIAL:-}"
while [ $# -gt 0 ]; do
	case "$1" in
	--serial) SERIAL="$2"; shift ;;
	-h|--help) sed -n '2,10p' "$0"; exit 0 ;;
	*) echo "unknown option: $1" >&2; exit 2 ;;
	esac
	shift
done
[ -n "$SERIAL" ] && export ANDROID_SERIAL="$SERIAL"

# The same NDK and minimum API level as the APK (build-android-apk.sh).
NDK="$ROOT/dependencies/android/$(jq -er '.dependencies.ndk.extracted_directory' "$PROFILE")"
MIN_SDK="$(jq -er '.android.min_sdk' "$PROFILE")"
CXX="$NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android${MIN_SDK}-clang++"
[ -x "$CXX" ] || { echo "error: no NDK compiler at $CXX (run build-android-apk.sh --fetch-only)" >&2; exit 1; }

OUT="$ROOT/build-android/gyro-device-test"
mkdir -p "$OUT"
BIN="$OUT/test_gyro_sensor_device"
"$CXX" -std=c++20 -O2 -g -Wall -fPIE -pie -static-libstdc++ \
	-DANDROID=1 -D_ANDROID=1 -DLINUX=1 -D_LINUX=1 -DPOSIX=1 -D_POSIX=1 -DPLATFORM_64BITS=1 \
	-DUSE_SDL3=1 -DGNUC -DNO_HOOK_MALLOC -DNO_MEMOVERRIDE_NEW_DELETE=1 -DNDEBUG \
	-I"$ROOT" -I"$ROOT/public" -I"$ROOT/public/tier0" -I"$ROOT/common" \
	"$ROOT/unittests/inputtest/android/test_gyro_sensor_device.cpp" \
	"$ROOT/inputsystem/gyro_sensor.cpp" "$ROOT/inputsystem/gyro_math.cpp" \
	-landroid -llog -o "$BIN"

adb push "$BIN" /data/local/tmp/test_gyro_sensor_device >/dev/null
adb shell /data/local/tmp/test_gyro_sensor_device
