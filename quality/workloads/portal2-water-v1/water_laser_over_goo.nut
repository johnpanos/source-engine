// sp_a2_laser_over_goo: the goo pit (Water with a flow map, the sludge layer
// and the view's reflection target), shot four times a second apart (the
// flow and the sludge animate), and from farther away at a grazing angle.

IncludeScript( "qa/materials_views" )

MV_ViewAt( "goo_t0", Vector( 3000, -1500, 160 ), Vector( 3250, -1750, -87 ) )
MV_ViewAt( "goo_t1", Vector( 3000, -1500, 160 ), Vector( 3250, -1750, -87 ) )
MV_ViewAt( "goo_t2", Vector( 3000, -1500, 160 ), Vector( 3250, -1750, -87 ) )
MV_ViewAt( "goo_t3", Vector( 3000, -1500, 160 ), Vector( 3250, -1750, -87 ) )
MV_ViewAt( "goo_far", Vector( 3300, -800, 40 ), Vector( 3300, -1800, -96 ) )
MV_Start( "water_laser_over_goo" )
