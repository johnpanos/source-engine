//////////////////////////////////////////////////////////////////////////////////////
// ValveSource HelperBoneObject Tool
// Version v1.0
// Author: Simon Inwood
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
var gToolName = "ValveSource HelperBoneObject";
var gDebug = true;

function XSILoadPlugin( in_reg )
{
	Trace( "ValveSource HelperBoneObject XSILoadPlugin called" );
	
	in_reg.Author = "Softimage";
	in_reg.Name = gToolName;
	in_reg.Major = 1;
	in_reg.Minor = 0;
	in_reg.Email = "xsisdk@softimage.com";
	in_reg.URL = "www.softimage.com";

	in_reg.RegisterCommand( "vstGetHelperBoneObject", "vstGetHelperBoneObject" );
	in_reg.RegisterCommand( "vstHelperBonesEnableAll", "vstHelperBonesEnableAll" );

	return true;
} 

//////////////////////////////////////////////////////////////////////////////////////
// Command	   :	vstHelperBonesEnableAll
// Description :	Enable all helperbones and helperbone triggers
//////////////////////////////////////////////////////////////////////////////////////
function vstHelperBonesEnableAll_Init( ctxt )
{
	vstDebugTrace("vstGetHelperBoneObject_Init");
	
	var oCmd = ctxt.Source;
	oCmd.Description = "Enable all helperbones and helperbone triggers";
	oCmd.ReturnValue = false;
	oCmd.SetFlag(siNoLogging, false);

	var oArgs = oCmd.Arguments;
	oArgs.AddWithHandler("Model",siArgHandlerSingleObj);
	
	return true;
}

function vstHelperBonesEnableAll_Execute(model)
{
	if (!model || classname(model) != "Model" ) return;
	
	var helperbones = GetHelperBones(model);
	
	var eHelperBones = new Enumerator(helperbones);
	var colBones = XSIFactory.CreateObject("XSI.Collection");
	colBones.unique = true;
	
	for ( ; ! eHelperBones.atEnd(); eHelperBones.moveNext()) 
	{	
		var oHelper = GetHelperObject(eHelperBones.item());
		
		if ( oHelper!=null  )
		{
			oHelper.data.Parameters("Enabled").Value = true;
			oHelper.EnableTriggers(true);
			
			var child = oHelper.GetChild();
			if ( classname(child)=="ChainBone")
			{
				if (child.index==0)
					colBones.add( child );
				else
					colBones.add( child.root.bones(0) );
			}
		}
	}
	
	// reset chain
	var eBones = new Enumerator(colBones);
	for ( ; ! eBones.atEnd(); eBones.moveNext()) 
	{	
		var bone = eBones.item();
		
		var ikchain = bone.properties("Kinematic Chain");
		var val = ikchain.parameters("blendik").value; 
		ikchain.parameters("blendik").value = (val) ? 0 : 1;
		ikchain.parameters("blendik").value = val;
	}
}

//////////////////////////////////////////////////////////////////////////////////////
// Command	  :	vstGetHelperBoneObject
// Description :	return JScript object for helperbone
//////////////////////////////////////////////////////////////////////////////////////
function vstGetHelperBoneObject_Init( ctxt )
{
	vstDebugTrace("vstGetHelperBoneObject_Init");
	
	var oCmd = ctxt.Source;
	oCmd.Description = "return JScript object for helperbone";
	oCmd.ReturnValue = true;
	oCmd.SetFlag(siNoLogging, true);

	var oArgs = oCmd.Arguments;
	oArgs.AddWithHandler("HelperBone",siArgHandlerSingleObj);
	
	return true;
}

function vstGetHelperBoneObject_Execute(o)
{
	vstDebugTrace("vstGetHelperBoneObject_Execute");

	return GetHelperObject(o);
	
}

