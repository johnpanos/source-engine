// sp_a2_laser_over_goo: the goo pit (Water), the laser catcher and the
// cable above the pit.

IncludeScript( "qa/materials_views" )

MV_ViewAt( "goo", Vector( 3000, -1500, 160 ), Vector( 3250, -1750, -87 ) )
MV_ViewAt( "cable", Vector( 3100, -1450, 200 ), Vector( 3330, -1690, 265 ) )
MV_ViewAt( "catcher", Vector( 3300, -1100, 120 ), Vector( 3488, -752, 64 ) )
MV_Start( "sp_a2_laser_over_goo" )
