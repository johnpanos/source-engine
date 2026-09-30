// sp_a2_laser_intro played through and solved on the player's own input
// (qa_walker.nut): every walk is planned in the running game and driven with
// keys and turn rates, and both portals are fired with +attack and +attack2
// at points the player looks at. Nothing moves or turns the player, and
// nothing places a portal, except input and the map itself. A screenshot is
// taken at each story beat (QA_SHOT); the harness pairs the shots of this
// build (on the relit map) and retail Portal 2 (on the shipped map) by name.
//
// Beats: arrival in the elevator, the entry door, the chamber, the wall
// falling in and the laser switching on, the floor portal where the laser
// lands, the ceiling portal over the catcher, the catcher powered, the lift
// rising, the exit door opening, the departure elevator, its doors closing
// and the level's end as the elevator leaves (about 3 s later).

IncludeScript( "qa/qa_driver" )
IncludeScript( "qa/qa_walker" )
IncludeScript( "qa/qa_clips" )
IncludeScript( "qa/qa_beats" )

// Where the laser lands on the floor, and the ceiling over the catcher.
function Laser_FloorHit()
{
	local emitter = QA_Ent( "laser_emitter" )
	local forward = emitter.GetForwardVector()
	local start = emitter.GetOrigin() + QA_Scale( forward, 24.0 )
	local end = start + QA_Scale( forward, 2000.0 )
	local hit = start + QA_Scale( end - start, TraceLine( start, end, emitter ) )
	QA_Log( "laser from " + QA_Vec( start ) + " lands at " + QA_Vec( hit ) )
	return hit
}

function Laser_CeilingOverCatcher()
{
	local catcher = QA_Ent( "catcher_1" )
	local start = catcher.GetOrigin() + Vector( 0, 0, 48 )
	local end = start + Vector( 0, 0, 1200 )
	local hit = start + QA_Scale( end - start, TraceLine( start, end, catcher ) )
	QA_Log( "ceiling over the catcher at " + QA_Vec( hit ) )
	return hit
}

function Laser_PortalNear( point, radius )
{
	local portal = null
	local seen = ""
	while ( ( portal = Entities.FindByClassname( portal, "prop_portal" ) ) != null )
	{
		seen += " " + QA_Vec( portal.GetOrigin() )
		if ( QA_Dist( portal.GetOrigin(), point ) < radius )
		{
			QA_Detail( "portal at " + QA_Vec( portal.GetOrigin() ) + " for " + QA_Vec( point ) )
			return portal
		}
	}
	QA_Detail( "no portal within " + radius + " of " + QA_Vec( point ) + "; portals:" + seen )
	return null
}

// Aim at <point> and fire (+attack blue, +attack2 orange); <check>.placed
// passes once a portal stands where the player aimed.
function Laser_Fire( check, button, point )
{
	Beat_Look( "aim_" + check, point )
	QA_Do( "fire " + check, function() : ( button, point )
	{
		local aim = typeof point == "function" ? point() : point
		local eye = QA_Player().EyePosition()
		local fraction = TraceLine( eye, aim + QA_Scale( aim - eye, 0.05 ), QA_Player() )
		QA_Log( "fire " + button + " from " + QA_Vec( eye ) + " at " + QA_Vec( aim ) +
		        format( " trace %.3f of %.1f", fraction, QA_Dist( eye, aim ) * 1.05 ) )
		local volume = null
		while ( ( volume = Entities.FindByClassname( volume, "func_noportal_volume" ) ) != null )
			QA_Log( "  noportal " + volume.GetName() + " " + QA_Vec( volume.GetCenter() ) )
		Beat_Press( button, 0.2 )
	}, 0.8 )
	QA_WaitFor( check + ".placed", function() : ( point )
	{
		return Laser_PortalNear( typeof point == "function" ? point() : point, 48.0 ) != null
	}, 3.0 )
}

// Both builds start the walk at the same map time (the build's driver starts
// after a frame count, retail's a second after the map spawns).
QA_WaitFor( "start.map_time", function()
{
	QA_Detail( format( "t=%.2f", Time() ) )
	return Time() >= 6.0
}, 30.0 )

QA_Do( "setup", function()
{
	SendToConsole( "con_drawnotify 0; cl_drawhud 0; hud_quickinfo 0; developer 0" )
	Walk_Init( null )
	QA_Watch( "door_0-testchamber_door", "OnOpen" )
	QA_Watch( "@exit_door-testchamber_door", "OnOpen" )
	QA_Watch( "catcher_1", "OnPowered" )
	QA_Watch( "catcher_1", "OnUnpowered" )
	Beat_Mark( "start", "OnTrigger" )
	Beat_Mark( "@wall_left_falling", "OnTrigger" )
	Beat_Mark( "laser_emitter_door", "OnFullyOpen" )
	Beat_Mark( "catcher_1", "OnPowered" )
	Beat_Mark( "lift_a", "OnFullyClosed" )
	Beat_Mark( "departure_elevator-elevator_arrive", "OnTrigger" )
	Beat_Mark( "departure_elevator-close", "OnTrigger" )
	Beat_Mark( "@transition_from_map", "OnTrigger" )
}, 0.5 )

