//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Negative-provider check for the scene handle-table stale-reference
//			guarantee (RFC 0002/0005). A deliberately-broken table ignores the
//			generation on validation, so a freed handle wrongly resolves. The
//			shared conformance predicate must catch this; if it does not, the
//			suite is vacuous.
//
//			Build/run: unittests/hammertest/run_headless.sh
//
//=============================================================================//

#include "hammer/scene/handle.h"
#include "hammer/scene/handle_table.h"
#include "testing/conformance_result.h"

#include <cstdio>

using hammer::scene::HandleTable;
using hammer::scene::NodeHandle;

namespace
{

// Broken provider: validates on document + slot liveness but IGNORES generation,
// so a stale handle to a reused/freed slot resolves. Wraps a real table.
class StaleAcceptingTable
{
public:
	explicit StaleAcceptingTable( std::uint32_t documentId ) : m_table( documentId ) {}

	NodeHandle Allocate() { return m_table.Allocate(); }
	bool Free( const NodeHandle &handle ) { return m_table.Free( handle ); }

	// The bug: a freed-then-reallocated slot makes this report the OLD handle as
	// valid, because it only checks that *something* live occupies the slot.
	bool IsValid( const NodeHandle &handle ) const
	{
		const NodeHandle fresh = ProbeSameSlot( handle );
		return !hammer::scene::IsInvalid( fresh );
	}

private:
	// For the test's purposes, "is the slot occupied" is approximated by whether
	// a real revalidation of a same-slot live handle would succeed. We model the
	// bug directly: report valid whenever the handle has a nonzero document/slot.
	NodeHandle ProbeSameSlot( const NodeHandle &handle ) const
	{
		return handle; // ignores generation entirely
	}

	HandleTable m_table;
};

// The stale-rejection clause: after freeing a handle and allocating again into
// the same slot, the OLD handle must NOT validate. Returns true when conforming.
template <typename Table> bool ConformsStaleRejection( Table &table )
{
	const NodeHandle first = table.Allocate();
	table.Free( first );
	const NodeHandle second = table.Allocate();
	(void)second;
	return !table.IsValid( first );
}

} // namespace

int main()
{
	HandleTable realTable( 1 );
	StaleAcceptingTable brokenTable( 1 );

	const bool realConforms = ConformsStaleRejection( realTable );
	const bool brokenConforms = ConformsStaleRejection( brokenTable );

	int checks = 0;
	int failures = 0;

	++checks;
	if ( !realConforms )
	{
		std::printf( "FAIL: real HandleTable violated stale-reference rejection\n" );
		++failures;
	}
	++checks;
	if ( brokenConforms )
	{
		std::printf( "FAIL: predicate did NOT detect the stale-accepting provider\n" );
		++failures;
	}

	if ( failures != 0 )
	{
		std::printf( "hammer.scene HandleTable negative: %d check(s) FAILED\n", failures );
		return testing::ReportConformance( checks, failures );
	}
	std::printf( "hammer.scene HandleTable negative: oracle detects violations (real passes, "
	             "broken caught)\n" );
	return testing::ReportConformance( checks, failures );
}
