//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of hammer/adapters/render/viewport_renderer.h.
//
//=============================================================================//

#include "viewport_renderer.h"

#include "render/graph/executor.h"
#include "render/graph/graph_builder.h"
#include "render/material/parameter_block.h"
#include "render/material/vmt_mapping.h"
#include "render/scene/draw_list.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <set>
#include <span>
#include <string>
#include <thread>
#include <utility>

namespace hammer::render_adapter
{

namespace
{

using namespace ::render;
using ::render::pass::lines::LineList;
using ::render::pass::lines::LinesTargets;
using ::render::pass::lines::LinesView;
using ::render::pass::lines::MeshBatch;
using ::render::pass::lines::Space;
using ::render::pass::lines::Topology;

constexpr device::Format kColorFormat = device::Format::kRGBA8Srgb;
constexpr device::Format kDepthFormat = device::Format::kD32Float;
constexpr std::uint64_t kUntextured = 1; // the untextured batch's material id
constexpr const char *kWhite = "hammer-white";
// The instances' view bits: the opaque pass's draw list, then the blended one.
constexpr std::uint32_t kOpaqueBit = 0;
constexpr std::uint32_t kBlendedBit = 1;

// The clear colors as the editor shows them, in linear light for the sRGB target.
float Linear( float display )
{
	return display <= 0.04045f ? display / 12.92f : std::pow( ( display + 0.055f ) / 1.055f, 2.4f );
}

device::ClearColor Clear( float r, float g, float b )
{
	return { Linear( r ), Linear( g ), Linear( b ), 1.0f };
}

// The unlit family reads vertex colors as gamma 2.2 (Source's convention); the
// editor's shading colors are display (sRGB) values. This table re-encodes a
// display byte so the family's decode gives its sRGB linear value, and the
// sRGB target shows the byte again (within a level of quantization).
const std::array<std::uint8_t, 256> &Gamma22FromDisplay()
{
	static const std::array<std::uint8_t, 256> table = []
	{
		std::array<std::uint8_t, 256> out{};
		for ( int i = 0; i < 256; ++i )
		{
			const double linear = Linear( float( i ) / 255.0f );
			out[i] = std::uint8_t( std::lround( std::pow( linear, 1.0 / 2.2 ) * 255.0 ) );
		}
		return out;
	}();
	return table;
}

std::string TextureName( const std::string &material )
{
	return "hammer-texture:" + material;
}

std::string FaceMeshName( std::uint64_t mesh )
{
	return "hammer-faces:" + std::to_string( mesh );
}

std::string EdgeMeshName( std::uint64_t chunk )
{
	return "hammer-edges:" + std::to_string( chunk );
}

std::string Edge2DMeshName( std::uint64_t chunk )
{
	return "hammer-edges-2d:" + std::to_string( chunk );
}

std::string ModelMeshName( std::uint64_t mesh )
{
	return "hammer-model:" + std::to_string( mesh );
}

// Instance content chunks are keyed apart from the id chunks (ChunkOf is at
// most 2^58).
constexpr std::uint64_t kInstanceChunk = std::uint64_t( 1 ) << 63;

// The untextured fill of a model mesh whose material has no texture.
constexpr scene::Rgb kModelFill{ 200, 200, 200 };

::render::math::Aabb BoundsOf( const std::vector<UnlitVertex> &vertices )
{
	::render::math::Aabb bounds;
	for ( const UnlitVertex &v : vertices )
	{
		const ::render::math::float3 p = { v.position[0], v.position[1], v.position[2] };
		if ( bounds.IsEmpty() )
		{
			bounds.min = bounds.max = p;
			continue;
		}
		bounds.min = { std::min( bounds.min.x, p.x ), std::min( bounds.min.y, p.y ),
		    std::min( bounds.min.z, p.z ) };
		bounds.max = { std::max( bounds.max.x, p.x ), std::max( bounds.max.y, p.y ),
		    std::max( bounds.max.z, p.z ) };
	}
	return bounds;
}

} // namespace

ViewportRenderer::ViewportRenderer(
    device::IRenderDevice2 &device, IMaterialTextures *textures, IModelSource *models )
    : m_Device( device ), m_Source( textures ), m_ModelSource( models ), m_MeshCache( device ),
      m_Textures( device ), m_Programs( device, m_Textures ),
      m_Scene( ::render::scene::CreateRenderScene() )
{
}

foundation::Expected<std::unique_ptr<ViewportRenderer>, ViewportStatus> ViewportRenderer::Create(
    device::IRenderDevice2 &device, IMaterialTextures *textures, IModelSource *models )
{
	std::unique_ptr<ViewportRenderer> renderer( new ViewportRenderer( device, textures, models ) );
	auto lines = pass::lines::LinesRenderer::Create( device, kColorFormat, kDepthFormat );
	auto unlit = material::UnlitFamily::Create( device, kColorFormat, kDepthFormat );
	if ( !lines || !unlit )
	{
		return foundation::MakeUnexpected( ViewportStatus::kPass );
	}
	renderer->m_Lines = std::move( lines ).Value();
	renderer->m_Unlit = std::move( unlit ).Value();
	for ( const material::FamilyDesc &family :
	    material::FamiliesFromMapping( material::BuiltinVmtMapping() ) )
	{
		(void)renderer->m_Families.Register( family );
	}
	renderer->m_UnlitSchema = renderer->m_Families.Find( "unlit" );
	if ( !renderer->m_UnlitSchema )
	{
		return foundation::MakeUnexpected( ViewportStatus::kPass );
	}
	device::TextureDesc white;
	white.format = kColorFormat;
	white.width = white.height = 1;
	white.usages = { device::ResourceUsage::kCopyDestination, device::ResourceUsage::kSampled };
	const std::byte texel[4] = {
	    std::byte( 255 ), std::byte( 255 ), std::byte( 255 ), std::byte( 255 ) };
	if ( !renderer->m_Textures.Stage( kWhite, white, texel ) ||
	     !renderer->AddProgram( kUntextured, kWhite, MaterialSurface() ) )
	{
		return foundation::MakeUnexpected( ViewportStatus::kDevice );
	}
	return renderer;
}

ViewportRenderer::~ViewportRenderer()
{
	// Our frames complete before their resources and the pass go.
	const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds( 10 );
	while ( !m_Pending.empty() && std::chrono::steady_clock::now() < deadline )
	{
		(void)m_Device.Poll();
		for ( auto it = m_Pending.begin(); it != m_Pending.end(); )
		{
			if ( m_Device.IsComplete( it->second.token ) )
			{
				Release( it->second );
				it = m_Pending.erase( it );
			}
			else
			{
				++it;
			}
		}
		std::this_thread::yield();
	}
	if ( m_LastToken.NamesSubmission() )
	{
		while ( !m_Device.IsComplete( m_LastToken ) && std::chrono::steady_clock::now() < deadline )
		{
			(void)m_Device.Poll();
			std::this_thread::yield();
		}
	}
	if ( device::IExternalImages *exporter = m_Device.ExternalImages() )
	{
		for ( const ExternalSlot &slot : m_External )
		{
			(void)m_Device.Release( slot.image.texture, m_LastToken );
			exporter->CloseHandle( slot.image.handle );
		}
	}
	m_External.clear();
	m_Lines.reset();
}

// The unlit family's claim on a material's surface, through the family's own
// rules (ClaimUnlit): the vertex colors (the editor's shading) modulate the
// base texture; $translucent blends, $additive adds, neither writes depth;
// $alphatest cuts under its reference (the family's rule: 0.7 when the VMT
// sets none, detail::AlphaTestReference). The family refuses $translucent with
// $additive (no blend mode for the pair); such a material draws translucent.
material::UnlitClaim ViewportRenderer::ClaimFor( const MaterialSurface &surface ) const
{
	material::ParameterBlock block( *m_UnlitSchema );
	(void)block.SetInt( "vertexcolor", 1 );
	(void)block.SetInt( "translucent", surface.translucent ? 1 : 0 );
	(void)block.SetInt( "additive", surface.additive && !surface.translucent ? 1 : 0 );
	(void)block.SetInt( "alphatest", surface.alphaTest ? 1 : 0 );
	(void)block.SetFloat( "alphatestreference", surface.alphaTestReference );
	(void)block.SetFloat( "alpha", surface.alpha );
	material::UnlitClaim claim = material::ClaimUnlit( block );
	if ( !claim.claimed )
	{
		// Not reachable with the parameters set above; draw it as before.
		claim = material::UnlitClaim();
		claim.claimed = true;
		claim.constants.flags[0] = 1.0f;
	}
	return claim;
}

// The unlit family's program for a material id: its base texture (a
// TextureCache name) modulated by the vertex colors, drawn as the surface
// claims.
foundation::Expected<std::uint64_t, ViewportStatus> ViewportRenderer::AddProgram(
    std::uint64_t id, const std::string &texture, const MaterialSurface &surface )
{
	auto request = m_Unlit->Request( ClaimFor( surface ), texture );
	if ( !request || !m_Programs.Set( id, request.Value() ) )
	{
		return foundation::MakeUnexpected( ViewportStatus::kDevice );
	}
	return id;
}

// Fetches the base texture of every material the solids name for the first
// time (misses are kept too) and stages its mip chain.
foundation::Expected<void, ViewportStatus> ViewportRenderer::ResolveMaterials(
    const std::vector<viewport::SolidDraw> &solids )
{
	std::set<std::string> named;
	for ( const viewport::SolidDraw &solid : solids )
	{
		for ( const viewport::FaceDraw &face : solid.faces )
		{
			named.insert( face.material );
		}
	}
	return ResolveMaterialNames( named );
}

foundation::Expected<void, ViewportStatus> ViewportRenderer::ResolveMaterialNames(
    const std::set<std::string> &names )
{
	if ( !m_Source )
	{
		return {};
	}
	for ( const std::string &name : names )
	{
		if ( name.empty() || m_Materials.count( name ) != 0 )
		{
			continue;
		}
		Material material;
		std::optional<MaterialImage> image = m_Source->BaseTexture( name );
		const std::vector<MipLevel> chain =
		    image ? BuildMipChain( *image ) : std::vector<MipLevel>();
		if ( !chain.empty() )
		{
			device::TextureDesc desc;
			desc.format = kColorFormat;
			desc.width = image->width;
			desc.height = image->height;
			desc.mipLevels = static_cast<std::uint32_t>( chain.size() );
			desc.usages = {
			    device::ResourceUsage::kCopyDestination, device::ResourceUsage::kSampled };
			std::vector<std::span<const std::byte>> levels;
			levels.reserve( chain.size() );
			for ( const MipLevel &level : chain )
			{
				levels.push_back( std::as_bytes( std::span( level.rgba ) ) );
			}
			const std::string texture = TextureName( name );
			if ( !m_Textures.StageMips( texture, desc, levels ) )
			{
				return foundation::MakeUnexpected( ViewportStatus::kDevice );
			}
			material.id = m_NextMaterial++;
			if ( auto added = AddProgram( material.id, texture, image->surface ); !added )
			{
				return foundation::MakeUnexpected( added.Error() );
			}
			material.size = TextureSize{ image->width, image->height };
			material.blended = ClaimFor( image->surface ).blend != device::BlendMode::kOpaque;
			++m_Stats.textures;
		}
		else
		{
			++m_Stats.missingTextures;
		}
		m_Materials.emplace( name, material );
	}
	return {};
}

void ViewportRenderer::Release( const Pending &pending )
{
	if ( pending.external >= 0 )
	{
		// The image stays in its slot; a frame that never reached the host
		// frees it.
		return;
	}
	(void)m_Device.Release( pending.color, pending.token );
	(void)m_Device.Release( pending.readback, pending.token );
}

// A free exported image of the size, made when there is none; free images of
// other sizes (a resized view) go.
foundation::Expected<int, ViewportStatus> ViewportRenderer::ExternalSlotFor(
    std::uint32_t width, std::uint32_t height )
{
	device::IExternalImages *exporter = m_Device.ExternalImages();
	if ( !exporter )
	{
		return foundation::MakeUnexpected( ViewportStatus::kUnsupported );
	}
	for ( std::size_t i = 0; i < m_External.size(); )
	{
		ExternalSlot &slot = m_External[i];
		if ( !slot.busy && ( slot.width != width || slot.height != height ) )
		{
			(void)m_Device.Release( slot.image.texture, m_LastToken );
			exporter->CloseHandle( slot.image.handle );
			m_External.erase( m_External.begin() + std::ptrdiff_t( i ) );
			for ( auto &[ticket, pending] : m_Pending )
			{
				if ( pending.external > int( i ) )
					--pending.external;
			}
			continue;
		}
		++i;
	}
	for ( std::size_t i = 0; i < m_External.size(); ++i )
	{
		if ( !m_External[i].busy )
		{
			m_External[i].busy = true;
			return int( i );
		}
	}
	device::TextureDesc desc;
	desc.format = kColorFormat;
	desc.width = width;
	desc.height = height;
	desc.usages = { device::ResourceUsage::kColorAttachment, device::ResourceUsage::kExternal };
	desc.debugName = "hammer-viewport-external";
	auto exported = exporter->CreateExported( desc );
	if ( !exported )
	{
		return foundation::MakeUnexpected( ViewportStatus::kDevice );
	}
	ExternalSlot slot;
	slot.image = exported.Value();
	slot.width = width;
	slot.height = height;
	slot.busy = true;
	m_External.push_back( slot );
	return int( m_External.size() - 1 );
}

void ViewportRenderer::ReturnFrame( std::uint64_t lease )
{
	for ( ExternalSlot &slot : m_External )
	{
		if ( slot.busy && slot.lease == lease && lease != 0 )
		{
			slot.busy = false;
			slot.lease = 0;
		}
	}
}

ViewportRenderer::ModelEntry *ViewportRenderer::ModelFor( const viewport::EntityDraw &entity )
{
	if ( !m_ModelSource || !viewport::IsStudioModelPath( entity.model ) )
	{
		return nullptr;
	}
	const std::string path = mdl::CanonicalModelPath( entity.model );
	auto found = m_Models.find( path );
	if ( found == m_Models.end() )
	{
		ModelEntry entry;
		auto loaded = m_ModelSource->Model( path );
		if ( loaded && loaded.Value().model.TriangleCount() > 0 )
		{
			entry.asset = std::move( loaded ).Value();
		}
		found = m_Models.emplace( path, std::move( entry ) ).first;
		++m_Stats.models;
	}
	return found->second.asset ? &found->second : nullptr;
}

foundation::Expected<const std::vector<ViewportRenderer::ModelMesh> *, ViewportStatus>
ViewportRenderer::VariantFor( ModelEntry &entry, std::int32_t skin, const scene::Rgb &tint )
{
	const std::array<int, 4> key = { skin, tint.r, tint.g, tint.b };
	if ( auto found = entry.variants.find( key ); found != entry.variants.end() )
	{
		return &found->second;
	}
	// The model's textures resolve like any face material.
	std::set<std::string> names;
	for ( const mdl::ResolvedMaterial &material : entry.asset->materials )
	{
		if ( material.found )
		{
			names.insert( material.name );
		}
	}
	if ( auto resolved = ResolveMaterialNames( names ); !resolved )
	{
		return foundation::MakeUnexpected( resolved.Error() );
	}
	const std::vector<ModelBatch> batches = BuildModelBatches(
	    *entry.asset, skin,
	    [this]( const std::string &material ) -> std::optional<TextureSize>
	    {
		    auto found = m_Materials.find( material );
		    return found == m_Materials.end() ? std::nullopt : found->second.size;
	    },
	    tint, kModelFill );
	const std::array<std::uint8_t, 256> &encode = Gamma22FromDisplay();
	std::vector<ModelMesh> meshes;
	for ( const ModelBatch &batch : batches )
	{
		const Material *material =
		    batch.material.empty() ? nullptr : &m_Materials.at( batch.material );
		std::vector<UnlitVertex> vertices = batch.vertices;
		for ( UnlitVertex &vertex : vertices )
		{
			for ( int c = 0; c < 3; ++c )
				vertex.color[c] = encode[vertex.color[c]];
		}
		ModelMesh mesh;
		mesh.material = material ? material->id : kUntextured;
		mesh.blended = material && material->blended;
		mesh.mesh = m_NextMesh++;
		mesh.bounds = BoundsOf( batch.vertices );
		resources::MeshData data;
		data.vertices = std::as_bytes( std::span<const UnlitVertex>( vertices ) );
		data.vertexStride = sizeof( UnlitVertex );
		data.indices = std::as_bytes( std::span<const std::uint32_t>( batch.indices ) );
		data.indexFormat = device::IndexFormat::kUint32;
		auto staged = m_MeshCache.Stage( ModelMeshName( mesh.mesh ), data );
		if ( !staged )
		{
			return foundation::MakeUnexpected( ViewportStatus::kDevice );
		}
		m_FaceMeshes.entries[mesh.mesh] = staged.Value();
		++m_Stats.stagedMeshes;
		meshes.push_back( mesh );
	}
	++m_Stats.modelVariants;
	return &entry.variants.emplace( key, std::move( meshes ) ).first->second;
}

void ViewportRenderer::DropChunk( Chunk &chunk, ::render::scene::ChangeSet &changes )
{
	for ( ::render::scene::InstanceId instance : chunk.instances )
	{
		changes.Remove( instance );
	}
	chunk.instances.clear();
}

foundation::Expected<void, ViewportStatus> ViewportRenderer::StageChunk(
    std::uint64_t chunkId, Chunk staged, ::render::scene::ChangeSet &changes )
{
	auto resident = m_Chunks.find( chunkId );
	if ( resident != m_Chunks.end() )
	{
		DropChunk( resident->second, changes );
	}
	if ( staged.solids.empty() && staged.entities.empty() )
	{
		// The chunk is gone: its meshes go behind the next submission.
		if ( resident != m_Chunks.end() )
		{
			for ( const auto &[material, mesh] : resident->second.meshes )
			{
				(void)m_MeshCache.Evict( FaceMeshName( mesh ) );
				m_FaceMeshes.entries.erase( mesh );
			}
			if ( resident->second.edges )
			{
				(void)m_MeshCache.Evict( EdgeMeshName( chunkId ) );
			}
			if ( resident->second.edges2D )
			{
				(void)m_MeshCache.Evict( Edge2DMeshName( chunkId ) );
			}
			m_Chunks.erase( resident );
		}
		return {};
	}
	if ( auto resolved = ResolveMaterials( staged.solids ); !resolved )
	{
		return resolved;
	}
	// Entities drawn as models: their model is read (and its variant staged)
	// here; the others draw their markers.
	std::vector<ModelEntry *> models( staged.entities.size(), nullptr );
	for ( std::size_t i = 0; i < staged.entities.size(); ++i )
	{
		const viewport::EntityDraw &entity = staged.entities[i];
		models[i] = ModelFor( entity );
		if ( viewport::IsStudioModelPath( entity.model ) && m_ModelSource )
		{
			++( models[i] ? staged.modelEntities : staged.missingModels );
		}
	}
	GeometryOptions options;
	options.sizes = [this]( const std::string &material ) -> std::optional<TextureSize>
	{
		auto found = m_Materials.find( material );
		return found == m_Materials.end() ? std::nullopt : found->second.size;
	};
	if ( staged.instanceContent )
	{
		options.tint = viewport::kInstanceTint;
		options.edgeColor = viewport::kInstanceEdgeColor;
	}
	viewport::RenderSnapshot objects;
	objects.solids = std::move( staged.solids );
	objects.entities = std::move( staged.entities );
	options.modelBox = [&]( const viewport::EntityDraw &entity ) -> std::optional<scene::Box>
	{
		const auto at = static_cast<std::size_t>( &entity - objects.entities.data() );
		if ( at >= models.size() || !models[at] )
		{
			return std::nullopt;
		}
		return ModelWorldBox( models[at]->asset->model, entity );
	};
	const SceneGeometry geometry = BuildSceneGeometry( objects, options );
	staged.solids = std::move( objects.solids );
	staged.entities = std::move( objects.entities );

	// One mesh and one scene instance per batch; a (chunk, material) keeps
	// its mesh id, so a restage replaces the mesh's buffers under its name.
	const std::map<std::uint64_t, std::uint64_t> previous =
	    resident == m_Chunks.end() ? std::map<std::uint64_t, std::uint64_t>()
	                               : resident->second.meshes;
	const std::array<std::uint8_t, 256> &encode = Gamma22FromDisplay();
	for ( const FaceBatch &batch : geometry.faces )
	{
		const Material *material =
		    batch.material.empty() ? nullptr : &m_Materials.at( batch.material );
		const std::uint64_t materialId = material ? material->id : kUntextured;
		auto kept = previous.find( materialId );
		const std::uint64_t mesh = kept != previous.end() ? kept->second : m_NextMesh++;
		std::vector<UnlitVertex> vertices = batch.vertices;
		for ( UnlitVertex &vertex : vertices )
		{
			for ( int c = 0; c < 3; ++c )
				vertex.color[c] = encode[vertex.color[c]];
		}
		resources::MeshData data;
		data.vertices = std::as_bytes( std::span<const UnlitVertex>( vertices ) );
		data.vertexStride = sizeof( UnlitVertex );
		auto entry = m_MeshCache.Stage( FaceMeshName( mesh ), data );
		if ( !entry )
		{
			return foundation::MakeUnexpected( ViewportStatus::kDevice );
		}
		m_FaceMeshes.entries[mesh] = entry.Value();
		staged.meshes[materialId] = mesh;
		++m_Stats.stagedMeshes;
		const bool blended = material && material->blended;
		::render::scene::MeshInstanceDesc desc;
		desc.mesh = mesh;
		desc.material = materialId;
		desc.localBounds = BoundsOf( batch.vertices );
		desc.viewMask = 1u << ( blended ? kBlendedBit : kOpaqueBit );
		const ::render::scene::InstanceId instance = m_Scene->Reserve();
		changes.Add( instance, desc );
		staged.instances.push_back( instance );
		staged.faceVertices += static_cast<std::uint32_t>( batch.vertices.size() );
		staged.texturedBatches += material ? 1 : 0;
		staged.blendedBatches += blended ? 1 : 0;
	}
	for ( std::size_t i = 0; i < staged.entities.size(); ++i )
	{
		ModelEntry *entry = models[i];
		if ( !entry )
		{
			continue;
		}
		const viewport::EntityDraw &entity = staged.entities[i];
		auto variant = VariantFor(
		    *entry, entity.modelKeys.skin, ModelTint( entity, staged.instanceContent ) );
		if ( !variant )
		{
			return foundation::MakeUnexpected( variant.Error() );
		}
		const ::render::math::float4x4 world = ModelWorld( entity );
		for ( const ModelMesh &mesh : *variant.Value() )
		{
			::render::scene::MeshInstanceDesc desc;
			desc.mesh = mesh.mesh;
			desc.material = mesh.material;
			desc.world = world;
			desc.localBounds = mesh.bounds;
			desc.viewMask = 1u << ( mesh.blended ? kBlendedBit : kOpaqueBit );
			const ::render::scene::InstanceId instance = m_Scene->Reserve();
			changes.Add( instance, desc );
			staged.instances.push_back( instance );
		}
	}
	for ( const auto &[materialId, mesh] : previous )
	{
		if ( staged.meshes.count( materialId ) == 0 )
		{
			(void)m_MeshCache.Evict( FaceMeshName( mesh ) );
			m_FaceMeshes.entries.erase( mesh );
		}
	}
	if ( geometry.edges.empty() )
	{
		if ( resident != m_Chunks.end() && resident->second.edges )
		{
			(void)m_MeshCache.Evict( EdgeMeshName( chunkId ) );
		}
	}
	else
	{
		resources::MeshData data;
		data.vertices = std::as_bytes( std::span<const LineVertex>( geometry.edges ) );
		data.vertexStride = sizeof( LineVertex );
		auto entry = m_MeshCache.Stage( EdgeMeshName( chunkId ), data );
		if ( !entry )
		{
			return foundation::MakeUnexpected( ViewportStatus::kDevice );
		}
		staged.edges = entry.Value();
		++m_Stats.stagedMeshes;
	}
	if ( geometry.edges2D.empty() )
	{
		if ( resident != m_Chunks.end() && resident->second.edges2D )
		{
			(void)m_MeshCache.Evict( Edge2DMeshName( chunkId ) );
		}
	}
	else
	{
		resources::MeshData data;
		data.vertices = std::as_bytes( std::span<const LineVertex>( geometry.edges2D ) );
		data.vertexStride = sizeof( LineVertex );
		auto entry = m_MeshCache.Stage( Edge2DMeshName( chunkId ), data );
		if ( !entry )
		{
			return foundation::MakeUnexpected( ViewportStatus::kDevice );
		}
		staged.edges2D = entry.Value();
		++m_Stats.stagedMeshes;
	}
	staged.triangles = geometry.triangles;
	staged.edgeVertices =
	    static_cast<std::uint32_t>( geometry.edges.size() + geometry.edges2D.size() );
	m_Chunks[chunkId] = std::move( staged );
	++m_Stats.stagedChunks;
	return {};
}

foundation::Expected<void, ViewportStatus> ViewportRenderer::SetScene(
    const viewport::RenderSnapshot &snapshot, std::uint64_t key )
{
	if ( m_HaveScene && key == m_Stats.key )
	{
		return {};
	}
	// The snapshot's objects by chunk (id order within each); each
	// instance's content is a chunk of its own.
	struct Incoming
	{
		std::vector<const viewport::SolidDraw *> first;
		std::vector<const viewport::EntityDraw *> second;
		bool instance = false;
	};
	std::map<std::uint64_t, Incoming> incoming;
	for ( const viewport::SolidDraw &solid : snapshot.solids )
	{
		incoming[ChunkOf( solid.id )].first.push_back( &solid );
	}
	for ( const viewport::EntityDraw &entity : snapshot.entities )
	{
		incoming[ChunkOf( entity.id )].second.push_back( &entity );
	}
	for ( const viewport::InstanceDraw &instance : snapshot.instances )
	{
		if ( instance.solids.empty() && instance.entities.empty() )
		{
			continue;
		}
		Incoming &content = incoming[kInstanceChunk | instance.id.value];
		content.instance = true;
		for ( const viewport::SolidDraw &solid : instance.solids )
			content.first.push_back( &solid );
		for ( const viewport::EntityDraw &entity : instance.entities )
			content.second.push_back( &entity );
	}
	auto same = []( const auto &staged, const auto &now )
	{
		return staged.size() == now.size() && std::equal( staged.begin(), staged.end(), now.begin(),
		                                          []( const auto &a, const auto *b )
		                                          {
			                                          return a == *b;
		                                          } );
	};

	m_Stats.stagedChunks = 0;
	m_Stats.stagedMeshes = 0;
	::render::scene::ChangeSet changes;
	std::vector<std::uint64_t> gone;
	for ( const auto &[chunk, resident] : m_Chunks )
	{
		if ( incoming.count( chunk ) == 0 )
			gone.push_back( chunk );
	}
	for ( std::uint64_t chunk : gone )
	{
		if ( auto dropped = StageChunk( chunk, Chunk(), changes ); !dropped )
		{
			return dropped;
		}
	}
	for ( const auto &[chunk, objects] : incoming )
	{
		auto resident = m_Chunks.find( chunk );
		if ( resident != m_Chunks.end() && same( resident->second.solids, objects.first ) &&
		     same( resident->second.entities, objects.second ) )
		{
			continue;
		}
		Chunk staged;
		staged.instanceContent = objects.instance;
		for ( const viewport::SolidDraw *solid : objects.first )
			staged.solids.push_back( *solid );
		for ( const viewport::EntityDraw *entity : objects.second )
			staged.entities.push_back( *entity );
		if ( auto rebuilt = StageChunk( chunk, std::move( staged ), changes ); !rebuilt )
		{
			return rebuilt;
		}
	}
	if ( !m_Scene->Commit( changes ) )
	{
		return foundation::MakeUnexpected( ViewportStatus::kDevice );
	}

	// The uploads run in their own submission; later frames on the queue see
	// them, and replaced buffers and groups retire behind its completion.
	auto encoder = m_Device.BeginEncoder( device::QueueKind::kGraphics );
	if ( !encoder )
	{
		return foundation::MakeUnexpected( ViewportStatus::kDevice );
	}
	m_MeshCache.RecordUploads( encoder.Value() );
	m_Textures.RecordUploads( encoder.Value() );
	m_Programs.RecordUploads( encoder.Value() );
	device::CommandEncoder encoders[] = { std::move( encoder ).Value() };
	auto token = m_Device.Submit( device::QueueKind::kGraphics, encoders, {} );
	if ( !token )
	{
		return foundation::MakeUnexpected( ViewportStatus::kDevice );
	}
	m_MeshCache.Retire( token.Value() );
	m_Textures.Retire( token.Value() );
	m_Programs.Retire( token.Value() );
	m_LastToken = token.Value();
	m_Bounds = snapshot.bounds;
	m_HaveScene = true;
	m_Stats.key = key;
	m_Stats.triangles = m_Stats.faceVertices = m_Stats.edgeVertices = 0;
	m_Stats.batches = m_Stats.texturedBatches = m_Stats.blendedBatches = 0;
	m_Stats.modelEntities = m_Stats.missingModels = m_Stats.instanceChunks = 0;
	for ( const auto &[id, chunk] : m_Chunks )
	{
		m_Stats.modelEntities += chunk.modelEntities;
		m_Stats.missingModels += chunk.missingModels;
		m_Stats.instanceChunks += chunk.instanceContent ? 1 : 0;
		m_Stats.triangles += chunk.triangles;
		m_Stats.faceVertices += chunk.faceVertices;
		m_Stats.edgeVertices += chunk.edgeVertices;
		m_Stats.batches += static_cast<std::uint32_t>( chunk.meshes.size() );
		m_Stats.texturedBatches += chunk.texturedBatches;
		m_Stats.blendedBatches += chunk.blendedBatches;
	}
	m_Stats.chunks = static_cast<std::uint32_t>( m_Chunks.size() );
	++m_Stats.stagings;
	return {};
}

foundation::Expected<ViewportRenderer::Ticket, ViewportStatus> ViewportRenderer::Render(
    const ViewRequest &request )
{
	const bool threeD = request.kind == viewport::ViewKind::Camera3D;
	if ( request.pixelWidth == 0 || request.pixelHeight == 0 ||
	     ( threeD ? !request.camera3D : !request.camera2D ) )
	{
		return foundation::MakeUnexpected( ViewportStatus::kInvalidView );
	}
	const LinesView view =
	    threeD ? ViewFor( *request.camera3D, m_Bounds ) : ViewFor( *request.camera2D );
	if ( view.width == 0 || view.height == 0 )
	{
		return foundation::MakeUnexpected( ViewportStatus::kInvalidView );
	}

	device::TextureDesc colorDesc;
	colorDesc.format = kColorFormat;
	colorDesc.width = request.pixelWidth;
	colorDesc.height = request.pixelHeight;
	colorDesc.usages = {
	    device::ResourceUsage::kColorAttachment, device::ResourceUsage::kCopySource };
	colorDesc.debugName = "hammer-viewport";
	device::TextureDesc depthDesc = colorDesc;
	depthDesc.format = kDepthFormat;
	depthDesc.usages = { device::ResourceUsage::kDepthWrite };
	depthDesc.debugName = "hammer-viewport-depth";
	device::BufferDesc readbackDesc;
	readbackDesc.size = std::uint64_t( request.pixelWidth ) * request.pixelHeight * 4;
	readbackDesc.usages = { device::ResourceUsage::kCopyDestination };
	readbackDesc.memory = device::MemoryKind::kReadback;
	readbackDesc.debugName = "hammer-viewport-readback";
	Pending pending{ {}, {}, {}, request.pixelWidth, request.pixelHeight, -1 };
	if ( request.external )
	{
		// An exported image the host shows as it is: no readback.
		auto slot = ExternalSlotFor( request.pixelWidth, request.pixelHeight );
		if ( !slot )
		{
			return foundation::MakeUnexpected( slot.Error() );
		}
		pending.external = slot.Value();
		pending.color = m_External[std::size_t( slot.Value() )].image.texture;
		colorDesc.usages = {
		    device::ResourceUsage::kColorAttachment, device::ResourceUsage::kExternal };
	}
	else
	{
		auto color = m_Device.CreateTexture( colorDesc );
		auto readback = m_Device.CreateBuffer( readbackDesc );
		if ( !color || !readback )
		{
			if ( color )
				(void)m_Device.Release( color.Value(), device::CompletionToken() );
			if ( readback )
				(void)m_Device.Release( readback.Value(), device::CompletionToken() );
			return foundation::MakeUnexpected( ViewportStatus::kDevice );
		}
		pending.color = color.Value();
		pending.readback = readback.Value();
	}
	auto fail = [&]( ViewportStatus status )
	{
		if ( pending.external >= 0 )
			m_External[std::size_t( pending.external )].busy = false;
		Release( pending );
		return foundation::MakeUnexpected( status );
	};

	graph::GraphBuilder builder;
	const graph::ResourceRef colorRef = builder.ImportTexture( "hammer-viewport", pending.color,
	    colorDesc, device::ResourceUsage::kUndefined,
	    request.external ? device::ResourceUsage::kExternal : device::ResourceUsage::kCopySource );
	const graph::ResourceRef depthRef = builder.CreateTexture( "hammer-viewport-depth", depthDesc );

	LinesTargets targets;
	targets.color = colorRef;
	targets.depth = depthRef;
	targets.width = request.pixelWidth;
	targets.height = request.pixelHeight;
	targets.clear = threeD ? Clear( 0.13f, 0.14f, 0.17f ) : Clear( 0.03f, 0.03f, 0.04f );

	// First pass (clearing): the solids through the families in the camera
	// view, the grid in 2D views.
	m_LastView = ViewStats();
	if ( threeD )
	{
		// The opaque and alpha-tested batches write depth; the blended ones
		// follow in a pass that loads the targets, back to front (ties keep
		// the list's material and mesh order), so they cover what is behind
		// them and never hide it.
		::render::scene::ViewDesc viewDesc;
		viewDesc.projection = view.worldToClip; // the view matrix stays identity
		viewDesc.viewBit = kOpaqueBit;
		const ::render::scene::SceneView sceneView = ::render::scene::MakeView( viewDesc );
		viewDesc.viewBit = kBlendedBit;
		const ::render::scene::SceneView blendedView = ::render::scene::MakeView( viewDesc );
		const std::shared_ptr<const ::render::scene::SceneSnapshot> snapshot = m_Scene->Snapshot();
		const ::render::scene::DrawList list =
		    ::render::scene::BuildDrawList( *snapshot, sceneView );
		::render::scene::DrawList blended =
		    ::render::scene::BuildDrawList( *snapshot, blendedView );
		std::stable_sort( blended.items.begin(), blended.items.end(),
		    []( const ::render::scene::DrawItem &a, const ::render::scene::DrawItem &b )
		    {
			    return a.depth > b.depth;
		    } );
		pass::opaque::OpaqueTargets opaque{
		    colorRef, depthRef, request.pixelWidth, request.pixelHeight, targets.clear };
		auto drawn = pass::opaque::AddOpaquePasses(
		    builder, *snapshot, list, sceneView, { m_FaceMeshes, m_Programs }, opaque );
		if ( !drawn )
		{
			return fail( ViewportStatus::kPass );
		}
		m_LastView = { drawn.Value().drawn, drawn.Value().unresolved, 0 };
		if ( !blended.items.empty() )
		{
			opaque.clearColor = false;
			opaque.clearDepth = false;
			auto over = pass::opaque::AddOpaquePasses(
			    builder, *snapshot, blended, blendedView, { m_FaceMeshes, m_Programs }, opaque );
			if ( !over )
			{
				return fail( ViewportStatus::kPass );
			}
			m_LastView.drawn += over.Value().drawn;
			m_LastView.unresolved += over.Value().unresolved;
			m_LastView.blended = over.Value().drawn;
		}
	}
	else
	{
		LineList grid;
		AppendGrid( request.grid, view.width, view.height, grid );
		if ( !m_Lines->AddPasses( builder, grid, {}, view, targets ) )
		{
			return fail( ViewportStatus::kPass );
		}
	}

	// Then the edges (depth-tested and biased in the camera view) and the
	// tool overlay.
	LineList overlay;
	AppendOverlay( request.overlay, overlay );
	std::vector<MeshBatch> batches;
	for ( const auto &[id, chunk] : m_Chunks )
	{
		if ( chunk.edges )
		{
			batches.push_back( { *chunk.edges, Topology::kLines, { Space::kWorld, threeD } } );
		}
		if ( chunk.edges2D && !threeD )
		{
			batches.push_back( { *chunk.edges2D, Topology::kLines, { Space::kWorld, false } } );
		}
	}
	LinesTargets scene = targets;
	scene.clearColor = false;
	scene.clearDepth = false;
	if ( !m_Lines->AddPasses( builder, overlay, batches, view, scene ) )
	{
		return fail( ViewportStatus::kPass );
	}
	if ( !request.external )
	{
		const std::uint32_t width = request.pixelWidth;
		const std::uint32_t height = request.pixelHeight;
		const graph::ResourceRef readbackRef =
		    builder.ImportBuffer( "hammer-viewport-readback", pending.readback, readbackDesc,
		        device::ResourceUsage::kUndefined, device::ResourceUsage::kCopyDestination );
		builder.AddPass( "hammer-viewport-readback", graph::PassKind::kCopy )
		    .Read( colorRef, device::ResourceUsage::kCopySource )
		    .Write( readbackRef, device::ResourceUsage::kCopyDestination )
		    .SideEffect()
		    .Execute(
		        [colorRef, readbackRef, width, height]( graph::RecordContext &context )
		        {
			        context.Encoder().CopyTextureToBuffer( context.Texture( colorRef ),
			            context.Buffer( readbackRef ), { 0, 0, 0, width, height } );
		        } );
	}
	auto compiled = graph::CompileGraph( std::move( builder ) );
	if ( !compiled )
	{
		return fail( ViewportStatus::kGraph );
	}
	graph::SerialGraphExecutor executor;
	auto executed = executor.Execute( compiled.Value(), m_Device );
	if ( !executed )
	{
		return fail( ViewportStatus::kGraph );
	}
	pending.token = executed.Value().token;
	m_Lines->Collect( pending.token );
	m_LastToken = pending.token;
	const Ticket ticket = m_NextTicket++;
	m_Pending.emplace( ticket, pending );
	return ticket;
}

foundation::Expected<std::optional<ViewPixels>, ViewportStatus> ViewportRenderer::Take(
    Ticket ticket )
{
	auto found = m_Pending.find( ticket );
	if ( found == m_Pending.end() )
	{
		return foundation::MakeUnexpected( ViewportStatus::kUnknownTicket );
	}
	(void)m_Device.Poll();
	if ( !m_Device.IsComplete( found->second.token ) )
	{
		return std::optional<ViewPixels>();
	}
	const Pending pending = found->second;
	m_Pending.erase( found );
	ViewPixels pixels;
	pixels.width = pending.width;
	pixels.height = pending.height;
	if ( pending.external >= 0 )
	{
		ExternalSlot &slot = m_External[std::size_t( pending.external )];
		slot.lease = m_NextLease++;
		pixels.external = ExternalFrame{ slot.image.handle, slot.image.fourcc, slot.image.modifier,
		    slot.image.offset, slot.image.stride, slot.lease };
		return std::optional<ViewPixels>( std::move( pixels ) );
	}
	pixels.rgba.resize( std::size_t( pending.width ) * pending.height * 4 );
	const bool read =
	    m_Device
	        .ReadBuffer( pending.readback, 0,
	            std::as_writable_bytes( std::span<std::uint8_t>( pixels.rgba ) ) )
	        .HasValue();
	Release( pending );
	if ( !read )
	{
		return foundation::MakeUnexpected( ViewportStatus::kDevice );
	}
	return std::optional<ViewPixels>( std::move( pixels ) );
}

foundation::Expected<ViewPixels, ViewportStatus> ViewportRenderer::RenderAndWait(
    const ViewRequest &request )
{
	auto ticket = Render( request );
	if ( !ticket )
	{
		return foundation::MakeUnexpected( ticket.Error() );
	}
	const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds( 20 );
	for ( ;; )
	{
		auto taken = Take( ticket.Value() );
		if ( !taken )
		{
			return foundation::MakeUnexpected( taken.Error() );
		}
		if ( taken.Value() )
		{
			return std::move( *taken.Value() );
		}
		if ( std::chrono::steady_clock::now() > deadline )
		{
			return foundation::MakeUnexpected( ViewportStatus::kDevice );
		}
		std::this_thread::yield();
	}
}

} // namespace hammer::render_adapter
