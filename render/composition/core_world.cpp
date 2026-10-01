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
#include "mdl/studio_model.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cctype>
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
	m_QueuedLighting.clear();
	m_StageSet = false;
	m_WorldCasters.reset();
	m_StaticCastsShadow.clear();
	{
		std::lock_guard<std::mutex> guard( m_ShadowLock );
		m_Casters.reset();
		m_ShadowWork.clear();
	}
	m_StaticMeshes.clear();
	m_StaticInstances.clear();
	m_StaticMaterials.clear();
	m_StageRuntimeDirect.store( false, std::memory_order_relaxed );
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
		m_ShadowWork.clear();
	}
	m_StaticMeshes.clear();
	m_StaticInstances.clear();
	m_StaticMaterials.clear();
	m_StageRuntimeDirect.store( false, std::memory_order_relaxed );
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
	if ( cutoutTriangles )
		std::fprintf( stderr,
		    "Render core: %zu alpha-tested world triangles need cutout "
		    "shadows; omitted from the solid caster\n",
		    cutoutTriangles );
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
	{
		stage->indirect = m_Capture.indirect;
		if ( m_Capture.indirectGradient.size() == m_Capture.indirect.size() )
			stage->indirectGradient = m_Capture.indirectGradient;
	}
	// Runtime direct light (r_core_runtime_direct) needs the indirect layer;
	// a map without one draws its total layer.
	stage->runtimeDirect =
	    m_RuntimeDirect.load( std::memory_order_relaxed ) && !stage->indirect.empty();
	m_StageRuntimeDirect.store( stage->runtimeDirect, std::memory_order_relaxed );
	stage->probes = m_Capture.probes;
	stage->reflectionWidth = m_Capture.reflectionWidth;
	stage->reflectionHeight = m_Capture.reflectionHeight;
	stage->reflectionProbes = m_Capture.reflection;
	std::fprintf( stderr,
	    "Render core: world stage: %zu meshlets, lightmap %ux%u%s%s%s, probes %s, reflection "
	    "probes %s\n",
	    data.surfaces.size(), stage->lightmap.width, stage->lightmap.height,
	    stage->lightmap.Directional() ? " directional" : "",
	    stage->indirect.empty()           ? ", no indirect layer"
	    : stage->indirectGradient.empty() ? ", indirect layer (flat)"
	                                      : ", indirect layer (directional)",
	    stage->runtimeDirect ? ", runtime direct light" : ", baked direct light",
	    stage->probes ? "yes" : "no", stage->reflectionProbes.empty() ? "no" : "yes" );
	data.stage = std::move( stage );
	const std::uint32_t materialBase = std::uint32_t( data.materials.size() );
	for ( pass::world::WorldMaterial &material : m_StaticMaterials )
		data.materials.push_back( material );
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
	data.staticInstances = m_StaticInstances;
	m_Pass.SetWorld( std::move( data ) );
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
	unsigned int instances = 0;
	unsigned int noShadow = 0;
	unsigned int cutout = 0;
	for ( std::uint32_t i = 0; i < m_StaticInstances.size(); ++i )
	{
		if ( !m_Pass.DrawsStaticInstance( i ) )
			continue;
		if ( !m_StaticCastsShadow[i] )
		{
			++noShadow;
			continue;
		}
		const pass::world::WorldData::StaticInstance &instance = m_StaticInstances[i];
		if ( instance.mesh >= m_StaticMeshes.size() )
			continue;
		const pass::world::WorldData::StaticMesh &mesh = m_StaticMeshes[instance.mesh];
		std::vector<const pass::world::WorldSurface *> surfaces;
		for ( std::size_t s = 0; s < mesh.surfaces.size(); ++s )
		{
			if ( instance.surfaceSelection &&
			     !std::binary_search( instance.surfaceSelection->begin(),
			         instance.surfaceSelection->end(), std::uint32_t( s ) ) )
				continue;
			std::uint32_t material = mesh.surfaces[s].material;
			if ( !mesh.skinMaterials.empty() )
				material = mesh.skinMaterials[instance.skin][s];
			if ( material < m_StaticMaterials.size() &&
			     OpaqueShadowMaterial( m_StaticMaterials[material] ) )
				surfaces.push_back( &mesh.surfaces[s] );
		}
		if ( surfaces.empty() )
		{
			++cutout;
			continue;
		}
		bool valid = true;
		for ( const pass::world::WorldSurface *surface : surfaces )
		{
			for ( std::uint32_t index = surface->firstIndex;
			    index < surface->firstIndex + surface->indexCount; ++index )
				valid = valid && mesh.indices[index] < mesh.vertices.size();
		}
		if ( !valid )
			continue;
		const std::uint32_t firstVertex = std::uint32_t( casters->positions.size() / 3 );
		for ( const material::SurfaceModelVertex &vertex : mesh.vertices )
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
		for ( const pass::world::WorldSurface *surface : surfaces )
		{
			for ( std::uint32_t index = surface->firstIndex;
			    index < surface->firstIndex + surface->indexCount; ++index )
			{
				const std::uint32_t vertex = firstVertex + mesh.indices[index];
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
	{
		std::lock_guard<std::mutex> guard( m_ShadowLock );
		casters->generation = ++m_CasterGeneration;
		m_Casters = std::move( casters );
		m_ShadowWork.clear();
	}
	std::fprintf( stderr,
	    "Render core: %u opaque static props cast core shadows (%u authored no-shadow, %u "
	    "cutout-only)\n",
	    instances, noShadow, cutout );
}

void CoreWorld::SetStaticProps( const RenderCoreStaticModel *models, unsigned int modelCount,
    const RenderCoreStaticProp *props, unsigned int propCount )
{
	if ( ( modelCount && !models ) || ( propCount && !props ) )
		return;
	m_StaticMeshes.clear();
	m_StaticMaterials.clear();
	m_StaticInstances.clear();
	m_StaticCastsShadow.clear();
	m_ModelPoseSources.clear();
	m_StaticMeshes.resize( modelCount );
	m_ModelPoseSources.resize( modelCount );
	m_StaticInstances.reserve( propCount );
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
		ModelPoseSource &poseSource = m_ModelPoseSources[i];
		poseSource.bodyParts = model.bodyParts;
		poseSource.lodCount = std::uint32_t( model.lodTextures.size() );
		if ( model.bones.size() <= 255 )
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
		mesh.skinMaterials.resize( model.skinFamilies.size() );
		bool complete = true;
		for ( const mdl::Mesh &part : model.meshes )
		{
			const bool resolvedLod = part.lod < source.materialLodCount;
			const bool unchangedLod = model.lodTextures[part.lod] == model.lodTextures[0];
			const std::uint32_t lodMaterialBase =
			    materialBase + ( resolvedLod ? part.lod * source.materialCount : 0u );
			const std::int32_t texture =
			    model.skinFamilies.empty()
			        ? part.textureRef
			        : ( part.textureRef >= 0 &&
			                      std::size_t( part.textureRef ) < model.skinFamilies[0].size()
			                  ? model.skinFamilies[0][std::size_t( part.textureRef )]
			                  : -1 );
			if ( texture < 0 || std::uint32_t( texture ) >= source.materialCount )
			{
				mesh.surfaces.clear();
				complete = false;
				break;
			}
			const std::uint32_t base = std::uint32_t( mesh.vertices.size() );
			for ( std::size_t vertexIndex = 0; vertexIndex < part.vertices.size(); ++vertexIndex )
			{
				const mdl::Vertex &from = part.vertices[vertexIndex];
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
				mesh.vertices.push_back( to );
				if ( !poseSource.poseToBone.empty() && vertexIndex < part.weights.size() )
				{
					const mdl::BoneWeights &weights = part.weights[vertexIndex];
					pass::skinning::SkinVertex skin;
					std::copy( to.position, to.position + 3, skin.position );
					std::copy( to.normal, to.normal + 3, skin.normal );
					std::copy( to.tangent, to.tangent + 4, skin.tangent );
					skin.weight0 = weights.weights[0];
					skin.weight1 = weights.count > 1 ? weights.weights[1] : 0.0f;
					for ( int bone = 0; bone < 3; ++bone )
						skin.bones |= std::uint32_t( weights.bones[bone] ) << ( bone * 8 );
					poseSource.vertices.push_back( skin );
				}
			}
			pass::world::WorldSurface surface;
			surface.material =
			    resolvedLod || unchangedLod ? lodMaterialBase + std::uint32_t( texture ) : ~0u;
			surface.firstIndex = std::uint32_t( mesh.indices.size() );
			for ( std::uint32_t index : part.indices )
				mesh.indices.push_back( base + index );
			surface.indexCount = std::uint32_t( mesh.indices.size() ) - surface.firstIndex;
			mesh.surfaces.push_back( surface );
			poseSource.surfaceBodies.push_back(
			    { std::uint32_t( part.bodyPart ), part.bodyModel, part.lod } );
			for ( std::size_t skin = 0; skin < model.skinFamilies.size(); ++skin )
			{
				const std::vector<std::int16_t> &family = model.skinFamilies[skin];
				const std::int32_t selected =
				    part.textureRef >= 0 && std::size_t( part.textureRef ) < family.size()
				        ? family[std::size_t( part.textureRef )]
				        : -1;
				mesh.skinMaterials[skin].push_back(
				    ( resolvedLod || unchangedLod ) && selected >= 0 &&
				            std::uint32_t( selected ) < source.materialCount
				        ? lodMaterialBase + std::uint32_t( selected )
				        : ~0u );
			}
		}
		poseSource.parsed = complete;
	}
	for ( unsigned int i = 0; i < propCount; ++i )
	{
		pass::world::WorldData::StaticInstance instance;
		instance.mesh = props[i].skin >= 0 ? props[i].model : ~0u;
		instance.skin = props[i].skin >= 0 ? std::uint32_t( props[i].skin ) : ~0u;
		if ( instance.mesh < m_ModelPoseSources.size() && m_ModelPoseSources[instance.mesh].parsed )
			instance.surfaceSelection = m_ModelPoseSources[instance.mesh].SelectedSurfaces( 0 );
		else
			instance.mesh = ~0u;
		for ( int row = 0; row < 3; ++row )
			std::copy(
			    props[i].world + row * 4, props[i].world + row * 4 + 4, instance.world + row * 4 );
		instance.world[15] = 1.0f;
		m_StaticInstances.push_back( instance );
		m_StaticCastsShadow.push_back( props[i].castsShadow );
	}
	unsigned int parsed = 0;
	for ( const pass::world::WorldData::StaticMesh &mesh : m_StaticMeshes )
		parsed += !mesh.surfaces.empty() ? 1u : 0u;
	std::fprintf( stderr, "Render core: static meshes: %u of %u parsed, %zu instances\n", parsed,
	    modelCount, m_StaticInstances.size() );
	if ( m_StageSet )
	{
		SetStage();
		SetStaticCasters();
	}
}

bool CoreWorld::StageCapture::UploadLightmap(
    const world_mesh_gpu::WorldLightmapUploadRequest &request )
{
	if ( request.regions )
	{
		// A partial update of the total layer: rectangles of its flat page
		// (a directional layer's left half), patched into this capture's
		// copy and sent to the pass as they are. One in the gradient half
		// sends the whole page.
		if ( lightmap.flat.empty() || request.height != lightmap.height ||
		     request.width != ( lightmap.Directional() ? 2 : 1 ) * lightmap.width )
			return false;
		std::vector<pass::world::WorldPass::StageRegion> regions;
		bool flatOnly = true;
		std::size_t offset = 0;
		for ( std::uint32_t i = 0; i < request.regionCount; ++i )
		{
			const world_mesh_gpu::ProbeAtlasRegion &region = request.regions[i];
			if ( region.x + region.width > lightmap.width ||
			     region.y + region.height > lightmap.height )
				flatOnly = false;
			regions.push_back( { region.x, region.y, region.width, region.height } );
			offset += std::size_t( region.width ) * region.height * 8;
		}
		const std::byte *texels = static_cast<const std::byte *>( request.regionTotal );
		if ( offset && !texels )
			return false;
		// The capture's copy, as the pass's page will be.
		std::size_t at = 0;
		for ( const auto &region : regions )
		{
			const std::size_t row = std::size_t( region.width ) * 8;
			for ( std::uint32_t y = 0; y < region.height; ++y, at += row )
			{
				const std::uint32_t py = region.y + y;
				for ( std::uint32_t x = 0; x < region.width; ++x )
				{
					const std::uint32_t px = region.x + x;
					std::vector<std::byte> &page =
					    px < lightmap.width ? lightmap.flat : lightmap.gradient;
					const std::uint32_t pageX = px < lightmap.width ? px : px - lightmap.width;
					std::memcpy( page.data() + ( std::size_t( py ) * lightmap.width + pageX ) * 8,
					    texels + at + std::size_t( x ) * 8, 8 );
				}
			}
		}
		if ( m_Owner.m_StageSet )
		{
			if ( flatOnly )
				m_Owner.m_Pass.SetStageLightmapRegions(
				    std::move( regions ), std::vector<std::byte>( texels, texels + offset ) );
			else
				m_Owner.m_Pass.SetStageLightmap( lightmap );
		}
		return true;
	}
	// The total layer's pages (the baked diffuse light, as render_lab draws
	// it without runtime direct light) and the indirect layer's pages (its
	// gradient page when the bake wrote its own).
	const std::size_t layerBytes = std::size_t( request.width ) * request.height * 8;
	pass::world::LightmapPages total;
	pass::world::LightmapPages indirect;
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
			indirect = pass::world::SplitLightmapLayer( layer, request.width, request.height );
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
	this->indirect = std::move( indirect.flat );
	indirectGradient = std::move( indirect.gradient );
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
		// A part of the current atlas and optional change: applied to their
		// kept copies, then passed on as regions. Before the first whole
		// volume there is nothing to apply it to.
		if ( !probes )
			return true;
		if ( probes->atlas.size() != atlasBytes || probes->atlasWidth != request.atlasWidth ||
		     probes->atlasHeight != request.atlasHeight ||
		     ( request.regionCount && ( !request.regions || !request.regionAtlas ) ) )
			return false;
		std::vector<pass::world::WorldPass::StageRegion> regions;
		std::vector<std::byte> atlasTexels;
		std::vector<std::byte> texels;
		std::size_t packedBytes = 0;
		for ( std::uint32_t i = 0; i < request.regionCount; ++i )
		{
			const world_mesh_gpu::ProbeAtlasRegion &region = request.regions[i];
			if ( region.x > request.atlasWidth || region.width > request.atlasWidth - region.x ||
			     region.y > request.atlasHeight || region.height > request.atlasHeight - region.y )
				return false;
			regions.push_back( { region.x, region.y, region.width, region.height } );
			packedBytes += std::size_t( region.width ) * region.height * 8;
		}
		const std::byte *atlas = static_cast<const std::byte *>( request.regionAtlas );
		std::size_t atlasOffset = 0;
		for ( const auto &region : regions )
		{
			const std::size_t row = std::size_t( region.width ) * 8;
			for ( std::uint32_t y = 0; y < region.height; ++y )
			{
				std::memcpy(
				    probes->atlas.data() +
				        ( std::size_t( region.y + y ) * request.atlasWidth + region.x ) * 8,
				    atlas + atlasOffset, row );
				atlasOffset += row;
			}
		}
		if ( packedBytes )
			atlasTexels.assign( atlas, atlas + packedBytes );
		if ( request.regionDelta )
		{
			const std::byte *packed = static_cast<const std::byte *>( request.regionDelta );
			if ( change.size() != atlasBytes )
				change.assign( atlasBytes, std::byte( 0 ) );
			std::size_t offset = 0;
			for ( std::uint32_t i = 0; i < request.regionCount; ++i )
			{
				const world_mesh_gpu::ProbeAtlasRegion &region = request.regions[i];
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
			m_Owner.m_Pass.SetStageProbeRegions( std::move( regions ), std::move( atlasTexels ),
			    std::move( texels ), std::move( volume ) );
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
	if ( !probes || probes->atlas.size() != atlasBytes ||
	     probes->atlasWidth != request.atlasWidth || probes->atlasHeight != request.atlasHeight )
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
	const std::byte *atlas = static_cast<const std::byte *>( request.atlas );
	probes->atlas.assign( atlas, atlas + atlasBytes );
	if ( m_Owner.m_StageSet )
	{
		m_Owner.m_Pass.SetStageProbeVolume( probes->atlas, change, std::move( volume ) );
	}
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

CoreWorld::ViewLightInputs CoreWorld::TakeViewLightInputs(
    const float worldToView[16], const float viewToClip[16], const float viewport[6] ) const
{
	ViewLightInputs in;
	if ( worldToView )
		std::copy( worldToView, worldToView + 16, in.worldToView );
	if ( viewToClip )
		std::copy( viewToClip, viewToClip + 16, in.viewToClip );
	if ( viewport )
		std::copy( viewport, viewport + 6, in.viewport );
	// A relit BSP retains switchable world lights but strips its baked ones.
	// Merge by individual lamp so that one live light does not hide the map's
	// always-on direct lights from the model point.
	in.lights = pass::lights::MergeMapLights( m_Lights.lights, m_MapLights );
	// The area lights: the map's (baked light fixtures) and the frame's
	// emitting surfaces; the sun: the map's.
	in.areas = ViewAreaLights( true );
	in.mapAreas = std::min( m_MapLights.areas.size(), in.areas.size() );
	in.sun = m_MapLights.sun;
	in.sunMask = m_StageSunMask;
	in.shadowQuality = m_ShadowQuality.load( std::memory_order_relaxed );
	if ( in.shadowQuality > 0 && m_ShadowMovers.load( std::memory_order_relaxed ) )
	{
		in.movers = m_Lights.occluders;
	}
	return in;
}

std::shared_ptr<const pass::world::StageViewLights> CoreWorld::StageViewLightsFor(
    const ViewLightInputs &in, std::shared_ptr<const ShadowWork> *shadows )
{
	*shadows = nullptr;
	const float *worldToView = in.worldToView;
	const float *viewToClip = in.viewToClip;
	const float *viewport = in.viewport;
	if ( viewport[2] < 1.0f || viewport[3] < 1.0f )
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
	const std::vector<light_set::RuntimeLight> &frameLights = in.lights;
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
	out->view.counts[2] = viewport[0];
	out->view.counts[3] = viewport[1];
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
	std::vector<pass::lights::AreaFroxelMask> areaMasks;
	if ( !pass::lights::AssignAreaLights( grid.Value(), in.areas, areaMasks ) )
		return nullptr;
	pass::lights::AppendAreaMasks( areaMasks, out->indices );
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
	const std::vector<area_light::AreaLight> &areas = in.areas;
	const std::optional<pass::lights::MapSun> &sun = in.sun;
	pass::shadows::ShadowPlan plan;
	if ( ( !shadowed.empty() || !areas.empty() || sun ) && in.shadowQuality > 0 )
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
		input.atlasSize = ShadowAtlasFor( in.shadowQuality );
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
	PackViewAreaLights( areas, in.mapAreas, plan.areaTiles, *out );
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
		out->sunShadow[2] = in.sunMask ? 1.0f : 0.0f;
	}
	// The view group's light records: at least one (an empty view's lists
	// name none of them).
	if ( out->lights.empty() )
		out->lights.emplace_back();
	if ( !plan.views.empty() )
	{
		out->shadowTiles = std::move( plan.tiles );
		work->views = std::move( plan.views );
		work->movers = in.movers;
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
    std::size_t mapAreas, const std::vector<int> &areaTiles, pass::world::StageViewLights &out )
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

bool CoreWorld::PoseModel(
    const RenderCorePosedModel &source, pass::world::WorldView::PosedModel &out ) const
{
	if ( !source.boneToWorld || source.model >= m_ModelPoseSources.size() ||
	     source.model >= m_StaticMeshes.size() )
		return false;
	const ModelPoseSource &pose = m_ModelPoseSources[source.model];
	const pass::world::WorldData::StaticMesh &mesh = m_StaticMeshes[source.model];
	if ( !pose.parsed || source.lod >= pose.lodCount || pose.poseToBone.empty() ||
	     source.boneCount < pose.poseToBone.size() || pose.vertices.size() != mesh.vertices.size() )
		return false;
	out.surfaceSelection = pose.SelectedSurfaces( source.body, source.lod );
	if ( !m_Pass.DrawsPosedModel( source.model, source.skin, source.phase, out.surfaceSelection ) )
		return false;
	// Only the selected topology borrows the live palette. The host may have
	// prepared no matrices for bones used exclusively by other body groups/LODs.
	std::vector<bool> usedVertices( pose.vertices.size() );
	for ( std::uint32_t surfaceId : *out.surfaceSelection )
	{
		const pass::world::WorldSurface &surface = mesh.surfaces[surfaceId];
		for ( std::uint32_t i = surface.firstIndex; i < surface.firstIndex + surface.indexCount;
		    ++i )
			usedVertices[mesh.indices[i]] = true;
	}
	std::vector<pass::skinning::SkinVertex> active;
	std::vector<std::uint32_t> activeIndices;
	std::vector<bool> usedBones( pose.poseToBone.size() );
	for ( std::uint32_t i = 0; i < pose.vertices.size(); ++i )
	{
		if ( !usedVertices[i] )
			continue;
		const pass::skinning::SkinVertex &vertex = pose.vertices[i];
		const auto weights = vertex.Weights();
		for ( unsigned int influence = 0; influence < 3; ++influence )
		{
			if ( weights[influence] != 0.0f )
			{
				const std::uint32_t bone = ( vertex.bones >> ( influence * 8 ) ) & 0xffu;
				if ( bone >= usedBones.size() )
					return false;
				usedBones[bone] = true;
			}
		}
		active.push_back( vertex );
		activeIndices.push_back( i );
	}
	std::vector<pass::skinning::BoneMatrix> palette( pose.poseToBone.size() );
	for ( std::size_t bone = 0; bone < palette.size(); ++bone )
	{
		if ( !usedBones[bone] )
			continue;
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
	out.vertices = mesh.vertices;
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
		view.posedModels.push_back( std::move( pose ) );
	}
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
	// A stage view's lights are clustered and its shadows planned when its
	// slot records (the render sequence), from what the frame holds now.
	std::shared_ptr<PendingView> pending;
	if ( m_StageSet && worldToView && viewToClip )
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
			pending->inputs = TakeViewLightInputs( worldToView, viewToClip, viewport );
			if ( m_QueuedLighting.size() < 64 )
				m_QueuedLighting.push_back( { hostFrame, m_Lights.revision, movers, pending } );
		}
		++m_StageLitViews;
	}
	else if ( !m_StageSet )
	{
		view.lights = AreaViewLights();
	}
	const std::uint32_t tag = m_Pass.QueueView( std::move( view ) );
	if ( tag == 0 )
		return false;
	if ( pending )
	{
		std::lock_guard<std::mutex> guard( m_ShadowLock );
		m_ShadowWork.emplace_back( tag, std::move( pending ) );
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

std::uint32_t CoreWorld::SlotStages() const
{
	const frame::DebugControls &debug = m_Renderer.AppliedDebug();
	return m_CoreOnly && !frame::PixelViewActive( debug ) &&
	               debug.legacy != frame::DebugLegacy::kSkip
	           ? 1u << static_cast<std::uint32_t>( frame::Stage::kHud )
	           : 0u;
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
	if ( !m_CoreOnly && !frame::PixelViewActive( debug ) &&
	     debug.legacy != frame::DebugLegacy::kSkip )
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
	out->staticInstancesQueued = stats.staticInstancesQueued;
	out->staticDrawsDrawn = stats.staticDrawsDrawn;
	out->posedModelsQueued = stats.posedModelsQueued;
	out->posedDrawsDrawn = stats.posedDrawsDrawn;
	out->dynamicDrawsDrawn = stats.dynamicDrawsDrawn;
	out->dynamicDrawsRefused = stats.dynamicDrawsRefused;
	std::snprintf( out->lastRefusal, sizeof( out->lastRefusal ), "%s", stats.lastRefusal.c_str() );
	out->debugHatches = m_Hatches.load( std::memory_order_relaxed );
	out->debugTints = m_Tints.load( std::memory_order_relaxed );
	out->debugViewsRedrawn = m_Redrawn.load( std::memory_order_relaxed );
	out->stageLights =
	    unsigned( pass::lights::MergeMapLights( m_Lights.lights, m_MapLights ).size() );
	out->stageLitViews = m_StageLitViews;
	out->stageLightingBuilds = m_StageLightingBuilds.load( std::memory_order_relaxed );
	out->stageRuntimeDirect = m_StageRuntimeDirect.load( std::memory_order_relaxed ) ? 1u : 0u;
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
	m_DynamicDraws.store( quality.dynamicDraws, std::memory_order_relaxed );
	m_CoreOnly = quality.coreOnly;
	m_AoQuality.store( std::clamp( quality.ambientOcclusion, 0, 4 ), std::memory_order_relaxed );
	m_ShadowQuality.store( std::clamp( quality.shadows, 0, 3 ), std::memory_order_relaxed );
	m_DepthPrepass.store( quality.depthPrepass != 0, std::memory_order_relaxed );
	m_ShadowMovers.store( quality.shadowMovers != 0, std::memory_order_relaxed );
	m_RuntimeDirect.store( quality.runtimeDirect != 0, std::memory_order_relaxed );
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
		    "0 0 %.2f shadow tiles drawn (count; %.2f kept, %.2f with movers per frame)\n",
		    double( tilesDrawn ) / frames, double( tilesKept ) / frames,
		    double( m_ShadowTilesMoving.exchange( 0, std::memory_order_relaxed ) ) / frames );
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

std::optional<pass::world::WorldSceneColor> CoreWorld::Capture( device::IRenderDevice2 &device,
    device::CommandEncoder &encoder, device::TextureId source,
    const device::TextureDesc &sourceDesc, std::uint64_t frame )
{
	auto captured = graph::RecordSceneColor( device, encoder, source, sourceDesc );
	if ( !captured )
		return std::nullopt;
	graph::RecordedSceneColor value = std::move( captured ).Value();
	pass::world::WorldSceneColor result{ value.texture, value.desc };
	m_SceneCaptures.emplace_back( frame, std::move( value.resources ) );
	return result;
}

std::uint32_t CoreWorld::QueueMesh( const legacy::CoreMeshDraw &draw )
{
	if ( !AcceptsMeshes() && draw.kind == legacy::CoreMeshKind::kSurface )
		return 0;
	if ( !draw.name || !draw.shader || !draw.vertices || !draw.indices || !draw.vertexCount ||
	     !draw.indexCount || ( draw.variableCount && !draw.variables ) )
		return 0;
	pass::world::WorldView view;
	std::copy_n( draw.toClip, 16, view.toClip );
	view.viewport = draw.viewport;
	pass::world::WorldView::DynamicDraw geometry;
	geometry.material.name = draw.name;
	geometry.material.shader =
	    draw.kind == legacy::CoreMeshKind::kStencilClear ? "UnlitGeneric" : draw.shader;
	if ( draw.kind == legacy::CoreMeshKind::kStencilClear )
	{
		geometry.material.variables.emplace_back( "$vertexcolor", "1" );
		geometry.material.variables.emplace_back( "$vertexalpha", "1" );
		geometry.material.variables.emplace_back( "$nofog", "1" );
	}
	geometry.material.mesh = draw.mesh;

	for ( std::uint32_t i = 0;
	    draw.kind != legacy::CoreMeshKind::kStencilClear && i < draw.variableCount; ++i )
	{
		const legacy::CoreMeshVariable &variable = draw.variables[i];
		if ( !variable.key || !variable.value )
			return 0;
		geometry.material.variables.emplace_back( variable.key, variable.value );
		const char *declared = m_Host && m_Host->materialDefault
		                           ? m_Host->materialDefault( draw.shader, variable.key )
		                           : nullptr;
		if ( !declared )
			declared = variable.defaultValue;
		if ( declared )
			geometry.material.defaults.emplace_back( variable.key, declared );
		if ( variable.textureHandle )
			geometry.material.textures.emplace_back( variable.key, variable.textureHandle );
	}
	geometry.vertices.assign( draw.vertices, draw.vertices + draw.vertexCount );
	geometry.indices.assign( draw.indices, draw.indices + draw.indexCount );
	geometry.lightmapPage = draw.lightmapPage;
	view.dynamicDraws.push_back( std::move( geometry ) );
	const std::uint32_t tag = m_Pass.QueueView( std::move( view ) );
	if ( tag )
	{
		StreamView stream;
		std::copy_n( draw.worldToView, 16, stream.view.begin() );
		std::copy_n( draw.viewToClip, 16, stream.projection.begin() );
		std::lock_guard<std::mutex> guard( m_ShadowLock );
		m_StreamViews.emplace( tag, stream );
	}
	return tag;
}

void CoreWorld::RecordSlot(
    std::uint32_t tag, device::CommandEncoder &encoder, const legacy::CorePassTarget &target )
{
	if ( tag == legacy::kCorePassLegacyHud )
		return;
	if ( target.device )
		std::erase_if( m_SceneCaptures,
		    [&]( std::pair<std::uint64_t, graph::InlineGraphResources> &old )
		    {
			    if ( target.frame == 0 || old.first >= target.frame )
				    return false;
			    old.second.Release( *target.device, target.submitted );
			    return true;
		    } );
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
	world.drawState = target.drawState;
	std::memcpy( world.clipPlanes, target.clipPlanes, sizeof( world.clipPlanes ) );
	world.overrideDepthRange = true;
	world.minDepth = target.minDepth;
	world.maxDepth = target.maxDepth;
	world.device = target.device;
	// The sRGB view when the target has one; else the unorm view, and the
	// shader encodes (the same curve, the output encoding frame term).
	world.encodeOutput = !target.colorSrgb.IsValid();
	world.color = world.encodeOutput ? target.color : target.colorSrgb;
	world.colorFormat = world.encodeOutput ? target.colorFormat : target.colorSrgbFormat;
	world.colorCopySource = target.colorCopySource;
	world.sceneColorCapture = this;
	world.depth = target.depth;
	world.depthFormat = target.depthFormat;
	world.width = target.width;
	world.height = target.height;
	world.samples = target.samples;
	world.textures = textures ? &*textures : nullptr;
	world.submitted = target.submitted;
	world.frame = target.frame;
	world.streamEpoch = target.streamEpoch;
	world.lightmapScale = target.lightmapScale;
	world.depthPrepass = m_DepthPrepass.load( std::memory_order_relaxed );
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
	std::shared_ptr<const PendingView> pending;
	{
		std::lock_guard<std::mutex> guard( m_ShadowLock );
		for ( const auto &[queued, view] : m_ShadowWork )
		{
			if ( queued == tag )
				pending = view;
		}
	}
	std::optional<StreamView> streamView;
	{
		std::lock_guard<std::mutex> guard( m_ShadowLock );
		if ( target.streamEpoch != 0 )
			std::erase_if( m_StreamViews,
			    [&]( const auto &entry )
			    {
				    return entry.second.recordedStream != 0 &&
				           entry.second.recordedStream != target.streamEpoch;
			    } );
		const auto stream = m_StreamViews.find( tag );
		if ( stream != m_StreamViews.end() )
		{
			stream->second.recordedStream = target.streamEpoch;
			streamView = stream->second;
		}
	}
	if ( streamView && m_StreamLightingFrame == target.frame &&
	     world.color == m_StreamLighting.color && world.depth == m_StreamLighting.depth &&
	     world.width == m_StreamLighting.width && world.height == m_StreamLighting.height &&
	     streamView->view == m_StreamLightingView.view &&
	     streamView->projection == m_StreamLightingView.projection )
	{
		world.lights = m_StreamLighting.lights;
		world.shadowAtlas = m_StreamLighting.shadowAtlas;
		world.shadowAtlasDesc = m_StreamLighting.shadowAtlasDesc;
		world.ambientOcclusion = m_StreamLighting.ambientOcclusion;
		world.ambientOcclusionDesc = m_StreamLighting.ambientOcclusionDesc;
	}

	std::shared_ptr<const ShadowWork> shadows;
	if ( pending )
	{
		std::call_once( pending->made,
		    [&]
		    {
			    pending->lights = StageViewLightsFor( pending->inputs, &pending->shadows );
			    m_StageLightingBuilds.fetch_add( 1, std::memory_order_relaxed );
		    } );
		world.lights = pending->lights;
		shadows = pending->shadows;
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
	if ( shadows && target.device && aoQuality == 0 &&
	     EnsureOcclusion( *target.device, encoder, target.width, target.height, target.submitted ) )
	{
		// Off: the pass binds the target, held at one (the neutral term).
		if ( !m_OcclusionNeutral )
		{
			encoder.TransitionTexture( m_Occlusion, device::ResourceUsage::kSampled,
			    device::ResourceUsage::kCopyDestination );
			encoder.ClearTexture( m_Occlusion, { 1, 1, 1, 1 } );
			encoder.TransitionTexture( m_Occlusion, device::ResourceUsage::kCopyDestination,
			    device::ResourceUsage::kSampled );
			m_OcclusionNeutral = true;
		}
		world.ambientOcclusion = m_Occlusion;
		world.ambientOcclusionDesc = m_OcclusionDesc;
	}
	if ( shadows && target.device && aoQuality > 0 &&
	     EnsureOcclusion( *target.device, encoder, target.width, target.height, target.submitted ) )
	{
		m_OcclusionNeutral = false;
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
	if ( pending )
	{
		std::copy_n( pending->inputs.worldToView, 16, m_StreamLightingView.view.begin() );
		std::copy_n( pending->inputs.viewToClip, 16, m_StreamLightingView.projection.begin() );
		m_StreamLightingFrame = target.frame;
		m_StreamLighting = world;
		m_StreamLighting.device = nullptr;
		m_StreamLighting.textures = nullptr;
		m_StreamLighting.sceneColorCapture = nullptr;
		m_StreamLighting.screenPasses = {};
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
	m_ShadowTilesKept.fetch_add( work.views.size() - dirty.size(), std::memory_order_relaxed );
	m_ShadowTilesDrawn.fetch_add( dirty.size(), std::memory_order_relaxed );
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
	// The moving casters (the frame's occluder boxes, render.dynamic-occlusion):
	// a unit cube placed by each box, per view that sees it.
	std::vector<std::vector<pass::shadows::ShadowCaster>> moverCasters( work.views.size() );
	std::vector<std::uint64_t> moverSignature( work.views.size(), 0 );
	bool anyMover = false;
	if ( !work.movers.empty() )
	{
		const resources::MeshEntry *box = BoxCasterMesh();
		for ( std::size_t v = 0; box && v < work.views.size(); ++v )
		{
			for ( const light_set::RuntimeOccluder &occluder : work.movers )
			{
				const dynamic_occlusion::Box &mover = occluder.box;
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
				if ( inside( work.views[v].viewProjection, bounds ) )
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
	const bool composite = anyMover || !atlas.moverTiles.empty();
	if ( dirty.empty() && !composite )
	{
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
	if ( !dirty.empty() )
	{
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
		uploads( builder );
		const graph::ResourceRef atlasRef = builder.ImportTexture(
		    "stage shadow atlas", atlas.texture, atlas.desc, atlas.usage, ResourceUsage::kSampled );
		pass::shadows::ShadowAtlasTarget target{ atlasRef, work.atlasSize, work.guardTexels };
		target.keep = !whole;
		if ( !m_ShadowRenderer->AddPasses( builder, target, views ) ||
		     !execute( std::move( builder ) ) )
			return {};
		atlas.usage = ResourceUsage::kSampled;
		atlas.drawn = work.views;
		atlas.generation = casters->generation;
		atlas.guardTexels = work.guardTexels;
	}
	if ( !composite )
	{
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
	++m_AtlasNext;
	*desc = atlas.desc;
	return atlas.composite;
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
	if ( m_ShadowDevice != &device )
		return;
	if ( m_Occlusion.IsValid() )
		(void)device.Release( m_Occlusion, device::CompletionToken() );
	m_Occlusion = device::TextureId();
	m_OcclusionDesc = device::TextureDesc();
	m_Ao.reset();
	for ( const Atlas &atlas : m_Atlases )
	{
		(void)device.Release( atlas.texture, device::CompletionToken() );
		if ( atlas.composite.IsValid() )
			(void)device.Release( atlas.composite, device::CompletionToken() );
	}
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
