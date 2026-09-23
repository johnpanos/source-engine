---- this script recquires the following input OBJ shapes
-- head=zero.obj
-- head_BlowNostril.obj
-- head_BrowOutV+Frown+RaiseBrowIn.obj
-- head_BrowOutV+RaiseBrowIn.obj
-- head_BrowOutV.obj
-- head_CheekV+PuckerLips.obj
-- head_CheekV+SmileFull.obj
-- head_CheekV+SneerNose.obj
-- head_CheekV.obj
-- head_ClenchJaw.obj
-- head_CloseLidLo.obj
-- head_CloseLidUp.obj
-- head_CompressLips+RaiseChin.obj
-- head_CompressLips.objf
-- head_DeflateCheek.obj
-- head_Dimple+FunnelLips.obj
-- head_Dimple+OpenLips+PuckerLips.obj
-- head_Dimple+OpenLips.obj
-- head_Dimple+PuckerLips.obj
-- head_Dimple.obj
-- head_Frown+RaiseBrowIn.obj
-- head_Frown+WrinkleNose.obj
-- head_Frown.obj
-- head_FunnelLips+OpenJaw.obj
-- head_FunnelLips.obj
-- head_InflateCheek.obj
-- head_InnerSquint.obj
-- head_JutJaw.obj
-- head_JutUpperLip+RaiseChin.obj
-- head_JutUpperLip.obj
-- head_LipCnrTwst+OpenUpperLip+SuckLipLo.obj
-- head_LipCnrTwst.obj
-- head_OpenJaw+OpenLips.obj
-- head_OpenJaw+OpenLowerLip+OpenUpperLip+Platysmus+SmileFull.obj
-- head_OpenJaw+OpenLowerLip+OpenUpperLip.obj
-- head_OpenJaw+PuckerLips.obj
-- head_OpenJaw.obj
-- head_OpenLips+PuckerLips.obj
-- head_OpenLips+SmileFull.obj
-- head_OpenLips.obj
-- head_OpenLowerLip+OpenUpperLip+Platysmus+SmileFull.obj
-- head_OpenLowerLip+OpenUpperLip.obj
-- head_OpenLowerLip+SuckLipLo.obj
-- head_OpenLowerLip.obj
-- head_OpenUpperLip+SuckLipUp.obj
-- head_OpenUpperLip.obj
-- head_OuterSquint+InnerSquint.obj
-- head_OuterSquint.obj
-- head_Platysmus.obj
-- head_PressNose.obj
-- head_PuckerLips+PuffLips.obj
-- head_PuckerLips+RaiseChin+SneerNose.obj
-- head_PuckerLips+RaiseChin.obj
-- head_PuckerLips.obj
-- head_PuffLips.obj
-- head_RaiseBrowIn.obj
-- head_RaiseChin.obj
-- head_SELECT_Jaw.obj
-- head_SELECT_JawEffect.obj
-- head_SELECT_Skull.obj
-- head_SELECT_centerLine.obj
-- head_SELECT_eyeInnerCorner.obj
-- head_SELECT_eyeOuterCorner.obj
-- head_SELECT_eyes.obj
-- head_SELECT_lipContacts.obj
-- head_SELECT_loLip.obj
-- head_SELECT_mouth.obj
-- head_SELECT_mouthCorner.obj
-- head_SELECT_socket.obj
-- head_SELECT_throat.obj
-- head_SELECT_upLip.obj
-- head_ScalpBack.obj
-- head_ScalpForward.obj
-- head_SmileFlat.obj
-- head_SmileFull.obj
-- head_SmileSharp.obj
-- head_SneerNose.obj
-- head_SuckJaw.obj
-- head_SuckLipLo.obj
-- head_SuckLipUp.obj
-- head_SuckNostril.obj
-- head_WrinkleNose.obj
-- 
-------------------------------------------------------------------------------

-- build the variables for selection, as currently some of the select shapes are "SELECT-eyes" and some are "SELECT_eyes
gSEL_centerLine 	= "SELECT-centerLine";
gSEL_eyeInnerCorner	= "SELECT-eyeInnerCorner";
gSEL_eyeOuterCorner	= "SELECT-eyeOuterCorner";
gSEL_eyes			= "SELECT-eyes";
gSEL_jaw			= "SELECT-jaw";
gSEL_jawEffect		= "SELECT-jawEffect";
gSEL_loLip			= "SELECT-loLip";
gSEL_mouthCorner	= "SELECT-mouthCorner";
gSEL_skull			= "SELECT-skull";
gSEL_throat			= "SELECT-throat";
gSEL_upLip			= "SELECT-upLip";

if selectSeparator then
	gSEL_centerLine 	= "SELECT" .. selectSeparator .. "centerLine";
	gSEL_eyeInnerCorner	= "SELECT" .. selectSeparator .. "eyeInnerCorner";
	gSEL_eyeOuterCorner	= "SELECT" .. selectSeparator .. "eyeOuterCorner";
	gSEL_eyes			= "SELECT" .. selectSeparator .. "eyes";
	gSEL_jaw			= "SELECT" .. selectSeparator .. "jaw";
	gSEL_jawEffect		= "SELECT" .. selectSeparator .. "jawEffect";
	gSEL_loLip			= "SELECT" .. selectSeparator .. "loLip";
	gSEL_mouthCorner	= "SELECT" .. selectSeparator .. "mouthCorner";
	gSEL_skull			= "SELECT" .. selectSeparator .. "skull";
	gSEL_throat			= "SELECT" .. selectSeparator .. "throat";
	gSEL_upLip			= "SELECT" .. selectSeparator .. "upLip";
end

