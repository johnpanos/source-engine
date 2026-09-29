// Walks the player through a map with the player's own input.
//
// A scenario gives the walker a goal (a point and a radius). The walker plans
// a route with A* over a grid it measures in the running game: each grid cell
// is a place the player can stand, found with TraceLine (the world and static
// props) and with the brushes TraceLine does not see (below). Neighbouring
// cells connect when the player can walk between them (a step up of at most
// 18 units, nothing in the way at knee and head height) or drop down from one
// to the other. A route that cannot be found yet (a door still closed) is
// planned again every second. Routes pay for running close to walls, so they
// keep to the middle of a corridor. Where the player still cannot pass, the
// walker notices it has stopped making progress, jumps once, then backs off,
// marks the node ahead impassable and plans again.
//
// TraceLine does not see player clips, nor brush entities that move (doors,
// trains, lifts): the trace filter skips MOVETYPE_PUSH entities for its mask.
// The harness installs both from the map's BSP as qa/qa_clips.nut; the walker
// places each entity's brushes at its current origin and tests the floor
// probe and the player's hull against them too, so it can stand on a lift
// and stops at a door that is shut.
//
// The walker then drives the player along the route the way a user does,
// through console commands only: +forward to walk, +left/+right with
// cl_yawspeed set in proportion to the heading error to turn (a keyboard or
// stick turn), +lookup/+lookdown with cl_pitchspeed to aim, +jump when stuck.
// Nothing sets the player's position, angles or velocity. Positions are only
// read, as a player reads the screen.
//
// The server only knows the player's body yaw (it lags the view by up to
// 45 degrees while standing) and no pitch, so the walker keeps its own view
// angles: the spawn view, then every turn it commands, integrated at the rate
// it set. A player keeps track of where they look the same way.
//
// Every 0.25 s it prints `QA_PATH <scenario> t=<time> <x> <y> <z> <yaw> <pitch>
// <state> body=<body yaw>` (the view it keeps) so the harness can draw and
// compare the route on both builds.
//
// Squirrel 2.2 (retail Portal 2): closures do not capture locals, so all state
// lives in the root table ::WALK.

::WALK <- {
	cell = 16.0          // grid cell size
	step = 18.0          // highest step the player walks up
	head = 72.0          // standing height
	body = 30.0          // height of the clearance and line-of-sight traces
	brow = 62.0          // second line-of-sight height (low beams)
	radius = 16.0        // half the player's width
	maxDrop = 1400.0     // deepest drop the walker takes
	dropPenalty = 400.0  // added cost of a drop: taken only when needed
	// The VM ends a call that runs longer than 30 ms, so planning is spread
	// over ticks by expansions and by traces.
	expandPerTick = 64
	tracesPerTick = 900
	maxExpand = 40000
	ignore = null        // one entity TraceLine ignores (the carried core)

	nodes = {}
	edges = {}
	blocked = {}
	blockedNodes = {}
	explained = {}
	traces = 0
	clipGrid = {}
	clipBucket = 256.0
	// The hull tested against brushes, a little inside the player's (32 wide,
	// 72 tall), as the engine's own traces leave a small margin: the player
	// passes under a door that leaves exactly 72 units.
	hullRadius = 15.0
	hullTop = 70.0
	solids = []
	movers = []

	state = "idle"       // idle planning walking arrived failed looking looked
	label = ""
	goal = null
	goalFloor = 0.0
	goalRadius = 40.0
	plan = null
	path = []
	index = 0
	replans = 0
	maxReplans = 12
	bestProgress = -1.0e9
	bestTime = 0.0
	jumped = false
	jumpUntil = 0.0

	look = null          // point to aim at (looking), or null
	lookSteady = 0
	pitch = 0.0          // pitch held while walking
	view = { pitch = 0.0, yaw = 0.0 }   // the view, from the inputs sent
	lastTick = -1.0
	yawSpeed = 210.0
	pitchSpeed = 225.0
	pressed = {}
	nextPath = 0.0
	started = false
	driver = null
}

// --- Input -------------------------------------------------------------------

function Walk_Key( name, down )
{
	local was = ( name in ::WALK.pressed ) && ::WALK.pressed[name]
	if ( was == down )
		return
	::WALK.pressed[name] <- down
	SendToConsole( ( down ? "+" : "-" ) + name )
}

function Walk_ReleaseAll()
{
	foreach ( name in [ "forward", "back", "left", "right", "lookup", "lookdown", "jump", "duck" ] )
		Walk_Key( name, false )
}

// Sets a turn-rate cvar when it moves by more than 10 %. The rate is sent
// rounded, and the view is integrated at exactly the value sent.
function Walk_Rate( cvar, current, value )
{
	if ( fabs( value - current ) <= 0.1 * current )
		return current
	value = floor( value * 10.0 + 0.5 ) / 10.0
	SendToConsole( cvar + " " + format( "%.1f", value ) )
	return value
}

