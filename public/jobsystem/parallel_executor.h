//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Bounded worker-pool executor for a SealedGraph (RFC 0003, Phase C).
//
//          Implements IGraphExecutor with a fixed worker budget. Only jobs whose
//          prerequisites are all terminal enter the ready queue; a worker never
//          blocks on a dependency, it takes the next ready job instead. Producer
//          output is published to consumers through the scheduling mutex
//          (release on completion / acquire on dequeue), satisfying the RFC's
//          happens-before requirement without a bespoke fence.
//
//          This first implementation uses synchronized per-lane ready queues,
//          which the RFC explicitly prefers until a work-stealing deque has its
//          own correctness and performance evidence. It must produce identical
//          terminal states to DeterministicExecutor for the same inputs.
//
//          Lane affinity (RFC 0003 "Sequences and physical affinity",
//          "Blocking operations and external completion"):
//            * Compute / Sequence jobs run on any compute worker or the pumping
//              main thread. Sequence ordering is carried by the sealed edges, so
//              successive jobs on a lane may use different physical threads.
//            * MainThread jobs run only on the calling (main) thread, and only
//              when it pumps. A worker never steals them.
//            * BlockingIO jobs run on a dedicated blocking lane (or the main
//              thread when no blocking workers exist); they never occupy a
//              compute worker.
//          If the configuration provides no servicer for a lane the graph uses,
//          the run reports RunResult::stalled instead of hanging.
//
//=============================================================================//

#ifndef JOBSYSTEM_PARALLEL_EXECUTOR_H
#define JOBSYSTEM_PARALLEL_EXECUTOR_H

#ifdef _WIN32
#pragma once
#endif

#include <memory>

#include "jobsystem/graph_executor.h"

namespace jobsystem
{

class ParallelExecutor : public IGraphExecutor
{
public:
	// nComputeWorkers == 0 runs the ready-driven loop entirely on the calling
	// thread (still dependency-correct, no dedicated threads, services every
	// lane). Otherwise nComputeWorkers threads service Compute/Sequence work while
	// the caller pumps main-thread work and helps. nBlockingWorkers dedicated
	// threads service BlockingIO work; when 0, the pumping main thread services it
	// instead so compute workers never absorb a blocking wait.
	explicit ParallelExecutor( int nComputeWorkers, int nBlockingWorkers = 0 );
	~ParallelExecutor() override;

	// A copy has the same configuration and its own (initially idle) workers.
	ParallelExecutor( const ParallelExecutor &other );
	ParallelExecutor &operator=( const ParallelExecutor &other );

	RunResult Execute( const SealedGraph &graph, const RunOptions &opts ) override;

private:
	// Once an executor is reused, its worker threads persist and park between
	// runs, so repeated runs do not pay thread creation. The first run, and a
	// run that finds them busy (a concurrent or nested Execute), use transient
	// workers instead; both paths share one scheduling implementation.
	class WorkerPool;

	int m_nWorkers;
	int m_nBlocking;
	std::unique_ptr<WorkerPool> m_pool;
};

} // namespace jobsystem

#endif // JOBSYSTEM_PARALLEL_EXECUTOR_H
