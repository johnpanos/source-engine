# Portal 2 source gap inventory

Generated from the imported Portal 2 VPC references and quoted includes.
The VPC list is a superset of conditional platform/feature selections; it is not a
proof that every row is needed for a Linux build. The include scan uses file-name
presence and cannot prove API or ABI compatibility. Compile/link/runtime checks
will reveal further gaps.

The initial inventory recorded 98 missing VPC references (84 unique paths) out
of 951 declared references and 67 quoted include paths with absent basenames.
The current VPC scan reports 53 missing references out of 973 declarations;
the lists below retain the initial snapshot for provenance.

Regenerate the VPC portion with `python3 scripts/waifulib/portal2_source_inventory.py`.
For the configured build, run
`python3 scripts/waifulib/portal2_source_inventory.py --selected-build build-p2`.

## Local source candidates checked

- The extracted CS:GO 2015 tree has ten matchmaking headers missing from this
  checkout. Its six already imported matchmaking headers are byte-identical,
  so the ten missing headers were imported and formatted for this tree. The
  later case-sensitive include spelling `tier1/keyvalues.h` now forwards to
  the existing `tier1/KeyValues.h` implementation.
- The extracted TF2 `trigger_catapult_shared.cpp` explicitly says its launch
  logic was copied from Portal 2. That logic now has Portal 2 player and
  prediction adapters here. The retained retail server's `DT_TriggerCatapult`
  table supplies the client receive field names and order.
- The installed retail client and server binaries also name
  `DT_BaseProjector`, `m_hFirstChild` and `m_bEnabled`. The server table
  initializer constructs the child handle then the enabled flag. The local
  projector send and receive tables now carry those fields. This is a
  binary-guided reconstruction, not recovered source.
- The retail client names `ClientProjectedEntityAmbientSoundProxy` and
  `ClientFizzlerMultiOriginSoundPlayer`; their server counterparts have empty
  derived network tables. The client classes now register those tables. The
  fizzler player spatializes its loop at the nearest enabled client cleanser
  volume. Native audio/gameplay acceptance remains unverified.
- The retail client `DT_InfoPortalScore` initializer identifies seven numeric
  score/time properties and a 260-byte game-description string. The client
  receiver now uses that table; direct client compilation passes. Its server
  counterpart and versus HUD remain absent.
- The retained source stores a physics surface index through a custom save
  field in `prop_paint_power_user.h`. A `MaterialIndexDataOpsProxy` RTTI name
  appears in the retail server. The reconstructed proxy stores the surface
  name and resolves it on restore. This format and save-game compatibility
  still need a retail comparison. The client file compiles; the server file
  passes probe mode with zero own diagnostics.
- The retail client names `CRadialButton`, `RadialButtonImage` and
  `SubmenuHotspot`. The missing radial button now implements the methods used
  by the retained radial menu on its existing polygon-button base. The file
  compiles directly; visual and input parity need a native client check.
- The retail client `CHUDPuzzleMakerMapOutOfDate` constructs title and
  description labels and loads `hud_puzzlemaker_map_out_of_date.res`. Its
  visibility method checks that Puzzle Maker is active, its editor is hidden,
  and the chamber has uncompiled changes. The reconstructed HUD follows those
  conditions and compiles for the selected Linux client profile. The Puzzle
  Maker API is conditional on `PORTAL2_PUZZLEMAKER` in the retained header and
  the VPC enables it only on Windows and macOS, so the Linux HUD remains hidden.
- The retail client `CHUDPuzzleMakerSaving` constructs a `SavingSpinner` panel,
  loads `hud_puzzlemaker_saving.res`, and gates drawing on
  `IPuzzleMaker::IsSaving()`. That core display behavior is reconstructed and
  compiles for the selected Linux client profile. The retail binary also has
  Puzzle Maker commands and help links in this file; those controls need a
  feature-enabled build and native UI check before parity can be claimed.
- The server `info_portal_score` send table carries the seven fields received
  by the client. Retail input handlers add the supplied integer directly to
  each team's score, and `ResetScore` clears those scores. The reconstructed
  entity now networks those fields and runs round and sudden-death decisions.
  Its timer transitions still need a native versus-map comparison.
