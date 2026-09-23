//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Sensitivity check for the scene-graph cycle invariant (RFC 0002/0005).
//			A permissive graph that accepts any reparent (no cycle check) must be
//			detected by the shared no-cycle conformance predicate; otherwise the
//			oracle does not actually pin the invariant.
//
//			Build/run: unittests/hammertest/run_headless.sh
//
//=============================================================================//

#include "hammer/scene/scene_graph.h"
#include "testing/conformance_result.h"

#include <cstdint>
#include <cstdio>

using hammer::scene::NodeHandle;
using hammer::scene::SceneGraph;

namespace
{

// Broken provider: hands out sequential handles and accepts EVERY reparent,
// including one that forms a cycle. Only implements what the predicate needs.
class PermissiveGraph
{
public:
	NodeHandle CreateNode()
	{
		NodeHandle handle;
		handle.document = 1;
		handle.index = m_next++;
		handle.generation = 1;
		return handle;
	}

	bool TrySetParent( const NodeHandle &, const NodeHandle & )
	{
		return true; // never rejects -- the bug
	}

private:
	std::uint32_t m_next = 0;
};

// The no-cycle clause: after b is parented under a, parenting a under b (a's
// descendant) must be REJECTED. Returns true when the provider enforces it.
template <typename Graph> bool ConformsNoCycle( Graph &graph )
{
	const NodeHandle a = graph.CreateNode();
	const NodeHandle b = graph.CreateNode();
	const bool firstOk = graph.TrySetParent( b, a );        // b under a: allowed
	const bool cycleRejected = !graph.TrySetParent( a, b ); // a under b: must fail
	return firstOk && cycleRejected;
}

} // namespace

int main()
{
	SceneGraph realGraph( 1 );
	PermissiveGraph brokenGraph;

	const bool realConforms = ConformsNoCycle( realGraph );
	const bool brokenConforms = ConformsNoCycle( brokenGraph );

	int checks = 0;
	int failures = 0;

	++checks;
	if ( !realConforms )
	{
		std::printf( "FAIL: real SceneGraph did not reject a cycle-forming reparent\n" );
		++failures;
	}
	++checks;
	if ( brokenConforms )
	{
		std::printf( "FAIL: predicate did NOT detect the permissive (cycle-accepting) provider\n" );
		++failures;
	}

	if ( failures != 0 )
	{
		std::printf( "hammer.scene SceneGraph negative: %d check(s) FAILED\n", failures );
		return testing::ReportConformance( checks, failures );
	}
	std::printf( "hammer.scene SceneGraph negative: oracle detects violations (real passes, "
	             "permissive caught)\n" );
	return testing::ReportConformance( checks, failures );
}
