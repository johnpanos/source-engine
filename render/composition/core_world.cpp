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
#include <array>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <limits>
#include <map>
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
    const RenderCoreWorldMaterial *materials, unsigned int materialCount, const char *entities )
{
	m_StageSet = false;
	m_StageWorld.reset();
	m_MapLights = pass::lights::MapLights();
	if ( entities )
	{
		if ( const auto parsed = pass::lights::ParseEntityLump( entities ) )
			m_MapLights = pass::lights::MapLightsFromEntities( *parsed );
		else
			std::fprintf( stderr, "Render core: the world stage's entity lump does not parse\n" );
	}
	std::fprintf( stderr,
	    "Render core: world stage's authored lights: %zu lights, %zu area lights, sun %s, %zu "
	    "projectors, %u unsupported\n",
	    m_MapLights.lights.size(), m_MapLights.areas.size(), m_MapLights.sun ? "yes" : "no",
	    m_MapLights.projectors.size(), m_MapLights.unsupported );
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
	// The stage's shadow casters: its positions and triangles, grouped into
	// chunks of kCasterCell units by their centroids.
	auto casters = std::make_shared<Casters>();
	casters->positions.reserve( data.vertices.size() * 3 );
	for ( const pass::world::WorldVertex &vertex : data.vertices )
		casters->positions.insert( casters->positions.end(), vertex.position, vertex.position + 3 );
	{
		constexpr float kCasterCell = 512.0f;
		std::map<std::array<int, 3>, std::vector<std::uint32_t>> cells;
		const std::vector<float> &p = casters->positions;
		for ( std::size_t t = 0; t + 2 < data.indices.size(); t += 3 )
		{
			std::array<int, 3> cell{};
			for ( int a = 0; a < 3; ++a )
			{
				const float centroid =
				    ( p[data.indices[t] * 3 + a] + p[data.indices[t + 1] * 3 + a] +
				        p[data.indices[t + 2] * 3 + a] ) /
				    3.0f;
				cell[a] = int( std::floor( centroid / kCasterCell ) );
			}
			auto &triangles = cells[cell];
			triangles.insert( triangles.end(), data.indices.begin() + std::ptrdiff_t( t ),
			    data.indices.begin() + std::ptrdiff_t( t + 3 ) );
		}
		casters->indices.reserve( data.indices.size() );
		for ( const auto &[cell, triangles] : cells )
		{
			Casters::Chunk chunk;
			chunk.firstIndex = std::uint32_t( casters->indices.size() );
			chunk.indexCount = std::uint32_t( triangles.size() );
			for ( int a = 0; a < 3; ++a )
			{
				chunk.min[a] = std::numeric_limits<float>::max();
				chunk.max[a] = -std::numeric_limits<float>::max();
			}
			for ( std::uint32_t index : triangles )
			{
				for ( int a = 0; a < 3; ++a )
				{
					chunk.min[a] = std::min( chunk.min[a], p[index * 3 + a] );
					chunk.max[a] = std::max( chunk.max[a], p[index * 3 + a] );
				}
			}
			casters->indices.insert( casters->indices.end(), triangles.begin(), triangles.end() );
			casters->chunks.push_back( chunk );
		}
	}
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
	// The sun's baked visibility packed in the total page's alpha: any texel
	// below one (render_lab's rule).
	m_StageSunMask = false;
	const std::vector<std::byte> &flat = m_Capture.lightmap.flat;
	for ( std::size_t t = 0; t + 8 <= flat.size() && !m_StageSunMask; t += 8 )
	{
		std::uint16_t alpha;
		std::memcpy( &alpha, flat.data() + t + 6, sizeof( alpha ) );
		// Half 0.999 is 0x3BFE; any alpha below it (positive halves order as
		// integers).
		m_StageSunMask = ( alpha & 0x8000u ) == 0 && alpha < 0x3BFEu;
	}
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
	// The change the capture holds (parts arrive relative to it).
	if ( m_Capture.table )
		m_Pass.SetStageChange( m_Capture.change, *m_Capture.table );
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
	if ( !request.gridTable || request.tableFloats % 4 != 0 )
		return false;
	pass::world::StageProbeVolume volume;
	volume.atlasWidth = request.atlasWidth;
	volume.atlasHeight = request.atlasHeight;
	volume.tableTexels = request.tableFloats / 4;
	volume.rows = request.gridCount + request.occluderCount;
	volume.table.assign(
	    request.gridTable, request.gridTable + std::size_t( volume.rows ) * request.tableFloats );
	const std::size_t atlasBytes = std::size_t( request.atlasWidth ) * request.atlasHeight * 8;
	if ( request.regions )
	{
		// A part of the change: applied to the kept change, and passed on as
		// regions. Before any whole volume there is nothing to apply it to.
		if ( !probes || probes->atlas.size() != atlasBytes )
			return true;
		std::vector<pass::world::WorldPass::StageRegion> regions;
		std::vector<std::byte> texels;
		if ( request.regionDelta )
		{
			const std::byte *packed = static_cast<const std::byte *>( request.regionDelta );
			if ( change.size() != atlasBytes )
				change.assign( atlasBytes, std::byte( 0 ) );
			std::size_t offset = 0;
			for ( std::uint32_t i = 0; i < request.regionCount; ++i )
			{
				const world_mesh_gpu::ProbeAtlasRegion &region = request.regions[i];
				if ( region.x + region.width > request.atlasWidth ||
				     region.y + region.height > request.atlasHeight )
					return false;
				regions.push_back( { region.x, region.y, region.width, region.height } );
				const std::size_t row = std::size_t( region.width ) * 8;
				for ( std::uint32_t y = 0; y < region.height; ++y )
				{
					std::memcpy( change.data() +
					                 ( std::size_t( region.y + y ) * request.atlasWidth + region.x ) *
					                     8,
					    packed + offset, row );
					offset += row;
				}
			}
			texels.assign( packed, packed + offset );
		}
		table = volume;
		if ( m_Owner.m_StageSet )
			m_Owner.m_Pass.SetStageChangeRegions(
			    std::move( regions ), std::move( texels ), std::move( volume ) );
		return true;
	}
	if ( !request.atlas )
		return false;
	// The stage's probe atlas is the first volume published (a traced
	// producer publishes a change from its first frame, so the bake alone may
	// never come): a world surface reads the lightmap and adds the change
	// (kSurfaceProbeBounce); the atlas itself lights only surfaces without a
	// lightmap.
	change.clear();
	if ( request.deltaAtlas )
	{
		const std::byte *delta = static_cast<const std::byte *>( request.deltaAtlas );
		change.assign( delta, delta + atlasBytes );
	}
	table = volume;
	if ( !probes || probes->atlas.size() != atlasBytes )
	{
		const bool late = m_Owner.m_StageSet;
		const std::byte *atlas = static_cast<const std::byte *>( request.atlas );
		// The stage's own table has the grids' rows alone.
		pass::world::StageProbeVolume first = volume;
		first.atlas.assign( atlas, atlas + atlasBytes );
		first.rows = request.gridCount;
		first.table.resize( std::size_t( first.rows ) * request.tableFloats );
		probes = std::move( first );
		// The stage was set before its first volume arrived: set it again
		// with it (SetStage passes the change on).
		if ( late )
		{
			m_Owner.SetStage();
			return true;
		}
	}
	if ( m_Owner.m_StageSet )
		m_Owner.m_Pass.SetStageChange( change, std::move( volume ) );
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
	change.clear();
	table.reset();
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
	// The frame's runtime lights, and the map's authored ones when the frame
	// has no world lights (a map compiled without vrad): each light once.
	std::vector<light_set::RuntimeLight> frameLights = m_Lights.lights;
	const bool worldLights = std::any_of( frameLights.begin(), frameLights.end(),
	    []( const light_set::RuntimeLight &light )
	    {
		    return light.kind == light_set::LightKind::World;
	    } );
	if ( !worldLights )
	{
		std::uint32_t nextId = 0x40000000u; // the map's own ids, apart from the engine's
		for ( light_set::RuntimeLight light : m_MapLights.lights )
		{
			light.id = nextId++;
			frameLights.push_back( light );
		}
	}
	pass::lights::ClusterLists lists;
	if ( !pass::lights::AssignLights( grid.Value(), frameLights, lists ) )
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
	// Only the lights the view's clusters list reach what it draws; the
	// others need no shadow for this view.
	std::vector<char> reaches( frameLights.size(), 0 );
	for ( std::uint32_t index : lists.lightIndices )
	{
		if ( index < reaches.size() )
			reaches[index] = 1;
	}
	std::vector<light_set::RuntimeLight> shadowed;
	std::vector<int> shadowedOf( frameLights.size(), -1 );
	for ( std::size_t i = 0; i < frameLights.size(); ++i )
	{
		const light_set::RuntimeLight &light = frameLights[i];
		if ( reaches[i] && ( light.shape == light_set::LightShape::Point ||
		                       light.shape == light_set::LightShape::Spot ) )
		{
			shadowedOf[i] = int( shadowed.size() );
			shadowed.push_back( light );
		}
	}
	// The area lights: the map's (baked light fixtures) and the frame's
	// emitting surfaces; the sun: the map's.
	const std::vector<area_light::AreaLight> areas = ViewAreaLights( true );
	const std::optional<pass::lights::MapSun> &sun = m_MapLights.sun;
	pass::shadows::ShadowPlan plan;
	if ( ( !shadowed.empty() || !areas.empty() || sun ) &&
	     m_ShadowQuality.load( std::memory_order_relaxed ) > 0 )
	{
		pass::shadows::ShadowPlanInput input;
		input.lights = shadowed;
		input.areas = areas;
		if ( sun )
			input.toSun = sun->toSun;
		input.camera.view = desc.view;
		// The cascades cover the view's frustum: its vertical field of view
		// and aspect from the projection's scales.
		const float yScale = desc.projection.rows[1].y;
		const float xScale = desc.projection.rows[0].x;
		if ( yScale > 0.0f && xScale > 0.0f )
		{
			input.camera.verticalFovRadians = 2.0f * std::atan( 1.0f / yScale );
			input.camera.aspect = yScale / xScale;
		}
		input.camera.nearZ = desc.nearZ;
		input.atlasSize = ShadowAtlasFor( m_ShadowQuality.load( std::memory_order_relaxed ) );
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
	for ( std::size_t i = 0; i < frameLights.size(); ++i )
	{
		const light_set::RuntimeLight &light = frameLights[i];
		const int k = shadowedOf[i];
		const int tile =
		    k >= 0 && std::size_t( k ) < plan.lightTiles.size() ? plan.lightTiles[k] : -1;
		const int tiles = tile >= 0 ? plan.lightTileCount[k] : 1;
		out->lights.push_back( material::PackSurfaceLight( light, tile, tiles, light.baked ) );
	}
	// The area lights and the sun, with their tiles.
	PackViewAreaLights(
	    areas, std::min( m_MapLights.areas.size(), areas.size() ), plan.areaTiles, *out );
	if ( sun )
	{
		out->sunDirection[0] = sun->toSun.x;
		out->sunDirection[1] = sun->toSun.y;
		out->sunDirection[2] = sun->toSun.z;
		out->sunDirection[3] =
		    float( std::tan( 0.5 * double( sun->spreadDegrees ) * 3.14159265358979 / 180.0 ) );
		out->sunColor[0] = sun->color.x;
		out->sunColor[1] = sun->color.y;
		out->sunColor[2] = sun->color.z;
		out->sunColor[3] = 1.0f;
		out->sunShadow[0] = float( plan.sunFirst );
		out->sunShadow[1] = float( plan.sunCount );
		out->sunShadow[2] = m_StageSunMask ? 1.0f : 0.0f;
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

std::vector<area_light::AreaLight> CoreWorld::ViewAreaLights( bool withMapAreas ) const
{
	std::vector<area_light::AreaLight> areas;
	if ( withMapAreas )
		areas = m_MapLights.areas;
	for ( const light_set::RuntimeAreaLight &area : m_Lights.areas )
		areas.push_back( area.light );
	if ( areas.size() > std::size_t( material::kSurfaceMaxAreaLights ) )
		areas.resize( std::size_t( material::kSurfaceMaxAreaLights ) );
	return areas;
}

void CoreWorld::PackViewAreaLights( const std::vector<area_light::AreaLight> &areas,
    std::size_t mapAreas, const std::vector<int> &areaTiles,
    pass::world::StageViewLights &out ) const
{
	// A map's light fixture is in the bake (the surface program adds its
	// specular alone); an emitting surface is not: the engine leaves the
	// surfaces the core draws out of its lightmap (area_lights.h), and a
	// stage's lightmap never held it.
	for ( std::size_t i = 0; i < areas.size(); ++i )
		out.areas.push_back( material::PackAreaLight(
		    areas[i], i < mapAreas, i < areaTiles.size() ? areaTiles[i] : -1 ) );
}

std::shared_ptr<const pass::world::StageViewLights> CoreWorld::AreaViewLights() const
{
	const std::vector<area_light::AreaLight> areas = ViewAreaLights( false );
	if ( areas.empty() )
		return nullptr;
	auto out = std::make_shared<pass::world::StageViewLights>();
	PackViewAreaLights( areas, 0, {}, *out );
	return out;
}

bool CoreWorld::DrawView( const unsigned int *surfaces, unsigned int count,
    const float worldToClip[16], const float viewport[6], unsigned long long hostFrame,
    const float worldToView[16], const float viewToClip[16], float waterZOffset )
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
	std::shared_ptr<const ShadowWork> shadows;
	if ( m_StageSet )
	{
		view.lights = StageViewLightsFor( worldToView, viewToClip, viewport, &shadows );
		m_StageLitViews += view.lights ? 1 : 0;
	}
	else
	{
		view.lights = AreaViewLights();
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

void CoreWorld::SetQuality( const RenderCoreWorldQuality &quality )
{
	m_AoQuality.store( std::clamp( quality.ambientOcclusion, 0, 4 ), std::memory_order_relaxed );
	m_ShadowQuality.store( std::clamp( quality.shadows, 0, 3 ), std::memory_order_relaxed );
}

void CoreWorld::SetGpuTimers( bool enabled )
{
	m_GpuTimersOn.store( enabled, std::memory_order_relaxed );
}

unsigned int CoreWorld::TakeGpuTimes( char *out, unsigned int size )
{
	if ( !out || size == 0 )
		return 0;
	out[0] = '\0';
	graph::PassTimerReport report;
	{
		std::lock_guard<std::mutex> guard( m_TimersLock );
		if ( !m_Timers )
			return 0;
		report = m_Timers->Take();
	}
	if ( report.frames == 0 )
		return 0;
	std::size_t used = 0;
	const double frames = double( report.frames );
	for ( const graph::PassTime &pass : report.passes )
	{
		if ( used + 1 >= size )
			break;
		const int written = std::snprintf( out + used, size - used, "%u %.3f %.2f %s\n", pass.depth,
		    pass.milliseconds / frames, double( pass.count ) / frames, pass.name.c_str() );
		if ( written < 0 )
			break;
		used = std::min( std::size_t( size ) - 1, used + std::size_t( written ) );
	}
	// The same views' CPU recording (shadows, prepass, lit world, GTAO).
	const std::uint64_t recordNs = m_RecordNs.exchange( 0, std::memory_order_relaxed );
	const std::uint64_t recordViews = m_RecordViews.exchange( 0, std::memory_order_relaxed );
	if ( recordViews && used + 1 < size )
	{
		const int written = std::snprintf( out + used, size - used,
		    "0 %.3f %.2f core world view (CPU recording, render sequence)\n",
		    double( recordNs ) * 1e-6 / frames, double( recordViews ) / frames );
		if ( written > 0 )
			used = std::min( std::size_t( size ) - 1, used + std::size_t( written ) );
	}
	// The shadow tiles the views drew, and kept from earlier frames (CPU
	// counts; the time is the shadow-depth passes').
	const std::uint64_t tilesDrawn = m_ShadowTilesDrawn.exchange( 0, std::memory_order_relaxed );
	const std::uint64_t tilesKept = m_ShadowTilesKept.exchange( 0, std::memory_order_relaxed );
	if ( tilesDrawn + tilesKept && used + 1 < size )
	{
		const int written = std::snprintf( out + used, size - used,
		    "0 0 %.2f shadow tiles drawn (count; %.2f kept per frame)\n",
		    double( tilesDrawn ) / frames, double( tilesKept ) / frames );
		if ( written > 0 )
			used = std::min( std::size_t( size ) - 1, used + std::size_t( written ) );
	}
	if ( report.overflowed && used + 1 < size )
		std::snprintf(
		    out + used, size - used, "0 0 %u (timestamps dropped)\n", report.overflowed );
	return report.frames;
}

graph::GpuPassTimers *CoreWorld::SlotTimers( const legacy::CorePassTarget &target )
{
	std::lock_guard<std::mutex> guard( m_TimersLock );
	// One decision per frame: the frame's slots share its encoders.
	if ( target.frame != m_TimersFrame )
	{
		m_TimersFrame = target.frame;
		m_TimersThisFrame = m_GpuTimersOn.load( std::memory_order_relaxed );
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
		m_Timers = std::make_unique<graph::GpuPassTimers>( *target.device );
	if ( !m_Timers->Supported() )
		return nullptr;
	m_Timers->BeginFrame( target.frame, target.submitted );
	return m_Timers.get();
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
	world.time = target.time;
	world.waterReflectTintScale = target.waterReflectTintScale;
	std::shared_ptr<const ShadowWork> shadows;
	{
		std::lock_guard<std::mutex> guard( m_ShadowLock );
		for ( const auto &[queued, work] : m_ShadowWork )
		{
			if ( queued == tag )
				shadows = work;
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
	if ( shadows && target.device && !shadows->views.empty() )
		world.shadowAtlas = DrawStageShadows(
		    *target.device, *shadows, target.submitted, target.frame, &world.shadowAtlasDesc );
	// The stage view's screen passes: GTAO over the pass's prepass.
	const int aoQuality = m_AoQuality.load( std::memory_order_relaxed );
	if ( shadows && target.device && aoQuality > 0 &&
	     EnsureOcclusion( *target.device, encoder, target.width, target.height, target.submitted ) )
	{
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
	m_Pass.Record( tag, encoder, world );
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
	{
		// A new shadow quality: this slot's atlas was last used by an earlier
		// frame, which `submitted` covers.
		(void)device.Release( atlas.texture, submitted );
		atlas = Atlas();
		atlas.desc.format = Format::kD32Float;
		atlas.desc.width = atlas.desc.height = work.atlasSize;
		atlas.desc.usages = { ResourceUsage::kDepthWrite, ResourceUsage::kSampled };
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
	m_ShadowTilesKept.fetch_add( work.views.size() - dirty.size(), std::memory_order_relaxed );
	m_ShadowTilesDrawn.fetch_add( dirty.size(), std::memory_order_relaxed );
	if ( dirty.empty() )
	{
		++m_AtlasNext;
		*desc = atlas.desc;
		return atlas.texture;
	}
	// Each view draws the chunks inside its frustum (a chunk's box wholly
	// outside one clip plane is culled).
	auto inside = []( const math::float4x4 &clip, const Casters::Chunk &chunk )
	{
		int outside[6] = {};
		for ( int corner = 0; corner < 8; ++corner )
		{
			const math::float4 h =
			    math::Transform( clip, { corner & 1 ? chunk.max[0] : chunk.min[0],
			                               corner & 2 ? chunk.max[1] : chunk.min[1],
			                               corner & 4 ? chunk.max[2] : chunk.min[2], 1.0f } );
			outside[0] += h.x < -h.w;
			outside[1] += h.x > h.w;
			outside[2] += h.y < -h.w;
			outside[3] += h.y > h.w;
			outside[4] += h.z < 0.0f;
			outside[5] += h.z > h.w;
		}
		return std::none_of( outside, outside + 6,
		    []( int count )
		    {
			    return count == 8;
		    } );
	};
	std::vector<std::vector<pass::shadows::ShadowCaster>> chunkCasters( dirty.size() );
	std::vector<pass::shadows::ShadowDepthView> views;
	views.reserve( dirty.size() );
	for ( std::size_t v = 0; v < dirty.size(); ++v )
	{
		for ( const Casters::Chunk &chunk : casters->chunks )
		{
			if ( inside( dirty[v]->viewProjection, chunk ) )
				chunkCasters[v].push_back(
				    { *mesh, math::float4x4::Identity(), chunk.firstIndex, chunk.indexCount } );
		}
		views.push_back( { dirty[v]->viewProjection, dirty[v]->tile, chunkCasters[v] } );
	}
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
	pass::shadows::ShadowAtlasTarget target{ atlasRef, work.atlasSize, work.guardTexels };
	target.keep = !whole;
	if ( !m_ShadowRenderer->AddPasses( builder, target, views ) )
		return {};
	auto compiled = graph::CompileGraph( std::move( builder ) );
	if ( !compiled )
		return {};
	graph::SerialGraphExecutor executor;
	executor.SetLabelObserver( m_SlotTimers );
	auto executed = executor.Execute( compiled.Value(), device );
	if ( !executed )
		return {};
	m_ShadowRenderer->Collect( executed.Value().token );
	m_CasterMeshes->Retire( executed.Value().token );
	atlas.usage = ResourceUsage::kSampled;
	atlas.drawn = work.views;
	atlas.generation = casters->generation;
	atlas.guardTexels = work.guardTexels;
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
	// After the idle wait: the timers' buffers may go at once.
	std::lock_guard<std::mutex> guard( m_TimersLock );
	if ( m_Timers && &m_Timers->Device() == &device )
		m_Timers.reset();
}

} // namespace render::composition
