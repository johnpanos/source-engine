'#######################################################################################
' ValveSource VMF Tools
' Version v1.0
' Author: Dominic Laflamme
'#######################################################################################

'#######################################################################################
' CONSTANTS 
'#######################################################################################

' debug constants, turns on stack tracing
' replace comment (') before fnend, fnbegin & fntrace to enable tracing
Const fndebug = True

' constants for plugin installation 
Const gToolName =  "ValveSource VMF Tools"
dim gToolPath : gToolPath = ""
Const gDefaultRigName = "ValveBiped"

'#######################################################################################
' Entry for registration of contents
'#######################################################################################

function XSILoadPlugin( in_reg )
	in_reg.Author = "Softimage"
	in_reg.Name = gToolName
	in_reg.Major = 1
	in_reg.Minor = 0

	call in_reg.RegisterCommand("vstMakeRoom", "vstMakeRoom")
	call in_reg.RegisterCommand("vstMakeDoor", "vstMakeDoor")
	call in_reg.RegisterCommand("vstMakeCorridor", "vstMakeCorridor")

	XSILoadPlugin = True
end function

'#######################################################################################
' COMMANDS
'#######################################################################################

'***************************************************************************************
' command	:	vstMakeRoom
' description	:	
'***************************************************************************************
Function vstMakeRoom_Init( in_ctxt )
	set oCmd = in_ctxt.Source
	oCmd.ReturnValue = True 
	dim args : set args = oCmd.arguments
	args.add "Root", siArgumentInput, "Room"
	args.add "size", siArgumentInput, -1 
	args.add "depth", siArgumentInput, -1 
	vstMakeRoom_Init = True
end Function

Function vstMakeRoom_Execute(room,size,depth)
	
	set vstMakeRoom_Execute = Nothing
	
	if ( size = -1 ) then
		size = InputBox ("Enter the size of the room", "MakeRoom", 256 )
	end if
	
	if ( depth = -1 ) then
		depth = InputBox ("Enter the depth of the walls", "MakeRoom", 1 )
	end if
	
	set master = MakeWall(size,depth)

	' translate the center
	Translate , 0, size / 2 , 0, siAbsolute, siPivot, siCtr, siY, , , , , , , , , , 0
	
	' create wall
	Duplicate master.fullname, , 2, 1, 1, 0, 0, 1, 0, 1, , , , , , , , , , , 0
	set master1 = selection(0)
	Rotate , 0, 0, 90, siAbsolute, siAdd, siObj, siXYZ, , , , , , , , 0
	
	Duplicate master.fullname, , 2, 1, 1, 0, 0, 1, 0, 1, , , , , , , , , , , 0
	set master2 = selection(0)
	Rotate , 0, 180, -90, siAbsolute, siAdd, siObj, siXYZ, , , , , , , , 0
	
	Duplicate master.fullname, , 2, 1, 1, 0, 0, 1, 0, 1, , , , , , , , , , , 0
	set master3 = selection(0)
	Rotate , 0, 0, 180,siAbsolute, siAdd, siObj, siXYZ, , , , , , , , 0
	
	Duplicate master.fullname, , 2, 1, 1, 0, 0, 1, 0, 1, , , , , , , , , , , 0
	set master4 = selection(0)
	Rotate , 90, 0, 90,siAbsolute, siAdd, siObj, siXYZ, , , , , , , , 0
	
	Duplicate master.fullname, , 2, 1, 1, 0, 0, 1, 0, 1, , , , , , , , , , , 0
	set master5 = selection(0)
	Rotate , -90, 0, 90,siAbsolute, siAdd, siObj, siXYZ, , , , , , , , 0
	
	Selectobj master.fullname & ","_
	& master1.fullname &_
	"," & master2.fullname &_
	"," & master3.fullname &_
	"," & master4.fullname & "," & master5.fullname
	ResetTransform , siCtr, siSRT, siXYZ
	
	CreateModel
	Selection(0).Name = room
	
	set vstMakeRoom_Execute  = Selection(0)
	
end Function

