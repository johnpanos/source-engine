// qa_portal_walk: the player shoots a portal pair with the gun and walks
// through it both ways (map: tools/quality/portal2_portal_map.py).
//
// Blue goes on the north wall (facing south) and orange on the east wall
// (facing west), both at floor level so the player can walk in. Walking north
// into blue leaves orange heading west; walking east into orange leaves blue
// heading south. A first orange shot at the black west wall must not leave a
// portal.
//
// The client traces its view every frame (cl_portal_view_trace, PVIEW lines)
// inside QA_WINDOW brackets; the workload's console checks require each window's
// view path to be continuous once its crossing is undone through the portal.

IncludeScript( "qa/qa_driver" )
IncludeScript( "qa/qa_portals" )

PW_Setup()

// A shot at black metal leaves no portal.
QA_Do( "aim at the black wall", function() { PW_Aim( Vector( -400, 0, 64 ), Vector( -640, 0, 56 ) ) }, 0.5 )
QA_Do( "black begin", function() { PW_Window( "black_wall", "BEGIN" ) }, 0.0 )
QA_Do( "fire at the black wall", function() { QA_Press( "attack2", 0.1 ) }, 1.5 )
QA_Do( "black end", function() { PW_Window( "black_wall", "END" ) }, 0.1 )
QA_Expect( "black_wall.no_portal", function()
{
	local p = PW_PortalNear( Vector( -640, 0, 56 ), 128 )
	QA_Detail( p == null ? "none" : "portal at " + QA_Vec( p.GetOrigin() ) )
	return p == null
} )

PW_Place( "blue", "attack", Vector( -192, 400, 64 ), Vector( -192, 640, 56 ), Vector( 0, -1, 0 ), "south" )
PW_Place( "orange", "attack2", Vector( 400, 192, 64 ), Vector( 640, 192, 56 ), Vector( -1, 0, 0 ), "west" )

// Crossing 1: walk north into blue, leave orange heading west.
QA_Do( "stand before blue", function() { PW_StandBefore( ::PW.portals.blue, 160 ) }, 1.0 )
QA_Do( "shot approach blue", function() { PW_Shot( "approach_blue" ) }, 0.3 )
QA_Do( "blue begin", function() { PW_Window( "blue_to_orange", "BEGIN" ) }, 0.0 )
QA_Do( "walk into blue", function()
{
	PW_Path( "blue_to_orange", 2.5 )
	QA_Press( "forward", 1.6 )
}, 2.6 )
QA_Do( "blue end", function() { PW_Window( "blue_to_orange", "END" ) }, 0.1 )
QA_Expect( "cross.blue_to_orange", function() { return PW_ExitedFrom( ::PW.portals.orange ) } )
QA_Expect( "cross.blue_to_orange_heading", function() { return PW_Heading( ::PW.portals.orange ) } )
QA_Do( "shot after blue", function() { PW_Shot( "after_blue" ) }, 0.3 )

// Crossing 2: turn around and walk back into orange, leave blue heading south.
QA_Do( "stand before orange", function() { PW_StandBefore( ::PW.portals.orange, 160 ) }, 1.0 )
QA_Do( "shot approach orange", function() { PW_Shot( "approach_orange" ) }, 0.3 )
QA_Do( "orange begin", function() { PW_Window( "orange_to_blue", "BEGIN" ) }, 0.0 )
QA_Do( "walk into orange", function()
{
	PW_Path( "orange_to_blue", 2.5 )
	QA_Press( "forward", 1.6 )
}, 2.6 )
QA_Do( "orange end", function() { PW_Window( "orange_to_blue", "END" ) }, 0.1 )
QA_Expect( "cross.orange_to_blue", function() { return PW_ExitedFrom( ::PW.portals.blue ) } )
QA_Expect( "cross.orange_to_blue_heading", function() { return PW_Heading( ::PW.portals.blue ) } )
QA_Do( "shot after orange", function() { PW_Shot( "after_orange" ) }, 0.3 )

// Crossing 3: blue to orange again at 20 frames a second. The client builds
// several commands per frame, so some reach the server after the teleport but
// were made before the client knew of it; the server turns their view angles
// through the portal (CPortal_Player::PlayerRunCommand). Without that they
// steer the player along the old heading, off orange's center line.
QA_Do( "stand before blue at 20 fps", function()
{
	PW_StandBefore( ::PW.portals.blue, 160 )
	SendToConsole( "host_framerate 0.05" )
}, 1.0 )
QA_Do( "slow begin", function() { PW_Window( "blue_to_orange_20fps", "BEGIN" ) }, 0.0 )
QA_Do( "walk into blue at 20 fps", function()
{
	PW_Path( "blue_to_orange_20fps", 3.5 )
	QA_Press( "forward", 3.0 )
}, 3.6 )
QA_Do( "slow end", function()
{
	PW_Window( "blue_to_orange_20fps", "END" )
	SendToConsole( "host_framerate 0" )
}, 0.2 )
QA_Expect( "cross_20fps.blue_to_orange", function() { return PW_ExitedFrom( ::PW.portals.orange ) } )
QA_Expect( "cross_20fps.on_center_line", function()
{
	local o = QA_Player().GetOrigin()
	local p = ::PW.portals.orange.GetOrigin()
	QA_Detail( "player " + QA_Vec( o ) + " orange " + QA_Vec( p ) )
	return fabs( o.y - p.y ) < 0.5
} )

QA_Start( "qa_portal_walk" )
