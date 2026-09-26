//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Negative (sensitivity) suite for adapters.source.vector_interop.v1.
//			Proves the shared adapter checks actually DETECT a broken boundary
//			policy, rather than passing trivially. A deliberately-wrong adapter
//			swaps the y and z components on the way into the strict library; the
//			substitution and round-trip clauses must fail against it.
//
//			Convention (matches the other *_negative suites): this binary exits 0
//			when it CORRECTLY catches the seeded violation, and 1 if the broken
//			adapter slipped through undetected (which would mean the oracle is not
//			trustworthy).
//
//=============================================================================//

#include "adapter_checks.h"

#include <cstdio>

// Deliberately-broken seam: swaps y/z going into the library. Everything else is
// forwarded to the real owner so the failure is isolated to the injected bug.
struct BuggyAdapter
{
	static mapgeometry::Vec3 ToVec3( const Vector &v )
	{
		return mapgeometry::Vec3( v.x, v.z, v.y ); // BUG: y/z swapped
	}
	static Vector ToVector( const mapgeometry::Vec3 &v ) { return Vector( v.x, v.y, v.z ); }
	static mapgeometry::AxisAlignedBox ToAABB( const BoundBox &b )
	{
		return mapgeometry::AxisAlignedBox( ToVec3( b.bmins ), ToVec3( b.bmaxs ) );
	}
	static void FromAABB( const mapgeometry::AxisAlignedBox &a, BoundBox &b )
	{
		b.bmins = ToVector( a.mins );
		b.bmaxs = ToVector( a.maxs );
	}
};

int main()
{
	const int failures = hammer_adapter_checks::RunAdapterChecks<BuggyAdapter>();
	if ( failures > 0 )
	{
		std::printf( "adapters.source.vector_interop negative: oracle caught the "
					 "broken seam (%d clause failures, as required)\n",
					 failures );
		return 0;
	}
	std::printf( "adapters.source.vector_interop negative: ORACLE UNSOUND -- broken "
				 "seam passed undetected\n" );
	return 1;
}
