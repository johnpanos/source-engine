#!/usr/bin/env bash
# Build the iOS or tvOS Portal client on Linux from a pinned Apple profile.
#
#   ./build-ios-app.sh [--profile FILE] [options]
#
# The product profile (default quality/product_profiles/portal-ios-native-vulkan.json;
# build-tvos-app.sh passes portal-tvos-native-vulkan.json) owns every pin:
# the host toolchain, SDL3, KTX, MoltenVK, the deployment target, the bundle
# id and the build directory, directly or through a pin_source. Its
# target.os (ios or tvos) selects the SDK, triple and Info.plist keys. This
# script orchestrates:
#
#   1. the host toolchain (tools/ios/build_toolchain.py: clang, ld64.lld and
#      Apple's ld64 for module objects)
#   2. the iPhoneOS or AppleTVOS SDK: --sdk DIR, or copied once from a Mac with
#      --fetch-sdk-from HOST (ssh; read-only on the Mac)
#   3. fetch + verify the pinned SDL3, KTX-Software and MoltenVK archives
#   4. cross-build the native dependencies with CMake into a prefix (SDL3,
#      the static KTX reader, freetype, libpng, libjpeg and curl from the
#      thirdparty submodule)
#   5. configure and build the engine with Waf (--ios-sdk --static-composition)
#      under a private lock and out directory
#
# Signing and installing happen on the Mac. Everything is written under the
# profile's build directory (build-ios/ or build-tvos/, gitignored) and
# dependencies/<os>/; the shared host toolchain is in dependencies/ios/.

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROFILE="$ROOT/quality/product_profiles/portal-ios-native-vulkan.json"
# Bump when a dependency recipe below changes, to force those rebuilds.
DEPS_RECIPE=2

SDK=""
FETCH_SDK_FROM=""
FETCH_ONLY=0
DEPS_ONLY=0
CLEAN=0
JOBS="$(nproc)"

usage()
{
	cat <<EOF
Usage: $0 [options]

  --profile FILE          Apple product profile
                          (default: quality/product_profiles/portal-ios-native-vulkan.json)
  --sdk DIR               iPhoneOS.sdk or AppleTVOS.sdk to build against
                          (default: dependencies/<os>/sdk/<SDK>)
  --fetch-sdk-from HOST   copy the SDK from Xcode on HOST over ssh first
  --fetch-only            only fetch and verify pinned archives
  --deps-only             stop after the native dependencies
  --clean                 remove build-ios/ first
  -j N                    parallel jobs (default: $JOBS)
  -h, --help              this help

The SDK comes from Xcode on the user's Mac (xcrun --sdk iphoneos|appletvos
--show-sdk-path); it is never downloaded from elsewhere.
EOF
}

while [ $# -gt 0 ]; do
	case "$1" in
	--profile) PROFILE="$(realpath "$2")"; shift ;;
	--sdk) SDK="$(realpath "$2")"; shift ;;
	--fetch-sdk-from) FETCH_SDK_FROM="$2"; shift ;;
	--fetch-only) FETCH_ONLY=1 ;;
	--deps-only) DEPS_ONLY=1 ;;
	--clean) CLEAN=1 ;;
	-j) JOBS="$2"; shift ;;
	-j*) JOBS="${1#-j}" ;;
	-h|--help) usage; exit 0 ;;
	*) echo "unknown option: $1" >&2; usage >&2; exit 2 ;;
	esac
	shift
done

log() { printf '\033[1;36m==> %s\033[0m\n' "$*" >&2; }
die() { printf '\033[1;31merror: %s\033[0m\n' "$*" >&2; exit 1; }
need() { command -v "$1" >/dev/null 2>&1 || die "missing host tool: $1"; }

for tool in jq curl sha256sum tar cmake ninja python3 pkg-config; do
	need "$tool"
