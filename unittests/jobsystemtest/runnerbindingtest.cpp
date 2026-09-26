//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: jobsystem.runner-bindings (RFC 0003 Phase B "runner bindings";
//			roadmap R10, R10-BINDINGS): ParallelExecutor's MainThread and
//			BlockingIO lanes bound to platform.task-runner.v1 runners.
//
//			- bound jobs run on their runner (thread / sequence identity) and
//			  never on compute workers or the caller;
//			- a lane that stalls unbound completes when bound (the control);
//			- dependencies across bound and compute lanes hold, with plain
//			  (non-atomic) data published from producer to consumer;
//			- a caller that is the runner services the lane itself;
//			- a runner that refuses, or drops an accepted job at shutdown,
//			  yields a stall and a returned run, never a hang;
//			- cancellation reaches bound jobs;
//			- on seeded random graphs with every lane bound, terminal states
//			  equal DeterministicExecutor's.
//
//=============================================================================//

#include "jobsystem/graph_executor.h"
#include "jobsystem/job_graph.h"
#include "jobsystem/parallel_executor.h"

#include "../../platform/runners/manual_task_runner.h"
#include "../../platform/runners/sequenced_task_runner.h"
#include "../../platform/runners/thread_task_runner.h"
#include "testing/checks.h"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <mutex>
#include <thread>
#include <vector>

using namespace jobsystem;

namespace
{

SealedGraph MustSeal( JobGraphBuilder &builder )
{
	auto sealed = builder.Seal();
	if ( !sealed )
		std::abort();
	return std::move( sealed.Value() );
}

// Runs Execute on a helper thread and waits at most `seconds`, so a hang is a
// failed check rather than a stuck suite. Returns false on timeout (the helper
// is then left running and the process exits after reporting).
bool ExecuteWithin( ParallelExecutor &executor, const SealedGraph &graph, const RunOptions &options,
    RunResult &out, int seconds = 30 )
{
	std::atomic<bool> done{ false };
	std::thread runner(
	    [&]
	    {
		    out = executor.Execute( graph, options );
		    done.store( true );
	    } );
	const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds( seconds );
	while ( !done.load() && std::chrono::steady_clock::now() < deadline )
		std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
	if ( !done.load() )
	{
		runner.detach();
		return false;
	}
	runner.join();
	return true;
}

// One job per lane kind.
JobDesc Job( const char *name, ExecutorToken lane, JobEntry fn )
{
	JobDesc d;
	d.name = name;
	d.executor = lane;
	d.function = std::move( fn );
	return d;
}

} // namespace

