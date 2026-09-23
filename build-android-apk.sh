#!/usr/bin/env bash
# Build the Android client APK (SDL3 + native Vulkan) from the pinned profile.
#
#   ./build-android-apk.sh [options]
#
# The product profile (quality/product_profiles/portal-android-native-vulkan.json)
# owns every pin: NDK, SDL3, SDK platform/build-tools, SDK levels, ABIs and the
# application id. This script only orchestrates:
#
#   1. fetch + verify the pinned NDK and SDL3 archives into dependencies/android/
#   2. cross-build the native dependencies per ABI with CMake (SDL3, freetype,
#      libpng, libjpeg, curl from the thirdparty submodule)
#   3. configure/build/install the engine with Waf under a private lock and
#      out directory per ABI (the shared build/ tree is never touched)
#   4. stage the .so files (unstripped copies kept for symbolization) and check
#      every DT_NEEDED resolves inside the APK or to an Android system library
#   5. package with the SDK build-tools: aapt2, javac + d8 (SDLActivity),
#      zipalign -P 16 (16 KB pages), apksigner
#   6. optionally install, push game content and launch over adb
#
# Everything is written under build-android/ (gitignored).

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROFILE="$ROOT/quality/product_profiles/portal-android-native-vulkan.json"
OUT="$ROOT/build-android"
CACHE="$ROOT/dependencies/android"
# Bump when a dependency recipe below changes, to force those rebuilds.
DEPS_RECIPE=2

ABIS=()
INSTALL=0
RUN=0
CLEAN=0
RELEASE=0
PACKAGE_ONLY=0
FETCH_ONLY=0
CONTENT=""
SERIAL="${ANDROID_SERIAL:-}"
JOBS="$(nproc)"
CCACHE=0

usage()
{
	cat <<EOF
Usage: $0 [options]

  --abi ABI           arm64-v8a or x86_64; repeatable (default: arm64-v8a)
  --all-abis          every ABI the profile declares
  --release           sign with ANDROID_KEYSTORE / ANDROID_KEYSTORE_PASS /
                      ANDROID_KEY_ALIAS [/ ANDROID_KEY_PASS] instead of the
                      debug keystore
  --install           adb install the APK (see --serial)
  --content DIR       push game content from DIR (containing hl2/, portal/,
                      platform/) to the app's external files directory; bin/
                      directories (desktop modules) are skipped
  --run               launch the app after installing
  --serial SERIAL     adb device (default: \$ANDROID_SERIAL or the only device)
  --package-only      repackage from existing native builds
  --fetch-only        only fetch and verify pinned archives
  --clean             remove build-android/ first
  --ccache            compile the engine through ccache
  -j N                parallel jobs (default: $JOBS)
  -h, --help          this help

Environment: ANDROID_SDK_ROOT or ANDROID_HOME (default: ~/Android/Sdk).
EOF
}

while [ $# -gt 0 ]; do
	case "$1" in
	--abi) ABIS+=("$2"); shift ;;
	--all-abis) ABIS=(ALL) ;;
	--release) RELEASE=1 ;;
	--install) INSTALL=1 ;;
	--content) CONTENT="$2"; shift ;;
	--run) RUN=1; INSTALL=1 ;;
	--serial) SERIAL="$2"; shift ;;
	--package-only) PACKAGE_ONLY=1 ;;
	--fetch-only) FETCH_ONLY=1 ;;
	--clean) CLEAN=1 ;;
	--ccache) CCACHE=1 ;;
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

for tool in jq curl sha256sum unzip tar cmake ninja python3 javac zip pkg-config; do
	need "$tool"
done
[ -f "$PROFILE" ] || die "missing profile $PROFILE"
p() { jq -er "$1" "$PROFILE"; }

APP_ID="$(p .android.application_id)"
APP_LABEL="$(p .android.application_label)"
VERSION_CODE="$(p .android.version_code)"
VERSION_NAME="$(p .android.version_name)"
MIN_SDK="$(p .android.min_sdk)"
TARGET_SDK="$(p .android.target_sdk)"
PLATFORM="$(p .android.compile_platform)"
PLATFORM_JAR_SHA="$(p .android.compile_platform_jar_sha256)"
BUILD_TOOLS="$(p .android.build_tools)"
JAVA_RELEASE="$(p .android.java_release)"
PAGE_ALIGN="$(p .android.page_size_alignment)"
MANIFEST_TEMPLATE="$ROOT/$(p .android.manifest_template)"
GAMES="$(p .configure_options.build_games)"

