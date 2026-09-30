// sp_a2_laser_intro (the relit map) as a frame-rate floor workload
// (tools/quality/frame_floor.py). The route covers what a player sees on the
// way through the chamber: the arrival elevator ride, the corridor and the
// entry door, the chamber as its wall falls and the laser switches on, the
// laser through a floor portal and out of a ceiling portal into the catcher,
// looks through both portals, the lift, the exit door and the departure
// elevator arriving. It stops before the level change (a load, not
// gameplay).
//
// The player flies (noclip) and is moved every server tick along straight
// segments at walking speed, so each frame renders a moving view, as in
// play. Noclip touches no triggers, so the map's story relays are fired
// here in the order the triggers would fire them; checks confirm each event
// happened. frame_floor.py judges every frame between the "floor_begin" and
// "floor_end" frame marks against the floor, and stops the game at the first
// frame below it.

IncludeScript( "qa/qa_driver" )
IncludeScript( "qa/floor_options" )

::FLOOR <- {
	path = []
	segment = 0
	segmentStart = 0.0
	moving = false
	shots = 0
}

// A named screenshot, only in preview runs (frame_floor.py --preview): a
// screenshot reads the frame back and would itself be a hitch.
function Floor_Shot( name )
{
	if ( !::FLOOR_OPTIONS.preview )
		return
	::FLOOR.shots++
	printl( "QA_SHOT " + ::QA.scenario + " " + format( "%02d_", ::FLOOR.shots ) + name )
	SendToConsole( "screenshot" )
}

function Floor_Mark( label )
{
	QA_Log( "frame mark " + label )
	SendToConsole( "vk_frame_mark " + label )
}

function Floor_LerpAngle( a, b, t )
{
	local d = b - a
	while ( d > 180.0 )
		d -= 360.0
	while ( d < -180.0 )
		d += 360.0
	return a + d * t
}

// One tick of the mover: the eye moves at the segment's speed from its start
// to its end, and the view turns with it.
function Floor_MoveTick()
{
	if ( !::FLOOR.moving )
		return
	local path = ::FLOOR.path
	local from = path[::FLOOR.segment]
	local to = path[::FLOOR.segment + 1]
	local length = QA_Dist( from.eye, to.eye )
	local duration = length > 1.0 ? length / to.speed : to.turn
	local t = duration > 0.0 ? ( Time() - ::FLOOR.segmentStart ) / duration : 1.0
	if ( t >= 1.0 )
	{
		QA_PlaceEye( to.eye, to.pitch, to.yaw )
		::FLOOR.segment++
		::FLOOR.segmentStart = Time()
		if ( ::FLOOR.segment + 1 >= path.len() )
		{
			::FLOOR.moving = false
			return
		}
	}
	else
	{
		local eye = from.eye + QA_Scale( to.eye - from.eye, t )
		QA_PlaceEye( eye, Floor_LerpAngle( from.pitch, to.pitch, t ),
		             Floor_LerpAngle( from.yaw, to.yaw, t ) )
	}
	EntFireByHandle( ::QA.driver, "RunScriptCode", "Floor_MoveTick()", 0.01, null, null )
}

// A waypoint: where the eye goes, the view there, and the speed to reach it
// (units per second; 150 is Portal 2's walking speed). A waypoint at the
// same place as the last one turns the view in <turn> seconds.
function Floor_Point( eye, pitch, yaw, speed = 150.0, turn = 1.0 )
{
	return { eye = eye, pitch = pitch, yaw = yaw, speed = speed, turn = turn }
}

// Flies the route from where the eye is now; <check> passes on arrival.
function Floor_Fly( check, points, timeout )
{
	QA_Do( "fly to " + check, function() : ( points )
	{
		local player = QA_Player()
		local here = Floor_Point( player.EyePosition(), ::QA.marks.rawin( "pitch" ) ? ::QA.marks.pitch : 0.0,
		                          ::QA.marks.rawin( "yaw" ) ? ::QA.marks.yaw : player.GetAngles().y )
		::FLOOR.path = [ here ]
		foreach ( point in points )
			::FLOOR.path.append( point )
		::FLOOR.segment = 0
		::FLOOR.segmentStart = Time()
		::FLOOR.moving = true
		Floor_MoveTick()
	}, 0.05 )
	QA_WaitFor( check, function()
	{
		QA_Detail( "segment " + ::FLOOR.segment + " of " + ( ::FLOOR.path.len() - 1 ) + " eye " +
		           QA_Vec( QA_Player().EyePosition() ) )
		return !::FLOOR.moving
	}, timeout )
}

