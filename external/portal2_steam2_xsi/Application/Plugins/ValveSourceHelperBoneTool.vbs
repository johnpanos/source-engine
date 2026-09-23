'#######################################################################################
' Valve HelperBone Tool
' Version v1.2
' Author: Simon Inwood ( simonin@softimage.com )
'#######################################################################################

'#######################################################################################
' CONSTANTS 
'#######################################################################################

' constants for plugin installation 
Const gToolName =  "ValveSource HelperBone Tool (VBScript)"
dim gToolPath : gToolPath = ""
Const gDefaultRigName = "ValveBiped"
Const gDefaultTriggerName = "Trigger"
Const gVRDSuffix = ".vrd"

' constants for pickobject 
Const siRMB = 0

' constants for UI 
Const gToolPropName = "HelperBoneTool"
Const gTriggerPrefix = "Trigger"

'#######################################################################################
' HelperBoneData(grid) constants
'#######################################################################################
Const idTriggerEnabled	= 0
Const idTriggerTol		= 1
Const idTriggerRotX		= 2
Const idTriggerRotY		= 3
Const idTriggerRotZ		= 4
Const idTargetRotX		= 5
Const idTargetRotY		= 6
Const idTargetRotZ		= 7
Const idTargetPosX		= 8
Const idTargetPosY		= 9
Const idTargetPosZ		= 10
Const idTriggerName = 11
Const gNumGridDataColumns = 12

'#######################################################################################
' event constants
'#######################################################################################
Const gSelectionEventName   = "HelperBoneTool_OnSelectionChange"
Const gNewSceneEventName    = "HelperBoneTool_OnEndNewScene"
Const gOpenSceneEventName   = "HelperBoneTool_OnEndOpenScene"

dim gMuteEvents : gMuteEvents = False

'#######################################################################################
' helperboneop constants
'#######################################################################################
Const gHelperBoneDataName	= "HelperBoneData"
Const gHelperBoneOpName		= "HelperBoneOp"
Const gHelperBonePrimOpName = "HelperBonePrimOp"
Const gHelperBoneOpPath		= ".kine.global.HelperBoneOp"

Const gParentBoneGroupName = "ParentBoneGroup"
Const gParentBonePort1Name = "globalkineport"

Const gChildBoneGroupName  = "ChildBoneGroup"
Const gChildBonePort1Name  = "globalkineport"

'#######################################################################################
' VRD file parsing constants
'#######################################################################################
Const gHelperDataTag = "<helper>"
Const siTriggerTag = "<trigger>"
Const gBoneDataTag = "<bone>"
Const siFS = " "

'#######################################################################################
' Entry for registration of contents
'#######################################################################################

function XSILoadPlugin( in_reg )
	in_reg.Author = "ValveSource and Softimage"
	in_reg.Name = gToolName
	in_reg.Major = 1
	in_reg.Minor = 2

	call in_reg.RegisterCommand("vstHelperBoneTestScene", "vstHelperBoneTestScene")
	call in_reg.RegisterCommand("vstSelectAllHelperBones", "vstSelectAllHelperBones")
	call in_reg.RegisterCommand("vstSetHelperBoneTriggerRot", "vstSetHelperBoneTriggerRot")
	call in_reg.RegisterCommand("vstAddHelperBoneTrigger", "vstAddHelperBoneTrigger")
	call in_reg.RegisterCommand("vstRemoveHelperBoneTrigger", "vstRemoveHelperBoneTrigger")
	call in_reg.RegisterCommand("vstRemoveAllHelperBoneTriggers", "vstRemoveAllHelperBoneTriggers")
	call in_reg.RegisterCommand("vstCreateHelperBone", "vstCreateHelperBone")
	call in_reg.RegisterCommand("vstCreateHelperBoneTool", "vstCreateHelperBoneTool")
	call in_reg.RegisterCommand("vstDeleteHelperBone", "vstDeleteHelperBone")
	call in_reg.RegisterCommand("vstImportHelperBones", "vstImportHelperBones")
	call in_reg.RegisterCommand("vstShowImportHelperBonesDialog", "vstShowImportHelperBonesDialog")
	call in_reg.RegisterCommand("vstRegisterEvents", "vstRegisterEvents")
	call in_reg.RegisterCommand("vstDeleteHelperBones","vstDeleteHelperBones")
	call in_reg.RegisterCommand("vstEditHelperBones","vstEditHelperBones")
	call in_reg.RegisterCommand("vstInputBox","vstInputBox")
	call in_reg.RegisterCommand("vstExportHelperBones","vstExportHelperBones")

	XSILoadPlugin = True
end function

'#######################################################################################
' COMMAND CALLBACKS
'#######################################################################################

'***************************************************************************************
' command		:	vstInputBox
' description	:	give access to inputbox to jscript
'***************************************************************************************
Function vstInputBox_Init( in_ctxt )
	set oCmd = in_ctxt.Source
	oCmd.ReturnValue = True 
	call oCmd.setflag(siNoLogging,True)
	dim args : set args = oCmd.arguments
	args.add "prompt", siArgumentInput	
	args.add "title", siArgumentInput	
	args.add "default", siArgumentInput	
	vstInputBox_Init = True
end Function

Function vstInputBox_Execute(prompt,title,defaultval)
	vstInputBox_Execute=InputBox(prompt,title,defaultval)
	
	if (typename(vstInputBox_Execute)="Empty") then
		vstInputBox_Execute	= ""
	end if
end Function

'***************************************************************************************
' command		:	vstHelperBoneTestScene_Execute
' description	:	set up a simple test scene with one helper bone
'***************************************************************************************
Function vstHelperBoneTestScene_Init( in_ctxt )
	set oCmd = in_ctxt.Source
	oCmd.ReturnValue = false 
	vstHelperBoneTestScene_Init = True
end Function

sub vstHelperBoneTestScene_Execute()

	logmessage "Create a simple test rig with a single helperbone."

	' create test rig
	NewScene , false
	Create2DSkeleton -1, 5, 0, -5, -1, 0, 0, 0, 0, 4
	AppendBone "eff", 2, -4, 0

	dim col : set col = XSIFactory.CreateObject("XSI.Collection")
	col.setastext "root"
	dim model : set model = ActiveSceneRoot.AddModel(col)
	model.Name = gDefaultRigName
	
	dim rootObj : set rootObj = Dictionary.GetObject("ValveBiped.bone")
	dim parent : set parent = Dictionary.GetObject("ValveBiped.bone")
	dim child : set child  = Dictionary.GetObject("ValveBiped.bone1")

	' create helperbone setup
	call vstCreateHelperBones( model.fullname, False )

	dim helper : set helper = vstCreateHelperBone( model, rootObj, parent, child )
	call helper.SetSize( 2, 2, 2)
	
	' add helperbone data
	call helper.AddTrigger( Array(True, 90, 0, 0, 60, 0, 0, 0, 0, 0, 0, "TriggerA") )
	call helper.AddTrigger( Array(True, 90, 0, 0, 86.6565, 0, 0, 0, 0, 2, 0, "TriggerB")  )
	call helper.AddTrigger( Array(True, 82.606, 0, 0, 130.0553, 45, 0, 0, 2, 1, 0, "TriggerC") )
	
	call vstEditHelperBones( rig )
	
end sub

'***************************************************************************************
' function		:	vstSelectAllHelperBones_Execute
' description	:	command callback for selecting all helper bones in rig.
'***************************************************************************************
Function vstSelectAllHelperBones_Init( in_ctxt )
	set oCmd = in_ctxt.Source
	oCmd.ReturnValue = false 
	oCmd.Description = "Select All HelperBones"
	oCmd.arguments.add "rig", siArgumentInput	
	vstSelectAllHelperBones_Init = True
end Function


sub vstSelectAllHelperBones_Execute( rig )

	set rig = GetRigArg(rig)
	if typename(rig) = "Nothing" then
		exit sub
	end if

	selectobj GetHelperBones(rig)

end sub

'***************************************************************************************
' function		:	vstSetHelperBoneTriggerRot_Execute
' description	:	set the helper trigger using the current orientation of parentObj to childObj
'***************************************************************************************
Function vstSetHelperBoneTriggerRot_Init( in_ctxt )
	set oCmd = in_ctxt.Source
	oCmd.ReturnValue = True 
	oCmd.arguments.addwithhandler "helper", siArgHandlerSingleObj
	oCmd.arguments.add "index", siArgumentInput, -1, siInt4
	vstSetHelperBoneTriggerRot_Init = True
