// sp_a2_triple_laser: a redirection cube carrying a laser is removed.
//
// Cube 2 sits in the wall laser, which fires east from (7472 -5728 18), turned
// north, so the wall laser continues through a child env_portal_laser that is
// parented to the cube. Removing the cube removes that child as an orphan
// (CBaseEntity::UpdateOnRemove), behind the wall laser's back; the wall laser
// must notice on its next think instead of removing the freed child again
// (the crash in CPortalLaser::FireLaser -> UTIL_Remove). The cube is then
// respawned by the map's dropper logic or not; either way the wall laser must
// keep firing and the game must reach QA_DONE.

IncludeScript( "qa/qa_driver" )

::CR <- {
	cubeSpot = Vector( 8000, -5728, 20 )
}

function CR_CubeLaser( cubeName )
{
	local laser = null
	while ( laser = Entities.FindByClassname( laser, "env_portal_laser" ) )
	{
		if ( QA_ParentName( laser ) == cubeName )
			return laser
	}
	return null
}

QA_Do( "setup", function()
{
	SendToConsole( "sv_cheats 1; con_drawnotify 0; cl_drawhud 0; developer 0" )
}, 1.0 )
QA_Sleep( 8.0 )
QA_Do( "leave the elevator", function()
{
	QA_StandIn( QA_EntNear( "trigger_once", Vector( 8000, -5184, 128 ) ), -90.0 )
}, 0.5 )
QA_WaitFor( "entry.cube_spawned", function()
{
	return Entities.FindByName( null, "new_box1" ) != null
}, 5.0 )
QA_Do( "view the cube", function() { QA_PlaceEye( Vector( 7700, -5560, 90 ), 18.0, -35.0 ) }, 1.0 )

QA_Do( "cube into the wall laser", function()
{
	local cube = QA_Ent( "new_box1" )
	cube.SetOrigin( ::CR.cubeSpot )
	cube.SetAngles( 0, 90, 0 )
	cube.SetVelocity( Vector( 0, 0, 0 ) )
}, 2.0 )
QA_Expect( "cube.redirects", function()
{
	local laser = CR_CubeLaser( "new_box1" )
	QA_Detail( laser ? "cube laser " + laser.entindex() : "no cube laser" )
	return laser != null
} )

// Remove the cube while it carries the child laser; the wall laser thinks
// every 0.1 s, so a few seconds cover many thinks.
QA_Do( "remove the cube", function() { EntFireByHandle( QA_Ent( "new_box1" ), "Kill", "", 0.0, null, null ) }, 3.0 )
QA_Expect( "removed.child_laser_gone", function()
{
	local cube = Entities.FindByName( null, "new_box1" )
	local laser = cube ? CR_CubeLaser( "new_box1" ) : null
	QA_Detail( cube ? "a cube named new_box1 exists" : "no cube" )
	return laser == null
} )
QA_Expect( "removed.wall_laser_alive", function()
{
	local laser = Entities.FindByClassnameNearest( "env_portal_laser", Vector( 7472, -5728, 18 ), 128.0 )
	QA_Detail( laser ? "wall laser " + laser.entindex() + " at " + QA_Vec( laser.GetOrigin() ) : "no wall laser" )
	return laser != null
} )

QA_Start( "sp_a2_triple_laser_cube_removed" )
