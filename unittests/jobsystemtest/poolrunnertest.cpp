//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The engine CThreadPool as a platform.task-runner.v1 runner (RFC
//			0001 rank 11 "adapt existing queues"; roadmap R10, R10-POOLRUNNER).
//			The shared runner suite runs against the pool runner (a plain,
//			parallel runner) and against a SequencedTaskRunner over it (ordered
//			sequences on the engine pool, no new threads). A pool without
//			threads is refused, since its AddJob would run jobs inside the post.
//
//			Run: <build>/unittests/jobsystemtest/jobsystempoolrunnertest
//			     (LD_LIBRARY_PATH: tier0, vstdlib)
//
//=============================================================================//

#include "platform/contracts/task_runner.h"
#include "vstdlib/jobthread.h"
#include "vstdlib/task_runner_pool_bridge.h"

#include "../../platform/runners/sequenced_task_runner.h"
#include "../../platform/runners/thread_task_runner.h"
#include "../platformtest/task_runner/task_runner_conformance.h"
#include "testing/checks.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace
{

std::uint64_t SteadyNanoseconds()
{
	return static_cast<std::uint64_t>( std::chrono::duration_cast<std::chrono::nanoseconds>(
	    std::chrono::steady_clock::now().time_since_epoch() )
	        .count() );
}

// Forwards to the pool runner and counts its tasks, so the driver can wait
// until every task due now has run (a plain runner has no ordering to wait
// on). A task counts as finished when it runs or is destroyed unrun.
class CountingRunner final : public platform::ITaskRunner
{
public:
	explicit CountingRunner( platform::ITaskRunner &inner ) : m_inner( inner ) {}

	platform::PostResult PostTask( platform::Task task ) override
	{
		return Post( std::move( task ), 0 );
	}
	platform::PostResult PostDelayedTask( platform::Task task, std::uint64_t delay ) override
	{
		return Post( std::move( task ), delay );
	}

	void WaitDue()
	{
		std::unique_lock lock( m_mutex );
		const std::uint64_t now = SteadyNanoseconds();
		m_changed.wait( lock,
		    [&]
		    {
			    for ( const auto &entry : m_open )
				    if ( entry.second <= now )
					    return false;
			    return true;
		    } );
	}

private:
	struct Finish
	{
		CountingRunner *owner;
		std::uint64_t id;
		Finish( CountingRunner *o, std::uint64_t i ) : owner( o ), id( i ) {}
		Finish( Finish &&other ) noexcept : owner( other.owner ), id( other.id )
		{
			other.owner = nullptr;
		}
		~Finish()
		{
			if ( owner )
				owner->Close( id );
		}
	};

	platform::PostResult Post( platform::Task task, std::uint64_t delay )
	{
		std::uint64_t id = 0;
		{
			std::lock_guard lock( m_mutex );
			id = m_next++;
			m_open.emplace_back( id, SteadyNanoseconds() + delay );
		}
		platform::Task wrapped = [finish = Finish( this, id ), task = std::move( task )]() mutable
		{
			task();
		};
		return delay ? m_inner.PostDelayedTask( std::move( wrapped ), delay )
		             : m_inner.PostTask( std::move( wrapped ) );
	}

	void Close( std::uint64_t id )
	{
		{
			std::lock_guard lock( m_mutex );
			for ( auto it = m_open.begin(); it != m_open.end(); ++it )
			{
				if ( it->first == id )
				{
					m_open.erase( it );
					break;
				}
			}
		}
		m_changed.notify_all();
	}

	platform::ITaskRunner &m_inner;
	std::mutex m_mutex;
	std::condition_variable m_changed;
	std::vector<std::pair<std::uint64_t, std::uint64_t>> m_open; // id, due
	std::uint64_t m_next = 0;
};

class PoolDriver final : public platformtest::RunnerDriver
{
public:
	PoolDriver( IThreadPool *pool, platform::ITaskRunner &timer )
	    : runner( CreateThreadPoolTaskRunner( pool, &timer ) ), counting( *runner )
	{
	}
	~PoolDriver() override { DestroyThreadPoolTaskRunner( runner ); }

	platform::ITaskRunner &Runner() override { return counting; }
	void Drain() override { counting.WaitDue(); }
	void Elapse( std::uint64_t ns ) override
	{
		std::this_thread::sleep_for(
		    std::chrono::nanoseconds( ns ) + std::chrono::milliseconds( 5 ) );
		Drain();
	}
	std::uint64_t NowNanoseconds() override { return SteadyNanoseconds(); }
	void Shutdown() override { ShutdownThreadPoolTaskRunner( runner ); }

	platform::ITaskRunner *runner;
	CountingRunner counting;
};

class SequenceDriver final : public platformtest::RunnerDriver
{
public:
	SequenceDriver( IThreadPool *pool, platform::ITaskRunner &timer )
	    : base( CreateThreadPoolTaskRunner( pool, &timer ) ), sequence( *base )
	{
	}
	~SequenceDriver() override
	{
		sequence.Shutdown();
		DestroyThreadPoolTaskRunner( base );
	}

	platform::ITaskRunner &Runner() override { return sequence; }
	platform::ISequencedTaskRunner *Sequenced() override { return &sequence; }
	void Drain() override
	{
		auto done = std::make_shared<std::atomic<bool>>( false );
		if ( sequence.PostTask(
		         [done]
		         {
			         done->store( true );
		         } ) != platform::PostResult::kAccepted )
			return;
		while ( !done->load() )
			std::this_thread::sleep_for( std::chrono::microseconds( 50 ) );
	}
	void Elapse( std::uint64_t ns ) override
	{
		std::this_thread::sleep_for(
		    std::chrono::nanoseconds( ns ) + std::chrono::milliseconds( 5 ) );
		Drain();
	}
	std::uint64_t NowNanoseconds() override { return SteadyNanoseconds(); }
	void Shutdown() override { sequence.Shutdown(); }

	platform::ITaskRunner *base;
	platform::SequencedTaskRunner sequence;
};

} // namespace

int main()
{
	testing::Checks checks;

	IThreadPool *pool = CreateThreadPool();
	ThreadPoolStartParams_t params;
	params.nThreads = 4;
	params.bExecOnThreadPoolThreadsOnly = false;
	pool->Start( params );
	checks.Equal( pool->NumThreads(), 4, "pool.started" );
	// The composition root chooses the delay timer; here a thread runner.
	platform::ThreadTaskRunner timer( "pool-runner-timer" );

	{
		PoolDriver driver( pool, timer );
		checks.That( driver.runner != nullptr, "pool.runner-created" );
		if ( driver.runner )
			platformtest::RunTaskRunnerConformance( checks, "pool", driver, 20000000 );
	}
	{
		SequenceDriver driver( pool, timer );
		platformtest::RunTaskRunnerConformance( checks, "sequence-over-pool", driver, 20000000 );
	}

	IThreadPool *empty = CreateThreadPool();
	ThreadPoolStartParams_t none;
	none.nThreads = 0;
	none.bExecOnThreadPoolThreadsOnly = false;
	empty->Start( none );
	checks.That(
	    CreateThreadPoolTaskRunner( empty, &timer ) == nullptr, "pool.threadless-pool-refused" );
	checks.That(
	    CreateThreadPoolTaskRunner( nullptr, &timer ) == nullptr, "pool.null-pool-refused" );
	checks.That(
	    CreateThreadPoolTaskRunner( pool, nullptr ) == nullptr, "pool.missing-timer-refused" );
	empty->Stop();
	DestroyThreadPool( empty );

	pool->Stop();
	DestroyThreadPool( pool );
	return checks.Report();
}
