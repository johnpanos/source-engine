# Portal 2 source gap inventory

Generated from the imported Portal 2 VPC references and quoted includes.
The VPC list is a superset of conditional platform/feature selections; it is not a
proof that every row is needed for a Linux build. The include scan uses file-name
presence and cannot prove API or ABI compatibility. Compile/link/runtime checks
will reveal further gaps.

262 missing VPC references (241 unique paths) out of 949 declared references.
The imported source also names 95 quoted include paths whose basename is absent from this checkout.

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

## Configured Linux source selection

These `.cpp` paths are selected by the current `build-p2` Waf configuration
after VPC platform conditions. They are the immediate source-presence build gate.

### Client (78)

- `game/client/portal/c_func_noportal_volume.cpp`
- `game/client/portal/c_func_portal_bumper.cpp`
- `game/client/portal/c_portal_gamestats.cpp`
- `game/client/portal/c_trigger_portal_cleanser.cpp`
- `game/client/portal2/c_baseprojectedentity.cpp`
- `game/client/portal2/c_baseprojector.cpp`
- `game/client/portal2/c_community_coop.cpp`
- `game/client/portal2/c_env_portal_laser.cpp`
- `game/client/portal2/c_fizzler_effects.cpp`
- `game/client/portal2/c_fizzler_multiorigin_sound_player.cpp`
- `game/client/portal2/c_hitbox_damaged_entity.cpp`
- `game/client/portal2/c_indicator_panel.cpp`
- `game/client/portal2/c_info_placement_helper.cpp`
- `game/client/portal2/c_info_portal_score.cpp`
- `game/client/portal2/c_item_photo.cpp`
- `game/client/portal2/c_level_placard_display.cpp`
- `game/client/portal2/c_mp_lobby_screen.cpp`
- `game/client/portal2/c_npc_hover_turret.cpp`
- `game/client/portal2/c_npc_personality_core.cpp`
- `game/client/portal2/c_paint_input.cpp`
- `game/client/portal2/c_paint_messages.cpp`
- `game/client/portal2/c_paint_sprayer.cpp`
- `game/client/portal2/c_paint_stream.cpp`
- `game/client/portal2/c_paintblob.cpp`
- `game/client/portal2/c_paintblob_render.cpp`
- `game/client/portal2/c_perfmonitor.cpp`
- `game/client/portal2/c_pointsurvey.cpp`
- `game/client/portal2/c_portal2_lesson.cpp`
- `game/client/portal2/c_portal_beam_helper.cpp`
- `game/client/portal2/c_portal_mp_stats.cpp`
- `game/client/portal2/c_portal_playerlocaldata.cpp`
- `game/client/portal2/c_portal_pointpush.cpp`
- `game/client/portal2/c_portal_race_checkpoint.cpp`
- `game/client/portal2/c_projected_entity_ambient_sound_proxy.cpp`
- `game/client/portal2/c_projectedwallentity.cpp`
- `game/client/portal2/c_prop_linked_portal_door.cpp`
- `game/client/portal2/c_prop_paint_bomb.cpp`
- `game/client/portal2/c_prop_rockettripwire.cpp`
- `game/client/portal2/c_prop_tractorbeam.cpp`
- `game/client/portal2/c_trigger_catapult.cpp`
- `game/client/portal2/c_trigger_paint_cleanser.cpp`
- `game/client/portal2/c_trigger_tractorbeam.cpp`
- `game/client/portal2/c_weapon_paintgun.cpp`
- `game/client/portal2/hud_challenge_stats_panel.cpp`
- `game/client/portal2/hud_coop_ping_indicator.cpp`
- `game/client/portal2/hud_indicator.cpp`
- `game/client/portal2/hud_multiplayerbasicinfo.cpp`
- `game/client/portal2/hud_puzzlemaker_map_out_of_date.cpp`
- `game/client/portal2/hud_puzzlemaker_saving.cpp`
- `game/client/portal2/hud_taunt_earned.cpp`
- `game/client/portal2/hud_viewfinder.cpp`
- `game/client/portal2/hud_vs_score_panel.cpp`
- `game/client/portal2/paint_hud_paint_ammo.cpp`
- `game/client/portal2/polygonbutton.cpp`
- `game/client/portal2/portal2_leaderboard.cpp`
- `game/client/portal2/portal2_leaderboard_manager.cpp`
- `game/client/portal2/radialbutton.cpp`
- `game/client/portal2/radialmenu_taunt.cpp`
- `game/client/portal2/vgui/portal_leaderboard_graph_panel.cpp`
- `game/client/portal2/vgui/portalclientscoreboard.cpp`
- `game/client/portal2/vgui/splinegraphpanel.cpp`
- `game/client/portal2/vgui/surveypanel.cpp`
- `game/client/portal2/vgui/vgui_base_progress_screen.cpp`
- `game/client/portal2/vgui/vgui_indicator_panel.cpp`
- `game/client/portal2/vgui/vgui_level_placard.cpp`
- `game/client/portal2/vgui/vgui_mp_credits_screen.cpp`
- `game/client/portal2/vgui/vgui_mp_lobby_screen.cpp`
- `game/client/portal2/vgui/vgui_mp_progress_screen.cpp`
- `game/client/portal2/vgui/vgui_sp_progress_screen.cpp`
- `game/shared/portal2/damage_database.cpp`
- `game/shared/portal2/hitbox_damaged_entity_shared.cpp`
- `game/shared/portal2/material_index_data_ops_proxy.cpp`
- `game/shared/portal2/paint_saverestore.cpp`
- `game/shared/portal2/portal2_leaderboard_bucketizer.cpp`
- `game/shared/portal2/portal_pointpush_shared.cpp`
- `game/shared/portal2/trigger_catapult_shared.cpp`
- `game/shared/portal2/weapon_item_base_shared.cpp`
- `game/shared/portal2/weapon_promo_items_shared.cpp`

