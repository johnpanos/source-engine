//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.composition's world (RFC 0016 K5); see core_world.h.
//
//=============================================================================//

#include "core_world.h"

#include "mapcontainer/world_lightmap.h"
#include "mapcontainer/world_mesh_decode.h"
#include "render/graph/compiled_graph.h"
#include "render/graph/executor.h"
#include "render/graph/graph_builder.h"
#include "render/pass/lights/clusters.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <optional>

namespace render::composition
{

static_assert( pass::world::kWorldTag == legacy::kCorePassForwarded,
    "world tags are the frontend's forwarded tags" );
static_assert(
    sizeof( RenderCoreWorldVertex ) == sizeof( pass::world::WorldVertex ) &&
        offsetof( RenderCoreWorldVertex, lightmapUv ) ==
            offsetof( pass::world::WorldVertex, lightmapUv ) &&
        offsetof( RenderCoreWorldVertex, color ) == offsetof( pass::world::WorldVertex, color ) &&
        offsetof( RenderCoreWorldVertex, normal ) == offsetof( pass::world::WorldVertex, normal ) &&
        offsetof( RenderCoreWorldVertex, lightmapOffset ) ==
            offsetof( pass::world::WorldVertex, lightmapOffset ),
    "the engine's world vertex is the pass's" );

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

private:
	legacy::ICoreTextures &m_Textures;
};

} // namespace

void CoreWorld::SetWorld( const RenderCoreWorldVertex *vertices, unsigned int vertexCount,
    const unsigned int *indices, unsigned int indexCount, const RenderCoreWorldSurface *surfaces,
    unsigned int surfaceCount, const RenderCoreWorldMaterial *materials,
    unsigned int materialCount )
{
	pass::world::WorldData data;
	data.vertices.resize( vertexCount );
	for ( unsigned int i = 0; i < vertexCount; ++i )
	{
		pass::world::WorldVertex &out = data.vertices[i];
		std::copy( vertices[i].position, vertices[i].position + 3, out.position );
		std::copy( vertices[i].uv, vertices[i].uv + 2, out.uv );
		std::copy( vertices[i].lightmapUv, vertices[i].lightmapUv + 2, out.lightmapUv );
		std::copy( vertices[i].color, vertices[i].color + 4, out.color );
		std::copy( vertices[i].normal, vertices[i].normal + 3, out.normal );
		std::copy( vertices[i].tangentS, vertices[i].tangentS + 3, out.tangentS );
		std::copy( vertices[i].tangentT, vertices[i].tangentT + 3, out.tangentT );
		out.lightmapOffset = vertices[i].lightmapOffset;
	}
	data.indices.assign( indices, indices + indexCount );
	data.surfaces.reserve( surfaceCount );
	for ( unsigned int i = 0; i < surfaceCount; ++i )
	{
		pass::world::WorldSurface surface;
		surface.material = surfaces[i].material;
		surface.lightmapPage = m_Host && m_Host->lightmapPageHandle
		                           ? m_Host->lightmapPageHandle( surfaces[i].lightmapPage )
		                           : 0;
		surface.firstIndex = surfaces[i].firstIndex;
		surface.indexCount = surfaces[i].indexCount;
		data.surfaces.push_back( surface );
	}
	data.materials = WorldMaterials( materials, materialCount );
	m_StageSet = false;
	m_Pass.SetWorld( std::move( data ) );
}

