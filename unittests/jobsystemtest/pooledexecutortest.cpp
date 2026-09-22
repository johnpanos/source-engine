//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: PooledExecutor lane, cancellation, and publication conformance.
//
//=============================================================================//

#include "jobsystem/pooled_executor.h"
#include "jobsystem/parallel_executor.h"

#include <atomic>
#include <cstdio>
#include <functional>
#include <mutex>
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
			std::printf( "  FAIL [%s] %s:%d: %s\n", g_test, __FILE__, __LINE__, #condition );      \
		}                                                                                          \
	} while ( 0 )

#define RUN( function )                                                                            \
	do                                                                                             \
	{                                                                                              \
		g_test = #function;                                                                        \
		std::printf( "- %s\n", g_test );                                                           \
		function();                                                                                \
	} while ( 0 )

// Deliberately never helps on the caller: accidental affine/blocking dispatch
// is observable as the wrong thread. Joins provide the IWorkerBackend barrier.
class ThreadBackend final : public IWorkerBackend
{
public:
	explicit ThreadBackend( int workers ) : m_workers( workers ) {}

	int WorkerCount() const override { return m_workers; }

	void ParallelFor( int count, const std::function<void( int )> &body ) override
	{
		++batches;
		dispatched += count;
		if ( beforeDispatch )
			beforeDispatch();
		std::vector<std::thread> threads;
		for ( int worker = 0; worker < m_workers; ++worker )
		{
			threads.emplace_back(
			    [&, worker]
			    {
				    for ( int index = worker; index < count; index += m_workers )
					    body( index );
			    } );
		}
		for ( std::thread &thread : threads )
			thread.join();
	}

	int batches = 0;
	int dispatched = 0;
	std::function<void()> beforeDispatch;

private:
	int m_workers;
};

class TraceSink final : public ITraceSink
{
public:
	explicit TraceSink( uint32_t count ) : states( count ), threads( count ) {}

	void OnJobState( uint32_t id, const char *, ExecutorToken, JobState state ) override
	{
		std::lock_guard<std::mutex> lock( m_mutex );
		states[id].push_back( state );
		if ( state == JobState::Running )
			threads[id] = std::this_thread::get_id();
	}

	std::vector<std::vector<JobState>> states;
	std::vector<std::thread::id> threads;

private:
	std::mutex m_mutex;
};

static JobHandle AddJob( JobGraphBuilder &builder, ExecutorToken executor, JobEntry function = {} )
{
	JobDesc desc;
	desc.name = "pooled-contract";
	desc.executor = executor;
	desc.function = std::move( function );
	return builder.AddJob( desc );
}

static SealedGraph Seal( JobGraphBuilder &builder )
{
	auto graph = builder.Seal();
	CHECK( graph.HasValue() );
	return graph.HasValue() ? std::move( graph.Value() ) : SealedGraph{};
}