end Function

function vstSetHelperBoneTriggerRot_Execute( helper, idx )
	vstDebugBegin "vstSetHelperBoneTriggerRot_Execute"

	vstSetHelperBoneTriggerRot_Execute = False
	
	dim bonedata : set bonedata = Helper_GetBoneData(helper)

	if ( typename(bonedata) = "Nothing" ) then
		call logmessage( "vstSetHelperBoneTriggerRot:  invalid helper argument")
		vstDebugEnd
		exit function
	end if

	dim griddata : set griddata = bonedata.parameters("Triggers").value
	
	if ( idx < 0 Or idx >= griddata.rowcount ) then
		call logmessage( "vstSetHelperBoneTriggerRot:  invalid trigger index argument")
		vstDebugEnd
		exit function
	end if

	dim op : set op = Helper_GetHelperBoneOp(helper)
	if typename(op) = "Nothing" then
		call logmessage( "vstSetHelperBoneTriggerRot:  damaged helper: " + helper.fullname)
		vstDebugEnd
		exit function
	end if

	dim parentObj : set parentObj = Helper_GetParent(op)
	dim childObj : set childObj = Helper_GetChild(op)

	' compute trigger orientation
	dim rchildObj : set rchildObj = ComputeLocalOrientation( parentObj, childObj )
	dim a : a =  rchildObj.GetXYZAnglesValues2()

	' update trigger
	dim aTriggerData : aTriggerData = griddata.getrowvalues(idx)
	
	aTriggerData(idTriggerRotX) = round(r2d(a(0)),4)
	aTriggerData(idTriggerRotY) = round(r2d(a(1)),4)
	aTriggerData(idTriggerRotZ) = round(r2d(a(2)),4)
	
	call griddata.setrowvalues( idx, aTriggerData )

	vstSetHelperBoneTriggerRot_Execute = True

	vstDebugEnd
end function

'***************************************************************************************
' function		:	vstAddHelperBoneTrigger_Execute
' description	:	add a new helper trigger entry. Base the trigger orientation on
' the pose bone and the target orientation/position on the current pose of the 
' helper bone
'***************************************************************************************
Function vstAddHelperBoneTrigger_Init( in_ctxt )
	dim oCmd : set oCmd = in_ctxt.Source
	oCmd.ReturnValue = True 
	dim oArgs : set oArgs = oCmd.arguments

	call oArgs.AddWithHandler("HelperBone",siArgHandlerSingleObj)

	vstAddHelperBoneTrigger_Init = True
end Function

function vstAddHelperBoneTrigger_Execute( helper )
	vstDebugBegin "vstAddHelperBoneTrigger_Execute"

	vstAddHelperBoneTrigger_Execute = -1
	
	dim op : set op = Helper_GetHelperBoneOp(helper)
	dim parentObj : set parentObj = Helper_GetParent(op)	
	dim childObj : set childObj = Helper_GetChild(op)	
	
	vstAddHelperBoneTrigger_Execute = AddTrigger(helper, parentObj, childObj)

	vstDebugEnd
end function

'***************************************************************************************
' function		:	vstRemoveAllHelperBoneTriggers_Execute
' description	:	remove all helper triggers
'***************************************************************************************
Function vstRemoveAllHelperBoneTriggers_Init( in_ctxt )
	dim oCmd : set oCmd = in_ctxt.Source
	oCmd.ReturnValue = True 
	
	dim oArgs : set oArgs = oCmd.arguments
	call oArgs.AddWithHandler("HelperBone",siArgHandlerSingleObj)
	oArgs.add "confirm", siArgumentInput, False, siBool

	vstRemoveAllHelperBoneTriggers_Init = True
end Function

function vstRemoveAllHelperBoneTriggers_Execute( helper, confirm )
	vstDebugBegin "vstRemoveAllHelperBoneTriggers_Execute"

	vstRemoveAllHelperBoneTriggers_Execute = False

	dim bonedata : set bonedata = Helper_GetBoneData(helper)
	dim griddata : set griddata = bonedata.parameters("Triggers").Value
	
	if ( 0=griddata.rowcount ) then
		vstRemoveAllHelperBoneTriggers_Execute = False
		exit function
	end if
		
	if cbool(confirm) then
		dim rtn : rtn = msgbox("Are you sure you want to remove all the helperbone triggers?",vbOKCancel,"Confirm Remove All")
		if rtn <> vbOk then
			vstDebugEnd
			exit function
		end if
	end if

	' update bonedata
	griddata.rowcount = 0

	vstRemoveAllHelperBoneTriggers_Execute = True
	
	vstDebugEnd
end function

'***************************************************************************************
' function		:	vstRemoveHelperBoneTrigger_Execute	
' description	:	delete helper trigger and remove its entry from config data
' file.
'***************************************************************************************

Function vstRemoveHelperBoneTrigger_Init( in_ctxt )
	set oCmd = in_ctxt.Source

	oCmd.returnvalue = True

	oCmd.arguments.addwithhandler "helper", siArgHandlerSingleObj
	oCmd.arguments.add "index", siArgumentInput,-1,siInt4
	oCmd.arguments.add "confirm", siArgumentInput,False,siBool

	vstRemoveHelperBoneTrigger_Init = True
end Function


function vstRemoveHelperBoneTrigger_Execute( helper, idx, confirm )
	vstDebugBegin "vstRemoveHelperBoneTrigger_Execute"
	vstRemoveHelperBoneTrigger_Execute=False

	if ( typename(helper) = "Nothing" ) then
		vstDebugEnd
		exit function
	end if

	dim bonedata : set bonedata = Helper_GetBoneData(helper)

	if ( typename(bonedata) = "Nothing" ) then
		call logmessage( "vstRemoveHelperBoneTrigger:  invalid helper argument")
		vstDebugEnd
		exit function
	end if

	dim griddata : set griddata = bonedata.parameters("Triggers").value
	
	if ( idx < 0 Or idx >= griddata.rowcount ) then
		call logmessage( "vstRemoveHelperBoneTrigger:  invalid trigger index argument")
		vstDebugEnd
		exit function
	end if
	
	if cbool(confirm) then
		dim triggername : triggername = CStr(griddata.getcell(idTriggerName,idx))
		dim msg : msg = "Are you sure you want to remove helper bone " & triggername & "?"
		dim rtn : rtn = msgbox(msg,vbOKCancel,"Confirm Remove Trigger")
		if rtn <> vbOk then
			vstDebugEnd
			exit function
		end if
	end if

	for iRow = idx To griddata.rowcount-1
		if ( iRow + 1 < griddata.rowcount-1 ) then
			call griddata.setrowvalues( iRow, griddata.getrowvalues(iRow+1))
		end if	
	next
	griddata.rowcount = griddata.rowcount-1
	
	vstRemoveHelperBoneTrigger_Execute=True

	vstDebugEnd
end function

'***************************************************************************************
' function	:	vstCreateHelperBone
' description:	
'***************************************************************************************
Function vstCreateHelperBone_Init( in_ctxt )
	set oCmd = in_ctxt.Source
	oCmd.ReturnValue = True 
	oCmd.arguments.addwithhandler "rig", siArgHandlerSingleObj
	oCmd.arguments.addwithhandler "root", siArgHandlerSingleObj
	oCmd.arguments.addwithhandler "parent", siArgHandlerSingleObj
	oCmd.arguments.addwithhandler "child", siArgHandlerSingleObj
	vstCreateHelperBone_Init = True
end Function

function vstCreateHelperBone_Execute(rig, rootObj, parentObj, childObj )
	vstDebugBegin "vstCreateHelperBone_Execute"

	set vstCreateHelperBone_Execute = Nothing
	
	set rig = getrigarg( rig )
	if typename(rig) = "Nothing" then
		call Application.LogMessage("vstCreateHelperBone: rig argument missing",siError)	
		vstDebugEnd
		exit function
	end if

	if typename(rootObj) = "Nothing" then
		call Application.LogMessage("vstCreateHelperBone: root argument missing",siError)
		vstDebugEnd
		exit function
	end if
	
	if typename(parentObj) = "Nothing" then 
		call Application.LogMessage("vstCreateHelperBone: parent argument missing",siError)
		vstDebugEnd
		exit function
	end if
	
	if typename(childObj) = "Nothing" then 
		call Application.LogMessage("vstCreateHelperBone: child argument missing",siError)
		vstDebugEnd
		exit function
	end if

	dim helperObj : set helperObj = CreateHelperBone( rig, prefix&"helper", rootObj, parentObj, childObj )

	set vstCreateHelperBone_Execute=vstGetHelperBoneObject(helperObj)

	vstDebugEnd
