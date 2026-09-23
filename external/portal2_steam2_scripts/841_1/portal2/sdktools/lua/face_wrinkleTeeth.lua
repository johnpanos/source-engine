-- build the variables for selection, as currently some of the select shapes are "SELECT-eyes" and some are "SELECT_eyes
gSEL_jaw	= "SELECT-jaw";
gSEL_skull	= "SELECT-skull";

if selectSeparator then
	gSEL_jaw	= "SELECT" .. selectSeparator .. "jaw";
	gSEL_skull	= "SELECT" .. selectSeparator .. "skull";
end

-- create wrinkle deltas for glowing teeth
ResetState();
SetState( gSEL_skull ) ;
ComputeWrinkle( "OpenUpperLip", 1);

-- create wrinkle deltas for glowing teeth
ResetState();
SetState( gSEL_jaw ) ;
ComputeWrinkle( "OpenLowerLip", 1);

-- create wrinkle deltas for glowing teeth
ResetState();
SetState( gSEL_skull ) ;
Select ("ALL");
Add ( gSEL_jaw, 1.0  ) ;
ComputeWrinkle( "OpenLips", 0.5 );

-- create wrinkle deltas for glowing teeth
ResetState();
SetState( gSEL_skull ) ;
ComputeWrinkle( "FunnelLipUp", -1);

-- create wrinkle deltas for glowing teeth
ResetState();
SetState( gSEL_jaw ) ;
ComputeWrinkle( "FunnelLipLo", -1);
