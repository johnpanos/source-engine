//========= F-Stop port =======================================================//
//
// Purpose: Pair oracle for the clean-room blobulator physics tiler
//          (blobulator.phys-tiler; public/blobulator/physics/PhysTiler.h).
//
// The F-Stop blob NPCs keep their particles in VPhysics and use the tiler only
// to find, each step, every pair of particles closer than the interaction
// radius. This suite checks that search against an independent brute-force
// oracle over all pairs, computed with the same float expression:
//
//   pairs      each particle's list holds exactly the other particles with
//              |b - a|^2 < r^2, with that squared distance bitwise, no self,
//              no duplicates, and a NULL terminator; lists are symmetric, so
//              the NPCs' "handle a pair when a < b" loop visits each pair once
//   geometry   random clouds at several radii and densities, grid-aligned
//              points on cell boundaries, pairs at exactly r (excluded) and
//              just inside it, negative coordinates, coincident particles, a
//              tiny clamped radius, far coordinates past the packed cell
//              range, and a NaN centre (no pairs, no crash)
//   frames     any grid origin and any insertion order give the same pairs,
//              equal inputs give identical lists, a new frame forgets the old
//              one, and the factory hands out distinct tilers and reuses them
//
// The negative controls apply the same checker to corrupted copies of a
// correct result (a dropped pair, an extra far particle, a duplicate, a self
// entry, a wrong distance, a broken symmetry) to show each defect is caught.
//
// Building the tiler with -DBLOBULATOR_PHYS_SEED_DEFECT makes it skip the
// cells on one side; the manifest's sensitivity row requires that to fail.
//
//=============================================================================//

#include "blobulator/physics/PhysTiler.h"

