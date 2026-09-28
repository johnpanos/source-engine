//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The legacy frame executor (RFC 0016 K3); see frame_source.h.
//
//=============================================================================//

#include "render/graph/compiled_graph.h"
#include "render/graph/executor.h"
#include "render/legacy/frame_source.h"

#include <string>
#include <utility>

namespace render::legacy
{

namespace
{

class Executor final : public ILegacyFrameExecutor
{
public:
	bool RunFrame( ILegacyFrameSource &source ) override
	{
		bool skip = false;
		if ( !source.Prepare( &skip ) )
			return false;
		if ( skip )
			return true;
		// One side-effect pass per stage: the compiler keeps side-effect
		// passes in declaration order, and the serial executor records them
		// on one encoder, so the stages form one submission in order.
		graph::GraphBuilder builder;
		bool recorded = true;
		for ( unsigned int i = 0; i < static_cast<unsigned int>( LegacyFrameStage::kCount ); ++i )
		{
			const LegacyFrameStage stage = static_cast<LegacyFrameStage>( i );
			if ( !source.HasStage( stage ) )
				continue;
			builder
			    .AddPass( std::string( "legacy-" ) + LegacyFrameStageName( stage ),
			        graph::PassKind::kRender )
			    .SideEffect()
			    .Execute(
			        [&source, &recorded, stage]( graph::RecordContext &context )
			        {
				        recorded = source.RecordStage( stage, context.Encoder() ) && recorded;
			        } );
		}
		auto compiled = graph::CompileGraph( std::move( builder ) );
		if ( !compiled )
		{
			(void)source.Finish( {}, false );
			return false;
		}
		auto executed = m_Executor.Execute( compiled.Value(), source.Device() );
		const bool submitted = executed.HasValue();
		m_LastPasses = submitted ? executed.Value().passes : 0u;
		const bool finished = source.Finish(
		    submitted ? executed.Value().token : device::CompletionToken{}, submitted );
		++m_Frames;
		return submitted && recorded && finished;
	}

	unsigned long long Frames() const override { return m_Frames; }
	unsigned int LastFramePasses() const override { return m_LastPasses; }

private:
	graph::SerialGraphExecutor m_Executor;
	unsigned long long m_Frames = 0;
	unsigned int m_LastPasses = 0;
};

} // namespace

ILegacyFrameExecutor &LegacyFrameExecutor()
{
	static Executor executor;
	return executor;
}

} // namespace render::legacy
