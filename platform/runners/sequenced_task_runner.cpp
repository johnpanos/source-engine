//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: A sequence over any task runner.
//
//=============================================================================//

#include "sequenced_task_runner.h"

#include <atomic>
#include <condition_variable>
#include <cstdlib>
#include <deque>
#include <map>
#include <mutex>
#include <thread>

namespace platform
{

struct SequencedTaskRunner::State
{
	explicit State( ITaskRunner &runner ) : base( runner ) {}

	ITaskRunner &base;
	std::mutex mutex;
	std::condition_variable idle;
	std::deque<Task> ready;
	// Delayed tasks stay here until due, so Shutdown destroys them; the base
	// runner holds only their ids.
	std::map<std::uint64_t, Task> delayed;
	std::uint64_t nextDelayed = 0;
	bool scheduled = false; // one base task is in flight
	bool shutDown = false;
	std::atomic<std::thread::id> running{};
};

namespace
{

using State = SequencedTaskRunner::State;

void RunOne( const std::shared_ptr<State> &state );

// The base runner refused a post: the sequence shuts down, and its pending
// tasks are destroyed without running (outside the lock).
void ShutDownAfterRefusal( const std::shared_ptr<State> &state )
{
	std::deque<Task> dropped;
	std::map<std::uint64_t, Task> droppedDelayed;
	std::lock_guard lock( state->mutex );
	state->shutDown = true;
	state->scheduled = false;
	dropped.swap( state->ready );
	droppedDelayed.swap( state->delayed );
	state->idle.notify_all();
}

// Posts the sequence's base task; false when the base refused it (the
// sequence is then shut down).
bool Schedule( const std::shared_ptr<State> &state )
{
	if ( state->base.PostTask(
	         [state]
	         {
		         RunOne( state );
	         } ) == PostResult::kAccepted )
		return true;
	ShutDownAfterRefusal( state );
	return false;
}

// Returns false (and destroys the task) when the sequence is shut down.
bool Enqueue( const std::shared_ptr<State> &state, Task task )
{
	std::unique_lock lock( state->mutex );
	if ( state->shutDown )
	{
		lock.unlock();
		return false;
	}
	state->ready.push_back( std::move( task ) );
	if ( state->scheduled )
		return true;
	state->scheduled = true;
	lock.unlock();
	return Schedule( state );
}

void RunOne( const std::shared_ptr<State> &state )
{
	Task task;
	{
		std::lock_guard lock( state->mutex );
		if ( state->shutDown || state->ready.empty() )
		{
			state->scheduled = false;
			return;
		}
		task = std::move( state->ready.front() );
		state->ready.pop_front();
		state->running.store( std::this_thread::get_id(), std::memory_order_release );
	}
	task();
	task = Task();
	bool more = false;
	{
		std::lock_guard lock( state->mutex );
		state->running.store( std::thread::id(), std::memory_order_release );
		more = !state->shutDown && !state->ready.empty();
		if ( !more )
			state->scheduled = false;
		state->idle.notify_all();
	}
	// One sequence task per base task, so a long sequence cannot starve the
	// base runner's other work.
	if ( more )
		(void)Schedule( state );
}

} // namespace

SequencedTaskRunner::SequencedTaskRunner( ITaskRunner &base )
    : m_state( std::make_shared<State>( base ) )
{
}

SequencedTaskRunner::~SequencedTaskRunner()
{
	Shutdown();
}

PostResult SequencedTaskRunner::PostTask( Task task )
{
	return Enqueue( m_state, std::move( task ) ) ? PostResult::kAccepted : PostResult::kShutDown;
}

PostResult SequencedTaskRunner::PostDelayedTask( Task task, std::uint64_t delayNanoseconds )
{
	if ( delayNanoseconds == 0 )
		return PostTask( std::move( task ) );
	std::uint64_t id = 0;
	{
		std::unique_lock lock( m_state->mutex );
		if ( m_state->shutDown )
		{
			lock.unlock();
			return PostResult::kShutDown;
		}
		id = m_state->nextDelayed++;
		m_state->delayed.emplace( id, std::move( task ) );
	}
	// The base runner keeps the delay; the task joins the sequence when due.
	std::shared_ptr<State> state = m_state;
	const PostResult posted = m_state->base.PostDelayedTask(
	    [state, id]
	    {
		    Task due;
		    {
			    std::lock_guard lock( state->mutex );
			    const auto it = state->delayed.find( id );
			    if ( it == state->delayed.end() )
				    return; // dropped by Shutdown
			    due = std::move( it->second );
			    state->delayed.erase( it );
		    }
		    Enqueue( state, std::move( due ) );
	    },
	    delayNanoseconds );
	if ( posted == PostResult::kShutDown )
		ShutDownAfterRefusal( m_state );
	return posted;
}

bool SequencedTaskRunner::RunsTasksInCurrentSequence() const
{
	return m_state->running.load( std::memory_order_acquire ) == std::this_thread::get_id();
}

void SequencedTaskRunner::Shutdown()
{
	if ( RunsTasksInCurrentSequence() )
		std::abort(); // a sequence cannot wait for its own running task
	std::deque<Task> dropped;
	std::map<std::uint64_t, Task> droppedDelayed;
	std::unique_lock lock( m_state->mutex );
	m_state->shutDown = true;
	dropped.swap( m_state->ready );
	droppedDelayed.swap( m_state->delayed );
	m_state->idle.wait( lock,
	    [this]
	    {
		    return m_state->running.load() == std::thread::id();
	    } );
	lock.unlock();
	// `dropped` is destroyed here, on the owner's thread, without running.
}

} // namespace platform
