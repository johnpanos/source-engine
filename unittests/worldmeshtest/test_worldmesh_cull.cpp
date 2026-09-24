//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Brute-force oracle for conservative WMSH meshlet culling
//          (world.meshlet-cull, RFC 0008 F4).
//
// Every rejection the culler makes is checked against an exhaustive
// reference: a cone rejection must leave no triangle facing the eye, and an
// occlusion rejection must leave no sample point of the box that some ray
// reaches before a drawn occluder. Seeded random scenes also require the
// cullers to reject something, and deliberately unsound cullers must be
// caught by the same reference.
//
//=============================================================================//

#include "worldmesh_cull.h"

#include "testing/conformance_result.h"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <utility>
#include <vector>

using worldmesh_cull::ConeFacesAway;
using worldmesh_cull::OcclusionBuffer;

namespace
{

unsigned long g_checks = 0;
unsigned long g_failures = 0;

void Check( bool condition, const char *what, int line )
{
	++g_checks;
	if ( !condition )
	{
		++g_failures;
		std::printf( "FAIL line %d: %s\n", line, what );
	}
}
#define CHECK( cond ) Check( ( cond ), #cond, __LINE__ )

struct Vec
{
	double x, y, z;
};
Vec operator+( Vec a, Vec b ) { return { a.x + b.x, a.y + b.y, a.z + b.z }; }
Vec operator-( Vec a, Vec b ) { return { a.x - b.x, a.y - b.y, a.z - b.z }; }
Vec operator*( Vec a, double s ) { return { a.x * s, a.y * s, a.z * s }; }
double Dot( Vec a, Vec b ) { return a.x * b.x + a.y * b.y + a.z * b.z; }
Vec Cross( Vec a, Vec b )
{
	return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
}
Vec Unit( Vec a ) { return a * ( 1.0 / std::sqrt( Dot( a, a ) ) ); }
void Store( Vec a, float out[3] )
{
	out[0] = float( a.x );
	out[1] = float( a.y );
	out[2] = float( a.z );
}
Vec Load( const float in[3] ) { return { in[0], in[1], in[2] }; }

struct Random
{
	uint64_t state;
	double Next()
	{
		state ^= state << 13;
		state ^= state >> 7;
		state ^= state << 17;
		return double( state >> 11 ) / double( 1ull << 53 );
	}
	double Range( double low, double high ) { return low + ( high - low ) * Next(); }
	Vec Direction()
	{
		for ( ;; )
		{
			const Vec v = { Range( -1, 1 ), Range( -1, 1 ), Range( -1, 1 ) };
			const double length = Dot( v, v );
			if ( length > 1e-4 && length <= 1.0 )
				return Unit( v );
		}
	}
};

struct Triangle
{
	float a[3], b[3], c[3];
	bool twoSided;
};

// A meshlet: triangles near center whose front faces lie near axis, and the
// sphere and cone a packer would store for it.
struct Meshlet
{
	std::vector<Triangle> triangles;
	float center[3];
	float radius;
	float axis[3];
	float cutoff;
};

Meshlet MakeMeshlet( Random &random, double spread )
{
	Meshlet meshlet;
	const Vec center = { random.Range( -200, 200 ), random.Range( -200, 200 ),
		random.Range( -200, 200 ) };
	const Vec axis = random.Direction();
	const double size = random.Range( 1, 40 );
	Vec sum = { 0, 0, 0 };
	std::vector<Vec> normals;
	for ( int i = 0; i < 21; ++i )
	{
		const Vec normal = Unit( axis + random.Direction() * spread );
		const Vec tangent = Unit( Cross( normal, random.Direction() ) );
		const Vec bitangent = Cross( normal, tangent );
		const Vec origin = center + random.Direction() * ( size * random.Next() );
		const double edge = random.Range( 0.1, size );
		Triangle triangle;
		Store( origin, triangle.a );
		Store( origin + tangent * edge, triangle.b );
		Store( origin + bitangent * edge, triangle.c );
		triangle.twoSided = false;
		meshlet.triangles.push_back( triangle );
		// The stored float corners decide the front face, as in the packer.
		const Vec n = Cross( Load( triangle.b ) - Load( triangle.a ),
		    Load( triangle.c ) - Load( triangle.a ) );
		if ( Dot( n, n ) > 1e-24 )
		{
			normals.push_back( Unit( n ) );
			sum = sum + Unit( n );
		}
	}
	double radius = 0;
	for ( const Triangle &triangle : meshlet.triangles )
	{
		for ( const float *corner : { triangle.a, triangle.b, triangle.c } )
			radius = std::fmax( radius, std::sqrt( Dot( Load( corner ) - center, Load( corner ) - center ) ) );
	}
	Store( center, meshlet.center );
	meshlet.radius = float( radius + 1e-3 );
	const Vec coneAxis = Unit( sum );
	double cutoff = 1;
	for ( const Vec &normal : normals )
		cutoff = std::fmin( cutoff, Dot( coneAxis, normal ) );
	Store( coneAxis, meshlet.axis );
	meshlet.cutoff = float( cutoff - 1e-5 );
	return meshlet;
}

// Reference: some triangle faces the eye at some corner.
bool AnyTriangleFacesEye( const Meshlet &meshlet, Vec eye )
{
	for ( const Triangle &triangle : meshlet.triangles )
	{
		const Vec n = Cross( Load( triangle.b ) - Load( triangle.a ),
		    Load( triangle.c ) - Load( triangle.a ) );
		for ( const float *corner : { triangle.a, triangle.b, triangle.c } )
		{
			if ( Dot( n, eye - Load( corner ) ) >= 0.0 )
				return true;
		}
	}
	return false;
}

void TestCone()
{
	Random random = { 0x9E3779B97F4A7C15ull };
	int culled = 0;
	int unsound = 0;
	int badCulled = 0;
	int badCaught = 0;
	for ( int trial = 0; trial < 20000; ++trial )
	{
		const Meshlet meshlet = MakeMeshlet( random, random.Range( 0.0, 0.8 ) );
		const Vec eye = { random.Range( -400, 400 ), random.Range( -400, 400 ),
			random.Range( -400, 400 ) };
		float eyeF[3];
		Store( eye, eyeF );
		const bool faces = AnyTriangleFacesEye( meshlet, eye );
		if ( ConeFacesAway( eyeF, meshlet.center, meshlet.radius, meshlet.axis, meshlet.cutoff,
		         1e-3f ) )
		{
			++culled;
			unsound += faces;
		}
		// A culler that ignores the sphere's extent: the reference must catch it.
		if ( ConeFacesAway( eyeF, meshlet.center, 0.0f, meshlet.axis, meshlet.cutoff, 0.0f ) )
		{
			++badCulled;
			badCaught += faces;
		}
	}
	std::printf( "cone: %d of 20000 culled, %d unsound; radius-free culler %d culled, %d caught\n",
	    culled, unsound, badCulled, badCaught );
	CHECK( unsound == 0 );
	CHECK( culled > 2000 );
	CHECK( badCaught > 0 );

	// Fixed cases: a floor seen from above is kept, from below culled; a
	// hemisphere-wide cone never culls; an eye inside the sphere never culls.
	const float center[3] = { 0, 0, 0 };
	const float up[3] = { 0, 0, 1 };
	const float above[3] = { 0, 0, 100 };
	const float below[3] = { 0, 0, -100 };
	CHECK( !ConeFacesAway( above, center, 10, up, 0.99f, 1e-3f ) );
	CHECK( ConeFacesAway( below, center, 10, up, 0.99f, 1e-3f ) );
	CHECK( !ConeFacesAway( below, center, 10, up, 0.0f, 1e-3f ) );
	CHECK( !ConeFacesAway( below, center, 200, up, 0.99f, 1e-3f ) );
}

// A perspective view: w is depth along forward, |x|, |y| <= w on screen.
struct View
{
	Vec eye, forward, right, up;
	double focal;
	float matrix[16];
	float eyeF[3];
	float zNear = 4;
	float zFar = 4000;

