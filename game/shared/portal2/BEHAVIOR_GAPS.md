# Portal 2 behavior gap goal

Goal: every entity class, input, output and keyvalue that the shipped Portal 2
maps and scripts use has a working implementation in the Portal 2 target
(`build-p2`). Then single-player and co-op maps can be played through without
missing entity behavior.

Source presence is already complete: `scripts/waifulib/portal2_source_inventory.py
--selected-build build-p2` reports 0 missing of 1361 declared paths. This record
tracks behavioral gaps. `MISSING.md` keeps the source-file provenance.

## Method (2026-09-24 audit)

- Map scan: the entity lumps of all 117 retail BSPs (`portal2`, `update`,
  `portal2_dlc1` and `portal2_dlc2`, both loose files and `pak01` VPKs). The scan
  collects classnames, I/O inputs and outputs (fields are ESC-separated) and
  keyvalues. It diffs them against the `LINK_ENTITY_TO_CLASS` and
  `DEFINE_INPUT*`/`DEFINE_KEYFIELD`/`DEFINE_OUTPUT` names in the sources that
  build-p2 compiles into `libserver.so`. A retail name is confirmed by a
  lowercase `name\0` search in the retail `linux32/server.so`.
- Retail factory and network diff: the `CEntityFactory<T>` RTTI names and the
  `DT_*` table names in the retail `server.so` and `client.so`, against ours.
  This finds entities that code or scripts create, which no map lists.
- Noise excluded: `instance:*;Input` targets left by instance compilation,
  commented-out (`//`) outputs, vrad-only light keys, `info_overlay_accessor`
  keys, and names that retail does not define either
  (`SetTonemapPercentBrightPixels`, `ProxyRelay`).

Behavior comes from the retail Linux `server.so`/`client.so` (Ghidra 12.0.4
headless, image base 0x10000; datadesc entries have a 0x40-byte stride). Base-game
behavior comes from the local `cstrike15_src` drop where it exists.

## Checklist

States: `todo`, `active`, `done` (built, with the evidence noted), `deferred`
(reason given).

### Critical

| ID | Gap | Maps and callers | State |
| --- | --- | --- | --- |
| G01 | `portal_stats_controller` (`CPortalStatsController`) is absent, so `OnLevelEnd` from the transition scripts never changes level | `sp_transition_list.nut`, `mp_coop_transition_list.nut` | done (see log) |
| G01a | `point_changelevel` transitions have no level connection, so the engine disconnects the arriving player ("Can't find connection"); the first fix paired the wrong landmarks, so the player arrived outside the map | every SP transition | done (see log; scenario `portal2-transition-v1`) |
| G01b | 64-bit user-message sizes: `sizeof( long )` registrations (8 bytes) against 4-byte `WRITE_LONG`/`WRITE_EHANDLE`, so the engine refuses `PaintEntity`, `ChangePaintColor`, `StartSurvey`, `ScoreboardTempUpdate` (and Portal 1 `EntityPortalled`) | paint on props, surveys, portal teleport fix-up | done; runtime check pending |
| G02 | Engine paint-map API is stubbed (`HasPaintmap`, `SpherePaintSurface`, `SphereTracePaintSurface`, paint-map save/restore), so gel cannot coat world surfaces | 30 maps set `paintinmap` | done on native Vulkan (see log); brush entities, save/restore and co-op join unverified |
| G03 | `func_portal_detector` is the Portal 1 version: no `OnStartTouchPortal`/`OnEndTouchPortal`/`OnEndTouchLinkedPortal`/`OnEndTouchBothLinkedPortals`, no `CheckAllIDs` | 10+ maps | done (see log) |
| G04 | `npc_portal_turret_floor` is the Portal 1 version: no `TurretRange`, `OnExplode`, `ShootAtMovingObjects`, `AllowShootThroughPortals`, `LoadAlternativeModels`, `CollisionType`, `UseSuperDamageScale` | 27 maps | partial: built; model variants verified, combat unverified |
| G05 | `npc_security_camera` is the Portal 1 version: no `LookAtBlue`/`LookAtOrange`, `TeamPlayerToLookAt`, `OnTaunted*` | co-op paint maps | done: built; co-op run unverified |
| G06 | `npc_wheatley_boss` is a bare stub; retail has a networked client/server pair (`DT_NPC_Wheatley_Boss`) | `sp_a4_finale4` | done (see log) |

