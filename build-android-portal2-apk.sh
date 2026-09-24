#!/usr/bin/env bash
# Build the Portal 2 Android APK (SDL3 + native Vulkan) and sync its content.
#
#   ./build-android-portal2-apk.sh [options] [build-android-apk.sh options]
#
# The Portal 2 profile (quality/product_profiles/portal2-android-native-vulkan.json)
# extends the Portal profile, so the pins, SDK levels and ABIs stay in one
# place; build-android-apk.sh does the building, packaging and verification
# under build-android-p2/.
#
# When the APK is installed (--install, --run) or only content is wanted
# (--content-only), the retail content is staged from the Steam installation by
# tools/quality/stage_portal2_runtime.py (VPKs, loose maps and materials, the
# retail search paths and the menu background) into build-android-p2-content/,
# then pushed to the app's external files directory. Only files missing on the
# device or differing in size or modification time are pushed, so the first
# sync moves about 11.5 GB and later ones only what changed.

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROFILE="$ROOT/quality/product_profiles/portal2-android-native-vulkan.json"
P2_STEAM_ROOT="${P2_STEAM_ROOT:-$HOME/.local/share/Steam/steamapps/common/Portal 2}"
STAGE="$ROOT/build-android-p2-content"

usage()
{
	cat <<EOF
Usage: $0 [options] [build-android-apk.sh options]

  --steam-root DIR    Portal 2 installation (default: \$P2_STEAM_ROOT or
                      $P2_STEAM_ROOT)
  --no-content        do not stage or push retail content
  --restage           rebuild the content stage from the installation

Everything else goes to build-android-apk.sh, e.g.:

  $0 --run                     # build arm64-v8a, install, sync content, launch
  $0 --content-only            # only sync content to the device
  $0 --content-only --install  # sync content, reinstall the last APK
  $0 --release --all-abis      # release APK for every declared ABI
EOF
	echo
	echo "build-android-apk.sh options:"
	"$ROOT/build-android-apk.sh" --help | sed -n '/^  --/,/^$/p'
}

args=()
CONTENT=auto
RESTAGE=0
WANTS_DEVICE=0
while [ $# -gt 0 ]; do
	case "$1" in
	--steam-root) P2_STEAM_ROOT="$2"; shift ;;
	--no-content) CONTENT=none ;;
	--restage) RESTAGE=1 ;;
	--content) CONTENT=given; args+=("$1" "$2"); shift ;;
	--profile) echo "error: $0 always uses $PROFILE" >&2; exit 2 ;;
	--install|--run|--content-only) WANTS_DEVICE=1; args+=("$1") ;;
	-h|--help) usage; exit 0 ;;
	*) args+=("$1") ;;
	esac
	shift
done

if [ "$CONTENT" = auto ] && [ "$WANTS_DEVICE" = 1 ]; then
	[ -f "$P2_STEAM_ROOT/portal2/pak01_dir.vpk" ] ||
		{ echo "error: no Portal 2 installation at $P2_STEAM_ROOT (--steam-root)" >&2; exit 1; }
	command -v vpk >/dev/null ||
		{ echo "error: missing host tool: vpk (pip install vpk); it extracts the menu image" >&2; exit 1; }
	[ "$RESTAGE" = 1 ] && rm -rf "$STAGE"
	printf '\033[1;36m==> %s\033[0m\n' "Staging Portal 2 content in $STAGE" >&2
	# Immutable assets are symlinks into the installation; the push follows them.
	python3 "$ROOT/tools/quality/stage_portal2_runtime.py" --mount-custom \
		--steam-root "$P2_STEAM_ROOT" --runtime "$STAGE"
	args+=(--content "$STAGE")
fi

exec "$ROOT/build-android-apk.sh" --profile "$PROFILE" "${args[@]}"
