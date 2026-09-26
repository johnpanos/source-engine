// Player movement in sp_a2_triple_laser's laser room (floor z=1, ceiling
// z=255, x 7457..8319): how fast the player walks and strafes with the
// movement keys held, how high a standing jump rises, and a carried cube
// taken through a wall portal pair. Momentum through portals depends on
// all of these, so they are measured first.

IncludeScript( "qa/qa_driver" )
IncludeScript( "qa/qa_physics" )

::MV <- {
	start = Vector( 7520, -5900, 1 )
	// A on the laser room's north wall, B on the entry hall's north wall;
	// both face south (the positions sp_a2_triple_laser_traverse fires at).
	a = Vector( 8000, -5505, 56 )
	b = Vector( 7424, -5313, 56 )
}

function MV_Stand( pos, yaw )
{
	local p = QA_Player()
	p.SetOrigin( pos )
	p.SetVelocity( Vector( 0, 0, 0 ) )
	QA_SetView( 0.0, yaw )
}

// Peak horizontal speed of a track and the distance covered.
function MV_Walk( key, prefix )
{
	local s = PH_Samples( key )
	local peak = 0.0
	foreach ( sample in s )
	{
		local v = sample.vel
		local flat = sqrt( v.x * v.x + v.y * v.y )
		if ( flat > peak )
			peak = flat
	}
	local d = s[s.len() - 1].pos - s[0].pos
	PH_Metric( prefix + ".peak_speed", peak )
	PH_Metric( prefix + ".distance", sqrt( d.x * d.x + d.y * d.y ) )
	return peak
}

QA_Do( "setup", function()
{
	SendToConsole( "sv_cheats 1; developer 0; con_drawnotify 0" )
	local laser = null
	while ( laser = Entities.FindByClassname( laser, "env_portal_laser" ) )
		EntFireByHandle( laser, "TurnOff", "", 0.0, null, null )
}, 1.0 )
QA_Sleep( 6.0 )
QA_Do( "leave the elevator", function()
{
	QA_StandIn( QA_EntNear( "trigger_once", Vector( 8000, -5184, 128 ) ), -90.0 )
}, 2.0 )

// Walking: +forward for 1.5 s from rest, facing +x down the room.
QA_Do( "stand at the west end", function() { MV_Stand( ::MV.start, 0.0 ) }, 1.0 )
QA_Do( "walk east", function()
{
	PH_Track( "walk", QA_Player(), 2.0, false )
	QA_Press( "forward", 1.5 )
}, 2.2 )
QA_Expect( "walk.moved", function()
{
	local peak = MV_Walk( "walk", "walk" )
	QA_Detail( format( "peak %.1f u/s", peak ) )
	return peak > 50.0
} )

// Strafing: +moveright for 1.5 s.
QA_Do( "stand again", function() { MV_Stand( ::MV.start, 90.0 ) }, 1.0 )
QA_Do( "strafe", function()
{
	PH_Track( "strafe", QA_Player(), 2.0, false )
	QA_Press( "moveright", 1.5 )
}, 2.2 )
QA_Expect( "strafe.moved", function()
{
	local peak = MV_Walk( "strafe", "strafe" )
	QA_Detail( format( "peak %.1f u/s", peak ) )
	return peak > 50.0
} )

// A standing jump: the rise of the origin and the time in the air.
QA_Do( "stand for the jump", function() { MV_Stand( ::MV.start, 0.0 ) }, 1.0 )
QA_Do( "jump", function()
{
	PH_Track( "jump", QA_Player(), 1.5, false )
	QA_Press( "jump", 0.1 )
}, 1.7 )
QA_Expect( "jump.rose", function()
{
	local s = PH_Samples( "jump" )
	local base = s[0].pos.z, apex = base, first = -1.0, last = -1.0
	foreach ( sample in s )
	{
		if ( sample.pos.z > apex )
			apex = sample.pos.z
		if ( sample.pos.z > base + 0.5 )
		{
			if ( first < 0.0 )
				first = sample.t
			last = sample.t
		}
	}
	PH_Metric( "jump.rise", apex - base )
	PH_Metric( "jump.airtime", last - first )
	QA_Detail( format( "rise %.1f airtime %.2f", apex - base, last - first ) )
	return apex - base > 10.0
} )

