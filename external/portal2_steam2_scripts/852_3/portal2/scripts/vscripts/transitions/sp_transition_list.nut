DBG <- 1
FORCE_GUN_AND_HALLWAY <- 0

FIRST_MAP_WITH_GUN <- "sp_intro_04"
FIRST_MAP_WITH_UPGRADE_GUN <- "sp_laser_redirect_intro"
FIRST_MAP_WITH_POTATO_GUN <- "sp_under_climb"
LAST_PLAYTEST_MAP <- "sp_wheatley_chamber"


CHAPTER_TITLES <- 
[
	{ map = "sp_intro_01", title_text = "CHAPTER 1", subtitle_text = "Wheatley", displayOnSpawn = false },
	{ map = "sp_incinerator_01_BADBAD", title_text = "CHAPTER 2", subtitle_text = "Testing Track", displayOnSpawn = true },
	{ map = "sp_sabotage_darkness", title_text = "CHAPTER 3", subtitle_text = "Jailbreak", displayOnSpawn = true },
]

// Display the chapter title
function DisplayChapterTitle()
{
	foreach (index, level in CHAPTER_TITLES)
	{
		if (level.map == GetMapName() )
		{
			EntFire( "@chapter_title_text", "SetText", level.title_text, 0.0 )
			EntFire( "@chapter_title_text", "display", 1.0 )
			
			EntFire( "@chapter_subtitle_text", "settext", level.subtitle_text, 0.0 )
			EntFire( "@chapter_subtitle_text", "display", 1.0 )
		}
	}
}

// Display the chapter title on spawn if it is flagged to show up on spawn
function TryDisplayChapterTitle()
{
	foreach (index, level in CHAPTER_TITLES)
	{
		if (level.map == GetMapName() && level.displayOnSpawn )
		{
			DisplayChapterTitle()
		}	
	}
}

LOOP_TIMER <- 0

initialized <- false