### High: base-game features (port from cstrike15_src)

| ID | Gap | Maps | State |
| --- | --- | --- | --- |
| G07 | `CBaseEntity`: `mincpulevel`/`maxcpulevel`/`mingpulevel`/`maxgpulevel`, `drawinfastreflection`, `disableshadowdepth`, `shadowdepthnocache`; `DisableDraw`/`EnableDraw` inputs | 115 / 18 | todo |
| G08 | `env_projectedtexture`: `SetLightStyle`, `colortransitiontime`, `brightnessscale`, `simpleprojection` | 31–109 | todo |
| G09 | `env_fade`: `FadeReverse`, `ReverseFadeDuration` | 14 | todo |
| G10 | `math_counter`: `OnChangedFromMax`/`OnChangedFromMin` | 6 | todo |
| G11 | `trigger_teleport`: `UseLandmarkAngles` | 77 | todo |
| G12 | `prop_dynamic`: `FadeAndKill`, `SuppressAnimSounds`, `AnimateEveryFrame` | 2–43 | todo |
| G13 | `info_particle_system`: `DestroyImmediately`, `StopPlayEndCap` | 6 / 2 | todo |
| G14 | `func_rotating`: `GetSpeed`/`OnGetSpeed`; `point_teleport`: `TeleportEntity`; `TeleportToCurrentPos` | 1–2 | todo |

### Medium: Portal 2 entity keys and inputs

| ID | Gap | State |
| --- | --- | --- |
| G15 | Cube/paint bomb `PaintPower`, `AllowFunnel`; paint bomb `BombType`, `PlaySpawnSound` | todo |
| G16 | `info_paint_sprayer` radius keys (`start_radius_*`, `end_radius_*`, `radius_grow_time_*`) | not a gap: retail `server.so` has no such keys (2026-09-28); blob size is `paintblob_min/max_radius_scale` |
| G17 | `env_portal_laser` `AutoAimEnabled`, `NoPlacementHelper` | todo |
| G18 | `npc_personality_core` `ModelSkin`, `AltModel`, `EnableReceivingFlashlight`/`DisableReceivingFlashlight` | todo |
| G19 | `info_placement_helper` `target_size`, `usesizelimit` | todo |
| G20 | `prop_vehicle_choreo_generic` view limits (`SetMin/MaxPitch/Yaw`), `SetCanShoot`, `UseAttachmentEyes`, `PlayerCanShoot` (done 2026-09-29, see log); `logic_playerproxy` `LowerWeapon` | todo (`PaintPlayerWithPortalPaint` done 2026-09-28, see log) |
| G21 | `prop_tractor_beam` `NoEmitterParticles`; `vgui_screen` `IsTransparent`; `vgui_neurotoxin_countdown` `countdown` (done, see log); `npc_bullseye` `AlwaysTransmit`; `point_viewcontrol` `TrackSpeed` | todo |
| G22 | Retail-networked base classes (`func_brush`, `func_movelinear`, `func_button`, `prop_door_rotating`, `func_portal_bumper` (done, see log)) and co-op stats (`portal_mp_stats`) | todo |

### Portal rendering