// Integrates the turn keys held since the last tick into the view.
function Walk_Integrate( now )
{
	local dt = ::WALK.lastTick < 0.0 ? 0.0 : now - ::WALK.lastTick
	::WALK.lastTick = now
	if ( dt <= 0.0 || dt > 0.5 )
		return
	local p = ::WALK.pressed
	local view = ::WALK.view
	if ( ( "left" in p ) && p.left )
		view.yaw += ::WALK.yawSpeed * dt
	if ( ( "right" in p ) && p.right )
		view.yaw -= ::WALK.yawSpeed * dt
	if ( ( "lookdown" in p ) && p.lookdown )
		view.pitch += ::WALK.pitchSpeed * dt
	if ( ( "lookup" in p ) && p.lookup )
		view.pitch -= ::WALK.pitchSpeed * dt
	// cl_pitchdown / cl_pitchup
	if ( view.pitch > 89.0 )
		view.pitch = 89.0
	if ( view.pitch < -89.0 )
		view.pitch = -89.0
	view.yaw = Walk_AngleDelta( view.yaw, 0.0 )
}

function Walk_AngleDelta( to, from )
{
	local d = to - from
	while ( d > 180.0 )
		d -= 360.0
	while ( d < -180.0 )
		d += 360.0
	return d
}

// Turn rate in degrees per second: proportional to the error, so the view
// settles in a few ticks without overshooting.
function Walk_TurnRate( error )
{
	local rate = fabs( error ) * 6.0
	if ( rate < 4.0 )
		rate = 4.0
	if ( rate > 300.0 )
		rate = 300.0
	return rate
}

// Holds the view yaw towards <yaw>; true when within <tolerance>.
function Walk_SteerYaw( yaw, tolerance )
{
	local error = Walk_AngleDelta( yaw, ::WALK.view.yaw )
	if ( fabs( error ) <= tolerance )
	{
		Walk_Key( "left", false )
		Walk_Key( "right", false )
		return true
	}
	::WALK.yawSpeed = Walk_Rate( "cl_yawspeed", ::WALK.yawSpeed, Walk_TurnRate( error ) )
	// +left turns towards larger yaw.
	Walk_Key( "left", error > 0.0 )
	Walk_Key( "right", error < 0.0 )
	return false
}

function Walk_SteerPitch( pitch, tolerance )
{
	local error = pitch - ::WALK.view.pitch
	if ( fabs( error ) <= tolerance )
	{
		Walk_Key( "lookup", false )
		Walk_Key( "lookdown", false )
		return true
	}
	::WALK.pitchSpeed = Walk_Rate( "cl_pitchspeed", ::WALK.pitchSpeed, Walk_TurnRate( error ) )
	// +lookdown increases pitch (looks down).
	Walk_Key( "lookdown", error > 0.0 )
	Walk_Key( "lookup", error < 0.0 )
	return false
}

// --- Measuring the map -------------------------------------------------------

function Walk_Trace( a, b )
{
	::WALK.traces++
	return TraceLine( a, b, ::WALK.ignore )
}

// Floor below (x, y) within maxDrop of <top>; null if <top> is inside
// geometry or there is no floor.
function Walk_FloorBelow( x, y, top )
{
	local start = Vector( x, y, top )
	if ( Walk_Trace( start, Vector( x, y, top + 1.0 ) ) < 1.0 )
		return null
	local brush = Walk_BrushFloor( x, y, top, top - ::WALK.maxDrop )
	if ( brush == "inside" )
		return null
	local f = Walk_Trace( start, Vector( x, y, top - ::WALK.maxDrop ) )
	local z = f >= 1.0 ? null : top - f * ::WALK.maxDrop
	if ( brush != null && ( z == null || brush > z ) )
		z = brush
	return z
}

// --- Collision TraceLine does not see --------------------------------------
//
// ::WALK_CLIPS (world player clips) and ::WALK_MOVERS (doors, trains and
// other brush entities, which the trace filter skips) come from
// qa/qa_clips.nut. Each brush is [ lo xyz, hi xyz, [ nx ny nz d ... ] ],
// solid where n . x <= d; a mover's brushes are in model coordinates and move
// with the entity's origin. The player's hull is tested lifted by a step
// (feet +19 to +70, 30 wide), so a clip ramp over stairs never blocks.

function Walk_BucketKey( bx, by )
{
	return bx + "," + by
}

function Walk_AddSolid( brush, mover, margin )
{
	local r = ::WALK.hullRadius
	local planes = brush[6]
	local expanded = []
	for ( local k = 0; k < planes.len(); k += 4 )
	{
		local nx = planes[k], ny = planes[k + 1], nz = planes[k + 2]
		local low = 19.0 * nz, high = ::WALK.hullTop * nz
		expanded.append( nx )
		expanded.append( ny )
		expanded.append( nz )
		expanded.append( planes[k + 3] + r * ( fabs( nx ) + fabs( ny ) ) - ( low < high ? low : high ) )
	}
	local solid = { lo = Vector( brush[0], brush[1], brush[2] ), hi = Vector( brush[3], brush[4], brush[5] ),
	                raw = planes, exp = expanded, mover = mover, ignored = false }
	local index = ::WALK.solids.len()
	::WALK.solids.append( solid )
	local size = ::WALK.clipBucket
	local origin = mover < 0 ? Vector( 0, 0, 0 ) : ::WALK.movers[mover].spawn
	local x0 = floor( ( brush[0] + origin.x - r - margin ) / size ).tointeger()
	local x1 = floor( ( brush[3] + origin.x + r + margin ) / size ).tointeger()
	local y0 = floor( ( brush[1] + origin.y - r - margin ) / size ).tointeger()
	local y1 = floor( ( brush[4] + origin.y + r + margin ) / size ).tointeger()
	for ( local bx = x0; bx <= x1; bx++ )
	{
		for ( local by = y0; by <= y1; by++ )
		{
			local key = Walk_BucketKey( bx, by )
			if ( !( key in ::WALK.clipGrid ) )
				::WALK.clipGrid[key] <- []
			::WALK.clipGrid[key].append( index )
		}
	}
}

