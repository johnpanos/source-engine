// sp_a1_intro2: a weighted cube goes through the same pickup controller as a
// personality core, but in the physics hold and with dropping allowed. The
// player picks it up with +use, carries it across the chamber in walking-size
// steps, and drops it on the floor button.

IncludeScript( "qa/qa_driver" )

QA_Do( "watch outputs", function()
{
	QA_Watch( "box", "OnPlayerPickup" )
	QA_Watch( "box", "OnPhysGunDrop" )
	QA_Watch( "button_1-button", "OnPressed" )
	::QA.marks.button <- QA_Ent( "button_1-button" ).GetOrigin()
}, 0.5 )

QA_WaitFor( "cube.at_rest", function()
{
	local v = QA_Ent( "box" ).GetVelocity().Length()
	QA_Detail( "cube " + QA_Vec( QA_Ent( "box" ).GetCenter() ) + format( " speed %.1f", v ) )
	return v < 1.0
}, 10.0 )

QA_Do( "walk up to the cube", function()
{
	local cube = QA_Ent( "box" ).GetCenter()
	local p = QA_Player()
	// Stand on the cube's floor, on the side facing the button.
	p.SetOrigin( Vector( cube.x - 60.0, cube.y, cube.z - 20.0 ) )
}, 1.0 )

QA_WaitFor( "cube.picked_up", function()
{
	if ( QA_Fired( "box", "OnPlayerPickup" ) >= 1 )
		return true
	QA_TryPickup( "box", 44.0 )
	QA_Detail( "cube " + QA_Vec( QA_Ent( "box" ).GetCenter() ) + " eye " + QA_Vec( QA_Player().EyePosition() ) )
	return false
}, 10.0 )

QA_Do( "level the view", function() { QA_SetView( 0.0, ::QA.marks.yaw ) }, 1.0 )

QA_WaitFor( "cube.held", function()
{
	QA_MarkViewOffset( "held", QA_Ent( "box" ) )
	local o = ::QA.marks.held
	QA_Detail( format( "view offset f=%.1f r=%.1f u=%.1f", o.f, o.r, o.u ) )
	return o.f > 20.0 && o.f < 120.0 && fabs( o.r ) < 60.0 && fabs( o.u ) < 60.0
}, 3.0 )

QA_Do( "turn", function() { QA_SetView( 0.0, ::QA.marks.yaw + 40.0 ) }, 1.0 )
QA_Expect( "cube.follows_turn", function() { return QA_HeldAt( QA_Ent( "box" ), "held", 16.0 ) } )

QA_Sleep( 3.0 )
QA_Expect( "cube.follows_after_idle", function() { return QA_HeldAt( QA_Ent( "box" ), "held", 16.0 ) } )

// Walk toward the button 12 units per 0.1 s (about walking speed), looking
// where we go; the cube must come along without being dropped.
QA_WaitFor( "cube.carried_across", function()
{
	if ( QA_Fired( "box", "OnPhysGunDrop" ) > 0 )
		throw "cube dropped on the way at " + QA_Vec( QA_Ent( "box" ).GetCenter() )
	local p = QA_Player()
	local eye = p.EyePosition()
	local goal = ::QA.marks.button + Vector( 80.0, 0.0, 0.0 )
	local d = goal - eye
	d.z = 0.0
	local remaining = d.Length()
	local yaw = QA_AnglesTo( eye, ::QA.marks.button ).yaw
	QA_Detail( "eye " + QA_Vec( eye ) + format( " %.0f to go", remaining ) + " cube " +
	           QA_Vec( QA_Ent( "box" ).GetCenter() ) )
	if ( remaining < 8.0 )
	{
		QA_SetView( 0.0, yaw )
		return true
	}
	local step = remaining < 12.0 ? remaining : 12.0
	QA_PlaceEye( eye + QA_Scale( d, step / remaining ), 0.0, yaw )
	return false
}, 20.0 )

QA_Sleep( 1.0 )
QA_Expect( "cube.still_held_at_button", function()
{
	local held = QA_HeldAt( QA_Ent( "box" ), "held", 20.0 )
	QA_Detail( ::QA.detail + " drops=" + QA_Fired( "box", "OnPhysGunDrop" ) )
	return held && QA_Fired( "box", "OnPhysGunDrop" ) == 0
} )

QA_Do( "hold it over the button", function()
{
	QA_MarkViewOffset( "drop", QA_Ent( "box" ) )
	if ( !QA_ReachTo( "drop", ::QA.marks.button + Vector( 0, 0, 30 ), QA_Player().EyePosition() ) )
		throw "button out of reach"
}, 1.5 )

QA_Do( "let go", function() { QA_Press( "use", 0.2 ) }, 1.0 )

QA_Expect( "cube.dropped", function()
{
	return QA_Fired( "box", "OnPhysGunDrop" ) == 1
} )

QA_WaitFor( "button.pressed", function()
{
	QA_Detail( "cube " + QA_Vec( QA_Ent( "box" ).GetCenter() ) + " button " + QA_Vec( ::QA.marks.button ) )
	return QA_Fired( "button_1-button", "OnPressed" ) >= 1
}, 5.0 )

QA_Start( "sp_a1_intro2" )
