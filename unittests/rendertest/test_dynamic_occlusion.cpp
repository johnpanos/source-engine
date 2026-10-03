//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.dynamic-occlusion.v1 - light blocked by moving objects
//          (render/dynamic_occlusion.h, RFC 0011): the segment/box test
//          against a brute-force oracle (points marched along the segment),
//          containment, a receiver's own boxes, area-light penumbrae,
//          distant lights, and how shadows from several lights mix. The
//          sensitivity builds substitute defective rules the oracle must
//          reject.
//
//===========================================================================//

#include "render/dynamic_occlusion.h"
#include "testing/conformance_result.h"

#include <cmath>
#include <cstdio>
#include <vector>

namespace
{
// A small deterministic generator (the suite's inputs are fixed).
struct Rng
{
	unsigned long long state;
	explicit Rng( unsigned long long seed ) : state( seed * 6364136223846793005ull + 1442695040888963407ull ) {}
	float Uniform( float lo, float hi )
	{
		state = state * 6364136223846793005ull + 1442695040888963407ull;
		return lo + ( hi - lo ) * float( ( state >> 40 ) & 0xffffff ) / float( 0x1000000 );
	}
};

using namespace dynamic_occlusion;

unsigned long g_checks = 0;
unsigned long g_failures = 0;
#if defined( DYNAMIC_OCCLUSION_SEEDED_AABB ) || defined( DYNAMIC_OCCLUSION_SEEDED_NO_SELF ) ||     \
    defined( DYNAMIC_OCCLUSION_SEEDED_HARD ) || defined( DYNAMIC_OCCLUSION_SEEDED_SHARED )
constexpr bool kSeeded = true;
#else
constexpr bool kSeeded = false;
#endif
unsigned long g_rejected = 0;

void Check( bool condition, const char *description )
{
	++g_checks;
	if ( !condition )
	{
		if ( kSeeded )
		{
			++g_rejected;
			std::fprintf( stderr, "seeded defect detected: %s\n", description );
			return;
		}
		++g_failures;
		std::fprintf( stderr, "FAIL: %s\n", description );
	}
}

// The rules under test: the real ones, or seeded defective ones.
bool HitsUnderTest( const Box &box, const float a[3], const float b[3] )
{
#if defined( DYNAMIC_OCCLUSION_SEEDED_AABB )
	// The box as the axis-aligned box of its extents (rotation ignored).
	Box aligned = box;
	for ( int a2 = 0; a2 < 3; ++a2 )
	{
		const float len =
		    std::sqrt( box.axes[a2][0] * box.axes[a2][0] + box.axes[a2][1] * box.axes[a2][1] +
		               box.axes[a2][2] * box.axes[a2][2] );
		for ( int k = 0; k < 3; ++k )
			aligned.axes[a2][k] = k == a2 ? len : 0.0f;
	}
	return SegmentHits( aligned, a, b );
#else
	return SegmentHits( box, a, b );
#endif
}

float VisibilityUnderTest(
    const float receiver[3], const Samples &samples, const std::vector<Box> &boxes, int self )
{
#if defined( DYNAMIC_OCCLUSION_SEEDED_NO_SELF )
	(void)self;
	return Visibility( receiver, samples, boxes, 0 );
#elif defined( DYNAMIC_OCCLUSION_SEEDED_HARD )
	// One blocked sample blocks the whole light: no penumbra.
	return Visibility( receiver, samples, boxes, self ) < 1.0f ? 0.0f : 1.0f;
#else
	return Visibility( receiver, samples, boxes, self );
#endif
}

void MixUnderTest( const float receiver[3], const float ( *contributions )[3],
    const Samples *samples, int count, const std::vector<Box> &boxes, int self, float out[3] )
{
#if defined( DYNAMIC_OCCLUSION_SEEDED_SHARED )
	// One visibility (the darkest light's) applied to every light.
	float least = 1.0f;
	for ( int i = 0; i < count; ++i )
		least = std::min( least, Visibility( receiver, samples[i], boxes, self ) );
	out[0] = out[1] = out[2] = 0.0f;
	for ( int i = 0; i < count; ++i )
		for ( int k = 0; k < 3; ++k )
			out[k] += contributions[i][k] * least;
#else
	MixLights( receiver, contributions, samples, count, boxes, self, out );
#endif
}

// The oracle: march the segment in fine steps and ask whether any point lies
// inside the box (containment by projection onto its axes).
bool MarchedHits( const Box &box, const float a[3], const float b[3], int steps )
{
	for ( int i = 0; i <= steps; ++i )
	{
		const float t = float( i ) / float( steps );
		const float p[3] = {
		    a[0] + t * ( b[0] - a[0] ), a[1] + t * ( b[1] - a[1] ), a[2] + t * ( b[2] - a[2] ) };
		if ( Contains( box, p ) )
			return true;
	}
	return false;
}

// A box rotated by yaw about z and pitch about y.
Box MakeBox( float cx, float cy, float cz, float hx, float hy, float hz, float yaw, float pitch,
    int entity = 0 )
{
	const float cyw = std::cos( yaw ), syw = std::sin( yaw ), cp = std::cos( pitch ),
	            sp = std::sin( pitch );
	// Columns of R = Rz(yaw) Ry(pitch).
	const float r[3][3] = {
	    { cyw * cp, syw * cp, -sp }, { -syw, cyw, 0.0f }, { cyw * sp, syw * sp, cp } };
	const float h[3] = { hx, hy, hz };
	Box box;
	box.center[0] = cx;
	box.center[1] = cy;
	box.center[2] = cz;
	for ( int a = 0; a < 3; ++a )
		for ( int k = 0; k < 3; ++k )
			box.axes[a][k] = r[a][k] * h[a];
	box.entity = entity;
	return box;
}

} // namespace

