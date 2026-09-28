// qa_portal_tunnel: two portals facing each other (map:
// tools/quality/portal2_portal_map.py).
//
// Blue goes on the north wall and orange straight across on the south wall,
// so each portal shows the other one inside it: every portal view contains a
// portal view, down to the renderer's recursion limit, and each of them draws
// the room's translucent sprites and glass. The player looks into both,
// sweeps the view across them, and walks through blue twice (out of orange
// heading north each time). A crash in nested portal views ends the run
// without QA_DONE, which fails the scenario.

IncludeScript( "qa/qa_driver" )
IncludeScript( "qa/qa_portals" )

::TUN <- {
	north = Vector( 0, 640, 56 )
	south = Vector( 0, -640, 56 )
}

PW_Setup()
PW_Place( "blue", "attack", Vector( 0, 400, 64 ), ::TUN.north, Vector( 0, -1, 0 ), "south" )
PW_Place( "orange", "attack2", Vector( 0, -400, 64 ), ::TUN.south, Vector( 0, 1, 0 ), "north" )

// Look into each portal from the middle of the room, then sweep across them.
QA_Do( "look north", function() { PW_Aim( Vector( 0, 0, 64 ), ::TUN.north ) }, 1.0 )
QA_Do( "shot north", function() { PW_Shot( "tunnel_north" ) }, 0.5 )
QA_Do( "look south", function() { PW_Aim( Vector( 0, 0, 64 ), ::TUN.south ) }, 1.0 )
QA_Do( "shot south", function() { PW_Shot( "tunnel_south" ) }, 0.5 )
QA_Do( "close to blue", function() { PW_Aim( Vector( 0, 560, 64 ), ::TUN.north ) }, 1.0 )
QA_Do( "shot close", function() { PW_Shot( "tunnel_close" ) }, 0.5 )
QA_Do( "sweep", function()
{
	PW_Aim( Vector( 0, 0, 64 ), ::TUN.north )
	SendToConsole( "cl_yawspeed 90" )
	QA_Press( "left", 4.0 )
}, 4.5 )
QA_Do( "sweep done", function() { SendToConsole( "cl_yawspeed 210" ) }, 0.2 )
QA_Expect( "views.survived", function() { return QA_Player() != null } )

// Walk north into blue twice; each time the player comes out of orange
// heading north.
QA_Do( "stand before blue", function() { PW_StandBefore( ::PW.portals.blue, 160 ) }, 1.0 )
QA_Do( "tunnel begin", function() { PW_Window( "tunnel", "BEGIN" ) }, 0.0 )
QA_Do( "walk into blue", function()
{
	PW_Path( "tunnel", 2.5 )
	QA_Press( "forward", 1.6 )
}, 2.6 )
QA_Do( "tunnel end", function() { PW_Window( "tunnel", "END" ) }, 0.1 )
QA_Expect( "cross.blue_to_orange", function() { return PW_ExitedFrom( ::PW.portals.orange ) } )
QA_Expect( "cross.heading_north", function() { return PW_Heading( ::PW.portals.orange ) } )
QA_Do( "again before blue", function() { PW_StandBefore( ::PW.portals.blue, 160 ) }, 1.0 )
QA_Do( "walk into blue again", function() { QA_Press( "forward", 1.6 ) }, 2.6 )
QA_Expect( "cross.again", function() { return PW_ExitedFrom( ::PW.portals.orange ) } )

QA_Start( "qa_portal_tunnel" )
