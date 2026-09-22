//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Frozen C++11 consumer of the C++20 batch implementation through the
//          real vstdlib worker pool, plus opt-in comparative batch timings.
//
//=============================================================================//

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <thread>
#include <vector>

#include "vstdlib/jobgraph_parallel.h"

static int s_checks = 0;
static int s_failures = 0;
static thread_local int s_hookDepth = 0;

static void Check( bool value, const char *expression, int line )
{
	++s_checks;
	if ( !value )
	{
		++s_failures;
		std::printf( "FAIL line %d: %s\n", line, expression );
	}
}
#define CHECK( expression ) Check( ( expression ), #expression, __LINE__ )

struct Item
{
	unsigned input;
	unsigned output;
	unsigned count;
	unsigned steps;
};

static unsigned Reference( unsigned value, unsigned steps )
{
	for ( unsigned i = 0; i < steps; ++i )
		value = value * 1664525u + 1013904223u;
	return value;
}

struct Work
{
	std::atomic<unsigned> errors{ 0 };
	std::atomic<unsigned> enters{ 0 };
	std::atomic<unsigned> exits{ 0 };

	void Begin()
	{
		if ( s_hookDepth++ != 0 )
			++errors;
		++enters;
	}
	void Process( Item &item )
	{
		if ( s_hookDepth != 1 )
			++errors;
		item.output = Reference( item.input, item.steps );
		++item.count;
	}
	void End()
	{
		if ( --s_hookDepth != 0 )
			++errors;
		++exits;
	}
};

static void PlainProcess( Item &item )
{
	item.output = Reference( item.input, item.steps );
	++item.count;
}

static std::vector<Item> Inputs( unsigned count, unsigned steps = 64 )
{
	std::vector<Item> items;
	for ( unsigned i = 0; i < count; ++i )
		items.push_back( Item{ i * 71 + 11, 0, 0, steps } );
	return items;
}

static void VerifyItems( const std::vector<Item> &items )
{
	for ( const Item &item : items )
	{
		CHECK( item.count == 1 );
		CHECK( item.output == Reference( item.input, item.steps ) );
	}
}

static void TestModes( IThreadPool *pool, bool graphsOnly )
{
	for ( unsigned count : { 0u, 1u, 2u, 17u, 1024u } )
	{
		for ( int mode = graphsOnly ? 1 : 0; mode < 3; ++mode )
		{
			std::vector<Item> items = Inputs( count );
			Work work;
			if ( mode == 0 )
			{
				CParallelProcessor<Item, CMemberFuncJobItemProcessor<Item, Work>> legacy(
				    "legacy" );
				legacy.m_ItemProcessor.Init( &work, &Work::Process, &Work::Begin, &Work::End );
				legacy.Run( items.data(), count, INT_MAX, pool );
			}
			else
			{
				CHECK( JobGraphParallelProcess( "graph", items.data(), count, &work, &Work::Process,
				    &Work::Begin, &Work::End, INT_MAX, pool,
				    mode == 1 ? jobsystem::BatchMode::Serial : jobsystem::BatchMode::Parallel ) );
			}
			VerifyItems( items );
			CHECK( work.errors == 0 );
			CHECK( work.enters == work.exits );
			CHECK( count == 0 ? work.enters == 0 : work.enters > 0 );
		}
	}
	std::vector<Item> items = Inputs( 37 );
	CHECK( JobGraphParallelProcess(
	    "free function", items.data(), items.size(), &PlainProcess, NULL, NULL, 0, pool ) );
	VerifyItems( items );
	CHECK( !JobGraphParallelProcess( "invalid", static_cast<Item *>( NULL ), 1, &PlainProcess ) );
	CHECK( !JobGraphParallelProcess(
	    "invalid", items.data(), 1, static_cast<void ( * )( Item & )>( NULL ) ) );
}

static void SetFlag( bool *flag )
{
	*flag = true;
}

static void TestNoQueueHelping( IThreadPool *pool )
{
	pool->SuspendExecution();
	bool unrelated = false;
	CJob *job = pool->QueueCall( &SetFlag, &unrelated );
	std::vector<Item> items = Inputs( 61 );
	CHECK( JobGraphParallelProcess(
	    "suspended pool", items.data(), items.size(), &PlainProcess, NULL, NULL, INT_MAX, pool ) );
	CHECK( !unrelated );
	VerifyItems( items );
	pool->ResumeExecution();
	job->Execute();
	CHECK( unrelated );
	job->Release();
}

struct NestedWork
{
	IThreadPool *pool;
	std::atomic<unsigned> errors{ 0 };
	void Process( Item &item )
	{
		std::vector<Item> children = Inputs( 9 );
		if ( !JobGraphParallelProcess( "child", children.data(), children.size(), &PlainProcess,
		         NULL, NULL, INT_MAX, pool ) )
			++errors;
		for ( const Item &child : children )
			if ( child.count != 1 || child.output != Reference( child.input, child.steps ) )
				++errors;
		PlainProcess( item );
	}
};