function Walk_LoadClips()
{
	if ( !( "WALK_CLIPS" in getroottable() ) || !( "WALK_MOVERS" in getroottable() ) )
		throw "no collision data (qa/qa_clips.nut is installed by portal2_storybeats.py)"
	::WALK.clipGrid = {}
	::WALK.solids = []
	::WALK.movers = []
	foreach ( brush in ::WALK_CLIPS )
		Walk_AddSolid( brush, -1, 0.0 )
	local found = 0
	foreach ( m in ::WALK_MOVERS )
	{
		local ent = Entities.FindByClassnameNearest( m[1], m[2], 128.0 )
		::WALK.movers.append( { name = m[0], ent = ent, spawn = m[2], offset = null } )
		if ( ent != null )
			found++
		// Doors slide: bucket a mover's brushes with room to move sideways.
		foreach ( brush in m[3] )
			Walk_AddSolid( brush, ::WALK.movers.len() - 1, 256.0 )
	}
	Walk_UpdateMovers()
	QA_Log( "walker: " + ::WALK_CLIPS.len() + " player clips, " + found + "/" + ::WALK_MOVERS.len() +
	        " movers, " + ::WALK.solids.len() + " brushes" )
}

// Where each mover is now: its offset from where the map placed it, or null
// once it is gone.
function Walk_UpdateMovers()
{
	foreach ( m in ::WALK.movers )
	{
		if ( m.ent == null || !m.ent.IsValid() )
		{
			m.offset = null
			continue
		}
		m.offset = m.ent.GetOrigin()
	}
}

// Indices of the brushes whose buckets the box (x0, y0)-(x1, y1) touches.
function Walk_Candidates( x0, y0, x1, y1 )
{
	local size = ::WALK.clipBucket
	local result = []
	local seen = {}
	for ( local bx = floor( x0 / size ).tointeger(); bx <= floor( x1 / size ).tointeger(); bx++ )
	{
		for ( local by = floor( y0 / size ).tointeger(); by <= floor( y1 / size ).tointeger(); by++ )
		{
			local key = Walk_BucketKey( bx, by )
			if ( !( key in ::WALK.clipGrid ) )
				continue
			foreach ( i in ::WALK.clipGrid[key] )
			{
				if ( i in seen )
					continue
				seen[i] <- true
				result.append( i )
			}
		}
	}
	return result
}

// Where a brush's coordinates are placed: the world's clips at the origin, a
// mover's at its entity's origin now; null once the entity is gone.
function Walk_Offset( solid )
{
	if ( solid.ignored )
		return null
	if ( solid.mover < 0 )
		return Vector( 0, 0, 0 )
	return ::WALK.movers[solid.mover].offset
}

// A brush the player's body is inside is not where the map data puts it (a
// lift carried the player there, or the entity changed): the player is not
// colliding with it, so a plan starting there ignores it.
function Walk_IgnoreEnclosing( feet )
{
	foreach ( solid in ::WALK.solids )
		solid.ignored = false
	local body = feet + Vector( 0, 0, 36.0 )
	foreach ( i in Walk_Candidates( body.x, body.y, body.x, body.y ) )
	{
		local solid = ::WALK.solids[i]
		local off = Walk_Offset( solid )
		if ( off == null )
			continue
		local hit = Walk_SegmentBrush( solid.raw, body - off, body - off )
		if ( hit != null && hit[1] )
		{
			solid.ignored = true
			QA_Log( "walk " + ::WALK.label + " ignores a brush of " +
			        ( solid.mover < 0 ? "the world's clips" : "mover " + ::WALK.movers[solid.mover].name ) +
			        " enclosing the player at " + QA_Vec( feet ) )
		}
	}
}

// Segment a->b against a convex brush's planes: null when it misses, else
// [ t enter, start inside ].
function Walk_SegmentBrush( planes, a, b )
{
	local t0 = 0.0, t1 = 1.0, inside = true
	for ( local k = 0; k < planes.len(); k += 4 )
	{
		local da = planes[k] * a.x + planes[k + 1] * a.y + planes[k + 2] * a.z - planes[k + 3]
		local db = planes[k] * b.x + planes[k + 1] * b.y + planes[k + 2] * b.z - planes[k + 3]
		if ( da > 0.0 && db > 0.0 )
			return null
		if ( da > 0.0 )
		{
			inside = false
			local t = da / ( da - db )
			if ( t > t0 )
				t0 = t
		}
		else if ( db > 0.0 )
		{
			local t = da / ( da - db )
			if ( t < t1 )
				t1 = t
		}
		if ( t0 > t1 )
			return null
	}
	return [ t0, inside ]
}

