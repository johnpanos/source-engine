//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.world (RFC 0016 K5): a batch's draw helpers and GPU-driven submission.
//
//=============================================================================//

#include "world_batch.h"

namespace render::pass::world
{

std::optional<PipelineId> WorldPass::Batch::statePipeline( const Resources::Material &m,
    PipelineId base, const material::SurfaceDrawState &state,
    const shaderlib::DebugSpecialization &debug )
{
	// The SSR targets' variant of the draw's point: three more outputs.
	if ( recordingSsr )
	{
		auto ssr = m.resolver->Program().VariantPipeline( base, material::kSurfaceSsrTargets, 0 );
		if ( !ssr )
		{
			note( "the SSR targets' variant was refused for " + m.program.name );
			return std::nullopt;
		}
		base = ssr.Value();
	}
	auto result = recordingTemporal
	                  ? m.resolver->Program().TemporalPipeline( base, state, viewFeatures, debug )
	                  : m.resolver->Program().ViewPipeline( base, state, viewFeatures, debug );
	if ( !result )
	{
		note( "view raster state: pipeline refused for " + m.program.name + " status " +
		      std::to_string( int( result.Error() ) ) + " depth format " +
		      std::to_string( int( target.depthFormat ) ) + " stencil " +
		      std::to_string( state.stencil.enabled ) + ": " +
		      m.resolver->Program().PipelineFailure() );
		return std::nullopt;
	}
	return result.Value();
}

// World and MDL-reader triangles face counterclockwise from outside.
// Their owner applies authored culling, including on posed models and
// private prepasses. Dynamic snapshots keep their captured raster state.
std::optional<PipelineId> WorldPass::Batch::surfaceStatePipeline( const Resources::Material &m,
    PipelineId base, material::SurfaceDrawState state, const shaderlib::DebugSpecialization &debug )
{
	state.cull = m.program.twoSided ? CullMode::kNone : CullMode::kBack;
	return statePipeline( m, base, state, debug );
}

template <typename Vertices, typename Indices>
void WorldPass::Batch::submitSurfaceFootprints( const WorldSurface &surface,
    const std::vector<Resources::Material> &materials, const Vertices &vertices,
    const Indices &indices, FootprintExtents &extents, const float *objectToWorld,
    const SurfaceFootprintGeometry *compact, const SurfaceFootprintBounds *box )
{
	if ( !target.mipFeedback || !view.viewport.width || !view.viewport.height ||
	     surface.firstIndex > indices.size() ||
	     surface.indexCount > indices.size() - surface.firstIndex )
		return;
	// One clip-space point onto the viewport, grown into the footprint's
	// screen box; false for a point behind the eye or not finite.
	const auto projectCorner =
	    [&]( const float clip[4], float &minX, float &minY, float &maxX, float &maxY )
	{
		if ( !std::isfinite( clip[0] ) || !std::isfinite( clip[1] ) || !std::isfinite( clip[3] ) ||
		     clip[3] <= 1.0e-6f )
			return false;
		const float x = std::clamp(
		    ( clip[0] / clip[3] * 0.5f + 0.5f ) * view.viewport.width, 0.0f, view.viewport.width );
		const float y = std::clamp( ( 0.5f - clip[1] / clip[3] * 0.5f ) * view.viewport.height,
		    0.0f, view.viewport.height );
		minX = std::min( minX, x );
		minY = std::min( minY, y );
		maxX = std::max( maxX, x );
		maxY = std::max( maxY, y );
		return true;
	};
	// The box's eight corners bound every vertex's projection while all of
	// them are in front of the eye; a corner behind it falls back to the
	// surface's own vertices, which may still all be in front.
	bool boxResolved = false;
	if ( extents.state == FootprintExtents::State::kUnknown && box )
	{
		extents.state = FootprintExtents::State::kInvalid;
		boxResolved = true;
		if ( box->valid )
		{
			float toClip[16];
			for ( int row = 0; row < 4; ++row )
				for ( int col = 0; col < 4; ++col )
				{
					float value = 0.0f;
					for ( int k = 0; k < 4; ++k )
						value += view.toClip[row * 4 + k] * ( objectToWorld
						                                            ? objectToWorld[k * 4 + col]
						                                            : ( k == col ? 1.0f : 0.0f ) );
					toClip[row * 4 + col] = value;
				}
			float minX = view.viewport.width, minY = view.viewport.height;
			float maxX = 0.0f, maxY = 0.0f;
			bool front = true;
			for ( int corner = 0; corner < 8 && front; ++corner )
			{
				const float p[3] = { ( corner & 1 ) ? box->hi[0] : box->lo[0],
				    ( corner & 2 ) ? box->hi[1] : box->lo[1],
				    ( corner & 4 ) ? box->hi[2] : box->lo[2] };
				float clip[4];
				for ( int row = 0; row < 4; ++row )
					clip[row] = toClip[row * 4 + 0] * p[0] + toClip[row * 4 + 1] * p[1] +
					            toClip[row * 4 + 2] * p[2] + toClip[row * 4 + 3];
				if ( !projectCorner( clip, minX, minY, maxX, maxY ) )
				{
					front = false;
					break;
				}
			}
			if ( !front )
			{
				extents.state = FootprintExtents::State::kUnknown;
				boxResolved = false;
			}
			else if ( maxX > minX && maxY > minY && box->maxU >= box->minU &&
			          box->maxV >= box->minV )
				extents = { FootprintExtents::State::kValid, minX, minY, maxX, maxY, box->minU,
				    box->minV, box->maxU, box->maxV };
		}
	}
	if ( !boxResolved && extents.state == FootprintExtents::State::kUnknown )
	{
		extents.state = FootprintExtents::State::kInvalid;
		if ( compact )
		{
			if ( compact->valid )
			{
				float minX = view.viewport.width, minY = view.viewport.height;
				float maxX = 0.0f, maxY = 0.0f;
				bool valid = true;
				for ( const std::array<float, 3> &position : compact->positions )
				{
					float clip[4] = {};
					for ( int row = 0; row < 4; ++row )
						clip[row] = view.toClip[row * 4 + 0] * position[0] +
						            view.toClip[row * 4 + 1] * position[1] +
						            view.toClip[row * 4 + 2] * position[2] +
						            view.toClip[row * 4 + 3];
					if ( !projectCorner( clip, minX, minY, maxX, maxY ) )
					{
						valid = false;
						break;
					}
				}
				if ( valid && maxX > minX && maxY > minY && compact->maxU >= compact->minU &&
				     compact->maxV >= compact->minV )
					extents = { FootprintExtents::State::kValid, minX, minY, maxX, maxY,
					    compact->minU, compact->minV, compact->maxU, compact->maxV };
			}
		}
		else
		{
			float minX = view.viewport.width;
			float minY = view.viewport.height;
			float maxX = 0.0f;
			float maxY = 0.0f;
			float minU = std::numeric_limits<float>::max();
			float minV = std::numeric_limits<float>::max();
			float maxU = std::numeric_limits<float>::lowest();
			float maxV = std::numeric_limits<float>::lowest();
			bool valid = true;
			for ( std::uint32_t i = 0; i < surface.indexCount; ++i )
			{
				const std::uint32_t vertexIndex = indices[surface.firstIndex + i];
				if ( vertexIndex >= vertices.size() )
					valid = false;
				if ( !valid )
					break;
				const auto &vertex = vertices[vertexIndex];
				if ( !std::isfinite( vertex.uv[0] ) || !std::isfinite( vertex.uv[1] ) )
				{
					valid = false;
					break;
				}
				float position[4] = {
				    vertex.position[0], vertex.position[1], vertex.position[2], 1.0f };
				if ( objectToWorld )
				{
					float transformed[4] = {};
					for ( int row = 0; row < 4; ++row )
						for ( int col = 0; col < 4; ++col )
							transformed[row] += objectToWorld[row * 4 + col] * position[col];
					std::copy_n( transformed, 4, position );
				}
				float clip[4] = {};
				for ( int row = 0; row < 4; ++row )
					for ( int col = 0; col < 4; ++col )
						clip[row] += view.toClip[row * 4 + col] * position[col];
				if ( !projectCorner( clip, minX, minY, maxX, maxY ) )
				{
					valid = false;
					break;
				}
				minU = std::min( minU, vertex.uv[0] );
				minV = std::min( minV, vertex.uv[1] );
				maxU = std::max( maxU, vertex.uv[0] );
				maxV = std::max( maxV, vertex.uv[1] );
			}
			if ( valid && maxX > minX && maxY > minY && maxU >= minU && maxV >= minV )
			{
				extents = { FootprintExtents::State::kValid, minX, minY, maxX, maxY, minU, minV,
				    maxU, maxV };
			}
		}
	}
	if ( extents.state != FootprintExtents::State::kValid )
		return;
	const float minX = extents.minX, minY = extents.minY, maxX = extents.maxX, maxY = extents.maxY,
	            minU = extents.minU, minV = extents.minV, maxU = extents.maxU, maxV = extents.maxV;
	const void **seen = extents.submitted;
	const void **free = nullptr;
	for ( int i = 0; i < 3; ++i )
	{
		if ( seen[i] == &materials )
			return;
		if ( !seen[i] && !free )
			free = &seen[i];
	}
	if ( free )
		*free = &materials;
	auto slots = footprintSlots.find( { &materials, surface.material } );
	if ( slots == footprintSlots.end() )
	{
		std::vector<FootprintSlot> resolved;
		const auto &handles = ( *claims )[surface.material].handles;
		const auto &texturesUsed = materials[surface.material].program.request.material.textures;
		for ( std::uint32_t slot = 0; slot < texturesUsed.size(); ++slot )
		{
			const auto handle = handles.find( texturesUsed[slot].name );
			if ( handle == handles.end() || handle->second <= 0 )
				continue;
			const auto info = target.textures->MipDescription( handle->second );
			if ( !info )
				continue;
			resolved.push_back( { slot, info->width, info->height, info->levels } );
		}
		slots =
		    footprintSlots
		        .emplace( std::pair{ static_cast<const void *>( &materials ), surface.material },
		            std::move( resolved ) )
		        .first;
	}
	for ( const FootprintSlot &slot : slots->second )
	{
		resources::VisibleTextureFootprint footprint;
		footprint.material = surface.material;
		footprint.textureSlot = slot.slot;
		footprint.textureWidth = slot.width;
		footprint.textureHeight = slot.height;
		footprint.mipLevels = slot.levels;
		footprint.screenWidth = maxX - minX;
		footprint.screenHeight = maxY - minY;
		footprint.uvWidth = maxU - minU;
		footprint.uvHeight = maxV - minV;
		target.mipFeedback->AddVisible( footprint );
	}
}

// A world material's pipeline, groups, the world's shared vertex and index
// buffers, and the view's draw constants: what both the per-surface runs
// and the GPU-driven buckets bind.
void WorldPass::Batch::bindSurfaceMaterial( const Resources::Material &m, PipelineId pipeline )
{
	encoder.SetPipeline( pipeline );
	if ( m.program.request.frameLayout.IsValid() )
		encoder.SetBindGroup(
		    BindGroupRole::kFrame, r.frameGroups[m.program.request.frameLayout.value].group );
	if ( m.program.request.viewLayout.IsValid() )
	{
		const std::uint64_t layout = m.program.request.viewLayout.value;
		const auto lit = litViews.find( layout );
		const auto reflect = reflectViews.find( layout );
		const auto refract = refractViews.find( layout );
		encoder.SetBindGroup( BindGroupRole::kView,
		    m.refractInput != 0 && refract != refractViews.end() ? refract->second.group
		    : lit != litViews.end()                              ? lit->second->group
		    : m.viewInput != 0 && reflect != reflectViews.end()  ? reflect->second.group
		                                                         : r.viewGroups[layout].group );
	}
	encoder.SetBindGroup( BindGroupRole::kMaterial, m.group.group );
	encoder.SetVertexBuffer( 0, r.vertices, 0 );
	if ( recordingTemporal )
		encoder.SetVertexBuffer( 1, r.vertices, 0 );
	encoder.SetIndexBuffer( r.indices, 0, IndexFormat::kUint32 );
	encoder.SetDrawConstants(
	    0, ( m.program.name == "water" && view.waterZOffset != 0.0f ? waterConstantBytes
	                                                                : constantBytes )
	           .first( m.program.request.drawConstantBytes ) );
}

void WorldPass::Batch::drawSurfaces( const std::vector<std::uint32_t> &list,
    const std::vector<Resources::Material> &materials,
    const std::function<std::optional<PipelineId>( const Resources::Material & )> &pipelineOf,
    bool breakdown )
{
	RecordSection family( encoder );
	std::uint32_t boundMaterial = ~0u;
	int boundPage = 0;
	bool pageBound = false;
	bool skipping = false;
	std::uint32_t runFirst = 0;
	std::uint32_t runCount = 0;
	auto flushRun = [&]()
	{
		if ( runCount )
			encoder.DrawIndexed( runCount, 1, runFirst, 0, 0 );
		runCount = 0;
	};
	for ( const std::uint32_t index : list )
	{
		const WorldSurface &surface = world->surfaces[index];
		const Resources::Material &m = materials[surface.material];
		if ( surface.material != boundMaterial )
		{
			flushRun();
			if ( breakdown )
				family.Select( "world / ", m.program.name );
			boundMaterial = surface.material;
			pageBound = false;
			const std::optional<PipelineId> pipeline = pipelineOf( m );
			skipping = !pipeline;
			if ( skipping )
			{
				complete = false;
				continue;
			}
			bindSurfaceMaterial( m, *pipeline );
		}
		if ( skipping )
			continue;
		submitSurfaceFootprints( surface, materials, world->vertices, world->indices,
		    worldFootprints.empty() ? scratchFootprint : worldFootprints[index], nullptr,
		    footprintGeometry && index < footprintGeometry->size() ? &( *footprintGeometry )[index]
		                                                           : nullptr );
		if ( m.program.request.drawLayout.IsValid() &&
		     ( !pageBound || surface.lightmapPage != boundPage ) )
		{
			flushRun();
			encoder.SetBindGroup(
			    BindGroupRole::kDraw, r.drawGroups[drawKey( m, surface.lightmapPage )].group );
			boundPage = surface.lightmapPage;
			pageBound = true;
		}
		if ( runCount && surface.firstIndex == runFirst + runCount )
		{
			runCount += surface.indexCount;
		}
		else
		{
			flushRun();
			runFirst = surface.firstIndex;
			runCount = surface.indexCount;
		}
	}
	flushRun();
}

// A model draw's FamilyDrawConstants: its object-to-world (identity for
// a pose, whose vertices are in world space) and object-to-clip.
material::FamilyDrawConstants WorldPass::Batch::modelDrawConstants( const StaticDraw &draw )
{
	const scene::MeshInstanceDesc *instance = draw.posed ? nullptr : &world->Prop( draw.instance );
	material::FamilyDrawConstants modelConstants;
	if ( instance )
		std::copy_n( &instance->world.rows[0].x, 16, modelConstants.world );
	else
	{
		for ( int i = 0; i < 4; ++i )
			modelConstants.world[i * 5] = 1.0f;
	}
	for ( int row = 0; row < 4; ++row )
	{
		for ( int col = 0; col < 4; ++col )
		{
			float value = 0.0f;
			for ( int k = 0; k < 4; ++k )
				value += constants.toClip[row * 4 + k] * modelConstants.world[k * 4 + col];
			modelConstants.toClip[row * 4 + col] = value;
		}
	}
	return modelConstants;
}

// Binds a model draw's pipeline, groups and buffers; with `instances`
// (an instanced point's per-instance records) instead of its draw
// constants.
void WorldPass::Batch::bindModel(
    const StaticDraw &draw, const Resources::Material &m, PipelineId pipeline, BufferId instances )
{
	encoder.SetPipeline( pipeline );
	if ( m.program.request.frameLayout.IsValid() )
		encoder.SetBindGroup(
		    BindGroupRole::kFrame, r.frameGroups[m.program.request.frameLayout.value].group );
	if ( m.program.request.viewLayout.IsValid() )
	{
		const std::uint64_t layout = m.program.request.viewLayout.value;
		const auto lit = modelLitViews.find( layout );
		const auto scene = sceneViews.find( layout );
		encoder.SetBindGroup( BindGroupRole::kView,
		    ( m.program.sceneColor || m.program.depthBlend ) && scene != sceneViews.end()
		        ? scene->second.group
		    : lit != modelLitViews.end() ? lit->second->group
		                                 : r.viewGroups[layout].group );
	}
	encoder.SetBindGroup( BindGroupRole::kMaterial, m.group.group );
	if ( m.program.request.drawLayout.IsValid() )
		encoder.SetBindGroup( BindGroupRole::kDraw, r.drawGroups[drawKey( m, 0 )].group );
	const material::FamilyDrawConstants modelConstants =
	    instances.IsValid() ? material::FamilyDrawConstants() : modelDrawConstants( draw );
	encoder.SetDrawConstants( 0, std::as_bytes( std::span( &modelConstants, 1 ) )
	                                 .first( m.program.request.drawConstantBytes ) );
	const State::ModelLevel &buffers = s.models.models[draw.mesh][draw.lod];
	encoder.SetVertexBuffer( 0, draw.posed ? posedBuffers[draw.instance] : buffers.vertices );
	if ( instances.IsValid() )
		encoder.SetVertexBuffer( 1, instances );
	else if ( recordingTemporal )
		encoder.SetVertexBuffer(
		    1, draw.posed ? previousPosedBuffers[draw.instance] : buffers.vertices );
	encoder.SetIndexBuffer( buffers.indices, 0, IndexFormat::kUint32 );
}

// A model draw's texture mip feedback (the per-surface footprint).
void WorldPass::Batch::modelFeedback( const StaticDraw &draw )
{
	const scene::MeshInstanceDesc *instance = draw.posed ? nullptr : &world->Prop( draw.instance );
	const WorldData::StaticMesh &mesh = world->staticMeshes[draw.mesh];
	const WorldSurface &surface = mesh.surfaces[draw.surface];
	WorldSurface feedbackSurface = surface;
	feedbackSurface.material = draw.material;
	// The level's own staging, whose indices the level's surface counts into.
	const WorldData::StaticMeshLod &level = mesh.lods[draw.lod];
	if ( level.vertices && level.indices && target.mipFeedback )
	{
		const auto &vertices =
		    draw.posed ? view.posedModels[draw.instance].vertices : *level.vertices;
		FootprintExtents &extents =
		    modelFootprints[{ draw.posed, draw.instance, draw.mesh, draw.lod, draw.surface }];
		// A static instance's box is the world's, built once; a pose's box
		// is measured once per view from its own vertices.
		SurfaceFootprintBounds posedBox;
		const SurfaceFootprintBounds *box = nullptr;
		if ( extents.state == FootprintExtents::State::kUnknown )
		{
			if ( draw.posed )
			{
				posedBox = MeasureFootprintBounds( surface, vertices, *level.indices );
				box = &posedBox;
			}
			else if ( modelFootprintBounds && draw.mesh < modelFootprintBounds->size() &&
			          draw.surface < ( *modelFootprintBounds )[draw.mesh].size() &&
			          draw.surface < mesh.surfaceLods.size() &&
			          mesh.surfaceLods[draw.surface] == draw.lod )
				box = &( *modelFootprintBounds )[draw.mesh][draw.surface];
		}
		submitSurfaceFootprints( feedbackSurface, r.modelMaterials, vertices, *level.indices,
		    extents, instance ? &instance->world.rows[0].x : nullptr, nullptr, box );
	}
}

void WorldPass::Batch::recordModel(
    const StaticDraw &draw, const Resources::Material &m, PipelineId pipeline )
{
	bindModel( draw, m, pipeline );
	modelFeedback( draw );
	const WorldSurface &surface = world->staticMeshes[draw.mesh].surfaces[draw.surface];
	encoder.DrawIndexed( surface.indexCount, 1, surface.firstIndex, 0, 0 );
}

// A world stage's screen passes (render_lab's order): the depth and
// normal prepass into the pass's own single-sample targets, with a
// resolver of their own, then the composition's ambient occlusion, which
// the lit view groups read.
bool WorldPass::Batch::opaquePbr( const Resources::Material &m )
{
	return m.program.blend == BlendMode::kOpaque && !m.program.sceneColor &&
	       m.program.name == "pbr";
}

math::float4x4 WorldPass::Batch::viewToClip()
{
	math::float4x4 toClip;
	for ( int row = 0; row < 4; ++row )
		toClip.rows[row] = { view.toClip[row * 4 + 0], view.toClip[row * 4 + 1],
		    view.toClip[row * 4 + 2], view.toClip[row * 4 + 3] };
	return toClip;
}

// Records the pyramid of r.prepassDepth (in kDepthWrite, outside a
// rendering) and leaves the depth in kDepthWrite again.
bool WorldPass::Batch::buildPyramid()
{
	using namespace render::culling;
	if ( !s.gpuOcclusion )
	{
		auto kernels = OcclusionKernels::Create( device );
		if ( kernels )
			s.gpuOcclusion = std::move( kernels ).Value();
	}
	if ( !s.gpuPointSampler.IsValid() )
	{
		SamplerDesc desc;
		desc.minFilter = desc.magFilter = desc.mipFilter = Filter::kNearest;
		desc.address = AddressMode::kClampToEdge;
		auto made = device.CreateSampler( desc );
		if ( made )
			s.gpuPointSampler = made.Value();
	}
	if ( !s.gpuOcclusion || !s.gpuPointSampler.IsValid() )
		return false;
	const OcclusionView occlusionView =
	    MakeOcclusionView( viewToClip(), target.width, target.height, 0 );
	BufferDesc viewDesc;
	viewDesc.size = sizeof( OcclusionView );
	viewDesc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kStorageRead };
	BufferDesc pyramidDesc;
	pyramidDesc.size = PyramidFloats( occlusionView ) * sizeof( float );
	pyramidDesc.usages = { ResourceUsage::kStorageWrite, ResourceUsage::kStorageRead };
	auto viewBuffer = device.CreateBuffer( viewDesc );
	auto pyramid = device.CreateBuffer( pyramidDesc );
	if ( viewBuffer )
		s.retiredBuffers.emplace_back( target.frame, viewBuffer.Value() );
	if ( pyramid )
		s.retiredBuffers.emplace_back( target.frame, pyramid.Value() );
	if ( !viewBuffer || !pyramid )
		return false;
	encoder.BeginLabel( "core world hiz" );
	encoder.TransitionBuffer(
	    viewBuffer.Value(), ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	encoder.WriteBuffer( viewBuffer.Value(), 0, std::as_bytes( std::span( &occlusionView, 1 ) ) );
	encoder.TransitionBuffer(
	    viewBuffer.Value(), ResourceUsage::kCopyDestination, ResourceUsage::kStorageRead );
	encoder.TransitionBuffer(
	    pyramid.Value(), ResourceUsage::kUndefined, ResourceUsage::kStorageWrite );
	encoder.TransitionTexture(
	    r.prepassDepth, ResourceUsage::kDepthWrite, ResourceUsage::kSampled );
	const bool recorded =
	    s.gpuOcclusion
	        ->RecordPyramid( encoder,
	            { r.prepassDepth, s.gpuPointSampler, viewBuffer.Value(), pyramid.Value() },
	            occlusionView )
	        .HasValue();
	encoder.TransitionTexture(
	    r.prepassDepth, ResourceUsage::kSampled, ResourceUsage::kDepthWrite );
	encoder.TransitionBuffer(
	    pyramid.Value(), ResourceUsage::kStorageWrite, ResourceUsage::kStorageRead );
	encoder.EndLabel();
	for ( BindGroupId group : s.gpuOcclusion->TakeRecorded() )
		s.retiredBindGroups.emplace_back( target.frame, group );
	if ( !recorded )
		return false;
	s.gpuPyramid = { pyramid.Value(), occlusionView, target.frame, r.prepassDepth, screenInputs };
	++s.stats.gpuPyramids;
	return true;
}

// The dispatches of GPU-driven submission (RFC 0016 S3/S4), outside a
// rendering: cull `instances` against the view (and this view's pyramid
// when `occlude`), then compact the kept ones' `templates` into one
// command list per bucket (counts as words, culling::CommandsOffset).
// The command buffer, invalid when the kernels or buffers were refused.
BufferId WorldPass::Batch::gpuCompactCommands( const std::vector<culling::CullInstance> &instances,
    const std::vector<culling::DrawTemplate> &templates,
    const std::vector<culling::DrawBucket> &buckets, bool occlude )
{
	using namespace render::culling;
	if ( !s.gpuCull )
	{
		auto cull = CullKernel::Create( device );
		auto compact = CompactKernel::Create( device );
		if ( cull && compact )
		{
			s.gpuCull = std::move( cull ).Value();
			s.gpuCompact = std::move( compact ).Value();
		}
	}
	const auto count = static_cast<std::uint32_t>( instances.size() );
	const auto bucketCount = static_cast<std::uint32_t>( buckets.size() );
	BufferId result;
	const math::float4x4 toClip = viewToClip();
	const CullView cullView = PackView( math::ExtractFrustum( toClip ), 32, count );
	std::vector<BufferId> made;
	auto make = [&]( std::uint64_t size, UsageSet usages )
	{
		BufferDesc desc;
		desc.size = std::max<std::uint64_t>( size, 16 );
		desc.usages = usages;
		auto created = device.CreateBuffer( desc );
		made.push_back( created ? created.Value() : BufferId() );
		return made.back();
	};
	const UsageSet in = { ResourceUsage::kCopyDestination, ResourceUsage::kStorageRead };
	const BufferId instanceBuffer = make( count * sizeof( CullInstance ), in );
	const BufferId viewBuffer = make( sizeof( CullView ), in );
	const BufferId templateBuffer = make( count * sizeof( DrawTemplate ), in );
	const BufferId bucketBuffer = make( bucketCount * sizeof( DrawBucket ), in );
	const BufferId mask = make( std::uint64_t( MaskWords( count ) ) * 4,
	    { ResourceUsage::kStorageWrite, ResourceUsage::kStorageRead } );
	const BufferId commands = make( CommandBufferBytes( count, bucketCount ),
	    { ResourceUsage::kStorageWrite, ResourceUsage::kIndirect } );
	OcclusionView occlusionView;
	BufferId occlusionViewBuffer;
	BufferId visibility;
	if ( occlude )
	{
		occlusionView = s.gpuPyramid.view;
		occlusionView.count = count;
		occlusionViewBuffer = make( sizeof( OcclusionView ), in );
		visibility = make( std::uint64_t( MaskWords( count ) ) * 4,
		    { ResourceUsage::kStorageWrite, ResourceUsage::kStorageRead } );
	}
	const bool ready = s.gpuCull && std::all_of( made.begin(), made.end(),
	                                    []( BufferId id )
	                                    {
		                                    return id.IsValid();
	                                    } );
	if ( ready )
	{
		encoder.BeginLabel( "core world gpu cull" );
		auto upload = [&]( BufferId buffer, std::span<const std::byte> bytes )
		{
			encoder.TransitionBuffer(
			    buffer, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
			encoder.WriteBuffer( buffer, 0, bytes );
			encoder.TransitionBuffer(
			    buffer, ResourceUsage::kCopyDestination, ResourceUsage::kStorageRead );
		};
		upload( instanceBuffer, std::as_bytes( std::span( instances ) ) );
		upload( viewBuffer, std::as_bytes( std::span( &cullView, 1 ) ) );
		upload( templateBuffer, std::as_bytes( std::span( templates ) ) );
		upload( bucketBuffer, std::as_bytes( std::span( buckets ) ) );
		encoder.TransitionBuffer( mask, ResourceUsage::kUndefined, ResourceUsage::kStorageWrite );
		const bool culled =
		    s.gpuCull->Record( encoder, { instanceBuffer, viewBuffer, mask, count } ).HasValue();
		encoder.TransitionBuffer( mask, ResourceUsage::kStorageWrite, ResourceUsage::kStorageRead );
		BufferId kept = mask;
		bool occluded = true;
		if ( occlude )
		{
			upload( occlusionViewBuffer, std::as_bytes( std::span( &occlusionView, 1 ) ) );
			encoder.TransitionBuffer(
			    visibility, ResourceUsage::kUndefined, ResourceUsage::kStorageWrite );
			occluded = s.gpuOcclusion
			               ->RecordOcclusion(
			                   encoder, { instanceBuffer, occlusionViewBuffer, s.gpuPyramid.pyramid,
			                                mask, visibility, count } )
			               .HasValue();
			encoder.TransitionBuffer(
			    visibility, ResourceUsage::kStorageWrite, ResourceUsage::kStorageRead );
			for ( BindGroupId group : s.gpuOcclusion->TakeRecorded() )
				s.retiredBindGroups.emplace_back( target.frame, group );
			kept = visibility;
		}
		encoder.TransitionBuffer(
		    commands, ResourceUsage::kUndefined, ResourceUsage::kStorageWrite );
		const bool compacted =
		    occluded && s.gpuCompact
		                    ->Record( encoder, { kept, templateBuffer, viewBuffer, bucketBuffer,
		                                           commands, count, bucketCount } )
		                    .HasValue();
		encoder.TransitionBuffer(
		    commands, ResourceUsage::kStorageWrite, ResourceUsage::kIndirect );
		encoder.EndLabel();
		for ( BindGroupId group : s.gpuCull->TakeRecorded() )
			s.retiredBindGroups.emplace_back( target.frame, group );
		for ( BindGroupId group : s.gpuCompact->TakeRecorded() )
			s.retiredBindGroups.emplace_back( target.frame, group );
		if ( culled && compacted )
			result = commands;
	}
	for ( BufferId id : made )
		if ( id.IsValid() )
			s.retiredBuffers.emplace_back( target.frame, id );
	return result;
}

WorldPass::Batch::GpuList WorldPass::Batch::gpuCullList( const std::vector<std::uint32_t> &list,
    const std::vector<Resources::Material> &materials, bool occlude )
{
	GpuList out;
	if ( list.empty() || list.size() > culling::kMaxCompactInstances )
		return out;
	using namespace render::culling;
	if ( s.gpuBoundsWorld != world.get() )
	{
		s.gpuBounds.assign( world->surfaces.size(), {} );
		for ( std::size_t i = 0; i < world->surfaces.size(); ++i )
		{
			const WorldSurface &surface = world->surfaces[i];
			CullInstance &bounds = s.gpuBounds[i];
			bounds.viewMask = ~0u;
			std::fill_n( bounds.min, 3, 3.4e38f );
			std::fill_n( bounds.max, 3, -3.4e38f );
			for ( std::uint32_t k = 0; k < surface.indexCount; ++k )
			{
				const std::uint32_t at = surface.firstIndex + k;
				if ( at >= world->indices.size() || world->indices[at] >= world->vertices.size() )
					continue;
				const float *p = world->vertices[world->indices[at]].position;
				for ( int c = 0; c < 3; ++c )
				{
					bounds.min[c] = std::min( bounds.min[c], p[c] );
					bounds.max[c] = std::max( bounds.max[c], p[c] );
				}
			}
		}
		s.gpuBoundsWorld = world.get();
	}
	const auto count = static_cast<std::uint32_t>( list.size() );
	std::vector<CullInstance> instances( count );
	std::vector<DrawTemplate> templates( count );
	std::vector<DrawBucket> buckets;
	for ( std::uint32_t k = 0; k < count; ++k )
	{
		const WorldSurface &surface = world->surfaces[list[k]];
		const Resources::Material &m = materials[surface.material];
		const int page = m.program.request.drawLayout.IsValid() ? surface.lightmapPage : 0;
		if ( out.buckets.empty() || out.buckets.back().material != surface.material ||
		     out.buckets.back().page != page )
		{
			out.buckets.push_back( { surface.material, page, k, 0 } );
			buckets.push_back( { k, 0 } );
		}
		++out.buckets.back().count;
		++buckets.back().count;
		instances[k] = s.gpuBounds[list[k]];
		if ( occlude )
		{
			// A surface facing the camera rasterizes at its box's nearest
			// depth up to rounding: one unit of margin keeps it from
			// occluding itself. Only opaque PBR surfaces are tested.
			for ( int c = 0; c < 3; ++c )
			{
				instances[k].min[c] -= 1.0f;
				instances[k].max[c] += 1.0f;
			}
			instances[k].flags = opaquePbr( m ) ? 0u : kCullNeverOcclude;
		}
		templates[k] = { surface.indexCount, surface.firstIndex, 0,
		    static_cast<std::uint32_t>( buckets.size() - 1 ) };
	}
	out.commands = gpuCompactCommands( instances, templates, buckets, occlude );
	if ( !out.commands.IsValid() )
		out.buckets.clear();
	return out;
}

WorldPass::Batch::GpuModels WorldPass::Batch::gpuCullModels( const std::vector<StaticDraw> &list,
    const std::vector<Resources::Material> &materials, bool occlude, bool viewDebug )
{
	using namespace render::culling;
	GpuModels out;
	if ( !target.gpuSubmission || !gpuDeviceCapable ||
	     !gpuCaps.Has( Capability::kIndirectFirstInstance ) || target.motion.IsValid() ||
	     !modelFootprintBounds )
		return out;
	out.taken.assign( list.size(), false );
	for ( std::size_t i = 0; i < list.size(); ++i )
	{
		const StaticDraw &draw = list[i];
		const Resources::Material &m = materials[draw.material];
		if ( draw.posed || m.program.blend != BlendMode::kOpaque || m.program.sceneColor ||
		     ( viewDebug &&
		         !frame::DebugSpecializationFor( view.debug, m.program.name ).IsNeutral() ) )
			continue;
		const WorldData::StaticMesh &mesh = world->staticMeshes[draw.mesh];
		if ( draw.mesh >= modelFootprintBounds->size() ||
		     draw.surface >= ( *modelFootprintBounds )[draw.mesh].size() ||
		     draw.surface >= mesh.surfaceLods.size() ||
		     mesh.surfaceLods[draw.surface] != draw.lod ||
		     !( *modelFootprintBounds )[draw.mesh][draw.surface].valid )
			continue;
		out.taken[i] = true;
		out.draws.push_back( draw );
	}
	std::sort( out.draws.begin(), out.draws.end(),
	    []( const StaticDraw &a, const StaticDraw &b )
	    {
		    return std::tie( a.cohort, a.material, a.mesh, a.posed, a.instance, a.surface ) <
		           std::tie( b.cohort, b.material, b.mesh, b.posed, b.instance, b.surface );
	    } );
	const auto count = static_cast<std::uint32_t>( out.draws.size() );
	std::vector<CullInstance> instances( count );
	std::vector<DrawTemplate> templates( count );
	std::vector<DrawBucket> buckets;
	std::vector<material::FamilyDrawConstants> records( count );
	for ( std::uint32_t k = 0; k < count; ++k )
	{
		const StaticDraw &draw = out.draws[k];
		if ( out.buckets.empty() || out.buckets.back().material != draw.material ||
		     out.buckets.back().mesh != draw.mesh || out.buckets.back().lod != draw.lod )
		{
			out.buckets.push_back( { draw.material, draw.mesh, draw.lod, k, 0 } );
			buckets.push_back( { k, 0 } );
		}
		++out.buckets.back().count;
		++buckets.back().count;
		records[k] = modelDrawConstants( draw );
		// The surface's object box in world space: centre and extents
		// through object-to-world (row-major, column vectors).
		const SurfaceFootprintBounds &box = ( *modelFootprintBounds )[draw.mesh][draw.surface];
		const float *w = records[k].world;
		const float margin = occlude ? 1.0f : 0.0f;
		CullInstance &instance = instances[k];
		instance.viewMask = ~0u;
		for ( int i = 0; i < 3; ++i )
		{
			float centre = w[i * 4 + 3], extent = 0.0f;
			for ( int j = 0; j < 3; ++j )
			{
				centre += w[i * 4 + j] * 0.5f * ( box.lo[j] + box.hi[j] );
				extent += std::fabs( w[i * 4 + j] ) * 0.5f * ( box.hi[j] - box.lo[j] );
			}
			instance.min[i] = centre - extent - margin;
			instance.max[i] = centre + extent + margin;
		}
		instance.flags = !occlude || opaquePbr( materials[draw.material] ) ? 0u : kCullNeverOcclude;
		const WorldSurface &surface = world->staticMeshes[draw.mesh].surfaces[draw.surface];
		templates[k] = { surface.indexCount, surface.firstIndex, 0,
		    static_cast<std::uint32_t>( buckets.size() - 1 ) };
	}
	if ( count != 0 && count <= kMaxCompactInstances )
	{
		BufferDesc recordDesc;
		recordDesc.size = std::uint64_t( count ) * sizeof( material::FamilyDrawConstants );
		recordDesc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kVertex };
		auto recordBuffer = device.CreateBuffer( recordDesc );
		if ( recordBuffer )
		{
			s.retiredBuffers.emplace_back( target.frame, recordBuffer.Value() );
			encoder.TransitionBuffer(
			    recordBuffer.Value(), ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
			encoder.WriteBuffer( recordBuffer.Value(), 0, std::as_bytes( std::span( records ) ) );
			encoder.TransitionBuffer(
			    recordBuffer.Value(), ResourceUsage::kCopyDestination, ResourceUsage::kVertex );
			out.commands = gpuCompactCommands( instances, templates, buckets, occlude );
			out.records = recordBuffer.Value();
		}
	}
	if ( !out.commands.IsValid() )
		return GpuModels(); // refused: every draw goes back to the per-draw loops
	return out;
}

// One indirect draw per bucket of `models` whose material `skip` does not
// name, bound as recordModel binds its first draw, with the instanced
// point of `pipelineOf`'s pipeline. The draws it covers (submitted; the
// GPU culls some).
std::uint64_t WorldPass::Batch::drawModelBuckets( const GpuModels &models,
    const std::vector<Resources::Material> &materials,
    const std::function<std::optional<PipelineId>( const Resources::Material & )> &pipelineOf,
    const std::function<bool( const Resources::Material & )> &skip )
{
	std::uint64_t covered = 0;
	const auto bucketCount = static_cast<std::uint32_t>( models.buckets.size() );
	for ( std::uint32_t b = 0; b < bucketCount; ++b )
	{
		const GpuModelBucket &bucket = models.buckets[b];
		const Resources::Material &m = materials[bucket.material];
		if ( skip( m ) )
			continue;
		const std::optional<PipelineId> base = pipelineOf( m );
		if ( !base )
		{
			complete = false;
			continue;
		}
		auto instanced = m.resolver->Program().InstancedPipeline( *base );
		if ( !instanced )
		{
			note( "the instanced point was refused for " + m.program.name + ": " +
			      m.resolver->Program().PipelineFailure() );
			complete = false;
			continue;
		}
		bindModel( models.draws[bucket.first], m, instanced.Value(), models.records );
		for ( std::uint32_t k = bucket.first; k < bucket.first + bucket.count; ++k )
			modelFeedback( models.draws[k] );
		encoder.DrawIndexedIndirectCount( models.commands,
		    culling::CommandsOffset( bucketCount ) +
		        std::uint64_t( bucket.first ) * sizeof( DrawIndexedIndirectCommand ),
		    models.commands, std::uint64_t( b ) * 4, bucket.count,
		    sizeof( DrawIndexedIndirectCommand ) );
		++s.stats.gpuIndirectDraws;
		covered += bucket.count;
	}
	return covered;
}

// One indirect draw per bucket of `gpu`, bound as drawSurfaces binds the
// same run; buckets whose material `skip` names are left out.
void WorldPass::Batch::gpuDrawList( const GpuList &gpu, const std::vector<std::uint32_t> &list,
    const std::vector<Resources::Material> &materials,
    const std::function<std::optional<PipelineId>( const Resources::Material & )> &pipelineOf,
    const std::function<bool( const Resources::Material & )> &skip )
{
	const auto bucketCount = static_cast<std::uint32_t>( gpu.buckets.size() );
	for ( std::uint32_t b = 0; b < bucketCount; ++b )
	{
		const GpuBucket &bucket = gpu.buckets[b];
		const Resources::Material &m = materials[bucket.material];
		if ( skip && skip( m ) )
			continue;
		const std::optional<PipelineId> pipeline = pipelineOf( m );
		if ( !pipeline )
		{
			complete = false;
			continue;
		}
		bindSurfaceMaterial( m, *pipeline );
		if ( m.program.request.drawLayout.IsValid() )
			encoder.SetBindGroup(
			    BindGroupRole::kDraw, r.drawGroups[drawKey( m, bucket.page )].group );
		for ( std::uint32_t k = bucket.first; k < bucket.first + bucket.count; ++k )
		{
			const std::uint32_t index = list[k];
			submitSurfaceFootprints( world->surfaces[index], materials, world->vertices,
			    world->indices, worldFootprints.empty() ? scratchFootprint : worldFootprints[index],
			    nullptr,
			    footprintGeometry && index < footprintGeometry->size()
			        ? &( *footprintGeometry )[index]
			        : nullptr );
		}
		encoder.DrawIndexedIndirectCount( gpu.commands,
		    culling::CommandsOffset( bucketCount ) +
		        std::uint64_t( bucket.first ) * sizeof( DrawIndexedIndirectCommand ),
		    gpu.commands, std::uint64_t( b ) * 4, bucket.count,
		    sizeof( DrawIndexedIndirectCommand ) );
		++s.stats.gpuIndirectDraws;
	}
}

bool WorldPass::Batch::PrepareDrawHelpers()
{
	if ( target.mipFeedback && world )
		worldFootprints.resize( world->surfaces.size() );
	preparation.End();
	return true;
}

bool WorldPass::Batch::PrepareGpuSubmission()
{
	std::copy_n( view.toClip, 16, screenInputs.begin() );
	gpuDeviceCapable = gpuCaps.Has( Capability::kCompute ) &&
	                   gpuCaps.Has( Capability::kStorageBuffers ) &&
	                   gpuCaps.Has( Capability::kDrawIndirectCount );
	const auto &viewStencil = target.drawState.stencil;
	const CompareOp viewCompare = target.drawState.depthCompare;
	occlusionWanted =
	    target.gpuSubmission && target.gpuOcclusion && gpuDeviceCapable && world->stage &&
	    view.viewport.x == 0.0f && view.viewport.y == 0.0f &&
	    view.viewport.width == float( target.width ) &&
	    view.viewport.height == float( target.height ) &&
	    ( !target.drawState.overrideDepth ||
	        ( target.drawState.depthTest &&
	            ( viewCompare == CompareOp::kLess || viewCompare == CompareOp::kLessEqual ||
	                viewCompare == CompareOp::kEqual ) ) ) &&
	    ( !viewStencil.enabled || viewStencil.writeMask == 0 ||
	        viewStencil.depthFail == StencilOp::kKeep );
	return true;
}

} // namespace render::pass::world