// 1. The arrival elevator has opened onto the stairs up to the chamber.
Beat_ShotDir( "arrival", 0.0, 0.0 )

// 2. Up the stairs to the entry door, which opens as the player comes.
// The door's trigger spans x -832 to -800.
Beat_Walk( "entry.reached", Vector( -790, 0, -100 ), 24.0, 60.0 )
QA_WaitFor( "entry_door.opens", function()
{
	return QA_Fired( "door_0-testchamber_door", "OnOpen" ) >= 1
}, 10.0 )
QA_Sleep( 1.0 )
Beat_ShotAt( "entry_door", Vector( -656, 0, -64 ) )

// 3. Into the chamber: stepping in starts it (the wall falls in, the
// catcher and the laser emitter come out).
Beat_Walk( "chamber.reached", Vector( -440, 0, -100 ), 40.0, 60.0 )
QA_Expect( "chamber.start", function() { return Beat_Fired( "start:OnTrigger" ) } )
Beat_ShotDir( "chamber", 0.0, 0.0 )
Beat_EventShot( "wall_falls", "@wall_left_falling:OnTrigger", 1.0, Vector( 176, 256, -40 ) )

// 4. The laser switches on and lands on the floor.
Beat_Walk( "stand.reached", Vector( -160, 96, -100 ), 32.0, 60.0 )
Beat_EventShot( "laser_on", "laser_emitter_door:OnFullyOpen", 2.0, Laser_FloorHit )

// 5. The puzzle: a portal where the laser lands, then, standing on the
// lowered lift, one in the ceiling over the catcher, so the beam comes down
// into it and the powered catcher raises the lift to the exit.
Laser_Fire( "portal_floor", "attack", Laser_FloorHit )
QA_Do( "shoot portal_floor", function() { Beat_Shot( "portal_floor" ) }, 1.0 )
Beat_ShotAt( "lift", Vector( 320, 0, -140 ) )
Beat_Walk( "lift.reached", Vector( 320, 0, -100 ), 24.0, 60.0 )
Laser_Fire( "portal_ceiling", "attack2", Laser_CeilingOverCatcher )
QA_WaitFor( "catcher.powered", function()
{
	return QA_Fired( "catcher_1", "OnPowered" ) >= 1
}, 5.0 )
Beat_ShotAt( "portal_ceiling", Laser_CeilingOverCatcher )
Beat_EventShot( "catcher_powered", "catcher_1:OnPowered", 1.5, function()
{
	return QA_Ent( "catcher_1" ).GetCenter()
} )

// 6. The lift rises to the exit; at the top the exit door opens.
Beat_WaitEvent( "lift_raised", "lift_a:OnFullyClosed", 10.0 )
QA_WaitFor( "exit_door.opens", function()
{
	return QA_Fired( "@exit_door-testchamber_door", "OnOpen" ) >= 1
}, 10.0 )
QA_Expect( "catcher.still_powered", function()
{
	return QA_Fired( "catcher_1", "OnUnpowered" ) == 0
} )
QA_Sleep( 1.0 )
Beat_ShotAt( "exit_door", Vector( 576, 0, 16 ) )

// 7. Through the exit to the departure elevator, which arrives and opens
// (this map disables the usual door-open relay at spawn: the elevator is
// "blocked" and comes up when the player nears it).
Beat_Walk( "departure.reached", Vector( 760, 0, 0 ), 40.0, 60.0 )
Beat_EventShot( "departure_elevator", "departure_elevator-elevator_arrive:OnTrigger", 2.5, [ 0.0, 0.0 ] )
// The elevator's player clip goes 3.5 s after it arrives.
QA_Sleep( 1.5 )

// 8. Into the elevator: its doors close and it leaves; the level ends.
// The car's threshold is the elevator model, which TraceLine does not see:
// walk to it on the plan, then straight in.
Beat_Walk( "elevator_door.reached", Vector( 1100, 0, -150 ), 24.0, 60.0 )
Beat_WalkStraight( "elevator.reached", Vector( 1232, 0, -190 ), 12.0, 20.0 )
Beat_EventShot( "elevator_doors", "departure_elevator-close:OnTrigger", 1.0, [ 0.0, 180.0 ] )
// The level change can follow at once and reset the VM: shoot and finish
// together.
QA_WaitFor( "level.ends", function()
{
	return Beat_Fired( "@transition_from_map:OnTrigger" )
}, 90.0 )
QA_Do( "shoot leaving", function() { Beat_Shot( "leaving" ) }, 0.01 )

QA_Start( "sp_a2_laser_intro_walk" )