static void TestNestedAndConcurrent( IThreadPool *pool )
{
	NestedWork work;
	work.pool = pool;
	std::vector<Item> first = Inputs( 123 ), second = Inputs( 123 );
	auto run = [&]( std::vector<Item> &items )
	{
		if ( !JobGraphParallelProcess( "parent", items.data(), items.size(), &work,
		         &NestedWork::Process, static_cast<void ( NestedWork::* )()>( NULL ),
		         static_cast<void ( NestedWork::* )()>( NULL ), INT_MAX, pool ) )
			++work.errors;
	};
	std::thread other(
	    [&]
	    {
		    run( second );
	    } );
	run( first );
	other.join();
	CHECK( work.errors == 0 );
	VerifyItems( first );
	VerifyItems( second );
}

struct WorkerProbe
{
	std::thread::id caller = std::this_thread::get_id();
	std::atomic<bool> observed{ false };
	void Process( Item &item )
	{
		if ( std::this_thread::get_id() != caller )
			observed.store( true );
		else
		{
			const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds( 1 );
			while ( !observed.load() && std::chrono::steady_clock::now() < deadline )
				std::this_thread::yield();
		}
		PlainProcess( item );
	}
};

static void TestOneWorkerParticipation( IThreadPool *pool )
{
	WorkerProbe probe;
	std::vector<Item> items = Inputs( 2 );
	CHECK( JobGraphParallelProcess( "one worker", items.data(), items.size(), &probe,
	    &WorkerProbe::Process, static_cast<void ( WorkerProbe::* )()>( NULL ),
	    static_cast<void ( WorkerProbe::* )()>( NULL ), INT_MAX, pool ) );
	CHECK( probe.observed.load() );
	VerifyItems( items );
}

static void Benchmark( IThreadPool *pool )
{
	std::printf( "BENCH units=microseconds sample_count=101 workers=%d\n", pool->NumThreads() );
	for ( unsigned count : { 1u, 8u, 128u, 2048u } )
	{
		for ( unsigned steps : { 0u, 2048u } )
		{
			std::vector<double> timings[3];
			std::vector<Item> items = Inputs( count, steps );
			for ( int repeat = 0; repeat < 111; ++repeat )
			{
				for ( int mode = 0; mode < 3; ++mode )
				{
					for ( Item &item : items )
						item.count = 0;
					const auto begin = std::chrono::steady_clock::now();
					if ( mode == 0 )
					{
						CParallelProcessor<Item, CFuncJobItemProcessor<Item>> legacy( "benchmark" );
						legacy.m_ItemProcessor.Init( &PlainProcess );
						legacy.Run( items.data(), count, INT_MAX, pool );
					}
					else
					{
						JobGraphParallelProcess( "benchmark", items.data(), count, &PlainProcess,
						    NULL, NULL, INT_MAX, pool,
						    mode == 1 ? jobsystem::BatchMode::Serial
						              : jobsystem::BatchMode::Parallel );
					}
					const auto end = std::chrono::steady_clock::now();
					if ( repeat >= 10 )
						timings[mode].push_back(
						    std::chrono::duration<double, std::micro>( end - begin ).count() );
					if ( repeat == 110 )
						VerifyItems( items );
				}
			}
			for ( int mode = 0; mode < 3; ++mode )
			{
				std::sort( timings[mode].begin(), timings[mode].end() );
				std::printf( "BENCH count=%u steps=%u mode=%s median=%.3f p95=%.3f\n", count, steps,
				    mode == 0   ? "legacy"
				    : mode == 1 ? "serial_graph"
				                : "parallel_graph",
				    timings[mode][50], timings[mode][95] );
			}
		}
	}
}

int main( int argc, char **argv )
{
	bool benchmark = false, graphsOnly = false;
	for ( int arg = 1; arg < argc; ++arg )
	{
		if ( std::strcmp( argv[arg], "--benchmark" ) == 0 )
			benchmark = true;
		else if ( std::strcmp( argv[arg], "--graphs-only" ) == 0 )
			graphsOnly = true;
		else
			return 2;
	}
	if ( benchmark && graphsOnly )
		return 2;
	for ( int workers : { 0, 1, 3 } )
	{
		IThreadPool *pool = CreateThreadPool();
		ThreadPoolStartParams_t params;
		params.nThreads = workers;
		CHECK( pool->Start( params ) );
		CHECK( pool->NumThreads() == workers );
		TestModes( pool, graphsOnly );
		if ( workers )
			TestNoQueueHelping( pool );
		TestNestedAndConcurrent( pool );
		if ( workers == 1 )
			TestOneWorkerParticipation( pool );
		if ( benchmark )
			Benchmark( pool );
		CHECK( pool->Stop() );
		DestroyThreadPool( pool );
	}
	std::printf( "%d checks, %d failures\n", s_checks, s_failures );
	return s_failures ? 1 : 0;
}
