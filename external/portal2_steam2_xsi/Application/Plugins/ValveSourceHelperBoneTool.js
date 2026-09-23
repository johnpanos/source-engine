//////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 Softimage.
// Valve Helper Bone Tool Version 1.2 (for XSI Version 4.2)
// Author: Simon Inwood (simonin@softimage.com)
//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
// constants
//////////////////////////////////////////////////////////////////////////////////////
var gToolName = "ValveSource HelperBone Tool";
var gToolPath = "";
var gVRDSuffix = ".vrd";
var gDefaultRig = "ValveBiped";

var gDefaultRootColor = WireframeColorToRGB(15 /*Red*/);
var gDefaultParentColor = WireframeColorToRGB(242 /* Bright Green */);
var gDefaultChildColor = WireframeColorToRGB(126 /* Yellow */);
var gDefaultWireframeColor = 656 /*blue*/;
		
//////////////////////////////////////////////////////////////////////////////////////
// HelperBoneData (grid) constants
//////////////////////////////////////////////////////////////////////////////////////
var idTriggerEnabled	= 0;
var idTriggerTol		= 1;
var idTriggerRotX		= 2;
var idTriggerRotY		= 3;
var idTriggerRotZ		= 4;
var idTargetRotX		= 5;
var idTargetRotY		= 6;
var idTargetRotZ		= 7;
var idTargetPosX		= 8;
var idTargetPosY		= 9;
var idTargetPosZ		= 10;
var idTriggerName		= 11;
var cGridDataColumns	= idTriggerName+1;

//////////////////////////////////////////////////////////////////////////////////////
// helperboneop constants
//////////////////////////////////////////////////////////////////////////////////////
var gHelperBoneOpName = "HelperBoneOp";

//////////////////////////////////////////////////////////////////////////////////////
// Plugin entry point
//////////////////////////////////////////////////////////////////////////////////////
function XSILoadPlugin( in_reg )
{
	// init tool path	
	var fso = XSIFactory.CreateActiveXObject("Scripting.FileSystemObject");
	
	gToolPath = fso.GetParentFolderName(fso.GetParentFolderName(in_reg.OriginPath)) + "\\";
	
	var strHelpFile = gToolName.replace(/\s*/g, "") + ".htm";
	
	in_reg.Author = "Valve and Softimage";
	in_reg.Name = gToolName;
	in_reg.Major = 1;
	in_reg.Minor = 2;
	in_reg.Help = fso.BuildPath(fso.BuildPath( gToolPath, "Doc" ), strHelpFile);
	in_reg.URL = "www.softimage.com";

	in_reg.RegisterCommand( "vstShowHelperBoneToolbar", "vstShowHelperBoneToolbar" );
	in_reg.RegisterCommand( "vstCreateHelperBones", "vstCreateHelperBones" );
	in_reg.RegisterCommand( "vstSelectHelperBone", "vstSelectHelperBone" );
	in_reg.RegisterCommand( "vstDeselectHelperBone", "vstDeselectHelperBone" );
	in_reg.RegisterCommand( "vstApplyHelperBonePrimOp", "vstApplyHelperBonePrimOp" );
	in_reg.RegisterCommand( "vstApplyHelperBoneOp", "vstApplyHelperBoneOp" );

	in_reg.RegisterProperty( "HelperBoneTool");
	in_reg.RegisterProperty( "HelperBoneData");

}

//////////////////////////////////////////////////////////////////////////////////////
// Property : HelperBoneTool
//////////////////////////////////////////////////////////////////////////////////////

function HelperBoneTool_Define( io_Context )
{
	var oThis =  io_Context.Source	
	
	// Generated based on the original SPDL version
	
	var siDoubleMin = -1.7976931348623157e+308
	var siDoubleMax = 1.7976931348623157e+308
	
	var dUIRotationMin = -180;
	var dUIRotationMax = 180;
	var dUIOffsetMin = -5;
	var dUIOffsetMax = 5;
	
	// required to avoid feedback from callbacks when 
	// setting property to a new helper.
	oThis.AddParameter2( "EditInProgress", siBool, false  );
	
	oThis.AddParameter2( "ID" ,siInt4, 0 );
	oThis.AddParameter2( "PathID" ,siString, "" );
	oThis.AddParameter2( "PrevID" ,siInt4, -1 );
	oThis.AddParameter( "HelperBoneRoot", siString, siClassifUnknown, siReadOnly );
	oThis.AddParameter( "HelperBoneParent", siString, siClassifUnknown, siReadOnly );
	oThis.AddParameter( "HelperBoneChild", siString, siClassifUnknown, siReadOnly );
	oThis.AddParameter2( "Mute", siBool, false  );
	oThis.AddParameter2( "TriggerID" ,siInt4, -1 );
	oThis.AddParameter2( "TriggerTolerance" ,siDouble,90,0,180,0,180,0,4,"TriggerTolerance" ,"Trigger tolerance in euler angles" )
	oThis.AddParameter2( "TriggerRotX" ,siDouble,0,siDoubleMin,siDoubleMax,dUIRotationMin,dUIRotationMax,0,4,"TriggerRotX" ,"Trigger rotation X euler angle" )
	oThis.AddParameter2( "TriggerRotY" ,siDouble,0,siDoubleMin,siDoubleMax,dUIRotationMin,dUIRotationMax,0,4,"TriggerRotY" ,"Trigger rotation Y euler angle" )
	oThis.AddParameter2( "TriggerRotZ" ,siDouble,0,siDoubleMin,siDoubleMax,dUIRotationMin,dUIRotationMax,0,4,"TriggerRotZ" ,"Trigger rotation Z euler angle" )
	oThis.AddParameter2( "TargetRotX" ,siDouble,0,siDoubleMin,siDoubleMax,dUIRotationMin,dUIRotationMax,0,4,"TargetRotX" ,"Target rotation X euler angle" )
	oThis.AddParameter2( "TargetRotY" ,siDouble,0,siDoubleMin,siDoubleMax,dUIRotationMin,dUIRotationMax,0,4,"TargetRotY" ,"Target rotation Y euler angle" )
	oThis.AddParameter2( "TargetRotZ" ,siDouble,0,siDoubleMin,siDoubleMax,dUIRotationMin,dUIRotationMax,0,4,"TargetRotZ" ,"Target rotation Z euler angle" )
	oThis.AddParameter2( "TargetPosX" ,siDouble,0,siDoubleMin,siDoubleMax,dUIOffsetMin,dUIOffsetMax,0,4,"TargetPosX" ,"Target position X" )
	oThis.AddParameter2( "TargetPosY" ,siDouble,0,siDoubleMin,siDoubleMax,dUIOffsetMin,dUIOffsetMax,0,4,"TargetPosY" ,"Target position Y" )
	oThis.AddParameter2( "TargetPosZ" ,siDouble,0,siDoubleMin,siDoubleMax,dUIOffsetMin,dUIOffsetMax,0,4,"TargetPosZ" ,"Target position Z" )
	oThis.AddParameter2( "RootBoneLength" ,siDouble,0,0,100,0,100,0,4,"RootBoneLength" ,"Length of root bone" )
	oThis.AddParameter2( "BoneDistance" ,siDouble,50,0,100,0,100,0,4,"BoneDistance" , "Percentage along length of root bone" )
	oThis.AddParameter2( "BoneOffsetX" ,siDouble,0,siDoubleMin,siDoubleMax,dUIOffsetMin,dUIOffsetMax,0,4,"BoneOffsetX" ,"Offset in X relative to the root." );
	oThis.AddParameter2( "BoneOffsetY" ,siDouble,0,siDoubleMin,siDoubleMax,dUIOffsetMin,dUIOffsetMax,0,4,"BoneOffsetY" ,"Offset in Y relative to the root." );
	oThis.AddParameter2( "BoneOffsetZ" ,siDouble,0,siDoubleMin,siDoubleMax,dUIOffsetMin,dUIOffsetMax,0,4,"BoneOffsetZ" ,"Offset in Z relative to the root." );
	oThis.AddParameter2( "BoneWidth" ,siDouble,3,0,50,1,5,0,4,"BoneWidth" ,"" );
	oThis.AddParameter2( "BoneHeight" ,siDouble,3,0,50,1,5,0,4,"BoneHeight" ,"" );
	oThis.AddParameter2( "BoneLength" ,siDouble,3,0,50,1,5,0,4,"BoneLength" ,"" );
	oThis.AddParameter2( "ImportVRDFile", siString, GetConfigFilesPath() + gDefaultRig + gVRDSuffix );
	oThis.AddParameter2( "ExportVRDFile", siString, GetConfigFilesPath() + gDefaultRig + gVRDSuffix );

	oThis.AddParameter2("RootBoneWireframeColorR", siDouble, gDefaultRootColor[0],0,1);
	oThis.AddParameter2("RootBoneWireframeColorG", siDouble, gDefaultRootColor[1],0,1);
	oThis.AddParameter2("RootBoneWireframeColorB", siDouble, gDefaultRootColor[2],0,1);
	oThis.AddParameter2("RootBoneWireframeColorA", siDouble, 0,0,1);
	
	oThis.AddParameter2("ParentBoneWireframeColorR", siDouble, gDefaultParentColor[0],0,1);
	oThis.AddParameter2("ParentBoneWireframeColorG", siDouble, gDefaultParentColor[1],0,1);
	oThis.AddParameter2("ParentBoneWireframeColorB", siDouble, gDefaultParentColor[2],0,1);
	oThis.AddParameter2("ParentBoneWireframeColorA", siDouble, 0,0,1);
	
	oThis.AddParameter2("ChildBoneWireframeColorR", siDouble, gDefaultChildColor[0],0,1);
	oThis.AddParameter2("ChildBoneWireframeColorG", siDouble, gDefaultChildColor[1],0,1);
	oThis.AddParameter2("ChildBoneWireframeColorB", siDouble, gDefaultChildColor[2],0,1);
	oThis.AddParameter2("ChildBoneWireframeColorA", siDouble, 0,0,1);

	oThis.AddParameter2( "Solo" ,siBool,false);
	oThis.AddParameter2( "DebugLevel", siInt4,0);
	oThis.AddParameter2( "ComputationSpace", siInt4,0);
	
}

