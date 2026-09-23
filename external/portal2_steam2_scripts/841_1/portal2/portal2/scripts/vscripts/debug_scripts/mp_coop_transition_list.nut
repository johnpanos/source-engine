DBG <- 1

HUB_MAP <- "mp_coop_lobby_2"

DAY_ONE_START_MAP <- "mp_coop_doors"
DAY_ONE_END_MAP <- "mp_coop_separation_1"
DAY_ONE_LENGTH <- 1

DAY_TWO_START_MAP <- "mp_coop_fling_3"
DAY_TWO_END_MAP <- "mp_coop_fan"
DAY_TWO_LENGTH <- 1

DAY_THREE_START_MAP <- "mp_coop_wall_intro"
DAY_THREE_END_MAP <- "mp_coop_wall_5"
DAY_THREE_LENGTH <- 1

DAY_FOUR_START_MAP <- "mp_coop_tbeam_fling_float_1"
DAY_FOUR_END_MAP <- "mp_coop_tbeam_polarity3"
DAY_FOUR_LENGTH <- 1

DAY_FIVE_START_MAP <- "mp_coop_paint_come_along"
DAY_FIVE_END_MAP <- "mp_coop_paint_speed_catch"
DAY_FIVE_LENGTH <- 1

DAY_SIX_START_MAP <- "mp_coop_rocket_block"
DAY_SIX_END_MAP <- "mp_coop_race_3"
DAY_SIX_LENGTH <- 1

MapPlayOrder<- [
// this is the order to play the multiplayer maps

//"mp_coop_mic_calibration",

// INTRO
"mp_coop_start",
"mp_coop_start_02",
"mp_coop_start_03",
"mp_coop_lobby_2",

// DAY 1 Start
//----TEAMBUILDING----
"mp_coop_doors",
"mp_coop_race_2",
"mp_coop_laser_2",
"mp_coop_rat_maze",
"mp_coop_laser_crusher",
"mp_coop_separation_1", // - make behind the scenes

// DAY 2 Start
//----FLINGING----
// "Mass and Velocity"
"mp_coop_fling_3",
"mp_coop_infinifling_train",
"mp_coop_come_along",
"mp_coop_fling_1",
"mp_coop_catapult_1",
"mp_coop_multifling_1",
"mp_coop_fling_crushers",
"mp_coop_fan",

// DAY 3 Start
//----LIGHTBRIDGES----
// "Hard-Light Surfaces"
"mp_coop_wall_intro",
"mp_coop_wall_2",
"mp_coop_catapult_wall_intro",
"mp_coop_wall_block",
"mp_coop_catapult_2",
// turret/wall intro
"mp_coop_turret_walls",
// fun, should make into a full map!
//"mp_coop_turret_ball",
"mp_coop_wall_5",

// DAY 4 Start
//----TBEAM----
// "Excursion Funnels"
"mp_coop_tbeam_fling_float_1",
"mp_coop_tbeam_redirect",
"mp_coop_tbeam_drill",
"mp_coop_tbeam_catch_grind_1",
"mp_coop_tbeam_laser_1",
"mp_coop_tbeam_polarity",
"mp_coop_tbeam_polarity2",
"mp_coop_tbeam_polarity3",

// DAY 5 Start
//----PAINT----
// "Mobility Gels"
"mp_coop_paint_come_along",
"mp_coop_paint_bridge_intro",
"mp_coop_paint_bridge",
// broken, needs minor work, perhaps make harder?
"mp_coop_paint_redirect",
"mp_coop_paint_speed_fling",
"mp_coop_paint_red_racer",
// can save as paint, remove paint fizzler
"mp_coop_paint_speed_catch",
// Need tbeams and paint map

// DAY 6 Start
//----ADVANCED----
// fun rocket map with turret blockers mechanics - need 2 player, fun
"mp_coop_rocket_block",
// bridges, infinifling, timed buttons, sentries  - add something, make advanced?
"mp_coop_race_3",

// ===========================================
// === THESE MAPS ARE IN LIMBO (good to ship, but no home yet)
// ===========================================
//----TEAMBUILDING----

//----FLING----
//----PLATES----
//----BRIDGES----
// maybe needs redesign, has bridges.  also has plate that only affects boxes - add something, make advanced?
"mp_coop_laser_1",

//----TBEAM----
// too similar to wall_2- could turn into tbeam map?  training?
"mp_coop_wall_1",
//----PAINT----
// try to save the walls jumps
"mp_coop_2guns_walljumps",
// might make work without paintgun
"mp_coop_2guns_longjump_intro",
// might make work without paintgun


// ===========================================
// === THESE MAPS ARE ON THE CUTTING BLOCK
// ===========================================
/// NEEDS WORK

// broken in unknown way ATM
"mp_coop_paint_crazy_box",
// its the double box collide map, not an interesting puzzle as is, would need some work
"mp_coop_wall_6",
// broken?  also goal not clear, would need testing - bridges 
"mp_coop_button_tower",

// ===========================================
// === THESE MAPS HAVE BEEN CUT
// ===========================================
// same as mp_coop_wall_block
"mp_coop_wall_stop",
// too much work
"mp_coop_trap_1",
// turned into tbeam map
"mp_coop_wall_catch",
// too much work
"mp_coop_rat_maze_2",
// hard to make work
"mp_coop_double_catch",
// cut this sucker!  paint fizzler is dead
"mp_coop_paint_fizzler_intro",
// can go in training track or cut, CUT!
"mp_coop_vertical_crushers",
// invents new lasers, can't afford to train
"mp_coop_crusher",
// not super interesting, suggest cut
"mp_coop_paint_bridge_02",
// another rat maze, could be cool need art
"mp_coop_vista", //**

// ===========================================
// === Paint Gun maps go here (paint gun is cut)
// ===========================================
"mp_coop_2guns_intro",
"mp_coop_2guns_swap",
"mp_coop_2guns_flingstick",
"mp_coop_2guns_bridge_jump",
"mp_coop_2guns_bridges",
"mp_coop_2guns_bridge_stick",
"mp_coop_2guns_longjumps",
"mp_coop_2guns_bridges2",

]

