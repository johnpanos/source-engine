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
//			lit by the lump's lights and projectors as render.composition's
//			map_media.h reads them for the product too (unshadowed in this
//			slice). --fog-scale s multiplies every density (0: the fixture's
//			density-zero state), --no-volumetric leaves the pass out, and
//			--fog-samples xy,depth sets the inject stage's samples per froxel
//			(default 2,4: 2 x 2 across and 4 along), --time n prints the
//			pass's median time over n more submissions,
//			and --inscatter-only clears the frame before the composite (the
//			in-scattered light alone, as Cycles' Volume Direct pass).
//
//			--output-peak p applies the game's output (render.pass.output:
//			exposure 1, the BT.2390 tone map from scene peak p onto SDR
//			headroom 1) before readback, so a game/lab comparison judges the
//			display values both show; without it the image is the frame's
//			linear scene values.
//
//			The frame's debug controls (RFC 0014) apply through the --debug-*
//			options, as cl_render_debug_* apply in the product: validated by
//			render.frame, turned into each program's specialization, drawn
//			with the program's debug pipeline.
//
//			Usage: render_lab --game <dir> --map <file.bsp> --eye x,y,z
//			           --forward x,y,z --up x,y,z --hfov degrees --size WxH
//			           --out <file.pfm> [--model <models/x.mdl> --model-origin
//			           x,y,z] [--model-no-shadow] [--mover x0,y0,z0,x1,y1,z1,material]
//			           [--validate] [--dump-mesh] [--core-direct]
//			           [--debug-view n] [--fog-scale s] [--no-volumetric]
//			           [--output-peak p] [--entities portal|portal2] [--time n]
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
#include "lab_shadows.h"
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
#include "render/material/scene_terms.h"
#include "render/material/vmt_import.h"
#include "render/math/matrix.h"
#include "render/pass/ao/ao.h"
#include "render/pass/bounce/bounce.h"
#include "render/pass/shadows/shadow_views.h"
#include "render/pass/lights/clusters.h"
#include "render/pass/shadows/shadow_passes.h"
#include "render/pass/ssr/ssr.h"
#include "render/resources/mesh_cache.h"
#include "render/pass/output/output.h"
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
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <span>
#include <sstream>
#include <string>
#include <string_view>
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
	fs::path assetPackage;
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
	// The game's output (render.pass.output, render.output.v1) applied to
	// the frame before readback, at this scene peak and SDR headroom 1, so
	// the image holds the display values a game frame shows; 0 leaves the
	// frame's linear scene values (the default).
	float outputPeak = 0.0f;
	// Whose server reads the map's entity keys (--entities portal|portal2;
	// env_projectedtexture's lightcolor differs, map_lights.h).
	pass::lights::EntityConvention entityConvention = pass::lights::EntityConvention::kPortal;
	// A diagnostic: the frame cleared to black before the composite, so the
	// image is the medium's in-scattered light alone (Cycles' Volume Direct).
	bool inscatterOnly = false;
	// Passes left out: the shadows, the ambient occlusion, the reflections.
	bool noShadows = false;
	bool noAo = false;
	bool noSsr = false;
	bool noBounce = false;
	// The model is drawn but casts no shadow: the fixtures' references make
	// every prop a receiver only (gi_reference_blender.py).
	bool modelNoShadow = false;
	// Moving objects the bake did not see (a closed door): boxes in world
	// units, drawn as a mesh (the probe volume's indirect light and every
	// light's direct light at runtime) with a world material, and cast into
	// the atlas as the product's movers are (render.dynamic-occlusion).
	struct Mover
	{
		math::float3 min{ 0, 0, 0 };
		math::float3 max{ 0, 0, 0 };
		std::string material;
	};
	std::vector<Mover> movers;
	std::uint32_t rsmSize = 128; // a projector's reflective shadow map, texels across
	std::uint32_t timeRepeats = 0;
	// The inject stage's stratified samples per froxel (across, along).
	pass::volumetric::VolumetricSampling fogSampling = composition::kFroxelSampling;
	// The light grid's subdivision for the medium (--fog-grid across,depth).
	std::uint32_t fogTileDivisor = composition::kFroxelTileDivisor;
	std::uint32_t fogSliceMultiplier = composition::kFroxelSliceMultiplier;
	frame::DebugControls debug;
};

// The texture cache's names of the map's lightmap pages.
constexpr const char *kLightmapPage = "lab:lightmap";
constexpr const char *kLightmapGradient = "lab:lightmap-gradient";
constexpr const char *kLightmapIndirect = "lab:lightmap-indirect";
constexpr const char *kSplitSumTable = "lab:split-sum";
constexpr const char *kLtcTable = "lab:ltc";

int Fail( const std::string &why )
{
	std::fprintf( stderr, "render_lab: %s\n", why.c_str() );
	return 1;
}

bool ParseVector( const char *text, math::float3 &out )
{
	return std::sscanf( text, "%f,%f,%f", &out.x, &out.y, &out.z ) == 3;
}

// --mover x0,y0,z0,x1,y1,z1,material: a box's corners and its material.
bool ParseMover( const char *text, Options::Mover &out )
{
	if ( !text )
		return false;
	int consumed = 0;
	if ( std::sscanf( text, "%f,%f,%f,%f,%f,%f,%n", &out.min.x, &out.min.y, &out.min.z, &out.max.x,
	         &out.max.y, &out.max.z, &consumed ) != 6 ||
	     consumed <= 0 )
		return false;
	out.material = text + consumed;
	return !out.material.empty() && out.min.x < out.max.x && out.min.y < out.max.y &&
	       out.min.z < out.max.z;
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
		else if ( arg == "--model-no-shadow" )
			options.modelNoShadow = true;
		else if ( arg == "--no-volumetric" )
			options.noVolumetric = true;
		else if ( arg == "--inscatter-only" )
			options.inscatterOnly = true;
		else if ( arg == "--no-shadows" )
			options.noShadows = true;
		else if ( arg == "--no-ao" )
			options.noAo = true;
		else if ( arg == "--no-ssr" )
			options.noSsr = true;
		else if ( arg == "--no-bounce" )
			options.noBounce = true;
		else if ( !value )
			return std::nullopt;
		else if ( arg == "--game" )
			options.game = take();
		else if ( arg == "--asset-package" )
			options.assetPackage = take();
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
		else if ( arg == "--mover" )
		{
			Options::Mover mover;
			if ( !ParseMover( value, mover ) )
				return std::nullopt;
			options.movers.push_back( std::move( mover ) );
			take();
		}
		else if ( arg == "--rsm-size" )
			options.rsmSize = std::uint32_t( std::max( 16, std::atoi( take() ) ) );
		else if ( arg == "--hfov" )
			options.horizontalFov = float( std::atof( take() ) );
		else if ( arg == "--fog-scale" )
			options.fogScale = float( std::atof( take() ) );
		else if ( arg == "--entities" )
		{
			const std::string game = take();
			if ( game != "portal" && game != "portal2" )
				return std::nullopt;
			options.entityConvention = game == "portal2" ? pass::lights::EntityConvention::kPortal2
			                                             : pass::lights::EntityConvention::kPortal;
		}
		else if ( arg == "--output-peak" )
			options.outputPeak = float( std::atof( take() ) );
		else if ( arg == "--time" )
			options.timeRepeats = std::uint32_t( std::atoi( take() ) );
		else if ( arg == "--fog-grid" && std::sscanf( value, "%u,%u", &options.fogTileDivisor,
		                                    &options.fogSliceMultiplier ) == 2 )
			take();
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
	bool mesh = false; // a model's: drawn without the lightmap
	bool sky = false;  // the sky (material::IsSkySurface): no shadow caster
};

std::optional<std::string> ImportMaterial(
    const GameFiles &files, const std::string &path, material::MaterialDesc &out )
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
	out = std::move( imported ).Value();
	return std::nullopt;
}

