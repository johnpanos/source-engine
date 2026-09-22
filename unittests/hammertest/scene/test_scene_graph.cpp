//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Conformance oracle for hammer::scene::SceneGraph (RFC 0002 H1 scene
//			boundary). Pins the required invariants: valid references, no cycles,
//			atomic reparent (failure leaves the graph unchanged), atomic subtree
//			delete with stale-handle rejection, and independent documents.
//
//			Build/run: unittests/hammertest/run_headless.sh
//
//=============================================================================//

#include "hammer/scene/scene_graph.h"

#include <cstdio>

using hammer::scene::NodeHandle;
using hammer::scene::ReparentResult;
using hammer::scene::SceneGraph;

namespace
{

int g_failures = 0;

void Check( bool condition, const char *expression, int line )
{
	if ( !condition )
	{
		std::printf( "FAIL %s:%d: %s\n", "test_scene_graph.cpp", line, expression );
		++g_failures;
	}
}

} // namespace

#define CHECK( expr ) Check( ( expr ), #expr, __LINE__ )

// Build a small tree and verify parent/child queries and node counting.
void TestBuildTree()
{
	SceneGraph graph( 1 );
	const NodeHandle root = graph.CreateNode();
	const NodeHandle a = graph.CreateNode();
	const NodeHandle b = graph.CreateNode();

	CHECK( graph.NodeCount() == 3 );
	CHECK( graph.SetParent( a, root ) == ReparentResult::kOk );
	CHECK( graph.SetParent( b, root ) == ReparentResult::kOk );
	CHECK( graph.GetParent( a ) == root );
	CHECK( graph.Children( root ).size() == 2 );
	CHECK( graph.IsAncestor( root, a ) );
	CHECK( !graph.IsAncestor( a, root ) );
}

// A reparent that would create a cycle is rejected and leaves the graph
// unchanged (atomic failure).
void TestCycleRejected()
{
	SceneGraph graph( 1 );
	const NodeHandle a = graph.CreateNode();
	const NodeHandle b = graph.CreateNode();
	CHECK( graph.SetParent( b, a ) == ReparentResult::kOk ); // b under a

	// Making a a child of b would form a cycle; must be rejected, no change.
	CHECK( graph.SetParent( a, b ) == ReparentResult::kWouldCycle );
	CHECK( graph.GetParent( a ).document == 0 ); // a is still a root
	CHECK( graph.GetParent( b ) == a );          // b still under a
	CHECK( graph.SetParent( a, a ) == ReparentResult::kSelfParent );
}

// Removing a node deletes its whole subtree atomically and invalidates every
// affected handle.
void TestRemoveSubtree()
{
	SceneGraph graph( 1 );
	const NodeHandle root = graph.CreateNode();
	const NodeHandle mid = graph.CreateNode();
	const NodeHandle leaf = graph.CreateNode();
	graph.SetParent( mid, root );
	graph.SetParent( leaf, mid );
	CHECK( graph.NodeCount() == 3 );

	CHECK( graph.Remove( mid ) );
	CHECK( graph.NodeCount() == 1 );      // only root remains
	CHECK( !graph.IsValid( mid ) );       // stale
	CHECK( !graph.IsValid( leaf ) );      // subtree stale too
	CHECK( graph.IsValid( root ) );       // untouched
	CHECK( graph.Children( root ).empty() ); // mid removed from root's children
	CHECK( !graph.Remove( mid ) );        // double remove rejected
}

// A handle from one document is rejected by another document's graph, even for
// reparenting.
void TestIndependentDocuments()
{
	SceneGraph docA( 10 );
	SceneGraph docB( 20 );
	const NodeHandle a = docA.CreateNode();
	const NodeHandle aChild = docA.CreateNode();
	const NodeHandle b = docB.CreateNode();

	CHECK( !docB.IsValid( a ) );
	CHECK( docB.SetParent( b, a ) == ReparentResult::kForeignDocument );
	CHECK( docA.SetParent( a, b ) == ReparentResult::kForeignDocument );
	// A stale/foreign child is rejected as invalid child.
	CHECK( docB.SetParent( a, b ) == ReparentResult::kInvalidChild );
	// Sanity: within-document reparent still works.
	CHECK( docA.SetParent( aChild, a ) == ReparentResult::kOk );
}

int main()
{
	TestBuildTree();
	TestCycleRejected();
	TestRemoveSubtree();
	TestIndependentDocuments();

	if ( g_failures != 0 )
	{
		std::printf( "hammer.scene SceneGraph: %d check(s) FAILED\n", g_failures );
		return 1;
	}
	std::printf( "hammer.scene SceneGraph: all checks passed\n" );
	return 0;
}
