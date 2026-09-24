// sp_a1_intro7: Wheatley falls off his management rail, the player picks him
// up, plugs him into the wall socket that opens the secret panel, looks away
// and back while he works, and picks him up again when the socket ejects him.
// Every story beat after the first two warps is driven by the map's own
// scenes and triggers; the test only moves and aims the player and presses
// +use, and waits for the outputs each beat fires.

IncludeScript( "qa/qa_driver" )

QA_Do( "watch outputs", function()
{
	QA_Watch( "@sphere", "OnPlayerPickup" )
	QA_Watch( "@sphere", "OnPlayerDrop" )
	QA_Watch( "catch_trigger_fake", "OnStartTouch" )
	QA_Watch( "sphere_impact_trigger", "OnStartTouch" )
	QA_Watch( "@plug_open_rl", "OnTrigger" )
	QA_Watch( "InstanceAuto9-socket_powered_rl", "OnTrigger" )
	QA_Watch( "@panel_open_rl", "OnTrigger" )
	QA_Watch( "wheatley_dont_watch_aisc", "OnConditionsSatisfied" )
	QA_Watch( "wheatley_watch_aisc", "OnConditionsSatisfied" )
	QA_Watch( "@plug_eject_rl", "OnTrigger" )
	QA_Watch( "find_elevator_relay", "OnTrigger" )
	::QA.marks.socket <- QA_Ent( "InstanceAuto9-socket_trigger" ).GetCenter()
}, 0.5 )

QA_Expect( "rail.attached", function()
{
	local s = QA_Ent( "@sphere" )
	QA_Detail( "parent=" + QA_ParentName( s ) )
	return QA_ParentName( s ) == "spherebot_1_bottom_swivel_1"
} )

// Entering the second room starts the management rail scene, which arms the
// catch trigger; standing in it starts the "on three" scenes that drop him.
QA_Do( "enter second room", function()
{
	QA_StandIn( QA_Ent( "second_room_trigger" ), 90.0 )
}, 1.0 )

QA_Do( "wait under the rail", function()
{
	QA_StandIn( QA_Ent( "catch_trigger_fake" ), 90.0 )
}, 0.5 )

QA_WaitFor( "rail.catch_scene", function()
{
	return QA_Fired( "catch_trigger_fake", "OnStartTouch" ) == 1
}, 60.0 )

QA_WaitFor( "rail.fell", function()
{
	local s = QA_Ent( "@sphere" )
	QA_Detail( "sphere " + QA_Vec( s.GetCenter() ) + " parent=" + QA_ParentName( s ) )
	return QA_Fired( "sphere_impact_trigger", "OnStartTouch" ) >= 1 && QA_ParentName( s ) == "<none>"
}, 40.0 )

QA_WaitFor( "floor.at_rest", function()
{
	local v = QA_Ent( "@sphere" ).GetVelocity()
	QA_Detail( "speed " + format( "%.1f", v.Length() ) )
	return v.Length() < 1.0
}, 10.0 )

// Pickup is enabled by the post-fall scene; keep trying +use until it takes.
QA_WaitFor( "floor.picked_up", function()
{
	if ( QA_Fired( "@sphere", "OnPlayerPickup" ) >= 1 )
		return true
	QA_TryPickup( "@sphere", 44.0 )
	QA_Detail( "sphere " + QA_Vec( QA_Ent( "@sphere" ).GetCenter() ) + " eye " + QA_Vec( QA_Player().EyePosition() ) )
	return false
}, 40.0 )

QA_Do( "level the view", function() { QA_SetView( 0.0, ::QA.marks.yaw ) }, 1.0 )

QA_WaitFor( "carry.held", function()
{
	QA_MarkViewOffset( "held", QA_Ent( "@sphere" ) )
	local o = ::QA.marks.held
	QA_Detail( format( "view offset f=%.1f r=%.1f u=%.1f", o.f, o.r, o.u ) )
	return o.f > 20.0 && o.f < 120.0 && fabs( o.r ) < 60.0 && fabs( o.u ) < 60.0
}, 3.0 )

QA_Do( "turn", function() { QA_SetView( 0.0, ::QA.marks.yaw + 50.0 ) }, 1.0 )
QA_Expect( "carry.follows_turn", function() { return QA_HeldAt( QA_Ent( "@sphere" ), "held", 16.0 ) } )

QA_Do( "press use", function() { QA_Press( "use", 0.2 ) }, 1.5 )
QA_Expect( "carry.drop_locked", function()
{
	local drops = QA_Fired( "@sphere", "OnPlayerDrop" )
	local held = QA_HeldAt( QA_Ent( "@sphere" ), "held", 16.0 )
	QA_Detail( ::QA.detail + " drops=" + drops )
	return held && drops == 0
} )

// The pickup scene opens the plug; its trigger is enabled 1.5 s later.
QA_WaitFor( "socket.opened", function()
{
	return QA_Fired( "@plug_open_rl", "OnTrigger" ) == 1
}, 40.0 )
QA_Sleep( 2.0 )

QA_Expect( "socket.waits_for_carry", function()
{
	return QA_Fired( "InstanceAuto9-socket_powered_rl", "OnTrigger" ) == 0
} )

