//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.composition's world (RFC 0016 K5): the world, its casters, static props and the pipeline store.
//
//=============================================================================//

#ifdef RENDER_CORE_VULKAN
#include "render/device/vulkan/provider.h"
#endif
#include "core_world_internal.h"

namespace render::composition
{

namespace
{

// The depth caster has no material texture or blend binding. Until its
// cutout/transmission program exists, neither kind casts a solid silhouette.
bool OpaqueShadowMaterial( const pass::world::WorldMaterial &material )
{
	for ( const auto &[key, value] : material.variables )
	{
		auto is = [&]( const char *name )
		{
			const std::size_t length = std::strlen( name );
			if ( key.size() != length )
				return false;
			for ( std::size_t i = 0; i < length; ++i )
				if ( std::tolower( static_cast<unsigned char>( key[i] ) ) != name[i] )
					return false;
			return true;
		};
		if ( !is( "$alphatest" ) && !is( "$translucent" ) && !is( "$alpha" ) )
			continue;
		char *end = nullptr;
		const float number = std::strtof( value.c_str(), &end );
		if ( end == value.c_str() || !std::isfinite( number ) ||
		     ( is( "$alpha" ) ? number < 1.0f : number != 0.0f ) )
			return false;
	}
	return true;
}

} // namespace

namespace
{
constexpr char kPipelineKeysHeader[] = "core-pipeline-keys/v1";
constexpr std::size_t kMaxPipelineKeys = 8192;
} // namespace

void CoreWorld::SetPipelineStore( const char *directory )
{
	const std::string path = directory && directory[0]
	                             ? std::string( directory ) + "/core_pipelines.keys"
	                             : std::string();
	if ( path == m_PipelineKeysPath )
		return;
	SavePipelineKeys();
	m_PipelineKeysPath = path;
	{
		std::lock_guard<std::mutex> lock( m_PipelineStoreLock );
		if ( m_PipelineStoreDirectory.empty() && directory && directory[0] )
			m_PipelineStoreDirectory = directory;
	}
	m_PipelineKeys.clear();
	if ( FILE *file = path.empty() ? nullptr : std::fopen( path.c_str(), "r" ) )
	{
		char line[1024];
		bool headed = false;
		while (
		    std::fgets( line, sizeof( line ), file ) && m_PipelineKeys.size() < kMaxPipelineKeys )
		{
			std::string text( line );
			while ( !text.empty() && ( text.back() == '\n' || text.back() == '\r' ) )
				text.pop_back();
			if ( !headed )
			{
				headed = true;
				if ( text != kPipelineKeysHeader )
					break; // another format: start empty
				continue;
			}
			if ( !text.empty() )
				m_PipelineKeys.insert( text );
		}
		std::fclose( file );
	}
	m_Pass.SetPipelinePrewarm( { m_PipelineKeys.begin(), m_PipelineKeys.end() } );
}

void CoreWorld::OpenPipelineStore( device::IRenderDevice2 &device )
{
	std::lock_guard<std::mutex> lock( m_PipelineStoreLock );
	if ( m_PipelineStoreTried || m_PipelineStoreDirectory.empty() )
		return;
	m_PipelineStoreTried = true;
#ifdef RENDER_CORE_VULKAN
	if ( device::vulkan::OpenPipelineStore( device, m_PipelineStoreDirectory.c_str() ) )
		m_PipelineStoreDevice = &device;
#else
	(void)device;
#endif
}

void CoreWorld::SavePipelineKeys()
{
	{
		std::lock_guard<std::mutex> lock( m_PipelineStoreLock );
#ifdef RENDER_CORE_VULKAN
		if ( m_PipelineStoreDevice )
			(void)device::vulkan::SavePipelineStore( *m_PipelineStoreDevice );
#endif
	}
	if ( m_PipelineKeysPath.empty() )
		return;
	const std::size_t before = m_PipelineKeys.size();
	for ( std::string &key : m_Pass.CreatedPipelineKeys() )
	{
		if ( m_PipelineKeys.size() >= kMaxPipelineKeys )
			break;
		m_PipelineKeys.insert( std::move( key ) );
	}
	if ( m_PipelineKeys.size() == before )
		return;
	// Replaced only once complete, so an interrupted save keeps the old list.
	const std::string temporary = m_PipelineKeysPath + ".tmp";
	FILE *file = std::fopen( temporary.c_str(), "w" );
	if ( !file )
		return;
	bool written = std::fprintf( file, "%s\n", kPipelineKeysHeader ) > 0;
	for ( const std::string &key : m_PipelineKeys )
		written = written && std::fprintf( file, "%s\n", key.c_str() ) > 0;
	if ( std::fclose( file ) != 0 || !written ||
	     std::rename( temporary.c_str(), m_PipelineKeysPath.c_str() ) != 0 )
		std::remove( temporary.c_str() );
	m_Pass.SetPipelinePrewarm( { m_PipelineKeys.begin(), m_PipelineKeys.end() } );
}

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
	m_SurfacePageIds.resize( surfaceCount );
	m_SurfacePageHandles.resize( surfaceCount );
	for ( unsigned int i = 0; i < surfaceCount; ++i )
	{
		m_SurfacePageIds[i] = surfaces[i].lightmapPage;
		pass::world::WorldSurface surface;
		surface.material = surfaces[i].material;
		surface.lightmapPage = m_Host && m_Host->lightmapPageHandle
		                           ? m_Host->lightmapPageHandle( surfaces[i].lightmapPage )
		                           : 0;
		m_SurfacePageHandles[i] = surface.lightmapPage;
		surface.firstIndex = surfaces[i].firstIndex;
		surface.indexCount = surfaces[i].indexCount;
		data.surfaces.push_back( surface );
	}
	data.materials = WorldMaterials( materials, materialCount );
	// A plain map's cubemaps, sent through StageUpload() before this call.
	data.reflection = m_Capture.reflection;
	if ( data.reflection )
		std::fprintf( stderr, "Render core: plain map: %u reflection probes from its cubemaps\n",
		    data.reflection->count );
	m_QueuedLighting.clear();
	m_StageSet = false;
	m_WorldCasters.reset();
	m_StaticCastsShadow.clear();
	{
		std::lock_guard<std::mutex> guard( m_ShadowLock );
		m_Casters.reset();
	}
	m_StaticMeshes.clear();
	m_Props.reset();
	m_StaticMaterials.clear();
	m_StageRuntimeDirect.store( false, std::memory_order_relaxed );
	m_StageHasIndirect.store( false, std::memory_order_relaxed );
	m_MapLights = pass::lights::MapLights();
	m_Media.reset();
	std::vector<unsigned char> opaqueTriangles( data.indices.size() / 3, 0 );
	for ( const auto &surface : data.surfaces )
	{
		if ( surface.material >= data.materials.size() ||
		     !OpaqueShadowMaterial( data.materials[surface.material] ) )
			continue;
		const auto first = std::min<std::size_t>( surface.firstIndex / 3, opaqueTriangles.size() );
		const auto end = std::min<std::size_t>(
		    ( surface.firstIndex + surface.indexCount ) / 3, opaqueTriangles.size() );
		std::fill(
		    opaqueTriangles.begin() + first, opaqueTriangles.begin() + std::max( first, end ), 1 );
	}
	SetWorldCasters( data, opaqueTriangles );
	m_Pass.SetWorld( std::move( data ) );
}