function HelperBoneTool_DefineLayout( io_Context )
{
	var oPPGItem;

	var oLayout = io_Context.Source
	oLayout.Clear();
	
	oLayout.AddTab( "General" );
	
		oLayout.AddRow();
			oPPGItem = oLayout.AddEnumControl( "ID", Array( "Bip01_L_Shoulder *", 0, "Bip01_L_Knee", 1) ) ;
			oPPGItem.SetAttribute( "NoLabel", true  ) ;
			oLayout.AddButton("CreateHelper", "Create");
			oLayout.AddButton("DeleteHelper", "Delete");
			oLayout.AddButton("RenameHelper", "Rename");
			oLayout.AddButton("SelectHelper","Select");
			oLayout.AddButton("ToggleLRHelper","L<->R");
		oLayout.EndRow();
		
		oLayout.AddGroup("HelperBone");
		
			oLayout.AddRow();
				oPPGItem = oLayout.AddItem( "Mute", "Mute" ) ;
				//oPPGItem.SetAttribute( siUICX, 10 ) ;
				oPPGItem.WidthPercentage = 20 ;
				oPPGItem.LabelPercentage = 80 ;
				oPPGItem.LabelMinPixels = 25 ;	
				oPPGItem = oLayout.AddStaticText(" ");
				oPPGItem.SetAttribute( siUICX, 400 ) ;
			oLayout.EndRow();
			
			oLayout.AddRow();
				oPPGItem = oLayout.AddItem("HelperBoneRoot","Root");
				oLayout.AddButton( "SelectRoot", "Select");
			oLayout.EndRow();
			oLayout.AddRow();
				oPPGItem = oLayout.AddItem("HelperBoneParent","Parent");
				oLayout.AddButton( "SelectParent", "Select");
			oLayout.EndRow();
			oLayout.AddRow();
				oPPGItem = oLayout.AddItem("HelperBoneChild","Child");
				oLayout.AddButton( "SelectChild", "Select");
			oLayout.EndRow();
			
			oLayout.AddGroup("Size");
				oLayout.AddRow();
				oPPGItem = oLayout.AddItem( "BoneWidth", "Width" ) ;
				oPPGItem.SetAttribute( siUINoSlider, false ) ;
				oPPGItem.SetAttribute( siUICX, 100 ) ;
				oPPGItem.WidthPercentage = 20 ;
				oPPGItem.LabelPercentage = 10 ;
				oPPGItem.LabelMinPixels = 20 ;	
	
				oPPGItem = oLayout.AddItem( "BoneHeight", "Height" ) ;
				oPPGItem.SetAttribute( siUINoSlider, false ) ;
				oPPGItem.SetAttribute( siUICX, 100 ) ;
				oPPGItem.WidthPercentage = 20 ;
				oPPGItem.LabelPercentage = 10 ;
				oPPGItem.LabelMinPixels = 20 ;	
	
				oPPGItem = oLayout.AddItem( "BoneLength", "Length" ) ;
				oPPGItem.SetAttribute( siUINoSlider, false ) ;
				oPPGItem.SetAttribute( siUICX, 100 ) ;
				oPPGItem.WidthPercentage = 20 ;
				oPPGItem.LabelPercentage = 10 ;
				oPPGItem.LabelMinPixels = 20 ;	
				oLayout.EndRow();
			oLayout.EndGroup();

			oLayout.AddGroup("Position");
				oLayout.AddRow();
					oPPGItem = oLayout.AddItem( "BoneDistance", "Distance %" ) ;
					oPPGItem.SetAttribute( siUINoSlider, false ) ;
					oPPGItem.SetAttribute( siUICX, 100 ) ;
					oPPGItem.WidthPercentage = 20 ;
					oPPGItem.LabelPercentage = 20 ;
					oPPGItem.LabelMinPixels = 60 ;	

					oPPGItem = oLayout.AddItem( "RootBoneLength", "Root Bone Length" ) ;
					oPPGItem.SetAttribute( siUINoSlider, true ) ;
					oPPGItem.SetAttribute( siUICX, 100 ) ;
					oPPGItem.WidthPercentage = 20 ;
					oPPGItem.LabelPercentage = 20 ;
					oPPGItem.LabelMinPixels = 60 ;	
				oLayout.EndRow();
				
				oLayout.AddRow();
					oPPGItem = oLayout.AddItem( "BoneOffsetX", "OffsetX" ) ;
					oPPGItem.SetAttribute( siUINoSlider, false ) ;
					oPPGItem.SetAttribute( siUICX, 100 ) ;
					oPPGItem.WidthPercentage = 20 ;
					oPPGItem.LabelPercentage = 20 ;
					oPPGItem.LabelMinPixels = 20 ;	
		
					oPPGItem = oLayout.AddItem( "BoneOffsetY", "OffsetY" ) ;
					oPPGItem.SetAttribute( siUINoSlider, false ) ;
					oPPGItem.SetAttribute( siUICX, 100 ) ;
					oPPGItem.WidthPercentage = 20 ;
					oPPGItem.LabelPercentage = 20 ;
					oPPGItem.LabelMinPixels = 20 ;	
		
					oPPGItem = oLayout.AddItem( "BoneOffsetZ", "OffsetZ" ) ;
					oPPGItem.SetAttribute( siUINoSlider, false ) ;
					oPPGItem.SetAttribute( siUICX, 100 ) ;
					oPPGItem.WidthPercentage = 20 ;
					oPPGItem.LabelPercentage = 20 ;
					oPPGItem.LabelMinPixels = 20 ;	
				oLayout.EndRow();
			oLayout.EndGroup();
		oLayout.EndGroup();

		oLayout.AddGroup( "Triggers");
			oLayout.AddRow();
				oPPGItem = oLayout.AddEnumControl( "TriggerID", Array( "Trigger0 *", 0, "Trigger1", 1) ) ;
				oPPGItem.SetAttribute( "NoLabel", true  ) ;
				
				oLayout.AddButton( "AddTrigger", "Add");
				oLayout.AddButton( "RemoveTrigger", "Remove");
				oLayout.AddButton( "RenameTrigger", "Rename");
				oLayout.AddStaticText(" ");
			oLayout.EndRow();
			
			oLayout.AddGroup( "Trigger");
				oLayout.AddRow();
					oPPGItem = oLayout.AddItem( "Solo");
					oPPGItem.SetAttribute( siUICX, 100 ) ;
					oPPGItem.WidthPercentage = 20 ;
					oPPGItem.LabelPercentage = 10 ;
					oPPGItem.LabelMinPixels = 20 ;	
					oPPGItem = oLayout.AddStaticText(" ");
					//oPPGItem.SetAttribute( siUICX, 500 ) ;
				//oLayout.EndRow();
				//oLayout.AddRow();
					oPPGItem = oLayout.AddItem( "TriggerTolerance", "Tolerance" )
					oPPGItem.SetAttribute( siUICX, 100 ) ;
					oPPGItem.WidthPercentage = 40 ;
					oPPGItem.LabelPercentage = 10 ;
					oPPGItem.LabelMinPixels = 20 ;	
					oPPGItem = oLayout.AddStaticText("");
					oPPGItem.SetAttribute( siUICX, 100 ) ;
					oPPGItem.WidthPercentage =20 ;
					oPPGItem.LabelPercentage = 10 ;
					oPPGItem.LabelMinPixels = 20 ;	
					oPPGItem = oLayout.AddStaticText("");
					oPPGItem.SetAttribute( siUICX, 100 ) ;
					oPPGItem.WidthPercentage = 20 ;
					oPPGItem.LabelPercentage = 10 ;
					oPPGItem.LabelMinPixels = 20 ;				
				oLayout.EndRow();
				oPPGItem = oLayout.AddStaticText("");
				oLayout.AddRow();
					oPPGItem = oLayout.AddItem( "TriggerRotX", "RotX" )
					oPPGItem.SetAttribute( "NoSlider", false );
					oPPGItem.SetAttribute( "NoLabel", false );
					oPPGItem.SetAttribute( siUICX, 100 ) ;
					oPPGItem.WidthPercentage = 25 ;
					oPPGItem.LabelPercentage = 10 ;
					oPPGItem.LabelMinPixels = 20 ;	
	
					oPPGItem = oLayout.AddItem( "TriggerRotY", "RotY" )
					oPPGItem.SetAttribute( "NoSlider", false );
					oPPGItem.SetAttribute( "NoLabel", false );
					oPPGItem.SetAttribute( siUICX, 100 ) ;
					oPPGItem.WidthPercentage = 25 ;
					oPPGItem.LabelPercentage = 10 ;
					oPPGItem.LabelMinPixels = 20 ;	
					
					oPPGItem = oLayout.AddItem( "TriggerRotZ", "RotZ" )
					oPPGItem.SetAttribute( "NoSlider", false );
					oPPGItem.SetAttribute( "NoLabel", false );
					oPPGItem.SetAttribute( siUICX, 100 ) ;
					oPPGItem.WidthPercentage = 25 ;
					oPPGItem.LabelPercentage = 10 ;
					oPPGItem.LabelMinPixels = 20 ;	
					
					oPPGItem = oLayout.AddButton("ClearTriggerRot", "Clr");
					oPPGItem.WidthPercentage = 12 ;
					oPPGItem = oLayout.AddButton("SetTriggerRot", "Set");
					oPPGItem.WidthPercentage = 12 ;
				oLayout.EndRow();	
			oLayout.EndGroup();

			oLayout.AddGroup( "Target");
				oLayout.AddRow();
					oPPGItem = oLayout.AddItem( "TargetRotX", "RotX" )
					oPPGItem.SetAttribute( "NoSlider", false );
					oPPGItem.SetAttribute( siUICX, 100 ) ;
					oPPGItem.WidthPercentage = 25 ;
					oPPGItem.LabelPercentage = 10 ;
					oPPGItem.LabelMinPixels = 20 ;	

					oPPGItem = oLayout.AddItem( "TargetRotY", "RotY" )
					oPPGItem.SetAttribute( "NoSlider", false );
					oPPGItem.SetAttribute( siUICX, 100 ) ;
					oPPGItem.WidthPercentage = 25 ;
					oPPGItem.LabelPercentage = 10 ;
					oPPGItem.LabelMinPixels = 20 ;	

					oPPGItem = oLayout.AddItem( "TargetRotZ", "RotZ" )
					oPPGItem.SetAttribute( "NoSlider", false );
					oPPGItem.SetAttribute( siUICX, 100 ) ;
					oPPGItem.WidthPercentage = 25 ;
					oPPGItem.LabelPercentage = 10 ;
					oPPGItem.LabelMinPixels = 20 ;	

					oPPGItem = oLayout.AddButton("ClearTargetRot", "Clr");
					oPPGItem.WidthPercentage = 12 ;
					oPPGItem = oLayout.AddButton("SetTargetRot","Set");
					oPPGItem.SetAttribute(siUIButtonDisable, true);
					oPPGItem.WidthPercentage = 12 ;
				oLayout.EndRow();	
				
				oLayout.AddRow();
					oPPGItem = oLayout.AddItem( "TargetPosX", "PosX" )
					oPPGItem.SetAttribute( "NoSlider", false );
					oPPGItem.SetAttribute( siUICX, 100 ) ;
					oPPGItem.WidthPercentage = 25 ;
					oPPGItem.LabelPercentage = 10 ;
					oPPGItem.LabelMinPixels = 20 ;	

					oPPGItem = oLayout.AddItem( "TargetPosY", "PosY" )
					oPPGItem.SetAttribute( "NoSlider", false );
					oPPGItem.SetAttribute( siUICX, 100 ) ;
					oPPGItem.WidthPercentage = 25 ;
					oPPGItem.LabelPercentage = 10 ;
					oPPGItem.LabelMinPixels = 20 ;
						
					oPPGItem = oLayout.AddItem( "TargetPosZ", "PosZ" )
					oPPGItem.SetAttribute( "NoSlider", false );
					oPPGItem.SetAttribute( siUICX, 100 ) ;
					oPPGItem.WidthPercentage = 25 ;
					oPPGItem.LabelPercentage = 10 ;
					oPPGItem.LabelMinPixels = 20 ;	

					oPPGItem = oLayout.AddButton("ClearTargetPos", "Clr");
					oPPGItem.WidthPercentage = 12 ;
					oPPGItem = oLayout.AddButton("SetTargetPos","Set");
					oPPGItem.SetAttribute(siUIButtonDisable, true);
					oPPGItem.WidthPercentage = 12 ;
				oLayout.EndRow();	
			oLayout.EndGroup();
				
			oLayout.AddRow();
				oLayout.AddButton("RemoveAllTriggers", "Remove All");
				//oPPGItem = oLayout.AddStaticText("");
				//oPPGItem.SetAttribute( siUICX, 0 ) ;
				//oLayout.AddButton("EnableAllTriggers", "Enable All");
				//oLayout.AddButton("DisableAllTriggers", "Disable All");
				oPPGItem = oLayout.AddStaticText("");
				//oPPGItem.SetAttribute( siUICX, 10 ) ;
				oLayout.AddStaticText("");
			oLayout.EndRow();
				

		oLayout.EndGroup();

	oLayout.AddRow();
		//oLayout.AddButton("CommiteChanges","Commit Changes");
		//oLayout.AddButton("UndoChanges","Undo Changes");
		//oLayout.AddButton("SetAsDefault", "Set As Default");
		oPPGItem = oLayout.AddStaticText("");
		oPPGItem.SetAttribute( siUICX, 100 ) ;
		oPPGItem.WidthPercentage = 20 ;
		oPPGItem.LabelPercentage = 10 ;
		oPPGItem.LabelMinPixels = 20 ;	
		oLayout.AddButton("ResetChain","ResetChain");
		oLayout.AddButton("Tools");
		oLayout.AddButton("Help");
	oLayout.EndRow();
	
	oLayout.AddTab( "Advanced" );
		oLayout.AddGroup("VRD Files");
			oLayout.AddRow();
				oLayout.AddButton("EditImportVRD", "Edit");
				oItem = oLayout.AddItem( "ImportVRDFile","",siControlFilePath );
				oItem.SetAttribute( "NoLabel", true );
				oItem.SetAttribute( siUIFileMustExist, true);
				oItem.SetAttribute( siUIOpenFile, true);
				oItem.SetAttribute( siUIFileFilter, "Valve Rig Data (*.vrd)|*.vrd");

				oLayout.AddButton("ImportVRD", "Import");
			oLayout.EndRow();	
			oLayout.AddRow();
				oLayout.AddButton("EditExportVRD", "Edit");
				oItem = oLayout.AddItem( "ExportVRDFile","",siControlFilePath );
				oItem.SetAttribute( "NoLabel", true );
				oItem.SetAttribute( siUIFileFilter, "Valve Rig Data (*.vrd)|*.vrd" );

				oLayout.AddButton("ExportVRD", "Export");
			oLayout.EndRow();	
		oLayout.EndGroup();	

		oLayout.AddGroup("Wireframe Colors");
			oLayout.AddColor("RootBoneWireframeColorR", "Helper's Root");
			oLayout.AddColor("ParentBoneWireframeColorR", "Parent Bone");
			oLayout.AddColor("ChildBoneWireframeColorR", "Child Bone");
		oLayout.EndGroup();	

		oLayout.AddGroup("Debug");
			oPPGItem = oLayout.AddEnumControl( "DebugLevel", Array( "off", 0, "function calls", 1, "trace statements", 2, "all", 4), "Level" ) ;
			oLayout.AddButton("EditHelperBoneData", "Edit HelperBone Data");
		oLayout.EndGroup();	
}

