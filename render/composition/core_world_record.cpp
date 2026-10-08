//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.composition's world (RFC 0016 K5): recording the world pass at its slots.
//
//=============================================================================//

#include "core_world_internal.h"

namespace render::composition
{

namespace
{

// The backend's textures, as the world pass asks for them.
class Textures final : public pass::world::IWorldTextures
{
public:
	explicit Textures( legacy::ICoreTextures &textures ) : m_Textures( textures ) {}
	device::TextureId Import( int handle, bool srgb ) override
	{
		return m_Textures.Import( handle, srgb );
	}
	device::SamplerDesc Sampler( int handle ) override { return m_Textures.Sampler( handle ); }
	bool Pending( int handle ) override { return m_Textures.Pending( handle ); }
	std::optional<MipInfo> MipDescription( int handle ) override
	{
		const auto info = m_Textures.MipDescription( handle );
		if ( !info )
			return std::nullopt;
		return MipInfo{ info->width, info->height, info->levels };
	}

private:
	legacy::ICoreTextures &m_Textures;
};

} // namespace

void CoreWorld::RecordSlot(
    std::uint32_t tag, device::CommandEncoder &encoder, const legacy::CorePassTarget &target )
{
	if ( ( tag & 0xff000000u ) == 0x88000000u )
	{
		std::optional<TemporalRequest> request;
		{
			std::lock_guard<std::mutex> lock( m_TemporalLock );
			if ( m_TemporalStream != target.streamEpoch )
			{
				m_RecordedTemporalRequests.clear();
				m_TemporalStream = target.streamEpoch;
			}
			auto found = m_TemporalRequests.find( tag );
			if ( found != m_TemporalRequests.end() )
			{
				if ( m_RecordedTemporalRequests.size() < 8 )
				{
					request = found->second;
					m_RecordedTemporalRequests.emplace( tag, *request );
				}
				m_TemporalRequests.erase( found );
			}
			else if ( auto kept = m_RecordedTemporalRequests.find( tag );
			    kept != m_RecordedTemporalRequests.end() )
				request = kept->second;
		}
		bool success = false;
		if ( request && target.device )
		{
			request->motionTargets = m_MotionTargets.size();
			for ( auto &object : request->objects )
			{
				const auto viewport = m_TemporalRecordedViewports.find( object.view );
				if ( m_TemporalViewportFrame != target.frame ||
				     m_TemporalViewportStream != target.streamEpoch ||
				     viewport == m_TemporalRecordedViewports.end() )
					continue;
				const auto &v = viewport->second;
				object.viewport = { v.x, v.y, v.width, v.height, v.minDepth, v.maxDepth };
				object.recordedViewport = true;
			}
#ifdef RENDER_CORE_VULKAN
			if ( !m_Temporal )
			{
				auto provider = device::vulkan::CreateFsr411( *target.device, m_TemporalAssets );
				if ( provider )
					m_Temporal = std::make_unique<CoreTemporal>(
					    *target.device, std::move( provider ).Value() );
			}
#endif
			auto motion = m_MotionTargets.find( target.color.value );
			if ( m_Temporal && motion != m_MotionTargets.end() )
				success = m_Temporal->Record(
				    encoder, target, motion->second.image, motion->second.depth, *request );
		}
		if ( !success )
		{
			++m_LightingFailures;
			std::fprintf( stderr,
			    "FSR game: reconstruction failed (request=%d device=%d provider=%d color=%llu "
			    "motion=%d samples=%u); no silent spatial fallback\n",
			    int( request.has_value() ), int( target.device != nullptr ),
			    int( m_Temporal != nullptr ), static_cast<unsigned long long>( target.color.value ),
			    int( m_MotionTargets.count( target.color.value ) ), target.samples );
			(void)encoder.TakeBackend();
		}
		return;
	}
	if ( IsPostTag( tag ) )
	{
		m_Output.RecordPost( tag );
		return;
	}
	if ( tag == legacy::kCorePassLegacyHud )
		return;
	if ( tag == kCostBegin )
	{
		m_CostFrame = target.frame;
		return;
	}
	if ( target.device )
		std::erase_if( m_SceneCaptures,
		    [&]( std::pair<std::uint64_t, graph::InlineGraphResources> &old )
		    {
			    if ( target.frame == 0 || old.first >= target.frame )
				    return false;
			    old.second.Release( *target.device, target.submitted );
			    return true;
		    } );
	if ( tag & legacy::kCorePassFrameEnd )
	{
		// cl_render_debug_legacy 1: magenta over the frame, then the frame's
		// top-level world views again; their depth test brings back exactly
		// the pixels where the core's surface is still the one seen. What
		// stays tinted is what the core did not draw (or legacy drew over).
		pass::debug::HatchTarget whole;
		whole.encodeOutput = !target.colorSrgb.IsValid() &&
		                     target.colorFormat != render::device::Format::kRGBA16Float;
		whole.color = target.colorSrgb.IsValid() ? target.colorSrgb : target.color;
		whole.format = target.colorSrgb.IsValid() ? target.colorSrgbFormat : target.colorFormat;
		whole.width = target.width;
		whole.height = target.height;
		whole.samples = target.samples;
		const float magenta[4] = { 1.0f, 0.0f, 1.0f, 0.5f };
		if ( target.device && m_Overlays.RecordTint( *target.device, encoder, whole, magenta ) )
			m_Tints.fetch_add( 1, std::memory_order_relaxed );
		const auto frame = m_FrameViews.find( target.frame );
		if ( frame != m_FrameViews.end() )
		{
			const std::vector<std::pair<std::uint32_t, device::TextureId>> views = frame->second;
			for ( const auto &[view, color] : views )
			{
				if ( color != target.color )
					continue;
				RecordSlot( view, encoder, target );
				m_Redrawn.fetch_add( 1, std::memory_order_relaxed );
			}
		}
		// Frames older than a few are done.
		std::erase_if( m_FrameViews,
		    [&]( const auto &entry )
		    {
			    return entry.first + 4 < target.frame;
		    } );
		return;
	}
	if ( tag & legacy::kCorePassLegacyOff )
	{
		// The frame's first slot under a pixel view: every pixel the core does
		// not draw shows the not-applicable hatch (RFC 0014).
		pass::debug::HatchTarget hatch;
		hatch.encodeOutput = !target.colorSrgb.IsValid() &&
		                     target.colorFormat != render::device::Format::kRGBA16Float;
		hatch.color = target.colorSrgb.IsValid() ? target.colorSrgb : target.color;
		hatch.format = target.colorSrgb.IsValid() ? target.colorSrgbFormat : target.colorFormat;
		hatch.width = target.width;
		hatch.height = target.height;
		hatch.samples = target.samples;
		if ( target.device && m_Overlays.RecordHatch( *target.device, encoder, hatch ) )
			m_Hatches.fetch_add( 1, std::memory_order_relaxed );
		return;
	}
	RecordWorldBatch( std::span( &tag, 1 ), encoder, target );
}

std::size_t CoreWorld::RecordOpaqueBatch( std::span<const std::uint32_t> tags,
    device::CommandEncoder &encoder, const legacy::CorePassTarget &target )
{
	if ( tags.empty() )
		return 0;
	const auto &state = target.drawState;
	// Only an ordinary opaque depth-writing view can share early depth.
	// Keep portal/stencil operations and special depth ranges in stream order.
	const bool safe = m_DepthPrepass.load( std::memory_order_relaxed ) && !state.overrideDepth &&
	                  !state.stencil.enabled && state.colorWrite == device::kColorWriteAll &&
	                  target.minDepth == 0.0f && target.maxDepth == 1.0f;
	const std::size_t count = safe ? m_Pass.OpaqueBatchSize( tags, target.streamEpoch ) : 1;

	if ( count == 1 )
		RecordSlot( tags.front(), encoder, target );
	else
		RecordWorldBatch( tags.first( count ), encoder, target );
	return count;
}

// The world pass's target for a slot: its draw state, output, depth and the
// temporal history its view reads. False when the slot records nothing.
bool CoreWorld::PrepareWorldTarget( std::uint32_t tag, device::CommandEncoder &encoder,
    const legacy::CorePassTarget &target, pass::world::WorldTarget &world )
{
	world.drawState = target.drawState;
	std::memcpy( world.clipPlanes, target.clipPlanes, sizeof( world.clipPlanes ) );
	world.overrideDepthRange = true;
	world.minDepth = target.minDepth;
	world.maxDepth = target.maxDepth;
	world.device = target.device;
	if ( target.frame && m_MipFeedbackConsumer )
	{
		auto &feedback = m_MipFeedbackFrames[target.frame];
		if ( !feedback )
			feedback = std::make_unique<resources::MipFeedbackFrame>( target.frame );
		world.mipFeedback = feedback.get();
		std::erase_if( m_MipFeedbackFrames,
		    [&]( const auto &entry )
		    {
			    return entry.first + 4 < target.frame;
		    } );
	}
	// The sRGB view when the target has one; else the unorm view, and the
	// shader encodes (the same curve, the output encoding frame term).
	world.encodeOutput =
	    !target.colorSrgb.IsValid() && target.colorFormat != device::Format::kRGBA16Float;
	world.color = target.colorSrgb.IsValid() ? target.colorSrgb : target.color;
	world.colorFormat = target.colorSrgb.IsValid() ? target.colorSrgbFormat : target.colorFormat;
	world.colorCopySource = target.colorCopySource;
	world.sceneColorCapture = this;
	world.depth = target.depth;
	world.depthFormat = target.depthFormat;
	world.width = target.width;
	world.height = target.height;
	world.samples = target.samples;
	if ( m_Pass.TemporalView( tag, target.streamEpoch ) && target.device && target.samples == 1 )
	{
		if ( m_TemporalViewportFrame != target.frame ||
		     m_TemporalViewportStream != target.streamEpoch )
		{
			m_TemporalRecordedViewports.clear();
			m_TemporalViewportFrame = target.frame;
			m_TemporalViewportStream = target.streamEpoch;
		}
		world.temporalViewport = [this]( std::uint64_t view, device::Viewport viewport )
		{
			if ( m_TemporalRecordedViewports.contains( view ) ||
			     m_TemporalRecordedViewports.size() < 128 )
				m_TemporalRecordedViewports.insert_or_assign( view, viewport );
		};
		// Imported swapchain ids change on recreation. Keep a small working set,
		// not one image per target ever seen. A previous frame's image can retire
		// behind submitted; an image already recorded in this frame cannot.
		constexpr std::size_t kMaxMotionTargets = 8;
		if ( !m_MotionTargets.contains( target.color.value ) &&
		     m_MotionTargets.size() >= kMaxMotionTargets )
		{
			auto oldest = std::min_element( m_MotionTargets.begin(), m_MotionTargets.end(),
			    []( const auto &a, const auto &b )
			    {
				    return a.second.frame < b.second.frame;
			    } );
			if ( ( !target.frame || oldest->second.frame >= target.frame ) &&
			     ( !target.streamEpoch || oldest->second.stream == target.streamEpoch ) )
			{
				++m_LightingFailures;
				std::fprintf( stderr, "FSR game: all motion targets are in use by this frame\n" );
				(void)encoder.TakeBackend();
				return false;
			}
			(void)target.device->Release(
			    device::ResourceId( oldest->second.image ), target.submitted );
			(void)target.device->Release(
			    device::ResourceId( oldest->second.depth ), target.submitted );
			m_MotionTargets.erase( oldest );
		}
		auto &motion = m_MotionTargets[target.color.value];
		if ( motion.width != target.width || motion.height != target.height ||
		     !motion.depth.IsValid() )
		{
			if ( motion.image.IsValid() )
				(void)target.device->Release(
				    device::ResourceId( motion.image ), target.submitted );
			if ( motion.depth.IsValid() )
				(void)target.device->Release(
				    device::ResourceId( motion.depth ), target.submitted );
			motion = {};
			device::TextureDesc desc;
			desc.format = device::Format::kRG16Float;
			desc.width = target.width;
			desc.height = target.height;
			desc.usages = { device::ResourceUsage::kColorAttachment,
			    device::ResourceUsage::kSampled, device::ResourceUsage::kCopySource };
			desc.debugName = "game temporal motion";
			auto created = target.device->CreateTexture( desc );
			if ( !created )
			{
				++m_LightingFailures;
				return false;
			}
			motion.image = created.Value();
			desc.format = device::Format::kR32Float;
			desc.debugName = "game temporal surface depth";
			auto depth = target.device->CreateTexture( desc );
			if ( !depth )
			{
				++m_LightingFailures;
				return false;
			}
			motion.depth = depth.Value();
			motion.width = target.width;
			motion.height = target.height;
			encoder.TransitionTexture( motion.image, device::ResourceUsage::kUndefined,
			    device::ResourceUsage::kColorAttachment );
			encoder.TransitionTexture( motion.depth, device::ResourceUsage::kUndefined,
			    device::ResourceUsage::kColorAttachment );
		}
		if ( motion.frame != target.frame || motion.stream != target.streamEpoch )
		{
			const device::ColorAttachment attachments[] = {
			    { motion.image, device::LoadOp::kClear, device::StoreOp::kStore,
			        { 65504.0f, 65504.0f, 0.0f, 0.0f }, {} },
			    { motion.depth, device::LoadOp::kClear, device::StoreOp::kStore,
			        { 1.0f, 0.0f, 0.0f, 0.0f }, {} } };
			device::RenderingDesc clear;
			clear.colors = attachments;
			clear.width = target.width;
			clear.height = target.height;
			encoder.BeginRendering( clear );
			encoder.EndRendering();
			motion.frame = target.frame;
			motion.stream = target.streamEpoch;
		}
		world.motion = motion.image;
		world.motionDepth = motion.depth;
	}
	return true;
}

void CoreWorld::RecordWorldBatch( std::span<const std::uint32_t> tags,
    device::CommandEncoder &encoder, const legacy::CorePassTarget &target )
{
	const std::uint32_t tag = tags.front();
	std::optional<Textures> textures;
	if ( target.textures )
		textures.emplace( *target.textures );
	pass::world::WorldTarget world;
	if ( !PrepareWorldTarget( tag, encoder, target, world ) )
		return;

	world.textures = textures ? &*textures : nullptr;
	world.submitted = target.submitted;
	world.frame = target.frame;
	world.streamEpoch = target.streamEpoch;
	world.lightmapScale = target.lightmapScale;
	world.depthPrepass = m_DepthPrepass.load( std::memory_order_relaxed );
	const int gpuSubmission = m_GpuSubmission.load( std::memory_order_relaxed );
	world.gpuSubmission = gpuSubmission != 0;
	world.gpuOcclusion = gpuSubmission >= 2;
	world.runtimeDirect = m_StageRuntimeDirect.load( std::memory_order_relaxed );
	world.ambientOcclusionTerm = m_AoQuality.load( std::memory_order_relaxed ) > 0;
	world.softShadows = m_ShadowPcss.load( std::memory_order_relaxed );
	world.probeBounce = m_ProbeBounce.load( std::memory_order_relaxed );
	world.outputScale = target.outputScale;
	std::copy( target.eye, target.eye + 3, world.eye );
	world.envmapScale = target.envmapScale;
	world.specular = target.specular;
	world.ssbumpNormalized = target.ssbumpNormalized;
	world.fogType = target.fog.type;
	std::copy( target.fog.color, target.fog.color + 3, world.fogColor );
	std::copy( target.fog.params, target.fog.params + 4, world.fogParams );
	world.fogEyeZ = target.fog.eyeZ;
	world.time = target.time;
	std::memcpy( world.foliage, target.foliage, sizeof( world.foliage ) );
	world.foliageAvailable = target.foliageAvailable;
	world.waterReflectTintScale = target.waterReflectTintScale;
	const std::shared_ptr<const pass::world::StageLightingInputs> inputs =
	    m_Pass.LightingInputs( tag, target.streamEpoch );
	const std::shared_ptr<const PendingView> pending =
	    std::dynamic_pointer_cast<const PendingView>( inputs );
	if ( inputs && !pending )
	{
		++m_LightingFailures;
		return;
	}
	std::optional<StreamView> streamView;
	{
		std::lock_guard<std::mutex> guard( m_ShadowLock );
		if ( target.streamEpoch != 0 )
			std::erase_if( m_StreamViews,
			    [&]( const auto &entry )
			    {
				    return entry.second.recordedStream != 0 &&
				           entry.second.recordedStream != target.streamEpoch;
			    } );
		const auto stream = m_StreamViews.find( tag );
		if ( stream != m_StreamViews.end() )
		{
			stream->second.recordedStream = target.streamEpoch;
			streamView = stream->second;
		}
	}
	if ( streamView && m_StreamLightingFrame == target.frame &&
	     world.color == m_StreamLighting.color && world.depth == m_StreamLighting.depth &&
	     world.width == m_StreamLighting.width && world.height == m_StreamLighting.height &&
	     streamView->view == m_StreamLightingView.view &&
	     streamView->projection == m_StreamLightingView.projection )
	{
		world.lights = m_StreamLighting.lights;
		world.shadowAtlas = m_StreamLighting.shadowAtlas;
		world.shadowAtlasDesc = m_StreamLighting.shadowAtlasDesc;
		world.ambientOcclusion = m_StreamLighting.ambientOcclusion;
		world.ambientOcclusionDesc = m_StreamLighting.ambientOcclusionDesc;
	}

	// A view that draws no world geometry of its own, in a frame whose stage
	// lighting another view already recorded: it reads that lighting, its
	// shadow atlas and its occlusion instead of planning a second set. The
	// viewmodel scope is this case: the frame's second view, drawing only the
	// handed-off model.
	const bool sharedStage =
	    pending && !m_Pass.ViewDrawsWorldGeometry( tag, target.streamEpoch ) &&
	    m_StreamLightingFrame == target.frame && m_StreamLighting.lights &&
	    m_StreamLighting.ambientOcclusion.IsValid() && world.color == m_StreamLighting.color &&
	    world.depth == m_StreamLighting.depth && world.width == m_StreamLighting.width &&
	    world.height == m_StreamLighting.height;
	if ( sharedStage )
	{
		m_SharedStageViews.fetch_add( 1, std::memory_order_relaxed );
		world.lights = m_StreamLighting.lights;
		world.shadowAtlas = m_StreamLighting.shadowAtlas;
		world.shadowAtlasDesc = m_StreamLighting.shadowAtlasDesc;
		world.ambientOcclusion = m_StreamLighting.ambientOcclusion;
		world.ambientOcclusionDesc = m_StreamLighting.ambientOcclusionDesc;
		// No prepass and no ambient occlusion pass of its own: there is no
		// world geometry in this view to build them from, and the occlusion
		// it reads is the frame's.
	}

	if ( pending && !target.device )
	{
		++m_LightingFailures;
		return;
	}
	if ( pending && target.device )
	{
		BindStageDevice( *target.device );
		if ( !m_ClusterKernel )
		{
			auto kernel = pass::lights::ClusterKernel::Create( *target.device );
			if ( !kernel )
			{
				++m_LightingFailures;
				return;
			}
			m_ClusterKernel = std::move( kernel ).Value();
		}
		if ( target.frame != m_ClusterFrame )
		{
			m_ClusterKernel->Collect( target.submitted );
			m_ClusterFrame = target.frame;
		}
	}
	// RFC 0014 D4: the view's sections are timed while the timers are on.
	graph::GpuPassTimers *timers = target.device ? SlotTimers( target ) : nullptr;
	m_SlotTimers = timers;
	const auto recordStarted = std::chrono::steady_clock::now();
	if ( timers )
	{
		timers->Attach( encoder );
		encoder.BeginLabel( "core world view" );
	}
	std::shared_ptr<const ShadowWork> shadows;
	if ( pending && !sharedStage )
	{
		if ( timers )
			encoder.BeginLabel( "prepare lights and shadows" );
		// The frame's cookie set, uploaded once per set; the old array goes
		// behind the frames that read it.
		if ( pending->inputs.cookies && pending->inputs.cookies != m_CookiesUploaded )
		{
			if ( m_Cookies )
				m_Cookies->Release( target.submitted );
			m_Cookies = std::make_unique<CookieArray>();
			m_CookiesUploaded = pending->inputs.cookies;
			if ( std::optional<std::string> why =
			         m_Cookies->Create( *target.device, *pending->inputs.cookies ) )
			{
				std::fprintf(
				    stderr, "render core: projector cookies refused: %s\n", why->c_str() );
				m_Cookies->Release( target.submitted );
				m_Cookies.reset();
			}
			else
				m_Cookies->RecordUpload( encoder );
		}
		const std::shared_ptr<PendingView::FrameLighting> lighting =
		    pending->ForFrame( target.frame );
		std::call_once( lighting->made,
		    [&]
		    {
			    lighting->lights =
			        StageViewLightsFor( pending->inputs, &lighting->shadows, encoder );
			    if ( lighting->shadows && !lighting->shadows->views.empty() )
				    lighting->shadowAtlas = DrawStageShadows( *target.device, *lighting->shadows,
				        target.submitted, target.frame, &lighting->shadowAtlasDesc );
			    m_StageLightingBuilds.fetch_add( 1, std::memory_order_relaxed );
		    } );
		if ( timers )
			encoder.EndLabel();
		if ( !lighting->lights )
		{
			if ( timers )
			{
				encoder.EndLabel();
				timers->Detach( encoder );
			}
			++m_LightingFailures;
			return;
		}
		world.lights = lighting->lights;
		shadows = lighting->shadows;
		world.shadowAtlas = lighting->shadowAtlas;
		world.shadowAtlasDesc = lighting->shadowAtlasDesc;
		// The stage's cutout casters, into this frame's atlas once.
		if ( shadows && lighting->shadowAtlas.IsValid() &&
		     ( !m_CutoutSurfaces.empty() || !m_CutoutStaticSurfaces.empty() ) &&
		     !shadows->views.empty() &&
		     !lighting->cutoutsDrawn.exchange( true, std::memory_order_relaxed ) )
		{
			auto cutouts = std::make_shared<pass::world::WorldCutoutShadows>();
			cutouts->atlas = lighting->shadowAtlas;
			cutouts->atlasSize = shadows->atlasSize;
			cutouts->surfaces = m_CutoutSurfaces;
			cutouts->staticSurfaces = m_CutoutStaticSurfaces;
			for ( const pass::shadows::ShadowPlanView &planned : shadows->views )
			{
				pass::world::WorldShadowView view;
				for ( int r = 0; r < 4; ++r )
					std::memcpy( view.viewProjection + r * 4, &planned.viewProjection.rows[r].x,
					    4 * sizeof( float ) );
				const pass::shadows::ShadowViewport inner =
				    pass::shadows::TileViewport( planned.tile, shadows->guardTexels );
				view.x = inner.x;
				view.y = inner.y;
				view.size = inner.size;
				cutouts->views.push_back( view );
			}
			world.cutoutShadows = std::move( cutouts );
		}
	}
	if ( timers )
		encoder.BeginLabel( "prepare screen passes" );
	// The stage view's screen passes: GTAO over the pass's prepass.
	const int aoQuality =
	    pending && pending->inputs.stageWorld ? m_AoQuality.load( std::memory_order_relaxed ) : 0;
	if ( shadows && pending->inputs.stageWorld && target.device && aoQuality == 0 &&
	     EnsureOcclusion( *target.device, encoder, target.width, target.height, target.submitted ) )
	{
		// Off: the pass binds the target, held at one (the neutral term).
		if ( !m_OcclusionNeutral )
		{
			encoder.TransitionTexture( m_Occlusion, device::ResourceUsage::kSampled,
			    device::ResourceUsage::kCopyDestination );
			encoder.ClearTexture( m_Occlusion, { 1, 1, 1, 1 } );
			encoder.TransitionTexture( m_Occlusion, device::ResourceUsage::kCopyDestination,
			    device::ResourceUsage::kSampled );
			m_OcclusionNeutral = true;
		}
		world.ambientOcclusion = m_Occlusion;
		world.ambientOcclusionDesc = m_OcclusionDesc;
	}
	if ( shadows && target.device && aoQuality > 0 &&
	     EnsureOcclusion( *target.device, encoder, target.width, target.height, target.submitted ) )
	{
		m_OcclusionNeutral = false;
		// Slices x steps per side and resolution, set by render.lab.gtao
		// against Cycles: ultra (8 x 8, full) and high (5 x 8 at half
		// resolution, a sixth of ultra's cost) pass every check; fewer than 8
		// steps miss a crease's near occluders and fewer than 5 slices leave
		// an open plane below 0.99 (medium 3 x 8 and low 2 x 6, both half).
		static constexpr std::uint32_t kSlices[] = { 2, 3, 5, 8 };
		static constexpr std::uint32_t kSteps[] = { 6, 8, 8, 8 };
		static constexpr bool kHalf[] = { true, true, true, false };
		const int preset = std::min( aoQuality, 4 ) - 1;
		pass::ao::AoParams params = m_Ao->Params();
		params.slices = kSlices[preset];
		params.steps = kSteps[preset];
		params.halfResolution = kHalf[preset];
		(void)m_Ao->SetParams( params );
		world.ambientOcclusion = m_Occlusion;
		world.ambientOcclusionDesc = m_OcclusionDesc;
		const std::shared_ptr<const ShadowWork> work = shadows;
		world.screenPasses = [this, work]( device::CommandEncoder &screen,
		                         const pass::world::WorldTarget::Prepass &prepass ) -> bool
		{
			pass::ao::AoTargets targets;
			targets.depth = prepass.depth;
			targets.normalRoughness = prepass.normalRoughness;
			targets.output = m_Occlusion;
			targets.outputUsage = device::ResourceUsage::kSampled;
			targets.width = prepass.width;
			targets.height = prepass.height;
			pass::ao::AoView aoView;
			aoView.view = work->view;
			aoView.projection = work->projection;
			std::copy( work->eye, work->eye + 3, aoView.eye );
			screen.BeginLabel( "core world gtao" );
			const bool recorded = bool( m_Ao->Record( screen, targets, aoView ) );
			screen.EndLabel();
			return recorded;
		};
	}
	if ( pending && !sharedStage )
	{
		std::copy_n( pending->inputs.worldToView, 16, m_StreamLightingView.view.begin() );
		std::copy_n( pending->inputs.viewToClip, 16, m_StreamLightingView.projection.begin() );
		m_StreamLightingFrame = target.frame;
		m_StreamLighting = world;
		m_StreamLighting.device = nullptr;
		m_StreamLighting.textures = nullptr;
		m_StreamLighting.sceneColorCapture = nullptr;
		m_StreamLighting.screenPasses = {};
	}

	if ( timers )
		encoder.EndLabel();
	// render.ssr.v1 (RFC 0016 K12): the stage view's lit pass writes the
	// reflections' inputs, then the trace replaces its image-based specular
	// where it hits, before the medium and the translucent stream.
	bool ssrView = false;
	if ( pending && !sharedStage && shadows && pending->inputs.stageWorld && target.device &&
	     m_SsrOn.load( std::memory_order_relaxed ) )
	{
		const char *refusal = target.samples != 1      ? "a multisampled target"
		                      : world.motion.IsValid() ? "a temporal view"
		                      : target.colorFormat != device::Format::kRGBA16Float
		                          ? "an 8-bit target"
		                      : !target.depth.IsValid() || !target.color.IsValid()
		                          ? "a target without imported color or depth"
		                          : nullptr;
		if ( refusal )
		{
			if ( m_SsrRefused.fetch_add( 1, std::memory_order_relaxed ) == 0 )
				std::fprintf(
				    stderr, "render core: screen-space reflections refused: %s\n", refusal );
		}
		else if ( EnsureSsr( *target.device, target.width, target.height, target.submitted ) )
		{
			if ( m_SsrFresh )
			{
				for ( const device::TextureId t : m_SsrTargets )
					encoder.TransitionTexture(
					    t, device::ResourceUsage::kUndefined, device::ResourceUsage::kSampled );
				encoder.TransitionTexture( m_SsrOutput, device::ResourceUsage::kUndefined,
				    device::ResourceUsage::kStorageWrite );
				m_SsrFresh = false;
			}
			if ( target.frame != m_SsrFrame )
			{
				m_Ssr->Collect( target.submitted );
				m_SsrCopy->Collect( target.submitted );
				m_SsrFrame = target.frame;
			}
			world.ssrNormalRoughness = m_SsrTargets[0];
			world.ssrIblRadiance = m_SsrTargets[1];
			world.ssrSpecularWeight = m_SsrTargets[2];
			ssrView = true;
		}
		else if ( m_SsrRefused.fetch_add( 1, std::memory_order_relaxed ) == 0 )
			std::fprintf( stderr, "render core: screen-space reflections refused: the device "
			                      "refused their targets or programs\n" );
	}
	m_Pass.RecordBatch( tags, encoder, world );
	if ( ssrView )
	{
		if ( timers )
			encoder.BeginLabel( "core world ssr" );
		RecordSsr( encoder, target, *shadows );
		if ( timers )
			encoder.EndLabel();
	}
	// The participating media over the view's opaque frame, once per view
	// and record frame (RFC 0016 K12; the lab's step g).
	if ( pending && !sharedStage && shadows && pending->inputs.stageWorld && m_Media &&
	     m_VolumetricOn.load( std::memory_order_relaxed ) && target.device &&
	     !pending->ForFrame( target.frame )->fogged.exchange( true, std::memory_order_relaxed ) )
	{
		if ( timers )
			encoder.BeginLabel( "core world volumetric" );
		RecordVolumetric( encoder, target, pending->inputs );
		if ( timers )
			encoder.EndLabel();
	}
	if ( timers )
	{
		encoder.EndLabel();
		timers->Detach( encoder );
		m_RecordNs.fetch_add( std::uint64_t( std::chrono::duration_cast<std::chrono::nanoseconds>(
		                          std::chrono::steady_clock::now() - recordStarted )
		                              .count() ),
		    std::memory_order_relaxed );
		m_RecordViews.fetch_add( 1, std::memory_order_relaxed );
	}
	m_SlotTimers = nullptr;
	for ( const auto recordedTag : tags )
	{
		bool topLevel = false;
		{
			std::lock_guard<std::mutex> guard( m_TopLevelLock );
			topLevel = m_TopLevel.count( recordedTag ) != 0;
		}
		if ( topLevel )
		{
			auto &views = m_FrameViews[target.frame];
			const auto entry = std::make_pair( recordedTag, target.color );
			if ( std::find( views.begin(), views.end(), entry ) == views.end() &&
			     views.size() < 64 )
				views.push_back( entry );
		}
	}
}

} // namespace render::composition
