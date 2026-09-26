// A hard light bridge through a portal pair in sp_a2_bridge_intro. The
// projector at (-800 -768 16) throws its bridge along +x. Portal A stands
// across the bridge 300 units out, facing the projector, its centre 32
// units above the bridge; portal B stands 324 units higher in open air,
// facing the most open horizontal direction. The bridge must stop at A and
// continue out of B 32 units below B's centre, and a player dropped on the
// continued bridge must stand on it.

IncludeScript( "qa/qa_driver" )
IncludeScript( "qa/qa_physics" )

::BP <- {
	projector = Vector( -800, -768, 16 )
	a = Vector( -500, -768, 48 )
	b = Vector( -500, -768, 372 )
	yaw = 0.0
	dir = Vector( 1, 0, 0 )
}

function BP_Walls()
{
	local walls = []
	local e = null
	while ( e = Entities.FindByClassname( e, "projected_wall_entity" ) )
		walls.append( e )
	return walls
}

QA_Do( "setup", function()
{
	SendToConsole( "sv_cheats 1; developer 0; con_drawnotify 0" )
	local best = -1.0
	foreach ( yaw in [ 0.0, 90.0, 180.0, 270.0 ] )
	{
		local d = Vector( cos( QA_Rad( yaw ) ), sin( QA_Rad( yaw ) ), 0 )
		// Room for the bridge and for a player standing on it.
		local room = PH_Ray( ::BP.b + Vector( 0, 0, -48 ), d, 3000.0 )
		local head = PH_Ray( ::BP.b + Vector( 0, 0, 24 ), d, 3000.0 )
		room = room < head ? room : head
		QA_Log( format( "yaw %.0f: %.0f units clear", yaw, room ) )
		if ( room > best )
		{
			best = room
			::BP.yaw = yaw
			::BP.dir = d
		}
	}
	PH_Metric( "b.clearance", best )
}, 1.0 )
QA_Sleep( 3.0 )
QA_Do( "place the portals", function()
{
	PH_Place( 0, ::BP.a, 0.0, 180.0 )
	PH_Place( 1, ::BP.b, 0.0, ::BP.yaw )
}, 0.5 )
QA_Expect( "portals.placed", function()
{
	PH_Activate( ::BP.a )
	PH_Activate( ::BP.b )
	return true
} )
QA_Sleep( 1.0 )
QA_Expect( "bridge.through_b", function()
{
	local walls = BP_Walls()
	local fromB = 0
	foreach ( w in walls )
	{
		QA_Log( "bridge at " + QA_Vec( w.GetOrigin() ) + " angles " + QA_Vec( w.GetAngles() ) +
		        " maxs " + QA_Vec( w.GetBoundingMaxs() ) )
		if ( QA_Dist( w.GetOrigin(), ::BP.b ) < 80.0 )
			fromB++
	}
	PH_Metric( "bridge.segments", walls.len() )
	PH_Flag( "bridge.ok_from_b", fromB == 1 )
	QA_Detail( walls.len() + " bridges, " + fromB + " from B" )
	return fromB == 1
} )
QA_Do( "drop the player on the second bridge", function()
{
	local p = QA_Player()
	p.SetOrigin( ::BP.b + QA_Scale( ::BP.dir, 160.0 ) + Vector( 0, 0, -8 ) )
	p.SetVelocity( Vector( 0, 0, 0 ) )
	QA_SetView( 0.0, ::BP.yaw )
	PH_Track( "stand", p, 2.0, false )
}, 2.2 )
QA_Expect( "player.on_second_bridge", function()
{
	local s = PH_Samples( "stand" )
	local last = s[s.len() - 1]
	PH_Metric( "stand.below_b", ::BP.b.z - last.pos.z )
	PH_Flag( "stand.ok_held", ::BP.b.z - last.pos.z < 48.0 )
	QA_Detail( "rest " + QA_Vec( last.pos ) + " vel " + QA_Vec( last.vel ) )
	return true
} )

QA_Start( "sp_a2_bridge_intro_bridge_portal" )