void CoreWorld::RefreshLightmapPages()
{
	if ( !m_Host || !m_Host->lightmapPageHandle || m_SurfacePageIds.empty() )
		return;
	// A handful of pages: resolve each id once.
	std::map<int, int> resolved;
	bool changed = false;
	for ( std::size_t i = 0; i < m_SurfacePageIds.size(); ++i )
	{
		const int id = m_SurfacePageIds[i];
		auto at = resolved.find( id );
		if ( at == resolved.end() )
			at = resolved.emplace( id, m_Host->lightmapPageHandle( id ) ).first;
		if ( m_SurfacePageHandles[i] != at->second )
		{
			m_SurfacePageHandles[i] = at->second;
			changed = true;
		}
	}
	if ( changed )
		m_Pass.RemapLightmapPages( m_SurfacePageHandles );
}

void CoreWorld::SetWorldCasters(
    const pass::world::WorldData &data, std::span<const unsigned char> opaqueTriangles )
{
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
			if ( !opaqueTriangles[t / 3] )
				continue;
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
		m_WorldCasters = casters;
		m_Casters = std::move( casters );
	}
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
		material.hasProxy = source.hasProxy;
		material.translucent = source.translucent;
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
	m_QueuedLighting.clear();
	m_StageSet = false;
	m_WorldCasters.reset();
	m_StaticCastsShadow.clear();
	{
		std::lock_guard<std::mutex> guard( m_ShadowLock );
		m_Casters.reset();
	}
	m_StaticMeshes.clear();
	m_Props.reset();
	m_StaticMaterials.clear();
	m_StageRuntimeDirect.store( false, std::memory_order_relaxed );
	m_StageHasIndirect.store( false, std::memory_order_relaxed );
	m_StageWorld.reset();
	m_MapLights = pass::lights::MapLights();
	m_Media.reset();
	if ( entities )
	{
		if ( const auto parsed = pass::lights::ParseEntityLump( entities ) )
		{
			m_MapLights = pass::lights::MapLightsFromEntities( *parsed );
			// The participating media (K12): the same parse render_lab reads.
			auto media = std::make_shared<MapMedia>( MediaFromEntities( *parsed ) );
			if ( media->present )
			{
				std::fprintf( stderr,
				    "render core: volumetric medium: %zu volumes, controller density %g, "
				    "height %g\n",
				    media->medium.volumes.size(), double( media->medium.fog.density ),
				    double( media->medium.fog.heightDensity ) );
				m_Media = std::move( media );
			}
		}
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
	data.materials = WorldMaterials( materials, materialCount );
	std::vector<unsigned char> opaqueTriangles( data.indices.size() / 3, 1 );
	for ( unsigned int i = 0; i < meshletCount; ++i )
	{
		const RenderCoreWorldMeshlet &meshlet = meshlets[i];
		if ( meshlet.material >= data.materials.size() ||
		     OpaqueShadowMaterial( data.materials[meshlet.material] ) )
			continue;
		const std::size_t first =
		    std::min<std::size_t>( meshlet.firstIndex / 3, opaqueTriangles.size() );
		const std::size_t end = std::min<std::size_t>(
		    ( std::uint64_t( meshlet.firstIndex ) + meshlet.indexCount + 2 ) / 3,
		    opaqueTriangles.size() );
		std::fill(
		    opaqueTriangles.begin() + first, opaqueTriangles.begin() + std::max( first, end ), 0 );
	}
	const std::size_t cutoutTriangles =
	    std::count( opaqueTriangles.begin(), opaqueTriangles.end(), 0 );
	// Those surfaces cast through their materials' coverage instead.
	m_CutoutSurfaces.clear();
	for ( unsigned int i = 0; i < meshletCount; ++i )
		if ( meshlets[i].material < data.materials.size() &&
		     !OpaqueShadowMaterial( data.materials[meshlets[i].material] ) )
			m_CutoutSurfaces.push_back( i );
	if ( cutoutTriangles )
		std::fprintf( stderr,
		    "Render core: %zu alpha-tested world triangles in %zu surfaces cast "
		    "through their materials' coverage\n",
		    cutoutTriangles, m_CutoutSurfaces.size() );
	SetWorldCasters( data, opaqueTriangles );
	data.surfaces.reserve( meshletCount );
	for ( unsigned int i = 0; i < meshletCount; ++i )
	{
		pass::world::WorldSurface surface;
		surface.material = meshlets[i].material;
		surface.firstIndex = meshlets[i].firstIndex;
		surface.indexCount = meshlets[i].indexCount;
		data.surfaces.push_back( surface );
	}
	m_StageWorld = std::make_shared<const pass::world::WorldData>( std::move( data ) );
	SetStage();
}

