//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render_lab (RFC 0016 K11): the render core composed alone, with no
//			engine, material system, legacy frontend or SDL, rendering scenes
//			from the formats the core serves so each lighting term is proven
//			against Cycles before it reaches the product.
//
//			This slice ("Lab composes the core alone"): a BSP2 map's world
//			mesh (WMSH) and its lightmap page (LMAP, linear RGBA16F: the
//			total layer, or with --core-direct the indirect layer, since the
//			core then draws the direct light; a directional page's halves
//			staged as its flat and gradient pages), and optionally a studio
//			model, drawn through the Vulkan
//			adapter into a linear RGBA16F target and written as a PFM
//			(portable float map, bottom row first). Materials come through
//			render.material's importer and the one program resolver.
//			Physically based world materials draw as their diffuse point: the
//			base color times the lightmap's total light (the full surface is
//			the Model assembly check).
//
//			Participating media (K11 step g): when the map's entity lump holds
//			an env_volumetric_fog_volume or env_volumetric_fog_controller,
//			render.pass.volumetric applies its fog to the opaque frame, on
//			render.pass.lights' ClusterGrid subdivided 8 x 4 (lab_media.h),
//			lit by the lump's lights and projectors (unshadowed in this
//			slice). --fog-scale s multiplies every density (0: the fixture's
//			density-zero state), --no-volumetric leaves the pass out, and
//			--fog-samples xy,depth sets the inject stage's samples per froxel
//			(default 2,4: 2 x 2 across and 4 along), --time n prints the
//			pass's median time over n more submissions,
//			and --inscatter-only clears the frame before the composite (the
//			in-scattered light alone, as Cycles' Volume Direct pass).
//
//			The frame's debug controls (RFC 0014) apply through the --debug-*
//			options, as cl_render_debug_* apply in the product: validated by
//			render.frame, turned into each program's specialization, drawn
//			with the program's debug pipeline.
//
//			Usage: render_lab --game <dir> --map <file.bsp> --eye x,y,z
//			           --forward x,y,z --up x,y,z --hfov degrees --size WxH
//			           --out <file.pfm> [--model <models/x.mdl> --model-origin
//			           x,y,z] [--validate] [--dump-mesh] [--core-direct]
//			           [--debug-view n] [--fog-scale s] [--no-volumetric] [--time n]
//			           [--debug-program name] [--debug-scale s]
//			           [--debug-range r] [--debug-threshold t] [--debug-brdf n]
//			           [--debug-furnace] [--debug-term name[,name...]]
//			           [--debug-force-roughness r] [--debug-force-metalness m]
//			       render_lab suite <name> [--validate] [--seeded <defect>]
//			           (render/lab/suites.h: the lab's suites, checks-v1)
//			Exit status: 0 drawn (and, with --validate, no validation
//			messages); 1 a failure, printed with its reason.
//
//=============================================================================//

#include "lab_media.h"
#include "lab_support.h"
#include "suites.h"

#include "foundation/expected.h"
#include "mapcontainer/map_container.h"
#include "mapcontainer/map_container_builder.h"
#include "mapcontainer/world_lightmap.h"
#include "mapcontainer/world_mesh_decode.h"
#include "mapcontainer/world_mesh_format.h"
#include "mdl/studio_model.h"
#include "render/device/device.h"
#include "render/device/vulkan/provider.h"
#include "render/frame/debug_specialization.h"
#include "render/material/lightmapped_family.h"
#include "render/material/material_programs.h"
#include "render/material/program_resolver.h"
#include "render/material/vmt_import.h"
#include "render/math/matrix.h"
#include "render/pass/lights/clusters.h"
#include "render/pass/volumetric/volumetric.h"
#include "render/resources/texture_cache.h"
#include "texturecontainer/texture_image.h"
#include "texturecontainer/vtf_image_reader.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <map>
#include <memory>
#include <optional>
#include <span>
#include <sstream>
#include <string>
#include <vector>

