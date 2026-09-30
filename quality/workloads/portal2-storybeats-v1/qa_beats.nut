// Story beats for a walkthrough on the player's own input (qa_walker.nut):
// named screenshots, walks, looks, and shots timed from the map's own
// outputs. Shared by the story-beat scenarios (tools/quality/
// portal2_storybeats.py pairs the shots of this build and retail by name).
//
// Squirrel 2.2 (retail Portal 2): state lives in the root table ::BEATS.

::BEATS <- {
	shots = 0
	times = {}
	handlers = 0
	pressed = null
}

// A named screenshot, with where the player stood and the view the walker
// keeps from its inputs (the server has no pitch).
function Beat_Shot( name )
{
	::BEATS.shots++
	local label = format( "%02d_", ::BEATS.shots ) + name
	local player = QA_Player()
	printl( "QA_SHOT " + ::QA.scenario + " " + label )
	QA_Log( "shot " + label + " feet " + QA_Vec( player.GetOrigin() ) +
	        format( " pitch %.1f yaw %.1f", ::WALK.view.pitch, ::WALK.view.yaw ) )
	SendToConsole( "screenshot" )
}

// Walk to a goal; <check> passes once the walker has arrived.
function Beat_Walk( check, goal, radius, timeout )
{
	QA_Do( "walk " + check, function() : ( check, goal, radius )
	{
		Walk_To( check, goal, radius )
	}, 0.1 )
	QA_WaitFor( check, function() { return Walk_Arrived() }, timeout )
}

// The last step straight on the keys (qa_walker.nut Walk_Straight).
function Beat_WalkStraight( check, goal, radius, timeout )
{
	QA_Do( "walk " + check, function() : ( check, goal, radius )
	{
		Walk_Straight( check, goal, radius )
	}, 0.1 )
	QA_WaitFor( check, function() { return Walk_Arrived() }, timeout )
}

// Look at <point> (a function stands for a point only known when the step
// runs); view.<name> passes once the view has settled on it.
function Beat_Look( name, point )
{
	QA_Do( "look " + name, function() : ( point )
	{
		Walk_LookAt( typeof point == "function" ? point() : point )
	}, 0.1 )
	QA_WaitFor( "view." + name, function() { return Walk_Looked() }, 8.0 )
}

// A beat seen from where the player stands: look, settle, shoot.
function Beat_ShotAt( name, point )
{
	Beat_Look( name, point )
	QA_Do( "shoot " + name, function() : ( name ) { Beat_Shot( name ) }, 1.0 )
}

function Beat_ShotDir( name, pitch, yaw )
{
	QA_Do( "look " + name, function() : ( pitch, yaw ) { Walk_LookDir( pitch, yaw ) }, 0.1 )
	QA_WaitFor( "view." + name, function() { return Walk_Looked() }, 8.0 )
	QA_Do( "shoot " + name, function() : ( name ) { Beat_Shot( name ) }, 1.0 )
}

// When each watched map output first fired (server time).
function Beat_Mark( name, output )
{
	local ent = QA_Ent( name )
	local key = name + ":" + output
	local handler = "Beat_Time_" + ::BEATS.handlers++
	ent.ValidateScriptScope()
	ent.GetScriptScope()[handler] <- function() : ( key )
	{
		if ( !( key in ::BEATS.times ) )
		{
			::BEATS.times[key] <- Time()
			QA_Log( "event " + key )
		}
	}
	ent.ConnectOutput( output, handler )
}

function Beat_Fired( event )
{
	return event in ::BEATS.times
}

// Waits until <event> (a Beat_Mark key) has fired: check event.<name>.
function Beat_WaitEvent( name, event, timeout )
{
	QA_WaitFor( "event." + name, function() : ( event )
	{
		QA_Detail( event + ( Beat_Fired( event ) ? " fired" : " not fired" ) )
		return Beat_Fired( event )
	}, timeout )
}

// A scripted beat: <delay> s after <event> first fired, look at <look> (a
// point, a function returning one, [ pitch, yaw ], or null to leave the view
// to the map's camera) and shoot.
function Beat_EventShot( name, event, delay, look )
{
	Beat_WaitEvent( name, event, 240.0 )
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
		local due = ::BEATS.times[event] + delay
		QA_Detail( format( "due %.2f now %.2f ", due, Time() ) + ::WALK.state )
		return Time() >= due && ( look == null || Walk_Looked() )
	}, delay + 12.0 )
	QA_Do( "shoot " + name, function() : ( name ) { Beat_Shot( name ) }, 0.3 )
}

// A button held for <seconds>, as a click is (+attack, +attack2, +use).
function Beat_Press( command, seconds )
{
	::BEATS.pressed = command
	SendToConsole( "+" + command )
	EntFireByHandle( ::QA.driver, "RunScriptCode", "Beat_Release()", seconds, null, null )
}

function Beat_Release()
{
	SendToConsole( "-" + ::BEATS.pressed )
}