--------------------------------------
--            upperface             --
--------------------------------------
if upperFaceSwitch then
		
	-- create CheekV_OuterSquint
	ResetState();
	SetState ( "OuterSquint" );
	Select( "ALL" );
	Add( "CheekV", 1.0 );
	Select( gSEL_eyes );
	GrowSelection (3);
	Interp( "OuterSquint" , 1.0, 1.25, "SPIKE" );
	SaveDelta( "CheekV_OuterSquint" );
	
	-- create CheekV_InnerSquint
	ResetState();
	SetState ( "CheekV" );
	Select( "ALL" );
	Add( "InnerSquint", 1.0 );
	Select( "InnerSquint" );
	Interp( "InnerSquint" , 0.1, .5, "SPIKE" );
	SaveDelta( "CheekV_InnerSquint" );
	
	-- create CheekV_InnerSquint_OuterSquint
	ResetState();
	SetState ( "OuterSquint_InnerSquint" );
	Select( "ALL" );
	Add( "CheekV", 1.0 );
	Select( "CloseLidUp" );
	Interp( "OuterSquint_InnerSquint" , 1 , .5, "SPIKE" );
	Select( gSEL_eyes );
	GrowSelection (4);
	Interp( "OuterSquint_InnerSquint" , 1.0, 1.25, "SPIKE" );
	SaveDelta( "CheekV_InnerSquint_OuterSquint" );
	
	-- create BrowOutV_InnerSquint
	ResetState();
	SetState ( "InnerSquint" );
	Select( "BrowOutV" );
	Interp( "BrowOutV" , 1 , 0.5, "SPIKE" );
	Select( gSEL_eyes );
	GrowSelection ( 3 ) ;
	Select( "ADD" , "CheekV" );
	Select( "SUBTRACT", "BrowOutV" );
	Interp( "InnerSquint" , 1 , 0.25, "SPIKE" );
	Select( "CloseLidLo" );
	Interp( "InnerSquint" , 1);
	SaveDelta( "BrowOutV_InnerSquint" );
	
	-- create BrowOutV_OuterSquint
	ResetState();
	SetState ( "OuterSquint" );
	Select( "ALL" );
	Add( "BrowOutV", 1.0 );
	Select( "OuterSquint_InnerSquint" );
	ShrinkSelection( 4 );
	Interp( "OuterSquint", 1 , .22, "SPIKE" );
	SaveDelta( "BrowOutV_OuterSquint" );
	
	-- create BrowOutV_OuterSquint_InnerSquint
	ResetState();
	SetState ( "OuterSquint_InnerSquint" );
	Select( "ALL" );
	Add( "BrowOutV", 1.0 );
	Select ("BrowOutV");
	ShrinkSelection( 1 );
	Interp( "BrowOutV_InnerSquint", 1 , .5, "SPIKE" );
	Select( "OuterSquint_InnerSquint" );
	ShrinkSelection( 4 );
	Interp( "OuterSquint_InnerSquint", 1 , .22, "SPIKE" );
	Select( "OuterSquint" );
	Interp( "OuterSquint_InnerSquint", .35 , 1, "SPIKE" );
	SaveDelta( "BrowOutV_OuterSquint_InnerSquint" );
	
	-- create RaiseBrowIn_InnerSquint
	ResetState();
	SetState ( "RaiseBrowIn" );
	Select( "ALL" );
	Add( "InnerSquint", 1.0 );
	Select( "InnerSquint");
	ShrinkSelection( 2 );
	Interp( "InnerSquint", 1 , .3, "SPIKE" );
	SaveDelta( "RaiseBrowIn_InnerSquint" );

	
	-- create CloseLidLo_CheekV
	ResetState();
	SetState ( "CloseLidLo" );
	Select( "ALL" );
	Add( "CheekV", 1.0 );
	Select( "CloseLidUp" );
	GrowSelection( 2 );
	Interp( "CloseLidLo", 1 , .2, "SPIKE" );
	SaveDelta( "CloseLidLo_CheekV" );
	
	-- create CloseLidUp_RaiseBrowIn
	ResetState();
	SetState ( "RaiseBrowIn" );
	Select( "CloseLidUp" );
	Interp( "CloseLidUp", 1.0 );
	SaveDelta( "CloseLidUp_RaiseBrowIn" );
	
	-- create CloseLidLo_OuterSquint
	ResetState();
	SetState ( "OuterSquint" );
	Select( "CloseLidUp" );
	Interp( "CloseLidUp", 1.0 );
	Select ( gSEL_eyeOuterCorner);
	GrowSelection ( 3 );
	Interp( "OuterSquint", .25 , .15, "SPIKE" );
	SaveDelta( "CloseLidUp_OuterSquint" );
	
	-- create CloseLidUp_InnerSquint
	ResetState();
	SetState ( "InnerSquint" );
	Select( "CloseLidUp" );
	Interp( "CloseLidUp", 1.0 );
	SaveDelta( "CloseLidUp_InnerSquint" );
	
	-- create CloseLidUp_InnerSquint_OuterSquint
	ResetState();
	SetState ( "InnerSquint_OuterSquint" );
	Select( "ALL" );
	Add( "CloseLidUp", 0.5 );
	Select( gSEL_eyes );
	GrowSelection ( 1);
	Select( "INTERSECT", "CloseLidUp" );
	Interp( "CloseLidUp", 1 );
	Select( gSEL_eyeOuterCorner );
	Interp( "InnerSquint_OuterSquint", 1, 0.5, "STRAIGHT" );
	SaveDelta( "CloseLidUp_InnerSquint_OuterSquint" );
	
	-- create InnerSquint_OuterSquint_RaiseBrowIn
	ResetState();
	SetState ( "InnerSquint_OuterSquint" );
	Select ( "ALL");
	Add( "RaiseBrowIn", 1 );
	SaveDelta( "InnerSquint_OuterSquint_RaiseBrowIn" );
	
	-- create CloseLidUp_InnerSquint_OuterSquint_RaiseBrowIn
	ResetState();
	SetState ( "InnerSquint_OuterSquint_RaiseBrowIn" );
	Select ( gSEL_eyes);
	GrowSelection(1);
	Interp( "CloseLidUp_InnerSquint_OuterSquint", 1.0, .25, "SPIKE" );
	SaveDelta( "CloseLidUp_InnerSquint_OuterSquint_RaiseBrowIn" );
	
	-- create BrowOutV_CloseLidUp
	ResetState();
	SetState ( "BrowOutV" );
	Select( gSEL_eyes );
	GrowSelection (2);
	Interp( "CloseLidUp", 1.0, 0.25, "SPIKE" );
	SaveDelta( "BrowOutV_CloseLidUp" );
	
	-- create BrowOutV_Frown
	ResetState();
	SetState( "BrowOutV" );
	Select( gSEL_centerLine);
	GrowSelection ( 2 );
	Interp( "Frown", 1.0, 1.3, "SPIKE" );
	SaveDelta( "BrowOutV_Frown" );
	
	-- create BrowOutV_CloseLidUp_RaiseBrowIn
	ResetState();
	SetState ( "BrowOutV_RaiseBrowIn" );
	Select( gSEL_eyes );
	GrowSelection (2);
	Interp( "CloseLidUp", 1.0, 0.1, "SPIKE" );
	SaveDelta( "BrowOutV_CloseLidUp_RaiseBrowIn" );
	
	-- create BrowOutV_CloseLidUp_Frown
	ResetState();
	SetState ( "BrowOutV_Frown" );
	Select( gSEL_eyes );
	GrowSelection (2);
	Interp( "CloseLidUp", 1.0, 0.1, "SPIKE" );
	Select( gSEL_eyeOuterCorner );
	GrowSelection ( 3);
	Interp( "BrowOutV_CloseLidUp", 1.0, 0.25, "SPIKE" );
	SaveDelta( "BrowOutV_CloseLidUp_Frown" );
	
	-- create InnerSquint_SneerNose
	ResetState();
	SetState ( "InnerSquint" );
	Select( "ALL" );
	Add( "SneerNose", 1.0 );
	Select( gSEL_eyes );
	GrowSelection( 1 );
	Interp( "InnerSquint" , 1.0, .25, "STRAIGHT" );
	Select( gSEL_eyeInnerCorner );
	GrowSelection( 3 );
	Interp( "InnerSquint" , 1.0, .25, "STRAIGHT" );	
	SaveDelta( "InnerSquint_SneerNose" );
	
	-- create InnerSquint_OuterSquint_SneerNose
	ResetState();
	SetState( "InnerSquint_OuterSquint" );
	Select( "ALL");
	Add( "SneerNose", 1.0 );
	Select( gSEL_eyes);
	GrowSelection ( 2 );
	Interp( "InnerSquint_OuterSquint", 1.0, 0.25, "STRAIGHT" );
	Select( gSEL_eyeInnerCorner );
	GrowSelection( 3 );
	Interp( "InnerSquint_SneerNose" , 1.0, .25, "STRAIGHT" );	
	SaveDelta( "InnerSquint_OuterSquint_SneerNose" );
	
	-- create CheekV_InnerSquint
	ResetState();
	SetState( "InnerSquint" );
	Select( "ALL");
	Add( "CheekV", 1.0 );
	Select( gSEL_eyes);
	GrowSelection ( 2 );
	Interp( "InnerSquint", 1.0, 0.2, "SPIKE" );
	SaveDelta( "CheekV_InnerSquint" );