std::vector<pass::world::WorldMaterial> CoreWorld::WorldMaterials(
    const RenderCoreWorldMaterial *materials, unsigned int materialCount ) const
{
	std::vector<pass::world::WorldMaterial> out;
	out.reserve( materialCount );
	for ( unsigned int i = 0; i < materialCount; ++i )
	{
		const RenderCoreWorldMaterial &source = materials[i];
		pass::world::WorldMaterial material;
		material.name = source.name ? source.name : "";
		material.shader = source.shader ? source.shader : "";
		for ( int v = 0; v < source.variableCount; ++v )
		{
			const char *key = source.keys[v] ? source.keys[v] : "";
			material.variables.emplace_back( key, source.values[v] ? source.values[v] : "" );
			if ( source.defaults && source.defaults[v] )
				material.defaults.emplace_back( key, source.defaults[v] );
			ITexture *texture = source.textures ? source.textures[v] : nullptr;
			if ( texture && m_Host && m_Host->textureHandle )
				material.textures.emplace_back( key, m_Host->textureHandle( texture ) );
		}
		out.push_back( std::move( material ) );
	}
	return out;
}

void CoreWorld::SetWorldMesh( const void *wmsh, unsigned long long wmshBytes,
    const RenderCoreWorldMeshlet *meshlets, unsigned int meshletCount,
    const RenderCoreWorldMaterial *materials, unsigned int materialCount )
{
	m_StageSet = false;
	m_StageWorld.reset();
	mapcontainer::WorldMeshData mesh;
	// A stage needs the map's mesh and its lightmap: without either the
	// core holds no world (every view draws legacy's), and says why.
	if ( !wmsh ||
	     mapcontainer::DecodeWorldMesh( wmsh, std::size_t( wmshBytes ), mesh ) !=
	         mapcontainer::WorldMeshError::Ok ||
	     m_Capture.lightmap.flat.empty() )
	{
		std::fprintf( stderr, "Render core: the world stage has no %s; the core draws no world\n",
		    m_Capture.lightmap.flat.empty() ? "lightmap (LMAP)" : "valid world mesh (WMSH)" );
		m_Pass.ClearWorld();
		return;
	}
	pass::world::WorldData data;
	// The world vertex: its normal and tangent frame, T = N x S times the
	// handedness (as render_lab builds it).
	data.vertices.resize( mesh.vertices.size() );
	for ( std::size_t i = 0; i < mesh.vertices.size(); ++i )
	{
		const mapcontainer::WorldMeshVertex &from = mesh.vertices[i];
		pass::world::WorldVertex &to = data.vertices[i];
		std::copy( from.position, from.position + 3, to.position );
		std::copy( from.uv, from.uv + 2, to.uv );
		std::copy( from.lightmapUv, from.lightmapUv + 2, to.lightmapUv );
		std::copy( from.normal, from.normal + 3, to.normal );
		std::copy( from.tangent, from.tangent + 3, to.tangentS );
		const float *n = from.normal;
		const float *t = from.tangent;
		to.tangentT[0] = ( n[1] * t[2] - n[2] * t[1] ) * from.tangentSign;
		to.tangentT[1] = ( n[2] * t[0] - n[0] * t[2] ) * from.tangentSign;
		to.tangentT[2] = ( n[0] * t[1] - n[1] * t[0] ) * from.tangentSign;
	}
	data.indices = std::move( mesh.indices );
	// The stage's shadow casters: its positions and triangles.
	auto casters = std::make_shared<Casters>();
	casters->positions.reserve( data.vertices.size() * 3 );
	for ( const pass::world::WorldVertex &vertex : data.vertices )
		casters->positions.insert( casters->positions.end(), vertex.position, vertex.position + 3 );
	casters->indices = data.indices;
	{
		std::lock_guard<std::mutex> guard( m_ShadowLock );
		casters->generation = ++m_CasterGeneration;
		m_Casters = std::move( casters );
		m_ShadowWork.clear();
	}
	data.surfaces.reserve( meshletCount );
	for ( unsigned int i = 0; i < meshletCount; ++i )
	{
		pass::world::WorldSurface surface;
		surface.material = meshlets[i].material;
		surface.firstIndex = meshlets[i].firstIndex;
		surface.indexCount = meshlets[i].indexCount;
		data.surfaces.push_back( surface );
	}
	data.materials = WorldMaterials( materials, materialCount );
	m_StageWorld = std::make_shared<const pass::world::WorldData>( std::move( data ) );
	SetStage();
}

