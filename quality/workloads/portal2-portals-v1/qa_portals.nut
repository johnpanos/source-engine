// Helpers shared by the qa_portal_walk scenarios (quality/workloads/portal2-portals-v1).
// They drive the gun and the player and find the portals it placed; the
// driver's QA_* functions come from qa_driver.nut, included first.

::PW <- {
	portals = {}
	envs = []
	pathUntil = 0.0
	pathTag = ""
}

function PW_Window( label, edge )
{
	printl( "QA_WINDOW " + label + " " + edge )
}

function PW_Shot( name )
{
	printl( "QA_SHOT " + ::QA.scenario + " " + name )
	SendToConsole( "screenshot" )
}

function PW_PathTick()
{
	local player = QA_Player()
	QA_Log( "path " + ::PW.pathTag + " origin " + QA_Vec( player.GetOrigin() ) + " vel " +
	        QA_Vec( player.GetVelocity() ) + " fwd " + QA_Vec( QA_Scale( player.GetForwardVector(), 100.0 ) ) )
	if ( Time() < ::PW.pathUntil )
		EntFireByHandle( ::QA.driver, "RunScriptCode", "PW_PathTick()", 0.03, null, null )
}

function PW_Path( tag, seconds )
{
	::PW.pathTag = tag
	::PW.pathUntil = Time() + seconds
	PW_PathTick()
}

// The portal nearest `aim` within `radius`, or null.
function PW_PortalNear( aim, radius )
{
	local best = null
	local p = null
	while ( p = Entities.FindByClassname( p, "prop_portal" ) )
	{
		QA_Log( "portal " + p.entindex() + " at " + QA_Vec( p.GetOrigin() ) + " angles " + QA_Vec( p.GetAngles() ) )
		if ( QA_Dist( p.GetOrigin(), aim ) < radius && ( best == null ||
		     QA_Dist( p.GetOrigin(), aim ) < QA_Dist( best.GetOrigin(), aim ) ) )
			best = p
	}
	return best
}

function PW_Aim( eye, target )
{
	local angles = QA_AnglesTo( eye, target )
	QA_PlaceEye( eye, angles.pitch, angles.yaw )
}

// Stand `distance` units in front of a wall portal (on the floor), facing it.
function PW_StandBefore( portal, distance )
{
	local o = portal.GetOrigin()
	local n = portal.GetForwardVector()
	local player = QA_Player()
	player.SetVelocity( Vector( 0, 0, 0 ) )
	player.SetOrigin( Vector( o.x + n.x * distance, o.y + n.y * distance, 0 ) )
	QA_SetView( 0.0, QA_Deg( atan2( -n.y, -n.x ) ) )
}

// The player left `portal` on its front side, moving away along its normal.
function PW_ExitedFrom( portal )
{
	local o = QA_Player().GetOrigin()
	local p = portal.GetOrigin()
	local n = portal.GetForwardVector()
	local d = o - p
	local out = d.x * n.x + d.y * n.y
	local side = fabs( d.x * n.y - d.y * n.x )
	QA_Detail( "player " + QA_Vec( o ) + " portal " + QA_Vec( p ) + " out " + out + " side " + side )
	return out > 16 && out < 260 && side < 48
}

function PW_Heading( portal )
{
	local f = QA_Player().GetForwardVector()
	local n = portal.GetForwardVector()
	QA_Detail( "forward " + QA_Vec( QA_Scale( f, 100.0 ) ) + " portal normal " + QA_Vec( n ) )
	return f.x * n.x + f.y * n.y > 0.97
}

function PW_Setup()
{
	QA_Do( "setup", function()
	{
		SendToConsole( "sv_cheats 1; con_drawnotify 0; cl_drawhud 0; developer 0; cl_portal_view_trace 1; cl_portal_view_trace_shots 40" )
	}, 2.0 )
	QA_Expect( "gun.held", function()
	{
		local gun = Entities.FindByClassname( null, "weapon_portalgun" )
		QA_Detail( gun == null ? "no gun" : "owner " + gun.GetOwner() )
		return gun != null && gun.GetOwner() == QA_Player()
	} )
}

// Fire `button` ("attack" blue, "attack2" orange) from `eye` at `aim`, then
// require a portal within `radius` of the aim point whose normal is `normal`
// (checks <color>.placed and <color>.faces_<facing>). Squirrel 2 closures do
// not capture locals, so each step runs bound to a table of the arguments
// (kept in ::PW.envs: bindenv holds its environment weakly).
function PW_Place( color, button, eye, aim, normal, facing, radius = 40 )
{
	local env = { color = color, button = button, eye = eye, aim = aim, normal = normal, radius = radius }
	::PW.envs.append( env )
	QA_Do( "aim " + color, function() { PW_Aim( eye, aim ) }.bindenv( env ), 0.5 )
	QA_Do( "fire " + color, function() { QA_Press( button, 0.1 ) }.bindenv( env ), 1.5 )
	QA_Expect( color + ".placed", function()
	{
		local p = PW_PortalNear( aim, radius )
		if ( p == null )
			return false
		::PW.portals[color] <- p
		QA_Detail( "at " + QA_Vec( p.GetOrigin() ) + " aimed " + QA_Vec( aim ) )
		return true
	}.bindenv( env ) )
	QA_Expect( color + ".faces_" + facing, function()
	{
		local n = ::PW.portals[color].GetForwardVector()
		QA_Detail( "normal " + QA_Vec( n ) + " at " + QA_Vec( ::PW.portals[color].GetOrigin() ) )
		return n.x * normal.x + n.y * normal.y + n.z * normal.z > 0.99
	}.bindenv( env ) )
}
