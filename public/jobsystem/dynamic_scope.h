//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Dynamic child scopes for the dependency-aware job system (RFC 0003,
//          "Dynamic work"). This is the deliberately-later extension that runs
//          "after sealed graphs are stable": inside a live scope, running work
//          may admit further child jobs whose prerequisites are other already-
//          admitted children.
//
//          The RFC's invariants are enforced here:
//            * Completion ownership is reserved before any child is published:
//              the scope opens holding one keepalive, so it cannot be declared
//              complete while the owner may still spawn (RFC "reserve child
//              completion ownership before publishing any child work").
//            * Scope completion includes every admitted child.
//            * A child may depend only on already-admitted children, never on the
//              completion of the parent scope that is waiting for it (structurally
//              impossible: there is no handle for the parent).
//            * Nested parallelism never blocks a worker on child jobs. A running
//              child that needs results SPLITS into a producer and a continuation
//              child that depends on it; workers only ever take ready work. The
//              only thread that blocks is the non-worker owner in Wait().
//            * Once the scope has fully drained it acquires no further children.
//
//          Failure/cancel semantics match the sealed graph: a child with a
//          Success dependency on a child that did not succeed is canceled, and
//          cancellation cascades. CancelPending() cooperatively cancels not-yet-
//          started children; running children are never force-terminated.
//
//=============================================================================//

#ifndef JOBSYSTEM_DYNAMIC_SCOPE_H
#define JOBSYSTEM_DYNAMIC_SCOPE_H

#ifdef _WIN32
#pragma once
#endif

#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <thread>
#include <vector>

#include "jobsystem/job_graph.h"
#include "jobsystem/graph_executor.h"

namespace jobsystem
{

class DynamicScope
{
public:
	struct ChildHandle
	{
		static constexpr uint32_t kInvalid = 0xFFFFFFFFu;
		uint32_t id = kInvalid;
		bool IsValid() const { return id != kInvalid; }
	};

	// nWorkers == 0 drains the scope on the owner thread when Wait() is called
	// (still dependency-correct). Otherwise nWorkers threads service child jobs.
	explicit DynamicScope( int nWorkers, const FrameContext &frame = FrameContext{} );
	~DynamicScope();

	DynamicScope( const DynamicScope & ) = delete;
	DynamicScope &operator=( const DynamicScope & ) = delete;

	// Admit a child job. deps are handles of ALREADY-admitted children this child
	// waits on (Success semantics). Thread-safe; may be called from inside a
	// running child to spawn a continuation. Returns an invalid handle if the
	// scope has already drained (no completion ownership remains to hold it).
	ChildHandle Spawn( const char *name, JobEntry fn, const std::vector<ChildHandle> &deps = {},
	    ExecutorToken exec = Executor::Compute() );

	// Cooperatively cancel children that have not started; running children run to
	// completion. Newly readied children after this point are canceled.
	void CancelPending();

	// Release the owner keepalive and block the CALLING thread until every admitted
	// child is terminal. The owner is not a worker: it waits, it does not help, so
	// nested parallelism never blocks a worker on child jobs. Call exactly once.
	void Wait();

	// Terminal state of a child. Valid after Wait() returns (or, for an already
	// finished child, at any time). Returns Admitted if never resolved.
	JobState StateOf( ChildHandle h ) const;

	// Counters, valid after Wait().
	uint32_t Admitted() const { return m_admittedCount; }
	uint32_t Succeeded() const { return m_succeeded; }
	uint32_t Failed() const { return m_failed; }
	uint32_t Canceled() const { return m_canceled; }

private:
	struct Child
	{
		const char *name;
		JobEntry fn;
		ExecutorToken exec;
		uint32_t remaining; // unresolved prerequisites
		bool willCancel;    // decided at readiness
		JobState state;
		std::vector<uint32_t> dependents;
	};

	void WorkerLoop();
	// Resolves a child; returns how many dependents it made ready.
	uint32_t ResolveLocked( std::unique_lock<std::mutex> &lk, uint32_t id, JobState terminal );
	// Wake idle workers for `count` newly ready children (and everyone when the
	// scope has drained). Called with the lock held.
	void WakeLocked( uint32_t count );
	// Run one popped child, releasing the lock while its function executes.
	// Returns how many dependents it made ready.
	uint32_t RunChildLocked( std::unique_lock<std::mutex> &lk, uint32_t id );

	const int m_nWorkers;
	FrameContext m_frame;

	mutable std::mutex m_mtx;
	std::condition_variable m_cv;     // workers: ready work or drained
	std::condition_variable m_doneCv; // owner in Wait(): drained
	uint32_t m_idleWorkers = 0;       // workers blocked on m_cv

	std::vector<Child> m_children;
	std::vector<uint32_t> m_ready;
	uint32_t m_outstanding = 1; // owner keepalive reserved before any child
	bool m_scopeCanceled = false;
	bool m_waitStarted = false;

	std::vector<std::thread> m_workers;

	// Snapshot counters published by Wait() for lock-free reads afterward.
	uint32_t m_admittedCount = 0;
	uint32_t m_succeeded = 0, m_failed = 0, m_canceled = 0;
};

} // namespace jobsystem

#endif // JOBSYSTEM_DYNAMIC_SCOPE_H