if [ ${#ABIS[@]} -eq 0 ]; then
	ABIS=(arm64-v8a)
elif [ "${ABIS[0]}" = ALL ]; then
	mapfile -t ABIS < <(p '.target.abis[]')
fi
for abi in "${ABIS[@]}"; do
	jq -e --arg a "$abi" '.target.abis | index($a)' "$PROFILE" >/dev/null ||
		die "ABI $abi is not declared by the profile"
done

if [ "$CLEAN" = 1 ]; then
	log "Removing $OUT"
	rm -rf "$OUT"
fi
mkdir -p "$OUT" "$CACHE"

# ---------------------------------------------------------------------------
# 1. Pinned archives
# ---------------------------------------------------------------------------
fetch_dependency()
{
	local name="$1"
	local url sha size archive dir
	url="$(p ".dependencies.$name.url")"
	sha="$(p ".dependencies.$name.sha256")"
	size="$(p ".dependencies.$name.archive_bytes")"
	archive="$CACHE/$(p ".dependencies.$name.cache_archive")"
	dir="$CACHE/$(p ".dependencies.$name.extracted_directory")"

	if [ ! -f "$archive" ]; then
		log "Fetching $name from $url"
		curl -fL --retry 3 -o "$archive.part" "$url"
		mv "$archive.part" "$archive"
	fi
	[ "$(stat -c %s "$archive")" = "$size" ] || die "$name archive size mismatch: $archive"
	# Verified once per archive; the stamp records the verified digest.
	if [ "$(cat "$archive.verified" 2>/dev/null)" != "$sha" ]; then
		log "Verifying $name"
		echo "$sha  $archive" | sha256sum -c --quiet - || die "$name SHA-256 mismatch: $archive"
		echo "$sha" > "$archive.verified"
	fi
	if [ ! -d "$dir" ]; then
		log "Extracting $name"
		local tmp
		tmp="$(mktemp -d "$CACHE/.extract-XXXXXX")"
		case "$archive" in
		*.zip) unzip -q "$archive" -d "$tmp" ;;
		*.tar.gz) tar -xzf "$archive" -C "$tmp" ;;
		*) die "unknown archive type: $archive" ;;
		esac
		[ -d "$tmp/$(basename "$dir")" ] || die "$name archive lacks $(basename "$dir")/"
		mv "$tmp/$(basename "$dir")" "$dir"
		rmdir "$tmp"
	fi
	printf '%s' "$dir"
}

NDK="$(fetch_dependency ndk)"
SDL3_SRC="$(fetch_dependency sdl3)"
grep -q "Pkg.Revision = $(p .dependencies.ndk.revision)" "$NDK/source.properties" ||
	die "NDK at $NDK is not revision $(p .dependencies.ndk.revision)"
[ "$FETCH_ONLY" = 1 ] && { log "Pinned archives verified"; exit 0; }

# ---------------------------------------------------------------------------
# SDK, JDK and signing inputs
# ---------------------------------------------------------------------------
SDK="${ANDROID_SDK_ROOT:-${ANDROID_HOME:-$HOME/Android/Sdk}}"
BT="$SDK/build-tools/$BUILD_TOOLS"
ANDROID_JAR="$SDK/platforms/$PLATFORM/android.jar"
for tool in aapt2 d8 zipalign apksigner; do
	[ -x "$BT/$tool" ] || die "missing $BT/$tool (install build-tools $BUILD_TOOLS)"
done
[ -f "$ANDROID_JAR" ] || die "missing $ANDROID_JAR (install platform $PLATFORM)"
echo "$PLATFORM_JAR_SHA  $ANDROID_JAR" | sha256sum -c --quiet - ||
	die "$ANDROID_JAR does not match the profile's pinned digest"

LLVM="$NDK/toolchains/llvm/prebuilt/linux-x86_64"
READELF="$LLVM/bin/llvm-readelf"
STRIP="$LLVM/bin/llvm-strip"