- The `portal_ui_controller` and `portal2_research_data_tracker` server hooks
  now satisfy retained callers. They deliberately omit UI transition and CSV
  research reporting while the gameplay build is restored. Both pass server
  probe compilation with zero own diagnostics.
- The retail server RTTI identifies `CBasePortalCombatWeapon` as
  `CWeaponItemBase`'s parent, and both binaries name the four promotional
  helmet/antenna classes and their network tables. Minimal non-firing wearable
  classes now register those entities on both sides. Cosmetic attachment and
  model behavior remain to be checked in a native co-op session. Both shared
  sources compile directly for the client and pass server probe compilation.
- Multiplayer stats now have the retained client/server entry points so portal
  shots, footsteps, deaths and map completion can compile. These calls are
  explicit no-ops: stat accumulation, persistence, network publication and
  online reporting are deferred as nonessential telemetry. Both sources
  compile in their selected profiles (server via probe mode).

The binary checks used the installed Linux32 `client.so` (ELF build ID
`42e0d50363156e0cb529f2d3c61803b824c1c8be`) and `server.so` (build ID
`f33867befd8c0b57a3c6b74acc38bf36b4e20a94`).

- `game/client/portal2/gameui.rar` opens with the password in `gameui.txt`.
  Its 287 files are byte-identical to the already imported `gameui` directory.