// This is the order to play the maps
MapPlayOrder<- [

// ===================================================
// ====================== ACT 1 ======================
// ===================================================

// ---------------------------------------------------
// 	Intro
// ---------------------------------------------------
"sp_intro_01",				// motel to box-on-button
//	"@elevator_destructed",
"sp_intro_02",				// portal carousel
//	"@elevator_destructed",
"sp_intro_03",				// fall-through-floor, dioramas, portal gun
//	"@elevator_destructed",
"sp_intro_04",				// box-in-hole for placing on button
//	"@elevator_destructed",
"sp_box_goalie",			// fling hinting
//	"@elevator_destructed",
"sp_fling_training",		// fling training
//	"@elevator_destructed",	
"sp_intro_06",
"sp_glados_01",	// glados 
	"@incinerator",

// ===================================================
// ====================== ACT 2 ======================
// ===================================================

"sp_incinerator_01", 		// upgraded portal gun track
//	"@elevator_destructed",

// ---------------------------------------------------
//	Lasers
// ---------------------------------------------------
"sp_laser_redirect_intro",
//		"@hallway",
"sp_laser_stairs",
//		"@hallway",		
"sp_laser_dual_lasers",
//	"@elevator_destructed",
"sp_laser_over_goo",
//		"@hallway",
"sp_laser_lift_pit_flings",
//		"@hallway",
"sp_fizzler_training",
//	"@elevator_destructed",


// ---------------------------------------------------
// 	Catapult
// ---------------------------------------------------
"sp_catapult_intro",
//	"@hallway",
"sp_trust_fling",
//	"@hallway",
"sp_catapult_fling_sphere_peek",
//"sp_catapult_laser_flings",	
//	"@hallway",
//"sp_catapult_box_smuggle", //sp_hole_in_the_sky",
//	"@hallway",
"sp_catapult_laser_ricochet",


// ---------------------------------------------------
// 	Bridges
// ---------------------------------------------------
"sp_bridge_intro",
"sp_bridge_the_gap",
"sp_laser_power_bridge",

// ---------------------------------------------------
// 	Turrets
// ---------------------------------------------------
"sp_turret_training_advanced",
//	"@hallway",
"sp_turret_blocker_intro",
//	"@hallway",
"sp_laser_vs_turret_intro", // Elevator Glados Chat - Should be removed?
"sp_pull_the_rug",


// ---------------------------------------------------
// 	Laser Redux
// ---------------------------------------------------
"sp_laser_relays",
//	"@hallway",
"sp_ring_around_the_turrets",
//	"@hallway",
"sp_column_blocker",		// Elevator_vista
"sp_laser_chaining",
//	"@hallway",
"sp_turret_tower",
//	"@hallway",
"sp_triple_laser",
//	"@hallway",

// ---------------------------------------------------
// 	Sabotage
// ---------------------------------------------------

"sp_sabotage_jailbreak_01",
"sp_sabotage_jailbreak_02",
"sp_sabotage_darkness",
"sp_sabotage_factory",
"sp_sabotage_panel_sneak",
"sp_sabotage_toxin_destruction",
"sp_sabotage_tube_ride",
//"sp_sabotage_offices",

// ---------------------------------------------------
// 	Glados Chamber Sequence
// ---------------------------------------------------
"sp_sabotage_glados",


// ===================================================
// ====================== ACT 3 ======================
// ===================================================

// ---------------------------------------------------
// 	Underground
// ---------------------------------------------------

	"@bottomless_pit",
"sp_under_bottomless_pit",
	"@hallway",
"sp_under_door",
	"@hallway",
"sp_under_lake",
	"@hallway",
"sp_under_jump_intro",
	"@test_dome_lift",
"sp_under_jump_bomb_flings",
	"@test_dome_lift",
"sp_under_jump_crazy_box",
	"@hallway",
"sp_under_cave",
	"@hallway",
"sp_under_speed_ramp",
	"@test_dome_lift",
"sp_under_speed_flings",
	"@hallway",
"sp_under_portal_intro",
	"@hallway",
"sp_under_climb",

// ===================================================
// ====================== ACT 4 ======================
// ===================================================

// ---------------------------------------------------
// 	Recapture
// ---------------------------------------------------
"sp_return_intro",

// ---------------------------------------------------
// 	Rocket Turrets
// ---------------------------------------------------
// "sp_rocket_turret_intro",
// "sp_rocket_box_trigger",
// "sp_rocket_redirect",

// ---------------------------------------------------
// 	Tractor beam
// ---------------------------------------------------
"sp_tbeam_intro",
"sp_tbeam_trust_drop",	
	"@hallway",
"sp_tbeam_wall_button_retrieve",
	"@hallway",
"sp_tbeam_polarity",
	"@hallway",
"sp_tbeam_catch",	// GRAD

// ---------------------------------------------------
// 	Crushers
// ---------------------------------------------------

// ---------------------------------------------------
// 	Graduation Combos
// ---------------------------------------------------
"sp_stop_the_box",	// Grad?
	"@hallway",
"sp_tbeam_laser_catapult", // Grad
	"@hallway",
//"sp_catapult_course"
//	"@hallway",
//"sp_box_over_goo", // Grad
//	"@hallway",
"sp_laser_bridge_platform",

// ---------------------------------------------------
// 	Bridge Redux
// ---------------------------------------------------

//"sp_turret_islands",
//	"@hallway",


// ---------------------------------------------------
// 	Checkpoint races
// ---------------------------------------------------
//"sp_checkpoint_intro",
//"sp_paint_speed_racer",

// ---------------------------------------------------
// 	Tractor Beam Redux
// ---------------------------------------------------
//"sp_tbeam_cut",
//"sp_tbeam_goo_ride",
//	"@hallway",


// ---------------------------------------------------
// Tbeam + Paint
// ---------------------------------------------------
//"sp_paint_jump_tbeam",
//	"@hallway",
"sp_paint_speed_tb_catch",
	"@hallway",
"sp_paint_jump_tbeam_polarity",	// GRAD
//"sp_paint_portal_tbeams",

// ---------------------------------------------------
// Wheatley Escape
// ---------------------------------------------------

"sp_wheatley_betrayal",
	"@hallway",
"sp_wheatley_bts_1",
	"@hallway",
"sp_escape_paint_speed_tbeam",
	"@hallway",

// ---------------------------------------------------
// 	FIXME: WHEATLEY BATTLE
// ---------------------------------------------------

"sp_wheatley_chamber",

// ===========================================
// === Staging - !!!all new maps here!!!
// ===========================================

// "sp_tbeam_box_smuggle",
// "sp_paint_jump_spiral_jumps",
// "sp_paint_stick_jump_fling",	// GRAD
// "sp_tbeam_turret_towers",
// "sp_suction_turrets",
// "sp_pull_to_fling",
// "sp_catapult_over_tower",
// "sp_catapult_multifling",
]


