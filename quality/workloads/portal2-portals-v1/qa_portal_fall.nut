// qa_portal_fall: floor portals (map: tools/quality/portal2_portal_map.py).
//
// Blue goes on the floor and orange high on the east wall. The player walks
// onto blue, falls in and flies out of orange heading west, then lands. Then
// orange moves to the ceiling straight above blue, and the player drops into
// an endless fall through the pair: each pass must come out of the ceiling,
// and the fall keeps speeding up. The client's view trace (PVIEW) must stay
// continuous through every crossing (the workload's console checks).

IncludeScript( "qa/qa_driver" )
IncludeScript( "qa/qa_portals" )

::FALL <- {
	floor = Vector( 0, -320, 0 )
	wall = Vector( 640, -192, 200 )
	ceiling = Vector( 0, -320, 448 )
	loopSpeeds = []
}

PW_Setup()
PW_Place( "blue", "attack", Vector( 0, -160, 64 ), ::FALL.floor, Vector( 0, 0, 1 ), "up" )
PW_Place( "orange", "attack2", Vector( 400, -192, 64 ), ::FALL.wall, Vector( -1, 0, 0 ), "west" )

// Walk south onto the floor portal: fall in, fly out of the wall heading west.
QA_Do( "stand near blue", function()
{
	local player = QA_Player()
	player.SetVelocity( Vector( 0, 0, 0 ) )
	player.SetOrigin( Vector( 0, -200, 0 ) )
	QA_SetView( 0.0, -90.0 )
}, 1.0 )
QA_Do( "floor begin", function() { PW_Window( "floor_to_wall", "BEGIN" ) }, 0.0 )
QA_Do( "walk onto blue", function()
{
	PW_Path( "floor_to_wall", 3.0 )
	QA_Press( "forward", 0.9 )
}, 3.2 )
QA_Do( "floor end", function() { PW_Window( "floor_to_wall", "END" ) }, 0.1 )
QA_Expect( "floor_to_wall.landed_west_of_orange", function()
{
	local o = QA_Player().GetOrigin()
	local w = ::FALL.wall
	QA_Detail( "player " + QA_Vec( o ) + " vel " + QA_Vec( QA_Player().GetVelocity() ) )
	return o.x < w.x - 24 && o.x > w.x - 400 && fabs( o.y - w.y ) < 64 && o.z < 4
} )
QA_Do( "shot after the fall", function() { PW_Shot( "after_floor_to_wall" ) }, 0.3 )

// The endless fall: orange on the ceiling above blue.
PW_Place( "orange_ceiling", "attack2", Vector( 0, -200, 64 ), ::FALL.ceiling, Vector( 0, 0, -1 ), "down" )
QA_Expect( "loop.orange_moved", function()
{
	// The gun moves its orange portal; it does not open a third one.
	local count = 0
	local p = null
	while ( p = Entities.FindByClassname( p, "prop_portal" ) )
		count++
	QA_Detail( count + " portals" )
	return ::PW.portals.orange_ceiling == ::PW.portals.orange
} )
QA_Do( "drop over blue", function()
{
	local player = QA_Player()
	player.SetVelocity( Vector( 0, 0, 0 ) )
	player.SetOrigin( ::FALL.floor + Vector( 0, 0, 24 ) )
	QA_SetView( 0.0, 90.0 )
}, 0.3 )
// The window opens after the drop, whose teleport is the script's own.
QA_Do( "loop begin", function() { PW_Window( "floor_ceiling_loop", "BEGIN" ) }, 0.0 )
QA_Do( "loop path", function() { PW_Path( "loop", 4.0 ) }, 4.0 )
QA_Do( "loop end", function() { PW_Window( "floor_ceiling_loop", "END" ) }, 0.0 )
QA_Expect( "loop.still_falling_in_column", function()
{
	local player = QA_Player()
	local o = player.GetOrigin()
	local v = player.GetVelocity()
	QA_Detail( "player " + QA_Vec( o ) + " vel " + QA_Vec( v ) )
	return fabs( o.x - ::FALL.floor.x ) < 24 && fabs( o.y - ::FALL.floor.y ) < 24 &&
	       o.z > -80 && o.z < 460 && v.z < -600
} )
QA_Do( "shot in the loop", function() { PW_Shot( "loop" ) }, 0.3 )
// Closing the pair drops the player onto the floor at loop speed: the hard
// landing (PlayerRoughLandingEffects, its sound) must not crash.
QA_Do( "close the portals", function()
{
	::FALL.landingSpeed <- -QA_Player().GetVelocity().z
	SendToConsole( "ent_fire prop_portal Fizzle" )
}, 2.0 )
QA_Expect( "loop.landed", function()
{
	local o = QA_Player().GetOrigin()
	QA_Detail( "player " + QA_Vec( o ) + " after falling at " + ::FALL.landingSpeed + " u/s" )
	return ::FALL.landingSpeed > 600 && o.z < 4 && QA_Player().GetHealth() > 0
} )

QA_Start( "qa_portal_fall" )
