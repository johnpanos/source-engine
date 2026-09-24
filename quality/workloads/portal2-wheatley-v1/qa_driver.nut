// Portal 2 gameplay scenario driver (tools/quality/portal2_scenarios.py).
//
// A scenario script includes this file, appends steps and calls QA_Start().
// Steps run one after another on server time: each is rescheduled through
// RunScriptCode on worldspawn, so the pacing does not depend on the frame
// rate of the headless client. Every result is one console line:
//
//   QA_CHECK <scenario>.<check> PASS|FAIL <detail>
//   QA_DONE <scenario> checks=<n> failures=<n>
//
// The harness fails a run that has no QA_DONE, a FAIL, a missing required
// check, or a script error. Squirrel 2.2 closures do not capture locals, so
// state lives in the root QA table and closures use free-variable lists.

::QA <- {
	scenario = "unnamed"
	steps = []
	index = 0
	stepStart = -1.0
	checks = 0
	failures = 0
	driver = null
	detail = ""
	done = false
	fired = {}
	marks = {}
}

// Step kinds: an action runs once and then waits; a wait polls its predicate
// every 0.1 s until it holds (PASS) or the timeout expires (FAIL).
function QA_Do( name, action, delay = 0.1 )
{
	::QA.steps.append( { name = name, kind = "do", fn = action, delay = delay, timeout = 0.0 } )
}

function QA_WaitFor( name, predicate, timeout )
{
	::QA.steps.append( { name = name, kind = "wait", fn = predicate, delay = 0.05, timeout = timeout } )
}

function QA_Expect( name, predicate )
{
	::QA.steps.append( { name = name, kind = "wait", fn = predicate, delay = 0.05, timeout = 0.0 } )
}

function QA_Sleep( seconds )
{
	QA_Do( "sleep", function() {}, seconds )
}

function QA_Log( message )
{
	printl( "QA_LOG " + ::QA.scenario + " t=" + Time() + " " + message )
}

function QA_Check( name, ok, detail )
{
	::QA.checks++
	if ( !ok )
	{
		::QA.failures++
	}
	printl( "QA_CHECK " + ::QA.scenario + "." + name + " " + ( ok ? "PASS" : "FAIL" ) + " " + detail )
	return ok
}

// Predicates describe what they saw here so a failure says why.
function QA_Detail( text )
{
	::QA.detail = text
}

function QA_Schedule( delay )
{
	EntFireByHandle( ::QA.driver, "RunScriptCode", "QA_Tick()", delay, null, null )
}

function QA_Start( scenario )
{
	::QA.scenario = scenario
	::QA.driver = Entities.FindByClassname( null, "worldspawn" )
	QA_Log( "start map=" + GetMapName() + " steps=" + ::QA.steps.len() )
	QA_Schedule( 0.1 )
}

function QA_Finish()
{
	::QA.done = true
	printl( "QA_DONE " + ::QA.scenario + " checks=" + ::QA.checks + " failures=" + ::QA.failures )
	SendToConsole( "wait 30; quit" )
}

function QA_Tick()
{
	if ( ::QA.done )
		return
	if ( ::QA.index >= ::QA.steps.len() )
	{
		QA_Finish()
		return
	}

	local step = ::QA.steps[::QA.index]
	if ( ::QA.stepStart < 0.0 )
	{
		::QA.stepStart = Time()
		::QA.detail = ""
	}

	local result = null
	local failed = false
	try
	{
		result = step.fn()
	}
	catch ( error )
	{
		QA_Check( step.name, false, "script error: " + error )
		failed = true
	}

	if ( !failed && step.kind == "wait" )
	{
		if ( result == true )
		{
			QA_Check( step.name, true, ::QA.detail )
		}
		else if ( Time() - ::QA.stepStart < step.timeout )
		{
			QA_Schedule( 0.1 )
			return
		}
		else
		{
			QA_Check( step.name, false, "after " + step.timeout + "s: " + ::QA.detail )
		}
	}
	else if ( !failed )
	{
		QA_Log( "did " + step.name )
	}

	::QA.index++
	::QA.stepStart = -1.0
	QA_Schedule( step.delay )
}

// --- Entities and outputs ----------------------------------------------------

function QA_Ent( name )
{
	local ent = Entities.FindByName( null, name )
	if ( ent == null )
		throw "no entity named " + name
	return ent
}

function QA_Player()
{
	local player = GetPlayer()
	if ( player == null )
		throw "no player"
	return player
}

function QA_Fire( target, input, parameter = "", delay = 0.0 )
{
	EntFire( target, input, parameter, delay )
}

// Counts how often <name>'s <output> fires; QA_Fired reads the count.
function QA_Watch( name, output )
{
	local ent = QA_Ent( name )
	local key = name + ":" + output
	local handler = "QA_Output_" + ::QA.fired.len()
	::QA.fired[key] <- 0
	ent.ValidateScriptScope()
	ent.GetScriptScope()[handler] <- function() : ( key )
	{
		::QA.fired[key]++
		QA_Log( "output " + key + " count=" + ::QA.fired[key] )
	}
	ent.ConnectOutput( output, handler )
}

function QA_Fired( name, output )
{
	local key = name + ":" + output
	if ( !( key in ::QA.fired ) )
		throw "output " + key + " is not watched"
	return ::QA.fired[key]
}

function QA_ParentName( ent )
{
	local parent = ent.GetMoveParent()
	return parent == null ? "<none>" : parent.GetName()
}

// --- View-space geometry -----------------------------------------------------

