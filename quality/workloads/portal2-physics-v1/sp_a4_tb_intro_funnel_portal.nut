// An excursion funnel through a portal pair in sp_a4_tb_intro. The emitter
// at (1664 512 -512) projects straight up. Portal A hangs in the beam 240
// units up, facing down, so the beam enters it; portal B hangs 600 units
// up, facing the most open horizontal direction, so the beam comes out of
// B sideways. A player dropped into the beam below A must be carried into
// A, come out of B and ride the second segment away from it.

IncludeScript( "qa/qa_driver" )
IncludeScript( "qa/qa_physics" )

::TP <- {
	emitter = Vector( 1664, 512, -512 )
	a = Vector( 1664, 512, -272 )
	b = Vector( 1664, 512, 88 )
	yaw = 0.0
	dir = Vector( 1, 0, 0 )
}

QA_Do( "setup", function()
{
	SendToConsole( "sv_cheats 1; developer 0; con_drawnotify 0" )
	// B faces the horizontal direction with the most room.
	local best = -1.0
	foreach ( yaw in [ 0.0, 90.0, 180.0, 270.0 ] )
	{
		local d = Vector( cos( QA_Rad( yaw ) ), sin( QA_Rad( yaw ) ), 0 )
		local room = PH_Ray( ::TP.b, d, 3000.0 )
		QA_Log( format( "yaw %.0f: %.0f units clear", yaw, room ) )
		if ( room > best )
		{
			best = room
			::TP.yaw = yaw
			::TP.dir = d
		}
	}
	PH_Metric( "b.clearance", best )
}, 1.0 )
QA_Sleep( 3.0 )
QA_Do( "place the portals", function()
{
	PH_Place( 0, ::TP.a, 90.0, 0.0 )
	PH_Place( 1, ::TP.b, 0.0, ::TP.yaw )
}, 0.5 )
QA_Expect( "portals.placed", function()
{
	PH_Activate( ::TP.a )
	PH_Activate( ::TP.b )
	return true
} )
QA_Sleep( 1.0 )
QA_Expect( "beam.through_b", function()
{
	// The projector makes one beam entity per segment.
	local n = 0, e = null
	while ( e = Entities.FindByClassname( e, "trigger_tractorbeam" ) )
	{
		n++
		QA_Log( "beam segment at " + QA_Vec( e.GetOrigin() ) + " angles " + QA_Vec( e.GetAngles() ) )
	}
	PH_Metric( "beam.segments", n )
	QA_Detail( n + " trigger_tractorbeam" )
	return n >= 2
} )
QA_Do( "into the beam", function()
{
	local p = QA_Player()
	p.SetOrigin( ::TP.emitter + Vector( 0, 0, 96 ) )
	p.SetVelocity( Vector( 0, 0, 0 ) )
	QA_SetView( 0.0, ::TP.yaw )
	PH_Track( "ride", p, 6.0, false )
}, 6.2 )
QA_Expect( "ride.through", function()
{
	local i = PH_TeleportIndex( "ride", 64.0 )
	local s = PH_Samples( "ride" )
	local last = s[s.len() - 1]
	local out = last.pos - ::TP.b
	local along = QA_Dot( out, ::TP.dir )
	// Speed along the second segment over the last second.
	local v = QA_Dot( last.vel, ::TP.dir )
	PH_Flag( "ride.ok_teleported", i >= 0 )
	PH_Metric( "ride.along_b", along )
	PH_Metric( "ride.speed_along_b", v )
	PH_Metric( "ride.drop_below_b", ::TP.b.z - last.pos.z )
	QA_Detail( "teleport tick " + i + " end " + QA_Vec( last.pos ) + " vel " + QA_Vec( last.vel ) )
	return true
} )

QA_Start( "sp_a4_tb_intro_funnel_portal" )
