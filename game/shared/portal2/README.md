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
  with its own `portal2/cfg` and `scripts`. A `mapspawn.nut` hook starts the
  same scripts on the server VM. It runs under `mutter --headless` with SDL on
  Xwayland.
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
