// sp_a1_wakeup played through on the player's own input (qa_walker.nut):
// every walk is planned in the running game and driven with keys and turn
// rates; nothing moves or turns the player except input and the map itself.
// A screenshot is taken at each story beat (QA_SHOT); the harness pairs the
// shots of this build and retail Portal 2 by name.
//
// Beats: arrival carrying Wheatley, the observation hall, the gantry door
// opening onto GLaDOS' chamber, "there she is", the chamber's far door, the
// stairwell, the jump down the shaft and the landing, the basement catwalk
// ("don't look down"), the breaker room and its cage, Wheatley plugged into
// the socket, the breakers switching and the platform rising into the
// chamber, GLaDOS waking, the claw pickup, the incinerator drop and the level
// change.

IncludeScript( "qa/qa_driver" )
IncludeScript( "qa/qa_walker" )
IncludeScript( "qa/qa_clips" )

::WAKE <- {
	shots = 0
	glados = Vector( 9392, 1184, 760 )
	times = {}
	handlers = 0
	plug = { phase = "settle", last = null, steady = 0, aim = null, nextAim = 0.0 }
}

function Wake_Shot( name )
{
	::WAKE.shots++
	local label = format( "%02d_", ::WAKE.shots ) + name
	local player = QA_Player()
	printl( "QA_SHOT " + ::QA.scenario + " " + label )
	// The view the walker keeps from its inputs (the server has no pitch).
	QA_Log( "shot " + label + " feet " + QA_Vec( player.GetOrigin() ) +
	        format( " pitch %.1f yaw %.1f", ::WALK.view.pitch, ::WALK.view.yaw ) )
	SendToConsole( "screenshot" )
}

function Wake_SphereHeld()
{
	local s = QA_Ent( "@sphere" )
	local d = QA_Dist( s.GetCenter(), QA_Player().EyePosition() )
	QA_Detail( format( "sphere %.1f from the eye", d ) + " drops=" + QA_Fired( "@sphere", "OnPlayerDrop" ) +
	           " parent=" + QA_ParentName( s ) )
	return d < 120.0
}

// Walk to a goal, then check it was reached still carrying Wheatley.
function Wake_Walk( check, goal, radius, timeout )
{
	QA_Do( "walk " + check, function() : ( check, goal, radius )
	{
		Walk_To( check, goal, radius )
	}, 0.1 )
	QA_WaitFor( check, function() { return Walk_Arrived() }, timeout )
}

// A beat seen from where the player stands: look, settle, shoot.
function Wake_ShotAt( name, point )
{
	// A function stands for a point only known when the beat comes.
	QA_Do( "look " + name, function() : ( point )
	{
		Walk_LookAt( typeof point == "function" ? point() : point )
	}, 0.1 )
	QA_WaitFor( "view." + name, function() { return Walk_Looked() }, 8.0 )
	QA_Do( "shoot " + name, function() : ( name ) { Wake_Shot( name ) }, 1.0 )
}

function Wake_ShotDir( name, pitch, yaw )
{
	QA_Do( "look " + name, function() : ( pitch, yaw ) { Walk_LookDir( pitch, yaw ) }, 0.1 )
	QA_WaitFor( "view." + name, function() { return Walk_Looked() }, 8.0 )
	QA_Do( "shoot " + name, function() : ( name ) { Wake_Shot( name ) }, 1.0 )
}

// GLaDOS' eye: the map parents its glow sprite to her eye attachment, so the
// sprite's origin follows her head as she wakes.
function Wake_GladosView()
{
	local glow = Entities.FindByName( null, "glados_eyeglow" )
	local eye = glow == null ? ::WAKE.glados : glow.GetOrigin()
	QA_Log( "glados eye " + QA_Vec( eye ) )
	return eye
}

// When each watched map output first fired (server time).
function Wake_Mark( name, output )
{
	local ent = QA_Ent( name )
	local key = name + ":" + output
	local handler = "Wake_Time_" + ::WAKE.handlers++
	ent.ValidateScriptScope()
	ent.GetScriptScope()[handler] <- function() : ( key )
	{
		if ( !( key in ::WAKE.times ) )
		{
			::WAKE.times[key] <- Time()
			QA_Log( "event " + key )
		}
	}
	ent.ConnectOutput( output, handler )
}

