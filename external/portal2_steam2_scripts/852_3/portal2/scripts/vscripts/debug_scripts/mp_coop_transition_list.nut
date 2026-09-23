DBG <- 1

HUB_MAP <- "mp_coop_lobby_2"

DAY_ONE_START_MAP <- "mp_coop_wall_intro"
DAY_ONE_END_MAP <- "mp_coop_wall_5"
DAY_ONE_LENGTH <- 1

DAY_TWO_START_MAP <- "mp_coop_come_along"
DAY_TWO_END_MAP <- "mp_coop_fan"
DAY_TWO_LENGTH <- 1

DAY_THREE_START_MAP <- "mp_coop_rat_maze"
DAY_THREE_END_MAP <- "mp_coop_rocket_block"
DAY_THREE_LENGTH <- 1

DAY_FOUR_START_MAP <- "mp_coop_paint_come_along"
DAY_FOUR_END_MAP <- "mp_coop_paint_red_racer"
DAY_FOUR_LENGTH <- 1

MapPlayOrder<- [

//PAX demo track
"mp_coop_start",
"mp_coop_laser_2",
"mp_coop_wall_intro_train",
"mp_coop_infinifling_train",

]

MapPlayOrderNormal<- [
// this is the order to play the multiplayer maps

//"mp_coop_mic_calibration",

// pressdemo track
//"mp_coop_doors",
//"mp_coop_laser_2",
//"mp_coop_laser_crusher",
//"mp_coop_wall_intro",
//"mp_coop_wall_catch",
//"mp_coop_race_2",
//"mp_coop_wall_straight_jump",

//PAX demo track
//"mp_coop_start",
//"mp_coop_laser_2",
//"mp_coop_wall_intro",
//"mp_coop_infinifling_train",

// INTRO
"mp_coop_start",
"mp_coop_start_02",
"mp_coop_start_03",
"mp_coop_fling_train",
"mp_coop_lobby_2",

// DAY 1 Start
"mp_coop_wall_intro",
"mp_coop_wall_2",
"mp_coop_catapult_wall_intro",
"mp_coop_wall_block",
"mp_coop_catapult_2",
"mp_coop_wall_5",

// DAY 2 Start
"mp_coop_come_along",
"mp_coop_fling_1",
"mp_coop_catapult_1",
"mp_coop_multifling_1",
"mp_coop_fling_crushers",
"mp_coop_fan",

// DAY 3 Start
"mp_coop_rat_maze",
"mp_coop_laser_crusher",
"mp_coop_vertical_crushers",
"mp_coop_turret_walls",
"mp_coop_laser_1",
"mp_coop_race_3",
"mp_coop_rocket_block",

// DAY 4 Start
"mp_coop_paint_come_along",
"mp_coop_paint_bridge_intro",
"mp_coop_paint_bridge",
"mp_coop_paint_bridge_02",
"mp_coop_paint_redirect",
"mp_coop_paint_fizzler_intro",
"mp_coop_paint_speed_fling",
"mp_coop_paint_red_racer",


// ===========================================
// === Maps that have bugs, are broken and/or cannot be tested, go here
// ===========================================
/// NEEDS WORK
"mp_coop_wall_stop",
"mp_coop_catapult_wall_intro",
"mp_coop_paint_crazy_box",
"mp_coop_wall_3",
"mp_coop_wall_1",
"mp_coop_rat_maze_2",
"mp_coop_crusher",
"mp_coop_turret_ball",
"mp_coop_double_catch",
"mp_coop_wall_6",

// ===========================================
// === Paint Gun maps go here
// ===========================================
"mp_coop_2guns_intro",
"mp_coop_2guns_swap",
"mp_coop_2guns_flingstick",
"mp_coop_2guns_bridge_jump",
"mp_coop_2guns_walljumps",
"mp_coop_2guns_bridges",
"mp_coop_2guns_bridge_stick",
"mp_coop_2guns_longjump_intro",
"mp_coop_2guns_longjumps",

// ===========================================
// === Incubation - !!!all maps needing retrofit/deletion here!!!
// ===========================================
"mp_coop_paint_speed_catch",
"mp_coop_trap_1",
"mp_coop_button_tower",
"mp_coop_vista",
"mp_coop_2guns_bridges2",
"mp_coop_race_2",
"mp_coop_wall_catch",


// ===========================================
// === CUT
// ===========================================
"mp_coop_fling_3,"
"mp_coop_laser_2,"
"mp_coop_fling_1,"

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
				printl( "=== Start of day 1" )
			}
		}
		else if( MapPlayOrder[index] == DAY_TWO_START_MAP )
		{
			nBranch++
			DAY_ONE_LENGTH = daymapcount
			daymapcount = 0
			
			if ( DBG )
			{
				printl( "< " + DAY_ONE_LENGTH + " levels in Day 1 >" )
				printl( "=== Start of day 2" )
			}
		}
		else if( MapPlayOrder[index] == DAY_THREE_START_MAP )
		{
			nBranch++
			DAY_TWO_LENGTH = daymapcount
			daymapcount = 0
			
			if ( DBG )
			{
				printl( "< " + DAY_TWO_LENGTH + " levels in Day 2 >" )
				printl( "=== Start of day 3" )
			}
		}
		else if( MapPlayOrder[index] == DAY_FOUR_START_MAP )
		{
			nBranch++
			DAY_THREE_LENGTH = daymapcount
			daymapcount = 0
			
			if ( DBG )
			{
				printl( "< " + DAY_THREE_LENGTH + " levels in Day 3 >" )
				printl( "=== Play these at your own risk" )
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
		
		if( MapPlayOrder[index] == DAY_FOUR_END_MAP )
		{
			nBranch++
			DAY_FOUR_LENGTH = daymapcount+1
			daymapcount = 0
			
			if ( DBG )
			{
				printl( "< " + DAY_FOUR_LENGTH + " levels in Day 4 >" )
				printl( "=== Incubation - !!!all maps needing retrofit/deletion here!!!" )
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
		EntFire( "@command", "command", "mp_unlock_taunt bigWave", 0.1 )
		EntFire( "@command", "command", "mp_unlock_taunt dance", 0.2 )
		EntFire( "@command", "command", "changelevel " + HUB_MAP, 1.3 )
	}
	else if( GetMapName() == DAY_TWO_END_MAP )
	{
		EntFire( "@command", "command", "mp_unlock_taunt laugh", 0.0 )
		EntFire( "@command", "command", "mp_unlock_taunt shrug", 0.1 )
		EntFire( "@command", "command", "mp_unlock_taunt teamlaugh", 0.2 )
		EntFire( "@command", "command", "changelevel " + HUB_MAP, 1.3 )
	}
	else if( GetMapName() == DAY_THREE_END_MAP )
	{
		EntFire( "@command", "command", "mp_unlock_taunt fistPump", 0.1 )
		EntFire( "@command", "command", "mp_unlock_taunt trickfire", 0.2 )
		EntFire( "@command", "command", "mp_unlock_taunt teamhug", 0.3 )
		EntFire( "@command", "command", "changelevel " + HUB_MAP, 1.3 )
	}
	else if( GetMapName() == DAY_FOUR_END_MAP )
	{
		EntFire( "@command", "command", "mp_unlock_taunt facepalm", 0.1 )
		EntFire( "@command", "command", "mp_unlock_taunt music", 0.2 )
		EntFire( "@command", "command", "mp_unlock_taunt teamChestBump", 0.3 )
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
