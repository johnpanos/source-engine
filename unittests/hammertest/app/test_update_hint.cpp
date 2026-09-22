//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Positive conformance suite for app.update_hint.v1 (RFC 0002/0005).
//			Pins the aggregation policy of hammer::app::UpdateHint: notify-code
//			bucketing, the bounded code count, and the unioned affected region.
//			Headless; no MFC/tier0/PCH. Build/run: unittests/hammertest/run_headless.sh
//
//=============================================================================//

#include "hammer/app/update_hint.h"

#include <cstdio>

using hammer::app::UpdateHint;
using hammer::geometry::AxisAlignedBox;
using hammer::geometry::Vec3;
using hammer::scene::NodeHandle;

namespace
{

int g_failures = 0;

void Check( bool ok, const char *label )
{
	if ( !ok )
	{
		std::printf( "FAIL: %s\n", label );
		++g_failures;
	}
}

NodeHandle Node( std::uint32_t index )
{
	return NodeHandle{ 1u, index, 1u };
}

AxisAlignedBox Box( float x0, float y0, float z0, float x1, float y1, float z1 )
{
	return AxisAlignedBox( Vec3( x0, y0, z0 ), Vec3( x1, y1, z1 ) );
}

} // namespace

int main()
{
	UpdateHint hint;

	// Fresh hint: no codes, invalid region.
	Check( hint.NotifyCodeCount() == 0, "fresh: no notify codes" );
	Check( !hint.UpdateRegion().IsValidBox(), "fresh: region invalid" );

	// Two nodes under the same code share one bucket; region unions both.
	Check( hint.PostUpdate( Node( 1 ), 100, Box( 0, 0, 0, 1, 1, 1 ) ), "post code 100 #1" );
	Check( hint.PostUpdate( Node( 2 ), 100, Box( 4, 4, 4, 5, 5, 5 ) ), "post code 100 #2" );
	Check( hint.NotifyCodeCount() == 1, "same code -> one bucket" );
	Check( hint.NotifyCodeAt( 0 ) == 100, "bucket 0 code == 100" );
	Check( hint.NodesAt( 0 ).size() == 2, "bucket 0 has two nodes" );
	Check( hint.NodesAt( 0 )[0] == Node( 1 ) && hint.NodesAt( 0 )[1] == Node( 2 ),
		   "bucket 0 preserves report order" );

	// Region is the union [0..5]^3.
	const AxisAlignedBox &region = hint.UpdateRegion();
	Check( region.IsValidBox(), "region valid after posts" );
	Check( region.mins[0] == 0 && region.mins[1] == 0 && region.mins[2] == 0,
		   "region mins == (0,0,0)" );
	Check( region.maxs[0] == 5 && region.maxs[1] == 5 && region.maxs[2] == 5,
		   "region maxs == (5,5,5)" );

	// A different code opens a second bucket.
	Check( hint.PostUpdate( Node( 3 ), 200, Box( -2, -2, -2, -1, -1, -1 ) ), "post code 200" );
	Check( hint.NotifyCodeCount() == 2, "different code -> two buckets" );
	Check( hint.NotifyCodeAt( 1 ) == 200, "bucket 1 code == 200" );
	Check( hint.UpdateRegion().mins[0] == -2, "region grows to include code 200 bounds" );

	// PreUpdate accumulates region without adding a bucket.
	hint.PreUpdate( Box( 9, 9, 9, 10, 10, 10 ) );
	Check( hint.NotifyCodeCount() == 2, "PreUpdate adds no bucket" );
	Check( hint.UpdateRegion().maxs[0] == 10, "PreUpdate grows region" );

	// Reset clears everything.
	hint.Reset();
	Check( hint.NotifyCodeCount() == 0, "reset clears buckets" );
	Check( !hint.UpdateRegion().IsValidBox(), "reset invalidates region" );

	// Capacity limit: fill all kMaxNotifyCodes distinct codes, then a new one is
	// rejected while an existing one still succeeds.
	UpdateHint full;
	for ( int i = 0; i < hammer::app::kMaxNotifyCodes; ++i )
	{
		Check( full.PostUpdate( Node( i ), 1000 + i, Box( 0, 0, 0, 1, 1, 1 ) ),
			   "fill distinct code" );
	}
	Check( full.NotifyCodeCount() == hammer::app::kMaxNotifyCodes, "all buckets filled" );

	const AxisAlignedBox before = full.UpdateRegion();
	Check( !full.PostUpdate( Node( 999 ), 9999, Box( 100, 100, 100, 200, 200, 200 ) ),
		   "new code past capacity is rejected" );
	Check( full.NotifyCodeCount() == hammer::app::kMaxNotifyCodes, "rejection adds no bucket" );
	Check( full.UpdateRegion().maxs[0] == before.maxs[0],
		   "rejected post does not grow region" );

	// An existing code still accepts more nodes even when full.
	Check( full.PostUpdate( Node( 42 ), 1000, Box( 0, 0, 0, 1, 1, 1 ) ),
		   "existing code accepted when full" );
	Check( full.NodesAt( 0 ).size() == 2, "existing bucket grew" );

	if ( g_failures != 0 )
	{
		std::printf( "app.update_hint: %d FAILURE(S)\n", g_failures );
		return 1;
	}
	std::printf( "app.update_hint: aggregation policy holds\n" );
	return 0;
}
