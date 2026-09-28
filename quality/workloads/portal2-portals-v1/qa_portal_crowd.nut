// qa_portal_crowd: many animated props seen directly and through portals (map:
// tools/quality/portal2_portal_map.py).
//
// Every animated model with 16 or more bones records itself in the client's
// previous-frame bone list on its first bone setup of a frame. The opaque
// renderables of each view set up bones on the engine pool, and each portal
// view is another view, so those first setups arrive from several threads at
// once and the list grows while they do. Blue and orange face each other
// across the room and waves of personality spheres and Chell models (56 and
// 77 bones) appear between them, doubling each time, while the player looks
// into blue and turns. Heap corruption in that list ends the run without
// QA_DONE.

IncludeScript( "qa/qa_driver" )
IncludeScript( "qa/qa_portals" )

::CROWD <- {
	north = Vector( 0, 640, 56 )
	south = Vector( 0, -640, 56 )
	spawned = 0
	models = [ "models/player/chell/player.mdl", "models/npcs/personality_sphere/personality_sphere.mdl" ]
}

// One new prop. ent_create drops it where the player looks; CROWD_Layout
// moves every prop to its own grid spot afterwards.
function CROWD_Spawn()
{
	local n = ::CROWD.spawned++
	SendToConsole( "ent_create prop_dynamic model " + ::CROWD.models[ n % 2 ] + " solid 0" )
}

// Every prop on a grid in the room's southern half, in creation order.
function CROWD_Layout()
{
	local n = 0
	local p = null
	while ( p = Entities.FindByClassname( p, "prop_dynamic" ) )
	{
		p.SetOrigin( Vector( -480 + ( n % 24 ) * 40, -560 + ( n / 24 ) * 48, 0 ) )
		p.SetAngles( 0, ( n * 37 ) % 360, 0 )
		n++
	}
}

// `count` props, eight per tick so the console buffer never overflows.
function CROWD_Wave( count )
{
	for ( local i = 0; i < count; i++ )
		EntFireByHandle( ::QA.driver, "RunScriptCode", "CROWD_Spawn()", 0.02 * ( i / 8 ), null, null )
	QA_Log( "wave of " + count )
}

PW_Setup()
PW_Place( "blue", "attack", Vector( 0, 400, 64 ), ::CROWD.north, Vector( 0, -1, 0 ), "south" )
PW_Place( "orange", "attack2", Vector( 0, -400, 64 ), ::CROWD.south, Vector( 0, 1, 0 ), "north" )

// Look into blue: the props are behind the player, so they are drawn through
// the portal pair (orange looks back at them), and on turning, directly.
QA_Do( "look into blue", function() { PW_Aim( Vector( 0, 200, 64 ), ::CROWD.north ) }, 0.5 )
foreach ( count in [ 8, 16, 32, 64, 128 ] )
{
	local env = { count = count }
	::PW.envs.append( env )
	QA_Do( "wave " + count, function() { CROWD_Wave( count ) }.bindenv( env ), 1.0 )
	QA_Do( "layout " + count, function() { CROWD_Layout() }, 0.5 )
	QA_Do( "turn " + count, function()
	{
		SendToConsole( "cl_yawspeed 180" )
		QA_Press( "left", 1.0 )
	}.bindenv( env ), 1.5 )
}
QA_Do( "yaw speed", function() { SendToConsole( "cl_yawspeed 210" ) }, 0.5 )
QA_Do( "shot crowd", function() { PW_Shot( "crowd" ) }, 0.5 )
QA_Expect( "crowd.spawned", function()
{
	local count = 0
	local p = null
	while ( p = Entities.FindByClassname( p, "prop_dynamic" ) )
		count++
	QA_Detail( count + " prop_dynamic of " + ::CROWD.spawned + " requested" )
	return count >= ::CROWD.spawned
} )

QA_Start( "qa_portal_crowd" )