- The installed Portal 2 Community Edition tree has no C/C++ sources, headers,
  PDBs, or debug files. Its local installation now has 64-bit Linux client and
  server modules as well as Windows DLLs and content. The Linux modules retain
  symbol tables but refer to separate `.dbg` files that are not installed.
  [The public P2:CE repository](https://github.com/StrataSource/Portal-2-Community-Edition)
  is a task tracker and does not publish its game source.
- The installed Portal 2 Linux client and server modules are 32-bit binaries.
  They retain symbol tables and refer to separate `.dbg` files that are not
  installed. Symbols and disassembly can inform behavior, but cannot supply
  the original C++ source or satisfy the 64-bit source build.
- The local F-Stop source zip has two filename matches among the selected
  missing Portal 2 files: `hud_viewfinder.cpp` and
  `info_placement_helper.cpp`. These implement F-Stop camera and placement
  behavior, with different dependencies and contracts; neither is a direct
  Portal 2 replacement.

SteamCMD `app_info_print 620` and `app_info_print 440000` (2026-09-22) show
no public debug-symbol depot. Portal 2 lists `public`, `beta`,
`previous_release`, and `demo_viewer` binary manifests. P2:CE's
Developer Content depot `440008` reports 0 bytes on the listed branches.
An anonymous SteamCMD attempt to inspect Portal 2's older `demo_viewer`
binary depot `624` failed with `missing license for depot (No subscription)`.
The installed depot manifests and files have no `.dbg` or `.pdb` payloads.
Older authenticated manifests and private branches remain unverified.

The [Steam2 archive catalog](https://femtendo.github.io/steam2-catalog/)
indexes historical Portal 2 depot file manifests. A path scan of depots
`621`–`629`, `645`, `660`, `841`, `843`, and prototype depot `852` found none
of the 136 distinct basenames in the configured missing `.cpp` list.
[Depot 841's file history](https://femtendo.github.io/steam2-catalog/data/vfiles/841.json.gz)
does list 21 Linux `.so.dbg` files in version `1`, but they are shared engine
modules such as `engine.so.dbg` and `vphysics.so.dbg`, with no Portal 2
`client.so.dbg` or `server.so.dbg`. The catalog indexes file paths and sizes;
it is not evidence that a matching debug payload is installed locally.

Steam2 depot `852` version `3` and depot `841` version `1` each include
macOS client/server dSYM bundles. The extracted bundles match their companion
Mach-O binaries by UUID. Their DWARF data contains source paths, function and
type information, and line mappings, but not the original C++ file bodies.
Across both builds, 94 of the 136 distinct selected missing `.cpp` paths
appear in the debug metadata. The binaries and dSYM files are retained
at `/home/john/Downloads/portal2-steam2-research` for source reconstruction;
no missing gameplay `.cpp`
has been recovered verbatim from these depots. The actual XSI tool source and
historical script sources found in Steam2 are preserved under `external/`.
Ghidra 12.0.4 generated [pseudocode references](../../../external/portal2_steam2_decompiled/README.md)
for 50 client and 48 server paths in depot 841, and 46 client and 45 server
paths in depot 852. These are not directly buildable implementations.

## Reconstructed sources

The 94 selected `.cpp` paths that have Steam2 pseudocode, and the headers the
VPCs list beside them, were written from that pseudocode, the DWARF skeletons
under [`skeleton/`](../../../external/portal2_steam2_decompiled/README.md) and
strings, tables and constants read from the retained binaries. Each such file
starts with a `Portal 2 reconstruction` header; none is original Valve source.
Code follows the Portal 2 API that the imported consumers use, so it does not
compile against this tree's Source SDK 2013 base game yet. Check a file with

    python3 tools/portal2/p2_compile_check.py server --probe <files>
    python3 tools/portal2/p2_compile_check.py client --probe <files>

which stubs absent foreign headers under `build-p2/p2-probe-shims` and reports
only diagnostics located in the checked file and its header. On 2026-09-23,
33 of 48 server-side and 26 of 50 client-side `.cpp` checks (shared files on
both sides) had no such diagnostics. Every remaining diagnostic names base-game
or engine API that the later Portal 2 base provides and this tree lacks, for
example split-screen helpers, `RenderableInstance_t`, the engine paint-map
calls, `sharedvar.h`, the blobulator library, `c_triggers.h` / networked
`CBaseTrigger`, `ConVar::GetColor`, `CBaseEntity::Forward`, the
`UTIL_TraceRay( ray, mask, filter, tr )` overload, the response-rules
`CriteriaSet` and `GetClientMode()`. The Portal 2 server still compiles the
Portal 1 `func_noportal_volume`, `func_portal_bumper`, `trigger_portal_cleanser`,
`npc_portal_turret_floor` and `npc_security_camera` sources; their Portal 2
networking and methods declared by the new headers need Portal 2 versions of
those files, and no Portal 2 `weapon_portalgun.h` exists yet.

## Reconstructed from the retail Linux client (2026-09-24)

These client paths have no leaked source and no Steam2 pseudocode, because the
features shipped after 2010. They were reconstructed from the retail
`portal2/bin/linux32/client.so`: RTTI names, vtables, strings, constants and
Ghidra 12.0.4 decompiler output (project `portal2_retail_linux_client` under
`/tmp/codex-portal2-ghidra`), plus the retail `.res` and script files in the
installed VPKs. Each file has a `Portal 2 reconstruction` header.

- `paint_hud_paint_ammo.cpp`: `CHUDPaintAmmo`. Retail `ShouldDraw()` returns
  false, and no `resource/ui/hud_paint_ammo.res` ships.
- `hud_challenge_stats_panel.cpp`: `CHUDChallengeStats`, plus the
  `hud_set_challenge_font_color`, `+leaderboard` and `-leaderboard` commands.
- `hud_vs_score_panel.cpp` and `.h`: `CHUDVSScorePanel`. `C_InfoPortalScore`
  (`c_info_portal_score.cpp`) drives it as retail does. No `.res` ships.
- `vgui/vgui_base_progress_screen.cpp` and `.h`, `vgui_sp_progress_screen.cpp`
  and `vgui_mp_progress_screen.cpp`: the elevator and co-op lightboards
  (`sp_progress_sign`, `mp_progress_sign`). The 2010 `mp_progress_sign` class
  was removed from `vgui_mp_lobby_screen.cpp`. The flicker trigger needs the
  panel's enabled state to follow the screen's active state, which
  `c_vguiscreen.cpp` now does under `PORTAL2`. Community-map lightboards (icons
  from Workshop tags) are unsupported.
- `vgui/vgui_mp_credits_screen.cpp`: `CVGUI_MP_CreditsScreen`
  (`mp_coop_credits_screen`).
- `portal2_leaderboard.cpp` and `.h`, `portal2_leaderboard_manager.cpp` and
  `.h`, and `game/shared/portal2/portal2_leaderboard_bucketizer.cpp` and `.h`:
  challenge mode leaderboards. The Steam UserStats and Web API requests compile
  only without `NO_STEAM`. This build defines `NO_STEAM`, so
  `GetLeaderboard()` returns NULL with a one-time DevWarning. The challenge map
  list and the GLaDOS reaction logic work without Steam.
- `c_community_coop.cpp` and `.h`: `C_CommunityCoopManager`. It compiles only
  with `PORTAL2_PUZZLEMAKER` and without `NO_STEAM`, like all of its BaseModUI
  callers. The Linux VPC configuration enables neither, so the unit is empty
  here.

One listed path has no retail counterpart and stays intentionally empty:

- `c_portal2_lesson.cpp`: the retail lesson classes (`CBaseLesson`,
  `CTextLesson`, `CIconLesson`, `CScriptedIconLesson`) all come from
  `c_baselesson.cpp`, and the binary records that source path. No other lesson
  class, string or RTTI name exists.

`c_portal_beam_helper.cpp` and `.h` were first kept empty for lack of an RTTI
name. On 2026-09-24 they were reconstructed from the retail Linux `client.so`
(2014), where the non-polymorphic `C_PortalBeamHelper` is called from
`C_PortalLaser` and `C_NPC_Portal_FloorTurret`: it re-traces a beam through up to
ten portal hops each frame and places one client `C_Beam` per segment. The 2010
Steam2 builds predate it and drew the laser as one server beam.

## Configured Linux source selection

On 2026-09-24, `./play_p2` still fails source-presence preflight with **23
client** and **12 server** selected `.cpp` paths missing. The catapult client,
shared catapult, client base-projector, projected sound proxy, fizzler sound
player, client score receiver, material-index proxy and radial button account for eight fewer
client paths and two fewer server paths than the first 28-client/13-server build
run. Those client files compile with the configured flags. The server catapult,
base-projector and material-index proxy files pass
`p2_compile_check.py --probe` with zero own diagnostics; a direct server
compile still fails in imported base-game headers. Probe mode does not
establish whole-target compile, link, network or gameplay behavior. With
`P2_ALLOW_INCOMPLETE=1`, Waf reaches compilation and stops on unrelated
missing `xlast_portal2/inc_coop_maps.inc` and `radialbutton.h` includes, among
the wider base-game/API gaps. Reproduce source presence with
`python3 scripts/waifulib/portal2_source_inventory.py --selected-build build-p2`.
The 20 selected source paths that remained on 2026-09-24 now have files.
Some are empty placeholder translation units with no gameplay behavior.
The configured source inventory reports zero missing paths.
`./play_p2` reaches compilation and still fails on missing headers and
later-engine API differences in retained sources. A keep-going build recorded
2,052 compiler diagnostics in `build-p2/portal2-stub-build.log`; no client or
server binary was produced by that run.
The incomplete-build diagnostic log is retained at
`build-p2/portal2-incomplete-build.log`. Its current first blockers include
`xlast_portal2/inc_coop_maps.inc`, `blobulator/Point3D.h`, and
`cegclientwrapper.h`, followed by Portal 2-specific base API gaps. It is
diagnostic evidence, not a successful product build.

The list below is the earlier 28-client/18-server inventory snapshot; it
records reconstruction targets, including files since added.

At snapshot time, these `.cpp` paths were selected by the `build-p2` Waf
configuration after VPC platform conditions. None had Steam2 pseudocode.

### Client (28)

- `game/client/portal2/c_baseprojector.cpp`
- `game/client/portal2/c_community_coop.cpp`
- `game/client/portal2/c_fizzler_multiorigin_sound_player.cpp`
- `game/client/portal2/c_info_portal_score.cpp`
- `game/client/portal2/c_portal2_lesson.cpp`
- `game/client/portal2/c_portal_beam_helper.cpp`
- `game/client/portal2/c_portal_mp_stats.cpp`
- `game/client/portal2/c_projected_entity_ambient_sound_proxy.cpp`
- `game/client/portal2/c_trigger_catapult.cpp`
- `game/client/portal2/hud_challenge_stats_panel.cpp`
- `game/client/portal2/hud_puzzlemaker_map_out_of_date.cpp`
- `game/client/portal2/hud_puzzlemaker_saving.cpp`
- `game/client/portal2/hud_vs_score_panel.cpp`
- `game/client/portal2/paint_hud_paint_ammo.cpp`
- `game/client/portal2/portal2_leaderboard.cpp`
- `game/client/portal2/portal2_leaderboard_manager.cpp`
- `game/client/portal2/radialbutton.cpp`
- `game/client/portal2/vgui/portal_leaderboard_graph_panel.cpp`
- `game/client/portal2/vgui/splinegraphpanel.cpp`
- `game/client/portal2/vgui/vgui_base_progress_screen.cpp`
- `game/client/portal2/vgui/vgui_mp_credits_screen.cpp`
- `game/client/portal2/vgui/vgui_mp_progress_screen.cpp`
- `game/client/portal2/vgui/vgui_sp_progress_screen.cpp`
- `game/shared/portal2/material_index_data_ops_proxy.cpp`
- `game/shared/portal2/portal2_leaderboard_bucketizer.cpp`
- `game/shared/portal2/trigger_catapult_shared.cpp`
- `game/shared/portal2/weapon_item_base_shared.cpp`
- `game/shared/portal2/weapon_promo_items_shared.cpp`

### Server (18)

- `game/server/portal2/challenge_mode_end_node.cpp`
- `game/server/portal2/fizzler_multiorigin_sound_player.cpp`
- `game/server/portal2/info_portal_score.cpp`
- `game/server/portal2/logic_timescale.cpp`
- `game/server/portal2/npc_wheatley_boss.cpp`
- `game/server/portal2/paint_sphere.cpp`
- `game/server/portal2/portal2_research_data_tracker.cpp`
- `game/server/portal2/portal_mp_stats.cpp`
- `game/server/portal2/portal_procedural_generator.cpp`
- `game/server/portal2/portal_ui_controller.cpp`
- `game/server/portal2/projected_entity_ambient_sound_proxy.cpp`
- `game/server/portal2/prop_hot_potato.cpp`
- `game/server/portal2/prop_monster_box.cpp`
- `game/server/portal2/trigger_catapult.cpp`
- `game/shared/portal2/material_index_data_ops_proxy.cpp`
- `game/shared/portal2/trigger_catapult_shared.cpp`
- `game/shared/portal2/weapon_item_base_shared.cpp`
- `game/shared/portal2/weapon_promo_items_shared.cpp`

## Client VPC paths (60)

- `common/ps3/vjobutils.cpp`
- `game/client/portal2/c_baseprojector.cpp`
- `game/client/portal2/c_community_coop.cpp`
- `game/client/portal2/c_community_coop.h`
- `game/client/portal2/c_fizzler_multiorigin_sound_player.cpp`
- `game/client/portal2/c_fizzler_multiorigin_sound_player.h`
- `game/client/portal2/c_info_portal_score.cpp`
- `game/client/portal2/c_portal2_lesson.cpp`
- `game/client/portal2/c_portal_beam_helper.cpp`
- `game/client/portal2/c_portal_beam_helper.h`
- `game/client/portal2/c_portal_mp_stats.cpp`
- `game/client/portal2/c_portal_mp_stats.h`
- `game/client/portal2/c_projected_entity_ambient_sound_proxy.cpp`
- `game/client/portal2/c_projected_entity_ambient_sound_proxy.h`
- `game/client/portal2/c_trigger_catapult.cpp`
- `game/client/portal2/c_trigger_catapult.h`
- `game/client/portal2/hud_challenge_stats_panel.cpp`
- `game/client/portal2/hud_puzzlemaker_map_out_of_date.cpp`
- `game/client/portal2/hud_puzzlemaker_saving.cpp`
- `game/client/portal2/hud_vs_score_panel.cpp`
- `game/client/portal2/paint_hud_paint_ammo.cpp`
- `game/client/portal2/portal2_econ_ui.cpp`
- `game/client/portal2/portal2_item_selection_panel.cpp`
- `game/client/portal2/portal2_item_selection_panel.h`
- `game/client/portal2/portal2_leaderboard.cpp`
- `game/client/portal2/portal2_leaderboard.h`
- `game/client/portal2/portal2_leaderboard_manager.cpp`
- `game/client/portal2/portal2_leaderboard_manager.h`
- `game/client/portal2/portal2_playermodelpanel.cpp`
- `game/client/portal2/portal2_playermodelpanel.h`
- `game/client/portal2/portal2_store_preview_item.cpp`
- `game/client/portal2/portal2_store_preview_item.h`
- `game/client/portal2/radialbutton.cpp`
- `game/client/portal2/radialbutton.h`
- `game/client/portal2/vgui/portal_leaderboard_graph_panel.cpp`
- `game/client/portal2/vgui/portal_leaderboard_graph_panel.h`
- `game/client/portal2/vgui/splinegraphpanel.cpp`
- `game/client/portal2/vgui/splinegraphpanel.h`
- `game/client/portal2/vgui/vgui_base_progress_screen.cpp`
- `game/client/portal2/vgui/vgui_base_progress_screen.h`
- `game/client/portal2/vgui/vgui_mp_credits_screen.cpp`
- `game/client/portal2/vgui/vgui_mp_progress_screen.cpp`
- `game/client/portal2/vgui/vgui_sp_progress_screen.cpp`
- `game/shared/portal2/material_index_data_ops_proxy.cpp`
- `game/shared/portal2/material_index_data_ops_proxy.h`
- `game/shared/portal2/portal2_item_constants.h`
- `game/shared/portal2/portal2_item_inventory.cpp`
- `game/shared/portal2/portal2_item_inventory.h`
- `game/shared/portal2/portal2_item_schema.cpp`
- `game/shared/portal2/portal2_item_schema.h`
- `game/shared/portal2/portal2_item_system.cpp`
- `game/shared/portal2/portal2_item_system.h`
- `game/shared/portal2/portal2_leaderboard_bucketizer.cpp`
- `game/shared/portal2/portal2_leaderboard_bucketizer.h`
- `game/shared/portal2/trigger_catapult_shared.cpp`
- `game/shared/portal2/vs_game_shared.h`
- `game/shared/portal2/weapon_item_base_shared.cpp`
- `game/shared/portal2/weapon_item_base_shared.h`
- `game/shared/portal2/weapon_promo_items_shared.cpp`
- `game/shared/portal2/weapon_promo_items_shared.h`

## Server VPC paths (38)

- `game/server/portal2/challenge_mode_end_node.cpp`
- `game/server/portal2/fizzler_multiorigin_sound_player.cpp`
- `game/server/portal2/fizzler_multiorigin_sound_player.h`
- `game/server/portal2/info_portal_score.cpp`
- `game/server/portal2/logic_timescale.cpp`
- `game/server/portal2/logic_timescale.h`
- `game/server/portal2/npc_wheatley_boss.cpp`
- `game/server/portal2/paint_sphere.cpp`
- `game/server/portal2/paint_sphere.h`
- `game/server/portal2/portal2_research_data_tracker.cpp`
- `game/server/portal2/portal2_research_data_tracker.h`
- `game/server/portal2/portal_mp_stats.cpp`
- `game/server/portal2/portal_mp_stats.h`
- `game/server/portal2/portal_procedural_generator.cpp`
- `game/server/portal2/portal_procedural_generator.h`
- `game/server/portal2/portal_ui_controller.cpp`
- `game/server/portal2/portal_ui_controller.h`
- `game/server/portal2/projected_entity_ambient_sound_proxy.cpp`
- `game/server/portal2/projected_entity_ambient_sound_proxy.h`
- `game/server/portal2/prop_hot_potato.cpp`
- `game/server/portal2/prop_monster_box.cpp`
- `game/server/portal2/prop_monster_box.h`
- `game/server/portal2/trigger_catapult.cpp`
- `game/server/portal2/trigger_catapult.h`
- `game/shared/portal2/material_index_data_ops_proxy.cpp`
- `game/shared/portal2/material_index_data_ops_proxy.h`
- `game/shared/portal2/portal2_item_inventory.cpp`
- `game/shared/portal2/portal2_item_inventory.h`
- `game/shared/portal2/portal2_item_schema.cpp`
- `game/shared/portal2/portal2_item_schema.h`
- `game/shared/portal2/portal2_item_system.cpp`
- `game/shared/portal2/portal2_item_system.h`
- `game/shared/portal2/trigger_catapult_shared.cpp`
- `game/shared/portal2/vs_game_shared.h`
- `game/shared/portal2/weapon_item_base_shared.cpp`
- `game/shared/portal2/weapon_item_base_shared.h`
- `game/shared/portal2/weapon_promo_items_shared.cpp`
- `game/shared/portal2/weapon_promo_items_shared.h`

## Missing quoted include names (67)

- `../common/xlast_portal2/inc_coop_maps.inc` — first referenced by `game/client/portal2/gameui/portal2/basemodpanel.cpp`
- `../common/xlast_portal2/inc_sp_maps.inc` — first referenced by `game/client/portal2/gameui/portal2/basemodpanel.cpp`
- `../SteamUI/PlatformMainPanel.h` — first referenced by `game/client/portal2/gameui/vguisystemmoduleloader.cpp`
- `baseplayer.h` — first referenced by `game/shared/cam_thirdperson.h`
- `blobulator/Implicit/ImpParticle.h` — first referenced by `game/client/portal2/c_paintblob_render.h`
- `blobulator/Implicit/ImpRenderer.h` — first referenced by `game/client/portal2/c_paintblob_render.cpp`
- `blobulator/Implicit/ImpTiler.h` — first referenced by `game/client/portal2/c_paintblob_render.cpp`
- `blobulator/Point3D.h` — first referenced by `game/client/portal2/c_paintblob_render.cpp`
- `blobulator/SmartArray.h` — first referenced by `game/client/portal2/c_paintblob_render.cpp`
- `c_combatweaponworldclone.h` — first referenced by `game/client/portal2/portal/c_portal_base2d.cpp`
- `c_community_coop.h` — first referenced by `game/client/portal2/gameui/portal2/vcommunitymapdialog.cpp`
- `c_portal_mp_stats.h` — first referenced by `game/shared/portal2/portal_mp_gamerules.cpp`
- `c_trigger_catapult.h` — first referenced by `game/shared/portal2/portal_player_shared.cpp`
- `c_triggers.h` — first referenced by `game/client/portal/c_trigger_portal_cleanser.h`
- `cache_hints.h` — first referenced by `game/shared/portal2/paint_blobs_shared.cpp`
- `cegclientwrapper.h` — first referenced by `game/shared/portal2/portal_placement.cpp`
- `CegClientWrapper.h` — first referenced by `game/shared/portal2/portal_placement.h`
- `cvisibilitymonitor.h` — first referenced by `game/server/portal2/prop_button.cpp`
- `econ_gcmessages.h` — first referenced by `game/client/portal2/gameui/portal2/vcommunitymapdialog.cpp`
- `econ_ui.h` — first referenced by `game/client/portal2/gameui/gameui_interface.cpp`
- `filesystem/IXboxInstaller.h` — first referenced by `game/client/gameui/basemodpanel.cpp`
- `game_controls/igameuisystemmgr.h` — first referenced by `game/client/gameui.cpp`
- `game_timescale_shared.h` — first referenced by `game/client/portal2/portal/c_prop_portal.cpp`
- `gc_clientsystem.h` — first referenced by `game/client/portal2/gameui/portal2/vcommunitymapdialog.cpp`
- `gc_serversystem.h` — first referenced by `game/server/portal2/portal/portal_player.cpp`
- `hud_locator_target.h` — first referenced by `game/client/portal2/radialmenu.cpp`
- `imaterialproxydict.h` — first referenced by `game/client/portal2/portal/materialproxy_portalstatic.cpp`
- `inputsystem/iinputstacksystem.h` — first referenced by `game/client/gameui.cpp`
- `matchmaking/imatchframework.h` — first referenced by `game/client/portal/c_portal_gamestats.h`
- `matchmaking/imatchsystem.h` — first referenced by `game/client/gameui/uigamedata.h`
- `matchmaking/iplayer.h` — first referenced by `game/client/gameui/uigamedata.h`
- `matchmaking/iplayermanager.h` — first referenced by `game/client/gameui/uigamedata.h`
- `matchmaking/iservermanager.h` — first referenced by `game/client/gameui/uigamedata.h`
- `matchmaking/mm_helpers.h` — first referenced by `game/shared/portal2/portal_mp_gamerules.cpp`
- `matchmaking/portal2/imatchext_portal2.h` — first referenced by `game/client/portal2/gameui/portal2/uigamedata.h`
- `matchmaking/swarm/imatchext_swarm.h` — first referenced by `game/client/portal2/gameui/engineinterface.h`
- `material_index_data_ops_proxy.h` — first referenced by `game/shared/portal2/prop_paint_power_user.h`
- `player_voice_listener.h` — first referenced by `game/shared/portal2/portal_gamerules.cpp`
- `portal2_item_inventory.h` — first referenced by `game/client/portal2/portal/c_portal_player.h`
- `portal2_leaderboard.h` — first referenced by `game/client/portal2/gameui/portal2/vportalleaderboardhud.h`
- `portal2_leaderboard_manager.h` — first referenced by `game/client/portal2/gameui/portal2/vdialoglistbutton.h`
- `portal2_research_data_tracker.h` — first referenced by `game/server/portal2/portal/portal_player.cpp`
- `portal_mp_stats.h` — first referenced by `game/shared/portal2/weapon_portalgun_shared.cpp`
- `portal_ui_controller.h` — first referenced by `game/shared/portal2/portal_mp_gamerules.cpp`
- `prop_monster_box.h` — first referenced by `game/server/portal2/prop_floor_button.cpp`
- `ps3/ps3_core.h` — first referenced by `game/client/gameui/gameui_interface.cpp`
- `ps3/ps3_win32stubs.h` — first referenced by `game/client/gameui/gameui_interface.cpp`
- `ps3/saverestore_ps3_api_ui.h` — first referenced by `game/client/portal2/gameui/engineinterface.h`
- `puzzlemaker/puzzlemaker.h` — first referenced by `game/client/portal2/gameui/portal2/vpuzzlemakermychambers.h`
- `radialbutton.h` — first referenced by `game/client/portal2/radialmenu.cpp`
- `rendersystem/irenderdevice.h` — first referenced by `public/meshutils/mesh.h`
- `sendprop_priorities.h` — first referenced by `game/server/portal2/portal/portal_player.cpp`
- `sharedvar.h` — first referenced by `game/client/portal2/c_paint_stream.h`
- `sysutil/sysutil_gamecontent.h` — first referenced by `game/client/portal2/gameui/portal2/vattractscreen.cpp`
- `sysutil/sysutil_oskdialog.h` — first referenced by `game/client/portal2/gameui/portal2/vsteamlinkdialog.cpp`
- `sysutil/sysutil_savedata.h` — first referenced by `game/client/portal2/gameui/portal2/vcommunitymapdialog.cpp`
- `tf_gamerules.h` — first referenced by `game/shared/Multiplayer/multiplayer_animstate.cpp`
- `tier1/tokenset.h` — first referenced by `game/client/gameui/uigamedata.cpp`
- `tier2/resourceprecacher.h` — first referenced by `game/client/portal2/gameui/basesavegamedialog.cpp`
- `tokenset.h` — first referenced by `game/client/gameui/uigamedata.h`
- `trigger_catapult.h` — first referenced by `game/shared/portal2/portal_player_shared.cpp`
- `utlreference.h` — first referenced by `game/client/portal2/c_weapon_paintgun.h`
- `vgui/portal_leaderboard_graph_panel.h` — first referenced by `game/client/portal2/gameui/portal2/vplaytestdemosdialog.cpp`
- `videocfg/videocfg.h` — first referenced by `game/client/portal2/gameui/portal2/vvideo.cpp`
- `vscript_server.h` — first referenced by `game/shared/portal2/portal_gamerules.cpp`
- `xlast_portal2/inc_coop_maps.inc` — first referenced by `game/client/portal2/gameui/portal2/uigamedata_storage.cpp`
- `xlast_portal2/inc_sp_maps.inc` — first referenced by `game/client/portal2/gameui/portal2/uigamedata_storage.cpp`
