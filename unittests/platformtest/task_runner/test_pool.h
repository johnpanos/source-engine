//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: A test-only parallel ITaskRunner (a few std::threads) so the
//			sequence adapter is certified over a base that really runs tasks
//			concurrently. It is itself run through the shared suite as a plain
//			(non-sequenced) runner. Not a product provider.
//
//=============================================================================//

#ifndef PLATFORMTEST_TEST_POOL_H
#define PLATFORMTEST_TEST_POOL_H

#include "platform/contracts/task_runner.h"

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <thread>
#include <vector>

namespace platformtest
{

class TestPool final : public platform::ITaskRunner
{
public:
	explicit TestPool( int threads )
	{
		for ( int i = 0; i < threads; ++i )
			m_threads.emplace_back(
			    [this]
			    {
				    Work();
			    } );
	}
	~TestPool() override { Shutdown(); }

	[[nodiscard]] platform::PostResult PostTask( platform::Task task ) override
	{
		return PostDelayedTask( std::move( task ), 0 );
	}
	[[nodiscard]] platform::PostResult PostDelayedTask(
	    platform::Task task, std::uint64_t delayNanoseconds ) override
	{
		const auto due = Clock::now() + std::chrono::nanoseconds( delayNanoseconds );
		std::unique_lock lock( m_mutex );
		if ( m_shutDown )
		{
			lock.unlock();
			return platform::PostResult::kShutDown;
		}
		m_pending.push_back( Entry{ due, m_next++, std::move( task ) } );
		std::push_heap( m_pending.begin(), m_pending.end(), Later );
		lock.unlock();
		m_wake.notify_all();
		return platform::PostResult::kAccepted;
	}

	// Waits until no task is running and none is due.
	void Drain()
	{
		std::unique_lock lock( m_mutex );
		while ( m_running > 0 || ( !m_pending.empty() && m_pending.front().due <= Clock::now() ) )
			m_idle.wait_for( lock, std::chrono::milliseconds( 1 ) );
	}

	void Shutdown()
	{
		std::vector<Entry> dropped;
		{
			std::lock_guard lock( m_mutex );
			m_shutDown = true;
			dropped.swap( m_pending );
		}
		m_wake.notify_all();
		for ( std::thread &thread : m_threads )
		{
			if ( thread.joinable() )
				thread.join();
		}
	}

private:
	using Clock = std::chrono::steady_clock;
	struct Entry
	{
		Clock::time_point due;
		std::uint64_t sequence = 0;
		platform::Task task;
	};
	static bool Later( const Entry &a, const Entry &b )
	{
		return a.due != b.due ? a.due > b.due : a.sequence > b.sequence;
	}

	void Work()
	{
		std::unique_lock lock( m_mutex );
		while ( !m_shutDown )
		{
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
			std::pop_heap( m_pending.begin(), m_pending.end(), Later );
			platform::Task task = std::move( m_pending.back().task );
			m_pending.pop_back();
			++m_running;
			lock.unlock();
			task();
			task = platform::Task();
			lock.lock();
			--m_running;
			m_idle.notify_all();
		}
	}

	std::mutex m_mutex;
	std::condition_variable m_wake;
	std::condition_variable m_idle;
	std::vector<Entry> m_pending;
	std::uint64_t m_next = 0;
	int m_running = 0;
	bool m_shutDown = false;
	std::vector<std::thread> m_threads;
};

} // namespace platformtest

#endif // PLATFORMTEST_TEST_POOL_H