static void Test_LanesAndOutputPublication()
{
	JobGraphBuilder builder;
	int computeValue = 0, mainValue = 0, blockingValue = 0, sequenceValue = 0;
	JobHandle compute = AddJob( builder, Executor::Compute(),
	    [&]( JobRunContext & )
	    {
		    computeValue = 37;
	    } );
	JobHandle main = AddJob( builder, Executor::MainThread(),
	    [&]( JobRunContext & )
	    {
		    mainValue = computeValue + 1;
	    } );
	JobHandle blocking = AddJob( builder, Executor::BlockingIO(),
	    [&]( JobRunContext & )
	    {
		    blockingValue = computeValue + 2;
	    } );
	JobHandle first = AddJob( builder, Executor::Sequence( 1 ),
	    [&]( JobRunContext & )
	    {
		    sequenceValue = 41;
	    } );
	JobHandle second = AddJob( builder, Executor::Sequence( 1 ),
	    [&]( JobRunContext & )
	    {
		    sequenceValue += mainValue + blockingValue;
	    } );
	builder.AddDependency( compute, main );
	builder.AddDependency( compute, blocking );
	builder.AddDependency( main, second );
	builder.AddDependency( blocking, second );
	// Empty affine jobs still obey their executor and emit a complete lifecycle.
	JobHandle emptyMain = AddJob( builder, Executor::MainThread() );
	JobHandle emptyBlocking = AddJob( builder, Executor::BlockingIO() );
	SealedGraph graph = Seal( builder );
	TraceSink trace( graph.JobCount() );
	RunOptions options;
	options.trace = &trace;
	ThreadBackend backend( 3 );
	RunResult result = PooledExecutor( &backend ).Execute( graph, options );
	CHECK( result.AllSucceeded() );
	CHECK( result.succeeded == 7 && result.executed == 5 );
	CHECK( result.unresolved == 0 );
	CHECK( backend.dispatched == 3 );
	CHECK( mainValue == 38 && blockingValue == 39 && sequenceValue == 118 );
	const std::thread::id caller = std::this_thread::get_id();
	for ( JobHandle handle : { main, blocking, emptyMain, emptyBlocking } )
		CHECK( trace.threads[handle.id] == caller );
	for ( JobHandle handle : { compute, first, second } )
		CHECK( trace.threads[handle.id] != caller );
	for ( const auto &states : trace.states )
		CHECK( states == std::vector<JobState>(
		                     { JobState::Ready, JobState::Running, JobState::Succeeded } ) );
}

static void Test_UnpumpedLanesStallEveryDependent()
{
	JobGraphBuilder builder;
	std::atomic<int> forbiddenRuns{ 0 };
	auto forbidden = [&]( JobRunContext & )
	{
		forbiddenRuns.fetch_add( 1 );
	};
	JobHandle main = AddJob( builder, Executor::MainThread(), forbidden );
	JobHandle blocking = AddJob( builder, Executor::BlockingIO(), forbidden );
	JobHandle success = AddJob( builder, Executor::Compute(), forbidden );
	JobHandle cleanup = AddJob( builder, Executor::Compute(), forbidden );
	JobHandle join = AddJob( builder, Executor::Compute(), forbidden );
	JobHandle descendant = AddJob( builder, Executor::Sequence( 2 ), forbidden );
	JobHandle independent = AddJob( builder, Executor::Compute() );
	JobHandle independentEnd = AddJob( builder, Executor::Sequence( 3 ) );
	builder.AddDependency( main, success, DependencyKind::Success );
	builder.AddDependency( main, cleanup, DependencyKind::Terminal );
	builder.AddDependency( blocking, join, DependencyKind::Terminal );
	builder.AddDependency( independent, join );
	builder.AddDependency( cleanup, descendant );
	builder.AddDependency( join, descendant );
	builder.AddDependency( independent, independentEnd );
	SealedGraph graph = Seal( builder );
	TraceSink trace( graph.JobCount() );
	RunOptions options;
	options.pumpMainThread = false;
	options.trace = &trace;
	ThreadBackend backend( 2 );
	RunResult result = PooledExecutor( &backend ).Execute( graph, options );
	CHECK( result.stalled && !result.AllSucceeded() );
	CHECK( result.unresolved == 6 && result.succeeded == 2 );
	CHECK( result.executed == 0 && result.canceled == 0 && result.failed == 0 );
	CHECK( forbiddenRuns.load() == 0 );
	for ( JobHandle handle : { main, blocking, success, cleanup, join, descendant } )
	{
		CHECK( result.states[handle.id] == JobState::Admitted );
		CHECK( trace.states[handle.id].empty() );
	}
	CHECK( result.states[independent.id] == JobState::Succeeded );
	CHECK( result.states[independentEnd.id] == JobState::Succeeded );
	CHECK( backend.batches == 0 ); // Empty compute nodes require no worker dispatch.
	// The independently implemented executor is the contract oracle for stalls.
	options.trace = nullptr;
	RunResult parallel = ParallelExecutor( 2 ).Execute( graph, options );
	CHECK( parallel.states == result.states );
	CHECK( parallel.unresolved == result.unresolved );
	CHECK( forbiddenRuns.load() == 0 );
}