void CoreWorld::SetStage()
{
	if ( !m_StageWorld )
		return;
	pass::world::WorldData data = *m_StageWorld;
	auto stage = std::make_shared<pass::world::WorldStage>();
	stage->lightmap = m_Capture.lightmap;
	if ( m_Capture.indirect.size() == m_Capture.lightmap.flat.size() )
		stage->indirect = m_Capture.indirect;
	stage->probes = m_Capture.probes;
	stage->reflectionWidth = m_Capture.reflectionWidth;
	stage->reflectionHeight = m_Capture.reflectionHeight;
	stage->reflectionProbes = m_Capture.reflection;
	std::fprintf( stderr,
	    "Render core: world stage: %zu meshlets, lightmap %ux%u%s%s, probes %s, reflection "
	    "probes %s\n",
	    data.surfaces.size(), stage->lightmap.width, stage->lightmap.height,
	    stage->lightmap.Directional() ? " directional" : "",
	    stage->indirect.empty() ? ", no indirect layer" : ", indirect layer",
	    stage->probes ? "yes" : "no", stage->reflectionProbes.empty() ? "no" : "yes" );
	data.stage = std::move( stage );
	m_Pass.SetWorld( std::move( data ) );
	m_StageSet = true;
}

bool CoreWorld::StageCapture::UploadLightmap(
    const world_mesh_gpu::WorldLightmapUploadRequest &request )
{
	// The total layer's pages (the baked diffuse light, as render_lab draws
	// it without runtime direct light) and the indirect layer's flat page.
	const std::size_t layerBytes = std::size_t( request.width ) * request.height * 8;
	pass::world::LightmapPages total;
	std::vector<std::byte> indirect;
	for ( std::uint32_t i = 0;
	    i < request.layerCount && i < world_mesh_gpu::kWorldLightmapMaxUploadLayers; ++i )
	{
		if ( !request.layers[i] )
			continue;
		const std::span<const std::byte> layer(
		    static_cast<const std::byte *>( request.layers[i] ), layerBytes );
		if ( request.roles[i] == world_mesh_gpu::WorldLightmapRole::Total )
			total = pass::world::SplitLightmapLayer( layer, request.width, request.height );
		else if ( request.roles[i] == world_mesh_gpu::WorldLightmapRole::Indirect )
			indirect = pass::world::SplitLightmapLayer( layer, request.width, request.height ).flat;
	}
	if ( total.flat.empty() )
		return false;
	// Recomposed while a stage draws: the pass updates its pages in place.
	if ( m_Owner.m_StageSet && !lightmap.flat.empty() )
	{
		lightmap = total;
		m_Owner.m_Pass.SetStageLightmap( std::move( total ) );
		return true;
	}
	lightmap = std::move( total );
	indirect.swap( this->indirect );
	return true;
}