end function


'***************************************************************************************
' function	:	vstCreateHelperBoneTool
' description:	UI callback command. Start a pick session to pick the parent and child
' bones requiring a helper bone ( the child doesn't have to be a child of the parent ).
'***************************************************************************************
Function vstCreateHelperBoneTool_Init( in_ctxt )
	set oCmd = in_ctxt.Source
	oCmd.ReturnValue = True 
	oCmd.arguments.addwithhandler "rig", siArgHandlerSingleObj
	vstCreateHelperBoneTool_Init = True
end Function

function vstCreateHelperBoneTool_Execute(rig)
	dim button, modifier
	
	set vstCreateHelperBoneTool_Execute=Nothing
	
	set rig = getrigarg( rig )
	if typename(rig) = "Nothing" then
		vstDebugEnd
		exit function
	end if

	' Pick Root
	dim rootObj : set rootObj = Nothing
	do
		Application.StatusBar ="Select helperbone's root"
		PickObject "Select parent bone", "Select helperbone's root", rootObj, button, modifier
	loop until ( button = siRMB ) Or typename(rootObj) <> "Nothing"

	if button = siRMB then
		vstDebugEnd
		exit function
	end if 

	' Pick Parent
	dim parent : set parent = Nothing
	do
		Application.StatusBar ="Select parent bone"
		PickObject "Select parent bone", "Select parent bone", parent, button, modifier

		dim col : set col = Nothing
		if ( typename(parent) <> "Nothing" ) then
				set col = sifilter( parent, "bone")
		end if		
		if typename(col) <> "Nothing" then
			set parent = col(0)
		else
			set parent = Nothing
		end if
	loop until ( button = siRMB ) Or typename(parent) <> "Nothing"

	if button = siRMB then
		vstDebugEnd
		exit function
	end if 

	' Pick Child
	dim child : set child = Nothing
	do
		Application.StatusBar ="Select child bone"
		PickObject "Select pose bone", "Select child bone", child, button, modifier
		if button = siRMB then
			vstDebugEnd
			exit function
		end if 
	loop until ( button = siRMB ) Or typename(child) <> "Nothing"
			
	' get prefix for helper
	dim pos : pos = instrrev(parent.name,"_")
	dim prefix
	if pos <> 0 then
		prefix = left(parent.name,pos)
	end if
		
	' Create helper			
	dim helper : set helper = CreateHelperBone( rig, prefix&"helper", rootObj, parent, child )

	' Add trigger using orientation of pose bone ( childObj)
	AddTrigger helper, parent, child
	
	vstSelectHelperBone helper 

	set vstCreateHelperBoneTool_Execute = vstGetHelperBoneObject(helper)
	
	vstDebugEnd
	
end function

'***************************************************************************************
' function	:	vstDeleteHelperBone_Execute
' description:	Delete helper bone and remove from the rig
'***************************************************************************************
Function vstDeleteHelperBone_Init( in_ctxt )
	set oCmd = in_ctxt.Source
	oCmd.ReturnValue = True
	oCmd.arguments.addwithhandler "helper", siArgHandlerSingleObj
	oCmd.arguments.add "confirm", siArgumentInput,False,siBool
	vstDeleteHelperBone_Init = True
end Function

function vstDeleteHelperBone_Execute(helper,confirm)
	vstDebugBegin "vstDeleteHelperBone_Execute:"
	 
	vstDeleteHelperBone_Execute=False

	vstDebugTrace "helper->" & typename(helper),4
	 
	if cbool(confirm) then
		dim rtn : rtn = msgbox("Are you sure you want to delete helper bone " & helper & "?",vbOKCancel,"Confirm Delete")
		if rtn <> vbOk then
			exit function
		end if
	end if

	DeleteObj helper
	
	vstDeleteHelperBone_Execute=True

	vstDebugEnd
end function

'***************************************************************************************
' command	:	vstShowImportHelperBonesDialog
' description	:	show filebrowser dialog for selecting vrd file and import helper bones
'***************************************************************************************
Function vstShowImportHelperBonesDialog_Init( in_ctxt )
	set oCmd = in_ctxt.Source
	oCmd.Description = "Import HelperBones from .vrd file"
	oCmd.ReturnValue = False
	call oCmd.setflag( siNoLogging, True )
	ImportHelperBones_Init = True
end Function

function vstShowImportHelperBonesDialog_Execute()
	vstDebugBegin "vstShowImportHelperBonesDialog_Execute" 

	set rig  = GetRigArg(rig)
	if typename(rig) = "Nothing" then
		if prompt then
			msgbox "Please select a Valve rig",,gToolName
		end if
		vstDebugEnd
		exit function
	end if

	dim InitialDirectory : InitialDirectory = GetConfigFilePath() 
	dim FileFilter : FileFilter = "Valve Rig Data (*.vrd)|*.vrd|All Files (*.*)|*.*||"
	
	dim oFileBrowser : set oFileBrowser = XSIUIToolkit.FileBrowser

	oFileBrowser.DialogTitle = "Import HelperBones"			' set the title of the file browser
	oFileBrowser.InitialDirectory = InitialDirectory		' set the initial directory
	oFileBrowser.FileBaseName = InitialDirectory & "\" & gDefaultRigName & gVRDSuffix		' set the default file name
	oFileBrowser.Filter = FileFilter		' set the filters
	oFileBrowser.ShowOpen					' show an open file dialog

	filename = oFileBrowser.FilePathName

	' if the user selected a file
	if ( filename <> "" ) then
		if ( vstImportHelperBones(rig, filename) ) then
			call InspectObj( Rig_GetTool(rig),"", gToolName, siLock )
		end if
	end if
	
	vstDebugEnd
end function

'***************************************************************************************
' command		:	vstImportHelperBones
' description	:	import helper bones
'***************************************************************************************
Function vstImportHelperBones_Init( in_ctxt )
	set oCmd = in_ctxt.Source
	oCmd.ReturnValue = True

	oCmd.arguments.addwithhandler "rig", siArgHandlerSingleObj
	oCmd.arguments.add "filename", siArgumentInput,"",siString

	vstImportHelperBones_Init = True
end Function

function vstImportHelperBones_Execute(rig,fname)
	vstDebugBegin "vstImportHelperBones"

	vstImportHelperBones_Execute = False
	
	if StrComp( fname, "", 1 ) = 0 then
		msgbox "Please verify the correct file name was given.",, gToolName
		vstDebugEnd
		exit function
	end if
	
	dim prop : set prop = Rig_GetTool(rig)	
	if typename(prop) = "Nothing" then
		set prop = vstCreateHelperBones(rig,false)
	end if

	dim fso : set fso = CreateObject("Scripting.FileSystemObject")

	if not fso.FileExists(fname) then
		msgbox fname & vbNewLine & "File not found."&vbNewLine&"Please verify the correct file name was given.",, gToolName
	else

		dim f : set f = fso.OpenTextFile( fname )
			
		if f.AtEndOfStream  then
			msgbox "Corrupt configuration file."&vbNewLine&fname&vbNewLine&"Please verify the contents of the file.",, gToolName
			f.Close
		else
			dim oProgressBar : set oProgressBar = XSIUIToolkit.ProgressBar
			oProgressBar.CancelEnabled = False
			oProgressBar.step = 5
			oProgressBar.caption = "Reading VRD File"
			oProgressBar.Visible = True

			' read data
			dim txt : txt = f.ReadAll
			f.Close

			oProgressBar.Increment

			' loop thru config file
			' ignore helpers that have already been created
			dim helper : set helper = Nothing
			dim startpos : startpos = 1
			do 
				' get a helper entry
				dim entry : entry = GetHelperBoneRecord( startpos, txt, "" )

				if entry <> "" then
					dim dict : set dict = GetHelperDictFromString( entry )
					vstDebugPrint dict

					if ( typename(dict) <> "Nothing" ) then
						' resolve model
						dict("helper") = rig &"."&	dict("helper")
						dict("rootObj") = rig &"."&	dict("rootObj")
						dict("parentObj") = rig &"."&	dict("parentObj")
						dict("childObj") = rig &"."&	dict("childObj")

						set helper = Nothing
						on error resume next
						set helper = Dictionary.GetObject( dict("helper") )
						on error goto 0

						if typename(helper) = "Nothing" then
							oProgressBar.caption = "Creating helperbone " & dict("helper")
							oProgressBar.Increment
							set helper = CreateHelperBoneFromDict( rig, dict )
						else
							oProgressBar.caption = "Updating helperbone " & dict("helper")
							oProgressBar.Increment
							call SetHelperBoneFromDict( helper, dict )
						end if
					else
						oProgressBar.Increment
						logmessage "Error invalid VRD record " & entry, siError
					end if

					startpos = startpos + len(entry)
				end if

			loop while entry <> ""

			oProgressBar.Visible = False

		end if

	end if

	vstImportHelperBones_Execute = True
	vstDebugEnd