end


--------------------------------------
--            lowerface             --
--------------------------------------

if lowerFaceSwitch then
	
	-- create control split PuckerLipUp from PuckerLips
	ResetState();
	SetState( "PuckerLips" );
	Select( gSEL_loLip);
	Interp( base, 1.0, 2.0, "STRAIGHT" );
	Select( gSEL_upLip);
	Interp( "PuckerLips", 1.0, 0.75, "STRAIGHT" );
	Select( gSEL_centerLine);
	GrowSelection( 1) ;
	Select( "INTERSECT", gSEL_loLip);
	Interp( base, 1.0, .75, "STRAIGHT");
	Select( gSEL_mouthCorner);
	Interp( "PuckerLips", 0.5, 1, "SPIKE");
	SaveDelta( "PuckerLipUp" );
	
	-- create control split PuckerLipLo from PuckerLips
	ResetState();
	SetState( "PuckerLips" );
	Select( gSEL_upLip);
	Interp( base, 1.0, 2.0, "STRAIGHT" );
	Select( gSEL_loLip);
	Interp( "PuckerLips", 1.0, 0.75, "STRAIGHT" );
	Select( gSEL_centerLine);
	GrowSelection( 2) ;
	Select( "INTERSECT", gSEL_upLip);
	Interp( base, 1.0, .5, "STRAIGHT");
	Select( gSEL_mouthCorner);
	Interp( "PuckerLips", 0.5, 1, "SPIKE");
	SaveDelta( "PuckerLipLo" );
	
	-- create combination PuckerLips
	ResetState();
	SetState( "PuckerLips" );
	SaveDelta( "PuckerLipLo_PuckerLipUp" );
	
	-- create combination OpenJaw_PuckerLipUp
	ResetState();
	SetState( "OpenJaw_PuckerLips" );
	Select( gSEL_loLip);
	Interp( "OpenJaw", 1.0, 2.0, "STRAIGHT" );
	Select( gSEL_upLip);
	Interp( "OpenJaw_PuckerLips", 1.0, 0.75, "STRAIGHT" );
	Select( gSEL_centerLine);
	GrowSelection( 2) ;
	Select( "INTERSECT", gSEL_loLip);
	Interp( "OpenJaw", 1.0, .5, "STRAIGHT");
	Select( gSEL_mouthCorner);
	Interp( "OpenJaw_PuckerLips", 0.5, 1, "SPIKE");
	SaveDelta( "OpenJaw_PuckerLipUp" );
	
	-- create combination OpenJaw_PuckerLipLo
	ResetState();
	SetState( "OpenJaw_PuckerLips" );
	Select( gSEL_upLip);
	Interp( "OpenJaw", 1.0, 2.0, "STRAIGHT" );
	Select( gSEL_loLip);
	Interp( "OpenJaw_PuckerLips", 1.0, 0.75, "STRAIGHT" );
	Select( gSEL_centerLine);
	GrowSelection( 2) ;
	Select( "INTERSECT", gSEL_upLip);
	Interp( "OpenJaw", 1.0, .5, "STRAIGHT");
	Select( gSEL_mouthCorner);
	Interp( "OpenJaw_PuckerLips", 0.5, 1, "SPIKE");
	SaveDelta( "OpenJaw_PuckerLipLo" );
	
	-- create combination OpenJaw_PuckerLipLo_PuckerLipUp
	ResetState();
	SetState( "OpenJaw_PuckerLips" );
	SaveDelta( "OpenJaw_PuckerLipLo_PuckerLipUp" );
	
	-- create combination OpenLips_PuckerLipUp
	ResetState();
	SetState( "OpenLips_PuckerLips" );
	Select( gSEL_loLip);
	Interp( "OpenLips", 1.0, 2.0, "STRAIGHT" );
	Select( gSEL_upLip);
	Interp( "OpenLips_PuckerLips", 1.0, 0.75, "STRAIGHT" );
	Select( gSEL_centerLine);
	GrowSelection( 2) ;
	Select( "INTERSECT", gSEL_loLip);
	Interp( "OpenLips", 1.0, .5, "STRAIGHT");
	Select( gSEL_mouthCorner);
	Interp( "OpenLips_PuckerLips", 0.5, 1, "SPIKE");
	SaveDelta( "OpenLips_PuckerLipUp" );
	
	-- create combination OpenLips_PuckerLipLo
	ResetState();
	SetState( "OpenLips_PuckerLips" );
	Select( gSEL_upLip);
	Interp( "OpenLips", 1.0, 2.0, "STRAIGHT" );
	Select( gSEL_loLip);
	Interp( "OpenLips_PuckerLips", 1.0, 0.75, "STRAIGHT" );
	Select( gSEL_centerLine);
	GrowSelection( 2) ;
	Select( "INTERSECT", gSEL_upLip);
	Interp( "OpenLips", 1.0, .5, "STRAIGHT");
	Select( gSEL_mouthCorner);
	Interp( "OpenLips_PuckerLips", 0.5, 1, "SPIKE");
	SaveDelta( "OpenLips_PuckerLipLo" );
	
	-- create combination split OpenLips_PuckerLipLo_PuckerLipUp
	ResetState();
	SetState( "OpenLips_PuckerLips" );
	SaveDelta( "OpenLips_PuckerLipLo_PuckerLipUp" );
		

	-- create combination OpenJaw_OpenLips_PuckerLipLo_PuckerLipUp
	ResetState();
	SetState( "OpenLips_PuckerLipLo_PuckerLipUp" );
	Select( "ALL" );
	Add ( "OpenJaw", 1 );
	Select( "ALL" );
	Interp( "OpenJaw_PuckerLips", 0.5 );
	SaveDelta( "OpenJaw_OpenLips_PuckerLipLo_PuckerLipUp" );
	

	
	-- create combination OpenJaw_OpenLips_PuckerLipLo
	ResetState();
	SetState( "OpenJaw_OpenLips" );
	Select( gSEL_loLip);
	Interp( "OpenJaw_PuckerLips", 1.0, 2.0, "STRAIGHT" );
	Select( gSEL_upLip);
	Interp( "OpenJaw_OpenLips", 1.0, 0.75, "STRAIGHT" );
	Select( gSEL_centerLine);
	GrowSelection( 4) ;
	Select( "INTERSECT", gSEL_loLip);
	Interp( "OpenJaw_OpenLips_PuckerLipLo_PuckerLipUp", 1.0, .5, "STRAIGHT");
	Select( gSEL_mouthCorner);
	Interp( "OpenJaw_OpenLips_PuckerLipLo_PuckerLipUp", 0.5, 3, "SPIKE");
	SaveDelta( "OpenJaw_OpenLips_PuckerLipLo" );
	
	-- create combination OpenJaw_OpenLips_PuckerLipUp
	ResetState();
	SetState( "OpenJaw_OpenLips" );
	Select( gSEL_upLip);
	Interp( "OpenJaw_PuckerLips", 1.0, 2.0, "STRAIGHT" );
	Select( gSEL_loLip);
	Interp( "OpenJaw_OpenLips", 1.0, 0.75, "STRAIGHT" );
	Select( gSEL_centerLine);
	GrowSelection( 4) ;
	Select( "INTERSECT", gSEL_upLip);
	Interp( "OpenJaw_OpenLips_PuckerLipLo_PuckerLipUp", 1.0, .5, "STRAIGHT");
	Select( gSEL_mouthCorner);
	Interp( "OpenJaw_OpenLips_PuckerLipLo_PuckerLipUp", 0.5, 3, "SPIKE");
	SaveDelta( "OpenJaw_OpenLips_PuckerLipUp" );
	
	-- create combo split Dimple_PuckerLipLo
	ResetState();
	SetState( "PuckerLipLo" );
	Select( "ALL" );
	Add( "Dimple", 0.25 );
	SaveDelta( "Dimple_PuckerLipLo" );
	
	-- create combo split Dimple_PuckerLipLo
	ResetState();
	SetState( "PuckerLipUp" );
	Select( "ALL" );
	Add( "Dimple", 0.25 );
	SaveDelta( "Dimple_PuckerLipUp" );
	
	-- create combo split Dimple_PuckerLipLo
	ResetState();
	SetState( "PuckerLips" );
	Select( "ALL" );
	Add( "Dimple", 0.33 );
	SaveDelta( "Dimple_PuckerLipLo_PuckerLipUp" );
	
	
	-- create combo split Dimple_OpenLips_PuckerLipLo_PuckerLipUp
	ResetState();
	SetState( "Dimple_OpenLips_PuckerLips" );
	SaveDelta( "Dimple_OpenLips_PuckerLipLo_PuckerLipUp" );
	
	
	-- create control split FunnelLipUp from FunnelLips 
	ResetState();
	SetState( "FunnelLips" );
	Select( gSEL_loLip);
	Interp( base, 1.0, 2.0, "STRAIGHT" );
	Select( gSEL_upLip);
	Interp( "FunnelLips", 1.0, 0.75, "STRAIGHT" );
	Select( gSEL_centerLine);
	GrowSelection( 2) ;
	Select( "INTERSECT", gSEL_loLip);
	Interp( base, 1.0, .75, "STRAIGHT");
	Select( gSEL_mouthCorner);
	Interp( "FunnelLips", 0.25, 1.5, "SPIKE");
	SaveDelta( "FunnelLipUp" );
	
	-- create control split FunnelLipLo from FunnelLips
	ResetState();
	SetState( "FunnelLips" );
	Select( gSEL_upLip);
	Interp( base, 1.0, 2.0, "STRAIGHT" );
	Select( gSEL_loLip);
	Interp( "FunnelLips", 1.0, 0.75, "STRAIGHT" );
	Select( gSEL_centerLine);
	GrowSelection( 3) ;
	Select( "INTERSECT", gSEL_upLip);
	Interp( base, 1.0, .5, "STRAIGHT");
	Select( gSEL_mouthCorner);
	Interp( "FunnelLips", 0.25, 1, "SPIKE");
	SaveDelta( "FunnelLipLo" );
	
	-- create combination split FunnelLips
	ResetState();
	SetState( "FunnelLips" );
	SaveDelta( "FunnelLipLo_FunnelLipUp" );
	
	-- create combo split Dimple_FunnelLipLo
	ResetState();
	SetState( "Dimple_FunnelLips" );
	Select( gSEL_upLip);
	Interp( "Dimple", 1.0, 0.5, "SPIKE" );
	SaveDelta( "Dimple_FunnelLipLo" );
	
	-- create combo split Dimple_FunnelLipUp
	ResetState();
	SetState( "Dimple_FunnelLips" );
	Select( gSEL_loLip);
	Interp( "Dimple", 1.0, 0.5, "SPIKE" );
	SaveDelta( "Dimple_FunnelLipUp" );
	
	-- create combo split Dimple_FunnelLipLo_FunnelLipUp
	ResetState();
	SetState( "Dimple_FunnelLips" );
	SaveDelta( "Dimple_FunnelLipLo_FunnelLipUp" );
	
	-- create combination OpenJaw_FunnelLipUp
	ResetState();
	SetState( "FunnelLips_OpenJaw" );
	Select( gSEL_loLip);
	Interp( "OpenJaw", 1.0, 2.0, "STRAIGHT" );
	Select( gSEL_upLip);
	Interp( "FunnelLips_OpenJaw", 1.0, 0.75, "STRAIGHT" );
	Select( gSEL_centerLine);
	GrowSelection( 2) ;
	Select( "INTERSECT", gSEL_loLip);
	Interp( "OpenJaw", 1.0, .5, "STRAIGHT");
	Select( gSEL_mouthCorner);
	Interp( "FunnelLips_OpenJaw", 0.5, 1, "SPIKE");
	SaveDelta( "OpenJaw_FunnelLipUp" );
	
	-- create combination OpenJaw_FunnelLipLo
	ResetState();
	SetState( "FunnelLips_OpenJaw" );
	Select( gSEL_upLip);
	Interp( "OpenJaw", 1.0, 2.0, "STRAIGHT" );
	Select( gSEL_loLip);
	Interp( "FunnelLips_OpenJaw", 1.0, 0.75, "STRAIGHT" );
	Select( gSEL_centerLine);
	GrowSelection( 2) ;
	Select( "INTERSECT", gSEL_upLip);
	Interp( "OpenJaw", 1.0, .5, "STRAIGHT");
	Select( gSEL_mouthCorner);
	Interp( "FunnelLips_OpenJaw", 0.5, 0.5, "SPIKE");
	SaveDelta( "OpenJaw_FunnelLipLo" );
	
	-- create combination OpenJaw_FunnelLipLo_FunnelLipUp
	ResetState();
	SetState( "FunnelLips_OpenJaw" );
	SaveDelta( "OpenJaw_FunnelLipLo_FunnelLipUp" );
	
	
	-- create control split PuffLipUp from PuffLips
	ResetState();
	SetState( "PuffLips" );
	Select( gSEL_loLip);
	Interp( base, 1.0);
	Select( gSEL_upLip);
	Interp( "PuffLips", 1.0, 0.75, "STRAIGHT" );
	Select( gSEL_centerLine);
	GrowSelection( 1) ;
	Select( "INTERSECT", gSEL_loLip);
	Interp( base, 1.0, .75, "STRAIGHT");
	Select( gSEL_loLip);
	Interp( base, 1.0);
	SaveDelta( "PuffLipUp" );
	
	-- create control split PuffLipLo from PuffLips
	ResetState();
	SetState( "PuffLips" );
	Select( gSEL_upLip);
	Interp( base, 1.0, 2.0, "STRAIGHT" );
	Select( gSEL_loLip);
	Interp( "PuffLips", 1.0, 0.75, "STRAIGHT" );
	Select( gSEL_centerLine);
	GrowSelection( 2) ;
	Select( "INTERSECT", gSEL_upLip);
	Interp( base, 1.0, .5, "STRAIGHT");
	Select( gSEL_upLip);
	Interp( base, 1.0);

	Select( "PressNose");
	Interp( base, 1.0);
	SaveDelta( "PuffLipLo" );

	-- create control split PuffLipLo from PuffLips
	SetState( "PuffLips" );
	SaveDelta( "PuffLipLo_PuffLipUp" );
	
	-- split PuckerLips_PuffLips into lower combination
	ResetState();
	SetState( "PuckerLipLo" );
	Select( gSEL_loLip);
	Interp( "PuckerLips_PuffLips", 1.0);
	Select( gSEL_centerLine);
	GrowSelection( 2) ;
	Select( "INTERSECT", gSEL_upLip);
	Interp( "PuckerLipLo", 1.0, .5, "STRAIGHT");
	SaveDelta( "PuckerLipLo_PuffLiplo" );	
	
	-- split PuckerLips_PuffLips into lower combination
	ResetState();
	SetState( "PuckerLipLo" );
	Select( "ALL");
	Add ( "PuffLipUp", 1.0 );
	SaveDelta( "PuffLipUp_PuckerLipLo" );
	
	-- split PuckerLips_PuffLips into lower combination
	ResetState();
	SetState( "PuckerLipUp" );
	Select( "ALL");
	Add ( "PuffLipLo", 1.0 );
	SaveDelta( "PuffLipLo_PuckerLipUp" );
	
	-- split PuckerLips_PuffLips into lower combination
	ResetState();
	SetState( "PuckerLipLo_PuckerLipUp" );
	Select( "ALL");
	Add ( "PuffLipUp", 1.0 );
	Select( gSEL_centerLine);
	GrowSelection( 2 ) ;
	Select( "INTERSECT", gSEL_upLip);
	Interp( "PuckerLips_PuffLips", 1.0, 1.5, "SPIKE");
	Select( gSEL_centerLine);
	GrowSelection( 2 ) ;
	Select( "INTERSECT", gSEL_loLip);
	Interp( "PuckerLipLo_PuckerLipUp", 1.0, 0.5, "DOME");
	SaveDelta( "PuffLipUp_PuckerLipLo_PuckerLipUp" );
	
	-- split PuckerLips_PuffLips into upper combination
	ResetState();
	SetState( "PuckerLipUp" );
	Select( gSEL_upLip);
	Interp( "PuckerLips_PuffLips", 1.0, 0.75, "STRAIGHT" );
	Select( gSEL_centerLine);
	GrowSelection( 1) ;
	Select( "INTERSECT", gSEL_loLip);
	Interp( "PuckerLipUp", 1.0, .75, "STRAIGHT");
	Select( "ALL");
	Interp( "PuckerLips_PuffLips", .3 );
	Select( gSEL_loLip);
	Interp( "PuffLipUp_PuckerLipLo_PuckerLipUp", .5 );
	SaveDelta( "PuckerLipUp_PuffLipUp" );
	
	
	-- split PuckerLips_PuffLips into lower combination
	ResetState();
	SetState( "PuckerLipLo_PuckerLipUp" );
	Select( "ALL");
	Add ( "PuffLipLo", 1.0 );
	Select( gSEL_centerLine);
	GrowSelection( 2 ) ;
	Select( "INTERSECT", gSEL_loLip);
	Interp( "PuckerLips_PuffLips", 1.0, 1.5, "SPIKE");
	SaveDelta( "PuffLipLo_PuckerLipLo_PuckerLipUp" );
	
	-- split PuckerLips_PuffLips into lower combination
	ResetState();
	SetState( "PuffLipUp_PuckerLipUp" );
	Select( "ALL");
	Add ( "PuffLipLo", 1.0 );
	SaveDelta( "PuffLipLo_PuffLipUp_PuckerLipUp" );
	
	-- split PuckerLips_PuffLips into lower combination
	ResetState();
	SetState( "PuffLipLo_PuckerLipLo" );
	Select( "ALL");
	Add ( "PuffLipUp", 1.0 );
	SaveDelta( "PuffLipLo_PuffLipUp_PuckerLipLo" );
	
	-- split PuckerLips_PuffLips into lower combination
	ResetState();
	SetState( "PuckerLips_PuffLips" );
	SaveDelta( "PuffLipLo_PuffLipUp_PuckerLipLo_PuckerLipUp" );
	
	-- rename combination PuckerLips_RaiseChin_SneerNose
	ResetState();
	SetState( "PuckerLips_RaiseChin_SneerNose" );
	SaveDelta( "PuckerLipUp_RaiseChin_SneerNose" );
	
	-- rename combination PuckerLips_RaiseChin
	ResetState();
	SetState( "PuckerLips_RaiseChin" );
	SaveDelta( "PuckerLipUp_RaiseChin" );
	
	-- OpenLips_OpenUpperLip
	ResetState();
	SetState( "OpenUpperLip" );
	Select( gSEL_centerLine);
	GrowSelection( 3 ) ;
	Select( "INTERSECT", gSEL_loLip);
	Interp( "OpenLips", 1.0, .15, "STRAIGHT");
	SaveDelta( "OpenLips_OpenUpperLip" );
	
	-- OpenLips_OpenLowerLip
	ResetState();
	SetState( "OpenLowerLip" );
	Select( gSEL_upLip);
	Interp( "OpenLips", 1.0, .25, "STRAIGHT");
	SaveDelta( "OpenLips_OpenLowerLip" );

	-- create combination OpenJaw_OpenLips_OpenUpperLip
	ResetState();
	SetState( "OpenJaw_OpenLowerLip_OpenUpperLip" );
	Select( gSEL_centerLine);
	Select( "INTERSECT", gSEL_loLip);
	GrowSelection( 3) ;
	Select( "ADD", gSEL_throat);
	Interp( "OpenJaw_OpenLips", 1.0, 2.5, "DOME" );
	Select( gSEL_centerLine);
	Select( "INTERSECT", gSEL_upLip);
	GrowSelection( 3) ;	
	Interp( "OpenJaw_OpenLowerLip_OpenUpperLip", 1.0, 1.5, "DOME" );
	SaveDelta( "OpenJaw_OpenLips_OpenUpperLip" );
		
	-- create combination OpenJaw_OpenLips_OpenLowerLip
	ResetState();
	SetState( "OpenJaw_OpenLowerLip_OpenUpperLip" );
	Select( gSEL_centerLine);
	Select( "INTERSECT", gSEL_upLip);
	GrowSelection( 3) ;
	Select( "ADD", "BlowNostril");
	Interp( "OpenJaw_OpenLips", 1.0, 2.5, "DOME" );
	SaveDelta( "OpenJaw_OpenLips_OpenLowerLip" );
	
	-- create OpenJaw_OpenUpperLip
	ResetState();
	SetState( "OpenJaw_OpenLips_OpenUpperLip" );
	Select( gSEL_centerLine);
	Select( "INTERSECT", gSEL_loLip);
	GrowSelection( 3) ;
	Interp( "OpenJaw", 1.0, 1.5, "SPIKE" );
	SaveDelta( "OpenJaw_OpenUpperLip" );

	-- create OpenJaw_OpenLowerLip
	ResetState();
	SetState( "OpenJaw_OpenLips_OpenLowerLip" );
	Select( gSEL_centerLine);
	Select( "INTERSECT", gSEL_upLip);
	GrowSelection( 3) ;
	Interp( "OpenJaw", 1.0, 2, "BELL" );
	Select( gSEL_centerLine);
	Select( "INTERSECT", gSEL_loLip);
	GrowSelection( 3 ) ;
	Interp( "OpenJaw_OpenLowerLip_OpenUpperLip", .75, .08, "bell" );
	GrowSelection( 2) ;
	Interp( "OpenJaw_OpenLowerLip_OpenUpperLip", 0.5 );
	SaveDelta( "OpenJaw_OpenLowerLip" );
	
	
	-- OpenUpperLip_Platysmus
	ResetState();
	SetState( "OpenUpperLip");
	Select ( "ALL" );
	Select ( "SUBTRACT", "OpenUpperLip" );
	Add ( "Platysmus", 1.0, 1.0, "Spike" );
	Select ( gSEL_mouthCorner );
	GrowSelection ( 2 ); 
	Interp ( "Platysmus", 1.0, 1.5, "STRAIGHT" ) ;
	Select( gSEL_upLip);
	Select ( "INTERSECT", gSEL_centerLine );
	Interp( "OpenUpperLip", 1, 2.5, "STRAIGHT");
	Select( gSEL_loLip);
	Interp ( "Platysmus", 1.0 ) ;
	SaveDelta( "OpenUpperLip_Platysmus" );
	
	-- OpenLowerLip_Platysmus
	ResetState();
	SetState( "OpenLowerLip");
	Select ( "ALL" );
	Add ( "Platysmus", 1);
	Select( gSEL_loLip);
	Interp( "OpenLowerLip_OpenUpperLip_SmileFull_Platysmus", 1);
	Select ( gSEL_mouthCorner );
	Interp ( "Platysmus", 1.0, 2.0, "spike") ;
	SaveDelta( "OpenLowerLip_Platysmus" );
	
	-- OpenLowerLip_OpenUpperLip_Platysmus
	ResetState();
	SetState( "OpenUpperLip_Platysmus");
	Select( gSEL_loLip);
	Interp( "OpenLowerLip_Platysmus", 1);
	Select ( gSEL_mouthCorner );
	Interp( "OpenLowerLip_Platysmus", 1);
	GrowSelection ( 2 ); 
	Interp( "OpenLowerLip_Platysmus", 0.5);
	SaveDelta( "OpenLowerLip_OpenUpperLip_Platysmus" );
	
	-- OpenUpperLip_SmileFull
	ResetState();
	SetState( "SmileFull" );
	Select( gSEL_upLip);
	Add ( "OpenUpperLip", 1);
	Select ( gSEL_mouthCorner );
	Interp ( "SmileFull", 1.0, .5, "STRAIGHT" ) ;
	Select( gSEL_upLip);
	Select( "Add", "SneerNose");
	Interp( "OpenLowerLip_OpenUpperLip_SmileFull_Platysmus", 1);
	Select( gSEL_loLip);
	GrowSelection( 2 );
	Interp( "SmileFull", 1);	
	SaveDelta( "OpenUpperLip_SmileFull" );
	
	-- OpenLowerLip_SmileFull
	ResetState();
	SetState( "SmileFull" );
	Select ( gSEL_loLip );
	Select( "INTERSECT", gSEL_centerLine);
	GrowSelection ( 1 ) ;
	Add ( "OpenLowerLip", 1, 1, "BELL");
	Select( gSEL_loLip);
	Interp( "OpenLowerLip_OpenUpperLip_SmileFull_Platysmus", .25, 2, "SPIKE");
	Select( gSEL_upLip);
	Interp( "SmileFull", 1 );
	SaveDelta( "OpenLowerLip_SmileFull" )
	
	-- OpenLowerLip_OpenUpperLip_SmileFull
	ResetState();
	SetState( "OpenLowerLip_SmileFull" );
	Select ("All");
	Select( gSEL_upLip);
	Select( "Add", "SneerNose");
	Interp( "OpenUpperLip_SmileFull", 1 );
	SaveDelta( "OpenLowerLip_OpenUpperLip_SmileFull" );
	Select ( gSEL_loLip );
	Select( "INTERSECT", gSEL_centerLine);
	GrowSelection ( 4 ) ;
	Interp( "OpenLowerLip_OpenUpperLip_SmileFull_Platysmus", 0.33, 0.75, "DOME");
	Select ( gSEL_loLip );
	Select( "INTERSECT", gSEL_centerLine);
	Interp( "OpenLowerLip_OpenUpperLip_SmileFull", 1, 0.5, "SPIKE");
	Select( gSEL_mouthCorner);
	Interp( "OpenUpperLip_SmileFull", 0.33, 2, "BELL" );
	SaveDelta( "OpenLowerLip_OpenUpperLip_SmileFull" );
	
	
	-- OpenLips_OpenUpperLip_SmileFull
	ResetState();
	SetState( "OpenUpperLip_SmileFull" );
	Select( gSEL_loLip);
	Interp( "OpenLips_SmileFull", 1.0);
	Select( gSEL_mouthCorner);
	Interp( "OpenLips_SmileFull", 1.0);
	Select( gSEL_mouthCorner);
	Interp( "OpenUpperLip_SmileFull", 1.0, 1.0, "STRAIGHT");		
	SaveDelta( "OpenLips_OpenUpperLip_SmileFull" );
	
	-- OpenLips_OpenLowerLip_SmileFull
	ResetState();
	SetState( "OpenLowerLip_SmileFull" );
	Select( gSEL_upLip);
	Interp( "OpenLips_SmileFull", 1.0);
	Select( gSEL_mouthCorner);
	Interp( "OpenLips_SmileFull", 1.0);
	Select( gSEL_mouthCorner);
	Interp( "OpenLowerLip_SmileFull", 1.0, 1.0, "STRAIGHT");		
	SaveDelta( "OpenLips_OpenLowerLip_SmileFull" );
	
	
	-- SmileFull_Platysmus
	ResetState();
	SetState( "SmileFull" );
	Select( "ALL");
	Interp ("Platysmus", 0.5 );
	SaveDelta( "SmileFull_Platysmus" );
	
	-- Platysmus_RaiseChin
	ResetState();
	SetState( "Platysmus" );
	Select( "ALL");
	Add ("RaiseChin", 1 );
	Select ( gSEL_mouthCorner );
	GrowSelection ( 3 );
	Interp ( "Platysmus", 1.0, 0.6, "SPIKE" ) ;
	SaveDelta( "Platysmus_RaiseChin" );
	
	-- JutUpperLip_Platysmus_RaiseChin
	ResetState();
	SetState( "JutUpperLip_RaiseChin" );
	Select ("ALL");
	Add ("Platysmus", 1 );
	Select ( gSEL_mouthCorner );
	GrowSelection ( 2 );
	Interp ( "Platysmus", 1.0, 0.4, "SPIKE" ) ;
	Select ("ALL" );
	Interp ( "JutUpperLip_RaiseChin", .2 ) ;
	Interp ( "Platysmus", .2 ) ;
	SaveDelta( "JutUpperLip_Platysmus_RaiseChin" );
	
	-- OpenUpperLip_SmileFull_Platysmus
	ResetState();
	SetState( "OpenLowerLip_OpenUpperLip_SmileFull_Platysmus" );
	Select( gSEL_loLip);
	Interp ( "SmileFull_Platysmus", 1.0, 1.3, "SPIKE" ) ;
	SaveDelta( "OpenUpperLip_SmileFull_Platysmus" );
	
	-- OpenLowerLip_SmileFull_Platysmus
	ResetState();
	SetState( "SmileFull_Platysmus" );
	Select( gSEL_loLip);
	Interp ( "OpenLowerLip_OpenUpperLip_SmileFull_Platysmus", 1.0, 1, "STRAIGHT" ) ;
	Select( gSEL_upLip);
	Interp ( "SmileFull_Platysmus", 1.0, 0.9, "STRAIGHT" ) ;
	Select ( gSEL_mouthCorner );
	GrowSelection ( 1 );
	Interp ( "SmileFull_Platysmus", 0.3, 0.1, "DOME" ) ;
	SaveDelta( "OpenLowerLip_SmileFull_Platysmus" );
	
	-- OpenJaw_Platysmus
	ResetState();
	SetState( "OpenJaw");
	Add ( "Platysmus" , 1.0 );
	Select ( gSEL_upLip );
	Select( "INTERSECT", gSEL_centerLine);
	Interp ( "OpenJaw", 0.75, 3.0, "DOME" ) ;
	SaveDelta( "OpenJaw_Platysmus" );
	
	-- OpenJaw_OpenLowerLip_Platysmus
	ResetState();
	SetState( "OpenJaw_Platysmus");
	Select ( gSEL_loLip );
	Select ( "Add", gSEL_throat );
	Interp ( "OpenJaw_OpenLowerLip_OpenUpperLip_Platysmus_SmileFull", 1.0 )
	Interp ( "OpenJaw_OpenLowerLip_OpenUpperLip_Platysmus_SmileFull", 1.0, 1.5, "SPIKE" )
	Select ( gSEL_upLip );	
	GrowSelection ( 2 ) ;
	Interp ( "OpenJaw_Platysmus", 1.0)	
	SaveDelta( "OpenJaw_OpenLowerLip_Platysmus" );
	
	-- OpenJaw_SmileFull
	ResetState();
	SetState( "OpenJaw");
	Select ( "ALL" );
	Add ( "SmileFull" , 1.0 );
	SaveDelta( "OpenJaw_SmileFull" );
	
	-- OpenJaw_Platysmus_SmileFull
	ResetState();
	SetState( "SmileFull_Platysmus");
	Select ( "ALL" );
	Add ( "OpenJaw" , 1.0 );
	SaveDelta( "OpenJaw_Platysmus_SmileFull" );
	
	-- OpenJaw_OpenLowerLip_SmileFull
	ResetState();
	SetState( "OpenLowerLip_SmileFull");
	Select ( "ALL" );
	Add ( "OpenJaw" , 1.0 );
	Select ( gSEL_loLip );	
	Interp ( "OpenJaw_OpenLowerLip", 0.25, 0.5, "STRAIGHT" )
	SaveDelta( "OpenJaw_OpenLowerLip_SmileFull" );
	Select ( gSEL_centerLine);
	Select ( "SUBTRACT", gSEL_upLip );	
	Interp ( "OpenJaw_OpenLowerLip", 0.75, 3, "STRAIGHT" )
	Select ( gSEL_loLip);
	Select ( "ADD", gSEL_throat );	
	GrowSelection ( 1 );
	Interp ( "OpenJaw_OpenLowerLip_OpenUpperLip", 0.5, 1, "SPIKE");
	Select ( gSEL_upLip);
	Select ( "ADD", "SneerNose" );	
	Interp ( "OpenJaw_SmileFull" , 1.0);
	SaveDelta( "OpenJaw_OpenLowerLip_SmileFull" );
	
	-- OpenJaw_OpenUpperLip_SmileFull
	ResetState();	
	SetState( "OpenJaw_SmileFull" );
	Select ( gSEL_upLip);
	Select ( "ADD", "SneerNose" );	
	Interp ( "OpenUpperLip_SmileFull" , 1.0, 1.0, "SPIKE" );
	Select ( gSEL_loLip);
	GrowSelection ( 2 );
	Interp ( "OpenJaw_SmileFull", 1.0 )
	Select ( gSEL_mouthCorner);
	Interp ( "SmileFull", 1, 3, "SPIKE" )	
	Select ( gSEL_upLip);
	Select ( "ADD", "SneerNose" );	
	Interp ( "OpenUpperLip_SmileFull" , 1.0);
	SaveDelta( "OpenJaw_OpenUpperLip_SmileFull" );
	
	
	-- OpenJaw_OpenLowerLip_OpenUpperLip_SmileFull
	ResetState();
	SetState( "OpenJaw_OpenLowerLip_OpenUpperLip_Platysmus_SmileFull");
	Select ( gSEL_loLip );	
	Select ( "Add", gSEL_throat);
	Interp ( "OpenJaw_OpenLowerLip_SmileFull", 1, 3, "SPIKE" )	
	Select ( gSEL_mouthCorner);
	Interp ( "OpenJaw_OpenLowerLip_OpenUpperLip_Platysmus_SmileFull", 0.5, 2, "STRAIGHT" )	;
	Interp ( "OpenJaw_OpenLowerLip_SmileFull", 0.5, 2, "STRAIGHT" )	;
	Select ( "CheekV");
	Interp ( "OpenJaw_OpenLowerLip_OpenUpperLip_Platysmus_SmileFull", .75, 1.75, "SPIKE" );	
	Select ( gSEL_loLip);
	Select ( "ADD", gSEL_throat );	
	GrowSelection ( 3 );
	Interp ( "OpenJaw_OpenLowerLip_SmileFull", 1);
	GrowSelection ( 1 );
	Interp ( "OpenJaw_OpenLowerLip_SmileFull", 0.5);
	Select ( gSEL_mouthCorner);
	Interp ( "OpenLowerLip_OpenUpperLip_SmileFull", 1, 2, "SPIKE" )	
	Select ( gSEL_upLip );
	Select ( "ADD", "SneerNose" );	
	Interp ( "OpenJaw_OpenUpperLip_SmileFull", 1.0, 0.5, "SPIKE" );
	Select ( "All" );
	Interp( "OpenJaw_OpenLowerLip_OpenUpperLip_Platysmus_SmileFull", 0.35);
	SaveDelta( "OpenJaw_OpenLowerLip_OpenUpperLip_SmileFull" );
	
	
	-- OpenJaw_OpenLowerLip_SmileFull_Platysmus
	ResetState();
	SetState( "OpenJaw_OpenLowerLip_OpenUpperLip_Platysmus_SmileFull");
	Select ( gSEL_upLip );	
	Select ( "ADD", "SneerNose" );	
	Select ( "ADD", "CheekV" );	
	Select ( "ADD", gSEL_mouthCorner );			
	Interp ( "OpenJaw_Platysmus_SmileFull", 1.0 );
	GrowSelection ( 1 );
	Interp ( "OpenJaw_Platysmus_SmileFull", 0.5 );
	SaveDelta( "OpenJaw_OpenLowerLip_SmileFull_Platysmus" );

	
	-- OpenJaw_OpenUpperLip_Platysmus
	ResetState();
	Select ( "All" );
	SetState( "OpenJaw_OpenUpperLip");
	Select ( gSEL_throat);
	GrowSelection ( 2 );
	Interp ( "OpenJaw_Platysmus", 1.0, 3.0, "SPIKE");
	Select ( gSEL_centerLine);
	Select ( "SUBTRACT", gSEL_loLip );	
	Interp ( "OpenJaw_OpenUpperLip", 1.0, 0.5, "DOME");	
	SaveDelta( "OpenJaw_OpenUpperLip_Platysmus" );
	
	-- OpenJaw_OpenLowerLip_OpenUpperLip_Platysmus
	ResetState();
	SetState( "OpenJaw_OpenLowerLip_Platysmus");
	Select ( gSEL_upLip );	
	Select ( "ADD", "SneerNose" );	
	Interp ( "OpenJaw_OpenLowerLip_OpenUpperLip", 1.0 );
	Interp ( "OpenJaw_OpenLowerLip_OpenUpperLip", 1.0, 2.0, "STRAIGHT" );
	Select ( "All" );
	Interp ( "OpenJaw_OpenLowerLip_OpenUpperLip_Platysmus_SmileFull", 0.33);
	Select ( gSEL_mouthCorner);
	GrowSelection ( 2 );
	Interp ( "OpenJaw_Platysmus", 0.33, 1, "STRAIGHT");
	Select ( gSEL_throat);
	GrowSelection ( 2 );	
	Interp ( "OpenJaw_OpenLowerLip_OpenUpperLip_Platysmus_SmileFull", 1, 2.75, "STRAIGHT" );	
	Select ( gSEL_mouthCorner);
	Select ( gSEL_upLip);	
	Interp ( "OpenJaw_OpenUpperLip_Platysmus", 0.5, 2.0, "DOME" );	
	SaveDelta( "OpenJaw_OpenLowerLip_OpenUpperLip_Platysmus" );
	
	-- OpenJaw_OpenUpperLip_SmileFull_Platysmus
	ResetState();
	Select ( "All" );
	SetState( "OpenJaw_OpenLowerLip_OpenUpperLip_Platysmus_SmileFull");
	Select ( gSEL_loLip);
	Select ( "Add", gSEL_throat);
	Interp ( "OpenJaw_Platysmus_SmileFull", 1.0, 2.0, "STRAIGHT");	
	SaveDelta( "OpenJaw_OpenUpperLip_SmileFull_Platysmus" );
	

	
	-- create SlideJawL.
	ResetState();
	Select ( "All" );
	SetState( "OpenJaw_OpenLowerLip_OpenUpperLip_Platysmus_SmileFull");
	Select ( gSEL_jawEffect);
	Select ( "Add", gSEL_loLip);
	Select ( "Add", gSEL_jaw);	
	Translate( 0.4, 0, 0, 1.5, "BELL" ); 
	Select ( gSEL_skull );
	Interp ( "base", 1.0 );
	Select ( gSEL_upLip );
	Select ( "ADD", "SneerNose" );
	Interp ( "OpenJaw_OpenLowerLip_OpenUpperLip_Platysmus_SmileFull", 0.5, .2, "SPIKE");
	Select ( "SneerNose" );	
	Interp ( "OpenJaw_OpenLowerLip_OpenUpperLip_Platysmus_SmileFull", .8, .3, "SPIKE");	
	Select ( "All" );
	AddCorrected ( "OpenJaw_OpenLowerLip_OpenUpperLip_Platysmus_SmileFull", -1.0);	
	SaveDelta( "SlideJawL" );
	
	-- create SlideJawR.
	ResetState();
	Select ( "All" );
	SetState( "OpenJaw_OpenLowerLip_OpenUpperLip_Platysmus_SmileFull");
	Select ( gSEL_jawEffect);
	Select ( "Add", gSEL_loLip);
	Select ( "Add", gSEL_jaw);	
	Translate( -0.4, 0, 0, 1.5, "BELL" ); 
	Select ( gSEL_skull);
	Interp ( "base", 1.0);
	Select ( gSEL_upLip);
	Select ( "ADD", "SneerNose" );
	Interp ( "OpenJaw_OpenLowerLip_OpenUpperLip_Platysmus_SmileFull", 0.5, .2, "SPIKE");
	Select ( "SneerNose" );	
	Interp ( "OpenJaw_OpenLowerLip_OpenUpperLip_Platysmus_SmileFull", .8, .3, "SPIKE");	
	Select ( "All" );
	AddCorrected ( "OpenJaw_OpenLowerLip_OpenUpperLip_Platysmus_SmileFull", -1.0);	
	SaveDelta( "SlideJawR" );

	-- create combination CheekV_PuckerLips_RaiseChin
	ResetState();
	SetState( "PuckerLips_RaiseChin" );
	Select ("ALL");
	Add ( "CheekV", 1.0 );
	if upperFaceSwitch then
		Select( gSEL_eyes );
		GrowSelection (4);
		Interp( "CheekV" , 1.0, 3, "SPIKE" );
	end
	SaveDelta( "CheekV_PuckerLipUp_RaiseChin" );
	
	-- CompressLips_JutUpperLip_RaiseChin
	ResetState();
	SetState( "JutUpperLip_RaiseChin" );
	Select ("ALL");
	Add ( "CompressLips", 0.75 );
	SaveDelta( "CompressLips_JutUpperLip_RaiseChin" );
	
	DeleteDelta( "PuckerLips" );
	DeleteDelta( "OpenJaw_PuckerLips" );
	DeleteDelta( "OpenLips_PuckerLips" );
	DeleteDelta( "Dimple_OpenLips_PuckerLips" );
	DeleteDelta( "FunnelLips" );
	DeleteDelta( "FunnelLips_OpenJaw" );
	DeleteDelta( "Dimple_FunnelLips" );

end

