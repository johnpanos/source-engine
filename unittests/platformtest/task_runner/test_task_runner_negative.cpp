//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: platform.task_runner.sensitivity: the shared task-runner suite must
//			catch each deliberately broken provider on the clause it breaks,
//			while a conforming provider passes. Passes (exit 0) when the oracle
//			distinguishes them.
//
//=============================================================================//

#include "task_runner_conformance.h"
#include "test_pool.h"

#include "../../../platform/runners/manual_task_runner.h"
#include "../../../platform/runners/thread_task_runner.h"
#include "testing/checks.h"

#include <cstdio>
#include <cstdlib>
#include <deque>
#include <mutex>
#include <string>

namespace
{

using platform::ISequencedTaskRunner;
using platform::ManualTaskRunner;
using platform::PostResult;
using platform::Task;
using platform::VirtualClock;
using platformtest::RunnerDriver;

// A manual-style sequenced runner with a switchable defect. Tasks run when the
// driver drains; delays use virtual time.
class DefectiveRunner final : public ISequencedTaskRunner
{
public:
	enum class Defect
	{
		None,
		Lifo,                 // runs the newest task first
		IgnoresDelay,         // runs delayed tasks at once
		AcceptsAfterShutdown, // says kAccepted after shutdown and drops the task
		RunsAfterShutdown,    // shutdown keeps the queue and it still runs
		Inline,               // runs the task inside the post
	};

	explicit DefectiveRunner( Defect defect ) : m_defect( defect ) {}

	PostResult PostTask( Task task ) override { return PostDelayedTask( std::move( task ), 0 ); }
	PostResult PostDelayedTask( Task task, std::uint64_t delay ) override
	{
		if ( m_defect == Defect::Inline && delay == 0 )
		{
			m_running = true;
			task();
			m_running = false;
			return PostResult::kAccepted;
		}
		std::lock_guard lock( m_mutex );
		if ( m_shutDown )
			return m_defect == Defect::AcceptsAfterShutdown ? PostResult::kAccepted
			                                                : PostResult::kShutDown;
		const std::uint64_t due =
		    m_defect == Defect::IgnoresDelay ? m_clock.Now().ticks : m_clock.Now().ticks + delay;
		m_queue.push_back( { due, std::move( task ) } );
		return PostResult::kAccepted;
	}
	bool RunsTasksInCurrentSequence() const override { return m_running; }

