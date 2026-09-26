//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: platform.task_runner (RFC 0001 rank 11 / RFC 0003, roadmap R10):
//			the shared platform.task-runner.v1 suite against every provider in
//			platform/runners and the test pool:
//
//			- ManualTaskRunner on a VirtualClock (sequenced, virtual time);
//			- ThreadTaskRunner (single thread, steady clock);
//			- SequencedTaskRunner over a ThreadTaskRunner, over the 4-thread test
//			  pool, and over a ManualTaskRunner;
//			- the test pool itself (a plain, parallel runner).
//
//			Plus provider-specific clauses: virtual time moves only when
//			advanced, AdvanceBy runs each task at its due time, and a sequence
//			outlives its owner's destruction safely while base tasks are queued.
//
//=============================================================================//

#include "task_runner_conformance.h"
#include "test_pool.h"

#include "../../../platform/runners/manual_task_runner.h"
#include "../../../platform/runners/sequenced_task_runner.h"
#include "../../../platform/runners/thread_task_runner.h"
#include "testing/checks.h"

#include <atomic>
#include <chrono>
#include <cstdio>
#include <memory>
#include <thread>
#include <vector>

namespace
{

using platform::ManualTaskRunner;
using platform::PostResult;
using platform::SequencedTaskRunner;
using platform::ThreadTaskRunner;
using platform::VirtualClock;
using platformtest::RunnerDriver;

std::uint64_t SteadyNanoseconds()
{
	return static_cast<std::uint64_t>( std::chrono::duration_cast<std::chrono::nanoseconds>(
	    std::chrono::steady_clock::now().time_since_epoch() )
	        .count() );
}

// Waits for a marker posted to a sequenced runner: every task posted before it
// has run. A refused marker (after shutdown) returns at once.
void DrainSequence( platform::ITaskRunner &runner )
{
	auto done = std::make_shared<std::atomic<bool>>( false );
	if ( runner.PostTask(
	         [done]
	         {
		         done->store( true, std::memory_order_release );
	         } ) != PostResult::kAccepted )
		return;
	const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds( 30 );
	while (
	    !done->load( std::memory_order_acquire ) && std::chrono::steady_clock::now() < deadline )
		std::this_thread::sleep_for( std::chrono::microseconds( 50 ) );
}

class ManualDriver final : public RunnerDriver
{
public:
	platform::ITaskRunner &Runner() override { return runner; }
	platform::ISequencedTaskRunner *Sequenced() override { return &runner; }
	void Drain() override { runner.RunUntilIdle(); }
	void Elapse( std::uint64_t ns ) override { runner.AdvanceBy( ns ); }
	std::uint64_t NowNanoseconds() override { return clock.Now().ticks; }
	void Shutdown() override { runner.Shutdown(); }

	VirtualClock clock;
	ManualTaskRunner runner{ clock };
};

class ThreadDriver final : public RunnerDriver
{
public:
	platform::ITaskRunner &Runner() override { return runner; }
	platform::ISequencedTaskRunner *Sequenced() override { return &runner; }
	platform::ISingleThreadTaskRunner *SingleThread() override { return &runner; }
	void Drain() override { DrainSequence( runner ); }
	void Elapse( std::uint64_t ns ) override
	{
		std::this_thread::sleep_for(
		    std::chrono::nanoseconds( ns ) + std::chrono::milliseconds( 5 ) );
		Drain();
	}
	std::uint64_t NowNanoseconds() override { return SteadyNanoseconds(); }
	void Shutdown() override { runner.Shutdown(); }

	ThreadTaskRunner runner{ "conformance" };
};

// A sequence over `Base`, which the driver owns and drains.
template <typename Base> class SequenceDriver final : public RunnerDriver
{
public:
	template <typename... Args>
	explicit SequenceDriver( Args &&...args ) : base( std::forward<Args>( args )... )
	{
	}
	platform::ITaskRunner &Runner() override { return sequence; }
	platform::ISequencedTaskRunner *Sequenced() override { return &sequence; }
	void Drain() override { DrainSequence( sequence ); }
	void Elapse( std::uint64_t ns ) override
	{
		std::this_thread::sleep_for(
		    std::chrono::nanoseconds( ns ) + std::chrono::milliseconds( 5 ) );
		Drain();
	}
	std::uint64_t NowNanoseconds() override { return SteadyNanoseconds(); }
	void Shutdown() override { sequence.Shutdown(); }

	Base base;
	SequencedTaskRunner sequence{ base };
};

class SequenceOverManualDriver final : public RunnerDriver
{
public:
	platform::ITaskRunner &Runner() override { return sequence; }
	platform::ISequencedTaskRunner *Sequenced() override { return &sequence; }
	void Drain() override { base.RunUntilIdle(); }
	void Elapse( std::uint64_t ns ) override { base.AdvanceBy( ns ); }
	std::uint64_t NowNanoseconds() override { return clock.Now().ticks; }
	void Shutdown() override { sequence.Shutdown(); }

	VirtualClock clock;
	ManualTaskRunner base{ clock };
	SequencedTaskRunner sequence{ base };
};

class PoolDriver final : public RunnerDriver
{
public:
	platform::ITaskRunner &Runner() override { return pool; }
	void Drain() override { pool.Drain(); }
	void Elapse( std::uint64_t ns ) override
	{
		std::this_thread::sleep_for(
		    std::chrono::nanoseconds( ns ) + std::chrono::milliseconds( 5 ) );
		Drain();
	}
	std::uint64_t NowNanoseconds() override { return SteadyNanoseconds(); }
	void Shutdown() override { pool.Shutdown(); }

	platformtest::TestPool pool{ 4 };
};

constexpr std::uint64_t kVirtualDelay = 1000000; // 1 ms of virtual time
constexpr std::uint64_t kRealDelay = 20000000;   // 20 ms

} // namespace

