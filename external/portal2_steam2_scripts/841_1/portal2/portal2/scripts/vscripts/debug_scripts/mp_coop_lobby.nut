function OpenHUBAreas()
{
	local numBranchesComplete = GetNumBranchesComplete();
	if ( numBranchesComplete == 1 )
	{
		EntFire( "trigger_reveal_doors", "enable", "", 0 )
		EntFire( "return_ramp_up_relay", "trigger", "", 0 )
	}
	
	if ( numBranchesComplete == 2 )
	{
		EntFire( "trigger_reveal_catapult", "enable", "", 0 )
		EntFire( "return_ramp_up_relay", "trigger", "", 0 )
		EntFire( "relay_quick_reveal_tracks", "trigger", "", 0 )
	}
	if ( numBranchesComplete == 3 )
	{
		EntFire( "trigger_reveal_bridge", "enable", "", 0 )
		EntFire( "return_ramp_up_relay", "trigger", "", 0 )
		EntFire( "relay_quick_reveal_tracks", "trigger", "", 0 )
		EntFire( "relay_quick_reveal_catapult", "trigger", "", 0 )
	}
	if ( numBranchesComplete == 4 )
	{
		EntFire( "trigger_reveal_tbeam", "enable", "", 0 )
		EntFire( "return_ramp_up_relay", "trigger", "", 0 )
		EntFire( "relay_quick_reveal_tracks", "trigger", "", 0 )
		EntFire( "relay_quick_reveal_catapult", "trigger", "", 0 )
		EntFire( "relay_quick_reveal_bridge", "trigger", "", 0 ) 
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
			EntFire( "@command", "command", "mp_earn_taunt", 2.0 )
		}
		else
		{
			EntFire( "@command", "command", "mp_earn_taunt", 2.0 )
			EntFire( "@command", "command", "mp_unlock_taunt robotDance", 3.0 )
		}
	}
	
	EntFire( "trigger_reveal_ramp", "enable", "", 0 )
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
