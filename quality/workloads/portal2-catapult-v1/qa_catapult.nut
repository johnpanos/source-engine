// Shared helpers for the trigger_catapult scenarios; include after qa_driver.
//
// The oracle is retail Portal 2's launch rule: a targeted catapult throws a
// player at constant time t = |d| / playerSpeed along d = (target - 32 z) -
// origin, adding g t / 2 upward, with g = sv_gravity = 600. Physics objects
// use physicsSpeed and aim at the target itself. A correct launch therefore
// comes down on the target, and flight gravity measures sv_gravity.

::QA_CATAPULT_GRAVITY <- 600.0

// Retail CTriggerCatapult::CalculateLaunchVector.
function QA_CatapultLaunchVector( from, target, speed, isPlayer )
{
	local d = target - from
	if ( isPlayer )
		d.z -= 32.0
	local t = d.Length() / speed
	local v = QA_Scale( d, 1.0 / t )
	v.z += ::QA_CATAPULT_GRAVITY * t * 0.5
	return v
}

function QA_FlatDist( a, b )
{
	local dx = a.x - b.x, dy = a.y - b.y
	return sqrt( dx * dx + dy * dy )
}

// Height of the floor under <ent>, where a standing player's origin rests.
function QA_FloorBelow( ent )
{
	local top = ent.GetOrigin()
	local bottom = top - Vector( 0, 0, 512 )
	local fraction = TraceLine( top, bottom, null )
	if ( fraction >= 1.0 )
		throw "no floor below " + ent.GetName()
	return top.z - 512.0 * fraction
}

// Starts recording the player's flight under <key>: QA_FlightTick samples it
// every driver tick and finds touchdown by extrapolating the last airborne
// sample down to <floorZ> (the player origin height on the landing floor).
function QA_FlightBegin( key, floorZ )
{
	::QA.marks[key] <- {
		floorZ = floorZ
		last = null
		apex = -100000.0
		gravity = []
		touchdown = null
		airborne = false
	}
}

// Returns true once the player has landed (or stopped falling) after being
// airborne. Call from a QA_WaitFor predicate.
function QA_FlightTick( key )
{
	local f = ::QA.marks[key]
	local p = QA_Player()
	local sample = { t = Time(), pos = p.GetOrigin(), vel = p.GetVelocity() }
	if ( sample.pos.z > f.apex )
		f.apex = sample.pos.z
	local flying = sample.vel.z != 0.0
	if ( flying )
	{
		if ( f.last != null && f.airborne && sample.t > f.last.t )
			f.gravity.append( ( f.last.vel.z - sample.vel.z ) / ( sample.t - f.last.t ) )
		f.airborne = true
		f.last = sample
		QA_Detail( "flying at " + QA_Vec( sample.pos ) + " vel " + QA_Vec( sample.vel ) )
		return false
	}
	if ( !f.airborne || f.last == null )
	{
		QA_Detail( "not airborne at " + QA_Vec( sample.pos ) + " vel " + QA_Vec( sample.vel ) )
		return false
	}
	// Solve last.z + vz s - g s^2 / 2 = floorZ for the later root.
	local l = f.last
	local a = -0.5 * ::QA_CATAPULT_GRAVITY, b = l.vel.z, c = l.pos.z - f.floorZ
	local disc = b * b - 4.0 * a * c
	local s = disc < 0.0 ? 0.0 : ( -b - sqrt( disc ) ) / ( 2.0 * a )
	if ( s < 0.0 )
		s = 0.0
	f.touchdown = Vector( l.pos.x + l.vel.x * s, l.pos.y + l.vel.y * s, f.floorZ )
	QA_Log( "touchdown " + QA_Vec( f.touchdown ) + " from " + QA_Vec( l.pos ) + " vel " +
	        QA_Vec( l.vel ) + format( " apex=%.1f", f.apex ) )
	return true
}

// Median of the per-tick gravity samples; robust to the launch tick.
function QA_FlightGravity( key )
{
	local g = clone ::QA.marks[key].gravity
	if ( g.len() == 0 )
		return 0.0
	g.sort()
	return g[g.len() / 2]
}

// Checks <prefix>.launched, .landed, .gravity, .touchdown and .rest for a
// player standing on <catapult>, which must throw them onto <target>.
function QA_PlayerFling( prefix, catapult, target, yaw )
{
	QA_Do( prefix + ": step on", function() : ( prefix, catapult, target, yaw )
	{
		::QA.marks[prefix + "_fired"] <- QA_Fired( catapult, "OnCatapulted" )
		QA_FlightBegin( prefix, QA_FloorBelow( QA_Ent( target ) ) )
		QA_StandIn( QA_Ent( catapult ), yaw )
	}, 0.0 )

	QA_WaitFor( prefix + ".launched", function() : ( prefix, catapult )
	{
		local n = QA_Fired( catapult, "OnCatapulted" ) - ::QA.marks[prefix + "_fired"]
		local p = QA_Player()
		QA_Detail( "fired=" + n + " at " + QA_Vec( p.GetOrigin() ) + " vel " + QA_Vec( p.GetVelocity() ) )
		return n == 1 && p.GetVelocity().z > 0.0
	}, 2.0 )

	QA_WaitFor( prefix + ".landed", function() : ( prefix )
	{
		return QA_FlightTick( prefix )
	}, 8.0 )

	QA_Expect( prefix + ".gravity", function() : ( prefix )
	{
		local g = QA_FlightGravity( prefix )
		QA_Detail( format( "median flight gravity %.1f over %d samples (expected %.0f)", g,
		           ::QA.marks[prefix].gravity.len(), ::QA_CATAPULT_GRAVITY ) )
		return fabs( g - ::QA_CATAPULT_GRAVITY ) < 15.0
	} )

	QA_Expect( prefix + ".touchdown", function() : ( prefix, target )
	{
		local td = ::QA.marks[prefix].touchdown
		local goal = QA_Ent( target ).GetOrigin()
		local miss = QA_FlatDist( td, goal )
		QA_Detail( "touchdown " + QA_Vec( td ) + " target " + QA_Vec( goal ) + format( " miss=%.1f", miss ) )
		return miss < 32.0
	} )

	QA_WaitFor( prefix + ".rest", function() : ( prefix, target )
	{
		local p = QA_Player()
		local f = ::QA.marks[prefix]
		QA_Detail( "at " + QA_Vec( p.GetOrigin() ) + " vel " + QA_Vec( p.GetVelocity() ) +
		           format( " floor z=%.1f", f.floorZ ) )
		return p.GetVelocity().Length() < 1.0 && fabs( p.GetOrigin().z - f.floorZ ) < 2.0 &&
		       QA_FlatDist( p.GetOrigin(), QA_Ent( target ).GetOrigin() ) < 160.0
	}, 4.0 )
}