#include "testing/conformance_result.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace
{

unsigned long g_nChecks = 0;
unsigned long g_nFailures = 0;

void Check( bool bCondition, const std::string &name, const std::string &detail = std::string() )
{
	++g_nChecks;
	if ( !bCondition )
	{
		++g_nFailures;
		std::printf( "FAIL %s%s%s\n", name.c_str(), detail.empty() ? "" : ": ", detail.c_str() );
	}
}

struct Point
{
	float x, y, z;
};

// Deterministic generator (no <random> distribution differences across libs).
struct Lcg
{
	uint64_t state;
	explicit Lcg( uint64_t seed ) : state( seed * 6364136223846793005ull + 1442695040888963407ull ) {}
	uint32_t Next()
	{
		state = state * 6364136223846793005ull + 1442695040888963407ull;
		return uint32_t( state >> 33 );
	}
	float Uniform( float lo, float hi ) { return lo + ( hi - lo ) * float( Next() ) / float( 0x7fffffffu ); }
};

// A neighbour list as (particle index, squared distance), in list order.
typedef std::vector<std::pair<int, float>> List;
typedef std::vector<List> Lists;

// The oracle: every ordered pair, the same float expression as the contract.
Lists BruteForce( const std::vector<Point> &points, float flRadius )
{
	float r = flRadius > 1e-3f ? flRadius : 1e-3f;
	float flRadiusSq = r * r;
	Lists lists( points.size() );
	for ( size_t i = 0; i < points.size(); ++i )
	{
		for ( size_t j = 0; j < points.size(); ++j )
		{
			if ( i == j )
				continue;
			float fx = points[j].x - points[i].x;
			float fy = points[j].y - points[i].y;
			float fz = points[j].z - points[i].z;
			float flDistSq = fx * fx + fy * fy + fz * fz;
			if ( flDistSq < flRadiusSq )
				lists[i].push_back( std::make_pair( int( j ), flDistSq ) );
		}
	}
	return lists;
}

struct TilerRun
{
	Lists lists;			// by particle index, in the tiler's list order
	bool bTerminated = true;
	bool bKnownPointers = true;
};

// Runs one frame and copies the lists out by particle index.
TilerRun RunTiler( PhysTiler &tiler, const std::vector<Point> &points, float flRadius,
	const Point3D &origin, const std::vector<int> &order )
{
	std::vector<PhysParticle> particles( points.size() );
	for ( size_t i = 0; i < points.size(); ++i )
		particles[i].center = Point3D( points[i].x, points[i].y, points[i].z );

	tiler.setInteractionRadius( flRadius );
	tiler.beginFrame( origin );
	for ( int i : order )
		tiler.insertParticle( &particles[i] );
	tiler.processTiles();

	TilerRun run;
	run.lists.resize( points.size() );
	PhysParticleCache *pCache = tiler.getParticleCache();
	for ( size_t i = 0; i < points.size(); ++i )
	{
		PhysParticleAndDist *pNode = pCache->get( &particles[i] );
		int nGuard = 0;
		for ( ; pNode->particle != NULL; ++pNode )
		{
			if ( ++nGuard > int( points.size() ) + 1 )
			{
				run.bTerminated = false;
				break;
			}
			ptrdiff_t nIndex = pNode->particle - particles.data();
			if ( nIndex < 0 || nIndex >= ptrdiff_t( points.size() ) )
			{
				run.bKnownPointers = false;
				continue;
			}
			run.lists[i].push_back( std::make_pair( int( nIndex ), pNode->distSq ) );
		}
	}
	tiler.endFrame();
	return run;
}

std::vector<int> Identity( size_t n )
{
	std::vector<int> order( n );
	for ( size_t i = 0; i < n; ++i )
		order[i] = int( i );
	return order;
}

// Compares lists as sets against the oracle; returns a description or "".
std::string Compare( const Lists &actual, const Lists &expected )
{
	if ( actual.size() != expected.size() )
		return "particle count";
	for ( size_t i = 0; i < actual.size(); ++i )
	{
		std::map<int, float> want;
		for ( const auto &entry : expected[i] )
			want[entry.first] = entry.second;
		std::map<int, int> seen;
		for ( const auto &entry : actual[i] )
		{
			char buf[160];
			if ( entry.first == int( i ) )
			{
				std::snprintf( buf, sizeof( buf ), "particle %zu lists itself", i );
				return buf;
			}
			if ( ++seen[entry.first] > 1 )
			{
				std::snprintf( buf, sizeof( buf ), "particle %zu lists %d twice", i, entry.first );
				return buf;
			}
			auto it = want.find( entry.first );
			if ( it == want.end() )
			{
				std::snprintf( buf, sizeof( buf ), "particle %zu lists non-neighbour %d", i, entry.first );
				return buf;
			}
			if ( std::memcmp( &it->second, &entry.second, sizeof( float ) ) != 0 )
			{
				std::snprintf( buf, sizeof( buf ), "particle %zu -> %d distSq %.9g, want %.9g", i,
					entry.first, entry.second, it->second );
				return buf;
			}
		}
		if ( seen.size() != want.size() )
		{
			for ( const auto &entry : want )
			{
				if ( !seen.count( entry.first ) )
				{
					char buf[160];
					std::snprintf( buf, sizeof( buf ), "particle %zu misses neighbour %d (distSq %.9g)", i,
						entry.first, entry.second );
					return buf;
				}
			}
		}
	}
	return "";
}

std::string Symmetry( const Lists &lists )
{
	for ( size_t i = 0; i < lists.size(); ++i )
	{
		for ( const auto &entry : lists[i] )
		{
			if ( entry.first < 0 || entry.first >= int( lists.size() ) )
				return "index out of range";
			const List &back = lists[entry.first];
			bool bFound = std::any_of( back.begin(), back.end(),
				[&]( const std::pair<int, float> &b ) { return b.first == int( i ); } );
			if ( !bFound )
			{
				char buf[96];
				std::snprintf( buf, sizeof( buf ), "%zu lists %d but not the reverse", i, entry.first );
				return buf;
			}
		}
	}
	return "";
}

// The NPCs' force loop visits a pair when the first particle's address is
// lower; with symmetric lists that is each unordered pair exactly once.
size_t PairsVisitedOnce( const Lists &lists )
{
	size_t nVisits = 0;
	for ( size_t i = 0; i < lists.size(); ++i )
		for ( const auto &entry : lists[i] )
			if ( int( i ) < entry.first )
				++nVisits;
	return nVisits;
}

size_t UnorderedPairs( const Lists &oracle )
{
	size_t n = 0;
	for ( const auto &list : oracle )
		n += list.size();
	return n / 2;
}

void CheckCase( PhysTiler &tiler, const std::string &name, const std::vector<Point> &points, float flRadius,
	const Point3D &origin = Point3D( 0, 0, 0 ) )
{
	TilerRun run = RunTiler( tiler, points, flRadius, origin, Identity( points.size() ) );
	Lists oracle = BruteForce( points, flRadius );
	Check( run.bTerminated, name + " lists-terminated" );
	Check( run.bKnownPointers, name + " lists-point-at-inserted-particles" );
	std::string diff = Compare( run.lists, oracle );
	Check( diff.empty(), name + " matches-brute-force", diff );
	std::string sym = Symmetry( run.lists );
	Check( sym.empty(), name + " symmetric", sym );
	Check( PairsVisitedOnce( run.lists ) == UnorderedPairs( oracle ), name + " each-pair-visited-once" );
}

std::vector<Point> Cloud( uint64_t seed, int n, float flExtent, float flOffset = 0.0f )
{
	Lcg rng( seed );
	std::vector<Point> points( n );
	for ( Point &p : points )
		p = { flOffset + rng.Uniform( -flExtent, flExtent ), flOffset + rng.Uniform( -flExtent, flExtent ),
			flOffset + rng.Uniform( -flExtent, flExtent ) };
	return points;
}

void TestRandomClouds()
{
	PhysTiler tiler;
	const float radii[] = { 0.5f, 1.0f, 2.3f, 7.0f };
	const int counts[] = { 0, 1, 2, 17, 200, 600 };
	const float extents[] = { 1.5f, 6.0f, 40.0f };
	int nCase = 0;
	for ( float r : radii )
		for ( int n : counts )
			for ( float e : extents )
			{
				char name[96];
				std::snprintf( name, sizeof( name ), "cloud.r%g.n%d.e%g", r, n, e );
				CheckCase( tiler, name, Cloud( 1000 + nCase++, n, e * r ), r );
			}
	// Negative coordinates and an offset grid origin.
	CheckCase( tiler, "cloud.negative", Cloud( 77, 300, 20.0f, -55.0f ), 2.0f, Point3D( 3.25f, -1.5f, 9.0f ) );
}

void TestBoundaries()
{
	PhysTiler tiler;
	// Integer lattice points: with r = 1 many sit on cell faces and many pairs
	// are exactly r apart, which the strict test excludes.
	std::vector<Point> lattice;
	for ( int x = -3; x <= 3; ++x )
		for ( int y = -3; y <= 3; ++y )
			for ( int z = -2; z <= 2; ++z )
				lattice.push_back( { float( x ), float( y ), float( z ) } );
	CheckCase( tiler, "lattice.r1", lattice, 1.0f );
	CheckCase( tiler, "lattice.r1.0001", lattice, 1.0001f );
	CheckCase( tiler, "lattice.r1.5", lattice, 1.5f );
	CheckCase( tiler, "lattice.r0.5-half-offset", lattice, 1.0f, Point3D( 0.5f, 0.5f, 0.5f ) );

	// A pair exactly r apart is not a pair; the next float down is.
	float r = 2.0f;
	float flInside = std::nextafter( r, 0.0f );
	CheckCase( tiler, "exact-r", { { 0, 0, 0 }, { r, 0, 0 } }, r );
	CheckCase( tiler, "just-inside-r", { { 0, 0, 0 }, { flInside, 0, 0 } }, r );
	{
		TilerRun run = RunTiler( tiler, { { 0, 0, 0 }, { r, 0, 0 } }, r, Point3D( 0, 0, 0 ), Identity( 2 ) );
		Check( run.lists[0].empty(), "exact-r excluded" );
		run = RunTiler( tiler, { { 0, 0, 0 }, { flInside, 0, 0 } }, r, Point3D( 0, 0, 0 ), Identity( 2 ) );
		Check( run.lists[0].size() == 1, "just-inside-r included" );
	}

	// Pairs straddling every face, edge and corner of the cell at the origin.
	std::vector<Point> straddle;
	const float d = 0.3f;
	for ( int x = -1; x <= 1; x += 2 )
		for ( int y = -1; y <= 1; y += 2 )
			for ( int z = -1; z <= 1; z += 2 )
				straddle.push_back( { x * d, y * d, z * d } );
	CheckCase( tiler, "straddle-corner", straddle, 1.0f );

	// Coincident particles are neighbours at distance 0.
	CheckCase( tiler, "coincident", { { 4, 4, 4 }, { 4, 4, 4 }, { 4, 4, 4 } }, 1.0f );

	// A radius below the minimum is clamped, not treated as zero.
	CheckCase( tiler, "tiny-radius", { { 0, 0, 0 }, { 0.0005f, 0, 0 }, { 0.01f, 0, 0 } }, 0.0f );
	tiler.setInteractionRadius( -5.0f );
	Check( tiler.getInteractionRadius() > 0.0f, "negative-radius clamped positive" );

	// Coordinates beyond the packed cell range still pair correctly.
	std::vector<Point> far = { { 1e9f, 0, 0 }, { 1e9f, 0.5f, 0 }, { -1e9f, 0, 0 }, { -1e9f, 0, 0.25f },
		{ 3e7f, 3e7f, 3e7f }, { 0, 0, 0 } };
	CheckCase( tiler, "far-coordinates", far, 1.0f );

	// A NaN centre has no pairs and does not disturb the others.
	float nan = std::numeric_limits<float>::quiet_NaN();
	CheckCase( tiler, "nan-centre", { { 0, 0, 0 }, { nan, 0, 0 }, { 0.5f, 0, 0 } }, 1.0f );
}

void TestFrames()
{
	PhysTiler tiler;
	std::vector<Point> points = Cloud( 4242, 250, 8.0f );
	Lists oracle = BruteForce( points, 1.3f );

	// Any origin gives the same pairs.
	const Point3D origins[] = { Point3D( 0, 0, 0 ), Point3D( 0.37f, -12.0f, 1e3f ), Point3D( -1e4f, 5.5f, 0.1f ) };
	int nOrigin = 0;
	for ( const Point3D &origin : origins )
	{
		TilerRun run = RunTiler( tiler, points, 1.3f, origin, Identity( points.size() ) );
		Check( Compare( run.lists, oracle ).empty(), "origin-invariance." + std::to_string( nOrigin++ ) );
	}

	// Any insertion order gives the same pairs.
	std::vector<int> order = Identity( points.size() );
	std::reverse( order.begin(), order.end() );
	TilerRun reversed = RunTiler( tiler, points, 1.3f, Point3D( 0, 0, 0 ), order );
	Check( Compare( reversed.lists, oracle ).empty(), "insertion-order-invariance.reversed" );
	Lcg rng( 9 );
	for ( size_t i = order.size(); i > 1; --i )
		std::swap( order[i - 1], order[rng.Next() % i] );
	TilerRun shuffled = RunTiler( tiler, points, 1.3f, Point3D( 0, 0, 0 ), order );
	Check( Compare( shuffled.lists, oracle ).empty(), "insertion-order-invariance.shuffled" );

	// Equal inputs give identical lists, order and bits included.
	TilerRun a = RunTiler( tiler, points, 1.3f, Point3D( 0, 0, 0 ), Identity( points.size() ) );
	PhysTiler other;
	TilerRun b = RunTiler( other, points, 1.3f, Point3D( 0, 0, 0 ), Identity( points.size() ) );
	bool bSame = a.lists.size() == b.lists.size();
	for ( size_t i = 0; bSame && i < a.lists.size(); ++i )
	{
		bSame = a.lists[i].size() == b.lists[i].size();
		for ( size_t k = 0; bSame && k < a.lists[i].size(); ++k )
			bSame = a.lists[i][k].first == b.lists[i][k].first
				&& std::memcmp( &a.lists[i][k].second, &b.lists[i][k].second, sizeof( float ) ) == 0;
	}
	Check( bSame, "deterministic-lists" );

	// A new frame forgets the previous one's particles.
	std::vector<PhysParticle> first( 3 ), second( 2 );
	first[0].center = Point3D( 0, 0, 0 );
	first[1].center = Point3D( 0.5f, 0, 0 );
	first[2].center = Point3D( 0, 0.5f, 0 );
	second[0].center = Point3D( 0, 0, 0 );
	second[1].center = Point3D( 0.25f, 0, 0 );
	tiler.setInteractionRadius( 1.0f );
	tiler.beginFrame( Point3D( 0, 0, 0 ) );
	for ( PhysParticle &p : first )
		tiler.insertParticle( &p );
	tiler.processTiles();
	tiler.endFrame();
	tiler.beginFrame( Point3D( 0, 0, 0 ) );
	for ( PhysParticle &p : second )
		tiler.insertParticle( &p );
	tiler.processTiles();
	Check( tiler.getParticleCount() == 2, "new-frame particle-count" );
	PhysParticleAndDist *pNode = tiler.getParticleCache()->get( &second[0] );
	Check( pNode[0].particle == &second[1] && pNode[1].particle == NULL, "new-frame lists only its particles" );
	// first[2] held slot 2, which does not exist in this frame.
	Check( tiler.getParticleCache()->get( &first[2] )->particle == NULL, "stale-particle empty-list" );
	tiler.endFrame();
}

void TestFactory()
{
	PhysTiler *a = PhysTilerFactory::factory->getTiler();
	PhysTiler *b = PhysTilerFactory::factory->getTiler();
	Check( a && b && a != b, "factory distinct-tilers" );
	a->setInteractionRadius( 9.0f );
	PhysTilerFactory::factory->returnTiler( a );
	PhysTiler *c = PhysTilerFactory::factory->getTiler();
	Check( c == a, "factory reuses-returned-tiler" );
	Check( c->getInteractionRadius() == 1.0f, "factory resets-radius" );
	PhysTilerFactory::factory->returnTiler( b );
	PhysTilerFactory::factory->returnTiler( c );
	PhysTilerFactory::factory->returnTiler( NULL );
	Check( true, "factory return-null-ignored" );
}

// The checker must reject corrupted copies of a correct result.
void TestNegativeControls()
{
	PhysTiler tiler;
	std::vector<Point> points = Cloud( 31337, 120, 4.0f );
	Lists oracle = BruteForce( points, 1.1f );
	TilerRun run = RunTiler( tiler, points, 1.1f, Point3D( 0, 0, 0 ), Identity( points.size() ) );
	Check( Compare( run.lists, oracle ).empty(), "control.baseline-matches" );

	size_t nWithPairs = 0;
	while ( nWithPairs < run.lists.size() && run.lists[nWithPairs].empty() )
		++nWithPairs;
	Check( nWithPairs < run.lists.size(), "control.has-pairs" );
	if ( nWithPairs >= run.lists.size() )
		return;
	const size_t i = nWithPairs;

	Lists dropped = run.lists;
	dropped[i].pop_back();
	Check( !Compare( dropped, oracle ).empty(), "control.dropped-pair-detected" );

	Lists extra = run.lists;
	int nFar = -1;
	for ( int j = 0; j < int( points.size() ) && nFar < 0; ++j )
		if ( j != int( i ) && std::none_of( oracle[i].begin(), oracle[i].end(),
			[&]( const std::pair<int, float> &e ) { return e.first == j; } ) )
			nFar = j;
	extra[i].push_back( std::make_pair( nFar, 0.5f ) );
	Check( !Compare( extra, oracle ).empty(), "control.non-neighbour-detected" );

	Lists duplicate = run.lists;
	duplicate[i].push_back( duplicate[i].front() );
	Check( !Compare( duplicate, oracle ).empty(), "control.duplicate-detected" );

	Lists self = run.lists;
	self[i].push_back( std::make_pair( int( i ), 0.0f ) );
	Check( !Compare( self, oracle ).empty(), "control.self-detected" );

	Lists distance = run.lists;
	distance[i].front().second = std::nextafter( distance[i].front().second, 100.0f );
	Check( !Compare( distance, oracle ).empty(), "control.one-ulp-distance-detected" );

	Lists asymmetric = run.lists;
	int nOther = asymmetric[i].front().first;
	List &back = asymmetric[nOther];
	back.erase( std::remove_if( back.begin(), back.end(),
		[&]( const std::pair<int, float> &e ) { return e.first == int( i ); } ), back.end() );
	Check( !Symmetry( asymmetric ).empty(), "control.asymmetry-detected" );
	Check( PairsVisitedOnce( asymmetric ) != UnorderedPairs( oracle )
		|| PairsVisitedOnce( dropped ) != UnorderedPairs( oracle ), "control.pair-count-detects-loss" );
}

} // namespace

int main()
{
	TestRandomClouds();
	TestBoundaries();
	TestFrames();
	TestFactory();
	TestNegativeControls();
	return testing::ReportConformance( g_nChecks, g_nFailures );
}