// Stages the textures a resolved program's material group names.
std::optional<std::string> StageProgramTextures(
    const GameFiles &files, resources::TextureCache &cache, const material::ResolvedProgram &program )
{
	for ( const material::ProgramTexture &texture : program.request.material.textures )
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

// Loads a VMT and resolves its program: a world surface's (a PBRMetalRough
// material is the surface program's pbr point with the lightmap basis, RFC
// 0016 K11 "World PBR materials"), or with `mesh` a dynamic model's.
std::optional<std::string> ResolveMaterial( const GameFiles &files,
    material::ProgramResolver &resolver, resources::TextureCache &cache, const std::string &path,
    bool mesh, SceneMaterial &out )
{
	material::MaterialDesc desc;
	if ( std::optional<std::string> why = ImportMaterial( files, path, desc ) )
		return why;
	auto program = mesh ? resolver.ResolveMesh( desc ) : resolver.Resolve( desc );
	if ( !program )
		return "material " + path + ": " + program.Error();
	out.program = std::move( program ).Value();
	out.mesh = mesh;
	out.sky = material::IsSkySurface( desc );
	return StageProgramTextures( files, cache, out.program );
}

// Stages a table (SplitSumTable, LtcTable) as an RGBA32F texture.
bool StageTable(
    resources::TextureCache &cache, const std::string &name, const material::PbrSplitSumTable &table )
{
	TextureDesc desc;
	desc.format = table.format;
	desc.width = table.width;
	desc.height = table.height;
	desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kSampled };
	return bool( cache.Stage( name, desc, std::as_bytes( std::span( table.texels ) ) ) );
}

