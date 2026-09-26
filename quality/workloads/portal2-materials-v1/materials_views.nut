// Portal 2 video retail-conformance views (portal2-materials-v1).
//
// tools/quality/portal2_material_shots.py runs these scripts on this build
// (through tools/quality/portal2_scenarios.py) and on the retail Portal 2
// binary (through a mapspawn.nut hook), at 1024x768 on both sides. Each view
// puts the eye at a fixed point with a fixed pitch and yaw, waits for the
// view to settle and takes a named screenshot: one `QA_SHOT <scenario>
// <view>` line per `screenshot` command. The checker pairs the shots by name
// and compares them with the retail reference.
//
// The player flies (noclip) and cannot be hurt or targeted, so the views do
// not depend on physics. Views are placed on server time right after the map
// has loaded, before any puzzle is touched.

IncludeScript( "qa/qa_driver" )

::MV <- {
	views = 0
}

function MV_Shot( name )
{
	printl( "QA_SHOT " + ::QA.scenario + " " + name )
	local player = QA_Player()
	QA_Log( "shot " + name + " eye " + QA_Vec( player.EyePosition() ) + " fwd " +
	        QA_Vec( QA_Scale( player.GetForwardVector(), 100.0 ) ) )
	SendToConsole( "screenshot" )
}

// The same console state on both sides: no HUD, notify text or view model,
// and the renderer settings printed for the record.
QA_Do( "setup", function()
{
	// Notify text off before sv_cheats, whose change retail prints on screen.
	SendToConsole( "con_drawnotify 0; con_notifytime 0; cl_drawhud 0; developer 0" )
	SendToConsole( "sv_cheats 1; r_drawviewmodel 0; god; notarget; noclip" )
	SendToConsole( "mat_picmip; mat_antialias; mat_forceaniso; mat_hdr_level; " +
	               "mat_colorcorrection; mat_motion_blur_enabled; r_flashlightdepthtexture; " +
	               "mat_queue_mode; gpu_level; cpu_level" )
}, 2.0 )

// A view: the eye at <eye>, looking at <pitch>, <yaw>; the shot is taken
// <settle> seconds later.
function MV_View( name, eye, pitch, yaw, settle = 1.5 )
{
	QA_Do( "place " + name, function() : ( eye, pitch, yaw )
	{
		QA_PlaceEye( eye, pitch, yaw )
	}, settle )
	QA_Do( "shot " + name, function() : ( name, eye )
	{
		// The eye must still be where the view put it (noclip holds it).
		local error = QA_Dist( QA_Player().EyePosition(), eye )
		QA_Check( "view." + name, error < 2.0, "eye " + QA_Vec( QA_Player().EyePosition() ) +
		          " is " + format( "%.2f", error ) + " from " + QA_Vec( eye ) )
		MV_Shot( name )
	}, 1.0 )
	::MV.views++
}

// A view from <eye> toward the point <target>.
function MV_ViewAt( name, eye, target, settle = 1.5 )
{
	local angles = QA_AnglesTo( eye, target )
	MV_View( name, eye, angles.pitch, angles.yaw, settle )
}

function MV_Start( scenario )
{
	QA_Do( "finish", function()
	{
		QA_Log( "views " + ::MV.views )
	}, 0.5 )
	QA_Start( scenario )
}
