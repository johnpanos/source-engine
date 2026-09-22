//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Conformance oracle for hammer::scene::HandleTable (RFC 0002). Pins the
//			stale-reference and independent-document guarantees the scene seam
//			requires: a freed handle stops resolving, a reused slot rejects the old
//			handle by generation, and one document's handle never resolves in
//			another document's table (even if the slot address is reused).
//
//			Build/run: unittests/hammertest/run_headless.sh
//
//=============================================================================//

#include "hammer/scene/handle_table.h"

#include <cstdio>

using hammer::scene::HandleTable;
using hammer::scene::IsInvalid;
using hammer::scene::kInvalidNodeHandle;
using hammer::scene::NodeHandle;

namespace
{

int g_failures = 0;

void Check( bool condition, const char *expression, int line )
{
	if ( !condition )
	{
		std::printf( "FAIL %s:%d: %s\n", "test_handle_table.cpp", line, expression );
		++g_failures;
	}
}

} // namespace

#define CHECK( expr ) Check( ( expr ), #expr, __LINE__ )

// A live handle resolves; a freed handle does not; double-free is rejected.
void TestAllocateFreeResolve()
{
	HandleTable table( 1 );
	CHECK( table.LiveCount() == 0 );

	const NodeHandle a = table.Allocate();
	CHECK( !IsInvalid( a ) );
	CHECK( table.IsValid( a ) );
	CHECK( table.LiveCount() == 1 );

	CHECK( table.Free( a ) );
	CHECK( !table.IsValid( a ) ); // stale after free
	CHECK( table.LiveCount() == 0 );
	CHECK( !table.Free( a ) ); // double free rejected
}

// Reusing a freed slot must not revive an old handle: the generation differs.
void TestSlotReuseRejectsStale()
{
	HandleTable table( 7 );
	const NodeHandle first = table.Allocate();
	CHECK( table.Free( first ) );

	const NodeHandle second = table.Allocate();
	// Same slot index reused, but the stale handle must not validate.
	CHECK( second.index == first.index );
	CHECK( second.generation != first.generation );
	CHECK( table.IsValid( second ) );
	CHECK( !table.IsValid( first ) );
}

// A handle from one document never resolves in another document's table, even
// when both reuse the same slot index. This is the "reused address" guard.
void TestIndependentDocuments()
{
	HandleTable docA( 100 );
	HandleTable docB( 200 );

	const NodeHandle a = docA.Allocate();
	const NodeHandle b = docB.Allocate();

	CHECK( a.index == b.index ); // same slot index in each table
	CHECK( a.document != b.document );
	CHECK( docA.IsValid( a ) && !docB.IsValid( a ) );
	CHECK( docB.IsValid( b ) && !docA.IsValid( b ) );
}

// The invalid handle and a zero-document table never resolve.
void TestInvalidHandles()
{
	HandleTable table( 5 );
	CHECK( !table.IsValid( kInvalidNodeHandle ) );

	HandleTable noDoc( 0 );
	const NodeHandle h = noDoc.Allocate();
	CHECK( IsInvalid( h ) );
	CHECK( !noDoc.IsValid( h ) );
}

int main()
{
	TestAllocateFreeResolve();
	TestSlotReuseRejectsStale();
	TestIndependentDocuments();
	TestInvalidHandles();

	if ( g_failures != 0 )
	{
		std::printf( "hammer.scene HandleTable: %d check(s) FAILED\n", g_failures );
		return 1;
	}
	std::printf( "hammer.scene HandleTable: all checks passed\n" );
	return 0;
}
