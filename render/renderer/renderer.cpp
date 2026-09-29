//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.renderer (RFC 0016); see renderer_factory.h.
//
//=============================================================================//

#include "render/graph/compiled_graph.h"
#include "render/graph/executor.h"
#include "render/renderer/renderer_factory.h"

#include <algorithm>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace render::renderer
{

namespace
{

class Renderer final : public frame::IRenderer
{
public:
	Renderer( device::IRenderDevice2 &device,
	    std::vector<std::unique_ptr<frame::IRenderFeature>> features,
	    std::vector<std::string> debugPrograms )
	    : m_Device( device ), m_Features( std::move( features ) ), m_Transients( device ),
	      m_Executor( m_Transients ), m_DebugPrograms( std::move( debugPrograms ) )
	{
		for ( const std::string &name : m_DebugPrograms )
			m_DebugProgramNames.push_back( name );
	}

	foundation::Expected<void, frame::FrameError> BeginFrame(
	    const frame::FrameDesc &desc ) override
	{
		if ( m_InFrame )
			return Fail( frame::FrameStatus::kAlreadyInFrame );
		if ( !desc.target.IsValid() && ( desc.width == 0 || desc.height == 0 ) )
			return Fail( frame::FrameStatus::kInvalidFrame );
		m_Frame = desc;
		m_InFrame = true;
		m_Current = frame::FrameStats();
		// The debug controls apply to the whole frame; a refused value keeps
		// the previous one (RFC 0014).
		if ( auto valid = frame::ValidateDebugControls( desc.debug, m_DebugProgramNames ); valid )
			m_AppliedDebug = desc.debug;
		else
		{
			m_LastDebugRejection = valid.Error();
			++m_Current.debugRejected;
			++m_Totals.debugRejected; // counted at once: the caller reports it this frame
		}
		m_Frame.debug = m_AppliedDebug;
		m_Tracker.Reset();
		(void)Record( frame::Stage::kFrameBegin );
		return {};
	}

	bool MarkStage( frame::Stage stage ) override
	{
		if ( !m_InFrame || stage == frame::Stage::kFrameBegin || stage == frame::Stage::kFrameEnd )
		{
			++m_Totals.orderViolations;
			return false;
		}
		return Record( stage );
	}

	foundation::Expected<frame::FrameStats, frame::FrameError> EndFrame() override
	{
		if ( !m_InFrame )
			return Fail( frame::FrameStatus::kNotInFrame );
		(void)Record( frame::Stage::kFrameEnd );
		m_InFrame = false;
		m_Current.views = m_Tracker.Views();

		graph::GraphBuilder builder;
		graph::ResourceRef target;
		if ( m_Frame.target.IsValid() )
		{
			target = builder.ImportTexture( "frame-target", m_Frame.target, m_Frame.targetDesc,
			    m_Frame.targetUsage, m_Frame.targetFinal );
		}
		else
		{
			device::TextureDesc desc;
			desc.format = device::Format::kRGBA8Unorm;
			desc.width = m_Frame.width;
			desc.height = m_Frame.height;
			target = builder.CreateTexture( "frame-target", desc );
		}
		frame::FeatureContext context{ builder, m_Frame, target, m_Current.stagesMarked,
		    static_cast<std::uint32_t>( m_Current.views ) };
		for ( const std::unique_ptr<frame::IRenderFeature> &feature : m_Features )
			feature->AddPasses( context );

		auto graph = graph::CompileGraph( std::move( builder ) );
		if ( !graph )
		{
			++m_Totals.failedFrames;
			frame::FrameError error;
			error.status = frame::FrameStatus::kGraph;
			error.graphStatus = static_cast<std::uint32_t>( graph.Error().status );
			return foundation::MakeUnexpected( error );
		}
		auto result = m_Executor.Execute( graph.Value(), m_Device );
		(void)m_Device.Poll();
		if ( !result )
		{
			++m_Totals.failedFrames;
			frame::FrameError error;
			error.status = frame::FrameStatus::kDevice;
			error.device = result.Error();
			return foundation::MakeUnexpected( error );
		}
		m_Current.frames = 1;
		m_Current.passes = result.Value().passes;
		m_Current.culledPasses = graph.Value().trace.culled.size();
		m_Current.transitions = result.Value().transitions;
		m_Current.transientsCreated = result.Value().transients;
		m_Current.transientsReused = result.Value().reused;
		m_Current.lastToken = result.Value().token;

		m_Totals.frames += 1;
		m_Totals.stagesMarked += m_Current.stagesMarked;
		m_Totals.orderViolations += m_Current.orderViolations;
		m_Totals.views += m_Current.views;
		m_Totals.passes += m_Current.passes;
		m_Totals.culledPasses += m_Current.culledPasses;
		m_Totals.transitions += m_Current.transitions;
		m_Totals.transientsCreated += m_Current.transientsCreated;
		m_Totals.transientsReused += m_Current.transientsReused;
		m_Totals.lastToken = m_Current.lastToken;
		return m_Current;
	}

	const frame::FrameStats &Totals() const override { return m_Totals; }
	const frame::DebugControls &AppliedDebug() const override { return m_AppliedDebug; }
	const frame::DebugControlsError &LastDebugRejection() const override
	{
		return m_LastDebugRejection;
	}
	std::size_t DebugProgramCount() const override { return m_DebugPrograms.size(); }
	const char *DebugProgramName( std::size_t index ) const override
	{
		return index < m_DebugPrograms.size() ? m_DebugPrograms[index].c_str() : nullptr;
	}
	bool ParseDebugTerms( const char *names, std::uint32_t *bits, char *unknown,
	    std::size_t unknownBytes ) const override
	{
		return frame::ParseDebugTerms( names, bits, unknown, unknownBytes );
	}

	void AddStageHooks( frame::IRenderStageHooks *hooks ) override
	{
		if ( hooks && std::find( m_Hooks.begin(), m_Hooks.end(), hooks ) == m_Hooks.end() )
			m_Hooks.push_back( hooks );
	}

	void RemoveStageHooks( frame::IRenderStageHooks *hooks ) override
	{
		std::erase( m_Hooks, hooks );
	}

private:
	static foundation::Unexpected<frame::FrameError> Fail( frame::FrameStatus status )
	{
		frame::FrameError error;
		error.status = status;
		return foundation::MakeUnexpected( error );
	}

	bool Record( frame::Stage stage )
	{
		++m_Current.stagesMarked;
		const bool ordered = m_Tracker.Mark( stage );
		if ( !ordered )
			++m_Current.orderViolations;
		for ( frame::IRenderStageHooks *hooks : m_Hooks )
			hooks->OnStage( stage, m_Tracker.Depth() );
		return ordered;
	}

	device::IRenderDevice2 &m_Device;
	std::vector<std::unique_ptr<frame::IRenderFeature>> m_Features;
	// Frame transients persist across frames; released behind their token.
	graph::TransientPool m_Transients;
	graph::SerialGraphExecutor m_Executor;
	std::vector<frame::IRenderStageHooks *> m_Hooks;
	frame::StageTracker m_Tracker;
	frame::FrameDesc m_Frame;
	frame::FrameStats m_Current;
	frame::FrameStats m_Totals;
	bool m_InFrame = false;
	std::vector<std::string> m_DebugPrograms;
	std::vector<std::string_view> m_DebugProgramNames; // views of m_DebugPrograms
	frame::DebugControls m_AppliedDebug;
	frame::DebugControlsError m_LastDebugRejection{ frame::DebugControlsStatus( 0 ), {} };
};

} // namespace

foundation::Expected<std::unique_ptr<frame::IRenderer>, RendererError> CreateRenderer(
    RendererDeps &&deps )
{
	if ( !deps.device )
		return foundation::MakeUnexpected( RendererError{ RendererStatus::kNoDevice } );
	const device::DeviceFacts &facts = deps.device->Facts();
	for ( const std::unique_ptr<frame::IRenderFeature> &feature : deps.features )
	{
		if ( const std::optional<device::Capability> missing =
		         device::FirstMissing( facts.capabilities, feature->Requirements().required ) )
			return foundation::MakeUnexpected(
			    RendererError{ RendererStatus::kMissingCapability, feature->Name(), *missing } );
	}
	return std::unique_ptr<frame::IRenderer>( std::make_unique<Renderer>(
	    *deps.device, std::move( deps.features ), std::move( deps.debugPrograms ) ) );
}

} // namespace render::renderer