'***************************************************************************************
' vstMakeRoom HELPER FUNCTIONS
'***************************************************************************************
function MakeWall(size,depth)

	CreatePrim "Grid", "MeshSurface"
	Selection(0).Name = "Wall"
	wall_fullname = Selection(0).fullname
	SetValue wall_fullname & ".polymsh.geom.subdivu", 1
	SetValue wall_fullname & ".polymsh.geom.subdivv", 1
	SetValue wall_fullname & ".grid.ulength", size
	SetValue wall_fullname & ".grid.vlength", size 

	SelectGeometryComponents wall_fullname & ".poly[0]"
	ApplyTopoOp "ExtrudeComponentAxis", wall_fullname & ".poly[0]", siUnspecified, siPersistentOperation
	SetValue wall_fullname & ".polymsh.extrudecompaxisop.inset", depth
	SetValue wall_fullname & ".polymsh.extrudecompaxisop.length", depth

	ApplyTopoOp "CreatePolygon", wall_fullname
	ConnectNewPolygonToEdge wall_fullname, 3
	ConnectNewPolygonToEdge wall_fullname, 1
	SelectObj wall_fullname & ".poly[LAST]"
	SelectFilter "Polygon"
	ActivateObjectSelTool

	FreezeObj
	
	set obj = Selection(0)

	set MakeWall = obj
	
end function

'***************************************************************************************
' command	:	vstMakeDoor
' description	:	
'***************************************************************************************
Function vstMakeDoor_Init( in_ctxt )
	set oCmd = in_ctxt.Source
	oCmd.ReturnValue = True 
	dim oArgs : set oArgs = oCmd.arguments
	call oArgs.AddWithHandler("Walls",siArgHandlerCollection)
	vstMakeDoor_Init = True
end Function

Function vstMakeDoor_Execute(walls)
	set vstMakeDoor_Execute = Nothing
	
	if ( typename(walls) = "Nothing" ) then
		call logmessage( "vstMakeDoor: invalid walls argument", siError )
	end if
	
	for each wall in walls 
		MakeDoor wall
	next
end Function