### Server (66)

- `game/server/portal2/baseprojectedentity.cpp`
- `game/server/portal2/baseprojector.cpp`
- `game/server/portal2/challenge_mode_end_node.cpp`
- `game/server/portal2/cpaintblob.cpp`
- `game/server/portal2/env_portal_laser.cpp`
- `game/server/portal2/fizzler_effects.cpp`
- `game/server/portal2/fizzler_multiorigin_sound_player.cpp`
- `game/server/portal2/func_placement_clip.cpp`
- `game/server/portal2/func_portalled.cpp`
- `game/server/portal2/indicator_panel.cpp`
- `game/server/portal2/info_coop_spawn.cpp`
- `game/server/portal2/info_placement_helper.cpp`
- `game/server/portal2/info_player_ping_detector.cpp`
- `game/server/portal2/info_portal_score.cpp`
- `game/server/portal2/item_nugget.cpp`
- `game/server/portal2/item_paint_power_pickup.cpp`
- `game/server/portal2/level_placard_display.cpp`
- `game/server/portal2/logic_timescale.cpp`
- `game/server/portal2/mp_lobby_screen.cpp`
- `game/server/portal2/npc_hover_turret.cpp`
- `game/server/portal2/npc_personality_core.cpp`
- `game/server/portal2/npc_wheatley_boss.cpp`
- `game/server/portal2/paint_bot_temp.cpp`
- `game/server/portal2/paint_database.cpp`
- `game/server/portal2/paint_sphere.cpp`
- `game/server/portal2/paint_sprayer.cpp`
- `game/server/portal2/paint_stream.cpp`
- `game/server/portal2/paint_swap_guns.cpp`
- `game/server/portal2/point_changelevel.cpp`
- `game/server/portal2/point_futbol_shooter.cpp`
- `game/server/portal2/point_laser_target.cpp`
- `game/server/portal2/pointsurvey.cpp`
- `game/server/portal2/portal2_research_data_tracker.cpp`
- `game/server/portal2/portal_mp_stats.cpp`
- `game/server/portal2/portal_playerlocaldata.cpp`
- `game/server/portal2/portal_pointpush.cpp`
- `game/server/portal2/portal_procedural_generator.cpp`
- `game/server/portal2/portal_race_checkpoint.cpp`
- `game/server/portal2/portal_ui_controller.cpp`
- `game/server/portal2/projected_entity_ambient_sound_proxy.cpp`
- `game/server/portal2/projectedwallentity.cpp`
- `game/server/portal2/prop_exploding_futbol.cpp`
- `game/server/portal2/prop_hot_potato.cpp`
- `game/server/portal2/prop_monster_box.cpp`
- `game/server/portal2/prop_paint_bomb.cpp`
- `game/server/portal2/prop_personality_sphere.cpp`
- `game/server/portal2/prop_physics_paintable.cpp`
- `game/server/portal2/prop_rockettripwire.cpp`
- `game/server/portal2/prop_tractorbeam.cpp`
- `game/server/portal2/propglassfutbol.cpp`
- `game/server/portal2/propwallprojector.cpp`
- `game/server/portal2/rocketprojectile.cpp`
- `game/server/portal2/trigger_catapult.cpp`
- `game/server/portal2/trigger_paint_cleanser.cpp`
- `game/server/portal2/trigger_ping_detector.cpp`
- `game/server/portal2/trigger_playerteam.cpp`
- `game/server/portal2/trigger_tractorbeam.cpp`
- `game/server/portal2/weapon_paintgun.cpp`
- `game/shared/portal2/damage_database.cpp`
- `game/shared/portal2/hitbox_damaged_entity_shared.cpp`
- `game/shared/portal2/material_index_data_ops_proxy.cpp`
- `game/shared/portal2/paint_saverestore.cpp`
- `game/shared/portal2/portal_pointpush_shared.cpp`
- `game/shared/portal2/trigger_catapult_shared.cpp`
- `game/shared/portal2/weapon_item_base_shared.cpp`
- `game/shared/portal2/weapon_promo_items_shared.cpp`