done
[ -f "$PROFILE" ] || die "missing profile $PROFILE"
# The profile with its "extends" chain resolved (a derived product such as
# Portal 2 names the profile that owns the shared pins); profile_extends.py
# owns the rule.
PROFILE_SOURCE="$PROFILE"
PROFILE="$(mktemp --suffix=.json)"
trap 'rm -f "$PROFILE"' EXIT
python3 "$ROOT/tools/quality/profile_extends.py" resolve "$PROFILE_SOURCE" > "$PROFILE" ||
	die "could not resolve $PROFILE_SOURCE"
p() { jq -er "$1" "$PROFILE"; }

# pin_profile JQ_PATH: the profile that owns a pin, following pin_source.
pin_profile()
{
	local source
	source="$(jq -r "$1.pin_source // empty" "$PROFILE")"
	echo "${source:+$ROOT/}${source:-$PROFILE}"
}

OS="$(p .target.os)"
case "$OS" in
	ios) SDK_PLATFORM=iphoneos; SDK_FILE=iPhoneOS.sdk; PLIST_PLATFORM=iPhoneOS ;;
	tvos) SDK_PLATFORM=appletvos; SDK_FILE=AppleTVOS.sdk; PLIST_PLATFORM=AppleTVOS ;;
	*) die "$PROFILE targets $OS, not ios or tvos" ;;
esac
BASE="$(realpath "$ROOT/dependencies")/$OS"
CACHE="$BASE/archives"
DEPLOYMENT_TARGET="$(p .target.deployment_target)"
BUILD_DIRECTORY="$(p ".$OS.build_directory")"
OUT="$ROOT/$BUILD_DIRECTORY"
TOOLCHAIN="$ROOT/$(p .host_toolchain.directory)"
TOOLCHAIN_PROFILE="$(pin_profile .host_toolchain)"
tp() { jq -er "$1" "$TOOLCHAIN_PROFILE"; }
SDL3_PROFILE="$(pin_profile .dependencies.sdl3)"
[ "$(jq -er .dependencies.sdl3.version "$SDL3_PROFILE")" = "$(p .dependencies.sdl3.version)" ] ||
	die "the SDL3 version differs from its pin source $SDL3_PROFILE"
KTX_PROFILE="$ROOT/$(p .dependencies.ktx_software.pin_source)"
[ "$(jq -er .dependencies.ktx_software.revision "$KTX_PROFILE")" = \
  "$(p .dependencies.ktx_software.revision)" ] ||
	die "the KTX revision differs from its pin source $KTX_PROFILE"
[ "$CLEAN" = 1 ] && rm -rf "$OUT"
mkdir -p "$OUT" "$CACHE"

# ---------------------------------------------------------------------------
# 1. Host toolchain
# ---------------------------------------------------------------------------
if [ "$FETCH_ONLY" = 1 ]; then
	python3 "$ROOT/tools/ios/build_toolchain.py" --profile "$TOOLCHAIN_PROFILE" --fetch-only
else
	python3 "$ROOT/tools/ios/build_toolchain.py" --profile "$TOOLCHAIN_PROFILE" --jobs "$JOBS" ||
		die "the iOS host toolchain did not build"
fi

# ---------------------------------------------------------------------------
# 2. iPhoneOS or AppleTVOS SDK
# ---------------------------------------------------------------------------
[ -n "$SDK" ] || SDK="$BASE/sdk/$SDK_FILE"
if [ -n "$FETCH_SDK_FROM" ]; then
	log "Copying the $SDK_FILE from $FETCH_SDK_FROM"
	remote="$(ssh -o BatchMode=yes "$FETCH_SDK_FROM" \
		"cd \"\$(xcrun --sdk $SDK_PLATFORM --show-sdk-path)\" && pwd -P")" ||
		die "no $SDK_FILE on $FETCH_SDK_FROM (is Xcode installed?)"
	rm -rf "$SDK.partial"
	mkdir -p "$SDK.partial"
	ssh -o BatchMode=yes "$FETCH_SDK_FROM" "tar -C '$remote' -czf - ." |
		tar --warning=no-unknown-keyword -xzf - -C "$SDK.partial" ||
		die "copying the SDK from $FETCH_SDK_FROM failed"
	rm -rf "$SDK"
	mv "$SDK.partial" "$SDK"
