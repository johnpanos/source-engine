//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Virtual clock and manual (virtual-time) sequenced task runner.
//
//=============================================================================//

#include "manual_task_runner.h"

#include <algorithm>
#include <cstdlib>

namespace platform
{

namespace
{

// std heap functions build a max-heap; this orders the earliest (due, post) first.
template <typename Entry> bool Later( const Entry &a, const Entry &b )
{
	return a.due != b.due ? a.due > b.due : a.sequence > b.sequence;
}

} // namespace

MonotonicTimestamp VirtualClock::Now() const
{
	return MonotonicTimestamp{ m_ticks.load( std::memory_order_acquire ) };
}

void VirtualClock::AdvanceBy( std::uint64_t nanoseconds )
{
	m_ticks.fetch_add( nanoseconds, std::memory_order_acq_rel );
}

void VirtualClock::AdvanceTo( std::uint64_t ticks )
{
	std::uint64_t current = m_ticks.load( std::memory_order_acquire );
	while ( current < ticks &&
	        !m_ticks.compare_exchange_weak( current, ticks, std::memory_order_acq_rel ) )
	{
	}
}

ManualTaskRunner::ManualTaskRunner( VirtualClock &clock ) : m_clock( clock ) {}

ManualTaskRunner::~ManualTaskRunner()
{
	Shutdown();
}

PostResult ManualTaskRunner::PostTask( Task task )
{
	return Post( std::move( task ), 0 );
}

PostResult ManualTaskRunner::PostDelayedTask( Task task, std::uint64_t delayNanoseconds )
{
	return Post( std::move( task ), delayNanoseconds );
}

PostResult ManualTaskRunner::Post( Task task, std::uint64_t delayNanoseconds )
{
	std::unique_lock lock( m_mutex );
	if ( m_shutDown )
	{
		lock.unlock(); // the refused task is destroyed outside the lock
		return PostResult::kShutDown;
	}
	m_pending.push_back( Entry{ m_clock.Now().ticks + delayNanoseconds, m_nextSequence++, std::move( task ) } );
	std::push_heap( m_pending.begin(), m_pending.end(), Later<Entry> );
	return PostResult::kAccepted;
}

bool ManualTaskRunner::RunsTasksInCurrentSequence() const
{
	return m_running.load( std::memory_order_acquire ) == std::this_thread::get_id();
}

bool ManualTaskRunner::PopDue( std::uint64_t now, Task &out )
{
	std::lock_guard lock( m_mutex );
	if ( m_shutDown || m_pending.empty() || m_pending.front().due > now )
		return false;
	std::pop_heap( m_pending.begin(), m_pending.end(), Later<Entry> );
	out = std::move( m_pending.back().task );
	m_pending.pop_back();
	return true;
}

int ManualTaskRunner::RunUntilIdle()
{
	// Driving the runner from one of its own tasks would run tasks inside a
	// task: a programmer error.
	if ( RunsTasksInCurrentSequence() )
		std::abort();
	int ran = 0;
	Task task;
	while ( PopDue( m_clock.Now().ticks, task ) )
	{
		m_running.store( std::this_thread::get_id(), std::memory_order_release );
		task();
		task = Task();
		m_running.store( std::thread::id(), std::memory_order_release );
		++ran;
	}
	return ran;
}

int ManualTaskRunner::AdvanceBy( std::uint64_t nanoseconds )
{
	const std::uint64_t target = m_clock.Now().ticks + nanoseconds;
	int ran = RunUntilIdle();
	while ( true )
	{
		std::uint64_t next = target;
		{
			std::lock_guard lock( m_mutex );
			if ( !m_shutDown && !m_pending.empty() )
				next = std::min( next, m_pending.front().due );
		}
		m_clock.AdvanceTo( next );
		ran += RunUntilIdle();
		if ( next >= target )
			return ran;
	}
}

size_t ManualTaskRunner::PendingCount() const
{
	std::lock_guard lock( m_mutex );
	return m_pending.size();
}

void ManualTaskRunner::Shutdown()
{
	std::vector<Entry> dropped;
	{
		std::lock_guard lock( m_mutex );
		m_shutDown = true;
		dropped.swap( m_pending );
	}
	// Tasks are destroyed here, on the owner's thread, without running.
}

} // namespace platform
