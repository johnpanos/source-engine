Const gToolName = "ValveSource Shape Tools (VBScript)"
Const gDebug = false
dim gToolPath : gToolPath = ""

function XSILoadPlugin( in_reg )
	in_reg.Author = "ValveSource and Softimage"
	in_reg.Name = gToolName
	in_reg.Major = 1
	in_reg.Minor = 1

	in_reg.RegisterCommand "vstMergeFreezeCharacter","vstMergeFreezeCharacter"
	in_reg.RegisterCommand "vstInitShapeKey","vstInitShapeKey"
	in_reg.RegisterCommand "vstCopyShapeKey","vstCopyShapeKey"
	in_reg.RegisterCommand "ValveFixBrokenFootRoll","ValveFixBrokenFootRoll"
		


	XSILoadPlugin = true
end function

function ValveFixBrokenFootRoll_Execute

deleteobj "ValveBiped.toe.kine.local.scriptedop"
applyop "BipedRigSteppingAngle", "ValveBiped.toe;ValveBiped.mid;ValveBiped.heel;ValveBiped.LeftRoll"
addexpr "ValveBiped.toe.kine.local.BipedRigSteppingAngle.SteppingAngle", "ValveBiped.CharacterControls.LeftSteppingAngle", False

deleteobj "ValveBiped.toe1.kine.local.scriptedop"
applyop "BipedRigSteppingAngle", "ValveBiped.toe1;ValveBiped.mid1;ValveBiped.heel1;ValveBiped.RightRoll"
addexpr "ValveBiped.toe1.kine.local.BipedRigSteppingAngle.SteppingAngle", "ValveBiped.CharacterControls.RightSteppingAngle", False

end function

'***************************************************************************************
' Command: vstCopyShapeKey
'***************************************************************************************
function vstCopyShapeKey_Init( ctxt )
	vstCopyShapeKey_Init=True
	
	dim oCmd : set oCmd = ctxt.Source
	oCmd.Description = "Copy polygon mesh shape key from specific frame"
end function

function vstCopyShapeKey_Execute
	on error resume next

	dim fullname
	fullname = selection(0).fullname

	if err.number <> 0 then
		Msgbox "Select a polymesh object first"
		exit function
	end if
	
	dim l_iFrameNum
	l_iFrameNum = InputBox ( "Copy this shape to which frame?", "vstCopyShapeKey", GetValue ("PlayControl.Current") + 1 )
	
	if l_iFrameNum = "" then
		logmessage "vstCopyShapeKey Command Cancelled", siWarning
		exit function
	end if
	
	SaveShapeKey fullname, , , Int(l_iFrameNum), , , , 1, siShapeLocalReferenceMode
	
end function

function GetClipAtFrame( oMesh, iFrame )

	set oModel = oMesh.Model
	set oMixer = Dictionary.GetObject(oModel.FullName& ".Mixer")
	
	'Get the tracks
	Dim oShape
	for each param in EnumElements(oMixer)
		if param.name = "Tracks" then
			for each trackParam in EnumElements(param)
				'Shape track container
				if trackParam.Name = "Shape" then	
					'Clip container
					for each shapeTrack in EnumElements(trackParam)
						for each shapeTrackParam in EnumElements(shapeTrack)							
							if shapeTrackParam.Name = "Clip list" then
								for each clip in EnumElements(shapeTrackParam)
									aFrame = int ( GetValue( clip.fullname & ".actionclip.timectrl.startoffset") )
									if aFrame = iFrame then
										logmessage "FOUND!"
										set GetClipAtFrame = clip
										exit function
									end if		
								next
							end if
						next
					next
				end if
			next
		end if
	next


end function

' allows plugin to unload from factory (debug only)
function can_unload()
	can_unload = true
end function

'#######################################################################################
'Additional code that was in the XSI_EXP-for-HL2 version but not the previous
'#######################################################################################

'***************************************************************************************
' command		:	vstInitShapeKey
' Shape Tools from Valve
' askowron - now a selfinstalled custom command
'***************************************************************************************
function vstInitShapeKey_Init( ctxt )
	vstInitShapeKey_Init=True
	
	dim oCmd : set oCmd = ctxt.Source
	oCmd.Description = "Create initial shape for the selected polygon mesh vertices"
end function

sub vstInitShapeKey_Execute
	PreviousAutoInspect = GetUserPref("AutoInspectEnabled")
	SetUserPref "AutoInspectEnabled", False
	on error resume next

	dim fullname
	fullname = selection(0).fullname

	if err.number <> 0 then
		Msgbox "Select a polymesh object first"
		exit sub
	end if
	
	dim This_FrameNum

	SetValue "PlayControl.Current", 0



	For i = 1 to 40

		SaveShapeKey fullname, , , Int(i), , , , 1, siShapeLocalReferenceMode
	Next 
	Msgbox "KeyFrame Initialization complete for frames 1-40"
	SetUserPref "AutoInspectEnabled", PreviousAutoInspect 			
	
end sub


