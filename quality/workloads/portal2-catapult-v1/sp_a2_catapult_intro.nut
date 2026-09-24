// sp_a2_catapult_intro: the first aerial faith plates. Each targeted plate
// must throw the player onto its info_target along retail's arc, and the
// physics-only plate under the cube dropper must fling the dropped cube
// straight up.

IncludeScript( "qa/qa_driver" )
IncludeScript( "qa/qa_catapult" )

QA_Do( "watch outputs", function()
{
	QA_Watch( "catapult2a", "OnCatapulted" )
	QA_Watch( "catapult2b", "OnCatapulted" )
	QA_Watch( "player_catapult", "OnCatapulted" )
	QA_Watch( "catapult2c", "OnCatapulted" )
}, 1.0 )

// Across the pit onto the far ledge, then back, then from the plate under
// the cube dropper (whose own trigger ignores players).
QA_PlayerFling( "plate2a", "catapult2a", "launch_target2a", 270.0 )
QA_PlayerFling( "plate2b", "catapult2b", "launch_target2b", 90.0 )
QA_PlayerFling( "plate2c", "player_catapult", "launch_target2b", 90.0 )

QA_Expect( "plate2c.cube_plate_ignores_player", function()
{
	local n = QA_Fired( "catapult2c", "OnCatapulted" )
	QA_Detail( "catapult2c fired=" + n )
	return n == 0
} )

// The button's dropper releases its cube onto catapult2c.
QA_Do( "drop the cube", function()
{
	::QA.marks.cubeRise <- 0.0
	SendToConsole( "developer 2" )
	QA_Fire( "cube_dropper-proxy", "OnProxyRelay1" )
}, 0.1 )

QA_WaitFor( "cube.launched_up", function()
{
	local cube = Entities.FindByClassname( null, "prop_weighted_cube" )
	if ( cube == null )
		throw "no prop_weighted_cube"
	local n = QA_Fired( "catapult2c", "OnCatapulted" )
	local v = cube.GetVelocity()
	if ( n > 0 && v.z > ::QA.marks.cubeRise )
		::QA.marks.cubeRise = v.z
	QA_Detail( "catapult2c fired=" + n + " cube at " + QA_Vec( cube.GetOrigin() ) + " vel " +
	           QA_Vec( v ) + format( " max rise %.1f", ::QA.marks.cubeRise ) )
	// physicsSpeed 850 straight up; the physics step can shave a little.
	return n >= 1 && ::QA.marks.cubeRise > 800.0 && fabs( v.x ) < 50.0 && fabs( v.y ) < 50.0
}, 10.0 )

QA_Start( "sp_a2_catapult_intro" )
