//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Dependency-aware job graph contracts (RFC 0003, Phase B).
//
//          A graph is built privately, validated, sealed, and only then
//          submitted to an executor. No root job may start while edges or
//          resource declarations are still being added. Sealing validates
//          handles, detects cycles (including sequence-induced ordering
//          edges), and rejects ambiguous resource conflicts rather than
//          inventing an order from job-registration order.
//
//          These are internal C++20 contracts. They deliberately do not
//          extend the legacy module ABI or CreateInterface (RFC 0003
//          "Architectural placement"). Executors that run a SealedGraph live
//          in graph_executor.h.
//
//=============================================================================//

#ifndef JOBSYSTEM_JOB_GRAPH_H
#define JOBSYSTEM_JOB_GRAPH_H

#ifdef _WIN32
#pragma once
#endif

#include <cstdint>
#include <functional>
#include <vector>

#include "jobsystem/expected.h"

namespace jobsystem
{

//-----------------------------------------------------------------------------
// Handles and execution vocabulary
//-----------------------------------------------------------------------------

// Opaque, stable-within-a-graph reference to a job.
struct JobHandle
{
	static constexpr uint32_t kInvalid = 0xFFFFFFFFu;
	uint32_t id = kInvalid;

	bool IsValid() const { return id != kInvalid; }
	bool operator==( JobHandle o ) const { return id == o.id; }
	bool operator!=( JobHandle o ) const { return id != o.id; }
};

// An execution lane. The scheduler binds a lane to a runner; a lane in a frame
// diagram does not necessarily allocate a dedicated thread.
enum class ExecutorKind : uint8_t
{
	Compute,     // any eligible worker; the parallel executor may steal these
	MainThread,  // main-thread-affine; requires an explicit host pump point
	BlockingIO,  // long/blocking work; must not occupy compute workers
	Sequence,    // ordered, non-overlapping; successive jobs may use any thread
};

struct ExecutorToken
{
	ExecutorKind kind = ExecutorKind::Compute;
	uint16_t     lane = 0; // sequence index for Sequence; 0 otherwise

	bool operator==( ExecutorToken o ) const { return kind == o.kind && lane == o.lane; }
};

namespace Executor
{
	inline ExecutorToken Compute()             { return ExecutorToken{ ExecutorKind::Compute,    (uint16_t)0 }; }
	inline ExecutorToken MainThread()          { return ExecutorToken{ ExecutorKind::MainThread, (uint16_t)0 }; }
	inline ExecutorToken BlockingIO()          { return ExecutorToken{ ExecutorKind::BlockingIO, (uint16_t)0 }; }
	inline ExecutorToken Sequence( uint16_t l ){ return ExecutorToken{ ExecutorKind::Sequence,   l }; }
}

// Importance among *runnable* jobs. Never bypasses dependencies and is not an
// OS real-time priority (RFC 0003 "capacity policy").
enum class JobPriority : uint8_t { Low, Normal, High };

// A resource identity for the validator: a domain, an epoch (tick/frame/level
// generation), and a disjoint partition. Two accesses conflict only when all
// three match; different partitions of the same domain/epoch are disjoint.
struct ResourceVersion
{
	uint32_t domain    = 0;
	uint32_t epoch     = 0;
	uint32_t partition = 0;