## Client VPC paths (142)

- `common/ps3/vjobutils.cpp`
- `game/client/portal/c_func_noportal_volume.cpp`
- `game/client/portal/c_func_noportal_volume.h`
- `game/client/portal/c_func_portal_bumper.cpp`
- `game/client/portal/c_func_portal_bumper.h`
- `game/client/portal/c_npc_portal_turret_floor.h`
- `game/client/portal/c_portal_gamestats.cpp`
- `game/client/portal/c_portal_gamestats.h`
- `game/client/portal/c_trigger_portal_cleanser.cpp`
- `game/client/portal/c_trigger_portal_cleanser.h`
- `game/client/portal2/c_baseprojectedentity.cpp`
- `game/client/portal2/c_baseprojectedentity.h`
- `game/client/portal2/c_baseprojector.cpp`
- `game/client/portal2/c_baseprojector.h`
- `game/client/portal2/c_community_coop.cpp`
- `game/client/portal2/c_community_coop.h`
- `game/client/portal2/c_env_portal_laser.cpp`
- `game/client/portal2/c_fizzler_effects.cpp`
- `game/client/portal2/c_fizzler_effects.h`
- `game/client/portal2/c_fizzler_multiorigin_sound_player.cpp`
- `game/client/portal2/c_fizzler_multiorigin_sound_player.h`
- `game/client/portal2/c_hitbox_damaged_entity.cpp`
- `game/client/portal2/c_hitbox_damaged_entity.h`
- `game/client/portal2/c_indicator_panel.cpp`
- `game/client/portal2/c_indicator_panel.h`
- `game/client/portal2/c_info_placement_helper.cpp`
- `game/client/portal2/c_info_placement_helper.h`
- `game/client/portal2/c_info_portal_score.cpp`
- `game/client/portal2/c_item_photo.cpp`
- `game/client/portal2/c_level_placard_display.cpp`
- `game/client/portal2/c_mp_lobby_screen.cpp`
- `game/client/portal2/c_mp_lobby_screen.h`
- `game/client/portal2/c_npc_hover_turret.cpp`
- `game/client/portal2/c_npc_personality_core.cpp`
- `game/client/portal2/c_paint_input.cpp`
- `game/client/portal2/c_paint_input.h`
- `game/client/portal2/c_paint_messages.cpp`
- `game/client/portal2/c_paint_sprayer.cpp`
- `game/client/portal2/c_paint_sprayer.h`
- `game/client/portal2/c_paint_stream.cpp`
- `game/client/portal2/c_paint_stream.h`
- `game/client/portal2/c_paintblob.cpp`
- `game/client/portal2/c_paintblob.h`
- `game/client/portal2/c_paintblob_render.cpp`
- `game/client/portal2/c_paintblob_render.h`
- `game/client/portal2/c_perfmonitor.cpp`
- `game/client/portal2/c_pointsurvey.cpp`
- `game/client/portal2/c_portal2_lesson.cpp`
- `game/client/portal2/c_portal_beam_helper.cpp`
- `game/client/portal2/c_portal_beam_helper.h`
- `game/client/portal2/c_portal_mp_stats.cpp`
- `game/client/portal2/c_portal_mp_stats.h`
- `game/client/portal2/c_portal_playerlocaldata.cpp`
- `game/client/portal2/c_portal_playerlocaldata.h`
- `game/client/portal2/c_portal_pointpush.cpp`
- `game/client/portal2/c_portal_pointpush.h`
- `game/client/portal2/c_portal_race_checkpoint.cpp`
- `game/client/portal2/c_projected_entity_ambient_sound_proxy.cpp`
- `game/client/portal2/c_projected_entity_ambient_sound_proxy.h`
- `game/client/portal2/c_projectedwallentity.cpp`
- `game/client/portal2/c_projectedwallentity.h`
- `game/client/portal2/c_prop_linked_portal_door.cpp`
- `game/client/portal2/c_prop_paint_bomb.cpp`
- `game/client/portal2/c_prop_paint_bomb.h`
- `game/client/portal2/c_prop_rockettripwire.cpp`
- `game/client/portal2/c_prop_tractorbeam.cpp`
- `game/client/portal2/c_trigger_catapult.cpp`
- `game/client/portal2/c_trigger_catapult.h`
- `game/client/portal2/c_trigger_paint_cleanser.cpp`
- `game/client/portal2/c_trigger_paint_cleanser.h`
- `game/client/portal2/c_trigger_tractorbeam.cpp`
- `game/client/portal2/c_trigger_tractorbeam.h`
- `game/client/portal2/c_weapon_paintgun.cpp`
- `game/client/portal2/c_weapon_paintgun.h`
- `game/client/portal2/hud_challenge_stats_panel.cpp`
- `game/client/portal2/hud_coop_ping_indicator.cpp`
- `game/client/portal2/hud_indicator.cpp`
- `game/client/portal2/hud_multiplayerbasicinfo.cpp`
- `game/client/portal2/hud_puzzlemaker_map_out_of_date.cpp`
- `game/client/portal2/hud_puzzlemaker_saving.cpp`
- `game/client/portal2/hud_taunt_earned.cpp`
- `game/client/portal2/hud_taunt_earned.h`
- `game/client/portal2/hud_viewfinder.cpp`
- `game/client/portal2/hud_vs_score_panel.cpp`
- `game/client/portal2/paint_hud_paint_ammo.cpp`
- `game/client/portal2/polygonbutton.cpp`
- `game/client/portal2/polygonbutton.h`
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
- `game/client/portal2/radialmenu_taunt.cpp`
- `game/client/portal2/radialmenu_taunt.h`
- `game/client/portal2/vgui/portal_leaderboard_graph_panel.cpp`
- `game/client/portal2/vgui/portal_leaderboard_graph_panel.h`
- `game/client/portal2/vgui/portalclientscoreboard.cpp`
- `game/client/portal2/vgui/portalclientscoreboard.h`
- `game/client/portal2/vgui/splinegraphpanel.cpp`
- `game/client/portal2/vgui/splinegraphpanel.h`
- `game/client/portal2/vgui/surveypanel.cpp`
- `game/client/portal2/vgui/surveypanel.h`
- `game/client/portal2/vgui/vgui_base_progress_screen.cpp`
- `game/client/portal2/vgui/vgui_base_progress_screen.h`
- `game/client/portal2/vgui/vgui_indicator_panel.cpp`
- `game/client/portal2/vgui/vgui_level_placard.cpp`
- `game/client/portal2/vgui/vgui_mp_credits_screen.cpp`
- `game/client/portal2/vgui/vgui_mp_lobby_screen.cpp`
- `game/client/portal2/vgui/vgui_mp_progress_screen.cpp`
- `game/client/portal2/vgui/vgui_sp_progress_screen.cpp`
- `game/shared/portal2/damage_database.cpp`
- `game/shared/portal2/damage_database.h`
- `game/shared/portal2/hitbox_damaged_entity_shared.cpp`
- `game/shared/portal2/hitbox_damaged_entity_shared.h`
- `game/shared/portal2/material_index_data_ops_proxy.cpp`
- `game/shared/portal2/material_index_data_ops_proxy.h`
- `game/shared/portal2/paint_saverestore.cpp`
- `game/shared/portal2/paint_saverestore.h`
- `game/shared/portal2/portal2_item_constants.h`
- `game/shared/portal2/portal2_item_inventory.cpp`
- `game/shared/portal2/portal2_item_inventory.h`
- `game/shared/portal2/portal2_item_schema.cpp`
- `game/shared/portal2/portal2_item_schema.h`
- `game/shared/portal2/portal2_item_system.cpp`
- `game/shared/portal2/portal2_item_system.h`
- `game/shared/portal2/portal2_leaderboard_bucketizer.cpp`
- `game/shared/portal2/portal2_leaderboard_bucketizer.h`
- `game/shared/portal2/portal_pointpush_shared.cpp`
- `game/shared/portal2/trigger_catapult_shared.cpp`
- `game/shared/portal2/vs_game_shared.h`
- `game/shared/portal2/weapon_item_base_shared.cpp`
- `game/shared/portal2/weapon_item_base_shared.h`
- `game/shared/portal2/weapon_promo_items_shared.cpp`
- `game/shared/portal2/weapon_promo_items_shared.h`