// The player's hull moving from feet position a to b touches a brush.
function Walk_ClipBlocks( a, b )
{
	local r = ::WALK.hullRadius
	local x0 = a.x < b.x ? a.x : b.x, x1 = a.x < b.x ? b.x : a.x
	local y0 = a.y < b.y ? a.y : b.y, y1 = a.y < b.y ? b.y : a.y
	local z0 = a.z < b.z ? a.z : b.z, z1 = a.z < b.z ? b.z : a.z
	foreach ( i in Walk_Candidates( x0, y0, x1, y1 ) )
	{
		local solid = ::WALK.solids[i]
		local off = Walk_Offset( solid )
		if ( off == null )
			continue
		local lo = solid.lo + off, hi = solid.hi + off
		if ( x1 < lo.x - r || x0 > hi.x + r || y1 < lo.y - r || y0 > hi.y + r ||
		     z1 < lo.z - ::WALK.hullTop || z0 > hi.z - 19.0 )
			continue
		if ( Walk_SegmentBrush( solid.exp, a - off, b - off ) != null )
			return true
	}
	return false
}

// The highest brush surface below <top> at (x, y) down to <bottom>: a
// number, null for none, or "inside" when <top> is inside a brush.
function Walk_BrushFloor( x, y, top, bottom )
{
	local best = null
	foreach ( i in Walk_Candidates( x, y, x, y ) )
	{
		local solid = ::WALK.solids[i]
		local off = Walk_Offset( solid )
		if ( off == null )
			continue
		local lo = solid.lo + off, hi = solid.hi + off
		if ( x < lo.x || x > hi.x || y < lo.y || y > hi.y || bottom > hi.z || top < lo.z )
			continue
		local hit = Walk_SegmentBrush( solid.raw, Vector( x, y, top ) - off, Vector( x, y, bottom ) - off )
		if ( hit == null )
			continue
		if ( hit[1] )
			return "inside"
		local z = top - hit[0] * ( top - bottom )
		if ( best == null || z > best )
			best = z
	}
	return best
}

// The player fits standing at (x, y) on floor z.
function Walk_Fits( x, y, z )
{
	if ( Walk_ClipBlocks( Vector( x, y, z ), Vector( x, y, z + 0.5 ) ) )
		return false
	if ( Walk_Trace( Vector( x, y, z + 2.0 ), Vector( x, y, z + ::WALK.head ) ) < 1.0 )
		return false
	local mid = Vector( x, y, z + ::WALK.body )
	local r = ::WALK.radius
	if ( Walk_Trace( mid, Vector( x + r, y, mid.z ) ) < 1.0 )
		return false
	if ( Walk_Trace( mid, Vector( x - r, y, mid.z ) ) < 1.0 )
		return false
	if ( Walk_Trace( mid, Vector( x, y + r, mid.z ) ) < 1.0 )
		return false
	if ( Walk_Trace( mid, Vector( x, y - r, mid.z ) ) < 1.0 )
		return false
	return true
}

function Walk_Key3( ix, iy, z )
{
	return ix + "," + iy + "," + floor( z / 8.0 + 0.5 ).tointeger()
}

function Walk_CellCenter( i )
{
	return ( i + 0.5 ) * ::WALK.cell
}

// The node at grid cell (ix, iy) whose floor is found looking down from
// <top>; null when the player cannot stand there.
function Walk_Node( ix, iy, top )
{
	local x = Walk_CellCenter( ix ), y = Walk_CellCenter( iy )
	local z = Walk_FloorBelow( x, y, top )
	if ( z == null )
		return null
	local key = Walk_Key3( ix, iy, z )
	if ( key in ::WALK.nodes )
		return ::WALK.nodes[key]
	local node = { key = key, ix = ix, iy = iy, x = x, y = y, z = z, ok = Walk_Fits( x, y, z ), wall = 0 }
	if ( node.ok )
		node.wall = Walk_WallCount( x, y, z )
	::WALK.nodes[key] <- node
	return node
}

// How many of the four directions have a wall within two cells: routes pay
// for running close to walls, so they keep to the middle of a corridor.
function Walk_WallCount( x, y, z )
{
	local mid = Vector( x, y, z + ::WALK.body ), d = 2.0 * ::WALK.cell + ::WALK.radius
	local count = 0
	foreach ( v in [ Vector( d, 0, 0 ), Vector( -d, 0, 0 ), Vector( 0, d, 0 ), Vector( 0, -d, 0 ) ] )
	{
		if ( Walk_Trace( mid, mid + v ) < 1.0 || Walk_ClipBlocks( Vector( x, y, z ), Vector( x, y, z ) + v ) )
			count++
	}
	return count
}

// Nothing blocks a player walking in a straight line from a to b (feet
// positions): two heights, the higher floor of the two.
function Walk_Clear( a, b )
{
	local z = a.z > b.z ? a.z : b.z
	// A route may start where the hull already touches a clip (the player
	// stands there, beside a thin clip rail): only the other end counts.
	local from = Vector( a.x, a.y, z )
	if ( ( "start" in a ) && Walk_ClipBlocks( from, from ) )
		from = Vector( b.x, b.y, z )
	if ( Walk_ClipBlocks( from, Vector( b.x, b.y, z ) ) )
		return false
	if ( Walk_Trace( Vector( a.x, a.y, z + ::WALK.body ), Vector( b.x, b.y, z + ::WALK.body ) ) < 1.0 )
		return false
	return Walk_Trace( Vector( a.x, a.y, z + ::WALK.brow ), Vector( b.x, b.y, z + ::WALK.brow ) ) >= 1.0
}

