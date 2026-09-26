// Excursion funnel (prop_tractor_beam) in sp_a4_tb_intro: the emitter at
// (1664 512 -512) projects straight up with linearForce 250. The player is
// put into the beam off its axis and carried; the beam is then reversed;
// then a cube rides it. Measured: steady axial speed, how far off the axis
// the rider settles, and the reversed speed.

IncludeScript( "qa/qa_driver" )
IncludeScript( "qa/qa_physics" )

::TB <- {
	emitter = Vector( 1664, 512, -512 )
}

// Median axial velocity over the samples taken <from>..<to> seconds after
// the track started, and the rider's distance from the beam axis at the end.
function TB_Measure( key, prefix, from, to )
{
	local s = PH_Samples( key )
	local t0 = s[0].t
	local vz = []
	foreach ( sample in s )
	{
		if ( sample.t - t0 >= from && sample.t - t0 <= to )
			vz.append( sample.vel.z )
	}
	if ( vz.len() == 0 )
		throw "no samples in the window"
	vz.sort()
	local median = vz[vz.len() / 2]
	local last = s[s.len() - 1].pos
	local off = QA_FlatDist( last, ::TB.emitter )
	PH_Metric( prefix + ".axial_speed", median )
	PH_Metric( prefix + ".axis_offset", off )
	QA_Log( prefix + format( " axial %.1f offset %.1f", median, off ) + " at " + QA_Vec( last ) )
	return { speed = median, offset = off }
}

QA_Do( "setup", function()
{
	SendToConsole( "sv_cheats 1; developer 0; con_drawnotify 0" )
	local up = PH_Ray( ::TB.emitter + Vector( 0, 0, 32 ), Vector( 0, 0, 1 ), 4000.0 )
	QA_Log( format( "beam column clear for %.0f units", up ) )
	PH_Metric( "beam.column", up )
}, 1.0 )
QA_Sleep( 4.0 )

// 1. The player dropped into the beam 40 units off its axis.
QA_Do( "into the beam", function()
{
	local p = QA_Player()
	p.SetOrigin( ::TB.emitter + Vector( 40, 0, 96 ) )
	p.SetVelocity( Vector( 0, 0, 0 ) )
	QA_SetView( 0.0, 90.0 )
	PH_Track( "ride", p, 3.0, false )
}, 3.2 )
QA_Expect( "ride.carried", function()
{
	local r = TB_Measure( "ride", "ride", 1.0, 2.5 )
	PH_Flag( "ride.ok_carried", r.speed > 50.0 )
	QA_Detail( format( "axial %.1f offset %.1f", r.speed, r.offset ) )
	return true
} )

// 2. Reversed: the same rider starting 300 units up is carried down.
QA_Do( "reverse", function()
{
	QA_Fire( "tractorbeam_emitter", "SetLinearForce", "-250" )
}, 1.0 )
QA_Do( "into the reversed beam", function()
{
	local p = QA_Player()
	p.SetOrigin( ::TB.emitter + Vector( 0, 0, 300 ) )
	p.SetVelocity( Vector( 0, 0, 0 ) )
	PH_Track( "down", p, 2.0, false )
}, 2.2 )
QA_Expect( "down.carried", function()
{
	local r = TB_Measure( "down", "down", 0.5, 1.2 )
	PH_Flag( "down.ok_carried", r.speed < -50.0 )
	QA_Detail( format( "axial %.1f offset %.1f", r.speed, r.offset ) )
	return true
} )

// 3. A cube in the forward beam.
QA_Do( "forward again, player out", function()
{
	QA_Fire( "tractorbeam_emitter", "SetLinearForce", "250" )
	local p = QA_Player()
	p.SetOrigin( ::TB.emitter + Vector( 300, 0, 32 ) )
	p.SetVelocity( Vector( 0, 0, 0 ) )
	SendToConsole( "ent_create prop_weighted_cube targetname qa_cube" )
}, 1.0 )
QA_Do( "cube into the beam", function()
{
	local cube = QA_Ent( "qa_cube" )
	cube.SetOrigin( ::TB.emitter + Vector( 40, 0, 96 ) )
	EntFireByHandle( cube, "Wake", "", 0.0, null, null )
	PH_Track( "cube", cube, 3.0, true )
}, 3.2 )
QA_Expect( "cube.carried", function()
{
	local r = TB_Measure( "cube", "cube", 1.0, 2.5 )
	PH_Flag( "cube.ok_carried", r.speed > 50.0 )
	QA_Detail( format( "axial %.1f offset %.1f", r.speed, r.offset ) )
	return true
} )

QA_Start( "sp_a4_tb_intro_funnel" )