//***************************************************************************************
// function		:	OnInit
// description	:	initialize property page. If not set) { set the import/export 
//				paths to point at current project. Also installs a selection
//				event to sync with selection changes
//***************************************************************************************
function HelperBoneTool_OnInit()
{
	vstDebugTrace("HelperBoneTool_OnInit");

	var model = PSet.Inspected(0).Parent;
	
	if ( ImportVRDFile.Value == "") 
	{
		ImportVRDFile.Value = GetConfigFilesPath() + model.name + gVRDSuffix;
	}

	if ( ExportVRDFile.Value == "") 
	{
		ExportVRDFile.Value = GetConfigFilesPath() + model.name + gVRDSuffix;
	}

	// install event
	vstRegisterEvents(false,true);
	vstRegisterEvents(true,true);
	
	HelperBoneTool_Init(model);
	
	PPG.Refresh() ;
}

function HelperBoneTool_Init(model)
{
	vstDebugTrace("HelperBoneTool_Init");
	
	// update helper bone list
	var helperbones = GetHelperBones(model);

	HelperBoneTool_RebuildHelperBoneCombo(PPG, helperbones);
	 
	// select helper bone
	var o = null;
	
	if ( Selection.count != 0) 
	{
		var helper = vstGetHelperBoneObject(selection(0));
		
		if (helper!=null)
			o = helper.object;
	}
	if ( null == o && helperbones.count )
		o = helperbones(0);
	
	if ( o == null || false == vstSelectHelperBone( o ) ) 
	{
		HelperBoneTool_SetAsDefault( PSet );
	}
}

function HelperBoneTool_SetAsDefault( oThis )
{
	oThis.ID.Value = 0;
	oThis.Mute.Value= false;
	oThis.BoneDistance.Value = 50;
	oThis.BoneOffsetX.Value = 0;
	oThis.BoneOffsetY.Value = 0;
	oThis.BoneOffsetZ.Value = 0;
	oThis.BoneWidth.Value = 3;
	oThis.BoneHeight.Value = 3;
	oThis.BoneLength.Value = 3;
	
	HelperBoneTool_TriggerSetAsDefault( oThis );
	
	oThis.ImportVRDFile.Value = GetConfigFilesPath() + gDefaultRig + gVRDSuffix;
	oThis.ExportVRDFile.Value = GetConfigFilesPath() + gDefaultRig + gVRDSuffix;

	oThis.RootBoneWireframeColorR.Value = gDefaultRootColor[0];
	oThis.RootBoneWireframeColorG.Value = gDefaultRootColor[1];
	oThis.RootBoneWireframeColorB.Value = gDefaultRootColor[2];
	
	oThis.ParentBoneWireframeColorR.Value = gDefaultParentColor[0];
	oThis.ParentBoneWireframeColorG.Value = gDefaultParentColor[1];
	oThis.ParentBoneWireframeColorB.Value = gDefaultParentColor[2];
	
	oThis.ChildBoneWireframeColorR.Value = gDefaultChildColor[0];
	oThis.ChildBoneWireframeColorG.Value = gDefaultChildColor[1];
	oThis.ChildBoneWireframeColorB.Value = gDefaultChildColor[2];

	oThis.PPGLayout.Item("ID").UIItems = Array();
}

function HelperBoneTool_TriggerSetAsDefault( oThis )
{
	vstDebugTrace("HelperBoneTool_TriggerSetAsDefault");
	
	oThis.Solo.Value = false;
	oThis.TriggerID.Value = 0;
	oThis.TriggerTolerance.Value = 90;
	oThis.TriggerRotX.Value = 0;
	oThis.TriggerRotY.Value = 0;
	oThis.TriggerRotZ.Value = 0;
	oThis.TargetRotX.Value = 0;
	oThis.TargetRotY.Value = 0;
	oThis.TargetRotZ.Value = 0;
	oThis.TargetPosX.Value = 0;
	oThis.TargetPosY.Value = 0;
	oThis.TargetPosZ.Value = 0;
	
	oThis.PPGLayout.Item("TriggerID").UIItems = Array();
}

