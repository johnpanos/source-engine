//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Graph executor that runs on an external IWorkerBackend (RFC 0003).
//
//          PooledExecutor drives a SealedGraph in dependency "waves": it
//          repeatedly gathers the jobs whose prerequisites are all terminal and
//          runs that independent set through the backend's ParallelFor barrier
//          and the caller pump, then advances readiness. This maps onto the engine's
//          existing ParallelProcess fork/join primitive (the mechanism particle,
//          bone, query-cache and packing seams already use), so real engine
//          worker pools can execute job graphs. Compute/Sequence jobs may use
//          the backend; MainThread/BlockingIO jobs run only on the caller with
//          RunOptions::pumpMainThread enabled. No dedicated blocking executor
//          is provided. Unpumped lanes and their dependents remain nonterminal
//          and are reported as stalled. Within a wave, the caller's jobs run
//          while workers run the wave's compute jobs
//          (IWorkerBackend::ParallelForWithCaller); a wave ends when both have. A null/zero-worker backend runs every
//          lane inline, matching ParallelExecutor's low-capacity mode.
//
//          Execute is a synchronous host entry point: it must not be nested
//          inside a job on the same backend. The wave join does not pump caller
//          work of later waves while workers run. For serviceable graphs its terminal states
//          must agree with the Deterministic/Parallel executors.
//
//=============================================================================//

#ifndef JOBSYSTEM_POOLED_EXECUTOR_H
#define JOBSYSTEM_POOLED_EXECUTOR_H

#ifdef _WIN32
#pragma once
#endif

#include "jobsystem/graph_executor.h"
#include "jobsystem/worker_backend.h"

namespace jobsystem
{

class PooledExecutor : public IGraphExecutor
{
public:
	// Borrowed; must outlive Execute() and return only after all invocations join.
	// A null backend or one reporting zero workers runs inline on the caller.
	explicit PooledExecutor( IWorkerBackend *backend ) : m_backend( backend ) {}

	RunResult Execute( const SealedGraph &graph, const RunOptions &opts ) override;

private:
	IWorkerBackend *m_backend;
};

} // namespace jobsystem

#endif // JOBSYSTEM_POOLED_EXECUTOR_H
