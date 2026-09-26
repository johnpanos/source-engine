//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.app.map_build_queue (RFC 0002 / roadmap R08, R08-ASYNC-BUILD;
//			the first product consumer of platform.task-runner.v1, R10): builds
//			run on the work runner, results arrive on the reply runner, one
//			build at a time, and a queue destroyed in flight drops its reply.
//			Driven on virtual time (ManualTaskRunner), plus one real-thread case
//			showing Start does not wait for the compile.
//
//=============================================================================//

#include "hammer/app/map_build_queue.h"
#include "testing/checks.h"

#include "../../../platform/runners/manual_task_runner.h"
#include "../../../platform/runners/thread_task_runner.h"

#include <atomic>
#include <chrono>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace
{

using hammer::app::BuildStart;
using hammer::app::MapBuildQueue;
using hammer::ports::MapBuildRequest;
using hammer::ports::MapBuildResult;

class FakeBuilder final : public hammer::ports::IMapBuilder
{
public:
	MapBuildResult Build( const MapBuildRequest &request ) override
	{
		if ( runner )
			onWorkRunner += runner->RunsTasksInCurrentSequence() ? 1 : 0;
		requests.push_back( request.vmfPath + ( request.fullQuality ? " full" : " fast" ) +
		                    ( request.publish ? " publish" : "" ) );
		if ( delay.count() )
			std::this_thread::sleep_for( delay );
		return MapBuildResult{ true, "pass", "built " + request.vmfPath };
	}

	platform::ISequencedTaskRunner *runner = nullptr;
	std::chrono::milliseconds delay{ 0 };
	int onWorkRunner = 0;
	std::vector<std::string> requests;
};

// Runs `body` as a task of `reply`, as a front end on its own thread does.
template <typename F> void OnReply( platform::ManualTaskRunner &reply, F body )
{
	(void)reply.PostTask( std::move( body ) );
	reply.RunUntilIdle();
}

} // namespace

int main()
{
	testing::Checks checks;

	{
		platform::VirtualClock clock;
		platform::ManualTaskRunner work( clock );
		platform::ManualTaskRunner reply( clock );
		FakeBuilder builder;
		builder.runner = &work;
		MapBuildQueue queue( builder, work, reply );

		std::vector<std::string> results;
		int doneOnReply = 0;
		auto done = [&]( const MapBuildResult &r )
		{
			results.push_back( r.status + ": " + r.detail );
			doneOnReply += reply.RunsTasksInCurrentSequence() ? 1 : 0;
		};

		BuildStart first = BuildStart::kUnavailable;
		BuildStart second = BuildStart::kUnavailable;
		OnReply( reply,
		    [&]
		    {
			    first = queue.Start( MapBuildRequest{ "maps/a.vmf", false, true }, done );
			    second = queue.Start( MapBuildRequest{ "maps/b.vmf", false, false }, done );
		    } );
		checks.That( first == BuildStart::kStarted, "queue.starts" );
		checks.That( second == BuildStart::kBusy, "queue.one-at-a-time" );
		checks.That( queue.Busy(), "queue.busy-while-building" );
		checks.That( builder.requests.empty(), "queue.start-does-not-build-inline" );

		work.RunUntilIdle();
		checks.Equal( builder.requests, std::vector<std::string>{ "maps/a.vmf fast publish" },
		    "queue.builds-the-request" );
		checks.Equal( builder.onWorkRunner, 1, "queue.builds-on-work-runner" );
		checks.That( results.empty() && queue.Busy(), "queue.reply-waits-for-reply-runner" );

		reply.RunUntilIdle();
		checks.Equal( results, std::vector<std::string>{ "pass: built maps/a.vmf" }, "queue.reply-delivers" );
		checks.Equal( doneOnReply, 1, "queue.reply-on-reply-runner" );
		checks.That( !queue.Busy(), "queue.idle-after-reply" );

		BuildStart again = BuildStart::kUnavailable;
		OnReply( reply, [&] { again = queue.Start( MapBuildRequest{ "maps/c.vmf", true, false }, done ); } );
		work.RunUntilIdle();
		reply.RunUntilIdle();
		checks.That( again == BuildStart::kStarted && results.size() == 2 &&
		                 builder.requests.back() == "maps/c.vmf full",
		    "queue.next-build-after-reply" );
	}

	// Destroyed in flight: the build still finishes, the reply is dropped.
	{
		platform::VirtualClock clock;
		platform::ManualTaskRunner work( clock );
		platform::ManualTaskRunner reply( clock );
		FakeBuilder builder;
		int called = 0;
		{
			MapBuildQueue queue( builder, work, reply );
			OnReply( reply,
			    [&] { (void)queue.Start( MapBuildRequest{ "maps/a.vmf" }, [&]( const MapBuildResult & ) { ++called; } ); } );
		}
		work.RunUntilIdle();
		reply.RunUntilIdle();
		checks.That( builder.requests.size() == 1 && called == 0, "queue.destroyed-in-flight-drops-reply" );
	}

	// A shut-down work runner: the build is unavailable and the queue idle.
	{
		platform::VirtualClock clock;
		platform::ManualTaskRunner work( clock );
		platform::ManualTaskRunner reply( clock );
		FakeBuilder builder;
		MapBuildQueue queue( builder, work, reply );
		work.Shutdown();
		BuildStart start = BuildStart::kStarted;
		OnReply( reply, [&] { start = queue.Start( MapBuildRequest{ "maps/a.vmf" }, {} ); } );
		checks.That( start == BuildStart::kUnavailable && !queue.Busy(), "queue.unavailable-when-work-shut-down" );
	}

	// Real threads: Start returns while the compile is still running.
	{
		platform::VirtualClock clock;
		platform::ThreadTaskRunner work( "hammer-build" );
		platform::ManualTaskRunner reply( clock );
		FakeBuilder builder;
		builder.runner = &work;
		builder.delay = std::chrono::milliseconds( 200 );
		MapBuildQueue queue( builder, work, reply );
		std::atomic<int> done{ 0 };
		std::chrono::steady_clock::duration startTook{};
		OnReply( reply,
		    [&]
		    {
			    const auto before = std::chrono::steady_clock::now();
			    (void)queue.Start( MapBuildRequest{ "maps/a.vmf" }, [&]( const MapBuildResult & ) { ++done; } );
			    startTook = std::chrono::steady_clock::now() - before;
		    } );
		checks.That( startTook < std::chrono::milliseconds( 100 ), "threads.start-does-not-wait" );
		const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds( 10 );
		while ( done.load() == 0 && std::chrono::steady_clock::now() < deadline )
		{
			reply.RunUntilIdle();
			std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
		}
		checks.That( done.load() == 1 && builder.onWorkRunner == 1 && !queue.Busy(), "threads.reply-arrives" );
		work.Shutdown();
	}

	return checks.Report();
}