function HelperBoneTool_SetTrigger( oThis, griddata, idxRow )
{
	vstDebugTrace("HelperBoneTool_SetTrigger");
	
	if (griddata.rowcount && griddata.columncount)
	{
		var iColumn =0;
		
		iColumn++; // skip solo'd
		oThis.TriggerTolerance.Value = griddata.getcell(iColumn++,idxRow);
		oThis.TriggerRotX.Value = griddata.getcell(iColumn++,idxRow);
		oThis.TriggerRotY.Value = griddata.getcell(iColumn++,idxRow);
		oThis.TriggerRotZ.Value = griddata.getcell(iColumn++,idxRow);
		oThis.TargetRotX.Value = griddata.getcell(iColumn++,idxRow);
		oThis.TargetRotY.Value = griddata.getcell(iColumn++,idxRow);
		oThis.TargetRotZ.Value = griddata.getcell(iColumn++,idxRow);
		oThis.TargetPosX.Value = griddata.getcell(iColumn++,idxRow);
		oThis.TargetPosY.Value = griddata.getcell(iColumn++,idxRow);
		oThis.TargetPosZ.Value = griddata.getcell(iColumn++,idxRow);

		// find solo'd trigger
		var bSolo = (griddata.rowcount > 1) && (true==Boolean(griddata.getcell(idTriggerEnabled,idxRow))); 

		for ( var iRow=0; iRow < griddata.rowcount; iRow++ )
		{
			if ( idxRow == iRow ) 
				continue;
							
			if ( (true==Boolean(griddata.getcell(idTriggerEnabled,iRow))) )
			{
				bSolo = false;  
				break;
			}	
		}
		oThis.Solo.Value = bSolo;
		
		// update trigger combo
		HelperBoneTool_RebuildTriggerCombo( oThis, griddata );
	}
	else
	{
		HelperBoneTool_TriggerSetAsDefault( oThis );
	}
}

function HelperBoneTool_CreateHelper_OnClicked()
{
	vstDebugTrace("HelperBoneTool_CreateHelper_OnClicked");

	try { 
		var o = vstCreateHelperBoneTool( PPG.Inspected(0).Parent );
	} catch (e) { o = null; }
	
	if ( o != null )
	{
		HelperBoneTool_OnInit();
	}
}

function HelperBoneTool_DeleteHelper_OnClicked()
{
	vstDebugTrace("HelperBoneTool_DeleteHelper_OnClicked");

	var oHelper = GetHelperObjectFromID( PSet.Inspected(0), Number(PSet.ID.Value) );
	
	try { 
		if (oHelper != null && vstDeleteHelperBone( oHelper.object, true ))
		{
			var helperbones = oHelper.GetHelperBones()
			if ( helperbones.count )
				HelperBoneTool_RebuildHelperBoneCombo( PPG, helperbones );
			else
				HelperBoneTool_SetAsDefault(PPG);
					
			PPG.Refresh();
		}
	} catch (e) { ; }

}

function HelperBoneTool_RenameHelper_OnClicked()
{
	vstDebugTrace("HelperBoneTool_RenameHelper_OnClicked");

	var oHelper = GetHelperObjectFromID( PSet.Inspected(0), Number(PSet.ID.Value) );
	
	if ( oHelper == null ) return;
	
	var newname = vstInputBox( "New name:", "Rename " + oHelper.object.name, oHelper.object.name );
	
	if ( String(newname) != "" )
	{
		oHelper.object.Name = newname;
		HelperBoneTool_RebuildHelperBoneCombo(PPG, oHelper.GetHelperBones());
		PPG.Refresh();
	}
}

function HelperBoneTool_ID_OnChanged()
{
	vstDebugTrace("HelperBoneTool_ID_OnChanged");
	if ( PPG.EditInProgress.Value ) return;
	
	var oHelper = GetHelperObjectFromID( PPG.Inspected(0), Number(PSet.ID.Value) );
	
	if ( oHelper != null )
	{
		var helperbones = oHelper.GetHelperBones();

		vstRegisterEvents( false );
		for ( var i = 0; i < helperbones.count; i++ )
			if ( helperbones(i).selected && ! helperbones(i).isequalto(oHelper.object)) 
				helperbones(i).selected = false;

		if (!oHelper.object.selected) oHelper.object.selected = true;
		vstRegisterEvents( true );

		vstSelectHelperBone(oHelper.object);

		PPG.Refresh();
	}
}

function HelperBoneTool_Mute_OnChanged()
{
	vstDebugTrace("HelperBoneTool_Mute_OnChanged" );
	if ( PPG.EditInProgress.Value ) return;
	
	var oHelper = GetHelperObjectFromID( PPG.Inspected(0), Number(PSet.ID.Value) );
	
	if (oHelper != null)
		oHelper.data.Parameters("Enabled").Value = (PSet.Mute.Value == false);
}

function HelperBoneTool_BoneWidth_OnChanged()
{
	vstDebugTrace("HelperBoneTool_BoneWidth_OnChanged" );
	if ( PPG.EditInProgress.Value ) return;
	
	var oHelper = GetHelperObjectFromID( PPG.Inspected(0), Number(PSet.ID.Value) );
	
	if (oHelper != null)
		oHelper.GetPrimOp().Parameters("Width").Value = PSet.BoneWidth.Value;
}

function HelperBoneTool_BoneHeight_OnChanged()
{
	vstDebugTrace("HelperBoneTool_BoneHeight_OnChanged");
	if ( PPG.EditInProgress.Value ) return;

	var oHelper = GetHelperObjectFromID( PPG.Inspected(0), Number(PSet.ID.Value) );
	
	if (oHelper != null)
		oHelper.GetPrimOp().Parameters("Height").Value = PSet.BoneHeight.Value;
}

function HelperBoneTool_BoneLength_OnChanged()
{
	vstDebugTrace("HelperBoneTool_BoneLength_OnChanged");
	if ( PPG.EditInProgress.Value ) return;

	var oHelper = GetHelperObjectFromID( PPG.Inspected(0), Number(PSet.ID.Value) );
	
	if (oHelper != null)
		oHelper.GetPrimOp().Parameters("Length").Value = PSet.BoneLength.Value;
}

function HelperBoneTool_BoneDistance_OnChanged()
{
	vstDebugTrace("HelperBoneTool_BoneDistance_OnChanged");
	if ( PPG.EditInProgress.Value ) return;

	var oHelper = GetHelperObjectFromID( PPG.Inspected(0), Number(PSet.ID.Value) );
	
	if (oHelper != null)
		oHelper.data.Parameters("BoneDistance").Value = PSet.BoneDistance.Value;
}

function HelperBoneTool_BoneOffsetX_OnChanged()
{
	vstDebugTrace("HelperBoneTool_BoneOffsetX_OnChanged");
	if ( PPG.EditInProgress.Value ) return;

	var oHelper = GetHelperObjectFromID( PPG.Inspected(0), Number(PSet.ID.Value) );
	
	if (oHelper != null)
		oHelper.data.Parameters("BoneOffsetX").Value = PSet.BoneOffsetX.Value;
}

function HelperBoneTool_BoneOffsetY_OnChanged()
{
	vstDebugTrace("HelperBoneTool_BoneOffsetY_OnChanged");
	if ( PPG.EditInProgress.Value ) return;

	var oHelper = GetHelperObjectFromID( PPG.Inspected(0), Number(PSet.ID.Value) );
	
	if (oHelper != null)
		oHelper.data.Parameters("BoneOffsetY").Value = PSet.BoneOffsetY.Value;
}

function HelperBoneTool_BoneOffsetZ_OnChanged()
{
	vstDebugTrace("HelperBoneTool_BoneOffsetZ_OnChanged");
	if ( PPG.EditInProgress.Value ) return;

	var oHelper = GetHelperObjectFromID( PPG.Inspected(0), Number(PSet.ID.Value) );
	
	if (oHelper != null)
		oHelper.data.Parameters("BoneOffsetZ").Value = PSet.BoneOffsetZ.Value;
}

function HelperBoneTool_Solo_OnChanged()
{
	vstDebugTrace("HelperBoneTool_Solo_OnChanged");
	if ( PPG.EditInProgress.Value ) return;

	var oHelper = GetHelperObjectFromID( PPG.Inspected(0), Number(PSet.ID.Value) );
	
	if (oHelper != null)
	{
		oHelper.SoloCurrentTrigger(Boolean(PSet.Solo.Value));
	}
}

function HelperBoneTool_TriggerID_OnChanged()
{
	vstDebugTrace("HelperBoneTool_TriggerID_OnChanged");
	if ( PPG.EditInProgress.Value ) return;

	var oHelper = GetHelperObjectFromID( PPG.Inspected(0), Number(PSet.ID.Value) );
	
	if (oHelper != null && oHelper.currenttrigger!=-1)
	{
		var griddata = oHelper.data.Parameters("Triggers").Value;

		PSet.TriggerTolerance.Value = griddata.getcell(idTriggerTol, oHelper.currenttrigger);
		PSet.TriggerRotX.Value = griddata.getcell(idTriggerRotX, oHelper.currenttrigger);
		PSet.TriggerRotY.Value = griddata.getcell(idTriggerRotY, oHelper.currenttrigger);
		PSet.TriggerRotZ.Value = griddata.getcell(idTriggerRotZ, oHelper.currenttrigger);
		PSet.TargetRotX.Value = griddata.getcell(idTargetRotX, oHelper.currenttrigger);
		PSet.TargetRotY.Value = griddata.getcell(idTargetRotY, oHelper.currenttrigger);
		PSet.TargetRotZ.Value = griddata.getcell(idTargetRotZ, oHelper.currenttrigger);
		PSet.TargetPosX.Value = griddata.getcell(idTargetPosX, oHelper.currenttrigger);
		PSet.TargetPosY.Value = griddata.getcell(idTargetPosY, oHelper.currenttrigger);
		PSet.TargetPosZ.Value = griddata.getcell(idTargetPosZ, oHelper.currenttrigger);
	
		// update solo'd
		var bSolo = Boolean(griddata.getcell(0, oHelper.currenttrigger));
		for (var iRow=0; iRow < griddata.rowcount; iRow++)
		{
			if (iRow==oHelper.currenttrigger) continue;
			if (Boolean(griddata.getcell(0, iRow)))
			{
				bSolo = false;
				break;
			}
		}
		PSet.Solo.Value = bSolo;

	}		
}
		
