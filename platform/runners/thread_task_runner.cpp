//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Native single-thread task runner.
//
//=============================================================================//

#include "thread_task_runner.h"

#include <algorithm>
#include <cstdlib>

namespace platform
{

namespace
{

template <typename Entry> bool Later( const Entry &a, const Entry &b )
{
	return a.due != b.due ? a.due > b.due : a.sequence > b.sequence;
}

} // namespace

ThreadTaskRunner::ThreadTaskRunner( std::string name ) : m_name( std::move( name ) )
{
	m_thread = std::thread(
	    [this]
	    {
		    Run();
	    } );
	m_threadId.store( m_thread.get_id(), std::memory_order_release );
}

ThreadTaskRunner::~ThreadTaskRunner()
{
	Shutdown();
}

PostResult ThreadTaskRunner::PostTask( Task task )
{
	return Post( std::move( task ), 0 );
}

PostResult ThreadTaskRunner::PostDelayedTask( Task task, std::uint64_t delayNanoseconds )
{
	return Post( std::move( task ), delayNanoseconds );
}

PostResult ThreadTaskRunner::Post( Task task, std::uint64_t delayNanoseconds )
{
	const Clock::time_point due = Clock::now() + std::chrono::nanoseconds( delayNanoseconds );
	std::unique_lock lock( m_mutex );
	if ( m_shutDown )
	{
		lock.unlock(); // the refused task is destroyed outside the lock
		return PostResult::kShutDown;
	}
	m_pending.push_back( Entry{ due, m_nextSequence++, std::move( task ) } );
	std::push_heap( m_pending.begin(), m_pending.end(), Later<Entry> );
	lock.unlock();
	m_wake.notify_one();
	return PostResult::kAccepted;
}

bool ThreadTaskRunner::RunsTasksInCurrentSequence() const
{
	return BelongsToCurrentThread();
}

bool ThreadTaskRunner::BelongsToCurrentThread() const
{
	return std::this_thread::get_id() == m_threadId.load( std::memory_order_acquire );
}

void ThreadTaskRunner::Run()
{
	std::unique_lock lock( m_mutex );
	while ( true )
	{
		if ( m_shutDown )
			return;
		if ( m_pending.empty() )
		{
			m_wake.wait( lock );
			continue;
		}
		if ( m_pending.front().due > Clock::now() )
		{
			m_wake.wait_until( lock, m_pending.front().due );
			continue;
		}
		std::pop_heap( m_pending.begin(), m_pending.end(), Later<Entry> );
		Task task = std::move( m_pending.back().task );
		m_pending.pop_back();
		lock.unlock();
		task();
		task = Task(); // destroyed on this thread, before the next task starts
		lock.lock();
	}
}

void ThreadTaskRunner::Shutdown()
{
	if ( BelongsToCurrentThread() )
		std::abort(); // a runner cannot join its own thread
	std::vector<Entry> dropped;
	{
		std::lock_guard lock( m_mutex );
		m_shutDown = true;
		dropped.swap( m_pending );
	}
	m_wake.notify_all();
	// Concurrent callers all return only after the thread has exited.
	std::call_once( m_joinOnce,
	    [this]
	    {
		    m_thread.join();
		    m_threadId.store( std::thread::id(), std::memory_order_release );
	    } );
	// `dropped` is destroyed here, on the owner's thread, without running.
}

} // namespace platform