static void Test_InlineModesServiceEveryLane()
{
	ThreadBackend zeroWorkers( 0 );
	IWorkerBackend *backends[] = { nullptr, &zeroWorkers };
	for ( IWorkerBackend *backend : backends )
	{
		JobGraphBuilder builder;
		std::vector<int> order;
		std::vector<std::thread::id> threads( 5 );
		const ExecutorToken lanes[] = { Executor::Compute(), Executor::MainThread(),
		    Executor::BlockingIO(), Executor::Sequence( 0 ), Executor::Sequence( 0 ) };
		for ( int index = 0; index < 5; ++index )
		{
			AddJob( builder, lanes[index],
			    [&, index]( JobRunContext & )
			    {
				    order.push_back( index );
				    threads[index] = std::this_thread::get_id();
			    } );
		}
		SealedGraph graph = Seal( builder );
		RunOptions options;
		options.pumpMainThread = false;
		RunResult result = PooledExecutor( backend ).Execute( graph, options );
		CHECK( result.AllSucceeded() && result.succeeded == 5 && result.executed == 5 );
		CHECK( order == std::vector<int>( { 0, 1, 2, 3, 4 } ) );
		for ( std::thread::id thread : threads )
			CHECK( thread == std::this_thread::get_id() );
	}
	CHECK( zeroWorkers.batches == 0 );
}

static void Test_CancellationBetweenWaveAdmissionAndStart()
{
	JobGraphBuilder builder;
	std::atomic<bool> canceled{ false };
	std::atomic<int> runs{ 0 };
	for ( ExecutorToken lane : { Executor::Compute(), Executor::MainThread(),
	          Executor::BlockingIO(), Executor::Sequence( 0 ) } )
	{
		AddJob( builder, lane,
		    [&]( JobRunContext & )
		    {
			    runs.fetch_add( 1 );
		    } );
	}
	SealedGraph graph = Seal( builder );
	ThreadBackend backend( 2 );
	backend.beforeDispatch = [&]
	{
		canceled.store( true, std::memory_order_release );
	};
	TraceSink trace( graph.JobCount() );
	RunOptions options;
	options.cancel = &canceled;
	options.trace = &trace;
	RunResult result = PooledExecutor( &backend ).Execute( graph, options );
	CHECK( result.canceled == 4 && result.executed == 0 );
	CHECK( result.succeeded == 0 && result.failed == 0 && !result.stalled );
	CHECK( runs.load() == 0 );
	for ( const auto &states : trace.states )
		CHECK( states == std::vector<JobState>( { JobState::Ready, JobState::Canceled } ) );
	// Cancellation already present at admission must not dispatch to the backend.
	const int batches = backend.batches;
	result = PooledExecutor( &backend ).Execute( graph, options );
	CHECK( result.canceled == 4 && result.executed == 0 );
	CHECK( backend.batches == batches );
}