// Where the laser meets the floor, and the ceiling over the catcher.
function Floor_LaserFloorHit()
{
	local emitter = QA_Ent( "laser_emitter" )
	local forward = emitter.GetForwardVector()
	local start = emitter.GetOrigin() + QA_Scale( forward, 24.0 )
	local end = start + QA_Scale( forward, 2000.0 )
	return start + QA_Scale( end - start, TraceLine( start, end, emitter ) )
}

function Floor_CeilingOverCatcher()
{
	local catcher = QA_Ent( "catcher_1" )
	local start = catcher.GetOrigin() + Vector( 0, 0, 48 )
	local end = start + Vector( 0, 0, 1200 )
	return start + QA_Scale( end - start, TraceLine( start, end, catcher ) )
}

// --- The route ---------------------------------------------------------------

QA_Do( "setup", function()
{
	QA_Watch( "door_0-testchamber_door", "OnOpen" )
	QA_Watch( "@exit_door-testchamber_door", "OnOpen" )
	QA_Watch( "catcher_1", "OnPowered" )
	QA_Watch( "@wall_left_falling", "OnTrigger" )
	QA_Watch( "laser_emitter_door", "OnFullyOpen" )
	QA_Watch( "departure_elevator-elevator_arrive", "OnTrigger" )
	SendToConsole( "god; notarget; hud_quickinfo 1" )
	Floor_Mark( "floor_begin" )
	Floor_Mark( "arrival" )
}, 0.1 )

// The arrival elevator rides down and opens: wait for the player to stop.
QA_WaitFor( "arrival.settled", function()
{
	local z = QA_Player().GetOrigin().z
	local last = ::QA.marks.rawin( "arrivalZ" ) ? ::QA.marks.arrivalZ : z + 100.0
	::QA.marks.arrivalZ <- z
	if ( fabs( z - last ) > 0.5 )
		::QA.marks.arrivalStill <- Time()
	QA_Detail( "feet z " + z )
	return Time() - ::QA.marks.arrivalStill > 3.0
}, 60.0 )

QA_Do( "arrival view", function() { Floor_Shot( "arrival" ) }, 1.0 )

QA_Do( "fly", function()
{
	SendToConsole( "noclip" )
	Floor_Mark( "corridor" )
}, 0.2 )

Floor_Fly( "corridor.reached", [
	Floor_Point( Vector( -1000, 0, -60 ), 0.0, 0.0 ),
	Floor_Point( Vector( -800, 0, -40 ), 0.0, 0.0 )
], 20.0 )

QA_Do( "open the entry door", function()
{
	QA_Fire( "door_0-door_open_relay", "Trigger" )
	Floor_Shot( "entry_door" )
}, 1.5 )

QA_WaitFor( "entry_door.opens", function()
{
	return QA_Fired( "door_0-testchamber_door", "OnOpen" ) >= 1
}, 5.0 )

QA_Do( "chamber", function() { Floor_Mark( "chamber" ) }, 0.05 )

Floor_Fly( "chamber.reached", [
	Floor_Point( Vector( -656, 0, -40 ), 0.0, 0.0 ),
	Floor_Point( Vector( -440, 0, -40 ), -10.0, 20.0 ),
	Floor_Point( Vector( -440, 0, -40 ), -25.0, 60.0, 150.0, 1.5 )
], 20.0 )

QA_Do( "the wall falls", function()
{
	QA_Fire( "start", "Trigger" )
	QA_Fire( "@wall_left_start", "Trigger" )
	Floor_Mark( "wall" )
	Floor_Shot( "chamber" )
}, 0.1 )

QA_WaitFor( "wall.falls", function()
{
	return QA_Fired( "@wall_left_falling", "OnTrigger" ) >= 1
}, 10.0 )

Floor_Fly( "stand.reached", [
	Floor_Point( Vector( -160, 96, -40 ), -30.0, 60.0 ),
	Floor_Point( Vector( -160, 96, -40 ), -20.0, 20.0, 150.0, 2.0 )
], 20.0 )

QA_WaitFor( "laser.on", function()
{
	return QA_Fired( "laser_emitter_door", "OnFullyOpen" ) >= 1
}, 20.0 )