function HelperBoneTool_TriggerTolerance_OnChanged()
{
	vstDebugTrace("HelperBoneTool_TriggerTolerance_OnChanged");
	if ( PPG.EditInProgress.Value ) return;

	var oHelper = GetHelperObjectFromID( PPG.Inspected(0), Number(PSet.ID.Value) );
	
	if (oHelper != null && oHelper.currenttrigger!=-1)
	{
		var griddata = oHelper.data.Parameters("Triggers").Value;		
		griddata.setcell( idTriggerTol, oHelper.currenttrigger, Number(PSet.TriggerTolerance.Value) );
	}
}

function HelperBoneTool_TriggerRotX_OnChanged()
{
	vstDebugTrace("HelperBoneTool_TriggerRotX_OnChanged");
	if ( PPG.EditInProgress.Value ) return;

	var oHelper = GetHelperObjectFromID( PPG.Inspected(0), Number(PSet.ID.Value) );
	
	if (oHelper != null && oHelper.currenttrigger!=-1)
	{
		var griddata = oHelper.data.Parameters("Triggers").Value;		
		griddata.setcell( idTriggerRotX, oHelper.currenttrigger, Number(PSet.TriggerRotX.Value) );
	
		if (Boolean(PSet.Solo.Value))
			oHelper.PoseChildToTrigger(oHelper.currenttrigger);
	}
}

function HelperBoneTool_TriggerRotY_OnChanged()
{
	vstDebugTrace("HelperBoneTool_TriggerRotY_OnChanged");
	if ( PPG.EditInProgress.Value ) return;

	var oHelper = GetHelperObjectFromID( PPG.Inspected(0), Number(PSet.ID.Value) );
	
	if (oHelper != null && oHelper.currenttrigger!=-1)
	{
		var griddata = oHelper.data.Parameters("Triggers").Value;		
		griddata.setcell( idTriggerRotY, oHelper.currenttrigger, Number(PSet.TriggerRotY.Value) );

		if (Boolean(PSet.Solo.Value))
			oHelper.PoseChildToTrigger(oHelper.currenttrigger);
	}
}

function HelperBoneTool_TriggerRotZ_OnChanged()
{
	vstDebugTrace("HelperBoneTool_TriggerRotZ_OnChanged");
	if ( PPG.EditInProgress.Value ) return;

	var oHelper = GetHelperObjectFromID( PPG.Inspected(0), Number(PSet.ID.Value) );
	
	if (oHelper != null && oHelper.currenttrigger!=-1)
	{
		var griddata = oHelper.data.Parameters("Triggers").Value;		
		griddata.setcell( idTriggerRotZ, oHelper.currenttrigger, Number(PSet.TriggerRotZ.Value) );

		if (Boolean(PSet.Solo.Value))
			oHelper.PoseChildToTrigger(oHelper.currenttrigger);
	}
}

function HelperBoneTool_TargetRotX_OnChanged()
{
	vstDebugTrace("HelperBoneTool_TargetRotX_OnChanged");
	if ( PPG.EditInProgress.Value ) return;

	var oHelper = GetHelperObjectFromID( PPG.Inspected(0), Number(PSet.ID.Value) );
	
	if (oHelper != null && oHelper.currenttrigger!=-1)
	{
		var griddata = oHelper.data.Parameters("Triggers").Value;		
		griddata.setcell( idTargetRotX, oHelper.currenttrigger, Number(PSet.TargetRotX.Value) );
	}
}

function HelperBoneTool_TargetRotY_OnChanged()
{
	vstDebugTrace("HelperBoneTool_TargetRotY_OnChanged");
	if ( PPG.EditInProgress.Value ) return;

	var oHelper = GetHelperObjectFromID( PPG.Inspected(0), Number(PSet.ID.Value) );
	
	if (oHelper != null && oHelper.currenttrigger!=-1)
	{
		var griddata = oHelper.data.Parameters("Triggers").Value;		
		griddata.setcell( idTargetRotY, oHelper.currenttrigger, Number(PSet.TargetRotY.Value) );
	}
}

function HelperBoneTool_TargetRotZ_OnChanged()
{
	vstDebugTrace("HelperBoneTool_TargetRotZ_OnChanged");
	if ( PPG.EditInProgress.Value ) return;

	var oHelper = GetHelperObjectFromID( PPG.Inspected(0), Number(PSet.ID.Value) );
	
	if (oHelper != null && oHelper.currenttrigger!=-1)
	{
		var griddata = oHelper.data.Parameters("Triggers").Value;		
		griddata.setcell( idTargetRotZ, oHelper.currenttrigger, Number(PSet.TargetRotZ.Value) );
	}
}

function HelperBoneTool_TargetPosX_OnChanged()
{
	vstDebugTrace("HelperBoneTool_TargetPosX_OnChanged");
	if ( PPG.EditInProgress.Value ) return;

	var oHelper = GetHelperObjectFromID( PPG.Inspected(0), Number(PSet.ID.Value) );
	
	if (oHelper != null && oHelper.currenttrigger!=-1)
	{
		var griddata = oHelper.data.Parameters("Triggers").Value;		
		griddata.setcell( idTargetPosX, oHelper.currenttrigger, Number(PSet.TargetPosX.Value) );
	}
}

function HelperBoneTool_TargetPosY_OnChanged()
{
	vstDebugTrace("HelperBoneTool_TargetPosY_OnChanged");
	if ( PPG.EditInProgress.Value ) return;

	var oHelper = GetHelperObjectFromID( PPG.Inspected(0), Number(PSet.ID.Value) );
	
	if (oHelper != null && oHelper.currenttrigger!=-1)
	{
		var griddata = oHelper.data.Parameters("Triggers").Value;		
		griddata.setcell( idTargetPosY, oHelper.currenttrigger, Number(PSet.TargetPosY.Value) );
	}
}

function HelperBoneTool_TargetPosZ_OnChanged()
{
	vstDebugTrace("HelperBoneTool_TargetPosZ_OnChanged");
	if ( PPG.EditInProgress.Value ) return;

	var oHelper = GetHelperObjectFromID( PPG.Inspected(0), Number(PSet.ID.Value) );
	
	if (oHelper != null && oHelper.currenttrigger!=-1)
	{
		var griddata = oHelper.data.Parameters("Triggers").Value;		
		griddata.setcell( idTargetPosZ, oHelper.currenttrigger, Number(PSet.TargetPosZ.Value) );
	}
}

function HelperBoneTool_RebuildHelperBoneCombo(oThis, helperbones)
{
	vstDebugTrace("HelperBoneTool_RebuildHelperBoneCombo " + String(helperbones));
	
	var aItems = Array();
	
	if (helperbones.count) 
	{
		var modelfullname = helperbones(0).model.fullname;
		var aValues = String(helperbones).split(",").sort();
		oThis.PathID.Value = aValues.join(",");
		
		aItems = new Array( aValues.length*2 );
		
		for (var i=0; i<aValues.length;i++)
		{
			aItems[ (i*2)+0 ] = aValues[i].substring(modelfullname.length+1);
			aItems[ (i*2)+1 ] = i;
		}
	}
		
	oThis.PPGLayout.Item("ID").UIItems = aItems;

}

function HelperBoneTool_RebuildTriggerCombo(oThis,griddata)
{
	var aItems = new Array( griddata.rowcount*2 );
	
	for (var iRow=0; iRow<griddata.rowcount;iRow++)
	{
		aItems[ (iRow*2)+0 ] = griddata.getcell(idTriggerName,iRow);
		aItems[ (iRow*2)+1 ] = iRow;
	}
	
	oThis.PPGLayout.Item("TriggerID").UIItems = aItems;
}

function HelperBoneTool_AddTrigger_OnClicked()
{
	vstDebugTrace("HelperBoneTool_AddTrigger_OnClicked");
	
	var oHelper = GetHelperObjectFromID( PPG.Inspected(0), Number(PSet.ID.Value) );
	
	if (oHelper != null)
	{
		var newidx = vstAddHelperBoneTrigger( oHelper.object );
		if (-1 != newidx)
		{
			HelperBoneTool_RebuildTriggerCombo(PPG,oHelper.data.parameters("Triggers").value);
			TriggerID.Value = newidx;
			PPG.Refresh();
		}
	}	
}

function HelperBoneTool_RemoveTrigger_OnClicked()
{
	vstDebugTrace("HelperBoneTool_RemoveTrigger_OnClicked");
	
	var oHelper = GetHelperObjectFromID( PPG.Inspected(0), Number(PSet.ID.Value) );
	
	if (oHelper != null &&  oHelper.currenttrigger!=-1)
	{
		try {
			if (vstRemoveHelperBoneTrigger( oHelper.object, oHelper.currenttrigger, true ))
			{
				HelperBoneTool_RebuildTriggerCombo(PPG,oHelper.data.parameters("Triggers").value);
				TriggerID.Value = oHelper.currenttrigger-1
				PPG.Refresh();
			}
		} catch (e) {;}		
	}
}

function HelperBoneTool_RenameTrigger_OnClicked()
{
	vstDebugTrace("HelperBoneTool_RenameTrigger_OnClicked");

	var oHelper = GetHelperObjectFromID( PPG.Inspected(0), Number(PSet.ID.Value) );
	
	if ( oHelper == null ) return;
	
	if ( -1 != oHelper.currenttrigger )
	{
		var aUIItems = PPGLayout.Item("TriggerID").UIItems.toArray();
		var triggername = aUIItems[ oHelper.currenttrigger * 2 ]
		
		var newname = vstInputBox( "New name:", "Rename " + triggername, triggername );
		
		if ( String(newname) != "" )
		{
			// update name in bonedata
			var griddata = oHelper.data.parameters("Triggers").value;
			griddata.setcell(idTriggerName, oHelper.currenttrigger, newname);
			
			HelperBoneTool_RebuildTriggerCombo(PPG,oHelper.data.parameters("Triggers").value);
			PPG.Refresh();
		}
	}
}