// Wide line of sight for smoothing: the centre line and both shoulders.
function Walk_ClearWide( a, b )
{
	if ( !Walk_Clear( a, b ) )
		return false
	local dx = b.x - a.x, dy = b.y - a.y
	local length = sqrt( dx * dx + dy * dy )
	if ( length < 1.0 )
		return true
	local s = ( ::WALK.radius - 2.0 ) / length
	local side = Vector( -dy * s, dx * s, 0.0 )
	return Walk_Clear( a + side, b + side ) && Walk_Clear( a - side, b - side )
}

// Cost of moving from node a to its neighbour b, or null if the move is not
// possible. Drops are one-way.
function Walk_EdgeCost( a, b )
{
	local key = a.key + ">" + b.key
	if ( ( key in ::WALK.blocked ) || ( b.key in ::WALK.blockedNodes ) )
		return null
	if ( key in ::WALK.edges )
		return ::WALK.edges[key]
	local cost = null
	local rise = b.z - a.z
	if ( b.ok && rise <= ::WALK.step )
	{
		local run = sqrt( ( b.x - a.x ) * ( b.x - a.x ) + ( b.y - a.y ) * ( b.y - a.y ) )
		if ( rise >= -::WALK.step )
		{
			if ( Walk_Clear( a, b ) )
				cost = run + fabs( rise ) + 6.0 * b.wall
		}
		else
		{
			// Walk off the edge at a's height: the column above b's floor up
			// to a's head height must be open too.
			local over = Vector( b.x, b.y, a.z )
			if ( Walk_Clear( a, over ) &&
			     Walk_Trace( Vector( b.x, b.y, a.z + ::WALK.step + 2.0 ),
			                 Vector( b.x, b.y, a.z + ::WALK.head ) ) >= 1.0 )
				cost = run - rise * 0.25 + ::WALK.dropPenalty
		}
	}
	::WALK.edges[key] <- cost
	return cost
}

// --- A* ----------------------------------------------------------------------

function Walk_Heap()
{
	return []
}

function Walk_HeapPush( heap, f, key )
{
	heap.append( [ f, key ] )
	local i = heap.len() - 1
	while ( i > 0 )
	{
		local up = ( ( i - 1 ) / 2 ).tointeger()
		if ( heap[up][0] <= heap[i][0] )
			break
		local t = heap[up]
		heap[up] = heap[i]
		heap[i] = t
		i = up
	}
}

function Walk_HeapPop( heap )
{
	local top = heap[0]
	local last = heap.pop()
	if ( heap.len() > 0 )
	{
		heap[0] = last
		local i = 0
		local n = heap.len()
		while ( true )
		{
			local l = 2 * i + 1, r = 2 * i + 2, m = i
			if ( l < n && heap[l][0] < heap[m][0] )
				m = l
			if ( r < n && heap[r][0] < heap[m][0] )
				m = r
			if ( m == i )
				break
			local t = heap[m]
			heap[m] = heap[i]
			heap[i] = t
			i = m
		}
	}
	return top
}

function Walk_H( node )
{
	local g = ::WALK.goal
	local dx = node.x - g.x, dy = node.y - g.y, dz = node.z - ::WALK.goalFloor
	return 1.15 * sqrt( dx * dx + dy * dy + dz * dz )
}

function Walk_AtGoal( node )
{
	local g = ::WALK.goal
	local dx = node.x - g.x, dy = node.y - g.y
	return dx * dx + dy * dy <= ::WALK.goalRadius * ::WALK.goalRadius &&
	       fabs( node.z - ::WALK.goalFloor ) <= 24.0
}

// Diagnosis for a failed plan: why each neighbour of <node> is not
// reachable (floor, fit, clip, line, edge).
function Walk_Explain( node )
{
	for ( local dx = -1; dx <= 1; dx++ )
	{
		for ( local dy = -1; dy <= 1; dy++ )
		{
			if ( dx == 0 && dy == 0 )
				continue
			local x = Walk_CellCenter( node.ix + dx ), y = Walk_CellCenter( node.iy + dy )
			local z = Walk_FloorBelow( x, y, node.z + ::WALK.step + 2.0 )
			local why = ""
			if ( z == null )
				why = "no floor"
			else
			{
				local head = Walk_Trace( Vector( x, y, z + 2.0 ), Vector( x, y, z + ::WALK.head ) )
				local clip = Walk_ClipBlocks( Vector( x, y, z ), Vector( x, y, z + 0.5 ) )
				local mid = Vector( x, y, z + ::WALK.body ), r = ::WALK.radius
				why = format( "floor %.1f head %.2f", z, head ) + ( clip ? " clip" : "" ) +
				      format( " sides %.2f %.2f %.2f %.2f",
				              Walk_Trace( mid, Vector( x + r, y, mid.z ) ), Walk_Trace( mid, Vector( x - r, y, mid.z ) ),
				              Walk_Trace( mid, Vector( x, y + r, mid.z ) ), Walk_Trace( mid, Vector( x, y - r, mid.z ) ) ) +
				      ( Walk_Clear( node, Vector( x, y, z ) ) ? " line ok" : " line blocked" )
			}
			QA_Log( "explain " + ( node.ix + dx ) + "," + ( node.iy + dy ) + " " + why )
		}
	}
}