	bool operator==( const ResourceVersion &o ) const
	{
		return domain == o.domain && epoch == o.epoch && partition == o.partition;
	}
};

//-----------------------------------------------------------------------------
// Job execution callback
//-----------------------------------------------------------------------------

class JobRunContext;
// A job succeeds unless it calls Fail(). It receives explicit frame/tick context
// via the run context; migrated computation must not reach for shared globals to
// establish its time (RFC 0003 "Time, randomness, and numerical behavior").
using JobEntry = std::function<void( JobRunContext & )>;

// Terminal outcome of a single job.
enum class JobState : uint8_t
{
	Created,    // in a builder, not yet sealed
	Admitted,   // sealed and submitted, waiting on prerequisites
	Ready,      // prerequisites satisfied, eligible to run
	Running,    // executing on some lane
	Succeeded,  // ran and did not fail
	Failed,     // ran and reported failure
	Canceled,   // never ran: a prerequisite did not succeed, or scope canceled
};

inline bool IsTerminal( JobState s )
{
	return s == JobState::Succeeded || s == JobState::Failed || s == JobState::Canceled;
}

//-----------------------------------------------------------------------------
// Job description and dependency kinds
//-----------------------------------------------------------------------------

struct JobDesc
{
	const char *name        = "unnamed";   // stable diagnostic name (required)
	ExecutorToken executor  = Executor::Compute();
	JobPriority   priority  = JobPriority::Normal;
	JobEntry      function;                // may be empty (a pure join node)
};

// Success:  consumer runs only if the producer Succeeded; otherwise Canceled.
// Terminal: consumer runs once the producer reaches any terminal state. Used by
//           cleanup/error continuations, which have no right to read an absent
//           output as valid data (RFC 0003 "Job state, errors, and cancellation").
enum class DependencyKind : uint8_t { Success, Terminal };

//-----------------------------------------------------------------------------
// Seal-time validation errors
//-----------------------------------------------------------------------------

enum class GraphErrorCode : uint8_t
{
	None,
	EmptyName,         // a job has a null/empty diagnostic name
	InvalidHandle,     // a dependency/resource decl referenced an unknown job
	SelfDependency,    // a job depends on itself
	Cycle,             // explicit or sequence-induced ordering cycle
	ResourceConflict,  // unordered write/write or read/write on one version
};

struct GraphError
{
	GraphErrorCode code = GraphErrorCode::None;
	const char *detail  = "";
	uint32_t jobA       = JobHandle::kInvalid; // primary job involved
	uint32_t jobB       = JobHandle::kInvalid; // second job for pairwise errors
};

//-----------------------------------------------------------------------------
// SealedGraph: immutable, validated, ready to execute exactly once per run.
//-----------------------------------------------------------------------------

class SealedGraph
{
public:
	struct Prereq { uint32_t producer; DependencyKind kind; };

	struct Job
	{
		const char   *name;
		ExecutorToken executor;
		JobPriority   priority;
		JobEntry      function;
		std::vector<Prereq> prereqs;   // incoming edges (who must finish first)
		std::vector<uint32_t> dependents; // outgoing edges (who waits on me)
	};

	uint32_t JobCount() const { return (uint32_t)m_jobs.size(); }
	const Job &GetJob( uint32_t i ) const { return m_jobs[i]; }

	// A stable topological order honoring explicit + sequence edges, with ties
	// broken by registration order. The deterministic executor runs in this
	// order; the parallel executor uses it only as a canonical reference.
	const std::vector<uint32_t> &TopoOrder() const { return m_topoOrder; }

private:
	friend class JobGraphBuilder;
	std::vector<Job>      m_jobs;
	std::vector<uint32_t> m_topoOrder;
};

//-----------------------------------------------------------------------------
// JobGraphBuilder: private construction; Seal() validates and freezes.
//-----------------------------------------------------------------------------

class JobGraphBuilder
{
public:
	JobGraphBuilder() = default;

	JobHandle AddJob( const JobDesc &desc );

	// Ordering edge: producer must finish before consumer becomes eligible.
	void AddDependency( JobHandle producer, JobHandle consumer,
	                    DependencyKind kind = DependencyKind::Success );

	// Resource access declarations, checked against ordering at Seal().
	void Read ( JobHandle job, const ResourceVersion &resource );
	void Write( JobHandle job, const ResourceVersion &resource );

	// Validate and freeze. On success the builder's contents are moved into the
	// SealedGraph and the builder must not be reused. On failure the graph is
	// left unexecuted and a structured error is returned.
	Expected<SealedGraph, GraphError> Seal();

	uint32_t PendingJobCount() const { return (uint32_t)m_jobs.size(); }

private:
	struct BuildJob
	{
		JobDesc desc;
	};
	struct Edge { uint32_t producer; uint32_t consumer; DependencyKind kind; };
	struct ResourceDecl { uint32_t job; ResourceVersion resource; bool write; };

	bool ValidHandle( JobHandle h ) const { return h.IsValid() && h.id < m_jobs.size(); }

	std::vector<BuildJob>     m_jobs;
	std::vector<Edge>         m_edges;
	std::vector<ResourceDecl> m_resources;
	// Sequence lane -> ordered list of jobs assigned to it (for implicit edges).
	std::vector<std::pair<uint16_t, uint32_t>> m_sequenceMembers;
};

} // namespace jobsystem

#endif // JOBSYSTEM_JOB_GRAPH_H
