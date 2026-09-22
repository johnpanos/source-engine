//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Graph executor that runs on an external IWorkerBackend (RFC 0003).
//
//          PooledExecutor drives a SealedGraph in dependency "waves": it
//          repeatedly gathers the jobs whose prerequisites are all terminal and
//          runs that independent set through the backend's ParallelFor barrier,
//          then advances readiness. This maps directly onto the engine's
//          existing ParallelProcess fork/join primitive (the mechanism particle,
//          bone, query-cache and packing seams already use), so real engine
//          worker pools can execute job graphs. It must agree with the
//          Deterministic/Parallel executors on every job's terminal state.
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
	// Borrowed; must outlive Execute(). If null, work runs inline on the caller.
	explicit PooledExecutor( IWorkerBackend *backend ) : m_backend( backend ) {}

	RunResult Execute( const SealedGraph &graph, const RunOptions &opts ) override;

private:
	IWorkerBackend *m_backend;
};

} // namespace jobsystem

#endif // JOBSYSTEM_POOLED_EXECUTOR_H