// Carry: pick the room's cube up, walk north into A and out of B.
QA_Do( "place the portals", function()
{
	PH_Place( 0, ::MV.a, 0.0, -90.0 )
	PH_Place( 1, ::MV.b, 0.0, -90.0 )
}, 0.5 )
QA_Expect( "carry.portals", function()
{
	PH_Activate( ::MV.a )
	PH_Activate( ::MV.b )
	return true
} )
QA_Do( "bring the cube", function()
{
	local cube = Entities.FindByClassname( null, "prop_weighted_cube" )
	if ( cube == null )
		throw "no prop_weighted_cube"
	::MV.cube <- cube
	QA_WatchEntity( cube, "cube", "OnPlayerPickup" )
	cube.SetOrigin( ::MV.a + Vector( 0, -220, -20 ) )
	cube.SetAngles( 0, 0, 0 )
	EntFireByHandle( cube, "Wake", "", 0.0, null, null )
	MV_Stand( ::MV.a + Vector( 0, -300, -55 ), 90.0 )
}, 1.5 )
QA_WaitFor( "carry.picked_up", function()
{
	local cube = ::MV.cube
	if ( QA_Fired( "cube", "OnPlayerPickup" ) > 0 )
		return true
	if ( !( "nextUse" in ::QA.marks ) || Time() >= ::QA.marks.nextUse )
	{
		QA_Approach( cube.GetCenter(), QA_Player().EyePosition(), 64.0 )
		QA_Press( "use", 0.1 )
		::QA.marks.nextUse <- Time() + 1.0
	}
	QA_Detail( "cube at " + QA_Vec( cube.GetCenter() ) )
	return false
}, 6.0 )
QA_Do( "face A", function()
{
	local p = QA_Player()
	local o = p.GetOrigin()
	p.SetOrigin( Vector( ::MV.a.x, ::MV.a.y - 200, o.z ) )
	p.SetVelocity( Vector( 0, 0, 0 ) )
	QA_SetView( 0.0, 90.0 )
}, 1.0 )
QA_Do( "walk through A", function()
{
	PH_Track( "carrier", QA_Player(), 3.0, false )
	PH_Track( "carried", ::MV.cube, 3.0, true )
	QA_Press( "forward", 1.6 )
}, 3.2 )
QA_Expect( "carry.player_crossed", function()
{
	local o = QA_Player().GetOrigin()
	PH_Flag( "carry.player_crossed", o.y < ::MV.b.y - 8.0 && fabs( o.x - ::MV.b.x ) < 96.0 )
	QA_Detail( "player " + QA_Vec( o ) )
	return true
} )
QA_Expect( "carry.cube_followed", function()
{
	local cube = ::MV.cube
	local p = QA_Player()
	local c = cube.GetCenter()
	local gap = QA_Dist( c, p.EyePosition() )
	local sameSide = c.y < ::MV.b.y && fabs( c.x - ::MV.b.x ) < 128.0
	PH_Metric( "carry.cube_eye_distance", gap )
	PH_Flag( "carry.cube_same_side", sameSide )
	QA_Detail( "cube " + QA_Vec( c ) + " eye " + QA_Vec( p.EyePosition() ) + format( " gap %.1f", gap ) )
	return true
} )
QA_Expect( "carry.still_held", function()
{
	// Dropped objects fall; a held one stays near eye height.
	local c = ::MV.cube.GetCenter()
	local held = c.z > 24.0
	PH_Flag( "carry.still_held", held )
	QA_Detail( "cube " + QA_Vec( c ) )
	return true
} )

QA_Start( "sp_a2_triple_laser_move" )