'***************************************************************************************
' vstMakeDoor HELPER FUNCTIONS
'***************************************************************************************
function MakeDoor( wall )

	set MakeDoor = Nothing
	
	if ( typename(wall) = "Nothing" ) then
		exit function
	end if
	
	modelname =  wall.model
	dim wall_name : wall_name =  wall.name
	dim wall_origfullname : wall_origfullname = wall.fullname
	wall.name = wall.name & "_part1"
	dim wall_fullname : wall_fullname =  wall.fullname
	
	' subdide room
	ActivateEdgeSelTool
	SelectGeometryComponents wall_fullname  & ".edge[0,2,8,10]"
	ApplyTopoOp "SubdivideEdge", wall_fullname  & ".edge[0,2,8,10]", siUnspecified, siPersistentOperation
	SetValue wall_fullname  & ".polymsh.subdivideedgeop.subdivisions", 3
	ActivateEdgeSelTool
	ActivateEdgeSelTool
	SelectGeometryComponents wall_fullname  & ".edge[20,21,26,LAST]"
	ApplyTopoOp "SubdivideEdge", wall_fullname  & ".edge[20,21,26,LAST]", siUnspecified, siPersistentOperation
	ActivateObjectSelTool
	ActivateObjectSelTool
	ActivateRaycastPolySelTool
	ActivateRaycastPolySelTool
	SelectGeometryComponents wall_fullname  & ".poly[LAST]"
	ToggleSelection wall_fullname  & ".poly[6]", , True
	ToggleSelection wall_fullname  & ".poly[10]", , True
	ApplyTopoOp "DeleteComponent", wall_fullname  & ".poly[6,10,LAST]", siUnspecified, siPersistentOperation
	
	' create top part
	SelectGeometryComponents wall_fullname  & ".poly[LAST]"
	ToggleSelection wall_fullname  & ".poly[8]", , True
	ToggleSelection wall_fullname  & ".poly[6]", , True
	ExtractFromComponents "ExtractPolygonsOp", , wall_name & "_part2", True, siImmediateOperation, siKeepGenOpInputs
	
	set tomove = selection(0)
	CopyPaste tomove.fullname, , modelname, 1
	
	topname = Selection(0).fullname
	
	' cap top part
	
	ApplyTopoOp "CreatePolygon", topname
	ConnectNewPolygonToEdge topname, 7
	ConnectNewPolygonToEdge topname, 2
	SelectObj topname & ".poly[LAST]"
	SelectFilter "Polygon"
	ActivateObjectSelTool
	ActivateObjectSelTool
	ApplyTopoOp "CreatePolygon", topname
	ConnectNewPolygonToEdge topname, 8
	ConnectNewPolygonToEdge topname, 1
	SelectObj topname & ".poly[LAST]"
	SelectFilter "Polygon"
	ActivateObjectSelTool
	ActivateObjectSelTool
	ApplyTopoOp "CreatePolygon", topname
	ConnectNewPolygonToEdge topname, 0
	ConnectNewPolygonToEdge topname, 9
	SelectObj topname & ".poly[LAST]"
	SelectFilter "Polygon"
	FreezeObj
	ActivateObjectSelTool
	ActivateObjectSelTool
	DeselectAll
	
	' create last part
	SelectObj wall_fullname, , True
	ActivateRaycastPolySelTool
	ActivateRaycastPolySelTool
	SelectGeometryComponents wall_fullname & ".poly[5]"
	ToggleSelection wall_fullname & ".poly[1]", , True
	ToggleSelection wall_fullname & ".poly[0]", , True
	ToggleSelection wall_fullname & ".poly[LAST]", , True
	ToggleSelection wall_fullname & ".poly[4]", , True
	ExtractFromComponents "ExtractPolygonsOp", , wall_name & "_part3", True, siImmediateOperation, siKeepGenOpInputs
	
	set tomove2 = selection(0)
	CopyPaste tomove2.fullname, , modelname, 1
	
	leftname = Selection(0).fullname
	
	' cap left part
	
	SelectGeometryComponents leftname & ".pnt[3,8]"
	ApplyTopoOp "Collapse", leftname & ".pnt[3,8]", siUnspecified, siPersistentOperation
	ApplyTopoOp "CreatePolygon", leftname
	ConnectNewPolygonToEdge leftname, 3
	ConnectNewPolygonToEdge leftname, 2
	FreezeObj
	
	' finish up
	
	ActivateVertexSelTool
	SelectGeometryComponents wall_fullname & ".pnt[0,2]"
	ApplyTopoOp "Collapse", wall_fullname & ".pnt[0,2]", siUnspecified, siPersistentOperation
	ApplyTopoOp "CreatePolygon", wall_fullname
	ConnectNewPolygonToEdge wall_fullname, 6
	ConnectNewPolygonToEdge wall_fullname, 0
	SelectObj wall_fullname & ".poly[LAST]"
	SelectFilter "Polygon"
	ActivateObjectSelTool
	
	' create alignement clusters
	SelectObj wall_fullname, , True
	SelectGeometryComponents wall_fullname  & ".pnt[0]"
	CreateCluster
	SetValue wall_fullname & ".polymsh.cls.Point.Name", "LowerRight"
	
	SelectObj wall_fullname, , True
	SelectGeometryComponents wall_fullname  & ".pnt[LAST]"
	CreateCluster
	SetValue wall_fullname & ".polymsh.cls.Point.Name", "LowerRightInner"
	
	SelectObj topname, , True
	SelectGeometryComponents topname & ".pnt[1]"
	CreateCluster
	SetValue topname & ".polymsh.cls.Point.Name", "UpperRight"
	
	SelectObj topname, , True
	SelectGeometryComponents topname & ".pnt[2]"
	CreateCluster
	SetValue topname & ".polymsh.cls.Point.Name", "UpperLeft"
	
	SelectObj leftname, , True
	SelectGeometryComponents leftname  & ".pnt[4]"
	CreateCluster
	SetValue leftname & ".polymsh.cls.Point.Name", "LowerLeft"
	SelectObj leftname, , True
	SelectGeometryComponents leftname  & ".pnt[LAST]"
	CreateCluster
	SetValue leftname & ".polymsh.cls.Point.Name", "LowerLeftInner"
	
	set prim  = getprim("Null", wall_name, wall.parent )
	parentobj prim, wall_origfullname & "_part*"
	