// A scripted beat: <delay> s after <event> first fired, look at <look> (a
// point, a function returning one, [ pitch, yaw ], or null to leave the view
// to the map's camera) and shoot.
function Wake_EventShot( name, event, delay, look )
{
	QA_WaitFor( "event." + name, function() : ( event )
	{
		QA_Detail( event + ( ( event in ::WAKE.times ) ? " fired" : " not fired" ) )
		return event in ::WAKE.times
	}, 240.0 )
	QA_Do( "look " + name, function() : ( look )
	{
		if ( look == null )
			Walk_Stop()
		else if ( typeof look == "array" )
			Walk_LookDir( look[0], look[1] )
		else
			Walk_LookAt( typeof look == "function" ? look() : look )
	}, 0.05 )
	QA_WaitFor( "view." + name, function() : ( event, delay, look )
	{
		local due = ::WAKE.times[event] + delay
		QA_Detail( format( "due %.2f now %.2f ", due, Time() ) + ::WALK.state )
		return Time() >= due && ( look == null || Walk_Looked() )
	}, delay + 12.0 )
	QA_Do( "shoot " + name, function() : ( name ) { Wake_Shot( name ) }, 0.3 )
}

// The socket, one step per tick of socket.plugged: wait for it to stop
// rising, then walk up to it and aim Wheatley into it, unless he is in it.
function Wake_Plug()
{
	local plug = ::WAKE.plug
	if ( "socket_powered_rl:OnTrigger" in ::WAKE.times )
	{
		QA_Detail( "plugged while " + plug.phase )
		return true
	}
	local trigger = Entities.FindByName( null, "basement_breakers_socket_trigger" )
	if ( trigger == null )
	{
		QA_Detail( "no socket trigger" )
		return false
	}
	local socket = trigger.GetCenter()
	local sphere = QA_Ent( "@sphere" ).GetCenter()
	QA_Detail( plug.phase + " sphere " + QA_Vec( sphere ) + " socket " + QA_Vec( socket ) )
	if ( plug.phase == "settle" )
	{
		plug.steady = ( plug.last != null && QA_Dist( plug.last, socket ) < 0.25 ) ? plug.steady + 1 : 0
		plug.last = socket
		if ( plug.steady < 10 )
			return false
		// The core's distance from the eye while held.
		local eye = QA_Player().EyePosition()
		local hold = QA_Dist( sphere, eye )
		local dz = eye.z - socket.z
		local horizontal = hold * hold > dz * dz ? sqrt( hold * hold - dz * dz ) : 8.0
		local from = QA_Player().GetOrigin() - socket
		local length = sqrt( from.x * from.x + from.y * from.y )
		local stand = socket + Vector( from.x / length * horizontal, from.y / length * horizontal, 0.0 )
		stand.z = QA_Player().GetOrigin().z + 40.0
		QA_Log( format( "socket settled; hold %.1f, stand %.1f away", hold, horizontal ) )
		Walk_To( "socket_stand", stand, 10.0 )
		plug.phase = "walk"
		return false
	}
	if ( plug.phase == "walk" )
	{
		if ( !Walk_Arrived() )
			return false
		plug.aim = socket
		plug.nextAim = Time() + 0.8
		Walk_LookAt( plug.aim )
		plug.phase = "aim"
		return false
	}
	if ( Time() >= plug.nextAim && ::WALK.state == "looked" )
	{
		plug.aim = plug.aim + QA_Scale( socket - sphere, 0.7 )
		plug.nextAim = Time() + 0.8
		Walk_LookAt( plug.aim )
		QA_Log( "aim " + QA_Vec( plug.aim ) + " sphere " + QA_Vec( sphere ) )
	}
	return false
}

// Both builds start the walk at the same map time (the build's driver starts
// after a frame count, retail's a second after the map spawns).
QA_WaitFor( "start.map_time", function()
{
	QA_Detail( format( "t=%.2f", Time() ) )
	return Time() >= 6.0
}, 30.0 )