int main()
{
	// Core triangle publication owns its payload and rolls back invalid frames.
	{
		float vertices[] = { 0, 0, 10, 8, 0, 10, 0, 8, 10 };
		TriangleFrame frame;
		TriangleInput input{ 101, 2, vertices, 3 };
		Check( frame.Publish( &input, 1 ), "physical frame accepts a triangle" );
		const auto first = frame.Entries()[0].version;
		const auto held = frame.Entries();
		vertices[0] = 1;
		Check( frame.Entries()[0].positions[0] == 0, "physical frame owns borrowed input" );
		Check( frame.Publish( &input, 1 ) && frame.Entries()[0].version > first,
		    "physical pose movement advances its revision" );
		const auto moved = frame.Entries()[0].version;
		Check( frame.Publish( &input, 1 ) && frame.Entries()[0].version == moved,
		    "unchanged physical pose keeps its revision" );
		Check( held[0].positions[0] == 0 && held[0].version == first,
		    "an earlier render snapshot keeps its geometry" );
		TriangleInput duplicate[] = { input, input };
		vertices[0] = 2;
		Check( !frame.Publish( duplicate, 2 ) && frame.Entries()[0].version == moved &&
		           frame.Entries()[0].positions[0] == 1,
		    "duplicate keys reject the whole frame" );
		Check( frame.Publish( &input, 1 ) && frame.Entries()[0].version == moved + 1,
		    "rejected publication also rolls back the revision counter" );
		const auto valid = frame.Entries()[0].version;
		vertices[0] = std::numeric_limits<float>::quiet_NaN();
		Check( !frame.Publish( &input, 1 ) && frame.Entries()[0].version == valid,
		    "nonfinite triangle positions preserve the preceding frame" );
		vertices[0] = 2;
		input.vertexCount = 4;
		Check( !frame.Publish( &input, 1 ), "incomplete triangle lists are refused" );
		input.vertexCount = 65538;
		Check( !frame.Publish( &input, 1 ), "oversize meshes are refused before reading input" );
		Check( !frame.Publish( nullptr, 1 ) && !frame.Publish( &input, kMaxOccluders + 1 ),
		    "null and oversize frames are refused" );
		Check( frame.Publish( nullptr, 0 ) && frame.Entries().empty(),
		    "empty publication retires vanished physical parts" );
		input.vertexCount = 3;
		Check( frame.Publish( &input, 1 ) && frame.Entries()[0].version > valid,
		    "map reset cannot reuse a stale geometry revision" );
	}

	// The segment/box test agrees with marching over random rotated boxes
	// and segments, apart from segments that only graze a box (within the
	// march's step, which the march can miss).
	{
		Rng rng( 1234 );
		auto pos = []( Rng &r ) { return r.Uniform( -60.0f, 60.0f ); }; auto ext = []( Rng &r ) { return r.Uniform( 2.0f, 30.0f ); }; auto ang = []( Rng &r ) { return r.Uniform( 0.0f, 6.2831853f ); };
		int agree = 0, total = 0, hits = 0;
		for ( int i = 0; i < 4000; ++i )
		{
			const Box box = MakeBox( pos( rng ) * 0.3f, pos( rng ) * 0.3f, pos( rng ) * 0.3f,
			    ext( rng ), ext( rng ), ext( rng ), ang( rng ), ang( rng ) );
			const float a[3] = { pos( rng ), pos( rng ), pos( rng ) };
			const float b[3] = { pos( rng ), pos( rng ), pos( rng ) };
			const bool marched = MarchedHits( box, a, b, 4000 );
			// Ambiguous: the march disagrees with itself when the box shrinks
			// or grows by a little (a grazing segment).
			Box smaller = box, larger = box;
			for ( int ax = 0; ax < 3; ++ax )
				for ( int k = 0; k < 3; ++k )
				{
					smaller.axes[ax][k] *= 0.98f;
					larger.axes[ax][k] *= 1.02f;
				}
			if ( MarchedHits( smaller, a, b, 4000 ) != MarchedHits( larger, a, b, 4000 ) )
				continue;
			++total;
			hits += marched;
			agree += HitsUnderTest( box, a, b ) == marched;
		}
		std::fprintf( stderr, "segment/box: %d of %d agree (%d hits)\n", agree, total, hits );
		Check( total > 3000 && hits > 200 && agree == total,
		    "segment/box matches marching on rotated boxes" );
	}

	// Containment and the segment's ends: a segment ending inside the box
	// hits it; one ending short of it does not.
	{
		const Box box = MakeBox( 0, 0, 0, 10, 10, 10, 0.3f, 0.2f );
		const float inside[3] = { 1, 1, 1 }, far[3] = { 100, 0, 0 }, near[3] = { 30, 0, 0 };
		Check( Contains( box, inside ) && !Contains( box, far ), "containment" );
		Check( HitsUnderTest( box, far, inside ), "a segment ending inside the box hits it" );
		Check( !HitsUnderTest( box, far, near ), "a segment stopping short of the box misses it" );
	}

	// A cube on the floor between a floor point and a light: the point is in
	// its shadow; beside it the point is lit.
	{
		std::vector<Box> boxes = { MakeBox( 0, 0, 16, 16, 16, 16, 0.4f, 0.0f, 7 ) };
		const float light[3] = { 100, 0, 60 };
		const Samples samples = PointSamples( light );
		const float shadowed[3] = { -40, 0, 0 };
		const float beside[3] = { -40, 80, 0 };
		Check( VisibilityUnderTest( shadowed, samples, boxes, 0 ) == 0.0f,
		    "the cube shadows the floor behind it" );
		Check( VisibilityUnderTest( beside, samples, boxes, 0 ) == 1.0f,
		    "the floor beside its shadow is lit" );
		const float under[3] = { 0, 0, 0 };
		Check( VisibilityUnderTest( under, samples, boxes, 0 ) == 0.0f,
		    "the floor under the cube is shadowed" );
	}

	// A receiver inside its own boxes ignores them; inside another entity's
	// box it is blocked.
	{
		std::vector<Box> boxes = { MakeBox( 0, 0, 0, 10, 10, 10, 0.0f, 0.0f, 3 ) };
		const float light[3] = { 0, 0, 300 };
		const float center[3] = { 0, 0, 0 };
		Check( VisibilityUnderTest( center, PointSamples( light ), boxes, 3 ) == 1.0f,
		    "a model is not shadowed by its own boxes" );
		Check( VisibilityUnderTest( center, PointSamples( light ), boxes, 4 ) == 0.0f,
		    "another entity's box blocks a receiver inside it" );
	}

	// An area light half hidden by a box: the receiver is in penumbra, lit by
	// the unhidden half of the samples.
	{
		area_light::Rect rect;
		rect.center[2] = 200.0f;
		rect.halfU[0] = 40.0f;
		rect.halfV[1] = -40.0f; // faces down
		// A thin wide slab under the light's -x half.
		std::vector<Box> boxes = { MakeBox( -20, 0, 150, 20, 60, 1, 0.0f, 0.0f ) };
		const float receiver[3] = { 0, 0, 0 };
		const float v = VisibilityUnderTest( receiver, RectSamples( rect ), boxes, 0 );
		Check( std::fabs( v - 0.5f ) < 1e-6f,
		    "an area light half hidden gives half its light (penumbra)" );
	}

	// A distant light: its sample stands far along its direction.
	{
		const float receiver[3] = { 10, 20, 30 };
		const float down[3] = { 0, 0, -2 };
		const Samples s = DistantSamples( receiver, down );
		Check( s.count == 1 && s.point[0][2] > receiver[2] + 1000.0f && s.point[0][0] == 10.0f,
		    "a distant light's sample stands up its direction" );
		std::vector<Box> roof = { MakeBox( 10, 20, 500, 50, 50, 5, 0.0f, 0.0f ) };
		Check(
		    VisibilityUnderTest( receiver, s, roof, 0 ) == 0.0f, "a roof shadows a distant light" );
	}

	// Mixing: two lights, a box blocking only one. The receiver keeps the
	// other light in full, and loses exactly the blocked one's share.
	{
		std::vector<Box> boxes = { MakeBox( 50, 0, 50, 10, 10, 10, 0.0f, 0.0f ) };
		const float receiver[3] = { 0, 0, 0 };
		const float lightA[3] = { 100, 0, 100 };  // behind the box
		const float lightB[3] = { -100, 0, 100 }; // clear
		const Samples samples[2] = { PointSamples( lightA ), PointSamples( lightB ) };
		const float contributions[2][3] = { { 0.3f, 0.3f, 0.3f }, { 0.5f, 0.2f, 0.1f } };
		float out[3];
		MixUnderTest( receiver, contributions, samples, 2, boxes, 0, out );
		Check( std::fabs( out[0] - 0.5f ) < 1e-6f && std::fabs( out[1] - 0.2f ) < 1e-6f &&
		           std::fabs( out[2] - 0.1f ) < 1e-6f,
		    "shadows from several lights mix: each light loses only its own share" );
	}

	if ( kSeeded )
	{
		std::fprintf( stderr, "%lu seeded rejection(s)\n", g_rejected );
		if ( g_rejected == 0 )
		{
			std::fprintf( stderr, "FAIL: the seeded defect was not detected\n" );
			return testing::ReportConformance( g_checks, g_checks );
		}
		return testing::ReportConformance( g_checks, 0 );
	}
	return testing::ReportConformance( g_checks, g_failures );
}
