# Dedicated composition migration slice

Updated: 2026-10-08 (R12 done; see the closure below)

The dedicated child application group now registers one `IAppSystem` compatibility
bridge. `ApplicationComposition` orders, connects, initializes, rolls back, and
tears down the product systems behind it. The bridge keeps the existing interface
names, `CreateInterface` lookup behavior during legacy `Connect` callbacks, and
the host's separate `PreInit`/`PostShutdown` phases. The child group's physics
module handle transfers to composition and unloads after provider teardown. New
composition capabilities stay unpublished until connection succeeds.

The dedicated exports are the first native service in this graph. Composition
owns `IDedicatedExportsService`; the preserved `IDedicatedExports` singleton
forwards mod-facing calls only while the service is bound. Other dedicated
services remain module-owned `IAppSystem` implementations reached through
explicit transition adapters. The bridge and the same authoritative composition
kernel source are compiled with the legacy target's libstdc++ ABI, so no STL
object crosses an ABI variant at this boundary.

## Evidence on Linux x86_64, Portal profile

- `WAFLOCK=.lock-waf-ded-composition-tests ./waf build
  --targets=dedicatedcompositiontest,appsystemgrouptest -j8`: passed. The tests
  cover mod-facing lookup, ordered lifecycle, rollback at connection and
  initialization, native service construction failure, restart, module unload
  order, and host cleanup.
- `python3 tools/quality/conformance.py check --domain Q-FOUNDATION`: 17/17
  suites matched, including 252 composition checks.
- `WAFLOCK=.lock-waf-ded-composition ./waf build
  --targets=dedicated,dedicated_launcher,server -j8`: passed.
- `python3 tools/quality/toolchain_boundary.py check
  build-ded-composition-tests/toolchain-invocations.json
  build-ded-composition/toolchain-invocations.json`: passed, zero errors.
- One Portal `testchmb_a_01` dedicated BSP/BSP2 installed-runtime probe passed;
  see `quality-results/ded-composition-smoke-final/evidence.json`.
- `python3 tools/stylelint/stylelint.py --changed`: no finding in this slice;
  the full current branch diff is red from a concurrent shader edit. `archlint
  check --all`, baseline verification, and loader inventory remain red from
  unrelated current-tree changes; no finding in the new bridge paths remains
  after the legacy ABI classification.

This does not close R06 or R12. The parent Steam application still owns its
bootstrap services, the module resolver is still legacy, and most dedicated
systems remain legacy implementations. The current dedicated
binary also links material/studio render support and has not met R12's absent
render/UI proof. Linux native runtime evidence does not certify Windows or the
other planned platforms.

State on 2026-09-25: the dedicated Waf target still links `materialsystem`,
`shaderapiempty` and `studiorender`, and `CSys::LoadModules` still loads the
physics provider by filename. The dedicated builds pass: the R01 re-audit
(2026-09-25) built `build.dedicated` (gcc) and `build.dedicated-clang` after
`97e298c6` replaced the protected `LoadModule` call. The clang build also
needed a `(void *)` cast of `CSys::GetProcAddress`'s `long` handle for
`dlsym` (`sys_linux.cpp:118`). [`quality/baseline.json`](../quality/baseline.json)
records both as `pass` (user decision). This is build evidence only; the
installed startup/shutdown and link gates of R12 remain open.

## R12 closure: no render or desktop UI in the dedicated product (done 2026-10-08)

R12 asks for installed startup, shutdown and partial-failure evidence, and
link and runtime evidence that render and desktop UI are absent. RFC 0001
requires them "absent rather than stubbed", so the null shader API under a
material system no longer counts.

### What changed

- **Composition.** The dedicated roots (`dedicated/sys_linux.cpp`,
  `dedicated/sys_windows.cpp`) compose no material system, studio renderer,
  studio data cache or shader API, and on Windows no VGUI or input system.
  The dedicated product builds none of them: `materialsystem`,
  `shaderapiempty`, `shaderlib`, `studiorender`, `vgui2/*` and
  `inputsystem` left its project list, and it installs no `SDL2.dll`.
- **Windows desktop UI retired.** The `-vgui` configuration window
  (`dedicated/vgui/`, deleted) and the `CVguiSteamApp` base are gone. The
  Windows server always runs its text console, as `-console` did.