fi
[ -f "$SDK/SDKSettings.json" ] ||
	die "no $SDK_FILE at $SDK; pass --sdk DIR or --fetch-sdk-from HOST"
SDK_NAME="$(jq -er .CanonicalName "$SDK/SDKSettings.json")"
case "$SDK_NAME" in
	"$SDK_PLATFORM"[0-9]*) ;;
	*) die "$SDK is $SDK_NAME, not a $SDK_FILE" ;;
esac
SDK_VERSION="$(jq -er .Version "$SDK/SDKSettings.json")"
SDK_MIN="$(p .sdk.minimum_version)"
[ "$(printf '%s\n%s\n' "$SDK_MIN" "$SDK_VERSION" | sort -V | head -1)" = "$SDK_MIN" ] ||
	die "$SDK_FILE $SDK_VERSION is older than the profile's minimum $SDK_MIN"
log "$SDK_FILE $SDK_VERSION at $SDK"

# ---------------------------------------------------------------------------
# 2b. Compiler runtime for the target
# ---------------------------------------------------------------------------
# Objective-C @available checks (SDL3) call __isPlatformVersionAtLeast from
# compiler-rt's os_version_check.c; libSystem supplies the other builtins.
# Xcode ships it as the resource directory's darwin/libclang_rt.<os>.a, which
# clang links for every iOS or tvOS target when present. Built from the
# pinned LLVM source against this SDK.
build_compiler_runtime()
{
	local resource runtime stamp key source
	resource="$("$TOOLCHAIN/bin/clang" -print-resource-dir)"
	runtime="$resource/lib/darwin/libclang_rt.$OS.a"
	stamp="$runtime.stamp"
	source="$(dirname "$TOOLCHAIN")/src/$(tp .host_toolchain.llvm.extracted_directory)/compiler-rt/lib/builtins/os_version_check.c"
	key="sdk=$SDK_VERSION target=$DEPLOYMENT_TARGET llvm=$(tp .host_toolchain.llvm.sha256)"
	if [ -f "$runtime" ] && [ "$(cat "$stamp" 2>/dev/null)" = "$key" ]; then
		return
	fi
	[ -f "$source" ] || die "missing $source (run tools/ios/build_toolchain.py)"
	log "Building the $OS compiler runtime (os_version_check)"
	mkdir -p "$(dirname "$runtime")"
	local object="$runtime.os_version_check.o"
	"$TOOLCHAIN/bin/clang" -target "arm64-apple-$OS$DEPLOYMENT_TARGET" -isysroot "$SDK" \
		-O2 -fPIC -c "$source" -o "$object" || die "compiling $source failed"
	rm -f "$runtime"
	"$TOOLCHAIN/bin/llvm-ar" rcs "$runtime" "$object" && rm -f "$object"
	echo "$key" > "$stamp"
}
[ "$FETCH_ONLY" = 1 ] || build_compiler_runtime