// Diagnosis: the floor heights on a grid, one line per row.
function Walk_Survey( x0, y0, x1, y1, step, top )
{
	for ( local y = y1; y >= y0; y -= step )
	{
		local line = ""
		for ( local x = x0; x <= x1; x += step )
		{
			local z = Walk_FloorBelow( x, y, top )
			line += z == null ? " ----" : format( " %4d", z.tointeger() )
		}
		QA_Log( format( "survey y=%.0f x=%.0f..%.0f:", y, x0, x1 ) + line )
	}
}

function Walk_StartPlan()
{
	local player = QA_Player()
	local origin = player.GetOrigin()
	Walk_IgnoreEnclosing( origin )
	local ix = floor( origin.x / ::WALK.cell ).tointeger()
	local iy = floor( origin.y / ::WALK.cell ).tointeger()
	// The route starts where the player stands, not at the cell's centre.
	local z = Walk_FloorBelow( origin.x, origin.y, origin.z + ::WALK.step + 2.0 )
	if ( z == null )
		z = origin.z
	// A new key per plan: edges cached from an earlier start do not apply.
	::WALK.starts <- ( "starts" in ::WALK ) ? ::WALK.starts + 1 : 1
	local start = { key = "start" + ::WALK.starts, ix = ix, iy = iy, x = origin.x, y = origin.y, z = z, ok = true,
	                start = true }
	::WALK.nodes[start.key] <- start
	local plan = { open = Walk_Heap(), g = {}, from = {}, closed = {}, expanded = 0,
	               traces = ::WALK.traces, started = Time(), best = start, bestH = Walk_H( start ) }
	plan.g[start.key] <- 0.0
	Walk_HeapPush( plan.open, Walk_H( start ), start.key )
	::WALK.plan = plan
	::WALK.state = "planning"
}

function Walk_PlanStep()
{
	local plan = ::WALK.plan
	local budget = ::WALK.expandPerTick
	local traceLimit = ::WALK.traces + ::WALK.tracesPerTick
	while ( budget-- > 0 && ::WALK.traces < traceLimit )
	{
		if ( plan.open.len() == 0 || plan.expanded >= ::WALK.maxExpand )
		{
			QA_Log( "walk " + ::WALK.label + " no route: expanded=" + plan.expanded + " closest " +
			        QA_Vec( Vector( plan.best.x, plan.best.y, plan.best.z ) ) +
			        format( " h=%.0f", plan.bestH ) )
			if ( !( ::WALK.label in ::WALK.explained ) )
			{
				::WALK.explained[::WALK.label] <- true
				Walk_Explain( plan.best )
				Walk_Survey( plan.best.x - 96.0, plan.best.y - 128.0, plan.best.x + 96.0, plan.best.y + 32.0,
				             8.0, plan.best.z + ::WALK.step + 2.0 )
			}
			::WALK.plan = null
			return false
		}
		local item = Walk_HeapPop( plan.open )
		local key = item[1]
		if ( key in plan.closed )
			continue
		plan.closed[key] <- true
		plan.expanded++
		local node = ::WALK.nodes[key]
		if ( Walk_AtGoal( node ) )
		{
			Walk_UsePath( plan, node )
			return true
		}
		local h = Walk_H( node )
		if ( h < plan.bestH )
		{
			plan.bestH = h
			plan.best = node
		}
		local g = plan.g[key]
		for ( local dx = -1; dx <= 1; dx++ )
		{
			for ( local dy = -1; dy <= 1; dy++ )
			{
				if ( dx == 0 && dy == 0 )
					continue
				local next = Walk_Node( node.ix + dx, node.iy + dy, node.z + ::WALK.step + 2.0 )
				if ( next == null || ( next.key in plan.closed ) )
					continue
				// A diagonal move must not cut a corner.
				if ( dx != 0 && dy != 0 )
				{
					local a = Walk_Node( node.ix + dx, node.iy, node.z + ::WALK.step + 2.0 )
					local b = Walk_Node( node.ix, node.iy + dy, node.z + ::WALK.step + 2.0 )
					if ( a == null || b == null || !a.ok || !b.ok )
						continue
				}
				local cost = Walk_EdgeCost( node, next )
				if ( cost == null )
					continue
				local total = g + cost
				if ( ( next.key in plan.g ) && plan.g[next.key] <= total )
					continue
				plan.g[next.key] <- total
				plan.from[next.key] <- key
				Walk_HeapPush( plan.open, total + Walk_H( next ), next.key )
			}
		}
	}
	return null
}

function Walk_UsePath( plan, last )
{
	local keys = [ last.key ]
	while ( keys[keys.len() - 1] in plan.from )
		keys.append( plan.from[keys[keys.len() - 1]] )
	local path = []
	for ( local i = keys.len() - 1; i >= 0; i-- )
	{
		local n = ::WALK.nodes[keys[i]]
		path.append( { x = n.x, y = n.y, z = n.z, key = n.key } )
	}
	::WALK.path = path
	::WALK.index = path.len() > 1 ? 1 : 0
	::WALK.plan = null
	::WALK.state = "walking"
	::WALK.bestProgress = -1.0e9
	::WALK.bestTime = Time()
	::WALK.jumped = false
	local drops = 0
	for ( local i = 1; i < path.len(); i++ )
		if ( path[i].z < path[i - 1].z - ::WALK.step )
			drops++
	QA_Log( "walk " + ::WALK.label + " route nodes=" + path.len() + " drops=" + drops + " expanded=" +
	        plan.expanded + " traces=" + ( ::WALK.traces - plan.traces ) +
	        format( " seconds=%.2f", Time() - plan.started ) )
}