function QA_Deg( radians )
{
	return radians * 180.0 / PI
}

function QA_Rad( degrees )
{
	return degrees * PI / 180.0
}

// Source angle convention: pitch down is positive; forward/right/up as in
// mathlib AngleVectors with zero roll.
function QA_Basis( pitch, yaw )
{
	local cp = cos( QA_Rad( pitch ) ), sp = sin( QA_Rad( pitch ) )
	local cy = cos( QA_Rad( yaw ) ), sy = sin( QA_Rad( yaw ) )
	return {
		forward = Vector( cp * cy, cp * sy, -sp )
		right = Vector( sy, -cy, 0.0 )
		up = Vector( sp * cy, sp * sy, cp )
	}
}

function QA_Dot( a, b )
{
	return a.x * b.x + a.y * b.y + a.z * b.z
}

function QA_Scale( v, s )
{
	return Vector( v.x * s, v.y * s, v.z * s )
}

function QA_Dist( a, b )
{
	return ( a - b ).Length()
}

function QA_Vec( v )
{
	return "(" + format( "%.1f %.1f %.1f", v.x, v.y, v.z ) + ")"
}

function QA_AnglesTo( from, to )
{
	local d = to - from
	local flat = sqrt( d.x * d.x + d.y * d.y )
	return { pitch = -QA_Deg( atan2( d.z, flat ) ), yaw = QA_Deg( atan2( d.y, d.x ) ) }
}

// The player entity's origin sits below the eye by a fixed view offset.
function QA_EyeOffset()
{
	local player = QA_Player()
	return player.EyePosition() - player.GetOrigin()
}

function QA_SetView( pitch, yaw )
{
	::QA.marks.pitch <- pitch
	::QA.marks.yaw <- yaw
	QA_Player().SetAngles( pitch, yaw, 0.0 )
}

function QA_PlaceEye( eye, pitch, yaw )
{
	local player = QA_Player()
	player.SetVelocity( Vector( 0, 0, 0 ) )
	player.SetOrigin( eye - QA_EyeOffset() )
	QA_SetView( pitch, yaw )
}

function QA_LookAt( target )
{
	local angles = QA_AnglesTo( QA_Player().EyePosition(), target )
	QA_SetView( angles.pitch, angles.yaw )
}

// Stand <distance> units from <target> along the horizontal line to <from>
// and look at it; used to reach an object before +use.
function QA_Approach( target, from, distance )
{
	local d = from - target
	d.z = 0.0
	local length = d.Length()
	if ( length < 1.0 )
		d = Vector( 1, 0, 0 )
	else
		d = QA_Scale( d, 1.0 / length )
	local eye = target + QA_Scale( d, distance )
	eye.z = target.z + 8.0
	local angles = QA_AnglesTo( eye, target )
	QA_PlaceEye( eye, angles.pitch, angles.yaw )
}

// Where the held object sits in view space (forward, right, up) right now.
function QA_ViewOffset( ent )
{
	local basis = QA_Basis( ::QA.marks.pitch, ::QA.marks.yaw )
	local d = ent.GetCenter() - QA_Player().EyePosition()
	return { f = QA_Dot( d, basis.forward ), r = QA_Dot( d, basis.right ), u = QA_Dot( d, basis.up ) }
}

function QA_ViewPoint( eye, pitch, yaw, offset )
{
	local basis = QA_Basis( pitch, yaw )
	return eye + QA_Scale( basis.forward, offset.f ) + QA_Scale( basis.right, offset.r ) +
	       QA_Scale( basis.up, offset.u )
}

// True when <ent> sits at the same view-space offset it had when <mark> was
// recorded, i.e. it follows the view.
function QA_HeldAt( ent, mark, tolerance )
{
	local offset = ::QA.marks[mark]
	local expected = QA_ViewPoint( QA_Player().EyePosition(), ::QA.marks.pitch, ::QA.marks.yaw, offset )
	local error = QA_Dist( ent.GetCenter(), expected )
	QA_Detail( "at " + QA_Vec( ent.GetCenter() ) + " expected " + QA_Vec( expected ) +
	           format( " error=%.1f", error ) )
	return error <= tolerance
}

function QA_MarkViewOffset( mark, ent )
{
	::QA.marks[mark] <- QA_ViewOffset( ent )
	local o = ::QA.marks[mark]
	QA_Log( "view offset " + mark + format( " f=%.1f r=%.1f u=%.1f", o.f, o.r, o.u ) )
}

// Places the player, facing <target> from the <from> side, so that an object
// held at view offset <mark> is carried to <target>.
function QA_CarryTo( mark, target, from )
{
	local offset = ::QA.marks[mark]
	local d = target - from
	local angles = QA_AnglesTo( from, target )
	angles.pitch = 0.0
	local basis = QA_Basis( angles.pitch, angles.yaw )
	local eye = target - QA_Scale( basis.forward, offset.f ) - QA_Scale( basis.right, offset.r ) -
	            QA_Scale( basis.up, offset.u )
	QA_PlaceEye( eye, angles.pitch, angles.yaw )
	QA_Log( "carry to " + QA_Vec( target ) + " eye " + QA_Vec( eye ) )
}

// Holds a +command button for <seconds> of server time; the client runs it.
function QA_Press( command, seconds )
{
	::QA.marks.pressed <- command
	SendToConsole( "+" + command )
	EntFireByHandle( ::QA.driver, "RunScriptCode", "QA_Release()", seconds, null, null )
}

function QA_Release()
{
	SendToConsole( "-" + ::QA.marks.pressed )
}
