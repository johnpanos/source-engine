//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.world (RFC 0016 K5): model geometry residency.
//
//=============================================================================//

#include "world_pass_internal.h"

namespace render::pass::world
{

void WorldPass::SetModelLevelSource( IModelLevelSource *source )
{
	m_State->levelSource = source;
}

std::vector<std::pair<std::uint32_t, std::uint32_t>> WorldPass::DrainReleasedStaging()
{
	State &s = *m_State;
	std::vector<std::pair<std::uint32_t, std::uint32_t>> released;
	for ( std::uint32_t m = 0; m < s.models.models.size(); ++m )
	{
		auto &model = s.models.models[m];
		for ( std::uint32_t l = 0; l < model.size(); ++l )
		{
			if ( model[l].stagingReleased )
			{
				released.emplace_back( m, l );
				model[l].stagingReleased = false;
			}
		}
	}
	return released;
}

// Render sequence: one (model, level)'s buffers, uploaded from the world's
// staging on first use and again after a release. When a model level source
// is set, the staging shared_ptrs are released after the first upload and
// the source resupplies the bytes for a re-upload. False when the level has
// no geometry left to upload (a named failure; the caller draws nothing).
bool WorldPass::UploadModelLevel( State &state, device::IRenderDevice2 &device,
    device::CommandEncoder &encoder, const WorldData::StaticMesh &mesh, std::uint32_t meshId,
    std::uint32_t lod, std::uint64_t frame )
{
	if ( meshId >= state.models.models.size() || lod >= mesh.lodCount() )
		return false;
	const WorldData::StaticMeshLod &source = mesh.lods[lod];
	if ( !source.Drawable() )
		return false;
	State::ModelLevel &level = state.models.models[meshId][lod];
	level.lastUsedFrame = frame;
	if ( level.resident )
		return true;
	// The staging shared_ptrs: present on first upload, absent after the
	// composition released them (stagingReleased). A re-upload without
	// staging asks the source; without a source either, the level stays out.
	std::optional<IModelLevelSource::LevelGeometry> resupplied;
	std::span<const std::byte> vertexBytes;
	std::span<const std::byte> indexBytes;
	if ( source.vertices && source.indices )
	{
		vertexBytes = std::as_bytes( std::span( *source.vertices ) );
		indexBytes = std::as_bytes( std::span( *source.indices ) );
	}
	else if ( state.levelSource )
	{
		resupplied = state.levelSource->ResupplyLevel( meshId, lod );
		if ( !resupplied || resupplied->vertices.size() != source.vertexCount ||
		     resupplied->indices.size() != source.indexCount )
			return false;
		vertexBytes = std::as_bytes( std::span( resupplied->vertices ) );
		indexBytes = std::as_bytes( std::span( resupplied->indices ) );
	}
	else
	{
		return false;
	}
	if ( vertexBytes.size() !=
	         std::size_t( source.vertexCount ) * sizeof( material::SurfaceModelVertex ) ||
	     indexBytes.size() != std::size_t( source.indexCount ) * sizeof( std::uint32_t ) )
		return false;
	BufferDesc desc;
	desc.size = vertexBytes.size();
	desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kVertex };
	desc.debugName = "model level vertices";
	auto vertices = device.CreateBuffer( desc );
	desc.size = indexBytes.size();
	desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kIndex };
	desc.debugName = "model level indices";
	auto indices = device.CreateBuffer( desc );
	if ( !vertices || !indices )
	{
		// A refused allocation keeps the level non-resident, so the next
		// frame that draws it tries again with no bytes dropped.
		if ( vertices )
			(void)device.Release( vertices.Value(), CompletionToken() );
		if ( indices )
			(void)device.Release( indices.Value(), CompletionToken() );
		return false;
	}
	level.vertices = vertices.Value();
	level.indices = indices.Value();
	level.resident = true;
	// The screen passes' prepass lists name the resident levels.
	++state.modelResidencyRevision;
	state.models.bytes += vertexBytes.size() + indexBytes.size();
	++state.stats.modelLevelUploads;
	encoder.TransitionBuffer(
	    level.vertices, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	encoder.WriteBuffer( level.vertices, 0, vertexBytes );
	encoder.TransitionBuffer(
	    level.vertices, ResourceUsage::kCopyDestination, ResourceUsage::kVertex );
	encoder.TransitionBuffer(
	    level.indices, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	encoder.WriteBuffer( level.indices, 0, indexBytes );
	encoder.TransitionBuffer(
	    level.indices, ResourceUsage::kCopyDestination, ResourceUsage::kIndex );
	// Mark the staging as releasable: the composition releases its own copies
	// on the next world publication. The source resupplies bytes for any
	// re-upload after that.
	if ( state.levelSource && !ModelLevelPinned( mesh, lod ) )
		level.stagingReleased = true;
	return true;
}

// Render sequence: adopt the world's model geometry (its (model, level)
// allocations survive a target format change and a stage republication), then
// release the levels no view has selected for kModelLevelIdleFrames, once per
// recorded frame.
void WorldPass::SweepModelResidency(
    State &state, const WorldData &world, const WorldTarget &target )
{
	// Reuse the resident levels only for a world that declares the same model
	// geometry revision: revision zero is "unversioned", so such a world (a lab
	// scene, a test fixture) uploads its own levels rather than inheriting
	// another world's buffers.
	bool sameGeometry = world.modelsRevision != 0 &&
	                    state.models.modelsRevision == world.modelsRevision &&
	                    state.models.models.size() == world.staticMeshes.size();
	for ( std::size_t mesh = 0; sameGeometry && mesh < world.staticMeshes.size(); ++mesh )
		sameGeometry = state.models.models[mesh].size() == world.staticMeshes[mesh].lodCount();
	if ( !sameGeometry )
	{
		// Another world's models: every level's buffers go with it, released at
		// a slot whose submitted token covers the frames that read them.
		for ( auto &model : state.models.models )
			for ( State::ModelLevel &level : model )
			{
				if ( level.vertices.IsValid() )
					state.retiredModelBuffers.push_back( { 0, level.vertices } );
				if ( level.indices.IsValid() )
					state.retiredModelBuffers.push_back( { 0, level.indices } );
			}
		state.models = State::ModelGeometry();
		// A new models revision replaces every level: the screen passes'
		// prepass lists are keyed by this too.
		++state.modelResidencyRevision;
		state.models.modelsRevision = world.modelsRevision;
		state.models.models.resize( world.staticMeshes.size() );
		for ( std::size_t mesh = 0; mesh < world.staticMeshes.size(); ++mesh )
			state.models.models[mesh].resize( world.staticMeshes[mesh].lodCount() );
	}
	if ( target.frame == 0 || state.residencyFrame == target.frame )
		return;
	state.residencyFrame = target.frame;
	for ( std::size_t mesh = 0; mesh < state.models.models.size(); ++mesh )
	{
		const WorldData::StaticMesh &data = world.staticMeshes[mesh];
		for ( std::size_t lod = 0; lod < state.models.models[mesh].size(); ++lod )
		{
			State::ModelLevel &level = state.models.models[mesh][lod];
			if ( !level.resident )
				continue;
			if ( !ModelLevelPinned( data, std::uint32_t( lod ) ) && level.lastUsedFrame != 0 &&
			     target.frame > level.lastUsedFrame + kModelLevelIdleFrames )
			{
				state.retiredModelBuffers.push_back( { target.frame, level.vertices } );
				state.retiredModelBuffers.push_back( { target.frame, level.indices } );
				state.models.bytes -=
				    std::size_t( data.lods[lod].vertexCount ) *
				        sizeof( material::SurfaceModelVertex ) +
				    std::size_t( data.lods[lod].indexCount ) * sizeof( std::uint32_t );
				++state.models.releasedLevels;
				// The screen passes' prepass lists name the resident levels.
				++state.modelResidencyRevision;
				level = State::ModelLevel();
			}
		}
	}
}

// Render sequence: the residency report the frame's views can read (the levels
// resident now, their bytes, and the levels released this world). Once per
// recorded frame, after this frame's uploads.
void WorldPass::PublishModelResidency( State &state )
{
	if ( state.residencyFrame == 0 || state.residencyReportedFrame == state.residencyFrame )
		return;
	state.residencyReportedFrame = state.residencyFrame;
	std::uint32_t resident = 0;
	std::uint64_t stagingBytes = 0;
	for ( std::uint32_t m = 0; m < state.models.models.size(); ++m )
	{
		const auto &model = state.models.models[m];
		for ( std::uint32_t l = 0; l < model.size(); ++l )
		{
			resident += model[l].resident ? 1u : 0u;
			if ( state.world && m < state.world->staticMeshes.size() )
			{
				const WorldData::StaticMeshLod &src = state.world->staticMeshes[m].lods[l];
				if ( src.vertices )
					stagingBytes += src.vertices->size() * sizeof( material::SurfaceModelVertex );
				if ( src.indices )
					stagingBytes += src.indices->size() * sizeof( std::uint32_t );
			}
		}
	}
	state.stats.modelLevelsResident = resident;
	state.stats.modelBufferBytes = state.models.bytes;
	state.stats.modelStagingBytes = stagingBytes;
	state.stats.modelLevelsReleased = state.models.releasedLevels > 0xfffffffeu
	                                      ? 0xfffffffeu
	                                      : std::uint32_t( state.models.releasedLevels );
}

} // namespace render::pass::world