end function

'***************************************************************************************
' command	:	vstMakeCorridor
' description	:	
'***************************************************************************************
Function vstMakeCorridor_Init( in_ctxt )
	set oCmd = in_ctxt.Source
	oCmd.ReturnValue = True 
	dim args : set args = oCmd.arguments
	dim oArgs : set oArgs = oCmd.Arguments
	call oArgs.AddWithHandler("Walls",siArgHandlerCollection)
	vstMakeCorridor_Init = True
end Function

Function vstMakeCorridor_Execute(Walls)
	set vstMakeCorridor_Execute = Nothing
	
	if ( typename(Walls) = "Nothing" ) then
		call logmessage( "vstMakeCorridor: invalid WallWithDoor argument", siError )
	end if
	
	dim objects(6)
	dim lowerleftx(2)
	dim lowerlefty(2)
	dim lowerleftz(2)
	dim lowerrightx(2)
	dim lowerrighty(2)
	dim lowerrightz(2)
	dim upperleftx(2)
	dim upperlefty(2)
	dim upperleftz(2)
	dim upperrightx(2)
	dim upperrighty(2)
	dim upperrightz(2)
	
	if Walls.Count <> 2  then
		msgbox "Please select the adjacent 2 wall with door"
		exit function
	end if

	dim a : a = 0
	for each model in Walls
	
		dim children : set children= Nothing
		on error resume next
		set children= model.children
		on error goto 0
		
		if (typename(children) = "Nothing") then
			msgbox "Please select the adjacent 2 wall with door"
			exit function
		end if
		if ( children.count <> 3 ) then
			msgbox "Please select the adjacent 2 wall with door"
			exit function
		end if
		for each o in children
			set objects(a) = o
			a = a + 1
		next
	next
	
	ResetTransform , siCtr, siSRT, siXYZ
	
	' upper right
	'LowerRight
	
	'
	' Floor
	'
	
	CreatePrim "Grid", "MeshSurface"
	set grid = Selection(0)
	SetValue grid.fullname & ".polymsh.geom.subdivu", 1
	SetValue grid.fullname & ".polymsh.geom.subdivv", 1
	FreezeObj
	
	CreateModel
	
	grid.model.name = "Corridor"
	
	GlobalModelName = grid.model
	
	
	dim lowerx1, lowery1, lowerz1
	dim lowerx2, lowery2, lowerz2
	
	MakeCorridor_DoIt objects, "LowerRight", "LowerLeft", lowerx1,lowery1,lowerz1,lowerx2,lowery2,lowerz2
	
	SelectGeometryComponents grid.fullname & ".pnt[1]"
	Translate , lowerx1, lowery1, lowerz1, siAbsolute, siView, siObj, siXYZ, , , , , , , , , , 0
	SelectGeometryComponents grid.fullname & ".pnt[LAST]"
	Translate , lowerx2, lowery2, lowerz2, siAbsolute, siView, siObj, siXYZ, , , , , , , , , , 0
	
	dim rlowerx1, rlowery1, rlowerz1
	dim rlowerx2, rlowery2, rlowerz2
	MakeCorridor_DoIt objects, "LowerLeft", "LowerRight", rlowerx1,rlowery1,rlowerz1,rlowerx2,rlowery2,rlowerz2
	
	SelectGeometryComponents grid.fullname & ".pnt[0]"
	Translate , rlowerx1, rlowery1, rlowerz1, siAbsolute, siView, siObj, siXYZ, , , , , , , , , , 0
	SelectGeometryComponents grid.fullname & ".pnt[2]"
	Translate , rlowerx2, rlowery2, rlowerz2, siAbsolute, siView, siObj, siXYZ, , , , , , , , , , 0
	
	ApplyTopoOp "ExtrudeComponentAxis", grid.fullname & ".poly[0]", siUnspecified, siPersistentOperation
	
	MakeCorridor_DoIt objects, "LowerLeftInner", "LowerRightInner", lowerx1,lowery1,lowerz1,lowerx2,lowery2,lowerz2
	SelectGeometryComponents grid.fullname & ".pnt[0]"
	Translate , lowerx1, lowery1, lowerz1, siAbsolute, siView, siObj, siXYZ, , , , , , , , , , 0
	SelectGeometryComponents grid.fullname & ".pnt[LAST]"
	Translate , lowerx2, lowery2, lowerz2, siAbsolute, siView, siObj, siXYZ, , , , , , , , , , 0
	
	MakeCorridor_DoIt objects, "LowerRightInner", "LowerLeftInner", lowerx1,lowery1,lowerz1,lowerx2,lowery2,lowerz2
	SelectGeometryComponents grid.fullname & ".pnt[6]"
	Translate , lowerx1, lowery1, lowerz1, siAbsolute, siView, siObj, siXYZ, , , , , , , , , , 0
	SelectGeometryComponents grid.fullname & ".pnt[4]"
	Translate , lowerx2, lowery2, lowerz2, siAbsolute, siView, siObj, siXYZ, , , , , , , , , , 0
	
	SelectObj grid.fullname, , True
	ApplyTopoOp "CreatePolygon", grid.fullname
	ConnectNewPolygonToEdge grid.fullname, 3
	ConnectNewPolygonToEdge grid.fullname, 1
	SelectObj grid.fullname & ".poly[LAST]"
	SelectFilter "Polygon"
	ActivateObjectSelTool
	FreezeObj
	
	'
	' Ceiling
	'
	
	
	CreatePrim "Grid", "MeshSurface"
	set grid = Selection(0)
	SetValue grid.fullname & ".polymsh.geom.subdivu", 1
	SetValue grid.fullname & ".polymsh.geom.subdivv", 1
	FreezeObj
	
	CopyPaste grid.fullname, , GlobalModelName, 1
	
	MakeCorridor_DoIt objects, "UpperRight", "UpperLeft", lowerx1,lowery1,lowerz1,lowerx2,lowery2,lowerz2
	
	SelectGeometryComponents grid.fullname & ".pnt[2]"
	Translate , lowerx1, lowery1, lowerz1, siAbsolute, siView, siObj, siXYZ, , , , , , , , , , 0
	SelectGeometryComponents grid.fullname & ".pnt[0]"
	Translate , lowerx2, lowery2, lowerz2, siAbsolute, siView, siObj, siXYZ, , , , , , , , , , 0
	
	MakeCorridor_DoIt objects, "UpperLeft", "UpperRight", lowerx1,lowery1,lowerz1,lowerx2,lowery2,lowerz2
	
	SelectGeometryComponents grid.fullname & ".pnt[LAST]"
	Translate , lowerx1, lowery1, lowerz1, siAbsolute, siView, siObj, siXYZ, , , , , , , , , , 0
	SelectGeometryComponents grid.fullname & ".pnt[1]"
	Translate , lowerx2, lowery2, lowerz2, siAbsolute, siView, siObj, siXYZ, , , , , , , , , , 0
	
	SelectObj grid.fullname, , True
	ActivateRaycastPolySelTool
	ActivateRaycastPolySelTool
	SelectGeometryComponents grid.fullname & ".poly[0]"
	ApplyTopoOp "ExtrudeComponentAxis", grid.fullname & ".poly[0]", siUnspecified, siPersistentOperation
	
	ApplyTopoOp "CreatePolygon", grid.fullname
	ConnectNewPolygonToEdge grid.fullname, 1
	ConnectNewPolygonToEdge grid.fullname, 3
	SelectObj grid.fullname & ".poly[LAST]"
	SelectFilter "Polygon"
	ActivateObjectSelTool
	FreezeObj
	
	'
	' Walls
	'
	
	FillPositions objects,  "LowerLeft", lowerleftx, lowerlefty, lowerleftz 
	FillPositions objects,  "LowerRight", lowerrightx, lowerrighty, lowerrightz 
	FillPositions objects,  "UpperRight", upperrightx, upperrighty, upperrightz
	FillPositions objects,  "UpperLeft", upperleftx, upperlefty, upperleftz
	
	CreatePrim "Grid", "MeshSurface"
	set grid = Selection(0)
	SetValue grid.fullname & ".polymsh.geom.subdivu", 1
	SetValue grid.fullname & ".polymsh.geom.subdivv", 1
	FreezeObj
	CopyPaste grid.fullname, , GlobalModelName, 1
	
	SelectGeometryComponents grid.fullname & ".pnt[0]"
	Translate , lowerleftx(0), lowerlefty(0), lowerleftz(0), siAbsolute, siView, siObj, siXYZ, , , , , , , , , , 0
	SelectGeometryComponents grid.fullname & ".pnt[1]"
	Translate , upperleftx(0), upperlefty(0), upperleftz(0), siAbsolute, siView, siObj, siXYZ, , , , , , , , , , 0
	
	SelectGeometryComponents grid.fullname & ".pnt[2]"
	Translate , lowerrightx(1), lowerrighty(1), lowerrightz(1), siAbsolute, siView, siObj, siXYZ, , , , , , , , , , 0
	SelectGeometryComponents grid.fullname & ".pnt[LAST]"
	Translate , upperrightx(1), upperrighty(1), upperrightz(1), siAbsolute, siView, siObj, siXYZ, , , , , , , , , , 0
	
	SelectObj grid.fullname, , True
	ActivateRaycastPolySelTool
	ActivateRaycastPolySelTool
	SelectGeometryComponents grid.fullname & ".poly[0]"
	ApplyTopoOp "ExtrudeComponentAxis", grid.fullname & ".poly[0]", siUnspecified, siPersistentOperation
	
	SelectObj grid.fullname, , True
	ApplyTopoOp "CreatePolygon", grid.fullname
	ConnectNewPolygonToEdge grid.fullname, 1
	ConnectNewPolygonToEdge grid.fullname, 3
	SelectObj grid.fullname & ".poly[LAST]"
	SelectFilter "Polygon"
	ActivateObjectSelTool
	FreezeObj
	
	'
	' Last Wall
	'
	
	CreatePrim "Grid", "MeshSurface"
	set grid = Selection(0)
	SetValue grid.fullname & ".polymsh.geom.subdivu", 1
	SetValue grid.fullname & ".polymsh.geom.subdivv", 1
	FreezeObj
	CopyPaste grid.fullname, , GlobalModelName, 1
	
	SelectGeometryComponents grid.fullname & ".pnt[0]"
	Translate , lowerleftx(1), lowerlefty(1), lowerleftz(1), siAbsolute, siView, siObj, siXYZ, , , , , , , , , , 0
	SelectGeometryComponents grid.fullname & ".pnt[1]"
	Translate , upperleftx(1), upperlefty(1), upperleftz(1), siAbsolute, siView, siObj, siXYZ, , , , , , , , , , 0
	
	SelectGeometryComponents grid.fullname & ".pnt[2]"
	Translate , lowerrightx(0), lowerrighty(0), lowerrightz(0), siAbsolute, siView, siObj, siXYZ, , , , , , , , , , 0
	SelectGeometryComponents grid.fullname & ".pnt[LAST]"
	Translate , upperrightx(0), upperrighty(0), upperrightz(0), siAbsolute, siView, siObj, siXYZ, , , , , , , , , , 0
	
	SelectObj grid.fullname, , True
	ActivateRaycastPolySelTool
	ActivateRaycastPolySelTool
	SelectGeometryComponents grid.fullname & ".poly[0]"
	ApplyTopoOp "ExtrudeComponentAxis", grid.fullname & ".poly[0]", siUnspecified, siPersistentOperation
	
	SelectObj grid.fullname, , True
	ApplyTopoOp "CreatePolygon", grid.fullname
	ConnectNewPolygonToEdge grid.fullname, 1
	ConnectNewPolygonToEdge grid.fullname, 3
	SelectObj grid.fullname & ".poly[LAST]"
	SelectFilter "Polygon"
	ActivateObjectSelTool
	FreezeObj
