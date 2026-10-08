//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: IWorkerBackend over its own threads (RFC 0003; test and tool roots).
//
//=============================================================================//

#include "jobsystem/thread_worker_backend.h"

#include <atomic>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>
#include <vector>

namespace jobsystem
{

namespace
{
enum class TicketState : uint8_t
{
	Queued,
	Running,
	Done,
	Withdrawn
};

struct Ticket
{
	WorkerTaskFn task;
	void *context;
	TicketState state = TicketState::Queued;
};
} // namespace

struct ThreadWorkerBackend::Impl
{
	std::mutex mtx;
	std::condition_variable work; // workers: a task was queued, or quit
	std::condition_variable done; // settlers: a task finished
	std::deque<Ticket *> queue;
	std::vector<std::thread> threads;
	bool quit = false;
	std::atomic<uint64_t> posted{ 0 }, run{ 0 }, withdrawn{ 0 };

	static Impl *&Current()
	{
		thread_local Impl *t_owner = nullptr;
		return t_owner;
	}

	void Worker()
	{
		Current() = this;
		std::unique_lock<std::mutex> lk( mtx );
		for ( ;; )
		{
			work.wait( lk,
			    [&]
			    {
				    return quit || !queue.empty();
			    } );
			if ( queue.empty() )
				return; // quit with nothing queued
			Ticket *ticket = queue.front();
			queue.pop_front();
			ticket->state = TicketState::Running;
			lk.unlock();
			ticket->task( ticket->context );
			run.fetch_add( 1, std::memory_order_relaxed );
			lk.lock();
			// The mutex publishes the task's writes to the settler.
			ticket->state = TicketState::Done;
			done.notify_all();
		}
	}
};

ThreadWorkerBackend::ThreadWorkerBackend( int workers ) : m_impl( std::make_unique<Impl>() )
{
	for ( int i = 0; i < workers; ++i )
		m_impl->threads.emplace_back(
		    [impl = m_impl.get()]
		    {
			    impl->Worker();
		    } );
}

ThreadWorkerBackend::~ThreadWorkerBackend()
{
	{
		std::lock_guard<std::mutex> lk( m_impl->mtx );
		m_impl->quit = true;
	}
	m_impl->work.notify_all();
	for ( std::thread &thread : m_impl->threads )
		thread.join();
}

int ThreadWorkerBackend::WorkerCount() const
{
	return (int)m_impl->threads.size();
}

void *ThreadWorkerBackend::PostTask( WorkerTaskFn task, void *context )
{
	if ( !task || m_impl->threads.empty() )
		return nullptr;
	Ticket *ticket = new Ticket{ task, context };
	{
		std::lock_guard<std::mutex> lk( m_impl->mtx );
		if ( m_impl->quit )
		{
			delete ticket;
			return nullptr;
		}
		m_impl->queue.push_back( ticket );
	}
	m_impl->posted.fetch_add( 1, std::memory_order_relaxed );
	m_impl->work.notify_one();
	return ticket;
}

bool ThreadWorkerBackend::SettleTask( void *opaque )
{
	Ticket *ticket = static_cast<Ticket *>( opaque );
	bool ran = true;
	{
		std::unique_lock<std::mutex> lk( m_impl->mtx );
		if ( ticket->state == TicketState::Queued )
		{
			// Still queued: unlink it, so no worker ever sees it.
			for ( auto it = m_impl->queue.begin(); it != m_impl->queue.end(); ++it )
			{
				if ( *it == ticket )
				{
					m_impl->queue.erase( it );
					break;
				}
			}
			ticket->state = TicketState::Withdrawn;
			ran = false;
		}
		else
		{
			m_impl->done.wait( lk,
			    [&]
			    {
				    return ticket->state == TicketState::Done;
			    } );
		}
	}
	if ( !ran )
		m_impl->withdrawn.fetch_add( 1, std::memory_order_relaxed );
	delete ticket;
	return ran;
}

bool ThreadWorkerBackend::ShouldRunInline()
{
	return Impl::Current() == m_impl.get();
}

uint64_t ThreadWorkerBackend::TasksPosted() const
{
	return m_impl->posted.load( std::memory_order_relaxed );
}

uint64_t ThreadWorkerBackend::TasksRun() const
{
	return m_impl->run.load( std::memory_order_relaxed );
}

uint64_t ThreadWorkerBackend::TasksWithdrawn() const
{
	return m_impl->withdrawn.load( std::memory_order_relaxed );
}

} // namespace jobsystem