- **Material definitions, one owner.** `materialsystem/vmt_definition.{h,cpp}`
  holds the rules that turn a `.vmt` into a definition, moved out of
  `cmaterial.cpp`, `shadersystem.cpp`, `cmaterialsystem.cpp` and
  `ctexture.cpp`:
  - patch expansion and the PBR definition checks;
  - the builtin fallback block a device profile selects;
  - conditional variables and the material flag names;
  - the `materials/` file names of materials and textures.

  The material system uses it with a profile built from its hardware config.
  The dedicated server uses it with `VmtDedicatedServerProfile()`: the values
  the empty shader API gave it (DX 90, `ps_2_b`, no HDR, no sRGB blending,
  `gpu_level` 3).
- **Server material bridge.** On the dedicated engine (`SWDS`),
  `engine/server_material.{h,cpp}` answers every `IMaterial` the engine and
  the preserved `IVModelInfo` queries hand out (`func_breakablesurf`, server
  mods) from the material's definition: name, variables, flags, and its
  representative texture's VTF header for sprite sizes and frames. It's a
  scoped compatibility bridge; rendering queries answer as an unshaded
  material does. `GL_LoadMaterial` and the collision and displacement surface
  properties go through it.
- **Device facts, one owner.** `engine/device_facts.{h,cpp}` answers the DX
  level and HDR state that map loading reads: from the hardware config on a
  client, and from the dedicated profile on a server (HDR always enabled,
  which is what the empty shader API reported).
- **Optional render dependencies.** MDLCache serves models, vertex,
  animation and collision data without a studio renderer (no studio meshes).
  The particle manager starts without a material system.

### Defects fixed

- **Surface properties.** On every one of 26 Portal maps the old dedicated
  server gave some world surfaces wrong surface properties. Its
  `FindVar("$surfaceprop")` on an unshaded material returned the material's
  `$flags` value (for example `2105472`), so `GetSurfaceIndex` failed and
  the surface took an invalid property. That's 1,121 of 6,083 surfaces,
  among them glass, metal and tile. A dedicated server now matches a listen
  server on all of them, which changes impact sounds and friction on
  dedicated servers toward listen-server behavior.
- **Sprite sizes.** Sprite sizes and frame counts came from the error
  texture (32×32, one frame) on the old dedicated server. They now come from
  the base texture, as on a listen server.

### Evidence (2026-10-08, worktree branch `r12-dedicated`)

| Check | Profile | Result |
| --- | --- | --- |
| `dedicated_server.py absence` (files, `DT_NEEDED`, defined code, `LD_DEBUG=files` loads) | linux-x86_64, clean install | 77 / 0; the install is 12 files |
| `dedicated_server.py absence --wineprefix` (files, imports, Wine module loads) | windows-x86_64 (MSVC 19.44, Wine) | 71 / 0 |
| `dedicated_server.py lifecycle` | linux-x86_64 | 9 / 0 (see below) |
| Windows server `+map testchmb_a_00 +quit` under Wine | windows-x86_64 | loads the map, exits 0 |
| `dedicated_server.py facts-check` against the listen-server reference | linux-x86_64, 26 Portal maps | 26 / 0, one recorded deviation |
| `dedicated_server.py selftest` | host | 6 / 0 |
| `dedicatedcompositiontest`, `appsystemgrouptest` | linux-x86_64 `--tests` | pass |
| `tier0_facade.py --platform windows-x86_64` (R103) | windows-x86_64 | 393 / 0 |
| `portal_boot.py --headless` | portal client | pass |
| A fresh listen-server recording against the reference | portal client, 26 maps | 26 / 0: the client's material loading is unchanged by the refactor |

The lifecycle check covers these cases:

- startup, map load, `status` and quit, with exit 0;
- a level change;
- the IVP fallback provider;
- a missing physics provider fails startup and names it;
- a missing map leaves the server running;
- a missing game ends without a crash signal.

Negative controls:

- The pre-R12 dedicated product fails `absence` on every axis: it installs,
  links, defines and loads the material system, studio renderer and shader
  API, and on Windows `vgui2.dll`, `inputsystem.dll` and `SDL2.dll`.