'***************************************************************************************
'	MergeCharacter
'
'	Description:
'	Merges the two selected objects and recreates the shape
'	animation at every frame.
'	If only one object is selected it will create a clean
'	copy of the object with the shape animation.
'
'	Author: Ronald Beirouti (Softimage Co.)
'
'	USAGE: 
'		1- Select the two objects to merge
'		2- Run this script
'
' askowron - now a selfinstalled custom command
'***************************************************************************************

Function vstMergeFreezeCharacter_Init( in_ctxt )
	set oCmd = in_ctxt.Source
	oCmd.ReturnValue = false 
	oCmd.Description = "Merge 2 objects and recreate the shape animation at each frame."
	
	oCmd.arguments.add "ob1", siArgumentInput	
	oCmd.arguments.add "ob2", siArgumentInput
	oCmd.arguments.add "bCreateModel", siArgumentInput
	vstMergeFreezeCharacter_Init = true
end Function


Sub vstMergeFreezeCharacter_Execute( inObject1, inObject2 , in_bCreateModel )
	PreviousAutoInspect = GetUserPref("AutoInspectEnabled")
	SetUserPref "AutoInspectEnabled", False

	dim inHead
	
	set inHead = GetValue( inObject1 )
	
	' Get the scene root (always handy)
	set oRoot = ActiveProject.ActiveScene.Root

	' Get the original object
	set oOriginalModel = inHead.model

	if not inObject2 = "" then
		bMerge = True
		set inBody = GetValue( inObject2 )
	else
		bMerge= False	
		set inBody = oOriginalModel.AddPolygonMesh
	end if
	
	' Merge the two geometries
	set oMergeOp = ApplyOp( "MeshMerge", inHead.fullname & "," & inBody.fullname ,,, outputObjs )(0)

	' Keep a reference to the new merged object
	set oNewMeshObj = outputObjs(0)

	' rename the new object
	oNewMeshObj.name = "NewCharacter"

	' Put the new mesh in a new model and rename it
	if in_bCreateModel then
		set oNewModel = oRoot.AddModel( oNewMeshObj )
		oNewModel.name = "Model"
	else
		set oNewModel = oOriginalModel
	end if
			
	' Create a mapping rule for each cluster that has shape animation
	set ClsCollection = oNewMeshObj.ActivePrimitive.geometry.clusters.filter("pnt")

	Set oShapeClsColl = CreateObject("XSI.Collection")
	Set oNewShapeClsColl = CreateObject("XSI.Collection")
	
	bShapeClsFound = False
	for each cls in ClsCollection
		
		pos = InStrRev( cls.name, "_" & inHead.name , -1, 1 )
		
		if pos > 0 then
			cls.name = Left( cls.name , pos-1 )
		end if
		
		for each clsprop in cls.properties
			if clsprop.type = "clskey" then
				bShapeClsFound = True
				exit for
			end if
		next
		
		if bShapeClsFound = True then
			bShapeClsFound = False
			oShapeClsColl.Add cls 
		end if
	next

	Set oDeleteClsColl = CreateObject("XSI.Collection")

	for each cls in oShapeClsColl
	
		set dupClsColl = SIDuplicate( cls )
		clsname = cls.name
		cls.name = "internal_cls_to_delete"
		dupClsColl(0).name = clsname
		
		oNewShapeClsColl.Add dupClsColl(0)
		oDeleteClsColl.Add cls
	next


	'Cleanup the cluster names
	for each cls in oNewMeshObj.ActivePrimitive.geometry.clusters
		pos = InStrRev( cls.name, "_" & inHead.name , -1, 1 )
		
		if pos > 0 then
			cls.name = Left( cls.name , pos-1 )
		end if
	next
	
	dim frameStart, frameEnd
	frameStart = CInt( GetValue("Project.Data.PlayControl.In") )
	frameEnd = CInt( GetValue("Project.Data.PlayControl.Out"))

	refresh
	for each cls in oNewShapeClsColl
		
		PlotShape cls, cls.name & "_ClsClip" ,frameStart , frameEnd , 1, true, 1
	next
	
	' Freeze the merge operation

	SetValue "PlayControl.Current", 0

	FreezeObj oNewMeshObj.activeprimitive.constructionhistory.find( "mergemesh" )

	'Convert shapes to local relative type

	for each cls in oNewShapeClsColl
		ConvertShapeReferenceMode cls, siShapeLocalReferenceMode
	next
	
	'Remove clusters created by empty mesh
	
	if not bMerge then
		for each cls in oNewMeshObj.ActivePrimitive.geometry.clusters
			pos = InStrRev( cls.name, "_" & inBody.name , -1, 1 )
			
			if pos > 0 then
				'cluster generated by empty mesh found -> add it to list of clusters to delete
				oDeleteClsColl.Add cls
			end if

		next
	end if

	deleteobj oDeleteClsColl
		
	if not bMerge then
		deleteobj inBody
	end if
	SetUserPref "AutoInspectEnabled", PreviousAutoInspect 			
		
end sub