void CoreWorld::SetStage()
{
	m_QueuedLighting.clear();
	if ( !m_StageWorld )
		return;
	pass::world::WorldData data = *m_StageWorld;
	auto stage = std::make_shared<pass::world::WorldStage>();
	// The sun's baked visibility in the gradient page's alpha (the LMAP
	// header's flag, or any texel below one in decoded pages).
	m_StageSunMask = m_Capture.sunMask && m_Capture.lightmap.Directional();
	stage->lightmap = m_Capture.lightmap;
	// Baked shadow masks serve the runtime direct light's lightmapped surfaces.
	if ( !m_Capture.shadowMask.flat.empty() &&
	     m_Capture.shadowMask.width == m_Capture.lightmap.width &&
	     m_Capture.shadowMask.height == m_Capture.lightmap.height )
	{
		stage->shadowMask = m_Capture.shadowMask;
		m_StageMaskLights = m_Capture.maskLights;
	}
	else
		m_StageMaskLights = nullptr;
	if ( m_Capture.indirect.flat.size() == m_Capture.lightmap.flat.size() &&
	     m_Capture.indirect.flatFormat == m_Capture.lightmap.flatFormat )
		stage->indirect = m_Capture.indirect;
	// Runtime direct light (r_core_runtime_direct) needs the indirect layer;
	// a map without one draws its total layer. The stage keeps both layers,
	// so the setting changes between frames (SetQuality).
	m_StageHasIndirect.store( !stage->indirect.flat.empty(), std::memory_order_relaxed );
	m_StageRuntimeDirect.store(
	    m_RuntimeDirect.load( std::memory_order_relaxed ) && !stage->indirect.flat.empty(),
	    std::memory_order_relaxed );
	stage->probes = m_Capture.probes;
	std::fprintf( stderr,
	    "Render core: world stage: %zu meshlets, lightmap %ux%u%s%s%s, probes %s, reflection "
	    "probes %s\n",
	    data.surfaces.size(), stage->lightmap.width, stage->lightmap.height,
	    stage->lightmap.Directional()
	        ? stage->lightmap.Blocks() ? " directional (BC6H/BC7)" : " directional"
	        : "",
	    stage->indirect.flat.empty()     ? ", no indirect layer"
	    : !stage->indirect.Directional() ? ", indirect layer (flat)"
	                                     : ", indirect layer (directional)",
	    m_StageRuntimeDirect.load( std::memory_order_relaxed ) ? ", runtime direct light"
	                                                           : ", baked direct light",
	    stage->probes ? "yes" : "no", m_Capture.reflection ? "yes" : "no" );
	data.stage = std::move( stage );
	data.reflection = m_Capture.reflection;
	const std::uint32_t materialBase = std::uint32_t( data.materials.size() );
	for ( pass::world::WorldMaterial &material : m_StaticMaterials )
		data.materials.push_back( material );
	// Release staging the pass has uploaded, so the new world data starts
	// without those shared_ptrs: re-uploads use the model level source.
	for ( const auto &[meshId, lodId] : m_Pass.DrainReleasedStaging() )
	{
		if ( meshId < m_StaticMeshes.size() && lodId < m_StaticMeshes[meshId].lods.size() )
		{
			m_StaticMeshes[meshId].lods[lodId].vertices.reset();
			m_StaticMeshes[meshId].lods[lodId].indices.reset();
		}
	}
	data.staticMeshes = m_StaticMeshes;
	for ( pass::world::WorldData::StaticMesh &mesh : data.staticMeshes )
	{
		for ( pass::world::WorldSurface &surface : mesh.surfaces )
			surface.material += materialBase;
		for ( std::vector<std::uint32_t> &family : mesh.skinMaterials )
		{
			for ( std::uint32_t &material : family )
			{
				if ( material != ~0u )
					material += materialBase;
			}
		}
	}
	data.props = m_Props;
	// The model geometry's revision, so the pass keeps the levels it already
	// uploaded when only the stage's lighting changed (a late probe volume).
	data.modelsRevision = m_ModelsRevision;
	m_Pass.SetWorld( std::move( data ) );
	m_Pass.SetModelLevelSource( m_ModelBytes.empty() ? nullptr : &m_ModelLevelSource );
	m_StageSet = true;
	// The change the capture holds (parts arrive relative to it).
	if ( m_Capture.table )
		m_Pass.SetStageChange( m_Capture.change, *m_Capture.table );
}

