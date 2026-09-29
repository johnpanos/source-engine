# Portal 2 shared source import

The initial 70 `.cpp` and `.h` files moved from the local `portal` source drop
match the corresponding files in the local `cstrike15_src/game/shared/portal`
archive byte for byte. Four additional shared files came from the local game
drop, and `paint_enum.h` came from the archive. The original files retain their
copyright headers and line endings. The aggregate SHA-256 of sorted original
file names, NUL separators, and each original file's SHA-256 digest is
`efd90b1a1bf85ff619401357da49b1266853e898e429eb83c827ffab32259b6f`.

This is the shared-code cohort for a **separate Portal 2 client/server target**.
The existing `portal` target selects `game/shared/portal` explicitly through
`game/client/client_portal.vpc` and `game/server/server_portal.vpc`. Its source
selection must remain independent of these files.

The separate Portal 2 Waf configuration is available through `./play_p2
--configure-only`. The 94 selected gameplay sources that have Steam2
pseudocode, with their headers, have been reconstructed from it; every such
file carries a `Portal 2 reconstruction` header and is not original Valve
source. The VPC source maps still name 42 selected sources without pseudocode.
[MISSING.md](MISSING.md) lists them, the remaining include gaps and the
probe-compile status of the reconstructed files. Waf stops before compiling an
incomplete Portal 2 client or server. The source target needs those files, the
Source SDK 2013 base-game and engine API adaptations the Portal 2 code expects,
and game-specific conformance runs before it can be launched.
Historical Steam2 source references are isolated under
[`external/portal2_steam2_xsi`](../../../external/portal2_steam2_xsi/README.md),
[`external/portal2_steam2_xsi_legacy`](../../../external/portal2_steam2_xsi_legacy/README.md),
[`external/portal2_steam2_scripts`](../../../external/portal2_steam2_scripts/README.md),
and [`external/portal2_steam2_maps`](../../../external/portal2_steam2_maps/README.md).
The first two contain XSI tool source; the others contain VScript, Lua, FGD, and VMF
variants. [Debug-symbol-guided pseudocode](../../../external/portal2_steam2_decompiled/README.md)
is also retained as a reconstruction reference. None supplies buildable
gameplay C++ or enters the current build.
After moving every exact-path match from the local game drop, the other local
source archive has no exact-path copy of a remaining VPC source except a
PS3-specific `vjobutils.cpp`. Remaining gameplay sources need implementation;
same-named files from other games require semantic review before reuse.

`./play_p2 --prepare` stages the installed Portal 2 VPKs by symlink into a
private runtime. `./play_p2 --retail` starts the installed retail binary when
immediate play is needed; the default command targets this repository's build.
The installed Linux `portal2_linux` and game modules are 32-bit, while this
branch's configured Linux target is 64-bit, so the retail modules cannot fill
the missing source target.

`./play_p2` runs on Box3D (`vphysics_box3d`), as `./play` does;
`PHYSICS=vphysics ./play_p2` or an explicit `-physics` argument selects IVP.
On 2026-09-24, player movement and noclip were compared headless against IVP on
`sp_a2_triple_laser` and `sp_a1_intro3`. The check covered falling, walking,
friction, jumping, crouching, pushing a cube, noclip in all directions, and the
arrival-elevator start. Client-view positions (`getpos`) matched on both
backends. That needed three base fixes:
- Single-player local transfer now carries the origin z that Portal 2 sends
  separately from XY (`engine/dt_localtransfer.cpp`); before, the view froze
  vertically.
- `func_tracktrain` gained Portal 2's `MoveToPathNode`, `TeleportToPathNode` and
  `SetMaxSpeed` inputs and `OnArrivedAtDestinationNode` output.
- A train stopped at a destination node coasts only the 0.1 s look-ahead
  (`game/server/trains.cpp`). Otherwise it pushed against the player every tick
  and held walking near 26 u/s.

Box3D now also matches IVP when noclip is turned off inside geometry: its
player controller's speed budget ignores velocity the game set since the last
step, as IVP's does (see [RFC 0004 progress](../../../RFC/0004-progress.md)).
Quitting `sp_a1_intro3`, which has pre-placed portals, used to hang in
`CPSCollisionEntity::UpdateOnRemove`. The portal's embedded simulator reported
network changes to a wrong owner address until the `portal_base2d.h` fix, and it
now exits cleanly on both backends.

### Retail comparison: `sp_a2_triple_laser` (2026-09-25)

`quality/workloads/portal2-triple-laser-v1` holds the scenarios for
`tools/quality/portal2_scenarios.py`: the puzzle solution with 17 screenshots
(`sp_a2_triple_laser`, 7 checks) and a walk through a portal pair and back
(`sp_a2_triple_laser_traverse`, 5 checks; the room's lasers are switched off
because they shove the player). The retail Linux binary ran the same scripts
headless through a `mapspawn.nut` hook, since retail ignores `+wait` on the
command line. It ran at 1024x768, the SDL offscreen driver's largest mode, so
the frames line up. Both scenarios pass on retail, and on this build with IVP
and Box3D and with the default queued material system (`mat_queue_mode 2`).
Fixed on the way:

- Walking through a portal crashed the client, then spun the view back.
  `C_BaseEntity::Remove` now defers releases while entities simulate, as CS:GO
  does: a portal's `Simulate` removes ghost renderables the simulate loop has
  not reached. The client also never called `ProcessPortalTeleportations`, so
  the local view was never rotated, which retail does in
  `FRAME_NET_UPDATE_POSTDATAUPDATE_END`.
