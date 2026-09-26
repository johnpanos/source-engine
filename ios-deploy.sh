#!/usr/bin/env bash
# Sign an unsigned iOS .app built on Linux and install it on the iPhone that is
# passed through to the macOS VM (see OpenCore-Boot.sh, `ssh macvm`).
#
# Copy of ~/src/mac/ios-deploy.sh with source-engine defaults: the app,
# BUNDLE_ID and the game content come from the product profile (default the
# Portal iOS profile: build-ios/Portal.app from ./build-ios-app.sh).
#
# usage: ./ios-deploy.sh [--profile FILE] [path/to/App.app] [--with-content] [--content DIR]... [--sign-only] [--no-launch] [--console]
#
#   --profile FILE  product profile ("extends" resolved by
#                 tools/quality/profile_extends.py), e.g.
#                 quality/product_profiles/portal2-ios-native-vulkan.json
#   --with-content  the profile's content: --content for each of its
#                 content.directories under content.stage_directory, or
#                 run/runtime/{platform,portal,hl2} when it declares none
#   --content DIR copy DIR (symlinks followed, bin/ skipped) to the app's
#                 Documents/<basename DIR> (Library/Caches/<basename DIR> on
#                 tvOS) before launching; repeatable.
#                 Unchanged files are skipped on later runs.
#   --device X    iphone | tv | a device name or identifier. Default: the
#                 connected device matching the app's platform (an iPhoneOS
#                 build goes to the iPhone, an AppleTVOS build to the Apple TV).
#   --sign-only   copy + sign on the Mac, skip install/launch
#   --no-launch   install (and copy content) but do not launch
#   --console     launch attached to the app's stdout/stderr (Ctrl-C to detach)
#
# env overrides: MACVM_HOST (macvm), BUNDLE_ID (profile .ios.bundle_id),
#                TEAM_ID (25GCRLE3DX), MACVM_KEYCHAIN_PW (john)
#
# Signing uses the free Personal Team, so profiles last 7 days. When the
# profile for BUNDLE_ID is missing or about to expire, a fresh one is made by
# building ~/deploy/profilegen (iOS; a copy of ~/Desktop/test) or
# ~/deploy/profilegen-tv (the same project switched to tvOS) with Xcode.
set -euo pipefail

ROOT=$(cd "$(dirname "$0")" && pwd)
PROFILE_JSON=$ROOT/quality/product_profiles/portal-ios-native-vulkan.json
HOST=${MACVM_HOST:-macvm}
TEAM=${TEAM_ID:-25GCRLE3DX}
KEYCHAIN_PW=${MACVM_KEYCHAIN_PW:-john}
DEVICE=""

APP=""
MODE=launch
CONTENT=()
WITH_CONTENT=0
while [ $# -gt 0 ]; do
  case "$1" in
    --profile) PROFILE_JSON=$(realpath "${2:?--profile needs a file}"); shift ;;
    --with-content) WITH_CONTENT=1 ;;
    --content) [ -d "${2:-}" ] || { echo "--content needs a directory" >&2; exit 2; }; CONTENT+=("${2%/}"); shift ;;
    --device) DEVICE=${2:?--device needs a value}; shift ;;
    --sign-only) MODE=sign ;;
    --no-launch) MODE=install ;;
    --console) MODE=console ;;
    -h|--help) sed -n '2,33p' "$0"; exit 0 ;;
    *) APP=${1%/} ;;
  esac
  shift
done