# ---------------------------------------------------------------------------
# 2. Native dependencies (per ABI)
# ---------------------------------------------------------------------------
cmake_dependency()
{
	local abi="$1" name="$2" source="$3"
	shift 3
	local build="$OUT/$abi/deps/build/$name"
	local prefix="$OUT/$abi/deps/prefix"
	log "[$abi] Building $name"
	cmake -S "$source" -B "$build" -G Ninja \
		-DCMAKE_TOOLCHAIN_FILE="$NDK/build/cmake/android.toolchain.cmake" \
		-DANDROID_ABI="$abi" \
		-DANDROID_PLATFORM="android-$MIN_SDK" \
		-DANDROID_STL=c++_shared \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_INSTALL_PREFIX="$prefix" \
		-DCMAKE_FIND_ROOT_PATH="$prefix" \
		-DCMAKE_POSITION_INDEPENDENT_CODE=ON \
		-DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
		"$@" >"$build.configure.log" 2>&1 ||
		{ tail -40 "$build.configure.log"; die "$name configure failed ($build.configure.log)"; }
	cmake --build "$build" -j "$JOBS" >"$build.build.log" 2>&1 ||
		{ tail -40 "$build.build.log"; die "$name build failed ($build.build.log)"; }
	cmake --install "$build" >"$build.install.log" 2>&1 ||
		{ tail -40 "$build.install.log"; die "$name install failed ($build.install.log)"; }
}

build_dependencies()
{
	local abi="$1"
	local prefix="$OUT/$abi/deps/prefix"
	local stamp="$OUT/$abi/deps/stamp"
	local key
	key="recipe=$DEPS_RECIPE ndk=$(p .dependencies.ndk.sha256) sdl3=$(p .dependencies.sdl3.sha256) \
thirdparty=$(git -C "$ROOT/thirdparty" rev-parse HEAD) api=$MIN_SDK"
	if [ "$(cat "$stamp" 2>/dev/null)" = "$key" ]; then
		log "[$abi] Native dependencies up to date"
		return
	fi
	rm -rf "$OUT/$abi/deps"
	mkdir -p "$OUT/$abi/deps/build" "$prefix/lib/pkgconfig"

	cmake_dependency "$abi" sdl3 "$SDL3_SRC" \
		-DSDL_SHARED=ON -DSDL_STATIC=OFF -DSDL_TEST_LIBRARY=OFF -DSDL_TESTS=OFF \
		-DSDL_EXAMPLES=OFF -DSDL_VULKAN=ON -DSDL_OPENGLES=ON
	cmake_dependency "$abi" freetype "$ROOT/thirdparty/freetype" \
		-DBUILD_SHARED_LIBS=OFF -DFT_DISABLE_ZLIB=ON -DFT_DISABLE_BZIP2=ON \
		-DFT_DISABLE_PNG=ON -DFT_DISABLE_HARFBUZZ=ON -DFT_DISABLE_BROTLI=ON
	cmake_dependency "$abi" libpng "$ROOT/thirdparty/libpng" \
		-DPNG_SHARED=OFF -DPNG_STATIC=ON -DPNG_EXECUTABLES=OFF -DPNG_TESTS=OFF \
		-DPNG_ARM_NEON=off
	cmake_dependency "$abi" libjpeg "$ROOT/thirdparty/libjpeg" \
		-DBUILD_STATIC=ON -DBUILD_EXECUTABLES=OFF -DBUILD_TESTS=OFF
	cmake_dependency "$abi" curl "$ROOT/thirdparty/curl" \
		-DBUILD_SHARED_LIBS=OFF -DBUILD_CURL_EXE=OFF -DBUILD_TESTING=OFF \
		-DHTTP_ONLY=ON -DCMAKE_USE_OPENSSL=OFF -DCURL_USE_OPENSSL=OFF -DCURL_ENABLE_SSL=OFF \
		-DUSE_LIBIDN2=OFF -DCURL_USE_LIBSSH2=OFF -DCURL_DISABLE_LDAP=ON \
		-DENABLE_MANUAL=OFF -DPICKY_COMPILER=OFF

	# The vendored libjpeg build installs no pkg-config file.
	[ -f "$prefix/lib/libjpeg.a" ] || die "libjpeg did not install lib/libjpeg.a"
	cat > "$prefix/lib/pkgconfig/libjpeg.pc" <<EOF
prefix=$prefix
libdir=\${prefix}/lib
includedir=\${prefix}/include

Name: libjpeg
Description: vendored IJG libjpeg (thirdparty/libjpeg)
Version: 9a
Libs: -L\${libdir} -ljpeg
Cflags: -I\${includedir}
EOF
	# zlib is the NDK's system library, which ships no pkg-config file;
	# libpng's .pc requires one.
	local zlib_version
	zlib_version="$(sed -n 's/^#define ZLIB_VERSION "\(.*\)"/\1/p' "$LLVM/sysroot/usr/include/zlib.h")"
	cat > "$prefix/lib/pkgconfig/zlib.pc" <<EOF
Name: zlib
Description: Android NDK system zlib
Version: $zlib_version
Libs: -lz
Cflags:
EOF
	echo "$key" > "$stamp"
}