namespace
{

using namespace render;
using namespace render::device;
using namespace render::lab;
namespace fs = std::filesystem;

struct Options
{
	fs::path game;
	fs::path map;
	fs::path out;
	fs::path model;
	math::float3 eye{ 0, 0, 0 };
	math::float3 forward{ 1, 0, 0 };
	math::float3 up{ 0, 0, 1 };
	math::float3 modelOrigin{ 0, 0, 0 };
	float horizontalFov = 90.0f;
	std::uint32_t width = 256;
	std::uint32_t height = 192;
	bool validate = false;
	bool dumpMesh = false;
	// The core draws the map's direct light, so world surfaces read the
	// bake's indirect layer (BakedLightmapLayer).
	bool coreDirect = false;
	// Participating media: every density times this; the pass left out.
	float fogScale = 1.0f;
	bool noVolumetric = false;
	// A diagnostic: the frame cleared to black before the composite, so the
	// image is the medium's in-scattered light alone (Cycles' Volume Direct).
	bool inscatterOnly = false;
	std::uint32_t timeRepeats = 0;
	// The inject stage's stratified samples per froxel (across, along).
	pass::volumetric::VolumetricSampling fogSampling;
	frame::DebugControls debug;
};

// The texture cache's names of the map's lightmap pages.
constexpr const char *kLightmapPage = "lab:lightmap";
constexpr const char *kLightmapGradient = "lab:lightmap-gradient";

int Fail( const std::string &why )
{
	std::fprintf( stderr, "render_lab: %s\n", why.c_str() );
	return 1;
}

bool ParseVector( const char *text, math::float3 &out )
{
	return std::sscanf( text, "%f,%f,%f", &out.x, &out.y, &out.z ) == 3;
}

std::optional<Options> ParseOptions( int argc, char **argv )
{
	Options options;
	for ( int i = 1; i < argc; ++i )
	{
		const std::string arg = argv[i];
		const char *value = i + 1 < argc ? argv[i + 1] : nullptr;
		auto take = [&]() -> const char *
		{
			++i;
			return value;
		};
		if ( arg == "--validate" )
			options.validate = true;
		else if ( arg == "--debug-furnace" )
			options.debug.furnace = true;
		else if ( arg == "--dump-mesh" )
			options.dumpMesh = true;
		else if ( arg == "--core-direct" )
			options.coreDirect = true;
		else if ( arg == "--no-volumetric" )
			options.noVolumetric = true;
		else if ( arg == "--inscatter-only" )
			options.inscatterOnly = true;
		else if ( !value )
			return std::nullopt;
		else if ( arg == "--game" )
			options.game = take();
		else if ( arg == "--map" )
			options.map = take();
		else if ( arg == "--out" )
			options.out = take();
		else if ( arg == "--model" )
			options.model = take();
		else if ( arg == "--eye" && ParseVector( value, options.eye ) )
			take();
		else if ( arg == "--forward" && ParseVector( value, options.forward ) )
			take();
		else if ( arg == "--up" && ParseVector( value, options.up ) )
			take();
		else if ( arg == "--model-origin" && ParseVector( value, options.modelOrigin ) )
			take();
		else if ( arg == "--hfov" )
			options.horizontalFov = float( std::atof( take() ) );
		else if ( arg == "--fog-scale" )
			options.fogScale = float( std::atof( take() ) );
		else if ( arg == "--time" )
			options.timeRepeats = std::uint32_t( std::atoi( take() ) );
		else if ( arg == "--fog-samples" &&
		          std::sscanf( value, "%u,%u", &options.fogSampling.samplesXY,
		              &options.fogSampling.samplesDepth ) == 2 )
			take();
		else if ( arg == "--size" &&
		          std::sscanf( value, "%ux%u", &options.width, &options.height ) == 2 )
			take();
		else if ( ParseDebugOption( arg, value, options.debug ) )
			take();
		else
			return std::nullopt;
	}
	if ( options.map.empty() || ( options.out.empty() && !options.dumpMesh ) ||
	     options.width == 0 || options.height == 0 )
		return std::nullopt;
	return options;
}

// A material of the scene, resolved to a program.
struct SceneMaterial
{
	material::ResolvedProgram program;
	std::uint64_t groupId = 0;
};

// Loads a VMT and resolves its program. A PBRMetalRough world material draws
// as its diffuse point: LightmappedGeneric with its base color.
std::optional<std::string> ResolveMaterial( const GameFiles &files,
    material::ProgramResolver &resolver, resources::TextureCache &cache, const std::string &path,
    SceneMaterial &out )
{
	std::string text;
	if ( !files.Read( "materials/" + path + ".vmt", text ) )
		return "material " + path + " has no VMT";
	const std::string ownPath = "materials/" + path + ".vmt";
	material::VmtImportContext context;
	context.path = ownPath;
	context.resolve = [&files]( std::string_view file ) -> std::optional<std::string>
	{
		std::string bytes;
		if ( !files.Read( std::string( file ), bytes ) )
			return std::nullopt;
		return bytes;
	};
	auto imported = material::ImportVmt( text, context );
	if ( !imported )
		return "material " + path + " does not import: " +
		       std::string( material::ImportStatusName( imported.Error().status ) ) + " " +
		       imported.Error().detail;
	material::MaterialDesc desc = std::move( imported ).Value();
	if ( desc.legacyShader == "pbrmetalrough" || desc.shader == "PBRMetalRough" )
	{
		std::vector<material::VmtPair> variables;
		for ( const material::VmtPair &variable : desc.variables )
		{
			if ( variable.key == "$basetexture" || variable.key == "$basecolortexture" )
				variables.push_back( { "$basetexture", variable.value } );
		}
		auto mapped = material::MapVariables( "LightmappedGeneric", std::move( variables ), {} );
		if ( !mapped )
			return "material " + path + ": its diffuse point does not map";
		desc = std::move( mapped ).Value();
	}
	auto program = resolver.Resolve( desc );
	if ( !program )
		return "material " + path + ": " + program.Error();
	out.program = std::move( program ).Value();
	// Stage the textures the program's material group names.
	for ( const material::ProgramTexture &texture : out.program.request.material.textures )
	{
		if ( texture.name.empty() || cache.Find( texture.name ) )
			continue;
		std::string bytes;
		if ( !files.Read( texture.name + ".vtf", bytes ) )
			return "texture " + texture.name + " is missing";
		auto image = texturecontainer::ReadVtfImage(
		    std::as_bytes( std::span( bytes.data(), bytes.size() ) ) );
		if ( !image )
			return "texture " + texture.name + " does not decode";
		if ( std::optional<std::string> why =
		         StageImage( cache, texture.name, image.Value(), texture.srgb ) )
			return why;
	}
	return std::nullopt;
}

int Run( const Options &options )
{
	const GameFiles files( options.game );

	// The map: its world mesh and lightmap page.
	std::optional<std::string> mapBytes = ReadFile( options.map );
	if ( !mapBytes )
		return Fail( "cannot read " + options.map.string() );
	mapcontainer::MemoryByteSource bytes(
	    std::as_bytes( std::span( mapBytes->data(), mapBytes->size() ) ) );
	auto opened = mapcontainer::OpenMemoryContainer( bytes, true );
	if ( !opened )
		return Fail( options.map.string() + " is not a map container" );
	mapcontainer::ContainerPtr container = std::move( opened ).Value();
	auto lump = [&]( std::uint32_t fourcc ) -> std::optional<std::string>
	{
		mapcontainer::MapLumpInfo info{};
		if ( !container->FindLump( fourcc, &info ) || info.storedSize != info.uncompressedSize )
			return std::nullopt;
		return mapBytes->substr( std::size_t( info.offset ), std::size_t( info.storedSize ) );
	};
	const std::optional<std::string> wmsh = lump( mapcontainer::kLumpWorldMesh );
	if ( !wmsh )
		return Fail( options.map.string() + " has no world mesh (WMSH)" );
	mapcontainer::WorldMeshData mesh;
	if ( const mapcontainer::WorldMeshError error =
	         mapcontainer::DecodeWorldMesh( wmsh->data(), wmsh->size(), mesh );
	    error != mapcontainer::WorldMeshError::Ok )
		return Fail( std::string( "WMSH: " ) + mapcontainer::WorldMeshErrorName( error ) );
	if ( options.dumpMesh )
	{
		double sums[8] = {};
		for ( const mapcontainer::WorldMeshVertex &v : mesh.vertices )
		{
			for ( int c = 0; c < 3; ++c )
			{
				sums[c] += v.position[c];
				sums[3 + c] += v.normal[c];
			}
			sums[6] += v.lightmapUv[0] + v.lightmapUv[1];
			sums[7] += v.uv[0] + v.uv[1];
		}
		std::printf( "wmsh version %u vertices %zu indices %zu batches %zu materials %zu\n",
		    mesh.version, mesh.vertices.size(), mesh.indices.size(), mesh.batches.size(),
		    mesh.materials.size() );
		std::printf( "sums position %.6g %.6g %.6g normal %.6g %.6g %.6g lightmap %.6g uv %.6g\n",
		    sums[0], sums[1], sums[2], sums[3], sums[4], sums[5], sums[6], sums[7] );
		for ( const std::string &name : mesh.materials )
			std::printf( "material %s\n", name.c_str() );
		if ( options.out.empty() )
			return 0;
	}
	const std::optional<std::string> lmap = lump( mapcontainer::kLumpWorldLightmap );
	if ( !lmap )
		return Fail( options.map.string() + " has no lightmap page (LMAP)" );
	mapcontainer::MapLumpInfo lmapInfo{};
	(void)container->FindLump( mapcontainer::kLumpWorldLightmap, &lmapInfo );
	mapcontainer::WorldLightmapLayout layout{};
	if ( const mapcontainer::WorldLightmapError error = mapcontainer::ValidateWorldLightmap(
	         lmap->data(), lmap->size(), lmapInfo.version, &layout );
	    error != mapcontainer::WorldLightmapError::Ok )
		return Fail( std::string( "LMAP: " ) + mapcontainer::WorldLightmapErrorName( error ) );
	// The layer the world's baked diffuse light comes from; a map without it
	// cannot be drawn so (no silent fallback to the total layer).
	const mapcontainer::WorldLightmapLayer bakedRole = BakedLightmapLayer( options.coreDirect );
	const int bakedLayer = mapcontainer::WorldLightmapLayerIndex( layout, bakedRole );
	if ( bakedLayer < 0 )
		return Fail( std::string( "LMAP carries no " ) +
		             mapcontainer::WorldLightmapLayerName( bakedRole ) + " layer" );

	// The entity lump's participating media and the lights it names.
	LabMedia media;
	{
		mapcontainer::MapLumpInfo info{};
		if ( container->FindLegacyLump( 0, &info ) && info.storedSize == info.uncompressedSize )
		{
			const auto parsed = ParseEntityLump(
			    mapBytes->substr( std::size_t( info.offset ), std::size_t( info.storedSize ) ) );
			if ( !parsed )
				return Fail( options.map.string() + ": the entity lump does not parse" );
			media = MediaFromEntities( *parsed );
		}
		media.medium.densityScale = options.fogScale;
	}
	const bool volumetric = media.present && !options.noVolumetric;

	// The debug controls, as the renderer validates a frame's.
	if ( auto valid = frame::ValidateDebugControls(
	         options.debug, material::ProgramResolver::ProgramNames() );
	    !valid )
		return Fail( valid.Error().message );

	// The device, with synchronization validation when asked.
	std::atomic<std::uint64_t> messages{ 0 };
	std::unique_ptr<IRenderDevice2> device;
	if ( std::optional<std::string> why = CreateLabDevice( options.validate, messages, device ) )
		return Fail( *why );
	int status = 0;
	{
		const Format colorFormat = Format::kRGBA16Float;
		const Format depthFormat = Format::kD32Float;
		auto resolver = material::ProgramResolver::Create( *device, colorFormat, depthFormat, 1 );
		if ( !resolver )
			return Fail( "no programs: " + resolver.Error() );
		resources::TextureCache cache( *device );
		material::GroupResidency groups( *device, cache );

		// The lightmap pages: linear light, sampled as it is (scale 1). The
		// flat page, and the gradient page the lightmap basis reads (a
		// directional page's right half; zero for a flat page, which is the
		// flat light bitwise).
		{
			const LightmapLayerPages pages = SplitLightmapLayer(
			    std::as_bytes( std::span( lmap->data() + layout.layerOffset[bakedLayer],
			        std::size_t( layout.layerBytes ) ) ),
			    layout.width, layout.height );
			if ( pages.flat.empty() )
				return Fail( std::string( "LMAP's " ) +
				             mapcontainer::WorldLightmapLayerName( bakedRole ) +
				             " layer does not split into pages" );
			TextureDesc desc;
			desc.format = Format::kRGBA16Float;
			desc.width = pages.width;
			desc.height = pages.height;
			desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kSampled };
			if ( !cache.Stage( kLightmapPage, desc, pages.flat ) )
				return Fail( "the lightmap page was refused" );
			const std::vector<std::byte> zero( pages.flat.size(), std::byte( 0 ) );
			if ( !cache.Stage( kLightmapGradient, desc,
			         pages.Directional() ? std::span<const std::byte>( pages.gradient )
			                             : std::span<const std::byte>( zero ) ) )
				return Fail( "the lightmap gradient page was refused" );
			std::printf( "render_lab: LMAP v%u %ux%u, the %s layer%s\n", layout.version,
			    layout.width, layout.height, mapcontainer::WorldLightmapLayerName( bakedRole ),
			    pages.Directional() ? ", directional" : "" );
		}

		// The map's probe volume (PRBV) for probe_volume.glsl, when it has
		// one: lab:probe-volume-atlas and lab:probe-volume-grids.
		if ( const std::optional<std::string> prbv = lump( mapcontainer::kLumpProbeVolume ) )
		{
			mapcontainer::ProbeVolumeLayout probeLayout{};
			if ( std::optional<std::string> why = StageProbeVolume( cache, "lab:probe-volume",
			         std::as_bytes( std::span( prbv->data(), prbv->size() ) ), probeLayout ) )
				return Fail( *why );
			std::printf( "render_lab: PRBV %u grid%s, %u layer%s, %u active probes\n",
			    probeLayout.gridCount, probeLayout.gridCount == 1 ? "" : "s",
			    probeLayout.layerCount, probeLayout.layerCount == 1 ? "" : "s",
			    probeLayout.activeProbes );
		}

		// The map's reflection probes (RPRB) for reflection_probes.glsl, when
		// it has them: lab:reflection-probes, blended, relit when they carry
		// relight bands.
		if ( const std::optional<std::string> rprb = lump( mapcontainer::kLumpReflectionProbes ) )
		{
			mapcontainer::ReflectionProbesLayout probesLayout{};
			if ( std::optional<std::string> why =
			         StageReflectionProbes( cache, "lab:reflection-probes",
			             std::as_bytes( std::span( rprb->data(), rprb->size() ) ),
			             mapcontainer::ReflectionProbeMode::Blend, true, probesLayout ) )
				return Fail( *why );
			std::printf( "render_lab: RPRB %u probe%s, %u mips from %u wide%s\n",
			    probesLayout.count, probesLayout.count == 1 ? "" : "s", probesLayout.mipCount,
			    probesLayout.width, probesLayout.relight ? ", relightable" : "" );
		}

		// The world's materials and their groups.
		std::vector<SceneMaterial> materials( mesh.materials.size() );
		std::uint64_t nextGroup = 1;
		std::map<std::uint64_t, std::uint64_t> drawGroupOf; // draw layout -> group id
		std::map<std::uint64_t, std::uint64_t> frameGroupOf;
		std::map<std::uint64_t, std::uint64_t> viewGroupOf; // view layout -> neutral view group
		material::FrameTerms terms;
		terms.lightmapScale = 1.0f; // LMAP holds linear light
		auto addGroups = [&]( SceneMaterial &m ) -> std::optional<std::string>
		{
			m.groupId = nextGroup++;
			if ( !groups.Set( m.groupId, m.program.request.material ) )
				return std::string( "a material group was refused" );
			const std::uint64_t drawLayout = m.program.request.drawLayout.value;
			if ( m.program.request.drawLayout.IsValid() && !drawGroupOf.count( drawLayout ) )
			{
				auto request = resolver.Value()->DrawGroup( m.program, { kLightmapPage } );
				if ( !request )
					return std::string( "no draw group" );
				drawGroupOf[drawLayout] = nextGroup++;
				if ( !groups.Set( drawGroupOf[drawLayout], *request ) )
					return std::string( "the draw group was refused" );
			}
			const std::uint64_t viewLayout = m.program.request.viewLayout.value;
			if ( m.program.request.viewLayout.IsValid() && !viewGroupOf.count( viewLayout ) )
			{
				if ( !m.program.request.neutralView )
					return std::string( "no view group" );
				viewGroupOf[viewLayout] = nextGroup++;
				if ( !groups.Set( viewGroupOf[viewLayout], *m.program.request.neutralView ) )
					return std::string( "the view group was refused" );
			}
			const std::uint64_t frameLayout = m.program.request.frameLayout.value;
			if ( m.program.request.frameLayout.IsValid() && !frameGroupOf.count( frameLayout ) )
			{
				auto request = resolver.Value()->FrameGroup( m.program, terms );
				if ( !request )
					return std::string( "no frame group" );
				frameGroupOf[frameLayout] = nextGroup++;
				if ( !groups.Set( frameGroupOf[frameLayout], *request ) )
					return std::string( "the frame group was refused" );
			}
			return std::nullopt;
		};
		for ( std::size_t i = 0; i < mesh.materials.size(); ++i )
		{
			if ( std::optional<std::string> why = ResolveMaterial(
			         files, *resolver.Value(), cache, mesh.materials[i], materials[i] ) )
				return Fail( *why );
			if ( std::optional<std::string> why = addGroups( materials[i] ) )
				return Fail( mesh.materials[i] + ": " + *why );
		}

		// The world's vertices as the program reads them.
		std::vector<material::SurfaceFlatVertex> vertices( mesh.vertices.size() );
		for ( std::size_t i = 0; i < mesh.vertices.size(); ++i )
		{
			const mapcontainer::WorldMeshVertex &from = mesh.vertices[i];
			material::SurfaceFlatVertex &to = vertices[i];
			std::copy( from.position, from.position + 3, to.position );
			std::copy( from.uv, from.uv + 2, to.uv );
			std::copy( from.lightmapUv, from.lightmapUv + 2, to.lightmapUv );
		}
		std::vector<std::uint32_t> indices = mesh.indices;
		struct Draw
		{
			std::size_t material;
			std::uint32_t firstIndex;
			std::uint32_t indexCount;
		};
		std::vector<Draw> draws;
		for ( const mapcontainer::WorldMeshBatch &batch : mesh.batches )
			draws.push_back( { batch.material, batch.firstIndex, batch.indexCount } );

		// The studio model: its first body at the origin, drawn unlit with
		// its skin's base textures (lighting one: the model's lighting is a
		// later lab slice).
		if ( !options.model.empty() )
		{
			auto model = mdl::LoadModel( files, options.model.string(), 0 );
			if ( !model )
				return Fail( "model " + options.model.string() + " does not load" );
			const mdl::Model &m = model.Value();
			for ( const mdl::Mesh &part : m.meshes )
			{
				const std::int16_t texture =
				    m.skinFamilies.empty() ? std::int16_t( part.textureRef )
				                           : m.skinFamilies[0][std::size_t( part.textureRef )];
				std::string path;
				for ( const std::string &directory : m.cdMaterials )
				{
					const std::string candidate = directory + m.textures[std::size_t( texture )];
					if ( files.Exists( "materials/" + candidate + ".vmt" ) )
					{
						path = candidate;
						break;
					}
				}
				if ( path.empty() )
					return Fail(
					    "model material " + m.textures[std::size_t( texture )] + " has no VMT" );
				SceneMaterial resolved;
				std::string text;
				(void)files.Read( "materials/" + path + ".vmt", text );
				const std::string ownPath = "materials/" + path + ".vmt";
				material::VmtImportContext context;
				context.path = ownPath;
				context.resolve = [&files]( std::string_view file ) -> std::optional<std::string>
				{
					std::string bytes;
					if ( !files.Read( std::string( file ), bytes ) )
						return std::nullopt;
					return bytes;
				};
				auto imported = material::ImportVmt( text, context );
				if ( !imported )
					return Fail( "model material " + path + " does not import" );
				std::vector<material::VmtPair> variables;
				for ( const material::VmtPair &variable : imported.Value().variables )
				{
					if ( variable.key == "$basetexture" )
						variables.push_back( variable );
				}
				auto mapped = material::MapVariables( "UnlitGeneric", std::move( variables ), {} );
				if ( !mapped )
					return Fail( "model material " + path + " does not map" );
				auto program = resolver.Value()->Resolve( mapped.Value() );
				if ( !program )
					return Fail( "model material " + path + ": " + program.Error() );
				resolved.program = std::move( program ).Value();
				for ( const material::ProgramTexture &t :
				    resolved.program.request.material.textures )
				{
					if ( t.name.empty() || cache.Find( t.name ) )
						continue;
					std::string bytes;
					if ( !files.Read( t.name + ".vtf", bytes ) )
						return Fail( "texture " + t.name + " is missing" );
					auto image = texturecontainer::ReadVtfImage(
					    std::as_bytes( std::span( bytes.data(), bytes.size() ) ) );
					if ( !image )
						return Fail( "texture " + t.name + " does not decode" );
					if ( std::optional<std::string> why =
					         StageImage( cache, t.name, image.Value(), t.srgb ) )
						return Fail( *why );
				}
				if ( std::optional<std::string> why = addGroups( resolved ) )
					return Fail( path + ": " + *why );
				materials.push_back( std::move( resolved ) );
				const std::uint32_t base = std::uint32_t( vertices.size() );
				for ( const mdl::Vertex &v : part.vertices )
				{
					material::SurfaceFlatVertex to;
					to.position[0] = v.position.x + options.modelOrigin.x;
					to.position[1] = v.position.y + options.modelOrigin.y;
					to.position[2] = v.position.z + options.modelOrigin.z;
					to.uv[0] = v.u;
					to.uv[1] = v.v;
					vertices.push_back( to );
				}
				const std::uint32_t first = std::uint32_t( indices.size() );
				for ( std::uint32_t index : part.indices )
					indices.push_back( base + index );
				draws.push_back(
				    { materials.size() - 1, first, std::uint32_t( part.indices.size() ) } );
			}
		}

		// Buffers and targets.
		auto buffer = [&]( std::uint64_t size, std::initializer_list<ResourceUsage> usages,
		                  MemoryKind memory = MemoryKind::kDeviceLocal ) -> BufferId
		{
			BufferDesc desc;
			desc.size = size;
			desc.usages = UsageSet( usages );
			desc.memory = memory;
			auto made = device->CreateBuffer( desc );
			return made ? made.Value() : BufferId();
		};
		const auto vertexBytes = std::as_bytes( std::span( vertices ) );
		const auto indexBytes = std::as_bytes( std::span( indices ) );
		const BufferId vertexBuffer = buffer(
		    vertexBytes.size(), { ResourceUsage::kCopyDestination, ResourceUsage::kVertex } );
		const BufferId indexBuffer =
		    buffer( indexBytes.size(), { ResourceUsage::kCopyDestination, ResourceUsage::kIndex } );
		const std::uint64_t pixelBytes = std::uint64_t( options.width ) * options.height * 8;
		const BufferId readback =
		    buffer( pixelBytes, { ResourceUsage::kCopyDestination }, MemoryKind::kReadback );
		TextureDesc colorDesc;
		colorDesc.format = colorFormat;
		colorDesc.width = options.width;
		colorDesc.height = options.height;
		colorDesc.usages = { ResourceUsage::kColorAttachment, ResourceUsage::kCopySource,
		    ResourceUsage::kCopyDestination };
		TextureDesc depthDesc = colorDesc;
		depthDesc.format = depthFormat;
		// Sampled by the volumetric composite.
		depthDesc.usages = { ResourceUsage::kDepthWrite, ResourceUsage::kSampled };
		auto color = device->CreateTexture( colorDesc );
		auto depth = device->CreateTexture( depthDesc );
		if ( !vertexBuffer.IsValid() || !indexBuffer.IsValid() || !readback.IsValid() || !color ||
		     !depth )
			return Fail( "a buffer or target was refused" );

		// The camera: the fixture's eye and basis, its horizontal field of view.
		const float aspect = float( options.width ) / float( options.height );
		const float horizontal = options.horizontalFov * 3.14159265358979f / 180.0f;
		const float vertical = 2.0f * std::atan( std::tan( horizontal / 2.0f ) / aspect );
		const math::float3 target{ options.eye.x + options.forward.x,
		    options.eye.y + options.forward.y, options.eye.z + options.forward.z };
		const math::float4x4 view = math::LookAt( options.eye, target, options.up );
		const math::float4x4 projection = math::Perspective( vertical, aspect, 1.0f, 65536.0f );
		const math::float4x4 toClip = math::Multiply( projection, view );
		material::FamilyDrawConstants constants;
		std::memcpy( constants.toClip, &toClip, sizeof( constants.toClip ) );

		// The fog's froxels: the light grid of this view (render.pass.lights,
		// its desktop limits) subdivided 8 x 4, the owner of the depth split.
		std::unique_ptr<pass::volumetric::VolumetricRenderer> fog;
		std::optional<pass::lights::ClusterGrid> fogGrid;
		pass::volumetric::FroxelLayout fogLayout;
		CookieArray cookies;
		if ( volumetric )
		{
			pass::lights::ClusterViewDesc gridView;
			gridView.view = view;
			gridView.projection = projection;
			gridView.widthPixels = options.width;
			gridView.heightPixels = options.height;
			gridView.nearZ = 1.0f;
			gridView.farZ = 65536.0f;
			auto lightGrid =
			    pass::lights::CreateClusterGrid( gridView, pass::lights::DesktopClusterLimits() );
			if ( !lightGrid )
				return Fail( "the light grid does not build" );
			auto fine = pass::lights::SubdivideClusterGrid( lightGrid.Value(), 8, 4 );
			if ( !fine )
				return Fail( "the fog's grid does not subdivide the light grid" );
			fogGrid = std::move( fine ).Value();
			fogLayout = FroxelLayoutOf( *fogGrid );
			auto created = pass::volumetric::VolumetricRenderer::Create( *device, colorFormat );
			if ( !created )
				return Fail( "the volumetric pass was refused" );
			fog = std::move( created ).Value();
			if ( std::optional<std::string> why =
			         cookies.Create( *device, files, media.cookieNames ) )
				return Fail( *why );
			std::printf(
			    "render_lab: volumetric fog: %zu volumes, %zu lights (%u unsupported), %zu "
			    "projectors, froxels %ux%ux%u, density scale %g\n",
			    media.medium.volumes.size(), media.lights.size(), media.unsupportedLights,
			    media.projectors.size(), fogLayout.tilesX, fogLayout.tilesY, fogLayout.slices,
			    double( media.medium.densityScale ) );
		}
		pass::volumetric::VolumetricView fogView;
		fogView.froxels = &fogLayout;
		fogView.projection = projection;
		fogView.eye = options.eye;
		pass::volumetric::VolumetricFrame fogFrame;
		fogFrame.medium = &media.medium;
		fogFrame.lights = media.lights;
		fogFrame.projectors = media.projectors;
		fogFrame.sampling = options.fogSampling;
		pass::volumetric::VolumetricTargets fogTargets;
		fogTargets.color = color.Value();
		fogTargets.depth = depth.Value();
		fogTargets.depthUsage = ResourceUsage::kDepthWrite;
		fogTargets.width = options.width;
		fogTargets.height = options.height;
		fogTargets.cookies = cookies.Texture();

		// Record: uploads, the world and the model, the readback.
		auto encoded = device->BeginEncoder( QueueKind::kGraphics );
		if ( !encoded )
			return Fail( "no encoder" );
		CommandEncoder &encoder = encoded.Value();
		cache.RecordUploads( encoder );
		groups.RecordUploads( encoder );
		if ( fog )
			cookies.RecordUpload( encoder );
		encoder.TransitionBuffer(
		    vertexBuffer, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		encoder.WriteBuffer( vertexBuffer, 0, vertexBytes );
		encoder.TransitionBuffer(
		    vertexBuffer, ResourceUsage::kCopyDestination, ResourceUsage::kVertex );
		encoder.TransitionBuffer(
		    indexBuffer, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		encoder.WriteBuffer( indexBuffer, 0, indexBytes );
		encoder.TransitionBuffer(
		    indexBuffer, ResourceUsage::kCopyDestination, ResourceUsage::kIndex );
		encoder.TransitionTexture(
		    color.Value(), ResourceUsage::kUndefined, ResourceUsage::kColorAttachment );
		encoder.TransitionTexture(
		    depth.Value(), ResourceUsage::kUndefined, ResourceUsage::kDepthWrite );
		const ColorAttachment attachments[] = {
		    { color.Value(), LoadOp::kClear, StoreOp::kStore, { 0, 0, 0, 1 }, {} } };
		RenderingDesc rendering;
		rendering.colors = attachments;
		rendering.depth = DepthAttachment{ depth.Value(), LoadOp::kClear, StoreOp::kStore, 1.0f };
		rendering.width = options.width;
		rendering.height = options.height;
		encoder.BeginRendering( rendering );
		encoder.SetViewport( { 0, 0, float( options.width ), float( options.height ), 0, 1 } );
		std::size_t drawn = 0;
		for ( const Draw &draw : draws )
		{
			const SceneMaterial &m = materials[draw.material];
			const material::ResidentGroup *group = groups.Group( m.groupId );
			if ( !group )
			{
				status = Fail( "a material group is not resident" );
				break;
			}
			auto pipeline = resolver.Value()->DebugPipeline(
			    m.program, frame::DebugSpecializationFor( options.debug, m.program.name ) );
			if ( !pipeline )
			{
				status = Fail( pipeline.Error() );
				break;
			}
			encoder.SetPipeline( pipeline.Value() );
			if ( m.program.request.frameLayout.IsValid() )
				encoder.SetBindGroup( BindGroupRole::kFrame,
				    groups.Group( frameGroupOf[m.program.request.frameLayout.value] )->group );
			if ( m.program.request.viewLayout.IsValid() )
				encoder.SetBindGroup( BindGroupRole::kView,
				    groups.Group( viewGroupOf[m.program.request.viewLayout.value] )->group );
			encoder.SetBindGroup( BindGroupRole::kMaterial, group->group );
			if ( m.program.request.drawLayout.IsValid() )
				encoder.SetBindGroup( BindGroupRole::kDraw,
				    groups.Group( drawGroupOf[m.program.request.drawLayout.value] )->group );
			encoder.SetVertexBuffer( 0, vertexBuffer, 0 );
			encoder.SetIndexBuffer( indexBuffer, 0, IndexFormat::kUint32 );
			encoder.SetDrawConstants( 0, std::as_bytes( std::span( &constants, 1 ) )
			                                 .first( m.program.request.drawConstantBytes ) );
			encoder.DrawIndexed( draw.indexCount, 1, draw.firstIndex, 0, 0 );
			++drawn;
		}
		encoder.EndRendering();
		if ( fog && options.inscatterOnly )
		{
			encoder.TransitionTexture(
			    color.Value(), ResourceUsage::kColorAttachment, ResourceUsage::kCopyDestination );
			encoder.ClearTexture( color.Value(), { 0, 0, 0, 1 } );
			encoder.TransitionTexture(
			    color.Value(), ResourceUsage::kCopyDestination, ResourceUsage::kColorAttachment );
		}
		if ( fog && status == 0 && !fog->Record( encoder, fogView, fogFrame, fogTargets ) )
			status = Fail( "the volumetric pass did not record" );
		encoder.TransitionTexture(
		    color.Value(), ResourceUsage::kColorAttachment, ResourceUsage::kCopySource );
		encoder.TransitionBuffer(
		    readback, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		encoder.CopyTextureToBuffer(
		    color.Value(), readback, { 0, 0, 0, options.width, options.height } );
		auto token = device->Submit( QueueKind::kGraphics, { &encoder, 1 }, {} );
		if ( !token )
			return Fail( "the frame was refused at submission" );
		(void)device->WaitIdle();
		cache.Retire( token.Value() );
		groups.Retire( token.Value() );
		if ( fog )
			fog->Collect( token.Value() );
		if ( status == 0 )
		{
			std::vector<std::byte> pixels( pixelBytes );
			if ( !device->ReadBuffer( readback, 0, pixels ) )
				return Fail( "the frame did not read back" );
			std::vector<float> rgba( std::size_t( options.width ) * options.height * 4 );
			for ( std::size_t i = 0; i < rgba.size(); ++i )
			{
				std::uint16_t half;
				std::memcpy( &half, pixels.data() + i * 2, sizeof( half ) );
				rgba[i] = HalfToFloat( half );
			}
			if ( !WritePfm( options.out, options.width, options.height, rgba ) )
				return Fail( "cannot write " + options.out.string() );
			std::printf( "render_lab: %zu draws, %zu vertices, %zu materials -> %s\n", drawn,
			    vertices.size(), materials.size(), options.out.string().c_str() );
		}
		// The pass's time (perf, binding rule 7): its three stages in a
		// submission of their own, submission to idle, median over the
		// repeats. The frame is already read back; each repeat composites
		// the fog over the target again.
		if ( fog && status == 0 && options.timeRepeats > 0 )
		{
			std::vector<double> times;
			for ( std::uint32_t i = 0; i < options.timeRepeats; ++i )
			{
				auto timed = device->BeginEncoder( QueueKind::kGraphics );
				if ( !timed )
					return Fail( "no encoder" );
				timed.Value().TransitionTexture(
				    color.Value(), ResourceUsage::kCopySource, ResourceUsage::kColorAttachment );
				if ( !fog->Record( timed.Value(), fogView, fogFrame, fogTargets ) )
					return Fail( "the volumetric pass did not record" );
				timed.Value().TransitionTexture(
				    color.Value(), ResourceUsage::kColorAttachment, ResourceUsage::kCopySource );
				const auto start = std::chrono::steady_clock::now();
				auto timedToken = device->Submit( QueueKind::kGraphics, { &timed.Value(), 1 }, {} );
				if ( !timedToken )
					return Fail( "a timed submission was refused" );
				(void)device->WaitIdle();
				times.push_back( std::chrono::duration<double, std::milli>(
				    std::chrono::steady_clock::now() - start )
				        .count() );
				fog->Collect( timedToken.Value() );
			}
			std::sort( times.begin(), times.end() );
			std::printf( "render_lab: volumetric time median %.3f ms, min %.3f ms over %u\n",
			    times[times.size() / 2], times.front(), options.timeRepeats );
		}
		for ( BufferId id : { vertexBuffer, indexBuffer, readback } )
			(void)device->Release( id, token.Value() );
		(void)device->Release( color.Value(), token.Value() );
		(void)device->Release( depth.Value(), token.Value() );
		(void)device->WaitIdle();
	}
	device.reset();
	if ( options.validate )
	{
		std::printf( "render_lab: validation messages %llu\n",
		    static_cast<unsigned long long>( messages.load() ) );
		if ( messages.load() != 0 )
			return 1;
	}
	return status;
}

} // namespace

int main( int argc, char **argv )
{
	if ( argc >= 3 && std::string( argv[1] ) == "suite" )
		return RunSuite( argc - 2, argv + 2 );
	const std::optional<Options> options = ParseOptions( argc, argv );
	if ( !options )
	{
		std::fprintf( stderr,
		    "usage: render_lab --game <dir> --map <file.bsp> --eye x,y,z --forward x,y,z --up "
		    "x,y,z --hfov degrees --size WxH --out <file.pfm> [--model <models/x.mdl> "
		    "--model-origin x,y,z] [--validate] [--dump-mesh] [--core-direct] [--debug-* ...]\n"
		    "           [--fog-scale s] [--no-volumetric] [--time n]\n"
		    "       render_lab suite <name> [--validate] [--seeded <defect>]\n" );
		return 2;
	}
	return Run( *options );
}
