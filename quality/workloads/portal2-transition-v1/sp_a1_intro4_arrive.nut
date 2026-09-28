// sp_a1_intro3 -> sp_a1_intro4, part 2: arrive in sp_a1_intro4 through the
// level change (tools/quality/portal2_scenarios.py, "arrival").
//
// The engine places the player at the same offset from sp_a1_intro4's
// info_landmark_entry as it left sp_a1_intro3's info_landmark_exit, inside the
// trigger that runs OnPostTransition(), which teleports the player into the
// arrival elevator. A misplaced player never touches that trigger and falls.
// The elevator then descends and opens, and the player walks out.

IncludeScript( "qa/qa_driver" )

::ARRIVE_ELEVATOR <- "arrival_elevator-elevator_1"

QA_Do( "close the transition window", function()
{
	printl( "QA_WINDOW transition END" )
	printl( "QA_WINDOW arrival BEGIN" )
} )

QA_Expect( "arrival.map", function()
{
	QA_Detail( GetMapName() )
	return GetMapName() == "sp_a1_intro4"
} )

QA_WaitFor( "arrival.in_elevator", function()
{
	local axis = QA_Ent( ::ARRIVE_ELEVATOR ).GetOrigin()
	local player = QA_Player().GetOrigin()
	local dx = player.x - axis.x
	local dy = player.y - axis.y
	QA_Detail( "player " + player + " elevator " + axis )
	// Inside the car: on its axis and above the floor of the arrival room.
	return sqrt( dx * dx + dy * dy ) < 48.0 && player.z > -120.0 && player.z < 700.0
}, 10.0 )

QA_Expect( "arrival.stats_controller", function()
{
	local controller = Entities.FindByClassname( null, "portal_stats_controller" )
	QA_Detail( controller == null ? "none" : "present" )
	return controller != null
} )

QA_WaitFor( "arrival.elevator_down", function()
{
	local train = QA_Ent( ::ARRIVE_ELEVATOR ).GetOrigin()
	local bottom = QA_Ent( "@elevator_1_bottom_path_1" ).GetOrigin()
	QA_Detail( "train z=" + train.z + " bottom z=" + bottom.z )
	return fabs( train.z - bottom.z ) < 4.0
}, 20.0 )

// arrival_elevator-open runs 0.5 s after the train stops; the door animation
// takes about two seconds.
QA_Sleep( 3.0 )

QA_Do( "save after arriving", function()
{
	SendToConsole( "save qa_transition_arrival" )
} )

// @arrival_teleport faces the player along +x, out of the open door.
QA_Do( "walk out", function()
{
	SendToConsole( "+forward" )
} )

QA_WaitFor( "arrival.walked_out", function()
{
	local player = QA_Player().GetOrigin()
	QA_Detail( "player " + player )
	return player.x > -1400.0
}, 6.0 )

QA_Do( "stop", function()
{
	SendToConsole( "-forward" )
	printl( "QA_WINDOW arrival END" )
}, 1.0 )

QA_Start( "sp_a1_intro3_to_intro4" )
