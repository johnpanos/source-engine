// sp_a1_wakeup: the player arrives carrying Wheatley (logic_auto ForcePickup),
// cannot drop him, carries him down the stairwell where the map forces the
// physics grab controller, and plugs him into the breaker room socket.
// Key events are reached with the map's own @debug_warp_* relays.

IncludeScript( "qa/qa_driver" )

QA_Do( "watch outputs", function()
{
	QA_Watch( "@sphere", "OnPlayerPickup" )
	QA_Watch( "@sphere", "OnPlayerDrop" )
	QA_Watch( "socket_powered_rl", "OnTrigger" )
	QA_Watch( "basement_breakers_start", "OnTrigger" )
	::QA.marks.startYaw <- QA_Player().GetAngles().y
	QA_SetView( 0.0, ::QA.marks.startYaw )
}, 1.0 )

// Map start: ForcePickup put him in front of the view.
QA_WaitFor( "spawn.held", function()
{
	QA_MarkViewOffset( "vm", QA_Ent( "@sphere" ) )
	local o = ::QA.marks.vm
	QA_Detail( format( "view offset f=%.1f r=%.1f u=%.1f", o.f, o.r, o.u ) )
	return o.f > 20.0 && o.f < 120.0 && fabs( o.r ) < 60.0 && fabs( o.u ) < 60.0
}, 3.0 )

QA_Do( "turn left", function() { QA_SetView( 0.0, ::QA.marks.startYaw + 60.0 ) }, 1.0 )
QA_Expect( "spawn.follows_turn", function() { return QA_HeldAt( QA_Ent( "@sphere" ), "vm", 16.0 ) } )

// Standing still long enough for the physics engine to sleep an idle body.
QA_Sleep( 4.0 )
QA_Do( "turn back", function() { QA_SetView( 0.0, ::QA.marks.startYaw ) }, 1.0 )
QA_Expect( "spawn.follows_after_idle", function() { return QA_HeldAt( QA_Ent( "@sphere" ), "vm", 16.0 ) } )

// OnPlayerPickup locks dropping: +use must not let go of him.
QA_Do( "press use", function() { QA_Press( "use", 0.2 ) }, 1.5 )
QA_Expect( "spawn.drop_locked", function()
{
	local drops = QA_Fired( "@sphere", "OnPlayerDrop" )
	local held = QA_HeldAt( QA_Ent( "@sphere" ), "vm", 16.0 )
	QA_Detail( ::QA.detail + " drops=" + drops )
	return held && drops == 0
} )

// Stairwell: the map warps player and core here while held.
QA_Do( "warp to stairwell", function() { QA_Fire( "@debug_warp_to_stairwell", "Trigger" ) }, 2.0 )
QA_WaitFor( "stairwell.still_held", function()
{
	QA_SetView( 0.0, QA_Player().GetAngles().y )
	local s = QA_Ent( "@sphere" )
	local d = QA_Dist( s.GetCenter(), QA_Player().EyePosition() )
	QA_Detail( format( "eye distance=%.1f", d ) + " drops=" + QA_Fired( "@sphere", "OnPlayerDrop" ) )
	return d < 120.0 && QA_Fired( "@sphere", "OnPlayerDrop" ) == 0
}, 3.0 )

// Bottom of the stairwell: the map forces the physics grab controller when
// the player walks into this trigger, which moves him from the view model
// pose to a physically simulated hold at eye height.
QA_Do( "walk into grab trigger", function()
{
	local trigger = QA_EntNear( "trigger_once", Vector( 10380, 1208, 99.24 ) )
	QA_WatchEntity( trigger, "stairs_grab_trigger", "OnStartTouch" )
	local p = QA_Player()
	local yaw = p.GetAngles().y
	QA_PlaceEye( trigger.GetCenter() + QA_EyeOffset(), 0.0, yaw )
}, 2.0 )

QA_WaitFor( "stairwell.physics_hold", function()
{
	local s = QA_Ent( "@sphere" )
	QA_MarkViewOffset( "phys", s )
	local o = ::QA.marks.phys
	QA_Detail( format( "view offset f=%.1f r=%.1f u=%.1f", o.f, o.r, o.u ) + " touched=" +
	           QA_Fired( "stairs_grab_trigger", "OnStartTouch" ) + " drops=" +
	           QA_Fired( "@sphere", "OnPlayerDrop" ) )
	local vm = ::QA.marks.vm
	local change = sqrt( ( o.f - vm.f ) * ( o.f - vm.f ) + ( o.u - vm.u ) * ( o.u - vm.u ) )
	return QA_Fired( "stairs_grab_trigger", "OnStartTouch" ) == 1 && o.f > 20.0 && o.f < 120.0 &&
	       change > 5.0 && QA_Fired( "@sphere", "OnPlayerDrop" ) == 0
}, 3.0 )

QA_Do( "turn with physics hold", function()
{
	QA_SetView( 0.0, QA_Player().GetAngles().y - 45.0 )
}, 1.5 )
QA_Expect( "stairwell.physics_follows_turn", function() { return QA_HeldAt( QA_Ent( "@sphere" ), "phys", 16.0 ) } )

