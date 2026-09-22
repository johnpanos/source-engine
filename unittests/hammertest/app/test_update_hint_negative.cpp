//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Sensitivity check for the bounded-notify-code clause of
//			app.update_hint.v1 (RFC 0002/0005). A provider that ignores the
//			kMaxNotifyCodes capacity limit accepts an unbounded number of distinct
//			codes -- the exact overflow the legacy CUpdateHint guarded against with
//			MAX_NOTIFY_CODES. The oracle must detect this; otherwise it does not
//			actually pin the capacity rule.
//
//			Convention: exit 0 when the real provider conforms AND the broken one
//			is caught; exit 1 otherwise.
//			Build/run: unittests/hammertest/run_headless.sh
//
//=============================================================================//

#include "hammer/app/update_hint.h"

#include <cstdio>
#include <vector>

using hammer::app::kMaxNotifyCodes;
using hammer::app::UpdateHint;
using hammer::geometry::AxisAlignedBox;
using hammer::geometry::Vec3;
using hammer::scene::NodeHandle;

namespace
{

// Broken provider: buckets by code like the real one, but never enforces the
// kMaxNotifyCodes limit, so a new code past capacity is wrongly accepted.
class UnboundedUpdateHint
{
public:
	bool PostUpdate( NodeHandle node, int notifyCode, const AxisAlignedBox &bounds )
	{
		for ( auto &bucket : m_buckets )
		{
			if ( bucket.first == notifyCode )
			{
				bucket.second.push_back( node );
				m_region.UpdateBounds( bounds );
				return true;
			}
		}
		m_buckets.push_back( { notifyCode, { node } } ); // BUG: no capacity check
		m_region.UpdateBounds( bounds );
		return true;
	}
	int NotifyCodeCount() const { return static_cast<int>( m_buckets.size() ); }

private:
	std::vector<std::pair<int, std::vector<NodeHandle>>> m_buckets;
	AxisAlignedBox m_region;
};

AxisAlignedBox UnitBox()
{
	return AxisAlignedBox( Vec3( 0, 0, 0 ), Vec3( 1, 1, 1 ) );
}

NodeHandle Node( std::uint32_t i )
{
	return NodeHandle{ 1u, i, 1u };
}

// The bounded-code clause: after posting kMaxNotifyCodes distinct codes, a new
// code must be rejected and the code count must stay at the cap. Returns true
// when the provider enforces the limit.
template <typename Hint> bool ConformsBoundedNotifyCodes( Hint &hint )
{
	for ( int i = 0; i < kMaxNotifyCodes; ++i )
	{
		hint.PostUpdate( Node( i ), 1000 + i, UnitBox() );
	}
	const bool acceptedOverflow =
		hint.PostUpdate( Node( 999 ), 9999, UnitBox() ); // a NEW code past capacity
	return !acceptedOverflow && hint.NotifyCodeCount() == kMaxNotifyCodes;
}

} // namespace

int main()
{
	UpdateHint real;
	UnboundedUpdateHint broken;

	const bool realConforms = ConformsBoundedNotifyCodes( real );
	const bool brokenConforms = ConformsBoundedNotifyCodes( broken );

	int failures = 0;
	if ( !realConforms )
	{
		std::printf( "FAIL: real UpdateHint did not enforce the notify-code capacity\n" );
		++failures;
	}
	if ( brokenConforms )
	{
		std::printf( "FAIL: oracle did not catch the unbounded provider\n" );
		++failures;
	}

	if ( failures != 0 )
	{
		std::printf( "app.update_hint negative: ORACLE UNSOUND (%d)\n", failures );
		return 1;
	}
	std::printf( "app.update_hint negative: oracle catches unbounded notify codes "
				 "(real enforces cap, broken caught)\n" );
	return 0;
}