# profile_value PYTHON_EXPRESSION: a fact from the resolved profile `p`.
profile_value() {
  python3 - "$ROOT" "$PROFILE_JSON" "$1" <<'PY'
import sys
sys.path.insert(0, sys.argv[1] + "/tools/quality")
from profile_extends import load_profile
p = load_profile(sys.argv[2])
os_keys = p[p["target"]["os"]]
print(eval(sys.argv[3], {"p": p, "os_keys": os_keys}))
PY
}
BUNDLE_ID=${BUNDLE_ID:-$(profile_value 'os_keys["bundle_id"]')}
# Entitlements the product asks for when its provisioning profile grants them
# (the capability is enabled for the App ID in Xcode); signing never fails
# for one the profile lacks.
OPTIONAL_ENTITLEMENTS=$(profile_value '" ".join(os_keys.get("optional_entitlements", []))')
[ -n "$APP" ] || APP="$ROOT/$(profile_value 'os_keys["build_directory"] + "/" + os_keys["app_bundle"]')"
if [ "$WITH_CONTENT" = 1 ]; then
  STAGE=$(profile_value 'p.get("content", {}).get("stage_directory", "")')
  if [ -n "$STAGE" ]; then
    for g in $(profile_value '" ".join(p["content"]["directories"])'); do
      [ -d "$ROOT/$STAGE/$g" ] || { echo "missing $ROOT/$STAGE/$g (stage the content first)" >&2; exit 2; }
      CONTENT+=("$ROOT/$STAGE/$g")
    done
  else
    for g in platform portal hl2; do CONTENT+=("$ROOT/run/runtime/$g"); done
  fi
fi
[ -n "$APP" ] && [ -f "$APP/Info.plist" ] || { echo "usage: $0 [--profile FILE] [path/to/App.app] [--with-content] [--content DIR]... [--sign-only|--no-launch|--console]" >&2; exit 2; }

NAME=$(basename "$APP")
# iPhoneOS -> iOS, AppleTVOS -> tvOS (the names devicectl and provisioning profiles use).
PLATFORM=$(python3 -c 'import plistlib, sys; p = plistlib.load(open(sys.argv[1], "rb")).get("CFBundleSupportedPlatforms", ["iPhoneOS"])[0]; print({"iPhoneOS": "iOS", "AppleTVOS": "tvOS"}.get(p, p))' "$APP/Info.plist")
case "$(echo "$DEVICE" | tr "[:upper:]" "[:lower:]")" in
  iphone|phone|ios) WANT=iOS ;;
  tv|appletv|tvos) WANT=tvOS ;;
  *) WANT="" ;;
esac
if [ -n "$WANT" ] && [ "$WANT" != "$PLATFORM" ]; then
  echo "$NAME is built for $PLATFORM; --device $DEVICE needs a $WANT build (built with the $( [ "$WANT" = tvOS ] && echo appletvos || echo iphoneos ) SDK)" >&2
  exit 2
fi
echo "==> $NAME is built for $PLATFORM"
echo "==> Copying $NAME to $HOST"
# Content is cached per app on the Mac: Portal and Portal 2 both have platform/.
ssh "$HOST" mkdir -p "deploy/content/$BUNDLE_ID"
rsync -a --delete "$APP/" "$HOST:deploy/$NAME/"

CONTENT_NAMES=""
if [ "$MODE" != sign ]; then
  for dir in "${CONTENT[@]}"; do
    echo "==> Syncing content $(basename "$dir")/ to $HOST"
    rsync -aL --delete --exclude=/bin/ "$dir/" "$HOST:deploy/content/$BUNDLE_ID/$(basename "$dir")/"
    CONTENT_NAMES+="$(basename "$dir") "
  done
fi

# ssh joins its arguments into one command string, so quote them (keeps an empty DEVICE).
ssh "$HOST" "bash -s -- $(printf '%q ' "$NAME" "$BUNDLE_ID" "$TEAM" "$KEYCHAIN_PW" "$MODE" "$DEVICE" "$CONTENT_NAMES" "$PLATFORM" "$OPTIONAL_ENTITLEMENTS")" <<'REMOTE'
set -euo pipefail
NAME=$1 BUNDLE_ID=$2 TEAM=$3 KEYCHAIN_PW=$4 MODE=$5 DEVICE=$6 CONTENT_NAMES=$7 PLATFORM=$8 OPTIONAL_ENTITLEMENTS=${9:-}
APP=$HOME/deploy/$NAME
PROFDIR="$HOME/Library/Developer/Xcode/UserData/Provisioning Profiles"
TMP=$(mktemp -d); trap 'rm -rf "$TMP"' EXIT

