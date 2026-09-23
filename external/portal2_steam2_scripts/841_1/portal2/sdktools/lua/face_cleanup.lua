-- build the variables for selection, as currently some of the select shapes are "SELECT-eyes" and some are "SELECT_eyes

gSEL_centerLine 	= "SELECT-centerLine";
gSEL_eyeInnerCorner	= "SELECT-eyeInnerCorner";
gSEL_eyeOuterCorner	= "SELECT-eyeOuterCorner";
gSEL_eyes			= "SELECT-eyes";
gSEL_jaw			= "SELECT-jaw";
gSEL_jawEffect		= "SELECT-jawEffect";
gSEL_loLip			= "SELECT-loLip";
gSEL_mouth			= "SELECT-mouth";
gSEL_mouthCorner	= "SELECT-mouthCorner";
gSEL_skull			= "SELECT-skull";
gSEL_socket			= "SELECT-socket";
gSEL_throat			= "SELECT-throat";
gSEL_tongue			= "SELECT-tongue";
gSEL_upLip			= "SELECT-upLip";

gSEL_lipcontacts	= "SELECT-lipcontacts";

if selectSeparator then
	gSEL_centerLine 	= "SELECT" .. selectSeparator .. "centerLine";
	gSEL_eyeInnerCorner	= "SELECT" .. selectSeparator .. "eyeInnerCorner";
	gSEL_eyeOuterCorner	= "SELECT" .. selectSeparator .. "eyeOuterCorner";
	gSEL_eyes			= "SELECT" .. selectSeparator .. "eyes";
	gSEL_jaw			= "SELECT" .. selectSeparator .. "jaw";
	gSEL_jawEffect		= "SELECT" .. selectSeparator .. "jawEffect";
	gSEL_loLip			= "SELECT" .. selectSeparator .. "loLip";
	gSEL_mouth			= "SELECT" .. selectSeparator .. "mouth";
	gSEL_mouthCorner	= "SELECT" .. selectSeparator .. "mouthCorner";
	gSEL_skull			= "SELECT" .. selectSeparator .. "skull";
	gSEL_socket			= "SELECT" .. selectSeparator .. "socket";
	gSEL_throat			= "SELECT" .. selectSeparator .. "throat";
	gSEL_tongue			= "SELECT" .. selectSeparator .. "tongue";
	gSEL_upLip			= "SELECT" .. selectSeparator .. "upLip";

	gSEL_lipcontacts	= "SELECT" .. selectSeparator .. "lipcontacts";
end

DeleteDelta( "NEWJAW" );
DeleteDelta( "FIXER" );
DeleteDelta( "ref" );

DeleteDelta( gSEL_centerLine );
DeleteDelta( gSEL_eyeInnerCorner );
DeleteDelta( gSEL_eyeOuterCorner );
DeleteDelta( gSEL_eyes );
DeleteDelta( gSEL_jaw );
DeleteDelta( gSEL_jawEffect );
DeleteDelta( gSEL_loLip );
DeleteDelta( gSEL_mouth );
DeleteDelta( gSEL_mouthCorner );
DeleteDelta( gSEL_skull );
DeleteDelta( gSEL_socket );
DeleteDelta( gSEL_throat );
DeleteDelta( gSEL_tongue );
DeleteDelta( gSEL_upLip );

DeleteDelta( gSEL_lipcontacts );	

DeleteDelta( "PuffLips" );