## Server VPC paths (120)

- `game/server/portal/npc_portal_turret_floor.h`
- `game/server/portal/npc_security_camera.h`
- `game/server/portal/trigger_portal_cleanser.h`
- `game/server/portal2/baseprojectedentity.cpp`
- `game/server/portal2/baseprojectedentity.h`
- `game/server/portal2/baseprojector.cpp`
- `game/server/portal2/baseprojector.h`
- `game/server/portal2/challenge_mode_end_node.cpp`
- `game/server/portal2/cpaintblob.cpp`
- `game/server/portal2/cpaintblob.h`
- `game/server/portal2/env_portal_laser.cpp`
- `game/server/portal2/env_portal_laser.h`
- `game/server/portal2/fizzler_effects.cpp`
- `game/server/portal2/fizzler_effects.h`
- `game/server/portal2/fizzler_multiorigin_sound_player.cpp`
- `game/server/portal2/fizzler_multiorigin_sound_player.h`
- `game/server/portal2/func_placement_clip.cpp`
- `game/server/portal2/func_portalled.cpp`
- `game/server/portal2/func_portalled.h`
- `game/server/portal2/hitbox_damaged_entity.h`
- `game/server/portal2/indicator_panel.cpp`
- `game/server/portal2/indicator_panel.h`
- `game/server/portal2/info_coop_spawn.cpp`
- `game/server/portal2/info_placement_helper.cpp`
- `game/server/portal2/info_placement_helper.h`
- `game/server/portal2/info_player_ping_detector.cpp`
- `game/server/portal2/info_portal_score.cpp`
- `game/server/portal2/item_nugget.cpp`
- `game/server/portal2/item_paint_power_pickup.cpp`
- `game/server/portal2/level_placard_display.cpp`
- `game/server/portal2/logic_timescale.cpp`
- `game/server/portal2/logic_timescale.h`
- `game/server/portal2/mp_lobby_screen.cpp`
- `game/server/portal2/npc_hover_turret.cpp`
- `game/server/portal2/npc_personality_core.cpp`
- `game/server/portal2/npc_wheatley_boss.cpp`
- `game/server/portal2/paint_bot_temp.cpp`
- `game/server/portal2/paint_bot_temp.h`
- `game/server/portal2/paint_database.cpp`
- `game/server/portal2/paint_database.h`
- `game/server/portal2/paint_sphere.cpp`
- `game/server/portal2/paint_sphere.h`
- `game/server/portal2/paint_sprayer.cpp`
- `game/server/portal2/paint_sprayer.h`
- `game/server/portal2/paint_stream.cpp`
- `game/server/portal2/paint_stream.h`
- `game/server/portal2/paint_swap_guns.cpp`
- `game/server/portal2/paint_swap_guns.h`
- `game/server/portal2/point_changelevel.cpp`
- `game/server/portal2/point_futbol_shooter.cpp`
- `game/server/portal2/point_laser_target.cpp`
- `game/server/portal2/point_laser_target.h`
- `game/server/portal2/pointsurvey.cpp`
- `game/server/portal2/pointsurvey.h`
- `game/server/portal2/portal2_research_data_tracker.cpp`
- `game/server/portal2/portal2_research_data_tracker.h`
- `game/server/portal2/portal_mp_stats.cpp`
- `game/server/portal2/portal_mp_stats.h`
- `game/server/portal2/portal_playerlocaldata.cpp`
- `game/server/portal2/portal_playerlocaldata.h`
- `game/server/portal2/portal_pointpush.cpp`
- `game/server/portal2/portal_pointpush.h`
- `game/server/portal2/portal_procedural_generator.cpp`
- `game/server/portal2/portal_procedural_generator.h`
- `game/server/portal2/portal_race_checkpoint.cpp`
- `game/server/portal2/portal_ui_controller.cpp`
- `game/server/portal2/portal_ui_controller.h`
- `game/server/portal2/projected_entity_ambient_sound_proxy.cpp`
- `game/server/portal2/projected_entity_ambient_sound_proxy.h`
- `game/server/portal2/projectedwallentity.cpp`
- `game/server/portal2/projectedwallentity.h`
- `game/server/portal2/prop_exploding_futbol.cpp`
- `game/server/portal2/prop_exploding_futbol.h`
- `game/server/portal2/prop_hot_potato.cpp`
- `game/server/portal2/prop_monster_box.cpp`
- `game/server/portal2/prop_monster_box.h`
- `game/server/portal2/prop_paint_bomb.cpp`
- `game/server/portal2/prop_paint_bomb.h`
- `game/server/portal2/prop_personality_sphere.cpp`
- `game/server/portal2/prop_physics_paintable.cpp`
- `game/server/portal2/prop_rockettripwire.cpp`
- `game/server/portal2/prop_rockettripwire.h`
- `game/server/portal2/prop_tractorbeam.cpp`
- `game/server/portal2/prop_tractorbeam.h`
- `game/server/portal2/propglassfutbol.cpp`
- `game/server/portal2/propwallprojector.cpp`
- `game/server/portal2/propwallprojector.h`
- `game/server/portal2/rocketprojectile.cpp`
- `game/server/portal2/rocketprojectile.h`
- `game/server/portal2/trigger_catapult.cpp`
- `game/server/portal2/trigger_catapult.h`
- `game/server/portal2/trigger_paint_cleanser.cpp`
- `game/server/portal2/trigger_paint_cleanser.h`
- `game/server/portal2/trigger_ping_detector.cpp`
- `game/server/portal2/trigger_playerteam.cpp`
- `game/server/portal2/trigger_tractorbeam.cpp`
- `game/server/portal2/trigger_tractorbeam.h`
- `game/server/portal2/weapon_paintgun.cpp`
- `game/server/portal2/weapon_paintgun.h`
- `game/shared/portal2/damage_database.cpp`
- `game/shared/portal2/damage_database.h`
- `game/shared/portal2/hitbox_damaged_entity_shared.cpp`
- `game/shared/portal2/hitbox_damaged_entity_shared.h`
- `game/shared/portal2/material_index_data_ops_proxy.cpp`
- `game/shared/portal2/material_index_data_ops_proxy.h`
- `game/shared/portal2/paint_saverestore.cpp`
- `game/shared/portal2/paint_saverestore.h`
- `game/shared/portal2/portal2_item_inventory.cpp`
- `game/shared/portal2/portal2_item_inventory.h`
- `game/shared/portal2/portal2_item_schema.cpp`
- `game/shared/portal2/portal2_item_schema.h`
- `game/shared/portal2/portal2_item_system.cpp`
- `game/shared/portal2/portal2_item_system.h`
- `game/shared/portal2/portal_pointpush_shared.cpp`
- `game/shared/portal2/trigger_catapult_shared.cpp`
- `game/shared/portal2/vs_game_shared.h`
- `game/shared/portal2/weapon_item_base_shared.cpp`
- `game/shared/portal2/weapon_item_base_shared.h`
- `game/shared/portal2/weapon_promo_items_shared.cpp`
- `game/shared/portal2/weapon_promo_items_shared.h`

