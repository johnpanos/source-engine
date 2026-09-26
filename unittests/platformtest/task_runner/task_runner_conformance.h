//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Shared conformance suite for platform.task-runner.v1
//			(public/platform/contracts/task_runner.h). Every provider that
//			claims a runner contract runs THIS predicate through a small driver
//			that knows how to let time pass and how to wait for the runner; the
//			sensitivity test feeds it deliberately broken providers.
//
//=============================================================================//

#ifndef PLATFORMTEST_TASK_RUNNER_CONFORMANCE_H
#define PLATFORMTEST_TASK_RUNNER_CONFORMANCE_H

#include "platform/contracts/task_runner.h"
#include "testing/checks.h"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace platformtest
{

// How the suite drives one provider.
class RunnerDriver
{
public:
	virtual ~RunnerDriver() = default;
	virtual platform::ITaskRunner &Runner() = 0;
	virtual platform::ISequencedTaskRunner *Sequenced() { return nullptr; }
	virtual platform::ISingleThreadTaskRunner *SingleThread() { return nullptr; }
	// Returns once every accepted task that is due now has run.
	virtual void Drain() = 0;
	// Lets `nanoseconds` of the runner's clock pass, then drains.
	virtual void Elapse( std::uint64_t nanoseconds ) = 0;
	// The runner's clock, in nanoseconds.
	virtual std::uint64_t NowNanoseconds() = 0;
	// The owner's shutdown (see the contract).
	virtual void Shutdown() = 0;
};

// Counts how a task ended: run, or destroyed without running.
struct TaskProbe
{
	std::atomic<int> ran{ 0 };
	std::atomic<int> destroyedUnrun{ 0 };
};

inline platform::Task ProbedTask( TaskProbe &probe, std::function<void()> body = {} )
{
	struct Guard
	{
		TaskProbe *probe;
		bool ran = false;
		explicit Guard( TaskProbe *p ) : probe( p ) {}
		Guard( Guard &&other ) noexcept : probe( other.probe ), ran( other.ran ) { other.probe = nullptr; }
		~Guard()
		{
			if ( probe && !ran )
				probe->destroyedUnrun.fetch_add( 1 );
		}
	};
	return [guard = Guard( &probe ), body = std::move( body )]() mutable
	{
		guard.ran = true;
		guard.probe->ran.fetch_add( 1 );
		if ( body )
			body();
	};
}

// `name` prefixes every check. `delayNanoseconds` is the delay used by the
// timing clauses (real providers need one large enough to measure).
inline void RunTaskRunnerConformance( testing::Checks &checks, const std::string &name,
    RunnerDriver &driver, std::uint64_t delayNanoseconds )
{
	using platform::PostResult;
	auto check = [&]( bool ok, const char *clause ) { checks.That( ok, name + "." + clause ); };
	platform::ITaskRunner &runner = driver.Runner();
	platform::ISequencedTaskRunner *sequenced = driver.Sequenced();
	platform::ISingleThreadTaskRunner *single = driver.SingleThread();

	// 1. Exactly once.
	{
		constexpr int kTasks = 200;
		std::vector<std::atomic<int>> counts( kTasks );
		bool accepted = true;
		for ( int i = 0; i < kTasks; ++i )
			accepted = accepted && runner.PostTask( [&counts, i] { counts[i].fetch_add( 1 ); } ) ==
			                           PostResult::kAccepted;
		driver.Drain();
		bool once = true;
		for ( auto &count : counts )
			once = once && count.load() == 1;
		check( accepted, "posts-accepted" );
		check( once, "exactly-once" );
	}

	// 2. Never inside the post, also when posting from a task: a task that
	// runs on the posting thread while that thread is still in the post ran
	// inline.
	{
		std::atomic<bool> inPost{ false };
		std::atomic<int> inlineRuns{ 0 };
		std::atomic<int> ran{ 0 };
		const std::thread::id poster = std::this_thread::get_id();
		inPost.store( true );
		const PostResult posted = runner.PostTask(
		    [&]
		    {
			    if ( inPost.load() && std::this_thread::get_id() == poster )
				    inlineRuns.fetch_add( 1 );
			    ran.fetch_add( 1 );
		    } );
		inPost.store( false );
		(void)runner.PostTask(
		    [&]
		    {
			    const std::thread::id outer = std::this_thread::get_id();
			    auto flag = std::make_shared<std::atomic<bool>>( true );
			    (void)runner.PostTask(
			        [&inlineRuns, &ran, outer, flag]
			        {
				        if ( flag->load() && std::this_thread::get_id() == outer )
					        inlineRuns.fetch_add( 1 );
				        ran.fetch_add( 1 );
			        } );
			    flag->store( false );
		    } );
		driver.Drain();
		driver.Drain();
		check( posted == PostResult::kAccepted && ran.load() == 2, "posted-tasks-run" );
		check( inlineRuns.load() == 0, "never-inline" );
	}

	// 3. Delayed tasks: not early, and they do run.
	{
		std::atomic<std::uint64_t> ranAt{ 0 };
		std::atomic<bool> ran{ false };
		const std::uint64_t postedAt = driver.NowNanoseconds();
		const PostResult posted = runner.PostDelayedTask(
		    [&]
		    {
			    ranAt.store( driver.NowNanoseconds() );
			    ran.store( true );
		    },
		    delayNanoseconds );
		driver.Drain();
		const bool early = ran.load() && ranAt.load() - postedAt < delayNanoseconds;
		driver.Elapse( delayNanoseconds );
		driver.Drain();
		check( posted == PostResult::kAccepted, "delayed-accepted" );
		check( !early && ran.load() && ranAt.load() - postedAt >= delayNanoseconds, "delayed-not-early" );

		// A zero delay is a plain post.
		std::atomic<bool> zero{ false };
		(void)runner.PostDelayedTask( [&zero] { zero.store( true ); }, 0 );
		driver.Drain();
		check( zero.load(), "zero-delay-runs" );
	}

	if ( sequenced )
	{
		// 4. Post order, one at a time, and publication between tasks.
		constexpr int kTasks = 400;
		std::mutex orderMutex;
		std::vector<int> order;
		std::atomic<int> inFlight{ 0 };
		std::atomic<int> overlaps{ 0 };
		std::atomic<int> outsideSequence{ 0 };
		int plain = 0; // not atomic: tasks of one sequence must publish to each other
		for ( int i = 0; i < kTasks; ++i )
		{
			(void)runner.PostTask(
			    [&, i]
			    {
				    if ( inFlight.fetch_add( 1 ) != 0 )
					    overlaps.fetch_add( 1 );
				    if ( !sequenced->RunsTasksInCurrentSequence() )
					    outsideSequence.fetch_add( 1 );
				    for ( int spin = 0; spin < 50; ++spin )
					    std::this_thread::yield();
				    ++plain;
				    {
					    std::lock_guard lock( orderMutex );
					    order.push_back( i );
				    }
				    inFlight.fetch_sub( 1 );
			    } );
		}
		driver.Drain();
		bool inOrder = static_cast<int>( order.size() ) == kTasks;
		for ( int i = 0; inOrder && i < kTasks; ++i )
			inOrder = order[i] == i;
		check( inOrder, "sequence-post-order" );
		check( overlaps.load() == 0, "sequence-no-overlap" );
		check( plain == kTasks, "sequence-publishes" );
		check( outsideSequence.load() == 0, "in-sequence-inside-task" );
		check( !sequenced->RunsTasksInCurrentSequence(), "not-in-sequence-outside" );
	}

	if ( single )
	{
		// 5. One physical thread.
		std::mutex idsMutex;
		std::vector<std::thread::id> ids;
		std::atomic<int> notBelonging{ 0 };
		for ( int i = 0; i < 20; ++i )
		{
			(void)runner.PostTask(
			    [&]
			    {
				    if ( !single->BelongsToCurrentThread() )
					    notBelonging.fetch_add( 1 );
				    std::lock_guard lock( idsMutex );
				    ids.push_back( std::this_thread::get_id() );
			    } );
		}
		driver.Drain();
		bool sameThread = ids.size() == 20;
		for ( const auto &id : ids )
			sameThread = sameThread && id == ids.front();
		check( sameThread && notBelonging.load() == 0, "single-thread" );
		check( !single->BelongsToCurrentThread(), "not-belonging-outside" );
	}

	// 6. Shutdown: queued and delayed tasks are destroyed unrun; nothing runs
	// afterwards; later posts are refused and destroyed.
	{
		TaskProbe queued;
		TaskProbe delayed;
		constexpr int kQueued = 500;
		for ( int i = 0; i < kQueued; ++i )
			(void)runner.PostTask( ProbedTask( queued ) );
		(void)runner.PostDelayedTask( ProbedTask( delayed ), 3600ull * 1000000000ull );
		driver.Shutdown();
		const int ranAtShutdown = queued.ran.load();
		driver.Elapse( delayNanoseconds );
		check( queued.ran.load() == ranAtShutdown, "nothing-runs-after-shutdown" );
		check( queued.ran.load() + queued.destroyedUnrun.load() == kQueued, "shutdown-accounts-every-task" );
		check( delayed.ran.load() == 0 && delayed.destroyedUnrun.load() == 1, "shutdown-drops-delayed" );

		TaskProbe late;
		const PostResult refused = runner.PostTask( ProbedTask( late ) );
		const PostResult refusedDelayed = runner.PostDelayedTask( ProbedTask( late ), 1 );
		driver.Elapse( delayNanoseconds );
		check( refused == PostResult::kShutDown && refusedDelayed == PostResult::kShutDown,
		    "post-after-shutdown-refused" );
		check( late.ran.load() == 0 && late.destroyedUnrun.load() == 2, "refused-task-destroyed" );
	}
}

} // namespace platformtest

#endif // PLATFORMTEST_TASK_RUNNER_CONFORMANCE_H