# ---------------------------------------------------------------------------
# 3. Pinned archives
# ---------------------------------------------------------------------------
# fetch_pin PROFILE JQ_PATH: download (if missing) and verify one pinned
# archive, then extract it under dependencies/<os>/src; prints the source dir.
fetch_pin()
{
	local profile="$1" path="$2"
	local url sha name dir
	url="$(jq -er "$path.url" "$profile")"
	sha="$(jq -er "$path.sha256" "$profile")"
	name="$(jq -er "$path.cache_archive" "$profile")"
	dir="$(jq -er "$path.extracted_directory" "$profile")"
	if [ ! -f "$CACHE/$name" ]; then
		log "Downloading $url"
		curl -fL --retry 3 -o "$CACHE/$name.partial" "$url" || die "download failed: $url"
		mv "$CACHE/$name.partial" "$CACHE/$name"
	fi
	echo "$sha  $CACHE/$name" | sha256sum -c --quiet - ||
		die "$CACHE/$name does not match its pinned sha256"
	mkdir -p "$BASE/src/$dir"
	if [ -z "$(ls -A "$BASE/src/$dir")" ]; then
		case "$name" in
			*.tar) tar -xf "$CACHE/$name" -C "$BASE/src/$dir" ;;
			*) tar -xzf "$CACHE/$name" -C "$BASE/src" ;;
		esac
	fi
	echo "$BASE/src/$dir"
}

SDL3_SRC="$(fetch_pin "$SDL3_PROFILE" .dependencies.sdl3)"
KTX_SRC="$(fetch_pin "$KTX_PROFILE" .dependencies.ktx_software)"
MOLTENVK_DIR="$(fetch_pin "$PROFILE" .dependencies.moltenvk)"
MOLTENVK_LIB="$MOLTENVK_DIR/$(p .dependencies.moltenvk.static_library)"
MOLTENVK_INCLUDE="$MOLTENVK_DIR/$(p .dependencies.moltenvk.include_directory)"
[ -f "$MOLTENVK_LIB" ] || die "missing $MOLTENVK_LIB"
[ "$FETCH_ONLY" = 1 ] && { log "Archives verified"; exit 0; }

# ---------------------------------------------------------------------------
# 4. Native dependencies
# ---------------------------------------------------------------------------
DEPS="$OUT/deps"
PREFIX="$DEPS/prefix"

# cmake_dependency NAME SOURCE [--target TARGET] [CMAKE_ARGS...]
cmake_dependency()
{
	local name="$1" source="$2"
	shift 2
	local target=""
	if [ "${1:-}" = --target ]; then
		target="$2"
		shift 2
	fi
	local build="$DEPS/build/$name"
	log "Building $name"
	cmake -S "$source" -B "$build" -G Ninja \
		-DCMAKE_TOOLCHAIN_FILE="$ROOT/tools/ios/ios-toolchain.cmake" \
		-DIOS_SDK="$SDK" -DIOS_TOOLCHAIN="$TOOLCHAIN" \
		-DIOS_DEPLOYMENT_TARGET="$DEPLOYMENT_TARGET" -DAPPLE_TARGET_OS="$OS" \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_INSTALL_PREFIX="$PREFIX" \
		-DCMAKE_PREFIX_PATH="$PREFIX" \
		-DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
		"$@" >"$build.configure.log" 2>&1 ||
		{ tail -40 "$build.configure.log"; die "$name configure failed ($build.configure.log)"; }
	cmake --build "$build" -j "$JOBS" ${target:+--target "$target"} >"$build.build.log" 2>&1 ||
		{ tail -40 "$build.build.log"; die "$name build failed ($build.build.log)"; }
	[ -n "$target" ] && return
	cmake --install "$build" >"$build.install.log" 2>&1 ||
		{ tail -40 "$build.install.log"; die "$name install failed ($build.install.log)"; }
}

