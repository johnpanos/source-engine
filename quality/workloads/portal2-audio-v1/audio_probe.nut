// Portal 2 audio probe (tools/quality/portal2_audio.py).
//
// Plays a fixed sequence of sounds on sp_a2_triple_laser and prints one
// AUDIO_MARK line per event with its server time. The harness records the
// mixed output (SDL's disk audio driver) and finds each event in it by
// cross-correlating the retail wave files. Retail Portal 2 and this build run
// the same script, so the metrics compare like with like.
//
// Every sound comes from a soundscript entry, as game code plays it: v1
// entries (plain waves) and v2 entries (Portal 2 operator stacks). Events are
// spaced so that their windows do not overlap, and `stopsound` separates the
// music tests. The step list is the contract with the workload file: a mark's
// label names the event or window there.

IncludeScript( "qa/qa_driver" )

::AP <- {
	// Soundscript entries the probe precaches, as map entities and scenes do.
	precache = [
		"Portal.button_down",
		"glados.sp_a1_wakeup_incinerator01",
		"glados.sp_a1_wakeup_incinerator02",
		"music.sp_intro_01.01_awake",
		"music.sp_intro_01.02_restasis",
		"music.sp_a1_intro1_b2b",
		"Weapon_Portalgun.fire_blue",
		"World.RobotArmMotorDown"
	]
	// Played without a precache: retail sends v2 entries as script handles.
	unprecached = "glados.botcoop_artifactone_hub01"
	lineA = "glados.sp_a1_wakeup_incinerator01"
	lineB = "glados.sp_a1_wakeup_incinerator02"
	eye = Vector( 7760, -5400, 64 )
	emitter = null
	direction = Vector( 1, 0, 0 )
	clear = 0.0
}

// The negative control swaps the two lines (portal2_audio.py --seed swap-line
// rewrites this assignment); the harness must then report the wrong line.
::AP_SWAP_LINES <- false

function AP_Mark( label )
{
	printl( "AUDIO_MARK " + label + " " + Time() )
}

function AP_EmitOnPlayer( label, entry )
{
	AP_Mark( label )
	QA_Player().EmitSound( entry )
}

function AP_Line( first )
{
	if ( ::AP_SWAP_LINES )
		first = !first
	return first ? ::AP.lineA : ::AP.lineB
}

// The emitter is cube 1 with its motion disabled, so it makes no impact sounds.
function AP_PlaceEmitter( distance )
{
	local cube = ::AP.emitter
	if ( distance > ::AP.clear - 48.0 )
		distance = ::AP.clear - 48.0
	cube.SetOrigin( ::AP.eye + QA_Scale( ::AP.direction, distance ) - Vector( 0, 0, 16 ) )
	cube.SetVelocity( Vector( 0, 0, 0 ) )
	QA_Log( "emitter at " + QA_Vec( cube.GetOrigin() ) + " distance " + distance )
}

function AP_EmitAt( label, entry, distance )
{
	AP_PlaceEmitter( distance )
	AP_Mark( label )
	::AP.emitter.EmitSound( entry )
}

QA_Do( "precache", function()
{
	foreach ( entry in ::AP.precache )
		QA_Player().PrecacheSoundScript( entry )
}, 0.5 )

QA_Do( "setup", function()
{
	SendToConsole( "sv_cheats 1; snd_mute_losefocus 0; volume 1; con_drawnotify 0; cl_drawhud 0" )
}, 0.5 )

// Stand in the entry hall facing east; the emitter line is the clearest of
// four horizontal directions (the same on both builds: the map decides).
QA_Do( "place the listener", function()
{
	QA_PlaceEye( ::AP.eye, 0.0, 0.0 )
	local best = 0.0
	foreach ( yaw in [ 0.0, 90.0, 180.0, 270.0 ] )
	{
		local dir = Vector( cos( QA_Rad( yaw ) ), sin( QA_Rad( yaw ) ), 0 )
		local reach = 1024.0 * TraceLine( ::AP.eye, ::AP.eye + QA_Scale( dir, 1024.0 ), QA_Player() )
		if ( reach > best )
		{
			best = reach
			::AP.direction = dir
		}
	}
	::AP.clear = best
	::AP.emitter = QA_Ent( "new_box" )
	QA_Fire( "new_box", "DisableMotion" )
	QA_Log( "emitter direction " + QA_Vec( ::AP.direction ) + " clear " + best )
}, 6.0 )

