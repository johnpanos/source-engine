// sp_a2_triple_laser: three lasers, two redirection cubes, one portal pair and
// three catchers. The catchers face north into the entry hall and every laser
// starts in the south room, so all three beams go through one portal pair:
// portal A on the placement helper where the defunct laser strikes the south
// room's north wall, portal B on the entry hall's north wall above the
// catchers. The defunct laser enters A directly, cube 1 under the ceiling
// laser aims its beam into A, and cube 2 turns the wall laser north into A.
// Each beam leaves B at its own angle and lands in its own catcher; all three
// powered open the exit door.
//
// The driver also takes a screenshot at each major step (QA_Shot); the harness
// names them in order and composites them against the retail reference.

IncludeScript( "qa/qa_driver" )

::TL <- {
	helperA = Vector( 8000, -5504, 48 )   // the map's portal placement helper
	ceilingSpot = Vector( 7792, -5808, 0 ) // where the ceiling laser meets the floor
	catchers = [ "catcher_1", "catcher_2", "catcher_3" ]
}

function QA_Shot( name )
{
	printl( "QA_SHOT " + ::QA.scenario + " " + name )
	local player = QA_Player()
	QA_Log( "shot " + name + " eye " + QA_Vec( player.EyePosition() ) + " fwd " +
	        QA_Vec( QA_Scale( player.GetForwardVector(), 100.0 ) ) )
	SendToConsole( "screenshot" )
}

// Probe the walls the solution uses.
function TL_WallAhead( from, dir )
{
	local to = from + QA_Scale( dir, 2000.0 )
	return from + QA_Scale( dir, 2000.0 * TraceLine( from, to, null ) )
}

QA_Do( "setup", function()
{
	SendToConsole( "sv_cheats 1; con_drawnotify 0; cl_drawhud 0; developer 0; mat_forceaniso 16; mat_picmip; mat_forceaniso; mat_trilinear; mat_dxlevel; mat_hdr_level; r_rootlod; mat_bumpmap; mat_detail_tex; mat_specular" )
	foreach ( name in ::TL.catchers )
	{
		QA_Watch( name, "OnPowered" )
		QA_Watch( name, "OnUnpowered" )
	}
	QA_Watch( "all_lasers_powered_rl", "OnTrigger" )
	QA_Watch( "@exit_door-testchamber_door", "OnFullyOpen" )
	QA_Watch( "new_box", "OnPlayerPickup" )
	local north = TL_WallAhead( Vector( 7420, -5400, 48 ), Vector( 0, 1, 0 ) )
	local south = TL_WallAhead( Vector( 8000, -5600, 48 ), Vector( 0, 1, 0 ) )
	QA_Log( "north wall above the catchers " + QA_Vec( north ) + ", helper wall " + QA_Vec( south ) )
	::TL.northWall <- north
}, 1.0 )

// 1: the arrival elevator, untouched.
QA_Sleep( 10.0 )
QA_Do( "shot arrival", function() { QA_Shot( "arrival" ) }, 1.0 )

// Out of the elevator through the entry trigger: it opens the entry door,
// turns on the chamber's shadowed projected light and spawns the second cube.
QA_Do( "leave the elevator", function()
{
	QA_StandIn( QA_EntNear( "trigger_once", Vector( 8000, -5184, 128 ) ), -90.0 )
}, 0.5 )
QA_WaitFor( "entry.cube_spawned", function()
{
	return Entities.FindByName( null, "new_box1" ) != null
}, 5.0 )

// 2: the entry hall: three dark catchers under their indicator strips.
QA_Do( "enter the chamber", function()
{
	QA_PlaceEye( Vector( 7760, -5340, 64 ), 12.0, -155.0 )
}, 1.5 )
QA_Do( "shot catchers", function() { QA_Shot( "catchers_unpowered" ) }, 1.0 )

// Material detail up close: the floor's ssbump tiles and detail dot grid
// (tile/white_floor_tile002a) and a black wall panel.
QA_Do( "look down at the floor", function()
{
	QA_PlaceEye( Vector( 7700, -5380, 64 ), 60.0, -100.0 )
}, 1.5 )
QA_Do( "shot floor", function() { QA_Shot( "floor_close" ) }, 1.0 )

QA_Expect( "catchers.start_unpowered", function()
{
	local n = 0
	foreach ( name in ::TL.catchers )
		n += QA_Fired( name, "OnPowered" )
	QA_Detail( "powered outputs " + n )
	return n == 0
} )

// 3: the south room: wall laser, ceiling laser, the defunct laser behind glass.
QA_Do( "look into the laser room", function()
{
	QA_PlaceEye( Vector( 7760, -5520, 64 ), 8.0, -80.0 )
}, 1.5 )
QA_Do( "shot laser room", function() { QA_Shot( "laser_room" ) }, 1.0 )

// 4: portal A on the helper where the defunct laser strikes.
QA_Do( "aim at the helper wall", function()
{
	local eye = Vector( 8000, -5700, 64 )
	local angles = QA_AnglesTo( eye, ::TL.helperA )
	QA_PlaceEye( eye, angles.pitch, angles.yaw )
}, 1.0 )
QA_Do( "fire portal A", function() { QA_Press( "attack", 0.1 ) }, 1.5 )

