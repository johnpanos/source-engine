//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Adapter policy shims for the adapters.source.vector_interop.v1
//			conformance suite. The real policy forwards to the single named owner
//			(hammer::adapters::source). A deliberately-broken policy lives in the
//			negative suite to prove the shared checks detect a boundary violation.
//
//			These suites require the Source platform layer (Vector, BoundBox), so
//			they run on the MSVC/Wine parity lane, not on linux-headless-core.
//
//=============================================================================//

#ifndef HAMMER_TEST_ADAPTERS_ADAPTER_POLICY_H
#define HAMMER_TEST_ADAPTERS_ADAPTER_POLICY_H

#include "hammer/adapters/source/vector_interop.h"

// The real boundary policy under test: forwards to the named owner verbatim.
struct RealSourceAdapter
{
	static hammer::geometry::Vec3 ToVec3( const Vector &v )
	{
		return hammer::adapters::source::ToVec3( v );
	}
	static Vector ToVector( const hammer::geometry::Vec3 &v )
	{
		return hammer::adapters::source::ToVector( v );
	}
	static hammer::geometry::AxisAlignedBox ToAABB( const BoundBox &b )
	{
		return hammer::adapters::source::ToAABB( b );
	}
	static void FromAABB( const hammer::geometry::AxisAlignedBox &a, BoundBox &b )
	{
		hammer::adapters::source::FromAABB( a, b );
	}
};

#endif // HAMMER_TEST_ADAPTERS_ADAPTER_POLICY_H
