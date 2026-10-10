//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.world (RFC 0016 K5): recording a batch of queued views.
//
//=============================================================================//

#include "world_batch.h"

namespace render::pass::world
{

void WorldPass::RecordBatch(
    std::span<const std::uint32_t> tags, CommandEncoder &encoder, const WorldTarget &target )
{
	if ( tags.empty() )
		return;
	RecordSection preparation( encoder );
	preparation.Select( "prepare world queue" );
	State &s = *m_State;
	std::vector<std::size_t> staticCohorts, posedCohorts;
	std::shared_ptr<const WorldData> world;
	std::shared_ptr<const std::vector<Claimed>> claims;
	std::shared_ptr<const std::vector<SurfaceFootprintGeometry>> footprintGeometry;
	std::shared_ptr<const std::vector<std::vector<SurfaceFootprintBounds>>> modelFootprintBounds;
	std::uint64_t generation = 0;
	WorldView view;
	// The slot's dynamic draws, shared with the queue (dynamic views never batch).
	std::shared_ptr<const State::QueuedDynamic> queuedDynamic;
	bool found = false;
	bool earlierWorld = false; // the slot's view was queued against an earlier world
	{
		std::lock_guard<std::mutex> guard( s.lock );
		if ( tags.size() > 1 && s.OpaqueBatchSize( tags, target.streamEpoch ) != tags.size() )
		{
			++s.stats.viewsFailed;
			s.stats.lastFailure = "an opaque batch crosses a view or material ordering boundary";
			return;
		}
		for ( std::size_t cohortIndex = 0; cohortIndex < tags.size(); ++cohortIndex )
		{
			const std::uint32_t serial = tags[cohortIndex] & kWorldSerialMask;
			WorldView cohort;
			std::shared_ptr<const State::QueuedDynamic> cohortDynamic;
			found = false;
			// A slot recorded again (the same stream for a capture) draws the
			// cohort it drew the first time; one of an earlier world draws nothing
			// and leaves the queue (the next frame's views) alone.
			bool again = false;
			if ( target.streamEpoch != 0 )
				std::erase_if( s.recorded,
				    [&]( const State::Queued &kept )
				    {
					    return kept.recordedStream != target.streamEpoch;
				    } );
			for ( auto kept = s.recorded.rbegin(); kept != s.recorded.rend() && !again; ++kept )
			{
				if ( kept->serial == serial )
				{
					again = true;
					found = kept->generation == s.generation;
					earlierWorld = !found;
					cohort = kept->view;
					cohortDynamic = kept->dynamic;
				}
			}
			// World views are issued in main-thread stream order. Dynamic tickets
			// are issued during render-call replay, possibly after the main thread
			// has queued another frame: their serials do not establish stream order.
			std::uint64_t recordingFrame = 0;
			bool dynamic = false;
			for ( const State::Queued &queued : s.views )
			{
				if ( queued.serial == serial )
				{
					recordingFrame = queued.view.hostFrame;
					dynamic = queued.dynamic != nullptr;
				}
			}
			while ( !again && !dynamic && !s.views.empty() && !s.views.front().dynamic &&
			        IssuedBefore( s.views.front().serial, serial ) )
			{
				s.Drop( s.views.front(), recordingFrame );
				s.views.pop_front();
			}
			auto queued = std::find_if( s.views.begin(), s.views.end(),
			    [&]( const State::Queued &entry )
			    {
				    return entry.serial == serial;
			    } );
			if ( !again && queued != s.views.end() )
			{
				// Only against the world it was queued for (SetWorld also clears
				// the queue; this holds if a slot records across the change).
				found = queued->generation == s.generation;
				earlierWorld = !found;
				cohort = queued->view;
				cohortDynamic = queued->dynamic;
				if ( cohort.hostFrame != 0 &&
				     ( s.recordedFrames.empty() || s.recordedFrames.back() != cohort.hostFrame ) )
				{
					s.recordedFrames.push_back( cohort.hostFrame );
					constexpr std::size_t kFramesKept = 64;
					while ( s.recordedFrames.size() > kFramesKept )
						s.recordedFrames.pop_front();
				}
				queued->recordedStream = target.streamEpoch;
				s.recorded.push_back( std::move( *queued ) );
				s.views.erase( queued );
				// The complete stream is replayable, including every dynamic slot.
				// The queue accepts at most this many slots before recording starts.
				constexpr std::size_t kRecordedKept = 8192;
				while ( s.recorded.size() > kRecordedKept )
					s.recorded.pop_front();
			}
			if ( !found )
				break;
			staticCohorts.insert( staticCohorts.end(), cohort.staticInstances.size(), cohortIndex );
			posedCohorts.insert( posedCohorts.end(), cohort.posedModels.size(), cohortIndex );
			if ( cohortIndex == 0 )
			{
				view = std::move( cohort );
				queuedDynamic = std::move( cohortDynamic );
			}
			else
			{
				view.staticInstances.insert( view.staticInstances.end(),
				    std::make_move_iterator( cohort.staticInstances.begin() ),
				    std::make_move_iterator( cohort.staticInstances.end() ) );
				view.posedModels.insert( view.posedModels.end(),
				    std::make_move_iterator( cohort.posedModels.begin() ),
				    std::make_move_iterator( cohort.posedModels.end() ) );
			}
		}
		world = s.world;
		claims = s.claims;
		footprintGeometry = s.footprintGeometry;
		modelFootprintBounds = s.modelFootprintBounds;
		generation = s.generation;
	}
	if ( earlierWorld )
	{
		// A frame recorded across a level change: its views were the earlier
		// world's, which is gone. Nothing to draw, and nothing failed.
		std::lock_guard<std::mutex> guard( s.lock );
		++s.stats.viewsSkipped;
		return;
	}
	if ( target.overrideDepthRange )
	{
		view.viewport.minDepth = target.minDepth;
		view.viewport.maxDepth = target.maxDepth;
	}
	// A stage view's lights made when its slot records (WorldTarget::lights).
	if ( target.lights )
		view.lights = target.lights;
	if ( !found || !world || !claims )
	{
		s.Fail( "a slot names no queued view of the current world" );
		return;
	}
	if ( view.stageLighting && !view.lights )
	{
		s.Fail( "a claimed stage view lost its queued lighting inputs" );
		return;
	}
	if ( !target.device || !target.color.IsValid() || !target.depth.IsValid() || !target.textures ||
	     ( target.motion.IsValid() && !target.motionDepth.IsValid() ) )
	{
		std::string missing;
		for ( const auto &[absent, what] :
		    { std::pair{ !target.device, "device" }, { !target.color.IsValid(), "color" },
		        { !target.depth.IsValid(), "depth" }, { !target.textures, "texture source" },
		        { target.motion.IsValid() && !target.motionDepth.IsValid(), "temporal depth" } } )
		{
			if ( absent )
				missing += missing.empty() ? what : std::string( ", " ) + what;
		}
		s.Fail( "the slot's target has no " + missing + " (a render-target texture)" );
		return;
	}
	if ( target.temporalViewport && view.temporalView )
		target.temporalViewport( view.temporalView, view.viewport );
	preparation.Select( "prepare world resources" );
	if ( s.device != target.device )
	{
		// A new backend device: the old one's objects went with it.
		s.variants.clear();
		s.retired.clear();
		s.retiredBuffers.clear();
		s.retiredBindGroups.clear();
		s.gpuCull.reset();
		s.gpuCompact.reset();
		s.gpuOcclusion.reset();
		s.gpuPointSampler = SamplerId();
		s.gpuPyramid = State::GpuPyramid();
		s.retiredGroups.clear();
		s.retiredTextures.clear();
		// The old device's model levels went with it; the world they belong to
		// publishes its staging again (SweepModelResidency adopts it).
		s.retiredModelBuffers.clear();
		s.models = State::ModelGeometry();
		++s.modelResidencyRevision;
		s.groupResources = GroupResources();
		s.device = target.device;
	}
	IRenderDevice2 &device = *s.device;
	IWorldTextures &textures = *target.textures;
	// The world's device objects for these formats. Objects of an earlier
	// world, or a set pushed out, retire with this frame: slots earlier in
	// it may have used them, so they are released at a later frame's slot,
	// whose submitted token covers this frame.
	constexpr std::size_t kMaxVariants = 4;
	if ( s.variantsGeneration != generation || s.variantsRuntimeDirect != target.runtimeDirect ||
	     s.variantsAmbientOcclusion != target.ambientOcclusionTerm )
	{
		for ( Resources &variant : s.variants )
			s.retired.emplace_back( target.frame, std::move( variant ) );
		s.variants.clear();
		s.variantsGeneration = generation;
		s.variantsRuntimeDirect = target.runtimeDirect;
		s.variantsAmbientOcclusion = target.ambientOcclusionTerm;
	}
	auto variant = std::find_if( s.variants.begin(), s.variants.end(),
	    [&]( const Resources &v )
	    {
		    return v.colorFormat == target.colorFormat && v.depthFormat == target.depthFormat &&
		           v.samples == target.samples;
	    } );
	if ( variant == s.variants.end() )
	{
		if ( s.variants.size() >= kMaxVariants )
		{
			s.retired.emplace_back( target.frame, std::move( s.variants.front() ) );
			s.variants.erase( s.variants.begin() );
		}
		Resources made;
		made.colorFormat = target.colorFormat;
		made.depthFormat = target.depthFormat;
		made.samples = target.samples;
		s.variants.push_back( std::move( made ) );
	}
	else if ( variant + 1 != s.variants.end() )
	{
		std::rotate( variant, variant + 1, s.variants.end() );
	}
	Resources &r = s.variants.back();
	if ( r.litFrame != target.frame || r.litStream != target.streamEpoch )
	{
		for ( auto &cached : r.litViews )
			s.retiredGroups.emplace_back( r.litFrame, std::move( cached.group ) );
		r.litViews.clear();
		r.litFrame = target.frame;
		r.litStream = target.streamEpoch;
	}
	if ( r.dynamicFrame != target.frame )
	{
		constexpr std::uint64_t kDynamicMaterialFrames = 8;
		constexpr std::size_t kMaxDynamicMaterials = 1024;
		const bool overfull = r.dynamicMaterials.size() > kMaxDynamicMaterials;
		std::erase_if( r.dynamicMaterials,
		    [&]( auto &entry )
		    {
			    Resources::Material &m = entry.second;
			    // A failure retries next frame (a texture may finish uploading).
			    if ( !overfull && !m.failed &&
			         m.lastUsed + kDynamicMaterialFrames >= target.frame &&
			         m.lastUsed <= target.frame )
				    return false;
			    s.retiredGroups.emplace_back( r.dynamicFrame, std::move( m.group ) );
			    return true;
		    } );
		r.dynamicFrame = target.frame;
	}
	std::erase_if( s.retired,
	    [&]( std::pair<std::uint64_t, Resources> &old )
	    {
		    if ( target.frame == 0 || old.first >= target.frame )
			    return false;
		    s.Release( old.second, target.submitted );
		    return true;
	    } );
	std::erase_if( s.retiredBuffers,
	    [&]( const std::pair<std::uint64_t, BufferId> &old )
	    {
		    if ( target.frame == 0 || old.first >= target.frame )
			    return false;
		    (void)device.Release( old.second, target.submitted );
		    return true;
	    } );
	std::erase_if( s.retiredBindGroups,
	    [&]( const std::pair<std::uint64_t, BindGroupId> &old )
	    {
		    if ( target.frame == 0 || old.first >= target.frame )
			    return false;
		    (void)device.Release( old.second, target.submitted );
		    return true;
	    } );
	// A released model level's buffers: its last reader may be a submission
	// this token already covers.
	std::erase_if( s.retiredModelBuffers,
	    [&]( const State::RetiredModelBuffer &old )
	    {
		    if ( target.frame == 0 || old.frame >= target.frame )
			    return false;
		    (void)device.Release( old.buffer, target.submitted );
		    return true;
	    } );
	std::erase_if( s.retiredTextures,
	    [&]( const std::pair<std::uint64_t, TextureId> &old )
	    {
		    if ( target.frame == 0 || old.first >= target.frame )
			    return false;
		    (void)device.Release( old.second, target.submitted );
		    return true;
	    } );
	std::erase_if( s.retiredGroups,
	    [&]( std::pair<std::uint64_t, Group> &old )
	    {
		    if ( target.frame == 0 || old.first == 0 || old.first >= target.frame )
			    return false;
		    s.ReleaseGroup( old.second, target.submitted, true );
		    return true;
	    } );
	Batch batch{ .pass = *this,
	    .tags = tags,
	    .encoder = encoder,
	    .target = target,
	    .preparation = preparation,
	    .s = s,
	    .staticCohorts = std::move( staticCohorts ),
	    .posedCohorts = std::move( posedCohorts ),
	    .world = std::move( world ),
	    .claims = std::move( claims ),
	    .footprintGeometry = std::move( footprintGeometry ),
	    .modelFootprintBounds = std::move( modelFootprintBounds ),
	    .generation = generation,
	    .view = std::move( view ),
	    .queuedDynamic = std::move( queuedDynamic ),
	    .device = device,
	    .textures = textures,
	    .r = r };
	batch.Run();
}

void WorldPass::Batch::Run()
{
	if ( !PrepareResources() )
		return;
	if ( !PrepareFrameTerms() )
		return;
	if ( !PrepareViewGroups() )
		return;
	if ( !PrepareSurfacesAndModels() )
		return;
	if ( !PrepareDrawHelpers() )
		return;
	if ( !RecordCutoutShadows() )
		return;
	if ( !PrepareGpuSubmission() )
		return;
	if ( !RecordScreenPasses() )
		return;
	if ( !PrepareDynamicDraws() )
		return;
	RecordView();
}

material::SurfaceDrawState WorldPass::Batch::prepassedState()
{
	material::SurfaceDrawState state = target.drawState;
	state.overrideDepth = true;
	state.depthTest = true;
	state.depthWrite = false;
	state.depthCompare = CompareOp::kEqual;
	return state;
}

std::uint64_t WorldPass::Batch::drawGpuModels(
    const std::function<std::optional<PipelineId>( const Resources::Material & )> &pipelineOf,
    const std::function<bool( const Resources::Material & )> &skip )
{
	return drawModelBuckets( gpuViewModels, r.modelMaterials, pipelineOf, skip );
}

bool WorldPass::Batch::captureSceneColor()
{
	if ( target.samples == 1 && !target.colorCopySource )
	{
		note( "the slot's color image does not support scene-color capture" );
		return false;
	}
	if ( !target.sceneColorCapture )
	{
		note( "the composition has no scene-color capture provider" );
		return false;
	}
	TextureDesc sourceDesc;
	sourceDesc.format = target.colorFormat;
	sourceDesc.width = target.width;
	sourceDesc.height = target.height;
	sourceDesc.sampleCount = target.samples;
	sourceDesc.usages = { ResourceUsage::kColorAttachment };
	if ( target.colorCopySource )
		sourceDesc.usages.Add( ResourceUsage::kCopySource );
	const std::optional<WorldSceneColor> captured = target.sceneColorCapture->Capture(
	    device, encoder, target.color, sourceDesc, target.frame );
	if ( !captured || !captured->texture.IsValid() )
	{
		note( "the scene-color capture resources were refused" );
		return false;
	}
	viewSceneColor = captured->texture;
	viewSceneColorDesc = captured->desc;
	return true;
}

bool WorldPass::Batch::RecordView()
{
	std::vector<ColorAttachment> colors = {
	    { target.color, LoadOp::kLoad, StoreOp::kStore, {}, {} } };
	RenderingDesc rendering;
	rendering.colors = colors;
	rendering.depth = DepthAttachment{ target.depth, LoadOp::kLoad, StoreOp::kStore, 1.0f };
	rendering.width = target.width;
	rendering.height = target.height;
	const auto &stencil = target.drawState.stencil;
	const bool depthPrepassSafe =
	    target.depthPrepass && world->stage && !target.drawState.overrideDepth &&
	    ( !stencil.enabled || stencil.writeMask == 0 ||
	        ( stencil.fail == StencilOp::kKeep && stencil.depthFail == StencilOp::kKeep &&
	            stencil.pass == StencilOp::kKeep ) );
	// The view's list, GPU-culled once for the depth prepass and the lit
	// pass (which draw the same buckets).
	const bool gpuCapable = gpuDeviceCapable;
	// This view's pyramid: built from the same frame's screen prepass with
	// the same camera, clip planes and viewport inputs.
	const bool gpuOccluding = occlusionWanted && s.gpuPyramid.pyramid.IsValid() &&
	                          s.gpuPyramid.frame == target.frame &&
	                          s.gpuPyramid.depth == r.prepassDepth &&
	                          s.gpuPyramid.inputs == screenInputs && s.gpuOcclusion;
	if ( target.gpuSubmission && !gpuCapable )
		++s.stats.gpuFallbacks;
	const GpuList gpuView = target.gpuSubmission && gpuCapable
	                            ? gpuCullList( order, r.materials, gpuOccluding )
	                            : GpuList();
	if ( gpuView.commands.IsValid() )
		s.stats.gpuOcclusionViews += gpuOccluding;
	// The view's opaque static models, GPU-driven (gpuCullModels); the
	// per-draw loops skip the draws it takes.
	gpuViewModels = gpuCullModels( staticDraws, r.modelMaterials, gpuOccluding, true );
	for ( std::size_t i = 0; i < staticDraws.size(); ++i )
		staticDraws[i].gpu =
		    gpuViewModels.taken.size() == staticDraws.size() && gpuViewModels.taken[i];
	if ( gpuViewModels.commands.IsValid() )
		++s.stats.gpuModelViews;
	bool worldDepthReady = depthPrepassSafe;
	// The opaque (and alpha-tested) surfaces' depth first, into the target's
	// own depth with its color masked. Eligible PBR lighting reads that depth
	// with an equal test and no writes, allowing early rejection despite
	// the material shader's clipping/discard. A
	// translucent surface's depth would hide what the stream draws behind
	// it, so it is not drawn here.
	if ( target.depthPrepass && world->stage )
	{
		std::vector<std::uint32_t> opaque;
		opaque.reserve( order.size() );
		for ( const std::uint32_t index : order )
		{
			if ( r.materials[world->surfaces[index].material].program.blend == BlendMode::kOpaque )
				opaque.push_back( index );
		}
		if ( !opaque.empty() )
		{
			encoder.BeginLabel( "core world depth" );
			encoder.BeginRendering( rendering );
			encoder.SetViewport( view.viewport );
			auto depthPipeline = [&]( const Resources::Material &m ) -> std::optional<PipelineId>
			{
				auto variant = m.resolver->VariantPipeline( m.program,
				    material::kSurfaceDepthNormal | material::kSurfaceDepthOnly,
				    material::kSurfaceSsrTargets );
				if ( !variant )
				{
					worldDepthReady = false;
					note( "the depth prepass: " + variant.Error() );
					return std::nullopt;
				}
				const auto state = surfaceStatePipeline( m, variant.Value(), target.drawState );
				if ( !state )
					worldDepthReady = false;
				return state;
			};
			// GPU-driven: the view's buckets of opaque materials, from the
			// lit pass's commands (the same surfaces `opaque` lists).
			if ( gpuView.commands.IsValid() )
				gpuDrawList( gpuView, order, r.materials, depthPipeline,
				    []( const Resources::Material &m )
				    {
					    return m.program.blend != BlendMode::kOpaque;
				    } );
			else
				drawSurfaces( opaque, r.materials, depthPipeline );
			encoder.EndRendering();
			encoder.EndLabel();
		}
	}
	// Opaque PBR models also occlude the world and each other. Record their
	// identical posed/static geometry before any expensive surface shading.
	// Stencil mutation and depth overrides are ordered effects, so those
	// views retain their existing stream ordering.
	const bool modelDepthPrepass = depthPrepassSafe;
	bool modelDepthReady = modelDepthPrepass;
	if ( modelDepthPrepass )
	{
		encoder.BeginLabel( "core model depth" );
		encoder.BeginRendering( rendering );
		encoder.SetViewport( view.viewport );
		auto depthPipeline = [&]( const Resources::Material &m ) -> std::optional<PipelineId>
		{
			auto variant = m.resolver->VariantPipeline( m.program,
			    material::kSurfaceDepthNormal | material::kSurfaceDepthOnly,
			    material::kSurfaceSsrTargets );
			if ( !variant )
			{
				modelDepthReady = false;
				complete = false;
				note( "the model depth prepass: " + variant.Error() );
				return std::nullopt;
			}
			const auto state = surfaceStatePipeline( m, variant.Value(), target.drawState );
			if ( !state )
			{
				modelDepthReady = false;
				complete = false;
			}
			return state;
		};
		for ( const StaticDraw &draw : staticDraws )
		{
			const Resources::Material &m = r.modelMaterials[draw.material];
			if ( !opaquePbr( m ) || draw.gpu )
				continue;
			if ( const auto state = depthPipeline( m ) )
				recordModel( draw, m, *state );
		}
		(void)drawGpuModels( depthPipeline,
		    [&]( const Resources::Material &m )
		    {
			    return !opaquePbr( m );
		    } );
		encoder.EndRendering();
		encoder.EndLabel();
	}
	encoder.BeginLabel( "core world" );
	recordingTemporal = target.motion.IsValid();
	if ( recordingTemporal )
	{
		colors.push_back( { target.motion, LoadOp::kLoad, StoreOp::kStore, {}, {} } );
		colors.push_back( { target.motionDepth, LoadOp::kLoad, StoreOp::kStore, {}, {} } );
		rendering.colors = colors;
	}
	// render.ssr.v1's inputs: three more attachments, cleared at the first
	// begin and kept across the scene-color captures' restarts.
	const TextureId ssrTargets[] = {
	    target.ssrNormalRoughness, target.ssrIblRadiance, target.ssrSpecularWeight };
	recordingSsr = world->stage && !recordingTemporal && target.samples == 1 &&
	               std::all_of( std::begin( ssrTargets ), std::end( ssrTargets ),
	                   []( TextureId t )
	                   {
		                   return t.IsValid();
	                   } );
	if ( recordingSsr )
	{
		const ClearColor clears[] = { { 0, 0, 1, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } };
		for ( int i = 0; i < 3; ++i )
		{
			encoder.TransitionTexture(
			    ssrTargets[i], ResourceUsage::kSampled, ResourceUsage::kColorAttachment );
			colors.push_back( { ssrTargets[i], LoadOp::kClear, StoreOp::kStore, clears[i], {} } );
		}
		rendering.colors = colors;
	}
	encoder.BeginRendering( rendering );
	encoder.SetViewport( view.viewport );
	if ( recordingSsr )
		for ( std::size_t i = colors.size() - 3; i < colors.size(); ++i )
			colors[i].load = LoadOp::kLoad;
	RecordSection draws( encoder );
	draws.Select( "world surfaces" );
	auto worldPipeline = [&]( const Resources::Material &m ) -> std::optional<PipelineId>
	{
		return surfaceStatePipeline( m, m.program.request.pipeline,
		    worldDepthReady && opaquePbr( m ) ? prepassedState() : target.drawState,
		    frame::DebugSpecializationFor( view.debug, m.program.name ) );
	};
	if ( gpuView.commands.IsValid() )
	{
		gpuDrawList( gpuView, order, r.materials, worldPipeline );
		++s.stats.gpuViews;
	}
	else
		drawSurfaces( order, r.materials, worldPipeline, true );
	draws.Select( "prepare model draw order" );
	// The model list is built in caller order. Opaque draws can be grouped by
	// material, while blended surfaces must stay after them and retain their
	// submitted order; their pipeline has depth writes disabled.
	const auto firstBlended = std::stable_partition( staticDraws.begin(), staticDraws.end(),
	    [&]( const StaticDraw &draw )
	    {
		    const material::ResolvedProgram &program = r.modelMaterials[draw.material].program;
		    return program.blend == BlendMode::kOpaque && !program.sceneColor;
	    } );
	std::sort( staticDraws.begin(), firstBlended,
	    [&]( const StaticDraw &a, const StaticDraw &b )
	    {
		    return std::tie( a.cohort, a.material, a.mesh, a.posed, a.instance, a.surface ) <
		           std::tie( b.cohort, b.material, b.mesh, b.posed, b.instance, b.surface );
	    } );
	draws.End();
	std::uint64_t drawnStatic = 0;
	draws.Select( "models static gpu" );
	drawnStatic += drawGpuModels(
	    [&]( const Resources::Material &m )
	    {
		    return surfaceStatePipeline( m, m.program.request.pipeline,
		        modelDepthReady && opaquePbr( m ) ? prepassedState() : target.drawState );
	    },
	    []( const Resources::Material & )
	    {
		    return false;
	    } );
	std::uint64_t drawnPosed = 0;
	bool captureAttempted = false;
	bool sceneViewsPrepared = true;
	for ( const StaticDraw &draw : staticDraws )
	{
		if ( draw.gpu )
			continue;
		const Resources::Material &m = r.modelMaterials[draw.material];
		draws.Select( m.program.sceneColor ? "models transmitting / "
		              : draw.posed         ? "models posed / "
		                                   : "models static / ",
		    m.program.name );
		if ( m.program.sceneColor && !captureAttempted )
		{
			RecordSection capture( encoder );
			capture.Select( "model scene color capture and bindings" );
			encoder.EndRendering();
			captureAttempted = true;
			if ( !captureSceneColor() )
				complete = false;
			else
			{
				// View groups stage their constants and image bindings. Prepare
				// every transmitting layout while the host encoder is outside
				// rendering; uploads inside a render section are invalid.
				for ( const StaticDraw &candidate : staticDraws )
				{
					const Resources::Material &material = r.modelMaterials[candidate.material];
					if ( material.program.sceneColor &&
					     material.program.request.viewLayout.IsValid() &&
					     !viewGroupReady( material ) )
						sceneViewsPrepared = false;
				}
			}
			encoder.BeginRendering( rendering );
			encoder.SetViewport( view.viewport );
		}
		if ( m.program.sceneColor && ( !viewSceneColor.IsValid() || !sceneViewsPrepared ) )
		{
			complete = false;
			continue;
		}
		const auto state = surfaceStatePipeline( m, m.program.request.pipeline,
		    modelDepthReady && opaquePbr( m ) ? prepassedState() : target.drawState,
		    frame::DebugSpecializationFor( view.debug, m.program.name ) );
		if ( !state )
		{
			complete = false;
			continue;
		}
		recordModel( draw, m, *state );
		if ( draw.posed )
			++drawnPosed;
		else
			++drawnStatic;
	}
	draws.End();
	std::uint64_t drawnDynamic = 0;
	for ( const DynamicDraw &draw : dynamicDraws )
	{
		const Resources::Material &m = *draw.material;
		draws.Select( "dynamic / ", m.program.name );
		if ( m.program.sceneColor )
		{
			encoder.EndRendering();
			const bool captured = captureSceneColor();
			const Group *scene = captured ? viewGroupReady( m ) : nullptr;
			encoder.BeginRendering( rendering );
			encoder.SetViewport( view.viewport );
			if ( !scene )
			{
				complete = false;
				continue;
			}
		}
		const auto state = statePipeline( m, draw.pipeline, target.drawState,
		    frame::DebugSpecializationFor( view.debug, m.program.name ) );
		if ( !state )
		{
			complete = false;
			continue;
		}
		encoder.SetPipeline( *state );
		encoder.SetBindGroup(
		    BindGroupRole::kFrame, r.frameGroups[m.program.request.frameLayout.value].group );
		const auto layout = m.program.request.viewLayout.value;
		const Group *group = m.program.depthBlend       ? &depthViews[layout]
		                     : m.program.sceneColor     ? &sceneViews[layout]
		                     : m.refractInput != 0      ? &refractViews[layout]
		                     : litViews.count( layout ) ? litViews[layout]
		                                                : &r.viewGroups[layout];
		encoder.SetBindGroup( BindGroupRole::kView, group->group );
		encoder.SetBindGroup( BindGroupRole::kMaterial, m.group.group );
		encoder.SetBindGroup( BindGroupRole::kDraw,
		    draw.lit ? draw.lit->group
		             : r.drawGroups[drawKey( m, draw.page, draw.source->capturedLightmap )].group );
		material::FamilyDrawConstants dynamicConstants = constants;
		std::copy_n( draw.source->modelToWorld, 16, dynamicConstants.world );
		encoder.SetDrawConstants( 0, std::as_bytes( std::span( &dynamicConstants, 1 ) )
		                                 .first( m.program.request.drawConstantBytes ) );
		encoder.SetVertexBuffer( 0, draw.vertices, draw.vertexOffset );
		if ( recordingTemporal )
			encoder.SetVertexBuffer( 1, draw.vertices, draw.vertexOffset );
		encoder.SetIndexBuffer( draw.indices, draw.indexOffset, draw.indexFormat );
		encoder.DrawIndexed( draw.count, 1, 0, 0, 0 );
		++drawnDynamic;
	}
	draws.End();
	encoder.EndRendering();
	if ( recordingSsr )
		for ( const TextureId t : ssrTargets )
			encoder.TransitionTexture(
			    t, ResourceUsage::kColorAttachment, ResourceUsage::kSampled );
	recordingSsr = false;
	encoder.EndLabel();
	preparation.Select( "retire world view resources" );
	for ( auto &group : transientLitViews )
		s.retiredGroups.emplace_back( target.frame, std::move( group ) );
	for ( auto &group : litDrawGroups )
		s.retiredGroups.emplace_back( target.frame, std::move( group ) );
	for ( auto &[layout, group] : sceneViews )
		s.retiredGroups.emplace_back( target.frame, std::move( group ) );
	for ( auto &[layout, group] : depthViews )
		s.retiredGroups.emplace_back( target.frame, std::move( group ) );
	for ( auto &[layout, group] : reflectViews )
		s.retiredGroups.emplace_back( target.frame, std::move( group ) );
	for ( auto &[layout, group] : refractViews )
		s.retiredGroups.emplace_back( target.frame, std::move( group ) );
	std::lock_guard<std::mutex> guard( s.lock );
	s.stats.viewsDrawn += tags.size();
	s.stats.surfacesDrawn += order.size();
	s.stats.staticDrawsDrawn += drawnStatic;
	s.stats.posedDrawsDrawn += drawnPosed;
	s.stats.dynamicDrawsDrawn += drawnDynamic;
	s.stats.cutoutShadowDraws += cutoutDraws;
	s.stats.cutoutShadowIndirectDraws += cutoutIndirectDraws;
	s.stats.cutoutShadowRefused += cutoutRefused;
	s.stats.cutoutShadowNotResident += cutoutNotResident;
	if ( !complete && failure.empty() && viewMaterialPending )
	{
		// Incomplete only for materials waiting on a texture still being
		// filled: drawn again once resident, not a failure.
		++s.stats.viewsPending;
	}
	else if ( !complete )
	{
		++s.stats.viewsFailed;
		s.stats.lastFailure =
		    failure.empty() ? "a view named surfaces the pass does not draw" : failure;
	}
	return true;
}

} // namespace render::pass::world