int main()
{
	testing::Checks checks;

	// Bound lanes run on their runners; the unbound control stalls.
	{
		platform::ThreadTaskRunner mainRunner( "main" );
		platform::ThreadTaskRunner ioThread( "io" );
		platform::SequencedTaskRunner ioSequence( ioThread );
		std::atomic<int> mainOnRunner{ 0 }, mainElsewhere{ 0 }, ioInSequence{ 0 }, ioElsewhere{ 0 };
		const std::thread::id caller = std::this_thread::get_id();
		std::atomic<int> onCaller{ 0 };
		JobGraphBuilder b;
		for ( int i = 0; i < 8; ++i )
		{
			b.AddJob( Job( "ui", Executor::MainThread(),
			    [&]( JobRunContext & )
			    {
				    ( mainRunner.BelongsToCurrentThread() ? mainOnRunner : mainElsewhere )
				        .fetch_add( 1 );
				    onCaller += std::this_thread::get_id() == caller;
			    } ) );
			b.AddJob( Job( "io", Executor::BlockingIO(),
			    [&]( JobRunContext & )
			    {
				    ( ioSequence.RunsTasksInCurrentSequence() ? ioInSequence : ioElsewhere )
				        .fetch_add( 1 );
				    onCaller += std::this_thread::get_id() == caller;
			    } ) );
		}
		SealedGraph g = MustSeal( b );

		RunOptions unbound;
		unbound.pumpMainThread = false;
		ParallelExecutor control( 2, 0 );
		RunResult stalled;
		checks.That( ExecuteWithin( control, g, unbound, stalled ) && stalled.stalled &&
		                 stalled.unresolved == 16,
		    "control.unbound-lanes-stall" );

		RunOptions bound = unbound;
		bound.mainThreadRunner = &mainRunner;
		bound.blockingRunner = &ioSequence;
		ParallelExecutor executor( 2, 0 );
		for ( int round = 0; round < 3; ++round ) // the first run and reused workers
		{
			RunResult result;
			const bool returned = ExecuteWithin( executor, g, bound, result );
			checks.That(
			    returned && result.AllSucceeded() && result.executed == 16, "bound.all-succeed" );
		}
		checks.Equal( mainOnRunner.load(), 24, "bound.main-jobs-on-main-runner" );
		checks.Equal( mainElsewhere.load(), 0, "bound.main-jobs-nowhere-else" );
		checks.Equal( ioInSequence.load(), 24, "bound.io-jobs-in-io-sequence" );
		checks.Equal( ioElsewhere.load(), 0, "bound.io-jobs-nowhere-else" );
		checks.Equal( onCaller.load(), 0, "bound.nothing-on-caller" );
	}

	// Dependencies across lanes, with plain data handed from job to job.
	{
		platform::ThreadTaskRunner mainRunner( "main" );
		platform::ThreadTaskRunner io( "io" );
		int stage[5] = {}; // not atomic: each job reads its producer's write
		JobGraphBuilder b;
		JobHandle h0 = b.AddJob( Job( "load", Executor::BlockingIO(),
		    [&]( JobRunContext & )
		    {
			    stage[0] = 1;
		    } ) );
		JobHandle h1 = b.AddJob( Job( "decode", Executor::Compute(),
		    [&]( JobRunContext & )
		    {
			    stage[1] = stage[0] + 1;
		    } ) );
		JobHandle h2 = b.AddJob( Job( "upload", Executor::MainThread(),
		    [&]( JobRunContext & )
		    {
			    stage[2] = stage[1] + 1;
		    } ) );
		JobHandle h3 = b.AddJob( Job( "save", Executor::BlockingIO(),
		    [&]( JobRunContext & )
		    {
			    stage[3] = stage[2] + 1;
		    } ) );
		JobHandle h4 = b.AddJob( Job( "report", Executor::Compute(),
		    [&]( JobRunContext & )
		    {
			    stage[4] = stage[3] + 1;
		    } ) );
		b.AddDependency( h0, h1 );
		b.AddDependency( h1, h2 );
		b.AddDependency( h2, h3 );
		b.AddDependency( h3, h4 );
		SealedGraph g = MustSeal( b );
		RunOptions options;
		options.pumpMainThread = false;
		options.mainThreadRunner = &mainRunner;
		options.blockingRunner = &io;
		ParallelExecutor executor( 3, 0 );
		RunResult result;
		checks.That( ExecuteWithin( executor, g, options, result ) && result.AllSucceeded(),
		    "chain.succeeds" );
		checks.Equal( stage[4], 5, "chain.publishes-across-lanes" );
	}

	// A caller that is the main runner services the lane itself (it cannot wait
	// for its own thread).
	{
		platform::ThreadTaskRunner mainRunner( "main" );
		std::atomic<int> onRunnerThread{ 0 };
		std::atomic<bool> finished{ false };
		RunResult result;
		(void)mainRunner.PostTask(
		    [&]
		    {
			    JobGraphBuilder b;
			    for ( int i = 0; i < 4; ++i )
				    b.AddJob( Job( "ui", Executor::MainThread(),
				        [&]( JobRunContext & )
				        {
					        onRunnerThread += mainRunner.BelongsToCurrentThread();
				        } ) );
			    SealedGraph g = MustSeal( b );
			    RunOptions options;
			    options.pumpMainThread = false;
			    options.mainThreadRunner = &mainRunner;
			    ParallelExecutor executor( 2, 0 );
			    result = executor.Execute( g, options );
			    finished.store( true );
		    } );
		const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds( 30 );
		while ( !finished.load() && std::chrono::steady_clock::now() < deadline )
			std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
		checks.That( finished.load() && result.AllSucceeded(), "self.caller-is-runner-completes" );
		checks.Equal( onRunnerThread.load(), 4, "self.jobs-on-runner-thread" );
		if ( !finished.load() )
			return checks.Report(); // the runner thread is wedged; do not join it
	}

	// A refusing runner: the bound job and its dependent are unresolved, and the
	// run returns.
	{
		platform::ThreadTaskRunner io( "io" );
		io.Shutdown();
		std::atomic<int> ran{ 0 };
		JobGraphBuilder b;
		JobHandle load = b.AddJob( Job( "load", Executor::BlockingIO(),
		    [&]( JobRunContext & )
		    {
			    ran++;
		    } ) );
		JobHandle use = b.AddJob( Job( "use", Executor::Compute(),
		    [&]( JobRunContext & )
		    {
			    ran++;
		    } ) );
		b.AddJob( Job( "free", Executor::Compute(),
		    [&]( JobRunContext & )
		    {
			    ran++;
		    } ) );
		b.AddDependency( load, use );
		SealedGraph g = MustSeal( b );
		RunOptions options;
		options.pumpMainThread = false;
		options.blockingRunner = &io;
		ParallelExecutor executor( 2, 0 );
		RunResult result;
		checks.That( ExecuteWithin( executor, g, options, result ) && result.stalled &&
		                 result.unresolved == 2 && ran.load() == 1,
		    "refused.stalls-and-returns" );
	}

	// A runner that accepts and then drops the job at shutdown.
	{
		platform::VirtualClock clock;
		platform::ManualTaskRunner io( clock ); // runs only when driven: never here
		std::atomic<int> ran{ 0 };
		JobGraphBuilder b;
		b.AddJob( Job( "load", Executor::BlockingIO(),
		    [&]( JobRunContext & )
		    {
			    ran++;
		    } ) );
		SealedGraph g = MustSeal( b );
		RunOptions options;
		options.pumpMainThread = false;
		options.blockingRunner = &io;
		ParallelExecutor executor( 2, 0 );
		RunResult result;
		std::atomic<bool> done{ false };
		std::thread run(
		    [&]
		    {
			    result = executor.Execute( g, options );
			    done.store( true );
		    } );
		while ( io.PendingCount() == 0 )
			std::this_thread::yield();
		std::this_thread::sleep_for( std::chrono::milliseconds( 5 ) );
		const bool waitedForRunner = !done.load();
		io.Shutdown();
		const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds( 30 );
		while ( !done.load() && std::chrono::steady_clock::now() < deadline )
			std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
		checks.That( waitedForRunner, "dropped.run-waits-for-runner" );
		checks.That( done.load() && result.stalled && result.unresolved == 1 && ran.load() == 0,
		    "dropped.stalls-and-returns" );
		if ( !done.load() )
		{
			run.detach();
			return checks.Report();
		}
		run.join();
	}

	// Cancellation reaches bound jobs: none runs, all are Canceled.
	{
		platform::ThreadTaskRunner mainRunner( "main" );
		platform::ThreadTaskRunner io( "io" );
		std::atomic<int> ran{ 0 };
		JobGraphBuilder b;
		b.AddJob( Job( "ui", Executor::MainThread(),
		    [&]( JobRunContext & )
		    {
			    ran++;
		    } ) );
		b.AddJob( Job( "io", Executor::BlockingIO(),
		    [&]( JobRunContext & )
		    {
			    ran++;
		    } ) );
		SealedGraph g = MustSeal( b );
		std::atomic<bool> cancel{ true };
		RunOptions options;
		options.pumpMainThread = false;
		options.mainThreadRunner = &mainRunner;
		options.blockingRunner = &io;
		options.cancel = &cancel;
		ParallelExecutor executor( 2, 0 );
		RunResult result;
		checks.That( ExecuteWithin( executor, g, options, result ) && result.canceled == 2 &&
		                 ran.load() == 0,
		    "cancel.reaches-bound-jobs" );
	}

	// Seeded random graphs over all lanes (including failures and Terminal
	// edges): terminal states equal the deterministic reference.
	{
		platform::ThreadTaskRunner mainRunner( "main" );
		platform::ThreadTaskRunner ioThread( "io" );
		platform::SequencedTaskRunner io( ioThread );
		ParallelExecutor executor( 3, 0 );
		DeterministicExecutor reference;
		std::uint32_t seed = 20260926u;
		auto next = [&seed]
		{
			seed = seed * 1664525u + 1013904223u;
			return seed >> 8;
		};
		int mismatches = 0;
		for ( int trial = 0; trial < 60; ++trial )
		{
			JobGraphBuilder b;
			const int n = 5 + static_cast<int>( next() % 20 );
			std::vector<JobHandle> handles;
			for ( int i = 0; i < n; ++i )
			{
				const unsigned lane = next() % 3;
				const bool fails = next() % 7 == 0;
				handles.push_back( b.AddJob( Job( "j",
				    lane == 0   ? Executor::Compute()
				    : lane == 1 ? Executor::MainThread()
				                : Executor::BlockingIO(),
				    [fails]( JobRunContext &ctx )
				    {
					    if ( fails )
						    ctx.Fail();
				    } ) ) );
				for ( int p = 0; p < i; ++p )
				{
					if ( next() % 5 == 0 )
						b.AddDependency( handles[p], handles[i],
						    next() % 3 == 0 ? DependencyKind::Terminal : DependencyKind::Success );
				}
			}
			SealedGraph g = MustSeal( b );
			RunOptions options;
			options.pumpMainThread = false;
			options.mainThreadRunner = &mainRunner;
			options.blockingRunner = &io;
			RunResult got;
			if ( !ExecuteWithin( executor, g, options, got ) )
			{
				checks.That( false, "random.returns" );
				return checks.Report();
			}
			const RunResult want = reference.Execute( g, RunOptions{} );
			mismatches += got.states != want.states || got.stalled;
		}
		checks.Equal( mismatches, 0, "random.equals-deterministic" );
	}

	return checks.Report();
}