// --------------------------------------------------------
// OnPostSpawn
// --------------------------------------------------------
function OnPostSpawn()
{
	local nBranch = 0
	local daymapcount = 0
	local mapcount = 0
	
	if ( DBG )
	{
		printl("================DUMPING MAP PLAY ORDER")
	}
	
	AddBranchLevelName( 0, "CLEAR ALL" )
	
	foreach( index, map in MapPlayOrder )
	{	
		if( MapPlayOrder[index] == DAY_ONE_START_MAP )
		{
			nBranch++
			daymapcount = 0
			
			if ( DBG )
			{
				printl( "=== Start of Track 1" )
			}
		}
		else if( MapPlayOrder[index] == DAY_TWO_START_MAP )
		{
			nBranch++
			DAY_ONE_LENGTH = daymapcount
			daymapcount = 0
			
			if ( DBG )
			{
				printl( "< " + DAY_ONE_LENGTH + " levels in Track 1 >" )
				printl( "=== Start of Track 2" )
			}
		}
		else if( MapPlayOrder[index] == DAY_THREE_START_MAP )
		{
			nBranch++
			DAY_TWO_LENGTH = daymapcount
			daymapcount = 0
			
			if ( DBG )
			{
				printl( "< " + DAY_TWO_LENGTH + " levels in Track 2 >" )
				printl( "=== Start of Track 3" )
			}
		}
		else if( MapPlayOrder[index] == DAY_FOUR_START_MAP )
		{
			nBranch++
			DAY_THREE_LENGTH = daymapcount
			daymapcount = 0
			
			if ( DBG )
			{
				printl( "< " + DAY_THREE_LENGTH + " levels in Track 3 >" )
				printl( "=== Start of Track 4" )
			}
		}
		else if( MapPlayOrder[index] == DAY_FIVE_START_MAP )
		{
			nBranch++
			DAY_FOUR_LENGTH = daymapcount
			daymapcount = 0
			
			if ( DBG )
			{
				printl( "< " + DAY_FOUR_LENGTH + " levels in Track 4 >" )
				printl( "=== Start of Track 5" )
			}
		}
		else if( MapPlayOrder[index] == DAY_SIX_START_MAP )
		{
			nBranch++
			DAY_FIVE_LENGTH = daymapcount
			daymapcount = 0
			
			if ( DBG )
			{
				printl( "< " + DAY_FIVE_LENGTH + " levels in Track 5 >" )
				printl( "=== Start of Track 6" )
			}
		}
	
		if ( DBG )
		{
			if( GetMapName() == MapPlayOrder[index] )
			{
				printl( index + " " + MapPlayOrder[index] + " <--- You Are Here" )
			}
			else
			{
				printl( index + " " + MapPlayOrder[index] )
			}
		}
					
		AddBranchLevelName( nBranch, MapPlayOrder[index] )
		
		if( MapPlayOrder[index] == DAY_SIX_END_MAP )
		{
			nBranch++
			DAY_SIX_LENGTH = daymapcount+1
			daymapcount = 0
			
			if ( DBG )
			{
				printl( "< " + DAY_SIX_LENGTH + " levels in Track 6 >" )
				printl( "=== PLEASE DO NOT TEST ANY MAP BELOW THIS LINE ===" )
			}
		}
		
		mapcount++
		daymapcount++	
	}
	
	if ( DBG )
	{
		printl( mapcount + " maps total." )
	
		printl("================END DUMP")
	}
}