build_dependencies()
{
	local stamp="$DEPS/stamp" key
	key="recipe=$DEPS_RECIPE sdk=$SDK_VERSION target=$DEPLOYMENT_TARGET \
toolchain=$(sha256sum "$TOOLCHAIN/manifest.json" | cut -d' ' -f1) \
sdl3=$(jq -er .dependencies.sdl3.sha256 "$SDL3_PROFILE") ktx=$(jq -er .dependencies.ktx_software.sha256 "$KTX_PROFILE") \
moltenvk=$(p .dependencies.moltenvk.sha256) thirdparty=$(git -C "$ROOT/thirdparty" rev-parse HEAD)"
	if [ "$(cat "$stamp" 2>/dev/null)" = "$key" ]; then
		log "Native dependencies up to date"
		return
	fi
	rm -rf "$DEPS"
	mkdir -p "$DEPS/build" "$PREFIX/lib/pkgconfig"

	cmake_dependency sdl3 "$SDL3_SRC" \
		-DSDL_SHARED=OFF -DSDL_STATIC=ON -DSDL_TEST_LIBRARY=OFF -DSDL_TESTS=OFF \
		-DSDL_EXAMPLES=OFF -DSDL_VULKAN=ON
	local ktx_options
	mapfile -t ktx_options < <(jq -r \
		'.dependencies.ktx_software.cmake_options | to_entries[] | "-D\(.key)=\(.value)"' "$KTX_PROFILE")
	cmake_dependency ktx "$KTX_SRC" --target ktx_read "${ktx_options[@]}" \
		-DKTX_GIT_VERSION_FULL="$(jq -er .dependencies.ktx_software.version "$KTX_PROFILE")"
	# On Apple targets KTX wraps the static reader in a framework; Waf takes
	# the archive where the pinned profile names it (lib/libktx_read.a).
	cp "$DEPS/build/ktx/lib/ktx_read.framework/ktx_read" "$DEPS/build/ktx/lib/libktx_read.a"
	# The SDK's zlib: freetype's bundled copy misreads the Apple headers.
	cmake_dependency freetype "$ROOT/thirdparty/freetype" \
		-DBUILD_SHARED_LIBS=OFF -DFT_DISABLE_ZLIB=OFF -DFT_REQUIRE_ZLIB=ON -DFT_DISABLE_BZIP2=ON \
		-DFT_DISABLE_PNG=ON -DFT_DISABLE_HARFBUZZ=ON -DFT_DISABLE_BROTLI=ON
	# libpng 1.6.38 reads a predefined TARGET_OS_MAC as classic Mac OS (fixed
	# upstream in 1.6.40); clang predefines the TARGET_OS_* macros, as Xcode does.
	# It also takes its prebuilt pnglibconf.h only when CMake's IOS is set; on
	# tvOS its host-side generator would preprocess without the target.
	local png_platform=()
	[ "$OS" = tvos ] && png_platform=(-DIOS=ON)
	cmake_dependency libpng "$ROOT/thirdparty/libpng" "${png_platform[@]}" \
		-DCMAKE_C_FLAGS=-fno-define-target-os-macros \
		-DPNG_SHARED=OFF -DPNG_STATIC=ON -DPNG_EXECUTABLES=OFF -DPNG_TESTS=OFF \
		-DPNG_ARM_NEON=off -DPNG_FRAMEWORK=OFF
	cmake_dependency libjpeg "$ROOT/thirdparty/libjpeg" \
		-DBUILD_STATIC=ON -DBUILD_EXECUTABLES=OFF -DBUILD_TESTS=OFF
	cmake_dependency curl "$ROOT/thirdparty/curl" \
		-DBUILD_SHARED_LIBS=OFF -DBUILD_CURL_EXE=OFF -DBUILD_TESTING=OFF \
		-DHTTP_ONLY=ON -DCMAKE_USE_OPENSSL=OFF -DCURL_USE_OPENSSL=OFF -DCURL_ENABLE_SSL=OFF \
		-DUSE_LIBIDN2=OFF -DCURL_USE_LIBSSH2=OFF -DCURL_DISABLE_LDAP=ON \
		-DENABLE_MANUAL=OFF -DPICKY_COMPILER=OFF

	# The vendored libjpeg build installs no pkg-config file.
	[ -f "$PREFIX/lib/libjpeg.a" ] || die "libjpeg did not install lib/libjpeg.a"
	cat > "$PREFIX/lib/pkgconfig/libjpeg.pc" <<EOF
prefix=$PREFIX
libdir=\${prefix}/lib
includedir=\${prefix}/include

Name: libjpeg
Description: vendored IJG libjpeg (thirdparty/libjpeg)
Version: 9a
Libs: -L\${libdir} -ljpeg
Cflags: -I\${includedir}
EOF
	# zlib is the SDK's system library, which ships no pkg-config file;
	# libpng's .pc requires one.
	local zlib_version
	zlib_version="$(sed -n 's/^#define ZLIB_VERSION "\(.*\)"/\1/p' "$SDK/usr/include/zlib.h")"
	cat > "$PREFIX/lib/pkgconfig/zlib.pc" <<EOF
Name: zlib
Description: $SDK_FILE system zlib
Version: $zlib_version
Libs: -lz
Cflags:
EOF
	# MoltenVK: Khronos's static release for the target, linked into the app.
	mkdir -p "$PREFIX/include"
	cp -R "$MOLTENVK_INCLUDE/." "$PREFIX/include/"
	cp "$MOLTENVK_LIB" "$PREFIX/lib/libMoltenVK.a"
	cat > "$PREFIX/lib/pkgconfig/vulkan.pc" <<EOF
prefix=$PREFIX
libdir=\${prefix}/lib
includedir=\${prefix}/include

Name: vulkan
Description: MoltenVK $(p .dependencies.moltenvk.version) (static; Vulkan portability over Metal)
Version: 1.4
Libs: -L\${libdir} -lMoltenVK -framework Metal -framework QuartzCore -framework IOSurface -framework UIKit -framework Foundation -framework CoreGraphics -lc++
Cflags: -I\${includedir}
EOF
	echo "$key" > "$stamp"
}

