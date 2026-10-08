//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Abstract worker backend for graph execution (RFC 0003, Phase C
//          "Compatibility and incremental integration" / legacy pool bridge;
//          phase I, J1 "Continuous execution in products").
//
//          This is the controlled executor boundary between the portable
//          scheduler and a concrete worker source. A backend runs tasks: it
//          has no fork/join, barrier or "run these n items" operation, so no
//          scheduler built on it can run a graph in waves. TaskExecutor
//          (task_executor.h) is the scheduler; it posts tasks that drain its
//          ready queue and exit when it is empty, so a borrowed worker never
//          parks inside a graph.
//
//          It is deliberately kept to a C++11-clean surface (no standard
//          library types) so a legacy C++11 target such as vstdlib can
//          implement it against the real engine thread pool without including
//          the C++20 job-graph headers. The scheduler drives it; it does not
//          know about job graphs.
//
//=============================================================================//

#ifndef JOBSYSTEM_WORKER_BACKEND_H
#define JOBSYSTEM_WORKER_BACKEND_H

namespace jobsystem
{

typedef void ( *WorkerTaskFn )( void *context );

class IWorkerBackend
{
public:
	virtual ~IWorkerBackend() {}

	// Number of worker threads available (0 => the scheduler runs everything
	// on its caller and posts nothing).
	virtual int WorkerCount() const = 0;

	// Arrange for task(context) to run exactly once on one of the backend's
	// workers. Returns an opaque non-null ticket when the task is accepted, and
	// null when it is refused (no workers, the backend is stopping): a refused
	// task never runs, and the scheduler does that work itself. Thread-safe,
	// and callable from inside a task. The task may start, and even finish,
	// before PostTask returns (a full queue may run it on the posting worker),
	// so the scheduler never posts while it holds a lock the task takes.
	// Everything the poster wrote before PostTask happens-before the task.
	virtual void *PostTask( WorkerTaskFn task, void *context ) = 0;

	// Settle an accepted ticket, exactly once, and release it. If the task has
	// not started it is withdrawn: it never runs, and the call returns false.
	// Otherwise the call returns true once the task has returned; everything
	// the task wrote happens-before that return. It never runs unrelated work
	// while it waits.
	virtual bool SettleTask( void *ticket ) = 0;

	// Whether a scheduler entered on the calling thread must run its graph
	// inline: true on one of the backend's own workers, where a nested run
	// would otherwise post work behind the job that waits for it (the nested
	// wait RFC 0003 forbids). Not const: a backend may count these calls.
	virtual bool ShouldRunInline() = 0;
};

} // namespace jobsystem

#endif // JOBSYSTEM_WORKER_BACKEND_H
