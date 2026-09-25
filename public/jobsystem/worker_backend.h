//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Abstract worker backend for graph execution (RFC 0003, Phase C
//          "Compatibility and incremental integration" / legacy pool bridge).
//
//          This is the controlled executor boundary between the portable
//          scheduler and a concrete worker source. It is deliberately kept to a
//          C++11-clean surface (only <functional>) so a legacy C++11 target
//          such as vstdlib can implement it against the real engine thread pool
//          without including the C++20 job-graph headers. The scheduler drives
//          it; it does not know about job graphs.
//
//=============================================================================//

#ifndef JOBSYSTEM_WORKER_BACKEND_H
#define JOBSYSTEM_WORKER_BACKEND_H

#include <functional>

namespace jobsystem
{

class IWorkerBackend
{
public:
	virtual ~IWorkerBackend() {}

	// Invoke body(0), body(1), ... body(n-1), possibly concurrently on worker
	// threads, and return only after every invocation has completed. The return
	// is a join barrier: writes made by the invocations happen-before whatever
	// the caller does next. Implementations must run each index exactly once.
	virtual void ParallelFor( int n, const std::function<void( int )> &body ) = 0;

	// As ParallelFor, and additionally run caller() once on the calling thread.
	// caller() is independent of every body(i) (the scheduler guarantees it) and
	// may run concurrently with them; the return joins both. The default runs
	// ParallelFor and then caller(), which overlaps nothing (the order before
	// this hook existed); a backend with asynchronous workers starts body(i)
	// before running caller().
	virtual void ParallelForWithCaller( int n, const std::function<void( int )> &body,
	    const std::function<void()> &caller )
	{
		ParallelFor( n, body );
		caller();
	}

	// Number of worker threads available (0 => work runs on the caller).
	virtual int WorkerCount() const = 0;
};

} // namespace jobsystem

#endif // JOBSYSTEM_WORKER_BACKEND_H
