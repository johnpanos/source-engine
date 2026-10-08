//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: jobsystem.continuous: TaskExecutor, the product graph executor, and
//          RFC 0003 phase I goal J1 "Continuous execution in products".
//
//          * Contract: lanes and publication, unpumped stalls, inline modes,
//            cancellation, failure cleanup, empty graphs, caller overlap.
//          * J1 oracles: a chain behind a short root finishes while an
//            independent long job runs (no wave barrier), and work made
//            ready at a later dependency level starts beside a running job
//            (no parked runner slot). Each is run against deliberately bad
//            executors (the retired wave executor, one posted runner) that it
//            must reject, so the oracle is shown to discriminate.
//          * Equivalence: 1,000 seeded random graphs (every lane, failures,
//            Terminal and Success edges, cancellation) agree with
//            DeterministicExecutor (pumped) or ParallelExecutor (unpumped
//            stalls), at budgets 1, 2 and 4; a seeded bad executor is caught.
//          * Adversarial backends: random start delays, random refusal, tasks
//            that never start before they are withdrawn, tasks run inside
//            PostTask, and a lifetime probe that no task runs after Execute.
//          * Stats: uncoveredReady == 0, posted == ran + withdrawn, and
//            peakRunners <= budget on every run. Wake latency p99 is printed.
//
//=============================================================================//

#include "jobsystem/parallel_executor.h"
#include "jobsystem/task_executor.h"
#include "jobsystem/thread_worker_backend.h"
#include "testing/conformance_result.h"
#include "wave_executor_reference.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstdio>
#include <deque>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

using namespace jobsystem;

static std::atomic<int> g_checks{ 0 };
static std::atomic<int> g_failures{ 0 };
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