// The map's own soundscape at the listener, then silence after stopsound.
QA_Do( "ambient", function() { AP_Mark( "ambient" ) }, 5.0 )
QA_Do( "silence", function() { AP_Mark( "silence" ); SendToConsole( "stopsound" ) }, 2.5 )

// A 2D wave through the console: the alignment reference for every run.
QA_Do( "calib", function()
{
	AP_Mark( "calib" )
	SendToConsole( "play buttons/button_synth_positive_01.wav" )
}, 2.5 )

QA_Do( "v1 item on the player", function() { AP_EmitOnPlayer( "item", "Portal.button_down" ) }, 2.5 )
QA_Do( "vo", function() { AP_EmitOnPlayer( "vo", AP_Line( true ) ) }, 7.5 )
QA_Do( "vo unprecached", function() { AP_EmitOnPlayer( "vo_unprecached", ::AP.unprecached ) }, 6.0 )

QA_Do( "music", function() { AP_EmitOnPlayer( "music", "music.sp_intro_01.01_awake" ) }, 8.0 )
QA_Do( "stop music", function() { AP_Mark( "music_stop" ); SendToConsole( "stopsound" ) }, 1.5 )

// The start stack of this entry starts music.sp_a1_intro1_b2b beside its own wave.
QA_Do( "chain", function() { AP_EmitOnPlayer( "chain", "music.sp_intro_01.02_restasis" ) }, 12.0 )
QA_Do( "stop chain", function() { AP_Mark( "chain_stop" ); SendToConsole( "stopsound" ) }, 1.5 )

QA_Do( "portal gun", function() { AP_EmitOnPlayer( "gun", "Weapon_Portalgun.fire_blue" ) }, 2.5 )

// Line B on the same entity and channel cuts line A.
QA_Do( "line A", function() { AP_EmitOnPlayer( "line_a", AP_Line( true ) ) }, 2.0 )
QA_Do( "line B", function() { AP_EmitOnPlayer( "line_b", AP_Line( false ) ) }, 6.5 )

// Music under a GLaDOS line: the dialog mix layer ducks the Music group.
QA_Do( "duck music", function() { AP_EmitOnPlayer( "duck_music", "music.sp_a1_intro1_b2b" ) }, 4.0 )
QA_Do( "duck line", function() { AP_EmitOnPlayer( "duck_line", AP_Line( false ) ) }, 6.0 )
QA_Do( "duck after", function() { AP_Mark( "duck_after" ) }, 3.0 )
QA_Do( "stop duck", function() { AP_Mark( "duck_stop" ); SendToConsole( "stopsound" ) }, 1.5 )

// Distance falloff: one v1 and one v2 entry from an entity at three distances.
QA_Do( "near v1", function() { AP_EmitAt( "near_v1", "Portal.button_down", 128.0 ) }, 2.0 )
QA_Do( "mid v1", function() { AP_EmitAt( "mid_v1", "Portal.button_down", 384.0 ) }, 2.0 )
QA_Do( "far v1", function() { AP_EmitAt( "far_v1", "Portal.button_down", 768.0 ) }, 2.0 )
QA_Do( "near v2", function() { AP_EmitAt( "near_v2", "World.RobotArmMotorDown", 128.0 ) }, 2.5 )
QA_Do( "mid v2", function() { AP_EmitAt( "mid_v2", "World.RobotArmMotorDown", 384.0 ) }, 2.5 )
QA_Do( "far v2", function() { AP_EmitAt( "far_v2", "World.RobotArmMotorDown", 768.0 ) }, 2.5 )

QA_Do( "end", function()
{
	AP_Mark( "end" )
	SendToConsole( "play buttons/button_synth_positive_01.wav" )
}, 3.0 )

QA_Start( "portal2_audio" )
