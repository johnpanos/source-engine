//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.world (RFC 0016 K5): a batch's surface order, model draws and view constants.
//
//=============================================================================//

#include "world_batch.h"

namespace render::pass::world
{

bool WorldPass::Batch::PrepareSurfacesAndModels()
{
	order.reserve( view.surfaces.size() );
	for ( const std::uint32_t index : view.surfaces )
	{
		if ( index >= world->surfaces.size() )
		{
			note( "a view named a surface the world does not have" );
			complete = false;
			continue;
		}
		const WorldSurface &surface = world->surfaces[index];
		const Resources::Material *m =
		    surface.material < claims->size() && ( *claims )[surface.material].draws
		        ? materialReady( surface.material )
		        : nullptr;
		if ( !m && failure.empty() && !lastMaterialPending )
			note( "world surface " + std::to_string( index ) + " names unclaimed material " +
			      std::to_string( surface.material ) );
		if ( !m ||
		     ( m->program.request.drawLayout.IsValid() &&
		         !drawGroupReady( *m, surface.lightmapPage ) ) ||
		     ( m->program.request.frameLayout.IsValid() && !frameGroupReady( *m ) ) )
		{
			complete = false;
			continue;
		}
		order.push_back( index );
	}
	std::sort( order.begin(), order.end(),
	    [&]( std::uint32_t a, std::uint32_t b )
	    {
		    const WorldSurface &x = world->surfaces[a];
		    const WorldSurface &y = world->surfaces[b];
		    if ( x.material != y.material )
			    return x.material < y.material;
		    return x.lightmapPage != y.lightmapPage ? x.lightmapPage < y.lightmapPage
		                                            : x.firstIndex < y.firstIndex;
	    } );
	preparation.Select( "prepare model materials and meshes" );
	// Model geometry residency: adopt the world's levels, then release the ones
	// no view has selected for a while (once per recorded frame).
	pass.SweepModelResidency( s, *world, target );
	posedBuffers = std::vector<BufferId>( view.posedModels.size() );
	previousPosedBuffers = std::vector<BufferId>( view.posedModels.size() );
	// One (model, level)'s buffers for this recording: uploaded from the
	// world's staging on first use, and again after the residency rule released
	// them. Null when the level has no geometry left to upload.
	auto modelLevel = [&]( std::uint32_t meshId, std::uint32_t lod ) -> const State::ModelLevel *
	{
		if ( meshId >= world->staticMeshes.size() || lod >= world->staticMeshes[meshId].lodCount() )
		{
			note( "a model names a level it does not have" );
			return nullptr;
		}
		const WorldData::StaticMesh &mesh = world->staticMeshes[meshId];
		if ( !pass.UploadModelLevel( s, device, encoder, mesh, meshId, lod, target.frame ) )
		{
			note( "model " + std::to_string( meshId ) + " level " + std::to_string( lod ) +
			      " has no geometry to upload" );
			return nullptr;
		}
		return &s.models.models[meshId][lod];
	};
	for ( std::size_t staticIndex = 0; staticIndex < view.staticInstances.size(); ++staticIndex )
	{
		const auto &draw = view.staticInstances[staticIndex];
		const std::uint32_t instanceId = draw.instance;
		if ( instanceId >= world->staticInstances.size() )
		{
			note( "a view named a static model instance the world does not have" );
			complete = false;
			continue;
		}
		const WorldData::StaticInstance &instance = world->staticInstances[instanceId];
		if ( instance.mesh >= world->staticMeshes.size() )
		{
			note( "a static model instance names no mesh" );
			complete = false;
			continue;
		}
		const WorldData::StaticMesh &mesh = world->staticMeshes[instance.mesh];
		if ( !ValidSurfaceSelection( draw.surfaceSelection, mesh.surfaces.size() ) )
		{
			note( "a static model has an invalid surface selection" );
			complete = false;
			continue;
		}
		if ( draw.surfaceSelection && draw.surfaceSelection->empty() )
			continue;
		for ( std::uint32_t surfaceId = 0; surfaceId < mesh.surfaces.size(); ++surfaceId )
		{
			if ( !SurfaceSelected( draw.surfaceSelection, surfaceId ) )
				continue;
			const std::uint32_t materialId = StaticMaterial( mesh, instance, surfaceId );
			const Resources::Material *material =
			    materialId < claims->size() && ( *claims )[materialId].draws
			        ? materialReadyIn( *r.modelResolver, r.modelMaterials, materialId )
			        : nullptr;
			if ( !material && failure.empty() && !lastMaterialPending )
				note( "static model " + std::to_string( instance.mesh ) + " surface " +
				      std::to_string( surfaceId ) + " names unclaimed material " +
				      std::to_string( materialId ) );
			if ( !material ||
			     ( material->program.request.drawLayout.IsValid() &&
			         !drawGroupReady( *material, 0 ) ) ||
			     ( material->program.request.frameLayout.IsValid() &&
			         !frameGroupReady( *material ) ) )
			{
				complete = false;
				continue;
			}
			// Each surface's level is its own allocation; the first surface
			// that needs one uploads it and the rest of the level reuses it.
			const std::uint32_t lod = mesh.LodOfSurface( surfaceId );
			if ( lod == ~0u || !modelLevel( instance.mesh, lod ) )
			{
				complete = false;
				continue;
			}
			staticDraws.push_back( { staticCohorts[staticIndex], false, instanceId, instance.mesh,
			    lod, surfaceId, materialId } );
		}
	}
	for ( std::uint32_t poseId = 0; poseId < view.posedModels.size(); ++poseId )
	{
		const WorldView::PosedModel &pose = view.posedModels[poseId];
		if ( pose.mesh >= world->staticMeshes.size() )
		{
			note( "a posed model names no mesh" );
			complete = false;
			continue;
		}
		const WorldData::StaticMesh &mesh = world->staticMeshes[pose.mesh];
		if ( pose.surfaceSelection && pose.surfaceSelection->empty() )
			continue;
		// A posed model draws one hardware level: the selection names that
		// level's surfaces, its index buffer is that level's own, and the host's
		// vertices are the pose of that level.
		WorldData::StaticInstance instance;
		instance.mesh = pose.mesh;
		instance.skin = pose.skin;
		std::uint32_t level = ~0u;
		bool levelRefused = false;
		for ( std::uint32_t surfaceId = 0; surfaceId < mesh.surfaces.size(); ++surfaceId )
		{
			if ( !SurfaceSelected( pose.surfaceSelection, surfaceId ) )
				continue;
			const std::uint32_t lod = mesh.LodOfSurface( surfaceId );
			if ( lod == ~0u || !modelLevel( pose.mesh, lod ) )
			{
				levelRefused = true;
				continue;
			}
			if ( level != ~0u && level != lod )
			{
				note( "a posed model's selection spans two hardware levels" );
				complete = false;
				levelRefused = true;
				break;
			}
			if ( pose.vertices.size() != mesh.lods[lod].vertexCount )
			{
				note( "a posed model's vertices are not its selected level's" );
				complete = false;
				levelRefused = true;
				break;
			}
			level = lod;
			const std::uint32_t materialId = StaticMaterial( mesh, instance, surfaceId );
			if ( materialId >= claims->size() )
			{
				note( "posed model " + std::to_string( pose.mesh ) + " surface " +
				      std::to_string( surfaceId ) + " names missing material " +
				      std::to_string( materialId ) );
				complete = false;
				continue;
			}
			const bool blended = ( *claims )[materialId].blended;
			if ( ( pose.phase == RenderCoreDrawPhase::kOpaque && blended ) ||
			     ( pose.phase == RenderCoreDrawPhase::kBlended && !blended ) )
				continue;
			const Resources::Material *material =
			    materialId < claims->size() && ( *claims )[materialId].draws
			        ? materialReadyIn( *r.modelResolver, r.modelMaterials, materialId )
			        : nullptr;
			if ( !material && failure.empty() && !lastMaterialPending )
				note( "posed model " + std::to_string( pose.mesh ) + " surface " +
				      std::to_string( surfaceId ) + " names unclaimed material " +
				      std::to_string( materialId ) );
			if ( !material ||
			     ( material->program.request.drawLayout.IsValid() &&
			         !drawGroupReady( *material, 0 ) ) ||
			     ( material->program.request.frameLayout.IsValid() &&
			         !frameGroupReady( *material ) ) )
			{
				complete = false;
				continue;
			}
			staticDraws.push_back(
			    { posedCohorts[poseId], true, poseId, pose.mesh, lod, surfaceId, materialId } );
		}
		if ( levelRefused || level == ~0u )
			continue;
		// The pose's own vertices, in the level's vertex order, uploaded for
		// this frame: the host's pose changes every frame, so this buffer is
		// per frame while the level's index buffer is resident.
		const auto bytes = std::as_bytes( std::span( pose.vertices ) );
		BufferDesc desc;
		desc.size = bytes.size();
		desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kVertex };
		desc.debugName = "posed model vertices";
		auto buffer = device.CreateBuffer( desc );
		if ( !buffer )
		{
			note( "a posed model's vertex buffer was refused" );
			complete = false;
			continue;
		}
		posedBuffers[poseId] = buffer.Value();
		s.retiredBuffers.emplace_back( target.frame, buffer.Value() );
		if ( target.motion.IsValid() )
		{
			if ( pose.previousVertices.size() != pose.vertices.size() )
			{
				note( "temporal model has no previous correspondence buffer" );
				complete = false;
				continue;
			}
			auto previous = device.CreateBuffer( desc );
			if ( !previous )
			{
				complete = false;
				continue;
			}
			previousPosedBuffers[poseId] = previous.Value();
			encoder.TransitionBuffer(
			    previous.Value(), ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
			encoder.WriteBuffer(
			    previous.Value(), 0, std::as_bytes( std::span( pose.previousVertices ) ) );
			encoder.TransitionBuffer(
			    previous.Value(), ResourceUsage::kCopyDestination, ResourceUsage::kVertex );
			s.retiredBuffers.emplace_back( target.frame, previous.Value() );
		}

		encoder.TransitionBuffer(
		    buffer.Value(), ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		encoder.WriteBuffer( buffer.Value(), 0, bytes );
		encoder.TransitionBuffer(
		    buffer.Value(), ResourceUsage::kCopyDestination, ResourceUsage::kVertex );
	}
	pass.PublishModelResidency( s );
	preparation.Select( "prepare world view" );
	// The view's planar reflection: the render target the view's programs
	// name (one per view: the client draws one reflection view), imported
	// now, as the stream drew it before this slot (a resize replaces its
	// image, so it is not kept across views).
	int reflectionHandle = 0;
	for ( const std::uint32_t index : order )
	{
		const Resources::Material &m = r.materials[world->surfaces[index].material];
		if ( m.viewInput == 0 || m.viewInput == reflectionHandle )
			continue;
		if ( reflectionHandle != 0 )
			note( "the view's programs read two planar reflections" );
		else
			reflectionHandle = m.viewInput;
	}
	if ( reflectionHandle != 0 )
		viewReflection = textures.Import( reflectionHandle, true );
	int refractionHandle = 0;
	for ( const std::uint32_t index : order )
	{
		const Resources::Material &m = r.materials[world->surfaces[index].material];
		if ( m.refractInput == 0 || m.refractInput == refractionHandle )
			continue;
		if ( refractionHandle != 0 )
			note( "the view's programs read two water refractions" );
		else
			refractionHandle = m.refractInput;
	}
	if ( refractionHandle != 0 )
		viewRefraction = textures.Import( refractionHandle, true );
	std::memcpy( constants.toClip, view.toClip, sizeof( constants.toClip ) );
	if ( view.viewport.width > 0.0f && view.viewport.height > 0.0f )
	{
		for ( int c = 0; c < 4; ++c )
		{
			constants.toClip[0 * 4 + c] += view.toClip[3 * 4 + c] / view.viewport.width;
			constants.toClip[1 * 4 + c] -= view.toClip[3 * 4 + c] / view.viewport.height;
		}
	}
	// The world is in world space: its object-to-world is the identity.
	for ( int i = 0; i < 4; ++i )
		constants.world[i * 5] = 1.0f;
	constantBytes = std::as_bytes( std::span( &constants, 1 ) );
	// The water point's draws with the view's water plane moved (the
	// client's waterZAdjust): world-to-clip after a translation along z.
	material::FamilyDrawConstants waterConstants = constants;
	for ( int row = 0; row < 4; ++row )
		waterConstants.toClip[row * 4 + 3] += constants.toClip[row * 4 + 2] * view.waterZOffset;
	waterConstantBytes = std::as_bytes( std::span( &waterConstants, 1 ) );
	return true;
}

} // namespace render::pass::world