// Portal B on the entry hall's north wall, above catcher 2.
QA_Do( "aim at the north wall", function()
{
	local target = Vector( 7420, ::TL.northWall.y, 56 )
	local eye = Vector( 7420, -5420, 64 )
	local angles = QA_AnglesTo( eye, target )
	QA_PlaceEye( eye, angles.pitch, angles.yaw )
}, 1.0 )
QA_Do( "fire portal B", function() { QA_Press( "attack2", 0.1 ) }, 1.5 )

QA_Do( "log portals", function()
{
	local p = null
	while ( p = Entities.FindByClassname( p, "prop_portal" ) )
		QA_Log( "portal " + p.GetName() + " at " + QA_Vec( p.GetOrigin() ) + " angles " + QA_Vec( p.GetAngles() ) )
}, 0.1 )

// The portals head on, from the entry hall (B) and the laser room (A).
QA_Do( "face portal B", function()
{
	QA_PlaceEye( Vector( 7424, -5440, 56 ), 0.0, 90.0 )
}, 1.5 )
QA_Do( "shot portal B", function() { QA_Shot( "portal_b" ) }, 1.0 )
QA_Do( "face portal A", function()
{
	QA_PlaceEye( Vector( 8000, -5640, 56 ), 0.0, 90.0 )
}, 1.5 )
QA_Do( "shot portal A", function() { QA_Shot( "portal_a" ) }, 1.0 )

QA_WaitFor( "defunct.through_portals", function()
{
	local n = 0
	foreach ( name in ::TL.catchers )
		n += QA_Fired( name, "OnPowered" )
	QA_Detail( "powered outputs " + n )
	return n >= 1
}, 3.0 )
QA_Do( "shot first catcher", function()
{
	QA_PlaceEye( Vector( 7560, -5330, 64 ), 15.0, -120.0 )
}, 1.0 )
QA_Do( "shot first catcher 2", function() { QA_Shot( "portal_beam" ) }, 1.0 )

// 5: pick up cube 1 and carry it (the held cube in view).
QA_Do( "walk to the cube", function()
{
	local cube = QA_Ent( "new_box" ).GetCenter()
	QA_Player().SetOrigin( Vector( cube.x + 60, cube.y + 20, 0 ) )
}, 1.0 )
QA_WaitFor( "cube.picked_up", function()
{
	if ( QA_Fired( "new_box", "OnPlayerPickup" ) >= 1 )
		return true
	QA_TryPickup( "new_box", 44.0 )
	return false
}, 10.0 )
QA_Do( "level the view", function() { QA_SetView( 5.0, 60.0 ) }, 1.0 )
QA_Do( "shot holding", function() { QA_Shot( "holding_cube" ) }, 1.0 )
QA_Do( "drop", function() { QA_Press( "use", 0.1 ) }, 1.0 )

// Cube 1 under the ceiling laser, facing a point just west of portal A's
// center so its beam leaves B toward catcher 3.
QA_Do( "place cube 1", function()
{
	local cube = QA_Ent( "new_box" )
	local aim = Vector( 7982, -5504, 24 )
	local d = aim - ::TL.ceilingSpot
	cube.SetOrigin( Vector( ::TL.ceilingSpot.x, ::TL.ceilingSpot.y, 20 ) )
	cube.SetAngles( 0, QA_Deg( atan2( d.y, d.x ) ), 0 )
	cube.SetVelocity( Vector( 0, 0, 0 ) )
}, 2.0 )

// Cube 2 in the wall laser, turning it north into portal A.
QA_Do( "place cube 2", function()
{
	local cube = QA_Ent( "new_box1" )
	cube.SetOrigin( Vector( 8000, -5728, 20 ) )
	cube.SetAngles( 0, 90, 0 )
	cube.SetVelocity( Vector( 0, 0, 0 ) )
}, 2.0 )

QA_Do( "trace the lasers", function() { SendToConsole( "sv_debug_laser_trace 1" ) }, 0.2 )
QA_Do( "stop tracing", function() { SendToConsole( "sv_debug_laser_trace 0" ) }, 0.1 )

QA_Do( "look at the cubes", function()
{
	foreach ( name in [ "new_box", "new_box1" ] )
	{
		local cube = QA_Ent( name )
		QA_Log( name + " at " + QA_Vec( cube.GetOrigin() ) + " angles " + QA_Vec( cube.GetAngles() ) +
		        " forward " + QA_Vec( QA_Scale( cube.GetForwardVector(), 100.0 ) ) )
	}
	QA_PlaceEye( Vector( 7700, -5560, 90 ), 18.0, -35.0 )
}, 1.0 )
QA_Do( "shot cubes", function() { QA_Shot( "cubes_redirect" ) }, 1.0 )

