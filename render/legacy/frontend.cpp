//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.legacy-frontend (RFC 0016); see core_backend.h. Built in
//			the legacy-interop dialect because RenderStageMarkers001 is an
//			IAppSystem.
//
//=============================================================================//

#include "frontend_material_blocks.h"
#include "queued_capabilities.h"
#include "render/legacy/core_backend.h"
#include "render/legacy/stage_markers.h"

#include <utility>

namespace render::legacy
{

static_assert( static_cast<int>( frame::Stage::kFrameBegin ) == RENDER_STAGE_FRAME_BEGIN &&
                   static_cast<int>( frame::Stage::kViewBegin ) == RENDER_STAGE_VIEW_BEGIN &&
                   static_cast<int>( frame::Stage::kSkybox ) == RENDER_STAGE_SKYBOX &&
                   static_cast<int>( frame::Stage::kOpaque ) == RENDER_STAGE_OPAQUE &&
                   static_cast<int>( frame::Stage::kTranslucent ) == RENDER_STAGE_TRANSLUCENT &&
                   static_cast<int>( frame::Stage::kViewModel ) == RENDER_STAGE_VIEW_MODEL &&
                   static_cast<int>( frame::Stage::kPostProcess ) == RENDER_STAGE_POST_PROCESS &&
                   static_cast<int>( frame::Stage::kViewEnd ) == RENDER_STAGE_VIEW_END &&
                   static_cast<int>( frame::Stage::kHud ) == RENDER_STAGE_HUD &&
                   static_cast<int>( frame::Stage::kFrameEnd ) == RENDER_STAGE_FRAME_END &&
                   static_cast<int>( frame::Stage::kCount ) == RENDER_STAGE_COUNT,
    "RenderStageMarker values are the frame port's stages" );

namespace
{

class StageMarkers final : public CBaseAppSystem<IRenderStageMarkers>
{
public:
	bool MarkStage( RenderStageMarker stage ) override
	{
		// The engine owns the frame's begin and end.
		if ( !m_Renderer || stage <= RENDER_STAGE_FRAME_BEGIN || stage >= RENDER_STAGE_FRAME_END )
		{
			++m_Violations;
			return false;
		}
		const bool ordered = m_Renderer->MarkStage( static_cast<frame::Stage>( stage ) );
		m_Violations += ordered ? 0u : 1u;
		return ordered;
	}

	unsigned int GetOrderViolations() const override { return m_Violations; }

	frame::IRenderer *m_Renderer = nullptr;
	unsigned int m_Violations = 0;
};

class LegacyStreamFeature final : public frame::IRenderFeature
{
public:
	const char *Name() const override { return "legacy-stream"; }
	frame::FeatureRequirements Requirements() const override { return {}; }

	// The legacy stream draws into the frame target. Until K3 it still draws
	// through the legacy backend directly; this pass holds its place.
	void AddPasses( frame::FeatureContext &context ) override
	{
		context.graph.AddPass( "legacy-stream", graph::PassKind::kRender )
		    .Write( context.target, device::ResourceUsage::kColorAttachment )
		    .Execute(
		        []( graph::RecordContext &record )
		        {
			        record.Encoder().BeginLabel(
			            "legacy stream (recorded by the backend until K3)" );
			        record.Encoder().EndLabel();
		        } );
	}
};

class LegacyFrontend final : public ILegacyFrontend
{
public:
	explicit LegacyFrontend( const LegacyShaderProvider *backend ) : m_Backend( backend )
	{
		if ( backend )
		{
			m_Provider.id = backend->id;
			m_Provider.legacyModuleName = backend->legacyModuleName;
			m_Provider.supportsQueuedRendering = backend->supportsQueuedRendering;
			m_Provider.create = nullptr;
			m_Provider.context = this;
			m_Provider.createFor = &LegacyFrontend::Create;
		}
	}

	const LegacyShaderProvider *Provider() const override
	{
		return m_Backend ? &m_Provider : nullptr;
	}
	IRenderStageMarkers *Markers() override { return &m_Markers; }
	IRenderMaterialBlocks *MaterialBlocks() override { return &m_MaterialBlocks; }
	std::unique_ptr<frame::IRenderFeature> CreateStreamFeature() override
	{
		return std::make_unique<LegacyStreamFeature>();
	}
	void BindRenderer( frame::IRenderer *renderer ) override { m_Markers.m_Renderer = renderer; }
	std::uint32_t ProviderCreates() const override { return m_Creates; }
	ILegacyCapabilities *Capabilities() override { return &m_Capabilities; }
	void BindRenderCallQueue( const RenderCallQueueHost *host ) override
	{
		m_Capabilities.BindQueue( host );
	}

private:
	static bool Create( void *context, LegacyShaderServices *services )
	{
		LegacyFrontend *self = static_cast<LegacyFrontend *>( context );
		const LegacyShaderProvider *backend = self->m_Backend;
		const bool created = backend->createFor ? backend->createFor( backend->context, services )
		                                        : backend->create && backend->create( services );
		self->m_Creates += created ? 1u : 0u;
		if ( created )
			self->m_Capabilities.Adopt( *services );
		return created;
	}

	const LegacyShaderProvider *m_Backend;
	LegacyShaderProvider m_Provider = {};
	StageMarkers m_Markers;
	FrontendMaterialBlocks m_MaterialBlocks;
	QueuedCapabilities m_Capabilities;
	std::uint32_t m_Creates = 0;
};

} // namespace

std::unique_ptr<ILegacyFrontend> CreateLegacyFrontend( const LegacyShaderProvider *backend )
{
	return std::make_unique<LegacyFrontend>( backend );
}

} // namespace render::legacy