int Run( const Options &options )
{
	const GameFiles files( options.game, options.assetPackage );
	if ( !options.assetPackage.empty() && !files.PackageReady() )
		return Fail( "--asset-package has no valid assets.index" );

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
	// cannot be drawn so (no silent fallback to the total layer). The
	// indirect layer, when the map carries one, is what the view's ambient
	// occlusion may darken; with --core-direct it is the basis itself
	// (kSurfaceRuntimeDirect), and the total layer's page is still bound for
	// the sun's baked mask.
	const mapcontainer::WorldLightmapLayer bakedRole = BakedLightmapLayer( options.coreDirect );
	if ( mapcontainer::WorldLightmapLayerIndex( layout, bakedRole ) < 0 )
		return Fail( std::string( "LMAP carries no " ) +
		             mapcontainer::WorldLightmapLayerName( bakedRole ) + " layer" );
	const int bakedLayer =
	    mapcontainer::WorldLightmapLayerIndex( layout, mapcontainer::WorldLightmapLayer::Total );
	const int indirectLayer =
	    mapcontainer::WorldLightmapLayerIndex( layout, mapcontainer::WorldLightmapLayer::Indirect );

	// The entity lump: participating media and the frame's lights.
	LabMedia media;
	LabLights lights;
	{
		mapcontainer::MapLumpInfo info{};
		if ( container->FindLegacyLump( 0, &info ) && info.storedSize == info.uncompressedSize )
		{
			const auto parsed = ParseEntityLump(
			    mapBytes->substr( std::size_t( info.offset ), std::size_t( info.storedSize ) ) );
			if ( !parsed )
				return Fail( options.map.string() + ": the entity lump does not parse" );
			media = MediaFromEntities( *parsed, options.entityConvention );
			lights = LightsFromEntities( *parsed, options.entityConvention );
		}
		media.medium.densityScale = options.fogScale;
	}
	const bool volumetric = media.present && !options.noVolumetric;
	// Each light counts once per surface: with the bake's total layer every
	// baked light's diffuse light is the bake's and the core adds its
	// specular lobe; with --core-direct (kSurfaceRuntimeDirect) the world's
	// program draws both lobes over the indirect layer, and a mesh's always
	// does (kSurfaceMeshDirect). Projectors are never baked.
	const bool diffuseInBake = true;
	auto passOn = [&]( std::uint32_t term )
	{
		return ( options.debug.termsOff & term ) == 0;
	};
	const bool aoOn = !options.noAo && passOn( shaderlib::kDebugTermAo );
	const bool ssrOn = !options.noSsr && passOn( shaderlib::kDebugTermSsr );
	std::printf( "render_lab: lights %zu, area lights %zu, projectors %zu, sun %s (%u unsupported)\n",
	    lights.lights.size(), lights.areas.size(), lights.projectors.size(),
	    lights.sun ? "yes" : "no", lights.unsupported );

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
		auto resolver = material::ProgramResolver::Create(
		    *device, colorFormat, depthFormat, 1, material::VertexLayout::kSurface );
		if ( !resolver )
			return Fail( "no programs: " + resolver.Error() );
		resources::TextureCache cache( *device );
		resources::MeshCache meshes( *device );
		material::GroupResidency groups( *device, cache );

		// The lightmap pages: linear light, sampled as it is (scale 1). The
		// flat page (its alpha the sun's baked visibility when the bake
		// packed it), the gradient page the lightmap basis reads (a
		// directional page's right half; zero for a flat page, which is the
		// flat light bitwise), and the indirect layer's flat page.
		bool directional = false;
		bool sunMask = false;
		bool indirectPage = false;
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
			// The gradient page: the total's, or with --core-direct the
			// indirect layer's own (zero when the bake wrote none).
			LightmapLayerPages indirect;
			if ( indirectLayer >= 0 )
				indirect = SplitLightmapLayer(
				    std::as_bytes( std::span( lmap->data() + layout.layerOffset[indirectLayer],
				        std::size_t( layout.layerBytes ) ) ),
				    layout.width, layout.height );
			const LightmapLayerPages &basis = options.coreDirect ? indirect : pages;
			directional =
			    basis.Directional() && basis.width == pages.width && basis.height == pages.height;
			if ( !cache.Stage( kLightmapGradient, desc,
			         directional ? std::span<const std::byte>( basis.gradient )
			                     : std::span<const std::byte>( zero ) ) )
				return Fail( "the lightmap gradient page was refused" );
			// The sun's mask: any texel of the total page's alpha below one.
			if ( lights.sun )
			{
				for ( std::size_t t = 0; t + 8 <= pages.flat.size() && !sunMask; t += 8 )
				{
					std::uint16_t alpha;
					std::memcpy( &alpha, pages.flat.data() + t + 6, sizeof( alpha ) );
					sunMask = HalfToFloat( alpha ) < 0.999f;
				}
			}
			if ( !indirect.flat.empty() && indirect.width == pages.width &&
			     indirect.height == pages.height )
			{
				if ( !cache.Stage( kLightmapIndirect, desc, indirect.flat ) )
					return Fail( "the lightmap's indirect page was refused" );
				indirectPage = true;
			}
			if ( options.coreDirect && !indirectPage )
				return Fail( "--core-direct needs the LMAP's indirect layer" );
			std::printf( "render_lab: LMAP v%u %ux%u, the %s layer%s%s%s\n", layout.version,
			    layout.width, layout.height, mapcontainer::WorldLightmapLayerName( bakedRole ),
			    directional ? ", directional" : "", sunMask ? ", sun mask" : "",
			    indirectPage ? ", indirect layer" : "" );
		}

		// The map's probe volume (PRBV) for probe_volume.glsl, when it has
		// one: lab:probe-volume-atlas and lab:probe-volume-grids.
		material::SurfaceMapTextures map;
		mapcontainer::ProbeVolumeLayout probeLayout{};
		if ( const std::optional<std::string> prbv = lump( mapcontainer::kLumpProbeVolume ) )
		{
			if ( std::optional<std::string> why = StageProbeVolume( cache, "lab:probe-volume",
			         std::as_bytes( std::span( prbv->data(), prbv->size() ) ), probeLayout ) )
				return Fail( *why );
			map.probeAtlas = "lab:probe-volume-atlas";
			map.probeGrids = "lab:probe-volume-grids";
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
			map.reflectionProbes = "lab:reflection-probes";
			std::printf( "render_lab: RPRB %u probe%s, %u mips from %u wide%s\n",
			    probesLayout.count, probesLayout.count == 1 ? "" : "s", probesLayout.mipCount,
			    probesLayout.width, probesLayout.relight ? ", relightable" : "" );
		}
		if ( !StageTable( cache, kSplitSumTable, material::SplitSumTable() ) ||
		     !StageTable( cache, kLtcTable, material::LtcTable() ) )
			return Fail( "the pbr point's tables were refused" );

		// The projected lights' bounce (render.pass.bounce), into the probe
		// volume's layout.
		const bool bounceOn = !options.noBounce && !map.probeAtlas.empty() &&
		                      !lights.projectors.empty() &&
		                      passOn( shaderlib::kDebugTermProjected );
		TextureDesc bounceDesc;
		TextureId bounceAtlas;
		if ( bounceOn )
		{
			const resources::TextureEntry *atlasEntry = cache.Find( map.probeAtlas );
			if ( !atlasEntry )
				return Fail( "the probe atlas is not staged" );
			bounceDesc.format = Format::kRGBA16Float;
			bounceDesc.width = atlasEntry->desc.width;
			bounceDesc.height = atlasEntry->desc.height;
			bounceDesc.usages = { ResourceUsage::kStorageWrite, ResourceUsage::kSampled,
			    ResourceUsage::kCopyDestination };
			auto made = device->CreateTexture( bounceDesc );
			if ( !made )
				return Fail( "the bounce atlas was refused" );
			bounceAtlas = made.Value();
			map.probeBounce = bounceAtlas;
			map.probeBounceDesc = bounceDesc;
		}
		// The same term policy selects the game's world-stage variants.
		material::SceneTermInputs termInputs;
		termInputs.runtimeDirect = options.coreDirect;
		termInputs.indirectLightmap = indirectPage;
		termInputs.totalDirectionalLightmap = directional && !options.coreDirect;
		termInputs.indirectDirectionalLightmap = directional && options.coreDirect;
		termInputs.probeVolume = !map.probeAtlas.empty();
		termInputs.probeBounce = bounceOn;
		termInputs.reflectionProbes = !map.reflectionProbes.empty();
		termInputs.ambientOcclusion = aoOn;
		const std::uint32_t sceneTerms = material::SceneTerms( termInputs );
		resolver.Value()->SetWorldPbr( true, sceneTerms );

		// The world's materials.
		std::vector<SceneMaterial> materials( mesh.materials.size() );
		for ( std::size_t i = 0; i < mesh.materials.size(); ++i )
		{
			if ( std::optional<std::string> why = ResolveMaterial(
			         files, *resolver.Value(), cache, mesh.materials[i], false, materials[i] ) )
				return Fail( *why );
		}

		// The world's vertices as the program reads them (the world vertex:
		// its normal and tangent frame, T = N x S times the handedness).
		std::vector<material::SurfaceWorldVertex> vertices( mesh.vertices.size() );
		for ( std::size_t i = 0; i < mesh.vertices.size(); ++i )
		{
			const mapcontainer::WorldMeshVertex &from = mesh.vertices[i];
			material::SurfaceWorldVertex &to = vertices[i];
			std::copy( from.position, from.position + 3, to.position );
			std::copy( from.uv, from.uv + 2, to.uv );
			std::copy( from.lightmapUv, from.lightmapUv + 2, to.lightmapUv );
			std::copy( from.normal, from.normal + 3, to.normal );
			std::copy( from.tangent, from.tangent + 3, to.tangentS );
			const math::float3 n{ from.normal[0], from.normal[1], from.normal[2] };
			const math::float3 t{ from.tangent[0], from.tangent[1], from.tangent[2] };
			const math::float3 b = math::Cross( n, t ) * from.tangentSign;
			to.tangentT[0] = b.x;
			to.tangentT[1] = b.y;
			to.tangentT[2] = b.z;
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
		// The model's indices (after the world's), left out of the casters
		// with --model-no-shadow.
		std::uint32_t modelFirstIndex = std::uint32_t( indices.size() );
		std::uint32_t modelIndexCount = 0;

		// The studio model: its first body at the origin, in world space on
		// the world vertex. PBRMetalRough and the supported VertexLitGeneric
		// subset use the same PBR mesh point with probes and clustered lights.
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
				if ( std::optional<std::string> why =
				         ResolveMaterial( files, *resolver.Value(), cache, path, true, resolved ) )
					return Fail( *why );
				materials.push_back( std::move( resolved ) );
				const std::uint32_t base = std::uint32_t( vertices.size() );
				for ( const mdl::Vertex &v : part.vertices )
				{
					material::SurfaceWorldVertex to;
					to.position[0] = v.position.x + options.modelOrigin.x;
					to.position[1] = v.position.y + options.modelOrigin.y;
					to.position[2] = v.position.z + options.modelOrigin.z;
					to.uv[0] = v.u;
					to.uv[1] = v.v;
					to.normal[0] = v.normal.x;
					to.normal[1] = v.normal.y;
					to.normal[2] = v.normal.z;
					const math::float3 n{ v.normal.x, v.normal.y, v.normal.z };
					const math::float3 t =
					    v.tangentSign != 0.0f
					        ? math::float3{ v.tangent.x, v.tangent.y, v.tangent.z }
					        : math::Normalize( math::Cross( n, std::fabs( n.z ) < 0.9f
					                                               ? math::float3{ 0, 0, 1 }
					                                               : math::float3{ 1, 0, 0 } ) );
					const math::float3 b =
					    math::Cross( n, t ) * ( v.tangentSign == 0.0f ? 1.0f : v.tangentSign );
					to.tangentS[0] = t.x;
					to.tangentS[1] = t.y;
					to.tangentS[2] = t.z;
					to.tangentT[0] = b.x;
					to.tangentT[1] = b.y;
					to.tangentT[2] = b.z;
					vertices.push_back( to );
				}
				const std::uint32_t first = std::uint32_t( indices.size() );
				for ( std::uint32_t index : part.indices )
					indices.push_back( base + index );
				draws.push_back(
				    { materials.size() - 1, first, std::uint32_t( part.indices.size() ) } );
			}
		}

		modelIndexCount = std::uint32_t( indices.size() ) - modelFirstIndex;

		// The movers: each box's six faces, outward, as the model's parts
		// are drawn (a mesh: its material's mesh point).
		for ( const Options::Mover &mover : options.movers )
		{
			SceneMaterial resolved;
			if ( std::optional<std::string> why = ResolveMaterial(
			         files, *resolver.Value(), cache, mover.material, true, resolved ) )
				return Fail( "mover material " + mover.material + ": " + *why );
			materials.push_back( std::move( resolved ) );
			const math::float3 lo = mover.min, hi = mover.max;
			// Each face: its normal, its tangent and four corners, counter-
			// clockwise seen from outside.
			struct Face
			{
				math::float3 normal, tangent;
				math::float3 corners[4];
			};
			const Face faces[6] = {
			    { { 1, 0, 0 }, { 0, 1, 0 },
			        { { hi.x, lo.y, lo.z }, { hi.x, hi.y, lo.z }, { hi.x, hi.y, hi.z },
			            { hi.x, lo.y, hi.z } } },
			    { { -1, 0, 0 }, { 0, -1, 0 },
			        { { lo.x, hi.y, lo.z }, { lo.x, lo.y, lo.z }, { lo.x, lo.y, hi.z },
			            { lo.x, hi.y, hi.z } } },
			    { { 0, 1, 0 }, { -1, 0, 0 },
			        { { hi.x, hi.y, lo.z }, { lo.x, hi.y, lo.z }, { lo.x, hi.y, hi.z },
			            { hi.x, hi.y, hi.z } } },
			    { { 0, -1, 0 }, { 1, 0, 0 },
			        { { lo.x, lo.y, lo.z }, { hi.x, lo.y, lo.z }, { hi.x, lo.y, hi.z },
			            { lo.x, lo.y, hi.z } } },
			    { { 0, 0, 1 }, { 1, 0, 0 },
			        { { lo.x, lo.y, hi.z }, { hi.x, lo.y, hi.z }, { hi.x, hi.y, hi.z },
			            { lo.x, hi.y, hi.z } } },
			    { { 0, 0, -1 }, { 1, 0, 0 },
			        { { lo.x, hi.y, lo.z }, { hi.x, hi.y, lo.z }, { hi.x, lo.y, lo.z },
			            { lo.x, lo.y, lo.z } } },
			};
			const std::uint32_t first = std::uint32_t( indices.size() );
			for ( const Face &face : faces )
			{
				const std::uint32_t base = std::uint32_t( vertices.size() );
				const math::float3 b = math::Cross( face.normal, face.tangent );
				const float uvs[4][2] = { { 0, 1 }, { 1, 1 }, { 1, 0 }, { 0, 0 } };
				for ( int c = 0; c < 4; ++c )
				{
					material::SurfaceWorldVertex v;
					v.position[0] = face.corners[c].x;
					v.position[1] = face.corners[c].y;
					v.position[2] = face.corners[c].z;
					v.uv[0] = uvs[c][0];
					v.uv[1] = uvs[c][1];
					v.normal[0] = face.normal.x;
					v.normal[1] = face.normal.y;
					v.normal[2] = face.normal.z;
					v.tangentS[0] = face.tangent.x;
					v.tangentS[1] = face.tangent.y;
					v.tangentS[2] = face.tangent.z;
					v.tangentT[0] = b.x;
					v.tangentT[1] = b.y;
					v.tangentT[2] = b.z;
					vertices.push_back( v );
				}
				for ( std::uint32_t index : { 0u, 1u, 2u, 0u, 2u, 3u } )
					indices.push_back( base + index );
			}
			draws.push_back(
			    { materials.size() - 1, first, std::uint32_t( indices.size() ) - first } );
		}
		if ( !options.movers.empty() )
			std::printf( "render_lab: %zu mover%s\n", options.movers.size(),
			    options.movers.size() == 1 ? "" : "s" );

		// The camera: the fixture's eye and basis, its horizontal field of view.
		const float aspect = float( options.width ) / float( options.height );
		const float horizontal = options.horizontalFov * 3.14159265358979f / 180.0f;
		const float vertical = 2.0f * std::atan( std::tan( horizontal / 2.0f ) / aspect );
		const math::float3 lookAt{ options.eye.x + options.forward.x,
		    options.eye.y + options.forward.y, options.eye.z + options.forward.z };
		const math::float4x4 view = math::LookAt( options.eye, lookAt, options.up );
		const math::float4x4 projection = math::Perspective( vertical, aspect, 1.0f, 65536.0f );
		const math::float4x4 toClip = math::Multiply( projection, view );
		material::FamilyDrawConstants constants;
		std::memcpy( constants.toClip, &toClip, sizeof( constants.toClip ) );

		// The shadows: every light the surface shades, into one atlas, cast
		// by the world and the model.
		const auto vertexBytes = std::as_bytes( std::span( vertices ) );
		const auto indexBytes = std::as_bytes( std::span( indices ) );
		LabShadows shadowing;
		std::unique_ptr<pass::shadows::ShadowDepthRenderer> shadowRenderer;
		if ( !options.noShadows )
		{
			resources::MeshData casterMesh;
			casterMesh.vertices = vertexBytes;
			casterMesh.vertexStride = sizeof( material::SurfaceWorldVertex );
			// The casters: every draw but the sky's (it casts no shadow) and,
			// with --model-no-shadow, the model's.
			std::vector<std::uint32_t> casterIndices;
			for ( const Draw &draw : draws )
			{
				const bool model = draw.firstIndex >= modelFirstIndex &&
				                   draw.firstIndex < modelFirstIndex + modelIndexCount;
				if ( materials[draw.material].sky || ( model && options.modelNoShadow ) )
					continue;
				casterIndices.insert( casterIndices.end(), indices.begin() + draw.firstIndex,
				    indices.begin() + draw.firstIndex + draw.indexCount );
			}
			casterMesh.indices = std::as_bytes( std::span( casterIndices ) );
			casterMesh.indexFormat = IndexFormat::kUint32;
			auto staged = meshes.Stage( "lab:casters", casterMesh );
			if ( !staged )
				return Fail( "the casters' mesh was refused" );
			{
				auto upload = device->BeginEncoder( QueueKind::kGraphics );
				if ( !upload )
					return Fail( "no encoder" );
				meshes.RecordUploads( upload.Value() );
				CommandEncoder encoders[] = { std::move( upload ).Value() };
				auto token = device->Submit( QueueKind::kGraphics, encoders, {} );
				if ( !token )
					return Fail( "the casters' upload was refused" );
				(void)device->WaitIdle();
				meshes.Retire( token.Value() );
			}
			auto renderer = pass::shadows::ShadowDepthRenderer::Create( *device );
			if ( !renderer )
				return Fail( "the shadow depth renderer was refused" );
			shadowRenderer = std::move( renderer ).Value();
			const pass::shadows::ShadowCaster casters[] = { { staged.Value(), math::float4x4::Identity() } };
			LabShadowCamera camera;
			camera.view = view;
			camera.verticalFovRadians = vertical;
			camera.aspect = aspect;
			camera.nearZ = 1.0f;
			camera.shadowDistance = 8192.0f;
			if ( std::optional<std::string> why = DrawShadows(
			         *device, *shadowRenderer, lights, casters, camera, shadowing ) )
				return Fail( *why );
			std::printf( "render_lab: shadows: %u views, %zu tiles, %u draws\n", shadowing.views,
			    shadowing.tiles.size(), shadowing.draws );
		}

		// The frame group: the tables, the map's probes, the area lights and
		// the sun.
		material::FrameTerms terms;
		terms.lightmapScale = 1.0f; // LMAP holds linear light
		terms.eye[0] = options.eye.x;
		terms.eye[1] = options.eye.y;
		terms.eye[2] = options.eye.z;
		terms.splitSumTable = kSplitSumTable;
		terms.ltcTable = kLtcTable;
		terms.map = map;
		for ( std::size_t i = 0; i < lights.areas.size(); ++i )
			terms.areas.push_back( material::PackAreaLight( lights.areas[i], diffuseInBake,
			    shadowing.areaTiles.empty() ? -1 : shadowing.areaTiles[i] ) );
		if ( lights.sun )
		{
			terms.sunDirection[0] = lights.sun->toSun.x;
			terms.sunDirection[1] = lights.sun->toSun.y;
			terms.sunDirection[2] = lights.sun->toSun.z;
			terms.sunDirection[3] = float(
			    std::tan( 0.5 * double( lights.sun->spreadDegrees ) * 3.14159265358979 / 180.0 ) );
			terms.sunColor[0] = lights.sun->color.x;
			terms.sunColor[1] = lights.sun->color.y;
			terms.sunColor[2] = lights.sun->color.z;
			terms.sunColor[3] = diffuseInBake ? 1.0f : 0.0f;
			terms.sunShadow[0] = float( shadowing.sunFirst );
			terms.sunShadow[1] = float( shadowing.sunCount );
			terms.sunShadow[2] = sunMask ? 1.0f : 0.0f;
		}

		// The view group: the view's clustered lights, its shadows, its
		// projectors and its occlusion.
		CookieArray cookies;
		const bool hasCookies = !lights.cookieNames.empty();
		if ( hasCookies )
		{
			if ( std::optional<std::string> why =
			         cookies.Create( *device, files, lights.cookieNames ) )
				return Fail( *why );
		}
		TextureDesc screenDesc;
		screenDesc.format = Format::kRGBA16Float;
		screenDesc.width = options.width;
		screenDesc.height = options.height;
		screenDesc.usages = { ResourceUsage::kStorageWrite, ResourceUsage::kSampled,
		    ResourceUsage::kCopyDestination };
		auto occlusion = device->CreateTexture( screenDesc );
		if ( !occlusion )
			return Fail( "the occlusion target was refused" );
		std::vector<projected_light::LightGpu> projectorRecords;
		for ( std::size_t i = 0; i < lights.projectors.size(); ++i )
			projectorRecords.push_back( projected_light::PackLightGpu( lights.projectors[i],
			    int( i ), shadowing.projectorTiles.empty() ? -1 : shadowing.projectorTiles[i] ) );
		std::unique_ptr<LabClusterLists> clusterLists;
		std::optional<material::GroupRequest> viewGroupRequest;
		{
			pass::lights::ClusterViewDesc desc;
			desc.view = view;
			desc.projection = projection;
			desc.widthPixels = options.width;
			desc.heightPixels = options.height;
			desc.nearZ = 1.0f;
			desc.farZ = 65536.0f;
			const pass::lights::ClusterLimits limits = pass::lights::DesktopClusterLimits();
			auto grid = pass::lights::CreateClusterGrid( desc, limits );
			if ( !grid )
				return Fail( "the light grid does not build" );
			clusterLists =
			    LabClusterLists::Create( *device, grid.Value(), lights.lights, lights.areas );
			if ( !clusterLists )
				return Fail( "GPU light assignment failed" );
			material::SurfaceViewGpu viewGpu;
			viewGpu.grid[0] = grid.Value().tilesX;
			viewGpu.grid[1] = grid.Value().tilesY;
			viewGpu.grid[2] = grid.Value().slices;
			viewGpu.grid[3] = limits.tileSizePixels;
			viewGpu.slices[0] = grid.Value().sliceScale;
			viewGpu.slices[1] = grid.Value().sliceBias;
			viewGpu.slices[2] = grid.Value().nearZ;
			const math::float4 &z = view.rows[2];
			viewGpu.viewDistance[0] = -z.x;
			viewGpu.viewDistance[1] = -z.y;
			viewGpu.viewDistance[2] = -z.z;
			viewGpu.viewDistance[3] = -z.w;
			viewGpu.counts[0] = float( projectorRecords.size() );
			std::vector<material::SurfaceLightGpu> records;
			for ( std::size_t i = 0; i < lights.lights.size(); ++i )
				records.push_back( material::PackSurfaceLight( lights.lights[i],
				    shadowing.lightTiles.empty() ? -1 : shadowing.lightTiles[i],
				    shadowing.lightLayouts.empty() ? RuntimeShadowLayout::kSingle
				                                   : shadowing.lightLayouts[i],
				    diffuseInBake ) );
			if ( records.empty() )
				records.emplace_back();
			material::SurfaceShadows shadowInputs;
			shadowInputs.atlas = shadowing.atlas;
			shadowInputs.atlasDesc = shadowing.atlasDesc;
			shadowInputs.tiles = shadowing.tiles;
			material::SurfaceProjectors projectorInputs;
			projectorInputs.lights = projectorRecords;
			if ( hasCookies )
			{
				projectorInputs.cookies = cookies.Texture();
				projectorInputs.cookiesDesc = cookies.Desc();
			}
			material::SurfaceScreenInputs screen;
			screen.ambientOcclusion = occlusion.Value();
			screen.ambientOcclusionDesc = screenDesc;
			viewGroupRequest = resolver.Value()->Program().ViewGroup(
			    viewGpu, {}, {}, records, shadowInputs, projectorInputs, screen );
			clusterLists->Bind( *viewGroupRequest );
		}

		// The groups of every material.
		std::uint64_t nextGroup = 1;
		std::map<std::uint64_t, std::uint64_t> drawGroupOf; // draw layout -> group id
		std::map<std::uint64_t, std::uint64_t> pbrDrawGroup;  // mesh (0/1) -> group id
		std::map<std::uint64_t, std::uint64_t> frameGroupOf;
		std::map<std::uint64_t, std::uint64_t> viewGroupOf;
		for ( std::size_t i = 0; i < materials.size(); ++i )
		{
			SceneMaterial &m = materials[i];
			m.groupId = nextGroup++;
			if ( !groups.Set( m.groupId, m.program.request.material ) )
				return Fail( "a material group was refused" );
			const bool pbr = m.program.name == "pbr";
			const std::uint64_t drawLayout = m.program.request.drawLayout.value;
			if ( pbr && !pbrDrawGroup.count( m.mesh ? 1 : 0 ) )
			{
				// A mesh draw has no lightmap texture inputs.
				const std::vector<std::string> inputs =
				    m.mesh ? std::vector<std::string>{}
				           : std::vector<std::string>{ kLightmapPage, kLightmapGradient,
				                 indirectPage ? kLightmapIndirect : "" };
				auto request = resolver.Value()->DrawGroup( m.program, inputs );
				if ( !request )
					return Fail( "no pbr draw group" );
				pbrDrawGroup[m.mesh ? 1 : 0] = nextGroup++;
				if ( !groups.Set( pbrDrawGroup[m.mesh ? 1 : 0], *request ) )
					return Fail( "the pbr draw group was refused" );
			}
			else if ( !pbr && m.program.request.drawLayout.IsValid() && !drawGroupOf.count( drawLayout ) )
			{
				// The page for each input the program declares (an unlit sky
				// or cable declares none).
				const std::vector<std::string> inputs( m.program.drawInputs.size(), kLightmapPage );
				auto request = resolver.Value()->DrawGroup( m.program, inputs );
				if ( !request )
					return Fail( "no draw group for program " + m.program.name );
				drawGroupOf[drawLayout] = nextGroup++;
				if ( !groups.Set( drawGroupOf[drawLayout], *request ) )
					return Fail( "the draw group was refused" );
			}
			const std::uint64_t viewLayout = m.program.request.viewLayout.value;
			if ( m.program.request.viewLayout.IsValid() && !viewGroupOf.count( viewLayout ) )
			{
				viewGroupOf[viewLayout] = nextGroup++;
				if ( !groups.Set( viewGroupOf[viewLayout], *viewGroupRequest ) )
					return Fail( "the view group was refused" );
			}
			const std::uint64_t frameLayout = m.program.request.frameLayout.value;
			if ( m.program.request.frameLayout.IsValid() && !frameGroupOf.count( frameLayout ) )
			{
				auto request = resolver.Value()->FrameGroup( m.program, terms );
				if ( !request )
					return Fail( "no frame group" );
				frameGroupOf[frameLayout] = nextGroup++;
				if ( !groups.Set( frameGroupOf[frameLayout], *request ) )
					return Fail( "the frame group was refused" );
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
		const BufferId vertexBuffer = buffer(
		    vertexBytes.size(), { ResourceUsage::kCopyDestination, ResourceUsage::kVertex } );
		const BufferId indexBuffer =
		    buffer( indexBytes.size(), { ResourceUsage::kCopyDestination, ResourceUsage::kIndex } );
		const std::uint64_t pixelBytes = std::uint64_t( options.width ) * options.height * 8;
		const BufferId readback =
		    buffer( pixelBytes, { ResourceUsage::kCopyDestination }, MemoryKind::kReadback );
		auto target = [&]( Format format, std::initializer_list<ResourceUsage> usages )
		{
			TextureDesc desc;
			desc.format = format;
			desc.width = options.width;
			desc.height = options.height;
			desc.usages = UsageSet( usages );
			auto made = device->CreateTexture( desc );
			return made ? made.Value() : TextureId();
		};
		const TextureId color = target( colorFormat, { ResourceUsage::kColorAttachment,
		                                                 ResourceUsage::kCopySource,
		                                                 ResourceUsage::kCopyDestination,
		                                                 ResourceUsage::kSampled } );
		const TextureId depth =
		    target( depthFormat, { ResourceUsage::kDepthWrite, ResourceUsage::kSampled } );
		const TextureId prepassNormal = target(
		    Format::kRGBA16Float, { ResourceUsage::kColorAttachment, ResourceUsage::kSampled } );
		const TextureId normalRoughness = target(
		    Format::kRGBA16Float, { ResourceUsage::kColorAttachment, ResourceUsage::kSampled } );
		const TextureId iblRadiance = target(
		    Format::kRGBA16Float, { ResourceUsage::kColorAttachment, ResourceUsage::kSampled } );
		const TextureId specularWeight = target(
		    Format::kRGBA16Float, { ResourceUsage::kColorAttachment, ResourceUsage::kSampled } );
		const TextureId reflected = target( Format::kRGBA16Float,
		    { ResourceUsage::kStorageWrite, ResourceUsage::kColorAttachment,
		        ResourceUsage::kCopySource, ResourceUsage::kSampled } );
		if ( !vertexBuffer.IsValid() || !indexBuffer.IsValid() || !readback.IsValid() ||
		     !color.IsValid() || !depth.IsValid() || !prepassNormal.IsValid() ||
		     !normalRoughness.IsValid() || !iblRadiance.IsValid() || !specularWeight.IsValid() ||
		     !reflected.IsValid() )
			return Fail( "a buffer or target was refused" );

		// The passes' programs: the occlusion and the reflections.
		std::unique_ptr<pass::ao::AmbientOcclusion> ambient;
		if ( aoOn )
		{
			auto created = pass::ao::AmbientOcclusion::Create( *device );
			if ( !created )
				return Fail( "the ambient occlusion pass was refused" );
			ambient = std::move( created ).Value();
		}
		std::unique_ptr<pass::ssr::ScreenSpaceReflections> reflections;
		if ( ssrOn )
		{
			auto created = pass::ssr::ScreenSpaceReflections::Create( *device, {} );
			if ( !created )
				return Fail( "the reflections pass was refused" );
			reflections = std::move( created ).Value();
		}

		// The fog's froxels: the light grid of this view (render.pass.lights,
		// its desktop limits) subdivided as render.composition's map_media.h
		// sets for the product too, the owner of the depth split.
		std::unique_ptr<pass::volumetric::VolumetricRenderer> fog;
		std::optional<pass::lights::ClusterGrid> fogGrid;
		pass::volumetric::FroxelLayout fogLayout;
		CookieArray fogCookies;
		if ( volumetric )
		{
			pass::lights::ClusterViewDesc gridView;
			gridView.view = view;
			gridView.projection = projection;
			gridView.widthPixels = options.width;
			gridView.heightPixels = options.height;
			gridView.nearZ = composition::kFroxelNearZ;
			gridView.farZ = composition::kFroxelFarZ;
			auto lightGrid =
			    pass::lights::CreateClusterGrid( gridView, pass::lights::DesktopClusterLimits() );
			if ( !lightGrid )
				return Fail( "the light grid does not build" );
			auto fine = pass::lights::SubdivideClusterGrid(
			    lightGrid.Value(), options.fogTileDivisor, options.fogSliceMultiplier );
			if ( !fine )
				return Fail( "the fog's grid does not subdivide the light grid" );
			fogGrid = std::move( fine ).Value();
			fogLayout = FroxelLayoutOf( *fogGrid );
			auto created = pass::volumetric::VolumetricRenderer::Create( *device, colorFormat );
			if ( !created )
				return Fail( "the volumetric pass was refused" );
			fog = std::move( created ).Value();
			if ( std::optional<std::string> why =
			         fogCookies.Create( *device, files, media.cookieNames ) )
				return Fail( *why );
			std::printf(
			    "render_lab: volumetric fog: %zu volumes, %zu lights (%u unsupported), %zu "
			    "projectors, froxels %ux%ux%u, density scale %g\n",
			    media.medium.volumes.size(), media.lights.size(), media.unsupportedLights,
			    media.projectors.size(), fogLayout.tilesX, fogLayout.tilesY, fogLayout.slices,
			    double( media.medium.densityScale ) );
		}
		// The game's output pass (--output-peak) into a frame of its own.
		std::unique_ptr<pass::output::OutputRenderer> output;
		TextureId displayed;
		if ( options.outputPeak > 0.0f )
		{
			auto created = pass::output::OutputRenderer::Create( *device, Format::kRGBA16Float );
			if ( !created )
				return Fail( "the output pass was refused" );
			output = std::move( created ).Value();
			displayed = target( Format::kRGBA16Float,
			    { ResourceUsage::kColorAttachment, ResourceUsage::kCopySource } );
			if ( !displayed.IsValid() )
				return Fail( "the output frame was refused" );
		}
		// The frame's final colour: the reflections' output, or the lit
		// colour without them.
		const TextureId final = reflections ? reflected : color;
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
		fogTargets.color = final;
		fogTargets.depth = depth;
		fogTargets.depthUsage = ResourceUsage::kSampled;
		fogTargets.width = options.width;
		fogTargets.height = options.height;
		fogTargets.cookies = fogCookies.Texture();

		// Record: uploads, the prepass, the occlusion, the lit frame, the
		// reflections, the fog, the readback.
		auto encoded = device->BeginEncoder( QueueKind::kGraphics );
		if ( !encoded )
			return Fail( "no encoder" );
		CommandEncoder &encoder = encoded.Value();
		cache.RecordUploads( encoder );
		groups.RecordUploads( encoder );
		if ( hasCookies )
			cookies.RecordUpload( encoder );
		if ( fog )
			fogCookies.RecordUpload( encoder );
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
		// The occlusion target is read by every pbr draw: one when no pass
		// writes it (the term's neutral value).
		encoder.TransitionTexture(
		    occlusion.Value(), ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		encoder.ClearTexture( occlusion.Value(), { 1, 1, 1, 1 } );
		encoder.TransitionTexture(
		    occlusion.Value(), ResourceUsage::kCopyDestination, ResourceUsage::kSampled );

		enum class Pass
		{
			kPrepass,
			kLit,
			kReflective
		};
		auto drawAll = [&]( Pass which, const material::FamilyDrawConstants *override ) -> bool
		{
			const bool prepass = which != Pass::kLit;
			for ( const Draw &draw : draws )
			{
				const SceneMaterial &m = materials[draw.material];
				const material::ResidentGroup *group = groups.Group( m.groupId );
				if ( !group )
				{
					status = Fail( "a material group is not resident" );
					return false;
				}
				// The prepass draws opaque surfaces only, with its variant; the
				// lit pass writes the SSR targets.
				if ( prepass && m.program.blend != BlendMode::kOpaque )
					continue;
				PipelineId pipelineId;
				if ( prepass )
				{
					auto variant = resolver.Value()->VariantPipeline( m.program,
					    which == Pass::kReflective ? material::kSurfaceRsm
					                               : material::kSurfaceDepthNormal,
					    material::kSurfaceSsrTargets );
					if ( !variant )
					{
						status = Fail( variant.Error() );
						return false;
					}
					pipelineId = variant.Value();
				}
				else
				{
					material::ResolvedProgram lit = m.program;
					if ( reflections )
					{
						auto variant = resolver.Value()->VariantPipeline(
						    m.program, material::kSurfaceSsrTargets, 0 );
						if ( !variant )
						{
							status = Fail( variant.Error() );
							return false;
						}
						lit.request.pipeline = variant.Value();
					}
					auto pipeline = resolver.Value()->DebugPipeline(
					    lit, frame::DebugSpecializationFor( options.debug, m.program.name ) );
					if ( !pipeline )
					{
						status = Fail( pipeline.Error() );
						return false;
					}
					pipelineId = pipeline.Value();
				}
				encoder.SetPipeline( pipelineId );
				if ( m.program.request.frameLayout.IsValid() )
					encoder.SetBindGroup( BindGroupRole::kFrame,
					    groups.Group( frameGroupOf[m.program.request.frameLayout.value] )->group );
				if ( m.program.request.viewLayout.IsValid() )
					encoder.SetBindGroup( BindGroupRole::kView,
					    groups.Group( viewGroupOf[m.program.request.viewLayout.value] )->group );
				encoder.SetBindGroup( BindGroupRole::kMaterial, group->group );
				if ( m.program.request.drawLayout.IsValid() )
				{
					const std::uint64_t drawGroup =
					    m.program.name == "pbr" ? pbrDrawGroup[m.mesh ? 1 : 0]
					                            : drawGroupOf[m.program.request.drawLayout.value];
					encoder.SetBindGroup( BindGroupRole::kDraw, groups.Group( drawGroup )->group );
				}
				encoder.SetVertexBuffer( 0, vertexBuffer, 0 );
				encoder.SetIndexBuffer( indexBuffer, 0, IndexFormat::kUint32 );
				encoder.SetDrawConstants(
				    0, std::as_bytes( std::span( override ? override : &constants, 1 ) )
				           .first( m.program.request.drawConstantBytes ) );
				encoder.DrawIndexed( draw.indexCount, 1, draw.firstIndex, 0, 0 );
			}
			return true;
		};

		// The projected lights' bounce: each projector's reflective shadow map
		// (the surfaces' albedo and depth through its frustum), then the
		// gather into the bounce atlas the lit pass samples.
		std::unique_ptr<pass::bounce::ProjectorBounce> bouncer;
		std::vector<TextureId> rsmTargets;
		if ( bounceOn )
		{
			encoder.TransitionTexture(
			    bounceAtlas, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
			encoder.ClearTexture( bounceAtlas, { 0, 0, 0, 0 } );
			encoder.TransitionTexture(
			    bounceAtlas, ResourceUsage::kCopyDestination, ResourceUsage::kSampled );
			auto created = pass::bounce::ProjectorBounce::Create( *device );
			if ( !created )
				return Fail( "the bounce pass was refused" );
			bouncer = std::move( created ).Value();
			const std::uint32_t rsmSize = options.rsmSize;
			std::vector<pass::bounce::ReflectiveShadowMap> maps;
			for ( std::size_t i = 0; i < lights.projectors.size(); ++i )
			{
				const projected_light::Light &light = lights.projectors[i];
				pass::shadows::FlashlightShadowDesc desc;
				desc.position = { light.origin[0], light.origin[1], light.origin[2] };
				desc.forward = { light.forward[0], light.forward[1], light.forward[2] };
				desc.up = { light.up[0], light.up[1], light.up[2] };
				desc.horizontalFovRadians = light.horizontalFovDegrees * 3.14159265f / 180.0f;
				desc.verticalFovRadians = light.verticalFovDegrees * 3.14159265f / 180.0f;
				desc.nearZ = std::max( light.nearZ, 1.0f );
				desc.farZ = light.farZ;
				auto shadowView = pass::shadows::BuildFlashlightShadowView( desc );
				if ( !shadowView )
					continue;
				TextureDesc rsmDesc;
				rsmDesc.format = Format::kRGBA16Float;
				rsmDesc.width = rsmDesc.height = rsmSize;
				rsmDesc.usages = { ResourceUsage::kColorAttachment, ResourceUsage::kSampled };
				auto albedo = device->CreateTexture( rsmDesc );
				rsmDesc.format = depthFormat;
				rsmDesc.usages = { ResourceUsage::kDepthWrite, ResourceUsage::kSampled };
				auto rsmDepth = device->CreateTexture( rsmDesc );
				if ( !albedo || !rsmDepth )
					return Fail( "a reflective shadow map was refused" );
				rsmTargets.push_back( albedo.Value() );
				rsmTargets.push_back( rsmDepth.Value() );
				encoder.TransitionTexture(
				    albedo.Value(), ResourceUsage::kUndefined, ResourceUsage::kColorAttachment );
				encoder.TransitionTexture(
				    rsmDepth.Value(), ResourceUsage::kUndefined, ResourceUsage::kDepthWrite );
				const ColorAttachment attachments[] = {
				    { albedo.Value(), LoadOp::kClear, StoreOp::kStore, { 0, 0, 0, 0 }, {} } };
				RenderingDesc rendering;
				rendering.colors = attachments;
				rendering.depth =
				    DepthAttachment{ rsmDepth.Value(), LoadOp::kClear, StoreOp::kStore, 1.0f };
				rendering.width = rendering.height = rsmSize;
				encoder.BeginRendering( rendering );
				encoder.SetViewport( { 0, 0, float( rsmSize ), float( rsmSize ), 0, 1 } );
				material::FamilyDrawConstants rsmConstants;
				std::memcpy( rsmConstants.toClip, &shadowView.Value().viewProjection,
				    sizeof( rsmConstants.toClip ) );
				const bool drawn = drawAll( Pass::kReflective, &rsmConstants );
				encoder.EndRendering();
				if ( !drawn )
					return status;
				encoder.TransitionTexture(
				    albedo.Value(), ResourceUsage::kColorAttachment, ResourceUsage::kSampled );
				encoder.TransitionTexture(
				    rsmDepth.Value(), ResourceUsage::kDepthWrite, ResourceUsage::kSampled );
				pass::bounce::ReflectiveShadowMap rsm;
				rsm.albedo = albedo.Value();
				rsm.depth = rsmDepth.Value();
				rsm.size = rsmSize;
				rsm.viewProjection = shadowView.Value().viewProjection;
				rsm.light = projectorRecords[i];
				maps.push_back( rsm );
			}
			pass::bounce::BounceInputs inputs;
			inputs.probeAtlas = cache.Find( map.probeAtlas )->texture;
			inputs.probeAtlasDesc = cache.Find( map.probeAtlas )->desc;
			inputs.probeGrids = cache.Find( map.probeGrids )->texture;
			inputs.gridCount = probeLayout.gridCount;
			for ( std::uint32_t g = 0; g < probeLayout.gridCount; ++g )
				inputs.maxProbesPerGrid = std::max( inputs.maxProbesPerGrid,
				    probeLayout.grids[g].dims[0] * probeLayout.grids[g].dims[1] *
				        probeLayout.grids[g].dims[2] );
			inputs.cookies = cookies.Texture();
			inputs.maps = maps;
			inputs.output = bounceAtlas;
			inputs.outputUsage = ResourceUsage::kSampled;
			if ( !maps.empty() && !bouncer->Record( encoder, inputs ) )
				return Fail( "the bounce pass did not record" );
		}

		// The prepass: depth, and the normal and roughness the occlusion reads.
		encoder.TransitionTexture(
		    depth, ResourceUsage::kUndefined, ResourceUsage::kDepthWrite );
		encoder.TransitionTexture(
		    prepassNormal, ResourceUsage::kUndefined, ResourceUsage::kColorAttachment );
		{
			const ColorAttachment attachments[] = {
			    { prepassNormal, LoadOp::kClear, StoreOp::kStore, { 0, 0, 1, 0 }, {} } };
			RenderingDesc rendering;
			rendering.colors = attachments;
			rendering.depth = DepthAttachment{ depth, LoadOp::kClear, StoreOp::kStore, 1.0f };
			rendering.width = options.width;
			rendering.height = options.height;
			encoder.BeginRendering( rendering );
			encoder.SetViewport( { 0, 0, float( options.width ), float( options.height ), 0, 1 } );
			const bool drawn = drawAll( Pass::kPrepass, nullptr );
			encoder.EndRendering();
			if ( !drawn )
				return status;
		}
		encoder.TransitionTexture(
		    prepassNormal, ResourceUsage::kColorAttachment, ResourceUsage::kSampled );
		if ( ambient )
		{
			encoder.TransitionTexture( depth, ResourceUsage::kDepthWrite, ResourceUsage::kSampled );
			pass::ao::AoView aoView;
			aoView.view = view;
			aoView.projection = projection;
			aoView.eye[0] = options.eye.x;
			aoView.eye[1] = options.eye.y;
			aoView.eye[2] = options.eye.z;
			pass::ao::AoTargets aoTargets;
			aoTargets.depth = depth;
			aoTargets.normalRoughness = prepassNormal;
			aoTargets.output = occlusion.Value();
			aoTargets.outputUsage = ResourceUsage::kSampled;
			aoTargets.width = options.width;
			aoTargets.height = options.height;
			if ( !ambient->Record( encoder, aoTargets, aoView ) )
				return Fail( "the ambient occlusion pass did not record" );
			encoder.TransitionTexture( depth, ResourceUsage::kSampled, ResourceUsage::kDepthWrite );
		}

		// The lit frame, over the prepass's depth.
		std::size_t drawn = draws.size();
		{
			encoder.TransitionTexture(
			    color, ResourceUsage::kUndefined, ResourceUsage::kColorAttachment );
			for ( TextureId t : { normalRoughness, iblRadiance, specularWeight } )
				encoder.TransitionTexture(
				    t, ResourceUsage::kUndefined, ResourceUsage::kColorAttachment );
			const ColorAttachment attachments[] = {
			    { color, LoadOp::kClear, StoreOp::kStore, { 0, 0, 0, 1 }, {} },
			    { normalRoughness, LoadOp::kClear, StoreOp::kStore, { 0, 0, 1, 0 }, {} },
			    { iblRadiance, LoadOp::kClear, StoreOp::kStore, { 0, 0, 0, 0 }, {} },
			    { specularWeight, LoadOp::kClear, StoreOp::kStore, { 0, 0, 0, 0 }, {} } };
			RenderingDesc rendering;
			rendering.colors = std::span<const ColorAttachment>( attachments, reflections ? 4 : 1 );
			rendering.depth = DepthAttachment{ depth, LoadOp::kLoad, StoreOp::kStore, 1.0f };
			rendering.width = options.width;
			rendering.height = options.height;
			encoder.BeginRendering( rendering );
			encoder.SetViewport( { 0, 0, float( options.width ), float( options.height ), 0, 1 } );
			const bool ok = drawAll( Pass::kLit, nullptr );
			encoder.EndRendering();
			if ( !ok )
				return status;
		}
		encoder.TransitionTexture( depth, ResourceUsage::kDepthWrite, ResourceUsage::kSampled );
		if ( reflections )
		{
			encoder.TransitionTexture( color, ResourceUsage::kColorAttachment, ResourceUsage::kSampled );
			for ( TextureId t : { normalRoughness, iblRadiance, specularWeight } )
				encoder.TransitionTexture(
				    t, ResourceUsage::kColorAttachment, ResourceUsage::kSampled );
			encoder.TransitionTexture(
			    reflected, ResourceUsage::kUndefined, ResourceUsage::kStorageWrite );
			pass::ssr::SsrDirectTargets ssrTargets;
			ssrTargets.depth = depth;
			ssrTargets.normalRoughness = normalRoughness;
			ssrTargets.iblRadiance = iblRadiance;
			ssrTargets.specularWeight = specularWeight;
			ssrTargets.lit = color;
			ssrTargets.output = reflected;
			ssrTargets.outputUsage = ResourceUsage::kStorageWrite;
			ssrTargets.width = options.width;
			ssrTargets.height = options.height;
			pass::ssr::SsrView ssrView;
			ssrView.toClip = toClip;
			ssrView.eye[0] = options.eye.x;
			ssrView.eye[1] = options.eye.y;
			ssrView.eye[2] = options.eye.z;
			if ( !reflections->Record( encoder, ssrTargets, ssrView ) )
				return Fail( "the reflections pass did not record" );
			encoder.TransitionTexture(
			    reflected, ResourceUsage::kStorageWrite, ResourceUsage::kColorAttachment );
		}
		if ( fog && options.inscatterOnly )
		{
			encoder.TransitionTexture(
			    final, ResourceUsage::kColorAttachment, ResourceUsage::kCopyDestination );
			encoder.ClearTexture( final, { 0, 0, 0, 1 } );
			encoder.TransitionTexture(
			    final, ResourceUsage::kCopyDestination, ResourceUsage::kColorAttachment );
		}
		if ( fog && status == 0 && !fog->Record( encoder, fogView, fogFrame, fogTargets ) )
			status = Fail( "the volumetric pass did not record" );
		// The game's output, when asked for: the frame's scene values to the
		// display values the game's back buffer holds.
		TextureId shown = final;
		if ( output && status == 0 )
		{
			pass::output::OutputDirectTargets outputTargets;
			outputTargets.scene = final;
			outputTargets.sceneUsage = ResourceUsage::kColorAttachment;
			outputTargets.sceneWidth = options.width;
			outputTargets.sceneHeight = options.height;
			outputTargets.target = displayed;
			outputTargets.targetUsage = ResourceUsage::kColorAttachment;
			outputTargets.width = options.width;
			outputTargets.height = options.height;
			pass::output::OutputParams outputParams;
			outputParams.scenePeak = options.outputPeak;
			encoder.TransitionTexture(
			    displayed, ResourceUsage::kUndefined, ResourceUsage::kColorAttachment );
			if ( !output->Record( encoder, outputTargets, outputParams ) )
				status = Fail( "the output pass did not record" );
			shown = displayed;
		}
		encoder.TransitionTexture(
		    shown, ResourceUsage::kColorAttachment, ResourceUsage::kCopySource );
		encoder.TransitionBuffer(
		    readback, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		encoder.CopyTextureToBuffer( shown, readback, { 0, 0, 0, options.width, options.height } );
		auto token = device->Submit( QueueKind::kGraphics, { &encoder, 1 }, {} );
		if ( !token )
			return Fail( "the frame was refused at submission" );
		(void)device->WaitIdle();
		cache.Retire( token.Value() );
		groups.Retire( token.Value() );
		meshes.Retire( token.Value() );
		if ( fog )
			fog->Collect( token.Value() );
		if ( output )
			output->Collect( token.Value() );
		if ( ambient )
			ambient->Collect( token.Value() );
		if ( reflections )
			reflections->Collect( token.Value() );
		if ( bouncer )
			bouncer->Collect( token.Value() );
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
			std::printf( "render_lab: %zu draws, %zu vertices, %zu materials%s%s -> %s\n", drawn,
			    vertices.size(), materials.size(), ambient ? ", ao" : "",
			    reflections ? ", ssr" : "", options.out.string().c_str() );
		}
		// The volumetric pass's time (perf, binding rule 7): its three
		// stages in a submission of their own, submission to idle, median
		// over the repeats.
		if ( fog && status == 0 && options.timeRepeats > 0 )
		{
			std::vector<double> times;
			for ( std::uint32_t i = 0; i < options.timeRepeats; ++i )
			{
				auto timed = device->BeginEncoder( QueueKind::kGraphics );
				if ( !timed )
					return Fail( "no encoder" );
				timed.Value().TransitionTexture(
				    final, ResourceUsage::kCopySource, ResourceUsage::kColorAttachment );
				if ( !fog->Record( timed.Value(), fogView, fogFrame, fogTargets ) )
					return Fail( "the volumetric pass did not record" );
				timed.Value().TransitionTexture(
				    final, ResourceUsage::kColorAttachment, ResourceUsage::kCopySource );
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
		for ( TextureId id : { color, depth, prepassNormal, normalRoughness, iblRadiance,
		          specularWeight, reflected, occlusion.Value() } )
			(void)device->Release( id, token.Value() );
		ReleaseShadows( *device, shadowing, token.Value() );
		for ( TextureId id : rsmTargets )
			(void)device->Release( id, token.Value() );
		if ( bounceAtlas.IsValid() )
			(void)device->Release( bounceAtlas, token.Value() );
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

// The inventory uses the renderer's actual static claim rule. Each record is
// shader\0(key\0value\0)*\0; an empty key ends it. No GPU is involved.
int ClaimBatch()
{
	const render::material::VmtProfile profile;
	std::printf( "CLAIM-BATCH/3\tdx=%d\tps20b=%d\thdr=%d\tsrgb=%d\tgpu=%d\tlowfill=%d\tsymbols=",
	    profile.dxLevel, profile.pixelShader20b, profile.hdr, profile.srgbBlending,
	    profile.gpuLevel, profile.reduceParticles );
	for ( std::size_t i = 0; i < profile.symbols.size(); ++i )
		std::printf( "%s%s", i ? "," : "", profile.symbols[i].c_str() );
	std::printf( "\n" );
	std::string shader;
	while ( std::getline( std::cin, shader, '\0' ) )
	{
		if ( shader.empty() )
			return 2;
		std::vector<render::material::VmtPair> variables;
		std::string key;
		while ( std::getline( std::cin, key, '\0' ) && !key.empty() )
		{
			std::string value;
			if ( !std::getline( std::cin, value, '\0' ) )
				return 2;
			variables.push_back( { std::move( key ), std::move( value ) } );
		}
		if ( !std::cin )
			return 2;
		const auto mapped = render::material::MapVariables( shader, std::move( variables ), {} );
		if ( !mapped )
		{
			for ( int i = 0; i < 6; ++i )
				std::printf( "%sG\timport: %s", i ? "\t" : "", mapped.Error().detail.c_str() );
			std::printf( "\t\t\t\n" );
			continue;
		}
		// Every row is the actual claim under a named scene-input combination.
		// The auditor turns the successful rows into explicit requirements.
		bool first = true;
		for ( bool probes : { false, true } )
		{
			for ( bool sceneColor : { false, true } )
			{
				if ( !first )
					std::printf( "\t" );
				first = false;
				const auto claim =
				    render::material::ClaimForMesh( mapped.Value(), probes, sceneColor );
				if ( claim )
					std::printf( "C\t%u", unsigned( claim.Value() ) );
				else
					std::printf( "G\t%s", claim.Error().c_str() );
			}
		}
		for ( bool worldStage : { false, true } )
		{
			std::printf( "\t" );
			// A world stage here is one with reflection probes (a stage map's
			// RPRB): a LightmappedGeneric env_cubemap reads them.
			const auto claim = render::material::ClaimForDrawing(
			    mapped.Value(), worldStage, nullptr, worldStage );
			if ( claim )
				std::printf( "C\t%u", unsigned( claim.Value() ) );
			else
				std::printf( "G\t%s", claim.Error().c_str() );
		}
		std::printf( "\t" );
		bool firstNumeric = true;
		constexpr std::string_view prefix = "numeric KeyValues residue ";
		for ( const std::string &diagnostic : mapped.Value().diagnostics )
		{
			if ( !diagnostic.starts_with( prefix ) )
				continue;
			std::printf( "%s%s", firstNumeric ? "" : ",", diagnostic.c_str() + prefix.size() );
			firstNumeric = false;
		}
		std::printf( "\t" );
		bool firstUnmapped = true;
		for ( const std::string &key : mapped.Value().unmapped )
		{
			std::printf( "%s%s", firstUnmapped ? "" : ",", key.c_str() );
			firstUnmapped = false;
		}
		std::printf( "\t%s\n", mapped.Value().family.c_str() );
	}
	return std::cin.bad() ? 2 : 0;
}

int main( int argc, char **argv )
{
	if ( argc >= 3 && std::string( argv[1] ) == "suite" )
		return RunSuite( argc - 2, argv + 2 );
	if ( argc == 2 && std::string( argv[1] ) == "claim-batch" )
		return ClaimBatch();
	const std::optional<Options> options = ParseOptions( argc, argv );
	if ( !options )
	{
		std::fprintf( stderr,
		    "usage: render_lab --game <dir> --map <file.bsp> --eye x,y,z --forward x,y,z --up "
		    "x,y,z --hfov degrees --size WxH --out <file.pfm> [--model <models/x.mdl> "
		    "--model-origin x,y,z] [--model-no-shadow] [--mover x0,y0,z0,x1,y1,z1,material] "
		    "[--validate] [--dump-mesh] [--core-direct] [--debug-* ...]\n"
		    "           [--fog-scale s] [--no-volumetric] [--output-peak p] [--entities portal|portal2] [--time n]\n"
		    "       render_lab suite <name> [--validate] [--seeded <defect>]\n"
		    "       render_lab claim-batch < NUL-delimited-materials\n" );
		return 2;
	}
	return Run( *options );
}
