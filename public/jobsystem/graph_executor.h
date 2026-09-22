//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Execution contracts for a SealedGraph (RFC 0003, Phase B/C).
//
//          An IGraphExecutor runs a sealed graph exactly once. Two executors
//          implement the same contract and MUST agree on every job's terminal
//          state for the same inputs:
//            * DeterministicExecutor - a single-threaded reference that runs in
//              the sealed stable topological order. Used for validation and as
//              the canonical order in three-mode equivalence tests.
//            * ParallelExecutor      - a bounded worker pool (parallel_executor.h).
//
//          Dependency semantics (RFC 0003 "Job state, errors, and cancellation"):
//            * A Success prerequisite that did not Succeed cancels its consumer;
//              the consumer never runs and never reads absent output as valid.
//            * A Terminal prerequisite only requires the producer to reach a
//              terminal state; cleanup/error continuations use this.
//          An unfinished job never occupies a worker merely to wait.
//
//=============================================================================//

#ifndef JOBSYSTEM_GRAPH_EXECUTOR_H
#define JOBSYSTEM_GRAPH_EXECUTOR_H

#ifdef _WIN32
#pragma once
#endif

#include <atomic>
#include <cstdint>
#include <vector>

#include "jobsystem/job_graph.h"

namespace jobsystem
{

// Explicit per-run time and epoch context. Jobs read time from here rather than
// mutating shared globals (RFC 0003 "Time, randomness, and numerical behavior").
struct FrameContext
{
	double   tickTime  = 0.0;
	double   frameTime  = 0.0;
	uint64_t epoch      = 0;
	void    *user       = nullptr; // opaque subsystem frame binding
};

// Passed to each job when it runs.
class JobRunContext
{
public:
	JobRunContext( const FrameContext &frame, uint32_t jobId )
		: m_frame( frame ), m_jobId( jobId ) {}

	const FrameContext &Frame() const { return m_frame; }
	double   TickTime()  const { return m_frame.tickTime; }
	double   FrameTime() const { return m_frame.frameTime; }
	uint32_t JobId()     const { return m_jobId; }

	// Cooperative failure: the job reports it could not produce valid output.
	void Fail()        { m_failed = true; }
	bool Failed() const { return m_failed; }

private:
	const FrameContext &m_frame;
	uint32_t m_jobId;
	bool     m_failed = false;
};

// Optional per-job lifecycle tracing (RFC 0003 "Tracing"). Called synchronously
// by the executing worker; implementations must keep overhead bounded and be
// safe to call from multiple workers concurrently.
class ITraceSink
{
public:
	virtual ~ITraceSink() = default;
	virtual void OnJobState( uint32_t jobId, const char *name, ExecutorToken executor, JobState state ) = 0;
};

struct RunOptions
{
	FrameContext frame;
	ITraceSink  *trace  = nullptr;
	// Cooperative cancellation. Checked before a job starts; running jobs are
	// never forcibly terminated. May be null.
	const std::atomic<bool> *cancel = nullptr;

	// Whether the calling thread services main-thread-affine jobs (an explicit
	// host pump point). When false and the graph carries MainThread work with no
	// other eligible servicer, the run is a detectable stall rather than a hang
	// (RFC 0003 "Sequences and physical affinity"). Executors that model no
	// affinity (Deterministic, Pooled) ignore this.
	bool pumpMainThread = true;
};

// Result of one graph execution: the terminal state of every job by id.
struct RunResult
{
	std::vector<JobState> states;
	uint32_t succeeded = 0;
	uint32_t failed    = 0;
	uint32_t canceled  = 0;
	uint32_t executed  = 0; // jobs whose function actually ran

	// True when the graph declared work for an executor lane that the run's
	// configuration provides no servicer for (e.g. MainThread jobs without a
	// main-thread pump, or BlockingIO jobs without a blocking lane). Such jobs and
	// their Success-dependents are left non-terminal; the run made no false claim
	// of completion. This is the "wait that cannot make progress" diagnostic.
	bool     stalled    = false;
	uint32_t unresolved = 0; // jobs left non-terminal because of a stall

	bool AllSucceeded() const { return failed == 0 && canceled == 0 && !stalled; }
};

class IGraphExecutor
{
public:
	virtual ~IGraphExecutor() = default;
	virtual RunResult Execute( const SealedGraph &graph, const RunOptions &opts ) = 0;
};

// Single-threaded reference executor: runs jobs in graph.TopoOrder().
class DeterministicExecutor : public IGraphExecutor
{
public:
	RunResult Execute( const SealedGraph &graph, const RunOptions &opts ) override;
};

} // namespace jobsystem

#endif // JOBSYSTEM_GRAPH_EXECUTOR_H
