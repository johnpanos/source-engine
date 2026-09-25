// sp_a2_triple_laser traversal: the player walks through a wall portal pair
// and back. Portal A sits on the laser room's placement helper and portal B on
// the entry hall's north wall; both face south, so walking north into A leaves
// B heading south and walking north into B leaves A heading south. The
// room's lasers are switched off first so they do not shove the player.
//
// Each crossing logs the player every 0.03 s (QA_LOG path lines) and screenshots
// the approach, the crossing and the exit, so the retail reference and our
// build can be compared step by step.

IncludeScript( "qa/qa_driver" )

::TT <- {
	helperA = Vector( 8000, -5504, 48 )
	pathUntil = 0.0
	pathTag = ""
	portals = {}
}

function QA_Shot( name )
{
	printl( "QA_SHOT " + ::QA.scenario + " " + name )
	SendToConsole( "screenshot" )
}

function TT_WallAhead( from, dir )
{
	local to = from + QA_Scale( dir, 2000.0 )
	return from + QA_Scale( dir, 2000.0 * TraceLine( from, to, null ) )
}

function TT_PathTick()
{
	local player = QA_Player()
	local v = player.GetVelocity()
	QA_Log( "path " + ::TT.pathTag + " origin " + QA_Vec( player.GetOrigin() ) + " eye " +
	        QA_Vec( player.EyePosition() ) + " vel " + QA_Vec( v ) + " fwd " +
	        QA_Vec( QA_Scale( player.GetForwardVector(), 100.0 ) ) )
	if ( Time() < ::TT.pathUntil )
		EntFireByHandle( ::QA.driver, "RunScriptCode", "TT_PathTick()", 0.03, null, null )
}

function TT_Path( tag, seconds )
{
	::TT.pathTag = tag
	::TT.pathUntil = Time() + seconds
	TT_PathTick()
}

function TT_Portals()
{
	local p = null
	while ( p = Entities.FindByClassname( p, "prop_portal" ) )
	{
		QA_Log( "portal " + p.GetName() + " at " + QA_Vec( p.GetOrigin() ) + " angles " + QA_Vec( p.GetAngles() ) )
		if ( p.GetOrigin().y > -5420 )
			::TT.portals.b <- p.GetOrigin()
		else
			::TT.portals.a <- p.GetOrigin()
	}
}

QA_Do( "setup", function()
{
	SendToConsole( "sv_cheats 1; con_drawnotify 0; cl_drawhud 0; developer 0" )
	local north = TT_WallAhead( Vector( 7420, -5400, 48 ), Vector( 0, 1, 0 ) )
	::TT.northWall <- north
}, 1.0 )
QA_Sleep( 8.0 )
QA_Do( "leave the elevator", function()
{
	QA_StandIn( QA_EntNear( "trigger_once", Vector( 8000, -5184, 128 ) ), -90.0 )
}, 2.0 )

// The same pair as the triple-laser solution, fired with the gun.
QA_Do( "aim at the helper wall", function()
{
	local eye = Vector( 8000, -5700, 64 )
	local angles = QA_AnglesTo( eye, ::TT.helperA )
	QA_PlaceEye( eye, angles.pitch, angles.yaw )
}, 1.0 )
QA_Do( "fire portal A", function() { QA_Press( "attack", 0.1 ) }, 1.5 )
QA_Do( "aim at the north wall", function()
{
	local target = Vector( 7420, ::TT.northWall.y, 56 )
	local eye = Vector( 7420, -5420, 64 )
	local angles = QA_AnglesTo( eye, target )
	QA_PlaceEye( eye, angles.pitch, angles.yaw )
}, 1.0 )
QA_Do( "fire portal B", function() { QA_Press( "attack2", 0.1 ) }, 1.5 )
QA_Do( "log portals", function() { TT_Portals() }, 0.1 )

// Lasers push the player aside (both builds); switch them off so the
// crossings test the portals alone.
QA_Do( "lasers off", function()
{
	local laser = null
	while ( laser = Entities.FindByClassname( laser, "env_portal_laser" ) )
		EntFireByHandle( laser, "TurnOff", "", 0.0, null, null )
}, 0.5 )
QA_Expect( "portals.placed", function()
{
	QA_Detail( "a=" + ( "a" in ::TT.portals ) + " b=" + ( "b" in ::TT.portals ) )
	return "a" in ::TT.portals && "b" in ::TT.portals
} )

// Crossing 1: walk north into A, come out of B facing south.
QA_Do( "stand before A", function()
{
	local a = ::TT.portals.a
	QA_Player().SetOrigin( Vector( a.x, a.y - 160, 0 ) )
	QA_Player().SetVelocity( Vector( 0, 0, 0 ) )
	QA_SetView( 0.0, 90.0 )
}, 1.0 )
QA_Do( "shot approach A", function() { QA_Shot( "approach_a" ) }, 0.3 )
QA_Do( "walk into A", function()
{
	TT_Path( "a_to_b", 2.5 )
	QA_Press( "forward", 1.4 )
}, 0.55 )
QA_Do( "shot entering A", function() { QA_Shot( "entering_a" ) }, 0.25 )
QA_Do( "shot crossing A", function() { QA_Shot( "crossing_a" ) }, 0.25 )
QA_Do( "shot exited B", function() { QA_Shot( "exited_b" ) }, 1.8 )
QA_Expect( "cross.a_to_b", function()
{
	local o = QA_Player().GetOrigin()
	local b = ::TT.portals.b
	QA_Detail( "player " + QA_Vec( o ) + " portal B " + QA_Vec( b ) )
	// Out of B on its south side, in the entry hall.
	return fabs( o.x - b.x ) < 64 && o.y < b.y - 16 && o.y > b.y - 260
} )
QA_Expect( "cross.a_to_b_heading", function()
{
	local f = QA_Player().GetForwardVector()
	QA_Detail( "forward " + QA_Vec( QA_Scale( f, 100.0 ) ) )
	return f.y < -0.9
} )
QA_Do( "shot after A", function() { QA_Shot( "after_a" ) }, 0.3 )

// Crossing 2: turn around where crossing 1 ended, walk north back into B and
// come out of A. (Placing the player directly in front of B starts it stuck
// in both builds.)
QA_Do( "turn around", function()
{
	QA_Player().SetVelocity( Vector( 0, 0, 0 ) )
	QA_SetView( 0.0, 90.0 )
}, 1.0 )
QA_Do( "shot approach B", function() { QA_Shot( "approach_b" ) }, 0.3 )
QA_Do( "walk into B", function()
{
	TT_Path( "b_to_a", 2.5 )
	QA_Press( "forward", 1.4 )
}, 0.55 )
QA_Do( "shot entering B", function() { QA_Shot( "entering_b" ) }, 0.25 )
QA_Do( "shot crossing B", function() { QA_Shot( "crossing_b" ) }, 0.25 )
QA_Do( "shot exited A", function() { QA_Shot( "exited_a" ) }, 1.8 )
QA_Expect( "cross.b_to_a", function()
{
	local o = QA_Player().GetOrigin()
	local a = ::TT.portals.a
	QA_Detail( "player " + QA_Vec( o ) + " portal A " + QA_Vec( a ) )
	return fabs( o.x - a.x ) < 64 && o.y < a.y - 16 && o.y > a.y - 260
} )
QA_Expect( "cross.b_to_a_heading", function()
{
	local f = QA_Player().GetForwardVector()
	QA_Detail( "forward " + QA_Vec( QA_Scale( f, 100.0 ) ) )
	return f.y < -0.9
} )

QA_Start( "sp_a2_triple_laser_traverse" )
