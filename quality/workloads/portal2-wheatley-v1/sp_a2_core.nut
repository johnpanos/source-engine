// sp_a2_core: the core transfer. Wheatley drops out of his tube, the core
// receptacle rises out of the floor and turns to track the player, and the
// player plugs Wheatley into it. Valve's debug_wheatley_spawn relay stands in
// for the stalemate sequence that leads here: it frees Wheatley, deploys the
// receptacle and puts the player next to them.

IncludeScript( "qa/qa_driver" )

QA_Do( "watch outputs", function()
{
	QA_Watch( "@sphere", "OnPlayerPickup" )
	QA_Watch( "@sphere", "OnPlayerDrop" )
	QA_Watch( "core_receptacle_vertical_mover_1", "OnFullyOpen" )
	QA_Watch( "socket_powered_rl", "OnTrigger" )
	QA_Watch( "core_socketed_relay", "OnTrigger" )
}, 0.5 )

QA_Expect( "tube.attached", function()
{
	local s = QA_Ent( "@sphere" )
	QA_Detail( "parent=" + QA_ParentName( s ) )
	return QA_ParentName( s ) == "tube_sphere_rotating"
} )

QA_Do( "free Wheatley and deploy the receptacle", function()
{
	QA_Fire( "debug_wheatley_spawn", "Trigger" )
}, 1.0 )

QA_WaitFor( "floor.landed", function()
{
	local s = QA_Ent( "@sphere" )
	local v = s.GetVelocity().Length()
	QA_Detail( "sphere " + QA_Vec( s.GetCenter() ) + format( " speed %.1f", v ) + " parent=" + QA_ParentName( s ) )
	return QA_ParentName( s ) == "<none>" && v < 1.0 && s.GetCenter().z < -20.0
}, 10.0 )

QA_WaitFor( "receptacle.deployed", function()
{
	return QA_Fired( "core_receptacle_vertical_mover_1", "OnFullyOpen" ) == 1
}, 30.0 )

QA_WaitFor( "floor.picked_up", function()
{
	if ( QA_Fired( "@sphere", "OnPlayerPickup" ) >= 1 )
		return true
	QA_TryPickup( "@sphere", 44.0 )
	QA_Detail( "sphere " + QA_Vec( QA_Ent( "@sphere" ).GetCenter() ) + " eye " + QA_Vec( QA_Player().EyePosition() ) )
	return false
}, 20.0 )

QA_Do( "level the view", function() { QA_SetView( 0.0, ::QA.marks.yaw ) }, 1.0 )

QA_WaitFor( "carry.held", function()
{
	QA_MarkViewOffset( "held", QA_Ent( "@sphere" ) )
	local o = ::QA.marks.held
	QA_Detail( format( "view offset f=%.1f r=%.1f u=%.1f", o.f, o.r, o.u ) )
	return o.f > 20.0 && o.f < 120.0 && fabs( o.r ) < 60.0 && fabs( o.u ) < 60.0
}, 3.0 )

QA_Do( "turn", function() { QA_SetView( 0.0, ::QA.marks.yaw - 50.0 ) }, 1.0 )
QA_Expect( "carry.follows_turn", function() { return QA_HeldAt( QA_Ent( "@sphere" ), "held", 16.0 ) } )

QA_Do( "press use", function() { QA_Press( "use", 0.2 ) }, 1.5 )
QA_Expect( "carry.drop_locked", function()
{
	local drops = QA_Fired( "@sphere", "OnPlayerDrop" )
	local held = QA_HeldAt( QA_Ent( "@sphere" ), "held", 16.0 )
	QA_Detail( ::QA.detail + " drops=" + drops )
	return held && drops == 0
} )

// The socket trigger is enabled 1.8 s after the receptacle finishes rising.
QA_Expect( "receptacle.waits_for_carry", function()
{
	return QA_Fired( "socket_powered_rl", "OnTrigger" ) == 0
} )

// The receptacle keeps turning toward the player, so aim again each second
// until the socket takes him.
QA_WaitFor( "receptacle.trigger_fired", function()
{
	if ( QA_Fired( "socket_powered_rl", "OnTrigger" ) == 1 )
		return true
	if ( !( "nextAim" in ::QA.marks ) || Time() >= ::QA.marks.nextAim )
	{
		local trigger = Entities.FindByName( null, "socket_trigger" )
		if ( trigger != null )
		{
			::QA.marks.socket <- trigger.GetCenter()
			QA_MarkViewOffset( "carry", QA_Ent( "@sphere" ) )
			if ( !QA_ReachTo( "carry", ::QA.marks.socket, QA_Player().EyePosition() ) )
				throw "receptacle at " + QA_Vec( ::QA.marks.socket ) + " is out of reach"
		}
		::QA.marks.nextAim <- Time() + 1.0
	}
	QA_Detail( "sphere " + QA_Vec( QA_Ent( "@sphere" ).GetCenter() ) + " socket " + QA_Vec( ::QA.marks.socket ) )
	return false
}, 12.0 )

QA_WaitFor( "receptacle.parented", function()
{
	local s = QA_Ent( "@sphere" )
	QA_Detail( "parent=" + QA_ParentName( s ) )
	return QA_ParentName( s ) == "core_receptacle_socket"
}, 3.0 )

QA_WaitFor( "receptacle.core_socketed", function()
{
	return QA_Fired( "core_socketed_relay", "OnTrigger" ) == 1
}, 2.0 )

// Once socketed he belongs to the socket: after the player steps back and
// turns, he must not be where a held core would be.
QA_Do( "step back", function()
{
	local p = QA_Player()
	local back = QA_Basis( 0.0, p.GetAngles().y ).forward
	QA_PlaceEye( p.EyePosition() - QA_Scale( back, 64.0 ), 0.0, p.GetAngles().y + 90.0 )
}, 1.5 )

QA_Expect( "receptacle.released_by_player", function()
{
	local s = QA_Ent( "@sphere" )
	local held = QA_HeldAt( s, "carry", 20.0 )
	QA_Detail( ::QA.detail + " parent=" + QA_ParentName( s ) )
	return QA_ParentName( s ) == "core_receptacle_socket" && !held
} )

QA_Start( "sp_a2_core" )