namespace
{

using Clock = std::chrono::steady_clock;

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

JobHandle AddJob( JobGraphBuilder &builder, ExecutorToken executor, JobEntry function = {} )
{
	JobDesc desc;
	desc.name = "job";
	desc.executor = executor;
	desc.function = std::move( function );
	return builder.AddJob( desc );
}

SealedGraph Seal( JobGraphBuilder &builder )
{
	auto graph = builder.Seal();
	CHECK( graph.HasValue() );
	return graph.HasValue() ? std::move( graph.Value() ) : SealedGraph{};
}

bool AwaitFlag( const std::atomic<bool> &flag, std::chrono::milliseconds limit )
{
	const auto deadline = Clock::now() + limit;
	while ( !flag.load( std::memory_order_acquire ) )
	{
		if ( Clock::now() > deadline )
			return false;
		std::this_thread::yield();
	}
	return true;
}

// The stats every TaskExecutor run must satisfy.
void CheckStats( const TaskRunStats &stats )
{
	CHECK( stats.uncoveredReady == 0 );
	CHECK( stats.tasksPosted == stats.tasksRan + stats.tasksWithdrawn );
	CHECK( stats.peakRunners <= stats.budget );
	if ( stats.inlineRun )
		CHECK( stats.tasksPosted == 0 && stats.tasksRefused == 0 );
}

RunResult RunTasks( IWorkerBackend *backend, const SealedGraph &graph, const RunOptions &options,
    TaskRunStats *out = nullptr )
{
	TaskRunStats stats;
	RunResult result = TaskExecutor( backend ).Execute( graph, options, &stats );
	CheckStats( stats );
	if ( out )
		*out = stats;
	return result;
}

//-----------------------------------------------------------------------------
// Adversarial backends.
//-----------------------------------------------------------------------------

// Each task gets a thread that waits a seeded delay before it runs, so starts
// arrive late and out of order. Settle joins.
class DelayBackend final : public IWorkerBackend
{
public:
	DelayBackend( int workers, uint64_t seed ) : m_workers( workers ), m_seed( seed ) {}
	int WorkerCount() const override { return m_workers; }
	void *PostTask( WorkerTaskFn task, void *context ) override
	{
		const uint64_t s = m_seed.fetch_add( 0x9e3779b97f4a7c15ull ) * 6364136223846793005ull;
		const auto delay = std::chrono::microseconds( ( s >> 40 ) % 300 );
		return new std::thread(
		    [task, context, delay]
		    {
			    std::this_thread::sleep_for( delay );
			    task( context );
		    } );
	}
	bool SettleTask( void *ticket ) override
	{
		auto *thread = static_cast<std::thread *>( ticket );
		thread->join();
		delete thread;
		return true;
	}
	bool ShouldRunInline() override { return false; }

private:
	int m_workers;
	std::atomic<uint64_t> m_seed;
};

// Refuses a seeded share of posts; accepted tasks run on a ThreadWorkerBackend.
class RefusingBackend final : public IWorkerBackend
{
public:
	RefusingBackend( int workers, unsigned percent ) : m_inner( workers ), m_percent( percent ) {}
	int WorkerCount() const override { return m_inner.WorkerCount(); }
	void *PostTask( WorkerTaskFn task, void *context ) override
	{
		if ( ( m_counter.fetch_add( 1 ) * 37u ) % 100u < m_percent )
		{
			++refused;
			return nullptr;
		}
		return m_inner.PostTask( task, context );
	}
	bool SettleTask( void *ticket ) override { return m_inner.SettleTask( ticket ); }
	bool ShouldRunInline() override { return m_inner.ShouldRunInline(); }
	std::atomic<unsigned> refused{ 0 };

private:
	ThreadWorkerBackend m_inner;
	unsigned m_percent;
	std::atomic<unsigned> m_counter{ 0 };
};

// Accepts every task and never starts one: a pool busy with other work until
// the run ends. Every task is withdrawn at settle; the caller does it all.
class StalledBackend final : public IWorkerBackend
{
public:
	explicit StalledBackend( int workers ) : m_workers( workers ) {}
	int WorkerCount() const override { return m_workers; }
	void *PostTask( WorkerTaskFn, void * ) override
	{
		++posted;
		return new int( 0 );
	}
	bool SettleTask( void *ticket ) override
	{
		delete static_cast<int *>( ticket );
		++withdrawn;
		return false;
	}
	bool ShouldRunInline() override { return false; }
	std::atomic<unsigned> posted{ 0 }, withdrawn{ 0 };

private:
	int m_workers;
};

// Runs the task to completion inside PostTask, as a full engine queue may on
// its posting worker. A scheduler that posted under its own lock deadlocks.
class InlinePostBackend final : public IWorkerBackend
{
public:
	explicit InlinePostBackend( int workers ) : m_workers( workers ) {}
	int WorkerCount() const override { return m_workers; }
	void *PostTask( WorkerTaskFn task, void *context ) override
	{
		task( context );
		return new int( 1 );
	}
	bool SettleTask( void *ticket ) override
	{
		delete static_cast<int *>( ticket );
		return true;
	}
	bool ShouldRunInline() override { return false; }

private:
	int m_workers;
};

// Records when tasks run and settle, to prove no task outlives Execute.
class LifetimeBackend final : public IWorkerBackend
{
public:
	explicit LifetimeBackend( int workers ) : m_inner( workers ) {}
	int WorkerCount() const override { return m_inner.WorkerCount(); }
	struct Wrap
	{
		LifetimeBackend *self;
		WorkerTaskFn task;
		void *context;
		void *ticket = nullptr;
	};
	static void Run( void *opaque )
	{
		Wrap *wrap = static_cast<Wrap *>( opaque );
		++wrap->self->running;
		wrap->task( wrap->context );
		--wrap->self->running;
	}
	void *PostTask( WorkerTaskFn task, void *context ) override
	{
		Wrap *wrap = new Wrap{ this, task, context };
		wrap->ticket = m_inner.PostTask( &Run, wrap );
		if ( !wrap->ticket )
		{
			delete wrap;
			return nullptr;
		}
		++outstanding;
		return wrap;
	}
	bool SettleTask( void *opaque ) override
	{
		Wrap *wrap = static_cast<Wrap *>( opaque );
		const bool ran = m_inner.SettleTask( wrap->ticket );
		delete wrap;
		--outstanding;
		return ran;
	}
	bool ShouldRunInline() override { return m_inner.ShouldRunInline(); }
	std::atomic<int> running{ 0 }, outstanding{ 0 };

private:
	ThreadWorkerBackend m_inner;
};

//-----------------------------------------------------------------------------
// Deliberately bad executors (negative controls).
//-----------------------------------------------------------------------------

// Dependency-correct, but everything runs on one posted task: one worker busy
// while the rest of the budget idles. Fails the J1 saturation oracle.
class OneRunnerExecutor final : public IGraphExecutor
{
public:
	explicit OneRunnerExecutor( IWorkerBackend *backend ) : m_backend( backend ) {}
	RunResult Execute( const SealedGraph &graph, const RunOptions &opts ) override
	{
		struct Call
		{
			const SealedGraph *graph;
			const RunOptions *opts;
			RunResult result;
			static void Task( void *self )
			{
				Call &call = *static_cast<Call *>( self );
				call.result = DeterministicExecutor().Execute( *call.graph, *call.opts );
			}
		} call{ &graph, &opts, {} };
		void *ticket = m_backend->PostTask( &Call::Task, &call );
		if ( !ticket || !m_backend->SettleTask( ticket ) )
			Call::Task( &call );
		return call.result;
	}

private:
	IWorkerBackend *m_backend;
};

// Treats every edge as a Success edge: cleanup continuations of failed or
// canceled producers are canceled. Fails the equivalence oracle.
class SuccessOnlyExecutor final : public IGraphExecutor
{
public:
	RunResult Execute( const SealedGraph &graph, const RunOptions &opts ) override
	{
		RunResult result;
		result.states.assign( graph.JobCount(), JobState::Admitted );
		for ( uint32_t id : graph.TopoOrder() )
		{
			bool cancel = opts.cancel && opts.cancel->load();
			for ( const SealedGraph::Prereq &p : graph.GetJob( id ).prereqs )
				cancel = cancel || result.states[p.producer] != JobState::Succeeded;
			if ( cancel )
			{
				result.states[id] = JobState::Canceled;
				result.canceled++;
				continue;
			}
			JobRunContext ctx( opts.frame, id );
			if ( graph.GetJob( id ).function )
			{
				graph.GetJob( id ).function( ctx );
				result.executed++;
			}
			result.states[id] = ctx.Failed() ? JobState::Failed : JobState::Succeeded;
			( ctx.Failed() ? result.failed : result.succeeded )++;
		}
		return result;
	}
};

//-----------------------------------------------------------------------------
// Contract tests (carried over from the retired PooledExecutor suite).
//-----------------------------------------------------------------------------

void Test_LanesAndOutputPublication()
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
	JobHandle emptyMain = AddJob( builder, Executor::MainThread() );
	JobHandle emptyBlocking = AddJob( builder, Executor::BlockingIO() );
	SealedGraph graph = Seal( builder );
	ThreadWorkerBackend backend( 3 );
	for ( int round = 0; round < 50; ++round )
	{
		computeValue = mainValue = blockingValue = sequenceValue = 0;
		TraceSink trace( graph.JobCount() );
		RunOptions options;
		options.trace = &trace;
		RunResult result = RunTasks( &backend, graph, options );
		CHECK( result.AllSucceeded() );
		CHECK( result.succeeded == 7 && result.executed == 5 );
		CHECK( result.unresolved == 0 );
		CHECK( mainValue == 38 && blockingValue == 39 && sequenceValue == 118 );
		// Affine and blocking work only ever runs on the pumping caller; compute
		// work may run anywhere.
		const std::thread::id caller = std::this_thread::get_id();
		for ( JobHandle handle : { main, blocking, emptyMain, emptyBlocking } )
			CHECK( trace.threads[handle.id] == caller );
		for ( const auto &states : trace.states )
			CHECK( states == std::vector<JobState>( { JobState::Running, JobState::Succeeded } ) );
	}
	(void)first;
}

