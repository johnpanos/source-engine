//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Continuous, ready-driven graph executor over a borrowed
//          IWorkerBackend (RFC 0003 phase I, goal J1 "Continuous execution in
//          products").
//
//          A job enters the ready queue the moment its last prerequisite
//          resolves, whatever else is running: there are no waves and no
//          barrier between jobs. Compute work is drained by runner tasks
//          posted to the backend. A runner takes ready Compute/Sequence jobs
//          until the queue is empty and then returns its worker to the pool;
//          it never sleeps inside the graph. Whenever ready compute work
//          exceeds the runners that will still look at the queue, new runners
//          are posted, up to the backend's WorkerCount(). That is the J1
//          invariant: no ready compute job waits while the run has a free
//          runner slot (TaskRunStats::uncoveredReady counts violations; it is
//          zero by construction and the suites check it).
//
//          The calling thread helps with compute work and is the only
//          servicer of MainThread jobs (when it pumps) and of BlockingIO jobs
//          (this executor has no blocking lane), exactly as ParallelExecutor's
//          caller does; RunOptions runner bindings are honored the same way.
//          Because the caller can run every compute job itself, progress never
//          depends on the backend starting a task: a refused, late or
//          withdrawn runner costs parallelism, never correctness. When the run
//          ends, runners that never started are withdrawn and running ones
//          are waited for, so no task touches the run after Execute returns.
//
//          The scheduling state, lanes, stall rules and terminal states are
//          ParallelExecutor's (one implementation in parallel_executor.cpp),
//          so both agree with DeterministicExecutor on every graph.
//
//          A null backend, a backend with no workers, or a call on one of the
//          backend's own workers (IWorkerBackend::ShouldRunInline) runs the
//          whole graph inline on the caller. Execute may be called from
//          several threads at once; each call owns its run.
//
//=============================================================================//

#ifndef JOBSYSTEM_TASK_EXECUTOR_H
#define JOBSYSTEM_TASK_EXECUTOR_H

#include <cstdint>

#include "jobsystem/graph_executor.h"
#include "jobsystem/worker_backend.h"

namespace jobsystem
{

// Observations of one TaskExecutor run, for evidence and the J1 checks.
struct TaskRunStats
{
	bool inlineRun = false;      // no task was posted: everything ran on the caller
	uint32_t budget = 0;         // most runner tasks at once (the backend's worker count)
	uint32_t tasksPosted = 0;    // runner tasks the backend accepted
	uint32_t tasksRefused = 0;   // runner tasks the backend refused
	uint32_t tasksRan = 0;       // accepted runners that started
	uint32_t tasksWithdrawn = 0; // accepted runners withdrawn unstarted at the end
	uint32_t peakRunners = 0;    // most runner tasks live at once
	uint32_t jobsOnRunners = 0;  // jobs whose function ran on a runner task
	uint32_t jobsOnCaller = 0;   // jobs whose function ran on the calling thread
	uint32_t jobsOnBound = 0;    // jobs whose function ran on a bound runner
	// Times the scheduler left ready compute work that no live runner would
	// take while it had a free runner slot and no post had been refused. The
	// J1 invariant says zero.
	uint32_t uncoveredReady = 0;
};

class TaskExecutor : public IGraphExecutor
{
public:
	// Borrowed; must outlive Execute().
	explicit TaskExecutor( IWorkerBackend *backend ) : m_backend( backend ) {}

	RunResult Execute( const SealedGraph &graph, const RunOptions &opts ) override
	{
		return Execute( graph, opts, nullptr );
	}

	// As Execute, and reports the run's observations in *stats when non-null.
	RunResult Execute( const SealedGraph &graph, const RunOptions &opts, TaskRunStats *stats );

private:
	IWorkerBackend *m_backend;
};

} // namespace jobsystem

#endif // JOBSYSTEM_TASK_EXECUTOR_H
