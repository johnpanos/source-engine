//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: External-completion token for the dependency-aware job system
//          (RFC 0003, "Blocking operations and external completion").
//
//          Bridges an out-of-band completion (async filesystem read, module
//          load, device callback) into the job graph's dependency/failure
//          semantics. The RFC requires defined, exactly-once behavior for the
//          four races it names:
//            * registration of a continuation before the event,
//            * completion arriving before registration,
//            * cancellation, and
//            * repeated notification.
//
//          ExternalCompletion is the small synchronization primitive that owns
//          those guarantees; it is deliberately independent of the graph so it
//          can be adversarially tested on its own (Q-JOBS negative fixtures).
//          An external producer drives it to exactly one terminal state; the
//          first Complete()/Cancel() wins and later calls are no-ops. A
//          continuation registered before OR after the transition fires exactly
//          once with the terminal result.
//
//          MakeExternalWait() adapts a token into a JobEntry for a BlockingIO
//          job: the job parks on the blocking lane (never a compute worker) until
//          the token is terminal, and fails if the token was canceled so its
//          Success-dependents cascade-cancel. A non-blocking "feed the ready
//          queue" integration belongs to the later dynamic-scope extension, where
//          a live scope can accept the continuation.
//
//=============================================================================//

#ifndef JOBSYSTEM_EXTERNAL_COMPLETION_H
#define JOBSYSTEM_EXTERNAL_COMPLETION_H

#ifdef _WIN32
#pragma once
#endif

#include <condition_variable>
#include <cstdint>
#include <functional>
#include <mutex>

#include "jobsystem/job_graph.h"
#include "jobsystem/graph_executor.h" // JobRunContext (MakeExternalWait uses Fail())

namespace jobsystem
{

enum class CompletionResult : uint8_t
{
	Pending,   // no terminal event yet
	Completed, // producer signaled success
	Canceled,  // producer signaled cancellation / failure
};

class ExternalCompletion
{
public:
	ExternalCompletion() = default;

	// Non-copyable, non-movable: consumers hold stable references/pointers to it
	// across threads for its whole lifetime.
	ExternalCompletion( const ExternalCompletion & ) = delete;
	ExternalCompletion &operator=( const ExternalCompletion & ) = delete;

	// Drive to a terminal state. Returns true iff THIS call performed the
	// Pending->terminal transition; a second call (of either kind) returns false
	// and changes nothing. Safe to call from any thread, any number of times.
	bool Complete() { return Transition( CompletionResult::Completed ); }
	bool Cancel() { return Transition( CompletionResult::Canceled ); }

	// Current state without blocking.
	CompletionResult Poll() const
	{
		std::lock_guard<std::mutex> lk( m_mtx );
		return m_result;
	}

	// Block until terminal, then return the terminal result.
	CompletionResult Wait()
	{
		std::unique_lock<std::mutex> lk( m_mtx );
		m_cv.wait( lk,
		    [&]
		    {
			    return m_result != CompletionResult::Pending;
		    } );
		return m_result;
	}

	// Register a continuation fired exactly once with the terminal result. If the
	// token is still pending, the continuation is stored and fires on the thread
	// that later transitions it; the call returns true. If the token is already
	// terminal, the continuation fires immediately on the calling thread and the
	// call returns false. At most one continuation may be registered.
	bool OnComplete( std::function<void( CompletionResult )> cb )
	{
		CompletionResult fireWith;
		{
			std::lock_guard<std::mutex> lk( m_mtx );
			if ( m_result == CompletionResult::Pending )
			{
				m_cb = std::move( cb ); // fires later, at the transition
				return true;
			}
			if ( m_cbFired )
				return false; // already fired for a prior registrant
			m_cbFired = true;
			fireWith = m_result;
		}
		cb( fireWith ); // already terminal: fire now, outside the lock
		return false;
	}

private:
	bool Transition( CompletionResult to )
	{
		std::function<void( CompletionResult )> cb;
		{
			std::lock_guard<std::mutex> lk( m_mtx );
			if ( m_result != CompletionResult::Pending )
				return false; // exactly-once: a terminal state is final
			m_result = to;
			if ( m_cb && !m_cbFired )
			{
				cb = std::move( m_cb );
				m_cbFired = true;
			}
		}
		m_cv.notify_all();
		if ( cb )
			cb( to ); // fire the continuation outside the lock, exactly once
		return true;
	}

	mutable std::mutex m_mtx;
	std::condition_variable m_cv;
	CompletionResult m_result = CompletionResult::Pending;
	std::function<void( CompletionResult )> m_cb;
	bool m_cbFired = false;
};

// Adapt a token into a BlockingIO job body: park on the blocking lane until the
// token is terminal; Fail() the job on cancellation so Success-dependents cascade.
// The token must outlive the job. The job MUST be declared Executor::BlockingIO().
inline JobEntry MakeExternalWait( ExternalCompletion *token )
{
	return [token]( JobRunContext &ctx )
	{
		if ( token->Wait() != CompletionResult::Completed )
			ctx.Fail();
	};
}

} // namespace jobsystem

#endif // JOBSYSTEM_EXTERNAL_COMPLETION_H
