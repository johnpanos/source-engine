// Momentum through portals in sp_a2_triple_laser's laser room (floor z=1,
// ceiling z=255, x 7457..8319, y -6015..-5505). The fixture places its own
// portal pair with NewLocation: A on the floor, B on the east wall facing
// west, then on the floor facing up. It drops the player and a cube into A
// and measures each crossing tick by tick:
//
//   fling_*   player floor -> wall: speed in, speed out, exit direction and
//             where the player lands;
//   bounce    player floor -> floor: the exit rise;
//   cube_*    weighted cube floor -> wall: speed in and out.
//
// Speed conservation is retail's rule ("speedy thing goes in, speedy thing
// comes out"); tools/quality/portal2_physics.py compares the QA_METRIC
// values with the recorded retail run.

IncludeScript( "qa/qa_driver" )
IncludeScript( "qa/qa_physics" )

::FL <- {
	a = Vector( 7600, -5760, 1 )
	wall = Vector( 8319, -5760, 64 )
	floorB = Vector( 8100, -5760, 1 )
}

function FL_Report( key, prefix, portalOut, normalOut )
{
	local s = PH_Samples( key )
	local i = PH_TeleportIndex( key, 64.0 )
	if ( i < 2 )
	{
		PH_Dump( key )
		throw "no teleport in " + key
	}
	local before = s[i - 1].vel.Length()
	// Velocity leaving B: the first sample after the teleport tick that is
	// clear of the portal (the teleport tick may still carry the old velocity).
	local after = s[i].vel
	if ( i + 1 < s.len() )
		after = s[i + 1].vel
	local speedOut = after.Length()
	local dir = speedOut > 0.0 ? QA_Scale( after, 1.0 / speedOut ) : Vector( 0, 0, 0 )
	PH_Metric( prefix + ".speed_in", before )
	PH_Metric( prefix + ".speed_out", speedOut )
	PH_Metric( prefix + ".exit_along_normal", QA_Dot( dir, normalOut ) )
	PH_Metric( prefix + ".exit_offset", QA_Dist( s[i].pos, portalOut ) )
	QA_Log( prefix + format( " in %.1f out %.1f", before, speedOut ) + " exit " + QA_Vec( s[i].pos ) +
	        " vel " + QA_Vec( after ) )
	return { index = i, speedIn = before, speedOut = speedOut }
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
QA_Do( "place A on the floor, B on the east wall", function()
{
	PH_Place( 0, ::FL.a, -90.0, 0.0 )
	PH_Place( 1, ::FL.wall, 0.0, 180.0 )
}, 0.5 )
QA_Expect( "portals.placed", function()
{
	local a = PH_Activate( ::FL.a ), b = PH_Activate( ::FL.wall )
	QA_Detail( "A " + QA_Vec( a.GetOrigin() ) + " B " + QA_Vec( b.GetOrigin() ) )
	return true
} )

// 1. Free fall from 180 units above A into the east wall.
QA_Do( "drop the player over A", function()
{
	local p = QA_Player()
	p.SetOrigin( ::FL.a + Vector( 0, 0, 180 ) )
	p.SetVelocity( Vector( 0, 0, 0 ) )
	QA_SetView( 0.0, 90.0 )
	PH_Track( "drop", p, 2.5, false )
}, 2.6 )
QA_Expect( "fling.crossed", function()
{
	local r = FL_Report( "drop", "fling", ::FL.wall, Vector( -1, 0, 0 ) )
	::FL.drop <- r
	QA_Detail( format( "in %.1f out %.1f", r.speedIn, r.speedOut ) )
	return true
} )
// The server tick: the median spacing of the per-tick samples.
QA_Expect( "engine.tick", function()
{
	local s = PH_Samples( "drop" )
	local d = []
	for ( local j = 1; j < s.len(); j++ )
		d.append( s[j].t - s[j - 1].t )
	d.sort()
	local tick = d[d.len() / 2]
	PH_Metric( "engine.sample_interval", tick )
	PH_Metric( "engine.frame_time", FrameTime() )
	QA_Detail( format( "samples every %.5f s (%d), FrameTime %.5f", tick, s.len(), FrameTime() ) )
	return true
} )
QA_Expect( "fling.landing", function()
{
	local s = PH_Samples( "drop" )
	local last = s[s.len() - 1]
	PH_Metric( "fling.landing_x", last.pos.x )
	PH_Metric( "fling.landing_dy", last.pos.y - ::FL.wall.y )
	PH_Metric( "fling.landing_z", last.pos.z )
	QA_Detail( "rest " + QA_Vec( last.pos ) + " vel " + QA_Vec( last.vel ) )
	return last.pos.x < ::FL.wall.x - 16.0
} )

// 2. The same drop with 900 u/s of downward speed: the speed must survive.
QA_Do( "throw the player down into A", function()
{
	local p = QA_Player()
	p.SetOrigin( ::FL.a + Vector( 0, 0, 180 ) )
	p.SetVelocity( Vector( 0, 0, -900 ) )
	QA_SetView( 0.0, 90.0 )
	PH_Track( "throw", p, 2.5, false )
}, 2.6 )
QA_Expect( "fast.crossed", function()
{
	local r = FL_Report( "throw", "fast", ::FL.wall, Vector( -1, 0, 0 ) )
	QA_Detail( format( "in %.1f out %.1f", r.speedIn, r.speedOut ) )
	return true
} )
QA_Expect( "fast.landing", function()
{
	local s = PH_Samples( "throw" )
	local last = s[s.len() - 1]
	PH_Metric( "fast.landing_x", last.pos.x )
	QA_Detail( "rest " + QA_Vec( last.pos ) )
	return true
} )

// 3. Floor to floor: B moves to the floor; a 100-unit drop into A comes
// back up out of B.
QA_Do( "move B to the floor", function() { PH_Place( 1, ::FL.floorB, -90.0, 0.0 ) }, 0.5 )
QA_Do( "open B", function() { PH_Activate( ::FL.floorB ) }, 0.5 )
QA_Do( "drop the player over A again", function()
{
	local p = QA_Player()
	p.SetOrigin( ::FL.a + Vector( 0, 0, 100 ) )
	p.SetVelocity( Vector( 0, 0, 0 ) )
	QA_SetView( 0.0, 90.0 )
	PH_Track( "bounce", p, 2.5, false )
}, 2.6 )
QA_Expect( "bounce.crossed", function()
{
	local r = FL_Report( "bounce", "bounce", ::FL.floorB, Vector( 0, 0, 1 ) )
	local s = PH_Samples( "bounce" )
	local apex = -100000.0
	for ( local j = r.index; j < s.len(); j++ )
	{
		if ( s[j].pos.z > apex )
			apex = s[j].pos.z
	}
	PH_Metric( "bounce.rise", apex - ::FL.floorB.z )
	QA_Detail( format( "in %.1f out %.1f rise %.1f", r.speedIn, r.speedOut, apex - ::FL.floorB.z ) )
	return true
} )

// 4. A weighted cube dropped into A with B back on the wall.
QA_Do( "move B back to the wall", function()
{
	PH_Place( 1, ::FL.wall, 0.0, 180.0 )
	local p = QA_Player()
	p.SetOrigin( Vector( 7700, -5900, 1 ) )
	p.SetVelocity( Vector( 0, 0, 0 ) )
}, 1.0 )
QA_Do( "open B again", function() { PH_Activate( ::FL.wall ) }, 0.5 )
QA_Do( "drop the cube into A", function()
{
	local cube = Entities.FindByClassname( null, "prop_weighted_cube" )
	if ( cube == null )
		throw "no prop_weighted_cube"
	cube.SetOrigin( ::FL.a + Vector( 0, 0, 180 ) )
	cube.SetAngles( 0, 0, 0 )
	// The cube has slept since the map started; a moved sleeping IVP body
	// stays where it is put until something wakes it.
	EntFireByHandle( cube, "Wake", "", 0.0, null, null )
	PH_Track( "cube", cube, 3.0, true )
}, 3.1 )
QA_Expect( "cube.crossed", function()
{
	local r = FL_Report( "cube", "cube", ::FL.wall, Vector( -1, 0, 0 ) )
	QA_Detail( format( "in %.1f out %.1f", r.speedIn, r.speedOut ) )
	return true
} )
QA_Expect( "cube.landing", function()
{
	local s = PH_Samples( "cube" )
	local last = s[s.len() - 1]
	PH_Metric( "cube.landing_x", last.pos.x )
	PH_Metric( "cube.landing_z", last.pos.z )
	QA_Detail( "rest " + QA_Vec( last.pos ) )
	return last.pos.x < ::FL.wall.x - 16.0
} )

QA_Start( "sp_a2_triple_laser_fling" )