# ---------------------------------------------------------------------------
# 3. Engine (per ABI), private Waf lock and out dir
# ---------------------------------------------------------------------------
build_engine()
{
	local abi="$1"
	local waf_arch
	waf_arch="$(jq -er --arg a "$abi" '.android.waf_arch[$a]' "$PROFILE")"
	local prefix="$OUT/$abi/deps/prefix"
	local out="$OUT/$abi/waf"
	local install="$OUT/$abi/install"
	local lock=".lock-waf-android-$abi"
	local configure=(./waf configure -T release -o "$out" --prefix="$install"
		--android="$waf_arch,clang,$MIN_SDK"
		--platform-provider="$(p .configure_options.platform_provider)"
		--render-backend="$(p .configure_options.render_backend)"
		--build-games="$GAMES" --disable-warns)
	[ "$CCACHE" = 1 ] && configure+=(--use-ccache)
	# Reconfigure when a configure input changes: options, dependencies, or
	# any wscript / Waf tool.
	local scripts
	scripts="$(cd "$ROOT" && { git ls-files -co --exclude-standard -- '*wscript' 'scripts/waifulib/*.py'; } |
		sort | xargs cat | sha256sum | cut -d' ' -f1)"
	local key="${configure[*]} deps=$(cat "$OUT/$abi/deps/stamp") scripts=$scripts"

	export ANDROID_NDK_HOME="$NDK"
	# Only the cross-built prefix is visible to pkg-config.
	export PKG_CONFIG_LIBDIR="$prefix/lib/pkgconfig:$prefix/share/pkgconfig"
	export PKG_CONFIG_PATH=""
	export PKG_CONFIG_SYSROOT_DIR=""
	cd "$ROOT"
	if [ "$(cat "$OUT/$abi/configure.stamp" 2>/dev/null)" != "$key" ] ||
	   [ ! -f "$out/c4che/_cache.py" ]; then
		log "[$abi] Configuring engine"
		WAFLOCK="$lock" "${configure[@]}" 2>&1 | tee "$OUT/$abi/configure.log" | tail -25
		[ "${PIPESTATUS[0]}" = 0 ] || die "engine configure failed ($OUT/$abi/configure.log)"
		echo "$key" > "$OUT/$abi/configure.stamp"
	fi
	log "[$abi] Building engine"
	rm -rf "$install"
	WAFLOCK="$lock" ./waf install -j "$JOBS" 2>&1 | tee "$OUT/$abi/build.log" |
		grep -E "error|Error|warning: unused|^'install' |^Waf:" | grep -v "^Waf: Entering\|^Waf: Leaving" | tail -60
	[ "${PIPESTATUS[0]}" = 0 ] || die "engine build failed ($OUT/$abi/build.log)"
}

# ---------------------------------------------------------------------------
# 4. Stage native libraries
# ---------------------------------------------------------------------------
# Android system libraries an app may link against (NDK stable APIs).
SYSTEM_LIBS="libc.so libm.so libdl.so liblog.so libandroid.so libvulkan.so libz.so \
libEGL.so libGLESv1_CM.so libGLESv2.so libGLESv3.so libOpenSLES.so libaaudio.so \
libjnigraphics.so libmediandk.so libcamera2ndk.so libnativewindow.so libamidi.so libsync.so"