static void Test_RunningJobFinishesWhilePendingJobsCancel()
{
	// One worker gives a deterministic pause between queued invocations. The
	// first job publishes cancellation while its own callback remains active.
	ThreadBackend worker( 1 );
	IWorkerBackend *backends[] = { nullptr, &worker };
	for ( IWorkerBackend *backend : backends )
	{
		JobGraphBuilder builder;
		std::atomic<bool> canceled{ false };
		int completed = 0;
		JobHandle running = AddJob( builder, Executor::Compute(),
		    [&]( JobRunContext & )
		    {
			    canceled.store( true, std::memory_order_release );
			    completed = 1;
		    } );
		std::atomic<int> forbiddenRuns{ 0 };
		auto forbidden = [&]( JobRunContext & )
		{
			forbiddenRuns.fetch_add( 1 );
		};
		JobHandle queued = AddJob( builder, Executor::Compute(), forbidden );
		JobHandle caller = AddJob( builder, Executor::MainThread(), forbidden );
		JobHandle dependent = AddJob( builder, Executor::Compute(), forbidden );
		JobHandle cleanup = AddJob( builder, Executor::BlockingIO(), forbidden );
		builder.AddDependency( queued, dependent );
		builder.AddDependency( running, cleanup, DependencyKind::Terminal );
		SealedGraph graph = Seal( builder );
		RunOptions options;
		options.cancel = &canceled;
		RunResult result = PooledExecutor( backend ).Execute( graph, options );
		CHECK( result.succeeded == 1 && result.executed == 1 && result.canceled == 4 );
		CHECK( result.failed == 0 && !result.stalled );
		CHECK( completed == 1 && forbiddenRuns.load() == 0 );
		CHECK( result.states[running.id] == JobState::Succeeded );
		for ( JobHandle handle : { queued, caller, dependent, cleanup } )
			CHECK( result.states[handle.id] == JobState::Canceled );
	}
}

static void Test_FailureCleanupAndRepeatedRuns()
{
	JobGraphBuilder builder;
	int cleanupRuns = 0;
	std::atomic<int> forbiddenRuns{ 0 };
	JobHandle producer = AddJob( builder, Executor::Compute(),
	    []( JobRunContext &context )
	    {
		    context.Fail();
	    } );
	JobHandle dependent = AddJob( builder, Executor::MainThread(),
	    [&]( JobRunContext & )
	    {
		    forbiddenRuns.fetch_add( 1 );
	    } );
	JobHandle cleanup = AddJob( builder, Executor::BlockingIO(),
	    [&]( JobRunContext & )
	    {
		    ++cleanupRuns;
	    } );
	builder.AddDependency( producer, dependent );
	builder.AddDependency( producer, cleanup, DependencyKind::Terminal );
	SealedGraph graph = Seal( builder );
	RunResult reference = DeterministicExecutor().Execute( graph, RunOptions{} );
	cleanupRuns = 0;
	ThreadBackend backend( 3 );
	PooledExecutor executor( &backend );
	for ( int repetition = 0; repetition < 20; ++repetition )
	{
		RunResult result = executor.Execute( graph, RunOptions{} );
		CHECK( result.states == reference.states );
		CHECK( result.executed == 2 && result.failed == 1 && result.canceled == 1 );
		CHECK( result.succeeded == 1 && result.unresolved == 0 && !result.stalled );
		CHECK( cleanupRuns == repetition + 1 );
	}
	CHECK( forbiddenRuns.load() == 0 );
}

static void Test_EmptyGraph()
{
	JobGraphBuilder builder;
	SealedGraph graph = Seal( builder );
	ThreadBackend backend( 2 );
	RunResult result = PooledExecutor( &backend ).Execute( graph, RunOptions{} );
	CHECK( result.AllSucceeded() && result.executed == 0 );
	CHECK( result.states.empty() && result.unresolved == 0 );
	CHECK( backend.batches == 0 );
}

int main()
{
	std::printf( "pooledexecutortest (RFC 0003 external backend contracts)\n" );
	RUN( Test_LanesAndOutputPublication );
	RUN( Test_UnpumpedLanesStallEveryDependent );
	RUN( Test_InlineModesServiceEveryLane );
	RUN( Test_CancellationBetweenWaveAdmissionAndStart );
	RUN( Test_RunningJobFinishesWhilePendingJobsCancel );
	RUN( Test_FailureCleanupAndRepeatedRuns );
	RUN( Test_EmptyGraph );
	std::printf( "\n%d checks, %d failures\n", g_checks, g_failures );
	return g_failures == 0 ? 0 : 1;
}
