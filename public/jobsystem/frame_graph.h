//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Host frame-graph composition (RFC 0003, "Tick and render-frame
//          composition" and "Game-system integration").
//
//          The host emits one ordered legacy node per existing phase, on a
//          single sequence lane, preserving current call order. A migrated
//          subsystem implements IFrameContributor and inserts gather / compute
//          / commit nodes into a designated region between two phases, without
//          reordering the legacy phases around it. Running with contributors
//          disabled is the legacy/rollback path.
//
//          The coordinator only *composes*; it does not own subsystem state and
//          has no hidden access to a contributor's private data.
//
//=============================================================================//

#ifndef JOBSYSTEM_FRAME_GRAPH_H
#define JOBSYSTEM_FRAME_GRAPH_H

#ifdef _WIN32
#pragma once
#endif

#include <vector>

#include "jobsystem/job_graph.h"
#include "jobsystem/graph_executor.h"

namespace jobsystem
{

// Handed to each contributor while the frame graph is being built. The anchors
// bracket the region a contributor may populate: its work runs after inputs are
// stable and its commit must complete before the publish phase.
struct FrameBuildContext
{
	FrameContext frame;
	bool         parallelHint = false; // may influence batch sizing only; never correctness
	JobHandle    afterInputs;          // stable-inputs phase; compute depends on this
	JobHandle    beforeCommit;         // publish phase; depends on the contributor's commit
};

// A migrated subsystem that contributes frame jobs. It receives a scoped builder
// and declared bindings; it does not gain a place in the legacy IGameSystem
// vtable (RFC 0003 "Game-system integration").
class IFrameContributor
{
public:
	virtual ~IFrameContributor() = default;
	virtual const char *Name() const = 0;
	virtual void Contribute( JobGraphBuilder &builder, const FrameBuildContext &ctx ) = 0;
};

// Composes ordered legacy phases with optional migrated contributors into one
// sealed frame graph. Phases run in registration order on sequence lane 0.
class FrameCoordinator
{
public:
	explicit FrameCoordinator( const FrameContext &frame ) : m_frame( frame ) {}

	// Register an ordered phase; returns its handle for anchoring. fn may be
	// empty (a pure ordering marker). Must be called before Build().
	JobHandle AddLegacyPhase( const char *name, JobEntry fn = JobEntry{} );

	// Borrowed; must outlive Build(). Contributors run inside the region between
	// the phase most recently marked as the stable-inputs anchor and the phase
	// marked as the publish anchor.
	void AddContributor( IFrameContributor *c ) { m_contributors.push_back( c ); }

	// Designate which already-added phases bracket the contributor region.
	void SetContributorRegion( JobHandle afterInputs, JobHandle beforeCommit )
	{
		m_afterInputs = afterInputs; m_beforeCommit = beforeCommit; m_regionSet = true;
	}

	// Build the sealed graph. When runContributors is false, only the legacy
	// phases are emitted (the rollback / legacy-comparison path).
	Expected<SealedGraph, GraphError> Build( bool runContributors );

	const FrameContext &Frame() const { return m_frame; }

private:
	struct Phase { const char *name; JobEntry fn; JobHandle handle; };

	FrameContext m_frame;
	std::vector<Phase> m_phases;
	std::vector<IFrameContributor *> m_contributors;
	JobHandle m_afterInputs;
	JobHandle m_beforeCommit;
	bool m_regionSet = false;
};

} // namespace jobsystem

#endif // JOBSYSTEM_FRAME_GRAPH_H