int main()
{
	testing::Checks checks;
	{
		ManualDriver driver;
		platformtest::RunTaskRunnerConformance( checks, "manual", driver, kVirtualDelay );
	}
	{
		ThreadDriver driver;
		platformtest::RunTaskRunnerConformance( checks, "thread", driver, kRealDelay );
	}
	{
		SequenceDriver<ThreadTaskRunner> driver( "sequence-base" );
		platformtest::RunTaskRunnerConformance(
		    checks, "sequence-over-thread", driver, kRealDelay );
	}
	{
		SequenceDriver<platformtest::TestPool> driver( 4 );
		platformtest::RunTaskRunnerConformance( checks, "sequence-over-pool", driver, kRealDelay );
	}
	{
		SequenceOverManualDriver driver;
		platformtest::RunTaskRunnerConformance(
		    checks, "sequence-over-manual", driver, kVirtualDelay );
	}
	{
		PoolDriver driver;
		platformtest::RunTaskRunnerConformance( checks, "pool", driver, kRealDelay );
	}

	// Virtual time moves only when the owner advances it, and AdvanceBy runs
	// each delayed task at its own due time, in due order.
	{
		VirtualClock clock;
		ManualTaskRunner runner( clock );
		std::vector<std::uint64_t> seen;
		(void)runner.PostDelayedTask(
		    [&]
		    {
			    seen.push_back( clock.Now().ticks );
		    },
		    300 );
		(void)runner.PostDelayedTask(
		    [&]
		    {
			    seen.push_back( clock.Now().ticks );
		    },
		    100 );
		(void)runner.PostDelayedTask(
		    [&]
		    {
			    seen.push_back( clock.Now().ticks );
		    },
		    200 );
		checks.Equal( runner.RunUntilIdle(), 0, "manual.nothing-due-without-advance" );
		checks.Equal( clock.Now().ticks, std::uint64_t( 0 ), "manual.clock-still" );
		checks.Equal( runner.AdvanceBy( 1000 ), 3, "manual.advance-runs-all" );
		checks.Equal(
		    seen, std::vector<std::uint64_t>{ 100, 200, 300 }, "manual.runs-at-due-times" );
		checks.Equal( clock.Now().ticks, std::uint64_t( 1000 ), "manual.advance-reaches-target" );

		// A task posted with a delay by a task runs at post time plus delay.
		seen.clear();
		(void)runner.PostTask(
		    [&]
		    {
			    (void)runner.PostDelayedTask(
			        [&]
			        {
				        seen.push_back( clock.Now().ticks );
			        },
			        50 );
		    } );
		runner.AdvanceBy( 100 );
		checks.Equal( seen, std::vector<std::uint64_t>{ 1050 }, "manual.chained-delay" );
	}

	// Destroying a sequence while its base task is queued is safe: the base
	// task finds the sequence shut down and runs nothing.
	{
		VirtualClock clock;
		ManualTaskRunner base( clock );
		platformtest::TaskProbe probe;
		{
			SequencedTaskRunner sequence( base );
			(void)sequence.PostTask( platformtest::ProbedTask( probe ) );
		}
		checks.Equal( base.PendingCount(), size_t( 1 ), "sequence.base-task-outlives-sequence" );
		base.RunUntilIdle();
		checks.That( probe.ran.load() == 0 && probe.destroyedUnrun.load() == 1,
		    "sequence.destroyed-sequence-runs-nothing" );
	}

	// A base runner that refuses the sequence's post shuts the sequence down.
	{
		VirtualClock clock;
		ManualTaskRunner base( clock );
		SequencedTaskRunner sequence( base );
		base.Shutdown();
		platformtest::TaskProbe probe;
		checks.That(
		    sequence.PostTask( platformtest::ProbedTask( probe ) ) == PostResult::kShutDown &&
		        probe.destroyedUnrun.load() == 1,
		    "sequence.base-refusal-refuses" );
		checks.That(
		    sequence.PostTask( [] {} ) == PostResult::kShutDown, "sequence.stays-shut-down" );
	}

	// Concurrent shutdowns of a thread runner both return after the join.
	{
		ThreadTaskRunner runner( "double-shutdown" );
		std::atomic<bool> release{ false };
		(void)runner.PostTask(
		    [&release]
		    {
			    while ( !release.load() )
				    std::this_thread::yield();
		    } );
		std::thread other(
		    [&runner]
		    {
			    runner.Shutdown();
		    } );
		std::this_thread::sleep_for( std::chrono::milliseconds( 5 ) );
		release.store( true );
		runner.Shutdown();
		other.join();
		checks.That(
		    runner.PostTask( [] {} ) == PostResult::kShutDown, "thread.concurrent-shutdown" );
	}

	// Once its thread has exited, no thread belongs to a thread runner, even a
	// new thread that reuses the exited thread's id.
	{
		ThreadTaskRunner runner( "exited" );
		std::atomic<std::thread::id> ranOn{};
		(void)runner.PostTask(
		    [&ranOn]
		    {
			    ranOn.store( std::this_thread::get_id() );
		    } );
		DrainSequence( runner );
		runner.Shutdown();
		int reused = 0;
		int claimed = 0;
		for ( int i = 0; i < 64; ++i )
		{
			std::thread probe(
			    [&]
			    {
				    reused += std::this_thread::get_id() == ranOn.load();
				    claimed +=
				        runner.BelongsToCurrentThread() || runner.RunsTasksInCurrentSequence();
			    } );
			probe.join();
		}
		checks.Equal( claimed, 0, "thread.no-owner-after-shutdown" );
		std::printf(
		    "thread.no-owner-after-shutdown: %d of 64 probe threads reused the id\n", reused );
	}

	return checks.Report();
}
