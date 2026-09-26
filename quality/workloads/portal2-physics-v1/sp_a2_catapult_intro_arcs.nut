// Faith plates (trigger_catapult) in sp_a2_catapult_intro, measured against
// retail: where the targeted plates put the player down, how high the arc
// rises, how long the flight takes, and how far the physics-only plate
// throws a cube. The flight helpers are portal2-catapult-v1's.

IncludeScript( "qa/qa_driver" )
IncludeScript( "qa/qa_physics" )
IncludeScript( "qa/qa_catapult" )

function ARC_Report( prefix, target )
{
	local f = ::QA.marks[prefix]
	local goal = QA_Ent( target ).GetOrigin()
	local td = f.touchdown
	if ( td == null )
		throw prefix + " never landed"
	PH_Metric( prefix + ".miss", QA_FlatDist( td, goal ) )
	PH_Metric( prefix + ".apex_above_target", f.apex - goal.z )
	PH_Metric( prefix + ".touchdown_x", td.x )
	PH_Metric( prefix + ".touchdown_y", td.y )
	PH_Metric( prefix + ".gravity", QA_FlightGravity( prefix ) )
	PH_Metric( prefix + ".launch_speed", ::QA.marks[prefix + "_launch"] )
}

// QA_PlayerFling's launch check, plus the launch speed of the first
// airborne tick.
function ARC_Fling( prefix, catapult, target, yaw )
{
	QA_PlayerFling( prefix, catapult, target, yaw )
	QA_Expect( prefix + ".measured", function() : ( prefix, target )
	{
		ARC_Report( prefix, target )
		return true
	} )
}

QA_Do( "watch outputs", function()
{
	SendToConsole( "sv_cheats 1; developer 0" )
	QA_Watch( "catapult2a", "OnCatapulted" )
	QA_Watch( "catapult2b", "OnCatapulted" )
	QA_Watch( "player_catapult", "OnCatapulted" )
	QA_Watch( "catapult2c", "OnCatapulted" )
	::QA.marks.plate2a_launch <- 0.0
	::QA.marks.plate2b_launch <- 0.0
}, 1.0 )
QA_Sleep( 4.0 )

// Launch speed: sampled every tick from the moment the player steps on.
function ARC_LaunchWatch( prefix )
{
	local p = QA_Player()
	local v = p.GetVelocity()
	if ( v.z > 0.0 && v.Length() > ::QA.marks[prefix + "_launch"] )
		::QA.marks[prefix + "_launch"] = v.Length()
	if ( Time() < ::QA.marks[prefix + "_watchUntil"] )
		EntFireByHandle( ::QA.driver, "RunScriptCode", "ARC_LaunchWatch(\"" + prefix + "\")", 0.01, null, null )
}

QA_Do( "watch 2a", function()
{
	::QA.marks.plate2a_watchUntil <- Time() + 2.5
	ARC_LaunchWatch( "plate2a" )
}, 0.0 )
ARC_Fling( "plate2a", "catapult2a", "launch_target2a", 270.0 )
QA_Do( "watch 2b", function()
{
	::QA.marks.plate2b_watchUntil <- Time() + 2.5
	ARC_LaunchWatch( "plate2b" )
}, 0.0 )
ARC_Fling( "plate2b", "catapult2b", "launch_target2b", 90.0 )

// The physics plate under the dropper throws the cube straight up at
// physicsSpeed 850 (retail: about 850^2 / 2g = 602 units).
QA_Do( "drop the cube", function()
{
	::QA.marks.cubeLaunchZ <- null
	::QA.marks.cubeApex <- -100000.0
	QA_Fire( "cube_dropper-proxy", "OnProxyRelay1" )
}, 0.1 )
QA_WaitFor( "cube.launched_up", function()
{
	local cube = Entities.FindByClassname( null, "prop_weighted_cube" )
	if ( cube == null )
		throw "no prop_weighted_cube"
	local n = QA_Fired( "catapult2c", "OnCatapulted" )
	local c = cube.GetCenter()
	if ( n > 0 )
	{
		if ( ::QA.marks.cubeLaunchZ == null )
			::QA.marks.cubeLaunchZ = c.z
		if ( c.z > ::QA.marks.cubeApex )
			::QA.marks.cubeApex = c.z
	}
	local rise = ::QA.marks.cubeLaunchZ == null ? 0.0 : ::QA.marks.cubeApex - ::QA.marks.cubeLaunchZ
	QA_Detail( "catapult2c fired=" + n + format( " rise=%.1f", rise ) )
	return n >= 1 && rise > 100.0 && c.z < ::QA.marks.cubeApex - 32.0
}, 12.0 )
QA_Expect( "cube.measured", function()
{
	PH_Metric( "cube.rise", ::QA.marks.cubeApex - ::QA.marks.cubeLaunchZ )
	return true
} )

QA_Start( "sp_a2_catapult_intro_arcs" )
