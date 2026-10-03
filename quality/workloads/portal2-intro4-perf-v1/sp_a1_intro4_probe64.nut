// User-selected High performance fixture (2026-10-01): sp_a1_intro4_probe64.
// Reuses the retained Intro4 profiling camera, with both orientations warmed
// before the measured bracket. This is a camera workload, not story parity.

IncludeScript( "qa/qa_driver" )
IncludeScript( "qa/floor_options" )

::INTRO4_EYE <- Vector( 145, -440, 90 )

function Intro4_View( yaw )
{
	QA_PlaceEye( ::INTRO4_EYE, 8.0, yaw )
	SendToConsole( "getpos" )
}

function Intro4_Position()
{
	if ( ::FLOOR_OPTIONS.preview )
		SendToConsole( "screenshot" )
	QA_Detail( "eye " + QA_Vec( QA_Player().EyePosition() ) )
	return QA_Dist( QA_Player().EyePosition(), ::INTRO4_EYE ) < 1.0
}

QA_Expect( "map.loaded", function()
{
	QA_Detail( GetMapName() )
	// VScript intentionally sees the original name for shipped story tables.
	return GetMapName() == "sp_a1_intro4"
} )

QA_Do( "verify installed map", function()
{
	SendToConsole( "echo QA_WINDOW map_identity BEGIN; status; echo QA_WINDOW map_identity END" )
}, 0.1 )

QA_Do( "warm arrival", function()
{
	SendToConsole( "sv_cheats 1; noclip; god" )
	Intro4_View( 20.0 )
	SendToConsole( "vk_frame_mark warm_arrival" )
}, 10.0 )

QA_Do( "warm reverse", function()
{
	Intro4_View( 200.0 )
	SendToConsole( "vk_frame_mark warm_reverse" )
}, 10.0 )

QA_Do( "settle arrival", function() { Intro4_View( 20.0 ) }, 2.0 )
QA_Expect( "view.arrival", Intro4_Position )

QA_Do( "measure arrival", function()
{
	SendToConsole( "vk_frame_mark floor_begin; vk_frame_mark arrival" )
}, 8.0 )

QA_Do( "turn toward reverse", function()
{
	Intro4_View( 200.0 )
	SendToConsole( "vk_frame_mark reverse" )
}, 8.0 )
QA_Expect( "view.reverse", Intro4_Position )

QA_Do( "return to arrival", function()
{
	Intro4_View( 20.0 )
	SendToConsole( "vk_frame_mark return" )
}, 4.0 )
QA_Expect( "view.return", Intro4_Position )

// Retain frames after the bracket so delayed GPU timestamps can drain.
QA_Do( "end measured bracket", function()
{
	SendToConsole( "vk_frame_mark floor_end" )
}, 2.0 )

QA_Start( "sp_a1_intro4_probe64" )
