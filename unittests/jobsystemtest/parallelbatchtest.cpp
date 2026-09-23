//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Independent item, lifetime, and nesting checks for synchronous job
//          batches. The fake backend supplies real threads and a join boundary;
//          no engine globals, content, or display are required.
//
//=============================================================================//

#include "jobsystem/parallel_batch.h"
#include "jobsystem/worker_backend.h"
#include "testing/conformance_result.h"

#include <algorithm>
#include <atomic>
#include <barrier>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <mutex>
#include <thread>
#include <vector>

namespace
{

int g_checks = 0;
int g_failures = 0;

void Check( bool condition, const char *expression, int line )
{
	++g_checks;
	if ( !condition )
	{
		++g_failures;
		std::printf( "FAIL parallelbatchtest.cpp:%d: %s\n", line, expression );
	}
}

#define CHECK( condition ) Check( ( condition ), #condition, __LINE__ )

// The backend refuses recursive dispatch on one of its own participants. A
// broken nested-batch adapter fails its item oracle instead of hanging the suite.
thread_local bool g_inBackend = false;

class ThreadBackend final : public jobsystem::IWorkerBackend
{
public:
	explicit ThreadBackend( int workers ) : m_workers( workers ) {}

	void ParallelFor( int count, const std::function<void( int )> &body ) override
	{
		++calls;
		if ( g_inBackend )
		{
			++recursiveCalls;
			return;
		}
		largestDispatch = std::max( largestDispatch, count );
		std::vector<std::thread> threads;
		auto invoke = [&]( int index )
		{
			g_inBackend = true;
			body( index );
			g_inBackend = false;
		};
		for ( int index = 1; index < count; ++index )
			threads.emplace_back( invoke, index );
		if ( count > 0 )
			invoke( 0 );
		for ( auto &thread : threads )
			thread.join();
	}

	int WorkerCount() const override { return m_workers; }

	std::atomic<unsigned> calls{ 0 };
	std::atomic<unsigned> recursiveCalls{ 0 };
	int largestDispatch = 0;

private:
	int m_workers;
};

thread_local std::vector<const void *> g_hookOwners;

struct Capture
{
	explicit Capture( unsigned count ) : inputs( count ), outputs( count ), visits( count )
	{
		for ( unsigned i = 0; i < count; ++i )
			inputs[i] = static_cast<int>( i % 31 ) - 15;
	}

