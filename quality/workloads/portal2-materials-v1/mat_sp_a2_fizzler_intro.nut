// sp_a2_fizzler_intro: the chamber fizzler (SolidEnergy, FizzlerVortex proxy)
// between its two emitters, and the laser emitter under the ceiling cables
// (SplineRope).

IncludeScript( "qa/materials_views" )

MV_ViewAt( "fizzler", Vector( 128, -160, 96 ), Vector( 128, -432, 80 ) )
MV_ViewAt( "cables", Vector( 300, -300, 300 ), Vector( 448, -32, 440 ) )
MV_ViewAt( "laser_emitter", Vector( 300, -700, 140 ), Vector( 560, -928, 40 ) )
MV_Start( "sp_a2_fizzler_intro" )
