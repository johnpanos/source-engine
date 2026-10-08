//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.world (RFC 0016 K5): a batch's screen passes (prepass and ambient occlusion).
//
//=============================================================================//

#include "world_batch.h"

namespace render::pass::world
{

bool WorldPass::Batch::RecordScreenPasses()
{
	screenInputs[16] = view.viewport.x;
	screenInputs[17] = view.viewport.y;
	screenInputs[18] = view.viewport.width;
	screenInputs[19] = view.viewport.height;
	std::memcpy( screenInputs.data() + 20, target.clipPlanes, sizeof( target.clipPlanes ) );
	std::copy_n( target.foliage[0], 4, screenInputs.begin() + 44 );
	const bool screenReusable =
	    target.frame != 0 && s.screenFrame == target.frame && s.screenDepth == r.prepassDepth &&
	    s.screenOutput == target.ambientOcclusion && s.screenInputs == screenInputs;
	if ( world->stage && target.screenPasses && target.ambientOcclusion.IsValid() &&
	     screenReusable )
		viewOcclusion = target.ambientOcclusion;
	if ( world->stage && target.screenPasses && target.ambientOcclusion.IsValid() &&
	     !screenReusable )
	{
		preparation.Select( "prepare world prepass" );
		if ( !r.prepassResolver )
		{
			auto resolver = material::ProgramResolver::Create( device, Format::kRGBA16Float,
			    Format::kD32Float, 1, material::VertexLayout::kSurface );
			if ( resolver )
			{
				r.prepassResolver = std::move( resolver ).Value();
				r.prepassResolver->SetWorldPbr(
				    true, WorldTerms( *world, target.runtimeDirect, target.ambientOcclusionTerm ) );
				r.prepassMaterials.resize( world->materials.size() );
				prewarmResolver( *r.prepassResolver, "prepass" );
			}
			else
			{
				note( "the prepass resolver: " + resolver.Error() );
			}
		}
		if ( !r.prepassModelResolver )
		{
			auto resolver = material::ProgramResolver::Create( device, Format::kRGBA16Float,
			    Format::kD32Float, 1, material::VertexLayout::kModel );
			if ( resolver )
			{
				r.prepassModelResolver = std::move( resolver ).Value();
				r.prepassModelResolver->SetWorldPbr(
				    true, WorldTerms( *world, target.runtimeDirect, target.ambientOcclusionTerm ) );
				r.prepassModelResolver->SetSceneColorAvailable( true );
				r.prepassModelMaterials.resize( world->materials.size() );
				prewarmResolver( *r.prepassModelResolver, "prepass-model" );
			}
			else
				note( "the model prepass resolver: " + resolver.Error() );
		}
		if ( r.prepassResolver && ( r.prepassDepthDesc.width != target.width ||
		                              r.prepassDepthDesc.height != target.height ) )
		{
			for ( TextureId old : { r.prepassDepth, r.prepassNormal } )
			{
				if ( old.IsValid() )
					s.retiredTextures.emplace_back( target.frame, old );
			}
			r.prepassDepth = r.prepassNormal = TextureId();
			TextureDesc depthDesc;
			depthDesc.format = Format::kD32Float;
			depthDesc.width = target.width;
			depthDesc.height = target.height;
			depthDesc.usages = { ResourceUsage::kDepthWrite, ResourceUsage::kSampled };
			depthDesc.debugName = "world prepass depth";
			TextureDesc normalDesc = depthDesc;
			normalDesc.format = Format::kRGBA16Float;
			normalDesc.usages = { ResourceUsage::kColorAttachment, ResourceUsage::kSampled };
			normalDesc.debugName = "world prepass normal";
			auto depthTexture = device.CreateTexture( depthDesc );
			auto normalTexture = device.CreateTexture( normalDesc );
			if ( depthTexture && normalTexture )
			{
				r.prepassDepth = depthTexture.Value();
				r.prepassNormal = normalTexture.Value();
				depthDesc.debugName = normalDesc.debugName = {};
				r.prepassDepthDesc = depthDesc;
				r.prepassNormalDesc = normalDesc;
				r.prepassUsed = false;
			}
			else
			{
				for ( auto *made : { &depthTexture, &normalTexture } )
				{
					if ( *made )
						(void)device.Release( made->Value(), CompletionToken() );
				}
				r.prepassDepthDesc = TextureDesc();
				note( "the prepass targets were refused" );
			}
		}
		if ( r.prepassResolver && r.prepassDepth.IsValid() )
		{
			// The lists are the world's, not the view's: the same entries are
			// drawn into the pass's own targets whichever view needs them, and
			// they change only with the world, its claims and which model
			// levels are resident. Built once for this resource set, retried
			// while any material or group in them is not ready.
			Resources::PrepassLists &cached = r.prepassLists;
			const bool reusable = cached.valid && cached.generation == generation &&
			                      cached.residencyRevision == s.modelResidencyRevision &&
			                      cached.claims == claims.get();
			std::vector<std::uint32_t> prepassStorage;
			std::vector<StaticDraw> prepassModelStorage;
			// A material or group that was not ready leaves an entry out, so a
			// partial build is never remembered: the next view tries again.
			bool listsComplete = true;
			if ( !reusable )
			{
				cached = Resources::PrepassLists();
				cached.generation = generation;
				cached.residencyRevision = s.modelResidencyRevision;
				cached.claims = claims.get();
			}
			std::vector<std::uint32_t> &prepass = reusable ? cached.surfaces : prepassStorage;
			// Draw, frame and view groups for one prepass material. Only the
			// index list is cached between calls; the view group especially is
			// this call's state, so a reused list resolves them again below.
			auto prepassGroupsReady = [&]( const Resources::Material &m, int page ) -> bool
			{
				return ( !m.program.request.drawLayout.IsValid() || drawGroupReady( m, page ) ) &&
				       ( !m.program.request.frameLayout.IsValid() || frameGroupReady( m ) ) &&
				       ( !m.program.request.viewLayout.IsValid() || viewGroupReady( m ) );
			};
			for ( std::uint32_t index = 0; index < world->surfaces.size() && !reusable; ++index )
			{
				const WorldSurface &surface = world->surfaces[index];
				if ( surface.material >= claims->size() || !( *claims )[surface.material].draws )
					continue;
				const Resources::Material *m =
				    materialReadyIn( *r.prepassResolver, r.prepassMaterials, surface.material );
				if ( !m || !prepassGroupsReady( *m, surface.lightmapPage ) )
				{
					complete = false;
					listsComplete = false;
					continue;
				}
				if ( opaquePbr( *m ) )
					prepass.push_back( index );
			}
			std::vector<StaticDraw> &prepassModels = reusable ? cached.models : prepassModelStorage;
			if ( r.prepassModelResolver && !reusable )
			{
				// The prepass covers the world's instances, not this view's, so
				// it draws only the levels already resident: a released level is
				// uploaded when a view selects it, not by this pass over every
				// instance (which would keep every level resident forever).
				for ( std::uint32_t id = 0; id < world->staticInstances.size(); ++id )
				{
					const auto &instance = world->staticInstances[id];
					if ( instance.mesh >= world->staticMeshes.size() )
						continue;
					const auto &mesh = world->staticMeshes[instance.mesh];
					for ( std::uint32_t surface = 0; surface < mesh.surfaces.size(); ++surface )
					{
						if ( !SurfaceSelected( instance.surfaceSelection, surface ) )
							continue;
						const std::uint32_t material = StaticMaterial( mesh, instance, surface );
						if ( material >= claims->size() || !( *claims )[material].draws ||
						     !world->materials[material].mesh )
							continue;
						const auto *m = materialReadyIn(
						    *r.prepassModelResolver, r.prepassModelMaterials, material );
						if ( !m )
						{
							complete = false;
							listsComplete = false;
							continue;
						}
						if ( !opaquePbr( *m ) )
							continue;
						const std::uint32_t lod = mesh.LodOfSurface( surface );
						if ( lod == ~0u || lod >= s.models.models[instance.mesh].size() ||
						     !s.models.models[instance.mesh][lod].resident )
							continue;
						if ( !prepassGroupsReady( *m, 0 ) )
						{
							complete = false;
							listsComplete = false;
							continue;
						}
						prepassModels.push_back(
						    { 0, false, id, instance.mesh, lod, surface, material } );
					}
				}
			}
			// A reused list must still resolve the groups its entries bind: they
			// belong to this call, not to the cache. An entry whose groups are not
			// ready now is left out of this record without changing the cached
			// list, so a later call can draw it.
			std::vector<std::uint32_t> reusedPrepass;
			std::vector<StaticDraw> reusedPrepassModels;
			const std::vector<std::uint32_t> *drawnPrepass = &prepass;
			const std::vector<StaticDraw> *drawnPrepassModels = &prepassModels;
			if ( reusable )
			{
				reusedPrepass.reserve( prepass.size() );
				for ( const std::uint32_t index : prepass )
				{
					const WorldSurface &surface = world->surfaces[index];
					const Resources::Material *m =
					    surface.material < claims->size() && ( *claims )[surface.material].draws
					        ? materialReadyIn(
					              *r.prepassResolver, r.prepassMaterials, surface.material )
					        : nullptr;
					if ( m && prepassGroupsReady( *m, surface.lightmapPage ) )
						reusedPrepass.push_back( index );
					else
						complete = false;
				}
				drawnPrepass = &reusedPrepass;
				if ( r.prepassModelResolver )
				{
					reusedPrepassModels.reserve( prepassModels.size() );
					for ( const StaticDraw &draw : prepassModels )
					{
						const auto *m = materialReadyIn(
						    *r.prepassModelResolver, r.prepassModelMaterials, draw.material );
						if ( m && prepassGroupsReady( *m, 0 ) )
							reusedPrepassModels.push_back( draw );
						else
							complete = false;
					}
					drawnPrepassModels = &reusedPrepassModels;
				}
			}
			// The targets rest in kSampled between views (undefined before
			// their first); their contents are rewritten whole.
			const ResourceUsage rest =
			    r.prepassUsed ? ResourceUsage::kSampled : ResourceUsage::kUndefined;
			encoder.TransitionTexture( r.prepassNormal, rest, ResourceUsage::kColorAttachment );
			encoder.TransitionTexture( r.prepassDepth, rest, ResourceUsage::kDepthWrite );
			r.prepassUsed = true;
			const ColorAttachment normal[] = {
			    { r.prepassNormal, LoadOp::kClear, StoreOp::kStore, { 0, 0, 1, 0 }, {} } };
			RenderingDesc prepassRendering;
			prepassRendering.colors = normal;
			prepassRendering.depth =
			    DepthAttachment{ r.prepassDepth, LoadOp::kClear, StoreOp::kStore, 1.0f };
			prepassRendering.width = target.width;
			prepassRendering.height = target.height;
			preparation.End();
			// GPU-driven: the world's prepass list frustum-culled to this
			// view (it lists every opaque surface of the world) and drawn per
			// bucket; never occlusion-tested, since it makes the pyramid.
			const GpuList prepassGpu = target.gpuSubmission && gpuDeviceCapable
			                               ? gpuCullList( *drawnPrepass, r.prepassMaterials, false )
			                               : GpuList();
			const GpuModels prepassModelsGpu =
			    gpuCullModels( *drawnPrepassModels, r.prepassModelMaterials, false, false );
			encoder.BeginLabel( "core world prepass" );
			encoder.BeginRendering( prepassRendering );
			// This private D32 target has no stencil and encodes projection
			// depth for reconstruction. Portal masks and depth-range remapping
			// belong to the final target, not this offscreen view.
			Viewport prepassViewport = view.viewport;
			prepassViewport.minDepth = 0.0f;
			prepassViewport.maxDepth = 1.0f;
			encoder.SetViewport( prepassViewport );
			auto prepassPipeline = [&]( const Resources::Material &m ) -> std::optional<PipelineId>
			{
				auto variant = m.resolver->VariantPipeline(
				    m.program, material::kSurfaceDepthNormal, material::kSurfaceSsrTargets );
				if ( !variant )
				{
					note( "the prepass: " + variant.Error() );
					return std::nullopt;
				}
				return surfaceStatePipeline( m, variant.Value(), material::SurfaceDrawState() );
			};
			if ( prepassGpu.commands.IsValid() )
			{
				gpuDrawList( prepassGpu, *drawnPrepass, r.prepassMaterials, prepassPipeline );
				++s.stats.gpuPrepassViews;
			}
			else
				drawSurfaces( *drawnPrepass, r.prepassMaterials, prepassPipeline );
			if ( occlusionWanted )
			{
				// The pyramid holds the world surfaces only: the models'
				// prepass levels need not be the levels this view shades.
				encoder.EndRendering();
				const bool built = buildPyramid();
				if ( !built )
					note( "the occlusion pyramid was not recorded" );
				ColorAttachment resumed[] = { normal[0] };
				resumed[0].load = LoadOp::kLoad;
				prepassRendering.colors = resumed;
				prepassRendering.depth =
				    DepthAttachment{ r.prepassDepth, LoadOp::kLoad, StoreOp::kStore, 1.0f };
				encoder.BeginRendering( prepassRendering );
				encoder.SetViewport( prepassViewport );
			}
			auto prepassModelPipeline =
			    [&]( const Resources::Material &m ) -> std::optional<PipelineId>
			{
				auto variant = m.resolver->VariantPipeline(
				    m.program, material::kSurfaceDepthNormal, material::kSurfaceSsrTargets );
				if ( !variant )
				{
					note( "the model prepass: " + variant.Error() );
					return std::nullopt;
				}
				return surfaceStatePipeline( m, variant.Value(), material::SurfaceDrawState() );
			};
			for ( std::size_t i = 0; i < drawnPrepassModels->size(); ++i )
			{
				if ( i < prepassModelsGpu.taken.size() && prepassModelsGpu.taken[i] )
					continue;
				const StaticDraw &draw = ( *drawnPrepassModels )[i];
				const auto &m = r.prepassModelMaterials[draw.material];
				if ( const auto pipeline = prepassModelPipeline( m ) )
					recordModel( draw, m, *pipeline );
				else
					complete = false;
			}
			(void)drawModelBuckets( prepassModelsGpu, r.prepassModelMaterials, prepassModelPipeline,
			    []( const Resources::Material & )
			    {
				    return false;
			    } );
			encoder.EndRendering();
			encoder.EndLabel();
			encoder.TransitionTexture(
			    r.prepassNormal, ResourceUsage::kColorAttachment, ResourceUsage::kSampled );
			encoder.TransitionTexture(
			    r.prepassDepth, ResourceUsage::kDepthWrite, ResourceUsage::kSampled );
			if ( target.screenPasses(
			         encoder, { r.prepassDepth, r.prepassNormal, target.width, target.height } ) )
			{
				viewOcclusion = target.ambientOcclusion;
				s.screenFrame = target.frame;
				s.screenDepth = r.prepassDepth;
				s.screenOutput = target.ambientOcclusion;
				s.screenInputs = screenInputs;
			}
			else
			{
				complete = false;
				note( "the camera's ambient occlusion was not recorded" );
			}
			// Keep a complete build for the next view and the next frame; the
			// aliases above are finished with.
			if ( !reusable && listsComplete )
			{
				cached.surfaces = std::move( prepassStorage );
				cached.models = std::move( prepassModelStorage );
				cached.valid = true;
				++s.stats.prepassListBuilds;
			}
			else if ( !reusable )
			{
				++s.stats.prepassListRetries;
			}
			else
			{
				++s.stats.prepassListReuses;
			}
		}
	}
	preparation.Select( "prepare lit view bindings" );
	// No screen passes (ambient occlusion off): the target holds the
	// neutral occlusion, one.
	if ( world->stage && !target.screenPasses && target.ambientOcclusion.IsValid() )
		viewOcclusion = target.ambientOcclusion;
	// The lit view groups, with the occlusion when it was recorded.
	std::erase_if( order,
	    [&]( std::uint32_t index )
	    {
		    const Resources::Material &m = r.materials[world->surfaces[index].material];
		    if ( m.program.request.viewLayout.IsValid() && !viewGroupReady( m ) )
		    {
			    complete = false;
			    return true;
		    }
		    return false;
	    } );
	std::erase_if( staticDraws,
	    [&]( const StaticDraw &draw )
	    {
		    const Resources::Material &m = r.modelMaterials[draw.material];
		    if ( m.program.sceneColor )
			    return false; // the snapshot is recorded after opaque draws
		    if ( m.program.request.viewLayout.IsValid() && !viewGroupReady( m ) )
		    {
			    complete = false;
			    return true;
		    }
		    return false;
	    } );
	preparation.Select( "prepare dynamic materials and uploads" );
	return true;
}

} // namespace render::pass::world
