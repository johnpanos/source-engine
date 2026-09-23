-- build the variables for selection, as currently some of the select shapes are "SELECT-eyes" and some are "SELECT_eyes
gSEL_mouth	= "SELECT-mouth";
gSEL_skull	= "SELECT-skull";
gSEL_socket	= "SELECT-socket";

if selectSeparator then
	gSEL_mouth	= "SELECT" .. selectSeparator .. "mouth";
	gSEL_skull	= "SELECT" .. selectSeparator .. "skull";
	gSEL_socket	= "SELECT" .. selectSeparator .. "socket";
end

	-- remove movement from socket and teeth
	Select( gSEL_socket );
	Select( "ADD", gSEL_mouth );
	if lowerFaceSwitch then
		TargetList = {
						"ClenchJaw",
						"CompressLips_RaiseChin",
						"CompressLips",
						"DeflateCheek",
						"Dimple_FunnelLips",
						"Dimple_OpenLips_PuckerLips",
						"Dimple_OpenLips",
						"Dimple",
						"FunnelLips",
						"InflateCheek",
						"LipCnrTwst_OpenUpperLip_SuckLipLo",
						"LipCnrTwst",
						"OpenLips_PuckerLips",
						"OpenLips_SmileFull",
						"OpenLips",
						"OpenLowerLip_OpenUpperLip_Platysmus_SmileFull",
						"OpenLowerLip_OpenUpperLip",
						"OpenLowerLip_SuckLipLo",
						"OpenLowerLip",
						"OpenUpperLip_SuckLipUp",
						"OpenUpperLip",
						"Platysmus",
						"PressNose",
						"PuckerLips_RaiseChin_SneerNose",
						"PuckerLips_RaiseChin",
						"PuckerLips",
						"PuffLips",
						"RaiseBrowIn",
						"RaiseChin",
						"ScalpBack",
						"ScalpForward",
						"SmileFlat",
						"SmileFull",
						"SmileSharp",
						"SneerNose",
						"SuckLipLo",
						"SuckLipUp",
						"SuckNostril",
		};
		RemoveFromDelta( TargetList );
	end
	if upperFaceSwitch then
		TargetList = {
						"BlowNostril",
						"BrowOutV_Frown_RaiseBrowIn",
						"BrowOutV_RaiseBrowIn",
						"BrowOutV",
						"CheekV_PuckerLips",
						"CheekV_SmileFull",
						"CheekV_SneerNose",
						"CheekV",
						"CloseLidLo",
						"CloseLidUp",
						"Frown_WrinkleNose",
						"Frown",
						"InnerSquint",
						"OuterSquint_InnerSquint",
						"OuterSquint",
						"WrinkleNose"
		};
		RemoveFromDelta( TargetList );	
	end
	Select( gSEL_skull );
	TargetList = {
					"OpenJaw_OpenLips",
					"OpenJaw_OpenLowerLip_OpenUpperLip_Platysmus_SmileFull",
					"OpenJaw_OpenLowerLip_OpenUpperLip",
					"OpenJaw_PuckerLips",
					"OpenJaw",	        
					"JutJaw",
					"JutUpperLip_RaiseChin",
					"JutUpperLip"
	};
	RemoveFromDelta( TargetList );