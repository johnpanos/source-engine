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
| G01a | `point_changelevel` transitions have no level connection, so the engine disconnects the arriving player ("Can't find connection") | every SP transition | done (see log) |
| G01b | 64-bit user-message sizes: `sizeof( long )` registrations (8 bytes) against 4-byte `WRITE_LONG`/`WRITE_EHANDLE`, so the engine refuses `PaintEntity`, `ChangePaintColor`, `StartSurvey`, `ScoreboardTempUpdate` (and Portal 1 `EntityPortalled`) | paint on props, surveys, portal teleport fix-up | done; runtime check pending |
| G02 | Engine paint-map API is stubbed (`HasPaintmap`, `SpherePaintSurface`, `SphereTracePaintSurface`, paint-map save/restore), so gel cannot coat world surfaces | 30 maps set `paintinmap` | todo |
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
| G16 | `info_paint_sprayer` radius keys (`start_radius_*`, `end_radius_*`, `radius_grow_time_*`) | todo |
| G17 | `env_portal_laser` `AutoAimEnabled`, `NoPlacementHelper` | todo |
| G18 | `npc_personality_core` `ModelSkin`, `AltModel`, `EnableReceivingFlashlight`/`DisableReceivingFlashlight` | todo |
| G19 | `info_placement_helper` `target_size`, `usesizelimit` | todo |
| G20 | `prop_vehicle_choreo_generic` view limits (`SetMin/MaxPitch/Yaw`), `SetCanShoot`, `UseAttachmentEyes`, `PlayerCanShoot`; `logic_playerproxy` `LowerWeapon`, `PaintPlayerWithPortalPaint` | todo |
| G21 | `prop_tractor_beam` `NoEmitterParticles`; `vgui_screen` `IsTransparent`; `vgui_neurotoxin_countdown` `countdown`; `npc_bullseye` `AlwaysTransmit`; `point_viewcontrol` `TrackSpeed` | todo |
| G22 | Retail-networked base classes (`func_brush`, `func_movelinear`, `func_button`, `prop_door_rotating`, `func_portal_bumper`) and co-op stats (`portal_mp_stats`) | todo |

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
