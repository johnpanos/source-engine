# Native Portal SDL3/Wayland/Vulkan profile

`portal-linux-wayland.json` owns the selected Linux x86_64 compatibility profile:
dependency versions, the DXVK Native archive URL/hash, compiler/ABI/dialects,
runtime provider selection, and required native checks. The `intent` and
`evidence` fields distinguish a configured target from verified product support.
Passing dependency checks does not certify graphics, gameplay, other platforms,
or release budgets.

The exact SDL3 **3.4.16** pkg-config version is a reproduction pin for this
profile, not a claim that every earlier/later SDL3 version is incompatible.
The same rule applies to the selected GCC and system dependency versions.
Use the ordinary distribution package manager to provide these versions, or
review a new profile and rerun its gates. Do not weaken an exact comparison to
make an untested host appear accepted. The Vulkan pkg-config pin identifies the
host SDK; real GPU/driver capabilities must still pass native startup.

DXVK Native comes from the upstream
[2.7.1 release](https://github.com/doitsujin/dxvk/releases/tag/v2.7.1), built for
the Steam Runtime. It is a native D3D9-to-Vulkan compatibility provider. It does
not require Wine. The selected WSI uses SDL3 as documented in the
[pinned upstream README](https://github.com/doitsujin/dxvk/blob/v2.7.1/README.md#dxvk-native).
Source-matched D3D9 shader artifacts remain inputs to that provider; this does
not establish a new native Vulkan material renderer or shader parity. The
product profile references the pinned offline compiler profile and versioned
opening-chambers workload. Wine is used by that offline compiler only, not by
the running game. See [shader artifact tooling](../../tools/quality/shader_artifacts.md).

From the repository root, fetch or verify the pinned dependency:

```sh
python3 tools/quality/product_profile.py fetch
python3 tools/quality/product_profile.py check \
  --dxvk-root build/dependencies/dxvk-native-2.7.1/usr
python3 -m unittest discover -s tools/quality/tests -p test_product_profile.py -v
```

The fetch command verifies archive size and SHA-256 before extraction. It
accepts only regular files, directories, and relative soname symlinks ending at
files in the same archive. Traversal, duplicate entries, special files, hard
links, link cycles, and excess expansion fail. Existing trees are compared with
the pinned archive on reuse; changed/missing/additional contents fail without
being overwritten. No downloaded commands are executed. Configuration can call
`load_profile`, `check_environment`, and `verify_dependency` from the same helper.

Use a separate Waf output directory and lock so the ordinary SDL2/OpenGL profile
is preserved. The example selects the current Portal compatibility profile:

```sh
WAFLOCK=.lock-waf-portal-vulkan ./waf configure \
  --platform-provider=sdl3 --render-backend=vulkan --build-games=portal \
  --dxvk-root=build/dependencies/dxvk-native-2.7.1/usr \
  -T release -o build-portal-vulkan --prefix=/tmp/source-engine-portal-vulkan
WAFLOCK=.lock-waf-portal-vulkan ./waf build -j8
WAFLOCK=.lock-waf-portal-vulkan ./waf install

SDL_VIDEO_DRIVER=wayland \
LD_LIBRARY_PATH="$PWD/build-portal-vulkan/tier0:$PWD/build-portal-vulkan/vstdlib" \
  build-portal-vulkan/unittests/platformtest/sdl3/sdl3_launcher_conformance
```

The native launcher test requires a real Wayland session and Vulkan-capable
window support. It checks input, window ownership, failure rollback and repeat
startup; absence of the required native surface fails rather than skips.

Supply an existing licensed Portal runtime containing `portal/gameinfo.txt` for
the product test. The runner creates its own writable staging tree and overlays
the Waf products; it does not rewrite the source content/runtime. The output
directory must be new for each run:

```sh
# Compile the versioned workload selected by the product profile.
python3 tools/quality/shader_artifacts.py \
  --out build-portal-vulkan/shaders/portal-matched --jobs 4

python3 tools/quality/portal_boot.py \
  --runtime /path/to/portal-runtime --build build-portal-vulkan \
  --shader-artifacts build-portal-vulkan/shaders/portal-matched \
  --out /path/to/new-portal-evidence \
  --require-vulkan --require-sdl3 --require-wayland --render-trace
```

Use `--map testchmb_a_01` for the second workload and `--width 1920 --height 1080`
for a full-HD capture. Each run needs a new output directory. An existing game
process can hold Source's single-instance lock; the runner reports that as failed
startup and does not stop the user's game.

The staged runtime is also playable without the runner's automatic screenshot
and quit sequence:

```sh
cd /path/to/new-portal-evidence/runtime
SDL_VIDEO_DRIVER=wayland SDL_VIDEODRIVER=wayland DXVK_WSI_DRIVER=SDL3 \
SteamAppId=400 SteamGameId=400 LD_LIBRARY_PATH="$PWD/bin" \
./hl2_launcher -game portal -windowed -w 1920 -h 1080 -novid -insecure \
  -renderer vulkan-compat +mat_queue_mode 0 +fps_max 120 +map testchmb_a_00
```

This pack covers the declared observed shader groups, not the entire game.
Additional map/material groups must be compiled and verified before a broader
playthrough claim. The original supplied runtime, configs and VPKs remain
unchanged; mutable files and replacement artifacts live in the private stage.


The boot runner records source/build identities, actual runtime module paths,
window and renderer markers, map startup, screenshot and process results.
It also verifies DXVK bytes again when staging, validates every shader source and
artifact hash, gives the replacement shaders explicit priority over VPKs, and
rejects uniform/blank images. Requested render traces are analyzed, not merely
checked for existence. See [render diagnostics](../../tools/quality/render_trace.md)
for per-material/state reports and their visibility limits.
Required profile acceptance also includes architecture/style gates and the
shared dynamic-library positive and negative-provider suites. Runtime evidence
belongs in the runner's output, not in a copied dependency matrix. Existing
repository provenance and distribution restrictions still apply to engine
code, external binaries, and separately supplied content.

# Android Portal SDL3/native Vulkan profile

`portal-android-native-vulkan.json` owns the Android client's facts:
- The NDK, SDL3, SDK platform and build-tools archives, with sizes and
  SHA-256 hashes.
- SDK levels and the declared ABIs.
- The manifest requirements: permissions, the Vulkan feature, the activity,
  and the configuration changes that keep the activity alive across rotation
  and foldable display swaps.
- The exact set of packaged native modules, the platform libraries they may
  link against, and the native entry point.

`build-android-apk.sh` fetches and verifies every pinned archive into
`dependencies/android/`, so no installed Android SDK is used. It needs a JDK,
plus `adb` for `--install`/`--run`. It builds each ABI under its own Waf lock
and `build-android/<abi>/` out directory, then packages and signs the APK.
Finally it runs the independent verifier [`android_apk.py`](../../tools/quality/android_apk.py)
on the result.

The verifier reads the finished APK and fails on:
- A dropped or undeclared module, or a wrong or missing ABI.
- A library built for the wrong machine, or with `PT_LOAD` alignment below
  16 KB.
- An unresolved `DT_NEEDED`, text relocations, or a missing `SDL_main`.
- A misaligned uncompressed library, or a missing activity class.
- Invalid or missing touch-control assets.
- Manifest drift (package, version, SDK levels, exact permissions, required
  features, native code, `extractNativeLibs`, required `configChanges`).
- Failure of `zipalign -c -P 16` or `apksigner verify`.
- The wrong build variant (`--variant`, default `debug`): a debug package that
  is not debuggable, or a release package that is debuggable, unsigned, or
  signed with the Android debug certificate.

When a module is intentionally added or removed, update
`android.packaged_libraries` in the same change.

```sh
./build-android-apk.sh --abi arm64-v8a            # or --all-abis
python3 tools/quality/android_apk.py check build-android/portal-0.1.0-arm64-v8a-debug.apk \
  --abi arm64-v8a --report /tmp/apk-check.json
python3 -m unittest discover -s tools/quality/tests -p 'test_android_*.py' -v
```

`--release` builds a release APK. It is not debuggable, and it is signed
with a key kept outside the repository. The default key is
`~/.android/source-engine-release.keystore` with the alias `source-engine`.
`ANDROID_KEYSTORE`, `ANDROID_KEY_ALIAS`, `ANDROID_KEYSTORE_PASS` and
`ANDROID_KEY_PASS` override these. Without a password variable, apksigner
prompts for it. The script checks the signing inputs before it builds
anything. `--new-release-key` creates the keystore once with `keytool`, and it
refuses to replace an existing one. Back up the key: updates to an installed or
published app must be signed with the same key. Distribution through a store
is still unverified. This produces an APK, not an Android App Bundle.

```sh
./build-android-apk.sh --new-release-key --all-abis   # first release build
./build-android-apk.sh --release --all-abis           # later release builds
./build-android-apk.sh --release --package-only       # re-sign existing native builds
```

The unit tests need no NDK or SDK. They build synthetic APKs and ELF files
and prove that each defect above is reported. They also check that the
manifest template and the profile agree, that every `vgui/touch/*` texture
`game/client/touch.cpp` names is generated by `tools/android/touch_icons.py`,
that the icons are deterministic, valid VTFs, and that the build script
parses.

CI (`.github/workflows/android.yml`) cross-builds and verifies every declared
ABI on each push and pull request. The unit tests run with the other
`tools/quality` tests in `composition.yml`.

These checks prove the package builds and is well formed. They do not provide
the profile's device evidence: installed-package smoke tests, rotation and
fold, and surface recreation stay `unverified` until they run on the declared
hardware.
