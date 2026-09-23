DBG <- 1
FORCE_GUN_AND_HALLWAY <- 0

FIRST_MAP_WITH_GUN <- "sp_a1_intro4"
FIRST_MAP_WITH_UPGRADE_GUN <- "sp_a2_laser_intro"
FIRST_MAP_WITH_POTATO_GUN <- "sp_a3_speed_ramp"
LAST_PLAYTEST_MAP <- "sp_a4_finale4"


CHAPTER_TITLES <- 
[
	{ map = "sp_a1_intro1", title_text = "CHAPTER 1", subtitle_text = "Wheatley", displayOnSpawn = false },
	{ map = "sp_a2_intro_BADBAD", title_text = "CHAPTER 2", subtitle_text = "Testing Track", displayOnSpawn = true },
	{ map = "sp_a2_bts3", title_text = "CHAPTER 3", subtitle_text = "Jailbreak", displayOnSpawn = true },
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
"sp_a1_intro1",				// motel to box-on-button
//	"@elevator_destructed",
"sp_a1_intro2",				// portal carousel
//	"@elevator_destructed",
"sp_a1_intro3",				// fall-through-floor, dioramas, portal gun
//	"@elevator_destructed",
"sp_a1_intro4",				// box-in-hole for placing on button
//	"@elevator_destructed",
"sp_a1_intro5",			// fling hinting
//	"@elevator_destructed",
"sp_a1_intro6",		// fling training
//	"@elevator_destructed",	
"sp_a1_intro7",
"sp_a1_wakeup",	// glados 
	"@incinerator",

// ===================================================
// ====================== ACT 2 ======================
// ===================================================

"sp_a2_intro", 		// upgraded portal gun track
//	"@elevator_destructed",

// ---------------------------------------------------
//	Lasers
// ---------------------------------------------------
"sp_a2_laser_intro",
//		"@hallway",
"sp_a2_laser_stairs",
//		"@hallway",		
"sp_a2_dual_lasers",
//	"@elevator_destructed",
"sp_a2_laser_over_goo",
//		"@hallway",

// ---------------------------------------------------
// 	Catapult
// ---------------------------------------------------
"sp_a2_catapult_intro",
//	"@hallway",
"sp_a2_trust_fling",
//	"@hallway",

// ---------------------------------------------------
//	More Lasers
// ---------------------------------------------------
"sp_a2_pit_flings",
//		"@hallway",
"sp_a2_fizzler_intro",
//	"@elevator_destructed",


// ---------------------------------------------------
//	Lasers + Catapult
// ---------------------------------------------------
"sp_a2_sphere_peek",
//"sp_catapult_laser_flings",	
//	"@hallway",
//"sp_catapult_box_smuggle", //sp_hole_in_the_sky",
//	"@hallway",
"sp_a2_ricochet",


// ---------------------------------------------------
// 	Bridges
// ---------------------------------------------------
"sp_a2_bridge_intro",
"sp_a2_bridge_the_gap",
//"sp_laser_power_bridge",

// ---------------------------------------------------
// 	Turrets
// ---------------------------------------------------
"sp_a2_turret_intro",
//	"@hallway",

"sp_a2_laser_relays", // breather
//	"@hallway",

"sp_a2_turret_blocker",
//	"@hallway",
"sp_a2_laser_vs_turret", // Elevator Glados Chat - Should be removed?
"sp_a2_pull_the_rug",


// ---------------------------------------------------
// 	Laser Redux
// ---------------------------------------------------
// "sp_a2_ring_around_turrets",
//	"@hallway",
"sp_a2_column_blocker",		// Elevator_vista
"sp_a2_laser_chaining",
//	"@hallway",
"sp_a2_turret_tower",
//	"@hallway",
"sp_a2_triple_laser",
//	"@hallway",

// ---------------------------------------------------
// 	Sabotage
// ---------------------------------------------------

"sp_a2_bts1",
"sp_a2_bts2",
"sp_a2_bts3",
"sp_a2_bts4",
"sp_a2_bts5",
"sp_a2_bts6",
//"sp_sabotage_offices",

// ---------------------------------------------------
// 	Glados Chamber Sequence
// ---------------------------------------------------
"sp_a2_core",


// ===================================================
// ====================== ACT 3 ======================
// ===================================================

// ---------------------------------------------------
// 	Underground
// ---------------------------------------------------

	"@bottomless_pit",
"sp_a3_01",
	"@hallway",
"sp_a3_02",
	"@hallway",
"sp_a3_03",
	"@test_dome_lift",
"sp_a3_jump_intro",
	"@test_dome_lift",
"sp_a3_bomb_flings",
	"@test_dome_lift",
"sp_a3_crazy_box",
	"@test_dome_lift",
"sp_a3_transition01",
	"@test_dome_lift",
"sp_a3_speed_ramp",
	"@test_dome_lift",
"sp_a3_speed_flings",
	"@test_dome_lift",
"sp_a3_portal_intro",
	"@hallway",
"sp_a3_end",

// ===================================================
// ====================== ACT 4 ======================
// ===================================================

// ---------------------------------------------------
// 	Recapture
// ---------------------------------------------------
"sp_a4_intro",

// ---------------------------------------------------
// 	Rocket Turrets
// ---------------------------------------------------
// "sp_rocket_turret_intro",
// "sp_rocket_box_trigger",
// "sp_rocket_redirect",

// ---------------------------------------------------
// 	Tractor beam
// ---------------------------------------------------
"sp_a4_tb_intro",
"sp_a4_tb_trust_drop",	
//	"@hallway",
"sp_a4_tb_wall_button",
//	"@hallway",
"sp_a4_tb_polarity",
//	"@hallway",
"sp_a4_tb_catch",	// GRAD

// ---------------------------------------------------
// 	Crushers
// ---------------------------------------------------

// ---------------------------------------------------
// 	Graduation Combos
// ---------------------------------------------------
"sp_a4_stop_the_box",	// Grad?
//	"@hallway",
"sp_a4_laser_catapult", // Grad
//	"@hallway",
//"sp_catapult_course"
//	"@hallway",
//"sp_box_over_goo", // Grad
//	"@hallway",
"sp_a4_laser_platform",

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
"sp_a4_speed_tb_catch",
//	"@hallway",
"sp_a4_jump_polarity",	// GRAD
//	"@hallway",
//"sp_paint_portal_tbeams",

// ---------------------------------------------------
// Wheatley Escape
// ---------------------------------------------------

"sp_a4_finale1",
	"@hallway",
"sp_a4_finale2",
	"@hallway",
"sp_a4_finale3",
	"@hallway",

// ---------------------------------------------------
// 	FIXME: WHEATLEY BATTLE
// ---------------------------------------------------

"sp_a4_finale4",

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
	printl( "MAPLOOP: ---- ERROR! Failed to find entity " + suffix + " while initiating map transition" );
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
			LOOP_TIMER = Time() + 10 // restart time in seconds
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

// ===================================================
// ===================================================
function OnPostSpawn()
{
	// Wire up Glados choreo output to the @elevator_entry door
	EntFire( "@elevator_entry", "AddOutput", "OnEntityTeleportFromMe @glados:RunScriptCode:LeftElevatorChamber()", 0, 0 )
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
// Attaches an output to the map @elevator_entry door that triggers
// when the player walks through
// --------------------------------------------------------
function LeftElevatorChamber()
{
	EntFire( "@elevator_entry", "AddOutput", "OnEntityTeleportFromMe:@glados:RunScriptCode:LeftElevatorChamber()", 0, 0 )
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

// this lets the elevator know that we are ready to transition.
function TransitionReady()
{
	::TransitionReady <- 1
}