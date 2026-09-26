// Portal edges in sp_a2_triple_laser's laser room (floor z=1, x
// 7457..8319, y -6015..-5505). What gets through a portal near its rim,
// and what hits the wall or floor beside it:
//
//   walk_<dx>   the player walks north into A on the north wall, dx units
//               right of its centre;
//   drop_<axis><d>  a cube dropped into A on the floor, d units off its
//               centre along x or y (a floor portal is 64 wide and 112
//               long; which axis is which follows the portal's roll);
//   funnel_<d>  the player dropped 180 units into A on the floor, d units
//               off its centre (retail steers falling players into floor
//               portals, sv_player_funnel_into_portals).
//
// B stays on the east wall facing west; crossing A means coming out of B.

IncludeScript( "qa/qa_driver" )
IncludeScript( "qa/qa_physics" )

::ED <- {
	north = Vector( 8000, -5505, 56 )
	floorA = Vector( 7600, -5760, 1 )
	wallB = Vector( 8319, -5760, 64 )
}

// True when the sampled entity came out of B: it teleported and ended on
// B's side of the room.
function ED_Crossed( key )
{
	local i = PH_TeleportIndex( key, 64.0 )
	if ( i < 0 )
		return false
	local s = PH_Samples( key )
	return QA_Dist( s[i].pos, ::ED.wallB ) < 96.0
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
QA_Do( "place A on the north wall, B on the east wall", function()
{
	PH_Place( 0, ::ED.north, 0.0, -90.0 )
	PH_Place( 1, ::ED.wallB, 0.0, 180.0 )
}, 0.5 )
QA_Expect( "portals.placed", function()
{
	PH_Activate( ::ED.north )
	PH_Activate( ::ED.wallB )
	return true
} )

// Walking into the wall portal off centre.
foreach ( dx in [ 0, 12, 20, 28, 40 ] )
{
	QA_Do( "stand before A at " + dx, function() : ( dx )
	{
		local p = QA_Player()
		p.SetOrigin( Vector( ::ED.north.x + dx, ::ED.north.y - 140, 1 ) )
		p.SetVelocity( Vector( 0, 0, 0 ) )
		QA_SetView( 0.0, 90.0 )
	}, 0.6 )
	QA_Do( "walk into A at " + dx, function() : ( dx )
	{
		PH_Track( "walk" + dx, QA_Player(), 1.8, false )
		QA_Press( "forward", 1.4 )
	}, 2.0 )
	QA_Expect( "walk_" + dx + ".done", function() : ( dx )
	{
		local crossed = ED_Crossed( "walk" + dx )
		local s = PH_Samples( "walk" + dx )
		local last = s[s.len() - 1].pos
		PH_Flag( "walk_" + dx + ".crossed", crossed )
		QA_Detail( ( crossed ? "crossed" : "blocked" ) + " at " + QA_Vec( last ) )
		return true
	} )
}

// A on the floor for the drops.
QA_Do( "move A to the floor", function()
{
	PH_Place( 0, ::ED.floorA, -90.0, 0.0 )
	local p = QA_Player()
	p.SetOrigin( Vector( 7700, -5950, 1 ) )
	p.SetVelocity( Vector( 0, 0, 0 ) )
}, 0.5 )
QA_Do( "open A", function() { PH_Activate( ::ED.floorA ) }, 0.5 )

foreach ( drop in [ [ "x", 0 ], [ "x", 20 ], [ "x", 30 ], [ "x", 44 ], [ "y", 20 ], [ "y", 44 ], [ "y", 64 ] ] )
{
	local tag = "drop_" + drop[0] + drop[1]
	local offset = drop[0] == "x" ? Vector( drop[1], 0, 0 ) : Vector( 0, drop[1], 0 )
	QA_Do( tag, function() : ( tag, offset )
	{
		local cube = Entities.FindByClassname( null, "prop_weighted_cube" )
		if ( cube == null )
			throw "no prop_weighted_cube"
		cube.SetOrigin( ::ED.floorA + offset + Vector( 0, 0, 120 ) )
		cube.SetAngles( 0, 0, 0 )
		EntFireByHandle( cube, "Wake", "", 0.0, null, null )
		PH_Track( tag, cube, 2.0, true )
	}, 2.2 )
	QA_Expect( tag + ".done", function() : ( tag )
	{
		local crossed = ED_Crossed( tag )
		local s = PH_Samples( tag )
		PH_Flag( tag + ".crossed", crossed )
		QA_Detail( ( crossed ? "crossed" : "stopped" ) + " at " + QA_Vec( s[s.len() - 1].pos ) )
		return true
	} )
}

// The player falling next to the floor portal.
foreach ( d in [ 24, 48, 72 ] )
{
	local tag = "funnel_" + d
	QA_Do( tag, function() : ( tag, d )
	{
		local p = QA_Player()
		p.SetOrigin( ::ED.floorA + Vector( d, 0, 180 ) )
		p.SetVelocity( Vector( 0, 0, 0 ) )
		QA_SetView( 0.0, 90.0 )
		PH_Track( tag, p, 2.0, false )
	}, 2.2 )
	QA_Expect( tag + ".done", function() : ( tag )
	{
		local crossed = ED_Crossed( tag )
		local s = PH_Samples( tag )
		PH_Flag( tag + ".crossed", crossed )
		QA_Detail( ( crossed ? "crossed" : "missed" ) + " at " + QA_Vec( s[s.len() - 1].pos ) )
		return true
	} )
}

QA_Start( "sp_a2_triple_laser_edges" )