function MapPostLoaded()
{
	local nBranch = 0
	local daymapcount = 1

	foreach( index, map in MapPlayOrder )
	{
		local bResetDayMapCount = false
		printl( "= Branch: " + nBranch + ", Level: " + daymapcount )
		
		if( MapPlayOrder[index] == DAY_ONE_START_MAP )
		{
			nBranch++
			bResetDayMapCount = true
		}
		else if( MapPlayOrder[index] == DAY_TWO_START_MAP )
		{
			nBranch++
			bResetDayMapCount = true
		}
		else if( MapPlayOrder[index] == DAY_THREE_START_MAP )
		{
			nBranch++
			bResetDayMapCount = true
		}
		else if( MapPlayOrder[index] == DAY_FOUR_START_MAP )
		{
			nBranch++
			bResetDayMapCount = true
		}
		else if( MapPlayOrder[index] == DAY_FIVE_START_MAP )
		{
			nBranch++
			bResetDayMapCount = true
		}
		else if( MapPlayOrder[index] == DAY_SIX_START_MAP )
		{
			nBranch++
			bResetDayMapCount = true
		}
		
		// reset the daymap count
		if ( bResetDayMapCount == true )
		{
			daymapcount = 1
		}
		
		if( GetMapName() == MapPlayOrder[index] )
		{
			EntFire( "@command", "command", "lobby_select_day " + nBranch, 0 )
			EntFire( "@command", "Command", "coop_lobby_select_level " + nBranch + " " + daymapcount + " 1", 0.1)	
			printl( "=== This is TRACK #" + nBranch + ", level " + daymapcount )
			return
		}
				
		daymapcount++
	}
}