end function


'***************************************************************************************
' function		:	vstDeleteHelperBones_Execute
' description	:	This will delete
' all the helper bones and the helperbonetool property stored under the model.
'
' return value	:	NA 
'
'***************************************************************************************
Function vstDeleteHelperBones_Init( in_ctxt )
	set oCmd = in_ctxt.Source
	oCmd.ReturnValue = false
	oCmd.Description = "Delete all helperbones from Valve rig"

	oCmd.arguments.add "rig", siArgumentInput
	oCmd.arguments.add "prompt", siArgumentInput,True,siBool

	vstDeleteHelperBones_Init = True
end Function

sub vstDeleteHelperBones_Execute(rig, prompt)

	set rig = GetRigArg(rig)
	if typename(rig) = "Nothing" then
		if prompt then
			msgbox "Please select a Valve rig",,gToolName
		end if
		vstDebugEnd
		exit sub
	end if

	if prompt then

		dim rtn : rtn = msgbox("Are you sure you want to delete the helper bone setup?",vbOKCancel,"Confirm Rig Reset")
		if rtn <> vbOk then
			vstDebugEnd
			exit sub
		end if
	end if

	dim prop : set prop = Rig_GetTool(rig)
	if typename(prop) <> "Nothing" then
	
		call vstRegisterEvents(false)

		set tool = Rig_GetTool(rig)
		if (typename(tool) <> "Nothing") then 	
			deleteobj tool
		end if	
		set col = GetHelperBones(rig)
		if (typename(col) <> "Nothing") then
			deleteobj col
		end if	
		set group1 = rig.Groups("HelperBones")
		if (typename(group1) <> "Nothing") then
			deleteobj group1
		end if
		set group2 = rig.Groups("HelperBoneRoot")
		if (typename(group2) <> "Nothing") then
			deleteobj group2
		end if
		set group3 = rig.Groups("HelperBoneParent")
		if (typename(group3) <> "Nothing") then
			deleteobj group3
		end if
		set group4 = rig.Groups("HelperBoneChild")
		if (typename(group4) <> "Nothing") then
			deleteobj group4
		end if
	end if

end sub

'***************************************************************************************
' function		:	vstEditHelperBones_Execute
' description	:	toolbar button callback for editing helper bones. Pops up the.
'					ppg for the axis interpolator property.
' return value	:	NA 
'***************************************************************************************
Function vstEditHelperBones_Init( in_ctxt )
	set oCmd = in_ctxt.Source
	oCmd.ReturnValue = false
	oCmd.Description = "Show HelperBones Editing Tool"

	oCmd.arguments.add "rig", siArgumentInput
	vstEditHelperBones_Init = True
end Function

sub vstEditHelperBones_Execute(rig)
	set rig = GetRigArg(rig)
	if typename(rig) = "Nothing" then
		msgbox "Please select a Valve rig",,gToolName
		vstDebugEnd
		exit sub
	end if

	dim prop : set prop = Rig_GetTool(rig)
	if typename(prop) <> "Nothing" then
		inspectobj Rig_GetTool(rig),,gToolName,siLock
	end if

end sub

'***************************************************************************************
' command		:	vstExportHelperBones
' description	:	export helper bone data to file
'***************************************************************************************
Function vstExportHelperBones_Init( in_ctxt )
	set oCmd = in_ctxt.Source
	oCmd.ReturnValue = false

	oCmd.arguments.add "rig", siArgumentInput
	oCmd.arguments.add "filename", siArgumentInput,"",siString
	oCmd.arguments.add "prompt", siArgumentInput,False,siBool

	vstExportHelperBones_Init = True
end Function


sub vstExportHelperBones_Execute(rig,fname,prompt)
	vstDebugBegin "vstExportHelperBones_Execute"
	 
	set rig = GetRigArg(rig)
	if typename(rig) = "Nothing" then
		if prompt then
			msgbox "Please select a Valve rig",,gToolName
		end if
		vstDebugEnd
		exit sub
	end if

	dim prop : set prop = Rig_GetTool(rig)	
	if typename(prop) = "Nothing" then
		call logmessage("vstExportHelperBones: no helperbones found")
		vstDebugEnd
		exit sub
	end if

	if StrComp( fname, "", 1 ) = 0 then
		if prompt then
			msgbox "Please verify the correct file name was given.",, gToolName
		end if
		vstDebugEnd
		exit sub
	end if

	dim fso : set fso = CreateObject("Scripting.FileSystemObject")
	
	dim dir : dir =  fso.GetParentFolderName(fname)

	if fso.FolderExists(dir) then
		dim f : set f = fso.CreateTextFile(fname, True)

		' export the  data
		dim helpers : set helpers = GetHelperBones(rig)
 
		' loop thru helpers
		dim helper : set helper = Nothing
		for each helper in helpers

			dim primop : set primop = Helper_GetHelperBonePrimOp(helper)
			dim bonedata : set bonedata = Helper_GetBoneData(helper)
			dim ikop : set ikop = Helper_GetHelperBoneOp(helper)
			
			if typename(ikop) <> "Nothing" And _
				typename(bonedata) <> "Nothing" And _
				typename(primop) <> "Nothing" then

				dim rootObj : set rootObj = helper.parent
				dim parentObj : set parentObj = Helper_GetParent(ikop)
				dim childObj : set childObj = Helper_GetChild(ikop)

				if	typename(rootObj) <> "Nothing" And _
					typename(parentObj) <> "Nothing" And _
					typename(childObj) <> "Nothing" then
				
					f.WriteLine "<helper> " & helper.name & siFS & rootObj.name & siFS & parentObj.name & siFS & childObj.name
					f.WriteLine "<display> "& primop.Width.value & siFS & primop.Height.value & siFS& primop.Length.value & siFS & bonedata.BoneDistance.Value
					f.WriteLine "<basepos> "& bonedata.BoneOffsetX.value & siFS & bonedata.BoneOffsetY.value & siFS & bonedata.BoneOffsetZ.value
					
					dim griddata : set griddata = bonedata.parameters("Triggers").Value
					dim aTriggerData
					for iRow = 0 to griddata.rowcount - 1
						aTriggerData = griddata.getrowvalues(iRow)
						dim strTrigger : strTrigger = join(aTriggerData, siFS)
						
						strTrigger=Mid(strTrigger, InStr(1, strTrigger, siFS)+1)
						f.WriteLine siTriggerTag & siFS & strTrigger
					next	
				end if
			end if
		next
		f.close
	else
		if prompt then
			msgbox dir & vbNewLine & "Directory not found."&vbNewLine&"Please verify the correct file name was given.",, gToolName
		end if
	end if


	vstDebugEnd
end sub

'#######################################################################################
' EVENT CALLBACKS
'#######################################################################################

sub vstRegisterEvents_Init(ctx)
	dim oCmd : set oCmd = ctx.source
	call oCmd.setflag(siNoLogging,True)
	dim args : set args = oCmd.arguments
	args.add "register", siArgumentInput, True, siBool	
	args.add "force", siArgumentInput, False, siBool	

end sub