bool CoreWorld::StageCapture::UploadProbeVolume(
    const world_mesh_gpu::ProbeVolumeUploadRequest &request )
{
	if ( !request.atlas || !request.gridTable || request.tableFloats % 4 != 0 )
		return false;
	pass::world::StageProbeVolume volume;
	volume.atlasWidth = request.atlasWidth;
	volume.atlasHeight = request.atlasHeight;
	volume.tableTexels = request.tableFloats / 4;
	volume.rows = request.gridCount + request.occluderCount;
	volume.table.assign(
	    request.gridTable, request.gridTable + std::size_t( volume.rows ) * request.tableFloats );
	const std::size_t atlasBytes = std::size_t( request.atlasWidth ) * request.atlasHeight * 8;
	// The stage's probe atlas is the first volume published (a traced
	// producer publishes a change from its first frame, so the bake alone may
	// never come): a world surface reads the lightmap and adds the change
	// (kSurfaceProbeBounce); the atlas itself lights only surfaces without a
	// lightmap.
	std::vector<std::byte> change;
	if ( request.deltaAtlas )
	{
		const std::byte *delta = static_cast<const std::byte *>( request.deltaAtlas );
		change.assign( delta, delta + atlasBytes );
	}
	if ( !probes || probes->atlas.size() != atlasBytes )
	{
		const bool late = m_Owner.m_StageSet;
		const std::byte *atlas = static_cast<const std::byte *>( request.atlas );
		volume.atlas.assign( atlas, atlas + atlasBytes );
		// The stage's own table has the grids' rows alone.
		pass::world::StageProbeVolume first = volume;
		first.rows = request.gridCount;
		first.table.resize( std::size_t( first.rows ) * request.tableFloats );
		probes = std::move( first );
		volume.atlas.clear();
		// The stage was set before its first volume arrived: set it again
		// with it, then take this change.
		if ( late )
			m_Owner.SetStage();
	}
	if ( m_Owner.m_StageSet )
		m_Owner.m_Pass.SetStageChange( std::move( change ), std::move( volume ) );
	return true;
}

bool CoreWorld::StageCapture::UploadReflectionProbes(
    const world_mesh_gpu::ReflectionProbesUploadRequest &request )
{
	reflectionWidth = request.texels ? request.width : 0;
	reflectionHeight = request.texels ? request.height : 0;
	reflection.clear();
	if ( request.texels )
	{
		const std::byte *texels = reinterpret_cast<const std::byte *>( request.texels );
		reflection.assign( texels, texels + std::size_t( request.width ) * request.height * 8 );
	}
	return true;
}

void CoreWorld::StageCapture::Release()
{
	lightmap = pass::world::LightmapPages();
	indirect.clear();
	probes.reset();
	reflectionWidth = reflectionHeight = 0;
	reflection.clear();
}