QA_Do( "carry into socket", function()
{
	::QA.marks.socket <- QA_Ent( "InstanceAuto9-socket_trigger" ).GetCenter()
	QA_MarkViewOffset( "carry", QA_Ent( "@sphere" ) )
	if ( !QA_ReachTo( "carry", ::QA.marks.socket, QA_Player().EyePosition() ) )
		throw "socket at " + QA_Vec( ::QA.marks.socket ) + " is out of reach"
}, 0.1 )

QA_WaitFor( "socket.trigger_fired", function()
{
	QA_Detail( "sphere " + QA_Vec( QA_Ent( "@sphere" ).GetCenter() ) + " socket " + QA_Vec( ::QA.marks.socket ) )
	return QA_Fired( "InstanceAuto9-socket_powered_rl", "OnTrigger" ) == 1
}, 5.0 )

QA_WaitFor( "socket.parented", function()
{
	local s = QA_Ent( "@sphere" )
	QA_Detail( "parent=" + QA_ParentName( s ) + " drops=" + QA_Fired( "@sphere", "OnPlayerDrop" ) )
	return QA_ParentName( s ) == "InstanceAuto9-sphere_socket" && QA_Fired( "@sphere", "OnPlayerDrop" ) == 1
}, 2.0 )

QA_WaitFor( "socket.panel_opens", function()
{
	return QA_Fired( "@panel_open_rl", "OnTrigger" ) == 1
}, 2.0 )

// Once socketed he belongs to the socket: after the player steps back and
// turns, he must not be where a held core would be.
QA_Do( "step back", function()
{
	local p = QA_Player()
	local back = QA_Basis( 0.0, p.GetAngles().y ).forward
	QA_PlaceEye( p.EyePosition() - QA_Scale( back, 64.0 ), 0.0, p.GetAngles().y + 90.0 )
}, 1.5 )

QA_Expect( "socket.released_by_player", function()
{
	local s = QA_Ent( "@sphere" )
	local held = QA_HeldAt( s, "carry", 20.0 )
	QA_Detail( ::QA.detail + " parent=" + QA_ParentName( s ) )
	return QA_ParentName( s ) == "InstanceAuto9-sphere_socket" && !held
} )

// "No watching": look away until Wheatley is satisfied, then look back.
QA_Do( "look away", function()
{
	local p = QA_Player()
	local toward = QA_AnglesTo( p.EyePosition(), ::QA.marks.socket )
	QA_SetView( 0.0, toward.yaw + 180.0 )
}, 0.5 )

QA_WaitFor( "panel.looked_away", function()
{
	return QA_Fired( "wheatley_dont_watch_aisc", "OnConditionsSatisfied" ) == 1
}, 60.0 )

QA_Do( "look back", function() { QA_LookAt( ::QA.marks.socket ) }, 0.5 )

QA_WaitFor( "panel.looked_back", function()
{
	return QA_Fired( "wheatley_watch_aisc", "OnConditionsSatisfied" ) == 1
}, 60.0 )

// The panel scene ejects him from the socket and re-enables pickup.
QA_WaitFor( "eject.released", function()
{
	local s = QA_Ent( "@sphere" )
	QA_Detail( "parent=" + QA_ParentName( s ) + " eject=" + QA_Fired( "@plug_eject_rl", "OnTrigger" ) )
	return QA_Fired( "@plug_eject_rl", "OnTrigger" ) == 1 && QA_ParentName( s ) == "<none>"
}, 30.0 )

QA_Sleep( 1.0 )
QA_WaitFor( "eject.picked_up_again", function()
{
	if ( QA_Fired( "@sphere", "OnPlayerPickup" ) >= 2 )
		return true
	QA_TryPickup( "@sphere", 44.0 )
	QA_Detail( "sphere " + QA_Vec( QA_Ent( "@sphere" ).GetCenter() ) + " eye " + QA_Vec( QA_Player().EyePosition() ) )
	return false
}, 20.0 )

// Carried out through the opened panel into the elevator hunt trigger, which
// only reacts to Wheatley himself.
QA_Do( "measure hold", function()
{
	QA_SetView( 0.0, ::QA.marks.yaw )
}, 1.0 )

QA_Do( "carry to the elevator trigger", function()
{
	QA_MarkViewOffset( "exit", QA_Ent( "@sphere" ) )
	// A trigger_once removes itself once it fires; keep where it was.
	::QA.marks.exitTrigger <- QA_Ent( "find_elevator_trigger" ).GetCenter()
	QA_CarryTo( "exit", ::QA.marks.exitTrigger, ::QA.marks.socket )
}, 0.1 )

QA_WaitFor( "exit.find_elevator", function()
{
	QA_Detail( "sphere " + QA_Vec( QA_Ent( "@sphere" ).GetCenter() ) + " trigger " + QA_Vec( ::QA.marks.exitTrigger ) )
	return QA_Fired( "find_elevator_relay", "OnTrigger" ) == 1
}, 5.0 )

QA_Expect( "exit.still_held", function()
{
	local d = QA_Dist( QA_Ent( "@sphere" ).GetCenter(), QA_Player().EyePosition() )
	QA_Detail( format( "eye distance %.1f", d ) + " drops=" + QA_Fired( "@sphere", "OnPlayerDrop" ) )
	return d < 120.0 && QA_Fired( "@sphere", "OnPlayerDrop" ) == 1
} )

QA_Start( "sp_a1_intro7" )