sub vstRegisterEvents_Execute(register,force)

	' install event
	dim eventInfo : set eventInfo = Nothing
	
	if (register) then
		dim filename : filename = _
			Application.Plugins(gToolName).filename

		set eventInfo = Application.EventInfos(gSelectionEventName)
		if typename(eventInfo) = "Nothing" then
			Application.Advise "OnSelectionChange",filename,,"VBScript",gSelectionEventName,,,False
		end if

		set eventInfo = Application.EventInfos(gNewSceneEventName)
		if typename(eventInfo) = "Nothing" then
			Application.Advise "OnEndNewScene",filename,,"VBScript",gNewSceneEventName,,,False
		end if
		
		set eventInfo = Application.EventInfos(gOpenSceneEventName)
		if typename(eventInfo) = "Nothing" then
			Application.Advise "OnEndSceneOpen",filename,,"VBScript",gOpenSceneEventName,,,False
		end if

		gMuteEvents=False
	else
		if (force) then
		
			set eventInfo = Application.EventInfos(gSelectionEventName)
			if typename(eventInfo) <> "Nothing" then
				Application.UnAdvise eventInfo.token
			end if

			set eventInfo = Application.EventInfos(gNewSceneEventName)
			if typename(eventInfo) <> "Nothing" then
				Application.UnAdvise eventInfo.token
			end if
			
			set eventInfo = Application.EventInfos(gOpenSceneEventName)
			if typename(eventInfo) <> "Nothing" then
				Application.UnAdvise eventInfo.token
			end if
					
		end if

		gMuteEvents=True
	end if
	
end sub

'***************************************************************************************
' function		:	XSIApplication_OnEndNewScene 
' description	:	end new scene event callback. 
' Remove helperbone tool events
'***************************************************************************************
function XSIApplication_OnEndNewScene ( data )
	gMuteEvents=True
end function

'***************************************************************************************
' function		:	XSIApplication_OnEndOpenScene
' description	:	end open scene event callback. 
' Remove helperbone tool events
'***************************************************************************************
function XSIApplication_OnEndSceneOpen( data, filename )
	gMuteEvents=True
end function

'***************************************************************************************
' function		:	XSIApplication_OnSelectionChange
' description	:	selection event callback. 
' Update the helperbone tool
'***************************************************************************************

sub XSIApplication_OnSelectionChange( data, changeType )

	vstDebugTrace "XSIApplication_OnSelectionChange: " & changeType

	if (gMuteEvents) then
		exit sub
	end if

	dim helper : set helper = Nothing

	on error resume next
	if (selection.count <> 0) then
		set helper=vstGetHelperBoneObject(selection(0))
	end if
	on error goto 0

	if (changeType = 0 ) then
	
		if (typename(helper) <> "Nothing" And typename(helper) <> "Empty") then
			helper.GetTool().ID.Value = helper.GetID()
		end if
	else
		if (typename(helper) = "Nothing" Or typename(helper) = "Empty") then
			on error resume next
			vstDeselectHelperBone
			on error goto 0
		end if
	end if


end sub

'#######################################################################################
' HELPER FUNCTIONS		
'#######################################################################################

'***************************************************************************************
' function	:	GetLocalTransformMatrix3
' description:	return local transform of childObj relative to parentObj
'***************************************************************************************
function GetLocalTransformMatrix3( byref tGparentObj, byref tGchildObj )
	dim mparentObj : set mparentObj = xsimath.creatematrix3
	dim mchildObj : set mchildObj = xsimath.creatematrix3
	
	tGparentObj.GetRotationMatrix3 mparentObj
	tGchildObj.GetRotationMatrix3 mchildObj

	mparentObj.TransposeInPlace
	mchildObj.MulInPlace mparentObj

	set GetLocalTransformMatrix3 = mchildObj
end function

'***************************************************************************************
' function	:	CreateHelperBoneFromDict
' description:	create a new helper bone and configure it using the data from the 
' dictionary
'***************************************************************************************
function CreateHelperBoneFromDict( rig, dict )
	vstDebugBegin "CreateHelperBoneFromDict"

	set CreateHelperBoneFromDict = Nothing

	' helper contains model name - remove
	dim startpos : startpos = instrrev( dict("helper"), "." ) + 1
	dim helpername : helpername = mid( dict("helper"), startpos )

	dim rootObj  : set rootObj = Nothing
	dim parentObj  : set parentObj = Nothing
	dim childObj  : set childObj = Nothing

	on error resume next
	set rootObj = Dictionary.GetObject(dict("rootObj"))
	set parentObj = Dictionary.GetObject(dict("parentObj"))
	set childObj = Dictionary.GetObject(dict("childObj"))
	on error goto 0

	if typename(rootObj) = "Nothing" then 
		logmessage gToolName &": skipping helperbone("+helpername+"), root not found = " & dict("rootObj")
		vstDebugEnd
		exit function
	end if 
	if typename(parentObj) = "Nothing" then
		logmessage gToolName &": skipping helperbone("+helpername+"), parent not found = " & dict("parentObj")
		vstDebugEnd
		exit function
	end if 
	if typename(childObj) = "Nothing" then
		logmessage gToolName &": skipping helperbone("+helpername+"), child not found = " & dict("childObj")
		vstDebugEnd
		exit function
	end if 

	dim helperObj : set helperObj = CreateHelperBone( rig, helpername, rootObj, parentObj, childObj )
		
	' set up helper bone data
	call SetHelperBoneFromDict( helperObj, dict )
	
	set CreateHelperBoneFromDict  = helperObj

	vstDebugEnd
end function

'***************************************************************************************
' function	:	SetHelperBoneFromDict
' description:	reset a helper bone's configuration from data in dictionary
'***************************************************************************************
sub SetHelperBoneFromDict( helper, dict )
	vstDebugBegin "SetHelperBoneFromDict"

	dim bonedata : set bonedata = Helper_GetBoneData(helper)
	dim primop : set primop = Helper_GetHelperBonePrimOp(helper)

	' update helper bone geometry
	if ( typename(primop) <> "Nothing" ) then
		primop.parameters("Width").value = dict("w")
		primop.parameters("Height").value = dict("h")
		primop.parameters("Length").value = dict("l")
	end if
	
	' update bone data operator
	if ( typename(bonedata) <> "Nothing" ) then
		bonedata.Parameters("BoneDistance").value = dict("perc")
		bonedata.Parameters("BoneOffsetX").value = dict("baseposx")
		bonedata.Parameters("BoneOffsetY").value = dict("baseposy")
		bonedata.Parameters("BoneOffsetZ").value = dict("baseposz")
	
		' replace triggers
		set griddata = bonedata.Parameters("Triggers").value

		griddata.rowcount = ubound(dict("triggertol"))
		vstDebugTrace "SetHelperBoneFromDict: " & helper.name & " triggers " & griddata.rowcount, 4

		if (griddata.rowcount) then			
			griddata.columncount = gNumGridDataColumns
			dim aTriggerTol : aTriggerTol = dict("triggertol")
			dim aTriggerOri : aTriggerOri = dict("triggerori")
			dim aTargetOri : aTargetOri = dict("targetori")
			dim aTargetPos : aTargetPos = dict("targetpos")
			dim aTriggerNames : aTriggerNames = dict("triggernames")
			
			for iRow = 0 to griddata.rowcount - 1
			
				griddata.setcell idTriggerEnabled, iRow, True
				griddata.setcell idTriggerTol, iRow, aTriggerTol(iRow)

				griddata.setcell idTriggerRotX, iRow, aTriggerOri((iRow*3)+0)
				griddata.setcell idTriggerRotY, iRow, aTriggerOri((iRow*3)+1)
				griddata.setcell idTriggerRotZ, iRow, aTriggerOri((iRow*3)+2)

				griddata.setcell idTargetRotX, iRow, aTargetOri((iRow*3)+0)
				griddata.setcell idTargetRotY, iRow, aTargetOri((iRow*3)+1)
				griddata.setcell idTargetRotZ, iRow, aTargetOri((iRow*3)+2)
				
				griddata.setcell idTargetPosX, iRow, aTargetPos((iRow*3)+0)
				griddata.setcell idTargetPosY, iRow, aTargetPos((iRow*3)+1)
				griddata.setcell idTargetPosZ, iRow, aTargetPos((iRow*3)+2)
				griddata.setcell idTriggerName, iRow, aTriggerNames(iRow)
			next
		end if	
	end if
	
	vstDebugEnd
end sub

'#######################################################################################
'  AxisInterpolatorTool PPG functions 
'#######################################################################################

