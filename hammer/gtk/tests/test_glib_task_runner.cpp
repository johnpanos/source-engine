//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The shared platform.task-runner.v1 suite against the GTK shell's
//			GlibTaskRunner (RFC 0002 / R08-ASYNC-BUILD), on a private GLib main
//			context iterated by this thread, plus a cross-thread post: a task
//			posted from another thread runs on the context's thread.
//
//			Build and run: hammer/gtk/tests/glib_task_runner.sh [OUT_DIR]
//
//=============================================================================//

#include "../glib_task_runner.h"
#include "task_runner_conformance.h"
#include "testing/checks.h"

#include <glib.h>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>
#include <thread>

namespace
{

std::uint64_t SteadyNanoseconds()
{
	return static_cast<std::uint64_t>( std::chrono::duration_cast<std::chrono::nanoseconds>(
	    std::chrono::steady_clock::now().time_since_epoch() )
	                                       .count() );
}

class GlibDriver final : public platformtest::RunnerDriver
{
public:
	GlibDriver() : context( g_main_context_new() ), runner( context ) {}
	~GlibDriver() override
	{
		runner.Shutdown();
		g_main_context_unref( context );
	}

	platform::ITaskRunner &Runner() override { return runner; }
	platform::ISequencedTaskRunner *Sequenced() override { return &runner; }
	platform::ISingleThreadTaskRunner *SingleThread() override { return &runner; }
	void Drain() override
	{
		auto done = std::make_shared<bool>( false );
		if ( runner.PostTask( [done] { *done = true; } ) != platform::PostResult::kAccepted )
			return;
		while ( !*done )
			g_main_context_iteration( context, TRUE );
	}
	void Elapse( std::uint64_t ns ) override
	{
		const std::uint64_t until = SteadyNanoseconds() + ns + 5000000;
		while ( SteadyNanoseconds() < until )
		{
			g_main_context_iteration( context, FALSE );
			std::this_thread::sleep_for( std::chrono::microseconds( 200 ) );
		}
		Drain();
	}
	std::uint64_t NowNanoseconds() override { return SteadyNanoseconds(); }
	void Shutdown() override { runner.Shutdown(); }
	bool DriverIsRunnerThread() const override { return true; }

	GMainContext *context;
	hammer::gtk::GlibTaskRunner runner;
};

} // namespace

int main()
{
	testing::Checks checks;
	{
		GlibDriver driver;
		platformtest::RunTaskRunnerConformance( checks, "glib", driver, 20000000 );
	}
	{
		GlibDriver driver;
		std::atomic<bool> ranOnOwner{ false };
		std::atomic<bool> ran{ false };
		std::thread poster(
		    [&]
		    {
			    (void)driver.runner.PostTask(
			        [&]
			        {
				        ranOnOwner.store( driver.runner.BelongsToCurrentThread() );
				        ran.store( true );
			        } );
		    } );
		poster.join();
		const std::uint64_t until = SteadyNanoseconds() + 5000000000ull;
		while ( !ran.load() && SteadyNanoseconds() < until )
			g_main_context_iteration( driver.context, FALSE );
		checks.That( ran.load() && ranOnOwner.load(), "glib.cross-thread-post-runs-on-owner" );
	}
	return checks.Report();
}
