ElevatorMotifs <- 
[
	{ map = "sp_a2_bridge_intro", speed = 200, loop =1  },
	{ map = "sp_a2_laser_over_goo", speed = 300, motifs = [ "@shaft_stoppage_1", "transition", ] },
	{ map = "sp_a2_column_blocker", speed = 200, motifs = [ "@shaft_vista", "transition", ] },


	{ map = "sp_a2_intro", speed = 200, motifs = [ "@shaft_1", "@shaft_2", "transition" ] },	
	{ map = "sp_a2_laser_intro", speed = 200, loop = 1 },
	{ map = "sp_a2_laser_stairs", speed = 200, loop = 1 },
	{ map = "sp_a2_dual_lasers", speed = 200, loop = 1 },
//	{ map = "sp_a2_laser_over_goo", speed = 200, loop = 1 },
	{ map = "sp_a2_catapult_intro", speed = 200, loop = 1 },
	{ map = "sp_a2_trust_fling", speed = 200, loop = 1 },
//	{ map = "sp_a2_pit_flings", speed = 200, loop = 1 },
//	{ map = "sp_a2_fizzler_intro", speed = 200, loop = 1 },
	{ map = "sp_a2_sphere_peek", speed = 200, loop = 1 },
	{ map = "sp_a2_ricochet", speed = 200, loop = 1 },
	{ map = "sp_a2_bridge_the_gap", speed = 200, loop = 1 },
	{ map = "sp_a2_turret_intro", speed = 200, loop = 1 },
	{ map = "sp_a2_laser_relays", speed = 200, loop = 1 },
	{ map = "sp_a2_turret_blocker", speed = 200, loop = 1 },
	{ map = "sp_a2_laser_vs_turret", speed = 200, loop = 1 },
	{ map = "sp_a2_pull_the_rug", speed = 200, loop = 1 },
	{ map = "sp_a2_ring_around_turrets", speed = 200, loop = 1 },
//	{ map = "sp_a2_laser_chaining", speed = 200, loop = 1 },
//	{ map = "sp_a2_turret_tower", speed = 200, loop = 1 },
//	{ map = "sp_a2_triple_laser", speed = 200, loop = 1 },
	{ map = "sp_a4_tb_trust_drop", speed = 200, loop = 1 },
]

function StartMoving()
{
	local foundLevel = false
	
	foreach (index, level in ElevatorMotifs)
	{
		if (level.map == GetMapName() && ("speed" in level) )
		{
			printl( "Starting elevator " + self.GetName() + " with speed " + level.speed )
			EntFire(self.GetName(),"SetSpeedReal",level.speed,0.0)
			foundLevel = true
		}
	}
	
	if (foundLevel == false) 
	{
		printl( "Using default elevator speed 300" )
		EntFire(self.GetName(),"SetSpeedReal","300",0.0)
	}
}

function PrepareTeleport()
{	
	local foundLevel = false
	local continualLoop = false
	
	foreach (index, level in ElevatorMotifs)
	{
		if ( level.map == GetMapName() )
		{
			if("loop" in level) 
			{
				continualLoop = true
			}
			else if ("motifs" in level)
			{
				printl( "Trying to connect to motif " + level.motifs[::MotifIndex] )

				if( level.motifs[::MotifIndex] == "transition" )
				{
					EntFire("@transition_with_survey","Trigger","",0.0)
					return
				}
				else
				{
					EntFire(self.GetName(),"SetRemoteDestination",level.motifs[::MotifIndex],0.0)
				}
				foundLevel = true
			}
		}
	}
	
	if (continualLoop == true)
	{
		if( ::TransitionReady == 1 || ::MotifIndex >= 4 )
		{
			EntFire("@transition_with_survey","Trigger","",0.0)
		}
		else if( (::MotifIndex)%2 == 0 )
		{
			EntFire(self.GetName(),"SetRemoteDestination","@shaft_1",0.0)
		}
		else
		{
			EntFire(self.GetName(),"SetRemoteDestination","@shaft_2",0.0)
		}
	}
	else 
	if (foundLevel == false)
	{
		printl("Using default elevator motif")
		
		if( ::MotifIndex == 0 )
//		{
//			EntFire(self.GetName(),"SetRemoteDestination","@shaft_1",0.0)
//			printl("connecting to motif shaft_1")
//		}
//		else if( ::MotifIndex == 1 )
//		{
//			EntFire(self.GetName(),"SetRemoteDestination","@shaft_2",0.0)		
//			printl("connecting to motif shaft_2")
//		}
//		else if( ::MotifIndex == 2 )
//		{
//			EntFire(self.GetName(),"SetRemoteDestination","@shaft_1",0.0)				
//			printl("connecting to motif shaft_1")
//		}
//		else if( ::MotifIndex == 3 )
		{
			EntFire("@transition_with_survey","Trigger","",0.0)
			printl("connecting to transition")
		}
	}
	
	EntFire(self.GetName(),"Enable",0.0)	
	::MotifIndex += 1
}

function OnPostSpawn()
{
	::MotifIndex <- 0
	::TransitionReady <- 0
}