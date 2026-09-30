//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.composition's world (RFC 0016 K5); see core_world.h.
//
//=============================================================================//

#include "core_world.h"

#include "mapcontainer/world_lightmap.h"
#include "mapcontainer/world_mesh_decode.h"

#include <algorithm>
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
	auto stage = std::make_shared<pass::world::WorldStage>();
	stage->lightmap = m_Capture.lightmap;
	if ( m_Capture.indirect.size() == m_Capture.lightmap.flat.size() )
		stage->indirect = m_Capture.indirect;
	stage->probes = m_Capture.probes;
	stage->reflectionWidth = m_Capture.reflectionWidth;
	stage->reflectionHeight = m_Capture.reflectionHeight;
	stage->reflectionProbes = m_Capture.reflection;
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
	// A volume without a change is the bake (world_mesh_upload.h): the one a
	// stage starts from, and its change is none.
	std::vector<std::byte> change;
	if ( request.deltaAtlas )
	{
		const std::byte *delta = static_cast<const std::byte *>( request.deltaAtlas );
		change.assign( delta, delta + atlasBytes );
	}
	else if ( !probes || probes->atlas.size() != atlasBytes || !m_Owner.m_StageSet )
	{
		const std::byte *atlas = static_cast<const std::byte *>( request.atlas );
		volume.atlas.assign( atlas, atlas + atlasBytes );
		// The stage's own table has the grids' rows alone.
		pass::world::StageProbeVolume baked = volume;
		baked.rows = request.gridCount;
		baked.table.resize( std::size_t( baked.rows ) * request.tableFloats );
		probes = std::move( baked );
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

bool CoreWorld::DrawView( const unsigned int *surfaces, unsigned int count,
    const float worldToClip[16], const float viewport[6], unsigned long long hostFrame )
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

} // namespace render::composition