stage_libraries()
{
	local abi="$1"
	local triple
	triple="$(jq -er --arg a "$abi" '.android.ndk_triple[$a]' "$PROFILE")"
	local stage="$OUT/apk/lib/$abi"
	local symbols="$OUT/$abi/symbols"
	rm -rf "$stage" "$symbols"
	mkdir -p "$stage" "$symbols"

	log "[$abi] Staging native libraries"
	find "$OUT/$abi/install" -name '*.so' -exec cp -f {} "$symbols/" \;
	cp -f "$OUT/$abi/deps/prefix/lib/libSDL3.so" "$symbols/"
	cp -f "$LLVM/sysroot/usr/lib/$triple/libc++_shared.so" "$symbols/"
	[ -f "$symbols/libmain.so" ] || die "the engine build produced no libmain.so"
	local lib
	for lib in "$symbols"/*.so; do
		"$STRIP" --strip-unneeded -o "$stage/$(basename "$lib")" "$lib"
	done

	# Every DT_NEEDED must be packaged or a system library: a missing one only
	# shows up on the device as a load failure.
	local missing=0 needed
	for lib in "$stage"/*.so; do
		for needed in $("$READELF" -d "$lib" | sed -n 's/.*(NEEDED).*\[\(.*\)\]/\1/p'); do
			if [ ! -f "$stage/$needed" ] && [[ " $SYSTEM_LIBS " != *" $needed "* ]]; then
				echo "  $(basename "$lib") needs $needed, which is not packaged" >&2
				missing=1
			fi
		done
	done
	[ "$missing" = 0 ] || die "unresolved native dependencies for $abi"
	echo "  $(ls "$stage" | wc -l) libraries, $(du -sh "$stage" | cut -f1) stripped"
}

# ---------------------------------------------------------------------------
# 5. Package
# ---------------------------------------------------------------------------
package_apk()
{
	local work="$OUT/apk"
	local abis_label
	abis_label="$(IFS=+; echo "${ABIS[*]}")"
	local kind=debug
	[ "$RELEASE" = 1 ] && kind=release
	APK="$OUT/portal-$VERSION_NAME-$abis_label-$kind.apk"

	log "Packaging $APK"
	rm -rf "$work/classes" "$work/dex" "$work/manifest"
	mkdir -p "$work/classes" "$work/dex" "$work/manifest"
	sed -e "s|@PACKAGE@|$APP_ID|g" -e "s|@APP_LABEL@|$APP_LABEL|g" \
		"$MANIFEST_TEMPLATE" > "$work/manifest/AndroidManifest.xml"

	# The app's own UI art, installed into <game>/custom/ at startup.
	rm -rf "$work/assets"
	python3 "$ROOT/tools/android/touch_icons.py" "$work/assets/touch" >/dev/null

	local link_mode=()
	[ "$RELEASE" = 1 ] || link_mode=(--debug-mode) # run-as, native debugging
	"$BT/aapt2" link -o "$work/base.apk" -I "$ANDROID_JAR" "${link_mode[@]}" -A "$work/assets" \
		--manifest "$work/manifest/AndroidManifest.xml" \
		--min-sdk-version "$MIN_SDK" --target-sdk-version "$TARGET_SDK" \
		--version-code "$VERSION_CODE" --version-name "$VERSION_NAME"

	# SDLActivity and its helpers, from the pinned SDL3 source.
	find "$SDL3_SRC/android-project/app/src/main/java" -name '*.java' > "$work/sources.txt"
	javac -nowarn -Xlint:-options --release "$JAVA_RELEASE" -classpath "$ANDROID_JAR" \
		-d "$work/classes" @"$work/sources.txt" 2>&1 | grep -v "^Note:" || true
	[ -f "$work/classes/org/libsdl/app/SDLActivity.class" ] || die "javac failed"
	"$BT/d8" --release --min-api "$MIN_SDK" --lib "$ANDROID_JAR" --output "$work/dex" \
		$(find "$work/classes" -name '*.class')

	cp "$work/base.apk" "$work/unaligned.apk"
	(cd "$work/dex" && zip -q -X "$work/unaligned.apk" classes.dex)
	local abi
	(cd "$work" && for abi in "${ABIS[@]}"; do zip -q -X -r unaligned.apk "lib/$abi"; done)

	"$BT/zipalign" -f -P $((PAGE_ALIGN / 1024)) 4 "$work/unaligned.apk" "$work/aligned.apk"
	"$BT/zipalign" -c -P $((PAGE_ALIGN / 1024)) 4 "$work/aligned.apk" >/dev/null ||
		die "zipalign verification failed"

	local sign=()
	if [ "$RELEASE" = 1 ]; then
		: "${ANDROID_KEYSTORE:?set ANDROID_KEYSTORE for --release}"
		: "${ANDROID_KEYSTORE_PASS:?set ANDROID_KEYSTORE_PASS for --release}"
		: "${ANDROID_KEY_ALIAS:?set ANDROID_KEY_ALIAS for --release}"
		sign=(--ks "$ANDROID_KEYSTORE" --ks-pass env:ANDROID_KEYSTORE_PASS
			--ks-key-alias "$ANDROID_KEY_ALIAS")
		[ -n "${ANDROID_KEY_PASS:-}" ] && sign+=(--key-pass env:ANDROID_KEY_PASS)
	else
		local keystore="$HOME/.android/debug.keystore"
		if [ ! -f "$keystore" ]; then
			log "Creating the standard debug keystore"
			mkdir -p "$HOME/.android"
			keytool -genkeypair -keystore "$keystore" -storepass android -keypass android \
				-alias androiddebugkey -keyalg RSA -keysize 2048 -validity 10000 \
				-dname "CN=Android Debug,O=Android,C=US" >/dev/null
		fi
		sign=(--ks "$keystore" --ks-pass pass:android --ks-key-alias androiddebugkey)
	fi
	"$BT/apksigner" sign "${sign[@]}" --out "$APK" "$work/aligned.apk"
	"$BT/apksigner" verify "$APK" || die "apksigner verification failed"
	rm -f "$APK.idsig"
	echo "  $(du -h "$APK" | cut -f1)  $APK"
}

# ---------------------------------------------------------------------------
# 6. Device
# ---------------------------------------------------------------------------
adb_cmd()
{
	if [ -n "$SERIAL" ]; then adb -s "$SERIAL" "$@"; else adb "$@"; fi
}

push_content()
{
	local source="$1"
	local dest="/sdcard/Android/data/$APP_ID/files"
	[ -d "$source" ] || die "content directory $source does not exist"
	log "Pushing content from $source to $dest"
	local top entry
	for top in platform "$GAMES" hl2; do
		[ -d "$source/$top" ] || { echo "  (no $top/ in $source)"; continue; }
		adb_cmd shell mkdir -p "$dest/$top"
		for entry in "$source/$top"/*; do
			# Desktop game modules; the APK carries this device's.
			[ "$(basename "$entry")" = bin ] && continue
			adb_cmd push --sync "$entry" "$dest/$top/" >/dev/null
		done
		# adb creates these as shell:ext_data_rw 0770, which the app's own uid
		# cannot enter; the game also writes configs, saves and stats here.
		# Android/data/<package> is visible to this app only.
		adb_cmd shell chmod -R a+rwX "$dest/$top"
		echo "  $top/ synced"
	done
}

# ---------------------------------------------------------------------------
for abi in "${ABIS[@]}"; do
	if [ "$PACKAGE_ONLY" = 0 ]; then
		build_dependencies "$abi"
		build_engine "$abi"
	fi
	stage_libraries "$abi"
done
package_apk

if [ "$INSTALL" = 1 ]; then
	need adb
	log "Installing on ${SERIAL:-the connected device}"
	adb_cmd install -r "$APK"
fi
[ -n "$CONTENT" ] && { need adb; push_content "$CONTENT"; }
if [ "$RUN" = 1 ]; then
	log "Launching $APP_ID"
	adb_cmd shell am start -n "$APP_ID/org.libsdl.app.SDLActivity"
	echo "  logs: adb logcat -s SDL SRCENG DEBUG AndroidRuntime"
fi
log "Done"
