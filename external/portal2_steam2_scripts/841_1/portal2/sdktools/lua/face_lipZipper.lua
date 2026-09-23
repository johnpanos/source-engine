---- this script recquires the following input OBJ shapes

gSEL_centerLine 	= "SELECT-centerLine";
gSEL_lipcontacts	= "SELECT-lipcontacts";

if selectSeparator then
	gSEL_centerLine 	= "SELECT" .. selectSeparator .. "centerLine";
	gSEL_lipcontacts	= "SELECT" .. selectSeparator .. "lipcontacts";
end

--------------------------------------
--            ZipLip             --
--------------------------------------


-- define lipzipper function
function CreateZipLips( deltaList, baseReferenceShape) 

	-- do the inner loop
	for i,ZipperTarget in ipairs( deltaList ) do
		ResetState();
		Select ("ALL");
		SetState ( ZipperTarget );
		SaveDelta( ZipperTarget .. "_ZipLip00" );
		
		ResetState();
		Select ("ALL");
		SetState ( ZipperTarget );
		Select( gSEL_lipcontacts);
		Interp( (baseReferenceShape) , 1, 1.5, "SPIKE" );
		SaveDelta( ZipperTarget .. "_ZipLip100" );
		
		ResetState();
		Select ("ALL");
		SetState ( ZipperTarget .. "_ZipLip100" );
		Select( gSEL_centerLine);
		Interp( ZipperTarget , 1, 0.5, "SPIKE" );
		SaveDelta( ZipperTarget .. "_ZipLip75" );
		
		ResetState();
		Select ("ALL");
		SetState ( ZipperTarget .. "_ZipLip100" );
		Select( gSEL_centerLine);
		GrowSelection( 1) ;
		Interp( ZipperTarget , 1, 0.5, "SPIKE" );
		SaveDelta( ZipperTarget .. "_ZipLip50" );
		
		ResetState();
		Select ("ALL");
		SetState ( ZipperTarget .. "_ZipLip100" );
		Select( gSEL_centerLine);
		GrowSelection( 2) ;
		Interp( ZipperTarget , 1, 0.5, "SPIKE" );
		SaveDelta( ZipperTarget .. "_ZipLip25" );
	end
end

-- define empty ziplip controls.
Select ("ALL");
ResetState();
SaveDelta( "ZipLip00" );
SaveDelta( "ZipLip25" );
SaveDelta( "ZipLip50" );
SaveDelta( "ZipLip75" );
SaveDelta( "ZipLip100" );

-- define combinations that will use ziplips from relative bases.
TargetList = {
	"OpenLips",
	"OpenUpperLip",
	"OpenLowerLip",
	"OpenLowerLip_OpenUpperLip",
	"OpenLips_OpenLowerLip",
	"SneerNose",
	"OpenLips_OpenUpperLip"
};
CreateZipLips( TargetList, "base");

TargetList = {
	"OpenLowerLip_OpenUpperLip_SmileFull",
	"OpenUpperLip_SmileFull",
	"OpenLowerLip_SmileFull",
	"OpenLips_SmileFull",
	"OpenLips_OpenLowerLip_SmileFull",
	"OpenLips_OpenUpperLip_SmileFull",
	"SmileFull"
};
CreateZipLips( TargetList, "SmileFull");

TargetList = {
	"OpenJaw",
	"OpenJaw_OpenLips",
	"OpenJaw_OpenLowerLip",
	"OpenJaw_OpenUpperLip",
	"OpenJaw_OpenLips_OpenLowerLip",
	"OpenJaw_OpenLips_OpenUpperLip",
	"OpenJaw_OpenLowerLip_OpenUpperLip"
	
};
CreateZipLips( TargetList, "OpenJaw");

TargetList = {
	"PuckerLipLo_PuckerLipUp",
	"OpenLips_PuckerLipLo_PuckerLipUp",
	"OpenJaw_PuckerLipUp",
	"OpenJaw_PuckerLipLo",
	"OpenJaw_PuckerLipLo_PuckerLipUp",
	"OpenJaw_OpenLips_PuckerLipLo_PuckerLipUp",
	"OpenJaw_OpenLips_PuckerLipLo",
	"OpenJaw_OpenLips_PuckerLipUp"
};
CreateZipLips( TargetList, "OpenJaw_PuckerLipLo_PuckerLipUp");

TargetList = {
	"PuckerLipLo_PuckerLipUp",
	"OpenLips_PuckerLipLo_PuckerLipUp"
};
CreateZipLips( TargetList, "PuckerLipLo_PuckerLipUp");

--TargetList = {
--
--	"OpenJaw",
--	"OpenLips",
--	"PuckerLipLo",
--	"PuckerLipUp",
--	"OpenJaw_OpenLips",
--	"OpenJaw_PuckerLipLo",
--	"OpenJaw_PuckerLipUp",
--	"OpenLips_PuckerLipLo",
--	"OpenLips_PuckerLipUp",
--	"PuckerLipLo_PuckerLipUp",
--	"OpenJaw_OpenLips_PuckerLipLo",
--	"OpenJaw_OpenLips_PuckerLipUp",
--	"OpenJaw_PuckerLipLo_PuckerLipUp",
--	"OpenLips_PuckerLipLo_PuckerLipUp",
--	"OpenJaw_OpenLips_PuckerLipLo_PuckerLipUp"

--};
--CreateZipLips( TargetList, "OpenJaw_OpenLips_PuckerLipLo_PuckerLipUp");




-- group controls
GroupControls( "ZipLips", "ZipLip00", "ZipLip25", "ZipLip50", "ZipLip75", "ZipLip100" );

-- make ziplip control into left and right sides.
SetStereoControl("ZipLips", true );

ComputeNormals();