function HelperBoneTool_RemoveAllTriggers_OnClicked()
{
	vstDebugTrace("HelperBoneTool_RemoveAllTriggers_OnClicked");

	var oHelper = GetHelperObjectFromID( PPG.Inspected(0), Number(PSet.ID.Value) );

	if ( oHelper!=null && vstRemoveAllHelperBoneTriggers( oHelper.object, true ) )
	{
		HelperBoneTool_TriggerSetAsDefault( PPG );
		PPG.Refresh();
	}
}

function HelperBoneTool_EnableAllTriggers_OnClicked()
{
	vstDebugTrace("HelperBoneTool_EnableAllTriggers_OnClicked");

	var oHelper = GetHelperObjectFromID( PPG.Inspected(0), Number(PSet.ID.Value) );

	if ( oHelper!=null  )
	{
		oHelper.EnableTriggers(true);
	}
}

function HelperBoneTool_ResetChain_OnClicked()
{
	vstDebugTrace("HelperBoneTool_ResetChain_OnClicked");

	vstHelperBonesEnableAll(PPG.Inspected(0).Parent);
	PSet.Mute.Value = false;
	PSet.Solo.Value = false;
}

function HelperBoneTool_DisableAllTriggers_OnClicked()
{
	vstDebugTrace("HelperBoneTool_DisableAllTriggers_OnClicked");

	var oHelper = GetHelperObjectFromID( PPG.Inspected(0), Number(PSet.ID.Value) );

	if ( oHelper!=null  )
	{
		oHelper.EnableTriggers(false);
	}
}

function HelperBoneTool_SetTriggerRot_OnClicked()
{
	vstDebugTrace("HelperBoneTool_SetTriggerRot_OnClicked");

	var oHelper = GetHelperObjectFromID( PPG.Inspected(0), Number(PSet.ID.Value) );

	if ( oHelper!=null && oHelper.currenttrigger!=-1 )
	{
		vstSetHelperBoneTriggerRot( oHelper.object, oHelper.currenttrigger );

		aRot = 	oHelper.GetTriggerRotation(oHelper.currenttrigger);

		PSet.TriggerRotX.Value = aRot[0];
		PSet.TriggerRotY.Value = aRot[1];
		PSet.TriggerRotZ.Value = aRot[2];
	}
}

function HelperBoneTool_ClearTriggerRot_OnClicked()
{
	vstDebugTrace("HelperBoneTool_ClearTriggerRot_OnClicked");

	var oHelper = GetHelperObjectFromID( PPG.Inspected(0), Number(PSet.ID.Value) );

	if ( oHelper!=null && oHelper.currenttrigger!=-1 )
	{
		oHelper.SetTriggerRotation( oHelper.currenttrigger, 0, 0, 0 );
		PSet.TriggerRotX.Value = 0;
		PSet.TriggerRotY.Value = 0;
		PSet.TriggerRotZ.Value = 0;
		if (Boolean(PSet.Solo.Value))
			oHelper.PoseChildToTrigger(oHelper.currenttrigger);
	}
}

function HelperBoneTool_ClearTargetRot_OnClicked()
{
	vstDebugTrace("HelperBoneTool_ClearTargetRot_OnClicked");

	var oHelper = GetHelperObjectFromID( PPG.Inspected(0), Number(PSet.ID.Value) );

	if ( oHelper!=null && oHelper.currenttrigger!=-1 )
	{
		oHelper.SetTargetRotation( oHelper.currenttrigger, 0, 0, 0 );
		PSet.TargetRotX.Value = 0;
		PSet.TargetRotY.Value = 0;
		PSet.TargetRotZ.Value = 0;
	}
}

function HelperBoneTool_ClearTargetPos_OnClicked()
{
	vstDebugTrace("HelperBoneTool_ClearTargetRot_OnClicked");

	var oHelper = GetHelperObjectFromID( PPG.Inspected(0), Number(PSet.ID.Value) );

	if ( oHelper!=null && oHelper.currenttrigger!=-1 )
	{
		oHelper.SetTargetPosition( oHelper.currenttrigger, 0, 0, 0 );
		PSet.TargetPosX.Value = 0;
		PSet.TargetPosY.Value = 0;
		PSet.TargetPosZ.Value = 0;
	}
}

function HelperBoneTool_ImportVRD_OnClicked()
{
	vstDebugTrace("HelperBoneTool_Import_OnClicked");
	
	var rig = PPG.Inspected(0).Parent;

	if (vstImportHelperBones( rig, PSet.ImportVRDFile.Value))
	{
		HelperBoneTool_Init(rig);
		PPG.Refresh();
	}
}

function HelperBoneTool_EditImportVRD_OnClicked()
{
	vstDebugTrace("HelperBoneTool_EditImportVRD_OnClicked");
	
	var oLayout = Desktop.ActiveLayout;
	var oView = oLayout.CreateView( "Script Editor", "VRD Import File")
	oView.SetAttributeValue("filename",PSet.ImportVRDFile.Value);
}

function HelperBoneTool_EditExportVRD_OnClicked()
{
	vstDebugTrace("HelperBoneTool_EditExportVRD_OnClicked");
	
	var oLayout = Desktop.ActiveLayout;
	var oView = oLayout.CreateView( "Script Editor", "VRD Export File")
	oView.SetAttributeValue("filename",PSet.ExportVRDFile.Value);
}

function HelperBoneTool_ExportVRD_OnClicked()
{
	vstDebugTrace("HelperBoneTool_Import_OnClicked");
	vstExportHelperBones( null, PSet.ExportVRDFile.Value, true );
}

function HelperBoneTool_Help_OnClicked()
{
	vstDebugTrace("HelperBoneTool_Help_OnClicked");
	
	var tool = Application.Plugins(gToolName);
	if ( tool.Help != "" )
	{
		Application.OpenNetView(tool.Help);
	}
}

function HelperBoneTool_Tools_OnClicked()
{
	vstDebugTrace("HelperBoneTool_Tools_OnClicked");
	
	Application.OpenView("ValveSource HelperBone Toolbar");
}

function HelperBoneTool_RootBoneWireframeColorR_OnChanged()
{
	vstDebugTrace("HelperBoneTool_RootBoneWireframeColorR_OnChanged");
	if ( PPG.EditInProgress.Value ) return;
	HelperBoneTool_RootBoneWireframeColor_OnChanged();
}

function HelperBoneTool_RootBoneWireframeColorG_OnChanged()
{
	vstDebugTrace("HelperBoneTool_RootBoneWireframeColorG_OnChanged");
	if ( PPG.EditInProgress.Value ) return;

	HelperBoneTool_RootBoneWireframeColor_OnChanged();
}

function HelperBoneTool_RootBoneWireframeColorB_OnChanged()
{
	vstDebugTrace("HelperBoneTool_RootBoneWireframeColorB_OnChanged");
	if ( PPG.EditInProgress.Value ) return;

	HelperBoneTool_RootBoneWireframeColor_OnChanged();
}

function HelperBoneTool_RootBoneWireframeColor_OnChanged()
{
	var RootBone_color = RGBToWireframeColor(PPG.RootBoneWireframeColorR.Value,PPG.RootBoneWireframeColorG.Value,PPG.RootBoneWireframeColorB.Value);
	
	SetWireframeColor( PPG.Inspected(0).Parent.Groups("HelperBoneRoot"), RootBone_color );
}

function HelperBoneTool_ParentBoneWireframeColorR_OnChanged()
{
	vstDebugTrace("HelperBoneTool_ParentBoneWireframeColorR_OnChanged");
	if ( PPG.EditInProgress.Value ) return;

	HelperBoneTool_ParentBoneWireframeColor_OnChanged();
}

function HelperBoneTool_ParentBoneWireframeColorG_OnChanged()
{
	vstDebugTrace("HelperBoneTool_ParentBoneWireframeColorG_OnChanged");
	if ( PPG.EditInProgress.Value ) return;

	HelperBoneTool_ParentBoneWireframeColor_OnChanged();
}

function HelperBoneTool_ParentBoneWireframeColorB_OnChanged()
{
	vstDebugTrace("HelperBoneTool_ParentBoneWireframeColorB_OnChanged");
	if ( PPG.EditInProgress.Value ) return;

	HelperBoneTool_ParentBoneWireframeColor_OnChanged();
}

function HelperBoneTool_ParentBoneWireframeColor_OnChanged()
{
	var ParentBone_color = RGBToWireframeColor(PPG.ParentBoneWireframeColorR.Value,PPG.ParentBoneWireframeColorG.Value,PPG.ParentBoneWireframeColorB.Value);
	
	SetWireframeColor( PPG.Inspected(0).Parent.Groups("HelperBoneParent"), ParentBone_color );
}

function HelperBoneTool_ChildBoneWireframeColorR_OnChanged()
{
	vstDebugTrace("HelperBoneTool_ChildBoneWireframeColorR_OnChanged");
	if ( PPG.EditInProgress.Value ) return;

	HelperBoneTool_ChildBoneWireframeColor_OnChanged();
}

function HelperBoneTool_ChildBoneWireframeColorG_OnChanged()
{
	vstDebugTrace("HelperBoneTool_ChildBoneWireframeColorG_OnChanged");
	if ( PPG.EditInProgress.Value ) return;

	HelperBoneTool_ChildBoneWireframeColor_OnChanged();
}

function HelperBoneTool_ChildBoneWireframeColorB_OnChanged()
{
	vstDebugTrace("HelperBoneTool_ChildBoneWireframeColorB_OnChanged");
	if ( PPG.EditInProgress.Value ) return;

	HelperBoneTool_ChildBoneWireframeColor_OnChanged();
}

function HelperBoneTool_ChildBoneWireframeColor_OnChanged()
{
	var childbone_color = RGBToWireframeColor(PPG.ChildBoneWireframeColorR.Value,PPG.ChildBoneWireframeColorG.Value,PPG.ChildBoneWireframeColorB.Value);
	
	SetWireframeColor( PPG.Inspected(0).Parent.Groups("HelperBoneChild"), childbone_color );
}

