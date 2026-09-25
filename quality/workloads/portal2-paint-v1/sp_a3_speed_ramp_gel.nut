// sp_a3_speed_ramp: two gel sprayers hang side by side over a goo pit,
// paint_sprayer_3 (repulsion, blue) and paint_sprayer_2 (propulsion, orange),
// both pointing down. They start only when the map's logic says so; this
// scenario starts them itself, frames both falling streams from a clear
// viewpoint beside them, freezes the blobs (blobs_paused) and takes the shots
// tools/quality/portal2_paint_shots.py compares: the empty view, the gel as
// the blobulator's isosurface, and as the fast spheres.

IncludeScript( "qa/qa_driver" )

::GEL <- {
	sprayers = [ "paint_sprayer_3", "paint_sprayer_2" ]
}

function QA_Shot( name )
{
	printl( "QA_SHOT " + ::QA.scenario + " " + name )
	SendToConsole( "screenshot" )
}

// The point <dist> along <dir> from <from>, stopped by the world.
function GEL_Reach( from, dir, dist )
{
	return from + QA_Scale( dir, dist * TraceLine( from, from + QA_Scale( dir, dist ), null ) )
}

QA_Do( "setup", function()
{
	SendToConsole( "sv_cheats 1; con_drawnotify 0; cl_drawhud 0; developer 0; r_drawviewmodel 0; god; noclip" )
}, 1.0 )

QA_Expect( "sprayers.found", function()
{
	local a = QA_Ent( ::GEL.sprayers[0] )
	local b = QA_Ent( ::GEL.sprayers[1] )
	QA_Detail( ::GEL.sprayers[0] + "=" + ( a ? QA_Vec( a.GetOrigin() ) : "missing" ) + " " +
		::GEL.sprayers[1] + "=" + ( b ? QA_Vec( b.GetOrigin() ) : "missing" ) )
	if ( !a || !b )
		return false
	// The streams fall from the sprayers into the pit; aim just below the
	// nozzles, halfway between the two streams (the pit's goo fogs the view).
	local top = QA_Scale( a.GetOrigin() + b.GetOrigin(), 0.5 )
	local floor = GEL_Reach( top, Vector( 0, 0, -1 ), 4000.0 )
	::GEL.target <- Vector( top.x, top.y, top.z - 160.0 )
	::GEL.fall <- top.z - floor.z
	return true
} )

// Stand beside the streams, across the axis joining them, where the world
// leaves the longest clear view of the target (at most 420 units away).
QA_Expect( "view.clear", function()
{
	local best = null
	local bestReach = 0.0
	foreach ( dir in [ Vector( 1, 0, 0 ), Vector( -1, 0, 0 ), Vector( 0, 1, 0 ), Vector( 0, -1, 0 ) ] )
	{
		local eye = GEL_Reach( ::GEL.target, dir, 420.0 )
		local reach = QA_Dist( eye, ::GEL.target )
		if ( reach > bestReach )
		{
			bestReach = reach
			best = ::GEL.target + QA_Scale( dir, reach - 16.0 )
		}
	}
	QA_Detail( "eye " + ( best ? QA_Vec( best ) : "none" ) + " reach " + bestReach + " fall " + ::GEL.fall )
	if ( !best || bestReach < 128.0 )
		return false
	::GEL.eye <- best
	return true
} )

QA_Do( "place the eye", function()
{
	local angles = QA_AnglesTo( ::GEL.eye, ::GEL.target )
	QA_PlaceEye( ::GEL.eye, angles.pitch, angles.yaw )
}, 2.0 )

QA_Do( "shot empty", function() { QA_Shot( "empty" ) }, 1.5 )

QA_Do( "start the sprayers", function()
{
	foreach ( name in ::GEL.sprayers )
		QA_Fire( name, "Start" )
}, 5.0 )

QA_Expect( "streams.flowing", function()
{
	local streams = 0
	for ( local s = Entities.FindByClassname( null, "paint_stream" ); s;
		s = Entities.FindByClassname( s, "paint_stream" ) )
		streams++
	QA_Detail( "paint_stream entities " + streams )
	return streams >= 2
} )

// Freeze the blobs so the three renderings draw the same particles.
QA_Do( "freeze", function()
{
	local angles = QA_AnglesTo( ::GEL.eye, ::GEL.target )
	QA_PlaceEye( ::GEL.eye, angles.pitch, angles.yaw )
	SendToConsole( "blobs_paused 1" )
}, 1.5 )
QA_Do( "shot isosurface", function() { QA_Shot( "isosurface" ) }, 1.5 )
QA_Do( "spheres", function() { SendToConsole( "r_paintblob_draw_isosurface 0" ) }, 1.0 )
QA_Do( "shot spheres", function() { QA_Shot( "spheres" ) }, 1.5 )
QA_Do( "restore", function() { SendToConsole( "r_paintblob_draw_isosurface 1; blobs_paused 0" ) }, 0.5 )

QA_Start( "sp_a3_speed_ramp_gel" )
