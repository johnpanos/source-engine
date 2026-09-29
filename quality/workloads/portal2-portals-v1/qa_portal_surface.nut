// qa_portal_surface: how a wall portal draws against its wall (map:
// tools/quality/portal2_portal_map.py).
//
// Blue goes on the north wall and orange on the south wall. The player looks
// at blue head-on from close, from far and at a glancing angle, where a
// portal drawn in the wall's own plane z-fights with it, and watches its edge
// particles. PW_Shot records each view.

IncludeScript( "qa/qa_driver" )
IncludeScript( "qa/qa_portals" )

::SURF <- {
	north = Vector( 0, 640, 56 )
	south = Vector( 0, -640, 56 )
}

PW_Setup()
PW_Place( "blue", "attack", Vector( 0, 400, 64 ), ::SURF.north, Vector( 0, -1, 0 ), "south" )
PW_Place( "orange", "attack2", Vector( 0, -400, 64 ), ::SURF.south, Vector( 0, 1, 0 ), "north" )

QA_Do( "surface begin", function() { PW_Window( "portal_surface", "BEGIN" ) }, 0.0 )
QA_Do( "look close", function() { PW_Aim( Vector( 0, 520, 64 ), ::SURF.north ) }, 1.5 )
QA_Do( "shot close", function() { PW_Shot( "surface_close" ) }, 0.5 )
QA_Do( "look far", function() { PW_Aim( Vector( 0, -300, 64 ), ::SURF.north ) }, 1.0 )
QA_Do( "shot far", function() { PW_Shot( "surface_far" ) }, 0.5 )
QA_Do( "look glancing", function() { PW_Aim( Vector( -420, 470, 64 ), ::SURF.north ) }, 1.0 )
QA_Do( "shot glancing", function() { PW_Shot( "surface_glancing" ) }, 0.5 )
QA_Do( "look glancing far", function() { PW_Aim( Vector( 560, 260, 64 ), ::SURF.north ) }, 1.0 )
QA_Do( "shot glancing far", function() { PW_Shot( "surface_glancing_far" ) }, 0.5 )
QA_Do( "look orange", function() { PW_Aim( Vector( 0, -520, 64 ), ::SURF.south ) }, 1.0 )
QA_Do( "shot orange", function() { PW_Shot( "surface_orange" ) }, 0.5 )
QA_Do( "surface end", function() { PW_Window( "portal_surface", "END" ) }, 0.1 )
QA_Expect( "surface.survived", function() { return QA_Player() != null } )

QA_Start( "qa_portal_surface" )