QA_WaitFor( "catchers.all_powered", function()
{
	local powered = []
	foreach ( name in ::TL.catchers )
		powered.append( QA_Fired( name, "OnPowered" ) - QA_Fired( name, "OnUnpowered" ) )
	QA_Detail( "net powered " + powered[0] + " " + powered[1] + " " + powered[2] )
	return powered[0] > 0 && powered[1] > 0 && powered[2] > 0
}, 4.0 )

QA_WaitFor( "exit.relay", function() { return QA_Fired( "all_lasers_powered_rl", "OnTrigger" ) >= 1 }, 3.0 )

// 6: three beams out of portal B into three lit catchers.
QA_Do( "look at the catchers", function()
{
	QA_PlaceEye( Vector( 7760, -5340, 64 ), 12.0, -155.0 )
}, 1.0 )
QA_Do( "shot all powered", function() { QA_Shot( "all_powered" ) }, 1.0 )

QA_WaitFor( "exit.door_open", function()
{
	return QA_Fired( "@exit_door-testchamber_door", "OnFullyOpen" ) >= 1
}, 6.0 )

// 7: the open exit door.
QA_Do( "go to the exit", function()
{
	QA_PlaceEye( Vector( 7300, -5376, 64 ), 5.0, 180.0 )
}, 1.5 )
QA_Do( "shot exit", function() { QA_Shot( "exit_open" ) }, 1.0 )

// 8: portal rendering cases the head-on shots miss: a steep oblique view,
// right up against the portal, and a ceiling portal seen from below.
QA_Do( "oblique portal B", function()
{
	local eye = Vector( 7250, -5400, 110 )
	local angles = QA_AnglesTo( eye, Vector( 7424, -5312, 56 ) )
	QA_PlaceEye( eye, angles.pitch, angles.yaw )
}, 1.5 )
QA_Do( "shot oblique", function() { QA_Shot( "portal_oblique" ) }, 1.0 )
QA_Do( "close to portal B", function()
{
	QA_PlaceEye( Vector( 7424, -5350, 64 ), 0.0, 90.0 )
}, 1.5 )
QA_Do( "shot close", function() { QA_Shot( "portal_close" ) }, 1.0 )
QA_Do( "aim at the ceiling", function()
{
	QA_PlaceEye( Vector( 7700, -5380, 64 ), -89.0, 90.0 )
}, 1.0 )
QA_Do( "fire portal B up", function() { QA_Press( "attack2", 0.1 ) }, 1.5 )
QA_Do( "look up at it", function()
{
	local p = null
	local ceiling = null
	while ( p = Entities.FindByClassname( p, "prop_portal" ) )
	{
		QA_Log( "portal at " + QA_Vec( p.GetOrigin() ) + " angles " + QA_Vec( p.GetAngles() ) )
		if ( p.GetOrigin().z > 100 )
			ceiling = p.GetOrigin()
	}
	if ( ceiling == null )
		throw "no ceiling portal"
	local eye = Vector( ceiling.x - 120, ceiling.y - 60, 64 )
	local angles = QA_AnglesTo( eye, ceiling )
	QA_PlaceEye( eye, angles.pitch, angles.yaw )
}, 1.5 )
QA_Do( "shot ceiling", function() { QA_Shot( "portal_ceiling" ) }, 1.0 )
QA_Do( "far from portal A", function()
{
	local eye = Vector( 8000, -5950, 64 )
	local angles = QA_AnglesTo( eye, Vector( 8000, -5504, 56 ) )
	QA_PlaceEye( eye, angles.pitch, angles.yaw )
}, 1.5 )
QA_Do( "shot far", function() { QA_Shot( "portal_far" ) }, 1.0 )

// 9: recursion: portal B on the laser room's south wall facing portal A, so
// each portal shows the other (the fast path's second stencil level).
QA_Do( "aim at the south wall", function()
{
	QA_PlaceEye( Vector( 7900, -5600, 64 ), 0.0, -90.0 )
}, 1.0 )
QA_Do( "fire portal B south", function() { QA_Press( "attack2", 0.1 ) }, 1.5 )
QA_Do( "look down the tunnel", function()
{
	local p = null
	while ( p = Entities.FindByClassname( p, "prop_portal" ) )
		QA_Log( "portal at " + QA_Vec( p.GetOrigin() ) + " angles " + QA_Vec( p.GetAngles() ) )
	local eye = Vector( 7960, -5700, 64 )
	local angles = QA_AnglesTo( eye, Vector( 8000, -5504, 56 ) )
	QA_PlaceEye( eye, angles.pitch, angles.yaw )
}, 1.5 )
QA_Do( "shot tunnel", function() { QA_Shot( "portal_recursion" ) }, 1.0 )

// 10: the player model itself (seen through portals above).
QA_Do( "third person", function()
{
	QA_PlaceEye( Vector( 7700, -5760, 64 ), 10.0, 0.0 )
	SendToConsole( "thirdperson" )
}, 2.0 )
QA_Do( "shot player", function() { QA_Shot( "player_model" ) }, 1.0 )
QA_Do( "first person", function() { SendToConsole( "firstperson" ) }, 0.5 )

QA_Start( "sp_a2_triple_laser" )
