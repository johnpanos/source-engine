// sp_a2_triple_laser: portals on two faces of the pillar, walked through both
// ways.
//
// The pillar is one world brush, x 7392..7456, y -5312..-5184. Blue fills its
// 64-unit south face (facing -y, both edges on the corners) and orange goes on
// its east face by the same corner, facing +x: angles (0 0 0). A new portal
// simulator's center is invalid (NaN), and under -ffast-math the "not moving"
// test in CPortalSimulator::MoveTo took NaN as equal to any center, so a first
// move to angles (0 0 0) was skipped. Orange's simulator then stayed unplaced:
// the player stopped at its surface, and at blue's too, since blue's link
// depends on orange's placement.

IncludeScript( "qa/qa_driver" )
IncludeScript( "qa/qa_portals" )

QA_Do( "setup", function()
{
	SendToConsole( "sv_cheats 1; con_drawnotify 0; cl_drawhud 0; developer 0" )
}, 1.0 )
QA_Sleep( 8.0 )
QA_Do( "leave the elevator", function()
{
	QA_StandIn( QA_EntNear( "trigger_once", Vector( 8000, -5184, 128 ) ), -90.0 )
}, 0.5 )
QA_Do( "take the dual gun", function() { SendToConsole( "give weapon_portalgun; upgrade_portalgun" ) }, 1.0 )

PW_Place( "blue", "attack", Vector( 7424, -5450, 64 ), Vector( 7424, -5312, 56 ), Vector( 0, -1, 0 ), "south" )
PW_Place( "orange", "attack2", Vector( 7640, -5270, 64 ), Vector( 7456, -5270, 56 ), Vector( 1, 0, 0 ), "east" )

QA_Do( "stand before orange", function() { PW_StandBefore( ::PW.portals.orange, 120 ) }, 1.0 )
QA_Do( "walk into orange", function()
{
	PW_Path( "orange_to_blue", 2.5 )
	QA_Press( "forward", 1.6 )
}, 2.6 )
QA_Expect( "cross.orange_to_blue", function() { return PW_ExitedFrom( ::PW.portals.blue ) } )
QA_Expect( "cross.orange_to_blue_heading", function() { return PW_Heading( ::PW.portals.blue ) } )

QA_Do( "stand before blue", function() { PW_StandBefore( ::PW.portals.blue, 120 ) }, 1.0 )
QA_Do( "walk into blue", function()
{
	PW_Path( "blue_to_orange", 2.5 )
	QA_Press( "forward", 1.6 )
}, 2.6 )
QA_Expect( "cross.blue_to_orange", function() { return PW_ExitedFrom( ::PW.portals.orange ) } )
QA_Expect( "cross.blue_to_orange_heading", function() { return PW_Heading( ::PW.portals.orange ) } )

QA_Start( "sp_a2_triple_laser_pillar_corner" )
