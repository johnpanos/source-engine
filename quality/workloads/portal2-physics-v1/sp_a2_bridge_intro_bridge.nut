// Hard light bridge and floor button in sp_a2_bridge_intro. The projector
// at (-800 -768 16) throws its bridge along +x. The player and a cube are
// dropped onto the bridge and must come to rest on it; the cube is then put
// on the floor button at (-480 -448 72), which must press, and lifted off,
// which must release.

IncludeScript( "qa/qa_driver" )
IncludeScript( "qa/qa_physics" )

::BR <- {
	projector = Vector( -800, -768, 16 )
	button = Vector( -480, -448, 72 )
}

function BR_Rest( key )
{
	local s = PH_Samples( key )
	return s[s.len() - 1]
}

QA_Do( "setup", function()
{
	SendToConsole( "sv_cheats 1; developer 0; con_drawnotify 0" )
	QA_Watch( "projector1", "OnFizzle" )
	local bridge = Entities.FindByClassname( null, "projected_wall_entity" )
	::BR.bridge <- bridge
	if ( bridge != null )
		QA_Log( "bridge at " + QA_Vec( bridge.GetOrigin() ) + " center " + QA_Vec( bridge.GetCenter() ) +
		        " mins " + QA_Vec( bridge.GetBoundingMins() ) + " maxs " + QA_Vec( bridge.GetBoundingMaxs() ) )
	local under = PH_Ray( ::BR.projector + Vector( 200, 0, 64 ), Vector( 0, 0, -1 ), 4000.0 )
	QA_Log( format( "trace down from above the bridge hits after %.1f", under ) )
}, 1.0 )
QA_Sleep( 4.0 )
QA_Expect( "bridge.exists", function()
{
	local bridge = Entities.FindByClassname( null, "projected_wall_entity" )
	QA_Detail( bridge == null ? "no projected_wall_entity" : "at " + QA_Vec( bridge.GetOrigin() ) )
	return bridge != null
} )

// 1. The player dropped 40 units onto the bridge, 400 units out.
QA_Do( "drop the player on the bridge", function()
{
	local p = QA_Player()
	p.SetOrigin( ::BR.projector + Vector( 400, 0, 40 ) )
	p.SetVelocity( Vector( 0, 0, 0 ) )
	QA_SetView( 0.0, 0.0 )
	PH_Track( "player", p, 2.0, false )
}, 2.2 )
QA_Expect( "player.stands", function()
{
	local rest = BR_Rest( "player" )
	PH_Metric( "player.rest_z", rest.pos.z )
	PH_Flag( "player.ok_on_bridge", rest.pos.z > ::BR.projector.z - 16.0 )
	QA_Detail( "rest " + QA_Vec( rest.pos ) + " vel " + QA_Vec( rest.vel ) )
	return true
} )

// 2. Walk along the bridge for a second.
QA_Do( "walk along the bridge", function()
{
	PH_Track( "walk", QA_Player(), 1.6, false )
	QA_Press( "forward", 1.0 )
}, 1.8 )
QA_Expect( "player.walks", function()
{
	local s = PH_Samples( "walk" )
	local last = s[s.len() - 1]
	PH_Metric( "walk.distance", last.pos.x - s[0].pos.x )
	PH_Metric( "walk.rest_z", last.pos.z )
	QA_Detail( "at " + QA_Vec( last.pos ) )
	return true
} )

// 3. A cube dropped onto the bridge beside the player.
QA_Do( "move the player off and drop a cube on the bridge", function()
{
	local p = QA_Player()
	p.SetOrigin( ::BR.projector + Vector( -120, 200, 0 ) )
	p.SetVelocity( Vector( 0, 0, 0 ) )
	local cube = Entities.FindByClassname( null, "prop_weighted_cube" )
	if ( cube == null )
		throw "no prop_weighted_cube"
	::BR.cube <- cube
	cube.SetOrigin( ::BR.projector + Vector( 300, 0, 80 ) )
	cube.SetAngles( 0, 0, 0 )
	EntFireByHandle( cube, "Wake", "", 0.0, null, null )
	PH_Track( "cube", cube, 3.0, true )
}, 3.2 )
QA_Expect( "cube.rests", function()
{
	local rest = BR_Rest( "cube" )
	PH_Metric( "cube.rest_z", rest.pos.z )
	PH_Metric( "cube.rest_speed", rest.vel.Length() )
	PH_Flag( "cube.ok_on_bridge", rest.pos.z > ::BR.projector.z - 16.0 )
	QA_Detail( "rest " + QA_Vec( rest.pos ) + " vel " + QA_Vec( rest.vel ) )
	return true
} )

// 4. The cube on the floor button, then lifted off it.
QA_Do( "watch the button", function()
{
	local button = Entities.FindByClassnameNearest( "prop_floor_button", ::BR.button, 16.0 )
	if ( button == null )
		throw "no prop_floor_button"
	QA_WatchEntity( button, "button", "OnPressed" )
	QA_WatchEntity( button, "button", "OnUnPressed" )
	::BR.cube.SetOrigin( ::BR.button + Vector( 0, 0, 40 ) )
	::BR.cube.SetAngles( 0, 0, 0 )
	EntFireByHandle( ::BR.cube, "Wake", "", 0.0, null, null )
	::BR.dropped <- Time()
}, 0.0 )
QA_WaitFor( "button.pressed", function()
{
	local n = QA_Fired( "button", "OnPressed" )
	QA_Detail( "OnPressed " + n + " cube " + QA_Vec( ::BR.cube.GetCenter() ) )
	return n >= 1
}, 3.0 )
QA_Expect( "button.press_time", function()
{
	PH_Metric( "button.press_delay", Time() - ::BR.dropped )
	PH_Flag( "button.ok_pressed", QA_Fired( "button", "OnPressed" ) >= 1 )
	return true
} )
QA_Do( "lift the cube off", function()
{
	::BR.cube.SetOrigin( ::BR.button + Vector( 0, 160, 40 ) )
	EntFireByHandle( ::BR.cube, "Wake", "", 0.0, null, null )
}, 0.0 )
QA_WaitFor( "button.released", function()
{
	local n = QA_Fired( "button", "OnUnPressed" )
	QA_Detail( "OnUnPressed " + n )
	return n >= 1
}, 3.0 )
QA_Expect( "button.release_count", function()
{
	PH_Flag( "button.ok_released", QA_Fired( "button", "OnUnPressed" ) >= 1 )
	PH_Metric( "button.presses", QA_Fired( "button", "OnPressed" ) )
	return true
} )

// 5. Last, the player put 64 units above the bridge 200 units out overlaps a
// solid that sits there (it also holds a dropped cube at z 144): retail
// leaves a stuck player where it was put.
QA_Do( "put the player into the solid", function()
{
	local p = QA_Player()
	p.SetOrigin( ::BR.projector + Vector( 200, 0, 64 ) )
	p.SetVelocity( Vector( 0, 0, 0 ) )
	QA_SetView( 0.0, 0.0 )
	PH_Track( "stuck", p, 1.5, false )
}, 1.7 )
QA_Expect( "stuck.held", function()
{
	local rest = BR_Rest( "stuck" )
	PH_Metric( "stuck.sink", ::BR.projector.z + 64.0 - rest.pos.z )
	QA_Detail( "rest " + QA_Vec( rest.pos ) )
	return true
} )

QA_Start( "sp_a2_bridge_intro_bridge" )
