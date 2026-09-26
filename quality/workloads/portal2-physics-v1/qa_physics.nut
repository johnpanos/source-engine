// Shared helpers for the Portal 2 physics scenarios (portal2-physics-v1);
// include after qa_driver. tools/quality/portal2_physics.py runs the same
// scripts on retail Portal 2 and on this build and compares the QA_METRIC
// lines they print with operation-specific tolerances.
//
//   QA_METRIC <scenario>.<name> <number>
//
// Sampling: PH_Track records an entity every server tick (a RunScriptCode
// event rescheduled 0.01 s ahead fires on the next 60 Hz tick), so exit
// speeds and teleports are measured from consecutive ticks, not from the
// driver's 0.1 s steps.

::PH <- {
	tracks = {}
}

function PH_Metric( name, value )
{
	printl( "QA_METRIC " + ::QA.scenario + "." + name + " " + format( "%.3f", value.tofloat() ) )
}

function PH_Flag( name, ok )
{
	PH_Metric( name, ok ? 1.0 : 0.0 )
}

// Places portal <index> (0 blue, 1 orange) of the single-player linkage
// group at <pos>, facing along (<pitch>, <yaw>), with the cheat command
// portal_place: it creates the portal when it does not exist and skips the
// placement rules, so the fixture decides exactly where each portal sits.
// (Retail's ent_create makes no prop_portal from a script.) Needs sv_cheats;
// the command runs on the next frame, so look the portal up a step later.
function PH_Place( index, pos, pitch, yaw )
{
	SendToConsole( format( "portal_place 0 %d %.2f %.2f %.2f %.2f %.2f 0", index, pos.x, pos.y, pos.z, pitch, yaw ) )
}

// The portal placed at <pos>, or null.
function PH_PortalAt( pos )
{
	return Entities.FindByClassnameNearest( "prop_portal", pos, 4.0 )
}

// Opens the portal placed at <pos>; throws when there is none.
function PH_Activate( pos )
{
	local portal = PH_PortalAt( pos )
	if ( portal == null )
		throw "no portal at " + QA_Vec( pos )
	EntFireByHandle( portal, "SetActivatedState", "1", 0.0, null, null )
	return portal
}

// --- Per-tick tracking ---------------------------------------------------

// Samples <ent> every tick for <seconds> under ::PH.tracks[key]: a list of
// { t, pos, vel }. vel is the entity's velocity (players) or the position
// difference to the previous tick (VPhysics props, whose scripted velocity
// is not maintained).
function PH_Track( key, ent, seconds, usePositions )
{
	::PH.tracks[key] <- { ent = ent, until = Time() + seconds, samples = [], positions = usePositions }
	PH_TrackTick( key )
}

function PH_TrackTick( key )
{
	local track = ::PH.tracks[key]
	if ( track.ent == null || !track.ent.IsValid() )
		return
	local pos = track.positions ? track.ent.GetCenter() : track.ent.GetOrigin()
	local sample = { t = Time(), pos = pos, vel = track.ent.GetVelocity() }
	local n = track.samples.len()
	if ( track.positions )
	{
		sample.vel = Vector( 0, 0, 0 )
		if ( n > 0 && sample.t > track.samples[n - 1].t )
			sample.vel = QA_Scale( pos - track.samples[n - 1].pos, 1.0 / ( sample.t - track.samples[n - 1].t ) )
	}
	// Two samples in one tick carry no information.
	if ( n == 0 || sample.t > track.samples[n - 1].t )
		track.samples.append( sample )
	if ( Time() < track.until )
		EntFireByHandle( ::QA.driver, "RunScriptCode", "PH_TrackTick(\"" + key + "\")", 0.01, null, null )
}

function PH_Samples( key )
{
	return ::PH.tracks[key].samples
}

// Index of the first sample that jumped more than <distance> units from the
// one before it (a teleport), or -1.
function PH_TeleportIndex( key, distance )
{
	local s = PH_Samples( key )
	for ( local i = 1; i < s.len(); i++ )
	{
		if ( QA_Dist( s[i].pos, s[i - 1].pos ) > distance )
			return i
	}
	return -1
}

// Logs every sample of <key> (for reading a failure's lead-up).
function PH_Dump( key )
{
	foreach ( s in PH_Samples( key ) )
		QA_Log( "track " + key + format( " t=%.3f", s.t ) + " pos " + QA_Vec( s.pos ) + " vel " + QA_Vec( s.vel ) )
}

function PH_FlatDist( a, b )
{
	local dx = a.x - b.x, dy = a.y - b.y
	return sqrt( dx * dx + dy * dy )
}

function PH_Ray( from, dir, length )
{
	return length * TraceLine( from, from + QA_Scale( dir, length ), null )
}