| ID | Gap | State |
| --- | --- | --- |
| G23 | Portal 2's stencil fast path (bitmask stencil, batched portal quads, early-Z, scissor) ran only in queued material mode, which native Vulkan never uses, so the Portal 1-style `_Old` path always ran; early-Z had no caller | Portal 2 done (see log); Portal 1 backport todo |
| G24 | Gel streams: the server never created the paint blob pool (crash on a sprayer's first blob), the blob materials were bound unreferenced, the `paintblob` shader was absent and Valve's blobulator library is unavailable | done on native Vulkan (see log); retail stream shape and the erase gel's opacity compared 2026-09-28; flashlight unverified |
| G25 | Portal ghosts: `portalstaticoverlay` was the Portal 1 shader, without Portal 2's `$ghostoverlay`, so the through-wall ring and brackets were drawn over every visible portal | done on native Vulkan (see log); DXVK source-matched `.vcs` packs need the new combos |
| G26 | Monitors: the client compiled `CViewRender::DrawMonitors` out (`USE_MONITORS` was defined only for `HL2_CLIENT_DLL` and `CSTRIKE_DLL`; Portal 1 defines the first, Portal 2 neither), so nothing drew `_rt_Camera` and every `func_monitor` screen (`dev/dev_tvmonitor1a`), including Act 4's Wheatley monitors, stayed black | done on native Vulkan (see log) |
| G27 | Ending credits (`env_portal_credits`, `sp_a4_finale4` and `sp_a5_credits`) draw nothing: the client is Portal 1's `CHudPortalCredits`. Its lyric and border colour comes from Portal 1's `color` parameter, which Portal 2's `credits.txt` replaces with `color_lyrics` and `color_credits`, so they draw black on black. Retail rewrote the reader and the drawing (fonts resolved when read, margins from the HUD aspect, subtitles moved onto the panel) | partial: scheme and song (see log); drawing todo |

### Low

- Deferred stubs: `portal_ui_controller`, `portal2_research_data_tracker`,
  `challenge_mode_end_node`, `prop_hot_potato`, `portal_procedural_generator`,
  `weapon_promo_items` and the co-op stats.
- Engine features this engine lacks: sound operator system and mix layers,
  loading-plaque and blur calls, demo custom data.
- Retail classes that no shipped map uses: `env_ambient_light`,
  `trigger_tonemap`, `skybox_swapper`, `prop_hallucination`,
  `sunlight_shadow_control` and others.

## Evidence log

Newest last. Each entry names the build and the check that passed.

- 2026-09-24, G01/G01a (worktree `p2-audit`, build-p2 release, Box3D): reconstructed
  `game/server/portal2/portal_stats_controller.cpp` from the retail
  `CPortalStatsController` (datadesc at 0x11fe2a0, Think, both inputs, and the
  game system at vtable 0xdf9268 that creates and removes it). Added retail's
  point_changelevel connection to `CChangeLevel::BuildChangeLevelList` (retail
  function 0xae6460). Headless isolated run: `+map sp_a1_intro3`, then the
  `@transition_script` `TransitionFromMap()` script. Console shows
  `Host_Changelevel` to `sp_a1_intro4` with no "Level transition ERROR", and the
  client stays connected until shutdown. Before the fix the same run logged
  "Can't find connection to sp_a1_intro3 from sp_a1_intro4" and dropped the
  player. Also: `ResetThisLevelStats` (no caller before) now runs at each level
  start.
- 2026-09-24, G01b: user message registrations use `sizeof( int32 )`; the
  engine's `DLL_MessageEnd` refuses a fixed-size message whose byte count
  differs. The Portal 1 file change needs the Portal 1 build check.
- 2026-09-24, G03: the Portal 2 build now compiles
  `game/server/portal2/portal/func_portal_detector.{h,cpp}` (from retail
  `CFuncPortalDetector` vtable slots 197–205, its datadesc, and the 2010 dSYM's
  `GetPortalDetectorList` list) instead of the Portal 1 file. `CProp_Portal`
  walks the detector list as the CS:GO drop's Portal 2 `prop_portal.cpp`
  does, and `func_portalled` keeps only retail's placed-portal override.
  Headless run on `sp_a2_bridge_the_gap`: a portal moved into
  `bridge_seeded_portal_detector` fired `OnStartTouchPortal` (and an unnamed
  detector's `wall_repair_relay`), and `Fizzle` fired `OnEndTouchPortal`.
- 2026-09-24, G04 (partial): the Portal 2 turret now has retail's
  `TurretRange` (default 1024), `CollisionType`, `AllowShootThroughPortals`
  (a new `SENSING_FLAGS_DONT_LOOK_THROUGH_PORTALS`, cleared by a physgun
  pickup), `UseSuperDamageScale`, `LoadAlternativeModels`/`ModelIndex`/
  `SetModel` (normal, boxed, backwards and skeleton), `DisableMotion`, the
  `PortalTurretBullet` ammo, and retail's damage scale (6.0) and bullet force
  (0.85). It fires one bullet per shot from each of the four barrels in turn;
  the backwards model fires rearward. `OnExplode` is on the `CNPC_FloorTurret`
  base, as in retail. All of this is under `#ifdef PORTAL2` in shared files.
  Headless `sp_a2_core`: the five chamber turrets spawn with the skeleton and
  boxed models the map requests. Range, portal vision and damage are not yet
  checked in play.
- 2026-09-24, G05: `npc_security_camera` (Portal 2 build) has retail's
  `TeamPlayerToLookAt`/`TeamToLookAt`/`LookAtPlayerPings` keys and the
  `LookAtBlue`/`LookAtOrange`/`LookAllTeams` inputs. As in retail, each input
  redeploys the camera and sets the watched team. `SearchThink` skips players
  of other teams. `TauntedByPlayer`/`Finished` fire `OnTaunted*` by team.
  Retail's `portal_player_ping` listener (`LookAtPlayerPings`) is not ported;
  no shipped map sets it. Needs a co-op runtime check.
- 2026-09-24, G06: `npc_wheatley_boss` was a bare 21-line class with no
  `Spawn`, so the boss never set its model. It is now rebuilt from the retail
  server `CNPC_Wheatley_Boss` (Precache/Spawn: 8000 health, human hull, no
  solid or movement, face and head capabilities, `NPCInit`, and bone followers
  from the model's keyvalues) with an empty `DT_NPC_Wheatley_Boss` table. It
  has a client `C_NPC_Wheatley_Boss`. Headless `sp_a4_finale4`: `@sphere`
  spawns with `glados_wheatley_boss.mdl`, 8000 health and the hull bounds. The
  model has no `bone_followers` keys, so none are made, as in retail.
- 2026-09-24, Box3D crash found by the G07/G08 multi-map scan: `sp_a2_bts3`
  crashed at load (`CStaticCollisionPolyhedronCache::Update` →
  `CPhysicsCollisionBox3D::PolyhedronFromConvex` →
  `GeneratePolyhedronFromPlanes`). IVP loads the map. Root cause (debug build
  under gdb): Box3D rebuilt each convex from the planes of its hull pieces. A
  382-point, 760-triangle convex (`models/props/sphere.phy`) produced 400
  distinct planes, and mathlib's `ClipLinkedGeometry` stack-allocates every
  point, line and polygon it creates and frees them only on return. That used
  about 8.4 MB of stack at plane 388 and overflowed the 8 MB limit. The
  optimized build sits near the limit, which is why it did not crash under
  gdb or valgrind. Fix: a new
  `ConvertTriangleMeshToPolyhedron` in mathlib (heap working sets, outward
  orientation from the centroid) builds the polyhedron from the convex's own
  triangles, as IVP does. Regression: new physics-conformance check
  `corpus.convex-polyhedron` (every convex of every corpus model). Before the
  fix Box3D crashed on 117 of 2123 Portal 1/HL2 models. After it, both
  providers pass on that corpus and on all 1076 Portal 2 models, and
  `sp_a2_bts3` loads on Box3D. The full gate's IVP reference passes. Box3D's
  three other gameplay failures (Chell `model-simulates` non-finite,
  `tumble-travel-bounded`, `held-floor-quiet`) are pre-existing: they fail the
  same way with the main tree's build.
- 2026-09-24, G23 (Portal 2): the fast path's queued-mode gate is lifted.
  It only protected the reuse of the level-0 quad vertex buffer, which this
  port replays through `CPortalQuadMeshReplayData`. `DrawEarlyZPortals` is
  now called from `CBaseWorldView::DrawExecute` for main and portal views, as
  in CS:GO. Oracle: headless `sp_a2_laser_relays` with an open portal pair in
  front of and behind the player (recursion visible). gdb `dprintf` shows the
  `_Old` path while `r_portal_fastpath 0` and the fast path plus early-Z
  otherwise. Screenshots old, fast and fast+early-Z differ only inside the
  animated portal rim (bounding box of >8 differences 609–672 × 288–397 at
  1280×683).
- 2026-09-25, G24 (build-p2 release, native Vulkan): gel streams render.
  - The blobulator is a clean-room library (no public reimplementation of
    Valve's was found): `public/blobulator` and
    `game/client/portal2/blobulator/ImpTiler.cpp`, a marching-tetrahedra
    isosurface of the blob metaballs on sparse tiles. The
    `blobulator.tiler-mesh` conformance suite (508 checks) covers surface
    distance, closure across tile seams, orientation, bridging, determinism
    and the tiler pool. Its seeded-defect row fails as required. The suite
    found two tiler bugs, both fixed: vertices off the surface (fixed-step
    refinement) and non-unit tangents.
  - The `paintblob` shader is ported from `cstrike15_src`
    (`materialsystem/stdshaders/paintblob_*`). The generated combo indices
    match the retail `.vcs` headers (ps20b 10240/20, vs20 24/12). Native
    Vulkan draws it with `shaders/paintblob.frag` after `skin.vert`, with
    combos in c27. Not ported: `FRESNEL_WARP`, `OPACITY_TEXTURE`,
    `CONTACT_SHADOW` and the flashlight (reported unimplemented; no retail
    gel material uses them).
  - The server creates the blob pool from `CPaintStream::Init`. The pool
    grows, because streams share it and each is capped by its own
    `maxblobcount`. `C_PaintStream` holds references to the blob materials.
  - Oracle: the `portal2-paint-v1` scenario (`sp_a3_speed_ramp`, both
    sprayers started, blobs frozen) passed with `mat_queue_mode` 0 and 2.
    `tools/quality/portal2_paint_shots.py` then passed on its shots: blue
    and orange gel present, and the isosurface differs from the sphere
    fallback. The checker has 10 fixture tests. It fails the earlier
    untextured run (both streams pale).
  - `r_paintblob_wireframe` draws nothing on native Vulkan (no wireframe
    shader there).
- 2026-09-25, G02 (build-p2-paint, native Vulkan): world paint.
  - `engine/paint.cpp` is the paint map, `VEnginePaint001` its interface
    for the game, and `engine/paint_render.cpp` with the `LightmappedPaint`
    shader (`shaders/lightmappedpaint.frag`) draws it.
  - Player powers were also gated off: `PaintPowerUser::UpdatePaintPowers`
    was `if( false )`.
  - Oracle: `tools/quality/portal2_paint.py suite`
    (`quality/workloads/portal2-paint-world-v1`, sp_a3_jump_intro against
    retail portal2_linux) passes 14/14. It failed 11 of 14 before, and its
    in-game negative controls fail as required. See the README's "Portal 2
    paint retail conformance".
- 2026-09-28, G01a correction (build-p2 release, Box3D, native Vulkan):
  the connection added on 2026-09-24 paired the wrong landmarks. Each map
  has an `info_landmark_exit` at `@exit_teleport`, where the player leaves,
  and an `info_landmark_entry` in the trigger that runs
  `OnPostTransition()`, where the next map's player arrives. The origin map
  now connects through its exit landmark and the destination through its
  entry landmark. Before, the player arrived 256 units outside the transition
  room of `sp_a1_intro4` and fell forever, so `OnPostTransition` never
  teleported them into the arrival elevator.
  - Also ported from `cstrike15_src`: the engine's `map_wants_save_disable`
    (the elevator's `StartMoving()` sets it; a player save is refused while
    it is set, autosaves are not). It is cleared on disconnect and when a
    transition loads the next level.
  - Oracle: the `portal2-transition-v1` scenario `sp_a1_intro3_to_intro4`
    (`tools/quality/portal2_scenarios.py`, which now lets a scenario continue
    on an `arrival` map) leaves sp_a1_intro3 through its own departure
    elevator and arrives through the level change. It passes 16/16, 3 of 3
    runs. The same run on the pre-fix `libserver.so` fails 5 checks: the
    player falls at (-2352, -3052), there is no `OnPostTransition`, no
    elevator ride or walk out, and the arrival save is refused.
- 2026-09-28, G24/G25/G20 (build-p2 release, native Vulkan): white gel and
  water against retail `portal2_linux`, 1024x768.
  - Gel streams fuse into one body again. The clean-room tiler's field kernel
    was `(1-x)^12`; retail's is `(1-x)^2 / 4`, read from the 2010 client's
    `CBucketBlobRenderer::RecalculateConstants` (`u = r^2 / (sqrt(2) R)^2`,
    `k = u^2 - u + 0.25`). The white stream in `sp_a3_portal_intro` now
    matches retail's shape; before, it broke into separate spheres.
  - The erase gel (water, `blob_surface_erase`) uses `OPACITY_TEXTURE` and
    `FRESNEL_WARP`, which the 2026-09-25 entry wrongly said no gel material
    uses. `paintblob.frag` now reads s6 (in the light warp's set; no blob
    material has both) and, as paintblob_ps20b does, `FRESNEL_WARP` only
    drops the translucent fresnel. Retail's water refracts the room; ours is
    still brighter (seen on `sp_a3_crazy_box`'s `paint_sprayer_erase_01`).
  - Portals on white gel: on `sp_a3_portal_intro` a shot at the bare floor
    places nothing and, once `paint_sprayer_2` has coated it, the same shot
    places a portal at (112, 192, -16), identically on retail and this build.
    `portal2_paint.py suite` still passes 14/14.
  - G25: `portalstaticoverlay` has Portal 2's `$ghostoverlay` (from
    `cstrike15_src`): a reverse z-test one unit off the wall, a one /
    inverse-source-alpha blend and the 120-240 unit fade, as new static
    combos with the largest strides, so existing combo indices (and shipped
    `.vcs` files) are unchanged. `fxc_prep.pl` regenerated the selectors (it
    reproduces the old ones byte for byte); the native ports follow. The
    visible portal lost its brackets, and the ghost through a wall matches
    retail (peak 53/106/150 against 58/114/149). The four existing
    `portalstaticoverlay` legacy-shader cases pass in both HDR modes.
  - G20: `logic_playerproxy` `PaintPlayerWithPortalPaint` calls
    `CPortal_Player::Paint( PORTAL_POWER, vec3_origin )`, as the retail
    handler does (found through its datadesc entry). The painted-player
    screen effect it starts is still never drawn: nothing calls
    `C_Portal_Player::RenderLocalScreenSpaceEffect`, and this tree has no
    `engine_post` shader to composite it.
- 2026-09-29, G26 (build-p2, native Vulkan, Box3D; the user's report of black
  Wheatley monitors): `game/client/viewrender.cpp` and `view.cpp` define
  `USE_MONITORS` for `PORTAL2` too. The server half was already built:
  `func_monitor`, `info_camera_link`, `point_camera` and the player's
  `PointCameraSetupVisibility`.
  - Oracle: `tools/quality/portal2_monitors.py suite` (checks-v1,
    `corpus.portal2.monitors`), 14 of 14. It deploys one prefab monitor on
    each of `sp_a4_tb_intro` and `sp_a4_intro`, derives the camera placement
    from the BSP, and shoots the screen at the camera's authored FOV of 30
    and at 60 (`ChangeFOV`).
  - Measured: Wheatley's eye covers 1.58 % and 1.55 % of the screen region,
    and the FOV change leaves it at 0.25 and 0.26 of that area.
  - Controls, each rejected: `cl_drawmonitors 0` from startup (0 %, as
    reported); `cl_drawmonitors 0` after the first shot (a stale
    `_rt_Camera`, ratio 1.00).
  - The pre-fix client fails 4 of the 10 checks run without controls (eye
    0 % on both maps).
  - The selftest (`corpus.portal2.monitors.selftest`) passes 11 of 11.
- 2026-09-29, G21 `vgui_neurotoxin_countdown` (build-p2, native Vulkan,
  Box3D): the Portal 2 build compiled Portal 1's countdown, whose screen
  showed the player's bonus progress, so the finale's neurotoxin, destruction
  and world timers (`sp_a4_finale4`) and the `sp_a2_bts1` door timers read
  00:00. Rebuilt under `#ifdef PORTAL2` from the 2010 server/client dSYMs
  (`CNeurotoxinCountdown::Think`, `C_NeurotoxinCountdown::ClientThink` and
  `GetCountdownTime`, the screen's `Update`) and the retail tables and strings
  (`m_flCountdownTime` on `DT_NeurotoxinCountdown`, and the screen's scheme
  `resource/basemodui_scheme.res`; Portal 2 ships no
  `NeurotoxinCountdownScreen.res`, so the digits had no font). The `countdown`
  key seeds the time, the server runs it down each tick while enabled, and the
  client runs its copy down between updates. Headless `sp_a2_bts1`:
  `ent_dump` reads 4.54, 3.59, then 0.12 after `Enable`. Headless
  `sp_a4_finale4` with `mat_force_tonemap_scale 1`: the monitor shows
  `04:58:09`, then the 120 s world timer. Not compared against retail frames.
- 2026-09-29, G20 `prop_vehicle_choreo_generic` (build-p2, native Vulkan,
  Box3D; the user's report that the `sp_a4_finale4` moon shot "does nothing"
  once control returns). The finale seats the player in `ending_vehicle` and
  `vehicle_shoot_relay` sends `SetCanShoot 1` and `SetMin/MaxPitch/Yaw`; none
  of these inputs existed, so the gun stayed holstered and the view stayed
  locked. Ported from retail `server.so` (datadesc and Ghidra:
  `m_bPlayerCanShoot`/`playercanshoot`, `m_bForceEyesToAttachment`/
  `useattachmenteyes`, the server vehicle's standard-weapons flag, which on a
  change holsters the gun and hides the crosshair or shows it, deploys and
  plays `end_draw`) and from the CS:GO client (view limits that ease toward
  new values, `SharedVehicleViewSmoothing`'s attachment-eyes view and
  Portal 2 FOV). A second bug kept the view frozen after that: an entity
  networked at the world origin with zero angles never computed its
  coordinate frame on the client, so `ending_vehicle`'s (at `0 0 0`)
  `EntityToWorldTransform()` was all zeros, its local eye attachment read
  `(0 0 0)` and `UpdateViewAngles` pinned the view level. `C_BaseEntity`'s
  constructor now starts it at identity, as CS:GO's does. Headless run of
  `stalemate_ending_relay`: the view follows the eye attachment up to the
  moon (pitch -81), the gun draws at 16 s, and a shot at the moon's centre
  places a portal at (384, 32, 1544); `moon_portal_detector` fires and
  `ending_relay` plays the moon ending. Before the fix the inputs were
  "unhandled" and the view read `setang 0 0 0` whatever the input. Not
  compared against retail frames.
- 2026-09-29, G27 (partial; build-p2, native Vulkan): `sp_a5_credits` renders
  black. Under gdb on the release client, the outro reads 328 names, 114
  lyric lines and 300 ASCII-art lines, and draws them with a valid font, but
  in the colour `0 0 0 192`. Portal 2's `credits.txt` has no `color` key,
  only `color_lyrics` (`240 182 0 70`) and `color_credits` (`0 0 0 192`).
  Retail `ReadParams` (client.so 0xbe8d80) reads only `scrolltime`,
  `separation`, `cursorblinktime`, `scrollcreditsstart`, `songstarttime`
  and those two colours. Fixed so far, as retail does: the panel takes
  `basemodui_scheme` (the only scheme with `CreditsOutroText`), shared with
  `GetClientSchemeFont` through a new `GetBaseModUIScheme()`, and the client
  no longer plays Portal's `music/portal_still_alive.mp3` (retail has no
  reference to it; the map plays `credits_music`). Open: retail's drawing,
  which needs its draw functions decompiled. The 2010 dSYM credits code is
  still Portal 1's, so it is no reference for this.
- 2026-09-29, G04 (build-p2, native Vulkan, Box3D): the turret holds fire
  while a window stands between it and its enemy. `IsEnemyBehindGlass`
  traces `CONTENTS_WINDOW` along the shot line (through the portal when it
  aims through one), from the 2010 server dSYM; the retail server's
  `ActiveThink` (vtable slot 0x9fc, 0x9806d0) calls the same function
  (0x97ff40) on both its shot paths and skips the shot, dry fire included.
  Headless `sp_a2_turret_intro`: a turret still shoots the player in the open
  (`AddMultiDamage` hits). The glass case is not checked at runtime.
- 2026-09-29, G22 `func_portal_bumper` (build-p2, native Vulkan): the
  server sends `DT_FuncPortalBumper` (`m_bActive`) as the 2010 server dSYM
  and the retail server do, so the client's predicted portal placement
  (`portal_placement.cpp` under `CLIENT_DLL`) can bump off them; the client
  class was reconstructed earlier but never received one. Headless
  `sp_a2_bridge_the_gap`: `report_entities` counts 30 bumpers on the server
  and `cl_showents` lists 9 `CFuncPortalBumper` on the client (those in the
  PVS). Portal 1 keeps its unnetworked bumper.