build_dependencies
[ "$DEPS_ONLY" = 1 ] && { log "Native dependencies in $PREFIX"; exit 0; }

# ---------------------------------------------------------------------------
# 5. Engine (Waf, private lock and out directory)
# ---------------------------------------------------------------------------
export PKG_CONFIG_LIBDIR="$PREFIX/lib/pkgconfig:$PREFIX/share/pkgconfig"
export PKG_CONFIG_PATH=""
export PKG_CONFIG_SYSROOT_DIR=""
cd "$ROOT"
WAF_OUT="$OUT/waf"
LOCK=".lock-waf-${BUILD_DIRECTORY#build-}"
CONFIGURE=(./waf configure -T release -o "$WAF_OUT" --prefix="$OUT/install"
	--ios-sdk="$SDK" --ios-toolchain="$TOOLCHAIN" --ios-deployment-target="$DEPLOYMENT_TARGET"
	--static-composition
	--platform-provider="$(p .configure_options.platform_provider)"
	--render-backend="$(p .configure_options.render_backend)"
	--build-games="$(p .configure_options.build_games)" --disable-warns
	--ktx-source-root="$KTX_SRC" --ktx-build-root="$DEPS/build/ktx")
log "Configuring engine"
WAFLOCK="$LOCK" "${CONFIGURE[@]}" 2>&1 | tee "$OUT/configure.log" | tail -25
[ "${PIPESTATUS[0]}" = 0 ] || die "engine configure failed ($OUT/configure.log)"
log "Building engine"
WAFLOCK="$LOCK" ./waf build -j "$JOBS" --targets=hl2_launcher 2>&1 | tee "$OUT/build.log" | tail -25
[ "${PIPESTATUS[0]}" = 0 ] || die "engine build failed ($OUT/build.log)"
EXECUTABLE="$WAF_OUT/launcher_main/hl2_launcher"

# Every first-party module is linked in and isolated; no first-party dylib.
# A game's own modules (Portal 2's vscript) come from its profile.
required=(launcher engine client server GameUI filesystem_stdio materialsystem shaderapivulkan vphysics)
mapfile -t -O "${#required[@]}" required < \
	<(jq -r '.static_composition.additional_required_modules // [] | .[]' "$PROFILE")
