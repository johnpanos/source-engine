// sp_a2_triple_laser: the entry hall (catchers under their indicator
// strips, white and black panels), the ssbump floor tiles up close, and a
// portal pair fired as in portal2-triple-laser-v1 (A on the laser room's
// placement helper, B on the entry hall's north wall): each portal seen from
// the front, showing the view through it (PortalRefract, the portal edge).

IncludeScript( "qa/materials_views" )

::ML <- {
	helperA = Vector( 8000, -5504, 48 ) // the map's portal placement helper
}

function ML_WallAhead( from, dir )
{
	local to = from + QA_Scale( dir, 2000.0 )
	return from + QA_Scale( dir, 2000.0 * TraceLine( from, to, null ) )
}

MV_View( "entry_hall", Vector( 7760, -5340, 64 ), 12.0, -155.0 )
MV_View( "floor", Vector( 7760, -5340, 64 ), 60.0, -120.0 )

QA_Do( "aim at the helper wall", function()
{
	::ML.northWall <- ML_WallAhead( Vector( 7420, -5400, 48 ), Vector( 0, 1, 0 ) )
	local eye = Vector( 8000, -5700, 64 )
	local angles = QA_AnglesTo( eye, ::ML.helperA )
	QA_PlaceEye( eye, angles.pitch, angles.yaw )
}, 1.0 )
QA_Do( "fire portal A", function() { QA_Press( "attack", 0.1 ) }, 1.5 )
QA_Do( "aim at the north wall", function()
{
	local target = Vector( 7420, ::ML.northWall.y, 56 )
	local eye = Vector( 7420, -5420, 64 )
	local angles = QA_AnglesTo( eye, target )
	QA_PlaceEye( eye, angles.pitch, angles.yaw )
}, 1.0 )
QA_Do( "fire portal B", function() { QA_Press( "attack2", 0.1 ) }, 2.0 )

MV_ViewLater( "portal_a", function()
{
	return { eye = Vector( 8000, -5680, 72 ), target = ::ML.helperA + Vector( 0, 0, 16 ) }
}, 4.0 )
MV_ViewLater( "portal_b", function()
{
	local wall = ::ML.northWall
	return { eye = Vector( 7420, wall.y - 170, 72 ), target = Vector( 7420, wall.y, 64 ) }
}, 4.0 )
MV_Start( "sp_a2_triple_laser" )
