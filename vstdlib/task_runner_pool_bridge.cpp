//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The engine thread pool as a platform.task-runner.v1 runner.
//
//=============================================================================//

#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>

#include "vstdlib/task_runner_pool_bridge.h"
#include "vstdlib/jobthread.h"
#include "platform/contracts/task_runner.h"

#include "tier0/memdbgon.h"

namespace
{

// Shared by the runner and the jobs and timer tasks it posted, which may run
// (and find nothing to do) after the runner is gone: the destruction context
// of the state is the last of the runner and its posted work.
struct PoolRunnerState
{
	explicit PoolRunnerState( IThreadPool *p ) : pool( p ) {}

	IThreadPool *const pool;
	std::mutex mutex;
	std::condition_variable idle;
	std::map<unsigned long long, platform::Task> queued;  // in the pool, not started
	std::map<unsigned long long, platform::Task> delayed; // waiting on the timer
	unsigned long long next = 0;
	int running = 0;
	bool shutDown = false;
};

class CPoolTaskJob : public CJob
{
public:
	CPoolTaskJob( std::shared_ptr<PoolRunnerState> state, unsigned long long id )
	    : m_state( std::move( state ) ), m_id( id )
	{
		SetFlags( JF_QUEUE ); // never run inside AddJob on the posting thread
	}

	virtual JobStatus_t DoExecute()
	{
		platform::Task task;
		{
			std::lock_guard<std::mutex> lock( m_state->mutex );
			auto it = m_state->queued.find( m_id );
			if ( it == m_state->queued.end() )
				return JOB_OK; // dropped by shutdown
			task = std::move( it->second );
			m_state->queued.erase( it );
			++m_state->running;
		}
		task();
		task = platform::Task();
		{
			std::lock_guard<std::mutex> lock( m_state->mutex );
			--m_state->running;
		}
		m_state->idle.notify_all();
		return JOB_OK;
	}

private:
	std::shared_ptr<PoolRunnerState> m_state;
	unsigned long long m_id;
};

platform::PostResult QueueOnPool(
    const std::shared_ptr<PoolRunnerState> &state, platform::Task task )
{
	unsigned long long id = 0;
	{
		std::unique_lock<std::mutex> lock( state->mutex );
		if ( state->shutDown )
		{
			lock.unlock();
			return platform::PostResult::kShutDown;
		}
		id = state->next++;
		state->queued.emplace( id, std::move( task ) );
	}
	CJob *pJob = new CPoolTaskJob( state, id );
	state->pool->AddJob( pJob );
	pJob->Release();
	return platform::PostResult::kAccepted;
}

class CThreadPoolTaskRunner final : public platform::ITaskRunner
{
public:
	CThreadPoolTaskRunner( IThreadPool *pPool, platform::ITaskRunner &timer )
	    : m_state( std::make_shared<PoolRunnerState>( pPool ) ), m_timer( timer )
	{
	}

	~CThreadPoolTaskRunner() override { Shutdown(); }

	platform::PostResult PostTask( platform::Task task ) override
	{
		return QueueOnPool( m_state, std::move( task ) );
	}

	platform::PostResult PostDelayedTask(
	    platform::Task task, std::uint64_t delayNanoseconds ) override
	{
		if ( delayNanoseconds == 0 )
			return PostTask( std::move( task ) );
		unsigned long long id = 0;
		{
			std::unique_lock<std::mutex> lock( m_state->mutex );
			if ( m_state->shutDown )
			{
				lock.unlock();
				return platform::PostResult::kShutDown;
			}
			id = m_state->next++;
			m_state->delayed.emplace( id, std::move( task ) );
		}
		// The timer keeps the delay and holds only the id, so shutdown can
		// destroy the task; when due, the task joins the pool.
		std::shared_ptr<PoolRunnerState> state = m_state;
		const platform::PostResult posted = m_timer.PostDelayedTask(
		    [state, id]
		    {
			    platform::Task due;
			    {
				    std::lock_guard<std::mutex> lock( state->mutex );
				    auto it = state->delayed.find( id );
				    if ( it == state->delayed.end() )
					    return; // dropped by shutdown
				    due = std::move( it->second );
				    state->delayed.erase( it );
			    }
			    (void)QueueOnPool( state, std::move( due ) );
		    },
		    delayNanoseconds );
		if ( posted != platform::PostResult::kAccepted )
		{
			platform::Task refused;
			std::lock_guard<std::mutex> lock( m_state->mutex );
			auto it = m_state->delayed.find( id );
			if ( it != m_state->delayed.end() )
			{
				refused = std::move( it->second );
				m_state->delayed.erase( it );
			}
		}
		return posted;
	}

	void Shutdown()
	{
		std::map<unsigned long long, platform::Task> dropped;
		std::map<unsigned long long, platform::Task> droppedDelayed;
		std::unique_lock<std::mutex> lock( m_state->mutex );
		m_state->shutDown = true;
		dropped.swap( m_state->queued );
		droppedDelayed.swap( m_state->delayed );
		m_state->idle.wait( lock,
		    [this]
		    {
			    return m_state->running == 0;
		    } );
		lock.unlock();
		// The dropped tasks are destroyed here, on the owner's thread, unrun.
	}

private:
	std::shared_ptr<PoolRunnerState> m_state;
	platform::ITaskRunner &m_timer;
};

} // namespace

VSTDLIB_INTERFACE platform::ITaskRunner *CreateThreadPoolTaskRunner(
    IThreadPool *pPool, platform::ITaskRunner *pDelayTimer )
{
	if ( !pPool || !pDelayTimer || pPool->NumThreads() <= 0 )
		return nullptr; // a pool without threads would run jobs inside the post
	return new CThreadPoolTaskRunner( pPool, *pDelayTimer );
}

VSTDLIB_INTERFACE void ShutdownThreadPoolTaskRunner( platform::ITaskRunner *pRunner )
{
	if ( pRunner )
		static_cast<CThreadPoolTaskRunner *>( pRunner )->Shutdown();
}

VSTDLIB_INTERFACE void DestroyThreadPoolTaskRunner( platform::ITaskRunner *pRunner )
{
	delete static_cast<CThreadPoolTaskRunner *>( pRunner );
}