	std::vector<int> inputs;
	std::vector<int> outputs;
	std::vector<std::atomic<unsigned>> visits;
	std::atomic<unsigned> entered{ 0 };
	std::atomic<unsigned> exited{ 0 };
	std::atomic<unsigned> active{ 0 };
	std::atomic<unsigned> peak{ 0 };
	std::atomic<unsigned> violations{ 0 };
	std::barrier<> *start = nullptr;
	void *extension = nullptr;
	bool requireHooks = true;
};

void Enter( void *opaque )
{
	auto &capture = *static_cast<Capture *>( opaque );
	g_hookOwners.push_back( opaque );
	++capture.entered;
	unsigned active = ++capture.active;
	unsigned peak = capture.peak.load();
	while ( peak < active && !capture.peak.compare_exchange_weak( peak, active ) )
	{
	}
	if ( capture.start )
		capture.start->arrive_and_wait();
}

void Leave( void *opaque )
{
	auto &capture = *static_cast<Capture *>( opaque );
	if ( g_hookOwners.empty() || g_hookOwners.back() != opaque )
		++capture.violations;
	else
		g_hookOwners.pop_back();
	--capture.active;
	++capture.exited;
}

void Process( void *opaque, unsigned index )
{
	auto &capture = *static_cast<Capture *>( opaque );
	if ( index >= capture.inputs.size() )
	{
		++capture.violations;
		return;
	}
	if ( capture.requireHooks && ( g_hookOwners.empty() || g_hookOwners.back() != opaque ) )
		++capture.violations;
	if ( capture.visits[index].fetch_add( 1 ) != 0 )
	{
		++capture.violations;
		return;
	}
	// Deliberately independent of the graph's participant partition/cursor.
	int value = capture.inputs[index];
	capture.outputs[index] = value * ( value + 3 ) + static_cast<int>( index );
	if ( index % 7 == 0 )
		std::this_thread::yield();
}

jobsystem::BatchDesc Describe( Capture &capture, unsigned participants )
{
	jobsystem::BatchDesc desc;
	desc.name = "batch.contract";
	desc.context = &capture;
	desc.count = static_cast<unsigned>( capture.inputs.size() );
	desc.process = Process;
	desc.begin = Enter;
	desc.end = Leave;
	desc.maxParticipants = participants;
	return desc;
}

bool MatchesIndependentOracle( const Capture &capture )
{
	for ( unsigned i = 0; i < capture.inputs.size(); ++i )
	{
		int input = capture.inputs[i];
		int expected = input * input + 3 * input + static_cast<int>( i );
		if ( capture.visits[i] != 1 || capture.outputs[i] != expected )
			return false;
	}
	return capture.violations == 0 && capture.active == 0 && capture.entered == capture.exited;
}

void TestInvalidAndEmpty()
{
	ThreadBackend backend( 3 );
	Capture capture( 8 );
	auto valid = Describe( capture, 4 );
	for ( unsigned invalid = 0; invalid < 5; ++invalid )
	{
		auto desc = valid;
		auto mode = jobsystem::BatchMode::Parallel;
		switch ( invalid )
		{
		case 0:
			desc.name = nullptr;
			break;
		case 1:
			desc.name = "";
			break;
		case 2:
			desc.process = nullptr;
			break;
		case 3:
			desc.maxParticipants = 0;
			break;
		case 4:
			mode = static_cast<jobsystem::BatchMode>( 99 );
			break;
		}
		CHECK( !jobsystem::ExecuteParallelBatch( desc, &backend, mode ) );
		CHECK( capture.entered == 0 && capture.exited == 0 && backend.calls == 0 );
		for ( const auto &visits : capture.visits )
			CHECK( visits == 0 );
	}
	auto empty = valid;
	empty.count = 0;
	empty.process = nullptr;
	empty.maxParticipants = 0;
	CHECK( jobsystem::ExecuteParallelBatch( empty, &backend, jobsystem::BatchMode::Parallel ) );
	CHECK( capture.entered == 0 && capture.exited == 0 && backend.calls == 0 );
	CHECK( !jobsystem::ExecuteParallelBatch(
	    empty, &backend, static_cast<jobsystem::BatchMode>( 99 ) ) );
	empty.name = nullptr;
	CHECK( !jobsystem::ExecuteParallelBatch( empty, &backend, jobsystem::BatchMode::Serial ) );
}

void TestCountsModesAndBounds()
{
	for ( int workers : { 0, 1, 3 } )
	{
		for ( unsigned count : { 1u, 2u, 7u, 113u, 1024u } )
		{
			for ( unsigned limit : { 1u, 2u, 4u, 31u } )
			{
				for ( auto mode : { jobsystem::BatchMode::Serial, jobsystem::BatchMode::Parallel } )
				{
					ThreadBackend backend( workers );
					Capture capture( count );
					auto desc = Describe( capture, limit );
					CHECK( jobsystem::ExecuteParallelBatch( desc, &backend, mode ) );
					CHECK( MatchesIndependentOracle( capture ) );
					unsigned bound =
					    mode == jobsystem::BatchMode::Serial
					        ? 1u
					        : std::min( { count, limit, static_cast<unsigned>( workers + 1 ) } );
					CHECK( capture.entered > 0 && capture.entered <= bound );
					CHECK( capture.peak <= bound );
					CHECK( backend.largestDispatch <= static_cast<int>( bound ) );
					CHECK( g_hookOwners.empty() );
					if ( mode == jobsystem::BatchMode::Serial )
						CHECK( backend.calls == 0 );
				}
			}
		}
	}
	Capture capture( 103 );
	CHECK( jobsystem::ExecuteParallelBatch(
	    Describe( capture, 5 ), nullptr, jobsystem::BatchMode::Parallel ) );
	CHECK( MatchesIndependentOracle( capture ) );
	CHECK( capture.entered == 1 );
}

void TestOracleSensitivity()
{
	Capture capture( 9 );
	CHECK( jobsystem::ExecuteParallelBatch(
	    Describe( capture, 1 ), nullptr, jobsystem::BatchMode::Serial ) );
	CHECK( MatchesIndependentOracle( capture ) );
	capture.visits[5] = 0;
	CHECK( !MatchesIndependentOracle( capture ) );
	capture.visits[5] = 2;
	CHECK( !MatchesIndependentOracle( capture ) );
	capture.visits[5] = 1;
	++capture.outputs[3];
	CHECK( !MatchesIndependentOracle( capture ) );
	--capture.outputs[3];
	++capture.exited;
	CHECK( !MatchesIndependentOracle( capture ) );
}

void TestOptionalHooksAndContext()
{
	Capture capture( 17 );
	capture.requireHooks = false;
	auto desc = Describe( capture, 3 );
	desc.begin = nullptr;
	desc.end = nullptr;
	ThreadBackend backend( 2 );
	CHECK( jobsystem::ExecuteParallelBatch( desc, &backend, jobsystem::BatchMode::Parallel ) );
	CHECK( MatchesIndependentOracle( capture ) );
	static std::atomic<unsigned> nullCalls{ 0 };
	desc.context = nullptr;
	desc.process = []( void *opaque, unsigned )
	{
		if ( !opaque )
			++nullCalls;
	};
	CHECK( jobsystem::ExecuteParallelBatch( desc, &backend, jobsystem::BatchMode::Parallel ) );
	CHECK( nullCalls == desc.count );
	static std::atomic<unsigned> hookCalls{ 0 };
	desc.begin = []( void * )
	{
		++hookCalls;
	};
	CHECK( jobsystem::ExecuteParallelBatch( desc, nullptr, jobsystem::BatchMode::Serial ) );
	CHECK( hookCalls == 1 );
	desc.begin = nullptr;
	desc.end = []( void * )
	{
		++hookCalls;
	};
	CHECK( jobsystem::ExecuteParallelBatch( desc, nullptr, jobsystem::BatchMode::Serial ) );
	CHECK( hookCalls == 2 );
}

struct NestedCapture
{
	NestedCapture( unsigned count, ThreadBackend &pool ) : capture( count ), backend( pool )
	{
		capture.extension = this;
	}
	Capture capture;
	ThreadBackend &backend;
	std::atomic<unsigned> childFailures{ 0 };
};

void RunNestedChild( void *opaque )
{
	auto &capture = *static_cast<Capture *>( opaque );
	auto &outer = *static_cast<NestedCapture *>( capture.extension );
	Capture child( 19 );
	if ( !jobsystem::ExecuteParallelBatch(
	         Describe( child, 4 ), &outer.backend, jobsystem::BatchMode::Parallel ) ||
	     !MatchesIndependentOracle( child ) || child.entered != 1 )
		++outer.childFailures;
}

void NestedProcess( void *opaque, unsigned index )
{
	RunNestedChild( opaque );
	Process( opaque, index );
}

void NestedEnter( void *opaque )
{
	Enter( opaque );
	RunNestedChild( opaque );
}

void NestedLeave( void *opaque )
{
	RunNestedChild( opaque );
	Leave( opaque );
}

void TestSaturatedNestedCalls()
{
	ThreadBackend backend( 3 );
	NestedCapture outer( 32, backend );
	std::barrier start( 4 );
	outer.capture.start = &start;
	auto desc = Describe( outer.capture, 4 );
	desc.process = NestedProcess;
	desc.begin = NestedEnter;
	desc.end = NestedLeave;
	CHECK( jobsystem::ExecuteParallelBatch( desc, &backend, jobsystem::BatchMode::Parallel ) );
	CHECK( MatchesIndependentOracle( outer.capture ) );
	CHECK( outer.capture.entered == 4 && outer.capture.peak == 4 );
	CHECK( outer.childFailures == 0 );
	CHECK( backend.recursiveCalls == 0 );
}

struct DelayedCapture
{
	DelayedCapture() : capture( 8 ) { capture.extension = this; }
	Capture capture;
	std::mutex mutex;
	std::condition_variable changed;
	bool started = false;
	bool release = false;
};

void DelayedProcess( void *opaque, unsigned index )
{
	auto &batch = *static_cast<Capture *>( opaque );
	auto &capture = *static_cast<DelayedCapture *>( batch.extension );
	if ( index == 0 )
	{
		std::unique_lock<std::mutex> lock( capture.mutex );
		capture.started = true;
		capture.changed.notify_all();
		capture.changed.wait( lock,
		    [&]
		    {
			    return capture.release;
		    } );
	}
	Process( opaque, index );
}

void TestCompletionAndLifetime()
{
	ThreadBackend backend( 3 );
	DelayedCapture capture;
	auto desc = Describe( capture.capture, 4 );
	desc.process = DelayedProcess;
	std::atomic<bool> returned{ false };
	bool success = false;
	std::thread caller(
	    [&]
	    {
		    success =
		        jobsystem::ExecuteParallelBatch( desc, &backend, jobsystem::BatchMode::Parallel );
		    returned = true;
	    } );
	{
		std::unique_lock<std::mutex> lock( capture.mutex );
		bool started = capture.changed.wait_for( lock, std::chrono::seconds( 5 ),
		    [&]
		    {
			    return capture.started;
		    } );
		CHECK( started );
		CHECK( !returned );
		CHECK( capture.capture.active > 0 );
		capture.release = true;
	}
	capture.changed.notify_all();
	caller.join();
	CHECK( success && returned );
	CHECK( MatchesIndependentOracle( capture.capture ) );
	CHECK( capture.capture.entered == capture.capture.exited && capture.capture.active == 0 );
}

void TestSerialOrderAndReuse()
{
	std::vector<unsigned> observed;
	jobsystem::BatchDesc desc;
	desc.name = "serial.order";
	desc.context = &observed;
	desc.count = 37;
	desc.maxParticipants = 19;
	desc.process = []( void *opaque, unsigned index )
	{
		static_cast<std::vector<unsigned> *>( opaque )->push_back( index );
	};
	ThreadBackend backend( 3 );
	for ( unsigned repetition = 0; repetition < 5; ++repetition )
	{
		observed.clear();
		CHECK( jobsystem::ExecuteParallelBatch( desc, &backend, jobsystem::BatchMode::Serial ) );
		CHECK( observed.size() == desc.count );
		for ( unsigned index = 0; index < observed.size(); ++index )
			CHECK( observed[index] == index );
	}
	CHECK( backend.calls == 0 );
}

} // namespace

int main()
{
	TestInvalidAndEmpty();
	TestCountsModesAndBounds();
	TestOracleSensitivity();
	TestOptionalHooksAndContext();
	TestSaturatedNestedCalls();
	TestCompletionAndLifetime();
	TestSerialOrderAndReuse();
	std::printf( "%d checks, %d failures\n", g_checks, g_failures );
	return testing::ReportConformance( g_checks, g_failures );
}