'***************************************************************************************
' function		:	AxisInterpToolPPG_GetCurrentTriggerIndex
'***************************************************************************************
function AxisInterpToolPPG_GetCurrentTriggerIndex()
	AxisInterpToolPPG_GetCurrentTriggerIndex=-1

	if TriggerName.Value = "" then
		vstDebugEnd
		exit function
	end if
	dim pos : pos = len(gTriggerPrefix)+1
	dim idx : idx = cint(mid(TriggerName.Value,pos))
	AxisInterpToolPPG_GetCurrentTriggerIndex = idx

end function

'***************************************************************************************
' function		:	AxisInterpToolPPG_GetThis
'***************************************************************************************
function AxisInterpToolPPG_GetThis()
	set AxisInterpToolPPG_GetThis = getvalue(pset.inspectedobjects)
end function

'***************************************************************************************
' function		:	AxisInterpToolPPG_GetModel
'***************************************************************************************
function AxisInterpToolPPG_GetModel()
	set AxisInterpToolPPG_GetModel = AxisInterpToolPPG_GetThis().parent
end function

'***************************************************************************************
' function		:	AxisInterpToolPPG_GetHelper
'***************************************************************************************
function AxisInterpToolPPG_GetHelper()
	vstDebugBegin "AxisInterpToolPPG_GetHelper"
	dim rig : set rig = AxisInterpToolPPG_GetModel()
	set AxisInterpToolPPG_GetHelper = Nothing

	on error resume next
	vstDebugTrace HelperID.Value,4
	set AxisInterpToolPPG_GetHelper = dictionary.getobject( HelperID.Value )
	'vstDebugTrace typename(AxisInterpToolPPG_GetHelper)

	vstDebugEnd
end function

'#######################################################################################
'  Helper bone functions  (first arg is helper bone )
'#######################################################################################

'***************************************************************************************
' function		:	Helper_GetHelperBonePrimOp
' description	:	return the primitive operator from helper bone
'***************************************************************************************
function Helper_GetHelperBonePrimOp( helper )
	set Helper_GetHelperBonePrimOp = Nothing

	on error resume next
	set Helper_GetHelperBonePrimOp = Dictionary.GetObject( helper + ".polymsh.HelperBonePrimOp" )

end function

'***************************************************************************************
' function		:	Helper_GetBoneData
' description	:	return the custom property containing helper bone data
'***************************************************************************************
function Helper_GetBoneData( helper )
	set Helper_GetBoneData = Nothing

	on error resume next
	set Helper_GetBoneData = Dictionary.GetObject( helper + ".HelperBoneData" )

end function

'***************************************************************************************
' function		:	Helper_GetHelperBoneOp
' description	:	return the axis interp operator from helper bone
'
' return value	:	Operator - axis interp op
'
'***************************************************************************************
function Helper_GetHelperBoneOp( helper )
	vstDebugBegin "Helper_GetHelperBoneOp"
	
	set Helper_GetHelperBoneOp = Nothing

	if typename(helper) = "Nothing" then
		vstDebugEnd
		exit function
	end if

	dim path : path = helper & gHelperBoneOpPath
	'vstDebugTrace path
	set Helper_GetHelperBoneOp = Nothing
	on error resume next 
	set Helper_GetHelperBoneOp = dictionary.getobject( path )

	vstDebugEnd
end function

'#######################################################################################
'  AxisInterpOp functions  (first arg is axis interp operator )
'#######################################################################################

'***************************************************************************************
' function		:	Helper_GetParent
' description	:	return the top bone from an axis interp op
'
' return value	:	X3DObject - top bone
'
'***************************************************************************************

function Helper_GetParent( op )
	
	if typename(op) = "String" then
		set op = getvalue(op)
	end if

	dim port : 	set port = op.port(gParentBonePort1Name,gParentBoneGroupName,0)
	
	dim kinestate : set kinestate = port.target2

	set Helper_GetParent = kinestate.parent.parent

end function

'***************************************************************************************
' function		:	Helper_GetChild
' description	:	return the bottom bone from an axis interp op
'
' return value	:	X3DObject - top bone
'
'***************************************************************************************

function Helper_GetChild( op )
	
	if typename(op) = "String" then
		set op = getvalue(op)
	end if

	dim port
	set port = op.port(gChildBonePort1Name,gChildBoneGroupName,0)
	
	dim kinestate
	set kinestate = port.target2

	set Helper_GetChild = kinestate.parent.parent

end function

'***************************************************************************************
' function		:	Helper_GetTool
' description	:	return the tool from an helper bone
'
' return value	:	CustomProperty
'
'***************************************************************************************

function Helper_GetTool( helper )
	vstDebugBegin "Helper_GetTool"
	vstDebugTrace "helper->" & helper,4

	set Helper_GetTool = Nothing

	if typename(helper) = "Nothing" then
		vstDebugEnd
		exit function
	end if

	set model = helper.model

	do while typename( model ) <> "Nothing" and not model.isequalto( ActiveSceneRoot )
		vstDebugTrace model,4

		set Helper_GetTool = model.properties(gToolPropName)
		vstDebugTrace Helper_GetTool,4

		if typename(Helper_GetTool) <> "Nothing" then
			exit do
		end if	
	
		set model = model.model
	loop 

	vstDebugEnd
end function

'#######################################################################################
' HELPER FUNCTIONS		
'#######################################################################################

'***************************************************************************************
' function	:	CreateHelperBone
' description:	Add a helper bone between parentObj and childObj.
'***************************************************************************************
function CreateHelperBone( rig, name, rootObj, parentObj, childObj )
	
	dim prop : set prop = Rig_GetTool(rig)

	' set up helper geometry
	dim helperObj : set helperObj = rootObj.AddPolygonMesh( ,, name )
	dim primop : set primop = vstApplyHelperBonePrimOp( helperObj )

	dim helperbonedata : set helperbonedata = helperObj.AddProperty( gHelperBoneDataName, false )

	if ( typename(rootObj) = "ChainBone" ) then
		AddExpr helperbonedata.RootBoneLength,  rootObj.length.fullname, True
	else 
		' maybe a shadow rig
		for each cns in rootObj.Kinematics.Constraints
			' assume there can only be 1 pose constraint
			if (cns.type = "posecns") then
				AddExpr helperbonedata.RootBoneLength, cns.Constraining(0).length.fullname, True
				exit for
			end if
		next
	end if

	' add new helper to helpers group
	dim group : set group = rig.Groups("HelperBones")

	if (typename(group) <> "Nothing") then
		call SIAddToGroup( group, helperObj )
	else
		call vstDebugTrace("CreateHelperBone: helperbones group missing")
	end if
	
	' connection operator
	call vstApplyHelperBoneOp(helperObj,rootObj,parentObj,childObj)

	set CreateHelperBone  = helperObj

end function

'***************************************************************************************
' function		:	GetHelperBones
'***************************************************************************************
function GetHelperBones(rig)

	set GetHelperBones = createobject("xsi.collection")
	dim group : set group = rig.groups("HelperBones")
	
	if (typename(group) = "Nothing") then
		vstDebugEnd		
		exit function
	end if
	
	GetHelperBones.additems group.members

	vstDebugEnd		
end function

'***************************************************************************************
' function		:	ReplaceIDWithName
' description	:	replace unique ids with bone names
'***************************************************************************************
function ReplaceIDWithName(txt)
	vstDebugBegin "ReplaceIDWithName "
	dim tmp : tmp = txt
	dim replaced : replaced = false
	do 
		dim startpos : startpos = instr( tmp, "_3dobject<" )
		if startpos = 0 then
			exit do
		end if

		dim endpos  : endpos = instr( startpos+1, txt, ">" )
		if endpos = 0 then
			exit do
		end if

		dim id : id = mid( txt, startpos, endpos-startpos+1 )
		
		dim oname : oname = "<UNKNOWN>"
		on error resume next
		set o = getvalue(id)
		oname = o.name
		on error goto 0

		tmp = replace( tmp, id, oname )
	loop 

	ReplaceIDWithName = tmp

end function

'***************************************************************************************
' function		:	ComputeLocalOrientation
' description	:	compute local orientation between parentObj & childObj
' return value  :   rotation
'***************************************************************************************
function ComputeLocalOrientation(parentObj, childObj)

	dim tGparentObj : set tGparentObj = parentObj.kinematics.global.transform
	dim tGchildObj : set tGchildObj = childObj.kinematics.global.transform
	dim m3LchildObj : set m3LchildObj = GetLocalTransformMatrix3( tGparentObj, tGchildObj )

	dim qchildObj : set qchildObj = XSIMath.CreateQuaternion
	m3LchildObj.GetQuaternion qchildObj

	dim rchildObj : set rchildObj = XSIMath.CreateRotation
	rchildObj.SetFromQuaternion qchildObj

	set ComputeLocalOrientation = rchildObj