// --- Following the route -----------------------------------------------------

function Walk_Dist2( a, b )
{
	local dx = a.x - b.x, dy = a.y - b.y
	return sqrt( dx * dx + dy * dy )
}

function Walk_Replan( reason )
{
	::WALK.replans++
	QA_Log( "walk " + ::WALK.label + " replan " + ::WALK.replans + ": " + reason )
	Walk_Key( "forward", false )
	if ( ::WALK.replans > ::WALK.maxReplans )
	{
		::WALK.state = "failed"
		Walk_ReleaseAll()
		return
	}
	Walk_StartPlan()
}

function Walk_Follow()
{
	local player = QA_Player()
	local pos = player.GetOrigin()
	local path = ::WALK.path
	local last = path[path.len() - 1]

	if ( Walk_Dist2( pos, last ) <= 20.0 && fabs( pos.z - last.z ) <= 32.0 )
	{
		Walk_Key( "forward", false )
		Walk_Key( "jump", false )
		::WALK.state = "arrived"
		QA_Log( "walk " + ::WALK.label + " arrived at " + QA_Vec( pos ) )
		return
	}

	// Track the route node nearest the player (a smoothed line passes
	// beside the nodes, not through them), then aim past it.
	local n = path.len()
	local nearest = ::WALK.index, nearestD = Walk_Dist2( pos, path[::WALK.index] )
	for ( local k = ::WALK.index + 1; k < n && k <= ::WALK.index + 10; k++ )
	{
		local d = Walk_Dist2( pos, path[k] )
		if ( d < nearestD && fabs( pos.z - path[k].z ) <= 48.0 )
		{
			nearest = k
			nearestD = d
		}
	}
	::WALK.index = nearest
	if ( nearestD <= 24.0 && nearest + 1 < n )
		::WALK.index = nearest + 1
	local current = path[::WALK.index]
	// Off the route: fell, was pushed, or a door moved.
	if ( Walk_Dist2( pos, current ) > 160.0 || pos.z < current.z - 80.0 && player.GetVelocity().z > -50.0 )
	{
		Walk_Replan( "off route at " + QA_Vec( pos ) + " expected near " + QA_Vec( Vector( current.x, current.y, current.z ) ) )
		return
	}

	// Aim at the farthest node in a clear line, stopping at a drop's lip.
	local target = ::WALK.index
	local feet = Vector( pos.x, pos.y, pos.z )
	for ( local k = ::WALK.index; k < n && k <= ::WALK.index + 10; k++ )
	{
		local p = path[k]
		if ( k > ::WALK.index && p.z < path[k - 1].z - ::WALK.step )
		{
			target = k
			break
		}
		if ( fabs( p.z - pos.z ) > 48.0 || !Walk_ClearWide( feet, Vector( p.x, p.y, p.z ) ) )
			break
		target = k
	}
	local aim = path[target]
	local yaw = QA_Deg( atan2( aim.y - pos.y, aim.x - pos.x ) )
	local error = Walk_AngleDelta( yaw, ::WALK.view.yaw )
	Walk_SteerYaw( yaw, 1.0 )
	Walk_SteerPitch( ::WALK.pitch, 2.0 )
	Walk_Key( "forward", fabs( error ) < 35.0 )

	// Stuck: no progress along the route for a while. Jump once; then treat
	// the node ahead as impassable and plan again.
	local now = Time()
	local progress = ::WALK.index * 1000.0 - Walk_Dist2( pos, current )
	if ( progress > ::WALK.bestProgress + 8.0 )
	{
		::WALK.bestProgress = progress
		::WALK.bestTime = now
	}
	else if ( now - ::WALK.bestTime > 1.5 )
	{
		QA_Log( "walk " + ::WALK.label + " stuck at " + QA_Vec( pos ) + " before " +
		        QA_Vec( Vector( current.x, current.y, current.z ) ) )
		::WALK.bestTime = now
		if ( !::WALK.jumped )
		{
			::WALK.jumped = true
			Walk_Key( "jump", true )
			::WALK.jumpUntil = now + 0.2
		}
		else
		{
			// Back off the obstacle, then plan around the node ahead.
			::WALK.blockedNodes[current.key] <- true
			::WALK.jumped = false
			Walk_Key( "forward", false )
			Walk_Key( "back", true )
			::WALK.backUntil <- now + 0.35
			::WALK.state = "backing"
			QA_Log( "walk " + ::WALK.label + " backs off; blocked node " + current.key )
			return
		}
	}
	if ( ::WALK.jumpUntil > 0.0 && now >= ::WALK.jumpUntil )
	{
		Walk_Key( "jump", false )
		::WALK.jumpUntil = 0.0
	}
}