// --------------------------------------------------------
// TransitionFromMap
// TAUNTS - 	highFive and smallWave already unlocked at the start in code
//		robotDance and thumbsUp already unlocked in mp_coop_lobby.nut file
// --------------------------------------------------------
function TransitionFromMap()
{
	MarkMapComplete( GetMapName() )
		
	if( GetMapName() == HUB_MAP )
	{
		TransitionToSection()
	}
	else if( GetMapName() == DAY_ONE_END_MAP )
	{
		EntFire( "@command", "command", "mp_unlock_taunt teamhug", 0.0 )
		EntFire( "@command", "command", "mp_unlock_taunt teamtease", 0.1 )
		EntFire( "@command", "command", "changelevel " + HUB_MAP, 1.3 )
	}
	else if( GetMapName() == DAY_TWO_END_MAP )
	{
		EntFire( "@command", "command", "mp_unlock_taunt laugh", 0.0 )
		EntFire( "@command", "command", "mp_unlock_taunt facepalm", 0.1 )
		EntFire( "@command", "command", "changelevel " + HUB_MAP, 1.3 )
	}
	else if( GetMapName() == DAY_THREE_END_MAP )
	{
		EntFire( "@command", "command", "mp_unlock_taunt trickfire", 0.0 )	
		EntFire( "@command", "command", "mp_unlock_taunt thumbsUp", 0.1 )
		EntFire( "@command", "command", "changelevel " + HUB_MAP, 1.3 )
	}
	else if( GetMapName() == DAY_FOUR_END_MAP )
	{
		//EntFire( "@command", "command", "mp_unlock_taunt RockPaperScissors", 0.0 )
		EntFire( "@command", "command", "changelevel " + HUB_MAP, 1.3 )
	}
	else if( GetMapName() == DAY_FIVE_END_MAP )
	{
		//EntFire( "@command", "command", "mp_unlock_taunt RockPaperScissors", 0.0 )
		EntFire( "@command", "command", "changelevel " + HUB_MAP, 1.3 )
	}
	else if( GetMapName() == DAY_SIX_END_MAP )
	{
		//EntFire( "@command", "command", "mp_unlock_taunt RockPaperScissors", 0.0 )
		EntFire( "@command", "command", "changelevel " + HUB_MAP, 1.3 )
	}
	else
	{	
		local bInBranches = 0
		foreach( index, map in MapPlayOrder )
		{	
			if( GetMapName() == MapPlayOrder[index] )
			{				
				if( index + 1 >= MapPlayOrder.len() )
				{
					if(DBG) printl( "Map " + GetMapName() + " is the last map" )
					EntFire( "end_of_playtest_text", "display", 0 )
					EntFire( "@command", "command", "disconnect", 2.6 )
				}
				else
				{
					if(DBG) printl( "Map " + GetMapName() + " connects to " + MapPlayOrder[ index + 1 ] )
					EntFire( "@command", "command", "changelevel " + MapPlayOrder[index + 1], 1.3 )
				}
				return
			}
		}
	}
}

// --------------------------------------------------------
// TransitionToLevelFromHub
// --------------------------------------------------------
function TransitionToLevelFromHub( nBranch )
{
	EntFire( "@command", "command", "lobby_select_day " + nBranch, 0 )
	
	local nStartLevel = 0	
	foreach( index, map in MapPlayOrder )
	{
		if( GetMapName() == MapPlayOrder[index] )
		{
			break
		}
		nStartLevel++
	}
	
	if ( nBranch > 1 )
	{
		nStartLevel = nStartLevel + DAY_ONE_LENGTH
	}
	
	if ( nBranch > 2 )
	{
		nStartLevel = nStartLevel + DAY_TWO_LENGTH
	}

	if ( nBranch > 3 )
	{
		nStartLevel = nStartLevel + DAY_THREE_LENGTH
	}

	if ( nBranch > 4 )
	{
		nStartLevel = nStartLevel + DAY_FOUR_LENGTH
	}

	if ( nBranch > 5 )
	{
		nStartLevel = nStartLevel + DAY_FIVE_LENGTH
	}
	
	if ( nBranch > 6 )
	{
		printl( "=== Branch is out of range!!!!!!!!!" )
		return
	}

	local nCurrentLevel = GetCoopBranchLevelIndex( nBranch )
	local index = nCurrentLevel + nStartLevel
	printl( "=== Branch = " + nBranch + ", named " + MapPlayOrder[index] )
	printl( "=== Going to Level #" + index + ", named " + MapPlayOrder[index] )
	EntFire( "@command", "command", "changelevel " + MapPlayOrder[index], 0.5 )
}


// --------------------------------------------------------
// ReturnToHubFromLevel
// --------------------------------------------------------
function ReturnToHubFromLevel()
{
	printl( "=== RETURNING TO HUB" )
	EntFire( "@command", "command", "changelevel " + HUB_MAP, 0.2 )
}