void CoreWorld::SetStaticCasters()
{
	if ( !m_WorldCasters )
		return;
	auto casters = std::make_shared<Casters>( *m_WorldCasters );
	m_CutoutStaticSurfaces.clear();
	unsigned int instances = 0;
	unsigned int noShadow = 0;
	unsigned int cutout = 0;
	for ( std::uint32_t i = 0; i < PropCount(); ++i )
	{
		if ( !m_Pass.DrawsStaticInstance( i ) )
			continue;
		if ( !m_StaticCastsShadow[i] )
		{
			++noShadow;
			continue;
		}
		const scene::MeshInstance &placed = m_Props->instances[i];
		struct
		{
			std::uint32_t mesh, skin;
			const float *world;
		} instance = { std::uint32_t( placed.desc.mesh ), std::uint32_t( placed.desc.material ),
		    &placed.desc.world.rows[0].x };
		if ( instance.mesh >= m_StaticMeshes.size() )
			continue;
		const pass::world::WorldData::StaticMesh &mesh = m_StaticMeshes[instance.mesh];
		// The instance's selected surfaces, grouped by the level that owns them:
		// a level is its own vertex and index allocation, so the caster geometry
		// is built one level at a time (only the selected levels' vertices are
		// copied; the triangles are the same ones the model draws).
		struct CasterLevel
		{
			const pass::world::WorldData::StaticMeshLod *block;
			std::vector<const pass::world::WorldSurface *> surfaces;
		};
		std::vector<CasterLevel> levels;
		for ( std::size_t s = 0; s < mesh.surfaces.size(); ++s )
		{
			if ( mesh.selection && !std::binary_search( mesh.selection->begin(),
			                           mesh.selection->end(), std::uint32_t( s ) ) )
				continue;
			std::uint32_t material = mesh.surfaces[s].material;
			if ( !mesh.skinMaterials.empty() )
				material = mesh.skinMaterials[instance.skin][s];
			if ( material >= m_StaticMaterials.size() )
				continue;
			if ( !OpaqueShadowMaterial( m_StaticMaterials[material] ) )
			{
				// Cast through its material's coverage (the world pass).
				m_CutoutStaticSurfaces.emplace_back( i, std::uint32_t( s ) );
				continue;
			}
			const std::uint32_t lod = mesh.LodOfSurface( s );
			if ( lod == ~0u || !mesh.lods[lod].vertices || !mesh.lods[lod].indices )
				continue;
			auto level = std::find_if( levels.begin(), levels.end(),
			    [&]( const CasterLevel &held )
			    {
				    return held.block == &mesh.lods[lod];
			    } );
			if ( level == levels.end() )
			{
				levels.push_back( { &mesh.lods[lod], {} } );
				level = levels.end() - 1;
			}
			level->surfaces.push_back( &mesh.surfaces[s] );
		}
		if ( levels.empty() )
		{
			++cutout;
			continue;
		}
		for ( const CasterLevel &level : levels )
		{
			const pass::world::WorldData::StaticMeshLod &block = *level.block;
			bool valid = true;
			for ( const pass::world::WorldSurface *surface : level.surfaces )
			{
				for ( std::uint32_t index = surface->firstIndex;
				    index < surface->firstIndex + surface->indexCount; ++index )
					valid = valid && index < block.indexCount &&
					        ( *block.indices )[index] < block.vertexCount;
			}
			if ( !valid )
				continue;
			const std::uint32_t firstVertex = std::uint32_t( casters->positions.size() / 3 );
			for ( const material::SurfaceModelVertex &vertex : *block.vertices )
			{
				for ( int axis = 0; axis < 3; ++axis )
				{
					const float *row = instance.world + axis * 4;
					casters->positions.push_back( row[0] * vertex.position[0] +
					                              row[1] * vertex.position[1] +
					                              row[2] * vertex.position[2] + row[3] );
				}
			}
			Casters::Chunk chunk;
			chunk.firstIndex = std::uint32_t( casters->indices.size() );
			for ( int axis = 0; axis < 3; ++axis )
			{
				chunk.min[axis] = std::numeric_limits<float>::max();
				chunk.max[axis] = -std::numeric_limits<float>::max();
			}
			for ( const pass::world::WorldSurface *surface : level.surfaces )
			{
				for ( std::uint32_t index = surface->firstIndex;
				    index < surface->firstIndex + surface->indexCount; ++index )
				{
					const std::uint32_t vertex = firstVertex + ( *block.indices )[index];
					casters->indices.push_back( vertex );
					for ( int axis = 0; axis < 3; ++axis )
					{
						const float position = casters->positions[vertex * 3 + axis];
						chunk.min[axis] = std::min( chunk.min[axis], position );
						chunk.max[axis] = std::max( chunk.max[axis], position );
					}
				}
			}
			chunk.indexCount = std::uint32_t( casters->indices.size() ) - chunk.firstIndex;
			if ( chunk.indexCount )
			{
				casters->chunks.push_back( chunk );
				++instances;
			}
		}
	}
	{
		std::lock_guard<std::mutex> guard( m_ShadowLock );
		casters->generation = ++m_CasterGeneration;
		m_Casters = std::move( casters );
	}
	std::fprintf( stderr,
	    "Render core: %u opaque static props cast core shadows (%u authored no-shadow, %u "
	    "cutout-only); %zu alpha-tested prop surfaces cast through their materials' coverage\n",
	    instances, noShadow, cutout, m_CutoutStaticSurfaces.size() );
}

