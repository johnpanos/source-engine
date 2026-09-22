//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Shared conformance checks for adapters.source.vector_interop.v1,
//			templated on an adapter policy so the exact same clauses run against
//			the real seam (positive suite) and a deliberately-broken seam
//			(negative suite). Returns the number of failed clauses.
//
//			The central clause is the SUBSTITUTION guarantee: running a reusable
//			geometry operation directly on the legacy BoundBox must produce the
//			same bmins/bmaxs as marshalling the box into the strict library
//			(ToAABB), running the operation there, and committing back (FromAABB).
//			This is what lets a thin UI sibling delegate geometry to the reusable
//			library.
//
//=============================================================================//

#ifndef HAMMER_TEST_ADAPTERS_ADAPTER_CHECKS_H
#define HAMMER_TEST_ADAPTERS_ADAPTER_CHECKS_H

#include "adapter_policy.h"
#include "boundbox.h"
#include "hammer/geometry/aabb.h"

#include <cstdio>
#include <cmath>

namespace hammer_adapter_checks
{

inline bool Eq( float a, float b ) { return std::fabs( a - b ) <= 1e-4f; }

inline bool BoxEq( const BoundBox &a, const BoundBox &b )
{
	return Eq( a.bmins.x, b.bmins.x ) && Eq( a.bmins.y, b.bmins.y ) &&
		   Eq( a.bmins.z, b.bmins.z ) && Eq( a.bmaxs.x, b.bmaxs.x ) &&
		   Eq( a.bmaxs.y, b.bmaxs.y ) && Eq( a.bmaxs.z, b.bmaxs.z );
}

// Seed a legacy BoundBox from a point sequence.
inline void Seed( BoundBox &box, const Vector *pts, int n )
{
	box.ResetBounds();
	for ( int i = 0; i < n; ++i )
	{
		box.UpdateBounds( pts[i] );
	}
}

// One substitution clause: apply `legacyOp` directly, and separately marshal the
// same seeded box through the adapter, apply `aabbOp` on the strict library, and
// commit back. The two legacy boxes must match.
template <class Adapter, class LegacyOp, class AabbOp>
bool SubstitutionHolds( const Vector *pts, int n, LegacyOp legacyOp, AabbOp aabbOp )
{
	BoundBox direct;
	Seed( direct, pts, n );
	legacyOp( direct );

	BoundBox viaLibrary;
	Seed( viaLibrary, pts, n );
	hammer::geometry::AxisAlignedBox box = Adapter::ToAABB( viaLibrary );
	aabbOp( box );
	Adapter::FromAABB( box, viaLibrary );

	return BoxEq( direct, viaLibrary );
}

template <class Adapter>
int RunAdapterChecks()
{
	int failures = 0;
	auto check = [&]( bool ok, const char *label )
	{
		if ( !ok )
		{
			std::printf( "FAIL adapter: %s\n", label );
			++failures;
		}
	};

	// (1) Round-trip identity: ToVector(ToVec3(v)) reproduces v exactly.
	const Vector samples[] = { Vector( 0, 0, 0 ), Vector( 1, -2, 3 ),
							   Vector( -7.5f, 11.25f, 99999.0f ), Vector( 3, 3, 3 ) };
	for ( const Vector &v : samples )
	{
		Vector rt = Adapter::ToVector( Adapter::ToVec3( v ) );
		check( rt.x == v.x && rt.y == v.y && rt.z == v.z, "round-trip identity" );
	}

	// (2) ToAABB reads bmins/bmaxs componentwise.
	const Vector pts[] = { Vector( 3, 11, -7 ), Vector( 19, -4, 9 ), Vector( -2, 27, 4 ) };
	BoundBox seeded;
	Seed( seeded, pts, 3 );
	hammer::geometry::AxisAlignedBox a = Adapter::ToAABB( seeded );
	check( Eq( a.mins[0], seeded.bmins.x ) && Eq( a.mins[1], seeded.bmins.y ) &&
			   Eq( a.mins[2], seeded.bmins.z ) && Eq( a.maxs[0], seeded.bmaxs.x ) &&
			   Eq( a.maxs[1], seeded.bmaxs.y ) && Eq( a.maxs[2], seeded.bmaxs.z ),
		   "ToAABB reads extent" );

	// (3) FromAABB writes bmins/bmaxs componentwise.
	BoundBox target;
	target.ResetBounds();
	Adapter::FromAABB( a, target );
	check( BoxEq( target, seeded ), "FromAABB writes extent" );

	// (4) Substitution guarantee across the reusable geometry operations. The
	//     .5-boundary SnapToGrid(16) case pins V_rint round-half-away-from-zero.
	const Vector halfPts[] = { Vector( 8, -8, 0 ), Vector( 24, 8, 16 ) };
	check( SubstitutionHolds<Adapter>(
			   halfPts, 2, []( BoundBox &b ) { b.SnapToGrid( 16 ); },
			   []( hammer::geometry::AxisAlignedBox &b ) { b.SnapToGrid( 16 ); } ),
		   "substitution: SnapToGrid(16) .5-boundary" );
	check( SubstitutionHolds<Adapter>(
			   pts, 3, []( BoundBox &b ) { b.SnapToGrid( 8 ); },
			   []( hammer::geometry::AxisAlignedBox &b ) { b.SnapToGrid( 8 ); } ),
		   "substitution: SnapToGrid(8)" );
	for ( int axis = 0; axis < 3; ++axis )
	{
		check( SubstitutionHolds<Adapter>(
				   pts, 3, [axis]( BoundBox &b ) { b.Rotate90( axis ); },
				   [axis]( hammer::geometry::AxisAlignedBox &b ) { b.Rotate90( axis ); } ),
			   "substitution: Rotate90" );
	}

	return failures;
}

} // namespace hammer_adapter_checks

#endif // HAMMER_TEST_ADAPTERS_ADAPTER_CHECKS_H