void Test_ComputeRunsOnRunnerTasks()
{
	// A compute job that waits for a main-thread job's start can only finish
	// on a runner task: the caller is busy with the main-thread job.
	ThreadWorkerBackend backend( 2 );
	for ( int round = 0; round < 20; ++round )
	{
		JobGraphBuilder builder;
		std::atomic<bool> computeStarted( false ), mainStarted( false );
		std::atomic<bool> computeSaw( false ), mainSaw( false );
		std::thread::id computeThread;
		AddJob( builder, Executor::Compute(),
		    [&]( JobRunContext & )
		    {
			    computeThread = std::this_thread::get_id();
			    computeStarted = true;
			    computeSaw = AwaitFlag( mainStarted, std::chrono::seconds( 5 ) );
		    } );
		AddJob( builder, Executor::MainThread(),
		    [&]( JobRunContext & )
		    {
			    mainStarted = true;
			    mainSaw = AwaitFlag( computeStarted, std::chrono::seconds( 5 ) );
		    } );
		SealedGraph graph = Seal( builder );
		TaskRunStats stats;
		RunResult result = RunTasks( &backend, graph, RunOptions{}, &stats );
		CHECK( result.AllSucceeded() );
		CHECK( computeSaw.load() && mainSaw.load() );
		CHECK( computeThread != std::this_thread::get_id() );
		CHECK( stats.jobsOnRunners == 1 && stats.jobsOnCaller == 1 );
		CHECK( stats.tasksPosted >= 1 && stats.tasksRan >= 1 );
	}
}

void Test_UnpumpedLanesStallEveryDependent()
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
	ThreadWorkerBackend backend( 2 );
	RunResult result = RunTasks( &backend, graph, options );
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
	options.trace = nullptr;
	RunResult parallel = ParallelExecutor( 2 ).Execute( graph, options );
	CHECK( parallel.states == result.states );
	CHECK( parallel.unresolved == result.unresolved );
}

void Test_InlineModesServiceEveryLane()
{
	ThreadWorkerBackend zeroWorkers( 0 );
	IWorkerBackend *backends[] = { nullptr, &zeroWorkers };
	for ( IWorkerBackend *backend : backends )
	{
		JobGraphBuilder builder;
		std::vector<std::thread::id> threads( 5 );
		const ExecutorToken lanes[] = { Executor::Compute(), Executor::MainThread(),
		    Executor::BlockingIO(), Executor::Sequence( 0 ), Executor::Sequence( 0 ) };
		for ( int index = 0; index < 5; ++index )
		{
			AddJob( builder, lanes[index],
			    [&, index]( JobRunContext & )
			    {
				    threads[index] = std::this_thread::get_id();
			    } );
		}
		SealedGraph graph = Seal( builder );
		RunOptions options;
		options.pumpMainThread = false; // inline services every lane anyway
		TaskRunStats stats;
		RunResult result = RunTasks( backend, graph, options, &stats );
		CHECK( result.AllSucceeded() && result.succeeded == 5 && result.executed == 5 );
		CHECK( stats.inlineRun );
		for ( std::thread::id thread : threads )
			CHECK( thread == std::this_thread::get_id() );
	}
	CHECK( zeroWorkers.TasksPosted() == 0 );
}

