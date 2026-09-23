function OpenHUBAreas()
{
	local numBranchesComplete = GetNumBranchesComplete();
	if ( numBranchesComplete >= 1 )
	{
		EntFire( "relay_track1_enable", "trigger", "", 2 )
	}
	
	if ( numBranchesComplete >= 2 )
	{
		EntFire( "relay_track3_enable", "trigger", "", 4 )
		EntFire( "relay_track4_enable", "trigger", "", 6 )
	}

	for ( local i = 0; i < 4; i++ )
	{
		for ( local j = 0; j < 16; j++ )
		{
			if ( IsLevelComplete( i, j ) )
			{
				EntFire( "texture_level_complete" + i + "" + j, "SetTextureIndex", "1", 0 )
			}
		}
	}
}

function CheckFirstEntry()
{
	local numLevelsComplete = GetNumLevelsComplete();
	
	if ( GetNumTauntsToAward() > 0 )
	{
		if ( numLevelsComplete > 0 )
		{
			EntFire( "relay_enable_all_tracks", "trigger", "", 15 )
			EntFire( "relay_gesture_artifact_1", "Trigger", "", 0 )	
		}
		else
		{
			EntFire( "@command", "command", "mp_unlock_taunt robotDance", 0.1 )
			EntFire( "@command", "command", "mp_unlock_taunt thumbsUp", 0.2 )
			EntFire( "relay_explain_make_human", "Trigger", "", 0.3 )
		}
	}
	else
	{
		EntFire( "relay_enable_all_tracks", "trigger", "", 0 )
	}
}

// --------------------------------------------------------
// checks to see if we have any new taunts
// --------------------------------------------------------
function CheckForNewTaunts()
{
	if ( GetNumTauntsToAward() > 0 )
	{
		EntFire( "@relay_has_new_taunt", "Trigger", "", 0.0 )
	}
}

function GetNumTauntsToAward()
{
	local nTauntsPending = GetNumPendingTaunts() 
			
	if ( GetNumPendingTaunts() > 0 )
	{
		return 1
	}
	
	return 0
}
