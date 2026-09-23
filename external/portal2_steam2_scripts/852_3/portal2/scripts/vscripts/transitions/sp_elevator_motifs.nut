ElevatorMotifs <- 
[
	{ map = "elevator_test_start", speed = 300, motifs = [ "@shaft_2", "@shaft_3", "transition", ] },
	{ map = "sp_bridge_intro", speed = 200, motifs = [ "@shaft_stoppage_1", "transition", ] },
	{ map = "sp_column_blocker", speed = 200, motifs = [ "@shaft_vista", "transition", ] },
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
	
	foreach (index, level in ElevatorMotifs)
	{
		if (level.map == GetMapName() && ("motifs" in level) )
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
}