void Test_NestedRunOnWorkerIsInline()
{
	ThreadWorkerBackend backend( 2 );
	std::atomic<int> innerInline{ 0 }, innerPooled{ 0 }, innerOk{ 0 };
	JobGraphBuilder inner;
	for ( int i = 0; i < 4; ++i )
		AddJob( inner, Executor::Compute(), []( JobRunContext & ) {} );
	SealedGraph innerGraph = Seal( inner );
	JobGraphBuilder outer;
	for ( int i = 0; i < 8; ++i )
	{
		AddJob( outer, Executor::Compute(),
		    [&]( JobRunContext & )
		    {
			    TaskRunStats stats;
			    RunResult r = TaskExecutor( &backend ).Execute( innerGraph, RunOptions{}, &stats );
			    if ( r.AllSucceeded() && r.succeeded == 4 )
				    ++innerOk;
			    ( stats.inlineRun ? innerInline : innerPooled )++;
			    if ( stats.inlineRun != backend.ShouldRunInline() )
				    ++innerOk; // poisons the count below
		    } );
	}
	SealedGraph outerGraph = Seal( outer );
	for ( int round = 0; round < 20; ++round )
	{
		innerInline = innerPooled = innerOk = 0;
		RunResult r = RunTasks( &backend, outerGraph, RunOptions{} );
		CHECK( r.AllSucceeded() );
		CHECK( innerOk == 8 );
		CHECK( innerInline + innerPooled == 8 );
	}
}

void Test_CancellationAtFirstPost()
{
	// The backend publishes cancellation at the run's first post, before any
	// job starts: every job is canceled and none runs.
	class CancelOnPost final : public IWorkerBackend
	{
	public:
		explicit CancelOnPost( std::atomic<bool> &flag ) : m_flag( flag ), m_inner( 2 ) {}
		int WorkerCount() const override { return 2; }
		void *PostTask( WorkerTaskFn task, void *context ) override
		{
			m_flag.store( true, std::memory_order_release );
			return m_inner.PostTask( task, context );
		}
		bool SettleTask( void *ticket ) override { return m_inner.SettleTask( ticket ); }
		bool ShouldRunInline() override { return false; }

	private:
		std::atomic<bool> &m_flag;
		ThreadWorkerBackend m_inner;
	};
	JobGraphBuilder builder;
	std::atomic<bool> canceled{ false };
	std::atomic<int> runs{ 0 };
	for ( int i = 0; i < 6; ++i )
		AddJob( builder, Executor::Compute(),
		    [&]( JobRunContext & )
		    {
			    runs.fetch_add( 1 );
		    } );
	for ( ExecutorToken lane : { Executor::MainThread(), Executor::BlockingIO() } )
		AddJob( builder, lane,
		    [&]( JobRunContext & )
		    {
			    runs.fetch_add( 1 );
		    } );
	SealedGraph graph = Seal( builder );
	CancelOnPost backend( canceled );
	RunOptions options;
	options.cancel = &canceled;
	RunResult result = RunTasks( &backend, graph, options );
	CHECK( result.canceled == 8 && result.executed == 0 && !result.stalled );
	CHECK( runs.load() == 0 );
	// Cancellation present at admission: still nothing runs.
	result = RunTasks( &backend, graph, options );
	CHECK( result.canceled == 8 && result.executed == 0 );
}

void Test_RunningJobFinishesWhilePendingJobsCancel()
{
	ThreadWorkerBackend worker( 1 );
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
		builder.AddDependency( running, queued );
		builder.AddDependency( running, caller );
		builder.AddDependency( queued, dependent );
		builder.AddDependency( running, cleanup, DependencyKind::Terminal );
		SealedGraph graph = Seal( builder );
		RunOptions options;
		options.cancel = &canceled;
		RunResult result = RunTasks( backend, graph, options );
		CHECK( result.succeeded == 1 && result.executed == 1 && result.canceled == 4 );
		CHECK( result.failed == 0 && !result.stalled );
		CHECK( completed == 1 && forbiddenRuns.load() == 0 );
		CHECK( result.states[running.id] == JobState::Succeeded );
		for ( JobHandle handle : { queued, caller, dependent, cleanup } )
			CHECK( result.states[handle.id] == JobState::Canceled );
	}
}