end Function


function MakeCorridor_DoIt ( objects, string1, string2, x1, y1, z1, x2, y2, z2 )


	dim found
	found =0
	
	dim obj1
	dim obj2
	obj2 = "nothing"
	
	for a = 0 to 5
	set oPrimitive = objects(a).activeprimitive
	set oGeometry = oPrimitive.geometry
	set oClusters = oGeometry.clusters
	set oClusterElementsCollection = oClusters(0).Elements
	
	dim cluster1
	dim cluster2
	
	for each oCluster in oClusters
	
	if oCluster.name = string1 then
	
		if obj2 = "nothing" then
			set cluster2 = oCluster
			set obj2 = objects(a)
		end if
		
	else 
		if oCluster.name = string2 then
			set cluster1 = oCluster
			set obj1 = objects(a)
		end if
		
	end if
	next
	
	next
	
	
	set oClusterElementsCollection = cluster1.Elements
	 aElements1 = oClusterElementsCollection.Array  
	 
	 set oClusterElementsCollection = cluster2.Elements
	 aElements2 = oClusterElementsCollection.Array  
	  
	aPositions = obj2.activeprimitive.Geometry.Points.PositionArray
	aPositions1 = obj1.activeprimitive.Geometry.Points.PositionArray
	
	'SelectGeometryComponents obj1.fullname & ".pnt["& aElements1(0) &" ]"
	'Translate , aPositions( 0, aElements2(0) ), aPositions( 1, aElements2(0) ), aPositions( 2, aElements2(0) ), siAbsolute, siView, siObj, siXYZ, , , , , , , , , , 0
	
	x1 = aPositions( 0, aElements2(0) )
	y1 = aPositions( 1, aElements2(0) )
	z1 = aPositions( 2, aElements2(0) )
	
	x2 = aPositions1( 0, aElements1(0) )
	y2 = aPositions1( 1, aElements1(0) )
	z2 = aPositions1( 2, aElements1(0) )
	
	

