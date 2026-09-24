# Portal 2 source gap inventory

Generated from the imported Portal 2 VPC references and quoted includes.
The VPC list is a superset of conditional platform/feature selections; it is not a
proof that every row is needed for a Linux build. The include scan uses file-name
presence and cannot prove API or ABI compatibility. Compile/link/runtime checks
will reveal further gaps.

98 missing VPC references (84 unique paths) out of 951 declared references.
The imported source also names 67 quoted include paths whose basename is absent from this checkout.

Regenerate the VPC portion with `python3 scripts/waifulib/portal2_source_inventory.py`.
For the configured build, run
`python3 scripts/waifulib/portal2_source_inventory.py --selected-build build-p2`.

## Local source candidates checked

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

## Configured Linux source selection

These `.cpp` paths are selected by the current `build-p2` Waf configuration
after VPC platform conditions. They are the immediate source-presence build gate.
None of them has Steam2 pseudocode.

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
