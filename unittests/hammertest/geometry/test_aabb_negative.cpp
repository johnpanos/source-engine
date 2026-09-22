//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Negative-provider check for the geometry.aabb.v1 contract (RFC 0002).
//			RFC 0002/0005 require a deliberately-broken provider to prove the
//			conformance suite actually detects violations and is not vacuous.
//
//			The shared conformance predicate below is run against the real
//			AxisAlignedBox (must pass) and against BuggyBox, which violates the
//			open-face intersection clause of the contract (must be detected).
//
//			Build/run: unittests/hammertest/run_headless.sh
//
//=============================================================================//

#include "hammer/geometry/aabb.h"

#include <cstdio>

using hammer::geometry::AxisAlignedBox;
using hammer::geometry::Vec3;

namespace
{

// A deliberately non-conforming provider: it uses CLOSED faces for intersection
// (> / <) where the contract requires OPEN faces (>= / <=). A box that only
// touches another at a face will wrongly report intersecting. Everything else
// delegates to the real implementation.
class BuggyBox
{
public:
	BuggyBox( const Vec3 &mins, const Vec3 &maxs ) : m_box( mins, maxs ) {}

	bool IsIntersectingBox( const Vec3 &mins, const Vec3 &maxs ) const
	{
		if ( ( m_box.mins[0] > maxs[0] ) || ( m_box.maxs[0] < mins[0] ) )
		{
			return false;
		}
		if ( ( m_box.mins[1] > maxs[1] ) || ( m_box.maxs[1] < mins[1] ) )
		{
			return false;
		}
		if ( ( m_box.mins[2] > maxs[2] ) || ( m_box.maxs[2] < mins[2] ) )
		{
			return false;
		}
		return true;
	}

private:
	AxisAlignedBox m_box;
};

// The open-face intersection clause of geometry.aabb.v1, expressed as a provider
// -parameterized predicate: a box [0,10]^3 must NOT intersect a box that only
// touches it at the x==10 face. Returns true when the provider conforms.
template <typename Box> bool ConformsOpenFaceIntersection( const Box &box )
{
	const bool touchesFace =
	    box.IsIntersectingBox( Vec3( 10.0f, 0.0f, 0.0f ), Vec3( 20.0f, 10.0f, 10.0f ) );
	const bool overlaps =
	    box.IsIntersectingBox( Vec3( 5.0f, 5.0f, 5.0f ), Vec3( 15.0f, 15.0f, 15.0f ) );
	// Conforming: touching faces do not intersect, genuine overlap does.
	return ( !touchesFace ) && overlaps;
}

} // namespace

int main()
{
	const AxisAlignedBox realBox( Vec3( 0.0f, 0.0f, 0.0f ), Vec3( 10.0f, 10.0f, 10.0f ) );
	const BuggyBox buggyBox( Vec3( 0.0f, 0.0f, 0.0f ), Vec3( 10.0f, 10.0f, 10.0f ) );

	const bool realConforms = ConformsOpenFaceIntersection( realBox );
	const bool buggyConforms = ConformsOpenFaceIntersection( buggyBox );

	int failures = 0;

	// The real provider MUST satisfy the contract clause.
	if ( !realConforms )
	{
		std::printf( "FAIL: real AxisAlignedBox violated the open-face intersection clause\n" );
		++failures;
	}

	// The oracle MUST detect the broken provider. If the buggy provider passed,
	// the suite is vacuous and cannot be trusted.
	if ( buggyConforms )
	{
		std::printf( "FAIL: conformance predicate did NOT detect the broken provider\n" );
		++failures;
	}

	if ( failures != 0 )
	{
		std::printf( "hammer.geometry AABB negative: %d check(s) FAILED\n", failures );
		return 1;
	}
	std::printf(
	    "hammer.geometry AABB negative: oracle detects violations (real passes, buggy caught)\n" );
	return 0;
}
