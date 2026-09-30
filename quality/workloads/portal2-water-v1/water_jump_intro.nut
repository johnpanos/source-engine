// sp_a3_jump_intro: old Aperture goo (Water with a flow map, the sludge layer
// and $forceenvmap: the reflection is the cube map vbsp patched in), from the
// map's env_cubemap at -256 1728 128, looking down and at a grazing angle.

IncludeScript( "qa/materials_views" )

MV_ViewAt( "goo_down", Vector( -256, 1728, 128 ), Vector( -256, 1400, -256 ) )
MV_ViewAt( "goo_grazing", Vector( -256, 1728, 128 ), Vector( -256, 200, -256 ) )
MV_Start( "water_jump_intro" )
