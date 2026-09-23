//////////////////////////////////////////////////////////////////////////////////////
// Valve Source Tools Version v1.2a
// Softimage 2005
//
// Description: 
//
// Author: Dominic Laflamme(dolaflam@softimage.com), Simon Inwood (simonin@softimage.com)
//////////////////////////////////////////////////////////////////////////////////////
var gDebug = false;
var gToolName = "ValveSource Tools";
var gToolPath = "";

function XSILoadPlugin( in_reg )
{
	Trace( "ValveSourceTools XSILoadPlugin called" );
	
	in_reg.Author = "Softimage";
	in_reg.Name = gToolName;
	in_reg.Major = 1;
	in_reg.Minor = 2;
	in_reg.Email = "xsisdk@softimage.com";
	in_reg.URL = "www.softimage.com";

	in_reg.RegisterMenu( siMenuMainTopLevelID, "ValveSource", false );		// plugin item has to be same as display name. (must enter into CQ)

	in_reg.RegisterCommand( "vstAbout", "vstAbout" );
	in_reg.RegisterCommand( "vstHelp", "vstHelp" );
	in_reg.RegisterCommand( "ValveSourceToolsNotImpl", "ValveSourceToolsNotImpl" );

	return true;
} 

//////////////////////////////////////////////////////////////////////////////////////
// Menu
//////////////////////////////////////////////////////////////////////////////////////
function ValveSource_Init( ctx ) 
{
	Trace("ValveSourceToolsMenu_Init");
	
	var oMenu = ctx.source;
	oMenu.Name = "ValveSourceTools";
	
	var oSubMenu;
	
	oMenu.AddCommandItem( "Export SMD...", "SMDExport2" );
	oMenu.AddCommandItem( "Import SMD...", "SMDImport" );
	oMenu.AddCommandItem( "QC/MDL Generator", "vst_qcmaker" );

	oMenu.AddSeparatorItem();
	oMenu.AddCommandItem( "Import WeightMap...", "WeightImport" );
	oMenu.AddCommandItem( "Export WeightMap...", "WeightExport" );
	oMenu.AddSeparatorItem();

	oSubMenu = oMenu.AddItem( "HelperBone Tool", siMenuItemSubmenu );
	oSubMenu.AddCommandItem( "HelperBone Toolbar", "vstShowHelperBoneToolbar" );
	oSubMenu.AddSeparatorItem();
	oSubMenu.AddCommandItem( "Edit HelperBones", "vstEditHelperBones" );
	oSubMenu.AddCommandItem( "Select All HelperBones", "vstSelectAllHelperBones" );
	oSubMenu.AddSeparatorItem();
	oSubMenu.AddCommandItem( "Create HelperBones", "vstCreateHelperBones" );
	oSubMenu.AddCommandItem( "Import HelperBones...", "vstShowImportHelperBonesDialog" );
	oSubMenu.AddSeparatorItem();
	oSubMenu.AddCommandItem( "Create Bone Symmetry Map", "vstCreateBoneSymmetryMap" );
	oSubMenu.AddCommandItem( "Mirror HelperBones", "vstMirrorHelperBones" );
	oSubMenu.AddSeparatorItem();
	oSubMenu.AddCommandItem( "Delete HelperBones", "vstDeleteHelperBones" );

	oSubMenu = oMenu.AddItem( "Shape Tools", siMenuItemSubmenu );
	oSubMenu.AddCommandItem( "Shape Toolbar", "vstShowShapeToolbar" );
	oSubMenu.AddSeparatorItem();
	oSubMenu.AddCommandItem( "Init Shape Keys", "vstInitShapeKey" );
	oSubMenu.AddCommandItem( "Copy Shape Key", "vstCopyShapeKey" );
	oSubMenu.AddSeparatorItem();
	oSubMenu.AddCommandItem( "Set Cluster", "vstSetCopyShapeCluster" );
	oSubMenu.AddCommandItem( "Blend Shape", "vstCopyShape" );

	oSubMenu = oMenu.AddItem( "VMF Tools", siMenuItemSubmenu );
	oSubMenu.AddCommandItem( "Import VMF...", "VMFImport" );
	oSubMenu.AddCommandItem( "Export VMF...", "VMFExport" );
	oSubMenu.AddCommandItem( "VMF Edit", "vstVMFEdit" );
	oSubMenu.AddSeparatorItem();
	oSubMenu.AddCommandItem( "Show Concave Edges", "ShowConcaveEdges" );
	oSubMenu.AddCommandItem( "Make Room", "vstMakeRoom" );
	oSubMenu.AddCommandItem( "Make Door", "vstMakeDoor" );
	//oSubMenu.AddCommandItem( "Make Corridor", "vstMakeCorridor" );
	
	oSubMenu = oMenu.AddItem( "Character Rigs", siMenuItemSubmenu );
	oSubMenu.AddCommandItem( "Male Rig", "vstCreateMaleRig" );
	oSubMenu.AddCommandItem( "Female Rig", "vstCreateFemaleRig" );
//	oSubMenu.AddCommandItem( "Shadow Rig", "vstCreateShadowRig" );
	oSubMenu.AddSeparatorItem();
	oSubMenu.AddCommandItem( "Biped Guide", "vstCreateGuide" );
	oSubMenu.AddCommandItem( "Rig from Guide", "vstCreateRigFromGuide" );
	oSubMenu.AddSeparatorItem();
	oSubMenu.AddCommandItem( "Fix broken footroll", "vstFixBrokenFootRoll" );
	
	
	//addding new/updated valve scripts here
	oMenu.AddSeparatorItem();
	oSubMenu = oMenu.AddItem( "Valve Scripts", siMenuItemSubmenu );
		oSubMenu.AddCommandItem( "Pose Library", "vst_poselibrary" );
		oSubMenu.AddCommandItem( "Mirror Pose", "vst_mirrorpose" );
		oSubMenu.AddCommandItem( "Attachment Tool", "vstAttachmentTool" );
		oSubMenu.AddCommandItem( "FK fingers", "vst_FKfingers" );

			oSubMenu.AddSeparatorItem();
		oSubMenu.AddCommandItem( "Update Synoptic HL2", "vst_synopticHL2" );
		oSubMenu.AddCommandItem( "Fix Fingers HL2", "vstFixHandBones" );
		//sk- removing since it causes too many issues
		//oSubMenu.AddCommandItem( "Add Toe Pivot HL2", "vstAddToePivot" );
	
	oMenu.AddSeparatorItem();
	oSubMenu = oMenu.AddItem( "Utilities", siMenuItemSubmenu );
	oSubMenu.AddCommandItem( "Clean Up Folders", "vstfolderCleanup" );
	oSubMenu.AddCommandItem( "Refresh Textures", "vstUpdateTextures" );
	oMenu.AddSeparatorItem();
	oMenu.AddCommandItem( "ValveSource Tools Help", "vstHelp" );
	oMenu.AddCommandItem( "About ValveSource Tools", "vstAbout" );
}

//////////////////////////////////////////////////////////////////////////////////////
// MenuItem Callbacks
//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
// Command : ValveSourceToolsNotImpl
//////////////////////////////////////////////////////////////////////////////////////

function ValveSourceToolsNotImpl_Execute()
{
	logmessage("Not Implemented");
}

//////////////////////////////////////////////////////////////////////////////////////
// Command : vstHelp
//////////////////////////////////////////////////////////////////////////////////////

function vstHelp_Execute()
{
	OpenNetView( GetToolPath() + "netview_valvesource.htm" );
}

//////////////////////////////////////////////////////////////////////////////////////
// Command :  vstAbout
//////////////////////////////////////////////////////////////////////////////////////

function vstAbout_Execute()
{
	var plugin = Application.Plugins(gToolName);
	
	var strTitle = "About " + plugin.Name;
	var strMessage = 
		plugin.Author + "\n" +
		plugin.Name + " V" + plugin.Major + "." + plugin.Minor + "\n";
		
	XSIUIToolkit.MsgBox( strMessage, siMsgOkOnly, strTitle );
}

//////////////////////////////////////////////////////////////////////////////////////
// Private Helper Functions
//////////////////////////////////////////////////////////////////////////////////////
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
