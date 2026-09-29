// sp_a1_wakeup: Wheatley carried in front of the view (the view model grab
// controller's held object clone, drawn with the world's FOV), and the
// destroyed chamber's hanging debris against the sky (models/props_hub/
// glados_chamber_dest01 and models/npcs/glados/glados_temp: shader Black, a
// fogged silhouette).

IncludeScript( "qa/materials_views" )

// The held core is drawn with the view models: keep them on for this map.
QA_Do( "view models", function() { SendToConsole( "r_drawviewmodel 1" ) }, 0.5 )

MV_View( "held_core", Vector( 6976, 496.6, 448 ), 0.0, 90.0, 2.0 )
MV_View( "debris", Vector( 10000, 1216, 552 ), -35.0, 0.0, 2.0 )
MV_View( "debris_side", Vector( 9400, 1216, 552 ), -35.0, 90.0, 2.0 )
MV_Start( "sp_a1_wakeup" )
