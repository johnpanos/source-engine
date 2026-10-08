//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.composition's world (RFC 0016 K5); see core_world.h.
//
//=============================================================================//

#include "core_world_internal.h"

namespace render::composition
{

static_assert( pass::world::kWorldTag == legacy::kCorePassForwarded,
    "world tags are the frontend's forwarded tags" );

static_assert( legacy::kCorePassLegacyHud ==
                   legacy::CorePassTag( static_cast<std::uint32_t>( frame::Stage::kHud ), 0 ),
    "the legacy UI exception is the frame catalog's top-level HUD stage" );

static_assert(
    sizeof( RenderCoreWorldVertex ) == sizeof( pass::world::WorldVertex ) &&
        offsetof( RenderCoreWorldVertex, lightmapUv ) ==
            offsetof( pass::world::WorldVertex, lightmapUv ) &&
        offsetof( RenderCoreWorldVertex, color ) == offsetof( pass::world::WorldVertex, color ) &&
        offsetof( RenderCoreWorldVertex, normal ) == offsetof( pass::world::WorldVertex, normal ) &&
        offsetof( RenderCoreWorldVertex, lightmapOffset ) ==
            offsetof( pass::world::WorldVertex, lightmapOffset ),
    "the engine's world vertex is the pass's" );

std::shared_ptr<const CoreWorld::PoseTopology> CoreWorld::PoseTopologyFor(
    std::uint32_t model, std::uint32_t lod, int body ) const
{
	const std::uint64_t key = ( std::uint64_t( model ) << 40 ) |
	                          ( std::uint64_t( lod & 0xff ) << 32 ) | std::uint32_t( body );
	{
		std::lock_guard<std::mutex> guard( m_PoseTopologyLock );
		if ( m_PoseTopologyRevision != m_ModelsRevision )
		{
			m_PoseTopology.clear();
			m_PoseTopologyRevision = m_ModelsRevision;
		}
		if ( auto found = m_PoseTopology.find( key ); found != m_PoseTopology.end() )
			return found->second;
	}
	auto topology = std::make_shared<PoseTopology>();
	const ModelPoseSource &pose = m_ModelPoseSources[model];
	const pass::world::WorldData::StaticMesh &mesh = m_StaticMeshes[model];
	topology->surfaces = pose.SelectedSurfaces( body, lod );
	const std::span<const pass::skinning::SkinVertex> skinning = pose.LevelVertices( lod );
	topology->valid = lod < mesh.lodCount() && mesh.lods[lod].indices &&
	                  skinning.size() == mesh.lods[lod].vertexCount;
	if ( topology->valid && !topology->surfaces.empty() )
	{
		const std::vector<std::uint32_t> &indices = *mesh.lods[lod].indices;
		std::vector<bool> usedVertices( skinning.size() );
		for ( std::uint32_t surfaceId : topology->surfaces )
		{
			const pass::world::WorldSurface &surface = mesh.surfaces[surfaceId];
			for ( std::uint32_t i = surface.firstIndex; i < surface.firstIndex + surface.indexCount;
			    ++i )
				usedVertices[indices[i]] = true;
		}
		std::vector<bool> usedBones( pose.poseToBone.size() );
		for ( std::uint32_t i = 0; i < skinning.size() && topology->valid; ++i )
		{
			if ( !usedVertices[i] )
				continue;
			const pass::skinning::SkinVertex &vertex = skinning[i];
			const auto weights = vertex.Weights();
			for ( unsigned int influence = 0; influence < 3; ++influence )
			{
				if ( weights[influence] != 0.0f )
				{
					const std::uint32_t bone = ( vertex.bones >> ( influence * 8 ) ) & 0xffu;
					if ( bone >= usedBones.size() )
					{
						topology->valid = false;
						break;
					}
					usedBones[bone] = true;
				}
			}
			topology->active.push_back( vertex );
			topology->activeIndices.push_back( i );
		}
		for ( std::uint32_t bone = 0; bone < usedBones.size(); ++bone )
			if ( usedBones[bone] )
				topology->bones.push_back( bone );
	}
	std::lock_guard<std::mutex> guard( m_PoseTopologyLock );
	if ( m_PoseTopologyRevision != m_ModelsRevision )
		return topology; // the models changed meanwhile: not kept
	return m_PoseTopology.try_emplace( key, std::move( topology ) ).first->second;
}

bool CoreWorld::PoseModel(
    const RenderCorePosedModel &source, pass::world::WorldView::PosedModel &out ) const
{
	if ( !source.boneToWorld || source.model >= m_ModelPoseSources.size() ||
	     source.model >= m_StaticMeshes.size() )
		return false;
	const ModelPoseSource &pose = m_ModelPoseSources[source.model];
	const pass::world::WorldData::StaticMesh &mesh = m_StaticMeshes[source.model];
	// A posed model draws one hardware level: its own vertex block is the base
	// the pose is applied to and its own index buffer is bound for the draw.
	const std::uint32_t lod = source.lod;
	if ( !pose.parsed || lod >= pose.lodCount || lod >= mesh.lodCount() ||
	     pose.poseToBone.empty() || source.boneCount < pose.poseToBone.size() )
		return false;
	const std::shared_ptr<const PoseTopology> topology =
	    PoseTopologyFor( source.model, lod, source.body );
	out.surfaceSelection = topology->surfaces;
	if ( !m_Pass.DrawsPosedModel( source.model, source.skin, source.phase, out.surfaceSelection ) )
		return false;
	if ( out.surfaceSelection->empty() )
		return true; // a blank level draws nothing; there is nothing to pose
	const pass::world::WorldData::StaticMeshLod &block = mesh.lods[lod];
	if ( !block.vertices || !block.indices )
		return false;
	if ( pose.LevelVertices( lod ).size() != block.vertexCount || !topology->valid )
		return false;
	// Only the selected topology borrows the live palette. The host may have
	// prepared no matrices for bones used exclusively by other body groups/LODs.
	const std::vector<pass::skinning::SkinVertex> &active = topology->active;
	const std::vector<std::uint32_t> &activeIndices = topology->activeIndices;
	std::vector<pass::skinning::BoneMatrix> palette( pose.poseToBone.size() );
	for ( std::uint32_t bone : topology->bones )
	{
		const float *world = source.boneToWorld + bone * 12;
		const pass::skinning::BoneMatrix &bind = pose.poseToBone[bone];
		for ( int row = 0; row < 3; ++row )
		{
			for ( int col = 0; col < 4; ++col )
			{
				float value = col == 3 ? world[row * 4 + 3] : 0.0f;
				for ( int k = 0; k < 3; ++k )
					value += world[row * 4 + k] * bind.rows[k][col];
				palette[bone].rows[row][col] = value;
			}
		}
	}
	std::vector<pass::skinning::SkinnedVertex> skinned( active.size() );
	pass::skinning::SkinReference( { active, palette, {}, {}, {} }, skinned );
	out.mesh = source.model;
	out.skin = source.skin;
	out.phase = source.phase;
	out.vertices = *block.vertices;
	for ( std::size_t i = 0; i < skinned.size(); ++i )
	{
		auto &vertex = out.vertices[activeIndices[i]];
		std::copy( skinned[i].position, skinned[i].position + 3, vertex.position );
		std::copy( skinned[i].normal, skinned[i].normal + 3, vertex.normal );
		std::copy( skinned[i].tangent, skinned[i].tangent + 4, vertex.tangent );
	}
	return true;
}

bool CoreWorld::DrawsStaticProp( unsigned int prop, unsigned int lod ) const
{
	if ( !m_StageSet || prop >= m_StaticInstances.size() )
		return false;
	const pass::world::WorldData::StaticInstance &instance = m_StaticInstances[prop];
	if ( instance.mesh >= m_ModelPoseSources.size() )
		return false;
	const ModelPoseSource &model = m_ModelPoseSources[instance.mesh];
	if ( !model.parsed || lod >= model.lodCount )
		return false;
	const auto selected = model.SelectedSurfaces( 0, lod );
	return m_Pass.DrawsPosedModel(
	    instance.mesh, instance.skin, RenderCoreDrawPhase::kAll, selected );
}

bool CoreWorld::DrawView( const unsigned int *surfaces, unsigned int count,
    const float worldToClip[16], const float viewport[6], unsigned long long hostFrame,
    const float worldToView[16], const float viewToClip[16], float waterZOffset,
    const RenderCoreStaticPropDraw *staticProps, unsigned int staticPropCount,
    const RenderCorePosedModel *posedModels, unsigned int posedModelCount )
{
	legacy::ICorePassSlots *slots = m_Frontend.CorePassSlots();
	if ( !slots || ( count == 0 && staticPropCount == 0 && posedModelCount == 0 ) ||
	     ( count && !surfaces ) || ( posedModelCount && !posedModels ) ||
	     ( ( staticPropCount || posedModelCount ) && !m_StageSet ) ||
	     ( staticPropCount && !staticProps ) )
		return false;
	for ( unsigned int i = 0; i < staticPropCount; ++i )
	{
		if ( !DrawsStaticProp( staticProps[i].prop, staticProps[i].lod ) )
			return false;
	}
	pass::world::WorldView view;
	if ( count )
		view.surfaces.assign( surfaces, surfaces + count );
	if ( staticPropCount )
	{
		view.staticInstances.reserve( staticPropCount );
		for ( unsigned int i = 0; i < staticPropCount; ++i )
		{
			const pass::world::WorldData::StaticInstance &instance =
			    m_StaticInstances[staticProps[i].prop];
			pass::world::WorldView::StaticInstance captured( staticProps[i].prop );
			captured.surfaceSelection =
			    m_ModelPoseSources[instance.mesh].SelectedSurfaces( 0, staticProps[i].lod );
			view.staticInstances.push_back( std::move( captured ) );
		}
	}
	for ( unsigned int i = 0; i < posedModelCount; ++i )
	{
		pass::world::WorldView::PosedModel pose;
		if ( !PoseModel( posedModels[i], pose ) )
			return false;
		if ( m_TemporalEnabled )
		{
			const auto &source = posedModels[i];
			const auto key =
			    std::make_pair( m_TemporalView, std::uint64_t( source.motionIdentity ) );
			const auto old = m_PreviousPoses.find( key );
			if ( m_TemporalView && source.motionIdentity && old != m_PreviousPoses.end() &&
			     old->second.model == source.model && old->second.body == source.body &&
			     old->second.lod == source.lod &&
			     old->second.vertices.size() == pose.vertices.size() )
				pose.previousVertices = old->second.vertices;
			else
			{
				pose.previousVertices = pose.vertices;
				for ( auto &vertex : pose.previousVertices )
					vertex.position[0] = std::numeric_limits<float>::quiet_NaN();
			}
			if ( m_TemporalView && source.motionIdentity )
				m_PendingPoses.insert_or_assign(
				    key, MotionPose{
				             source.model, source.lod, source.body, pose.vertices, source.skin } );
		}

		view.posedModels.push_back( std::move( pose ) );
	}
	std::memcpy( view.toClip, worldToClip, sizeof( view.toClip ) );
	view.viewport = {
	    viewport[0], viewport[1], viewport[2], viewport[3], viewport[4], viewport[5] };
	view.hostFrame = hostFrame;
	view.temporalView = m_TemporalEnabled ? m_TemporalView : 0;
	std::copy_n( worldToClip, 16, view.motionToClip );
	if ( m_TemporalEnabled && view.viewport.width > 0 && view.viewport.height > 0 )
		for ( unsigned c = 0; c < 4; ++c )
		{
			view.motionToClip[c] -= 2 * m_JitterX / view.viewport.width * worldToClip[12 + c];
			view.motionToClip[4 + c] += 2 * m_JitterY / view.viewport.height * worldToClip[12 + c];
		}

	if ( m_TemporalEnabled && m_TemporalView )
	{
		const auto old = m_PreviousCameras.find( m_TemporalView );
		if ( old != m_PreviousCameras.end() && old->second.viewport.width == view.viewport.width &&
		     old->second.viewport.height == view.viewport.height )
		{
			std::copy( old->second.toClip.begin(), old->second.toClip.end(), view.previousToClip );
			view.previousViewValid = true;
		}
		MotionCamera current;
		std::copy_n( view.motionToClip, 16, current.toClip.begin() );
		current.viewport = view.viewport;
		m_PendingCameras.insert_or_assign( m_TemporalView, current );
	}

	view.debug = m_Renderer.AppliedDebug();
	// The camera's right in the water plane: the view's x axis (the first
	// row of world-to-view) with its z dropped, normalized.
	if ( worldToView )
	{
		const float length =
		    std::sqrt( worldToView[0] * worldToView[0] + worldToView[1] * worldToView[1] );
		if ( length > 0.0f )
		{
			view.viewRight[0] = worldToView[0] / length;
			view.viewRight[1] = worldToView[1] / length;
		}
	}
	view.waterZOffset = waterZOffset;
	// A view that draws no *world* geometry of its own (the client's viewmodel
	// scope, which pushes its own 3D view and queues only the handed-off model
	// it draws) has no world to plan lighting or screen passes for: the depth
	// and normal prepass and the ambient occlusion over them are the world's,
	// and the world was drawn by the frame's own view. When that view's stage
	// lighting is already recorded, this view's slot reads it below, so the
	// frame builds one shadow atlas and one ambient-occlusion pass rather than
	// one per view. A frame whose world view has not recorded yet, and a view
	// with its own surfaces or static instances, keep their own lighting.
	view.drawsWorldGeometry = !view.surfaces.empty() || !view.staticInstances.empty();
	// A stage view's lights are clustered and its shadows planned when its
	// slot records (the render sequence), from what the frame holds now.
	std::shared_ptr<PendingView> pending;
	if ( worldToView && viewToClip && ( m_StageSet || !m_Lights.areas.empty() ) )
	{
		const bool movers = m_ShadowMovers.load( std::memory_order_relaxed );
		const int quality = m_ShadowQuality.load( std::memory_order_relaxed );
		std::erase_if( m_QueuedLighting,
		    [&]( const QueuedLighting &entry )
		    {
			    return entry.frame != hostFrame || entry.revision != m_Lights.revision;
		    } );
		for ( const QueuedLighting &entry : m_QueuedLighting )
		{
			const ViewLightInputs &in = entry.pending->inputs;
			if ( entry.movers == movers && in.shadowQuality == quality &&
			     std::equal( worldToView, worldToView + 16, in.worldToView ) &&
			     std::equal( viewToClip, viewToClip + 16, in.viewToClip ) &&
			     std::equal( viewport, viewport + 6, in.viewport ) )
			{
				pending = entry.pending;
				break;
			}
		}
		if ( !pending )
		{
			pending = std::make_shared<PendingView>();
			RefreshCookies();
			pending->inputs = TakeViewLightInputs( worldToView, viewToClip, viewport );
			if ( m_QueuedLighting.size() < 64 )
				m_QueuedLighting.push_back( { hostFrame, m_Lights.revision, movers, pending } );
		}
		++m_StageLitViews;
	}
	view.stageLighting = pending;
	const std::uint32_t tag = m_Pass.QueueView( std::move( view ) );
	if ( tag == 0 )
		return false;
	if ( m_Renderer.AppliedDebug().legacy == frame::DebugLegacy::kTint && m_ViewDepth <= 1 )
	{
		std::lock_guard<std::mutex> guard( m_TopLevelLock );
		m_TopLevel.insert( tag );
		while ( m_TopLevel.size() > 256 )
			m_TopLevel.erase( m_TopLevel.begin() );
	}
	slots->MarkSlot( tag );
	return true;
}

void CoreWorld::OnStage( frame::Stage, std::uint32_t depth )
{
	m_ViewDepth = depth;
}

// No stage slot: the legacy UI exception at the top-level HUD is retired
// (the screen UI is the core's, render.ui-draw-list.v1), so a core-only frame
// stays core-only to its present.
std::uint32_t CoreWorld::SlotStages() const
{
	return 0u;
}

void CoreWorld::EndFrame()
{
	if ( m_Renderer.AppliedDebug().legacy != frame::DebugLegacy::kTint )
		return;
	if ( legacy::ICorePassSlots *slots = m_Frontend.CorePassSlots() )
		slots->MarkSlot( legacy::kCorePassForwarded | legacy::kCorePassFrameEnd );
}

void CoreWorld::BeginFrame()
{
	if ( m_TemporalEnabled )
	{
		const auto halton = []( unsigned index, unsigned base )
		{
			float result = 0, scale = 1;
			while ( index )
			{
				scale /= float( base );
				result += scale * float( index % base );
				index /= base;
			}
			return result;
		};
		const unsigned sample = ( m_JitterSequence++ % 32 ) + 1;
		m_JitterX = halton( sample, 2 ) - 0.5f;
		m_JitterY = halton( sample, 3 ) - 0.5f;
	}

	const frame::DebugControls &debug = m_Renderer.AppliedDebug();
	if ( debug.costOverlay )
		if ( legacy::ICorePassSlots *slots = m_Frontend.CorePassSlots() )
			slots->MarkSlot( kCostBegin );
	if ( !m_CoreOnly && !frame::PixelViewActive( debug ) &&
	     debug.legacy != frame::DebugLegacy::kSkip )
		return;
	if ( legacy::ICorePassSlots *slots = m_Frontend.CorePassSlots() )
	{
		const frame::DebugControls &applied = m_Renderer.AppliedDebug();
		const bool effects = m_CoreOnly && !frame::PixelViewActive( applied ) &&
		                     applied.legacy != frame::DebugLegacy::kSkip;
		slots->MarkSlot( legacy::kCorePassForwarded | legacy::kCorePassLegacyOff |
		                 ( effects ? legacy::kCorePassCustomEffects : 0u ) );
	}
}

unsigned long long CoreWorld::Failures() const
{
	return m_Pass.Failures() + m_LightingFailures.load();
}

graph::GpuPassTimers *CoreWorld::SlotTimers( const legacy::CorePassTarget &target )
{
	std::lock_guard<std::mutex> guard( m_TimersLock );
	// One decision per frame: the frame's slots share its encoders.
	if ( target.frame != m_TimersFrame )
	{
		m_TimersFrame = target.frame;
		m_TimersThisFrame =
		    m_GpuTimersOn.load( std::memory_order_relaxed ) || m_CostFrame == target.frame;
	}
	if ( !m_TimersThisFrame )
	{
		if ( m_Timers )
		{
			// This frame's `submitted` covers every frame the timers wrote;
			// their buffers go behind it.
			m_Timers->BeginFrame( target.frame, target.submitted );
			m_Timers.reset();
		}
		return nullptr;
	}
	if ( !m_Timers || &m_Timers->Device() != target.device )
	{
		// Portal views and their shadow subviews exceed the generic 512-timestamp
		// debug limit; keep their pass labels instead of silently truncating them.
		m_Timers = std::make_unique<graph::GpuPassTimers>( *target.device, 4096 );
	}
	if ( !m_Timers->Supported() )
		return nullptr;
	m_Timers->BeginFrame( target.frame, target.submitted );
	return m_Timers.get();
}

const char *CoreWorld::MaterialDefault( const char *shader, const char *key )
{
	if ( !m_Host || !m_Host->materialDefault )
		return nullptr;
	std::lock_guard<std::mutex> guard( m_DefaultsLock );
	const std::pair<const char *, const char *> address( shader, key );
	if ( auto at = m_DefaultsByAddress.find( address );
	    at != m_DefaultsByAddress.end() && *at->second.shader == shader && *at->second.key == key )
		return at->second.value->c_str();
	auto byShader = m_MaterialDefaults.find( std::string_view( shader ) );
	const std::string *found = nullptr;
	if ( byShader != m_MaterialDefaults.end() )
	{
		auto value = byShader->second.find( std::string_view( key ) );
		if ( value != byShader->second.end() )
			found = &value->second;
	}
	if ( !found )
	{
		const char *value = m_Host->materialDefault( shader, key );
		if ( !value )
			return nullptr;
		if ( byShader == m_MaterialDefaults.end() )
			byShader = m_MaterialDefaults.try_emplace( shader ).first;
		found = &byShader->second.try_emplace( key, value ).first->second;
	}
	const auto keyEntry = byShader->second.find( std::string_view( key ) );
	if ( m_DefaultsByAddress.size() >= 65536 )
		m_DefaultsByAddress.clear(); // bounded if a caller's strings are transient
	m_DefaultsByAddress[address] = { &byShader->first, &keyEntry->first, found };
	return found->c_str();
}

// The moving casters' mesh: a cube of half-size one (an occluder box's
// placement scales it), staged once.
const resources::MeshEntry *CoreWorld::BoxCasterMesh()
{
	static constexpr char kName[] = "stage box caster";
	if ( const resources::MeshEntry *found = m_CasterMeshes->Find( kName ) )
		return found;
	static const float kCorners[24] = {
	    -1, -1, -1, 1, -1, -1, -1, 1, -1, 1, 1, -1, -1, -1, 1, 1, -1, 1, -1, 1, 1, 1, 1, 1 };
	static const std::uint32_t kTriangles[36] = { 0, 1, 3, 0, 3, 2, 4, 6, 7, 4, 7, 5, 0, 4, 5, 0, 5,
	    1, 2, 3, 7, 2, 7, 6, 0, 2, 6, 0, 6, 4, 1, 5, 7, 1, 7, 3 };
	resources::MeshData data;
	data.vertices = std::as_bytes( std::span( kCorners ) );
	data.vertexStride = 3 * sizeof( float );
	data.indices = std::as_bytes( std::span( kTriangles ) );
	data.indexFormat = device::IndexFormat::kUint32;
	if ( !m_CasterMeshes->Stage( kName, data ) )
		return nullptr;
	return m_CasterMeshes->Find( kName );
}

void CoreWorld::BindStageDevice( device::IRenderDevice2 &device )
{
	// Progressive enhancement: a device without the BC formats (Adreno 730)
	// draws the lightmap's decoded RGBA16F pages and RGBA16F reflection
	// probes instead of the lump's blocks; the stage is set again with them.
	if ( !device.Facts().capabilities.Has( device::Capability::kTextureCompressionBC ) )
	{
		bool decoded = m_Capture.lightmap.Blocks() && m_Capture.DecodePages();
		auto &probes = m_Capture.reflection;
		if ( probes && probes->format == device::Format::kBC6HUfloat )
		{
			std::vector<std::byte> texels;
			if ( mapcontainer::DecodeReflectionProbeRadiance( probes->radiance.data(),
			         probes->radiance.size(), probes->face, probes->baseMip, probes->mips,
			         probes->count * 6, &texels ) )
			{
				probes->radiance = std::move( texels );
				probes->format = device::Format::kRGBA16Float;
				decoded = true;
			}
			else
			{
				probes.reset(); // drawn without reflection probes
				decoded = true;
			}
		}
		if ( decoded && m_StageSet )
			SetStage();
	}
	if ( m_ShadowDevice == &device )
		return;
	// A new backend device: the old one's objects went with it.
	m_ClusterKernel.reset();
	m_ClusterFrame = 0;
	m_ShadowRenderer.reset();
	m_CasterMeshes.reset();
	m_TriangleRevisions.clear();
	m_Atlases.clear();
	m_AtlasNext = 0;
	m_AtlasFrame = 0;
	m_FrameAtlasIndices.clear();
	m_CastersStaged = 0;
	m_Ao.reset();
	m_Volumetric.reset();
	m_VolumetricFormat = device::Format::kUnknown;
	ReleaseSsr( device, device::CompletionToken() );
	m_Ssr.reset();
	m_SsrCopy.reset();
	if ( m_Cookies )
		m_Cookies->Release( device::CompletionToken() );
	m_Cookies.reset();
	m_CookiesUploaded.reset();
	m_Occlusion = device::TextureId();
	m_OcclusionDesc = device::TextureDesc();
	m_ShadowDevice = &device;
}

std::optional<pass::world::IModelLevelSource::LevelGeometry>
CoreWorld::ModelLevelSource::ResupplyLevel( std::uint32_t mesh, std::uint32_t lod )
{
	if ( mesh >= m_Owner.m_ModelBytes.size() )
		return std::nullopt;
	const ModelBytes &raw = m_Owner.m_ModelBytes[mesh];
	if ( raw.mdl.empty() )
		return std::nullopt;
	mdl::ModelBytes bytes;
	bytes.mdl = { raw.mdl.data(), raw.mdl.size() };
	bytes.vvd = { raw.vvd.data(), raw.vvd.size() };
	bytes.vtx = { raw.vtx.data(), raw.vtx.size() };
	auto parsed = mdl::ParseModelGeometryVariants( bytes );
	if ( !parsed )
		return std::nullopt;
	const mdl::Model &model = parsed.Value();
	const std::size_t levelCount = model.lodTextures.size();
	if ( lod >= levelCount )
		return std::nullopt;
	std::vector<const mdl::Mesh *> parts;
	for ( const mdl::Mesh &part : model.meshes )
	{
		if ( part.lod == lod )
			parts.push_back( &part );
	}
	if ( parts.empty() )
		return std::nullopt;
	LevelGeometry result;
	for ( const mdl::Mesh *part : parts )
	{
		const bool resolvedLod = part->lod < raw.materialLodCount;
		const bool unchangedLod = model.lodTextures[part->lod] == model.lodTextures[0];
		const std::int32_t texture =
		    model.skinFamilies.empty()
		        ? part->textureRef
		        : ( part->textureRef >= 0 &&
		                      std::size_t( part->textureRef ) < model.skinFamilies[0].size()
		                  ? model.skinFamilies[0][std::size_t( part->textureRef )]
		                  : -1 );
		if ( !resolvedLod && !unchangedLod )
			continue;
		if ( texture < 0 || std::uint32_t( texture ) >= raw.materialCount )
			return std::nullopt;
		const std::uint32_t base = std::uint32_t( result.vertices.size() );
		for ( const mdl::Vertex &from : part->vertices )
		{
			material::SurfaceModelVertex to;
			to.position[0] = from.position.x;
			to.position[1] = from.position.y;
			to.position[2] = from.position.z;
			to.normal[0] = from.normal.x;
			to.normal[1] = from.normal.y;
			to.normal[2] = from.normal.z;
			to.uv[0] = from.u;
			to.uv[1] = from.v;
			if ( from.tangentSign != 0.0f )
			{
				to.tangent[0] = from.tangent.x;
				to.tangent[1] = from.tangent.y;
				to.tangent[2] = from.tangent.z;
				to.tangent[3] = from.tangentSign;
			}
			else
			{
				const bool useZ = std::fabs( from.normal.z ) < 0.9f;
				float x = useZ ? from.normal.y : 0.0f;
				float y = useZ ? -from.normal.x : from.normal.z;
				float z = useZ ? 0.0f : -from.normal.y;
				const float length = std::sqrt( x * x + y * y + z * z );
				if ( length > 0.0f )
				{
					x /= length;
					y /= length;
					z /= length;
				}
				to.tangent[0] = x;
				to.tangent[1] = y;
				to.tangent[2] = z;
				to.tangent[3] = 1.0f;
			}
			result.vertices.push_back( to );
		}
		for ( std::uint32_t index : part->indices )
			result.indices.push_back( base + index );
	}
	return result;
}

void CoreWorld::ModelLevelSource::PrefetchLevel( std::uint32_t, std::uint32_t )
{
	// Synchronous resupply: the raw MDL bytes are in memory and the parse is
	// fast enough to run inline on the render sequence. A future async
	// prefetch would start the parse on a worker here.
}

} // namespace render::composition
