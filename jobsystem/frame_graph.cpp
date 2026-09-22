//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: FrameCoordinator composition (RFC 0003).
//
//=============================================================================//

#include "jobsystem/frame_graph.h"

#include <utility>

namespace jobsystem
{

JobHandle FrameCoordinator::AddLegacyPhase( const char *name, JobEntry fn )
{
	// Pre-assign the handle id: phases are emitted first, in this order, so the
	// builder will assign matching ids in Build().
	JobHandle h{ (uint32_t)m_phases.size() };
	m_phases.push_back( Phase{ name, std::move( fn ), h } );
	return h;
}

Expected<SealedGraph, GraphError> FrameCoordinator::Build( bool runContributors )
{
	JobGraphBuilder b;

	// Ordered legacy phases on sequence lane 0 (registration order preserved).
	for ( const Phase &p : m_phases )
	{
		JobDesc d;
		d.name     = p.name;
		d.executor = Executor::Sequence( 0 );
		d.function = p.fn;
		JobHandle h = b.AddJob( d );
		(void)h; // id equals p.handle.id by construction
	}

	if ( runContributors && !m_contributors.empty() )
	{
		if ( !m_regionSet )
		{
			return MakeUnexpected( GraphError{ GraphErrorCode::InvalidHandle,
				"contributor region not set; call SetContributorRegion first",
				JobHandle::kInvalid, JobHandle::kInvalid } );
		}
		FrameBuildContext ctx;
		ctx.frame        = m_frame;
		ctx.parallelHint = true;
		ctx.afterInputs  = m_afterInputs;
		ctx.beforeCommit = m_beforeCommit;
		for ( IFrameContributor *c : m_contributors )
		{
			c->Contribute( b, ctx );
		}
	}

	return b.Seal();
}

} // namespace jobsystem