void Test_FailureCleanupAndRepeatedRuns()
{
	JobGraphBuilder builder;
	std::atomic<int> cleanupRuns{ 0 }, forbiddenRuns{ 0 };
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
	ThreadWorkerBackend backend( 3 );
	TaskExecutor executor( &backend );
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

void Test_EmptyGraph()
{
	JobGraphBuilder builder;
	SealedGraph graph = Seal( builder );
	ThreadWorkerBackend backend( 2 );
	RunResult result = RunTasks( &backend, graph, RunOptions{} );
	CHECK( result.AllSucceeded() && result.executed == 0 );
	CHECK( result.states.empty() && result.unresolved == 0 );
	CHECK( backend.TasksPosted() == 0 );
}

//-----------------------------------------------------------------------------
// J1 oracles.
//-----------------------------------------------------------------------------

// A long job waits for the end of a chain behind a short root. A continuous
// executor finishes the chain beside the long job; a wave executor runs the
// chain's second link only after the first wave, which contains the long job.
bool NoWaveBarrier( IGraphExecutor &executor, std::chrono::milliseconds limit )
{
	std::atomic<bool> chainDone( false ), longSaw( false );
	JobGraphBuilder b;
	AddJob( b, Executor::Compute(),
	    [&]( JobRunContext & )
	    {
		    longSaw = AwaitFlag( chainDone, limit );
	    } );
	JobHandle previous;
	for ( int i = 0; i < 6; ++i )
	{
		JobHandle link = AddJob( b, i % 2 ? Executor::Sequence( 4 ) : Executor::Compute(),
		    i == 5 ? JobEntry(
		                 [&]( JobRunContext & )
		                 {
			                 chainDone.store( true, std::memory_order_release );
		                 } )
		           : JobEntry( []( JobRunContext & ) {} ) );
		if ( previous.IsValid() )
			b.AddDependency( previous, link );
		previous = link;
	}
	SealedGraph graph = Seal( b );
	RunResult r = executor.Execute( graph, RunOptions{} );
	return r.AllSucceeded() && longSaw.load();
}

// Work that becomes ready at a later dependency level starts while an earlier
// job still runs: a long root A, and a short root B whose three dependents all
// rendezvous with A. Needs four servicers at once (budget 4: runners and the
// caller), and no barrier between B's level and A's.
bool LaterLevelSaturates( IGraphExecutor &executor, std::chrono::milliseconds limit )
{
	std::atomic<int> arrived( 0 );
	std::atomic<bool> allArrived( false ), ok( true );
	auto rendezvous = [&]( JobRunContext & )
	{
		if ( ++arrived == 4 )
			allArrived.store( true, std::memory_order_release );
		if ( !AwaitFlag( allArrived, limit ) )
			ok = false;
	};
	JobGraphBuilder b;
	AddJob( b, Executor::Compute(), rendezvous );                                // A
	JobHandle root = AddJob( b, Executor::Compute(), []( JobRunContext & ) {} ); // B
	for ( int i = 0; i < 3; ++i )
		b.AddDependency( root, AddJob( b, Executor::Compute(), rendezvous ) );
	SealedGraph graph = Seal( b );
	RunResult r = executor.Execute( graph, RunOptions{} );
	return r.AllSucceeded() && ok.load() && arrived.load() == 4;
}

void Test_J1NoWaveBarrier()
{
	for ( int workers : { 1, 2, 4 } )
	{
		ThreadWorkerBackend backend( workers );
		TaskExecutor tasks( &backend );
		bool ok = true;
		for ( int round = 0; ok && round < 25; ++round )
			ok = NoWaveBarrier( tasks, std::chrono::seconds( 5 ) );
		CHECK( ok ); // stops at the first miss: a broken executor fails fast
	}
	// The real engine pool runs this oracle in corpus.jobs.pool-bridge.
}

void Test_J1LaterLevelSaturates()
{
	ThreadWorkerBackend backend( 3 ); // three runners plus the caller
	TaskExecutor tasks( &backend );
	bool ok = true;
	for ( int round = 0; ok && round < 25; ++round )
		ok = LaterLevelSaturates( tasks, std::chrono::seconds( 5 ) );
	CHECK( ok );
	ThreadWorkerBackend wide( 8 );
	TaskExecutor wideTasks( &wide );
	ok = true;
	for ( int round = 0; ok && round < 25; ++round )
		ok = LaterLevelSaturates( wideTasks, std::chrono::seconds( 5 ) );
	CHECK( ok );
}

void Test_J1OraclesRejectBadExecutors()
{
	// Short limits: the bad executors are expected to time out.
	const auto limit = std::chrono::milliseconds( 150 );
	ThreadWorkerBackend backend( 3 );
	jobsystemtest::WaveExecutorReference wave( &backend );
	OneRunnerExecutor oneRunner( &backend );
	int waveCaught = 0, oneCaught = 0;
	for ( int round = 0; round < 5; ++round )
	{
		waveCaught += NoWaveBarrier( wave, limit ) ? 0 : 1;
		waveCaught += LaterLevelSaturates( wave, limit ) ? 0 : 1;
		oneCaught += LaterLevelSaturates( oneRunner, limit ) ? 0 : 1;
		oneCaught += NoWaveBarrier( oneRunner, limit ) ? 0 : 1;
	}
	std::printf( "  bad executors caught: wave %d/10, one-runner %d/10\n", waveCaught, oneCaught );
	CHECK( waveCaught == 10 );
	CHECK( oneCaught == 10 );
	// The bad executors are still dependency-correct: equivalence alone cannot
	// reject them, which is why the timing oracles exist.
	JobGraphBuilder b;
	JobHandle a = AddJob( b, Executor::Compute(),
	    []( JobRunContext &c )
	    {
		    c.Fail();
	    } );
	b.AddDependency( a, AddJob( b, Executor::Compute() ), DependencyKind::Terminal );
	b.AddDependency( a, AddJob( b, Executor::Compute() ), DependencyKind::Success );
	SealedGraph g = Seal( b );
	const RunResult reference = DeterministicExecutor().Execute( g, RunOptions{} );
	CHECK( wave.Execute( g, RunOptions{} ).states == reference.states );
	CHECK( oneRunner.Execute( g, RunOptions{} ).states == reference.states );
}

// A runner returns its worker as soon as the ready queue is empty: while a
// long main-thread job runs and no compute work is ready, no runner task is
// left occupying a worker inside the graph (J1: workers do not park in a run).
bool RunnersReturnWhenIdle( IGraphExecutor &executor, LifetimeBackend &backend )
{
	std::atomic<bool> computeDone( false ), sawIdle( false );
	JobGraphBuilder b;
	JobHandle compute = AddJob( b, Executor::Compute(),
	    [&]( JobRunContext & )
	    {
		    computeDone.store( true, std::memory_order_release );
	    } );
	JobHandle main = AddJob( b, Executor::MainThread(),
	    [&]( JobRunContext & )
	    {
		    // Wait for the compute job, then for every runner to leave.
		    if ( !AwaitFlag( computeDone, std::chrono::seconds( 2 ) ) )
			    return;
		    const auto deadline = Clock::now() + std::chrono::milliseconds( 500 );
		    while ( Clock::now() < deadline )
		    {
			    if ( backend.running.load() == 0 )
			    {
				    sawIdle = true;
				    return;
			    }
			    std::this_thread::yield();
		    }
	    } );
	(void)compute;
	(void)main;
	SealedGraph graph = Seal( b );
	RunResult r = executor.Execute( graph, RunOptions{} );
	return r.AllSucceeded() && sawIdle.load();
}

void Test_J1RunnersReturnWorkersWhenIdle()
{
	LifetimeBackend backend( 2 );
	TaskExecutor tasks( &backend );
	bool ok = true;
	for ( int round = 0; ok && round < 50; ++round )
		ok = RunnersReturnWhenIdle( tasks, backend );
	CHECK( ok );
}

// Wake latency: the time from a root's end to the start of each of the four
// jobs it releases, with four runner slots. Recorded, not judged (RFC 0003
// J1 records the p99).
void Test_J1WakeLatency()
{
	ThreadWorkerBackend backend( 4 );
	TaskExecutor tasks( &backend );
	std::vector<double> samples;
	for ( int round = 0; round < 400; ++round )
	{
		Clock::time_point released;
		std::mutex mutex;
		std::vector<double> starts;
		JobGraphBuilder b;
		JobHandle root = AddJob( b, Executor::Compute(),
		    [&]( JobRunContext & )
		    {
			    released = Clock::now();
		    } );
		for ( int i = 0; i < 4; ++i )
			b.AddDependency( root,
			    AddJob( b, Executor::Compute(),
			        [&]( JobRunContext & )
			        {
				        const double us =
				            std::chrono::duration<double, std::micro>( Clock::now() - released )
				                .count();
				        std::lock_guard<std::mutex> lock( mutex );
				        starts.push_back( us );
				        // Hold the slot so each dependent needs its own servicer.
				        std::this_thread::sleep_for( std::chrono::microseconds( 50 ) );
			        } ) );
		SealedGraph g = Seal( b );
		RunResult r = tasks.Execute( g, RunOptions{} );
		CHECK( r.AllSucceeded() );
		samples.insert( samples.end(), starts.begin(), starts.end() );
	}
	std::sort( samples.begin(), samples.end() );
	auto at = [&]( double q )
	{
		return samples.empty()
		           ? 0.0
		           : samples[std::min( samples.size() - 1, (size_t)( q * samples.size() ) )];
	};
	std::printf( "  INFO wake latency (us): p50 %.1f p90 %.1f p99 %.1f max %.1f over %zu starts\n",
	    at( 0.5 ), at( 0.9 ), at( 0.99 ), samples.empty() ? 0.0 : samples.back(), samples.size() );
	CHECK( samples.size() == 1600 );
}

//-----------------------------------------------------------------------------
// Equivalence on seeded random graphs.
//-----------------------------------------------------------------------------

struct Rng
{
	uint64_t s;
	uint32_t Next()
	{
		s = s * 6364136223846793005ull + 1442695040888963407ull;
		return (uint32_t)( s >> 33 );
	}
	uint32_t Below( uint32_t n ) { return n ? Next() % n : 0; }
};

struct RandomGraph
{
	SealedGraph graph;
	std::vector<std::atomic<int>> runs;
	std::vector<std::atomic<uint32_t>> value;
	std::atomic<int> violations{ 0 };
	std::atomic<bool> cancel{ false };
	uint32_t cancelAt = UINT32_MAX;
	explicit RandomGraph( uint32_t n ) : runs( n ), value( n ) {}
	void Reset()
	{
		for ( auto &r : runs )
			r.store( 0 );
		for ( auto &v : value )
			v.store( 0 );
		violations = 0;
		cancel = false;
	}
};

// Each job reads its producers' values (published by the executor) and
// writes its own; a job with a seeded id fails, and one may cancel the scope.
std::unique_ptr<RandomGraph> MakeRandomGraph( uint64_t seed, bool affine )
{
	Rng rng{ seed * 0x9e3779b97f4a7c15ull + 1 };
	const uint32_t n = 2 + rng.Below( 40 );
	auto rg = std::make_unique<RandomGraph>( n );
	if ( rng.Below( 4 ) == 0 )
		rg->cancelAt = rng.Below( n );
	std::vector<std::vector<uint32_t>> producers( n );
	JobGraphBuilder b;
	std::vector<JobHandle> handles;
	RandomGraph *g = rg.get();
	for ( uint32_t i = 0; i < n; ++i )
	{
		const uint32_t lane = affine ? rng.Below( 6 ) : 0;
		const bool fails = rng.Below( 9 ) == 0;
		for ( uint32_t p = 0; p < i; ++p )
			if ( rng.Below( 5 ) == 0 )
				producers[i].push_back( p );
		JobDesc d;
		d.name = "random";
		d.executor = lane == 1   ? Executor::MainThread()
		             : lane == 2 ? Executor::BlockingIO()
		             : lane == 3 ? Executor::Sequence( 1 + rng.Below( 3 ) )
		                         : Executor::Compute();
		const std::vector<uint32_t> mine = producers[i];
		const bool hasFunction = rng.Below( 8 ) != 0;
		if ( hasFunction )
		{
			d.function = [g, i, mine, fails]( JobRunContext &ctx )
			{
				g->runs[i].fetch_add( 1 );
				uint32_t v = i * 2654435761u;
				for ( uint32_t p : mine )
					v ^= g->value[p].load( std::memory_order_relaxed ) + p;
				g->value[i].store( v, std::memory_order_relaxed );
				if ( i == g->cancelAt )
					g->cancel.store( true, std::memory_order_release );
				if ( fails )
					ctx.Fail();
			};
		}
		handles.push_back( b.AddJob( d ) );
		for ( uint32_t p : mine )
			b.AddDependency( handles[p], handles[i],
			    rng.Below( 3 ) == 0 ? DependencyKind::Terminal : DependencyKind::Success );
	}
	rg->graph = std::move( b.Seal().Value() );
	return rg;
}

bool SameRun( const RunResult &a, const RunResult &b )
{
	return a.states == b.states && a.succeeded == b.succeeded && a.failed == b.failed &&
	       a.canceled == b.canceled && a.executed == b.executed && a.stalled == b.stalled &&
	       a.unresolved == b.unresolved;
}

// Graphs whose cancellation is published by a job are compared only when the
// cancel point is deterministic in the reference: cancellation races with
// concurrently running jobs, so those graphs are checked for exactly-once and
// for no job starting after a canceled prerequisite instead.
void Test_EquivalenceOnSeededGraphs()
{
	ThreadWorkerBackend b1( 1 ), b2( 2 ), b4( 4 );
	IWorkerBackend *backends[] = { &b1, &b2, &b4 };
	uint32_t compared = 0, cancelRuns = 0, mismatches = 0;
	for ( uint64_t seed = 1; seed <= 1000; ++seed )
	{
		const bool affine = seed % 3 != 0;
		auto rg = MakeRandomGraph( seed, affine );
		for ( bool pump : { true, false } )
		{
			RunOptions options;
			options.pumpMainThread = pump;
			options.cancel = &rg->cancel;
			rg->Reset();
			// Pumped: DeterministicExecutor is the canonical oracle. Unpumped:
			// ParallelExecutor without blocking workers states the stall rules.
			RunResult expected = pump ? DeterministicExecutor().Execute( rg->graph, options )
			                          : ParallelExecutor( 2 ).Execute( rg->graph, options );
			std::vector<uint32_t> expectedValues;
			for ( auto &v : rg->value )
				expectedValues.push_back( v.load() );
			for ( IWorkerBackend *backend : backends )
			{
				rg->Reset();
				RunResult r = RunTasks( backend, rg->graph, options );
				bool once = true;
				for ( auto &x : rg->runs )
					once = once && x.load() <= 1;
				CHECK( once );
				if ( rg->cancelAt != UINT32_MAX )
				{
					++cancelRuns;
					CHECK( r.succeeded + r.failed + r.canceled + r.unresolved ==
					       rg->graph.JobCount() );
					continue;
				}
				++compared;
				const bool same = SameRun( r, expected );
				bool values = true;
				for ( uint32_t i = 0; i < rg->value.size(); ++i )
					values = values && rg->value[i].load() == expectedValues[i];
				if ( !same || !values )
				{
					++mismatches;
					std::printf( "  seed %llu pump %d workers %d differs\n",
					    (unsigned long long)seed, pump ? 1 : 0, backend->WorkerCount() );
				}
			}
		}
	}
	std::printf( "  %u runs compared, %u cancel-race runs checked for exactly-once\n", compared,
	    cancelRuns );
	CHECK( mismatches == 0 );
	CHECK( compared >= 3000 );
}

void Test_EquivalenceRejectsBadExecutor()
{
	SuccessOnlyExecutor bad;
	uint32_t caught = 0, graphs = 0;
	for ( uint64_t seed = 1; seed <= 200; ++seed )
	{
		auto rg = MakeRandomGraph( seed, false );
		if ( rg->cancelAt != UINT32_MAX )
			continue;
		++graphs;
		RunResult expected = DeterministicExecutor().Execute( rg->graph, RunOptions{} );
		rg->Reset();
		if ( !SameRun( bad.Execute( rg->graph, RunOptions{} ), expected ) )
			++caught;
	}
	std::printf( "  success-only executor differs on %u of %u graphs\n", caught, graphs );
	CHECK( caught > 0 );
}

//-----------------------------------------------------------------------------
// Adversarial backends.
//-----------------------------------------------------------------------------

void CheckAgainstReference( IWorkerBackend &backend, uint64_t seeds, TaskRunStats *sum )
{
	for ( uint64_t seed = 1; seed <= seeds; ++seed )
	{
		auto rg = MakeRandomGraph( seed + 5000, true );
		rg->cancelAt = UINT32_MAX;
		RunResult expected = DeterministicExecutor().Execute( rg->graph, RunOptions{} );
		rg->Reset();
		TaskRunStats stats;
		RunResult r = RunTasks( &backend, rg->graph, RunOptions{}, &stats );
		CHECK( SameRun( r, expected ) );
		sum->tasksPosted += stats.tasksPosted;
		sum->tasksRefused += stats.tasksRefused;
		sum->tasksRan += stats.tasksRan;
		sum->tasksWithdrawn += stats.tasksWithdrawn;
		sum->jobsOnRunners += stats.jobsOnRunners;
		sum->jobsOnCaller += stats.jobsOnCaller;
	}
}

void Test_DelayedStarts()
{
	DelayBackend backend( 3, 11 );
	TaskRunStats sum;
	CheckAgainstReference( backend, 150, &sum );
	CHECK( sum.tasksRan > 0 );
}

void Test_RefusedPosts()
{
	RefusingBackend backend( 3, 50 );
	TaskRunStats sum;
	CheckAgainstReference( backend, 150, &sum );
	CHECK( sum.tasksRefused > 0 && backend.refused == sum.tasksRefused );
	CHECK( sum.tasksPosted > 0 );
	RefusingBackend always( 3, 100 );
	TaskRunStats none;
	CheckAgainstReference( always, 50, &none );
	CHECK( none.tasksPosted == 0 && none.jobsOnRunners == 0 && none.tasksRefused > 0 );
}

void Test_TasksThatNeverStart()
{
	StalledBackend backend( 4 );
	TaskRunStats sum;
	CheckAgainstReference( backend, 100, &sum );
	CHECK( sum.tasksPosted > 0 );
	CHECK( sum.tasksRan == 0 && sum.tasksWithdrawn == sum.tasksPosted );
	CHECK( backend.posted == backend.withdrawn );
	CHECK( sum.jobsOnRunners == 0 );
}

void Test_TasksRunInsidePost()
{
	InlinePostBackend backend( 3 );
	TaskRunStats sum;
	CheckAgainstReference( backend, 150, &sum );
	CHECK( sum.tasksRan == sum.tasksPosted && sum.tasksWithdrawn == 0 );
}

void Test_NoTaskOutlivesExecute()
{
	LifetimeBackend backend( 3 );
	TaskExecutor executor( &backend );
	for ( uint64_t seed = 1; seed <= 200; ++seed )
	{
		auto rg = MakeRandomGraph( seed + 9000, seed % 2 == 0 );
		rg->cancelAt = UINT32_MAX;
		executor.Execute( rg->graph, RunOptions{} );
		CHECK( backend.running.load() == 0 );
		CHECK( backend.outstanding.load() == 0 );
	}
}

void Test_ConcurrentCallers()
{
	ThreadWorkerBackend backend( 3 );
	TaskExecutor shared( &backend );
	std::vector<std::thread> threads;
	std::atomic<int> bad{ 0 };
	for ( int t = 0; t < 4; ++t )
	{
		threads.emplace_back(
		    [&, t]
		    {
			    for ( uint64_t seed = 1; seed <= 60; ++seed )
			    {
				    auto rg = MakeRandomGraph( seed * 7 + t, false );
				    rg->cancelAt = UINT32_MAX;
				    RunResult expected = DeterministicExecutor().Execute( rg->graph, RunOptions{} );
				    rg->Reset();
				    if ( !SameRun( shared.Execute( rg->graph, RunOptions{} ), expected ) )
					    ++bad;
			    }
		    } );
	}
	for ( std::thread &thread : threads )
		thread.join();
	CHECK( bad.load() == 0 );
}

} // namespace

