//////////////////////////////////////////////////////////////////////////////////////
// ValveSource Character Rig Tools Version v1.2a
// Softimage 2005
//
// Description: various commands for creating Valve character rigs and guides
//////////////////////////////////////////////////////////////////////////////////////

var gToolName = "ValveSource CharacterRig Tools";
var gDebug = false;
var gToolPath = "";

function XSILoadPlugin( in_reg )
{
	Trace( "ValveSourceShadowRig XSILoadPlugin called" );
	
	in_reg.Author = "Softimage";
	in_reg.Name = gToolName;
	in_reg.Major = 1;
	in_reg.Minor = 0;
	in_reg.Email = "xsisdk@softimage.com";
	in_reg.URL = "www.softimage.com";

//	in_reg.RegisterCommand("vstCreateShadowRig", "vstCreateShadowRig");
	in_reg.RegisterCommand("vstCreateMaleRig", "vstCreateMaleRig");
	in_reg.RegisterCommand("vstCreateFemaleRig", "vstCreateFemaleRig");
	in_reg.RegisterCommand("vstCreateGuide", "vstCreateGuide" );
	in_reg.RegisterCommand("vstFixBrokenFootRoll","vstFixBrokenFootRoll");

	return true;
} 

//////////////////////////////////////////////////////////////////////////////////////
// Command: vstFixBrokenFootRoll
//////////////////////////////////////////////////////////////////////////////////////
function vstFixBrokenFootRoll_Execute()
{

	try {
		var scriptedop1 = Dictionary.GetObject("ValveBiped.LToe.kine.local.scriptedop")
	
		deleteobj( scriptedop1  );
		applyop( "BipedRigSteppingAngle", "ValveBiped.LToe;ValveBiped.LBall;ValveBiped.LHeel;ValveBiped.LeftRoll" );
		addexpr( "ValveBiped.LToe.kine.local.BipedRigSteppingAngle.SteppingAngle", "ValveBiped.CharacterControls.LeftSteppingAngle", false);
	} catch (e) {;}
	
	try {
		var scriptedop2 = Dictionary.GetObject("ValveBiped.RToe.kine.local.scriptedop")
	
		deleteobj( scriptedop2 );
		applyop("BipedRigSteppingAngle", "ValveBiped.RToe;ValveBiped.RBall;ValveBiped.RHeel;ValveBiped.RightRoll" );
		addexpr("ValveBiped.RToe.kine.local.BipedRigSteppingAngle.SteppingAngle", "ValveBiped.CharacterControls.RightSteppingAngle", false);
	} catch (e) {;}
	
}

//////////////////////////////////////////////////////////////////////////////////////
// Command :  vstCreateShadowRig
//////////////////////////////////////////////////////////////////////////////////////
//function vstCreateShadowRig_Execute()
//{
//	var CurrentDir = GetToolPath() + "Data\\Scripts\\";
//	
//	ImportModel( GetToolPath() + "Data\\Models\\Biped_Guide.emdl" );
//	
//	Include (CurrentDir + "BipedFromGuide_03_NullShadow.js");
//	Include (CurrentDir + "Rename_ShadowNulls_01.js");
//	Include (CurrentDir + "MatchShadowNulls2OriginalSkeleton_02b.js");
//	Include (CurrentDir + "MakeControls_01.js");
//	Include (CurrentDir + "Organize01.js");
//	deselectAll();
//}

//////////////////////////////////////////////////////////////////////////////////////
// Command :  vstCreateMaleRig
//////////////////////////////////////////////////////////////////////////////////////
function vstCreateMaleRig_Execute(ctx)
{
	ImportModel( GetToolPath() + "Data\\Models\\Male\\ValveBiped.emdl" );
}

//////////////////////////////////////////////////////////////////////////////////////
// Command :  vstCreateFemaleRig
//////////////////////////////////////////////////////////////////////////////////////
function vstCreateFemaleRig_Execute(ctx)
{
	ImportModel( GetToolPath() + "Data\\Models\\Female\\ValveBiped.emdl" );
}

//////////////////////////////////////////////////////////////////////////////////////
// Command :  vstCreateGuide
//////////////////////////////////////////////////////////////////////////////////////
function vstCreateGuide_Execute()
{
	var PreviousAutoInspect = GetUserPref("AutoInspectEnabled");
	SetUserPref( "AutoInspectEnabled", false );
	ImportModel( GetToolPath() + "Data\\Models\\ValveGuide.emdl");
	SetUserPref( "AutoInspectEnabled", PreviousAutoInspect );
}

//////////////////////////////////////////////////////////////////////////////////////
// Private Helper Functions
//////////////////////////////////////////////////////////////////////////////////////
function Include ( oNameScript ){
	logmessage( "Running script: " + oNameScript );
	var fso = new ActiveXObject ("scripting.filesystemobject");
	var TS1 = fso.OpenTextFile( oNameScript );
	var R1 = TS1.ReadAll();
	return (eval(R1));
	TS1.close();  //i think i have to do this?
}

function Trace( str )
{
	if ( gDebug )
		Application.Logmessage(str);
}

function GetToolPlugin()
{
	return Application.Plugins(gToolName);	 
}

function GetToolPath()
{
	if ( gToolPath == "" )
	{
		var fso = XSIFactory.CreateActiveXObject("Scripting.FileSystemObject");
		
		var strPluginsPath = GetToolPlugin().OriginPath;
		gToolPath = fso.GetParentFolderName(fso.GetParentFolderName(strPluginsPath)) + "\\";
	}
	return gToolPath;
}
