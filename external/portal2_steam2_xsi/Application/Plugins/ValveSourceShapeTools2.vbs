Const gToolName = "ValveSource Shape Tools (VBScript)"
Const gDebug = false
dim gToolPath : gToolPath = ""

function XSILoadPlugin( in_reg )
	in_reg.Author = "ValveSource and Softimage"
	in_reg.Name = gToolName
	in_reg.Major = 1
	in_reg.Minor = 1


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




