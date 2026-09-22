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
Existing Source D3D9 shader artifacts remain inputs to that provider; this does
not establish a new native Vulkan material renderer or shader parity.

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
LD_LIBRARY_PATH="$PWD/build/dependencies/dxvk-native-2.7.1/usr/lib" \
python3 tools/quality/portal_boot.py \
  --runtime /path/to/portal-runtime --build build-portal-vulkan \
  --out build/portal-native-evidence-run1 \
  --require-vulkan --require-sdl3 --require-wayland
```

The boot runner records source/build identities, actual runtime module paths,
window and renderer markers, map startup, screenshot and process results.
Required profile acceptance also includes architecture/style gates and the
shared dynamic-library positive and negative-provider suites. Runtime evidence
belongs in the runner's output, not in a copied dependency matrix. Existing
repository provenance and distribution restrictions still apply to engine
code, external binaries, and separately supplied content.