std::shared_ptr<const pass::world::StageViewLights> CoreWorld::StageViewLightsFor(
    const float worldToView[16], const float viewToClip[16], const float viewport[6],
    std::shared_ptr<const ShadowWork> *shadows ) const
{
	*shadows = nullptr;
	if ( !worldToView || !viewToClip || viewport[2] < 1.0f || viewport[3] < 1.0f )
		return nullptr;
	auto matrix = []( const float m[16] )
	{
		math::float4x4 out;
		for ( int r = 0; r < 4; ++r )
			out.rows[r] = { m[r * 4 + 0], m[r * 4 + 1], m[r * 4 + 2], m[r * 4 + 3] };
		return out;
	};
	pass::lights::ClusterViewDesc desc;
	desc.view = matrix( worldToView );
	desc.projection = matrix( viewToClip );
	desc.widthPixels = std::uint32_t( viewport[2] );
	desc.heightPixels = std::uint32_t( viewport[3] );
	// The depth range from the projection (depth 0 at the near plane, 1 at
	// the far one, clip w = -view z): near = m23 / m22, far = m23 / (m22 + 1).
	const float a = viewToClip[2 * 4 + 2];
	const float b = viewToClip[2 * 4 + 3];
	desc.nearZ = a != 0.0f ? b / a : 0.0f;
	desc.farZ = a + 1.0f != 0.0f ? b / ( a + 1.0f ) : 0.0f;
	if ( !std::isfinite( desc.farZ ) || desc.farZ <= desc.nearZ )
		desc.farZ = 65536.0f;
	const pass::lights::ClusterLimits limits = pass::lights::DesktopClusterLimits();
	auto grid = pass::lights::CreateClusterGrid( desc, limits );
	if ( !grid )
		return nullptr;
	pass::lights::ClusterLists lists;
	if ( !pass::lights::AssignLights( grid.Value(), m_Lights.lights, lists ) )
		return nullptr;
	auto out = std::make_shared<pass::world::StageViewLights>();
	out->view.grid[0] = grid.Value().tilesX;
	out->view.grid[1] = grid.Value().tilesY;
	out->view.grid[2] = grid.Value().slices;
	out->view.grid[3] = limits.tileSizePixels;
	out->view.slices[0] = grid.Value().sliceScale;
	out->view.slices[1] = grid.Value().sliceBias;
	out->view.slices[2] = grid.Value().nearZ;
	const math::float4 &z = desc.view.rows[2];
	out->view.viewDistance[0] = -z.x;
	out->view.viewDistance[1] = -z.y;
	out->view.viewDistance[2] = -z.z;
	out->view.viewDistance[3] = -z.w;
	out->froxels.resize( lists.froxels.size() * sizeof( pass::lights::FroxelRange ) );
	std::memcpy( out->froxels.data(), lists.froxels.data(), out->froxels.size() );
	out->indices.assign( 16, std::byte( 0 ) ); // ClusterIndexHeader
	const auto listed = std::as_bytes( std::span( lists.lightIndices ) );
	out->indices.insert( out->indices.end(), listed.begin(), listed.end() );
	// The shadows of the view's point and spot lights (render.shadows.v1:
	// one tile per spot, six per point light), planned for this view.
	std::vector<light_set::RuntimeLight> shadowed;
	std::vector<int> shadowedOf( m_Lights.lights.size(), -1 );
	for ( std::size_t i = 0; i < m_Lights.lights.size(); ++i )
	{
		const light_set::RuntimeLight &light = m_Lights.lights[i];
		if ( light.shape == light_set::LightShape::Point ||
		     light.shape == light_set::LightShape::Spot )
		{
			shadowedOf[i] = int( shadowed.size() );
			shadowed.push_back( light );
		}
	}
	pass::shadows::ShadowPlan plan;
	if ( !shadowed.empty() )
	{
		pass::shadows::ShadowPlanInput input;
		input.lights = shadowed;
		input.camera.view = desc.view;
		input.atlasSize = kStageShadowAtlas;
		input.guardTexels = 4;
		if ( pass::shadows::PlanShadows( input, plan ) )
			plan = pass::shadows::ShadowPlan(); // refused: unshadowed
	}
	// A baked light's diffuse light is in the lightmap: the core adds its
	// specular lobe alone (each light counts once per surface).
	auto work = std::make_shared<ShadowWork>();
	work->view = desc.view;
	// The projection the world pass rasterizes with: D3D9 pixel centers, a
	// half pixel right and down (render.pass.world), so the screen passes
	// reconstruct each pixel where it was drawn.
	work->projection = desc.projection;
	if ( const auto fromView = math::Inverse( desc.view ) )
	{
		work->eye[0] = fromView->rows[0].w;
		work->eye[1] = fromView->rows[1].w;
		work->eye[2] = fromView->rows[2].w;
	}
	for ( int c = 0; c < 4; ++c )
	{
		const float w = ( &desc.projection.rows[3].x )[c];
		( &work->projection.rows[0].x )[c] += w / viewport[2];
		( &work->projection.rows[1].x )[c] -= w / viewport[3];
	}
	for ( std::size_t i = 0; i < m_Lights.lights.size(); ++i )
	{
		const light_set::RuntimeLight &light = m_Lights.lights[i];
		const int k = shadowedOf[i];
		const int tile =
		    k >= 0 && std::size_t( k ) < plan.lightTiles.size() ? plan.lightTiles[k] : -1;
		const int tiles = tile >= 0 ? plan.lightTileCount[k] : 1;
		out->lights.push_back( material::PackSurfaceLight( light, tile, tiles, light.baked ) );
	}
	// The view group's light records: at least one (an empty view's lists
	// name none of them).
	if ( out->lights.empty() )
		out->lights.emplace_back();
	if ( !plan.views.empty() )
	{
		out->shadowTiles = std::move( plan.tiles );
		work->views = std::move( plan.views );
		work->atlasSize = plan.atlasSize;
		work->guardTexels = plan.guardTexels;
	}
	*shadows = std::move( work );
	return out;
}

