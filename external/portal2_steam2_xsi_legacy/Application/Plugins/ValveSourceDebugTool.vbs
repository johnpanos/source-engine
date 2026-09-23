'***************************************************************************************
' ValveSource Debug Tool
' Version v0.1
' Author: Simon Inwood
'***************************************************************************************

'***************************************************************************************
' CONSTANTS 
'***************************************************************************************

' debug constants, turns on stack tracing
' replace comment (') before fnend, fnbegin & fntrace to enable tracing
dim gDebugLevel : gDebugLevel = 0

' constants for plugin installation 
Const gToolName =  "ValveSource Debug Tool"
dim gToolPath : gToolPath = ""

'***************************************************************************************
' Plugin Entry Point
'***************************************************************************************
function XSILoadPlugin( in_reg )
	in_reg.Author = "Softimage"
	in_reg.Name = gToolName
	in_reg.Major = 0
	in_reg.Minor = 1

	call in_reg.RegisterCommand("vstDebugSetLevel", "vstDebugSetLevel")
	call in_reg.RegisterCommand("vstDebugPrint", "vstDebugPrint")
	call in_reg.RegisterCommand("vstDebugTrace", "vstDebugTrace")
	call in_reg.RegisterCommand("vstDebugBegin", "vstDebugBegin")
	call in_reg.RegisterCommand("vstDebugEnd", "vstDebugEnd")

	XSILoadPlugin = True
end function

'***************************************************************************************
' Command	:	vstDebugSetLevel
'***************************************************************************************
Function vstDebugSetLevel_Init( in_ctxt )
	set oCmd = in_ctxt.Source
	oCmd.ReturnValue = True 
	oCmd.Description = "DebugOff:0 DebugCalls:1 DebugTrace:2"
	dim oArgs : set oArgs = oCmd.arguments
	oArgs.Add "level"
	
	vstDebugSetLevel_Init = True
end Function

Function vstDebugSetLevel_Execute(level)
	vstDebugSetLevel_Execute=True
	
	gDebugLevel = CInt(level)
end Function


'***************************************************************************************
' Command	:	vstDebugPrint
'***************************************************************************************
Function vstDebugPrint_Init( in_ctxt )
	set oCmd = in_ctxt.Source
	oCmd.ReturnValue = True 
	call oCmd.SetFlag(siNoLogging,True)
	dim oArgs : set oArgs = oCmd.arguments
	oArgs.Add "object"
	oArgs.Add "str"
	
	vstDebugPrint_Init = True
end Function

Function vstDebugPrint_Execute(o,str)
	vstDebugPrint_Execute=True
	
	DebugTrace str & DebugPrint(o), 2 
end Function

'***************************************************************************************
' Command	:	vstDebugBegin
'***************************************************************************************
Function vstDebugBegin_Init( in_ctxt )
	set oCmd = in_ctxt.Source
	oCmd.ReturnValue = True 
	call oCmd.SetFlag(siNoLogging,True)
	dim oArgs : set oArgs = oCmd.arguments
	oArgs.Add "str"
	
	vstDebugBegin_Init = True
end Function

Function vstDebugBegin_Execute(str)
	vstDebugBegin_Execute=True
	
	DebugBegin str
end Function

'***************************************************************************************
' Command	:	vstDebugEnd
'***************************************************************************************
Function vstDebugEnd_Init( in_ctxt )
	set oCmd = in_ctxt.Source
	oCmd.ReturnValue = True 
	call oCmd.SetFlag(siNoLogging,True)
	dim oArgs : set oArgs = oCmd.arguments
	
	vstDebugEnd_Init = True
end Function

Function vstDebugEnd_Execute()
	vstDebugEnd_Execute=True
	
	DebugEnd 
end Function

'***************************************************************************************
' Command	:	vstDebugTrace
'***************************************************************************************
Function vstDebugTrace_Init( in_ctxt )
	set oCmd = in_ctxt.Source
	oCmd.ReturnValue = True 
	call oCmd.SetFlag(siNoLogging,True)
	dim oArgs : set oArgs = oCmd.arguments
	oArgs.Add "str", siArgumentInput, ""
	oArgs.Add "level", siArgumentInput, 2
	
	vstDebugTrace_Init = True
