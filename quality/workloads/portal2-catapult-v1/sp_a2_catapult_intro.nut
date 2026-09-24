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

// The button's dropper releases its cube onto catapult2c, which throws it
// straight up at physicsSpeed 850: it must rise about 850^2 / (2 g) = 602
// units above the plate. (A VPhysics prop's scripted velocity is not
// maintained, so the rise is measured from positions.)
QA_Do( "drop the cube", function()
{
	::QA.marks.cubeLaunchZ <- null
	::QA.marks.cubeApex <- -100000.0
	::QA.marks.cubeDrift <- 0.0
	QA_Fire( "cube_dropper-proxy", "OnProxyRelay1" )
}, 0.1 )

QA_WaitFor( "cube.launched_up", function()
{
	local cube = Entities.FindByClassname( null, "prop_weighted_cube" )
	if ( cube == null )
		throw "no prop_weighted_cube"
	local plate = QA_Ent( "catapult2c" )
	local n = QA_Fired( "catapult2c", "OnCatapulted" )
	local c = cube.GetCenter()
	if ( n > 0 )
	{
		if ( ::QA.marks.cubeLaunchZ == null )
			::QA.marks.cubeLaunchZ = c.z
		if ( c.z > ::QA.marks.cubeApex )
			::QA.marks.cubeApex = c.z
		local drift = QA_FlatDist( c, plate.GetCenter() )
		if ( drift > ::QA.marks.cubeDrift )
			::QA.marks.cubeDrift = drift
	}
	local rise = ::QA.marks.cubeLaunchZ == null ? 0.0 : ::QA.marks.cubeApex - ::QA.marks.cubeLaunchZ
	QA_Detail( "catapult2c fired=" + n + " cube at " + QA_Vec( c ) +
	           format( " rise=%.1f drift=%.1f", rise, ::QA.marks.cubeDrift ) )
	// Falling back onto the plate relaunches it, so wait for the apex to pass.
	return n >= 1 && rise > 0.85 * 602.0 && rise < 1.1 * 602.0 && ::QA.marks.cubeDrift < 24.0 &&
	       c.z < ::QA.marks.cubeApex - 32.0
}, 12.0 )

// The same cube dropped on catapult2a, which also takes physics objects,
// must be thrown onto launch_target2a at physicsSpeed 650 (no player
// offset). Retail zeroes a launched object's drag so it flies the arc.
QA_Do( "drop the cube on catapult2a", function()
{
	local cube = Entities.FindByClassname( null, "prop_weighted_cube" )
	local plate = QA_Ent( "catapult2a" )
	::QA.marks.cubeFired <- QA_Fired( "catapult2a", "OnCatapulted" )
	::QA.marks.cubeTrack <- { samples = [] }
	cube.SetOrigin( plate.GetCenter() + Vector( 0, 0, 96 ) )
}, 0.0 )

QA_WaitFor( "cube_fling.launched", function()
{
	local n = QA_Fired( "catapult2a", "OnCatapulted" ) - ::QA.marks.cubeFired
	QA_Detail( "catapult2a fired=" + n )
	return n >= 1
}, 3.0 )

// Where the cube comes down: the first tick whose position leaves the
// ballistic path through the two before it marks contact, and the arc from
// the last free sample is solved down to the height the cube stopped at.
QA_WaitFor( "cube_fling.touchdown", function()
{
	local cube = Entities.FindByClassname( null, "prop_weighted_cube" )
	local track = ::QA.marks.cubeTrack
	local sample = { t = Time(), pos = cube.GetCenter() }
	QA_Detail( "cube at " + QA_Vec( sample.pos ) )
	local hit = QA_BallisticContact( track, sample )
	if ( hit == null )
		return false
	local goal = QA_Ent( "launch_target2a" ).GetOrigin()
	::QA.marks.cubeMiss <- QA_FlatDist( hit, goal )
	QA_Detail( "cube came down at " + QA_Vec( hit ) + " target " + QA_Vec( goal ) +
	           format( " miss=%.1f", ::QA.marks.cubeMiss ) )
	return true
}, 6.0 )

QA_Expect( "cube_fling.on_target", function()
{
	QA_Detail( format( "miss=%.1f", ::QA.marks.cubeMiss ) )
	return ::QA.marks.cubeMiss < 40.0
} )

QA_Start( "sp_a2_catapult_intro" )
