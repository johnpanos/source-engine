# RFC 0001 Phase B completion

Updated: 2026-09-25

Phase B removes the app-framework pseudo-module path for factories that are
already linked into a product. It does not complete the later typed provider,
extension-host, or public-loader retirement phases.

## Status

Phase B is complete in the source tree. The temporary legacy-factory bridge is
separate from native module ownership, all current linked consumers use explicit
`IAppSystem` instances, and architecture lint prevents factory values from being
passed back to `LoadModule`.

| Requirement | Result | Evidence |
| --- | --- | --- |
| Rename `LoadModule( CreateInterfaceFn )` | Complete | `CAppSystemGroup::AddLegacyFactory`; `ARCH106` rejects the retired spelling and factory-valued `LoadModule` calls |
| Accept explicit `IAppSystem` instances | Complete | `AddSystem( IAppSystem *, ... )` returns the accepted instance; `appsystemgrouptest` composes and runs one directly |
| Convert linked cvar systems | Complete | `VStdLib_GetICVar()` is the typed linked accessor used by app framework, dedicated server, Hammer launcher, GC, and retained tool callers |
| Convert linked filesystem systems | Complete | The dedicated composition injects its linked `g_pFileSystem`; filename-loaded desktop filesystem providers remain real modules for later phases |
| Convert SDL and tool systems | Complete | SDL is injected through `CreateSDLMgr()`; linked data-model/serializer systems remain direct and StudioMDL now injects `StudioMdl_GetDataCache()` |
| Remove fake unload ownership | Complete | Native modules live in `m_Modules`; legacy factories live in `m_LegacyFactories`, are removed with system composition state, and never reach `Sys_UnloadModule` |

The unit-test launcher now gives discovered test-library paths to the app-system
group as real modules. It no longer opens a library manually and then registers
its factory as a second pseudo-module.

## Verification

The following checks are the reproducible Phase B evidence:

```text
python3 -m unittest discover -s tools/archlint/tests -v
./waf configure --tests --use-sdl=0 --use-togl=0 --use-ccache \
  --disable-warns -T release -o build-phase-b-tests \
  --prefix=/tmp/source-engine-phase-b-out
./waf build --targets=appsystemgrouptest -j4
LD_LIBRARY_PATH=build-phase-b-tests/tier0:build-phase-b-tests/vstdlib \
  build-phase-b-tests/unittests/appframeworktest/appsystemgrouptest
./waf build --targets=unittest -j8
./waf configure --dedicated --use-sdl=0 --use-togl=0 --use-ccache \
  --disable-warns -T release -o build-phase-b-dedicated \
  --prefix=/tmp/source-engine-phase-b-dedicated-out
./waf build --targets=dedicated,dedicated_launcher -j8
python3 tools/archlint/archlint.py check --all
python3 tools/archlint/archlint.py baseline --verify
python3 tools/archlint/archlint.py inventory --verify
```

`appsystemgrouptest` checks explicit and temporary legacy-factory composition,
lookup during connect, forward connect/init order, reverse shutdown/disconnect
order, and application callbacks. The Linux test executable, migrated unit-test
launcher, and dedicated library build pass. The dedicated build also carries a narrow macro fix that
keeps its POSIX `GetProcAddress` method name from expanding through the
instrumented `dlsym` wrapper; the underlying `dlsym` operation remains covered
by loader telemetry.

No native Windows, Apple, or Android runner is available in this workspace, so
those profiles are not newly certified by this phase. Their ordinary profile
gates remain required before a platform support claim.

## Later work not claimed here

Phase C now has typed linked launcher factories and a dedicated-server caller
cohort under verification. Phase D has a working linked renderer/null catalog
and SDL3/Wayland/Vulkan Portal consumer. The [current progress
record](0001-portal-vulkan-progress.md) distinguishes these delivered slices from
the remaining provider catalogs and first-party shader loader. Phases E–G still own tool executable cleanup, named extension
hosts, and removal of the public loader. Consequently, portfolio item R39 remains
open even though its Phase B sub-scope is complete.

Source state on 2026-09-25 (not a gate claim):

- Phase C: the client launcher adds linked instances for input, the material
  system, data and model caches, studio render, video services, VGUI and the engine.
  The dedicated root uses the same pattern through its
  [composition bridge](0001-dedicated-composition-progress.md). Both roots
  still load the physics provider by filename (`-physics`, default
  `vphysics`). The launcher still loads the filesystem, `sourcevr`, `p4lib`,
  `vstdlib` and `hammer_dll` by filename.
- Phase D: the launcher selects window, input, video and audio providers from
  typed linked descriptors (`-window-provider`, `-input-provider`,
  `-video-provider`, `-audio-provider`). The standard shader library is linked
  into client and tools products and bound through
  `MaterialSystem_BindBuiltinShaderProvider`. Filename shader loading is left
  only for mod shaders, through the shader-extension compatibility host. No
  progress record yet covers the window, input, video and audio catalogs.
- Phase E: see the [Phase E record](0001-phase-e-progress.md).
