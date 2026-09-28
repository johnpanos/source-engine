// sp_a1_intro3 -> sp_a1_intro4, part 1: leave sp_a1_intro3 through its own
// departure elevator (tools/quality/portal2_scenarios.py, "arrival").
//
// The player is moved through the triggers of the exit corridor: the one
// that opens the elevator tube and the one that clears its blockage, then
// into the elevator. The map's own logic does the rest: the trigger in the
// elevator starts the ride (StartMoving sets map_wants_save_disable), the
// shaft's last path_track runs FailSafeTransition, @transition_from_map
// teleports the player to @exit_teleport in the transition room, whose
// trigger runs TransitionFromMap(). That fires portal_stats_controller's
// OnLevelEnd, and the controller runs RealTransitionFromMap(), which fires
// @changelevel. The level change resets the VM, so this part hands off during
// the ride; the harness judges the rest from the console window "transition"
// and sp_a1_intro4_arrive.nut.

IncludeScript( "qa/qa_driver" )

::DEPART_ELEVATOR <- "departure_elevator-elevator_1"

QA_Do( "open the transition window", function()
{
	printl( "QA_WINDOW transition BEGIN" )
	SendToConsole( "sv_cheats 1; god" )
}, 0.5 )

QA_Do( "watch the elevator and the exit relay", function()
{
	QA_Watch( "departure_elevator-elevator_arrive", "OnTrigger" )
	// The handler logs each firing even after the hand-off.
	QA_Watch( "@transition_from_map", "OnTrigger" )
} )

QA_Do( "walk into the tube door trigger", function()
{
	QA_Player().SetOrigin( Vector( -1344, 3704, -150 ) )
}, 1.0 )

QA_Do( "walk into the blockage trigger", function()
{
	QA_Player().SetOrigin( Vector( -1344, 3936, -120 ) )
} )

QA_WaitFor( "departure.elevator_arrived", function()
{
	QA_Detail( "elevator_arrive fired " + QA_Fired( "departure_elevator-elevator_arrive", "OnTrigger" ) +
		" times" )
	return QA_Fired( "departure_elevator-elevator_arrive", "OnTrigger" ) > 0
}, 10.0 )

// The relay opens the tube 2 s later and removes the elevator's player clip
// at 3.5 s.
QA_Sleep( 4.5 )

QA_Do( "step into the elevator", function()
{
	QA_Player().SetOrigin( Vector( -1344, 4304, -330 ) )
} )

QA_WaitFor( "departure.elevator_descends", function()
{
	local train = QA_Ent( ::DEPART_ELEVATOR ).GetOrigin()
	local player = QA_Player().GetOrigin()
	QA_Detail( "train z=" + train.z + " player z=" + player.z )
	// The player rides the train: it stays inside the car.
	return train.z < -1000.0 && fabs( player.z - train.z ) < 96.0
}, 20.0 )

QA_Expect( "departure.stats_controller", function()
{
	local controller = Entities.FindByClassname( null, "portal_stats_controller" )
	QA_Detail( controller == null ? "none" : "present" )
	return controller != null
} )

// StartMoving set map_wants_save_disable; the window must show this refused.
QA_Do( "save during the ride", function()
{
	SendToConsole( "save qa_transition_ride" )
} )

// The engine names the level change's landmark only at developer 1.
QA_Do( "hand off", function()
{
	SendToConsole( "developer 1" )
	QA_Handoff()
} )

QA_Start( "sp_a1_intro3_to_intro4" )