QA_Do( "setup", function()
{
	SendToConsole( "con_drawnotify 0; cl_drawhud 0; hud_quickinfo 0; developer 0" )
	Walk_Init( "@sphere" )
	QA_Watch( "@sphere", "OnPlayerPickup" )
	QA_Watch( "@sphere", "OnPlayerDrop" )
	QA_Watch( "training_door", "OnOpen" )
	QA_Watch( "basement_breakers_socket_relay", "OnTrigger" )
	Wake_Mark( "socket_powered_rl", "OnTrigger" )
	Wake_Mark( "basement_breakers_start", "OnTrigger" )
	Wake_Mark( "basement_breakers_up", "OnTrigger" )
	Wake_Mark( "basement_elevator_complete_rl", "OnTrigger" )
	Wake_Mark( "relay_start_claw_pickup", "OnTrigger" )
	Wake_Mark( "ghostAnim", "OnAnimationDone" )
	Wake_Mark( "@transition_from_map", "OnTrigger" )
	QA_Watch( "basement_breaker_room_entry_trigger", "OnTrigger" )
	QA_Watch( "do_not_touch_anything_trigger", "OnTrigger" )
	QA_WatchEntity( QA_EntNear( "trigger_once", Vector( 10888, 1196, -263.86 ) ), "landed", "OnTrigger" )
}, 0.5 )

// 1. The map starts with Wheatley forced into the player's hands.
QA_WaitFor( "arrival.holding", function() { return Wake_SphereHeld() }, 15.0 )
Wake_ShotDir( "arrival", 0.0, 90.0 )

// 2. North through the entry door into the observation hall.
Wake_Walk( "observation.reached", Vector( 6976, 900, 440 ), 48.0, 60.0 )
Wake_ShotDir( "observation", 0.0, 90.0 )

// 3. The hallway east towards the chamber.
Wake_Walk( "hallway.reached", Vector( 7000, 1216, 440 ), 48.0, 60.0 )
Wake_ShotDir( "hallway", 0.0, 0.0 )

// 4. The gantry door: stepping up to it makes GLaDOS' chamber door open.
Wake_Walk( "gantry.reached", Vector( 8040, 1216, 440 ), 40.0, 90.0 )
QA_WaitFor( "gantry.door_opens", function()
{
	return QA_Fired( "training_door", "OnOpen" ) >= 1
}, 60.0 )
QA_Sleep( 1.5 )
Wake_ShotAt( "gantry_door", Vector( 8400, 1216, 470 ) )

// 5. Into the chamber: "there she is".
Wake_Walk( "chamber.reached", Vector( 8876, 1216, 440 ), 48.0, 60.0 )
Wake_ShotAt( "there_she_is", ::WAKE.glados )

// 6. Across the chamber to its far door.
Wake_Walk( "chamber_exit.reached", Vector( 10470, 1320, 440 ), 40.0, 90.0 )
Wake_ShotDir( "chamber_exit", 0.0, 0.0 )

// 7. The top of the stairwell.
Wake_Walk( "stairs.reached", Vector( 10828, 1344, 440 ), 40.0, 90.0 )
Wake_ShotDir( "stairs", 20.0, 270.0 )

// 8. The lip of the shaft Wheatley asks the player to jump down.
Wake_Walk( "jump_edge.reached", Vector( 10944, 1030, 300 ), 36.0, 120.0 )
Wake_ShotDir( "jump_edge", 50.0, 90.0 )

// 9. Down the shaft to the basement.
Wake_Walk( "landed.reached", Vector( 10900, 1240, -240 ), 24.0, 60.0 )
QA_Expect( "landed.trigger", function() { return QA_Fired( "landed", "OnTrigger" ) >= 1 } )
Wake_ShotDir( "landed", 0.0, 180.0 )

// 10. West along the basement ("come through here").
Wake_Walk( "basement.reached", Vector( 10620, 1250, -300 ), 48.0, 90.0 )
Wake_ShotDir( "basement", 0.0, 180.0 )

// 11. The catwalk over the pit: "don't look down".
Wake_Walk( "catwalk.reached", Vector( 9990, 1228, -300 ), 40.0, 90.0 )
Wake_ShotDir( "catwalk", 45.0, 180.0 )