end function



function GetPositionFromSameRoom ( string1, string2, x1, y1, z1, x2, y2, z2, ModelToSkip )


	dim found
	found =0
	
	dim obj1
	dim obj2
	obj2 = "nothing"
	
	for a = 0 to 5
	set oPrimitive = objects(a).activeprimitive
	set oGeometry = oPrimitive.geometry
	set oClusters = oGeometry.clusters
	set oClusterElementsCollection = oClusters(0).Elements
	
	dim cluster1
	dim cluster2
	dim modelname
	
	for each oCluster in oClusters
	
	if oCluster.name = string1 then
	
		if obj2 = "nothing" then
	
			set cluster2 = oCluster
			set obj2 = objects(a)
			modelname = obj2.model.fullname
			
			logmessage modelname
		end if
		
	else 
		if oCluster.name = string2 then
	
			if obj2 <> "nothing" then
	
				logmessage " AAA: " & objects(a).model.fullname
				if modelname = objects(a).model.fullname then 
					set cluster1 = oCluster
					set obj1 = objects(a)
					ModelToSkip = modelname
				end if
			else 
			
				set cluster2 = oCluster
				set obj2 = objects(a)
				modelname = obj2.model.fullname
			
			end if
			
				
		end if
		
	end if
	next
	
	next
	
	
	set oClusterElementsCollection = cluster1.Elements
	 aElements1 = oClusterElementsCollection.Array  
	 
	 set oClusterElementsCollection = cluster2.Elements
	 aElements2 = oClusterElementsCollection.Array  
	  
	aPositions = obj2.activeprimitive.Geometry.Points.PositionArray
	aPositions1 = obj1.activeprimitive.Geometry.Points.PositionArray
	
	'SelectGeometryComponents obj1.fullname & ".pnt["& aElements1(0) &" ]"
	'Translate , aPositions( 0, aElements2(0) ), aPositions( 1, aElements2(0) ), aPositions( 2, aElements2(0) ), siAbsolute, siView, siObj, siXYZ, , , , , , , , , , 0
	
	x1 = aPositions( 0, aElements2(0) )
	y1 = aPositions( 1, aElements2(0) )
	z1 = aPositions( 2, aElements2(0) )
	
	x2 = aPositions1( 0, aElements1(0) )
	y2 = aPositions1( 1, aElements1(0) )
	z2 = aPositions1( 2, aElements1(0) )