void CoreWorld::SetStaticProps( const RenderCoreStaticModel *models, unsigned int modelCount,
    const RenderCoreStaticProp *props, unsigned int propCount )
{
	if ( ( modelCount && !models ) || ( propCount && !props ) )
		return;
	m_StaticMeshes.clear();
	m_StaticMaterials.clear();
	m_Props.reset();
	m_StaticCastsShadow.clear();
	m_ModelPoseSources.clear();
	m_ModelBytes.clear();
	++m_ModelsRevision;
	m_StaticMeshes.resize( modelCount );
	m_ModelPoseSources.resize( modelCount );
	m_ModelBytes.resize( modelCount );
	for ( unsigned int i = 0; i < modelCount; ++i )
	{
		const RenderCoreStaticModel &source = models[i];
		if ( !source.mdl || !source.vvd || !source.vtx || !source.materials ||
		     source.mdlBytes > std::numeric_limits<std::size_t>::max() ||
		     source.vvdBytes > std::numeric_limits<std::size_t>::max() ||
		     source.vtxBytes > std::numeric_limits<std::size_t>::max() )
			continue;
		mdl::ModelBytes bytes;
		bytes.mdl = { static_cast<const char *>( source.mdl ), std::size_t( source.mdlBytes ) };
		bytes.vvd = { static_cast<const char *>( source.vvd ), std::size_t( source.vvdBytes ) };
		bytes.vtx = { static_cast<const char *>( source.vtx ), std::size_t( source.vtxBytes ) };
		auto parsed = mdl::ParseModelGeometryVariants( bytes );
		if ( !parsed )
		{
			std::fprintf( stderr, "Render core: static model %s: %s\n",
			    source.name ? source.name : "(unnamed)", mdl::Describe( parsed.Error() ).c_str() );
			continue;
		}
		const mdl::Model &model = parsed.Value();
		// Keep the raw MDL bytes for zero-staging resupply.
		ModelBytes &raw = m_ModelBytes[i];
		raw.mdl.assign( static_cast<const char *>( source.mdl ), std::size_t( source.mdlBytes ) );
		raw.vvd.assign( static_cast<const char *>( source.vvd ), std::size_t( source.vvdBytes ) );
		raw.vtx.assign( static_cast<const char *>( source.vtx ), std::size_t( source.vtxBytes ) );
		raw.materialCount = source.materialCount;
		raw.materialLodCount = source.materialLodCount;
		ModelPoseSource &poseSource = m_ModelPoseSources[i];
		poseSource.bodyParts = model.bodyParts;
		poseSource.lodCount = std::uint32_t( model.lodTextures.size() );
		// Only a model the host may pose needs its per-frame pose source: a
		// static-only model's vertices are never skinned by the core, so its
		// skinning copy would be a second CPU copy of geometry it keeps once.
		// The host declares it (RenderCoreStaticModel::posed, which defaults
		// to true), so a host that does not know keeps every model posable.
		const bool posed = source.posed;
		if ( posed && model.bones.size() <= 255 )
		{
			for ( const mdl::Bone &bone : model.bones )
			{
				pass::skinning::BoneMatrix matrix;
				for ( int row = 0; row < 3; ++row )
					std::copy( bone.poseToBone.m[row].begin(), bone.poseToBone.m[row].end(),
					    matrix.rows[row] );
				poseSource.poseToBone.push_back( matrix );
			}
		}
		const std::uint32_t materialBase = std::uint32_t( m_StaticMaterials.size() );
		const std::uint64_t allMaterialCount =
		    std::uint64_t( source.materialCount ) * source.materialLodCount;
		if ( !source.materialLodCount ||
		     allMaterialCount > std::numeric_limits<unsigned int>::max() )
			continue;
		std::vector<pass::world::WorldMaterial> materials =
		    WorldMaterials( source.materials, unsigned( allMaterialCount ) );
		for ( pass::world::WorldMaterial &material : materials )
		{
			material.mesh = true;
			m_StaticMaterials.push_back( std::move( material ) );
		}
		pass::world::WorldData::StaticMesh &mesh = m_StaticMeshes[i];
		mesh.posed = posed;
		mesh.skinMaterials.resize( model.skinFamilies.size() );
		// One block per hardware level, each with its own vertex and index
		// allocation (RFC 0016 model geometry residency), published as shared
		// staging: the composition and the pass hold one allocation between them,
		// never a copy each.
		const std::size_t levelCount = model.lodTextures.size();
		std::vector<std::vector<const mdl::Mesh *>> partsByLevel( levelCount );
		for ( const mdl::Mesh &part : model.meshes )
		{
			if ( part.lod < partsByLevel.size() )
				partsByLevel[part.lod].push_back( &part );
		}
		// Each level's pose source, in the same vertex order as its block (only a
		// model the host may pose needs one).
		std::vector<std::vector<pass::skinning::SkinVertex>> skinByLevel( levelCount );
		bool complete = true;
		for ( std::size_t lod = 0; lod < partsByLevel.size() && complete; ++lod )
		{
			// A level with no geometry is still a level: the host selects it, and
			// an empty selection is a valid blank model rather than a refusal.
			if ( partsByLevel[lod].empty() )
			{
				mesh.AddLevel( {}, {} );
				continue;
			}
			std::vector<material::SurfaceModelVertex> vertices;
			std::vector<std::uint32_t> indices;
			std::vector<pass::world::WorldSurface> surfaces;
			std::vector<std::vector<std::uint32_t>> levelSkins( model.skinFamilies.size() );
			std::vector<pass::skinning::SkinVertex> &skin = skinByLevel[lod];
			for ( const mdl::Mesh *part : partsByLevel[lod] )
			{
				const bool resolvedLod = part->lod < source.materialLodCount;
				const bool unchangedLod = model.lodTextures[part->lod] == model.lodTextures[0];
				const std::uint32_t lodMaterialBase =
				    materialBase + ( resolvedLod ? part->lod * source.materialCount : 0u );
				const std::int32_t texture =
				    model.skinFamilies.empty()
				        ? part->textureRef
				        : ( part->textureRef >= 0 &&
				                      std::size_t( part->textureRef ) < model.skinFamilies[0].size()
				                  ? model.skinFamilies[0][std::size_t( part->textureRef )]
				                  : -1 );
				if ( texture < 0 || std::uint32_t( texture ) >= source.materialCount )
				{
					complete = false;
					break;
				}
				const std::uint32_t base = std::uint32_t( vertices.size() );
				for ( std::size_t vertexIndex = 0; vertexIndex < part->vertices.size();
				    ++vertexIndex )
				{
					const mdl::Vertex &from = part->vertices[vertexIndex];
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
						// A model without a tangent block may still draw its
						// unbumped material. Keep a stable perpendicular frame.
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
					vertices.push_back( to );
					if ( posed && !poseSource.poseToBone.empty() &&
					     vertexIndex < part->weights.size() )
					{
						const mdl::BoneWeights &weights = part->weights[vertexIndex];
						pass::skinning::SkinVertex vertex;
						std::copy( to.position, to.position + 3, vertex.position );
						std::copy( to.normal, to.normal + 3, vertex.normal );
						std::copy( to.tangent, to.tangent + 4, vertex.tangent );
						vertex.weight0 = weights.weights[0];
						vertex.weight1 = weights.count > 1 ? weights.weights[1] : 0.0f;
						for ( int bone = 0; bone < 3; ++bone )
							vertex.bones |= std::uint32_t( weights.bones[bone] ) << ( bone * 8 );
						skin.push_back( vertex );
					}
				}
				pass::world::WorldSurface surface;
				surface.material =
				    resolvedLod || unchangedLod ? lodMaterialBase + std::uint32_t( texture ) : ~0u;
				// The level's own index range, from zero: a level is its own
				// allocation.
				surface.firstIndex = std::uint32_t( indices.size() );
				for ( std::uint32_t index : part->indices )
					indices.push_back( base + index );
				surface.indexCount = std::uint32_t( indices.size() ) - surface.firstIndex;
				surfaces.push_back( surface );
				// Keep the body/LOD selector indexed exactly like the surfaces,
				// which are grouped by LOD above. Static-only models need this
				// selection metadata too; only their skinning copy is omitted.
				poseSource.surfaceBodies.push_back(
				    { std::uint32_t( part->bodyPart ), part->bodyModel, part->lod } );
				for ( std::size_t skin = 0; skin < model.skinFamilies.size(); ++skin )
				{
					const std::vector<std::int16_t> &family = model.skinFamilies[skin];
					const std::int32_t selected =
					    part->textureRef >= 0 && std::size_t( part->textureRef ) < family.size()
					        ? family[std::size_t( part->textureRef )]
					        : -1;
					levelSkins[skin].push_back(
					    ( resolvedLod || unchangedLod ) && selected >= 0 &&
					            std::uint32_t( selected ) < source.materialCount
					        ? lodMaterialBase + std::uint32_t( selected )
					        : ~0u );
				}
			}
			if ( !complete )
				break;
			pass::world::WorldData::StaticMeshLod block;
			block.vertexCount = std::uint32_t( vertices.size() );
			block.indexCount = std::uint32_t( indices.size() );
			block.vertices = std::make_shared<const std::vector<material::SurfaceModelVertex>>(
			    std::move( vertices ) );
			block.indices =
			    std::make_shared<const std::vector<std::uint32_t>>( std::move( indices ) );
			mesh.AddLevel( std::move( block ), std::move( surfaces ), std::move( levelSkins ) );
		}
		if ( !complete )
		{
			// One unusable material slot rejects the model whole: a level whose
			// materials could not be resolved draws nothing rather than part of
			// itself with another material.
			mesh = {};
			poseSource = {};
		}
		else if ( posed )
		{
			poseSource.vertices = std::move( skinByLevel );
		}
		poseSource.parsed = complete;
	}
	// Each parsed model's default selection (level 0).
	for ( unsigned int i = 0; i < modelCount; ++i )
	{
		if ( m_ModelPoseSources[i].parsed )
			m_StaticMeshes[i].selection = m_ModelPoseSources[i].SelectedSurfaces( 0 );
	}
	// The props are scene instances, committed in one change set: prop order
	// is instance order. A prop whose model did not parse is added with no
	// mesh (~0u), so it keeps its index and draws nothing. The host's render
	// box bounds it (it culls with the same box); a host that gave none
	// leaves it unbounded, so it is never culled.
	auto propScene = scene::CreateRenderScene();
	scene::ChangeSet placed;
	for ( unsigned int i = 0; i < propCount; ++i )
	{
		const std::uint32_t mesh = props[i].skin >= 0 ? props[i].model : ~0u;
		scene::MeshInstanceDesc desc;
		desc.mesh =
		    mesh < m_ModelPoseSources.size() && m_ModelPoseSources[mesh].parsed ? mesh : ~0u;
		desc.material = props[i].skin >= 0 ? std::uint32_t( props[i].skin ) : ~0u;
		std::copy_n( props[i].world, 12, &desc.world.rows[0].x );
		constexpr float kUnbounded = 1.0e18f;
		desc.localBounds =
		    props[i].hasBounds
		        ? render::math::Aabb{ { props[i].boundsMin[0], props[i].boundsMin[1],
		                                  props[i].boundsMin[2] },
		              { props[i].boundsMax[0], props[i].boundsMax[1], props[i].boundsMax[2] } }
		        : render::math::Aabb{ { -kUnbounded, -kUnbounded, -kUnbounded },
		              { kUnbounded, kUnbounded, kUnbounded } };
		placed.Add( propScene->Reserve(), desc );
		m_StaticCastsShadow.push_back( props[i].castsShadow );
	}
	m_Props = propCount && propScene->Commit( placed ) ? propScene->Snapshot() : nullptr;
	unsigned int parsed = 0;
	for ( const pass::world::WorldData::StaticMesh &mesh : m_StaticMeshes )
		parsed += !mesh.surfaces.empty() ? 1u : 0u;
	std::fprintf( stderr, "Render core: static meshes: %u of %u parsed, %zu instances\n", parsed,
	    modelCount, PropCount() );
	if ( m_StageSet )
	{
		SetStage();
		SetStaticCasters();
	}
}

} // namespace render::composition
