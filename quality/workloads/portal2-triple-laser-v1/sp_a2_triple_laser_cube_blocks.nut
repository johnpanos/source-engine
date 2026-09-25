// sp_a2_triple_laser redirection cube: cube 2 sits in the wall laser, which
// fires east from (7472 -5728 18), and is turned north, west and south, then
// moved out of the beam. The client draws each beam by re-tracing it every
// frame (C_PortalLaser::ClientThink), so the incoming beam must end at the cube
// on the client as well as on the server, whichever way the cube faces. The
// client once traced through the cube's translucent glass and drew the wall
// laser straight through it.
//
// Each step opens a QA_WINDOW with sv_debug_laser_trace and
// cl_debug_laser_trace on; the workload's console checks read the server and
// client beam ends inside it.

IncludeScript( "qa/qa_driver" )

::CB <- {
	cubeSpot = Vector( 8000, -5728, 20 )
	awaySpot = Vector( 7700, -5900, 20 )
}

function QA_Shot( name )
{
	printl( "QA_SHOT " + ::QA.scenario + " " + name )
	SendToConsole( "screenshot" )
}

function CB_Place( yaw, origin )
{
	local cube = QA_Ent( "new_box1" )
	cube.SetOrigin( origin )
	cube.SetAngles( 0, yaw, 0 )
	cube.SetVelocity( Vector( 0, 0, 0 ) )
	QA_Log( "cube at " + QA_Vec( cube.GetOrigin() ) + " yaw " + yaw )
}

// The cube's own beam is an env_portal_laser parented to it
function CB_CubeLaser()
{
	local laser = null
	while ( laser = Entities.FindByClassname( laser, "env_portal_laser" ) )
	{
		if ( QA_ParentName( laser ) == "new_box1" )
			return laser
	}
	return null
}

// Both traces on for half a second, bracketed for the console checks
function CB_Window( label )
{
	QA_Do( "trace " + label, function() : ( label )
	{
		printl( "QA_WINDOW " + label + " BEGIN" )
		SendToConsole( "sv_debug_laser_trace 1; cl_debug_laser_trace 1" )
	}, 0.5 )
	QA_Do( "stop tracing " + label, function() : ( label )
	{
		SendToConsole( "sv_debug_laser_trace 0; cl_debug_laser_trace 0" )
		printl( "QA_WINDOW " + label + " END" )
	}, 0.3 )
}

function CB_Turn( label, yaw )
{
	QA_Do( "turn " + label, function() : ( yaw ) { CB_Place( yaw, ::CB.cubeSpot ) }, 2.0 )
	CB_Window( label )
	QA_Do( "shot " + label, function() : ( label ) { QA_Shot( "cube_" + label ) }, 1.0 )
}

QA_Do( "setup", function()
{
	SendToConsole( "sv_cheats 1; con_drawnotify 0; cl_drawhud 0; developer 0" )
}, 1.0 )
QA_Sleep( 8.0 )

// The entry trigger spawns cube 2
QA_Do( "leave the elevator", function()
{
	QA_StandIn( QA_EntNear( "trigger_once", Vector( 8000, -5184, 128 ) ), -90.0 )
}, 0.5 )
QA_WaitFor( "entry.cube_spawned", function()
{
	return Entities.FindByName( null, "new_box1" ) != null
}, 5.0 )
QA_Do( "view the cube", function() { QA_PlaceEye( Vector( 7700, -5560, 90 ), 18.0, -35.0 ) }, 1.0 )

CB_Turn( "north", 90 )
QA_Expect( "cube.redirects", function()
{
	local laser = CB_CubeLaser()
	QA_Detail( laser ? "cube laser " + laser.entindex() : "no cube laser" )
	return laser != null
} )
CB_Turn( "west", 180 )
CB_Turn( "south", 270 )

QA_Do( "move away", function() { CB_Place( 90, ::CB.awaySpot ) }, 2.0 )
CB_Window( "away" )
QA_Do( "shot away", function() { QA_Shot( "cube_away" ) }, 1.0 )
QA_Expect( "away.cube_laser_removed", function()
{
	local laser = CB_CubeLaser()
	QA_Detail( laser ? "cube laser " + laser.entindex() + " remains" : "no cube laser" )
	return laser == null
} )

QA_Start( "sp_a2_triple_laser_cube_blocks" )
