'***************************************************************************************
' ValveSource BackComp Tool
' Version v0.1
' Author: Simon Inwood
' Description: commands to provide minimal backcomp with rigs configures with the
' HelperBone Tool v1.0 (axisinterpolator)
'***************************************************************************************

'***************************************************************************************
' CONSTANTS 
'***************************************************************************************

' constants for plugin installation 
Const gToolName =  "ValveSource BackComp Tool"
dim gToolPath : gToolPath = ""

'***************************************************************************************
' Plugin Entry Point
'***************************************************************************************
function XSILoadPlugin( in_reg )
	in_reg.Author = "Softimage"
	in_reg.Name = gToolName
	in_reg.Major = 1
	in_reg.Minor = 0

	call in_reg.RegisterCommand("vstSelectHelperBones10", "vstSelectHelperBones10")

	XSILoadPlugin = True
end function

'***************************************************************************************
' Command	:	vstSelectHelperBones10
' description	:	select helperbones from axisinterp tool v1.0
'***************************************************************************************
Function vstSelectHelperBones10_Init( in_ctxt )
	set oCmd = in_ctxt.Source
	oCmd.ReturnValue = True 
	dim oArgs : set oArgs = oCmd.arguments
	
	vstSelectHelperBones10_Init = True
end Function

Function vstSelectHelperBones10_Execute()
	vstSelectHelperBones10_Execute=False
	
	selectobj GetHelperBones(getvalue("ValveBiped"))
end Function

'***************************************************************************************
' Command	:	GetHelperBones
' description	:	return all objects with an axisinterpop connected to its global kinematicstate
'***************************************************************************************

function GetHelperBones(rig)
	dim col : set col = findobjects(,"{F44E51E9-FAB3-4AC6-BE2C-809CB6482128}")

	set GetHelperBones = createobject("xsi.collection")

	if typename(col) <> "Nothing" then
		dim op	
		for each op in col
			dim port : set port = op.outputports(0)
			if typename(port) <> "Nothing" then
				dim helper : set helper = port.target2.parent.parent
				if helper.model = rig then
					GetHelperBones.add helper
				end if
			end if
		next
	end if

end function