QA_Do( "laser view", function()
{
	Floor_Mark( "laser" )
	QA_LookAt( Floor_LaserFloorHit() )
	Floor_Shot( "laser_on" )
}, 2.0 )

// Portals where a player puts them: blue on the floor where the laser lands,
// orange on the ceiling over the catcher, which sends the laser down into it.
QA_Do( "place the portals", function()
{
	Floor_Mark( "portals" )
	local floor = Floor_LaserFloorHit()
	local ceiling = Floor_CeilingOverCatcher()
	::QA.marks.floorPortal <- floor
	::QA.marks.ceilingPortal <- ceiling
	QA_Log( "floor portal " + QA_Vec( floor ) + " ceiling portal " + QA_Vec( ceiling ) )
	SendToConsole( format( "portal_place 0 0 %.1f %.1f %.1f -90 0 0", floor.x, floor.y, floor.z + 1.0 ) )
	SendToConsole( format( "portal_place 0 1 %.1f %.1f %.1f 90 0 0", ceiling.x, ceiling.y, ceiling.z - 1.0 ) )
}, 1.0 )

QA_WaitFor( "catcher.powered", function()
{
	return QA_Fired( "catcher_1", "OnPowered" ) >= 1
}, 10.0 )

// Look down into the floor portal (the view through it looks down from the
// ceiling), then up at the ceiling portal, then walk around the laser.
QA_Do( "portal views", function()
{
	Floor_Mark( "portal_views" )
	local floor = ::QA.marks.floorPortal
	::QA.marks.overFloor <- Vector( floor.x - 120.0, floor.y, -40.0 )
	Floor_Shot( "catcher_powered" )
}, 0.1 )

Floor_Fly( "floor_portal.viewed", [
	Floor_Point( Vector( -40, 180, -30 ), 45.0, -60.0 ),
	Floor_Point( Vector( -40, 180, -30 ), 70.0, -90.0, 150.0, 1.5 ),
	Floor_Point( Vector( 60, -20, -40 ), -60.0, -90.0 ),
	Floor_Point( Vector( 60, -20, -40 ), -80.0, 90.0, 150.0, 1.5 ),
	Floor_Point( Vector( 160, -200, -40 ), 0.0, 135.0 ),
	Floor_Point( Vector( 160, -200, -40 ), 10.0, 250.0, 150.0, 2.0 )
], 30.0 )

QA_Do( "floor portal view", function() { Floor_Shot( "portal_views" ) }, 0.1 )

// The lift the catcher raises, and the exit door it opens.
QA_Do( "exit", function()
{
	Floor_Mark( "exit" )
	QA_Fire( "@exit_door-door_open_relay", "Trigger" )
}, 0.1 )

Floor_Fly( "lift.reached", [
	Floor_Point( Vector( 320, 0, -40 ), 0.0, 0.0 ),
	Floor_Point( Vector( 320, 0, 40 ), 0.0, 180.0, 100.0, 1.5 ),
	Floor_Point( Vector( 320, 0, 40 ), 10.0, 0.0, 150.0, 1.5 )
], 30.0 )

QA_WaitFor( "exit_door.opens", function()
{
	return QA_Fired( "@exit_door-testchamber_door", "OnOpen" ) >= 1
}, 10.0 )

QA_Do( "departure", function()
{
	Floor_Mark( "departure" )
	QA_Fire( "@summon_elevator", "Trigger" )
	Floor_Shot( "exit_door" )
}, 0.1 )

Floor_Fly( "departure.reached", [
	Floor_Point( Vector( 576, 0, 24 ), 0.0, 0.0 ),
	Floor_Point( Vector( 900, 0, 0 ), 0.0, 0.0 ),
	Floor_Point( Vector( 900, 0, 0 ), 0.0, 180.0, 150.0, 2.0 ),
	Floor_Point( Vector( 900, 0, 0 ), 0.0, 0.0, 150.0, 2.0 )
], 30.0 )

QA_WaitFor( "departure_elevator.arrives", function()
{
	return QA_Fired( "departure_elevator-elevator_arrive", "OnTrigger" ) >= 1
}, 20.0 )

QA_Do( "departure elevator", function()
{
	Floor_Mark( "elevator" )
	Floor_Shot( "departure_elevator" )
}, 4.0 )

QA_Do( "end", function() { Floor_Mark( "floor_end" ) }, 0.5 )

QA_Start( "sp_a2_laser_intro_tour" )