end function


function GetPositionFromSameRoomWithSkip ( string1, string2, x1, y1, z1, x2, y2, z2, ModelToSkip )

	dim found
	found =0
	
	dim obj1
	dim obj2
	obj2 = "nothing"
	
	for a = 0 to 5
	set oPrimitive = objects(a).activeprimitive
	set oGeometry = oPrimitive.geometry
	set oClusters = oGeometry.clusters
	set oClusterElementsCollection = oClusters(0).Elements
	
	dim cluster1
	dim cluster2
	dim modelname
	
	for each oCluster in oClusters
	
	if oCluster.name = string1 then
	
		if objects(a).model.name <> ModelToSkip then 
			if obj2 = "nothing" then
				set cluster2 = oCluster
				set obj2 = objects(a)
				modelname = obj2.model.fullname
			end if
		end if
		
	else 
		if oCluster.name = string2 then
	
			if objects(a).model.name <> ModelToSkip then 
				if modelname = objects(a).model.fullname then 
					set cluster1 = oCluster
					set obj1 = objects(a)
				end if
			end if		
		end if
		
	end if
	next
	
	next
	
	
	set oClusterElementsCollection = cluster1.Elements
	 aElements1 = oClusterElementsCollection.Array  
	 
	 set oClusterElementsCollection = cluster2.Elements
	 aElements2 = oClusterElementsCollection.Array  
	  
	aPositions = obj2.activeprimitive.Geometry.Points.PositionArray
	aPositions1 = obj1.activeprimitive.Geometry.Points.PositionArray
	
	'SelectGeometryComponents obj1.fullname & ".pnt["& aElements1(0) &" ]"
	'Translate , aPositions( 0, aElements2(0) ), aPositions( 1, aElements2(0) ), aPositions( 2, aElements2(0) ), siAbsolute, siView, siObj, siXYZ, , , , , , , , , , 0
	
	x1 = aPositions( 0, aElements2(0) )
	y1 = aPositions( 1, aElements2(0) )
	z1 = aPositions( 2, aElements2(0) )
	
	x2 = aPositions1( 0, aElements1(0) )
	y2 = aPositions1( 1, aElements1(0) )
	z2 = aPositions1( 2, aElements1(0) )

end function

function FillPositions ( objects, string1, arrayx, arrayy, arrayz )

	dim found
	found =0
	
	dim obj1
	dim obj2
	obj2 = "nothing"
	
	for a = 0 to 5
		set oPrimitive = objects(a).activeprimitive
		set oGeometry = oPrimitive.geometry
		set oClusters = oGeometry.clusters
		set oClusterElementsCollection = oClusters(0).Elements
	
		dim cluster1
		dim cluster2
		dim modelname
		
		cluster1 = "nothing"
	
		for each oCluster in oClusters
	
		if oCluster.name = string1 then
		
			'
			' fill array
			'
		
			set oClusterElementsCollection = oCluster.Elements
			aElements2 = oClusterElementsCollection.Array  
	 
			aPositions = objects(a).activeprimitive.Geometry.Points.PositionArray
			arrayx (found) = aPositions( 0, aElements2(0) )
			arrayy (found) = aPositions( 1, aElements2(0) )
			arrayz (found) = aPositions( 2, aElements2(0) )
		
			found = found + 1
		
		end if
				
		next
	
	next

end function
