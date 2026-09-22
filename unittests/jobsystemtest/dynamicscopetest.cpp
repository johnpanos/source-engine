//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Conformance tests for DynamicScope (RFC 0003, "Dynamic work"). The
//          RFC requires dynamic child-scope tests before the extension is
//          enabled. Covers: fan-out, continuations depending on admitted
//          children, a running child splitting into producer + continuation,
//          the single-worker no-block invariant, cancel cascade, cooperative
//          scope cancellation, dependence on already-completed work, rejection
//          of children after drain, the zero-worker owner-drain path, and a
//          randomized stress graph across worker counts with no hangs.
//
//=============================================================================//

#include "jobsystem/dynamic_scope.h"

#include <atomic>
#include <cstdio>
#include <thread>
#include <vector>

using namespace jobsystem;

static int g_checks = 0;
static int g_failures = 0;
static const char *g_curTest = "";

#define CHECK( cond )                                                                              \
	do                                                                                             \
	{                                                                                              \
		++g_checks;                                                                                \
		if ( !( cond ) )                                                                           \
		{                                                                                          \
			++g_failures;                                                                          \
			std::printf( "  FAIL [%s] %s:%d: %s\n", g_curTest, __FILE__, __LINE__, #cond );        \
		}                                                                                          \
	} while ( 0 )

#define RUN( fn )                                                                                  \
	do                                                                                             \
	{                                                                                              \
		g_curTest = #fn;                                                                           \
		std::printf( "- %s\n", #fn );                                                              \
		fn();                                                                                      \
	} while ( 0 )

using CH = DynamicScope::ChildHandle;

static void Test_FanOut()
{
	for ( int workers = 0; workers <= 4; ++workers )
	{
		DynamicScope scope( workers );
		std::atomic<int> n{ 0 };
		std::vector<CH> hs;
		for ( int i = 0; i < 16; ++i )
			hs.push_back( scope.Spawn( "leaf",
			    [&]( JobRunContext & )
			    {
				    n++;
			    } ) );
		scope.Wait();
		CHECK( n.load() == 16 );
		CHECK( scope.Succeeded() == 16 && scope.Admitted() == 16 );
		for ( CH h : hs )
			CHECK( scope.StateOf( h ) == JobState::Succeeded );
	}
}

// A continuation depends on an admitted producer and reads its published output.
static void Test_ContinuationSeesProducerOutput()
{
	for ( int workers = 1; workers <= 4; ++workers )
	{
		DynamicScope scope( workers );
		std::atomic<int> value{ 0 };
		CH prod = scope.Spawn( "produce",
		    [&]( JobRunContext & )
		    {
			    value.store( 42 );
		    } );
		std::atomic<int> seen{ -1 };
		scope.Spawn( "consume",
		    [&]( JobRunContext & )
		    {
			    seen.store( value.load() );
		    },
		    { prod } );
		scope.Wait();
		CHECK( seen.load() == 42 ); // producer output published to the continuation
	}
}

// A running child SPLITS into a producer (spawns sub-children) and a continuation
// that depends on them. With a single worker this only completes if the producer
// does NOT block the worker on its children (RFC: no worker blocks on child jobs).
static void Test_SplitProducerContinuationSingleWorker()
{
	DynamicScope scope( 1 );
	std::atomic<int> sum{ 0 };
	std::atomic<int> contSaw{ -1 };
	scope.Spawn( "producer",
	    [&]( JobRunContext & )
	    {
		    // Fan out three children, then a continuation depending on them. The
		    // producer returns immediately; it does not wait for the children.
		    std::vector<CH> kids;
		    for ( int i = 0; i < 3; ++i )
			    kids.push_back( scope.Spawn( "kid",
			        [&]( JobRunContext & )
			        {
				        sum.fetch_add( 10 );
			        } ) );
		    scope.Spawn(
		        "continuation",
		        [&]( JobRunContext & )
		        {
			        contSaw.store( sum.load() );
		        },
		        kids );
	    } );
	scope.Wait();
	CHECK( sum.load() == 30 );
	CHECK( contSaw.load() == 30 ); // continuation ran after all three kids
}

// A producer that fails cancels its Success-dependent, which cascades.
static void Test_CancelCascade()
{
	for ( int workers = 1; workers <= 4; ++workers )
	{
		DynamicScope scope( workers );
		std::atomic<bool> bRan{ false }, dRan{ false };
		CH a = scope.Spawn( "A",
		    []( JobRunContext &c )
		    {
			    c.Fail();
		    } );
		CH b = scope.Spawn( "B",
		    [&]( JobRunContext & )
		    {
			    bRan = true;
		    },
		    { a } );
		CH d = scope.Spawn( "D",
		    [&]( JobRunContext & )
		    {
			    dRan = true;
		    },
		    { b } );
		scope.Wait();
		CHECK( scope.StateOf( a ) == JobState::Failed );
		CHECK( scope.StateOf( b ) == JobState::Canceled );
		CHECK( scope.StateOf( d ) == JobState::Canceled );
		CHECK( !bRan.load() && !dRan.load() );
	}
}

// A dependency on an already-terminal child is accounted immediately at spawn.
static void Test_DependOnAlreadyCompleted()
{
	DynamicScope scope( 2 );
	std::atomic<bool> aDone{ false };
	CH a = scope.Spawn( "A",
	    [&]( JobRunContext & )
	    {
		    aDone = true;
	    } );
	// Spin (bounded) until A is terminal, then spawn a dependent on the completed A.
	for ( int i = 0; i < 1000000 && scope.StateOf( a ) != JobState::Succeeded; ++i )
		std::this_thread::yield();
	CHECK( scope.StateOf( a ) == JobState::Succeeded );
	std::atomic<bool> bRan{ false };
	CH b = scope.Spawn( "B",
	    [&]( JobRunContext & )
	    {
		    bRan = true;
	    },
	    { a } );
	CHECK( b.IsValid() );
	scope.Wait();
	CHECK( bRan.load() && scope.StateOf( b ) == JobState::Succeeded );
}

// After the scope drains, no further children may be admitted.
static void Test_SpawnAfterDrainRejected()
{
	DynamicScope scope( 2 );
	scope.Spawn( "x", []( JobRunContext & ) {} );
	scope.Wait();
	CH late = scope.Spawn( "late", []( JobRunContext & ) {} );
	CHECK( !late.IsValid() );
}

// Cooperative scope cancellation cancels not-yet-started children.
static void Test_CancelPending()
{
	DynamicScope scope( 2 );
	std::atomic<int> ran{ 0 };
	scope.CancelPending(); // cancel before any child can start
	std::vector<CH> hs;
	for ( int i = 0; i < 8; ++i )
		hs.push_back( scope.Spawn( "c",
		    [&]( JobRunContext & )
		    {
			    ran++;
		    } ) );
	scope.Wait();
	CHECK( ran.load() == 0 );
	CHECK( scope.Canceled() == 8 );
	for ( CH h : hs )
		CHECK( scope.StateOf( h ) == JobState::Canceled );
}

// Zero-worker scope: the owner drains the ready set itself in dependency order.
static void Test_ZeroWorkerOwnerDrain()
{
	DynamicScope scope( 0 );
	std::vector<int> order;
	CH a = scope.Spawn( "a",
	    [&]( JobRunContext & )
	    {
		    order.push_back( 0 );
	    } );
	CH b = scope.Spawn( "b",
	    [&]( JobRunContext & )
	    {
		    order.push_back( 1 );
	    },
	    { a } );
	scope.Spawn( "c",
	    [&]( JobRunContext & )
	    {
		    order.push_back( 2 );
	    },
	    { b } );
	scope.Wait();
	CHECK( order.size() == 3 );
	CHECK( order[0] == 0 && order[1] == 1 && order[2] == 2 );
}

// Randomized DAG of continuations across worker counts: exactly-once, no hangs,
// and every child with a satisfied chain succeeds.
static void Test_StressRandomDag()
{
	const int N = 200;
	for ( int workers = 1; workers <= 4; ++workers )
	{
		DynamicScope scope( workers );
		std::vector<std::atomic<int>> runs( N );
		for ( int i = 0; i < N; ++i )
			runs[i].store( 0 );
		std::vector<CH> hs;
		hs.reserve( N );
		uint32_t seed = 12345u + (uint32_t)workers;
		auto rnd = [&]
		{
			seed = seed * 1103515245u + 12345u;
			return seed >> 16;
		};
		for ( int i = 0; i < N; ++i )
		{
			std::vector<CH> deps;
			if ( i > 0 )
			{
				int nd = (int)( rnd() % 3 );
				for ( int k = 0; k < nd; ++k )
					deps.push_back( hs[rnd() % (uint32_t)i] );
			}
			hs.push_back( scope.Spawn(
			    "s",
			    [&runs, i]( JobRunContext & )
			    {
				    runs[i].fetch_add( 1 );
			    },
			    deps ) );
		}
		scope.Wait();
		CHECK( scope.Admitted() == (uint32_t)N );
		CHECK( scope.Succeeded() == (uint32_t)N ); // no failures/cancels in this graph
		bool once = true;
		for ( int i = 0; i < N; ++i )
			if ( runs[i].load() != 1 )
				once = false;
		CHECK( once );
	}
}

int main()
{
	std::printf( "dynamicscopetest (RFC 0003 dynamic child scopes)\n" );
	RUN( Test_FanOut );
	RUN( Test_ContinuationSeesProducerOutput );
	RUN( Test_SplitProducerContinuationSingleWorker );
	RUN( Test_CancelCascade );
	RUN( Test_DependOnAlreadyCompleted );
	RUN( Test_SpawnAfterDrainRejected );
	RUN( Test_CancelPending );
	RUN( Test_ZeroWorkerOwnerDrain );
	RUN( Test_StressRandomDag );
	std::printf( "\n%d checks, %d failures\n", g_checks, g_failures );
	return g_failures == 0 ? 0 : 1;
}