# Each SSH login is its own security session, so unlock here or codesign
# fails with errSecInternalComponent.
security unlock-keychain -p "$KEYCHAIN_PW" "$HOME/Library/Keychains/login.keychain-db"

# Newest PLATFORM profile for TEAM.BUNDLE_ID that is valid for at least another hour.
find_profile() {
  local best="" best_exp=0 f exp
  for f in "$PROFDIR"/*.mobileprovision; do
    [ -e "$f" ] || continue
    security cms -D -i "$f" > "$TMP/p.plist" 2>/dev/null || continue
    [ "$(plutil -extract Entitlements.application-identifier raw -o - "$TMP/p.plist" 2>/dev/null)" = "$TEAM.$BUNDLE_ID" ] || continue
    [ "$(plutil -extract Platform.0 raw -o - "$TMP/p.plist" 2>/dev/null)" = "$PLATFORM" ] || continue
    exp=$(TZ=UTC date -j -f "%Y-%m-%dT%H:%M:%SZ" "$(plutil -extract ExpirationDate raw -o - "$TMP/p.plist")" +%s)
    if [ "$exp" -gt $(( $(date +%s) + 3600 )) ] && [ "$exp" -gt "$best_exp" ]; then best=$f best_exp=$exp; fi
  done
  printf '%s' "$best"
}

PROFILE=$(find_profile)
if [ -z "$PROFILE" ]; then
  echo "==> No valid $PLATFORM profile for $BUNDLE_ID; asking Xcode for a new one"
  GEN=$HOME/deploy/profilegen
  if [ "$PLATFORM" = tvOS ]; then
    GEN=$HOME/deploy/profilegen-tv
    [ -d "$GEN" ] || { echo "missing $GEN (tvOS copy of the test project)" >&2; exit 1; }
  fi
  [ -d "$GEN" ] || cp -R "$HOME/Desktop/test" "$GEN"
  (cd "$GEN" && xcodebuild -project test.xcodeproj -scheme test \
      -destination "generic/platform=$PLATFORM" -derivedDataPath ./dd -allowProvisioningDeviceRegistration \
      PRODUCT_BUNDLE_IDENTIFIER="$BUNDLE_ID" DEVELOPMENT_TEAM="$TEAM" CODE_SIGN_STYLE=Automatic \
      -allowProvisioningUpdates build > "$TMP/profilegen.log" 2>&1) ||
    { tail -20 "$TMP/profilegen.log"; echo "profile generation failed" >&2; exit 1; }
  PROFILE=$(find_profile)
  [ -n "$PROFILE" ] || { echo "Xcode did not produce a $PLATFORM profile for $BUNDLE_ID" >&2; exit 1; }
fi
security cms -D -i "$PROFILE" > "$TMP/p.plist"
echo "==> Profile: $(plutil -extract Name raw -o - "$TMP/p.plist") (expires $(plutil -extract ExpirationDate raw -o - "$TMP/p.plist"))"

IDENTITY=$(security find-identity -v -p codesigning | awk '/Apple Development/ {print $2; exit}')
[ -n "$IDENTITY" ] || { echo "no Apple Development signing identity in the keychain" >&2; exit 1; }

# The same entitlements Xcode gives a development build of this app.
cat > "$TMP/ent.plist" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0"><dict>
  <key>application-identifier</key><string>$TEAM.$BUNDLE_ID</string>
  <key>com.apple.developer.team-identifier</key><string>$TEAM</string>
  <key>get-task-allow</key><true/>
  <key>keychain-access-groups</key><array><string>$TEAM.$BUNDLE_ID</string></array>
</dict></plist>
EOF
for key in $OPTIONAL_ENTITLEMENTS; do
  if plutil -extract "Entitlements.$key" raw -o - "$TMP/p.plist" >/dev/null 2>&1; then
    plutil -insert "$key" -bool YES "$TMP/ent.plist"
    echo "    entitlement $key (granted by the profile)"
  else
    echo "    entitlement $key not in the profile: enable the capability for $BUNDLE_ID in Xcode to get it"
  fi
done

echo "==> Signing $NAME as $BUNDLE_ID"
plutil -replace CFBundleIdentifier -string "$BUNDLE_ID" "$APP/Info.plist"
cp "$PROFILE" "$APP/embedded.mobileprovision"
# Nested code first (deepest paths first), then the app itself.
find "$APP" -mindepth 1 \( -name '*.dylib' -o -name '*.framework' -o -name '*.appex' \) -print |
  awk '{ print length, $0 }' | sort -rn | cut -d' ' -f2- |
  while IFS= read -r nested; do
    codesign -f -s "$IDENTITY" --timestamp=none "$nested"
  done
codesign -f -s "$IDENTITY" --timestamp=none --generate-entitlement-der --entitlements "$TMP/ent.plist" "$APP"
codesign --verify --deep --strict "$APP"
echo "    signed and verified"

[ "$MODE" = sign ] && exit 0

# Resolve --device (alias, name, identifier or UDID) or pick the first
# connected device whose platform matches the app.
xcrun devicectl list devices --json-output "$TMP/devices.json" > /dev/null
DEVICE=$(python3 - "$TMP/devices.json" "$PLATFORM" "$DEVICE" <<'PY'
import json, sys
path, platform, want = sys.argv[1], sys.argv[2], sys.argv[3].lower()
aliases = {"iphone", "phone", "ios", "tv", "appletv", "tvos"}
for d in json.load(open(path))["result"]["devices"]:
    hw, props = d.get("hardwareProperties", {}), d.get("deviceProperties", {})
    if d.get("connectionProperties", {}).get("tunnelState") == "unavailable":
        continue
    if want and want not in aliases and want not in (
            d["identifier"].lower(), str(hw.get("udid", "")).lower(), props.get("name", "").lower()):
        continue
    if hw.get("platform") != platform:
        if want and want not in aliases:
            sys.exit(f"{props.get('name')} runs {hw.get('platform')}, but this app is a {platform} build")
        continue
    print(d["identifier"])
    break
PY
)
[ -n "$DEVICE" ] || { echo "no connected $PLATFORM device found (xcrun devicectl list devices)" >&2; exit 1; }

echo "==> Installing on $DEVICE"
xcrun devicectl device install app --device "$DEVICE" "$APP"

# tvOS apps have no persistent storage outside the bundle: the app reads its
# content from Library/Caches there (launcher_main/ios_main.cpp), which the
# system may purge; rerun with --with-content after a purge.
CONTENT_ROOT=Documents
[ "$PLATFORM" = tvOS ] && CONTENT_ROOT=Library/Caches
for c in $CONTENT_NAMES; do
  echo "==> Copying content $c/ to $CONTENT_ROOT/$c (unchanged files skipped)"
  xcrun devicectl device copy to --device "$DEVICE" --domain-type appDataContainer \
    --domain-identifier "$BUNDLE_ID" --source "$HOME/deploy/content/$BUNDLE_ID/$c" --destination "$CONTENT_ROOT/$c" --quiet
done

case "$MODE" in
  launch)  echo "==> Launching $BUNDLE_ID"
           xcrun devicectl device process launch --device "$DEVICE" --terminate-existing "$BUNDLE_ID" ;;
  console) echo "==> Launching $BUNDLE_ID with console"
           xcrun devicectl device process launch --device "$DEVICE" --terminate-existing --console "$BUNDLE_ID" ;;
esac
REMOTE