// 12. The breaker room's door.
Wake_Walk( "breaker_room.reached", Vector( 8976, 1236, -440 ), 16.0, 120.0 )
QA_WaitFor( "breaker_room.entry_trigger", function()
{
	return QA_Fired( "basement_breaker_room_entry_trigger", "OnTrigger" ) >= 1
}, 5.0 )
Wake_ShotAt( "breaker_room", Vector( 8976, 1088, -420 ) )

// 13. Into the cage: "don't touch anything".
Wake_Walk( "cage.reached", Vector( 8976, 1112, -440 ), 20.0, 60.0 )
QA_WaitFor( "cage.trigger", function()
{
	return QA_Fired( "do_not_touch_anything_trigger", "OnTrigger" ) >= 1
}, 5.0 )
Wake_ShotDir( "cage", 10.0, 270.0 )

// 14. The socket rises out of the hatch under Wheatley. If he is not in it
// once it has stopped, walk to where the held core reaches it and look at
// it, correcting the aim by where he actually is, as a player lines a held
// object up with a target.
QA_WaitFor( "socket.opens", function()
{
	return QA_Fired( "basement_breakers_socket_relay", "OnTrigger" ) >= 1
}, 90.0 )
QA_WaitFor( "socket.plugged", function() { return Wake_Plug() }, 45.0 )
QA_Do( "stop", function() { Walk_Stop() }, 0.1 )

// 15. Scripted beats, each shot a fixed time after the map's own output
// fires, so both builds shoot the same moment.
Wake_EventShot( "plugged", "socket_powered_rl:OnTrigger", 1.5, function()
{
	return QA_Ent( "@sphere" ).GetCenter()
} )
Wake_EventShot( "breakers", "basement_breakers_start:OnTrigger", 5.0, [ -20.0, 270.0 ] )
QA_Expect( "breakers.start", function() { return "basement_breakers_start:OnTrigger" in ::WAKE.times } )
Wake_EventShot( "platform_rising", "basement_breakers_up:OnTrigger", 5.0, [ -60.0, 270.0 ] )
QA_Expect( "platform.rises", function() { return "basement_breakers_up:OnTrigger" in ::WAKE.times } )

// 16. The platform stops in GLaDOS' chamber and she wakes: look her in the
// eye. From the lift the shaft hides her on both builds; her face comes into
// view in the claw's camera ride (17).
QA_WaitFor( "platform.arrived", function()
{
	return "basement_elevator_complete_rl:OnTrigger" in ::WAKE.times
}, 60.0 )
Wake_EventShot( "glados_asleep", "basement_elevator_complete_rl:OnTrigger", 2.0, Wake_GladosView )
Wake_EventShot( "glados_wakes", "basement_elevator_complete_rl:OnTrigger", 16.0, Wake_GladosView )
Wake_EventShot( "glados_its_you", "basement_elevator_complete_rl:OnTrigger", 32.0, Wake_GladosView )

// 17. The claw takes the player; the camera rides along. Then the
// incinerator, and down it: the level ends.
Wake_EventShot( "claw", "relay_start_claw_pickup:OnTrigger", 3.0, null )
QA_Expect( "claw.pickup", function() { return "relay_start_claw_pickup:OnTrigger" in ::WAKE.times } )
Wake_EventShot( "claw_glados", "relay_start_claw_pickup:OnTrigger", 12.0, null )
Wake_EventShot( "claw_carry", "relay_start_claw_pickup:OnTrigger", 24.0, null )
Wake_EventShot( "incinerator", "ghostAnim:OnAnimationDone", 0.3, null )
QA_Expect( "incinerator.drop", function() { return "ghostAnim:OnAnimationDone" in ::WAKE.times } )
// The level change follows within a second and resets the VM: shoot and
// finish at once.
QA_WaitFor( "level.ends", function()
{
	return "@transition_from_map:OnTrigger" in ::WAKE.times
}, 60.0 )
QA_Do( "shoot falling", function() { Wake_Shot( "falling" ) }, 0.01 )

QA_Start( "sp_a1_wakeup_walk" )
