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
	bool scheduled = false; // one base task is in flight
	bool shutDown = false;
	std::atomic<std::thread::id> running{};
};

namespace
{

using State = SequencedTaskRunner::State;

void RunOne( const std::shared_ptr<State> &state );

// Posts the sequence's base task. On refusal the sequence shuts down: its
// pending tasks are destroyed without running (outside the lock).
void Schedule( const std::shared_ptr<State> &state )
{
	if ( state->base.PostTask( [state] { RunOne( state ); } ) == PostResult::kAccepted )
		return;
	std::deque<Task> dropped;
	std::lock_guard lock( state->mutex );
	state->shutDown = true;
	state->scheduled = false;
	dropped.swap( state->ready );
	state->idle.notify_all();
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
	Schedule( state );
	return true;
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
		Schedule( state );
}

} // namespace

SequencedTaskRunner::SequencedTaskRunner( ITaskRunner &base ) : m_state( std::make_shared<State>( base ) ) {}

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
	{
		std::lock_guard lock( m_state->mutex );
		if ( m_state->shutDown )
			return PostResult::kShutDown;
	}
	// The base runner keeps the delay; the task joins the sequence when due.
	std::shared_ptr<State> state = m_state;
	return m_state->base.PostDelayedTask(
	    [state, task = std::move( task )]() mutable { Enqueue( state, std::move( task ) ); },
	    delayNanoseconds );
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
	std::unique_lock lock( m_state->mutex );
	m_state->shutDown = true;
	dropped.swap( m_state->ready );
	m_state->idle.wait( lock, [this] { return m_state->running.load() == std::thread::id(); } );
	lock.unlock();
	// `dropped` is destroyed here, on the owner's thread, without running.
}

} // namespace platform
