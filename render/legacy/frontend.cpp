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

#include <atomic>
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

// The frontend's core-pass recorder (core_passes.h). Until the world and
// props draw from the scene (K5 plan, step 4) it records a probe: nothing
// but a label (kEmpty, whose frames must equal a frame without slots), or a
// clear of the slot's color target (kSeededClear, the probe's negative
// control).
class CorePassRecorder final : public ICorePassRecorder
{
public:
	void SetProbe( CorePassProbe probe ) { m_Probe = probe; }

	std::uint32_t SlotStages() const override
	{
		const std::uint32_t probe = m_Probe == CorePassProbe::kNone
		                                ? 0u
		                                : 1u << static_cast<std::uint32_t>( frame::Stage::kOpaque );
		return probe | ( m_Forwarded ? m_Forwarded->SlotStages() : 0u );
	}

	void SetForwarded( ICorePassRecorder *recorder ) { m_Forwarded = recorder; }
	bool AcceptsMeshes() const override { return m_Forwarded && m_Forwarded->AcceptsMeshes(); }

	std::uint32_t QueueMesh( const legacy::CoreMeshDraw &draw ) override
	{
		return m_Forwarded ? m_Forwarded->QueueMesh( draw ) : 0;
	}

	std::size_t RecordOpaqueBatch( std::span<const std::uint32_t> tags,
	    device::CommandEncoder &encoder, const CorePassTarget &target ) override
	{
		if ( tags.empty() )
			return 0;
		if ( !m_Forwarded || !( tags.front() & kCorePassForwarded ) )
			return ICorePassRecorder::RecordOpaqueBatch( tags, encoder, target );
		const auto end = std::find_if( tags.begin(), tags.end(),
		    []( auto tag )
		    {
			    return !( tag & kCorePassForwarded );
		    } );
		const auto count = m_Forwarded->RecordOpaqueBatch(
		    tags.first( std::size_t( end - tags.begin() ) ), encoder, target );
		m_Recorded.fetch_add( count, std::memory_order_relaxed );
		return count;
	}

	void RecordSlot(
	    std::uint32_t tag, device::CommandEncoder &encoder, const CorePassTarget &target ) override
	{
		m_Recorded.fetch_add( 1, std::memory_order_relaxed );
		if ( ( tag & kCorePassForwarded ) || tag == kCorePassLegacyHud )
		{
			if ( m_Forwarded )
				m_Forwarded->RecordSlot( tag, encoder, target );
			return;
		}
		encoder.BeginLabel( "core pass" );
		if ( m_Probe == CorePassProbe::kSeededClear && target.color.IsValid() )
		{
			const device::ColorAttachment colors[] = { { target.color, device::LoadOp::kClear,
			    device::StoreOp::kStore, { 1.0f, 0.0f, 1.0f, 1.0f }, {} } };
			device::RenderingDesc rendering;
			rendering.colors = colors;
			rendering.width = target.width;
			rendering.height = target.height;
			encoder.BeginRendering( rendering );
			encoder.EndRendering();
		}
		encoder.EndLabel();
		(void)tag;
	}

	// The output belongs to the composition's forwarded recorder, which may
	// name the output pass (a feature, beside this frontend in the layers).
	bool RecordOutput(
	    device::CommandEncoder &encoder, const CoreOutputTargets &targets ) override
	{
		return m_Forwarded && m_Forwarded->RecordOutput( encoder, targets );
	}

	void FrameSubmitted( device::CompletionToken token, bool submitted ) override
	{
		if ( m_Forwarded )
			m_Forwarded->FrameSubmitted( token, submitted );
	}
	void ReleaseDevice( device::IRenderDevice2 &device ) override
	{
		if ( m_Forwarded )
			m_Forwarded->ReleaseDevice( device );
	}

	std::uint64_t Recorded() const { return m_Recorded.load( std::memory_order_relaxed ); }

private:
	CorePassProbe m_Probe = CorePassProbe::kNone;
	ICorePassRecorder *m_Forwarded = nullptr;
	std::atomic<std::uint64_t> m_Recorded{ 0 };
};

// Queues a slot, in frame order, at each stage the recorder asks for.
class CoreSlotHook final : public frame::IRenderStageHooks
{
public:
	CoreSlotHook( const CorePassRecorder &recorder, QueuedCapabilities &capabilities )
	    : m_Recorder( recorder ), m_Capabilities( capabilities )
	{
	}

	void OnStage( frame::Stage stage, std::uint32_t depth ) override
	{
		const std::uint32_t bit = 1u << static_cast<std::uint32_t>( stage );
		if ( !( m_Recorder.SlotStages() & bit ) )
			return;
		if ( ICorePassSlots *slots = m_Capabilities.CorePassSlots() )
			slots->MarkSlot( CorePassTag( static_cast<std::uint32_t>( stage ), depth ) );
	}

private:
	const CorePassRecorder &m_Recorder;
	QueuedCapabilities &m_Capabilities;
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
	void BindRenderer( frame::IRenderer *renderer ) override
	{
		if ( m_Markers.m_Renderer )
			m_Markers.m_Renderer->RemoveStageHooks( &m_SlotHook );
		m_Markers.m_Renderer = renderer;
		if ( renderer )
			renderer->AddStageHooks( &m_SlotHook );
	}
	ICorePassRecorder *CorePasses() override { return &m_CorePasses; }
	void SetCorePassProbe( CorePassProbe probe ) override { m_CorePasses.SetProbe( probe ); }
	std::uint64_t CorePassesRecorded() const override { return m_CorePasses.Recorded(); }
	void SetForwardedRecorder( ICorePassRecorder *recorder ) override
	{
		m_CorePasses.SetForwarded( recorder );
	}
	ICorePassSlots *CorePassSlots() override { return m_Capabilities.CorePassSlots(); }
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
	CorePassRecorder m_CorePasses;
	CoreSlotHook m_SlotHook{ m_CorePasses, m_Capabilities };
	std::uint32_t m_Creates = 0;
};

} // namespace

std::unique_ptr<ILegacyFrontend> CreateLegacyFrontend( const LegacyShaderProvider *backend )
{
	return std::make_unique<LegacyFrontend>( backend );
}

} // namespace render::legacy
