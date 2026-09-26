// sp_a3_jump_intro: the chamber's floor trench (x -448..256, y 448..768,
// floor z -184, open above) is coated with repulsion gel by the map's own
// floor sprayers. This scenario measures the gel's powers and draws in it:
//
//   floor_blue    the trench as the map painted it (shot)
//   bounce        the rebound of a 160-unit drop onto the blue floor
//   floor_orange  after an orange (propulsion) strip is swept along y=560
//   speed         top walking speed on blue (control), then along the strip
//   floor_erased  after water (erase) is swept over a patch near x=0
//   bounce_erased the rebound of the same drop onto the erased patch
//   wall_white    after conversion gel is swept up the x=256 end wall
//   portal        a portal shot at the unpainted wall (control), then at
//                 the white patch
//
// tools/quality/portal2_paint.py compares the QA_MEASURE numbers and the
// shots with retail Portal 2.

IncludeScript( "qa/qa_driver" )
IncludeScript( "qa/paint_lib" )

::JI <- {
	floor = -184.0
	blueDrop = Vector( -330, 700, -24 )
	erased = Vector( 0, 700, -184 )
	runStart = Vector( -420, 560, -182 )
	wallPainted = Vector( 256, 700, -120 )
	wallControl = Vector( 256, 520, -120 )
}

function JI_FloorView()
{
	local eye = Vector( -430, 460, -40 )
	local a = QA_AnglesTo( eye, Vector( 0, 640, -184 ) )
	PW_Noclip( true )
	QA_PlaceEye( eye, a.pitch, a.yaw )
}

function JI_WallView( target )
{
	local eye = Vector( 0, target.y, -110 )
	local a = QA_AnglesTo( eye, target )
	PW_Noclip( true )
	QA_PlaceEye( eye, a.pitch, a.yaw )
}

QA_Do( "setup", function() { PW_Setup(); PW_Noclip( true ) }, 1.0 )

QA_Expect( "sprayers.found", function()
{
	local n = PW_FindSprayers()
	QA_Detail( n + " info_paint_sprayer" )
	return n >= 3
} )

// Let the map's floor sprayers finish their coat.
QA_Do( "floor view", JI_FloorView, 4.0 )
QA_Do( "shot floor_blue", function() { PW_Shot( "floor_blue" ) }, 1.0 )

// Repulsion: drop onto the blue floor.
QA_Do( "drop on blue", function()
{
	PW_PlaceFeet( ::JI.blueDrop, 0.0 )
	PW_StartSampling()
}, 0.05 )
QA_WaitFor( "bounce.sampled", function() { return PW_Sample( 3.0 ) }, 4.0 )
QA_Do( "measure bounce", function()
{
	local r = PW_Rebound()
	PW_Measure( "bounce.rise", r.rise )
	PW_Measure( "bounce.floor", r.floor )
}, 0.1 )

// The same run on the blue floor, before any orange is sprayed (control).
QA_Do( "run on blue", function()
{
	PW_PlaceFeet( Vector( -420, 720, -182 ), 0.0 )
	QA_Press( "forward", 1.6 )
	PW_StartSampling()
}, 0.05 )
QA_WaitFor( "speed.control_sampled", function() { return PW_Sample( 1.6 ) }, 3.0 )
QA_Do( "measure control speed", function() { PW_Measure( "speed.blue_top", PW_TopSpeed() ) }, 0.5 )

// Propulsion: sweep an orange strip along y=560 and run along it.
QA_Do( "spray orange", function()
{
	PW_StartSweep( 0, 2, Vector( -440, 560, -120 ), Vector( 240, 560, -120 ), 90, 0, 4.0 )
}, 0.1 )
QA_WaitFor( "orange.swept", PW_Sweeping, 6.0 )
QA_Do( "orange settles", JI_FloorView, 2.5 )
QA_Do( "shot floor_orange", function() { PW_Shot( "floor_orange" ) }, 1.0 )

QA_Do( "run on orange", function()
{
	PW_PlaceFeet( ::JI.runStart, 0.0 )
	QA_Press( "forward", 1.6 )
	PW_StartSampling()
}, 0.05 )
QA_WaitFor( "speed.sampled", function() { return PW_Sample( 1.6 ) }, 3.0 )
QA_Do( "measure speed", function() { PW_Measure( "speed.orange_top", PW_TopSpeed() ) }, 0.5 )

// Water: erase a patch of the blue floor near x=0, then drop onto it.
QA_Do( "spray water", function()
{
	PW_StartSweep( 1, 4, Vector( -60, 700, -110 ), Vector( 60, 700, -110 ), 90, 0, 3.0 )
}, 0.1 )
QA_WaitFor( "water.swept", PW_Sweeping, 5.0 )
QA_Do( "water settles", JI_FloorView, 2.5 )
QA_Do( "shot floor_erased", function() { PW_Shot( "floor_erased" ) }, 1.0 )

QA_Do( "drop on erased", function()
{
	PW_PlaceFeet( Vector( ::JI.erased.x, ::JI.erased.y, ::JI.blueDrop.z ), 0.0 )
	PW_StartSampling()
}, 0.05 )
QA_WaitFor( "bounce_erased.sampled", function() { return PW_Sample( 3.0 ) }, 4.0 )
QA_Do( "measure erased bounce", function()
{
	local r = PW_Rebound()
	PW_Measure( "bounce_erased.rise", r.rise )
}, 0.1 )

// Conversion gel: a portal shot at the bare end wall first (control), then
// the wall is painted white and shot again.
QA_Do( "aim control", function() { JI_WallView( ::JI.wallControl ) }, 1.0 )
QA_Do( "fire control", function() { QA_Press( "attack", 0.1 ) }, 1.5 )
QA_Do( "measure control portal", function()
{
	PW_Measure( "portal.control_distance", PW_NearestPortal( ::JI.wallControl ) )
}, 0.1 )

QA_Do( "spray white", function()
{
	PW_StartSweep( 2, 3, Vector( 230, 640, -60 ), Vector( 230, 760, -60 ), 0, 0, 3.0 )
}, 0.1 )
QA_WaitFor( "white.swept_high", PW_Sweeping, 5.0 )
QA_Do( "spray white low", function()
{
	PW_StartSweep( 2, 3, Vector( 230, 760, -140 ), Vector( 230, 640, -140 ), 0, 0, 3.0 )
}, 0.1 )
QA_WaitFor( "white.swept_low", PW_Sweeping, 5.0 )
QA_Do( "white settles", function() { JI_WallView( ::JI.wallPainted ) }, 2.5 )
QA_Do( "shot wall_white", function() { PW_Shot( "wall_white" ) }, 1.0 )
QA_Do( "fire white", function() { QA_Press( "attack", 0.1 ) }, 1.5 )
QA_Do( "measure white portal", function()
{
	PW_Measure( "portal.white_distance", PW_NearestPortal( ::JI.wallPainted ) )
}, 0.1 )
QA_Do( "shot wall_portal", function() { PW_Shot( "wall_portal" ) }, 1.0 )

QA_Start( "sp_a3_jump_intro_powers" )