int main()
{
	std::printf( "taskexecutortest (RFC 0003 phase I, J1)\n" );
	RUN( Test_LanesAndOutputPublication );
	RUN( Test_ComputeRunsOnRunnerTasks );
	RUN( Test_UnpumpedLanesStallEveryDependent );
	RUN( Test_InlineModesServiceEveryLane );
	RUN( Test_NestedRunOnWorkerIsInline );
	RUN( Test_CancellationAtFirstPost );
	RUN( Test_RunningJobFinishesWhilePendingJobsCancel );
	RUN( Test_FailureCleanupAndRepeatedRuns );
	RUN( Test_EmptyGraph );
	RUN( Test_J1NoWaveBarrier );
	RUN( Test_J1LaterLevelSaturates );
	RUN( Test_J1OraclesRejectBadExecutors );
	RUN( Test_J1RunnersReturnWorkersWhenIdle );
	RUN( Test_J1WakeLatency );
	RUN( Test_EquivalenceOnSeededGraphs );
	RUN( Test_EquivalenceRejectsBadExecutor );
	RUN( Test_DelayedStarts );
	RUN( Test_RefusedPosts );
	RUN( Test_TasksThatNeverStart );
	RUN( Test_TasksRunInsidePost );
	RUN( Test_NoTaskOutlivesExecute );
	RUN( Test_ConcurrentCallers );
	std::printf( "%d checks, %d failures\n", g_checks.load(), g_failures.load() );
	return testing::ReportConformance( g_checks.load(), g_failures.load() );
}