//////////////////////////////////////////////////////////////////////////////////////
// helper objects
//////////////////////////////////////////////////////////////////////////////////////
function GetHelperObject(o)
{
	if ( null == o ) return null;
	if ( typeof(o) != "object" ) return null;
	
	helper = new Object();
	helper.object = o;	
	
	helper.data = null;
	helper.primop = null;
	helper.model = null;
	helper.ikop = null;
	helper.child = null;
	helper.parent = null;
	helper.root = null;
	helper.id = -1;
	helper.currenttrigger = -1;

	helper.Init = function()
	{		
		//
		// Class initialization
		try {
			this.data = this.object.properties("HelperBoneData");	
		} catch (e) {;}
		
		if (this.data==null) 
		{
			vstDebugTrace("GetHelperObject: helperbone data not found");
			return false;
		}
		vstDebugTrace("GetHelperObject: found helperbone data",4);

		// locate trigger in list in combo box
		this.currenttrigger = this.GetTool().parameters("TriggerID").Value;
		
		return true;
	}
	
	// 
	// define methods
	helper.GetID = function()
	{
		if (this.id==-1)
		{
			// locate helper in list of helper in combo box
			try
			{
				vstDebugTrace("GetHelperObject: find id for " + this.object.name,4);
				var aUIItems = this.GetTool().ppglayout.Item("ID").UIItems.toArray();
				vstDebugTrace("GetHelperObject: num UIItems " + aUIItems.length,4);
				
				for ( var i=0; i < aUIItems.length/2; i++ )
				{
					var match = (aUIItems[ i*2 ]==this.object.name);
					
					vstDebugTrace("GetHelperObject: item " +  match + ":" + i,4);
					
					if (match)
					{
						this.id = i;
						vstDebugTrace("GetHelperObject: found object " + this.object.name + ":" + this.id,4);
						break;
					}
				}
				
				if (this.id==-1) 
					throw "helperbone id not found";
				else	
					vstDebugTrace("GetHelperObject: found helperbone id",4);
			}
			catch(e) 
			{ 
				vstDebugTrace("GetHelperObject: helperbone id not found");
			};
		}
		return this.id;
	}
	
	helper.GetRoot = function()
	{
		if (!this.root)
		{
			try {
				this.root = this.object.parent;
				if (this.root==null) 
					throw "helperbone root not found";
			}
			catch(e) 
			{ 
				vstDebugTrace("GetHelperObject: helperbone root not found");
				return null;
			};
			vstDebugTrace("GetHelperObject: found helperbone root",4);
		}
		return this.root;
	}

	helper.GetParent = function()
	{
		if (!this.parent)
		{
			try {
				var pport = this.GetIKOp().port("globalkineport","ParentBoneGroup",0);
				var pkinestate = pport.target2;
				this.parent = pkinestate.parent.parent;
				if (this.parent==null) 
					throw "helperbone parent not found";
			}
			catch(e) 
			{ 
				vstDebugTrace("GetHelperObject: helperbone parent not found");
				return null;
			};
			vstDebugTrace("GetHelperObject: found helperbone parent",4);
		}
		return this.parent;
	}
	
	helper.GetChild = function()
	{
		if (!this.child)
		{
			try {
				var cport = this.GetIKOp().port("globalkineport","ChildBoneGroup",0);
				var ckinestate = cport.target2;
				this.child = ckinestate.parent.parent;
				if (this.child==null) 
					throw "helperbone child not found";
			}
			catch(e) 
			{ 
				vstDebugTrace("GetHelperObject: helperbone child not found");
				return null;
			};
			vstDebugTrace("GetHelperObject: found helperbone child",4);

		}
		return this.child;
	}
	
	helper.GetIKOp = function()
	{
		if (!this.ikop)
		{
			try { this.ikop = Dictionary.GetObject( this.object.fullname + ".kine.global.HelperBoneOp" ) } 
			catch(e) 
			{ 
				vstDebugTrace("GetHelperObject: helperbone op not found");
				return null;
			};
			vstDebugTrace("GetHelperObject: found helperbone op",4);
		}
		
		return this.ikop;
	}
	
	helper.GetTool = function()
	{
		if (!this.tool)
		{
			try {
				this.tool = this.GetModel().properties("HelperBoneTool");
				if (this.tool==null) 
					throw "helperbone tool not found";
			}
			catch(e) 
			{ 
				vstDebugTrace("GetHelperObject: helperbone tool not found");
				return null;
			};
			vstDebugTrace("GetHelperObject: found helperbone tool",4);
		}
		return this.tool;
	}
	
	helper.GetModel = function()
	{
		if (!this.model) 
		{
			this.model = this.object.model;		
			if (!this.model) 
			{
				vstDebugTrace("GetHelperObject: helperbone model not found");
				return null;
			}
			vstDebugTrace("GetHelperObject: found helperbone model",4);
		}

		return this.model;
	}
	
	helper.GetPrimOp = function()
	{
		if (!this.primop)
		{
			try { helper.primop = Dictionary.GetObject( o.fullname + ".polymsh.HelperBonePrimOp" ); } 
			catch(e) 
			{ 
				vstDebugTrace("GetHelperObject: helperbone primop not found");
				return null;
			};
			vstDebugTrace("GetHelperObject: found helperbone primop",4);
		}
		
		return this.primop;
	}
	
	helper.SetSize = function(w,h,l)
	{
		var primop = GetPrimOp();
		
		if ( ! primop ) return;
		
		primop.Width.Value = 2;
		primop.Height.Value = 2;
		primop.Height.Value = 2;
	}
	
	helper.AddTrigger = function(aTriggerData)
	{
		if ( ! this.data ) return -1;
	
		try  {
			var size = aTriggerData.length;
			if ( size < 12 ) return -1;
		} catch (e) { return -1; }
		
		var griddata = this.data.Parameters("Triggers").value;
		
		if (griddata.columncount != 12 /*gNumGridDataColumns*/) 
		{
			griddata.columncount = 12 /*gNumGridDataColumns*/;
		}
		
		var idx = griddata.rowcount;
		griddata.rowcount = idx+1;
		griddata.setrowvalues( idx, aTriggerData );
		
		return idx;
	}
	
	helper.EnableTriggers = function(bEnabled)
	{
		if ( ! this.data ) return -1;
	
		// re-enable / disable IK on bones
		var child = this.GetChild();
		var firstbone = null;
			
		if ( classname(child) == "ChainBone" )
		{
			if ( child.index == 0 ) 
				firstbone = child;
			else	
				firstbone = child.root.bones(0);
				
			var ikchain = firstbone.properties("Kinematic Chain");
			ikchain.parameters("muteikop").value = (bEnabled==false);
		}
		
		// re-enable all triggers
		var griddata = this.data.Parameters("Triggers").value;
		
		var cRows = griddata.rowcount;
	
		for ( var iRow=0; iRow < griddata.rowcount; iRow++ )	
		{
			griddata.setcell( idTriggerEnabled, iRow, bEnabled );
		}

		// re-enable constraints
		var eCns = new Enumerator( this.GetChild().kinematics.constraints );
		for ( ; !eCns.atEnd(); eCns.moveNext() )
			 	eCns.item().Parameters("Active").Value = (bEnabled==true);
	}

	helper.SetTriggerRotation = function(iTrigger, x, y, z)
	{
		if ( ! this.data ) return -1;
	
		var griddata = this.data.Parameters("Triggers").value;
		
		var cRows = griddata.rowcount;
	
		if ( iTrigger < 0 || iTrigger > cRows ) return;
	
		griddata.setcell( idTriggerRotX, iTrigger, x );
		griddata.setcell( idTriggerRotY, iTrigger, y );
		griddata.setcell( idTriggerRotZ, iTrigger, z );
	}
	

	helper.GetTriggerRotation = function(iTrigger)
	{
		if ( ! this.data ) return -1;
	
		var griddata = this.data.Parameters("Triggers").value;
		
		var cRows = griddata.rowcount;
	
		if ( iTrigger < 0 || iTrigger > cRows ) return;
	
		var aRot = new Array(3);
	
		aRot[0] = griddata.getcell( idTriggerRotX, iTrigger);
		aRot[1] = griddata.getcell( idTriggerRotY, iTrigger );
		aRot[2] = griddata.getcell( idTriggerRotZ, iTrigger);
	
		return aRot;
	}

	helper.SetTargetRotation = function(iTrigger, x, y, z)
	{
		if ( ! this.data ) return -1;
	
		var griddata = this.data.Parameters("Triggers").value;
		
		var cRows = griddata.rowcount;
	
		if ( iTrigger < 0 || iTrigger > cRows ) return;
	
		griddata.setcell( idTargetRotX, iTrigger, x );
		griddata.setcell( idTargetRotY, iTrigger, y );
		griddata.setcell( idTargetRotZ, iTrigger, z );
	}
	
	helper.SetTargetPosition = function(iTrigger, x, y, z)
	{
		if ( ! this.data ) return -1;
	
		var griddata = this.data.Parameters("Triggers").value;
		
		var cRows = griddata.rowcount;
	
		if ( iTrigger < 0 || iTrigger > cRows ) return;
	
		griddata.setcell( idTargetPosX, iTrigger, x );
		griddata.setcell( idTargetPosY, iTrigger, y );
		griddata.setcell( idTargetPosZ, iTrigger, z );
	}
	
	helper.GetHelperBones = function()
	{
		return GetHelperBones(this.model);
	}

	helper.PoseChildToTrigger = function(idxTrigger)
	{
		if ( idxTrigger < 0 ) return;
		
		var rot = XSIMath.CreateRotation();
		var aRot = this.GetTriggerRotation(idxTrigger);
		
		rot.SetFromXYZAnglesValues( 
			XSIMath.DegreesToRadians(aRot[0]), 
			XSIMath.DegreesToRadians(aRot[1]), 
			XSIMath.DegreesToRadians(aRot[2]) ); 
		
		var ltransfo = XSIMath.CreateTransform();
		
		ltransfo.SetRotation( rot );
		
		// transform rotation into world space 
		var wtransfo = XSIMath.MapObjectPoseToWorldSpace( this.GetParent().kinematics.global.transform, ltransfo);
		var wtransfo = XSIMath.MapObjectPoseToWorldSpace( this.GetParent().kinematics.global.transform, ltransfo);

		wtransfo.GetRotation(rot);
		
		// set child pose
		var gtransfo = this.GetChild().kinematics.global.transform;
		
		gtransfo.setrotation( rot );
		 
		this.GetChild().kinematics.global.transform = gtransfo;
	}
	
	helper.SoloCurrentTrigger = function(bSolo)
	{
		if  ( this.currenttrigger==-1) return;
	 
		// disable/enable IK
		var child = this.GetChild();
		var firstbone = null;
		
		if ( classname(child) == "ChainBone" )
		{
			if ( child.index == 0 ) 
				firstbone = child;
			else	
				firstbone = child.root.bones(0);
				
			var ikchain = firstbone.properties("Kinematic Chain");
			ikchain.parameters("muteikop").value = bSolo;
		}
		
		var eCns = new Enumerator( this.GetChild().kinematics.constraints );
		for ( ; !eCns.atEnd(); eCns.moveNext() )
			 	eCns.item().Parameters("Active").Value = (bSolo==false);
		
		// disable all other triggers
		var griddata = this.data.Parameters("Triggers").Value;
		
		for ( var iRow=0; iRow < griddata.rowcount; iRow++ )
		{
			var enabled = bSolo ? (iRow == this.currenttrigger) : true;
			
			griddata.setcell( idTriggerEnabled, iRow, enabled );
		}

		// pose child bone to match trigger orientation
		if (bSolo)
			this.PoseChildToTrigger(this.currenttrigger);
		else
		{
			// make sure bone pops back
			if (firstbone)
			{
				var ikchain = firstbone.properties("Kinematic Chain");
				var val = ikchain.parameters("blendik").value; 
				ikchain.parameters("blendik").value = (val) ? 0 : 1;
				ikchain.parameters("blendik").value = val;
			}
		}
	}

	if (!helper.Init()) 
		return null;
		
	return helper;
}

function GetHelperBones(model)
{
	var col = XSIFactory.CreateObject("XSI.Collection");

	if (model.groups("HelperBones")==null)
		return col;
		
	col.additems(model.groups("HelperBones").Members);	
	
	return col;
}

function Trace( str )
{
	if ( gDebug )
		Application.Logmessage(str);
}