// The breaker room: warp to its floor with the map's relay, then walk into
// the breaker cage. Carrying Wheatley past the entry trigger opens the hatch;
// touching the cage floor trigger starts Wheatley's "don't touch anything"
// scenes, and the last of them opens the socket.
QA_Do( "warp to breaker room", function()
{
	QA_Watch( "basement_breakers_socket_relay", "OnTrigger" )
	QA_Watch( "basement_breaker_room_entry_trigger", "OnTrigger" )
	QA_Watch( "do_not_touch_anything_trigger", "OnTrigger" )
	QA_Fire( "@debug_warp_to_elevator", "Trigger" )
}, 1.5 )

QA_WaitFor( "breaker.arrived_holding", function()
{
	local p = QA_Player()
	local d = QA_Dist( QA_Ent( "@sphere" ).GetCenter(), p.EyePosition() )
	QA_Detail( "player " + QA_Vec( p.GetOrigin() ) + format( " eye distance=%.1f", d ) )
	return p.GetOrigin().z < -440.0 && d < 120.0 && QA_Fired( "@sphere", "OnPlayerDrop" ) == 0
}, 5.0 )

QA_Do( "face the cage", function()
{
	local p = QA_Player()
	QA_PlaceEye( Vector( 8976, 1300, p.EyePosition().z ), 0.0, 270.0 )
}, 1.0 )

// He has to stay low under the door header and then rise above the socket,
// which comes up out of the hatch as he passes the entry trigger. Looking up
// before the eye is through the door would put the header between eye and
// core, which the grab controller treats as an obstruction and lets go.
QA_Do( "walk into the cage", function()
{
	QA_Trace( "@sphere", 3.0 )
	QA_Press( "forward", 1.6 )
}, 0.05 )

QA_WaitFor( "breaker.through_door", function()
{
	local eye = QA_Player().EyePosition()
	QA_Detail( "eye " + QA_Vec( eye ) + " drops=" + QA_Fired( "@sphere", "OnPlayerDrop" ) )
	if ( eye.y > 1176.0 )
		return false
	QA_SetView( -40.0, 270.0 )
	return QA_Fired( "@sphere", "OnPlayerDrop" ) == 0
}, 3.0 )

QA_Sleep( 2.0 )

QA_WaitFor( "breaker.entered_holding", function()
{
	local p = QA_Player()
	local d = QA_Dist( QA_Ent( "@sphere" ).GetCenter(), p.EyePosition() )
	QA_Detail( "player " + QA_Vec( p.GetOrigin() ) + format( " eye distance=%.1f", d ) +
	           " entry=" + QA_Fired( "basement_breaker_room_entry_trigger", "OnTrigger" ) +
	           " floor=" + QA_Fired( "do_not_touch_anything_trigger", "OnTrigger" ) )
	return QA_Fired( "basement_breaker_room_entry_trigger", "OnTrigger" ) == 1 &&
	       QA_Fired( "do_not_touch_anything_trigger", "OnTrigger" ) == 1 && d < 120.0 &&
	       QA_Fired( "@sphere", "OnPlayerDrop" ) == 0
}, 3.0 )

QA_WaitFor( "breaker.scene_opens_socket", function()
{
	return QA_Fired( "basement_breakers_socket_relay", "OnTrigger" ) == 1
}, 60.0 )

QA_Expect( "breaker.socket_waits_for_carry", function()
{
	::QA.marks.socket <- QA_Ent( "basement_breakers_socket_trigger" ).GetCenter()
	QA_Detail( "sphere " + QA_Vec( QA_Ent( "@sphere" ).GetCenter() ) + " socket " + QA_Vec( ::QA.marks.socket ) )
	return QA_Fired( "socket_powered_rl", "OnTrigger" ) == 0
} )

// The trigger is enabled 2 s after the relay; the socket model rises with
// the hatch, so wait until it stops moving before aiming at it.
QA_Sleep( 2.2 )
QA_WaitFor( "breaker.socket_settled", function()
{
	local now = QA_Ent( "basement_breakers_socket_trigger" ).GetCenter()
	local moved = QA_Dist( now, ::QA.marks.socket )
	::QA.marks.socket <- now
	QA_Detail( "socket " + QA_Vec( now ) + format( " moved %.2f", moved ) )
	return moved < 0.25
}, 10.0 )

QA_Do( "measure hold", function() { QA_MarkViewOffset( "carry", QA_Ent( "@sphere" ) ) }, 0.2 )

QA_Do( "carry into socket", function()
{
	if ( !QA_ReachTo( "carry", ::QA.marks.socket, QA_Player().EyePosition() ) )
		throw "socket at " + QA_Vec( ::QA.marks.socket ) + " is out of reach"
}, 0.1 )

QA_WaitFor( "socket.trigger_fired", function()
{
	QA_Detail( "sphere " + QA_Vec( QA_Ent( "@sphere" ).GetCenter() ) + " socket " + QA_Vec( ::QA.marks.socket ) )
	return QA_Fired( "socket_powered_rl", "OnTrigger" ) == 1
}, 5.0 )

QA_WaitFor( "socket.parented", function()
{
	local s = QA_Ent( "@sphere" )
	QA_Detail( "parent=" + QA_ParentName( s ) + " drops=" + QA_Fired( "@sphere", "OnPlayerDrop" ) )
	return QA_ParentName( s ) == "core_receptacle_socket" && QA_Fired( "@sphere", "OnPlayerDrop" ) == 1
}, 2.0 )

QA_WaitFor( "socket.breakers_start", function()
{
	return QA_Fired( "basement_breakers_start", "OnTrigger" ) == 1
}, 4.0 )

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
	return QA_ParentName( s ) == "core_receptacle_socket" && !held
} )

QA_Start( "sp_a1_wakeup" )
