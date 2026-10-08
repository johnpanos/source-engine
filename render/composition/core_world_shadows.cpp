//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.composition's world (RFC 0016 K5): the stage's shadow atlas and GPU occlusion.
//
//=============================================================================//

#include "core_world_internal.h"

namespace render::composition
{

// Each view draws the chunks inside its frustum (a chunk's box wholly
// outside one clip plane is culled). The six clip half-spaces of a view
// (-w <= x, y <= w, 0 <= z <= w) are planes over its matrix's rows, and a
// box is wholly outside one when its corner farthest along the plane's
// normal is: one dot product per plane instead of eight transforms.
CoreWorld::ClipPlanes CoreWorld::PlanesOf( const math::float4x4 &clip )
{
	const math::float4 *r = clip.rows;
	const auto add = []( const math::float4 &a, const math::float4 &b, float sign )
	{
		return math::float4{
		    a.x + sign * b.x, a.y + sign * b.y, a.z + sign * b.z, a.w + sign * b.w };
	};
	return { add( r[3], r[0], 1.0f ), add( r[3], r[0], -1.0f ), add( r[3], r[1], 1.0f ),
	    add( r[3], r[1], -1.0f ), r[2], add( r[3], r[2], -1.0f ) };
}

bool CoreWorld::ChunkInside( const ClipPlanes &planes, const Casters::Chunk &chunk )
{
	for ( const math::float4 &n : planes )
	{
		const float far = n.x * ( n.x >= 0.0f ? chunk.max[0] : chunk.min[0] ) +
		                  n.y * ( n.y >= 0.0f ? chunk.max[1] : chunk.min[1] ) +
		                  n.z * ( n.z >= 0.0f ? chunk.max[2] : chunk.min[2] ) + n.w;
		if ( far < 0.0f )
			return false;
	}
	return true;
}

// The frame's moving casters per shadow view: the occluder boxes and the
// physical triangle meshes inside each view, with an order-independent
// signature of what each view saw. False when a mesh cannot be staged.
bool CoreWorld::CollectMovingCasters( const ShadowWork &work,
    const std::vector<ClipPlanes> &viewPlanes,
    std::vector<std::vector<pass::shadows::ShadowCaster>> &moverCasters,
    std::vector<std::uint64_t> &moverSignature, bool &anyMover )
{
	if ( !work.movers.empty() )
	{
		const resources::MeshEntry *box = BoxCasterMesh();
		for ( std::size_t v = 0; box && v < work.views.size(); ++v )
		{
			for ( const light_set::RuntimeOccluder &occluder : work.movers )
			{
				const dynamic_occlusion::Box &mover = occluder.box;
				// The physical mesh replaces this entity's coarse boxes on the core.
				if ( std::any_of( work.triangles.begin(), work.triangles.end(),
				         [&]( const auto &part )
				         {
					         return part.entity == mover.entity;
				         } ) )
					continue;
				Casters::Chunk bounds;
				math::float4x4 place;
				for ( int r = 0; r < 3; ++r )
				{
					const float extent = std::fabs( mover.axes[0][r] ) +
					                     std::fabs( mover.axes[1][r] ) +
					                     std::fabs( mover.axes[2][r] );
					bounds.min[r] = mover.center[r] - extent;
					bounds.max[r] = mover.center[r] + extent;
					( &place.rows[r].x )[0] = mover.axes[0][r];
					( &place.rows[r].x )[1] = mover.axes[1][r];
					( &place.rows[r].x )[2] = mover.axes[2][r];
					( &place.rows[r].x )[3] = mover.center[r];
				}
				place.rows[3] = { 0.0f, 0.0f, 0.0f, 1.0f };
				if ( ChunkInside( viewPlanes[v], bounds ) )
				{
					moverCasters[v].push_back( { *box, place } );
					// Order-independent: a sum of each mover's mixed key.
					std::uint64_t key = ( std::uint64_t( std::uint32_t( mover.entity ) ) << 32 ) ^
					                    ( std::uint64_t( std::uint32_t( mover.part ) ) << 20 ) ^
					                    occluder.version;
					key ^= key >> 33;
					key *= 0xff51afd7ed558ccdull;
					key ^= key >> 33;
					moverSignature[v] += key | 1u; // never 0 with a mover
					anyMover = true;
				}
			}
		}
	}
	std::vector<std::string> liveTriangleMeshes;
	for ( const auto &part : work.triangles )
	{
		if ( part.positions.empty() || part.positions.size() % 9 || !part.version ||
		     std::any_of( part.positions.begin(), part.positions.end(),
		         []( float value )
		         {
			         return !std::isfinite( value );
		         } ) )
			return false;
		const std::string name = "stage physical caster " + std::to_string( part.entity ) + ":" +
		                         std::to_string( part.part );
		liveTriangleMeshes.push_back( name );
		auto &revision = m_TriangleRevisions[name];
		if ( revision != part.version )
		{
			resources::MeshData data;
			data.vertices = std::as_bytes( std::span( part.positions ) );
			data.vertexStride = 3 * sizeof( float );
			if ( !m_CasterMeshes->Stage( name, data ) )
				return false;
			revision = part.version;
		}
		const resources::MeshEntry *geometry = m_CasterMeshes->Find( name );
		if ( !geometry )
			return false;
		Casters::Chunk bounds;
		for ( int k = 0; k < 3; ++k )
			bounds.min[k] = bounds.max[k] = part.positions[k];
		for ( std::size_t i = 0; i < part.positions.size(); ++i )
		{
			bounds.min[i % 3] = std::min( bounds.min[i % 3], part.positions[i] );
			bounds.max[i % 3] = std::max( bounds.max[i % 3], part.positions[i] );
		}
		for ( std::size_t v = 0; v < work.views.size(); ++v )
		{
			if ( !ChunkInside( viewPlanes[v], bounds ) )
				continue;
			moverCasters[v].push_back( { *geometry, math::float4x4::Identity() } );
			std::uint64_t key = part.version ^
			                    ( std::uint64_t( std::uint32_t( part.entity ) ) << 32 ) ^
			                    ( std::uint64_t( std::uint32_t( part.part ) ) << 20 );
			key ^= key >> 33;
			key *= 0xff51afd7ed558ccdull;
			key ^= key >> 33;
			moverSignature[v] += key | 1u;
			anyMover = true;
		}
	}
	for ( auto it = m_TriangleRevisions.begin(); it != m_TriangleRevisions.end(); )
	{
		if ( std::find( liveTriangleMeshes.begin(), liveTriangleMeshes.end(), it->first ) ==
		     liveTriangleMeshes.end() )
		{
			(void)m_CasterMeshes->Evict( it->first );
			it = m_TriangleRevisions.erase( it );
		}
		else
			++it;
	}
	return true;
}

device::TextureId CoreWorld::DrawStageShadows( device::IRenderDevice2 &device,
    const ShadowWork &work, device::CompletionToken submitted, std::uint64_t frame,
    device::TextureDesc *desc )
{
	using namespace render::device;
	BindStageDevice( device );
	if ( !m_ShadowRenderer )
	{
		auto created = pass::shadows::ShadowDepthRenderer::Create( device );
		if ( !created )
			return {};
		m_ShadowRenderer = std::move( created ).Value();
	}
	std::shared_ptr<const Casters> casters;
	{
		std::lock_guard<std::mutex> guard( m_ShadowLock );
		casters = m_Casters;
	}
	if ( !casters || casters->indices.empty() )
		return {};
	if ( !m_CasterMeshes )
		m_CasterMeshes = std::make_unique<resources::MeshCache>( device );
	if ( m_CastersStaged != casters->generation )
	{
		if ( !m_CasterName.empty() )
			(void)m_CasterMeshes->Evict( m_CasterName );
		m_CasterName = "stage casters " + std::to_string( casters->generation );
		resources::MeshData data;
		data.vertices = std::as_bytes( std::span( casters->positions ) );
		data.vertexStride = 3 * sizeof( float );
		data.indices = std::as_bytes( std::span( casters->indices ) );
		data.indexFormat = IndexFormat::kUint32;
		if ( !m_CasterMeshes->Stage( m_CasterName, data ) )
			return {};
		m_CastersStaged = casters->generation;
	}
	const resources::MeshEntry *mesh = m_CasterMeshes->Find( m_CasterName );
	if ( !mesh )
		return {};
	// This frame's next atlas of the pool.
	if ( frame != m_AtlasFrame )
	{
		m_AtlasFrame = frame;
		m_AtlasNext = 0;
		m_FrameAtlasIndices.clear();
	}
	if ( m_AtlasNext == m_Atlases.size() )
	{
		Atlas atlas;
		atlas.desc.format = Format::kD32Float;
		atlas.desc.width = atlas.desc.height = work.atlasSize;
		atlas.desc.usages = { ResourceUsage::kDepthWrite, ResourceUsage::kSampled,
		    ResourceUsage::kCopySource, ResourceUsage::kCopyDestination };
		atlas.desc.debugName = "stage shadow atlas";
		auto texture = device.CreateTexture( atlas.desc );
		if ( !texture )
			return {};
		atlas.texture = texture.Value();
		atlas.desc.debugName = {};
		m_Atlases.push_back( atlas );
	}
	Atlas &atlas = m_Atlases[m_AtlasNext];
	if ( atlas.desc.width != work.atlasSize )
	{
		// A new shadow quality: this slot's atlas was last used by an earlier
		// frame, which `submitted` covers.
		(void)device.Release( atlas.texture, submitted );
		if ( atlas.composite.IsValid() )
			(void)device.Release( atlas.composite, submitted );
		atlas = Atlas();
		atlas.desc.format = Format::kD32Float;
		atlas.desc.width = atlas.desc.height = work.atlasSize;
		atlas.desc.usages = { ResourceUsage::kDepthWrite, ResourceUsage::kSampled,
		    ResourceUsage::kCopySource, ResourceUsage::kCopyDestination };
		atlas.desc.debugName = "stage shadow atlas";
		auto texture = device.CreateTexture( atlas.desc );
		atlas.desc.debugName = {};
		if ( !texture )
		{
			m_Atlases.erase( m_Atlases.begin() + std::ptrdiff_t( m_AtlasNext ) );
			return {};
		}
		atlas.texture = texture.Value();
	}
	// The views the atlas does not hold as they are now; all of them for a
	// new atlas or new casters (Doom Eternal's and HDRP's cached shadows:
	// the stage's casters are static, so a light that did not move keeps
	// its tiles).
	auto same = []( const pass::shadows::ShadowPlanView &a, const pass::shadows::ShadowPlanView &b )
	{
		return a.tile == b.tile &&
		       std::memcmp( &a.viewProjection, &b.viewProjection, sizeof( a.viewProjection ) ) == 0;
	};
	const bool whole = atlas.usage == ResourceUsage::kUndefined ||
	                   atlas.generation != casters->generation ||
	                   atlas.guardTexels != work.guardTexels;
	std::vector<const pass::shadows::ShadowPlanView *> dirty;
	for ( const pass::shadows::ShadowPlanView &view : work.views )
	{
		if ( whole || std::none_of( atlas.drawn.begin(), atlas.drawn.end(),
		                  [&]( const pass::shadows::ShadowPlanView &held )
		                  {
			                  return same( held, view );
		                  } ) )
			dirty.push_back( &view );
	}
	const std::size_t dirtyBeforeShare = dirty.size();
	struct SharedTiles
	{
		std::size_t source = 0;
		std::vector<pass::shadows::ShadowDepthRenderer::TileRemap> remaps;
	};
	std::vector<SharedTiles> shared;
	std::vector<const pass::shadows::ShadowPlanView *> sharedViews;
	auto isSunView = []( int first, int count, std::size_t view )
	{
		return first >= 0 && count > 0 && view >= std::size_t( first ) &&
		       view < std::size_t( first + count );
	};
	for ( const pass::shadows::ShadowPlanView *view : dirty )
	{
		const std::size_t viewIndex = std::size_t( view - work.views.data() );
		if ( isSunView( work.sunFirst, work.sunCount, viewIndex ) )
			continue; // sun cascades are fitted to this camera's frustum
		for ( std::size_t sourceIndex : m_FrameAtlasIndices )
		{
			if ( sourceIndex >= m_Atlases.size() || sourceIndex == m_AtlasNext )
				continue;
			const Atlas &source = m_Atlases[sourceIndex];
			if ( source.usage != ResourceUsage::kSampled || source.desc.width != work.atlasSize ||
			     source.generation != casters->generation ||
			     source.guardTexels != work.guardTexels )
				continue;
			bool found = false;
			for ( std::size_t heldIndex = 0; heldIndex < source.drawn.size(); ++heldIndex )
			{
				if ( isSunView( source.sunFirst, source.sunCount, heldIndex ) )
					continue;
				// Match by light-space viewProjection only: the same light
				// may be packed at a different tile position in this view's
				// atlas because the sun cascades (camera-dependent) shifted
				// the layout. A tile of another size is not this tile: the
				// plan resizes a light's tile as other lights come and go, and
				// the copy moves a tile's texels one for one.
				if ( source.drawn[heldIndex].tile.size != view->tile.size ||
				     std::memcmp( &source.drawn[heldIndex].viewProjection, &view->viewProjection,
				         sizeof( view->viewProjection ) ) != 0 )
					continue;
				auto group = std::find_if( shared.begin(), shared.end(),
				    [&]( const SharedTiles &candidate )
				    {
					    return candidate.source == sourceIndex;
				    } );
				if ( group == shared.end() )
				{
					shared.push_back( { sourceIndex, {} } );
					group = shared.end() - 1;
				}
				group->remaps.push_back( { source.drawn[heldIndex].tile, view->tile } );
				sharedViews.push_back( view );
				found = true;
				break;
			}
			if ( found )
				break;
		}
	}
	std::erase_if( dirty,
	    [&]( const pass::shadows::ShadowPlanView *view )
	    {
		    return std::find( sharedViews.begin(), sharedViews.end(), view ) != sharedViews.end();
	    } );
	m_ShadowTilesKept.fetch_add( work.views.size() - dirtyBeforeShare, std::memory_order_relaxed );
	m_ShadowTilesShared.fetch_add( sharedViews.size(), std::memory_order_relaxed );
	m_ShadowTilesDrawn.fetch_add( dirty.size(), std::memory_order_relaxed );
	std::vector<ClipPlanes> viewPlanes;
	viewPlanes.reserve( work.views.size() );
	for ( const pass::shadows::ShadowPlanView &view : work.views )
		viewPlanes.push_back( PlanesOf( view.viewProjection ) );
	// The moving casters (the frame's occluder boxes, render.dynamic-occlusion):
	// a unit cube placed by each box, per view that sees it.
	std::vector<std::vector<pass::shadows::ShadowCaster>> moverCasters( work.views.size() );
	std::vector<std::uint64_t> moverSignature( work.views.size(), 0 );
	bool anyMover = false;
	if ( !CollectMovingCasters( work, viewPlanes, moverCasters, moverSignature, anyMover ) )
		return {};
	const bool composite = anyMover || !atlas.moverTiles.empty();
	if ( dirty.empty() && shared.empty() && !composite )
	{
		atlas.sunFirst = work.sunFirst;
		atlas.sunCount = work.sunCount;
		m_FrameAtlasIndices.push_back( m_AtlasNext );
		++m_AtlasNext;
		*desc = atlas.desc;
		return atlas.texture;
	}
	const auto uploads = [this]( graph::GraphBuilder &builder )
	{
		builder.AddPass( "stage caster uploads", graph::PassKind::kCopy )
		    .SideEffect()
		    .Execute(
		        [this]( graph::RecordContext &context )
		        {
			        m_CasterMeshes->RecordUploads( context.Encoder() );
		        } );
	};
	const auto execute = [&]( graph::GraphBuilder &&builder ) -> bool
	{
		auto compiled = graph::CompileGraph( std::move( builder ) );
		if ( !compiled )
			return false;
		graph::SerialGraphExecutor executor;
		executor.SetLabelObserver( m_SlotTimers );
		auto executed = executor.Execute( compiled.Value(), device );
		if ( !executed )
			return false;
		m_ShadowRenderer->Collect( executed.Value().token );
		m_CasterMeshes->Retire( executed.Value().token );
		return true;
	};
	// The static tiles whose view changed (cached otherwise).
	if ( !dirty.empty() || !shared.empty() )
	{
		std::vector<std::vector<pass::shadows::ShadowCaster>> chunkCasters( dirty.size() );
		std::vector<pass::shadows::ShadowDepthView> views;
		views.reserve( dirty.size() );
		for ( std::size_t v = 0; v < dirty.size(); ++v )
		{
			const ClipPlanes planes = PlanesOf( dirty[v]->viewProjection );
			chunkCasters[v].reserve( casters->chunks.size() );
			for ( const Casters::Chunk &chunk : casters->chunks )
			{
				if ( ChunkInside( planes, chunk ) )
					chunkCasters[v].push_back(
					    { *mesh, math::float4x4::Identity(), chunk.firstIndex, chunk.indexCount } );
			}
			views.push_back( { dirty[v]->viewProjection, dirty[v]->tile, chunkCasters[v] } );
		}
		graph::GraphBuilder builder;
		uploads( builder );
		const graph::ResourceRef atlasRef = builder.ImportTexture(
		    "stage shadow atlas", atlas.texture, atlas.desc, atlas.usage, ResourceUsage::kSampled );
		if ( whole && !shared.empty() )
		{
			builder.AddPass( "clear shared stage shadow atlas", graph::PassKind::kCopy )
			    .Write( atlasRef, ResourceUsage::kCopyDestination )
			    .Execute(
			        [atlasRef]( graph::RecordContext &context )
			        {
				        context.Encoder().ClearTexture(
				            context.Texture( atlasRef ), { 1.0f, 0.0f, 0.0f, 0.0f } );
			        } );
		}
		for ( const SharedTiles &tiles : shared )
		{
			const Atlas &source = m_Atlases[tiles.source];
			const graph::ResourceRef sourceRef = builder.ImportTexture( "shared stage shadow atlas",
			    source.texture, source.desc, source.usage, ResourceUsage::kSampled );
			pass::shadows::ShadowDepthRenderer::AddTileRemapCopy(
			    builder, sourceRef, atlasRef, tiles.remaps );
		}
		pass::shadows::ShadowAtlasTarget target{ atlasRef, work.atlasSize, work.guardTexels };
		target.keep = !whole || !shared.empty();
		if ( ( !views.empty() && !m_ShadowRenderer->AddPasses( builder, target, views ) ) ||
		     !execute( std::move( builder ) ) )
			return {};
		atlas.usage = ResourceUsage::kSampled;
		atlas.drawn = work.views;
		atlas.sunFirst = work.sunFirst;
		atlas.sunCount = work.sunCount;
		atlas.generation = casters->generation;
		atlas.guardTexels = work.guardTexels;
		m_FrameAtlasIndices.push_back( m_AtlasNext );
	}
	if ( !composite )
	{
		atlas.sunFirst = work.sunFirst;
		atlas.sunCount = work.sunCount;
		if ( m_FrameAtlasIndices.empty() || m_FrameAtlasIndices.back() != m_AtlasNext )
			m_FrameAtlasIndices.push_back( m_AtlasNext );
		++m_AtlasNext;
		*desc = atlas.desc;
		return atlas.texture;
	}
	// The frame's atlas (Doom Eternal's cached shadows with moving casters):
	// a tile a mover reaches now or reached last frame, one whose static
	// depth changed, or one the frame's atlas never held, is restored from
	// the static atlas; the movers are drawn over their tiles.
	if ( !atlas.composite.IsValid() )
	{
		TextureDesc compositeDesc = atlas.desc;
		compositeDesc.debugName = "stage shadow atlas (movers)";
		auto texture = device.CreateTexture( compositeDesc );
		if ( !texture )
			return {};
		atlas.composite = texture.Value();
		atlas.compositeUsage = ResourceUsage::kUndefined;
		atlas.compositeHeld.clear();
		atlas.compositeGeneration = 0;
		atlas.compositeGuardTexels = 0;
	}
	std::vector<pass::shadows::ShadowTile> restore;
	std::vector<pass::shadows::ShadowDepthView> moverViews;
	std::vector<std::pair<pass::shadows::ShadowTile, std::uint64_t>> moverTiles;
	for ( std::size_t v = 0; v < work.views.size(); ++v )
	{
		const pass::shadows::ShadowPlanView &view = work.views[v];
		// What the frame's atlas holds in this tile: this view's static depth
		// with the movers of `drawnSignature` over it.
		const bool held = atlas.compositeUsage != ResourceUsage::kUndefined &&
		                  atlas.compositeGeneration == casters->generation &&
		                  atlas.compositeGuardTexels == work.guardTexels &&
		                  std::any_of( atlas.compositeHeld.begin(), atlas.compositeHeld.end(),
		                      [&]( const pass::shadows::ShadowPlanView &kept )
		                      {
			                      return same( kept, view );
		                      } ) &&
		                  std::none_of( dirty.begin(), dirty.end(),
		                      [&]( const pass::shadows::ShadowPlanView *changed )
		                      {
			                      return changed->tile == view.tile;
		                      } );
		std::uint64_t drawnSignature = 0;
		for ( const auto &[tile, signature] : atlas.moverTiles )
		{
			if ( tile == view.tile )
				drawnSignature = signature;
		}
		if ( moverSignature[v] )
			moverTiles.emplace_back( view.tile, moverSignature[v] );
		if ( held && drawnSignature == moverSignature[v] )
			continue; // nothing in the tile moved
		restore.push_back( view.tile );
		if ( !moverCasters[v].empty() )
		{
			pass::shadows::ShadowDepthView drawn{ view.viewProjection, view.tile, moverCasters[v] };
			drawn.clearTile = false;
			moverViews.push_back( drawn );
		}
	}
	graph::GraphBuilder builder;
	uploads( builder );
	const graph::ResourceRef staticRef = builder.ImportTexture(
	    "stage shadow atlas", atlas.texture, atlas.desc, atlas.usage, ResourceUsage::kSampled );
	const graph::ResourceRef compositeRef = builder.ImportTexture( "stage shadow atlas (movers)",
	    atlas.composite, atlas.desc, atlas.compositeUsage, ResourceUsage::kSampled );
	if ( restore.empty() )
	{
		atlas.compositeHeld = work.views;
		atlas.moverTiles = std::move( moverTiles );
		atlas.compositeGeneration = casters->generation;
		atlas.compositeGuardTexels = work.guardTexels;
		atlas.sunFirst = work.sunFirst;
		atlas.sunCount = work.sunCount;
		if ( m_FrameAtlasIndices.empty() || m_FrameAtlasIndices.back() != m_AtlasNext )
			m_FrameAtlasIndices.push_back( m_AtlasNext );
		++m_AtlasNext;
		*desc = atlas.desc;
		return atlas.composite;
	}
	pass::shadows::ShadowDepthRenderer::AddTileCopy( builder, staticRef, compositeRef, restore );
	pass::shadows::ShadowAtlasTarget target{ compositeRef, work.atlasSize, work.guardTexels };
	target.keep = true;
	if ( ( !moverViews.empty() && !m_ShadowRenderer->AddPasses( builder, target, moverViews ) ) ||
	     !execute( std::move( builder ) ) )
		return {};
	m_ShadowTilesMoving.fetch_add( moverViews.size(), std::memory_order_relaxed );
	atlas.compositeUsage = ResourceUsage::kSampled;
	atlas.compositeHeld = work.views;
	atlas.moverTiles = std::move( moverTiles );
	atlas.compositeGeneration = casters->generation;
	atlas.compositeGuardTexels = work.guardTexels;
	atlas.sunFirst = work.sunFirst;
	atlas.sunCount = work.sunCount;
	if ( m_FrameAtlasIndices.empty() || m_FrameAtlasIndices.back() != m_AtlasNext )
		m_FrameAtlasIndices.push_back( m_AtlasNext );
	++m_AtlasNext;
	*desc = atlas.desc;
	return atlas.composite;
}

bool CoreWorld::EnsureOcclusion( device::IRenderDevice2 &device, device::CommandEncoder &encoder,
    std::uint32_t width, std::uint32_t height, device::CompletionToken submitted )
{
	using namespace render::device;
	BindStageDevice( device );
	if ( !m_Ao )
	{
		auto created = pass::ao::AmbientOcclusion::Create( device );
		if ( !created )
			return false;
		m_Ao = std::move( created ).Value();
	}
	if ( m_Occlusion.IsValid() && m_OcclusionDesc.width == width &&
	     m_OcclusionDesc.height == height )
		return true;
	// A resize: the old target goes behind the frames that used it.
	if ( m_Occlusion.IsValid() )
		(void)device.Release( m_Occlusion, submitted );
	m_OcclusionDesc = TextureDesc();
	m_OcclusionDesc.format = Format::kRGBA16Float;
	m_OcclusionDesc.width = width;
	m_OcclusionDesc.height = height;
	m_OcclusionDesc.usages = {
	    ResourceUsage::kStorageWrite, ResourceUsage::kSampled, ResourceUsage::kCopyDestination };
	m_OcclusionDesc.debugName = "stage ambient occlusion";
	auto texture = device.CreateTexture( m_OcclusionDesc );
	m_OcclusionDesc.debugName = {};
	if ( !texture )
	{
		m_Occlusion = TextureId();
		return false;
	}
	m_Occlusion = texture.Value();
	m_OcclusionNeutral = true; // cleared to one below
	// One (no occlusion) until a view records it.
	encoder.TransitionTexture(
	    m_Occlusion, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	encoder.ClearTexture( m_Occlusion, { 1, 1, 1, 1 } );
	encoder.TransitionTexture(
	    m_Occlusion, ResourceUsage::kCopyDestination, ResourceUsage::kSampled );
	return true;
}

void CoreWorld::ReleaseShadows( device::IRenderDevice2 &device )
{
	// Timers also exist for legacy BSP views that never bind a shadow device.
	// Release their borrowed device before the backend goes away, even then.
	{
		std::lock_guard<std::mutex> guard( m_TimersLock );
		if ( m_Timers && &m_Timers->Device() == &device )
			m_Timers.reset();
	}
	if ( m_ShadowDevice != &device )
		return;
	if ( m_Occlusion.IsValid() )
		(void)device.Release( m_Occlusion, device::CompletionToken() );
	m_Occlusion = device::TextureId();
	m_OcclusionDesc = device::TextureDesc();
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
	for ( const Atlas &atlas : m_Atlases )
	{
		(void)device.Release( atlas.texture, device::CompletionToken() );
		if ( atlas.composite.IsValid() )
			(void)device.Release( atlas.composite, device::CompletionToken() );
	}
	m_Atlases.clear();
	m_AtlasNext = 0;
	m_AtlasFrame = 0;
	m_FrameAtlasIndices.clear();
	m_CasterMeshes.reset();
	m_TriangleRevisions.clear();
	m_ClusterKernel.reset();
	m_ClusterFrame = 0;
	m_ShadowRenderer.reset();
	m_CastersStaged = 0;
	m_ShadowDevice = nullptr;
}

} // namespace render::composition
