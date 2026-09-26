//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: platform.sequence_checker (RFC 0001 rank 11 "diagnostic sequence
//			checks"; roadmap R10, R10-SEQCHECK). One predicate states what a
//			sequence-affinity checker must accept and reject; SequenceChecker
//			must pass it and a thread-only checker (the common mistake) must
//			fail its cross-thread sequence clause. The diagnostic macro must
//			abort a child process that breaks the rule.
//
//=============================================================================//

#include "platform/contracts/sequence_checker.h"
#include "test_pool.h"

#include "../../../platform/runners/manual_task_runner.h"
#include "../../../platform/runners/sequenced_task_runner.h"
#include "../../../platform/runners/thread_task_runner.h"
#include "testing/checks.h"

#include <sys/wait.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstring>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <thread>

namespace
{

using platform::PostResult;

// The broken alternative: remembers only the thread.
class ThreadOnlyChecker
{
public:
	bool CalledOnValidSequence() const
	{
		std::lock_guard lock( m_mutex );
		if ( !m_bound )
		{
			m_bound = true;
			m_thread = std::this_thread::get_id();
			return true;
		}
		return m_thread == std::this_thread::get_id();
	}
	void Detach() { m_bound = false; }

private:
	mutable std::mutex m_mutex;
	mutable bool m_bound = false;
	mutable std::thread::id m_thread;
};

void WaitFor( platform::ITaskRunner &runner )
{
	auto done = std::make_shared<std::atomic<bool>>( false );
	if ( runner.PostTask(
	         [done]
	         {
		         done->store( true );
	         } ) != PostResult::kAccepted )
		return;
	while ( !done->load() )
		std::this_thread::sleep_for( std::chrono::microseconds( 50 ) );
}

template <typename Checker>
void RunCheckerPredicate( testing::Checks &checks, const std::string &name )
{
	auto check = [&]( bool ok, const char *clause )
	{
		checks.That( ok, name + "." + clause );
	};

	// One sequence over a 4-thread pool: every task is on the sequence, even
	// though the tasks run on different threads.
	{
		platformtest::TestPool pool( 4 );
		platform::SequencedTaskRunner sequence( pool );
		platform::SequencedTaskRunner other( pool );
		Checker checker;
		std::atomic<int> invalid{ 0 };
		std::mutex threadsMutex;
		std::set<std::thread::id> threads;
		for ( int i = 0; i < 400; ++i )
		{
			(void)sequence.PostTask(
			    [&]
			    {
				    invalid += checker.CalledOnValidSequence() ? 0 : 1;
				    std::lock_guard lock( threadsMutex );
				    threads.insert( std::this_thread::get_id() );
				    std::this_thread::yield();
			    } );
		}
		WaitFor( sequence );
		check( invalid.load() == 0, "valid-across-threads-of-one-sequence" );
		check( threads.size() > 1, "sequence-used-several-threads" );

		std::atomic<int> otherValid{ 0 };
		(void)other.PostTask(
		    [&]
		    {
			    otherValid += checker.CalledOnValidSequence() ? 1 : 0;
		    } );
		WaitFor( other );
		check( otherValid.load() == 0, "invalid-on-another-sequence" );
		check( !checker.CalledOnValidSequence(), "invalid-outside-the-sequence" );

		// Detach hands the state to a new owner: the next check binds.
		checker.Detach();
		check( checker.CalledOnValidSequence(), "detach-rebinds" );
		std::atomic<int> afterDetach{ 1 };
		(void)sequence.PostTask(
		    [&]
		    {
			    afterDetach = checker.CalledOnValidSequence() ? 1 : 0;
		    } );
		WaitFor( sequence );
		check( afterDetach.load() == 0, "detached-then-bound-to-thread" );
	}

	// Bound to a thread outside tasks: valid on that thread only.
	{
		Checker checker;
		check( checker.CalledOnValidSequence(), "thread-binds" );
		check( checker.CalledOnValidSequence(), "thread-stays-valid" );
		std::atomic<bool> fromOther{ true };
		std::thread other(
		    [&]
		    {
			    fromOther = checker.CalledOnValidSequence();
		    } );
		other.join();
		check( !fromOther.load(), "invalid-on-another-thread" );
	}

	// Single-thread runners: their sequence is their thread.
	{
		platform::ThreadTaskRunner runner( "checker" );
		Checker checker;
		std::atomic<int> valid{ 0 };
		for ( int i = 0; i < 10; ++i )
			(void)runner.PostTask(
			    [&]
			    {
				    valid += checker.CalledOnValidSequence() ? 1 : 0;
			    } );
		WaitFor( runner );
		check( valid.load() == 10, "valid-on-single-thread-runner" );
		check( !checker.CalledOnValidSequence(), "invalid-off-single-thread-runner" );
	}

	// A manual runner's tasks run on the driving thread but are its sequence,
	// not that thread.
	{
		platform::VirtualClock clock;
		platform::ManualTaskRunner runner( clock );
		Checker checker;
		bool inTask = false;
		(void)runner.PostTask(
		    [&]
		    {
			    inTask = checker.CalledOnValidSequence();
		    } );
		runner.RunUntilIdle();
		check( inTask, "binds-inside-manual-task" );
		check( !checker.CalledOnValidSequence(), "driving-thread-is-not-the-sequence" );
	}
}

// The diagnostic macro aborts a process that breaks the rule.
bool MacroAbortsOffSequence()
{
	const pid_t child = fork();
	if ( child == 0 )
	{
		platform::SequenceChecker checker;
		PLATFORM_CHECK_SEQUENCE( checker ); // binds to this thread
		std::thread other(
		    [&]
		    {
			    PLATFORM_CHECK_SEQUENCE( checker );
		    } );
		other.join();
		_exit( 0 ); // the check did not fire
	}
	int status = 0;
	waitpid( child, &status, 0 );
	return WIFSIGNALED( status ) && WTERMSIG( status ) == SIGABRT;
}

} // namespace

int main()
{
	testing::Checks checks;
	RunCheckerPredicate<platform::SequenceChecker>( checks, "checker" );

	// The negative control: a thread-only checker must fail the cross-thread
	// sequence clause (reported to a private sink, then asserted here).
	{
		char *buffer = nullptr;
		size_t size = 0;
		std::FILE *sink = open_memstream( &buffer, &size );
		{
			testing::Checks inner( sink );
			RunCheckerPredicate<ThreadOnlyChecker>( inner, "thread-only" );
		}
		std::fclose( sink );
		const std::string failures( buffer ? buffer : "", size );
		std::free( buffer );
		checks.That( failures.find( "thread-only.valid-across-threads-of-one-sequence" ) !=
		                 std::string::npos,
		    "negative.thread-only-checker-rejected" );
	}

	checks.That( PLATFORM_SEQUENCE_CHECKS == 1, "macro.enabled-in-this-build" );
	checks.That( MacroAbortsOffSequence(), "macro.aborts-off-sequence" );
	return checks.Report();
}
