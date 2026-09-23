//////////////////////////////////////////////////////////////////////////////////////
// ValveSource BoneSymmetryMap Tool
// Version v1.0a
// Author: Simon Inwood
//////////////////////////////////////////////////////////////////////////////////////

var gToolName = "ValveSource BoneSymmetryMap Tool";
var gDebug = false;

function XSILoadPlugin( in_reg )
{
	Trace( "ValveSource BoneSymmetryMap XSILoadPlugin called" );
	
	in_reg.Author = "Softimage";
	in_reg.Name = gToolName;
	in_reg.Major = 0;
	in_reg.Minor = 1;
	in_reg.Email = "xsisdk@softimage.com";
	in_reg.URL = "www.softimage.com";

	in_reg.RegisterCommand( "vstMirrorHelperBones", "vstMirrorHelperBones" );
	in_reg.RegisterCommand( "vstCreateBoneSymmetryMap", "vstCreateBoneSymmetryMap" );
	in_reg.RegisterProperty( "BoneSymmetryMap" );

	return true;
} 

//////////////////////////////////////////////////////////////////////////////////////
// Command : vstMirrorHelperBones
//////////////////////////////////////////////////////////////////////////////////////

function vstMirrorHelperBones_Init(in_ctxt)
{
	var oCmd = in_ctxt.Source;
	oCmd.Description = "Create mirrored helperbones using BoneSymmetryMap"	
}

function vstMirrorHelperBones_Execute()
{
	Trace("vstMirrorHelperBones_Execute");
	logmessage( "vstMirrorHelperBones: not implemented");
}

//////////////////////////////////////////////////////////////////////////////////////
// Property : BoneSymmetryMap
//////////////////////////////////////////////////////////////////////////////////////
function BoneSymmetryMap_Define( ctxt )
{
	var oCustomProperty;
	oCustomProperty = ctxt.Source;
	oCustomProperty.AddParameter2("MirrorPlane",siInt4,0,0,2,0,2,0,siPersistable);
	var oParam = oCustomProperty.AddGridParameter("MirrorData");
	var oGridData = oParam.Value;
	oGridData.RowCount = 1;
	oGridData.ColumnCount = 8;
	
	var i=0;
	oGridData.SetColumnLabel( i++, "Left Bone" ) ;
	oGridData.SetColumnLabel( i++, "Right Bone" ) ;
	oGridData.SetColumnLabel( i++, "PosX" ) ;
	oGridData.SetColumnLabel( i++, "PosY" ) ;
	oGridData.SetColumnLabel( i++, "PosZ" ) ;
	oGridData.SetColumnLabel( i++, "RotX" ) ;
	oGridData.SetColumnLabel( i++, "RotY" ) ;
	oGridData.SetColumnLabel( i++, "RotZ" ) ;

	i=0;
	oGridData.SetRowValues( i++, new Array("Bip01_L_Clavicle", "Bip01_R_Clavicle", 1, 1, -1, -1, -1, 1) ) ;

	return true;
}

function BoneSymmetryMap_DefineLayout( ctxt )
{
	var oLayout,oPPGItem;
	oLayout = ctxt.Source;
	oLayout.Clear();

	var aMirrorPlanes = new Array(
		"YZ plane (x=0)", 0,
		"XZ plane (y=0)", 1,
		"XY plane (z=0)", 2
	);	

	oLayout.AddStaticText( "PROTOTYPE ONLY - NON FUNCTIONAL");

	oLayout.AddRow();
		oPPGItem = oLayout.AddEnumControl( "MirrorPlane", aMirrorPlanes ) ;
		oPPGItem.SetAttribute( siUICX, 150 ) ;
		oPPGItem.WidthPercentage = 25 ;
		oPPGItem.LabelPercentage = 20 ;
		oPPGItem.LabelMinPixels = 40 ;	
		oPPGItem = oLayout.AddStaticText(" ");
		oPPGItem.SetAttribute( siUICX, 300 ) ;
		
	oLayout.EndRow();

	oLayout.AddGroup();
		oLayout.AddRow();
			oLayout.AddButton( "AddRow", "Add" );
			oLayout.AddButton( "RemoveRow", "Remove" );
			oLayout.AddStaticText(" ");
			oLayout.AddButton( "ValidateRow", "Validate" );
			oLayout.AddStaticText(" ");
			oLayout.AddButton( "PickBones", "Pick bones" );
		oLayout.EndRow();
		
		oPPGItem = oLayout.AddItem("MirrorData");
		oPPGItem.SetAttribute(siUICX, 550 ) ;
		oPPGItem.SetAttribute(siUIValueOnly,true);
		oPPGItem.SetAttribute(siUIGridLockColumnHeader, true );
		oPPGItem.SetAttribute(siUIGridHideRowHeader, true );
		oPPGItem.SetAttribute(siUIGridColumnWidths, "0:150:150:40:40:40:40:40:40");

		oLayout.AddRow();
			oLayout.AddButton( "ValidateAll", "ValidateAll" );
			oLayout.AddButton( "SortAZ", "SortAZ" );
			oLayout.AddStaticText(" ");
			oLayout.AddButton( "RemoveAllRows", "RemoveAll" );
			oLayout.AddStaticText(" ");
			oLayout.AddButton( "ImportSymmetryMapFromCSVFile", "Import CSV..." );
			oLayout.AddButton( "ExportSymmetryMapFromCSVFile", "Export CSV..." );
		oLayout.EndRow();
	oLayout.EndGroup();
	
	return true;
}