// --------------------------------------------------------
// OnPostTransition - we just transitioned, teleport us to the correct place.
// --------------------------------------------------------
function OnPostTransition()
{
	local foundMap = false
	
	if( FORCE_GUN_AND_HALLWAY )
	{
		printl( "Forcing hallways" )
		
		EntFire( "entry_door_1", "SetPartner", "@hallway_entry", 0.0 )
		EntFire( "@entry_door", "SetPartner", "@hallway_entry", 0.0 )
		EntFire( "@hallway_entry_teleport", "Teleport", 0, 0 )
				
		return
	}
	
	foreach (index, map in MapPlayOrder)
	{	
		if (GetMapName() == MapPlayOrder[index])
		{
			foundMap = true
			
			// hook up our entry elevator
			if( index - 1 >= 0 )
			{
				if( MapPlayOrder[index-1].find("@") == null )
				{
					printl( "Teleporting to default start pos" )
					EntFire( "entry_door_1", "SetPartner", "@elevator_entry", 0.0 )
					EntFire( "@entry_door", "SetPartner", "@elevator_entry", 0.0 ) // also fire @entry_door
					EntFire( "@elevator_entry_teleport", "Teleport", 0, 0 )		
				}
				else
				{
					printl( "Trying to teleport to " + MapPlayOrder[index - 1] + "_teleport" )
					EntFire( "entry_door_1", "SetPartner", MapPlayOrder[index - 1] + "_entry", 0.0 )
					EntFire( "@entry_door", "SetPartner", MapPlayOrder[index - 1] + "_entry", 0.0 ) // also fire @entry_door
					EntFire( MapPlayOrder[index - 1] + "_entry_teleport", "Teleport", 0, 0 )			
				}
			}
			break
		}
	}
		
	if (foundMap == false )
	{
		EntFire( "entry_door_1", "SetPartner", "@elevator_entry", 0.0 )
		EntFire( "@entry_door", "SetPartner", "@elevator_entry", 0.0 ) // also fire @entry_door
		EntFire( "@elevator_entry_teleport", "Teleport", 0, 0 )
	}
}

// --------------------------------------------------------
// EntFire_MapLoopHelper
// --------------------------------------------------------
function EntFire_MapLoopHelper( classname, suffix, command, param, delay )
{
	// This calls EntFire on an entity of a given type, named with the given suffix.
	// This deals with instance name mangling (though it doesn't guarantee uniqueness)
	local suffix_len = suffix.len()
	for ( local ent = Entities.FindByClassname( null, classname ); ent != null; ent = Entities.FindByClassname( ent, classname ) )
	{
		local ent_name = ent.GetName()
		local suffix_offset = ent_name.find( suffix )
		if ( ( suffix_offset != null ) && ( suffix_offset == ( ent_name.len() - suffix_len ) ) )
		{
			EntFire( ent_name, command, param, delay )
			return
		}
	}
	printl( "MAPLOOP: ---- ERROR! Failed to find entity " + ent_suffix + " while initiating map transition" );
}