end Function

Function vstDebugTrace_Execute( o, level)
	vstDebugTrace_Execute=True
	
	DebugTrace o,level 
end Function

'#######################################################################################
' DEBUG FUNCTIONS		:	vstTrace, vstDebugPrint, DebugBegin, DebugEnd
'#######################################################################################

dim fnproc, fnprevproc, fnindent

function DebugPrint( o )
	
	if typename(o) = "ISIVector3" then
		DebugPrint = "("&o.x & ","&o.y & ","&o.z&")"
	elseif typename(o) = "ISIMatrix3" then
		DebugPrint  =  _
		"|" & FormatNumber(o(0,0),2) &", " &  FormatNumber(o(1,0),2) & ", " &  FormatNumber(o(2,0),2) & "|" & vbNewLine & _
		"|" & FormatNumber(o(1,0),2) &", " &  FormatNumber(o(1,1),2) & ", " &  FormatNumber(o(2,1),2) & "|"& vbNewLine & _
		"|" & FormatNumber(o(2,0),2) &", " &  FormatNumber(o(1,2),2) & ", " &  FormatNumber(o(2,2),2) & "|"
	elseif typename(o) = "ISIMatrix4" then
		DebugPrint  =  _
		"|" & FormatNumber(o(0,0),2) &", " &  FormatNumber(o(1,0),2) & ", " &  FormatNumber(o(2,0),2) & ", " &  FormatNumber(o(3,0),2)& "|" & vbNewLine & _
		"|" & FormatNumber(o(1,0),2) &", " &  FormatNumber(o(1,1),2) & ", " &  FormatNumber(o(2,1),2) & ", " &  FormatNumber(o(3,1),2)& "|" & vbNewLine & _
		"|" & FormatNumber(o(2,0),2) &", " &  FormatNumber(o(1,2),2) & ", " &  FormatNumber(o(2,2),2) & ", " &  FormatNumber(o(3,2),2)& "|" & vbNewLine & _
		"|" & FormatNumber(o(3,0),2) &", " &  FormatNumber(o(1,3),2) & ", " &  FormatNumber(o(2,3),2) & ", " &  FormatNumber(o(3,3),2)& "|"
	elseif typename(o) = "ISIQuaternion" then
		DebugPrint  =  _
		"[" & FormatNumber(o.W,2) &" V(" &  FormatNumber(o.x,2) & ", " &  FormatNumber(o.y,2) & ", " &  FormatNumber(o.z,2)& ")]" 
	elseif typename(o) = "ISIRotation" then
		dim a : a = o.GetXYZAnglesValues2 

		DebugPrint  = "(" & FormatNumber(r2d(a(0)),2) &", " &  FormatNumber(r2d(a(1)),2) & ", " &  FormatNumber(r2d(a(2)),2) & ")" 
	elseif typename(dict) = "Dictionary" then
		dim helper : helper = (dict("helper"))
		dim root :  root = (dict("root"))
		dim bone1 : bone1 = (dict("bone1"))
		dim bone2 : bone2 = (dict("bone2"))
		
		str = "setup->" & helper &":" & root & ":" & bone1 & ":" & bone2 & vbCrLf & _
			"basepos->" & dict("baseposx") &"," & dict("baseposy") & "," & dict("baseposz")
		logmessage fntab() & "fndumpdict: " & str
	elseif typename(o) <> "Nothing" then
		on error resume next
		DebugPrint = cstr(o)
		on error goto 0		
	end if
	
end function

function fntab()
	for i = 1 to fnindent
		fntab = fntab & vbTab
	next
end function

sub DebugTrace( val, level )
	if gDebugLevel And level then
		dim str : str = DebugPrint(val)
		logmessage fntab() & "DebugTrace: " & str
	end if
end sub

sub DebugBegin( str )
	fnprevproc = fnproc
	fnproc = str
	fnindent=fnindent+1
	if gDebugLevel And 1 then
		logmessage fntab() & "DebugBegin: " & cstr(str)
	end if
end sub

sub DebugEnd()
	if gDebugLevel And 1 then
		logmessage fntab() & "DebugEnd"
	end if
	fnindent=fnindent-1
	fnproc = fnprevproc
end sub

