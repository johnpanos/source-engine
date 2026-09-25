//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: DeclaredFrameGraph contract (RFC 0003 R20/R21, RFC 0005 Q-JOBS).
//
//          Serial runs are array order with batch items ascending; edges come
//          only from the host chain and conflicting declarations; pooled runs
//          overlap exactly the unordered nodes and publish a batch's outputs
//          to every later conflicting node; each item runs once with
//          begin/end bracketing each participating runner; nested batches
//          run inline; invalid input runs nothing; the sealed graph is reused
//          while the shape is unchanged.
//
//=============================================================================//

#include "jobsystem/declared_frame_graph.h"
#include "jobsystem/worker_backend.h"
#include "testing/conformance_result.h"

#include <atomic>
#include <chrono>
#include <cstdio>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

using namespace jobsystem;

static int g_checks = 0;
static int g_failures = 0;
static const char *g_test = "";

#define CHECK( condition )                                                                         \
	do                                                                                             \
	{                                                                                              \
		++g_checks;                                                                                \
		if ( !( condition ) )                                                                      \
		{                                                                                          \
			++g_failures;                                                                          \
			std::printf( "  FAIL [%s] line %d: %s\n", g_test, __LINE__, #condition );              \
		}                                                                                          \
	} while ( 0 )

#define RUN( function )                                                                            \
	do                                                                                             \
	{                                                                                              \
		g_test = #function;                                                                        \
		std::printf( "- %s\n", g_test );                                                           \
		function();                                                                                \
	} while ( 0 )

// Workers start before the caller's jobs, like the engine pool bridge.
class ThreadBackend final : public IWorkerBackend
{
public:
	explicit ThreadBackend( int workers ) : m_workers( workers ) {}

	int WorkerCount() const override { return m_workers; }

	void ParallelFor( int count, const std::function<void( int )> &body ) override
	{
		ParallelForWithCaller( count, body, [] {} );
	}

	void ParallelForWithCaller( int count, const std::function<void( int )> &body,
	    const std::function<void()> &caller ) override
	{
		std::atomic<int> next( 0 );
		auto drain = [&]
		{
			for ( int i = next++; i < count; i = next++ )
				body( i );
		};
		std::vector<std::thread> threads;
		for ( int worker = 0; worker < m_workers; ++worker )
			threads.emplace_back( drain );
		caller();
		drain();
		for ( std::thread &thread : threads )
			thread.join();
	}

private:
	int m_workers;
};

static bool AwaitFlag( const std::atomic<bool> &flag, int ms = 5000 )
{
	const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds( ms );
	while ( !flag.load() )
	{
		if ( std::chrono::steady_clock::now() > deadline )
			return false;
		std::this_thread::yield();
	}
	return true;
}

//-----------------------------------------------------------------------------
// A small recording frame: host nodes and one batch, with configurable
// declarations.
//-----------------------------------------------------------------------------

struct Recorder
{
	std::mutex mutex;
	std::vector<std::string> events;
	std::thread::id caller = std::this_thread::get_id();
	int hostOffCaller = 0;

	void Add( const std::string &event )
	{
		std::lock_guard<std::mutex> lock( mutex );
		events.push_back( event );
	}
};

struct HostNode
{
	Recorder *recorder;
	const char *name;
	std::function<void()> body;

	static void Run( void *context )
	{
		HostNode &node = *static_cast<HostNode *>( context );
		if ( std::this_thread::get_id() != node.recorder->caller )
			node.recorder->hostOffCaller++;
		node.recorder->Add( node.name );
		if ( node.body )
			node.body();
	}
};

struct BatchNode
{
	Recorder *recorder;
	unsigned items = 0;
	std::vector<int> ran;    // times each item ran
	std::vector<int> values; // item outputs
	std::atomic<int> begins{ 0 }, ends{ 0 };
	std::function<void( unsigned )> body;
	bool record = true;

	static unsigned Count( void *context )
	{
		BatchNode &node = *static_cast<BatchNode *>( context );
		node.ran.assign( node.items, 0 );
		node.values.assign( node.items, 0 );
		return node.items;
	}
	static void Process( void *context, unsigned index )
	{
		BatchNode &node = *static_cast<BatchNode *>( context );
		node.ran[index]++;
		node.values[index] = (int)index * 3 + 1;
		if ( node.record )
			node.recorder->Add( "item" + std::to_string( index ) );
		if ( node.body )
			node.body( index );
	}
	static void Begin( void *context ) { static_cast<BatchNode *>( context )->begins++; }
	static void End( void *context ) { static_cast<BatchNode *>( context )->ends++; }
};

static FrameNodeDesc Host( HostNode &node, const FrameAccess *access, unsigned n )
{
	FrameNodeDesc desc = {};
	desc.name = node.name;
	desc.kind = FRAME_NODE_HOST;
	desc.context = &node;
	desc.run = &HostNode::Run;
	desc.access = access;
	desc.accessCount = n;
	return desc;
}

static FrameNodeDesc Batch(
    const char *name, BatchNode &node, const FrameAccess *access, unsigned n )
{
	FrameNodeDesc desc = {};
	desc.name = name;
	desc.kind = FRAME_NODE_BATCH;
	desc.context = &node;
	desc.count = &BatchNode::Count;
	desc.process = &BatchNode::Process;
	desc.begin = &BatchNode::Begin;
	desc.end = &BatchNode::End;
	desc.access = access;
	desc.accessCount = n;
	return desc;
}

enum Domain
{
	D_INPUT = 1,
	D_ITEMS,
	D_UNRELATED,
};

static const FrameAccess kWriteInput[] = { { D_INPUT, true } };
static const FrameAccess kBatchAccess[] = { { D_INPUT, false }, { D_ITEMS, true } };
static const FrameAccess kReadItems[] = { { D_ITEMS, false } };
static const FrameAccess kUnrelated[] = { { D_UNRELATED, true } };
static const FrameAccess kAll[] = { { FRAME_DOMAIN_ALL, true } };

//-----------------------------------------------------------------------------

static void Test_SerialIsArrayOrder()
{
	Recorder recorder;
	HostNode gather = { &recorder, "gather", {} };
	HostNode unrelated = { &recorder, "unrelated", {} };
	HostNode commit = { &recorder, "commit", {} };
	BatchNode batch;
	batch.recorder = &recorder;
	batch.items = 5;
	const FrameNodeDesc nodes[] = { Host( gather, kWriteInput, 1 ),
	    Batch( "compute", batch, kBatchAccess, 2 ), Host( unrelated, kUnrelated, 1 ),
	    Host( commit, kReadItems, 1 ) };

	DeclaredFrameGraph graph;
	ThreadBackend backend( 3 );
	// A backend is ignored in serial mode.
	DeclaredFrameRun run = graph.Run( nodes, 4, &backend, FRAME_GRAPH_SERIAL );
	CHECK( run.valid && run.hostNodesRun == 3 && run.batchItemsRun == 5 && run.batchRunners == 1 );
	const std::vector<std::string> expected = {
	    "gather", "item0", "item1", "item2", "item3", "item4", "unrelated", "commit" };
	CHECK( recorder.events == expected );
	CHECK( recorder.hostOffCaller == 0 );
	CHECK( batch.begins == 1 && batch.ends == 1 );

	// Declared edges: the batch follows gather and precedes commit; nothing
	// orders the batch against the unrelated host node.
	CHECK( !graph.MayOverlap( 0, 1 ) );
	CHECK( graph.MayOverlap( 1, 2 ) );
	CHECK( !graph.MayOverlap( 1, 3 ) );
	CHECK( !graph.MayOverlap( 0, 2 ) && !graph.MayOverlap( 2, 3 ) ); // host chain
	CHECK( graph.OverlappingPairs() == 1 );
}

static void Test_AllDomainOrdersEverything()
{
	Recorder recorder;
	HostNode gather = { &recorder, "gather", {} };
	HostNode legacy = { &recorder, "legacy", {} };
	BatchNode batch;
	batch.recorder = &recorder;
	batch.items = 2;
	const FrameNodeDesc nodes[] = { Host( gather, kWriteInput, 1 ),
	    Batch( "compute", batch, kBatchAccess, 2 ), Host( legacy, kAll, 1 ) };
	DeclaredFrameGraph graph;
	CHECK( graph.Run( nodes, 3, nullptr, FRAME_GRAPH_SERIAL ).valid );
	CHECK( graph.OverlappingPairs() == 0 );

	// A batch with no declarations overlaps every non-conflicting node.
	const FrameNodeDesc undeclared[] = { Host( gather, kWriteInput, 1 ),
	    Batch( "compute", batch, nullptr, 0 ), Host( legacy, kUnrelated, 1 ) };
	CHECK( graph.Run( undeclared, 3, nullptr, FRAME_GRAPH_SERIAL ).valid );
	CHECK( graph.MayOverlap( 0, 1 ) && graph.MayOverlap( 1, 2 ) );
}

static void Test_PooledOverlapsUnorderedNodes()
{
	// The batch's items wait for the unrelated host node to start, and it waits
	// for an item to start: both finish only if they run concurrently.
	for ( int round = 0; round < 10; ++round )
	{
		Recorder recorder;
		std::atomic<bool> itemStarted( false ), hostStarted( false );
		std::atomic<bool> itemSaw( false ), hostSaw( false );
		HostNode gather = { &recorder, "gather", {} };
		HostNode unrelated = { &recorder, "unrelated", [&]
		    {
			    hostStarted = true;
			    hostSaw = AwaitFlag( itemStarted );
		    } };
		HostNode commit = { &recorder, "commit", {} };
		BatchNode batch;
		batch.recorder = &recorder;
		batch.items = 1;
		batch.body = [&]( unsigned )
		{
			itemStarted = true;
			itemSaw = AwaitFlag( hostStarted );
		};
		const FrameNodeDesc nodes[] = { Host( gather, kWriteInput, 1 ),
		    Batch( "compute", batch, kBatchAccess, 2 ), Host( unrelated, kUnrelated, 1 ),
		    Host( commit, kReadItems, 1 ) };
		DeclaredFrameGraph graph;
		ThreadBackend backend( 2 );
		DeclaredFrameRun run = graph.Run( nodes, 4, &backend, FRAME_GRAPH_POOLED );
		CHECK( run.valid && run.batchRunners == 3 && run.batchItemsRun == 1 );
		CHECK( itemSaw.load() && hostSaw.load() );
		CHECK( recorder.hostOffCaller == 0 );
		CHECK( recorder.events.front() == "gather" && recorder.events.back() == "commit" );
	}
}

static void Test_PooledPublishesToConflictingNodes()
{
	// Many items on several runners; the commit node reads every output, and
	// the count comes from the gather node that runs first.
	for ( int round = 0; round < 20; ++round )
	{
		Recorder recorder;
		BatchNode batch;
		batch.recorder = &recorder;
		batch.record = false;
		HostNode gather = { &recorder, "gather", [&]
		    {
			    batch.items = 1000 + (unsigned)round;
		    } };
		int sum = -1;
		HostNode commit = { &recorder, "commit", [&]
		    {
			    sum = 0;
			    for ( int value : batch.values )
				    sum += value;
		    } };
		const FrameNodeDesc nodes[] = { Host( gather, kWriteInput, 1 ),
		    Batch( "compute", batch, kBatchAccess, 2 ), Host( commit, kReadItems, 1 ) };
		DeclaredFrameGraph graph;
		ThreadBackend backend( 3 );
		DeclaredFrameRun run = graph.Run( nodes, 3, &backend, FRAME_GRAPH_POOLED );
		const unsigned n = batch.items;
		CHECK( run.valid && run.batchItemsRun == n );
		int once = 0;
		for ( int count : batch.ran )
			once += count == 1 ? 1 : 0;
		CHECK( once == (int)n );
		CHECK( sum == (int)( 3 * ( n * ( n - 1 ) / 2 ) + n ) );
		// Each participating runner is bracketed; an empty one is not.
		CHECK( batch.begins == batch.ends && batch.begins >= 1 && batch.begins <= 4 );
		CHECK( graph.OverlappingPairs() == 0 );
	}
}

static void Test_EmptyBatchRunsNoHooks()
{
	Recorder recorder;
	HostNode gather = { &recorder, "gather", {} };
	BatchNode batch;
	batch.recorder = &recorder;
	batch.items = 0;
	const FrameNodeDesc nodes[] = {
	    Host( gather, kWriteInput, 1 ), Batch( "compute", batch, kBatchAccess, 2 ) };
	DeclaredFrameGraph graph;
	ThreadBackend backend( 2 );
	DeclaredFrameRun run = graph.Run( nodes, 2, &backend, FRAME_GRAPH_POOLED );
	CHECK( run.valid && run.batchItemsRun == 0 );
	CHECK( batch.begins == 0 && batch.ends == 0 );
}

static void Test_NestedBatchRunsInline()
{
	// A pooled graph started from inside a batch item runs serially.
	Recorder recorder;
	std::atomic<int> nestedRunners( -1 );
	std::atomic<int> nestedOffThread( 0 );
	BatchNode inner;
	inner.recorder = &recorder;
	inner.record = false;
	inner.items = 8;
	BatchNode outer;
	outer.recorder = &recorder;
	outer.record = false;
	outer.items = 1;
	ThreadBackend backend( 2 );
	outer.body = [&]( unsigned )
	{
		const std::thread::id self = std::this_thread::get_id();
		BatchNode *pInner = &inner;
		inner.body = [&, self]( unsigned )
		{
			if ( std::this_thread::get_id() != self )
				nestedOffThread++;
		};
		const FrameNodeDesc nested[] = { Batch( "inner", *pInner, kBatchAccess, 2 ) };
		DeclaredFrameGraph graph;
		DeclaredFrameRun run = graph.Run( nested, 1, &backend, FRAME_GRAPH_POOLED );
		nestedRunners = run.valid ? (int)run.batchRunners : -2;
	};
	HostNode gather = { &recorder, "gather", {} };
	const FrameNodeDesc nodes[] = {
	    Host( gather, kWriteInput, 1 ), Batch( "outer", outer, kBatchAccess, 2 ) };
	DeclaredFrameGraph graph;
	CHECK( graph.Run( nodes, 2, &backend, FRAME_GRAPH_POOLED ).valid );
	CHECK( nestedRunners == 1 );
	CHECK( nestedOffThread == 0 );
}

static void Test_InvalidInputAndReuse()
{
	Recorder recorder;
	HostNode a = { &recorder, "a", {} };
	HostNode b = { &recorder, "b", {} };
	FrameNodeDesc nodes[] = { Host( a, kWriteInput, 1 ), Host( b, kReadItems, 1 ) };
	DeclaredFrameGraph graph;
	CHECK( graph.Run( nodes, 2, nullptr, FRAME_GRAPH_SERIAL ).valid );
	CHECK( graph.Run( nodes, 2, nullptr, FRAME_GRAPH_SERIAL ).valid );
	CHECK( graph.SealCount() == 1 );

	// A changed declaration is a new shape.
	nodes[1].access = kWriteInput;
	CHECK( graph.Run( nodes, 2, nullptr, FRAME_GRAPH_SERIAL ).valid );
	CHECK( graph.SealCount() == 2 );

	// Pooled with a backend is a different shape from serial.
	ThreadBackend backend( 2 );
	CHECK( graph.Run( nodes, 2, &backend, FRAME_GRAPH_POOLED ).valid );
	CHECK( graph.SealCount() == 3 );

	recorder.events.clear();
	FrameNodeDesc bad[] = { Host( a, kWriteInput, 1 ), Host( b, kReadItems, 1 ) };
	bad[1].run = nullptr;
	CHECK( !graph.Run( bad, 2, nullptr, FRAME_GRAPH_SERIAL ).valid );
	bad[1] = Host( b, kReadItems, 1 );
	bad[0].name = "";
	CHECK( !graph.Run( bad, 2, nullptr, FRAME_GRAPH_SERIAL ).valid );
	bad[0] = Host( a, nullptr, 1 ); // access count without access
	CHECK( !graph.Run( bad, 2, nullptr, FRAME_GRAPH_SERIAL ).valid );
	BatchNode batch;
	batch.recorder = &recorder;
	FrameNodeDesc badBatch = Batch( "x", batch, kBatchAccess, 2 );
	badBatch.count = nullptr;
	CHECK( !graph.Run( &badBatch, 1, nullptr, FRAME_GRAPH_SERIAL ).valid );
	CHECK( !graph.Run( nodes, 2, nullptr, (FrameGraphMode)7 ).valid );
	CHECK( !graph.Run( nullptr, 1, nullptr, FRAME_GRAPH_SERIAL ).valid );
	CHECK( recorder.events.empty() );
	CHECK( graph.Run( nullptr, 0, nullptr, FRAME_GRAPH_SERIAL ).valid );
}

int main()
{
	std::printf( "declaredframetest (RFC 0003 declared frame graph)\n" );
	RUN( Test_SerialIsArrayOrder );
	RUN( Test_AllDomainOrdersEverything );
	RUN( Test_PooledOverlapsUnorderedNodes );
	RUN( Test_PooledPublishesToConflictingNodes );
	RUN( Test_EmptyBatchRunsNoHooks );
	RUN( Test_NestedBatchRunsInline );
	RUN( Test_InvalidInputAndReuse );
	std::printf( "\n%d checks, %d failures\n", g_checks, g_failures );
	return testing::ReportConformance( g_checks, g_failures );
}
