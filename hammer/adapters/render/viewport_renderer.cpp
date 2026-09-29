//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of hammer/adapters/render/viewport_renderer.h.
//
//=============================================================================//

#include "viewport_renderer.h"

#include "render/graph/executor.h"
#include "render/graph/graph_builder.h"
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
// The neutral material's base texture (a 1x1 white, by the importer's name),
// also every program's neutral lightmap page.
constexpr const char *kWhiteTexture = "hammer/white";
constexpr const char *kWhite = "materials/hammer/white";
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

// The preview program decodes vertex colors as gamma 2.2 (lightmapped.vert,
// ResolvePreview sets the gamma term, Source's convention); the editor's
// shading colors are display (sRGB) values. This table re-encodes a display
// byte so the decode gives its sRGB linear value, and the sRGB target shows
// the byte again (within a level of quantization).
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

::render::math::Aabb BoundsOf( const std::vector<FaceVertex> &vertices )
{
	::render::math::Aabb bounds;
	for ( const FaceVertex &v : vertices )
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
      m_Textures( device ), m_Programs( device, m_Textures ), m_Groups( device, m_Textures ),
      m_Scene( ::render::scene::CreateRenderScene() )
{
}

foundation::Expected<std::unique_ptr<ViewportRenderer>, ViewportStatus> ViewportRenderer::Create(
    device::IRenderDevice2 &device, IMaterialTextures *textures, IModelSource *models )
{
	std::unique_ptr<ViewportRenderer> renderer( new ViewportRenderer( device, textures, models ) );
	auto lines = pass::lines::LinesRenderer::Create( device, kColorFormat, kDepthFormat );
	auto resolver = material::ProgramResolver::Create( device, kColorFormat, kDepthFormat );
	if ( !lines || !resolver )
	{
		return foundation::MakeUnexpected( ViewportStatus::kPass );
	}
	renderer->m_Lines = std::move( lines ).Value();
	renderer->m_Resolver = std::move( resolver ).Value();
	// The neutral material: its white base texture is staged by AddPreview.
	MaterialImage white;
	white.width = white.height = 1;
	white.rgba = { 255, 255, 255, 255 };
	auto neutral = SourceMaterialFromVariables(
	    "UnlitGeneric", { { "$basetexture", kWhiteTexture } }, { { kWhite, white } } );
	if ( !neutral )
	{
		return foundation::MakeUnexpected( ViewportStatus::kPass );
	}
	auto added = renderer->AddPreview( kUntextured, neutral.Value() );
	if ( !added )
	{
		return foundation::MakeUnexpected( added.Error() );
	}
	if ( !added.Value().material || !added.Value().ignored.empty() ||
	     added.Value().material->blended )
	{
		return foundation::MakeUnexpected( ViewportStatus::kPass );
	}
	renderer->m_Untextured = *added.Value().material;
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

std::vector<PreviewVertex> ViewportRenderer::ToPreview( const std::vector<FaceVertex> &vertices )
{
	const std::array<std::uint8_t, 256> &encode = Gamma22FromDisplay();
	std::vector<PreviewVertex> out( vertices.size() );
	for ( std::size_t i = 0; i < vertices.size(); ++i )
	{
		const FaceVertex &in = vertices[i];
		PreviewVertex &v = out[i];
		std::copy( in.position, in.position + 3, v.position );
		std::copy( in.uv, in.uv + 2, v.uv );
		for ( int c = 0; c < 3; ++c )
			v.color[c] = encode[in.color[c]];
		v.color[3] = in.color[3];
	}
	return out;
}

foundation::Expected<std::uint64_t, ViewportStatus> ViewportRenderer::GroupsFor(
    const material::ResolvedProgram &program, std::string &failure )
{
	std::uint64_t drawGroup = 0;
	const material::ProgramRequest &request = program.request;
	if ( request.drawLayout.IsValid() )
	{
		auto found = m_DrawPages.find( request.drawLayout.value );
		if ( found == m_DrawPages.end() )
		{
			// Lighting is one: the lightmap is bound, not read, so a white page
			// serves every input.
			std::vector<std::string> inputs;
			for ( const std::string &input : program.drawInputs )
			{
				if ( input != "lightmap" )
				{
					failure = "its program reads draw input " + input + ", which the editor lacks";
					return std::uint64_t( 0 );
				}
				inputs.push_back( kWhite );
			}
			const std::optional<material::GroupRequest> group =
			    m_Resolver->DrawGroup( program, inputs );
			if ( !group )
			{
				failure = "its draw group did not resolve";
				return std::uint64_t( 0 );
			}
			const std::uint64_t id = m_NextGroup++;
			if ( !m_Groups.Set( id, *group ) )
			{
				return foundation::MakeUnexpected( ViewportStatus::kDevice );
			}
			found = m_DrawPages.emplace( request.drawLayout.value, id ).first;
		}
		drawGroup = found->second;
	}
	if ( request.frameLayout.IsValid() && m_Frames.count( request.frameLayout.value ) == 0 )
	{
		// The LDR terms (lighting one reads no lightmap scale); the target has
		// an sRGB view, so the shader does not encode.
		const std::optional<material::GroupRequest> group =
		    m_Resolver->FrameGroup( program, material::FrameTerms{} );
		if ( !group )
		{
			failure = "its frame group did not resolve";
			return std::uint64_t( 0 );
		}
		const std::uint64_t id = m_NextGroup++;
		if ( !m_Groups.Set( id, *group ) )
		{
			return foundation::MakeUnexpected( ViewportStatus::kDevice );
		}
		m_Frames.emplace( request.frameLayout.value, id );
	}
	if ( request.viewLayout.IsValid() )
	{
		failure = "its program reads a view group, which the editor lacks";
	}
	return drawGroup;
}

foundation::Expected<ViewportRenderer::Preview, ViewportStatus> ViewportRenderer::AddPreview(
    std::uint64_t id, const SourceMaterial &source )
{
	Preview out;
	auto resolved = m_Resolver->ResolvePreview( source.desc );
	if ( !resolved )
	{
		out.failure = resolved.Error();
		return out;
	}
	material::ResolvedProgram &program = resolved.Value().program;
	if ( program.request.vertexStride != sizeof( PreviewVertex ) )
	{
		out.failure = "its program reads another vertex than the preview's";
		return out;
	}
	// The textures the program samples, by its names; the first is the base
	// texture, whose size maps the faces' texture axes.
	std::optional<TextureSize> size;
	for ( const material::ProgramTexture &texture : program.request.material.textures )
	{
		// An input named empty is a term that is off: the programs bind their
		// neutral texture there themselves.
		if ( texture.name.empty() )
		{
			continue;
		}
		if ( texture.dimension != device::TextureDimension::k2D )
		{
			out.failure = "its program samples a cube map (" + texture.name +
			              "), which the editor does not stage";
			return out;
		}
		auto image = source.textures.find( texture.name );
		if ( m_StagedTextures.count( texture.name ) == 0 )
		{
			const std::vector<MipLevel> chain = image == source.textures.end()
			                                        ? std::vector<MipLevel>()
			                                        : BuildMipChain( image->second );
			if ( chain.empty() )
			{
				out.missingTexture = true;
				return out;
			}
			device::TextureDesc desc;
			desc.format = kColorFormat;
			desc.width = chain.front().width;
			desc.height = chain.front().height;
			desc.mipLevels = static_cast<std::uint32_t>( chain.size() );
			desc.usages = {
			    device::ResourceUsage::kCopyDestination, device::ResourceUsage::kSampled };
			std::vector<std::span<const std::byte>> levels;
			levels.reserve( chain.size() );
			for ( const MipLevel &level : chain )
			{
				levels.push_back( std::as_bytes( std::span( level.rgba ) ) );
			}
			if ( !m_Textures.StageMips( texture.name, desc, levels ) )
			{
				return foundation::MakeUnexpected( ViewportStatus::kDevice );
			}
			m_StagedTextures.insert( texture.name );
		}
		const resources::TextureEntry *staged = m_Textures.Find( texture.name );
		if ( !size && staged )
		{
			size = TextureSize{ staged->desc.width, staged->desc.height };
		}
	}
	auto drawGroup = GroupsFor( program, out.failure );
	if ( !drawGroup )
	{
		return foundation::MakeUnexpected( drawGroup.Error() );
	}
	if ( !out.failure.empty() )
	{
		return out;
	}
	if ( !m_Programs.Set( id, program.request ) )
	{
		return foundation::MakeUnexpected( ViewportStatus::kDevice );
	}
	Material material;
	material.id = id;
	material.size = size;
	material.blended = program.blend != device::BlendMode::kOpaque;
	material.drawGroup = drawGroup.Value();
	out.material = material;
	out.ignored = std::move( resolved.Value().ignored );
	return out;
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
		Material material; // no size: drawn untextured
		auto source = m_Source->Material( name );
		if ( !source )
		{
			m_Stats.failures[name] = source.Error();
		}
		else
		{
			auto preview = AddPreview( m_NextMaterial, source.Value() );
			if ( !preview )
			{
				return foundation::MakeUnexpected( preview.Error() );
			}
			if ( preview.Value().material )
			{
				material = *preview.Value().material;
				++m_NextMaterial;
				++m_Stats.textures;
				for ( const std::string &variable : preview.Value().ignored )
					++m_Stats.ignored[variable];
				m_Stats.approximatedMaterials += preview.Value().ignored.empty() ? 0 : 1;
			}
			else if ( !preview.Value().missingTexture )
			{
				m_Stats.failures[name] = preview.Value().failure;
			}
		}
		if ( !material.size )
		{
			++m_Stats.missingTextures;
		}
		m_Stats.failedMaterials = static_cast<std::uint32_t>( m_Stats.failures.size() );
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

const ModelAsset &ViewportRenderer::ModelEntry::Posed( std::int32_t sequence )
{
	auto found = posed.find( sequence );
	if ( found == posed.end() )
	{
		found = posed.emplace( sequence, PosedModel( *asset, sequence ) ).first;
	}
	return found->second;
}

foundation::Expected<const std::vector<ViewportRenderer::ModelMesh> *, ViewportStatus>
ViewportRenderer::VariantFor(
    ModelEntry &entry, std::int32_t sequence, std::int32_t skin, const scene::Rgb &tint )
{
	const std::array<int, 5> key = { sequence, skin, tint.r, tint.g, tint.b };
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
	    entry.Posed( sequence ), skin,
	    [this]( const std::string &material ) -> std::optional<TextureSize>
	    {
		    auto found = m_Materials.find( material );
		    return found == m_Materials.end() ? std::nullopt : found->second.size;
	    },
	    tint, kModelFill );
	std::vector<ModelMesh> meshes;
	for ( const ModelBatch &batch : batches )
	{
		const Material &material =
		    batch.material.empty() ? m_Untextured : m_Materials.at( batch.material );
		const std::vector<PreviewVertex> vertices = ToPreview( batch.vertices );
		ModelMesh mesh;
		mesh.material = material.id;
		mesh.drawGroup = material.drawGroup;
		mesh.blended = material.blended;
		mesh.mesh = m_NextMesh++;
		mesh.bounds = BoundsOf( batch.vertices );
		resources::MeshData data;
		data.vertices = std::as_bytes( std::span<const PreviewVertex>( vertices ) );
		data.vertexStride = sizeof( PreviewVertex );
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
		ModelEntry &entry = *models[at];
		return ModelWorldBox(
		    entry.Posed( ModelSequence( entry.asset->model, entity.modelKeys ) ).model, entity );
	};
	const SceneGeometry geometry = BuildSceneGeometry( objects, options );
	staged.solids = std::move( objects.solids );
	staged.entities = std::move( objects.entities );

	// One mesh and one scene instance per batch; a (chunk, material) keeps
	// its mesh id, so a restage replaces the mesh's buffers under its name.
	const std::map<std::uint64_t, std::uint64_t> previous =
	    resident == m_Chunks.end() ? std::map<std::uint64_t, std::uint64_t>()
	                               : resident->second.meshes;
	for ( const FaceBatch &batch : geometry.faces )
	{
		const bool textured = !batch.material.empty();
		const Material &material = textured ? m_Materials.at( batch.material ) : m_Untextured;
		const std::uint64_t materialId = material.id;
		auto kept = previous.find( materialId );
		const std::uint64_t mesh = kept != previous.end() ? kept->second : m_NextMesh++;
		const std::vector<PreviewVertex> vertices = ToPreview( batch.vertices );
		resources::MeshData data;
		data.vertices = std::as_bytes( std::span<const PreviewVertex>( vertices ) );
		data.vertexStride = sizeof( PreviewVertex );
		auto entry = m_MeshCache.Stage( FaceMeshName( mesh ), data );
		if ( !entry )
		{
			return foundation::MakeUnexpected( ViewportStatus::kDevice );
		}
		m_FaceMeshes.entries[mesh] = entry.Value();
		staged.meshes[materialId] = mesh;
		++m_Stats.stagedMeshes;
		const bool blended = material.blended;
		::render::scene::MeshInstanceDesc desc;
		desc.mesh = mesh;
		desc.material = materialId;
		desc.drawGroup = material.drawGroup;
		desc.localBounds = BoundsOf( batch.vertices );
		desc.viewMask = 1u << ( blended ? kBlendedBit : kOpaqueBit );
		const ::render::scene::InstanceId instance = m_Scene->Reserve();
		changes.Add( instance, desc );
		staged.instances.push_back( instance );
		staged.faceVertices += static_cast<std::uint32_t>( batch.vertices.size() );
		staged.texturedBatches += textured ? 1 : 0;
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
		auto variant = VariantFor( *entry, ModelSequence( entry->asset->model, entity.modelKeys ),
		    entity.modelKeys.skin, ModelTint( entity, staged.instanceContent ) );
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
			desc.drawGroup = mesh.drawGroup;
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
	m_Groups.RecordUploads( encoder.Value() );
	device::CommandEncoder encoders[] = { std::move( encoder ).Value() };
	auto token = m_Device.Submit( device::QueueKind::kGraphics, encoders, {} );
	if ( !token )
	{
		return foundation::MakeUnexpected( ViewportStatus::kDevice );
	}
	m_MeshCache.Retire( token.Value() );
	m_Textures.Retire( token.Value() );
	m_Programs.Retire( token.Value() );
	m_Groups.Retire( token.Value() );
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
		// The programs' frame groups (one per frame layout) and neutral pages.
		std::vector<const material::DrawGroup *> frames;
		for ( const auto &[layout, id] : m_Frames )
		{
			if ( const material::DrawGroup *group = m_Groups.Group( id ) )
				frames.push_back( group );
		}
		const pass::opaque::OpaqueSources sources{ .meshes = m_FaceMeshes,
		    .programs = m_Programs,
		    .drawGroups = &m_Groups,
		    .frames = frames };
		auto drawn =
		    pass::opaque::AddOpaquePasses( builder, *snapshot, list, sceneView, sources, opaque );
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
			    builder, *snapshot, blended, blendedView, sources, opaque );
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
