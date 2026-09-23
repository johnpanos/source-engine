// --------------------------------------------------------
// StartVideo
// --------------------------------------------------------

ENTRANCE_VIDEO <- 0
EXIT_VIDEO <- 1
ENTRANCE_DESTRUCTED_VIDEO <- 2
EXIT_DESTRUCTED_VIDEO <- 3

function StopEntryVideo(width,height)
{
	StopVideo(ENTRANCE_VIDEO,width,height)
}

function StopExitVideo(width,height)
{
	StopVideo(EXIT_VIDEO,width,height)
}

function StopVideo(videoType,width,height)
{
	for(local i=0;i<width;i+=1)
	{
		for(local j=0;j<height;j+=1)
		{
			local panelNum = 1 + width*j + i
			local signName
			
			if (videoType == EXIT_VIDEO || videoType == EXIT_DESTRUCTED_VIDEO )
			{
				signName = "@exit_sign" + panelNum + "-screen"
			}
			else
			{
				signName = "@entry_sign" + panelNum + "-screen"
			}
			
			EntFire(signName, "Disable", "", 0)
		}
	}
}

function StartEntryVideo(width,height)
{
	EntFire("@entry_video_master", "Enable", "", 0)
	StartVideo(ENTRANCE_VIDEO,width,height)
}

function StartExitVideo(width,height)
{
	printl("starting")
	EntFire("@exit_video_master", "Enable", "", 0)
	StartVideo(EXIT_VIDEO,width,height)
}

function StartDestructedEntryVideo(width,height)
{
	EntFire("@entry_video_master", "Enable", "", 0)
	StartVideo(ENTRANCE_DESTRUCTED_VIDEO,width,height)
}

function StartDestructedExitVideo(width,height)
{
	EntFire("@exit_video_master", "Enable", "", 0)
	StartVideo(EXIT_DESTRUCTED_VIDEO,width,height)
}

function StartVideo(videoType,width,height)
{
	for(local i=0;i<width;i+=1)
	{
		for(local j=0;j<height;j+=1)
		{
			local panelNum = 1 + width*j + i
			local signName
			local panelName
			local armName
			
			if (videoType == EXIT_VIDEO || videoType == EXIT_DESTRUCTED_VIDEO )
			{
				signName = "@exit_sign" + panelNum + "-screen"

				panelName = "@exit_sign" + panelNum + "-sign_panel"
				armName = "@exit_sign_arm" + panelNum

			}
			else
			{
				signName = "@entry_sign" + panelNum + "-screen"
			
				panelName = "@entry_sign" + panelNum + "-sign_panel"
				armName = "@entry_sign_arm" + panelNum
			}		
					
			if( j < 2 && RandomInt(0,1) == 0 && ( ( videoType == ENTRANCE_DESTRUCTED_VIDEO ) || ( videoType == EXIT_DESTRUCTED_VIDEO ) ) )
			{
				EntFire(signName, "setparent", panelName, 0)
				EntFire(panelName, "setparent", armName, 0)
				EntFire(panelName, "setparentattachmentmaintainoffset", "panel_attach", 0)
					
				local anim = RandomInt(0,3)
				
				switch ( anim )
				{
					case 0:
						EntFire(armName, "setanimationnoreset", "32_in_straight", 0)
						break
					case 1:
						EntFire(armName, "setanimationnoreset", "32_out_straight_A", 0)
						break
					case 2:
						EntFire(armName, "setanimationnoreset", "32_out_straight_B", 0)
						break
					case 3:
						EntFire(armName, "setanimationnoreset", "32_out_straight_C", 0)
						break
				}
			}
			
			if( RandomInt(0,4) == 0 && ( videoType == ENTRANCE_DESTRUCTED_VIDEO || videoType == EXIT_DESTRUCTED_VIDEO ) )
			{
				EntFire(signName, "Kill", "", 0)
			}
			else
			{
				EntFire(signName, "SetUseCustomUVs", 1, 0)
				
				local uMin = (i+0.0001)/(width)
				local uMax = (i+1.0001)/(width)
				local vMin = (j+0.0001)/(height)
				local vMax = (j+1.0001)/(height)
				 
				EntFire(signName, "SetUMin", uMin, 0)
				EntFire(signName, "SetUMax", uMax, 0)
				EntFire(signName, "SetVMin", vMin, 0)
				EntFire(signName, "SetVMax", vMax, 0)

				EntFire(signName, "Enable", "", 0)
				
//				printl(signName + " " + uMin + " " + uMax + " " + vMin + " " + vMax )
			}
		}
	}
}