// --------------------------------------------------------
// Think
// --------------------------------------------------------
function Think()
{	
	// Start the game loop if the cvar is set
	if ( LoopSinglePlayerMaps() )
	{
		// initialize the timer
		if( LOOP_TIMER == 0 )
		{
			LOOP_TIMER = Time() + 4 // restart time in seconds
// DONOTCHECKIN:			LOOP_TIMER = Time() + 10 // restart time in seconds
		}
		
		// transition to the next map if the timer has expired
		if ( LOOP_TIMER < Time() )
		{
			// reset loop timer
			LOOP_TIMER = 0

			printl( "\nMAPLOOP: timer expired, moving on..." )

			// Ensure point_viewcontrollers are disabled
			EntFire( "point_viewcontrol", "disable", 0, 0 )
		
			// Change the level (this sequence was originally in the 'transition_without_survey' logic_relay)
			EntFire_MapLoopHelper( "trigger_once",   "survey_trigger",    "Disable",       "",                    0.0 )
			EntFire_MapLoopHelper( "env_fade",       "exit_fade",         "Fade",          "",                    0.0 )
			EntFire_MapLoopHelper( "point_teleport", "exit_teleport",     "Teleport",      "",                    0.3 )
			EntFire_MapLoopHelper( "point_teleport", "@exit_teleport",    "Teleport",      "",                    0.3 )
			EntFire_MapLoopHelper( "logic_script",   "transition_script", "RunScriptCode", "TransitionFromMap()", 0.4 )
		}
	}
	
	
	if (initialized)
	{
		return
	}
	initialized = true;
	
	local portalGunCommand = ""
	local portalGunSecondCommand = ""
	local foundMap = false
	
	if( FORCE_GUN_AND_HALLWAY )
	{
		printl( "Forcing hallways" )
		
		EntFire( "entry_door_1", "SetPartner", "@hallway_entry", 0.0 )
		EntFire( "@entry_door", "SetPartner", "@hallway_entry", 0.0 ) // also fire @entry_door
		
		EntFire( "exit_door_1", "SetPartner", "@hallway_exit", 0.0 )
		EntFire( "@exit_door", "SetPartner", "@hallway_exit", 0.0 ) // also fire @exit_door
		DumpMapList()
		TryDisplayChapterTitle()
		
		portalGunCommand = "give weapon_portalgun"
		portalGunSecondCommand = "upgrade_portalgun"
		
		printl( "=======================Trying to run " + portalGunCommand )
		EntFire( "command", "Command", portalGunCommand, 0.0 )
		EntFire( "@command", "Command", portalGunCommand, 0.0 )
		
		printl( "=======================Trying to run " + portalGunSecondCommand )
		EntFire( "command", "Command", portalGunSecondCommand, 0.1 )
		EntFire( "@command", "Command", portalGunSecondCommand, 0.1 )

		return
	}
	
	foreach (index, map in MapPlayOrder)
	{
		if (MapPlayOrder[index] == FIRST_MAP_WITH_GUN)
		{
			portalGunCommand = "give weapon_portalgun"
		}		
		else if (MapPlayOrder[index] == FIRST_MAP_WITH_UPGRADE_GUN)
		{
			portalGunSecondCommand = "upgrade_portalgun"
		}
		else if (MapPlayOrder[index] == FIRST_MAP_WITH_POTATO_GUN)
		{
			portalGunSecondCommand = "upgrade_potatogun"
		}
		
		if (GetMapName() == MapPlayOrder[index])
		{
			foundMap = true
			
			printl("================Linking entry and exit")
			// hook up our entry elevator
			if( index - 1 >= 0 )
			{
				
				if( MapPlayOrder[index-1].find("@") == null )
				{
					printl( "USING ENTRY DEFAULTS" )
					EntFire( "entry_door_1", "SetPartner", "@elevator_entry", 0.0 )
					EntFire( "@entry_door", "SetPartner", "@elevator_entry", 0.0 ) // also fire @entry_door
				}
				else
				{
					printl( "Linking entry door to " + MapPlayOrder[index - 1] )
					EntFire( "entry_door_1", "SetPartner", MapPlayOrder[index - 1] + "_entry", 0.0 )
					EntFire( "@entry_door", "SetPartner", MapPlayOrder[index - 1] + "_entry", 0.0 ) // also fire @entry_door
				}
			}
			
			// hook up the exit elevator
			if( index + 1 < MapPlayOrder.len() )
			{
				if( MapPlayOrder[index+1].find("@") == null )
				{
					printl( "USING EXIT DEFAULT" )
					EntFire( "exit_door_1", "SetPartner", "@elevator_exit", 0.0 )				
					EntFire( "@exit_door", "SetPartner", "@elevator_exit", 0.0 ) // also fire @exit_door
				}
				else
				{
					printl( "Linking exit door to " + MapPlayOrder[index + 1] )
					EntFire( "exit_door_1", "SetPartner", MapPlayOrder[index + 1] + "_exit", 0.0 )
					EntFire( "@exit_door", "SetPartner", MapPlayOrder[index + 1] + "_exit", 0.0 )	// also fire @exit_door
				}
			}
			else
			{
				printl( "USING EXIT DEFAULT" )
				EntFire( "exit_door_1", "SetPartner", "@elevator_exit", 0.0 )
				EntFire( "@exit_door", "SetPartner", "@elevator_exit", 0.0 ) // also fire @exit_door
			}
			break
		}
	}
		
	if (foundMap == false )
	{
		printl("================Map not in main track")
		
		printl( "USING ENTRY DEFAULTS" )
		EntFire( "entry_door_1", "SetPartner", "@elevator_entry", 0.0 )
		EntFire( "@entry_door", "SetPartner", "@elevator_entry", 0.0 ) // also fire @entry_door
		
		printl( "USING EXIT DEFAULT" )
		EntFire( "exit_door_1", "SetPartner", "@elevator_exit", 0.0 )
		EntFire( "@exit_door", "SetPartner", "@elevator_exit", 0.0 ) // also fire @exit_door
	}

	DumpMapList()
	TryDisplayChapterTitle()
	
	if (portalGunCommand != "")
	{
		printl( "=======================Trying to run " + portalGunCommand )
		EntFire( "command", "Command", portalGunCommand, 0.0 )
		EntFire( "@command", "Command", portalGunCommand, 0.0 )
	}

	if (portalGunSecondCommand != "")
	{
		printl( "=======================Trying to run " + portalGunSecondCommand )
		EntFire( "command", "Command", portalGunSecondCommand, 0.1 )
		EntFire( "@command", "Command", portalGunSecondCommand, 0.1 )
	}
}