- The self-test's seeded violations each fail.
- A stale recorded deviation fails `compare`.

The listen-server reference is
`quality/fixtures/dedicated-server/portal-listen-facts.json`, recorded with
`dedicated_server.py facts --listen` from the Portal client on native Vulkan,
headless. It covers 26 maps: collision and displacement surface properties,
plus sprite facts and materials of every model loaded on both sides (128 to
251 per map).

The one recorded deviation is in `deviations.json`. On a listen server,
`particle/smokestack.vmt`'s `ParticleSphere` shader doesn't treat
`$basetexture` as a texture, so its sprite size comes from `$bumpmap`. The
server has no shader schemas. Only `env_sprite` bounds read sprite sizes on a
server, and this material isn't one.

Conformance rows: `dedicated.selftest`, `dedicated.absence`,
`dedicated.lifecycle`, `dedicated.material-facts` and `dedicated.join`.

Reproduction:

```sh
rm -rf out/dedicated-linux/dev/install && ./kiln build dedicated-linux && ./kiln package dedicated-linux
python3 tools/quality/dedicated_server.py absence --install out/dedicated-linux/dev/install \
    --runtime out/dedicated-linux/dev/runtime
python3 tools/quality/dedicated_server.py lifecycle --runtime out/dedicated-linux/dev/runtime
python3 tools/quality/dedicated_server.py facts-check --runtime out/dedicated-linux/dev/runtime \
    --reference quality/fixtures/dedicated-server/portal-listen-facts.json \
    --deviations quality/fixtures/dedicated-server/deviations.json
./kiln build dedicated-windows && ./kiln package dedicated-windows
python3 tools/quality/dedicated_server.py absence --install out/dedicated-windows/dev/install \
    --runtime out/dedicated-windows/dev/runtime --wineprefix out/dedicated-windows/dev/wineprefix
./kiln build portal && ./kiln package portal
python3 tools/quality/dedicated_server.py join --runtime out/dedicated-linux/dev/runtime \
    --client-runtime out/portal-linux-native-vulkan/dev/runtime
```

### Open items and scope

- **Engine render units.** The dedicated `libengine` builds none of the
  engine's render translation units: `gl_rsurf`, `l_studio`, `r_decal`,
  `decal_clip`, `Overlay`, `disp`, `disp_interface`, `disp_mapload`,
  `gl_shader`, `matsys_interface`, `r_linefile` and `OcclusionSystem`.
  The geometry the server reads from them is now in
  `engine/brush_model_geometry.cpp`, built into every engine and deleted
  from the originals: brush model planes, surface centroids and
  `R_ComputeLightingOrigin`. The remaining client-only calls in
  `modelloader`, `staticpropmgr`, `host`, `cmd`, `sys_engine` and
  `cbenchmark` are compiled out under `SWDS`. Server displacement collision
  comes from the collision model's own trees, and the facts gate shows the
  same displacement surface properties as before. `absence` also rejects
  defined engine render entry points (`R_DrawWorldLists`, `DispInfo_*`,
  `Shader_*`, `CModelRender::`, `COverlayMgr::`, `COcclusionSystem::` and
  others), with a seeded self-test case.
- **A client joins and plays.** `dedicated_server.py join`
  (`dedicated.join`) starts the dedicated server and a headless Portal
  client, which connects over UDP. Both pass `net_usesocketsforloopback 1`,
  because packets to 127.0.0.1 otherwise take the in-process loopback, which
  a second process can't reach. The client walks forward (`+forward`) and
  becomes an active player on `testchmb_a_01`. An RCON `changelevel` then
  moves it to `testchmb_a_02`, where it is active again. Result: 5 of 5
  checks. An earlier version of this record said no client could join a
  dedicated server. That was wrong: the test never used real sockets.
- **Physics by filename.** The dedicated root still loads its physics
  provider by filename. That's R39's concern, not an R12 criterion.
- **Profiles.** R12's dedicated profiles are `dedicated-linux` and
  `dedicated-windows` (MSVC under Wine), and both gates pass. No Apple or
  Android dedicated product is declared. Apple and MSVC runners are optional
  (user decision, 2026-09-25). Hosted CI was not run: the user tests
  locally, and the rows are in the manifest for when it is.