bool CoreWorld::DrawView( const unsigned int *surfaces, unsigned int count,
    const float worldToClip[16], const float viewport[6], unsigned long long hostFrame,
    const float worldToView[16], const float viewToClip[16] )
{
	legacy::ICorePassSlots *slots = m_Frontend.CorePassSlots();
	if ( !slots || count == 0 )
		return false;
	pass::world::WorldView view;
	view.surfaces.assign( surfaces, surfaces + count );
	std::memcpy( view.toClip, worldToClip, sizeof( view.toClip ) );
	view.viewport = {
	    viewport[0], viewport[1], viewport[2], viewport[3], viewport[4], viewport[5] };
	view.hostFrame = hostFrame;
	view.debug = m_Renderer.AppliedDebug();
	std::shared_ptr<const ShadowWork> shadows;
	if ( m_StageSet )
	{
		view.lights = StageViewLightsFor( worldToView, viewToClip, viewport, &shadows );
		m_StageLitViews += view.lights ? 1 : 0;
	}
	const std::uint32_t tag = m_Pass.QueueView( std::move( view ) );
	if ( tag == 0 )
		return false;
	if ( shadows )
	{
		std::lock_guard<std::mutex> guard( m_ShadowLock );
		m_ShadowWork.emplace_back( tag, std::move( shadows ) );
		while ( m_ShadowWork.size() > 64 ) // a frame's views, re-recorded ones included
			m_ShadowWork.pop_front();
	}
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

void CoreWorld::EndFrame()
{
	if ( m_Renderer.AppliedDebug().legacy != frame::DebugLegacy::kTint )
		return;
	if ( legacy::ICorePassSlots *slots = m_Frontend.CorePassSlots() )
		slots->MarkSlot( legacy::kCorePassForwarded | legacy::kCorePassFrameEnd );
}

void CoreWorld::BeginFrame()
{
	const frame::DebugControls &debug = m_Renderer.AppliedDebug();
	if ( !frame::PixelViewActive( debug ) && debug.legacy != frame::DebugLegacy::kSkip )
		return;
	if ( legacy::ICorePassSlots *slots = m_Frontend.CorePassSlots() )
		slots->MarkSlot( legacy::kCorePassForwarded | legacy::kCorePassLegacyOff );
}

unsigned long long CoreWorld::Failures() const
{
	return m_Pass.Failures();
}

void CoreWorld::GetStats( RenderCoreWorldStats *out ) const
{
	if ( !out )
		return;
	const pass::world::WorldStats stats = m_Pass.Stats();
	*out = RenderCoreWorldStats{};
	out->materials = stats.materials;
	out->claimedMaterials = stats.claimedMaterials;
	out->surfaces = stats.surfaces;
	out->claimedSurfaces = stats.claimedSurfaces;
	out->viewsQueued = stats.viewsQueued;
	out->viewsDrawn = stats.viewsDrawn;
	out->viewsFailed = stats.viewsFailed;
	out->viewsSkipped = stats.viewsSkipped;
	out->surfacesDrawn = stats.surfacesDrawn;
	out->debugHatches = m_Hatches.load( std::memory_order_relaxed );
	out->debugTints = m_Tints.load( std::memory_order_relaxed );
	out->debugViewsRedrawn = m_Redrawn.load( std::memory_order_relaxed );
	out->stageLights = unsigned( m_Lights.lights.size() );
	out->stageLitViews = m_StageLitViews;
	std::snprintf( out->lastFailure, sizeof( out->lastFailure ), "%s", stats.lastFailure.c_str() );
	std::size_t used = 0;
	for ( const auto &[reason, count] : stats.gaps )
	{
		if ( used + 1 >= sizeof( out->gaps ) )
			break;
		const int written = std::snprintf(
		    out->gaps + used, sizeof( out->gaps ) - used, "%u %s\n", count, reason.c_str() );
		if ( written < 0 )
			break;
		used = std::min( sizeof( out->gaps ) - 1, used + std::size_t( written ) );
	}
	used = 0;
	for ( const auto &[name, count] : stats.claimed )
	{
		if ( used + 1 >= sizeof( out->claimed ) )
			break;
		const int written = std::snprintf(
		    out->claimed + used, sizeof( out->claimed ) - used, "%u %s\n", count, name.c_str() );
		if ( written < 0 )
			break;
		used = std::min( sizeof( out->claimed ) - 1, used + std::size_t( written ) );
	}
}

void CoreWorld::RecordSlot(
    std::uint32_t tag, device::CommandEncoder &encoder, const legacy::CorePassTarget &target )
{
	if ( tag & legacy::kCorePassFrameEnd )
	{
		// cl_render_debug_legacy 1: magenta over the frame, then the frame's
		// top-level world views again; their depth test brings back exactly
		// the pixels where the core's surface is still the one seen. What
		// stays tinted is what the core did not draw (or legacy drew over).
		pass::debug::HatchTarget whole;
		whole.encodeOutput = !target.colorSrgb.IsValid();
		whole.color = whole.encodeOutput ? target.color : target.colorSrgb;
		whole.format = whole.encodeOutput ? target.colorFormat : target.colorSrgbFormat;
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
		hatch.encodeOutput = !target.colorSrgb.IsValid();
		hatch.color = hatch.encodeOutput ? target.color : target.colorSrgb;
		hatch.format = hatch.encodeOutput ? target.colorFormat : target.colorSrgbFormat;
		hatch.width = target.width;
		hatch.height = target.height;
		hatch.samples = target.samples;
		if ( target.device && m_Overlays.RecordHatch( *target.device, encoder, hatch ) )
			m_Hatches.fetch_add( 1, std::memory_order_relaxed );
		return;
	}
	std::optional<Textures> textures;
	if ( target.textures )
		textures.emplace( *target.textures );
	pass::world::WorldTarget world;
	world.device = target.device;
	// The sRGB view when the target has one; else the unorm view, and the
	// shader encodes (the same curve, the output encoding frame term).
	world.encodeOutput = !target.colorSrgb.IsValid();
	world.color = world.encodeOutput ? target.color : target.colorSrgb;
	world.colorFormat = world.encodeOutput ? target.colorFormat : target.colorSrgbFormat;
	world.depth = target.depth;
	world.depthFormat = target.depthFormat;
	world.width = target.width;
	world.height = target.height;
	world.samples = target.samples;
	world.textures = textures ? &*textures : nullptr;
	world.submitted = target.submitted;
	world.frame = target.frame;
	world.lightmapScale = target.lightmapScale;
	world.outputScale = target.outputScale;
	std::copy( target.eye, target.eye + 3, world.eye );
	world.envmapScale = target.envmapScale;
	world.specular = target.specular;
	world.ssbumpNormalized = target.ssbumpNormalized;
	world.fogType = target.fog.type;
	std::copy( target.fog.color, target.fog.color + 3, world.fogColor );
	std::copy( target.fog.params, target.fog.params + 4, world.fogParams );
	world.fogEyeZ = target.fog.eyeZ;
	std::shared_ptr<const ShadowWork> shadows;
	{
		std::lock_guard<std::mutex> guard( m_ShadowLock );
		for ( const auto &[queued, work] : m_ShadowWork )
		{
			if ( queued == tag )
				shadows = work;
		}
	}
	if ( shadows && target.device && !shadows->views.empty() )
		world.shadowAtlas =
		    DrawStageShadows( *target.device, *shadows, target.frame, &world.shadowAtlasDesc );
	// The stage view's screen passes: GTAO over the pass's prepass.
	if ( shadows && target.device &&
	     EnsureOcclusion( *target.device, encoder, target.width, target.height, target.submitted ) )
	{
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
			return bool( m_Ao->Record( screen, targets, aoView ) );
		};
	}
	m_Pass.Record( tag, encoder, world );
	bool topLevel = false;
	{
		std::lock_guard<std::mutex> guard( m_TopLevelLock );
		topLevel = m_TopLevel.count( tag ) != 0;
	}
	if ( topLevel )
	{
		auto &views = m_FrameViews[target.frame];
		const auto entry = std::make_pair( tag, target.color );
		if ( std::find( views.begin(), views.end(), entry ) == views.end() && views.size() < 64 )
			views.push_back( entry );
	}
}