// --------------------------------------------------------
// TransitionFromMap
// --------------------------------------------------------
function DumpMapList()
{
	if(DBG)
	{
		local mapcount = 0
		
		printl("================DUMPING MAP PLAY ORDER")
		
		foreach( index, map in MapPlayOrder )
		{
			// weed out our transitions
			if( MapPlayOrder[index].find("@") == null )
			{
				if( GetMapName() == MapPlayOrder[index] )
				{
					printl( mapcount + " " + MapPlayOrder[index] + " <--- You Are Here" )
				}
				else
				{
					printl( mapcount + " " + MapPlayOrder[index] )
				}
				mapcount++
			}
			
		}
		printl( mapcount + " maps total." )
		
		printl("================END DUMP")
	}
}

// --------------------------------------------------------
// TransitionFromMap
// --------------------------------------------------------
function TransitionFromMap()
{
	local next_map = null
	foreach( index, map in MapPlayOrder )
	{
		if( GetMapName() == MapPlayOrder[index] )
		{
			// make good
			local skipIndex = index
			for(local i=0;i<2;i+=1)
			{
				if( skipIndex + 1 < MapPlayOrder.len() )
				{
					if( MapPlayOrder[skipIndex + 1].find("@") != null )
					{
						skipIndex++
					}
					else
					{
						break
					}
				}
			}		
			
			if( ( skipIndex + 1 < MapPlayOrder.len() ) &&
			    ( GetMapName() != LAST_PLAYTEST_MAP  )    )
			{
				next_map = MapPlayOrder[ skipIndex + 1 ]
				if(DBG) printl( "Map " + GetMapName() + " connects to " + next_map )

				if ( Entities.FindByName( null, "@changelevel" ) == null )
				{
					if(DBG) printl( "('@changelevel' entity missing, using 'map' command instead)" )
					SendToConsole( "map " + next_map );
				}
				else
				{
					EntFire( "@changelevel", "Changelevel", next_map, 0.0 )			
				}
			}
		}
	}
	
	if ( next_map == null )
	{
		if(DBG) printl( "Map " + GetMapName() + " is the last map" )
		EntFire( "end_of_playtest_text", "display", 0 )
		EntFire( "@end_of_playtest_text", "display", 0 )

		// If we are in the map loop and at the end of the list, start over at the beginning
		if ( LoopSinglePlayerMaps() )
		{
			printl( "MAPLOOP: No more maps, restarting loop." )
			next_map = MapPlayOrder[0]
			if ( Entities.FindByName( null, "@changelevel" ) == null )
			{
				SendToConsole( "map " + next_map );
			}
			else
			{
				EntFire( "@changelevel", "Changelevel", next_map, 0.0 )			
			}
		}
	}

	printl( "" )
}



// --------------------------------------------------------
// MakeBatFile - dumps the map list in a formatted way, for easy recompilin'
// --------------------------------------------------------
function MakeBatFile()
{
		local mapcount = 0
		
		printl("================DUMPING maps formatted for batch file")
		
		foreach( index, map in MapPlayOrder )
		{
			printl( "call build " + MapPlayOrder[index] )	
		}
		
		foreach( index, map in MapPlayOrder )
		{
			printl( "call p2_buildcubemaps " + MapPlayOrder[index] )	
		}
}
