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

// The breaker room: warp into the elevator shaft, which enables the socket.
QA_Do( "warp inside elevator", function() { QA_Fire( "@debug_warp_inside_of_elevator", "Trigger" ) }, 4.0 )
QA_WaitFor( "elevator.landed_holding", function()
{
	local p = QA_Player()
	local s = QA_Ent( "@sphere" )
	QA_SetView( 0.0, p.GetAngles().y )
	local d = QA_Dist( s.GetCenter(), p.EyePosition() )
	QA_Detail( "player " + QA_Vec( p.GetOrigin() ) + format( " eye distance=%.1f", d ) )
	return p.GetOrigin().z < -560.0 && d < 120.0
}, 8.0 )

QA_Do( "measure hold", function() { QA_MarkViewOffset( "carry", QA_Ent( "@sphere" ) ) }, 0.2 )

QA_Do( "carry into socket", function()
{
	local trigger = QA_Ent( "basement_breakers_socket_trigger" )
	::QA.marks.socketFrom <- QA_Player().EyePosition()
	QA_CarryTo( "carry", trigger.GetCenter(), ::QA.marks.socketFrom )
}, 0.1 )

QA_WaitFor( "socket.trigger_fired", function()
{
	QA_Detail( "sphere " + QA_Vec( QA_Ent( "@sphere" ).GetCenter() ) + " socket trigger " +
	           QA_Vec( QA_Ent( "basement_breakers_socket_trigger" ).GetCenter() ) )
	return QA_Fired( "socket_powered_rl", "OnTrigger" ) == 1
}, 5.0 )

QA_WaitFor( "socket.parented", function()
{
	local s = QA_Ent( "@sphere" )
	QA_Detail( "parent=" + QA_ParentName( s ) )
	return QA_ParentName( s ) == "core_receptacle_socket"
}, 2.0 )

QA_WaitFor( "socket.breakers_start", function()
{
	return QA_Fired( "basement_breakers_start", "OnTrigger" ) == 1
}, 4.0 )

// Once socketed he belongs to the socket, not the player.
QA_Do( "step back", function()
{
	::QA.marks.socketed <- QA_Ent( "@sphere" ).GetCenter()
	local p = QA_Player()
	local back = QA_Basis( 0.0, p.GetAngles().y ).forward
	QA_PlaceEye( p.EyePosition() - QA_Scale( back, 64.0 ), 0.0, p.GetAngles().y + 90.0 )
}, 1.5 )

QA_Expect( "socket.released_by_player", function()
{
	local moved = QA_Dist( QA_Ent( "@sphere" ).GetCenter(), ::QA.marks.socketed )
	QA_Detail( format( "moved %.1f after the player stepped away", moved ) )
	return moved < 8.0
} )

QA_Start( "sp_a1_wakeup" )