function HelperBoneTool_SelectHelper_OnClicked()
{
	var oHelper = GetHelperObjectFromID( PPG.Inspected(0), Number(PSet.ID.Value) );
	SelectObj( oHelper.object );
}

function HelperBoneTool_SelectRoot_OnClicked()
{
	SelectObj( PPG.HelperBoneRoot.Value );
}

function HelperBoneTool_SelectParent_OnClicked()
{
	SelectObj( PPG.HelperBoneParent.Value );
}

function HelperBoneTool_SelectChild_OnClicked()
{
	SelectObj( PPG.HelperBoneChild.Value );
}

function HelperBoneTool_ToggleLRHelper_OnClicked()
{
	var oHelper = GetHelperObjectFromID( PPG.Inspected(0), Number(PSet.ID.Value) );
	
	var fullname = oHelper.object.fullname;
	var Lfullname = oHelper.object.fullname.replace(/_R_/g, "_L_");
	
	if ( Lfullname != fullname )
	{
		SelectObj( Lfullname );
		return;
	}
	
	var Rfullname = oHelper.object.fullname.replace(/_L_/g, "_R_");
	
	if ( Rfullname != fullname )
	{
		SelectObj( Rfullname );
		return;
	}
}

function HelperBoneTool_DebugLevel_OnChanged()
{
	vstDebugTrace("HelperBoneTool_DebugLevel_OnChanged");

	vstDebugSetLevel( PSet.DebugLevel.Value );
}

function HelperBoneTool_EditHelperBoneData_OnClicked()
{
	vstDebugTrace("HelperBoneTool_DebugLevel_OnChanged");

	var oHelper = GetHelperObjectFromID( PPG.Inspected(0), Number(PSet.ID.Value) );

	if ( oHelper )
		InspectObj( oHelper.data );
}


//////////////////////////////////////////////////////////////////////////////////////
// Operator		:	HelperBonePrimOp
// Description	:	Primitive operator which generated helperbone's geometry
//////////////////////////////////////////////////////////////////////////////////////
function HelperBonePrimOp_GetFaces()
{
	var i = 0;
	var f = new Array( 6 * 5 );

	f[i++] = 4; f[i++] = 0; ; f[i++] = 2; f[i++] = 3; f[i++] = 1; 
	f[i++] = 4; f[i++] = 0; ; f[i++] = 1; f[i++] = 5; f[i++] = 4; 
	f[i++] = 4; f[i++] = 0; ; f[i++] = 4; f[i++] = 6; f[i++] = 2; 
	f[i++] = 4; f[i++] = 1; ; f[i++] = 3; f[i++] = 7; f[i++] = 5; 
	f[i++] = 4; f[i++] = 2; ; f[i++] = 6; f[i++] = 7; f[i++] = 3; 
	f[i++] = 4; f[i++] = 4; ; f[i++] = 5; f[i++] = 7; f[i++] = 6; 

	return f;
}

function HelperBonePrimOp_GeneratePrim( g, center_pivot, length, width, height )
{
	var pvtz = 0.0;
	if ( center_pivot == true ) {
		pvtz = ( length/2.0) * -1; 
	}

	var vsize = 8*3;
	var v = new Array( vsize );
 
	i=0;  
	v[i++] = -width / 2; 
	v[i++] = -height / 2; 
	v[i++] = pvtz;

	v[i++] = width / 2 ;
	v[i++] = -height / 2;
	v[i++] = pvtz;
	 
	v[i++] = -width / 2;
	v[i++] = height / 2; 
	v[i++] = pvtz;

	v[i++] = width / 2; 
	v[i++] = height / 2;
	v[i++] = pvtz;

	var offset = i;

	for (; i<vsize; ) {
		v[i] = v[i-offset]; i++;
		v[i] = v[i-offset]; i++;
		v[i] = v[i-offset] + length; i++;
	} 
	
	g.set( v, HelperBonePrimOp_GetFaces() );
}

function HelperBonePrimOp_Update( ctx, output, inprim )
{
	var center_pivot = (ctx.Parameters.Item("CenterPivot").Value!=0);
	var length = ctx.Parameters.Item("Length").Value;
	var width = ctx.Parameters.Item("Width").Value;
	var height = ctx.Parameters.Item("Height").Value;

	HelperBonePrimOp_GeneratePrim( output.value.geometry, center_pivot, length, width, height );	
}

//////////////////////////////////////////////////////////////////////////////////////
// Command		:	vstApplyHelperBonePrimOp
// Description	:	apply primitive operator which generated helperbone's geometry
//////////////////////////////////////////////////////////////////////////////////////
function vstApplyHelperBonePrimOp_Init( ctxt )
{
	vstDebugTrace("vstApplyHelperBonePrimOp_Init");
	
	var oCmd = ctxt.Source;
	oCmd.Description = "";
	oCmd.ReturnValue = true;

	var oArgs = oCmd.Arguments;
	oArgs.AddWithHandler("HelperBone",siArgHandlerSingleObj);
	
	return true;
}

function vstApplyHelperBonePrimOp_Execute(helper)
{
	vstDebugTrace("vstApplyHelperBonePrimOp_Execute");
	
	var code = 
		HelperBonePrimOp_GetFaces.toString() +
		HelperBonePrimOp_GeneratePrim.toString() + 
		HelperBonePrimOp_Update.toString();
		
	var op = XSIFactory.CreateScriptedOp( "HelperBonePrimOp", code, "JScript" );

	// define connections
	var maingroup = op.addportgroup( "MainGroup" );
	var ioport = op.addioport( helper.activeprimitive, "Prim", maingroup.index );

	// define parameters
	op.addparameter(XSIFactory.CreateParamDef2( "CenterPivot", siBool, true ));
	op.addparameter(XSIFactory.CreateParamDef2( "Length", siDouble, 5, 0, 1000 ));
	op.addparameter(XSIFactory.CreateParamDef2( "Width", siDouble, 5, 0, 1000 ));
	op.addparameter(XSIFactory.CreateParamDef2( "Height", siDouble, 5, 0, 1000 ));

	// connect operator
	op.connect(helper);
	
	return op;
}
	
//////////////////////////////////////////////////////////////////////////////////////
// Command		:	vstApplyHelperBoneOp
// Description	:	apply helperboneop operator 
//////////////////////////////////////////////////////////////////////////////////////
function vstApplyHelperBoneOp_Init( ctxt )
{
	vstDebugTrace("vstApplyHelperBoneOp_Init");
	
	var oCmd = ctxt.Source;
	oCmd.Description = "";
	oCmd.ReturnValue = true;

	var oArgs = oCmd.Arguments;
	oArgs.AddWithHandler("HelperBone",siArgHandlerSingleObj);
	oArgs.AddWithHandler("Root",siArgHandlerSingleObj);
	oArgs.AddWithHandler("Parent",siArgHandlerSingleObj);
	oArgs.AddWithHandler("Child",siArgHandlerSingleObj);
	
	return true;
}

function vstApplyHelperBoneOp_Execute(helper, root, parent, child)
{
	vstDebugTrace("vstApplyHelperBoneOp_Execute");
	
	var strConnection = Array(helper, root, parent, child).join(";");

	return ApplyOperator( gHelperBoneOpName, strConnection );
}
	

//////////////////////////////////////////////////////////////////////////////////////
// Command		:	vstCreateHelperBones
// Description		:	Add helper bone tool to rig
//////////////////////////////////////////////////////////////////////////////////////
function vstCreateHelperBones_Init( ctxt )
{
	vstDebugTrace("CreateHelperBoneTool_Init");
	
	var oCmd = ctxt.Source;
	oCmd.Description = "";
	oCmd.SetFlag(siSupportsKeyAssignment,true);
	oCmd.ReturnValue = true;
	oCmd.Description = "Create and setup helperbone tool and add it to Valve rig";

	var oArgs = oCmd.Arguments;
	oArgs.AddWithHandler("Rig",siArgHandlerCollection);
	oArgs.Add("Interactive",siBool,true);
	
	return true;
}

function vstCreateHelperBones_Execute(objects,interactive)
{
	vstDebugTrace("vstCreateHelperBones_Execute");

	var rig = GetRigObject(objects(0));
	
	if (rig==null)
	{
		try { 
			rig = GetRigObject(Dictionary.GetObject(gDefaultRig));
		} catch(e) {;}
	}
	
	if (rig)
	{
		var tool = rig.GetHelperBoneTool();

		var group, color;
		
		group = rig.model.AddGroup();
		group.Name = "HelperBones";
		
		group = rig.model.AddGroup();
		group.Name = "HelperBoneRoot";
		group.addproperty("Display Property");
		SetWireframeColor(group, RGBToWireframeColor(tool.RootBoneWireframeColorR.Value,tool.RootBoneWireframeColorG.Value,tool.RootBoneWireframeColorB.Value) );
		
		group = rig.model.AddGroup();
		group.Name = "HelperBoneParent";
		group.addproperty("Display Property");
		SetWireframeColor(group, RGBToWireframeColor(tool.ParentBoneWireframeColorR.Value,tool.ParentBoneWireframeColorG.Value,tool.ParentBoneWireframeColorB.Value) );

		group = rig.model.AddGroup();
		group.Name = "HelperBoneChild";
		group.addproperty("Display Property");
		SetWireframeColor(group, RGBToWireframeColor(tool.ChildBoneWireframeColorR.Value,tool.ChildBoneWireframeColorG.Value,tool.ChildBoneWireframeColorB.Value) );
		
		if ( interactive ) 
		{
			InspectObj( tool,"", gToolName, siLock ) ;
		}
	}
	else
	{
		if (interactive)
			XSIUIToolkit.MsgBox( "Please load a Valve Rig", siMsgOkOnly, gToolName );
		else
			Application.LogMessage( gToolName + ": InvalidArgument", siError);
	}
	
	return tool;
}

