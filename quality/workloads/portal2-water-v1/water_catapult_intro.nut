// sp_a2_catapult_intro: the goo under the catapults (Water with a flow map,
// the sludge layer and the view's reflection target): from behind the first
// faith plate toward the exit door (the player's view entering the chamber),
// and looking down on the goo beside the plates.

IncludeScript( "qa/materials_views" )

MV_ViewAt( "goo_across", Vector( -64, 760, -384 ), Vector( -64, -1456, -430 ) )
MV_ViewAt( "goo_down", Vector( -64, 300, -330 ), Vector( 250, -100, -512 ) )
MV_Start( "water_catapult_intro" )