device::TextureId CoreWorld::DrawStageShadows( device::IRenderDevice2 &device,
    const ShadowWork &work, std::uint64_t frame, device::TextureDesc *desc )
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
	}
	if ( m_AtlasNext == m_Atlases.size() )
	{
		Atlas atlas;
		atlas.desc.format = Format::kD32Float;
		atlas.desc.width = atlas.desc.height = work.atlasSize;
		atlas.desc.usages = { ResourceUsage::kDepthWrite, ResourceUsage::kSampled };
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
		return {};
	const pass::shadows::ShadowCaster caster[] = { { *mesh, math::float4x4::Identity() } };
	std::vector<pass::shadows::ShadowDepthView> views;
	views.reserve( work.views.size() );
	for ( const pass::shadows::ShadowPlanView &view : work.views )
		views.push_back( { view.viewProjection, view.tile, caster } );
	graph::GraphBuilder builder;
	builder.AddPass( "stage caster uploads", graph::PassKind::kCopy )
	    .SideEffect()
	    .Execute(
	        [this]( graph::RecordContext &context )
	        {
		        m_CasterMeshes->RecordUploads( context.Encoder() );
	        } );
	const graph::ResourceRef atlasRef = builder.ImportTexture(
	    "stage shadow atlas", atlas.texture, atlas.desc, atlas.usage, ResourceUsage::kSampled );
	if ( !m_ShadowRenderer->AddPasses(
	         builder, { atlasRef, work.atlasSize, work.guardTexels }, views ) )
		return {};
	auto compiled = graph::CompileGraph( std::move( builder ) );
	if ( !compiled )
		return {};
	graph::SerialGraphExecutor executor;
	auto executed = executor.Execute( compiled.Value(), device );
	if ( !executed )
		return {};
	m_ShadowRenderer->Collect( executed.Value().token );
	m_CasterMeshes->Retire( executed.Value().token );
	atlas.usage = ResourceUsage::kSampled;
	++m_AtlasNext;
	*desc = atlas.desc;
	return atlas.texture;
}

void CoreWorld::BindStageDevice( device::IRenderDevice2 &device )
{
	if ( m_ShadowDevice == &device )
		return;
	// A new backend device: the old one's objects went with it.
	m_ShadowRenderer.reset();
	m_CasterMeshes.reset();
	m_Atlases.clear();
	m_CastersStaged = 0;
	m_Ao.reset();
	m_Occlusion = device::TextureId();
	m_OcclusionDesc = device::TextureDesc();
	m_ShadowDevice = &device;
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
	if ( m_ShadowDevice != &device )
		return;
	if ( m_Occlusion.IsValid() )
		(void)device.Release( m_Occlusion, device::CompletionToken() );
	m_Occlusion = device::TextureId();
	m_OcclusionDesc = device::TextureDesc();
	m_Ao.reset();
	for ( const Atlas &atlas : m_Atlases )
		(void)device.Release( atlas.texture, device::CompletionToken() );
	m_Atlases.clear();
	m_CasterMeshes.reset();
	m_ShadowRenderer.reset();
	m_CastersStaged = 0;
	m_ShadowDevice = nullptr;
}

} // namespace render::composition