	void Build()
	{
		const Vec rows[3] = { right * focal, up * focal, forward };
		const int index[3] = { 0, 1, 3 };
		for ( int i = 0; i < 16; ++i )
			matrix[i] = 0;
		for ( int r = 0; r < 3; ++r )
		{
			matrix[4 * index[r] + 0] = float( rows[r].x );
			matrix[4 * index[r] + 1] = float( rows[r].y );
			matrix[4 * index[r] + 2] = float( rows[r].z );
			matrix[4 * index[r] + 3] = float( -Dot( rows[r], eye ) );
		}
		matrix[10] = 1;
		Store( eye, eyeF );
	}
	bool OnScreen( Vec point ) const
	{
		const Vec d = point - eye;
		const double w = Dot( d, forward );
		return w >= zNear && w <= zFar && std::fabs( focal * Dot( d, right ) ) <= w &&
		       std::fabs( focal * Dot( d, up ) ) <= w;
	}
};

// Reference: the ray from the eye to point meets a drawn occluder first.
bool Hidden( const View &view, const std::vector<Triangle> &occluders, Vec point )
{
	const Vec direction = point - view.eye;
	for ( const Triangle &triangle : occluders )
	{
		const Vec a = Load( triangle.a ), b = Load( triangle.b ), c = Load( triangle.c );
		const Vec n = Cross( b - a, c - a );
		if ( !triangle.twoSided && Dot( n, view.eye - a ) <= 0.0 )
			continue;
		const double denominator = Dot( n, direction );
		if ( std::fabs( denominator ) < 1e-12 )
			continue;
		const double t = Dot( n, a - view.eye ) / denominator;
		if ( t <= 0.0 || t >= 1.0 )
			continue;
		const Vec hit = view.eye + direction * t;
		// Parts the GPU clips away do not occlude.
		if ( !view.OnScreen( hit ) )
			continue;
		if ( Dot( Cross( b - a, hit - a ), n ) >= 0 && Dot( Cross( c - b, hit - b ), n ) >= 0 &&
		     Dot( Cross( a - c, hit - c ), n ) >= 0 )
			return true;
	}
	return false;
}

// Reference: some on-screen sample of the box is not hidden.
bool BoxVisible( const View &view, const std::vector<Triangle> &occluders, const float mins[3],
    const float maxs[3] )
{
	const int steps = 6;
	for ( int i = 0; i <= steps; ++i )
	{
		for ( int j = 0; j <= steps; ++j )
		{
			for ( int k = 0; k <= steps; ++k )
			{
				const Vec point = { mins[0] + ( maxs[0] - mins[0] ) * i / steps,
					mins[1] + ( maxs[1] - mins[1] ) * j / steps,
					mins[2] + ( maxs[2] - mins[2] ) * k / steps };
				if ( view.OnScreen( point ) && !Hidden( view, occluders, point ) )
					return true;
			}
		}
	}
	return false;
}

void TestOcclusion()
{
	Random random = { 0xD1B54A32D192ED03ull };
	int tested = 0;
	int culled = 0;
	int unsound = 0;
	int badCulled = 0;
	int badCaught = 0;
	int backFaceIgnored = 0;
	OcclusionBuffer buffer;
	for ( int scene = 0; scene < 300; ++scene )
	{
		View view;
		view.eye = { random.Range( -50, 50 ), random.Range( -50, 50 ), random.Range( -50, 50 ) };
		view.forward = random.Direction();
		view.right = Unit( Cross( view.forward, random.Direction() ) );
		view.up = Cross( view.right, view.forward );
		view.focal = random.Range( 0.8, 2.0 );
		view.Build();
		// Large walls across the view at random depths and slants, some one-sided
		// walls turned away (they must not occlude), some crossing the near plane.
		std::vector<Triangle> occluders;
		const int walls = 1 + int( random.Next() * 4 );
		for ( int i = 0; i < walls; ++i )
		{
			const double depth = random.Range( 2, 300 );
			const Vec normal = Unit( view.forward * -1.0 + random.Direction() * 0.6 );
			const Vec tangent = Unit( Cross( normal, random.Direction() ) );
			const Vec bitangent = Cross( normal, tangent );
			const Vec middle = view.eye + view.forward * depth +
			                   ( view.right * random.Range( -1, 1 ) + view.up * random.Range( -1, 1 ) ) *
			                       depth * 0.5;
			const double size = depth * random.Range( 0.3, 2.0 );
			Triangle triangle;
			Store( middle - tangent * size - bitangent * size, triangle.a );
			Store( middle + tangent * size * 2.0 - bitangent * size, triangle.b );
			Store( middle - tangent * size + bitangent * size * 2.0, triangle.c );
			triangle.twoSided = random.Next() < 0.3;
			// Wound to face the eye unless this wall is deliberately turned away.
			const Vec n = Cross( Load( triangle.b ) - Load( triangle.a ),
			    Load( triangle.c ) - Load( triangle.a ) );
			const bool away = random.Next() < 0.25;
			if ( ( Dot( n, view.eye - Load( triangle.a ) ) > 0 ) == away )
				std::swap( triangle.b, triangle.c );
			occluders.push_back( triangle );
		}
		buffer.Begin( view.matrix, view.eyeF, view.zNear, view.zFar );
		for ( const Triangle &triangle : occluders )
			buffer.AddOccluder( triangle.a, triangle.b, triangle.c, triangle.twoSided );
		buffer.Finish();
		for ( int i = 0; i < 60; ++i )
		{
			const double depth = random.Range( 1, 600 );
			const Vec middle = view.eye + view.forward * depth +
			                   ( view.right * random.Range( -1.2, 1.2 ) +
			                       view.up * random.Range( -1.2, 1.2 ) ) *
			                       ( depth / view.focal );
			const Vec half = { random.Range( 0.1, 20 ), random.Range( 0.1, 20 ),
				random.Range( 0.1, 20 ) };
			float mins[3], maxs[3];
			Store( middle - half, mins );
			Store( middle + half, maxs );
			++tested;
			const bool visible = BoxVisible( view, occluders, mins, maxs );
			if ( buffer.IsBoxOccluded( mins, maxs ) )
			{
				++culled;
				unsound += visible;
			}
			// An unsound culler that tests the box center instead of its nearest point.
			if ( view.OnScreen( middle ) && Hidden( view, occluders, middle ) )
			{
				++badCulled;
				badCaught += visible;
			}
		}
		// Every occluder turned away and one-sided: nothing may be occluded.
		buffer.Begin( view.matrix, view.eyeF, view.zNear, view.zFar );
		for ( Triangle triangle : occluders )
		{
			const Vec n = Cross( Load( triangle.b ) - Load( triangle.a ),
			    Load( triangle.c ) - Load( triangle.a ) );
			if ( Dot( n, view.eye - Load( triangle.a ) ) > 0 )
				std::swap( triangle.b, triangle.c );
			buffer.AddOccluder( triangle.a, triangle.b, triangle.c, false );
		}
		buffer.Finish();
		backFaceIgnored += buffer.RasterizedOccluders() == 0;
	}
	std::printf( "occlusion: %d of %d boxes culled, %d unsound; center culler %d culled, %d caught; "
	             "%d of 300 back-facing scenes rasterized nothing\n",
	    culled, tested, unsound, badCulled, badCaught, backFaceIgnored );
	CHECK( unsound == 0 );
	CHECK( culled > tested / 20 );
	CHECK( badCaught > 0 );
	CHECK( backFaceIgnored == 300 );

	// Fixed case: a wall facing the eye hides a box behind it but not one in
	// front of it, and not a box that reaches the near plane.
	View view;
	view.eye = { 0, 0, 0 };
	view.forward = { 1, 0, 0 };
	view.right = { 0, -1, 0 };
	view.up = { 0, 0, 1 };
	view.focal = 1;
	view.Build();
	Triangle wall;
	Store( { 100, -1000, -1000 }, wall.a );
	Store( { 100, 0, 1000 }, wall.b );
	Store( { 100, 1000, -1000 }, wall.c );
	const Vec n = Cross( Load( wall.b ) - Load( wall.a ), Load( wall.c ) - Load( wall.a ) );
	if ( Dot( n, view.eye - Load( wall.a ) ) < 0 )
		std::swap( wall.b, wall.c );
	buffer.Begin( view.matrix, view.eyeF, view.zNear, view.zFar );
	buffer.AddOccluder( wall.a, wall.b, wall.c, false );
	buffer.Finish();
	const float behindMin[3] = { 150, -10, -10 }, behindMax[3] = { 170, 10, 10 };
	const float frontMin[3] = { 50, -10, -10 }, frontMax[3] = { 70, 10, 10 };
	const float nearMin[3] = { 1, -1, -1 }, nearMax[3] = { 200, 1, 1 };
	const float straddleMin[3] = { 90, -10, -10 }, straddleMax[3] = { 110, 10, 10 };
	CHECK( buffer.RasterizedOccluders() == 1 );
	CHECK( buffer.CoveredCells() > OcclusionBuffer::kWidth * OcclusionBuffer::kHeight / 2 );
	CHECK( buffer.IsBoxOccluded( behindMin, behindMax ) );
	CHECK( !buffer.IsBoxOccluded( frontMin, frontMax ) );
	CHECK( !buffer.IsBoxOccluded( nearMin, nearMax ) );
	CHECK( !buffer.IsBoxOccluded( straddleMin, straddleMax ) );
	// The same wall seen from behind (one-sided) occludes nothing.
	buffer.Begin( view.matrix, view.eyeF, view.zNear, view.zFar );
	buffer.AddOccluder( wall.a, wall.c, wall.b, false );
	buffer.Finish();
	CHECK( !buffer.IsBoxOccluded( behindMin, behindMax ) );
	buffer.Begin( view.matrix, view.eyeF, view.zNear, view.zFar );
	buffer.AddOccluder( wall.a, wall.c, wall.b, true );
	buffer.Finish();
	CHECK( buffer.IsBoxOccluded( behindMin, behindMax ) );
}

} // namespace

int main()
{
	TestCone();
	TestOcclusion();
	return testing::ReportConformance( g_checks, g_failures );
}