end function

'***************************************************************************************
' function		:	AddTrigger
' description	:	add new trigger. the trigger orientation is the orientation
' of childObj relative to parentObj and the default target is the orientation/position of
' the helper bone relative to parentObj
'***************************************************************************************
function AddTrigger(helper, parentObj, childObj)
	vstDebugBegin "AddTrigger"

	AddTrigger = -1
	
	' compute trigger
	dim rchildObj : set rchildObj = ComputeLocalOrientation( parentObj, childObj )
	dim a : a =  rchildObj.GetXYZAnglesValues2()

	dim bonedata : set bonedata = Helper_GetBoneData(helper)
	dim griddata : set griddata = bonedata.Parameters("Triggers").value
	if (griddata.columncount <> gNumGridDataColumns) then
		griddata.columncount = gNumGridDataColumns
	end if
	
	idx = griddata.rowcount
	
	dim aTriggerData : aTriggerData = _
		Array( True, 90, _
			round(r2d(a(0)),4), round(r2d(a(1)),4), round(r2d(a(2)),4), _
			0,0,0, _
			0,0,0, _
			(gDefaultTriggerName&idx) )
	
	griddata.rowcount = idx+1
	call griddata.setrowvalues( idx, aTriggerData )
	
	AddTrigger = idx
	
	vstDebugEnd
end function

'***************************************************************************************
' function		:	CreateHelperDict
' description	:	create the helper data dictionary
'
' return value	:	Dictionary
'
'***************************************************************************************
function CreateHelperDict( _
	helper, rootObj, parentObj, childObj, _
	w, h, l, perc, _
	offsetx, offsety, offsetz, _
	atrigori, atgtori, atgtpos, atrigtol, atrignames _
)
	vstDebugBegin "CreateHelperDict"

	set dict = createobject("Scripting.Dictionary")

	if typename(helper)="String" then
		dict("helper") = helper
		dict("rootObj") = rootObj
		dict("parentObj") = parentObj
		dict("childObj") = childObj
	else
		set dict("helper") = helper
		set dict("rootObj") = rootObj
		set dict("parentObj") = parentObj
		set dict("childObj") = childObj
	end if

	dict("w") = w
	dict("h") = h
	dict("l") = l
	dict("perc") = perc

	dict("baseposx") = offsetx
	dict("baseposy") = offsety
	dict("baseposz") = offsetz

	dict("triggertol") = atrigtol
	dict("triggerori") = atrigori
	dict("targetori") = atgtori
	dict("targetpos") = atgtpos
	dict("triggernames") = atrignames

	set CreateHelperDict = dict

	vstDebugEnd
end function

'***************************************************************************************
' function		:	GetHelperDictAsString
' description	:	create the helper's entry in the data text field
'
' return value	:	string
'
'***************************************************************************************
function GetHelperDictAsString( dict )
	vstDebugBegin "GetHelperDictAsString"

	dim w : w = dict("w") 
	dim h : h = dict("h")
	dim l : l = dict("l")
	dim perc : perc = dict("perc")
	dim offsetx : offsetx = dict("baseposx")
	dim offsety : offsety = dict("baseposy")
	dim offsetz : offsetz = dict("baseposz")

	dim triggers : triggers = GetHelperTriggersAsString(dict)

	dim helperid : helperid = dict("helper")
	dim rootObjid : rootObjid = dict("rootObj")
	dim parentObjid : parentObjid = dict("parentObj")
	dim childObjid : childObjid = dict("childObj")

	dim newentry : newentry = _
		"<helper> " & helperid & siFS & rootObjid & siFS & parentObjid & siFS & childObjid & vbCrLf & _
		"<display> "&w&siFS & h & siFS&l& siFS&perc & vbCrLf & _
		"<basepos> "&offsetx&siFS&offsety&siFS&offsetz&"" & vbCrLf & _
		triggers & vbCrLf

	GetHelperDictAsString = newentry
		
	vstDebugEnd
end function

'***************************************************************************************
' function		:	SetHelperBoneTrigger
' description	:	set the current helper trigger in axis interpolator tool property
'
' return value	:	na
'
'***************************************************************************************
sub SetHelperBoneTrigger( prop, idx, atriggerori, atargetori, atargetpos, atriggertol, atrignames )
	vstDebugBegin "SetHelperBoneTrigger " & idx

	if ( idx < 0 or idx > ubound(atriggerori) ) then
		err.raise vbObjectError
		vstDebugEnd
		exit sub
	end if

	prop.Parameters("TriggerName").value = gTriggerPrefix & idx
	prop.Parameters("TriggerTolerance").value = atriggertol(idx+0)

	idx = idx * 3

	prop.Parameters("TriggerRotX").value = atriggerori(idx+0)
	prop.Parameters("TriggerRotY").value = atriggerori(idx+1)
	prop.Parameters("TriggerRotZ").value = atriggerori(idx+2)

	prop.Parameters("TargetRotX").value = atargetori(idx+0)
	prop.Parameters("TargetRotY").value = atargetori(idx+1)
	prop.Parameters("TargetRotZ").value = atargetori(idx+2)

	prop.Parameters("TargetPosX").value = atargetpos(idx+0)
	prop.Parameters("TargetPosY").value = atargetpos(idx+1)
	prop.Parameters("TargetPosZ").value = atargetpos(idx+2)

	vstDebugEnd
end sub

'****************************************************************************************
' function		:	GetUniqueID
' description	:	return unique id for helper object
'
' return value	:	string	
'
'****************************************************************************************
function GetUniqueID( o )
	GetUniqueID = "_3dobject<"&xsiutils.DataRepository.GetIdentifier(o)&">"
end function

'****************************************************************************************
' function		:	GetHelperTriggersAsString
' description	:	return a string containing helper trigger data (from dialog values)
'
' return value	:	string
'
'****************************************************************************************
function GetHelperTriggersAsString( dict )
	vstDebugBegin "GetHelperTriggersAsString"

	GetHelperTriggersAsString = ""

	if typename(dict) <> "Dictionary" then
		vstDebugTrace "Error: invalid dictionary"
		exit function
	end if

	dim aTrigTol : aTrigTol = dict("triggertol")
	dim aTrigOri : aTrigOri = dict("triggerori")
	dim aTgtOri : aTgtOri = dict("targetori")
	dim aTgtPos : aTgtPos = dict("targetpos")

	dim j : j = 0
	for i=lbound(aTrigOri) to ubound(aTrigOri) step 3
		if i > 0 then
			GetHelperTriggersAsString =  GetHelperTriggersAsString & vbCrLf 
		end if

		GetHelperTriggersAsString = GetHelperTriggersAsString & _
			siTriggerTag&siFS & _
			aTrigTol(j)&siFS &_
			aTrigOri(i+0) &siFS&aTrigOri(i+1) &siFS&aTrigOri(i+2) & siFS&_
			aTgtOri(i+0) &siFS&aTgtOri(i+1) &siFS&aTgtOri(i+2) & siFS&_
			aTgtPos(i+0) &siFS&aTgtPos(i+1) &siFS&aTgtPos(i+2) 
		j=j+1
	next

	vstDebugEnd
end function

'****************************************************************************************
' function		:	GetHelperBoneRecord
' description	:	return string of helper data from text
'
' return value	:	string
'
'****************************************************************************************
function GetHelperBoneRecord( startpos, txt, id )
	vstDebugBegin "GetHelperBoneRecord"

	GetHelperBoneRecord = ""

	startpos = instr( startpos, txt, gHelperDataTag&siFS&id )

	if startpos = 0 then
		vstDebugEnd
		exit function
	end if

	dim endpos : endpos = instr( startpos+1, txt, gHelperDataTag&siFS )
	
	' last entry
	if endpos = 0 then
		GetHelperBoneRecord = mid( txt, startpos )
	else
		GetHelperBoneRecord = mid( txt, startpos, endpos-startpos )
	end if 

	vstDebugEnd
end function