	void RunUntilIdle()
	{
		while ( true )
		{
			Task task;
			{
				std::lock_guard lock( m_mutex );
				if ( m_shutDown && m_defect != Defect::RunsAfterShutdown )
					return;
				auto pick = m_queue.end();
				for ( auto it = m_queue.begin(); it != m_queue.end(); ++it )
				{
					if ( it->first > m_clock.Now().ticks )
						continue;
					if ( pick == m_queue.end() || m_defect == Defect::Lifo )
						pick = it;
					if ( m_defect != Defect::Lifo )
						break;
				}
				if ( pick == m_queue.end() )
					return;
				task = std::move( pick->second );
				m_queue.erase( pick );
			}
			m_running = true;
			task();
			m_running = false;
		}
	}
	void AdvanceBy( std::uint64_t ns )
	{
		m_clock.AdvanceBy( ns );
		RunUntilIdle();
	}
	void Shutdown()
	{
		std::deque<std::pair<std::uint64_t, Task>> dropped;
		std::lock_guard lock( m_mutex );
		m_shutDown = true;
		if ( m_defect != Defect::RunsAfterShutdown )
			dropped.swap( m_queue );
	}
	VirtualClock &Clock() { return m_clock; }

private:
	Defect m_defect;
	VirtualClock m_clock;
	std::mutex m_mutex;
	std::deque<std::pair<std::uint64_t, Task>> m_queue;
	bool m_shutDown = false;
	bool m_running = false;
};

class DefectiveDriver final : public RunnerDriver
{
public:
	explicit DefectiveDriver( DefectiveRunner::Defect defect ) : runner( defect ) {}
	platform::ITaskRunner &Runner() override { return runner; }
	platform::ISequencedTaskRunner *Sequenced() override { return &runner; }
	void Drain() override { runner.RunUntilIdle(); }
	void Elapse( std::uint64_t ns ) override { runner.AdvanceBy( ns ); }
	std::uint64_t NowNanoseconds() override { return runner.Clock().Now().ticks; }
	void Shutdown() override { runner.Shutdown(); }
	DefectiveRunner runner;
};

// Claims a sequence but hands every task straight to a 4-thread pool.
class OverlappingSequence final : public ISequencedTaskRunner
{
public:
	PostResult PostTask( Task task ) override { return pool.PostTask( std::move( task ) ); }
	PostResult PostDelayedTask( Task task, std::uint64_t delay ) override
	{
		return pool.PostDelayedTask( std::move( task ), delay );
	}
	bool RunsTasksInCurrentSequence() const override { return true; }
	platformtest::TestPool pool{ 4 };
};

class OverlappingDriver final : public RunnerDriver
{
public:
	platform::ITaskRunner &Runner() override { return runner; }
	platform::ISequencedTaskRunner *Sequenced() override { return &runner; }
	void Drain() override { runner.pool.Drain(); }
	void Elapse( std::uint64_t ns ) override
	{
		std::this_thread::sleep_for(
		    std::chrono::nanoseconds( ns ) + std::chrono::milliseconds( 5 ) );
		Drain();
	}
	std::uint64_t NowNanoseconds() override
	{
		return static_cast<std::uint64_t>( std::chrono::duration_cast<std::chrono::nanoseconds>(
		    std::chrono::steady_clock::now().time_since_epoch() )
		        .count() );
	}
	void Shutdown() override { runner.pool.Shutdown(); }
	OverlappingSequence runner;
};

// A thread runner that claims every thread as its own.
class EveryThreadRunner final : public platform::ISingleThreadTaskRunner
{
public:
	PostResult PostTask( Task task ) override { return inner.PostTask( std::move( task ) ); }
	PostResult PostDelayedTask( Task task, std::uint64_t delay ) override
	{
		return inner.PostDelayedTask( std::move( task ), delay );
	}
	bool RunsTasksInCurrentSequence() const override { return true; }
	bool BelongsToCurrentThread() const override { return true; }
	platform::ThreadTaskRunner inner{ "every-thread" };
};

class EveryThreadDriver final : public RunnerDriver
{
public:
	platform::ITaskRunner &Runner() override { return runner; }
	platform::ISequencedTaskRunner *Sequenced() override { return &runner; }
	platform::ISingleThreadTaskRunner *SingleThread() override { return &runner; }
	void Drain() override
	{
		std::atomic<bool> done{ false };
		if ( runner.PostTask(
		         [&done]
		         {
			         done.store( true );
		         } ) != PostResult::kAccepted )
			return;
		while ( !done.load() )
			std::this_thread::yield();
	}
	void Elapse( std::uint64_t ns ) override
	{
		std::this_thread::sleep_for(
		    std::chrono::nanoseconds( ns ) + std::chrono::milliseconds( 5 ) );
		Drain();
	}
	std::uint64_t NowNanoseconds() override
	{
		return static_cast<std::uint64_t>( std::chrono::duration_cast<std::chrono::nanoseconds>(
		    std::chrono::steady_clock::now().time_since_epoch() )
		        .count() );
	}
	void Shutdown() override { runner.inner.Shutdown(); }
	EveryThreadRunner runner;
};

// Runs the shared suite against a driver into a private report, and returns
// the FAIL lines it printed.
std::string FailuresOf( RunnerDriver &driver, std::uint64_t delay )
{
	char *buffer = nullptr;
	size_t size = 0;
	std::FILE *stream = open_memstream( &buffer, &size );
	{
		testing::Checks inner( stream );
		platformtest::RunTaskRunnerConformance( inner, "under-test", driver, delay );
	}
	std::fclose( stream );
	std::string out( buffer ? buffer : "", size );
	std::free( buffer );
	return out;
}

} // namespace

int main()
{
	testing::Checks checks;
	using Defect = DefectiveRunner::Defect;

	{
		DefectiveDriver control( Defect::None );
		const std::string failures = FailuresOf( control, 1000 );
		checks.That( failures.empty(), "control-passes" );
		if ( !failures.empty() )
			std::printf( "%s", failures.c_str() );
	}

	struct Case
	{
		Defect defect;
		const char *clause;
	};
	const Case cases[] = {
	    { Defect::Lifo, "under-test.sequence-post-order" },
	    { Defect::IgnoresDelay, "under-test.delayed-not-early" },
	    { Defect::AcceptsAfterShutdown, "under-test.post-after-shutdown-refused" },
	    { Defect::RunsAfterShutdown, "under-test.nothing-runs-after-shutdown" },
	    { Defect::Inline, "under-test.never-inline" },
	};
	for ( const Case &c : cases )
	{
		DefectiveDriver driver( c.defect );
		checks.That( FailuresOf( driver, 1000 ).find( c.clause ) != std::string::npos,
		    std::string( "detects " ) + c.clause );
	}
	{
		OverlappingDriver driver;
		checks.That( FailuresOf( driver, 20000000 ).find( "under-test.sequence-no-overlap" ) !=
		                 std::string::npos,
		    "detects under-test.sequence-no-overlap" );
	}
	{
		EveryThreadDriver driver;
		checks.That( FailuresOf( driver, 20000000 )
		                     .find( "under-test.belonging-outside-only-on-runner-thread" ) !=
		                 std::string::npos,
		    "detects under-test.belonging-outside-only-on-runner-thread" );
	}
	return checks.Report();
}