function BoneSymmetryMap_OnInit( )
{
	Application.LogMessage("OnInit called");
}

function BoneSymmetryMap_MirrorPlane_OnChanged( )
{
}

function BoneSymmetryMap_MirrorPlane_OnChanged( )
{
	Application.LogMessage("BoneSymmetryMap_MirrorPlane_OnChanged called");
	var oParam;
	oParam = PPG.MirrorPlane;
	var paramVal;
	paramVal = oParam.Value;
	Application.LogMessage("New value: " + paramVal);
}

function BoneSymmetryMap_MirrorData_OnChanged( )
{
	Application.LogMessage("BoneSymmetryMap_MirrorData_OnChanged called");
	var oParam = PPG.MirrorData;
	var oGridData = oParam.Value;
}

function BoneSymmetryMap_AddRow_OnClicked()
{
	var oParam = PPG.MirrorData;
	var oGridData = oParam.Value;
	
	oGridData.RowCount += 1;
}

function BoneSymmetryMap_PickBones_OnClicked()
{
	var oParam = PPG.MirrorData;
	var oGridData = oParam.Value;
	
	ActivateObjectSelTool();
	
	var rtn = PickObject( "Pick Left Bone", "Pick Right Bone" );
	var bone = rtn("PickedElement");
	var button = rtn("ButtonPressed");
	
	if (button==1)
	{	
		leftBone=bone;
		rightBone=bone;
	}
	else if (button==2) 
	{ 
		leftBone=bone;
		rightBone=bone;
	}
	
	// find matching bone
	
	// compute transform
	
	var idx = oGridData.RowCount;
	oGridData.RowCount += 1;
	oGridData.SetRowValues( idx, new Array( leftBone.Name, rightBone.Name, 0, 0, 0, 0, 0, 0 ) );
	
}

function BoneSymmetryMap_RemoveAllRows_OnClicked()
{
	var oParam = PPG.MirrorData;
	var oGridData = oParam.Value;
	
	oGridData.RowCount = 0;
}

//////////////////////////////////////////////////////////////////////////////////////
// Command : vstCreateBoneSymmetryMap
//////////////////////////////////////////////////////////////////////////////////////
function vstCreateBoneSymmetryMap_Init( ctxt )
{
	var oCmd = ctxt.Source;
	oCmd.Description = "Create a BoneSymmetryMap and add it to Valve rig"	
	oCmd.SetFlag(siSupportsKeyAssignment,true);
	oCmd.ReturnValue = true;

	var oArgs = oCmd.Arguments;
	oArgs.AddWithHandler("Models",siArgHandlerCollection);
	
	return true;
}

function vstCreateBoneSymmetryMap_Execute( objects )
{
	Application.LogMessage("vstCreateBoneSymmetryMap_Execute called");
	
	var colModels = sifilter( objects, siModelFilter );
	logmessage( typeof( colModels ) );
	
	if ( ""+colModels != "null" && colModels.Count )
	{
		var colProps = AddProp( "BoneSymmetryMap", colModels ).Value("Value");
		
		InspectObj( colProps );
	}
	
	return colProps;
}

//////////////////////////////////////////////////////////////////////////////////////
// Private Helper Functions
//////////////////////////////////////////////////////////////////////////////////////
function Trace( str )
{
	if ( gDebug )
		Application.Logmessage(str);
}