python3 "$ROOT/tools/quality/static_composition.py" check --tree "$WAF_OUT" \
	--program launcher_main/hl2_launcher $(printf -- '--require %s ' "${required[@]}") ||
	die "the static composition check failed"

# ---------------------------------------------------------------------------
# 6. App bundle (unsigned; signing and installing happen on the Mac)
# ---------------------------------------------------------------------------
APP="$OUT/$(p ".$OS.app_bundle")"
log "Assembling $APP"
rm -rf "$APP"
mkdir -p "$APP"
cp "$EXECUTABLE" "$APP/$(p ".$OS.executable")"
# The app's own UI art (the touch-control icons, as in the Android APK);
# ios_main.cpp installs it into <game>/custom/ at startup.
python3 "$ROOT/tools/android/touch_icons.py" "$APP/touch" >/dev/null
printf 'APPL????' > "$APP/PkgInfo"
python3 - "$PROFILE" "$APP/Info.plist" "$SDK_NAME" "$SDK_VERSION" "$PLIST_PLATFORM" <<'PLIST'
import json, plistlib, sys
profile, out, sdk_name, sdk_version, platform = sys.argv[1:6]
data = json.load(open(profile))
target = data['target']
ios = data[target['os']]
plist = {
    'CFBundleDevelopmentRegion': 'en',
    'CFBundleDisplayName': ios['display_name'],
    'CFBundleExecutable': ios['executable'],
    'CFBundleIdentifier': ios['bundle_id'],
    'CFBundleInfoDictionaryVersion': '6.0',
    'CFBundleName': ios['display_name'],
    'CFBundlePackageType': 'APPL',
    'CFBundleShortVersionString': ios['version'],
    'CFBundleVersion': str(ios['build_number']),
    'CFBundleSupportedPlatforms': [platform],
    'DTPlatformName': platform.lower(),
    'DTPlatformVersion': sdk_version,
    'DTSDKName': sdk_name,
    'LSRequiresIPhoneOS': True,
    'MinimumOSVersion': target['deployment_target'],
    'UIDeviceFamily': ios['device_family'],
    'UIRequiredDeviceCapabilities': ios['required_device_capabilities'],
    # A game category: App Store placement, and on iOS what makes the system
    # consider the app for Game Mode.
    'LSApplicationCategoryType': ios['app_category'],
}
if target['os'] == 'ios':
    plist.update({
        'UISupportedInterfaceOrientations': ios['orientations'],
        'UISupportedInterfaceOrientations~ipad': ios['orientations'],
        # A launch screen makes iOS run the app at the device's native size.
        'UILaunchScreen': {},
        'UIRequiresFullScreen': True,
        'UIStatusBarHidden': True,
        # SDL3 takes pointer (trackpad, mouse) input as UIKit indirect events.
        'UIApplicationSupportsIndirectInputEvents': True,
        # ProMotion: without it iOS caps an iPhone app at 60 Hz; the game's
        # fps_max and vsync then choose the rate.
        'CADisableMinimumFrameDurationOnPhone': True,
        # Game Mode (iOS 18+): while the game is in front the system gives it
        # the highest CPU and GPU priority, lowers background work and doubles
        # the Bluetooth controller and AirPods sampling rate. The system still
        # decides whether to enter it.
        'GCSupportsGameMode': ios.get('game_mode', False),
        'LSSupportsGameMode': ios.get('game_mode', False),
        # Content goes into Documents with the Files app or xcrun devicectl.
        'UIFileSharingEnabled': True,
        'LSSupportsOpeningDocumentsInPlace': True,
    })
# tvOS: always landscape and full screen; no Documents sharing (the content
# is in Library/Caches, see ios_main.cpp).
with open(out, 'wb') as stream:
    plistlib.dump(plist, stream)
PLIST
log "Built $APP (unsigned): sign and install it on the Mac"