function CheckDayLights()
{	
	local section = GetCoopSectionIndex()
	
	if( section == 1 )
	{
		EntFire( "light_day1", "TurnOn", "", 0.0 )
	}
	else
	{
		EntFire( "light_day1", "TurnOff", "", 0.0 )	
	}
	
	if( section == 2 ) 
	{
		EntFire( "light_day2", "TurnOn", "", 0.0 )
	}
	else
	{
		EntFire( "light_day2", "TurnOff", "", 0.0 )	
	}
	
	if( section == 3 )
	{
		EntFire( "light_day3", "TurnOn", "", 0.0 )
	}
	else
	{
		EntFire( "light_day3", "TurnOff", "", 0.0 )	
	}
	
	if( section != 0 )
	{
		EntFire( "@exit_door_1", "Open", "", 0.0 )
	}
}

function TransitionToSection()
{	
	local section = GetCoopSectionIndex()
	
	printl("Transitioning " + section )
	
	if( section == 0 )
	{
		EntFire( "@command", "command", "changelevel mp_coop_lobby_2", 1.3 )
	}
	else if( section == 1 )
	{
		TransitionToDayOne()
	}
	else if( section == 2 )
	{
		TransitionToDayTwo()
	}
	else if( section == 3 )
	{
		TransitionToDayThree()
	}	
	else if( section == 4 )
	{
		TransitionToDayFour()
	}	
	else if( section == 5 )
	{
		TransitionToDayFive()
	}	
	else if( section == 6 )
	{
		TransitionToDaySix()
	}	
}


function TransitionToDayOne()
{
	EntFire( "@command", "command", "changelevel " + DAY_ONE_START_MAP, 0.1 )
}

function TransitionToDayTwo()
{
	EntFire( "@command", "command", "changelevel " + DAY_TWO_START_MAP, 0.1 )
}

function TransitionToDayThree()
{
	EntFire( "@command", "command", "changelevel " + DAY_THREE_START_MAP, 0.1 )
}

function TransitionToDayFour()
{
	EntFire( "@command", "command", "changelevel " + DAY_FOUR_START_MAP, 0.1 )
}

function TransitionToDayFive()
{
	EntFire( "@command", "command", "changelevel " + DAY_FIVE_START_MAP, 0.1 )
}

function TransitionToDaySix()
{
	EntFire( "@command", "command", "changelevel " + DAY_SIX_START_MAP, 0.1 )
}

////////////////////////
// level select buttons
////////////////////////
function SubtractLevelSelect( nBranch )
{
	local nCurrentLevel = GetCoopBranchLevelIndex( nBranch )
	printl( "=== Selecting Day " + nBranch + ", Level " + (nCurrentLevel - 1) )
	if ( nCurrentLevel <= 1 )
	{
		return
	}

	nCurrentLevel = nCurrentLevel - 1
	EntFire( "@command", "Command", "coop_lobby_select_level " + nBranch + " " + nCurrentLevel, 0)	
}

function AddLevelSelect( nBranch )
{
	if ( nBranch < 1 )
	{
		return 
	}
	
	local nCurrentLevel = GetCoopBranchLevelIndex( nBranch )
	local nMaxLevelsInDay = DAY_ONE_LENGTH

	printl( "=== Selecting Day " + nBranch + ", Level " + (nCurrentLevel + 1) )
		
	if ( nBranch == 2 )
	{
		nMaxLevelsInDay = DAY_TWO_LENGTH
	}
	else if ( nBranch == 3 )
	{
		nMaxLevelsInDay = DAY_THREE_LENGTH
	}
	else if ( nBranch == 4 )
	{
		nMaxLevelsInDay = DAY_FOUR_LENGTH
	}
	//printl( "=== nMaxLevelsInDay = " + nMaxLevelsInDay )

	nCurrentLevel = nCurrentLevel + 1
	if ( nCurrentLevel > nMaxLevelsInDay )
	{
		printl( "nCurrentLevel (" + nCurrentLevel + ") > nMaxLevelsInDay (" + nMaxLevelsInDay + ")" )
		return 
	}
		
	EntFire( "@command", "Command", "coop_lobby_select_level " + nBranch + " " + nCurrentLevel, 0)
}
