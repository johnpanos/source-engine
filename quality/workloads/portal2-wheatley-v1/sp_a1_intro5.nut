// sp_a1_intro5: a weighted cube goes through the same pickup controller as a
// personality core, but in the physics hold and with dropping allowed. The
// first dropper's cube is moved onto the floor of the first button (the key
// event: a cube on the button's floor); the player picks it up with +use,
// walks it over to the button and drops it there.

IncludeScript( "qa/qa_driver" )

QA_Do( "watch outputs", function()
{
	QA_Watch( "cube_dropper_1-cube_dropper_box", "OnPlayerPickup" )
	QA_Watch( "cube_dropper_1-cube_dropper_box", "OnPhysGunDrop" )
	QA_Watch( "button_1-button", "OnPressed" )
	::QA.marks.button <- QA_Ent( "button_1-button" ).GetOrigin()
}, 0.5 )

// 160 units from the button, toward the dropper, on the button's own floor.
QA_Do( "place the cube", function()
{
	local cube = QA_Ent( "cube_dropper_1-cube_dropper_box" )
	local toward = cube.GetCenter() - ::QA.marks.button
	toward.z = 0.0
	toward = QA_Scale( toward, 1.0 / toward.Length() )
	local spot = ::QA.marks.button + QA_Scale( toward, 160.0 )
	local top = Vector( spot.x, spot.y, ::QA.marks.button.z + 64.0 )
	local floorZ = top.z - 256.0 * TraceLine( top, top - Vector( 0, 0, 256 ), cube )
	if ( fabs( floorZ - ::QA.marks.button.z ) > 12.0 )
		throw format( "no floor at the button's height there (%.1f vs %.1f)", floorZ, ::QA.marks.button.z )
	::QA.marks.toward <- toward
	cube.SetOrigin( Vector( spot.x, spot.y, floorZ + 24.0 ) )
}, 0.5 )

QA_WaitFor( "cube.at_rest", function()
{
	local v = QA_Ent( "cube_dropper_1-cube_dropper_box" ).GetVelocity().Length()
	QA_Detail( "cube " + QA_Vec( QA_Ent( "cube_dropper_1-cube_dropper_box" ).GetCenter() ) + format( " speed %.1f", v ) )
	return v < 1.0
}, 10.0 )

QA_Do( "walk up to the cube", function()
{
	// Stand beyond the cube, facing it and the button behind it.
	local cube = QA_Ent( "cube_dropper_1-cube_dropper_box" ).GetCenter()
	local feet = cube + QA_Scale( ::QA.marks.toward, 60.0 )
	feet.z = ::QA.marks.button.z + 2.0
	QA_Player().SetOrigin( feet )
}, 1.0 )

QA_WaitFor( "cube.picked_up", function()
{
	if ( QA_Fired( "cube_dropper_1-cube_dropper_box", "OnPlayerPickup" ) >= 1 )
		return true
	QA_TryPickup( "cube_dropper_1-cube_dropper_box", 44.0 )
	QA_Detail( "cube " + QA_Vec( QA_Ent( "cube_dropper_1-cube_dropper_box" ).GetCenter() ) + " eye " + QA_Vec( QA_Player().EyePosition() ) )
	return false
}, 10.0 )

QA_Do( "level the view", function() { QA_SetView( 0.0, ::QA.marks.yaw ) }, 1.0 )

QA_WaitFor( "cube.held", function()
{
	QA_MarkViewOffset( "held", QA_Ent( "cube_dropper_1-cube_dropper_box" ) )
	local o = ::QA.marks.held
	QA_Detail( format( "view offset f=%.1f r=%.1f u=%.1f", o.f, o.r, o.u ) )
	return o.f > 20.0 && o.f < 120.0 && fabs( o.r ) < 60.0 && fabs( o.u ) < 60.0
}, 3.0 )

QA_Do( "turn", function() { QA_SetView( 0.0, ::QA.marks.yaw + 40.0 ) }, 1.0 )
QA_Expect( "cube.follows_turn", function() { return QA_HeldAt( QA_Ent( "cube_dropper_1-cube_dropper_box" ), "held", 16.0 ) } )

QA_Sleep( 3.0 )
QA_Expect( "cube.follows_after_idle", function() { return QA_HeldAt( QA_Ent( "cube_dropper_1-cube_dropper_box" ), "held", 16.0 ) } )

// Walk to the button with +forward, steering toward it every 0.1 s; the cube
// must come along, down the ledge, without being dropped.
QA_Do( "start walking", function()
{
	QA_SetView( 0.0, QA_AnglesTo( QA_Player().EyePosition(), ::QA.marks.button ).yaw )
	SendToConsole( "+forward" )
}, 0.1 )

QA_WaitFor( "cube.carried_across", function()
{
	if ( QA_Fired( "cube_dropper_1-cube_dropper_box", "OnPhysGunDrop" ) > 0 )
	{
		SendToConsole( "-forward" )
		throw "cube dropped on the way at " + QA_Vec( QA_Ent( "cube_dropper_1-cube_dropper_box" ).GetCenter() )
	}
	local eye = QA_Player().EyePosition()
	local d = ::QA.marks.button - eye
	d.z = 0.0
	QA_Detail( "eye " + QA_Vec( eye ) + format( " %.0f to go", d.Length() ) + " cube " +
	           QA_Vec( QA_Ent( "cube_dropper_1-cube_dropper_box" ).GetCenter() ) )
	if ( d.Length() < 110.0 )
	{
		SendToConsole( "-forward" )
		return true
	}
	QA_SetView( 0.0, QA_AnglesTo( eye, ::QA.marks.button ).yaw )
	return false
}, 20.0 )

QA_Sleep( 1.0 )
QA_Expect( "cube.still_held_at_button", function()
{
	local held = QA_HeldAt( QA_Ent( "cube_dropper_1-cube_dropper_box" ), "held", 20.0 )
	QA_Detail( ::QA.detail + " drops=" + QA_Fired( "cube_dropper_1-cube_dropper_box", "OnPhysGunDrop" ) )
	return held && QA_Fired( "cube_dropper_1-cube_dropper_box", "OnPhysGunDrop" ) == 0
} )

QA_Do( "hold it over the button", function()
{
	QA_MarkViewOffset( "drop", QA_Ent( "cube_dropper_1-cube_dropper_box" ) )
	if ( !QA_ReachTo( "drop", ::QA.marks.button + Vector( 0, 0, 30 ), QA_Player().EyePosition() ) )
		throw "button out of reach"
}, 1.5 )

QA_Do( "let go", function() { QA_Press( "use", 0.2 ) }, 1.0 )

QA_Expect( "cube.dropped", function()
{
	return QA_Fired( "cube_dropper_1-cube_dropper_box", "OnPhysGunDrop" ) == 1
} )

QA_WaitFor( "button.pressed", function()
{
	QA_Detail( "cube " + QA_Vec( QA_Ent( "cube_dropper_1-cube_dropper_box" ).GetCenter() ) + " button " + QA_Vec( ::QA.marks.button ) )
	return QA_Fired( "button_1-button", "OnPressed" ) >= 1
}, 5.0 )

QA_Start( "sp_a1_intro5" )