//////////////////////////////////////////////////////////////////////////////////////
// Command		:	vstDeselectHelperBone
//////////////////////////////////////////////////////////////////////////////////////
function vstDeselectHelperBone_Init( ctxt )
{
	vstDebugTrace("vstDeselectHelperBone_Init");
	
	var oCmd = ctxt.Source;
	oCmd.Description = "Deselect helperbone from editing";
	
	return true;
}

function vstDeselectHelperBone_Execute()
{
	//vstDebugTrace("vstDeselectHelperBone_Execute: ");

	var col = XSIFactory.CreateObject( "XSI.Collection" );
	try {
		col.SetAsText ( "*.HelperBoneTool" );
	} catch (e) {;}
	
	for (var i=0; i<col.count; i++)
	{
		var o = col(i);

		var model = o.parent;
		
		var group1 = model.Groups("HelperBoneRoot");

		if (group1) 
		{
			if (group1.members && group1.members.count) 
			{
				SIRemoveFromGroup(  group1, group1.Members );
			}	
		}
		var group2 = model.Groups("HelperBoneParent");
		if (group2) 
		{
			if (group2.members && group2.members.count) 
			{
				SIRemoveFromGroup(  group2, group2.Members );
			}	
		}
		var group3 = model.Groups("HelperBoneChild");
		if (group3) 
		{
			if (group3.members && group3.members.count) 
			{
				SIRemoveFromGroup( group3, group3.Members );
			}	
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////
// Command		:	vstSelectHelperBone
//////////////////////////////////////////////////////////////////////////////////////
function vstSelectHelperBone_Init( ctxt )
{
	vstDebugTrace("vstSelectHelperBone_Init");
	
	var oCmd = ctxt.Source;
	oCmd.Description = "Select helperbone for editing";
	oCmd.ReturnValue = true;
	oCmd.setflag( siNoLogging, false );
	var oArgs = oCmd.Arguments;
	oArgs.AddWithHandler("HelperBone",siArgHandlerCollection);
	
	return true;
}

function vstSelectHelperBone_Execute(col)
{
	vstDebugTrace("vstSelectHelperBone_Execute: " + String(col));

	if (!col.count) return false;
	
	var o = col(0);
	var helper = vstGetHelperBoneObject(o);
	
	if ( helper==null ) 
	{ 
		vstDebugTrace("vstSelectHelperBone_Execute: object not helper");
		return false;
	}

	var oThis = helper.GetTool();

	oThis.EditInProgress.Value = true;
	
	oThis.Mute.Value = false==Boolean(helper.data.Parameters("Enabled").Value);
	
	oThis.HelperBoneRoot.Value = helper.GetRoot().fullname;
	oThis.HelperBoneParent.Value = helper.GetParent().fullname;
	oThis.HelperBoneChild.Value = helper.GetChild().fullname;
	
	oThis.BoneDistance.Value = helper.data.Parameters("BoneDistance").Value;

	oThis.RootBoneLength.ReadOnly = false;	
	oThis.RootBoneLength.Value = helper.data.Parameters("RootBoneLength").Value;
	oThis.RootBoneLength.ReadOnly = true;	
	oThis.BoneOffsetX.Value = helper.data.Parameters("BoneOffsetX").Value;
	oThis.BoneOffsetY.Value = helper.data.Parameters("BoneOffsetY").Value;
	oThis.BoneOffsetZ.Value = helper.data.Parameters("BoneOffSetZ").Value;
	oThis.BoneWidth.Value = helper.GetPrimOp().Parameters("Width").value;
	oThis.BoneHeight.Value = helper.GetPrimOp().Parameters("Height").value;
	oThis.BoneLength.Value = helper.GetPrimOp().Parameters("Length").value;

	// set trigger data
	var griddata = helper.data.Parameters("Triggers").Value;
	HelperBoneTool_SetTrigger( oThis, griddata, 0 );
	
	oThis.TriggerID.Value = (griddata.rowcount) ? 0 : -1;
		
	// this will cause an OnChanged event
	if ( oThis.PrevID.Value != helper.GetID() ) 
	{	
		// restore previous helper bone Wireframe color
		var group1 = helper.GetModel().Groups("HelperBoneRoot");
		if ( group1 && group1.members.count )
			SIRemoveFromGroup(  group1, group1.Members );
			
		var group2 = helper.GetModel().Groups("HelperBoneParent");
		if ( group2 && group2.members.count )
			SIRemoveFromGroup(  group2, group2.Members );
			
		var group3 = helper.GetModel().Groups("HelperBoneChild");
		if ( group3 && group3.members.count )
			SIRemoveFromGroup(  group3, group3.Members );
		
		oThis.ID.Value = helper.GetID();
	}
	
	SIAddToGroup(  helper.GetModel().Groups("HelperBoneRoot"), helper.GetRoot() );
	SiAddToGroup(  helper.GetModel().Groups("HelperBoneParent"), helper.GetParent() );
	SIAddToGroup(  helper.GetModel().Groups("HelperBoneChild"), helper.GetChild() );
			
	oThis.PrevID.Value = oThis.ID.Value;
		
	oThis.EditInProgress.Value = false;
		
	return true;
}

function SetWireframeColor( object, color )
{
	vstDebugTrace("SetWireframeColor");
	
	var display = object.properties("Display");
	if (display.isa(siSharedPSet))
	{
		display = MakeLocal( display, siNodePropagation )(0);
	}
	
	display.parameters("wirecol").value = color;
}

////////////////////////////////////////////////////////////////
// Command: vstShowHelperBoneToolbar
////////////////////////////////////////////////////////////////

function vstShowHelperBoneToolbar_Execute()
{
	OpenView("ValveSource HelperBone Toolbar");
}

//////////////////////////////////////////////////////////
//
// RGB to Wire(frame) Color (FOR UI LOGIC)
//
// Wirecolor is a 10 bit value (0 to 1023), with the least
// significant bit igonred. 
// Each of the R,G,B channels is 3 bits (a number from 0 to 7).
// Encoded as:
//
// bit   9  8  7  6  5  4  3  2  1  0  
//     |B2|B1|B0|G2|G1|G0|R2|R1|R0| 0|
//     ---------|--------|--------|--|
//        blue    green     red 
//
// To convert to wirecolor, trucate each channel down to 3 bits
// and leftshift 1bit for Red, 4bits for Green and 7bits for blue.
//
///////////////////////////////////////////////////////////
function WireframeColorToRGB(lWireframeColor)
{
	var aColor = new Array(3);
	
	aColor[0] = ((lWireframeColor >>> 1) & 0x7)/7;
	aColor[1] = ((lWireframeColor >>> 4) & 0x7)/7;
	aColor[2] = ((lWireframeColor >>> 7) & 0x7)/7;
	 
	return aColor;
}

function RGBToWireframeColor(dR,dG,dB)
{
	//convert RGB to wirecolor
	var wirecolR,wirecolG,wirecolB;

	wirecolR = (Math.round(dR * 7)) << 1
	wirecolG = (Math.round(dG * 7)) << 4
	wirecolB = (Math.round(dB * 7)) << 7

	return wirecolR | wirecolG | wirecolB;
}

//////////////////////////////////////////////////////////////////////////////////////
// Property : HelperBoneData
//////////////////////////////////////////////////////////////////////////////////////

function HelperBoneData_Define( io_Context )
{
	var oThis =  io_Context.Source	
	oThis.AddParameter2("Enabled",siBool,true,false,true,false,true,0,siPersistable);
	oThis.AddParameter2("BoneDistance",siDouble,50,0,100,0,100,0,siPersistable | siAnimatable);
	oThis.AddParameter2("RootBoneLength",siDouble,0,0,100,0,100,0,siPersistable | siAnimatable);
	oThis.AddParameter2("BoneOffsetX",siDouble,0,-400,400,-400,400,0,siPersistable | siAnimatable);
	oThis.AddParameter2("BoneOffsetY",siDouble,0,-400,400,-400,400,0,siPersistable | siAnimatable);
	oThis.AddParameter2("BoneOffsetZ",siDouble,0,-400,400,-400,400,0,siPersistable | siAnimatable);
	oThis.AddGridParameter("Triggers");
	oThis.AddParameter2( "ComputationSpace", siInt4,0);
}

//////////////////////////////////////////////////////////////////////////////////////
// Object : Rig Object
//////////////////////////////////////////////////////////////////////////////////////

function GetRigObject( model )
{
	vstDebugTrace("GetRigObject");
	
	if (!model) return null;
	vstDebugTrace("GetRigObject: model = " +  model,4);
	
	var o = new Object();
	o.model = model;
	o.tool = model.Properties("HelperBoneTool");
	o.GetHelperBoneTool = Rig_GetHelperBoneTool;
	
	return o;
}

function Rig_GetHelperBoneTool()
{
	vstDebugTrace("Rig_GetHelperBoneTool");
	
	var tool = this.model.Properties("HelperBoneTool");
	
	if (!tool)
	{
		tool = this.model.AddProperty( "HelperBoneTool", false ) ;
	}
	
	return tool;
}

function GetHelperObjectFromID(tool, id)
{
	vstDebugTrace("GetHelperObjectFromID " + Number(id));

	var model = tool.parent;
	var group = model.groups("HelperBones");
	if (!group) return null;
	
	var helperbones = group.Members;
	if (!helperbones || !helperbones.count ) return null;

	var aPathItems = tool.PathID.Value.split(",");
	
	if ( id < 0 || id > aPathItems.length ) 
	{
		vstDebugTrace("GetHelperObjectFromID: invalid index");
		return null;
	}
	
	var o = null;
	try {
		o = Dictionary.GetObject( aPathItems[id] );
	}
	catch (e) { return null;}
	
	return vstGetHelperBoneObject(o);
}

function GetHelperBones(model)
{
	var col = XSIFactory.CreateObject("XSI.Collection");

	if (model.groups("HelperBones")==null)
		return col;
		
	col.additems(model.groups("HelperBones").Members);	
	
	return col;
}

//////////////////////////////////////////////////////////////////////////////////////
// Private Helper Functions
//////////////////////////////////////////////////////////////////////////////////////

function GetConfigFilesPath()
{
	return GetToolPath() + "Data\\ConfigFiles\\";
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

