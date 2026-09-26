// Helpers for the portal2-paint-world-v1 scenarios (tools/quality/portal2_paint.py).
//
// The scenarios paint the world themselves: they move one of the map's
// info_paint_sprayer entities, change its paint type (0 bounce, 2 speed,
// 3 portal, 4 erase) and sweep it along a path while it sprays. They measure
// what retail Portal 2 and this build both expose to a server script (the
// player's origin and velocity, the portals placed) and print one line per
// number for the checker to compare against the retail reference:
//
//   QA_MEASURE <scenario> <name> <value>
//
// Screenshots are named with QA_SHOT <scenario> <name>, as in portal2-paint-v1.

::PW <- {
	sprayers = []
	noclip = false
	sweep = null
	samples = []
	t0 = 0.0
}

function PW_Measure( name, value )
{
	printl( "QA_MEASURE " + ::QA.scenario + " " + name + " " + format( "%.3f", value ) )
}

function PW_Shot( name )
{
	printl( "QA_SHOT " + ::QA.scenario + " " + name )
	SendToConsole( "screenshot" )
}

// Captions, notify lines and the view model would cover the frame.
function PW_Setup()
{
	SendToConsole( "sv_cheats 1; god; con_drawnotify 0; cl_drawhud 0; r_drawviewmodel 0; developer 0; " +
		"closecaption 0; cc_subtitles 0; hud_quickinfo 0" )
}

// noclip toggles; keep track of the state the scenario asked for.
function PW_Noclip( on )
{
	if ( ::PW.noclip != on )
	{
		SendToConsole( "noclip" )
		::PW.noclip = on
	}
}

function PW_FindSprayers()
{
	::PW.sprayers = []
	for ( local s = Entities.FindByClassname( null, "info_paint_sprayer" ); s;
		s = Entities.FindByClassname( s, "info_paint_sprayer" ) )
		::PW.sprayers.append( s )
	return ::PW.sprayers.len()
}

function PW_Lerp( a, b, t )
{
	return a + QA_Scale( b - a, t )
}

// Starts sprayer <index> with <type> at <from>, facing <pitch>/<yaw>; a
// PW_Sweeping wait step then moves it to <to> over <seconds> and stops it.
function PW_StartSweep( index, type, from, to, pitch, yaw, seconds )
{
	local s = ::PW.sprayers[index]
	s.SetOrigin( from )
	s.SetAngles( pitch, yaw, 0 )
	EntFireByHandle( s, "Stop", "", 0, null, null )
	EntFireByHandle( s, "ChangePaintType", "" + type, 0, null, null )
	EntFireByHandle( s, "Start", "", 0.05, null, null )
	::PW.sweep <- { sprayer = s, from = from, to = to, start = Time(), seconds = seconds }
}

function PW_Sweeping()
{
	local w = ::PW.sweep
	local t = ( Time() - w.start ) / w.seconds
	if ( t >= 1.0 )
	{
		w.sprayer.SetOrigin( w.to )
		EntFireByHandle( w.sprayer, "Stop", "", 0, null, null )
		QA_Detail( "swept to " + QA_Vec( w.to ) )
		return true
	}
	w.sprayer.SetOrigin( PW_Lerp( w.from, w.to, t ) )
	return false
}

function PW_StopAll()
{
	foreach ( s in ::PW.sprayers )
		EntFireByHandle( s, "Stop", "", 0, null, null )
}

// Places the player's feet at <feet> with no velocity, walking (noclip off).
function PW_PlaceFeet( feet, yaw )
{
	PW_Noclip( false )
	local player = QA_Player()
	player.SetVelocity( Vector( 0, 0, 0 ) )
	player.SetOrigin( feet )
	QA_SetView( 0.0, yaw )
}

function PW_StartSampling()
{
	::PW.samples = []
	::PW.t0 = Time()
}

// Records the player's origin and velocity; true once <seconds> have passed.
function PW_Sample( seconds )
{
	local player = QA_Player()
	::PW.samples.append( { t = Time() - ::PW.t0, o = player.GetOrigin(), v = player.GetVelocity() } )
	QA_Detail( ::PW.samples.len() + " samples" )
	return Time() - ::PW.t0 >= seconds
}

// Highest rise of the feet above the lowest point reached, after that point.
function PW_Rebound()
{
	local low = 0
	for ( local i = 1; i < ::PW.samples.len(); i++ )
		if ( ::PW.samples[i].o.z < ::PW.samples[low].o.z )
			low = i
	local peak = ::PW.samples[low].o.z
	for ( local i = low; i < ::PW.samples.len(); i++ )
		if ( ::PW.samples[i].o.z > peak )
			peak = ::PW.samples[i].o.z
	return { floor = ::PW.samples[low].o.z, rise = peak - ::PW.samples[low].o.z }
}

function PW_TopSpeed()
{
	local top = 0.0
	foreach ( s in ::PW.samples )
	{
		local h = sqrt( s.v.x * s.v.x + s.v.y * s.v.y )
		if ( h > top )
			top = h
	}
	return top
}

function PW_Portals()
{
	local list = []
	for ( local p = Entities.FindByClassname( null, "prop_portal" ); p;
		p = Entities.FindByClassname( p, "prop_portal" ) )
		list.append( p )
	return list
}

// Distance from <point> to the nearest portal, or 100000 without one.
function PW_NearestPortal( point )
{
	local best = 100000.0
	foreach ( p in PW_Portals() )
	{
		local d = QA_Dist( p.GetOrigin(), point )
		if ( d < best )
			best = d
	}
	return best
}