'****************************************************************************************
' function		:	GetHelperTriggersFromStringArray
' description	:	parse array of string triggers and return values in arrays
'
' output byref  :	atrigtol, atrigori, atgtori, atgtpos
'
'****************************************************************************************
function GetHelperTriggersFromStringArray( lines, row, byref aTrigTol, byref aTrigOri, byref aTgtOri, byref aTgtPos, byref aTrigNames )
	vstDebugBegin "GetHelperTriggersFromStringArray"

	' triggers
	dim ctriggers : ctriggers = 0
	if ( ubound(lines) >= row ) then
		dim ctrigs : ctrigs = ubound(lines)-row+1

		'vstDebugTrace dict("helper") &" trigs:" & ctrigs,4
		dim size : size = ctrigs * 3
		redim aTrigTol(ctrigs-1), aTrigOri(size-1), aTgtOri(size-1), aTgtPos(size-1), aTrigNames(ctrigs-1)

		dim j, idx
		for j=row to ubound(lines)
			line = trim(lines(j))
			if len(line) <> 0 then
				i=1 : s = Split(line, siFS) : idx = (j-row)*3
				call vstDebugTrace("Processing " & line,4)

				if s(0) = siTriggerTag then
					aTrigTol((j-row) + 0) = cdbl(trim(s(i))) : i=i+1 ' trigger tolerance

					aTrigOri(idx + 0) = cdbl(trim(s(i))) : i=i+1 ' trigger ori x
					aTrigOri(idx + 1)  = cdbl(trim(s(i))) : i=i+1 ' trigger ori y
					aTrigOri(idx + 2) = cdbl(trim(s(i))) : i=i+1 ' trigger ori z

					aTgtOri(idx + 0) = cdbl(trim(s(i))) : i=i+1 ' target ori x
					aTgtOri(idx + 1) = cdbl(trim(s(i))) : i=i+1 ' target ori y
					aTgtOri(idx + 2) = cdbl(trim(s(i))) : i=i+1 ' target ori z

					aTgtPos(idx + 0) = cdbl(trim(s(i))) : i=i+1 ' target pos x
					aTgtPos(idx + 1) = cdbl(trim(s(i))) : i=i+1 ' target pos y
					aTgtPos(idx + 2) = cdbl(trim(s(i))) : i=i+1 ' target pos z

					if ( ubound(s) >= i ) then
						aTrigNames(j-row) = trim(s(i))
					else
						aTrigNames(j-row) = gDefaultTriggerName & ctriggers
					end if
					ctriggers = ctriggers +1
				end if
			end if
		next
		if size <> ctriggers then
			size = ctriggers*3
			redim preserve aTrigOri(size-1), aTgtOri(size-1), aTgtPos(size-1)
		end if
	else
		aTrigTol = Array()
		aTrigOri = Array()
		aTgtOri = Array()
		aTgtPos = Array()
		aTrigNames = Array()
	end if

	vstDebugEnd
end function

'****************************************************************************************
' function		:	GetHelperDictFromString
' description	:	return dictionary of helper data string
'
' return value	:	String
'
'****************************************************************************************
function GetHelperDictFromString( entry )
	vstDebugBegin "GetHelperDictFromString"

	set GetHelperDictFromString = Nothing

	dim lines : lines = split(entry, vbCrLf)
	'vstDebugTrace "lines : " & ubound(lines),4

	if ubound(lines) = -1 then
		vstDebugEnd
		exit function
	end if

	'  bone info 
	dim i : i = 1
	dim row : row = 0
	dim line : line = lines(row)
	
	vstDebugTrace "Processing: " & line,4
	dim s : s = Split(line, siFS)
	dim helper		: helper	= trim(s(i)) : i=i+1	'helper
	dim rootObj		: rootObj	= trim(s(i)) : i=i+1	'rootObj bone
	dim parentObj	: parentObj	= trim(s(i)) : i=i+1	'parent bone
	dim childObj	: childObj  = trim(s(i)) : i=i+1	'child bone
	row = row + 1

	'  display info 
	line = lines(row)
	vstDebugTrace "Processing: " & line,4
	i=1 : s = Split(line, siFS)
	dim w : w = cdbl(trim(s(i))) : i=i+1 'w
	dim h : h = cdbl(trim(s(i))) : i=i+1 'h
	dim l : l = cdbl(trim(s(i))) : i=i+1 'l
	dim perc : perc = cdbl(trim(s(i))) : i=i+1 'perc along done
	row = row + 1

	'  base pos
	line = lines(row)
	vstDebugTrace "Processing: " & line,4
	i=1 : s = Split(line, siFS)
	dim baseposx : baseposx = cdbl(trim(s(i))) : i=i+1 'x
	dim baseposy : baseposy = cdbl(trim(s(i))) : i=i+1 'y
	dim baseposz : baseposz = cdbl(trim(s(i))) : i=i+1 'z
	row = row + 1
	
	dim aTrigTol, aTrigOri, aTgtOri, aTgtPos, aTrigNames

	GetHelperTriggersFromStringArray lines, row, aTrigTol, aTrigOri, aTgtOri, aTgtPos, aTrigNames
	vstDebugTrace "triggernames: " & join(aTrigNames),4

	set GetHelperDictFromString = CreateHelperDict( _
		helper, rootObj, parentObj, childObj, _
		w, h, l, perc, _
		baseposx, baseposy, baseposz, _
		aTrigOri, aTgtOri, aTgtPos, aTrigTol, aTrigNames )

	vstDebugTrace "CreateHelperDict: " & helper & GetHelperDictAsString(GetHelperDictFromString),4


	vstDebugEnd
end function

'***************************************************************************************
' function		:	GetRigArg
' description	:	rig argument handler. if the model is not defined then return the
'					model called gDefaultRigName. If the model is an string then look up the 
'					object. If the object is a model return it otherwise return the
'					model that the object belongs too.
'
' return value	:	NA 
'
'***************************************************************************************
function GetRigArg( o )
	vstDebugBegin "GetRigArg" 
	set GetRigArg = Nothing

	if typename(o) = "String" or typename(o)="Empty" or typename(o) = "Nothing" then
		if o="" then
			if selection.count = 0 then
				set o = ActiveSceneRoot.FindChild( gDefaultRigName )
			else
				set o = selection(0)
			end if
		else
			set o = getvalue( o )
		end if
	elseif typename(o) <> "Nothing" then
		set GetRigArg = o
	end if

	if typename(o) <> "Nothing" And typename(o) <> "Model" then
		set GetRigArg = o.model
	else 
		set GetRigArg = o
	end if

	vstDebugTrace GetRigArg,4
	vstDebugEnd
end function

'****************************************************************************************
' function		:	Rig_GetTool
' description	:	Return axis interpolator tool property from rig model
'
' return value	:	property 
'
'****************************************************************************************
function Rig_GetTool(rig)
	set Rig_GetTool = rig.Properties(gToolPropName)
end function

'***************************************************************************************
' function		:	GetToolPath()
' description	:	return installation path for plugin.
'
' return value	:	string
'
'***************************************************************************************
function GetToolPlugin()
	set GetToolPlugin = Application.Plugins(gToolName)
end function

Function GetToolPath()

	if ( gToolPath = "" ) then
		set fso = XSIFactory.CreateActiveXObject("Scripting.FileSystemObject")
		
		strPluginsPath = GetToolPlugin().OriginPath
		
		gToolPath = fso.GetParentFolderName(fso.GetParentFolderName(strPluginsPath)) & "\"
	end if
	GetToolPath = gToolPath

End Function

function GetConfigFilePath()
	GetConfigFilePath = GetToolPath() & "Data\ConfigFiles"
end function 

'#######################################################################################
' MATH FUNCTIONS		:	D2R, R2D
'#######################################################################################
dim pi : pi = 4 * Atn(1) ' Calculate the value of pi.

function d2r( d )
	d2r =  d  * (pi/180)
end function

function r2d( r )
	r2d = r * (180/pi)
end function

function q2rot( q )
	set q2rot = xsimath.createrotation  
	q2rot.SetFromQuaternion (q)
end function

' arccosine (inverse cosine) is the inverse of te cosine function

function acos(X)
	if X = 1 then
		acos = 0
	elseif  X = -1 then
		acos = pi		
	else
		acos = Atn(-X / Sqr(-X * X + 1)) + (pi/2)
	end if
end function

' allows plugin to unload from factory (debug only)
function can_unload()
	can_unload = True
end function