- Light-panel glass (`glass/glasswindow_refract01*`: `$localrefract`,
  `$envmapsaturation`) was dropped on native Vulkan and showed as white slabs.
  The refract pipeline now has Portal 2's `LOCALREFRACT` and cubemap terms.
- Elevator video screens showed white (no video provider), then black. Portal
  2 targets now configure `--video-provider=bink`, the FFmpeg provider. Its
  decode loop handles `EAGAIN`, draining, looping and `SetTime`. Its texture
  is the frame's size and BGR, and it reinstalls its regenerator before each
  upload: the VGUI surface replaces the regenerator of a procedural material's
  base texture.
- `dev/bloomadd` (Portal 2's bloom composite) and SpriteCard's `ANIMBLEND` and
  `ADDSELF` now run on native Vulkan (`kTexturedModeSpriteCard`).

Still different from retail: there are no projected-texture lights or shadows
(native flashlight passes are unimplemented, video-options P7); SpriteCard
`DEPTHBLEND`, the `warp2_warp` particle, `PortalStaticOverlay`, `EyeRefract`
and `dev/motion_blur` are missing; laser beams are wider and redder; one laser
segment shows as a horizontal line in the view through a portal; the elevator
video is about 1.35 times brighter. One heap corruption (a `double free` abort)
was seen once in about 20 runs and not reproduced.

### Laser through a redirection cube (2026-09-25)

A laser that hit a redirection cube kept going: the cube sent out its own
beam, and the client also drew the incoming beam straight through the cube.
The server stopped the beam at the cube. The client re-traces every beam each
frame with `MASK_SHOT & ~CONTENTS_WINDOW`, as retail does, and
`StandardFilterRules` skips "see-through" entities for masks without
`CONTENTS_WINDOW`. This SDK's engine marks a studio model translucent when any
of its materials is (the cube's `reflecto_cube_glass` is `$translucent`). The
client's `IsTransparent` counts that, but the server's only reads the render
mode. Portal 2's engine, like CS:GO's (`Mod_ComputeTranslucencyType`), never
sets that flag on studio models. So on the Portal 2 client,
`StandardFilterRules` now treats a studio model as see-through only when its
render mode is not normal, as retail and the server do. `IsTransparent` itself
is unchanged because it also picks the render group.

`sp_a2_triple_laser_cube_blocks` turns cube 2 north, west and south in the
wall laser, then moves it away. `cl_debug_laser_trace` (the client counterpart
of `sv_debug_laser_trace`) prints each drawn beam's end. The workload's
`console_checks` require the client and server beams to end at the cube in
every orientation, and the beam to reach the wall once the cube has gone. A
console check reads the lines between the driver's `QA_WINDOW <label>
BEGIN/END` markers; the evaluator's fixtures cover it. On the log from before
the fix, the three client checks fail (the beam ends at the far wall, x=8320)
and the server checks pass.

### Portal traversal and portal crashes (2026-09-28)

`quality/workloads/portal2-portals-v1` runs on `qa_portal_walk`, a chamber
built for it (`tools/quality/portal2_portal_map.py`; the runner compiles and
installs it through the workload's `maps` entry, with `vmf_map_build.py` and
Portal 2 materials from the staged runtime). The player fires both portals
with the gun and:

- `qa_portal_walk`: a shot at black metal leaves no portal; blue on the north
  wall, orange on the east wall, both where aimed; walks through both ways,
  then once more at 20 frames a second;
- `qa_portal_fall`: walks onto a floor portal and flies out of a wall portal,
  then falls through a floor/ceiling pair for four seconds and lands hard;
- `qa_portal_tunnel`: two portals facing each other, so every portal view
  holds another, with glow sprites and glass in view; looks and sweeps across
  them and walks through twice;
- `sp_a2_triple_laser_cube_removed`: a redirection cube carrying a laser is
  removed;
- `qa_portal_crowd`: 248 animated props (personality spheres and Chell
  models) appear in doubling waves between two facing portals while the
  player looks into them and turns.

The workload passes on this build (`./play_p2`'s Box3D and job-graph
arguments) and on a clang AddressSanitizer build of the same tree
(`--sanitize=address`; `new_delete_type_mismatch=0`, see below), as does the
triple-laser workload.

`cl_portal_view_trace 1` prints the client's final view every frame (`PVIEW`)
and each portal's pose when it changes (`PVIEW_PORTAL`);
`cl_portal_view_trace_shots <units>` also screenshots every frame while the
eye is that close to a portal opening. A `view_continuity` console check
(`tools/quality/portal_view_trace.py`) requires each frame's eye and view
direction to follow from the previous frame's, directly or through a portal's
matrix, within speed and turn limits, with the declared number of crossings.
Its fixtures (`tools/quality/tests/test_portal_view_trace.py`) fail a jump, a
view the teleport did not turn, a walk with no crossing, a crossing through an
inactive portal and a snap back.

Retail Portal 2 ran `qa_portal_walk` on the same BSP: both portals landed at
the same points, and the walks ended at x = 500.0 against 499.2 here, on the
centre line at 20 frames a second too. Retail has no view trace, so only its
QA checks compare.

Fixed:

- **Laser crash when a cube is removed.** A laser that continues through a
  portal or off a cube does so through a child `env_portal_laser`, parented
  to the cube. Removing the cube (fizzler, dropper, `Kill`) removes the child
  as an orphan, and the parent laser's raw `m_pChildLaser` then pointed at
  freed memory: its next think crashed in `CPortalLaser::FireLaser ->
  UTIL_Remove`. The same raw-pointer pattern crashed a level transition in
  `CLaserCatcher::UpdateOnRemove` (its target was freed first). The child
  laser, placement helper, sound proxies, catcher target and the target's
  catcher are now handles. With the old code the cube scenario ends in
  SIGSEGV with that stack; with handles it passes.
- **Commands made before the client saw a teleport.** The client sends
  `command_acknowledgements_pending` and `predictedPortalTeleportations` in
  every command, but the server ignored them, so commands built before the
  client knew of a teleport steered the player along the pre-portal heading:
  3 to 10 units off the exit portal's centre line at 20 to 30 frames a
  second. `CPortal_Player::PlayerRunCommand` now turns their view angles
  through the pending portal transforms, as CS:GO's Portal 2 code does.
- **World list read after portal views rebuilt it.**
  `CRendering3dView::DrawTranslucentRenderables` bound `info` to the view's
  world list before drawing portal views, which release and rebuild that list;
  CS:GO binds it after. The pooled list object is usually handed straight back,
  so this was not reproduced as a crash, but a nested portal view can take it.
- **Crash placing a portal: 32-bit pointer offset.**
  `CStaticCollisionPolyhedronCache` packs the world's brush polyhedrons into
  one block and moves their pointers by `int memoryOffset`. On 64-bit heaps
  more than 2 GB apart the offset truncates, the cached polyhedrons point
  nowhere, and the first portal placed next to a brush crashes in
  `ClipPolyhedron` (from `CPortalSimulator::CreatePolyhedrons`). glibc's
  arena usually keeps the two blocks close, so this build survived; the ASan
  build crashed on every first shot, and other allocators (Android's Scudo,
  Apple's malloc zones) can place them apart too. Portal 1's copy of the file
  already used `intp`; this one does too now.
- **Heap corruption from pooled bone setup.** Every animated model with 16 or
  more bones appends itself to `g_PreviousBoneSetups` on its first bone setup
  of a frame. The opaque-renderables bone setup of each view
  (`r_threaded_renderables` with the pooled `r_renderable_job_graph`, on by
  default here) runs that on pool threads and the main thread at once, and
  the append had no lock, so a growing list was reallocated under another
  thread: `double free or corruption (!prev)` from `realloc` in
  `C_BaseAnimating::SetupBones`, 2 aborts in 13 soak runs of
  `sp_a2_triple_laser`. Each portal view is another view, and entities seen
  only through a portal are appended there. A gdb probe caught a worker and
  the main thread in `SetupBones` in the same batch with the batch flag
  clear. The list now takes a lock for appends and removals, as CS:GO's
  `MarkForThreadedBoneSetup` does. Three crowd runs without the lock did not
  crash, so this rests on the probe, the abort stacks and CS:GO's fix.
- **Out of per-frame render data with many models in portal views.** The
  queued material system copies each model's bones into a per-frame arena of
  2200 KB. `qa_portal_crowd` filled it ("Out of memory in render data!") and
  the render thread then crashed in `CStudioRender::DrawModel` on a NULL bone
  array. Retail Portal 2's material system (CS:GO's) sizes the arena 6600 KB
  for `portal2` because every portal view redraws the scene; this engine now
  does too, and `CStudioRenderContext::DrawModel` skips a model whose bones or
  flex weights could not be copied instead of queueing a NULL draw.
- **Native Vulkan screenshot overflow.** The native backend's diagnostic
  report, printed on every screenshot, named stream records from a table of
  three kinds; queries and scene captures are kinds 3 to 5, so a frame with
  either read past the table and printed a stray pointer. The table now has
  all six and a bounds check (found by the ASan build).

- **Freeze with a light bridge through a portal (Box3D).** A bridge seen
  through a portal is a convex 4121 units long, 64 wide and 1/64 unit thick;
  Box3D's quickhull (`b3HullBuilder_ConnectFaces`) never returned on it, so
  `sp_a2_bridge_intro_bridge_portal` hung the moment the portals were placed.
  The Box3D provider now treats a convex thinner than 0.5 units and than
  1/1000 of its size as flat, flattening it onto its middle plane and
  colliding with a 0.5-unit slab, as it already did for flat pieces. The
  shared physics suite has `collide.sliver-convex-builds` and
  `collide.sliver-convex-blocks` with those exact points; without the change
  the suite hangs (killed at 400 s). The rest of the gate is unchanged: IVP's
  three known reference deficiencies and Box3D's three baseline gameplay
  failures, with every seeded fault detected. On retail's references the
  scenario now passes; the player stands 0.3 units higher on a bridge than
  retail (inside the tolerance).

Crashes in the user's core dumps from 2026-09-25/26 match the laser fix (both
stacks). One render crash, in the nested `DrawTranslucentRenderables` under
`DrawPortalsUsingStencils`, is in the function the world-list fix changes but
was not reproduced. Four `PlayerRoughLandingEffects` sound crashes all fell
between 02:56 and 03:20 on 09-26, while the sound emitter was being changed;
they do not reproduce (`qa_portal_fall` now lands the player at about 1,050
units/s and checks it, `loop.landed`).
Left open: the ASan build reports `new-delete-type-mismatch` in
`CSquirrelVM::TranslateCall` (a sized delete of a mismatched type in the
VScript bridge); it does not crash a normal build.

`portal2_physics.py` also runs these portal cases against retail. Retail
references for `sp_a2_bridge_intro_bridge_portal` and
`sp_a4_tb_intro_funnel_portal` were missing and are now recorded (3 retail
runs each). Open, all pre-existing:
`sp_a4_tb_intro_funnel_portal` carries the player 207 units along the exit
portal's funnel where retail carries 477, and drops 56 below it (retail 36);
`sp_a2_triple_laser_fling.cube.landing_x` (a cube flung through a portal
lands 155 units short of retail); `engine.frame_time`; and
`sp_a2_bridge_intro_bridge.stuck.sink` (64 units, retail 0), which fails the
same before and after the bridge change. The funnel, fling and bridge
scenarios pass under ASan.

### Portal surface: z-fighting and edge particles (2026-09-28)

User report: "Portal Base2D has glitched particles and z-fighting". The new
`qa_portal_surface` scenario (`portal2-portals-v1`) places both portals on
facing walls of `qa_portal_walk` and shoots blue close, far and at grazing
angles, and orange close. It ran on this build and on retail (the retail
capture recipe of `portal2_material_shots.py`, on a mirror whose
`portal2/maps` holds the same BSP).

Reproduced: stripes and wall-coloured patches across the portal view and rim,
the view lost at grazing angles, and pink, orange and rainbow shards around
the rim. Retail shows none of them.

Fixed:

- **Z-fighting.** Retail's `Portal` and `PortalRefract` vertex shaders (CS:GO
  `portal_vs20`, `portal_refract_vs20`) push each vertex along its normal by
  its TEXCOORD1.x, the decal offset the portal meshes carry (up to 0.24 units
  toward the camera, 1 unit in views nested more than one deep). This
  engine's shaders are Portal 1's and read no offset, and the Portal 2 client
  had dropped its own CPU offset, so the stencil hole, the pre-stencil
  refract pass and the rim effects were drawn in the wall's plane.
  `CPortalShaderDecalOffset` (`portalrenderable_flatbasic.h`) applies the
  same push as an object-space model translation, only when the bound shader
  is `Portal*`/`PortalRefract*`; WriteZ and the static overlays stay in the
  wall's plane, as in retail. Shared shaders and Portal 1 are unchanged.
- **Rainbow shards.** SpriteCard's spline format has declared TEXCOORD4 (sheet
  range) and TEXCOORD5 (end colour) since the Portal 2 sprite-trail port, but
  `render_rope` wrote only TEXCOORD0-3, so the native backend read stale
  vertex memory as UVs and tail colour. The rope now writes CS:GO's vertex
  (`FastRopeVertexNormal_t`): full sheet range, the end particle's colour
  (alpha x alpha2) and, for `$orientation 3`, both end normals in
  TEXCOORD6/7. SpriteCard declares 8 texcoords for `$orientation 3` spline
  cards, and the native backend orients those cards by the interpolated
  normals (CS:GO `splinecard_vsxx.fxc` ORIENTATION 3). The D3D9 splinecard
  shader still faces the camera.
- **End caps.** The particle library ignored `operator end cap state` (168
  operators in retail PCFs set it). `portal_edge`'s decay, alpha ramp, drift and
  scatter operators run only in its end cap, so here the ring particles died
  after 1.5 s (the root rope had 0 particles), faded to alpha 0 and drifted up
  to 50 units off the wall. Ported from CS:GO: `m_nOpEndCapState` gates
  operators, renderers and emitters; `StopEmission( ..., bPlayEndCap )` runs
  the end cap and starts "end cap effect" children (`fire_01.pcf`).
  `CParticleProperty::StopEmission` takes CS:GO's `bForceRemoveInstantly` and
  `bPlayEndCap`. The portal edge and the tractor beam arms
  (`tractor_beam_arm_b` decays only in its end cap) stop with their end cap.
- **The exit portal's edge in the portal view.** The pink and orange shards
  inside the rim were the other portal's edge particles, seen from behind its
  wall by the view through the portal. Ported CS:GO's renderer culling ("cull
  system when CP normal faces away from camera", "cull system starting at
  this recursion depth") for `render_animated_sprites`, `render_sprite_trail`
  and `render_rope`, and the system's "maximum portal recursion depth";
  `CNewParticleEffect::DrawModel` passes the Portal 2 view recursion level.

Evidence (build-p2, native Vulkan, private runtime): `portal2-portals-v1`
passes 7 of 7 scenarios; `client.particle-plane-crossing` and `.original` pass.
In `qa_portal_surface` the stripes, patches and shards are gone; with
`r_portal_fastpath 0`, `r_portal_earlyz 0`, ghosting off, stencil depth 1 and
complex frustums off the portal draws the same. `portal2_material_shots.py
suite` gives 60 pass, 10 fail: `sp_a2_triple_laser.portal_b.tiles` now fails
(82.3%, was 87.0% on a 2026-09-25 capture) because the orange rim is drawn
whole instead of broken by the wall. The rim itself is brighter and thicker
than retail's, which `portal_a`/`portal_b.portal_rim` already pin.

Open: retail draws a soft glow band just outside the rim (+44 blue inside the
band with particles on; 16,896 band pixels at 1024x768). Overriding
`particle/beam_portaledge_04_oriented_add` with `$overbrightfactor 40` in a
retail `portal2_dlc3/pak01_dir.vpk` (VPK version 1; loose files there are not
read) lights exactly that band, so it is `portal_edge`'s root rope. Here the
same rope lands mostly under the rim (2,024 band pixels, +5 blue): its
particles sit at 0.94 of the portal's size, and the edge texture is brightest
on the ribbon's centre line. Its simulation inputs (ring, radius, alpha,
normals, control points) match the PCF; the cause is not found.
Also open: `info_particle_system` has no `StopPlayEndCap` input
(`sp_a2_bts3`, `sp_a4_finale4` fire it); adding it changes that entity's
networked table for every game.

### Portal 2 video retail conformance (2026-09-25)

This section covers materials, shaders, proxies, textures and particles as
rendered on native Vulkan, compared with retail Portal 2 from matched
screenshots.

`quality/workloads/portal2-materials-v1` holds fixed views: an eye point with a
pitch and yaw, noclip, no HUD or view model. There are 13 views on five maps:
`sp_a2_bridge_intro`, `sp_a2_fizzler_intro`, `sp_a2_laser_over_goo`,
`sp_a2_triple_laser` and `sp_a4_tb_intro`.

[`tools/quality/portal2_material_shots.py`](../../../tools/quality/portal2_material_shots.py)
shoots the views at 1024x768 on both sides.

- On this build, the views run through `portal2_scenarios.py` (SDL offscreen).
  The same engine arguments as `./play_p2` apply (`+mat_colorcorrection 1`),
  plus `+sv_cheats 1`.
- On retail, `portal2_linux` runs in a private symlink mirror of the install,
  with its own `portal2/cfg`, `update/cfg` and `scripts`. A `mapspawn.nut`
  hook starts the same scripts on the server VM. It runs under
  `mutter --headless` with SDL on Xwayland.
  - Retail saves `config.cfg` to `update/cfg`, which Steam Cloud syncs and
    every staged runtime mounts ahead of its own cfg. Until 2026-09-28 the
    mirrors linked `update/` whole, so harness settings (`hud_quickinfo 0`
    from the paint workload, `closecaption 0`, `snd_mute_losefocus 0`) were
    saved into the player's config, and `./play_p2` lost its crosshair
    brackets. `stage_portal2_runtime.private_retail_write_dir` now gives
    every mirror a private `update/cfg`.
  - Retail needs a running Steam client ("Steam is not running" otherwise). The
    session uses the user's Steam when one is running. Only when none is running
    does it start one inside the compositor and shut it down afterwards.
  - Retail's settings are its own saved ones: MSAA 4x, anisotropic 16x and
    `mat_hdr_level 2`. The views set `mat_forceaniso 16` on both sides.

`check` judges a build capture against `reference.json`, which is recorded from
a retail capture and holds numbers only:

- a 16x12 grid of tile means per shot;
- statistics for each declared region.

The checks, listed in `checks.json`, are:

- `<map>.run`: the scenario ran and every view was placed.
- `<map>.shaders`: no unknown shader beyond retail's own.
- `<map>.proxies`: no missing material proxy.
- `<map>.conditionals`: the material system understood every VMT conditional
  that retail's did.
- `<map>.<view>.tiles`: at least 85% of tiles are within 24 levels of retail.
  Two retail runs agree within 8 levels, except animated goo and beams.
- `<map>.<view>.<region>`: each region pins one defect.
- `<map>.<view>.native_drops`: the native backend's per-screenshot census names
  no dropped material. The one exemption is `___error`, from the texture
  manager's precache draws; gdb backtraces show that is its only source.

Rows in `quality/conformance.manifest.json` (profile `linux-host-corpus`):

| Suite | Kind | Now |
| --- | --- | --- |
| `corpus.portal2.video-retail` | 60 checks, known failure | 34 pass, 26 fail; first divergence `sp_a2_bridge_intro.projector.tiles` (the white void, item 2 below) |
| `...video-retail.cables` / `.seeded-no-ropes` (`+r_drawropes 0`) | region + negative control | pass / rejected |
| `...video-retail.ssbump-floor` / `.seeded-ssbump-unscaled` (`+mat_ssbump_normalize 0`) | region + negative control | pass / rejected |
| `...video-retail.comparator` and 4 `.comparator.seeded-*` rows | synthetic frames, no content | pass / all 4 rejected |

The in-game rows are optional. They need `SOURCE_PORTAL2_STEAM_ROOT` (a Portal 2
install), a GPU and a built `build-p2`; `SOURCE_PORTAL2_BUILD` selects another
tree. Without them the rows are skipped and never certified.

```sh
python3 tools/quality/portal2_material_shots.py capture --side retail --out quality-results/p2mat-retail
python3 tools/quality/portal2_material_shots.py record --retail quality-results/p2mat-retail
python3 tools/quality/portal2_material_shots.py capture --side build --build build-p2 --out quality-results/p2mat-build
python3 tools/quality/portal2_material_shots.py check --capture quality-results/p2mat-build
SOURCE_PORTAL2_STEAM_ROOT=~/.local/share/Steam/steamapps/common/Portal\ 2 \
  python3 tools/quality/conformance.py check --suite corpus.portal2.video-retail
```

`capture --renderdoc` runs the build under `renderdoccmd`; a view script sends
`vk_renderdoc_capture`.

Fixed, each confirmed against retail by the checks above (before -> after):

- **SSBump lighting 1.73x too bright.** Portal 2's LightmappedGeneric, like
  CS:GO's, always scales an ssbump's three basis weights by 1/sqrt(3).
  - This SDK's shader never scales them. Its native port only did so for
    `$ssbumpmathfix`, a parameter Portal 2's shaders do not have (retail
    `stdshader_dx9.so` holds no such string).
  - Portal 2's chamber floors and walls are ssbump, so frames were about 1.4x
    retail in 8-bit sRGB. That held with the tone-mapping scale forced to 1 on
    both sides, so it was not exposure.
  - Native Vulkan now scales them when the game is Portal 2
    (`mat_ssbump_normalize -1`; 0 and 1 force either way).
  - Result: `sp_a2_triple_laser` floor mean 106 -> 77 (retail 72), and the
    entry-hall and floor shots' tiles 30% / 0% -> 97% / 100%.
- **Cables missing.** Every rope material (`cable/*.vmt`) names `SplineRope`,
  which this shader library lacked, so ropes drew as the error material and
  were dropped.
  - `SplineRope` is now a fallback to `Cable_DX9` (`cable_dx9.cpp`). The pixel
    shaders are the same, and this client builds rope geometry on the CPU in
    Cable's vertex format.
- **HDR cubemaps black.** RGBA16161616F textures (every map `c*.hdr`
  env_cubemap, the authored HDR cubemaps) became 8-bit images and never
  received texels.
  - They are now `R16G16B16A16_SFLOAT` images with the half floats uploaded
    as-is.
  - Result: the observation glass in `sp_a2_fizzler_intro` has mean 31 -> 64
    (retail 64).
- **Observation windows and WorldVertexTransition dropped.**
  - Native Vulkan compared shader names against `WorldVertexTransition`, but
    the material system reports the fallback it chose, `WorldVertexTransition_DX9`.
  - Screen-space Refract was dropped whenever `$envmapsaturation` was not 1.
    Portal 2's observation glass uses 0.5, which this library's refract_ps2x
    also applies.
  - Both now draw, and the census lists neither.
- **Tractor beam column never drawn.** A model-less trigger has empty render
  bounds, so the leaf system never reached `C_Trigger_TractorBeam::DrawModel`.
  - The trigger now bounds the beam from start to end.
  - Its translucency is bridged onto `IsTransparent`, and it refreshes its
    render bounds when the beam changes.
- **Portal 2's GPU-level VMT conditionals ignored.** Portal 2 materials pick
  variables (`"GPU>=1?$reflecttexture"`) and whole blocks (`"GPU>=1" { ... }`,
  which carry the WorldVertexTransition blends' bump maps and blend modulation)
  by `gpu_level`. This SDK's material system knew neither: it warned for every
  `GPU<n?` variable and dropped every `GPU>=n` block.
  - `cmaterial.cpp` now evaluates both as CS:GO does, against the game's
    `gpu_level` (3, high, when no game registers one).
  - The warnings are gone, and `<map>.conditionals` checks for them.
- **`BloomAdd` proxy missing.** It is named by `dev/bloomadd`, and the material
  system reported it missing on every map. CS:GO's proxy is now ported into
  `viewpostprocess.cpp`.
- **Light bridges crashed the client.** The paint agent fixed this in
  `c_projectedwallentity.cpp`; the bridge view found it.

Still different, ranked by visible impact:

1. **Toxic goo.** Water with Portal 2's `$flowmap` is not implemented by the
   native pipeline, and its draws are dropped.
   - Above a pit the region shows a white slab (catcher view: mean 177 against
     retail's 33).
   - This SDK's water shader has no flowmap either.
2. **No projected-texture shadow depth.** It shows as the void above the
   `sp_a2_bridge_intro` chamber: white on retail, black here.
   - It is not the sky. Retail stays white with `r_drawskybox 0`, and the
     map's only two `SURF_SKY` faces are out of view.
   - Retail turns it black with `r_flashlightdepthtexture 0; r_shadows 0`.
     The shadow depth views (`CShadowDepthView`) set a white clear colour,
     which the main view's colour clear then uses.
   - Native Vulkan has no flashlight depth textures
     (`r_flashlightdepthtexture 0`), so the frame keeps its black clear.
     Projected textures themselves are also unimplemented (flashlight passes).
3. **The laser emitter glow is missing.** Retail draws a bright halo where the
   beam leaves the emitter.
   - It is the particle system `laser_start_glow` (particles/laser_relay_effects.pcf,
     material `particle/particle_glow_05_add_15ob`, `$overbrightfactor 15`).
     The server dispatches it (`CPortalLaser::TurnOnGlow`).
   - No draw of it is in the frame here.
4. **Portal rims.** In `sp_a2_triple_laser`'s portal views, retail draws a
   thick, soft glow around each rim. Here it is a thin, bright outline with
   flame bites, and yellow rather than orange on the orange portal.
   - `PortalStaticOverlay` (`models/portals/portalstaticoverlay_*_noz`, Portal
     2's through-wall portal ghosting, `$ghostoverlay`) is dropped natively.
5. **The tractor beam column is pale.** It has no blue swirl (SolidEnergy
   detail layers), and its base particles are green where retail's are blue.
6. **Exteriors and rusty models.** Lit exteriors behind broken walls are blown
   out, and rusty truss models are grey rather than brown
   (`laser_over_goo.cable.rust_beams` contrast 3.1x retail).
7. **Other native gaps.** `dev/motion_blur` (MotionBlur) is dropped in every
   frame. Projected textures (flashlight passes) are unimplemented. The signed
   texture `normalizesigned` (UVWQ8888) is sampled while empty.
8. **The tractor-beam floor is about 10% brighter** than retail.

The captures used for these numbers are under `quality-results/p2mat-*` and are
not committed.

### Portal 2 audio retail conformance (2026-09-25)

`quality/workloads/portal2-audio-v1` plays a fixed sequence of soundscript
entries on `sp_a2_triple_laser`: a console `play` for alignment, a v1 entry on
the player, a GLaDOS line (precached and not), music, a music entry whose start
stack starts a second entry, the portal gun, a line cut by the next, music under
a line, and a v1 and a v2 world sound at 96, 256 and 512 units. Each event
prints `AUDIO_MARK <label> <server time>`. The listener stands inside the
chamber's `laser_chamber_med_02` soundscape (`"dsp" "1"`, automatic room DSP);
outside every soundscape radius, retail's room-DSP mix parameters depend on a
start-up race (next paragraph).

`tools/quality/portal2_audio.py` records the mixer output through SDL's disk
audio driver, on this build (SDL offscreen video) and on the 32-bit retail
binary (headless `mutter` with Xwayland; the retail binary crashes on SDL's
Wayland backend). Both run the driver from a `mapspawn.nut` hook, because
retail ignores `+wait` on the command line, and both run with
`+snd_surround_speakers 2`: retail otherwise opens a 5.1 device. Each event is
found by normalized cross-correlation with the retail wave, decoded with
ffmpeg from the installed VPKs. The metrics are per-channel least-squares
gains, window levels, left/right balance and correlation, falloff ratios,
ducking and the cut of a replaced line. `retail-reference.json` holds the
metrics of one retail capture (numbers only). A second retail capture passes
all 24 checks; retail is sample-repeatable (most events correlate at 0.99-1.00
between runs).

Reproduce (retail needs Steam running and 32-bit `libbz2.so.1.0` and
`libpng12.so.0` in `--retail-libs`):

    python3 tools/quality/portal2_audio.py capture --side retail --out <dir> --retail-libs <dir>
    python3 tools/quality/portal2_audio.py record --capture <dir>
    python3 tools/quality/portal2_audio.py check --build build-p2 --out <dir>
    python3 tools/quality/portal2_audio.py selftest

`selftest` renders a capture from the reference metrics (each retail wave at
its retail onset and gains, plus the ducking and noise-like windows). The
render passes all 24 checks, and six seeded defects each fail their target
check: a muted right channel, dropped music, a swapped line, a missing chained
entry, no ducking and +3 dB dialog. `--console` passes diagnostic console
commands to the driver (for example `developer 1` and `snd_showstart 2`, which
print each sound's mix group, `dspmix` and speaker volumes on both builds).

Findings, most important first:

1. **No sound operator system.** About 3,500 retail entries are
   `soundentry_version 2` with `operator_stacks` (retail `engine.so` has the
   CS:GO-era `CSosOperator*` classes). This build plays their `wave` through the
   legacy path. Measured: the start stack's `sys_start_entry` never runs, so
   `music.sp_intro_01.02_restasis` does not start `music.sp_a1_intro1_b2b`
   (chain window -6 dB against retail's +12 dB relative to the calibration);
   dialog is 5.7 dB and music 3.5 dB louder than retail (retail's
   `update_dialog` and `update_music_stereo` stacks set volume, speakers and a
   zero DSP send); the v2 world sound falls off differently. v2 entries that
   were never precached are silent (`SV_StartSound: ... not precached`), while
   retail sends them as script handles and plays them.
2. **Portal 2's `soundmixers.txt` was misread** (fixed). The Half-Life 2 mixer
   parser took the `SoundMixers` section name for a mixer, looked up
   `Default_Mix` as a mix group (a write at index -1) and read every other
   number as a group name, so no group volume, level, DSP send, mix layer or
   layer trigger applied. The mixer now lives in `engine/audio/snd_mixgroups.cpp`
   (the CS:GO-era mixer Portal 2 ships with, adapted from a previous session's
   worktree port). It reads both layouts: Portal 2's
   `MixGroups`/`SoundMixers`/`MixLayers`/`LayerTriggers` and Half-Life 2's
   `GROUPRULES` with one volume per group, so Portal and Half-Life 2 keep
   their mix. `SND_Spatialize` applies the group volume, soundlevel scale and
   DSP send scale, as CS:GO does. `snd_list /` with `developer 1` assigns the
   same group and volume as retail to every sound in the probe. Before, this
   build failed 12 of the 24 checks; after, 9. Fixed: the v1 item level,
   the portal gun level, dialog ducking of music (-5.5 dB, retail -5.5 dB) and
   the near v1 world level. The new failure, the v2 world sound's level, is an
   operator-stack entry that the mixer now also attenuates.
3. **`dsp_room` defaults to 0; retail Portal 2's is 1** (automatic room DSP;
   CS:GO's `snd_dsp.cpp` has `#ifdef PORTAL2`). Retail builds an automatic
   preset at start-up (`dsp_automatic` 60), and whichever of the room and
   automatic presets is applied last sets the global DSP mix parameters. In
   normal retail runs that is the automatic one (mix 0.22-0.6, sounds under
   95 dB send 10%: near sounds are almost dry). With `developer 1` from
   start-up it is room preset 104 (0.63-0.8), as on this build. With the
   listener outside every soundscape, this build sends 0.32-0.72 of every
   sound to the room DSP, which adds an echo at 132 ms and -5 dB (seen with
   `dsp_off 1`, which removes it). Not fixed: the engine is built without
   `PORTAL2`.
4. **Soundscape ambience** is 4.6 dB louder than retail at the same place.
   Not yet diagnosed.
5. `dsp_off 1` once segfaulted about 5 s into a run; not reproduced under gdb.
6. The manifest warns about the `new_sound_scripts_must_go_below_here`
   marker (cosmetic; retail uses it for the PS3 hash table).

Open: the operator system and script-handle transmission (finding 1); the
room-DSP default (finding 3); five checks removed because two retail runs
disagreed at this listener position (stopsound silence, the far reverb tail,
v2 near fidelity and v2 falloff at 256 and 512 units); the suite is not yet
registered in the conformance manifest.

### Portal 2 paint retail conformance (2026-09-25)

`quality/workloads/portal2-paint-world-v1` runs one scenario,
`sp_a3_jump_intro_powers`, on this build and on retail `portal2_linux` at
1024x768. It uses the chamber's floor trench (x -448..256, y 448..768,
floor z -184), which the map paints blue. The scenario moves the map's
`info_paint_sprayer`s and sweeps them with `ChangePaintType`: an orange
strip, water over part of the blue, and white gel up the x=256 end wall.

It measures on the server, each tick:
- the rebound of a 160-unit drop on blue;
- the top walking speed on blue, then on orange;
- the rebound on the erased patch;
- portal placement on the bare wall, then on the white patch.

It also takes a shot of each coat. `tools/quality/portal2_paint.py`
compares both against `reference.json`, which holds numbers from a retail
run and no retail pixels. The retail side reuses
`portal2_material_shots.py capture`. Rules and tolerances are in
`checks.json`.

The suite had 14 checks, and 11 failed before this work. All 14 now pass on
`build-p2-paint`. Retail and this build now agree:
- rebound 253.3 on both;
- speed on orange 800, speed on blue 175;
- rebound on the erased patch 0;
- a portal on white gel (0.03 units away), none on the bare wall;
- painted-area fractions within 0.15 of retail.

Defects found, most severe first:

1. **No paint map in the engine.** The CS:GO-era `IVEngineClient` and
   `IVEngineServer` paint members were stubs, so gel never coated a
   surface. The fix is `engine/paint.cpp`, ported from the CS:GO engine
   that retail ships (its ConVars and `svc_PaintmapData` are in the retail
   `engine.so`). It is exposed as the new interface `VEnginePaint001`
   (`public/engine/ienginepaint.h`), and the game connects to it in
   `portal2_shared_compat.cpp`. Pages follow the lightmap pages, are
   allocated after the sort infos and dropped with them. Indices and save
   data are bounds checked.
2. **Player powers switched off.** `PaintPowerUser::UpdatePaintPowers` was
   `if( false )`, so no power ever activated. The server's
   `projectedwallentity_shared.cpp` and the cube's `DisabledThink` also
   hard-coded no paint, and `CBaseEntity::InputRemovePaint` was empty.
3. **Painted surfaces were not drawn.** The engine now draws each painted
   opaque world surface again, with its page's `LightmappedPaint` material
   (`engine/paint_render.cpp`). That shader is a stdshader
   (`materialsystem/stdshaders/lightmappedpaint_*`), and native Vulkan runs
   `shaders/lightmappedpaint.frag`, a port of retail's thick paint (splats,
   bubbles, reflection). Retail binds the paint map at s9; here it is s0, so
   the pass reuses LightmappedGeneric's layout.
   Found on the way:
   - The shader's texture defaults were missing.
   - An early `discard` broke texture derivatives.
   - `BuildMSurfaceVertexArrays` leaves the tangent frame and bump offset
     unwritten on most surfaces, so the paint pass writes its own vertices.
4. **Save/restore and co-op join had no paint.** Saves now hold the engine's
   run-length encoded paint records, loaded once the client has laid out
   the map's lightmap pages. A joining co-op client gets them through the
   now-registered `LoadPaintmapData` message; before, the reconstructed
   code sent unregistered messages. Neither path is tested yet.
5. **Light bridges crashed on draw.** The bridge's `$color` vars pointed into
   unreferenced materials, which get uncached after the first frame.
   `C_ProjectedWallEntity` now holds references to them.
6. **HDR cubemaps were empty on native** (fixed by the video agent). Gel had
   no reflection until RGBA16F cubemaps uploaded.

Negative controls:
- `r_hidepaintedsurfaces 1` fails exactly the 7 coverage and colour checks.
- `sv_paint_detection_sphere_radius 0` fails exactly the 4 power checks.
- The comparator self-test seeds 11 defects, each of which must fail the
  checks it pins (23 checks).
- Rules whose prerequisite failed report "unverifiable": water cannot be
  shown to erase gel that was never there.

Manifest rows: `corpus.portal2.paint-retail` (optional; needs
`SOURCE_PORTAL2_STEAM_ROOT`, a GPU and `build-p2`), its two seeded rows, and
`corpus.portal2.paint-retail.comparator`.

```sh
python3 tools/quality/portal2_paint.py capture --side retail --out <dir>
python3 tools/quality/portal2_paint.py record --retail <dir>
python3 tools/quality/portal2_paint.py suite --build build-p2-paint --runtime run/runtime-p2-paint --out <dir>
python3 tools/quality/portal2_paint.py self-test
```

Other differences found:
- Retail's single-player server simulates in 1/30 s steps
  (`sv_alternateticks 1`). This engine ticks every 0.015 s. Handed to the
  physics work.
- An unpainted wall is about 12% brighter here in linear light than retail,
  which is global exposure. Painted floor is 1.3–1.5x brighter. That is
  within the colour tolerance (40 per channel) but not diagnosed. The
  suspect is the lightmap's sRGB decode in the paint pass: it follows the HDR
  mode, as retail's does.
- `sp_a3_speed_ramp`: retail's gel streams show larger merged blobs than
  this build's clean-room blobulator. Not measured yet.

Open:
- Paint on brush entities (moving panels) is not drawn; world surfaces
  only.
- D3D9: retail's `.vcs` has one more static combo than this port, so its
  indices do not match.
- Save/restore, co-op join, and paint on cubes and props are not measured.
- The blobulator is not compared statistically with retail.
- The in-game manifest rows need `build-p2` rebuilt with this code.
- Stylelint flags `portal2_engine_compat.h:414`: legacy lines next to a
  deletion, in a namespace the formatter would reindent with spaces. They
  are left as they are.

The repository's provenance and distribution warning in the root README also
applies to this import.