## Missing quoted include names (95)

- `../SteamUI/PlatformMainPanel.h` — first referenced by `game/client/portal2/gameui/vguisystemmoduleloader.cpp`
- `../common/xlast_portal2/inc_coop_maps.inc` — first referenced by `game/client/portal2/gameui/portal2/basemodpanel.cpp`
- `../common/xlast_portal2/inc_sp_maps.inc` — first referenced by `game/client/portal2/gameui/portal2/basemodpanel.cpp`
- `../portal2/func_portalled.h` — first referenced by `game/server/portal2/portal/prop_portal.h`
- `CegClientWrapper.h` — first referenced by `game/shared/portal2/portal_placement.h`
- `baseprojectedentity.h` — first referenced by `game/shared/portal2/baseprojectedentity_shared.h`
- `baseprojector.h` — first referenced by `game/server/portal2/portal/portal_base2d.cpp`
- `c_baseprojectedentity.h` — first referenced by `game/client/portal2/portal/c_portal_base2d.cpp`
- `c_baseprojector.h` — first referenced by `game/shared/portal2/baseprojectedentity_shared.cpp`
- `c_combatweaponworldclone.h` — first referenced by `game/client/portal2/portal/c_portal_base2d.cpp`
- `c_community_coop.h` — first referenced by `game/client/portal2/gameui/portal2/basemodpanel.cpp`
- `c_func_noportal_volume.h` — first referenced by `game/shared/portal2/portal_placement.cpp`
- `c_func_portal_bumper.h` — first referenced by `game/shared/portal2/portal_placement.cpp`
- `c_info_placement_helper.h` — first referenced by `game/shared/portal2/weapon_portalgun_shared.cpp`
- `c_keyvalue_saver.h` — first referenced by `game/shared/portal2/portal_mp_gamerules.cpp`
- `c_npc_portal_turret_floor.h` — first referenced by `game/shared/portal2/portal_grabcontroller_shared.cpp`
- `c_paint_sprayer.h` — first referenced by `game/shared/portal2/paint_sprayer_shared.cpp`
- `c_paint_stream.h` — first referenced by `game/shared/portal2/paint_stream_shared.cpp`
- `c_paintblob.h` — first referenced by `game/shared/portal2/paint_blobs_shared.cpp`
- `c_portal_gamestats.h` — first referenced by `game/client/portal2/gameui/portal2/uigamedata_storage.cpp`
- `c_portal_mp_stats.h` — first referenced by `game/shared/portal2/portal_mp_gamerules.cpp`
- `c_portal_playerlocaldata.h` — first referenced by `game/client/portal2/portal/c_portal_player.h`
- `c_projectedwallentity.h` — first referenced by `game/client/portal2/radialmenu.cpp`
- `c_trigger_catapult.h` — first referenced by `game/shared/portal2/portal_player_shared.cpp`
- `c_trigger_paint_cleanser.h` — first referenced by `game/shared/portal2/paint_blobs_shared.cpp`
- `c_trigger_portal_cleanser.h` — first referenced by `game/shared/portal2/portal_placement.cpp`
- `c_trigger_tractorbeam.h` — first referenced by `game/client/portal2/radialmenu.cpp`
- `c_triggers.h` — first referenced by `game/shared/portal2/portal_placement.cpp`
- `c_weapon_paintgun.h` — first referenced by `game/client/portal2/portal/c_portal_player.cpp`
- `cache_hints.h` — first referenced by `game/shared/portal2/paint_blobs_shared.cpp`
- `cegclientwrapper.h` — first referenced by `game/client/portal2/gameui/portal2/uigamedata.cpp`
- `cpaintblob.h` — first referenced by `game/shared/portal2/paint_blobs_shared.cpp`
- `cvisibilitymonitor.h` — first referenced by `game/server/portal2/prop_button.cpp`
- `econ_gcmessages.h` — first referenced by `game/client/portal2/gameui/portal2/basemodpanel.cpp`
- `econ_ui.h` — first referenced by `game/client/portal2/gameui/gameui_interface.cpp`
- `env_portal_laser.h` — first referenced by `game/server/portal2/portal/portal_player.cpp`
- `filesystem/IXboxInstaller.h` — first referenced by `game/client/gameui/basemodpanel.cpp`
- `game_controls/igameuisystemmgr.h` — first referenced by `game/client/gameui/basemodpanel.cpp`
- `game_timescale_shared.h` — first referenced by `game/client/portal2/portal/c_prop_portal.cpp`
- `gc_clientsystem.h` — first referenced by `game/client/portal2/gameui/portal2/basemodpanel.cpp`
- `gc_serversystem.h` — first referenced by `game/server/portal2/portal/portal_player.cpp`
- `hud_locator_target.h` — first referenced by `game/client/portal2/radialmenu.cpp`
- `imaterialproxydict.h` — first referenced by `game/client/portal2/c_prop_floor_button.cpp`
- `info_placement_helper.h` — first referenced by `game/shared/portal2/baseprojectedentity_shared.cpp`
- `matchmaking/imatchframework.h` — first referenced by `game/client/gameui/basemodpanel.h`
- `matchmaking/imatchsystem.h` — first referenced by `game/client/gameui/uigamedata.h`
- `matchmaking/iplayer.h` — first referenced by `game/client/gameui/uigamedata.h`
- `matchmaking/iplayermanager.h` — first referenced by `game/client/gameui/uigamedata.h`
- `matchmaking/iservermanager.h` — first referenced by `game/client/gameui/uigamedata.h`
- `matchmaking/mm_helpers.h` — first referenced by `game/client/gameui/uigamedata.h`
- `matchmaking/portal2/imatchext_portal2.h` — first referenced by `game/client/portal2/gameui/portal2/steamcloudsync.cpp`
- `matchmaking/swarm/imatchext_swarm.h` — first referenced by `game/client/portal2/gameui/engineinterface.h`
- `material_index_data_ops_proxy.h` — first referenced by `game/shared/portal2/prop_paint_power_user.h`
- `npc_portal_turret_floor.h` — first referenced by `game/server/portal2/portal/portal_player.cpp`
- `npc_security_camera.h` — first referenced by `game/server/portal2/portal/portal_player.h`
- `paint_database.h` — first referenced by `game/shared/portal2/paintable_entity.h`
- `paint_sprayer.h` — first referenced by `game/shared/portal2/paint_sprayer_shared.cpp`
- `paint_stream.h` — first referenced by `game/shared/portal2/paint_stream_shared.cpp`
- `paint_swap_guns.h` — first referenced by `game/server/portal2/portal/portal_player.cpp`
- `player_voice_listener.h` — first referenced by `game/shared/portal2/portal_gamerules.cpp`
- `pointsurvey.h` — first referenced by `game/server/portal2/portal/portal_player.cpp`
- `portal2/vgui/portalclientscoreboard.h` — first referenced by `game/client/portal2/portal/clientmode_portal.cpp`
- `portal2/vgui/surveypanel.h` — first referenced by `game/client/portal2/portal/clientmode_portal.cpp`
- `portal2_item_inventory.h` — first referenced by `game/client/portal2/portal/c_portal_player.h`
- `portal2_leaderboard.h` — first referenced by `game/client/portal2/gameui/portal2/vportalleaderboardhud.h`
- `portal2_leaderboard_manager.h` — first referenced by `game/client/portal2/gameui/portal2/vdialoglistbutton.h`
- `portal2_research_data_tracker.h` — first referenced by `game/server/portal2/portal/portal_player.cpp`
- `portal_mp_stats.h` — first referenced by `game/server/portal2/portal/portal_player.cpp`
- `portal_playerlocaldata.h` — first referenced by `game/server/portal2/portal/portal_player.h`
- `portal_ui_controller.h` — first referenced by `game/server/portal2/portal/portal_player.cpp`
- `projectedwallentity.h` — first referenced by `game/shared/portal2/portal_base2d_shared.cpp`
- `prop_monster_box.h` — first referenced by `game/server/portal2/prop_floor_button.cpp`
- `ps3/ps3_core.h` — first referenced by `game/client/gameui/gameui_interface.cpp`
- `ps3/ps3_win32stubs.h` — first referenced by `game/client/gameui/gameui_interface.cpp`
- `ps3/saverestore_ps3_api_ui.h` — first referenced by `game/client/portal2/gameui/engineinterface.h`
- `puzzlemaker/puzzlemaker.h` — first referenced by `game/client/portal2/gameui/portal2/basemodpanel.cpp`
- `radialbutton.h` — first referenced by `game/client/portal2/radialmenu.cpp`
- `radialmenu_taunt.h` — first referenced by `game/client/portal2/radialmenu.cpp`
- `sendprop_priorities.h` — first referenced by `game/server/portal2/portal/portal_player.cpp`
- `sysutil/sysutil_gamecontent.h` — first referenced by `game/client/portal2/gameui/portal2/vattractscreen.cpp`
- `sysutil/sysutil_oskdialog.h` — first referenced by `game/client/portal2/gameui/portal2/vsteamlinkdialog.cpp`
- `sysutil/sysutil_savedata.h` — first referenced by `game/client/portal2/gameui/portal2/vattractscreen.cpp`
- `tier1/tokenset.h` — first referenced by `game/client/gameui/uigamedata.cpp`
- `tier2/resourceprecacher.h` — first referenced by `game/client/portal2/gameui/basesavegamedialog.cpp`
- `tokenset.h` — first referenced by `game/client/gameui/uigamedata.h`
- `trigger_catapult.h` — first referenced by `game/server/portal2/portal/portal_player.cpp`
- `trigger_paint_cleanser.h` — first referenced by `game/shared/portal2/paint_blobs_shared.cpp`
- `trigger_portal_cleanser.h` — first referenced by `game/server/portal2/prop_weightedcube.cpp`
- `trigger_tractorbeam.h` — first referenced by `game/shared/portal2/portal_player_shared.cpp`
- `vgui/portal_leaderboard_graph_panel.h` — first referenced by `game/client/portal2/gameui/portal2/vplaytestdemosdialog.cpp`
- `videocfg/videocfg.h` — first referenced by `game/client/portal2/gameui/portal2/vadvancedvideo.cpp`
- `vscript_server.h` — first referenced by `game/shared/portal2/portal_gamerules.cpp`
- `weapon_paintgun.h` — first referenced by `game/server/portal2/portal/portal_player.cpp`
- `xlast_portal2/inc_coop_maps.inc` — first referenced by `game/client/portal2/gameui/portal2/uigamedata_storage.cpp`
- `xlast_portal2/inc_sp_maps.inc` — first referenced by `game/client/portal2/gameui/portal2/uigamedata_storage.cpp`
