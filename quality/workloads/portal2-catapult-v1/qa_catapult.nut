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

// Starts recording the player's flight under <key>: QA_FlightTick samples it
// every driver tick and finds touchdown by extrapolating the last airborne
// sample down to the floor the player lands on.
function QA_FlightBegin( key )
{
	::QA.marks[key] <- {
		floorZ = 0.0
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
	// On the ground now: the origin rests on the landing floor. Solve
	// last.z + vz s - g s^2 / 2 = floorZ for the later root.
	f.floorZ = sample.pos.z
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
		QA_FlightBegin( prefix )
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

	// Comes to rest on the floor it landed on (a landing slides on a little).
	QA_WaitFor( prefix + ".rest", function() : ( prefix, target )
	{
		local p = QA_Player()
		local f = ::QA.marks[prefix]
		QA_Detail( "at " + QA_Vec( p.GetOrigin() ) + " vel " + QA_Vec( p.GetVelocity() ) +
		           format( " landing floor z=%.1f", f.floorZ ) )
		return p.GetVelocity().Length() < 1.0 && fabs( p.GetOrigin().z - f.floorZ ) < 2.0 &&
		       QA_FlatDist( p.GetOrigin(), QA_Ent( target ).GetOrigin() ) < 160.0
	}, 4.0 )
}

// Contact detection for a thrown object sampled by position only. <track>
// holds { samples = [] }; returns the touchdown point once the newest sample
// leaves the free-fall path (g = sv_gravity) predicted from the two before
// it, else null.
function QA_BallisticContact( track, sample )
{
	local s = track.samples
	s.append( sample )
	if ( s.len() < 3 )
		return null
	local p0 = s[s.len() - 3], p1 = s[s.len() - 2], p2 = s[s.len() - 1]
	local d01 = p1.t - p0.t, d12 = p2.t - p1.t
	if ( d01 <= 0.0 || d12 <= 0.0 )
		return null
	local g = ::QA_CATAPULT_GRAVITY
	// Velocity at p1 from the chord p0 -> p1 under constant gravity.
	local v = QA_Scale( p1.pos - p0.pos, 1.0 / d01 )
	v.z -= 0.5 * g * d01
	local predicted = p1.pos + QA_Scale( v, d12 )
	predicted.z -= 0.5 * g * d12 * d12
	// Still flying (or not yet thrown: resting samples predict a fall).
	if ( QA_Dist( predicted, p2.pos ) < 4.0 || v.z > 0.0 || v.Length() < 100.0 )
		return null
	// Solve p1.z + v.z s - g s^2 / 2 = p2.z for the later root.
	local a = -0.5 * g, b = v.z, c = p1.pos.z - p2.pos.z
	local disc = b * b - 4.0 * a * c
	local t = disc < 0.0 ? 0.0 : ( -b - sqrt( disc ) ) / ( 2.0 * a )
	t = t < 0.0 ? 0.0 : ( t > d12 ? d12 : t )
	local hit = Vector( p1.pos.x + v.x * t, p1.pos.y + v.y * t, p2.pos.z )
	QA_Log( "contact " + QA_Vec( hit ) + " after " + QA_Vec( p1.pos ) + " vel " + QA_Vec( v ) )
	return hit
}
