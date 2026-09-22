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
//          This first implementation uses one synchronized ready queue, which
//          the RFC explicitly prefers until a work-stealing deque has its own
//          correctness and performance evidence. It must produce identical
//          terminal states to DeterministicExecutor for the same inputs.
//
//=============================================================================//

#ifndef JOBSYSTEM_PARALLEL_EXECUTOR_H
#define JOBSYSTEM_PARALLEL_EXECUTOR_H

#ifdef _WIN32
#pragma once
#endif

#include "jobsystem/graph_executor.h"

namespace jobsystem
{

class ParallelExecutor : public IGraphExecutor
{
public:
	// nWorkers == 0 runs the ready-driven loop on the calling thread (still
	// dependency-correct, no dedicated threads). Otherwise nWorkers threads
	// service the graph while the caller waits for completion.
	explicit ParallelExecutor( int nWorkers );

	RunResult Execute( const SealedGraph &graph, const RunOptions &opts ) override;

private:
	int m_nWorkers;
};

} // namespace jobsystem

#endif // JOBSYSTEM_PARALLEL_EXECUTOR_H