function Walk_Look()
{
	local player = QA_Player()
	local angles = QA_AnglesTo( player.EyePosition(), ::WALK.look )
	local yawOk = Walk_SteerYaw( angles.yaw, 0.5 )
	local pitchOk = Walk_SteerPitch( angles.pitch, 0.5 )
	if ( yawOk && pitchOk )
	{
		::WALK.lookSteady++
		if ( ::WALK.lookSteady >= 6 )
			::WALK.state = "looked"
	}
	else
	{
		::WALK.lookSteady = 0
	}
}

function Walk_Tick()
{
	EntFireByHandle( ::WALK.driver, "RunScriptCode", "Walk_Tick()", 0.001, null, null )
	local player = GetPlayer()
	if ( player == null )
		return
	local now = Time()
	Walk_Integrate( now )
	Walk_UpdateMovers()
	if ( now >= ::WALK.nextPath )
	{
		// Position, the view the walker keeps, the body yaw the server has.
		::WALK.nextPath = now + 0.25
		local o = player.GetOrigin()
		printl( "QA_PATH " + ::QA.scenario + " t=" + format( "%.2f", now ) + " " +
		        format( "%.1f %.1f %.1f %.1f %.1f", o.x, o.y, o.z, ::WALK.view.yaw, ::WALK.view.pitch ) +
		        " " + ::WALK.state + format( " body=%.1f", player.GetAngles().y ) )
	}
	if ( ::WALK.state == "planning" )
	{
		local result = Walk_PlanStep()
		if ( result == false )
		{
			// No route yet: a door may still be closed. Try again shortly.
			::WALK.state = "noroute"
			::WALK.retryAt <- now + 1.0
		}
	}
	else if ( ::WALK.state == "noroute" )
	{
		if ( now >= ::WALK.retryAt )
		{
			// Doors change what the map allows: forget cached edges, and the
			// nodes marked impassable, which may have been wrong.
			::WALK.edges = {}
			::WALK.nodes = {}
			::WALK.blockedNodes = {}
			Walk_StartPlan()
		}
	}
	else if ( ::WALK.state == "walking" )
	{
		Walk_Follow()
	}
	else if ( ::WALK.state == "backing" )
	{
		if ( now >= ::WALK.backUntil )
		{
			Walk_Key( "back", false )
			Walk_Replan( "after backing off" )
		}
	}
	else if ( ::WALK.state == "looking" )
	{
		Walk_Look()
	}
}

// --- Scenario interface ------------------------------------------------------

function Walk_Init( ignoreName )
{
	::WALK.driver = Entities.FindByClassname( null, "worldspawn" )
	::WALK.ignore = ignoreName == null ? null : Entities.FindByName( null, ignoreName )
	Walk_LoadClips()
	// Keyboard look (+lookup/+lookdown) works only with mouse look off.
	SendToConsole( "cl_mouselook 0; lookspring 0; cl_yawspeed 210; cl_pitchspeed 225" )
	::WALK.yawSpeed = 210.0
	::WALK.pitchSpeed = 225.0
	// Nothing has turned the view yet: it is the spawn view, level, facing
	// the way the body faces.
	::WALK.view.pitch = 0.0
	::WALK.view.yaw = QA_Player().GetAngles().y
	if ( !::WALK.started )
	{
		::WALK.started = true
		Walk_Tick()
	}
}

// Starts walking to <goal> (a point near the floor; the floor under it is
// the target) within <radius>. Poll Walk_Arrived().
function Walk_To( label, goal, radius )
{
	Walk_ReleaseAll()
	::WALK.label = label
	::WALK.goal = goal
	::WALK.goalRadius = radius
	local z = Walk_FloorBelow( goal.x, goal.y, goal.z )
	if ( z == null )
		throw "no floor below goal " + label + " " + QA_Vec( goal )
	::WALK.goalFloor = z
	::WALK.replans = 0
	::WALK.path = []
	::WALK.blockedNodes = {}
	// Doors may have moved since the last walk.
	::WALK.edges = {}
	::WALK.nodes = {}
	QA_Log( "walk " + label + " to " + QA_Vec( goal ) + format( " floor=%.1f radius=%.0f", z, radius ) )
	Walk_StartPlan()
}

function Walk_Arrived()
{
	if ( ::WALK.state == "failed" )
		throw "walk " + ::WALK.label + " failed"
	QA_Detail( "state=" + ::WALK.state + " at " + QA_Vec( QA_Player().GetOrigin() ) + " node " +
	           ::WALK.index + "/" + ::WALK.path.len() + " replans=" + ::WALK.replans )
	return ::WALK.state == "arrived"
}

// Turns the view (input only) to look at <point>; poll Walk_Looked().
function Walk_LookAt( point )
{
	Walk_Key( "forward", false )
	::WALK.look = point
	::WALK.lookSteady = 0
	::WALK.state = "looking"
}

function Walk_LookDir( pitch, yaw )
{
	local eye = QA_Player().EyePosition()
	local basis = QA_Basis( pitch, yaw )
	Walk_LookAt( eye + QA_Scale( basis.forward, 1000.0 ) )
}

function Walk_Looked()
{
	QA_Detail( "state=" + ::WALK.state + format( " view %.1f %.1f", ::WALK.view.pitch, ::WALK.view.yaw ) )
	return ::WALK.state == "looked"
}

function Walk_Stop()
{
	::WALK.state = "idle"
	Walk_ReleaseAll()
}
